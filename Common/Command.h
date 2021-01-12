#pragma once

#include "list.h"

class ICommand;
typedef CMyList<ICommand*> CommandList;

/************************************************************************/
/*                                命令                                  */
/************************************************************************/
class ICommand 
{
public:
    ICommand();
    virtual ~ICommand();

public:
    virtual void UnExcute() = 0; //撤销
    virtual void Excute() = 0; //重做
};


/************************************************************************/
/*                              命令管理                                */
/************************************************************************/
class CCommandManager
{
public:
    CCommandManager();
    virtual ~CCommandManager();

public:
    void UnExcute(); //撤销
    void Excute(); //重做
    void Insert(ICommand *pCommand);

protected:
    void RemoveAll(CommandList& lstCommand);

protected:
    CommandList m_lstUnExcuteCommands; //撤销链表
    CommandList m_lstExcuteCommands; //重做链表
};



/************************************************************************/
/*             实现(为了兼容MFC的CList,将所有函数设为inline)            */
/************************************************************************/
inline ICommand::ICommand()
{
}


inline ICommand::~ICommand()
{
}


inline CCommandManager::CCommandManager()
{
}


inline CCommandManager::~CCommandManager()
{
    RemoveAll(m_lstUnExcuteCommands);
    RemoveAll(m_lstExcuteCommands);
}

//撤销
inline void CCommandManager::UnExcute()
{
    if (!m_lstUnExcuteCommands.IsEmpty())
    {
        ICommand *lpCommand = m_lstUnExcuteCommands.GetTail();
        lpCommand->UnExcute();

        m_lstUnExcuteCommands.RemoveTail();
        m_lstExcuteCommands.AddTail(lpCommand);
    }
}

//重做
inline void CCommandManager::Excute()
{
    if (!m_lstExcuteCommands.IsEmpty())
    {
        ICommand *lpCommand = m_lstExcuteCommands.GetTail();
        lpCommand->Excute();

        m_lstExcuteCommands.RemoveTail();
        m_lstUnExcuteCommands.AddTail(lpCommand);
    }
}


inline void CCommandManager::Insert(ICommand *lpCommand)
{
    m_lstUnExcuteCommands.AddTail(lpCommand);
    RemoveAll(m_lstExcuteCommands);
}


inline void CCommandManager::RemoveAll(CommandList& lstCommand)
{
    CommandList::iterator itCommand = lstCommand.begin();
    for (; itCommand != lstCommand.end(); itCommand++)
    {
        delete *itCommand;
    }
    lstCommand.RemoveAll();
}