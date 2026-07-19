#include <winsock2.h>
#include <ws2tcpip.h>
#include "HypervWOLWorker.h"

#include <comdef.h>
#include <wbemidl.h>
#include <iostream>
#include <string>
#include <cwctype>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

static constexpr int       WOL_PORT        = 9;
static constexpr int       WOL_PACKET_MIN  = 102;
static constexpr ULONGLONG WOL_COOLDOWN_MS = 60000;
static constexpr ULONGLONG VM_CACHE_TTL_MS = 300000;

// Escapes backslashes in a WMI InstanceID for use inside WQL strings.
static std::wstring EscapeWql(const std::wstring& s)
{
    std::wstring out;
    out.reserve(s.size());
    for (wchar_t c : s)
        out += (c == L'\\') ? L"\\\\\\\\" : std::wstring(1, c);
    return out;
}

// Connects to root\virtualization\v2 and sets proxy blanket.
static bool WmiConnect(IWbemLocator*& pLoc, IWbemServices*& pSvc)
{
    HRESULT hr = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, reinterpret_cast<LPVOID*>(&pLoc));
    if (FAILED(hr)) return false;
    hr = pLoc->ConnectServer(_bstr_t(L"root\\virtualization\\v2"),
        nullptr, nullptr, nullptr, 0, nullptr, nullptr, &pSvc);
    if (FAILED(hr)) { pLoc->Release(); pLoc = nullptr; return false; }
    CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr,
        RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
    return true;
}

static std::wstring VariantToWString(const VARIANT& value)
{
    switch (value.vt)
    {
    case VT_BSTR:
        return value.bstrVal ? value.bstrVal : L"";
    case VT_NULL:
    case VT_EMPTY:
        return L"";
    default:
        return L"";
    }
}

static bool TryParseMacString(const std::wstring& input, std::array<unsigned char, 6>& macOut)
{
    std::wstring hexOnly;
    hexOnly.reserve(input.size());
    for (wchar_t c : input)
    {
        if (iswxdigit(c))
            hexOnly += c;
    }
    if (hexOnly.size() != 12)
        return false;

    for (int b = 0; b < 6; ++b)
    {
        wchar_t hex[3] = { hexOnly[b * 2], hexOnly[b * 2 + 1], L'\0' };
        wchar_t* end = nullptr;
        const unsigned long val = wcstoul(hex, &end, 16);
        if (end != hex + 2)
            return false;
        macOut[b] = static_cast<unsigned char>(val);
    }
    return true;
}

std::wstring HypervWOLWorker::Trim(const std::wstring& value)
{
    size_t start = 0;
    while (start < value.size() && iswspace(value[start]))
        ++start;
    size_t end = value.size();
    while (end > start && iswspace(value[end - 1]))
        --end;
    return value.substr(start, end - start);
}

bool HypervWOLWorker::ParseListenSpec(const std::wstring& listenSpec, std::vector<ListenEndpoint>& endpoints)
{
    endpoints.clear();

    if (listenSpec.empty())
    {
        endpoints.push_back({ L"0.0.0.0", 9 });
        return true;
    }

    size_t start = 0;
    while (start <= listenSpec.size())
    {
        size_t end = listenSpec.find_first_of(L",;", start);
        std::wstring token = Trim(listenSpec.substr(start, end == std::wstring::npos ? std::wstring::npos : end - start));
        if (!token.empty())
        {
            const size_t colon = token.find(L':');
            std::wstring ip = colon == std::wstring::npos ? token : token.substr(0, colon);
            std::wstring portText = colon == std::wstring::npos ? L"" : token.substr(colon + 1);
            ip = Trim(ip);
            portText = Trim(portText);
            if (ip.empty())
                ip = L"0.0.0.0";

            unsigned long port = 8;
            if (!portText.empty())
            {
                wchar_t* endPtr = nullptr;
                port = wcstoul(portText.c_str(), &endPtr, 10);
                if (endPtr == portText.c_str() || *endPtr != L'\0' || port == 0 || port > 65535)
                {
                    std::wcerr << L"[Config] Ungueltiger Port in Listen-Eintrag: '" << token << L"'" << std::endl;
                    if (end == std::wstring::npos)
                        break;
                    start = end + 1;
                    continue;
                }
            }

            endpoints.push_back({ ip, static_cast<unsigned short>(port) });
        }

        if (end == std::wstring::npos)
            break;
        start = end + 1;
    }

    if (endpoints.empty())
        endpoints.push_back({ L"0.0.0.0", 9 });

    return true;
}

