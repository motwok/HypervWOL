/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Auto-generated from WMI class Msvm_SyntheticEthernetPortSettingData
 * Namespace: root\virtualization\v2
 */

#pragma once

#include <windows.h>
#include <wbemidl.h>
#include <comdef.h>
#include <string>
#include <vector>

namespace WMI {
namespace Virtualization {

// Msvm_SyntheticEthernetPortSettingData - Synthetic network adapter configuration
class Msvm_SyntheticEthernetPortSettingData
{
public:
    // Constructors
    Msvm_SyntheticEthernetPortSettingData();
    explicit Msvm_SyntheticEthernetPortSettingData(IWbemClassObject* pObject);
    ~Msvm_SyntheticEthernetPortSettingData();

    // Copy/Move
    Msvm_SyntheticEthernetPortSettingData(const Msvm_SyntheticEthernetPortSettingData& other);
    Msvm_SyntheticEthernetPortSettingData& operator=(const Msvm_SyntheticEthernetPortSettingData& other);
    Msvm_SyntheticEthernetPortSettingData(Msvm_SyntheticEthernetPortSettingData&& other) noexcept;
    Msvm_SyntheticEthernetPortSettingData& operator=(Msvm_SyntheticEthernetPortSettingData&& other) noexcept;

    // Validation
    bool IsValid() const { return m_pObject != nullptr; }

    // Static factory methods
    static std::vector<Msvm_SyntheticEthernetPortSettingData> Enumerate(IWbemServices* pSvc);
    static std::vector<Msvm_SyntheticEthernetPortSettingData> GetForVirtualSystemSettingData(
        IWbemServices* pSvc, 
        const std::wstring& vssdInstanceID);

    // Properties (strongly typed)
    std::wstring GetInstanceID() const;
    std::wstring GetCaption() const;
    std::wstring GetDescription() const;
    std::wstring GetElementName() const;

    std::wstring GetAddress() const;           // Dynamic MAC address
    std::wstring GetStaticMacAddress() const;  // Static MAC address
    bool GetStaticMacAddressEnabled() const;   // Whether static MAC is enabled

    std::wstring GetVirtualSystemIdentifiers(std::vector<std::wstring>& values) const;

    // System path for WMI operations
    std::wstring GetPath() const;

    // Internal
    IWbemClassObject* GetWbemObject() const { return m_pObject; }

private:
    IWbemClassObject* m_pObject;

    std::wstring GetStringProperty(const wchar_t* name) const;
    bool GetBoolProperty(const wchar_t* name) const;
    std::vector<std::wstring> GetStringArrayProperty(const wchar_t* name) const;
};

} // namespace Virtualization
} // namespace WMI
