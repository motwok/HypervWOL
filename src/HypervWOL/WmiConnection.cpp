/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * WMI Connection helper for generated classes
 */

#include "WmiConnection.h"

namespace WMI {
namespace Virtualization {

WmiConnection::WmiConnection()
    : m_comInitialized(false)
    , m_pLoc(nullptr)
    , m_pSvc(nullptr)
{
}

WmiConnection::~WmiConnection()
{
    if (m_pSvc)
    {
        m_pSvc->Release();
        m_pSvc = nullptr;
    }

    if (m_pLoc)
    {
        m_pLoc->Release();
        m_pLoc = nullptr;
    }

    if (m_comInitialized)
    {
        CoUninitialize();
        m_comInitialized = false;
    }
}

bool WmiConnection::Connect(const std::wstring& namespacePath)
{
    // Initialize COM
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (SUCCEEDED(hr))
    {
        m_comInitialized = true;
    }
    else if (hr != RPC_E_CHANGED_MODE)
    {
        return false;
    }

    // Create WMI locator
    hr = CoCreateInstance(
        CLSID_WbemLocator,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        reinterpret_cast<LPVOID*>(&m_pLoc));

    if (FAILED(hr))
        return false;

    // Connect to namespace
    hr = m_pLoc->ConnectServer(
        _bstr_t(namespacePath.c_str()),
        nullptr,
        nullptr,
        nullptr,
        0,
        nullptr,
        nullptr,
        &m_pSvc);

    if (FAILED(hr))
        return false;

    // Set security levels
    hr = CoSetProxyBlanket(
        m_pSvc,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        nullptr,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        nullptr,
        EOAC_NONE);

    return SUCCEEDED(hr);
}

} // namespace Virtualization
} // namespace WMI
