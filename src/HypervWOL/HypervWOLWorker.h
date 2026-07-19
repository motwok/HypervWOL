#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <array>
#include <map>
#include <mutex>
#include <string>
#include <vector>

// Holds the WMI object path and display name of a Hyper-V VM.
struct VmInfo
{
    std::wstring name;    // ElementName from Msvm_ComputerSystem
    std::wstring wmiPath; // __PATH for ExecMethod calls
};

// Listens for WOL magic packets on one or more interface:port entries.
// A MAC→VM cache is built from Hyper-V WMI and refreshed every VM_CACHE_TTL_MS.
// An incoming WOL packet is only acted on when its MAC is present in the cache.
// Repeated packets for the same MAC are suppressed for WOL_COOLDOWN_MS.
class HypervWOLWorker
{
public:
    // listenSpec: comma/semicolon separated list of entries in the form ip:port.
    // If empty, defaults to L"0.0.0.0:9".
    void Run(HANDLE stopEvent, const std::wstring& listenSpec = L"");

private:
    using MacKey = std::array<unsigned char, 6>;

    struct ListenEndpoint
    {
        std::wstring ip;
        unsigned short port;
    };

    // Validates a WOL magic packet and extracts the target MAC into macOut.
    static bool ParseWolPacket(const unsigned char* data, int len, unsigned char macOut[6]);

    static bool ParseListenSpec(const std::wstring& listenSpec, std::vector<ListenEndpoint>& endpoints);
    static std::wstring Trim(const std::wstring& value);
    static DWORD WINAPI ListenerThreadProc(LPVOID lpParam);

    // Queries Hyper-V WMI and rebuilds m_vmCache (MAC → VmInfo).
    // Returns true when at least one VM was found.
    bool RefreshVmCache();

    // Requests start of the VM identified by info via WMI RequestStateChange(2).
    // Returns true when the call succeeded.
    static bool StartVm(const VmInfo& info);

    struct ListenerThreadContext
    {
        HypervWOLWorker* worker;
        SOCKET socket;
        ListenEndpoint endpoint;
        HANDLE stopEvent;
    };

    // MAC address → VM info; rebuilt by RefreshVmCache every VM_CACHE_TTL_MS.
    std::map<MacKey, VmInfo> m_vmCache;
    std::mutex               m_cacheMutex;
    std::map<MacKey, ULONGLONG> m_lastTrigger;
    std::mutex               m_lastTriggerMutex;
    ULONGLONG                m_cacheTimestamp = 0; // GetTickCount64() of last refresh
};

