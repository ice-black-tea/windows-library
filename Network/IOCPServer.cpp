#include <process.h>
#include "Packet.h"
#include "IOCPServer.h"

#ifndef __AFX_H__
#define TRACE
#endif // __AFX_H__


struct OVERLAPPEDPLUS
{
    OVERLAPPED m_ol;
    IO_TYPE    m_ioType;

    OVERLAPPEDPLUS(IO_TYPE ioType)
    {
        memset(this, 0, sizeof(OVERLAPPEDPLUS));
        m_ioType = ioType;
    }
};


CIOCPServer::CIOCPServer()
    : m_nWorkerCnt(0)
    , m_nCurrentCnt(0)
    , m_nBusyCnt(0)
    , m_hCompletionPort(NULL)
    , m_hExitEvent(NULL)
    , m_hAcceptEvent(WSA_INVALID_EVENT)
    , m_hListenThread(INVALID_HANDLE_VALUE)
{
    
}


CIOCPServer::~CIOCPServer()
{
    ShutDown();
}


bool CIOCPServer::Initialize(PFNIOCPNotify pfnNotify, int nMaxConnect, int nPort)
{
    bool bRet = false;

    m_pfnNotify = pfnNotify;
    m_nMaxConnect = nMaxConnect;
    m_nPort = nPort;

    //退出事件
    m_hExitEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
    if (m_hExitEvent == NULL)
    {
        TRACE(TEXT("CreateEvent error: %ld\n"), WSAGetLastError());;
        goto FAIL_LABLE;
    }

    //创建Socket
    bRet = m_ListenSocket.WSASocket(SOCK_STREAM, 0, NULL, WSA_FLAG_OVERLAPPED);
    if (!bRet)
    {
        TRACE(TEXT("WSASocket error: %ld\n"), WSAGetLastError());;
        goto FAIL_LABLE;
    }

    //创建事件
    m_hAcceptEvent = WSACreateEvent();
    if (m_hAcceptEvent == WSA_INVALID_EVENT)
    {
        TRACE(TEXT("WSACreateEvent error: %ld\n"), WSAGetLastError());;
        goto FAIL_LABLE;
    }

    //创建接收事件
    int nRet = m_ListenSocket.WSAEventSelect(m_hAcceptEvent, FD_ACCEPT);
    if (nRet == SOCKET_ERROR)
    {
        TRACE(TEXT("WSAEventSelect error: %ld\n"), WSAGetLastError());;
        goto FAIL_LABLE;
    }

    //绑定端口
    bRet = m_ListenSocket.Bind(INADDR_ANY, nPort);
    if (!bRet)
    {
        TRACE(TEXT("Bind error: %ld\n"), WSAGetLastError());;
        goto FAIL_LABLE;
    }

    //监听
    bRet = m_ListenSocket.Listen();
    if (!bRet)
    {
        TRACE(TEXT("Listen error: %ld\n"), WSAGetLastError());;
        goto FAIL_LABLE;
    }

    //创建完成端口
    bRet = CreateCompletionPort();
    if (!bRet)
    {
        TRACE(TEXT("CreateCompletionPort error: %ld\n"), WSAGetLastError());;
        goto FAIL_LABLE;
    }

    //监听线程
    m_hListenThread = (HANDLE)_beginthreadex(
        NULL,
        0,
        ListenThreadProc,
        this,
        0,
        NULL);
    if (m_hListenThread == INVALID_HANDLE_VALUE)
    {
        TRACE(TEXT("_beginthreadex error: %ld\n"), WSAGetLastError());;
        goto FAIL_LABLE;
    }

    return true;

FAIL_LABLE:

    ShutDown();

    return false;
}


void CIOCPServer::ShutDown()
{
    if (m_hExitEvent == NULL)
    {
        return;
    }

    ResetEvent(m_hExitEvent);
    CloseCompletionPort();
    
    if (m_ListenSocket.IsValid())
    {
        m_ListenSocket.Close();
    }

    if (m_hAcceptEvent != WSA_INVALID_EVENT)
    {
        WSACloseEvent(m_hAcceptEvent);
        m_hAcceptEvent = WSA_INVALID_EVENT;
    }

    if (m_hListenThread != INVALID_HANDLE_VALUE)
    {
        WaitForSingleObject(m_hListenThread, INFINITE);
        CloseHandle(m_hListenThread);
        m_hListenThread = INVALID_HANDLE_VALUE;
    }

    if (m_hExitEvent != NULL)
    {
        CloseHandle(m_hExitEvent);
        m_hExitEvent = NULL;
    }
}


