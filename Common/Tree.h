// MyTree.h: interface for the CMyTree class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYTREE_H__A1340351_C1D4_415A_A650_F6C3CA0C1402__INCLUDED_)
#define AFX_MYTREE_H__A1340351_C1D4_415A_A650_F6C3CA0C1402__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFX_H__
#define CTree CMyTree
typedef void* POSITION;
#endif // __AFX_H__

#ifndef NULL 
#define NULL 0
#endif

#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

#include <time.h>
#include "List.h"

template <typename T>
class CMyTree  
{
public:
    CMyTree();
    CMyTree(const CMyTree<T>& obj);
    virtual ~CMyTree();

public:
    //获取结点数
    inline int GetCount() const;
    //获取根节点
    inline POSITION GetRoot() const;
    //是否为空
    inline bool IsEmpty() const;

    //获取值
    inline static T& GetAt(POSITION posDest);
    //获取父节点
    inline static POSITION GetParent(POSITION posDest);
    //获取左子树
    inline static POSITION GetLeft(POSITION posDest);
    //获取右子树
    inline static POSITION GetRight(POSITION posDest);
    //获取下一个值
    inline static POSITION GetNext(POSITION posDest);

    //插入结点
    POSITION Insert(const T& Data);
    //移除结点
    CMyTree<T>& RemovAt(POSITION posDest);
    //移除结点
    CMyTree<T>& RemovAt(POSITION posDest, bool(*pfnMatch)(const T&, void*), void *pParam);

    //查找结点
    POSITION Find(const T& Data);

    //前序遍历
    void PreOrder(void(*pfnAction)(const T&, void*), void *pParam) const;
    //中序遍历
    void InOrder(void(*pfnAction)(const T&, void*), void *pParam) const;
    //后序遍历
    void PostOrder(void(*pfnAction)(const T&, void*), void *pParam) const;
    //层序遍历
    void LayerOrder(void(*pfnAction)(const T&, void*), void *pParam) const;

    //移除树
    CMyTree<T>& RemoveAll();

    CMyTree<T>& operator= (const CMyTree<T>& obj);

private:
    struct CNode
    {
        inline CNode();
        inline CNode(const T& Data);
        
        T      m_Data; //数据
        int    m_nHeight;
        CNode *m_pParent;
        CNode *m_pLeft;
        CNode *m_pRight;
        CNode *m_pNext;
    };
private:
    CNode *m_pRoot; //根节点
    int m_nCount; //结点树

private:
    //结点高度
    inline int Height(CNode *pNode);
    //调整高度
    void AdjustHeight(CNode *pNodeCur);
    //右单旋转
    void RightRotate(CNode *pK1);
    //左单旋转
    void LeftRotate(CNode *pK1);

    //拷贝树
    CMyTree<T>& Copy(const CMyTree<T>& obj);
};

template <typename T>
CMyTree<T>::CMyTree()
    : m_pRoot(NULL), m_nCount(0)
{

}

template <typename T>
CMyTree<T>::CMyTree(const CMyTree<T>& obj)
    : m_pRoot(NULL), m_nCount(0)
{
    Copy(obj);
}

template <typename T>
CMyTree<T>::~CMyTree()
{
    RemoveAll();
}

//获取结点数
template <typename T>
inline int CMyTree<T>::GetCount() const
{
    return m_nCount;
}

//获取根节点
template <typename T>
inline POSITION CMyTree<T>::GetRoot() const
{
    return m_pRoot;
}

//是否为空
template <typename T>
inline bool CMyTree<T>::IsEmpty() const
{
    return m_nCount == 0;
}

//获取值
template <typename T>
inline T& CMyTree<T>::GetAt(POSITION posDest)
{
    return ((CNode*)posDest)->m_Data;
}

//获取父节点
template <typename T>
inline POSITION CMyTree<T>::GetParent(POSITION posDest)
{
    return ((CNode*)posDest)->m_pParent;
}

//获取左子树
template <typename T>
inline POSITION CMyTree<T>::GetLeft(POSITION posDest)
{
    return ((CNode*)posDest)->m_pLeft;
}

//获取左子树
template <typename T>
inline POSITION CMyTree<T>::GetRight(POSITION posDest)
{
    return ((CNode*)posDest)->m_pRight;
}

//获取下一个结点
template <typename T>
inline POSITION CMyTree<T>::GetNext(POSITION posDest)
{
    return ((CNode*)posDest)->m_pNext;
}

