/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Auto-generated from WMI class Msvm_SyntheticEthernetPortSettingData
 * Namespace: root\virtualization\v2
 */

#include "Msvm_SyntheticEthernetPortSettingData.h"

namespace WMI {
namespace Virtualization {

// Escapes backslashes in WQL strings
static std::wstring EscapeWql(const std::wstring& s)
{
    std::wstring out;
    out.reserve(s.size());
    for (wchar_t c : s)
        out += (c == L'\\') ? L"\\\\" : std::wstring(1, c);
    return out;
}

// Constructors / Destructors
Msvm_SyntheticEthernetPortSettingData::Msvm_SyntheticEthernetPortSettingData()
    : m_pObject(nullptr)
{
}

Msvm_SyntheticEthernetPortSettingData::Msvm_SyntheticEthernetPortSettingData(IWbemClassObject* pObject)
    : m_pObject(pObject)
{
    if (m_pObject)
        m_pObject->AddRef();
}

Msvm_SyntheticEthernetPortSettingData::~Msvm_SyntheticEthernetPortSettingData()
{
    if (m_pObject)
    {
        m_pObject->Release();
        m_pObject = nullptr;
    }
}

// Copy constructor
Msvm_SyntheticEthernetPortSettingData::Msvm_SyntheticEthernetPortSettingData(const Msvm_SyntheticEthernetPortSettingData& other)
    : m_pObject(other.m_pObject)
{
    if (m_pObject)
        m_pObject->AddRef();
}

// Copy assignment
Msvm_SyntheticEthernetPortSettingData& Msvm_SyntheticEthernetPortSettingData::operator=(const Msvm_SyntheticEthernetPortSettingData& other)
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
Msvm_SyntheticEthernetPortSettingData::Msvm_SyntheticEthernetPortSettingData(Msvm_SyntheticEthernetPortSettingData&& other) noexcept
    : m_pObject(other.m_pObject)
{
    other.m_pObject = nullptr;
}

// Move assignment
Msvm_SyntheticEthernetPortSettingData& Msvm_SyntheticEthernetPortSettingData::operator=(Msvm_SyntheticEthernetPortSettingData&& other) noexcept
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
std::vector<Msvm_SyntheticEthernetPortSettingData> Msvm_SyntheticEthernetPortSettingData::Enumerate(IWbemServices* pSvc)
{
    std::vector<Msvm_SyntheticEthernetPortSettingData> result;
    if (!pSvc) return result;

    IEnumWbemClassObject* pEnum = nullptr;
    HRESULT hr = pSvc->ExecQuery(
        _bstr_t(L"WQL"),
        _bstr_t(L"SELECT * FROM Msvm_SyntheticEthernetPortSettingData"),
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

std::vector<Msvm_SyntheticEthernetPortSettingData> Msvm_SyntheticEthernetPortSettingData::GetForVirtualSystemSettingData(
    IWbemServices* pSvc,
    const std::wstring& vssdInstanceID)
{
    std::vector<Msvm_SyntheticEthernetPortSettingData> result;
    if (!pSvc || vssdInstanceID.empty())
        return result;

    std::wstring query = L"ASSOCIATORS OF {Msvm_VirtualSystemSettingData.InstanceID='"
        + EscapeWql(vssdInstanceID)
        + L"'} WHERE ResultClass=Msvm_SyntheticEthernetPortSettingData";

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
        while (pEnum->Next(WBEM_INFINITE, 1, &pObj, &returned) == WBEM_S_NO_ERROR && returned > 0)
        {
            result.emplace_back(pObj);
            pObj->Release();
        }
        pEnum->Release();
    }

    return result;
}

// Property getters
std::wstring Msvm_SyntheticEthernetPortSettingData::GetInstanceID() const
{
    return GetStringProperty(L"InstanceID");
}

std::wstring Msvm_SyntheticEthernetPortSettingData::GetCaption() const
{
    return GetStringProperty(L"Caption");
}

std::wstring Msvm_SyntheticEthernetPortSettingData::GetDescription() const
{
    return GetStringProperty(L"Description");
}

std::wstring Msvm_SyntheticEthernetPortSettingData::GetElementName() const
{
    return GetStringProperty(L"ElementName");
}

std::wstring Msvm_SyntheticEthernetPortSettingData::GetAddress() const
{
    return GetStringProperty(L"Address");
}

std::wstring Msvm_SyntheticEthernetPortSettingData::GetStaticMacAddress() const
{
    return GetStringProperty(L"StaticMacAddress");
}

bool Msvm_SyntheticEthernetPortSettingData::GetStaticMacAddressEnabled() const
{
    return GetBoolProperty(L"StaticMacAddress");
}

std::wstring Msvm_SyntheticEthernetPortSettingData::GetVirtualSystemIdentifiers(std::vector<std::wstring>& values) const
{
    values = GetStringArrayProperty(L"VirtualSystemIdentifiers");
    return values.empty() ? L"" : values[0];
}

std::wstring Msvm_SyntheticEthernetPortSettingData::GetPath() const
{
    return GetStringProperty(L"__PATH");
}

// Helper methods
std::wstring Msvm_SyntheticEthernetPortSettingData::GetStringProperty(const wchar_t* name) const
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

bool Msvm_SyntheticEthernetPortSettingData::GetBoolProperty(const wchar_t* name) const
{
    if (!m_pObject)
        return false;

    VARIANT vt;
    VariantInit(&vt);
    HRESULT hr = m_pObject->Get(name, 0, &vt, nullptr, nullptr);

    bool result = false;
    if (SUCCEEDED(hr) && vt.vt == VT_BOOL)
        result = (vt.boolVal != VARIANT_FALSE);

    VariantClear(&vt);
    return result;
}

std::vector<std::wstring> Msvm_SyntheticEthernetPortSettingData::GetStringArrayProperty(const wchar_t* name) const
{
    std::vector<std::wstring> result;
    if (!m_pObject)
        return result;

    VARIANT vt;
    VariantInit(&vt);
    HRESULT hr = m_pObject->Get(name, 0, &vt, nullptr, nullptr);

    if (SUCCEEDED(hr) && (vt.vt & VT_ARRAY))
    {
        SAFEARRAY* psa = vt.parray;
        if (psa)
        {
            LONG lBound, uBound;
            SafeArrayGetLBound(psa, 1, &lBound);
            SafeArrayGetUBound(psa, 1, &uBound);

            for (LONG i = lBound; i <= uBound; i++)
            {
                BSTR bstr = nullptr;
                SafeArrayGetElement(psa, &i, &bstr);
                if (bstr)
                {
                    result.push_back(bstr);
                    SysFreeString(bstr);
                }
            }
        }
    }

    VariantClear(&vt);
    return result;
}

} // namespace Virtualization
} // namespace WMI
