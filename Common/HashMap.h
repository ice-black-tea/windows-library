// MyHashMap.h: interface for the CHashMap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYHASHMAP_H__3438EF94_BF24_4186_AC34_9695705ECD2A__INCLUDED_)
#define AFX_MYHASHMAP_H__3438EF94_BF24_4186_AC34_9695705ECD2A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <memory.h>

#ifndef NULL 
#define NULL 0
#endif

template <typename T>
inline int Hash(T key)
{
    return ((int)key > 0 ? (int)key : -(int)key) * 73;
}

//typedef int K;
//typedef int V;
template <typename K, typename V>
class CHashMap  
{
public:
	CHashMap(int nSize = 11);
    CHashMap(const CHashMap& obj);
	virtual ~CHashMap();

public:
    //哈希表大小
    inline int GetSize() const;
    //元素个数
    inline int GetCount() const;
    //是否为空
    inline bool IsEmpty() const;

    //初始化哈希表
    CHashMap& InitHashMap(int nSize);

    //插入数据
    CHashMap& SetAt(const K& key, const V& value);
    //查找数据
    bool Lookup(const K& key, V& value) const;

    //移除键
    bool RemoveKey(const K& key);
    //移除所有
    CHashMap& RemoveAll();

    V& operator[] (const K& key);
    CHashMap& operator= (const CHashMap& obj);

public:
    class CPair
    {
    public:
        K m_Key;
        V m_Value;
    };

private:
    class CNode
    {
    public:
        CNode();
        CNode(const K& key);
        CNode(const K& key, const V& Data);

    public:
        CPair  m_Pair;
        CNode *m_pNext;
    };

public:
    //迭代器
    class iterator
    {
    public:
        friend class CHashMap<K, V>;
        inline iterator(CHashMap *m_pHashMap = NULL, CNode *pNode = NULL);
        inline iterator& operator++();
        inline const iterator operator++(int);
        inline CPair* operator*() const;
        inline CPair* operator->() const;
        inline iterator& operator= (const iterator& obj);
        inline bool operator== (const iterator& obj) const;
        inline bool operator!= (const iterator& obj) const;
    private:
        CNode    *m_pNode;
        CHashMap *m_pHashMap;
    };

    iterator begin();
    iterator end();

protected:
    CNode **m_pHashMap;
    int     m_nSzie;
    int     m_nCount;

private:
    inline CNode *Find(int nHash, const K& key) const;
    inline void Copy(const CHashMap& obj);
};

template <typename K, typename V>
CHashMap<K, V>::CHashMap(int nSize)
    : m_pHashMap(NULL), m_nSzie(0)
{
    InitHashMap(nSize);
}

template <typename K, typename V>
CHashMap<K, V>::CHashMap(const CHashMap& obj)
{
    Copy(obj);
}

template <typename K, typename V>
CHashMap<K, V>::~CHashMap()
{
    RemoveAll();
}

//哈希表大小
template <typename K, typename V>
inline int CHashMap<K, V>::GetSize() const
{
    return m_nSzie;
}

//元素个数
template <typename K, typename V>
inline int CHashMap<K, V>::GetCount() const
{
    return m_nCount;
}

//是否为空
template <typename K, typename V>
inline bool CHashMap<K, V>::IsEmpty() const
{
    return m_nCount == 0;
}

//初始化哈希表
template <typename K, typename V>
CHashMap<K, V>& CHashMap<K, V>::InitHashMap(int nSize)
{
    RemoveAll();

    m_nSzie = nSize;
    m_pHashMap = new CNode *[m_nSzie];
    memset(m_pHashMap, 0, m_nSzie * sizeof(CNode*));

    return *this;
}