//插入数据
template <typename T>
POSITION CMyTree<T>::Insert(const T& Data)
{
    CNode *pNodeInsert = new CNode(Data);

    if (m_pRoot == NULL)
    {
        m_pRoot = pNodeInsert;
        m_nCount++;
        return pNodeInsert;
    }

    CNode *pNodeCur = m_pRoot;

    while (pNodeCur != NULL)
    {
        if (pNodeCur->m_Data > Data) //往左子树插
        {
            if (pNodeCur->m_pLeft != NULL)
            {
                pNodeCur = pNodeCur->m_pLeft;
            }
            else
            {
                pNodeCur->m_pLeft = pNodeInsert;
                pNodeInsert->m_pParent = pNodeCur;
                break;
            }
        }
        else if (Data > pNodeCur->m_Data) //往右子树插
        {
            if (pNodeCur->m_pRight != NULL)
            {
                pNodeCur = pNodeCur->m_pRight;
            }
            else
            {
                pNodeCur->m_pRight = pNodeInsert;
                pNodeInsert->m_pParent = pNodeCur;
                break;
            }
        }
        else
        {
            pNodeInsert->m_pNext = pNodeCur->m_pNext;
            pNodeCur->m_pNext = pNodeInsert;
            m_nCount++;
            return pNodeInsert;
        }
    }

    AdjustHeight(pNodeInsert->m_pParent);

    m_nCount++;

    return pNodeInsert;
}

//移除结点
template <typename T>
CMyTree<T>& CMyTree<T>::RemovAt(POSITION posDest)
{
    CNode *pNodeRemove = (CNode*)posDest;

    if (pNodeRemove == NULL)
    {
        return *this;
    }

    //如果删除结点有两个子节点，先将他与左子树最大值交换位置
    if (pNodeRemove->m_pLeft != NULL && pNodeRemove->m_pRight != NULL)
    {
        CNode *pNodeDest = pNodeRemove->m_pLeft;

        while (pNodeDest->m_pRight != NULL)
        {
            pNodeDest = pNodeDest->m_pRight;
        }

        CNode *pNodeNext = pNodeRemove->m_pNext;
        pNodeRemove->m_Data = pNodeDest->m_Data;
        pNodeRemove->m_pNext = pNodeDest->m_pNext;
        pNodeRemove = pNodeDest;
        pNodeRemove->m_pNext = pNodeNext;
    }

    CNode *pNodeParent = pNodeRemove->m_pParent;

    //如果是叶子节点，直接删除
    if (pNodeRemove->m_pLeft == NULL && pNodeRemove->m_pRight == NULL)
    {
        if (pNodeParent == NULL)
        {
            m_pRoot = NULL;
        }
        else if (pNodeParent->m_pLeft == pNodeRemove)
        {
            pNodeParent->m_pLeft = NULL;
        }
        else
        {
            pNodeParent->m_pRight = NULL;
        }
    }
    else //有一个子节点，将子节点移动到删除结点处
    {
        CNode *pNodeChild = NULL;

        if (pNodeRemove->m_pLeft != NULL)
        {
            pNodeChild = pNodeRemove->m_pLeft;
        }
        else
        {
            pNodeChild = pNodeRemove->m_pRight;
        }

        if (pNodeParent == NULL)
        {
            m_pRoot = pNodeChild;
        }
        else if (pNodeRemove == pNodeParent->m_pLeft)
        {
            pNodeParent->m_pLeft = pNodeChild;
        }
        else
        {
            pNodeParent->m_pRight = pNodeChild;
        }

        pNodeChild->m_pParent = pNodeParent;
    }

    AdjustHeight(pNodeRemove->m_pParent);

    while (pNodeRemove != NULL)
    {
        CNode *pNodeTemp = pNodeRemove;
        pNodeRemove = pNodeRemove->m_pNext;
        delete pNodeTemp;
        m_nCount--;
    }

    return *this;
}

