/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * WMI Connection helper for generated classes
 */

#pragma once

#include <windows.h>
#include <wbemidl.h>
#include <comdef.h>
#include <string>

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

namespace WMI {
namespace Virtualization {

// RAII wrapper for WMI connection
class WmiConnection
{
public:
    WmiConnection();
    ~WmiConnection();

    // Connect to Hyper-V WMI namespace
    bool Connect(const std::wstring& namespacePath = L"root\\virtualization\\v2");

    // Check if connected
    bool IsConnected() const { return m_pSvc != nullptr; }

    // Get services pointer for generated classes
    IWbemServices* GetServices() const { return m_pSvc; }

private:
    // No copy
    WmiConnection(const WmiConnection&) = delete;
    WmiConnection& operator=(const WmiConnection&) = delete;

    bool m_comInitialized;
    IWbemLocator* m_pLoc;
    IWbemServices* m_pSvc;
};

} // namespace Virtualization
} // namespace WMI
