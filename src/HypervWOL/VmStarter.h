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

#include "VmMacCatalog.h"

#include "WmiConnection.h"
#include "generated/Msvm_ComputerSystem.h"

#include <iostream>

class VmStarter
{
public:
    static bool StartVm(const VmInfo& info)
    {
        WMI::Virtualization::WmiConnection conn;
        if (!conn.Connect())
            return false;

        auto vm = WMI::Virtualization::Msvm_ComputerSystem::Get(conn.GetServices(), info.name);
        if (!vm.IsValid())
            return false;

        if (vm.GetEnabledState() == WMI::Virtualization::Msvm_ComputerSystem::Enabled)
        {
            std::wcout << L"VM '" << info.name << L"' is already running." << std::endl;
            return true;
        }

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
};
