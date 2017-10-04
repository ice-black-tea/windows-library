#include "Packet.h"
#include "PacketFactory.h"
#include "CustomPacket.h"
#include "../include/zlib/zlib.h"


CPacket::CPacket()
    : m_pPacket(NULL)
{
    
}


CPacket::CPacket(PacketType type)
{
    m_pPacket = CPacketFactory::CreatePacket(type);
}


CPacket::CPacket(ICustomPacket *pPacket)
    : m_pPacket(pPacket)
{

}


CPacket::CPacket(const CPacket& obj)
{

}


CPacket::~CPacket()
{
    if (m_pPacket != NULL)
    {
        delete m_pPacket;
        m_pPacket = NULL;
    }
}


bool CPacket::OnWrite(CBuffer& Buff, AccessToken *pToken)
{
    if (m_pPacket == NULL)
    {
        return false;
    }

    //记录一下原来有多长
    PacketSize nSize = (PacketSize)Buff.GetReadableSize();

    //为数据包头占好坑
    PacketHead *pHead = (PacketHead*)Buff.Write(sizeof(PacketHead));

    CBuffer tempBuff;

    //写入数据包数据
    if (!m_pPacket->OnWrite(tempBuff))
    {
        return false;
    }

    if (!tempBuff.IsEmpty())
    {
        //压缩数据
        uLong nSrcLen = (uLong)tempBuff.GetReadableSize();
        uLong nDestLen = (uLong)((double)nSrcLen * 1.001) + 12;
        int	nRet = compress(
            (Byte*)Buff.Write(nDestLen), &nDestLen,
            (Byte*)tempBuff.GetBuffer(), nSrcLen);

        if (nRet != Z_OK)
        {
            Buff.SetBufferSize(nSize);
            return false;
        }

        Buff.SetBufferSize(nSize + nDestLen + sizeof(PacketHead));
    }

    //修改数据包头
    pHead->m_nSize = (PacketSize)Buff.GetReadableSize() - nSize;
    pHead->m_nSrcSize = (PacketSize)tempBuff.GetReadableSize();
    pHead->m_nType = (PacketSize)m_pPacket->GetPacketType();
    pHead->m_Token = pToken != NULL ? *pToken : NULL;

    return true;
}


bool CPacket::OnRead(CBuffer& Buff, AccessToken *pToken)
{
    if (m_pPacket != NULL)
    {
        delete m_pPacket;
        m_pPacket = NULL;
    }

    if (Buff.GetReadableSize() < sizeof(PacketHead))
    {
        return false;
    }

    PacketHead *pHead = (PacketHead*)Buff.GetBuffer();

    //数据包没接收完
    if (pHead->m_nSize > Buff.GetReadableSize())
    {
        return false;
    }

    //接收token
    if (pToken != NULL)
    {
        *pToken = pHead->m_Token;
    }

    Buff.Delete(sizeof(PacketHead));

    CBuffer tempBuff;

    if (pHead->m_nSrcSize != 0)
    {
        //解压数据
        uLongf nSrcLen = (uLongf)pHead->GetBuffSize();
        uLongf nDestLen = (uLongf)pHead->m_nSrcSize;
        int	nRet = uncompress(
            (Bytef*)tempBuff.Write(nDestLen), &nDestLen,
            (Bytef*)Buff.Read(nSrcLen), nSrcLen);

        if (nRet != Z_OK)
        {
            return false;
        }
    }

    //动态创建数据包
    m_pPacket = CPacketFactory::CreatePacket(pHead->m_nType);

    if (m_pPacket == NULL)
    {
        return false;
    }

    //处理数据包
    if (!m_pPacket->OnRead(tempBuff))
    {
        return false;
    }

    return true;
}


bool CPacket::OnRecv(CMySocket& Socket, AccessToken *pToken)
{
    CBuffer Buff;

    //占好数据包头的坑
    PacketHead *pHead = (PacketHead*)Buff.Write(sizeof(PacketHead));

    if (pHead == NULL)
    {
        return false;
    }

    //接收数据包头
    if (!Socket.RecvAll(pHead, sizeof(PacketHead)))
    {
        return false;
    }

    //接收数据长度
    PacketSize nSize = pHead->GetBuffSize();

    //接收包数据
    void *pBuff = Buff.Write(nSize);

    if (!Socket.RecvAll(pBuff, nSize))
    {
        return false;
    }

    //读数据
    if (!OnRead(Buff, pToken))
    {
        return false;
    }

    return true;
}


bool CPacket::OnSend(CMySocket& Socket, AccessToken *pToken)
{
    static CLock lock;

    CBuffer Buff;

    //写入数据包
    if (!OnWrite(Buff, pToken))
    {
        return false;
    }

    LOCK_SCOPE(lock);

    //发送数据包
    if (!Socket.SendAll(Buff, (PacketSize)Buff.GetReadableSize()))
    {
        return false;
    }

    return true;
}


bool CPacket::OnProcess(void *lpParam)
{
    if (m_pPacket == NULL)
    {
        return false;
    }

    return m_pPacket->OnProcess(lpParam);
}
