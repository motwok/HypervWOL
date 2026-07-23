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

#include "Config.h"
#include <cstring>

static bool IsFlag(const wchar_t* arg, const wchar_t* name)
{
    return (wcscmp(arg, (std::wstring(L"-") + name).c_str()) == 0 ||
            wcscmp(arg, (std::wstring(L"/") + name).c_str()) == 0);
}

Config Config::FromArgs(int argc, wchar_t* argv[])
{
    Config cfg;

    for (int i = 1; i < argc; ++i)
    {
        if (IsFlag(argv[i], L"console"))
            cfg.consoleMode = true;
        else
            cfg.endpoints.Add(argv[i]);
    }

    if (cfg.endpoints.empty())
        cfg.endpoints.Add(L"0.0.0.0:9");

    return cfg;
}