//插入数据
template <typename K, typename V>
CHashMap<K, V>& CHashMap<K, V>::SetAt(const K& key, const V& value)
{
    int nHash = Hash(key) % m_nSzie;
    CNode *pNode = Find(nHash, key);

    if (pNode != NULL)
    {
        pNode->m_Pair.m_Value = value;
        return *this;
    }

    CNode *pNodeInsert = new CNode(key, value);

    if (m_pHashMap[nHash] == NULL)
    {
        m_pHashMap[nHash] = pNodeInsert;
    }
    else
    {
        pNodeInsert->m_pNext = m_pHashMap[nHash]->m_pNext;
        m_pHashMap[nHash]->m_pNext = pNodeInsert;
    }

    m_nCount++;

    return *this;
}

//查找数据
template <typename K, typename V>
bool CHashMap<K, V>::Lookup(const K& key, V& value) const
{
    int nHash = Hash(key) % m_nSzie;
    CNode *pNode = Find(nHash, key);

    if (pNode != NULL)
    {
        value = pNode->m_Pair.m_Value;
        return true;
    }
    
    return false;
}

//移除键值
template <typename K, typename V>
bool CHashMap<K, V>::RemoveKey(const K& key)
{
    int nHash = Hash(key) % m_nSzie;
    CNode *pNodeCur = m_pHashMap[nHash];
    CNode *pNodePrev = NULL;

    while (pNodeCur != NULL)
    {
        if (pNodeCur->m_Pair.m_Key == key)
        {
            if (pNodePrev == NULL)
            {
                m_pHashMap[nHash] = pNodeCur->m_pNext;
            }
            else
            {
                pNodePrev->m_pNext = pNodeCur->m_pNext;
            }

            delete pNodeCur;

            m_nCount--;

            return true;
        }

        pNodePrev = pNodeCur;
        pNodeCur = pNodeCur->m_pNext;
    }

    return false;
}

//移除所有
template <typename K, typename V>
CHashMap<K, V>& CHashMap<K, V>::RemoveAll()
{
    if (m_pHashMap != NULL)
    {
        for (int i = 0; i < m_nSzie; i++)
        {
            CNode *pNode = m_pHashMap[i];

            while (pNode != NULL)
            {
                CNode *pTemp = pNode;
                pNode = pNode->m_pNext;

                delete pTemp;
            }
        }

        delete[] m_pHashMap;
        m_pHashMap = NULL;
    }
    
    m_nSzie = 0;
    m_nCount = 0;

    return *this;
}

template <typename K, typename V>
V& CHashMap<K, V>::operator[] (const K& key)
{
    int nHash = Hash(key) % m_nSzie;
    CNode *pNode = Find(nHash, key);

    if (pNode != NULL)
    {
        return pNode->m_Pair.m_Value;
    }

    CNode *pNodeInsert = new CNode(key);

    if (m_pHashMap[nHash] == NULL)
    {
        m_pHashMap[nHash] = pNodeInsert;
    }
    else
    {
        pNodeInsert->m_pNext = m_pHashMap[nHash]->m_pNext;
        m_pHashMap[nHash]->m_pNext = pNodeInsert;
    }

    m_nCount++;

    return pNodeInsert->m_Pair.m_Value;
}

template <typename K, typename V>
CHashMap<K, V>& CHashMap<K, V>::operator= (const CHashMap& obj)
{
    if (this == &obj)
    {
        return *this;
    }

    RemoveAll();
    Copy(obj);

    return *this;
}

template <typename K, typename V>
CHashMap<K, V>::CNode::CNode()
    : m_pNext(NULL)
{

}

template <typename K, typename V>
CHashMap<K, V>::CNode::CNode(const K& key)
    : m_pNext(NULL)
{
    m_Pair.m_Key = key;
}

template <typename K, typename V>
CHashMap<K, V>::CNode::CNode(const K& key, const V& Data)
    : m_pNext(NULL)
{
    m_Pair.m_Key = key;
    m_Pair.m_Value = Data;
}

template <typename K, typename V>
inline typename CHashMap<K, V>::CNode *CHashMap<K, V>::Find(int nHash, const K& key) const
{
    CNode *pNode = m_pHashMap[nHash];

    while (pNode != NULL)
    {
        if (pNode->m_Pair.m_Key == key)
        {
            return pNode;
        }
        
        pNode = pNode->m_pNext;
    }

    return NULL;
}