bool CIOCPServer::CreateCompletionPort()
{
    //创建完成端口
    m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (m_hCompletionPort == NULL)
    {
        TRACE(TEXT("CreateIoCompletionPort error: %ld\n"), WSAGetLastError());;
        return false;
    }

    m_nWorkerCnt = 0;

    //设置线程数
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    int nWorker = sysInfo.dwNumberOfProcessors * 2;

    //创建工作线程
    for (int i = 0; i < nWorker; i++)
    {
        HANDLE hThread = (HANDLE)_beginthreadex(
            NULL,
            0,
            ThreadPoolFunc,
            this,
            0,
            NULL);

        if (hThread == NULL)
        {
            TRACE(TEXT("_beginthreadex error: %ld\n"), WSAGetLastError());;
            return false;
        }

        m_nWorkerCnt++;
        CloseHandle(hThread);
    }

    return true;
}


void CIOCPServer::CloseCompletionPort()
{
    if (m_hCompletionPort != NULL)
    {
        while (m_nWorkerCnt)
        {
            PostQueuedCompletionStatus(m_hCompletionPort, 0, (ULONG_PTR)NULL, NULL);
            Sleep(100);
        }

        CloseHandle(m_hCompletionPort);
        m_hCompletionPort = NULL;
    }

    m_ClientMgr.RemoveAll();
}

unsigned CIOCPServer::ListenThreadProc(LPVOID lParam)
{
    CIOCPServer *pThis = (CIOCPServer*)lParam;

    while (pThis->IsRunning())
    {
        //瞅瞅有没有新的连接
        DWORD dwRet = WSAWaitForMultipleEvents(1, &pThis->m_hAcceptEvent, FALSE, 200, FALSE);

        if (dwRet == WSA_WAIT_TIMEOUT)
        {
            continue;
        }

        //枚举事件
        WSANETWORKEVENTS events;
        bool bRet = pThis->m_ListenSocket.WSAEnumNetworkEvents(pThis->m_hAcceptEvent, &events);

        if (!bRet)
        {
            break;
        }

        //瞅瞅接收有没有出错
        if (events.lNetworkEvents & FD_ACCEPT)
        {
            if (events.iErrorCode[FD_ACCEPT_BIT] != 0)
            {
                break;
            }

            pThis->OnAccept();//该接收连接了
        }
    }

    return 0;
}


unsigned __stdcall CIOCPServer::ThreadPoolFunc(LPVOID lpParam)
{
    ULONG ulFlags = MSG_PARTIAL;
    CIOCPServer* pThis = (CIOCPServer*)lpParam;
    HANDLE hCompletionPort = pThis->m_hCompletionPort;

    InterlockedIncrement(&pThis->m_nCurrentCnt);

    while (pThis->IsRunning())
    {
        DWORD dwIoSize = 0;
        ClientContext* pContext = NULL;
        LPOVERLAPPED lpOverlapped = NULL;

        BOOL bRet = GetQueuedCompletionStatus(
            hCompletionPort,
            &dwIoSize,
            (PULONG_PTR)&pContext,
            &lpOverlapped,
            INFINITE);

        //忙碌线程数
        int nBusyCnt = InterlockedIncrement(&pThis->m_nBusyCnt);

        OVERLAPPEDPLUS* pOverlapPlus = CONTAINING_RECORD(lpOverlapped, OVERLAPPEDPLUS, m_ol);

        if (!bRet && GetLastError() != WAIT_TIMEOUT)
        {
            if (pContext != NULL && pThis->IsRunning())
            {
                pThis->RemoveClient(pContext);
            }
        }
        else if (bRet && pOverlapPlus != NULL && pContext != NULL)
        {
            OnMsgHandle(pOverlapPlus->m_ioType, pThis, pContext, dwIoSize);
        }

        if (pOverlapPlus)
        {
            delete pOverlapPlus;
        }

        InterlockedDecrement(&pThis->m_nBusyCnt);
    }

    InterlockedDecrement(&pThis->m_nWorkerCnt);
    InterlockedDecrement(&pThis->m_nCurrentCnt);

    return 0;
}


void CIOCPServer::Send(AccessToken& Token, CBuffer& Buff, PacketSize nSize)
{
    ClientContext* pContext = m_ClientMgr.Find(Token);

    if (pContext == NULL || pContext->m_Token != Token)
    {
        return;
    }

    LOCK_SCOPE(pContext->m_lockSend);

    bool bPost = pContext->m_SendBuff.IsEmpty();

    if (nSize == 0)
    {
        nSize = Buff.GetReadableSize();
    }

    pContext->m_SendBuff.Write(Buff.Read(nSize), nSize);

    m_pfnNotify(NC_SEND, pContext);

    if (bPost)
    {
        PostSend(pContext);
    }
}


