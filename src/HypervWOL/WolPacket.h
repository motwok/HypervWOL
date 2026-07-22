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
#include "MacAddress.h"

class WolPacket
{
public:
    using MacKey = MacAddress::MacKey;

    template <size_t N>
    static bool IsValid(const std::array<unsigned char, N>& data, int len)
    {
        if (len < WOL_PACKET_MIN || len > static_cast<int>(N))
            return false;

        for (int i = 0; i < 6; ++i)
        {
            if (data[i] != 0xFF)
                return false;
        }

        for (int rep = 1; rep < 16; ++rep)
        {
            if (memcmp(data.data() + 6, data.data() + 6 + rep * 6, 6) != 0)
                return false;
        }

        return true;
    }

    template <size_t N>
    static bool TryExtractMac(const std::array<unsigned char, N>& data, int len, MacAddress& macOut)
    {
        if (!IsValid(data, len))
            return false;

        memcpy(macOut.GetData().data(), data.data() + 6, 6);
        return true;
    }

private:
    static constexpr int WOL_PACKET_MIN = 102;
};
