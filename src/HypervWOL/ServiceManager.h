#pragma once

#include <windows.h>
#include <string>

#include "HypervWOLWorker.h"

class ServiceManager
{
public:
    ServiceManager();
    ~ServiceManager();

    bool Initialize(const std::wstring& serviceName);

    // listenSpec: comma/semicolon separated list of interface:port entries.
    // Empty string uses the default listener 0.0.0.0:9.
    void SetBindIp(const std::wstring& bindIp) { m_BindIp = bindIp; }

    void Run();
    void Stop();

private:
    static VOID WINAPI ServiceMain(DWORD argc, LPWSTR* argv);
    static DWORD WINAPI ServiceCtrlHandler(DWORD controlCode, DWORD eventType, LPVOID eventData, LPVOID context);
    static DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);

    void ReportServiceStatus(DWORD currentState, DWORD exitCode, DWORD waitHint);

    static ServiceManager* s_Instance;
    SERVICE_STATUS        m_ServiceStatus;
    SERVICE_STATUS_HANDLE m_ServiceStatusHandle;
    HANDLE                m_ServiceStopEvent;
    std::wstring          m_ServiceName;
    std::wstring          m_BindIp;      // interface IP passed to the worker
    HypervWOLWorker       m_Worker;
};
