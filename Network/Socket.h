#pragma once

#include "../Common/StringA.h"

#ifdef WIN32
#include <WinSock2.h> // 这个需要定义在Windows.h之前
#pragma comment(lib, "ws2_32.lib")
#else // Linux Version
#endif // WIN32

#ifndef __AFX_H__
typedef class CMySocket CSocket;
#endif // !__AFX_H__

class CMySocket
{
public:
    CMySocket();
    CMySocket(SOCKET s);
    CMySocket(int nType, int nProtocol = 0, int af = AF_INET);
    virtual ~CMySocket();

protected:
    CMySocket(const CMySocket& obj);

public:
    inline operator SOCKET& ();
    inline operator SOCKET () const;
    inline CMySocket& operator= (SOCKET s);

    inline bool IsValid() const;
    inline bool Socket(SOCKET s);
    inline bool Socket(int nType = SOCK_STREAM, int nProtocol = IPPROTO_TCP, int af = AF_INET);
    inline void Close();

    inline bool Bind(sockaddr& sAddr, int nAddrLen = sizeof(sockaddr), int af = AF_INET);
    inline bool Bind(unsigned long nAddr, unsigned short nPort, int af = AF_INET); //服务器地址可以填INADDR_ANY
    inline bool Bind(const char *pszAddr, unsigned short nPort, int af = AF_INET);
    inline bool Listen(int nBackLog = SOMAXCONN);
    inline bool Connect(sockaddr& sAddr, int nAddrLen = sizeof(sockaddr));
    inline bool Connect(unsigned long nAddr, unsigned short nPort, int af = AF_INET);
    inline bool Connect(const char *pszAddr, unsigned short nPort, int af = AF_INET);
    inline SOCKET Accept(sockaddr& sAddr, int& nAddrLen);
    inline int Recv(void *pBuff, int nLen, int nFlags = 0); //接收指定最大长度数据,nRecvLen为0时代表已关闭
    inline int Send(const void *pBuff, int nLen, int nFlags = 0); //发送指定最大长度数据
    inline int RecvFrom(void* pBuff, int nLen, sockaddr *psRecvAddr, int *pnAddrLen, int nFlags = 0);
    inline int SendTo(const void *pBuff, int nLen, const sockaddr* psSendAddr, int nAddrLen, int nFlags = 0);

    bool RecvAll(void *pBuff, int nLen, int nFlags = 0); //接收指定长度数据
    bool SendAll(const void *pBuff, int nLen, int nFlags = 0); //发送固定长度数据
    
protected:
    SOCKET m_Socket;

#ifdef WIN32
public:
    static bool WSAStartup();
    static void WSACleanup();
    inline bool WSASocket(int nType, int nProtocol, LPWSAPROTOCOL_INFO lpProtocolInfo, DWORD dwFlags, int af = AF_INET, GROUP g = 0);
    inline bool WSAEventSelect(WSAEVENT hEventObject, long lNetworkEvents);
    inline bool WSAEnumNetworkEvents(WSAEVENT hEventObject, LPWSANETWORKEVENTS lpNetworkEvents);
    inline int WSASend(LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags,
        LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine = NULL);
    inline int WSARecv(LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags,
        LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine = NULL);
    static struct _Socket { _Socket() { WSAStartup(); } ~_Socket() { WSACleanup(); } }s; //省的忘了调用WSAStartup
#endif // WIN32
};

inline CMySocket::operator SOCKET& ()
{
    return m_Socket;
}

inline CMySocket::operator SOCKET() const
{
    return m_Socket;
}

inline CMySocket& CMySocket::operator= (SOCKET s)
{
    if (m_Socket != INVALID_SOCKET)
        closesocket(m_Socket);
    m_Socket = s;

    return *this;
}

inline bool CMySocket::IsValid() const
{
    return m_Socket == INVALID_SOCKET;
}

inline bool CMySocket::Socket(int nType, int nProtocol, int af)
{
    if (m_Socket != INVALID_SOCKET)
        closesocket(m_Socket);
    m_Socket = socket(af, nType, nProtocol);
    return m_Socket != INVALID_SOCKET;
}

inline void CMySocket::Close()
{
    if (m_Socket != INVALID_SOCKET)
    {
        closesocket(m_Socket);
        m_Socket = INVALID_SOCKET;
    }
}

