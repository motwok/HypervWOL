/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Auto-generated from WMI class Msvm_VirtualSystemSettingData
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

// Msvm_VirtualSystemSettingData - Virtual machine configuration settings
class Msvm_VirtualSystemSettingData
{
public:
    // Constructors
    Msvm_VirtualSystemSettingData();
    explicit Msvm_VirtualSystemSettingData(IWbemClassObject* pObject);
    ~Msvm_VirtualSystemSettingData();

    // Copy/Move
    Msvm_VirtualSystemSettingData(const Msvm_VirtualSystemSettingData& other);
    Msvm_VirtualSystemSettingData& operator=(const Msvm_VirtualSystemSettingData& other);
    Msvm_VirtualSystemSettingData(Msvm_VirtualSystemSettingData&& other) noexcept;
    Msvm_VirtualSystemSettingData& operator=(Msvm_VirtualSystemSettingData&& other) noexcept;

    // Validation
    bool IsValid() const { return m_pObject != nullptr; }

    // Static factory methods
    static std::vector<Msvm_VirtualSystemSettingData> Enumerate(IWbemServices* pSvc);
    static Msvm_VirtualSystemSettingData GetForComputerSystem(IWbemServices* pSvc, const std::wstring& computerSystemPath);

    // Properties (strongly typed)
    std::wstring GetInstanceID() const;
    std::wstring GetCaption() const;
    std::wstring GetDescription() const;
    std::wstring GetElementName() const;

    std::wstring GetConfigurationDataRoot() const;
    std::wstring GetConfigurationFile() const;
    std::wstring GetConfigurationID() const;
    std::wstring GetLogDataRoot() const;
    std::wstring GetRecoveryFile() const;
    std::wstring GetSnapshotDataRoot() const;
    std::wstring GetSuspendDataRoot() const;
    std::wstring GetSwapFileDataRoot() const;

    std::wstring GetVirtualSystemIdentifier() const;
    std::wstring GetVirtualSystemType() const;

    // System path for WMI operations
    std::wstring GetPath() const;

    // Internal
    IWbemClassObject* GetWbemObject() const { return m_pObject; }

private:
    IWbemClassObject* m_pObject;

    std::wstring GetStringProperty(const wchar_t* name) const;
};

} // namespace Virtualization
} // namespace WMI