//移除结点
template <typename T>
CMyTree<T>& CMyTree<T>::RemovAt(POSITION posDest, bool(*pfnMatch)(const T&, void*), void *pParam)
{
    CNode *pNodeRemove = (CNode *)posDest;

    if (pNodeRemove == NULL)
    {
        return *this;
    }

    CNode *pNodeCur = pNodeRemove;

    //删除链表中除头结点外满足条件的结点
    while (pNodeCur->m_pNext != NULL)
    {
        CNode *pNodeTemp = pNodeCur->m_pNext;

        if (pfnMatch(pNodeTemp->m_Data, pParam))
        {
            pNodeCur->m_pNext = pNodeTemp->m_pNext;
            delete pNodeTemp;
            m_nCount--;
        }
        else
        {
            pNodeCur = pNodeCur->m_pNext;
        }
    }

    //头结点也满足条件
    if (pfnMatch(pNodeTemp->m_Data, pParam))
    {
        //头结点下一个不为空，则将下一个移动到头结点位置
        if (pNodeRemove->m_pNext != NULL)
        {
            //删除头结点
            CNode *pNodeHead = pNodeRemove->m_pNext;
            CNode *pNodeParent = pNodeRemove->m_pParent;

            pNodeHead->m_nHeight = pNodeRemove->m_nHeight;
            pNodeHead->m_pParent = pNodeRemove->m_pParent;
            pNodeHead->m_pLeft = pNodeRemove->m_pLeft;
            pNodeHead->m_pRight = pNodeRemove->m_pRight;

            if (pNodeParent == NULL)
            {
                m_pRoot = pNodeHead;
            }
            else if (pNodeParent->m_pLeft == pNodeRemove)
            {
                pNodeParent->m_pLeft = pNodeHead;
            }
            else
            {
                pNodeParent->m_pRight = pNodeHead;
            }

            delete pNodeRemove;
            m_nCount--;
        }
        else //否则删除头结点
        {
            RemovAt(posDest);
        }
    }

    return *this;
}

//查找结点
template <typename T>
POSITION CMyTree<T>::Find(const T& Data)
{
    CNode *pNodeCur = m_pRoot;
    
    while (pNodeCur != NULL)
    {
        if (pNodeCur->m_Data > Data) //在左子树中找
        {
            pNodeCur = pNodeCur->m_pLeft;
        }
        else if (Data > pNodeCur->m_Data) //在右子树中找
        {
            pNodeCur = pNodeCur->m_pRight;
        }
        else
        {
            return pNodeCur;
        }
    }

    return NULL;
}

//前序遍历
template <typename T>
void CMyTree<T>::PreOrder(void(*pfnAction)(const T&, void*), void *pParam) const
{
    CStack<CNode*> stack;
    
    CNode *pNodeCur = m_pRoot;
    
    while (pNodeCur != NULL || !stack.IsEmpty())
    {
        while (pNodeCur != NULL)
        {
            stack.Push(pNodeCur);

            if (pfnAction != NULL)
            {
                CNode *pNodeTemp = pNodeCur;
                while (pNodeTemp != NULL)
                {
                    (*pfnAction)(pNodeTemp->m_Data, pParam);
                    pNodeTemp = pNodeTemp->m_pNext;
                }
            }

            pNodeCur = pNodeCur->m_pLeft;
        }
        
        pNodeCur = stack.Pop();

        pNodeCur = pNodeCur->m_pRight;
    }
}

//中序遍历
template <typename T>
void CMyTree<T>::InOrder(void(*pfnAction)(const T&, void*), void *pParam) const
{
    CStack<CNode*> stack;

    CNode *pNodeCur = m_pRoot;

    while (pNodeCur != NULL || !stack.IsEmpty())
    {
        while (pNodeCur != NULL)
        {
            stack.Push(pNodeCur);

            pNodeCur = pNodeCur->m_pLeft;
        }

        pNodeCur = stack.Pop();
        
        if (pfnAction != NULL)
        {
            CNode *pNodeTemp = pNodeCur;
            while (pNodeTemp != NULL)
            {
                (*pfnAction)(pNodeTemp->m_Data, pParam);
                pNodeTemp = pNodeTemp->m_pNext;
            }
        }
        
        pNodeCur = pNodeCur->m_pRight;
    }
}

//后序遍历
template <typename T>
void CMyTree<T>::PostOrder(void(*pfnAction)(const T&, void*), void *pParam) const
{
    CStack<CNode*> stack;
    
    CNode *pNodeCur = m_pRoot; //当前结点
    CNode *pNodePrev = m_pRoot; //前一个出栈结点

    while (pNodeCur != NULL || !stack.IsEmpty())
    {
        while (pNodeCur != NULL)
        {
            stack.Push(pNodeCur);

            pNodeCur = pNodeCur->m_pLeft;
        }

        pNodeCur = stack.Top();

        if (pNodeCur->m_pRight == NULL ||
            pNodeCur->m_pRight == pNodePrev)
        {
            stack.Pop();

            if (pfnAction != NULL)
            {
                CNode *pNodeTemp = pNodeCur;
                while (pNodeTemp != NULL)
                {
                    (*pfnAction)(pNodeTemp->m_Data, pParam);
                    pNodeTemp = pNodeTemp->m_pNext;
                }
            }

            pNodePrev = pNodeCur;
            pNodeCur = NULL;
        }
        else
        {
            pNodeCur = pNodeCur->m_pRight;
        }
    }
}

