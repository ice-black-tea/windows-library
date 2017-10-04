// SmartPtr.h: interface for the CSmartPtr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SMARTPTR_H__AD1B2FA9_BDD6_4C94_8E95_421270AC3CD4__INCLUDED_)
#define AFX_SMARTPTR_H__AD1B2FA9_BDD6_4C94_8E95_421270AC3CD4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "RefCnt.h"

/************************************************************************/
/*                 智能指针（模板类型需要继承CRefCnt）                  */
/************************************************************************/
template<typename T>
class CSmartPtr
{
public:
    CSmartPtr();
    CSmartPtr(T *pObject);
    CSmartPtr(const CSmartPtr<T>& obj);
    ~CSmartPtr();

public:
    CSmartPtr<T>& operator= (T *pObj);
    CSmartPtr<T>& operator= (const CSmartPtr<T>& obj);

    inline operator T* ();
    inline operator const T* () const;
    inline T* operator-> ();
    inline T& operator* ();
    inline T* operator++ ();
    inline T* operator++ (int);
    inline T* operator-- ();
    inline T* operator-- (int);
    inline bool operator! ();

#ifndef __AFX_H__
    inline T** operator& (); //mfc模板里面用这个会报错
#endif // __AFX_H__

protected:
    T *m_pObject;
};

template<typename T>
CSmartPtr<T>::CSmartPtr()
{
    m_pObject = NULL;
}

template<typename T>
CSmartPtr<T>::CSmartPtr(T *pData)
{
    m_pObject = pData;

    if (m_pObject != NULL)
    {
        m_pObject->AddRef();
    }
}

template<typename T>
CSmartPtr<T>::CSmartPtr(const CSmartPtr& obj)
{
    m_pObject = obj.m_pObject;

    if (m_pObject != NULL)
    {
        m_pObject->AddRef();
    }
}

template<typename T>
CSmartPtr<T>::~CSmartPtr()
{
    if (m_pObject != NULL && m_pObject->Release())
    {
        delete m_pObject;
        m_pObject = NULL;
    }
}

template<typename T>
CSmartPtr<T>& CSmartPtr<T>::operator= (T *pObj)
{
    if (m_pObject == pObj)
    {
        return *this;
    }

    if (m_pObject != NULL && m_pObject->Release())
    {
        delete m_pObject;
        m_pObject = NULL;
    }

    m_pObject = pObj;

    if (m_pObject != NULL)
    {
        m_pObject->AddRef();
    }

    return *this;
}

template<typename T>
CSmartPtr<T>& CSmartPtr<T>::operator= (const CSmartPtr<T>& obj)
{
    if (m_pObject == obj.m_pObject)
    {
        return *this;
    }
    
    if (m_pObject != NULL && m_pObject->Release())
    {
        delete m_pObject;
        m_pObject = NULL;
    }

    m_pObject = obj.m_pObject;

    if (m_pObject != NULL)
    {
        m_pObject->AddRef();
    }
    
    return *this;
}

template<typename T>
inline CSmartPtr<T>::operator T* ()
{
    return m_pObject;
}

template<typename T>
inline CSmartPtr<T>::operator const T*() const
{
    return m_pObject;
}

template<typename T>
inline T* CSmartPtr<T>::operator-> ()
{
    return m_pObject;
}

template<typename T>
inline T& CSmartPtr<T>::operator* ()
{
    return *m_pObject;
}

template<typename T>
inline T* CSmartPtr<T>::operator++ ()
{
    return ++m_pObject;
}

template<typename T>
inline T* CSmartPtr<T>::operator++ (int)
{
    return m_pObject++;
}

template<typename T>
inline T* CSmartPtr<T>::operator-- ()
{
    return --m_pObject;
}

template<typename T>
inline T* CSmartPtr<T>::operator-- (int)
{
    return m_pObject--;
}

template<typename T>
inline bool CSmartPtr<T>::operator! ()
{
    return m_pObject == NULL;
}

#ifndef __AFX_H__
template<typename T>
inline T** CSmartPtr<T>::operator& ()
{
    return &m_pObject;
}
#endif // __AFX_H__

#endif // !defined(AFX_SMARTPTR_H__AD1B2FA9_BDD6_4C94_8E95_421270AC3CD4__INCLUDED_)