// ---------------------------------------------------------------------------
// ParseWolPacket
// Validates a WOL magic packet (6x0xFF + 16xMAC) and extracts the MAC.
// ---------------------------------------------------------------------------
bool HypervWOLWorker::ParseWolPacket(const unsigned char* data, int len, unsigned char macOut[6])
{
    if (len < WOL_PACKET_MIN) return false;
    for (int i = 0; i < 6; ++i)
        if (data[i] != 0xFF) return false;
    memcpy(macOut, data + 6, 6);
    for (int rep = 0; rep < 16; ++rep)
        if (memcmp(macOut, data + 6 + rep * 6, 6) != 0) return false;
    return true;
}

// ---------------------------------------------------------------------------
// RefreshVmCache
// Queries Hyper-V WMI for all VMs and their virtual NIC MAC addresses.
// Rebuilds m_vmCache (MAC -> VmInfo) and updates m_cacheTimestamp.
//
// WMI traversal per VM:
//   Msvm_ComputerSystem
//     +-- Msvm_VirtualSystemSettingData  (via Msvm_SettingsDefineState)
//           +-- Msvm_SyntheticEthernetPortSettingData  (via Msvm_VirtualSystemSettingDataComponent)
// ---------------------------------------------------------------------------
bool HypervWOLWorker::RefreshVmCache()
{
    std::map<MacKey, VmInfo> newCache;
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    const bool comOwned = SUCCEEDED(hr);
    IWbemLocator*  pLoc = nullptr;
    IWbemServices* pSvc = nullptr;
    if (!WmiConnect(pLoc, pSvc))
    {
        if (comOwned) CoUninitialize();
        std::wcerr << L"[Cache] WMI connect failed." << std::endl;
        return false;
    }
    wchar_t hostComputerName[MAX_COMPUTERNAME_LENGTH + 1] = {};
    DWORD hostComputerNameLen = static_cast<DWORD>(std::size(hostComputerName));
    GetComputerNameW(hostComputerName, &hostComputerNameLen);

    IEnumWbemClassObject* pVmEnum = nullptr;
    hr = pSvc->ExecQuery(_bstr_t(L"WQL"),
        _bstr_t(L"SELECT * FROM Msvm_ComputerSystem"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pVmEnum);
    if (FAILED(hr))
    {
        pSvc->Release(); pLoc->Release();
        if (comOwned) CoUninitialize();
        std::wcerr << L"[Cache] VM query failed." << std::endl;
        return false;
    }
    IWbemClassObject* pVm = nullptr;
    ULONG returned = 0;
    while (pVmEnum->Next(WBEM_INFINITE, 1, &pVm, &returned) == WBEM_S_NO_ERROR && returned > 0)
    {
        VARIANT vtName; VariantInit(&vtName);
        VARIANT vtPath; VariantInit(&vtPath);
        VARIANT vtCaption; VariantInit(&vtCaption);
        pVm->Get(L"ElementName", 0, &vtName, nullptr, nullptr);
        pVm->Get(L"__PATH",      0, &vtPath, nullptr, nullptr);
        pVm->Get(L"Name",        0, &vtCaption, nullptr, nullptr);
        pVm->Release();
        const std::wstring vmName    = (vtName.bstrVal && vtName.bstrVal[0]) ? vtName.bstrVal : L"<unknown>";
        const std::wstring vmWmiPath = vtPath.bstrVal ? vtPath.bstrVal : L"";
        const std::wstring vmSystemName = vtCaption.bstrVal ? vtCaption.bstrVal : L"";
        VariantClear(&vtName); VariantClear(&vtPath); VariantClear(&vtCaption);
        if (vmWmiPath.empty()) continue;
        if (_wcsicmp(vmSystemName.c_str(), hostComputerName) == 0)
            continue;
        IEnumWbemClassObject* pVssdEnum = nullptr;
        const std::wstring vssdQ = L"ASSOCIATORS OF {" + vmWmiPath
            + L"} WHERE AssocClass=Msvm_SettingsDefineState ResultClass=Msvm_VirtualSystemSettingData";
        hr = pSvc->ExecQuery(_bstr_t(L"WQL"), _bstr_t(vssdQ.c_str()),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pVssdEnum);
        if (FAILED(hr)) continue;
        IWbemClassObject* pVssd = nullptr;
        if (pVssdEnum->Next(WBEM_INFINITE, 1, &pVssd, &returned) != WBEM_S_NO_ERROR || returned == 0)
        { pVssdEnum->Release(); continue; }
        pVssdEnum->Release();
        VARIANT vtVssdId; VariantInit(&vtVssdId);
        pVssd->Get(L"InstanceID", 0, &vtVssdId, nullptr, nullptr);
        const std::wstring vssdId = vtVssdId.bstrVal ? vtVssdId.bstrVal : L"";
        VariantClear(&vtVssdId); pVssd->Release();
        if (vssdId.empty()) continue;
        IEnumWbemClassObject* pNicEnum = nullptr;
        const std::wstring nicQ = L"ASSOCIATORS OF {Msvm_VirtualSystemSettingData.InstanceID='"
            + EscapeWql(vssdId)
            + L"'} WHERE ResultClass=Msvm_SyntheticEthernetPortSettingData";
        hr = pSvc->ExecQuery(_bstr_t(L"WQL"), _bstr_t(nicQ.c_str()),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pNicEnum);
        if (FAILED(hr)) continue;
        IWbemClassObject* pNic = nullptr;
        while (pNicEnum->Next(WBEM_INFINITE, 1, &pNic, &returned) == WBEM_S_NO_ERROR && returned > 0)
        {
            MacKey key{};
            bool parsed = false;
            std::wstring macText;

            VARIANT vtAddr; VariantInit(&vtAddr);
            pNic->Get(L"Address", 0, &vtAddr, nullptr, nullptr);
            macText = VariantToWString(vtAddr);
            VariantClear(&vtAddr);
            parsed = TryParseMacString(macText, key);

            if (!parsed)
            {
                VARIANT vtStaticMac; VariantInit(&vtStaticMac);
                pNic->Get(L"StaticMacAddress", 0, &vtStaticMac, nullptr, nullptr);
                macText = VariantToWString(vtStaticMac);
                VariantClear(&vtStaticMac);
                parsed = TryParseMacString(macText, key);
            }

            pNic->Release();

            if (!parsed)
            {
                std::wcout << L"[Cache] VM '" << vmName << L"': keine MAC-Adresse in der Hyper-V-WMI gefunden." << std::endl;
                continue;
            }

            newCache[key] = { vmName, vmWmiPath };
            std::wcout << L"[Cache] MAC " << macText
                       << L" -> VM '" << vmName << L"'" << std::endl;
        }
        pNicEnum->Release();
    }
    pVmEnum->Release();
    pSvc->Release(); pLoc->Release();
    if (comOwned) CoUninitialize();

    {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        m_vmCache.swap(newCache);
        m_cacheTimestamp = GetTickCount64();
    }

    std::wcout << L"[Cache] " << m_vmCache.size()
               << L" MAC entries cached, TTL " << (VM_CACHE_TTL_MS / 60000) << L" min." << std::endl;
    return !m_vmCache.empty();
}

// ---------------------------------------------------------------------------
// StartVm
// Calls Msvm_ComputerSystem.RequestStateChange(2) for the given VM.
// ---------------------------------------------------------------------------
bool HypervWOLWorker::StartVm(const VmInfo& info)
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    const bool comOwned = SUCCEEDED(hr);
    IWbemLocator*  pLoc = nullptr;
    IWbemServices* pSvc = nullptr;
    if (!WmiConnect(pLoc, pSvc)) { if (comOwned) CoUninitialize(); return false; }
    IWbemClassObject* pVm = nullptr;
    hr = pSvc->GetObjectW(_bstr_t(info.wmiPath.c_str()), 0, nullptr, &pVm, nullptr);
    if (FAILED(hr)) { pSvc->Release(); pLoc->Release(); if (comOwned) CoUninitialize(); return false; }
    VARIANT vtState; VariantInit(&vtState);
    pVm->Get(L"EnabledState", 0, &vtState, nullptr, nullptr);
    const UINT16 state = vtState.uiVal;
    VariantClear(&vtState); pVm->Release();
    if (state == 2)
    {
        std::wcout << L"VM '" << info.name << L"' is already running." << std::endl;
        pSvc->Release(); pLoc->Release(); if (comOwned) CoUninitialize(); return true;
    }
    IWbemClassObject* pClass = nullptr;
    hr = pSvc->GetObjectW(_bstr_t(L"Msvm_ComputerSystem"), 0, nullptr, &pClass, nullptr);
    if (FAILED(hr)) { pSvc->Release(); pLoc->Release(); if (comOwned) CoUninitialize(); return false; }
    IWbemClassObject* pInParamsClass = nullptr;
    hr = pClass->GetMethod(L"RequestStateChange", 0, &pInParamsClass, nullptr);
    pClass->Release();
    if (FAILED(hr)) { pSvc->Release(); pLoc->Release(); if (comOwned) CoUninitialize(); return false; }
    IWbemClassObject* pInParams = nullptr;
    hr = pInParamsClass->SpawnInstance(0, &pInParams);
    pInParamsClass->Release();
    if (FAILED(hr)) { pSvc->Release(); pLoc->Release(); if (comOwned) CoUninitialize(); return false; }
    VARIANT vtReq; VariantInit(&vtReq); vtReq.vt = VT_I4; vtReq.lVal = 2;
    hr = pInParams->Put(L"RequestedState", 0, &vtReq, 0);
    if (FAILED(hr))
    {
        pInParams->Release();
        pSvc->Release(); pLoc->Release();
        if (comOwned) CoUninitialize();
        std::wcerr << L"VM '" << info.name << L"': Failed to set RequestedState (hr=0x"
                   << std::hex << hr << L")." << std::endl;
        return false;
    }
    IWbemClassObject* pOut = nullptr;
    hr = pSvc->ExecMethod(_bstr_t(info.wmiPath.c_str()),
        _bstr_t(L"RequestStateChange"), 0, nullptr, pInParams, &pOut, nullptr);
    pInParams->Release();

    if (SUCCEEDED(hr) && pOut)
    {
        VARIANT vtReturn; VariantInit(&vtReturn);
        if (SUCCEEDED(pOut->Get(L"ReturnValue", 0, &vtReturn, nullptr, nullptr)))
        {
            if (vtReturn.vt == VT_UI4 && vtReturn.ulVal == 4096)
            {
                std::wcout << L"VM '" << info.name << L"': start requested (async job started)." << std::endl;
            }
            else if (vtReturn.vt == VT_UI4 && vtReturn.ulVal == 0)
            {
                std::wcout << L"VM '" << info.name << L"': start requested." << std::endl;
            }
            else
            {
                std::wcerr << L"VM '" << info.name << L"': RequestStateChange returned "
                           << vtReturn.ulVal << L"." << std::endl;
            }
        }
        VariantClear(&vtReturn);
    }
    else if (SUCCEEDED(hr))
    {
        std::wcout << L"VM '" << info.name << L"': start requested." << std::endl;
    }
    else
    {
        std::wcerr << L"VM '" << info.name << L"': RequestStateChange failed (hr=0x"
                   << std::hex << hr << L")." << std::endl;
    }

    if (pOut) pOut->Release();
    pSvc->Release(); pLoc->Release();
    if (comOwned) CoUninitialize();
    return SUCCEEDED(hr);
}

