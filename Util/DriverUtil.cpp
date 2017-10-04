#include "DriverUtil.h"


CDriverUtil::CDriverUtil()
{
}


CDriverUtil::CDriverUtil(CString strPathName)
{
    SetPathName(strPathName);
}


CDriverUtil::~CDriverUtil()
{
}


void CDriverUtil::SetPathName(CMyString strPathName)
{
    m_strPathName = strPathName;
    m_strServiceName = strPathName;

    int nIndex = m_strServiceName.ReverseFind(TEXT('\\'));

    if (nIndex >= 0)
    {
        m_strServiceName.Remove(0, nIndex + 1);
    }

    nIndex = m_strServiceName.Find(TEXT('.'));

    if (nIndex >= 0)
    {
        m_strServiceName.Remove(nIndex);
    }
}

DWORD CDriverUtil::Install(BOOL bStart)
{
    DWORD dwError = ERROR_SUCCESS;
    SC_HANDLE hSCM = NULL;
    SC_HANDLE hService = NULL;

    hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (hSCM == NULL)
    {
        dwError = GetLastError();
        goto EXIT_LABLE;
    }

    hService = CreateService(
        hSCM,
        m_strServiceName,
        m_strServiceName,
        SERVICE_ALL_ACCESS,
        SERVICE_KERNEL_DRIVER,
        SERVICE_DEMAND_START,
        SERVICE_ERROR_IGNORE,
        m_strPathName,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL);

    if (hService == NULL)
    {
        dwError = GetLastError();
        goto EXIT_LABLE;
    }

    if (bStart)
    {
        if (!StartService(hService, 0, 0))
        {
            dwError = GetLastError();
            goto EXIT_LABLE;
        }
    }

EXIT_LABLE:
    if (hService != NULL)
    {
        CloseServiceHandle(hService);
        hService = NULL;
    }

    if (hSCM != NULL)
    {
        CloseServiceHandle(hSCM);
        hSCM = NULL;
    }

    return dwError;
}


DWORD CDriverUtil::Start()
{
    DWORD dwError = ERROR_SUCCESS;
    SC_HANDLE hSCM = NULL;
    SC_HANDLE hService = NULL;

    hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (hSCM == NULL)
    {
        dwError = GetLastError();
        goto EXIT_LABLE;
    }

    hService = OpenService(hSCM, m_strServiceName, SERVICE_ALL_ACCESS);

    if (hService == NULL)
    {
        dwError = GetLastError();
        goto EXIT_LABLE;
    }

    if (!StartService(hService, 0, 0))
    {
        dwError = GetLastError();
        goto EXIT_LABLE;
    }

EXIT_LABLE:
    if (hService != NULL)
    {
        CloseServiceHandle(hService);
        hService = NULL;
    }

    if (hSCM != NULL)
    {
        CloseServiceHandle(hSCM);
        hSCM = NULL;
    }

    return dwError;
}


DWORD CDriverUtil::Stop()
{
    DWORD dwError = ERROR_SUCCESS;
    SC_HANDLE hSCM = NULL;
    SC_HANDLE hService = NULL;

    hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (hSCM == NULL)
    {
        dwError = GetLastError();
        goto EXIT_LABLE;
    }

    hService = OpenService(hSCM, m_strServiceName, SERVICE_ALL_ACCESS);

    if (hService == NULL)
    {
        dwError = GetLastError();
        goto EXIT_LABLE;
    }

    SERVICE_STATUS status;
    QueryServiceStatus(hService, &status);

    if (status.dwCurrentState == SERVICE_STOPPED ||
        status.dwCurrentState == SERVICE_STOP_PENDING)
    {
        dwError = ERROR_SERVICE_DOES_NOT_EXIST;
        goto EXIT_LABLE;
    }

    ControlService(hService, SERVICE_CONTROL_STOP, &status);

    do
    {
        Sleep(50);
        QueryServiceStatus(hService, &status);

    } while (status.dwCurrentState != SERVICE_STOPPED);

EXIT_LABLE:

    if (hService != NULL)
    {
        CloseServiceHandle(hService);
        hService = NULL;
    }

    if (hSCM != NULL)
    {
        CloseServiceHandle(hSCM);
        hSCM = NULL;
    }

    return dwError;
}


DWORD CDriverUtil::Uninstall(BOOL bStop)
{
    DWORD dwError = ERROR_SUCCESS;
    SC_HANDLE hSCM = NULL;
    SC_HANDLE hService = NULL;

    hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (hSCM == NULL)
    {
        dwError = GetLastError();
        goto EXIT_LABLE;
    }

    hService = OpenService(hSCM, m_strServiceName, SERVICE_ALL_ACCESS);

    if (hService == NULL)
    {
        dwError = GetLastError();
        goto EXIT_LABLE;
    }

    if (bStop)
    {
        SERVICE_STATUS status;
        QueryServiceStatus(hService, &status);

        if (status.dwCurrentState != SERVICE_STOPPED &&
            status.dwCurrentState != SERVICE_STOP_PENDING)
        {
            ControlService(hService, SERVICE_CONTROL_STOP, &status);

            do
            {
                Sleep(50);
                QueryServiceStatus(hService, &status);

            } while (status.dwCurrentState != SERVICE_STOPPED);
        }
    }

    if (!DeleteService(hService))
    {
        dwError = GetLastError();
        goto EXIT_LABLE;
    }

EXIT_LABLE:
    if (hService != NULL)
    {
        CloseServiceHandle(hService);
        hService = NULL;
    }

    if (hSCM != NULL)
    {
        CloseServiceHandle(hSCM);
        hSCM = NULL;
    }

    return dwError;
}


DWORD CDriverUtil::ReInstall()
{
    DWORD dwError = ERROR_SUCCESS;
    SC_HANDLE hSCM = NULL;
    SC_HANDLE hService = NULL;

    hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (hSCM == NULL)
    {
        dwError = GetLastError();
        goto EXIT_LABLE;
    }

    hService = OpenService(hSCM, m_strServiceName, SERVICE_ALL_ACCESS);

    if (hService != NULL)
    {
        SERVICE_STATUS status;
        QueryServiceStatus(hService, &status);

        if (status.dwCurrentState != SERVICE_STOPPED &&
            status.dwCurrentState != SERVICE_STOP_PENDING)
        {
            ControlService(hService, SERVICE_CONTROL_STOP, &status);

            do
            {
                Sleep(50);
                QueryServiceStatus(hService, &status);

            } while (status.dwCurrentState != SERVICE_STOPPED);
        }

        DeleteService(hService);
        CloseServiceHandle(hService);
    }

    hService = CreateService(
        hSCM,
        m_strServiceName,
        m_strServiceName,
        SERVICE_ALL_ACCESS,
        SERVICE_KERNEL_DRIVER,
        SERVICE_DEMAND_START,
        SERVICE_ERROR_IGNORE,
        m_strPathName,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL);

    if (hService == NULL)
    {
        dwError = GetLastError();
        goto EXIT_LABLE;
    }

    if (!StartService(hService, 0, 0))
    {
        dwError = GetLastError();
        goto EXIT_LABLE;
    }

EXIT_LABLE:
    if (hService != NULL)
    {
        CloseServiceHandle(hService);
        hService = NULL;
    }

    if (hSCM != NULL)
    {
        CloseServiceHandle(hSCM);
        hSCM = NULL;
    }

    return dwError;
}