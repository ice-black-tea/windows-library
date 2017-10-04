#include "CustomPacket.h"


CHashMap<PacketType, PFNCREATEPACKET> *CPacketRegister::m_pMap = NULL;


CPacketRegister::CPacketRegister(PacketType type, PFNCREATEPACKET pfn)
{
    static CHashMap<PacketType, PFNCREATEPACKET> map(11);
    
    m_pMap = &map;
    m_pMap->SetAt(type, pfn);
}


CPacketRegister::~CPacketRegister()
{

}