template <typename K, typename V>
inline void CHashMap<K, V>::Copy(const CHashMap& obj)
{
    m_nSzie = obj.m_nSzie;
    m_pHashMap = new CNode *[m_nSzie];
    memset(m_pHashMap, 0, m_nSzie * sizeof(CNode*));

    m_nCount = 0;

    for (int i = 0; i < m_nSzie; i++)
    {
        CNode *pNode = obj.m_pHashMap[i];
        
        while (pNode != NULL)
        {
            SetAt(pNode->m_Pair.m_Key, pNode->m_Pair.m_Value);

            pNode = pNode->m_pNext;
        }
    }
}

template <typename K, typename V>
CHashMap<K, V>::iterator::iterator(CHashMap *m_pHashMap, CNode *pNode)
    : m_pNode(pNode)
    , m_pHashMap(m_pHashMap)
{

}

template <typename K, typename V>
typename CHashMap<K, V>::iterator& CHashMap<K, V>::iterator::operator++()
{
    if (m_pNode->m_pNext != NULL)
    {
        m_pNode = m_pNode->m_pNext;
        return *this;
    }

    int i = Hash(m_pNode->m_Pair.m_Key) % m_pHashMap->m_nSzie + 1;
    for (; i < m_pHashMap->m_nSzie; i++)
    {
        if (m_pHashMap->m_pHashMap[i] != NULL)
        {
            m_pNode = m_pHashMap->m_pHashMap[i];
            return *this;
        }
    }

    m_pNode = NULL;
    return *this;
}

template <typename K, typename V>
typename const CHashMap<K, V>::iterator CHashMap<K, V>::iterator::operator++(int)
{
    iterator it = *this;

    if (m_pNode->m_pNext != NULL)
    {
        m_pNode = m_pNode->m_pNext;
        return it;
    }

    int i = Hash(m_pNode->m_Pair.m_Key) % m_pHashMap->m_nSzie + 1;
    for (; i < m_pHashMap->m_nSzie; i++)
    {
        if (m_pHashMap->m_pHashMap[i] != NULL)
        {
            m_pNode = m_pHashMap->m_pHashMap[i];
            return it;
        }
    }

    m_pNode = NULL;
    return it;
}

template <typename K, typename V>
typename CHashMap<K, V>::CPair *CHashMap<K, V>::iterator::operator*() const
{
    return &m_pNode->m_Pair;
}

template <typename K, typename V>
typename CHashMap<K, V>::CPair *CHashMap<K, V>::iterator::operator->() const
{
    return &m_pNode->m_Pair;
}

template <typename K, typename V>
typename CHashMap<K, V>::iterator& CHashMap<K, V>::iterator::operator= (const iterator& obj)
{
    m_pNode = obj.m_pNode;
    m_pHashMap = obj.m_pHashMap;
    return *this;
}

template <typename K, typename V>
typename bool CHashMap<K, V>::iterator::operator== (const iterator& obj) const
{
    return m_pNode == obj.m_pNode;
}

template <typename K, typename V>
typename bool CHashMap<K, V>::iterator::operator!= (const iterator& obj) const
{
    return m_pNode != obj.m_pNode;
}

template <typename K, typename V>
typename CHashMap<K, V>::iterator CHashMap<K, V>::begin()
{
    iterator it(this);

    for (int i = 0; i < m_nSzie; i++)
    {
        if (m_pHashMap[i] != NULL)
        {
            it.m_pNode = m_pHashMap[i];
            return it;
        }
    }

    return it;
}

template <typename K, typename V>
typename CHashMap<K, V>::iterator CHashMap<K, V>::end()
{
    return iterator(this, NULL);
}

#endif // !defined(AFX_MYHASHMAP_H__3438EF94_BF24_4186_AC34_9695705ECD2A__INCLUDED_)
