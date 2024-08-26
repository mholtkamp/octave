#if PLATFORM_DOLPHIN

#include "Network/Network.h"

#include "Log.h"

#include <malloc.h>
#include <unistd.h>
#include <string.h>

#include <ogcsys.h>
#include <gccore.h>
#include <network.h>

static uint32_t sLocalIp = 0;
static uint32_t sGateway = 0;
static uint32_t sSubnetMask = 0;

static bool sActive = false;

void NET_Initialize()
{
    struct in_addr localIp, netMask, gateway;
    int32_t result = if_configex(&localIp, &netMask, &gateway, true, 1);

    if (result >= 0)
    {
        sLocalIp = ntohl(localIp.s_addr);
        sSubnetMask = ntohl(netMask.s_addr);
        sGateway = ntohl(gateway.s_addr);

        sActive = true;
    }
    else
    {
        LogError("Failed to initialize Network");
        sActive = false;
    }
}

void NET_Shutdown()
{

}

void NET_Update()
{

}

bool NET_IsActive()
{
    return sActive;
}

SocketHandle NET_SocketCreate()
{
    return net_socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
}

void NET_SocketBind(SocketHandle socketHandle, uint32_t ipAddr, uint16_t port)
{
    struct sockaddr_in bindAddr;
    bindAddr.sin_family = AF_INET;
    bindAddr.sin_addr.s_addr = htonl(ipAddr);
    bindAddr.sin_port = htons(port);

    if (net_bind(socketHandle, (struct sockaddr *) &bindAddr, sizeof(bindAddr)) < 0 )
    {
        LogError("Failed to bind socket");
    }
}

int32_t NET_SocketRecv(SocketHandle socketHandle, char* buffer, uint32_t size)
{
    return net_recv(socketHandle, buffer, size, 0);
}

int32_t NET_SocketRecvFrom(SocketHandle socketHandle, char* buffer, uint32_t size, uint32_t& addr, uint16_t& port)
{
    struct sockaddr_in fromAddr;
    uint32_t fromAddrLen = (uint32_t) sizeof(fromAddr);
    int32_t numBytes = net_recvfrom(socketHandle, buffer, size, 0, (struct sockaddr*) &fromAddr, &fromAddrLen);
    addr = ntohl(fromAddr.sin_addr.s_addr);
    port = ntohs(fromAddr.sin_port);
    return numBytes;
}

int32_t NET_SocketSendTo(SocketHandle socketHandle, const char* buffer, uint32_t size, uint32_t addr, uint16_t port)
{
    // I can't send messages on Wii unless toAddr.sin_len = 8 is set and passed in as the socket addr length in net_sendto.
    // I don't know why but check out this devkitpro forum post: https://devkitpro.org/viewtopic.php?f=3&t=2177&p=5509&hilit=udp#p5509
    // sendto was returning -22 otherwise.
    struct sockaddr_in toAddr;
    toAddr.sin_family = AF_INET;
    toAddr.sin_addr.s_addr = htonl(addr);
    toAddr.sin_port = htons(port);
    toAddr.sin_len = 8;
    int32_t bytesSent = net_sendto(socketHandle, buffer, size, 0, (struct sockaddr*) &toAddr, toAddr.sin_len);
    
    //if (bytesSent < 0)
    //{
    //    LogError("Sock: %d, BytesSent = %d", socketHandle, bytesSent);
    //    LogWarning("LocalIP = %08x", sLocalIp);
    //}
    
    return bytesSent;
}

void NET_SocketClose(SocketHandle socketHandle)
{
    net_close(socketHandle);
}

void NET_SocketSetBlocking(SocketHandle socketHandle, bool blocking)
{
    int32_t flag = !blocking;
    net_ioctl(socketHandle, FIONBIO, &flag);
}

void NET_SocketSetBroadcast(SocketHandle socketHandle, bool broadcast)
{
    int broadcastEnable = (int) broadcast;
    int32_t result = net_setsockopt(socketHandle, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));

    if (result != 0)
    {
        LogError("Failed to set Broadcast flag on socket.");
    }
}

void NET_SocketGetIpAndPort(SocketHandle socketHandle, uint32_t& ipAddr, uint16_t& port)
{
    // TODO
}

uint32_t NET_IpStringToUint32(const char* ipString)
{
    uint32_t retAddr = 0;
    struct in_addr addr = {};
    inet_aton(ipString, &addr);
    retAddr = ntohl(addr.s_addr);
    return retAddr;
}

void NET_IpUint32ToString(uint32_t ipUint32, char* outIpString)
{
    struct sockaddr_in sa = {};
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(ipUint32);
    sa.sin_port = 0;

    char* staticString = inet_ntoa(sa.sin_addr);

    if (staticString != nullptr)
    {
        strncpy(outIpString, staticString, 16);
        outIpString[15] = 0;
    }
}

uint32_t NET_GetIpAddress()
{
    return sLocalIp;
}

uint32_t NET_GetSubnetMask()
{
    return sSubnetMask;
}

#endif
