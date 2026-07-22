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

#include <cwctype>
#include <iostream>
#include <string>
#include <vector>

class ListenerAddressList
{
public:
    struct Endpoint
    {
        std::wstring ip;
        unsigned short port;
    };

    ListenerAddressList(const std::wstring& listenSpec = L"")
    {
        Parse(listenSpec);
    }

    bool Parse(const std::wstring& listenSpec)
    {
        m_endpoints.clear();

        if (listenSpec.empty())
        {
            m_endpoints.push_back({ L"0.0.0.0", 9 });
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

                m_endpoints.push_back({ ip, static_cast<unsigned short>(port) });
            }

            if (end == std::wstring::npos)
                break;
            start = end + 1;
        }

        if (m_endpoints.empty())
            m_endpoints.push_back({ L"0.0.0.0", 9 });

        return true;
    }

    size_t size() const { return m_endpoints.size(); }
    bool empty() const { return m_endpoints.empty(); }

    const Endpoint& operator[](size_t index) const { return m_endpoints[index]; }

    std::vector<Endpoint>::const_iterator begin() const { return m_endpoints.begin(); }
    std::vector<Endpoint>::const_iterator end() const { return m_endpoints.end(); }

    const std::vector<Endpoint>& GetEndpoints() const { return m_endpoints; }

private:
    static std::wstring Trim(const std::wstring& value)
    {
        size_t start = 0;
        while (start < value.size() && iswspace(value[start]))
            ++start;
        size_t end = value.size();
        while (end > start && iswspace(value[end - 1]))
            --end;
        return value.substr(start, end - start);
    }

    std::vector<Endpoint> m_endpoints;
};
