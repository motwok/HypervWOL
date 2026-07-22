/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Auto-generated from WMI class Msvm_ComputerSystem
 * Namespace: root\virtualization\v2
 *
 * This file contains strongly-typed C++ wrapper for Hyper-V WMI class.
 * Generated in the style of mgmtclassgen.exe
 */

#pragma once

#include <windows.h>
#include <wbemidl.h>
#include <comdef.h>
#include <string>
#include <vector>

#pragma comment(lib, "wbemuuid.lib")

namespace WMI {
namespace Virtualization {

// Forward declarations
class Msvm_VirtualSystemSettingData;

// Msvm_ComputerSystem - Represents a virtual machine or host computer system
class Msvm_ComputerSystem
{
public:
    // Constructors
    Msvm_ComputerSystem();
    explicit Msvm_ComputerSystem(IWbemClassObject* pObject);
    ~Msvm_ComputerSystem();

    // Copy/Move
    Msvm_ComputerSystem(const Msvm_ComputerSystem& other);
    Msvm_ComputerSystem& operator=(const Msvm_ComputerSystem& other);
    Msvm_ComputerSystem(Msvm_ComputerSystem&& other) noexcept;
    Msvm_ComputerSystem& operator=(Msvm_ComputerSystem&& other) noexcept;

    // Validation
    bool IsValid() const { return m_pObject != nullptr; }

    // Static factory methods
    static std::vector<Msvm_ComputerSystem> Enumerate(IWbemServices* pSvc);
    static Msvm_ComputerSystem Get(IWbemServices* pSvc, const std::wstring& name);

    // Properties (strongly typed)
    std::wstring GetCaption() const;
    std::wstring GetDescription() const;
    std::wstring GetElementName() const;
    std::wstring GetInstallDate() const;
    std::wstring GetName() const;
    std::wstring GetStatus() const;

    UINT16 GetEnabledState() const;
    UINT16 GetHealthState() const;
    UINT16 GetOperationalStatus(std::vector<UINT16>& values) const;
    UINT16 GetRequestedState() const;

    std::wstring GetCreationClassName() const;
    std::wstring GetNameFormat() const;
    std::wstring GetPrimaryOwnerContact() const;
    std::wstring GetPrimaryOwnerName() const;

    std::vector<std::wstring> GetRoles() const;

    // System path for WMI operations
    std::wstring GetPath() const;

    // Methods (strongly typed)
    enum EnabledStateValue : UINT16 {
        Unknown = 0,
        Other = 1,
        Enabled = 2,
        Disabled = 3,
        ShuttingDown = 4,
        NotApplicable = 5,
        EnabledButOffline = 6,
        InTest = 7,
        Deferred = 8,
        Quiesce = 9,
        Starting = 10,
        Paused = 32768,
        Suspended = 32769,
        Saving = 32773,
        Stopping = 32774,
        Pausing = 32776,
        Resuming = 32777
    };

    enum RequestedStateValue : UINT16 {
        Enabled_Requested = 2,
        Disabled_Requested = 3,
        ShutDown = 4,
        Offline = 6,
        Deferred_Requested = 8,
        Quiesce_Requested = 9,
        Reboot = 10,
        Reset = 11,
        Saving_Requested = 32773,
        Pausing_Requested = 32776,
        Resuming_Requested = 32777
    };

    // RequestStateChange method
    struct RequestStateChangeResult {
        UINT32 ReturnValue;
        IWbemClassObject* Job;
    };
    RequestStateChangeResult RequestStateChange(IWbemServices* pSvc, RequestedStateValue requestedState, FILETIME* timeoutPeriod = nullptr);

    // Internal
    IWbemClassObject* GetWbemObject() const { return m_pObject; }

private:
    IWbemClassObject* m_pObject;

    // Helper methods
    std::wstring GetStringProperty(const wchar_t* name) const;
    UINT16 GetUint16Property(const wchar_t* name) const;
    std::vector<std::wstring> GetStringArrayProperty(const wchar_t* name) const;
    std::vector<UINT16> GetUint16ArrayProperty(const wchar_t* name) const;
};

} // namespace Virtualization
} // namespace WMI
