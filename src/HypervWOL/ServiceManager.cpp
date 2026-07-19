#include "ServiceManager.h"
#include <iostream>

ServiceManager* ServiceManager::s_Instance = nullptr;

ServiceManager::ServiceManager()
    : m_ServiceStatusHandle(nullptr)
    , m_ServiceStopEvent(nullptr)
    , m_BindIp(L"")
{
    ZeroMemory(&m_ServiceStatus, sizeof(m_ServiceStatus));
    s_Instance = this;
}

ServiceManager::~ServiceManager()
{
    if (m_ServiceStopEvent)
    {
        CloseHandle(m_ServiceStopEvent);
    }
    s_Instance = nullptr;
}

bool ServiceManager::Initialize(const std::wstring& serviceName)
{
    m_ServiceName = serviceName;

    SERVICE_TABLE_ENTRYW serviceTable[] =
    {
        { const_cast<LPWSTR>(m_ServiceName.c_str()), ServiceMain },
        { nullptr, nullptr }
    };

    if (!StartServiceCtrlDispatcherW(serviceTable))
    {
        return false;
    }

    return true;
}

VOID WINAPI ServiceManager::ServiceMain(DWORD argc, LPWSTR* argv)
{
    if (!s_Instance) return;

    // Optional first service parameter: listen spec (comma/semicolon separated ip:port list)
    if (argc >= 2 && argv[1] && argv[1][0] != L'\0')
        s_Instance->m_BindIp = argv[1];
    s_Instance->m_ServiceStatusHandle = RegisterServiceCtrlHandlerExW(
        s_Instance->m_ServiceName.c_str(),
        ServiceCtrlHandler,
        nullptr); 

    if (!s_Instance->m_ServiceStatusHandle)
    {
        return;
    }

    s_Instance->m_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    s_Instance->m_ServiceStatus.dwServiceSpecificExitCode = 0;

    s_Instance->ReportServiceStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

    s_Instance->m_ServiceStopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!s_Instance->m_ServiceStopEvent)
    {
        s_Instance->ReportServiceStatus(SERVICE_STOPPED, GetLastError(), 0);
        return;
    }

    s_Instance->ReportServiceStatus(SERVICE_RUNNING, NO_ERROR, 0);

    HANDLE hThread = CreateThread(nullptr, 0, ServiceWorkerThread, nullptr, 0, nullptr);
    if (hThread)
    {
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
    }

    s_Instance->ReportServiceStatus(SERVICE_STOPPED, NO_ERROR, 0);
}

DWORD WINAPI ServiceManager::ServiceCtrlHandler(DWORD controlCode, DWORD eventType, LPVOID eventData, LPVOID context)
{
    if (!s_Instance) return ERROR_CALL_NOT_IMPLEMENTED;

    switch (controlCode)
    {
    case SERVICE_CONTROL_STOP:
        s_Instance->ReportServiceStatus(SERVICE_STOP_PENDING, NO_ERROR, 3000);
        SetEvent(s_Instance->m_ServiceStopEvent);
        return NO_ERROR;

    case SERVICE_CONTROL_INTERROGATE:
        return NO_ERROR;

    default:
        return ERROR_CALL_NOT_IMPLEMENTED;
    }
}

DWORD WINAPI ServiceManager::ServiceWorkerThread(LPVOID lpParam)
{
    if (s_Instance)
    {
        s_Instance->m_Worker.Run(s_Instance->m_ServiceStopEvent, s_Instance->m_BindIp);
    }
    return 0;
}

void ServiceManager::ReportServiceStatus(DWORD currentState, DWORD exitCode, DWORD waitHint)
{
    static DWORD checkPoint = 1;

    m_ServiceStatus.dwCurrentState = currentState;
    m_ServiceStatus.dwWin32ExitCode = exitCode;
    m_ServiceStatus.dwWaitHint = waitHint;

    if (currentState == SERVICE_START_PENDING)
        m_ServiceStatus.dwControlsAccepted = 0;
    else
        m_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    if ((currentState == SERVICE_RUNNING) || (currentState == SERVICE_STOPPED))
        m_ServiceStatus.dwCheckPoint = 0;
    else
        m_ServiceStatus.dwCheckPoint = checkPoint++;

    SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);
}

void ServiceManager::Run()
{
    // This method is called when running in console mode
    std::wcout << L"HypervWOL Service starting in console mode..." << std::endl;

    m_ServiceStopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!m_ServiceStopEvent)
    {
        std::wcerr << L"Failed to create stop event" << std::endl;
        return;
    }

    std::wcout << L"Service running. Press Ctrl+C to stop..." << std::endl;
    m_Worker.Run(m_ServiceStopEvent, m_BindIp);

    std::wcout << L"Service stopped." << std::endl;
}

void ServiceManager::Stop()
{
    if (m_ServiceStopEvent)
    {
        SetEvent(m_ServiceStopEvent);
    }
}