// ---------------------------------------------------------------------------
// ListenerThreadProc
// Receives packets on a single socket until stopEvent is signalled or the socket is closed.
// ---------------------------------------------------------------------------
DWORD WINAPI HypervWOLWorker::ListenerThreadProc(LPVOID lpParam)
{
    auto* ctx = reinterpret_cast<ListenerThreadContext*>(lpParam);
    if (!ctx || !ctx->worker) return 0;

    unsigned char buf[1024];
    while (WaitForSingleObject(ctx->stopEvent, 0) != WAIT_OBJECT_0)
    {
        sockaddr_in sender{};
        int senderLen = sizeof(sender);
        int received = recvfrom(ctx->socket, reinterpret_cast<char*>(buf), sizeof(buf), 0,
            reinterpret_cast<sockaddr*>(&sender), &senderLen);
        if (received <= 0)
        {
            if (WaitForSingleObject(ctx->stopEvent, 0) == WAIT_OBJECT_0)
                break;
            const int err = WSAGetLastError();
            if (err == WSAENOTSOCK || err == WSAEINTR || err == WSAECONNRESET)
                break;
            continue;
        }

        unsigned char mac[6];
        if (!ParseWolPacket(buf, received, mac))
            continue;

        wchar_t senderIp[INET_ADDRSTRLEN];
        InetNtopW(AF_INET, &sender.sin_addr, senderIp, INET_ADDRSTRLEN);
        MacKey key{};
        memcpy(key.data(), mac, 6);

        VmInfo vmInfo;
        bool found = false;
        {
            std::lock_guard<std::mutex> lock(ctx->worker->m_cacheMutex);
            const auto cacheIt = ctx->worker->m_vmCache.find(key);
            if (cacheIt != ctx->worker->m_vmCache.end())
            {
                vmInfo = cacheIt->second;
                found = true;
            }
        }

        if (!found)
        {
            wprintf(L"WOL from %s  MAC %02X:%02X:%02X:%02X:%02X:%02X  (no VM in cache, ignored)\n",
                senderIp, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            continue;
        }

        const ULONGLONG now = GetTickCount64();
        bool inCooldown = false;
        {
            std::lock_guard<std::mutex> lock(ctx->worker->m_lastTriggerMutex);
            auto cdIt = ctx->worker->m_lastTrigger.find(key);
            if (cdIt != ctx->worker->m_lastTrigger.end() && (now - cdIt->second) < WOL_COOLDOWN_MS)
            {
                inCooldown = true;
            }
            else
            {
                ctx->worker->m_lastTrigger[key] = now;
            }
        }

        if (inCooldown)
        {
            wprintf(L"WOL from %s  MAC %02X:%02X:%02X:%02X:%02X:%02X  VM '%ls'  (cooldown, ignored)\n",
                senderIp, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
                vmInfo.name.c_str());
            continue;
        }

        wprintf(L"WOL from %s  MAC %02X:%02X:%02X:%02X:%02X:%02X  -> VM '%ls'\n",
            senderIp, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
            vmInfo.name.c_str());
        StartVm(vmInfo);
    }

    return 0;
}

// ---------------------------------------------------------------------------
// Run
// Opens one or more UDP sockets on configured interface:port entries and processes incoming WOL packets.
// Maintains a MAC->VM cache (TTL: VM_CACHE_TTL_MS) and a per-MAC cooldown.
// Blocks until stopEvent is signalled.
// ---------------------------------------------------------------------------
void HypervWOLWorker::Run(HANDLE stopEvent, const std::wstring& listenSpec)
{
    if (!stopEvent) return;
    WSADATA wsaData{};
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    { std::wcerr << L"WSAStartup failed." << std::endl; return; }

    std::vector<ListenEndpoint> endpoints;
    ParseListenSpec(listenSpec, endpoints);

    std::vector<ListenerThreadContext> contexts;
    contexts.reserve(endpoints.size());
    std::vector<HANDLE> threads;
    threads.reserve(endpoints.size());

    RefreshVmCache();

    for (const auto& endpoint : endpoints)
    {
        SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock == INVALID_SOCKET)
        {
            std::wcerr << L"socket() failed for " << endpoint.ip << L":" << endpoint.port
                       << L"  WSA error: " << WSAGetLastError() << std::endl;
            continue;
        }

        BOOL reuse = TRUE;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&reuse), sizeof(reuse));

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(endpoint.port);
        if (InetPtonW(AF_INET, endpoint.ip.c_str(), &addr.sin_addr) != 1)
        {
            std::wcerr << L"Invalid listen IP: " << endpoint.ip << std::endl;
            closesocket(sock);
            continue;
        }

        if (bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR)
        {
            std::wcerr << L"bind() failed on " << endpoint.ip << L":" << endpoint.port
                       << L"  WSA error: " << WSAGetLastError() << std::endl;
            closesocket(sock);
            continue;
        }

        std::wcout << L"WOL listener active on " << endpoint.ip << L":" << endpoint.port << std::endl;
        contexts.push_back({ this, sock, endpoint, stopEvent });
        HANDLE thread = CreateThread(nullptr, 0, ListenerThreadProc, &contexts.back(), 0, nullptr);
        if (!thread)
        {
            std::wcerr << L"Failed to create listener thread for " << endpoint.ip << L":" << endpoint.port << std::endl;
            closesocket(sock);
            contexts.pop_back();
            continue;
        }
        threads.push_back(thread);
    }

    if (threads.empty())
    {
        std::wcerr << L"No WOL listener sockets could be created." << std::endl;
        WSACleanup();
        return;
    }

    while (WaitForSingleObject(stopEvent, 1000) != WAIT_OBJECT_0)
    {
        if (GetTickCount64() - m_cacheTimestamp >= VM_CACHE_TTL_MS)
            RefreshVmCache();
    }

    for (auto& ctx : contexts)
        closesocket(ctx.socket);

    for (HANDLE thread : threads)
    {
        WaitForSingleObject(thread, INFINITE);
        CloseHandle(thread);
    }

    WSACleanup();
}