void CIOCPServer::Send(ClientContext* pContext, CBuffer& Buff, PacketSize nSize)
{
    if (pContext == NULL)
    {
        return;
    }

    LOCK_SCOPE(pContext->m_lockSend);

    bool bPost = pContext->m_SendBuff.IsEmpty();

    if (nSize == 0)
    {
        nSize = Buff.GetReadableSize();
    }

    pContext->m_SendBuff.Write(Buff.Read(nSize), nSize);

    m_pfnNotify(NC_SEND, pContext);

    if (bPost)
    {
        PostSend(pContext);
    }
}


void CIOCPServer::Send(AccessToken& Token, CPacket Packet)
{
    ClientContext* pContext = m_ClientMgr.Find(Token);

    if (pContext == NULL || pContext->m_Token != Token)
    {
        return;
    }

    LOCK_SCOPE(pContext->m_lockSend);

    bool bPost = pContext->m_SendBuff.IsEmpty();

    Packet.OnWrite(pContext->m_SendBuff);

    m_pfnNotify(NC_SEND, pContext);

    if (bPost)
    {
        PostSend(pContext);
    }
}


void CIOCPServer::Send(ClientContext* pContext, CPacket Packet)
{
    if (pContext == NULL)
    {
        return;
    }

    LOCK_SCOPE(pContext->m_lockSend);

    bool bPost = pContext->m_SendBuff.IsEmpty();

    Packet.OnWrite(pContext->m_SendBuff);

    m_pfnNotify(NC_SEND, pContext);

    if (bPost)
    {
        PostSend(pContext);
    }
}


