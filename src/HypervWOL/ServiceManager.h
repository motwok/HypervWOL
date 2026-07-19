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

#pragma once

#include <windows.h>
#include <string>

#include "HypervWOLWorker.h"

class ServiceManager
{
public:
    ServiceManager();
    ~ServiceManager();

    bool Initialize(const std::wstring& serviceName);

    // listenSpec: comma/semicolon separated list of interface:port entries.
    // Empty string uses the default listener 0.0.0.0:9.
    void SetBindIp(const std::wstring& bindIp) { m_BindIp = bindIp; }

    void Run();
    void Stop();

private:
    static VOID WINAPI ServiceMain(DWORD argc, LPWSTR* argv);
    static DWORD WINAPI ServiceCtrlHandler(DWORD controlCode, DWORD eventType, LPVOID eventData, LPVOID context);
    static DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);

    void ReportServiceStatus(DWORD currentState, DWORD exitCode, DWORD waitHint);

    static ServiceManager* s_Instance;
    SERVICE_STATUS        m_ServiceStatus;
    SERVICE_STATUS_HANDLE m_ServiceStatusHandle;
    HANDLE                m_ServiceStopEvent;
    std::wstring          m_ServiceName;
    std::wstring          m_BindIp;      // interface IP passed to the worker
    HypervWOLWorker       m_Worker;
};
