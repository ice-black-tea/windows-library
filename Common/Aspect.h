#pragma once

template <typename T>
class IAspect
{
public:
    IAspect(T *pObject);
    virtual ~IAspect();

public:
    inline T* operator-> ();

protected:
    T* m_pObject;
};

template <typename T>
IAspect<T>::IAspect(T *pObject)
    : m_pObject(pObject)
{

}

template <typename T>
IAspect<T>::~IAspect()
{

}

template <typename T>
inline T* IAspect<T>::operator-> ()
{
    return m_pObject;
}