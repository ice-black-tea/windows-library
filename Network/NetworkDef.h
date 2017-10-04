#pragma once

typedef void* AccessToken;
typedef unsigned char PacketType;
typedef unsigned long PacketSize;

const PacketType PACKET_TYPE_UNUSED = 0xFF;
const PacketSize PACKET_SIZE_RECEIVE = 0x1000;

//iocp模型io类型
typedef enum emIO_TYPE
{
    IO_INITIALIZE,
    IO_FINALIZE,
    IO_READ,
    IO_WRITE,
}IO_TYPE;

//iocp模型回调函数通知类型
typedef enum _NOTIFY_TYPE
{
    /*
        客户端连接(如设置Token)
    */
    NC_CONNECT,

    /*
        客户端断开
    */
    NC_DISCONNECT,

    /*
        发送数据
    */
    NC_SEND,
    
    /*
        投递接收请求
        可以修改投递接收请求的大小pContext->m_wsaRecvBuff.len
    */
    NC_POSTRECEIVE,

    /*
        收到数据
        可以对pContext->m_RecvBuff中的数据进行解析处理
    */
    NC_RECEIVE,

}NOTIFY_TYPE;

//声明自定义数据包
#define DECLARE_PACKET(class_name) \
    friend class CPacket; \
    static CPacketRegister m_Register; \
    static class_name* CreatePacket(); \
    virtual PacketType GetPacketType();

//自定义数据包实现
#define IMPLEMENT_PACKET(class_name) \
    CPacketRegister class_name::m_Register(PT_##class_name, (PFNCREATEPACKET)CreatePacket); \
    PacketType class_name::GetPacketType() { return (PacketType)PT_##class_name; } \
    class_name* class_name::CreatePacket() { return new class_name; }

//获取数据包类型
#define GET_PACKET_TYPE(class_name) ((PacketType)PT_##class_name)
#define PACKET_TYPE(class_name) PT_##class_name,

//定义IO请求处理事件
#define BEGIN_IOMESSAGE_MAP() static void OnMsgHandle(IO_TYPE ioType, CIOCPServer* pIocp, ClientContext* pContext, DWORD dwIoSize) { switch (ioType) {
#define DECLARE_IOMESSAGE(ioType, pfnHandle) case ioType: pIocp->pfnHandle(pContext, dwIoSize); break;
#define END_IOMESSAGE_MAP() } }
