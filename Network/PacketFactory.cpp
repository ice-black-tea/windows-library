#include "PacketFactory.h"


CPacketFactory::CPacketFactory()
{
    
}


CPacketFactory::~CPacketFactory()
{

}


ICustomPacket *CPacketFactory::CreatePacket(PacketType type)
{
    PFNCREATEPACKET pfnCreate = NULL;

    if (!CPacketRegister::m_pMap->Lookup(type, pfnCreate))
    {
        return NULL;
    }

    return (*pfnCreate)();
}

