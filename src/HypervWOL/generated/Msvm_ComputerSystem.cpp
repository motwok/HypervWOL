/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Auto-generated from WMI class Msvm_ComputerSystem
 * Namespace: root\virtualization\v2
 */

#include "Msvm_ComputerSystem.h"
#include <comutil.h>

#pragma comment(lib, "comsuppw.lib")

namespace WMI {
namespace Virtualization {

// Constructors / Destructors
Msvm_ComputerSystem::Msvm_ComputerSystem()
    : m_pObject(nullptr)
{
}

Msvm_ComputerSystem::Msvm_ComputerSystem(IWbemClassObject* pObject)
    : m_pObject(pObject)
{
    if (m_pObject)
        m_pObject->AddRef();
}

Msvm_ComputerSystem::~Msvm_ComputerSystem()
{
    if (m_pObject)
    {
        m_pObject->Release();
        m_pObject = nullptr;
    }
}

// Copy constructor
Msvm_ComputerSystem::Msvm_ComputerSystem(const Msvm_ComputerSystem& other)
    : m_pObject(other.m_pObject)
{
    if (m_pObject)
        m_pObject->AddRef();
}

// Copy assignment
Msvm_ComputerSystem& Msvm_ComputerSystem::operator=(const Msvm_ComputerSystem& other)
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
Msvm_ComputerSystem::Msvm_ComputerSystem(Msvm_ComputerSystem&& other) noexcept
    : m_pObject(other.m_pObject)
{
    other.m_pObject = nullptr;
}

// Move assignment
Msvm_ComputerSystem& Msvm_ComputerSystem::operator=(Msvm_ComputerSystem&& other) noexcept
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
std::vector<Msvm_ComputerSystem> Msvm_ComputerSystem::Enumerate(IWbemServices* pSvc)
{
    std::vector<Msvm_ComputerSystem> result;
    if (!pSvc) return result;

    IEnumWbemClassObject* pEnum = nullptr;
    HRESULT hr = pSvc->ExecQuery(
        _bstr_t(L"WQL"),
        _bstr_t(L"SELECT * FROM Msvm_ComputerSystem"),
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

Msvm_ComputerSystem Msvm_ComputerSystem::Get(IWbemServices* pSvc, const std::wstring& name)
{
    if (!pSvc || name.empty())
        return Msvm_ComputerSystem();

    std::wstring query = L"SELECT * FROM Msvm_ComputerSystem WHERE Name='" + name + L"'";
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
            Msvm_ComputerSystem result(pObj);
            pObj->Release();
            pEnum->Release();
            return result;
        }
        pEnum->Release();
    }

    return Msvm_ComputerSystem();
}

// Property getters
std::wstring Msvm_ComputerSystem::GetCaption() const
{
    return GetStringProperty(L"Caption");
}

std::wstring Msvm_ComputerSystem::GetDescription() const
{
    return GetStringProperty(L"Description");
}

std::wstring Msvm_ComputerSystem::GetElementName() const
{
    return GetStringProperty(L"ElementName");
}

std::wstring Msvm_ComputerSystem::GetInstallDate() const
{
    return GetStringProperty(L"InstallDate");
}

std::wstring Msvm_ComputerSystem::GetName() const
{
    return GetStringProperty(L"Name");
}

std::wstring Msvm_ComputerSystem::GetStatus() const
{
    return GetStringProperty(L"Status");
}

UINT16 Msvm_ComputerSystem::GetEnabledState() const
{
    return GetUint16Property(L"EnabledState");
}

UINT16 Msvm_ComputerSystem::GetHealthState() const
{
    return GetUint16Property(L"HealthState");
}

UINT16 Msvm_ComputerSystem::GetOperationalStatus(std::vector<UINT16>& values) const
{
    values = GetUint16ArrayProperty(L"OperationalStatus");
    return values.empty() ? 0 : values[0];
}

UINT16 Msvm_ComputerSystem::GetRequestedState() const
{
    return GetUint16Property(L"RequestedState");
}

std::wstring Msvm_ComputerSystem::GetCreationClassName() const
{
    return GetStringProperty(L"CreationClassName");
}

std::wstring Msvm_ComputerSystem::GetNameFormat() const
{
    return GetStringProperty(L"NameFormat");
}

std::wstring Msvm_ComputerSystem::GetPrimaryOwnerContact() const
{
    return GetStringProperty(L"PrimaryOwnerContact");
}

std::wstring Msvm_ComputerSystem::GetPrimaryOwnerName() const
{
    return GetStringProperty(L"PrimaryOwnerName");
}

std::vector<std::wstring> Msvm_ComputerSystem::GetRoles() const
{
    return GetStringArrayProperty(L"Roles");
}

std::wstring Msvm_ComputerSystem::GetPath() const
{
    return GetStringProperty(L"__PATH");
}

// Methods
Msvm_ComputerSystem::RequestStateChangeResult Msvm_ComputerSystem::RequestStateChange(
    IWbemServices* pSvc,
    RequestedStateValue requestedState,
    FILETIME* timeoutPeriod)
{
    RequestStateChangeResult result = { 0, nullptr };

    if (!pSvc || !m_pObject)
        return result;

    // Get class definition
    IWbemClassObject* pClass = nullptr;
    HRESULT hr = pSvc->GetObject(_bstr_t(L"Msvm_ComputerSystem"), 0, nullptr, &pClass, nullptr);
    if (FAILED(hr))
        return result;

    // Get method
    IWbemClassObject* pInParamsDefinition = nullptr;
    hr = pClass->GetMethod(L"RequestStateChange", 0, &pInParamsDefinition, nullptr);
    pClass->Release();
    if (FAILED(hr))
        return result;

    // Spawn instance
    IWbemClassObject* pInParams = nullptr;
    hr = pInParamsDefinition->SpawnInstance(0, &pInParams);
    pInParamsDefinition->Release();
    if (FAILED(hr))
        return result;

    // Set parameters
    VARIANT vtRequestedState;
    VariantInit(&vtRequestedState);
    vtRequestedState.vt = VT_I4;
    vtRequestedState.lVal = requestedState;
    pInParams->Put(L"RequestedState", 0, &vtRequestedState, 0);
    VariantClear(&vtRequestedState);

    if (timeoutPeriod)
    {
        VARIANT vtTimeout;
        VariantInit(&vtTimeout);
        // Convert FILETIME to datetime string
        // Simplified - in real implementation convert properly
        vtTimeout.vt = VT_NULL;
        pInParams->Put(L"TimeoutPeriod", 0, &vtTimeout, 0);
        VariantClear(&vtTimeout);
    }

    // Execute method
    std::wstring path = GetPath();
    IWbemClassObject* pOutParams = nullptr;
    hr = pSvc->ExecMethod(
        _bstr_t(path.c_str()),
        _bstr_t(L"RequestStateChange"),
        0,
        nullptr,
        pInParams,
        &pOutParams,
        nullptr);

    pInParams->Release();

    if (SUCCEEDED(hr) && pOutParams)
    {
        VARIANT vtReturn;
        VariantInit(&vtReturn);
        pOutParams->Get(L"ReturnValue", 0, &vtReturn, nullptr, nullptr);
        if (vtReturn.vt == VT_I4 || vtReturn.vt == VT_UI4)
            result.ReturnValue = vtReturn.ulVal;
        VariantClear(&vtReturn);

        VARIANT vtJob;
        VariantInit(&vtJob);
        pOutParams->Get(L"Job", 0, &vtJob, nullptr, nullptr);
        if (vtJob.vt == VT_UNKNOWN && vtJob.punkVal)
        {
            vtJob.punkVal->QueryInterface(IID_IWbemClassObject, (void**)&result.Job);
        }
        VariantClear(&vtJob);

        pOutParams->Release();
    }

    return result;
}

// Helper methods
std::wstring Msvm_ComputerSystem::GetStringProperty(const wchar_t* name) const
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

UINT16 Msvm_ComputerSystem::GetUint16Property(const wchar_t* name) const
{
    if (!m_pObject)
        return 0;

    VARIANT vt;
    VariantInit(&vt);
    HRESULT hr = m_pObject->Get(name, 0, &vt, nullptr, nullptr);

    UINT16 result = 0;
    if (SUCCEEDED(hr) && (vt.vt == VT_I2 || vt.vt == VT_UI2))
        result = vt.uiVal;

    VariantClear(&vt);
    return result;
}

std::vector<std::wstring> Msvm_ComputerSystem::GetStringArrayProperty(const wchar_t* name) const
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

std::vector<UINT16> Msvm_ComputerSystem::GetUint16ArrayProperty(const wchar_t* name) const
{
    std::vector<UINT16> result;
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
                UINT16 value = 0;
                SafeArrayGetElement(psa, &i, &value);
                result.push_back(value);
            }
        }
    }

    VariantClear(&vt);
    return result;
}

} // namespace Virtualization
} // namespace WMI
