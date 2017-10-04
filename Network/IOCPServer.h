#pragma once

#include "../Common/Singleton.h"
#include "NetworkDef.h"
#include "Socket.h"
#include "ClientManager.h"
#include "IOCPSpeed.h"
#include "Packet.h"

typedef void(*PFNIOCPNotify)(UINT, ClientContext*);

class CIOCPServer
{
    DECLARE_SINGLETON(CIOCPServer)
protected:
    CIOCPServer();
    virtual ~CIOCPServer();

public:
    inline bool IsRunning();
    inline int GetPort();
    inline int GetConnections();

    bool Initialize(PFNIOCPNotify pfnNotify, int nMaxConnect, int nPort); //初始化
    void ShutDown(); //结束

    bool CreateCompletionPort(); //创建完成端口
    void CloseCompletionPort(); //关闭完成端口

    static unsigned __stdcall ListenThreadProc(LPVOID lpParam); //监听线程
    static unsigned __stdcall ThreadPoolFunc(LPVOID lpParam); //线程池线程

    void Send(AccessToken& Token, CBuffer& Buff, PacketSize nSize = 0); //默认数据包大小为buff大小
    void Send(ClientContext* pContext, CBuffer& Buff, PacketSize nSize = 0); //默认数据包大小为buff大小
    void Send(AccessToken& Token, CPacket Packet); //发送数据包
    void Send(ClientContext* pContext, CPacket Packet); //发送数据包
    bool PostSend(ClientContext* pContext); //投递发送请求
    void PostRecv(ClientContext* pContext); //投递接收请求
    void PostRemove(ClientContext* pContext); //投递移除客户端请求
    void RemoveClient(ClientContext* pContext); //移除客户端

    CMyStringA GetHostName();
    CMyStringA GetHostName(SOCKET socket);
    CMyStringA GetHostAddr();
    CMyStringA GetHostAddr(const char *pszHostName);

protected:
    BEGIN_IOMESSAGE_MAP()
        DECLARE_IOMESSAGE(IO_INITIALIZE, OnInitialize)
        DECLARE_IOMESSAGE(IO_FINALIZE, OnFinalize)
        DECLARE_IOMESSAGE(IO_READ, OnRead)
        DECLARE_IOMESSAGE(IO_WRITE, OnWrite)
    END_IOMESSAGE_MAP()

    void OnAccept(); //接收连接
    void OnInitialize(ClientContext* pContext, DWORD dwIoSize);
    void OnFinalize(ClientContext* pContext, DWORD dwIoSize);
    void OnRead(ClientContext* pContext, DWORD dwIoSize);
    void OnWrite(ClientContext* pContext, DWORD dwIoSize);

public:
    inline DWORD GetSendSpeed();
    inline DWORD GetRecvSpeed();

protected:
    LONG      m_nPort;
    LONG      m_nWorkerCnt; //工作线程数
    LONG      m_nCurrentCnt; //当前线程程数
    LONG      m_nBusyCnt; //忙碌线程数
    UINT      m_nMaxConnect; //最大连接数
    HANDLE    m_hExitEvent; //退出事件
    HANDLE    m_hCompletionPort; //完成端口句柄
    CMySocket m_ListenSocket; //监听Socket
    WSAEVENT  m_hAcceptEvent; //Socket接收事件
    HANDLE    m_hListenThread; //监听线程
    PFNIOCPNotify m_pfnNotify; //回调函数
    CClientManager m_ClientMgr;

protected:
    CIOCPSpeed m_SendSpeed;
    CIOCPSpeed m_RecvSpeed;
};


inline bool CIOCPServer::IsRunning()
{
    return m_hExitEvent != NULL && WaitForSingleObject(m_hExitEvent, 0) == WAIT_OBJECT_0;
}


inline int CIOCPServer::GetPort()
{
    return IsRunning() ? m_nPort : 0;
}


inline int CIOCPServer::GetConnections()
{
    return m_ClientMgr.GetCount();
}


inline DWORD CIOCPServer::GetSendSpeed()
{
    return m_SendSpeed.GetSpeed();
}


inline DWORD CIOCPServer::GetRecvSpeed()
{
    return m_RecvSpeed.GetSpeed();
}