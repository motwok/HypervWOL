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

#include <array>
#include <cstring>
#include <string>

class MacAddress
{
public:
    static constexpr size_t LENGTH = 6;
    using MacKey = std::array<unsigned char, LENGTH>;

    MacAddress() : m_data{} {}

    explicit MacAddress(const MacKey& data) : m_data(data) {}

    MacAddress(unsigned char b0, unsigned char b1, unsigned char b2,
               unsigned char b3, unsigned char b4, unsigned char b5)
        : m_data{b0, b1, b2, b3, b4, b5}
    {
    }

    const MacKey& GetData() const
    {
        return m_data;
    }

    MacKey& GetData()
    {
        return m_data;
    }

    unsigned char GetByte(size_t index) const
    {
        if (index >= LENGTH)
            throw std::out_of_range("MacAddress index out of range");
        return m_data[index];
    }

    void SetByte(size_t index, unsigned char value)
    {
        if (index >= LENGTH)
            throw std::out_of_range("MacAddress index out of range");
        m_data[index] = value;
    }

    bool operator==(const MacAddress& other) const
    {
        return m_data == other.m_data;
    }

    bool operator!=(const MacAddress& other) const
    {
        return !(*this == other);
    }

    bool operator<(const MacAddress& other) const
    {
        return m_data < other.m_data;
    }

    std::wstring ToString() const
    {
        wchar_t buffer[18];
        swprintf_s(buffer, sizeof(buffer) / sizeof(wchar_t),
                   L"%02X-%02X-%02X-%02X-%02X-%02X",
                   m_data[0], m_data[1], m_data[2],
                   m_data[3], m_data[4], m_data[5]);
        return std::wstring(buffer);
    }

    static bool TryParse(const std::wstring& input, MacAddress& result)
    {
        unsigned int bytes[6];
        if (swscanf_s(input.c_str(), L"%02X-%02X-%02X-%02X-%02X-%02X",
                      &bytes[0], &bytes[1], &bytes[2],
                      &bytes[3], &bytes[4], &bytes[5]) != 6)
        {
            return false;
        }

        for (int i = 0; i < 6; ++i)
        {
            if (bytes[i] > 0xFF)
                return false;
            result.m_data[i] = static_cast<unsigned char>(bytes[i]);
        }

        return true;
    }

private:
    MacKey m_data;
};
