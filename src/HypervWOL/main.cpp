#include <windows.h>
#include <iostream>
#include <string>
#include "ServiceManager.h"

ServiceManager* g_pServiceManager = nullptr;

BOOL WINAPI ConsoleCtrlHandler(DWORD ctrlType)
{
    switch (ctrlType)
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
        if (g_pServiceManager)
        {
            std::wcout << L"\nStopping service..." << std::endl;
            g_pServiceManager->Stop();
        }
        return TRUE;
    default:
        return FALSE;
    }
}

int wmain(int argc, wchar_t* argv[])
{
    const std::wstring serviceName = L"HypervWOL";

    ServiceManager serviceManager;
    g_pServiceManager = &serviceManager;

    // Parse optional -interface <listenSpec> / /interface <listenSpec> argument
    for (int i = 1; i < argc - 1; ++i)
    {
        if (wcscmp(argv[i], L"-interface") == 0 || wcscmp(argv[i], L"/interface") == 0)
        {
            serviceManager.SetBindIp(argv[i + 1]);
            break;
        }
    }

    // Check if running with console argument
    if (argc > 1 && (wcscmp(argv[1], L"-console") == 0 || wcscmp(argv[1], L"/console") == 0))
    {
        // Run in console mode for testing
        SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
        serviceManager.Run();
        return 0;
    }

    // Also allow: HypervWOL.exe -console -interface 192.168.1.10:9,0.0.0.0:8
    for (int i = 1; i < argc; ++i)
    {
        if (wcscmp(argv[i], L"-console") == 0 || wcscmp(argv[i], L"/console") == 0)
        {
            SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
            serviceManager.Run();
            return 0;
        }
    }

    // Run as a service
    if (!serviceManager.Initialize(serviceName))
    {
        DWORD error = GetLastError();
        std::wcerr << L"Failed to start service. Error: " << error << std::endl;
        return 1;
    }

    return 0;
}
