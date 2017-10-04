#pragma once

#include <Windows.h>
#include <ntsecapi.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    ULONG Flags;
    USHORT LoadCount;
    USHORT TlsIndex;
    union {
        LIST_ENTRY HashLinks;
        struct {
            PVOID SectionPointer;
            ULONG CheckSum;
        };
    };
    union {
        ULONG TimeDateStamp;
        PVOID LoadedImports;
    };
    PVOID EntryPointActivationContext;
    PVOID PatchInformation;
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

typedef HMODULE(WINAPI *PFNLoadLibraryA)(
    LPCSTR lpFileName);

typedef BOOL(WINAPI *PFNDllEntry)(
    HINSTANCE hinstDLL,
    DWORD fdwReason,
    LPVOID lpvReserved);

/************************************************************************/
/*                     加载PE文件,简陋版LoadLibrary                     */
/************************************************************************/
HMODULE LoadDLL(LPCTSTR lpFileName);

/************************************************************************/
/*            获取函数地址（没有绝对地址），同GetProcAddress            */
/************************************************************************/
FARPROC GetProcAddr(
    HMODULE hModule,    // handle to DLL module
    LPCSTR lpProcName); // function name

#ifdef __cplusplus
}
#endif