//投递一个发送请求
bool CIOCPServer::PostSend(ClientContext* pContext)
{
    WSABUF wsaSendBuff;
    wsaSendBuff.buf = (CHAR*)pContext->m_SendBuff.GetBuffer();
    wsaSendBuff.len = (ULONG)pContext->m_SendBuff.GetReadableSize();
    DWORD dwSendBytes = 0;

    OVERLAPPEDPLUS* pOverLapedPlus = new OVERLAPPEDPLUS(IO_WRITE);

    if (pOverLapedPlus == NULL)
    {
        return false;
    }

    int nRet = WSASend(
        pContext->m_Socket,
        &wsaSendBuff,
        1,
        &dwSendBytes,
        0,
        &pOverLapedPlus->m_ol,
        NULL);

    if (nRet == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
    {
        TRACE(TEXT("WSASend error: %ld\n"), WSAGetLastError());;
        return false;
    }

    return true;
}


//投递接收请求
void CIOCPServer::PostRecv(ClientContext* pContext)
{
    ULONG ulFlags = MSG_PARTIAL;
    DWORD dwNumberOfBytesRecvd;
    OVERLAPPEDPLUS *lpOverlapped = new OVERLAPPEDPLUS(IO_READ);

    pContext->m_wsaRecvBuff.len = PACKET_SIZE_RECEIVE;
    m_pfnNotify(NC_POSTRECEIVE, pContext); //接收消息回调
    pContext->m_wsaRecvBuff.buf = (CHAR*)pContext->m_RecvBuff.Write(pContext->m_wsaRecvBuff.len);//pContext->m_szRecvBuff;

    UINT nRetVal = WSARecv(
        pContext->m_Socket,
        &pContext->m_wsaRecvBuff,
        1,
        &dwNumberOfBytesRecvd,
        &ulFlags,
        &lpOverlapped->m_ol,
        NULL);

    if (nRetVal == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
    {
        TRACE(TEXT("WSARecv error: %ld\n"), WSAGetLastError());;
        RemoveClient(pContext);
    }
}


void CIOCPServer::PostRemove(ClientContext* pContext)
{
    OVERLAPPEDPLUS *lpOverlapped = new OVERLAPPEDPLUS(IO_FINALIZE);

    BOOL bSuccess = PostQueuedCompletionStatus(m_hCompletionPort, 0, (ULONG_PTR)pContext, &lpOverlapped->m_ol);

    if ((!bSuccess && GetLastError() != ERROR_IO_PENDING))
    {
        TRACE(TEXT("PostQueuedCompletionStatus error: %ld\n"), WSAGetLastError());;
        RemoveClient(pContext);
        return;
    }
}


void CIOCPServer::RemoveClient(ClientContext* pContext)
{
    m_pfnNotify(NC_DISCONNECT, pContext); //断开连接回调

    m_ClientMgr.Remove(pContext);
}


CStringA CIOCPServer::GetHostName()
{
    char szHost[MAX_PATH];

    gethostname(szHost, sizeof(szHost));

    return szHost;
}


CStringA CIOCPServer::GetHostName(SOCKET socket)
{
    sockaddr_in sockAddr = { 0 };

    int nAddrLen = sizeof(sockAddr);

    BOOL bResult = getpeername(socket, (sockaddr*)&sockAddr, &nAddrLen);

    return bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "";
}


CStringA CIOCPServer::GetHostAddr()
{
    CStringA strAddr;

    HOSTENT *host = gethostbyname(GetHostName());

    if (host != NULL)
    {
        for (int i = 0;; i++)
        {
            strAddr += inet_ntoa(*(IN_ADDR*)host->h_addr_list[i]);

            if (host->h_addr_list[i] + host->h_length >= host->h_name)
            {
                break;
            }

            strAddr += "/";
        }
    }

    return strAddr;
}


CStringA CIOCPServer::GetHostAddr(const char *pszHostName)
{
    CStringA strAddr;

    HOSTENT *host = gethostbyname(pszHostName);

    if (host != NULL)
    {
        for (int i = 0;; i++)
        {
            strAddr += inet_ntoa(*(IN_ADDR*)host->h_addr_list[i]);

            if (host->h_addr_list[i] + host->h_length >= host->h_name)
            {
                break;
            }

            strAddr += "/";
        }
    }

    return strAddr;
}


void CIOCPServer::OnAccept()
{
    //接收连接
    sockaddr_in	sAddr;
    int	nLen = sizeof(SOCKADDR_IN);
    SOCKET sClient = m_ListenSocket.Accept((sockaddr&)sAddr, nLen);

    if (sClient == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)
    {
        TRACE(TEXT("Accept error: %ld\n"), WSAGetLastError());;
        return;
    }

    ClientContext* pContext = m_ClientMgr.Create();

    if (pContext == NULL)
    {
        TRACE(TEXT("Create error: %ld\n"), WSAGetLastError());;
        return;
    }

    pContext->m_Socket = sClient;

    HANDLE hCompletionPort = CreateIoCompletionPort(
        (HANDLE)sClient, m_hCompletionPort, (ULONG_PTR)pContext, 0);

    if (hCompletionPort != m_hCompletionPort)
    {
        m_ClientMgr.Remove(pContext);
        return;
    }

    OVERLAPPEDPLUS *lpOverlapped = new OVERLAPPEDPLUS(IO_INITIALIZE);

    BOOL bSuccess = PostQueuedCompletionStatus(m_hCompletionPort, 0, (ULONG_PTR)pContext, &lpOverlapped->m_ol);

    if ((!bSuccess && GetLastError() != ERROR_IO_PENDING))
    {
        TRACE(TEXT("PostQueuedCompletionStatus error: %ld\n"), WSAGetLastError());
        m_ClientMgr.Remove(pContext);
        return;
    }

    m_pfnNotify(NC_CONNECT, pContext); //客户端连接回调

    m_ClientMgr.Insert(pContext);

    //投递接收请求
    PostRecv(pContext);
}


void CIOCPServer::OnInitialize(ClientContext* pContext, DWORD dwIoSize)
{

}


void CIOCPServer::OnFinalize(ClientContext* pContext, DWORD dwIoSize)
{
    RemoveClient(pContext);
}


void CIOCPServer::OnRead(ClientContext* pContext, DWORD dwIoSize)
{
    if (dwIoSize == 0)
    {
        RemoveClient(pContext);
        return;
    }

    m_RecvSpeed.SetSpeed(dwIoSize);

    PacketSize nSize = (PacketSize)pContext->m_RecvBuff.GetReadableSize();
    nSize = nSize - pContext->m_wsaRecvBuff.len + dwIoSize;
    pContext->m_RecvBuff.SetBufferSize(nSize);

    m_pfnNotify(NC_RECEIVE, pContext);

    PostRecv(pContext);
}


void CIOCPServer::OnWrite(ClientContext* pContext, DWORD dwIoSize)
{
    m_SendSpeed.SetSpeed(dwIoSize);

    LOCK_SCOPE(pContext->m_lockSend);

    if (!pContext->m_SendBuff.IsEmpty())
    {
        pContext->m_SendBuff.Read(dwIoSize);
    }

    if (!pContext->m_SendBuff.IsEmpty())
    {
        PostSend(pContext);
    }
}

