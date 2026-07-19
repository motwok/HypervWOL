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