//层序遍历
template <typename T>
void CMyTree<T>::LayerOrder(void(*pfnAction)(const T&, void*), void *pParam) const
{
    if (m_pRoot == NULL)
    {
        return;
    }

    CQueue<CNode*> queue;

    CNode *pNodeCur = m_pRoot;

    while (true)
    {
        if (pfnAction != NULL)
        {
            CNode *pNodeTemp = pNodeCur;
            while (pNodeTemp != NULL)
            {
                (*pfnAction)(pNodeTemp->m_Data, pParam);
                pNodeTemp = pNodeTemp->m_pNext;
            }
        }

        if (pNodeCur->m_pLeft != NULL)
        {
            queue.EnQueue(pNodeCur->m_pLeft);
        }

        if (pNodeCur->m_pRight != NULL)
        {
            queue.EnQueue(pNodeCur->m_pRight);
        }

        if (queue.IsEmpty())
        {
            break;
        }

        pNodeCur = queue.DeQueue();
    }
}

//拷贝树
template <typename T>
CMyTree<T>& CMyTree<T>::Copy(const CMyTree<T>& obj)
{
    if (obj.m_pRoot == NULL)
    {
        return *this;
    }
    
    CQueue<CNode*> queue;
    
    CNode *pNodeCur = obj.m_pRoot;
    
    while (true)
    {
        CNode *pNodeTemp = pNodeCur;
        while (pNodeTemp != NULL)
        {
            Insert(pNodeTemp->m_Data);
            pNodeTemp = pNodeTemp->m_pNext;
        }
        
        if (pNodeCur->m_pLeft != NULL)
        {
            queue.EnQueue(pNodeCur->m_pLeft);
        }
        
        if (pNodeCur->m_pRight != NULL)
        {
            queue.EnQueue(pNodeCur->m_pRight);
        }
        
        if (queue.IsEmpty())
        {
            break;
        }
        
        pNodeCur = queue.DeQueue();
    }

    return *this;
}

//移除树
template <typename T>
CMyTree<T>& CMyTree<T>::RemoveAll()
{
    CStack<CNode*> stack;
    
    CNode *pNodeCur = m_pRoot;
    CNode *pNodePrev = m_pRoot;
    
    //后序遍历删除结点
    while (pNodeCur != NULL || !stack.IsEmpty())
    {
        while (pNodeCur != NULL)
        {
            stack.Push(pNodeCur);
            
            pNodeCur = pNodeCur->m_pLeft;
        }
        
        pNodeCur = stack.Top();
        
        if (pNodeCur->m_pRight == NULL ||
            pNodeCur->m_pRight == pNodePrev)
        {
            stack.Pop();
            
            pNodePrev = pNodeCur;

            while (pNodeCur != NULL)
            {
                CNode *pNodeTemp = pNodeCur;
                pNodeCur = pNodeCur->m_pNext;
                delete pNodeTemp;
            }
        }
        else
        {
            pNodeCur = pNodeCur->m_pRight;
        }
    }

    m_pRoot = NULL;
    m_nCount = 0;
    
    return *this;
}

template <typename T>
CMyTree<T>& CMyTree<T>::operator= (const CMyTree<T>& obj)
{
    if (this == &obj)
    {
        return *this;
    }

    RemoveAll();
    Copy(obj);

    return *this;
}

template <typename T>
CMyTree<T>::CNode::CNode()
    : m_pParent(NULL)
    , m_nHeight(1)
    , m_pLeft(NULL)
    , m_pRight(NULL)
    , m_pNext(NULL)
{

}

template <typename T>
CMyTree<T>::CNode::CNode(const T& Data)
    : m_Data(Data)
    , m_nHeight(1)
    , m_pParent(NULL)
    , m_pLeft(NULL)
    , m_pRight(NULL)
    , m_pNext(NULL)
{

};


template <typename T>
inline int CMyTree<T>::Height(CNode *pNode)
{
    return pNode == NULL ? 0 : pNode->m_nHeight;
}

