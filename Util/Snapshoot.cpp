#include "Snapshoot.h"


BOOL CSnapshoot::EnumProcess(PFNEnumProcess pfnEnum, DWORD dwPid, LPVOID pParam)
{
    HANDLE         hProcessSnap = NULL;
    BOOL           bRet = TRUE;
    PROCESSENTRY32 pe32 = { 0 };

    //  Take a snapshot of all processes in the system. 

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, dwPid);

    if (hProcessSnap == INVALID_HANDLE_VALUE)
        return (FALSE);

    //  Fill in the size of the structure before using it. 

    pe32.dwSize = sizeof(PROCESSENTRY32);

    //  Walk the snapshot of the processes, and for each process, 
    //  display information. 

    if (Process32First(hProcessSnap, &pe32))
    {
        do
        {
            switch (pfnEnum(&pe32, pParam))
            {
            case ENUM_RET_TRUE:
                bRet = TRUE;
                goto EXIT_LABLE;
            case ENUM_RET_FALSE:
                bRet = FALSE;
                goto EXIT_LABLE;
            }

        } while (Process32Next(hProcessSnap, &pe32));
    }
    else
        bRet = FALSE;    // could not walk the list of processes 

    // Do not forget to clean up the snapshot object. 

EXIT_LABLE:
    CloseHandle(hProcessSnap);
    return (bRet);
}


BOOL CSnapshoot::EnumThread(PFNEnumThread pfnEnum, DWORD dwPid, LPVOID pParam)
{
    HANDLE         hThreadSnap = NULL;
    BOOL           bRet = TRUE;
    THREADENTRY32  te32 = { 0 };

    //  Take a snapshot of all processes in the system. 

    hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, dwPid);

    if (hThreadSnap == INVALID_HANDLE_VALUE)
        return (FALSE);

    //  Fill in the size of the structure before using it. 

    te32.dwSize = sizeof(THREADENTRY32);

    //  Walk the snapshot of the processes, and for each process, 
    //  display information. 

    if (Thread32First(hThreadSnap, &te32))
    {
        do
        {
            if (dwPid == 0 || te32.th32OwnerProcessID == dwPid)
            {
                switch (pfnEnum(&te32, pParam))
                {
                case ENUM_RET_TRUE:
                    bRet = TRUE;
                    goto EXIT_LABLE;
                case ENUM_RET_FALSE:
                    bRet = FALSE;
                    goto EXIT_LABLE;
                }
            }

        } while (Thread32Next(hThreadSnap, &te32));
    }
    else
        bRet = FALSE;    // could not walk the list of processes 

    // Do not forget to clean up the snapshot object. 

EXIT_LABLE:
    CloseHandle(hThreadSnap);
    return (bRet);
}


BOOL CSnapshoot::EnumModule(PFNEnumModule pfnEnum, DWORD dwPid, LPVOID pParam)
{
    HANDLE        hModuleSnap = NULL;
    BOOL          bRet = TRUE;
    MODULEENTRY32 me32 = { 0 };

    // Take a snapshot of all modules in the specified process. 

    hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPid);
    
    if (hModuleSnap == INVALID_HANDLE_VALUE)
        return (FALSE);

    // Fill the size of the structure before using it. 

    me32.dwSize = sizeof(MODULEENTRY32);

    // Walk the module list of the process, and find the module of 
    // interest. Then copy the information to the buffer pointed 
    // to by lpMe32 so that it can be returned to the caller. 

    if (Module32First(hModuleSnap, &me32))
    {
        do
        {
            switch (pfnEnum(&me32, pParam))
            {
            case ENUM_RET_TRUE:
                bRet = TRUE;
                goto EXIT_LABLE;
            case ENUM_RET_FALSE:
                bRet = FALSE;
                goto EXIT_LABLE;
            }

        } while (Module32Next(hModuleSnap, &me32));
    }
    else
        bRet = FALSE;           // could not walk module list 

    // Do not forget to clean up the snapshot object. 

EXIT_LABLE:
    CloseHandle(hModuleSnap);
    return (bRet);
}