#pragma once

#include "list.h"

class ICmd;
typedef CMyList<ICmd*> CommandList;

/************************************************************************/
/*                                命令                                  */
/************************************************************************/
class ICmd 
{
public:
    ICmd();
    virtual ~ICmd();

public:
    virtual void UnExcute() = 0; //撤销
    virtual void Excute() = 0; //重做
};


/************************************************************************/
/*                              命令管理                                */
/************************************************************************/
class CCmdManager
{
public:
    CCmdManager();
    virtual ~CCmdManager();

public:
    void UnExcute(); //撤销
    void Excute(); //重做
    void Insert(ICmd *pCmd);

protected:
    void RemoveAll(CommandList& lstCmds);

protected:
    CommandList m_lstUnExcuteCmds; //撤销链表
    CommandList m_lstExcuteCmds; //重做链表
};



/************************************************************************/
/*             实现(为了兼容MFC的CList,将所有函数设为inline)            */
/************************************************************************/
inline ICmd::ICmd()
{
}


inline ICmd::~ICmd()
{
}


inline CCmdManager::CCmdManager()
{
}


inline CCmdManager::~CCmdManager()
{
    RemoveAll(m_lstUnExcuteCmds);
    RemoveAll(m_lstExcuteCmds);
}

//撤销
inline void CCmdManager::UnExcute()
{
    if (!m_lstUnExcuteCmds.IsEmpty())
    {
        ICmd *pCmd = m_lstUnExcuteCmds.GetTail();
        pCmd->UnExcute();

        m_lstUnExcuteCmds.RemoveTail();
        m_lstExcuteCmds.AddTail(pCmd);
    }
}

//重做
inline void CCmdManager::Excute()
{
    if (!m_lstExcuteCmds.IsEmpty())
    {
        ICmd *pCmd = m_lstExcuteCmds.GetTail();
        pCmd->Excute();

        m_lstExcuteCmds.RemoveTail();
        m_lstUnExcuteCmds.AddTail(pCmd);
    }
}


inline void CCmdManager::Insert(ICmd *pCmd)
{
    m_lstUnExcuteCmds.AddTail(pCmd);
    RemoveAll(m_lstExcuteCmds);
}


inline void CCmdManager::RemoveAll(CommandList& lstCmds)
{
    CommandList::iterator itCmd = lstCmds.begin();
    for (; itCmd != lstCmds.end(); itCmd++)
    {
        delete *itCmd;
    }
    lstCmds.RemoveAll();
}