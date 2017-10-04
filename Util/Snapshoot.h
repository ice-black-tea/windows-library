#pragma once

#include <Windows.h>
#include <Tlhelp32.h>

typedef enum emENUM_RET_TYPE
{
    ENUM_CONTINUE,
    ENUM_RET_TRUE,
    ENUM_RET_FALSE
}ENUM_RET_TYPE;

//遍历进程回调函数，返回ENUM_CONTINUE则继续遍历
typedef ENUM_RET_TYPE (*PFNEnumProcess)(PROCESSENTRY32 *pProcessEntry, LPVOID pParam);

//遍历线程回调函数，返回ENUM_CONTINUE则继续遍历
typedef ENUM_RET_TYPE (*PFNEnumThread)(THREADENTRY32 *pThreadEntry, LPVOID pParam);

//遍历模块回调函数，返回ENUM_CONTINUE则继续遍历
typedef ENUM_RET_TYPE (*PFNEnumModule)(MODULEENTRY32 *pModuleEntry, LPVOID pParam);

class CSnapshoot
{
public:
    //遍历进程，默认返回TRUE
    static BOOL EnumProcess(PFNEnumProcess pfnEnum, DWORD dwPid = 0, LPVOID pParam = NULL);
    
    //遍历线程，默认返回TRUE
    static BOOL EnumThread(PFNEnumThread pfnEnum, DWORD dwPid = 0, LPVOID pParam = NULL);

    //遍历模块，默认返回TRUE
    static BOOL EnumModule(PFNEnumModule pfnEnum, DWORD dwPid = 0, LPVOID pParam = NULL);
};

