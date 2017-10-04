#include "Socket.h"
#include <assert.h>


CMySocket::CMySocket()
    : m_Socket(INVALID_SOCKET)
{

}

CMySocket::CMySocket(SOCKET s)
    : m_Socket(s)
{

}

CMySocket::CMySocket(int nType, int nProtocol, int af)
    : m_Socket(INVALID_SOCKET)
{
    Socket(nType, nProtocol, af);
}

CMySocket::~CMySocket()
{
    Close();
}

CMySocket::CMySocket(const CMySocket& obj)
{

}

bool CMySocket::SendAll(const void *pBuff, int nLen, int nFlags)
{
    assert(nLen > 0);

    if (pBuff == NULL)
    {
        return false;
    }

    const char *pszBuff = (const char*)pBuff;

    int nSendLen = 0;

    while (nSendLen < nLen)
    {
        int nCurLen = send(m_Socket, pszBuff + nSendLen, nLen - nSendLen, nFlags);

        if (nCurLen == SOCKET_ERROR)
        {
            return false;
        }

        nSendLen += nCurLen;
    }

    return true;
}

bool CMySocket::RecvAll(void *pBuff, int nLen, int nFlags)
{
    assert(nLen > 0);

    if (pBuff == NULL)
    {
        return false;
    }

    char *pszBuff = (char*)pBuff;

    int nRecvLen = 0;

    while (nRecvLen < nLen)
    {
        int nCurLen = recv(m_Socket, pszBuff + nRecvLen, nLen - nRecvLen, nFlags);

        if (nCurLen == SOCKET_ERROR || nCurLen == 0)
        {
            return false;
        }

        nRecvLen += nCurLen;
    }

    return true;
}

#ifdef WIN32

CMySocket::_Socket CMySocket::s;

bool CMySocket::WSAStartup()
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD(2, 2);

    err = ::WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        return false;
    }

    /* Confirm that the WinSock DLL supports 2.2.*/
    /* Note that if the DLL supports versions greater    */
    /* than 2.2 in addition to 2.2, it will still return */
    /* 2.2 in wVersion since that is the version we      */
    /* requested.                                        */

    if (LOBYTE(wsaData.wVersion) != 2 ||
        HIBYTE(wsaData.wVersion) != 2) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        ::WSACleanup();
        return false;
    }

    return true;
}

void CMySocket::WSACleanup()
{
    ::WSACleanup();
}

#endif