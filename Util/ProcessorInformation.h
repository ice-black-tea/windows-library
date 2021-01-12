#pragma once

#include <Windows.h>
#include "../Common/Singleton.h"


class CProcessorInformation
{
    DECLARE_SINGLETON(CProcessorInformation)
private:
    CProcessorInformation();
    ~CProcessorInformation();

public:
    inline DWORD GetNumaNodeCount();
    inline DWORD GetProcessorPackageCount();
    inline DWORD GetProcessorCoreCount();
    inline DWORD GetLogicalProcessorCount();
    inline DWORD GetProcessorL1CacheCount();
    inline DWORD GetProcessorL2CacheCount();
    inline DWORD GetProcessorL3CacheCount();

private:
    DWORD m_dwNumaNodeCount;
    DWORD m_dwProcessorPackageCount;
    DWORD m_dwProcessorCoreCount;
    DWORD m_dwLogicalProcessorCount;
    DWORD m_dwProcessorL1CacheCount;
    DWORD m_dwProcessorL2CacheCount;
    DWORD m_dwProcessorL3CacheCount;

private:
    static DWORD CountSetBits(ULONG_PTR bitMask);
};

DWORD CProcessorInformation::GetNumaNodeCount()
{
    return m_dwNumaNodeCount;
}

DWORD CProcessorInformation::GetProcessorPackageCount()
{
    return m_dwProcessorPackageCount;
}

DWORD CProcessorInformation::GetProcessorCoreCount()
{
    return m_dwProcessorCoreCount;
}

DWORD CProcessorInformation::GetLogicalProcessorCount()
{
    return m_dwLogicalProcessorCount;
}

DWORD CProcessorInformation::GetProcessorL1CacheCount()
{
    return m_dwProcessorL1CacheCount;
}

DWORD CProcessorInformation::GetProcessorL2CacheCount()
{
    return m_dwProcessorL2CacheCount;
}

DWORD CProcessorInformation::GetProcessorL3CacheCount()
{
    return m_dwProcessorL3CacheCount;
}
