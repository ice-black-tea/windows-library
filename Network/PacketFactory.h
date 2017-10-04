#pragma once

#include "CustomPacket.h"


//动态创建数据包类
class CPacketFactory
{
protected:
    CPacketFactory();
    ~CPacketFactory();

public:
    static ICustomPacket *CreatePacket(PacketType type);
};


