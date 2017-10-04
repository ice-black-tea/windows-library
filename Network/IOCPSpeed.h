#pragma once

#include "../Util/Lock.h"

class CIOCPSpeed
{
public:
    CIOCPSpeed();
    ~CIOCPSpeed();

public:
    DWORD GetSpeed();
    void SetSpeed(DWORD nBytes);

protected:
    CLock  m_Lock;
    DWORD  m_dwTick;
    DWORD  m_dwBytes;
    DWORD  m_dwSpeed;
};


inline CIOCPSpeed::CIOCPSpeed()
    : m_dwTick(0)
    , m_dwBytes(0)
{
    
}


inline CIOCPSpeed::~CIOCPSpeed()
{

}


inline DWORD CIOCPSpeed::GetSpeed()
{
    return GetTickCount() - m_dwTick >= 1000 ? 0 : m_dwSpeed;
}


inline void CIOCPSpeed::SetSpeed(DWORD dwBytes)
{
    LOCK_SCOPE(m_Lock);
    m_dwBytes += dwBytes;
    DWORD dwTick = GetTickCount();
    DWORD dwCostTick = dwTick - m_dwTick;
    if (dwCostTick >= 1000)
    {
        m_dwTick = dwTick;
        m_dwSpeed = m_dwBytes * 1000 / dwCostTick;
        m_dwBytes = 0;
    }
}