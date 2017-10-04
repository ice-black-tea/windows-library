#pragma once

#include "Socket.h"
#include "../Common.h"
#include "NetworkDef.h"
#include "CustomPacket.h"
#include "ClientContext.h"
#include "PacketFactory.h"

#pragma pack(push)
#pragma pack(1)
typedef struct tagPacketHead
{
    AccessToken m_Token;
    PacketSize  m_nSize;
    PacketSize  m_nSrcSize;
    PacketType  m_nType;
    tagPacketHead() { memset(this, 0, sizeof(tagPacketHead)); }
    PacketSize GetBuffSize() { return m_nSize - sizeof(tagPacketHead); }
}PacketHead;
#pragma pack(pop)

class CPacket : public CRefCnt
{
public:
    CPacket();
    CPacket(PacketType type);
    CPacket(ICustomPacket *pPacket);
    CPacket(const CPacket& obj);
    virtual ~CPacket();

public:
    inline static PacketType PeekType(CBuffer& Buff);
    inline static PacketSize PeekSize(CBuffer& Buff);
    inline static AccessToken PeekToken(CBuffer& Buff);
    inline static PacketHead* PeekHead(CBuffer& Buff);
    inline PacketType GetType();
    bool OnWrite(CBuffer& Buff, AccessToken *pToken = NULL);
    bool OnRead(CBuffer& Buff, AccessToken *pToken = NULL);
    bool OnRecv(CMySocket& Socket, AccessToken *pToken = NULL);
    bool OnSend(CMySocket& Socket, AccessToken *pToken = NULL);
    bool OnProcess(void *lpParam = NULL);

protected:
    ICustomPacket *m_pPacket;
};

inline PacketType CPacket::GetType()
{
    return m_pPacket != NULL ? m_pPacket->GetPacketType() : PACKET_TYPE_UNUSED;
}

inline PacketType PeekType(CBuffer& Buff)
{
    return Buff.GetReadableSize() >= sizeof(PacketHead) ?
        ((PacketHead*)Buff.GetBuffer())->m_nType : PACKET_TYPE_UNUSED;
}

inline PacketSize CPacket::PeekSize(CBuffer& Buff)
{
    return Buff.GetReadableSize() >= sizeof(PacketHead) ?
        ((PacketHead*)Buff.GetBuffer())->m_nSize : 0;
}

inline AccessToken CPacket::PeekToken(CBuffer& Buff)
{
    return Buff.GetReadableSize() >= sizeof(PacketHead) ?
        ((PacketHead*)Buff.GetBuffer())->m_Token : NULL;
}

inline PacketHead* CPacket::PeekHead(CBuffer& Buff)
{
    return Buff.GetReadableSize() >= sizeof(PacketHead) ?
        (PacketHead*)Buff.GetBuffer() : NULL;
}

typedef struct tagPacketPtr : public CSmartPtr<CPacket>
{
    tagPacketPtr() { }
    tagPacketPtr(PacketType type) : CSmartPtr<CPacket>(new CPacket(type)) { }
    tagPacketPtr(ICustomPacket *pPacket) : CSmartPtr<CPacket>(new CPacket(pPacket)) { }
    tagPacketPtr(CPacket *pPacket) : CSmartPtr<CPacket>(pPacket) { }
    ~tagPacketPtr() { }
}PacketPtr;

inline bool operator>> (CMySocket& Socket, CPacket packet)
{
    return packet.OnRecv(Socket);
}

inline bool operator<< (CMySocket& Socket, CPacket packet)
{
    return packet.OnSend(Socket);
}