#pragma once

#include "NetworkDef.h"
#include "Socket.h"
#include "../Util/Lock.h"
#include "../Common/Buffer.h"

typedef struct tagClientContext
{
    AccessToken m_Token;
    CMySocket   m_Socket;
    WSABUF      m_wsaRecvBuff;
    CBuffer     m_RecvBuff;
    CBuffer     m_SendBuff;
    CLock       m_lockSend;
    LPVOID      m_pParam;
}ClientContext;