inline bool CMySocket::Bind(sockaddr& sAddr, int nAddrLen, int af)
{
    return bind(m_Socket, &sAddr, nAddrLen) != SOCKET_ERROR;
}

inline bool CMySocket::Bind(unsigned long nAddr, unsigned short nPort, int af)
{
    sockaddr_in addr;
    addr.sin_family = af;
    addr.sin_addr.S_un.S_addr = htonl(nAddr);
    addr.sin_port = htons(nPort);
    return Bind((sockaddr&)addr, sizeof(sockaddr_in), af);
}

inline bool CMySocket::Bind(const char *pszAddr, unsigned short nPort, int af)
{
    sockaddr_in addr;
    addr.sin_family = af;
    addr.sin_addr.S_un.S_addr = inet_addr(pszAddr);
    addr.sin_port = htons(nPort);
    return Bind((sockaddr&)addr, sizeof(sockaddr_in), af);
}

inline bool CMySocket::Connect(sockaddr& sAddr, int nAddrLen)
{
    return connect(m_Socket, &sAddr, nAddrLen) != SOCKET_ERROR;
}

inline bool CMySocket::Connect(unsigned long nAddr, unsigned short nPort, int af)
{
    sockaddr_in addr;
    addr.sin_family = af;
    addr.sin_addr.S_un.S_addr = htonl(nAddr);
    addr.sin_port = htons(nPort);
    return Connect((sockaddr&)addr, sizeof(sockaddr));
}

inline bool CMySocket::Connect(const char *pszAddr, unsigned short nPort, int af)
{
    sockaddr_in addr;
    addr.sin_family = af;
    addr.sin_addr.S_un.S_addr = inet_addr(pszAddr);
    addr.sin_port = htons(nPort);
    return Connect((sockaddr&)addr, sizeof(sockaddr));
}

inline bool CMySocket::Listen(int nBackLog)
{
    return listen(m_Socket, nBackLog) != SOCKET_ERROR;
}

inline SOCKET CMySocket::Accept(sockaddr& sAddr, int& nAddrLen)
{
    return accept(m_Socket, &sAddr, &nAddrLen);
}

inline int CMySocket::Recv(void *pBuff, int nLen, int nFlags)
{
    return recv(m_Socket, (char*)pBuff, nLen, nFlags);
}

inline int CMySocket::Send(const void *pBuff, int nLen, int nFlags)
{
    return send(m_Socket, (const char*)pBuff, nLen, nFlags);
}

inline int CMySocket::RecvFrom(void* pBuff, int nLen, sockaddr *psRecvAddr, int *pnAddrLen, int nFlags)
{
    return recvfrom(m_Socket, (char*)pBuff, nLen, nFlags, psRecvAddr, pnAddrLen);
}

inline int CMySocket::SendTo(const void *pBuff, int nLen, const sockaddr *psSendAddr, int nAddrLen, int nFlags)
{
    return sendto(m_Socket, (const char*)pBuff, nLen, nFlags, psSendAddr, nAddrLen);
}

#ifdef WIN32
inline bool CMySocket::WSASocket(int nType, int nProtocol, LPWSAPROTOCOL_INFO lpProtocolInfo, DWORD dwFlags, int af, GROUP g)
{
    m_Socket = ::WSASocket(af, nType, nProtocol, lpProtocolInfo, g, dwFlags);
    return m_Socket != INVALID_SOCKET;
}

inline bool CMySocket::WSAEventSelect(WSAEVENT hEventObject, long lNetworkEvents)
{
    return ::WSAEventSelect(m_Socket, hEventObject, lNetworkEvents) != SOCKET_ERROR;
}

inline int CMySocket::WSASend(LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags,
    LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
    return ::WSASend(m_Socket, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine);
}

inline int CMySocket::WSARecv(LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags,
        LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
    return ::WSARecv(m_Socket, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpOverlapped, lpCompletionRoutine);
}

inline bool CMySocket::WSAEnumNetworkEvents(WSAEVENT hEventObject, LPWSANETWORKEVENTS lpNetworkEvents)
{
    return ::WSAEnumNetworkEvents(m_Socket, hEventObject, lpNetworkEvents) != SOCKET_ERROR;
}
#endif // WIN32