//调整高度
template <typename T>
void CMyTree<T>::AdjustHeight(CNode *pNodeCur)
{
    while (pNodeCur != NULL)
    {
        int nLeft = Height(pNodeCur->m_pLeft);
        int nRight = Height(pNodeCur->m_pRight);
        int nHeight = nLeft - nRight;

        pNodeCur->m_nHeight = MAX(nLeft, nRight) + 1;

        if (nHeight >= 2) //根->左
        {
            CNode *pLeft = pNodeCur->m_pLeft;
            
            //根->左->左
            if (Height(pLeft->m_pLeft) >= Height(pLeft->m_pRight))
            {
                RightRotate(pNodeCur);
            }
            else //根->左->右
            {
                LeftRotate(pLeft);
                RightRotate(pNodeCur);
            }
        }
        else if (nHeight <= -2) //根->右
        {
            CNode *pRight = pNodeCur->m_pRight;
            
            //根->右->右
            if (Height(pRight->m_pLeft) <= Height(pRight->m_pRight))
            {
                LeftRotate(pNodeCur);
            }
            else //根->右->左
            {
                RightRotate(pRight);
                LeftRotate(pNodeCur);
            }
        }

        pNodeCur = pNodeCur->m_pParent;
    }
}

//右旋
template <typename T>
void CMyTree<T>::RightRotate(CNode *pK1) //右单旋转
{
    /*
               D                 D
            K1    E          K2     E
         K2    C    →    A     K1
       A    B                 B    C

             A和C结点不受影响

       k1 = max(ha, hb) + 1 - hc => hc = max(ha, hb) + 1 - k1
       k2 = ha - hb => ha = hb + k2

       k1' = hb - hc = hb - max(ha, hb) - 1 + k1
       ha > hb 即 k2 > 0 => k1' = hb - ha - 1 + k1 = k1 - k2 - 1
       ha < hb 即 k2 < 0 => k1' = hb - hb - 1 + k1 = k1 - 1

       k2' = ha - max(hb, hc) - 1
       hb > hc 即 k1' > 0 => k2' = ha - hb - 1 = k2 - 1
       hb < hc 即 k1' < 0 => k2' = ha - hc - 1 = hb + k2 - hc - 1 = k2 + k1' - 1
    */

    CNode *pK2 = pK1->m_pLeft;
    CNode *pB = pK2->m_pRight;
    CNode *pD = pK1->m_pParent;

    if (pB != NULL)
    {
        pB->m_pParent = pK1;
    }

    if (pD == NULL)
    {
        m_pRoot = pK2;
    }
    else if (pK1 == pD->m_pLeft)
    {
        pD->m_pLeft = pK2;
    }
    else
    {
        pD->m_pRight = pK2;
    }

    pK1->m_pLeft = pB;
    pK1->m_pParent = pK2;
    pK1->m_nHeight = MAX(Height(pB), Height(pK1->m_pRight)) + 1;

    pK2->m_pRight = pK1;
    pK2->m_pParent = pD;
    pK2->m_nHeight = MAX(Height(pK2->m_pLeft), Height(pK1)) + 1;
}

//左旋
template <typename T>
void CMyTree<T>::LeftRotate(CNode *pK1) //右单旋转
{
    /*
             D                     D
          K1                    K2
       A     K2     →       K1     C 
           B    C         A     B

             A和C结点不受影响

       k1 = ha - max(hb, hc) - 1 => ha = max(hb, hc) + 1 + k1
       k2 = hb - hc => hc = hb - k2
  
       k1' = ha - hb = max(hb, hc) + 1 - hb + k1
       hb > hc 即 k2 > 0 => k1' = hb + 1 - hb + k1 = k1 + 1
       hb < hc 即 k2 < 0 => k1' = hc + 1 - hb + k1 = k1 - k2 + 1
    
       k2' = max(ha, hb) + 1 - hc
       ha > hb 即 k1' > 0 => k2' = ha + 1 - hc = ha + 1 - hb + k2 = k2 + k1' + 1
       ha < hb 即 k1' < 0 => k2' = hb + 1 - hc = k2 + 1
    */

    CNode *pK2 = pK1->m_pRight;
    CNode *pB = pK2->m_pLeft;
    CNode *pD = pK1->m_pParent;

    if (pB != NULL)
    {
        pB->m_pParent = pK1;
    }

    if (pD == NULL)
    {
        m_pRoot = pK2;
    }
    else if (pK1 == pD->m_pLeft)
    {
        pD->m_pLeft = pK2;
    }
    else
    {
        pD->m_pRight = pK2;
    }

    pK1->m_pRight = pB;
    pK1->m_pParent = pK2;
    pK1->m_nHeight = MAX(Height(pK1->m_pLeft), Height(pB)) + 1;

    pK2->m_pLeft = pK1;
    pK2->m_pParent = pD;
    pK2->m_nHeight = MAX(Height(pK1), Height(pK2->m_pRight)) + 1;
}

#endif // !defined(AFX_MYTREE_H__A1340351_C1D4_415A_A650_F6C3CA0C1402__INCLUDED_)
