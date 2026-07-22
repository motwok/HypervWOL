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
#include <array>
#include <cwchar>
#include <cwctype>
#include <iostream>
#include <map>
#include <mutex>
#include <string>

#include "MacAddress.h"
#include "WmiConnection.h"
#include "generated/Msvm_ComputerSystem.h"
#include "generated/Msvm_VirtualSystemSettingData.h"
#include "generated/Msvm_SyntheticEthernetPortSettingData.h"

// Holds the WMI object path and display name of a Hyper-V VM.
struct VmInfo
{
    std::wstring name;    // ElementName from Msvm_ComputerSystem
    std::wstring wmiPath; // __PATH for ExecMethod calls
};

class VmMacCatalog
{
public:
    using MacKey = MacAddress::MacKey;
    using Cache = std::map<MacAddress, VmInfo>;
    using iterator = Cache::iterator;
    using const_iterator = Cache::const_iterator;

    bool Refresh()
    {
        std::map<MacAddress, VmInfo> newCache;

        WMI::Virtualization::WmiConnection conn;
        if (!conn.Connect())
        {
            std::wcerr << L"[Cache] WMI connect failed." << std::endl;
            return false;
        }

        wchar_t hostComputerName[MAX_COMPUTERNAME_LENGTH + 1] = {};
        DWORD hostComputerNameLen = MAX_COMPUTERNAME_LENGTH + 1;
        GetComputerNameW(hostComputerName, &hostComputerNameLen);

        auto vms = WMI::Virtualization::Msvm_ComputerSystem::Enumerate(conn.GetServices());

        for (auto& vm : vms)
        {
            const std::wstring vmName = vm.GetElementName();
            const std::wstring vmWmiPath = vm.GetPath();
            const std::wstring vmSystemName = vm.GetName();

            if (vmWmiPath.empty())
                continue;
            if (_wcsicmp(vmSystemName.c_str(), hostComputerName) == 0)
                continue;

            auto vssd = WMI::Virtualization::Msvm_VirtualSystemSettingData::GetForComputerSystem(
                conn.GetServices(), vmWmiPath);

            if (!vssd.IsValid())
                continue;

            const std::wstring vssdId = vssd.GetInstanceID();
            if (vssdId.empty())
                continue;

            auto nics = WMI::Virtualization::Msvm_SyntheticEthernetPortSettingData::GetForVirtualSystemSettingData(
                conn.GetServices(), vssdId);

            for (auto& nic : nics)
            {
                bool parsedAny = false;

                const std::wstring dynamicMacText = nic.GetAddress();
                MacAddress dynamicKey;
                if (TryParseMacString(dynamicMacText, dynamicKey))
                {
                    newCache[dynamicKey] = { vmName, vmWmiPath };
                    std::wcout << L"[Cache] Dynamic MAC " << dynamicMacText
                               << L" -> VM '" << vmName << L"'" << std::endl;
                    parsedAny = true;
                }

                const std::wstring staticMacText = nic.GetStaticMacAddress();
                MacAddress staticKey;
                if (TryParseMacString(staticMacText, staticKey))
                {
                    if (!parsedAny || staticKey != dynamicKey)
                    {
                        newCache[staticKey] = { vmName, vmWmiPath };
                        std::wcout << L"[Cache] Static MAC " << staticMacText
                                   << L" -> VM '" << vmName << L"'" << std::endl;
                    }
                    parsedAny = true;
                }

                if (!parsedAny)
                {
                    std::wcout << L"[Cache] VM '" << vmName << L"': keine MAC-Adresse in der Hyper-V-WMI gefunden." << std::endl;
                    continue;
                }
            }
        }

        {
            std::lock_guard<std::mutex> lock(m_cacheMutex);
            m_vmCache.swap(newCache);
            m_cacheTimestamp = GetTickCount64();
            std::wcout << L"[Cache] " << m_vmCache.size()
                       << L" MAC entries cached, TTL " << (VM_CACHE_TTL_MS / 60000) << L" min." << std::endl;
            return !m_vmCache.empty();
        }
    }

    // Map-like interface
    const VmInfo& operator[](const MacAddress& mac) const
    {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        return m_vmCache.at(mac);
    }

    bool TryGetVmInfo(const MacAddress& mac, VmInfo& vmInfo) const
    {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        const auto cacheIt = m_vmCache.find(mac);
        if (cacheIt == m_vmCache.end())
            return false;

        vmInfo = cacheIt->second;
        return true;
    }

    const_iterator find(const MacAddress& mac) const
    {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        return m_vmCache.find(mac);
    }

    const_iterator begin() const
    {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        return m_vmCache.begin();
    }

    const_iterator end() const
    {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        return m_vmCache.end();
    }

    size_t size() const
    {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        return m_vmCache.size();
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        return m_vmCache.empty();
    }

    bool contains(const MacAddress& mac) const
    {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        return m_vmCache.find(mac) != m_vmCache.end();
    }

    ULONGLONG GetLastRefreshTick() const
    {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        return m_cacheTimestamp;
    }

private:
    static constexpr ULONGLONG VM_CACHE_TTL_MS = 300000;

    static bool TryParseMacString(const std::wstring& input, MacAddress& macOut)
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
            macOut.SetByte(b, static_cast<unsigned char>(val));
        }
        return true;
    }

    std::map<MacAddress, VmInfo> m_vmCache;
    mutable std::mutex m_cacheMutex;
    ULONGLONG m_cacheTimestamp = 0;
};
