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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "ListenerAddressList.h"
#include "MacAddress.h"
#include "VmMacCatalog.h"

#include <map>
#include <mutex>
#include <string>
#include <vector>

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
    using MacKey = MacAddress;

    using ListenEndpoint = ListenerAddressList::Endpoint;

    static DWORD WINAPI ListenerThreadProc(LPVOID lpParam);

    struct ListenerThreadContext
    {
        HypervWOLWorker* worker;
        SOCKET socket;
        ListenEndpoint endpoint;
        HANDLE stopEvent;
    };

    VmMacCatalog m_vmCatalog;
    std::map<MacKey, ULONGLONG> m_lastTrigger;
    std::mutex               m_lastTriggerMutex;
};

