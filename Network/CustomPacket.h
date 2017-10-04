#pragma once

#include "../Common.h"
#include "NetworkDef.h"

/*
使用自定义数据包类型需要：

1.在.h中添加以下内容（只需添加一次，例如在stdafx.h中定义）
*************************************************************
//新增数据包在此处注册
#define REGISTER_PACKET(register) \
    register(CScreenPacket) \
    register(CCmdProcPacket) \
    register(CLoginPacket)

//定义数据包类型
enum { REGISTER_PACKET(PACKET_TYPE) };
*************************************************************

2.在.cpp中添加以下内容（只需添加一次，例如在stdafx.cpp中定义）
*************************************************************
REGISTER_PACKET(IMPLEMENT_PACKET)                           
*************************************************************
*/

// 动态创建的数据包类需要两个步骤
// 1.继承ICustomPacket，在类的申明中添加DECLARE_PACKET宏
// 2.在REGISTER_PACKET宏中添加该类
class ICustomPacket
{
public:
    virtual bool OnWrite(CBuffer& Buff) = 0; //从缓冲区写数据
    virtual bool OnRead(CBuffer& Buff) = 0; //从缓冲区读数据
    virtual bool OnProcess(void *lpParam) = 0; //执行对应操作

public:
    //加了DECLARE_PACKET宏之后不需要声明和实现这个纯虚函数
    virtual PacketType GetPacketType() = 0;
};


typedef ICustomPacket* (*PFNCREATEPACKET)();

//数据包注册类
class CPacketRegister
{
public:
    CPacketRegister(PacketType type, PFNCREATEPACKET pfn);
    ~CPacketRegister();

protected:
    friend class CPacketFactory;
    static CHashMap<PacketType, PFNCREATEPACKET> *m_pMap;
};
