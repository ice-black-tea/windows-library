#pragma once

#ifndef NULL
#define NULL 0
#endif // !NULL


/************************************************************************/
/*                             引用计数类                               */
/************************************************************************/
class CRefCnt
{
public:
    CRefCnt(); //构造函数
    ~CRefCnt(); //析构函数

public:
    void AddRef(); //引用计数加一
    bool Release(); //引用计数减一

private:
    int m_nRefCnt; //计数器
};

//构造函数
inline CRefCnt::CRefCnt()
    : m_nRefCnt(0)
{

}

//析构函数
inline CRefCnt::~CRefCnt()
{

}

//引用计数加一
inline void CRefCnt::AddRef()
{
    m_nRefCnt++;
}

//引用计数减一
inline bool CRefCnt::Release()
{
    return --m_nRefCnt == 0;
}