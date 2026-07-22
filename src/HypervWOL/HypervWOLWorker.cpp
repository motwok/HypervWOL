/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Copyright (C) 2026 Emmo "mo2000" Emminghaus mo2000 at mo2000 dot de
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <winsock2.h>
#include <ws2tcpip.h>
#include "HypervWOLWorker.h"

// Strongly-typed WMI classes generated from Hyper-V WMI schema
// These classes provide compile-time type safety for all WMI operations
#include "WmiConnection.h"
#include "generated/Msvm_ComputerSystem.h"

#include <iostream>
#include <string>
#include <cwctype>

#pragma comment(lib, "ws2_32.lib")

static constexpr int       WOL_PACKET_MIN  = 102;
static constexpr ULONGLONG WOL_COOLDOWN_MS = 60000;
static constexpr ULONGLONG VM_CACHE_TTL_MS = 300000;

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

            unsigned long port = 9;
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
// StartVm
// Calls Msvm_ComputerSystem.RequestStateChange(2) for the given VM.
// Uses strongly-typed generated WMI class - NO dynamic calls!
// ---------------------------------------------------------------------------
bool HypervWOLWorker::StartVm(const VmInfo& info)
{
    // Connect to WMI
    WMI::Virtualization::WmiConnection conn;
    if (!conn.Connect())
        return false;

    // Get VM object using strongly-typed class
    auto vm = WMI::Virtualization::Msvm_ComputerSystem::Get(conn.GetServices(), info.name);
    if (!vm.IsValid())
        return false;

    // Check current state - strongly typed enum!
    if (vm.GetEnabledState() == WMI::Virtualization::Msvm_ComputerSystem::Enabled)
    {
        std::wcout << L"VM '" << info.name << L"' is already running." << std::endl;
        return true;
    }

    // Request state change - strongly typed method call!
    auto result = vm.RequestStateChange(
        conn.GetServices(),
        WMI::Virtualization::Msvm_ComputerSystem::Enabled_Requested);

    if (result.ReturnValue == 4096)
    {
        std::wcout << L"VM '" << info.name << L"': start requested (async job started)." << std::endl;
        if (result.Job)
            result.Job->Release();
        return true;
    }
    else if (result.ReturnValue == 0)
    {
        std::wcout << L"VM '" << info.name << L"': start requested." << std::endl;
        return true;
    }
    else
    {
        std::wcerr << L"VM '" << info.name << L"': RequestStateChange returned "
                   << result.ReturnValue << L"." << std::endl;
        return false;
    }
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
        const bool found = ctx->worker->m_vmCatalog.TryGetVmInfo(key, vmInfo);

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

    m_vmCatalog.Refresh();

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
        if (GetTickCount64() - m_vmCatalog.GetLastRefreshTick() >= VM_CACHE_TTL_MS)
            m_vmCatalog.Refresh();
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
