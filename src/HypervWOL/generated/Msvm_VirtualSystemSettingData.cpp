/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Auto-generated from WMI class Msvm_VirtualSystemSettingData
 * Namespace: root\virtualization\v2
 */

#include "Msvm_VirtualSystemSettingData.h"

namespace WMI {
namespace Virtualization {

// Constructors / Destructors
Msvm_VirtualSystemSettingData::Msvm_VirtualSystemSettingData()
    : m_pObject(nullptr)
{
}

Msvm_VirtualSystemSettingData::Msvm_VirtualSystemSettingData(IWbemClassObject* pObject)
    : m_pObject(pObject)
{
    if (m_pObject)
        m_pObject->AddRef();
}

Msvm_VirtualSystemSettingData::~Msvm_VirtualSystemSettingData()
{
    if (m_pObject)
    {
        m_pObject->Release();
        m_pObject = nullptr;
    }
}

// Copy constructor
Msvm_VirtualSystemSettingData::Msvm_VirtualSystemSettingData(const Msvm_VirtualSystemSettingData& other)
    : m_pObject(other.m_pObject)
{
    if (m_pObject)
        m_pObject->AddRef();
}

// Copy assignment
Msvm_VirtualSystemSettingData& Msvm_VirtualSystemSettingData::operator=(const Msvm_VirtualSystemSettingData& other)
{
    if (this != &other)
    {
        if (m_pObject)
            m_pObject->Release();
        m_pObject = other.m_pObject;
        if (m_pObject)
            m_pObject->AddRef();
    }
    return *this;
}

// Move constructor
Msvm_VirtualSystemSettingData::Msvm_VirtualSystemSettingData(Msvm_VirtualSystemSettingData&& other) noexcept
    : m_pObject(other.m_pObject)
{
    other.m_pObject = nullptr;
}

// Move assignment
Msvm_VirtualSystemSettingData& Msvm_VirtualSystemSettingData::operator=(Msvm_VirtualSystemSettingData&& other) noexcept
{
    if (this != &other)
    {
        if (m_pObject)
            m_pObject->Release();
        m_pObject = other.m_pObject;
        other.m_pObject = nullptr;
    }
    return *this;
}

// Static methods
std::vector<Msvm_VirtualSystemSettingData> Msvm_VirtualSystemSettingData::Enumerate(IWbemServices* pSvc)
{
    std::vector<Msvm_VirtualSystemSettingData> result;
    if (!pSvc) return result;

    IEnumWbemClassObject* pEnum = nullptr;
    HRESULT hr = pSvc->ExecQuery(
        _bstr_t(L"WQL"),
        _bstr_t(L"SELECT * FROM Msvm_VirtualSystemSettingData"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &pEnum);

    if (SUCCEEDED(hr))
    {
        IWbemClassObject* pObj = nullptr;
        ULONG returned = 0;
        while (pEnum->Next(WBEM_INFINITE, 1, &pObj, &returned) == WBEM_S_NO_ERROR && returned > 0)
        {
            result.emplace_back(pObj);
            pObj->Release();
        }
        pEnum->Release();
    }

    return result;
}

Msvm_VirtualSystemSettingData Msvm_VirtualSystemSettingData::GetForComputerSystem(
    IWbemServices* pSvc,
    const std::wstring& computerSystemPath)
{
    if (!pSvc || computerSystemPath.empty())
        return Msvm_VirtualSystemSettingData();

    std::wstring query = L"ASSOCIATORS OF {" + computerSystemPath +
        L"} WHERE AssocClass=Msvm_SettingsDefineState ResultClass=Msvm_VirtualSystemSettingData";

    IEnumWbemClassObject* pEnum = nullptr;
    HRESULT hr = pSvc->ExecQuery(
        _bstr_t(L"WQL"),
        _bstr_t(query.c_str()),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &pEnum);

    if (SUCCEEDED(hr))
    {
        IWbemClassObject* pObj = nullptr;
        ULONG returned = 0;
        if (pEnum->Next(WBEM_INFINITE, 1, &pObj, &returned) == WBEM_S_NO_ERROR && returned > 0)
        {
            Msvm_VirtualSystemSettingData result(pObj);
            pObj->Release();
            pEnum->Release();
            return result;
        }
        pEnum->Release();
    }

    return Msvm_VirtualSystemSettingData();
}

// Property getters
std::wstring Msvm_VirtualSystemSettingData::GetInstanceID() const
{
    return GetStringProperty(L"InstanceID");
}

std::wstring Msvm_VirtualSystemSettingData::GetCaption() const
{
    return GetStringProperty(L"Caption");
}

std::wstring Msvm_VirtualSystemSettingData::GetDescription() const
{
    return GetStringProperty(L"Description");
}

std::wstring Msvm_VirtualSystemSettingData::GetElementName() const
{
    return GetStringProperty(L"ElementName");
}

std::wstring Msvm_VirtualSystemSettingData::GetConfigurationDataRoot() const
{
    return GetStringProperty(L"ConfigurationDataRoot");
}

std::wstring Msvm_VirtualSystemSettingData::GetConfigurationFile() const
{
    return GetStringProperty(L"ConfigurationFile");
}

std::wstring Msvm_VirtualSystemSettingData::GetConfigurationID() const
{
    return GetStringProperty(L"ConfigurationID");
}

std::wstring Msvm_VirtualSystemSettingData::GetLogDataRoot() const
{
    return GetStringProperty(L"LogDataRoot");
}

std::wstring Msvm_VirtualSystemSettingData::GetRecoveryFile() const
{
    return GetStringProperty(L"RecoveryFile");
}

std::wstring Msvm_VirtualSystemSettingData::GetSnapshotDataRoot() const
{
    return GetStringProperty(L"SnapshotDataRoot");
}

std::wstring Msvm_VirtualSystemSettingData::GetSuspendDataRoot() const
{
    return GetStringProperty(L"SuspendDataRoot");
}

std::wstring Msvm_VirtualSystemSettingData::GetSwapFileDataRoot() const
{
    return GetStringProperty(L"SwapFileDataRoot");
}

std::wstring Msvm_VirtualSystemSettingData::GetVirtualSystemIdentifier() const
{
    return GetStringProperty(L"VirtualSystemIdentifier");
}

std::wstring Msvm_VirtualSystemSettingData::GetVirtualSystemType() const
{
    return GetStringProperty(L"VirtualSystemType");
}

std::wstring Msvm_VirtualSystemSettingData::GetPath() const
{
    return GetStringProperty(L"__PATH");
}

// Helper methods
std::wstring Msvm_VirtualSystemSettingData::GetStringProperty(const wchar_t* name) const
{
    if (!m_pObject)
        return L"";

    VARIANT vt;
    VariantInit(&vt);
    HRESULT hr = m_pObject->Get(name, 0, &vt, nullptr, nullptr);

    std::wstring result;
    if (SUCCEEDED(hr) && vt.vt == VT_BSTR && vt.bstrVal)
        result = vt.bstrVal;

    VariantClear(&vt);
    return result;
}

} // namespace Virtualization
} // namespace WMI
