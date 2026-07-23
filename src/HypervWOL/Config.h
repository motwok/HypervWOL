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
#include <string>
#include "ListenerAddressList.h"

// Holds all command-line settings for HypervWOL.
// Does not contain a parsed endpoint list; callers build a ListenerAddressList
// from listenSpec when needed.
struct Config
{
    bool               consoleMode = false;
    ListenerAddressList endpoints;           // parsed interface:port entries
    std::wstring        serviceName = L"HypervWOL";

    // Parses argc/argv and returns a populated Config.
    static Config FromArgs(int argc, wchar_t* argv[]);

    // Returns true when the config is usable:
    //   - serviceName is not empty
    //   - endpoints contains at least one entry
    //   - endpoints does not exceed MAXIMUM_WAIT_OBJECTS - 1 (one slot reserved for the stop event)
    bool IsValid() const
    {
        return !serviceName.empty()
            && !endpoints.empty()
            && endpoints.size() <= static_cast<size_t>(MAXIMUM_WAIT_OBJECTS - 1);
    }
};
