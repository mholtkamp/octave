#if PLATFORM_LINUX

#include "Network/Network.h"

#include "Log.h"

#include <unistd.h>
#include <arpa/inet.h>
#include <ifaddrs.h>

void NET_Initialize()
{

}

void NET_Shutdown()
{

}

void NET_Update()
{

}

bool NET_IsActive()
{
    return true;
}


SocketHandle NET_SocketCreate()
{
    return socket(AF_INET, SOCK_DGRAM, 0);
}

void NET_SocketBind(SocketHandle socketHandle, uint32_t ipAddr, uint16_t port)
{
    struct sockaddr_in bindAddr;
    bindAddr.sin_family = AF_INET;
    bindAddr.sin_addr.s_addr = htonl(ipAddr);
    bindAddr.sin_port = htons(port);

    if (bind(socketHandle, (const struct sockaddr *) &bindAddr, sizeof(bindAddr)) < 0 )
    {
        LogError("Failed to bind socket");
    }
}

int32_t NET_SocketRecv(SocketHandle socketHandle, char* buffer, uint32_t size)
{
    return recv(socketHandle, buffer, size, 0);
}

int32_t NET_SocketRecvFrom(SocketHandle socketHandle, char* buffer, uint32_t size, uint32_t& addr, uint16_t& port)
{
    struct sockaddr_in fromAddr;
    uint32_t fromAddrLen = (uint32_t) sizeof(fromAddr);
    int32_t numBytes = recvfrom(socketHandle, buffer, size, 0, (struct sockaddr*) &fromAddr, &fromAddrLen);
    addr = ntohl(fromAddr.sin_addr.s_addr);
    port = ntohs(fromAddr.sin_port);
    return numBytes;
}

int32_t NET_SocketSendTo(SocketHandle socketHandle, const char* buffer, uint32_t size, uint32_t addr, uint16_t port)
{
    struct sockaddr_in toAddr;
    toAddr.sin_family = AF_INET;
    toAddr.sin_addr.s_addr = htonl(addr);
    toAddr.sin_port = htons(port);
    int32_t bytesSent = sendto(socketHandle, buffer, size, 0, (const struct sockaddr*) &toAddr, (uint32_t) sizeof(toAddr));
    return bytesSent;
}

void NET_SocketClose(SocketHandle socketHandle)
{
    close(socketHandle);
}

void NET_SocketSetBlocking(SocketHandle socketHandle, bool blocking)
{
    int32_t flag = !blocking;
    ioctl(socketHandle, FIONBIO, &flag);
}

void NET_SocketSetBroadcast(SocketHandle socketHandle, bool broadcast)
{
    int broadcastEnable = (int) broadcast;
    int32_t result = setsockopt(socketHandle, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));

    if (result != 0)
    {
        LogError("Failed to set Broadcast flag on socket.");
    }
}

void NET_SocketGetIpAndPort(SocketHandle socketHandle, uint32_t& ipAddr, uint16_t& port)
{
    struct sockaddr_in localAddr = {};
    socklen_t len = sizeof(localAddr);
    getsockname(socketHandle, (struct sockaddr *) &localAddr, &len);
    ipAddr = ntohl(localAddr.sin_addr.s_addr);
    port = ntohs(localAddr.sin_port);
}

uint32_t NET_IpStringToUint32(const char* ipString)
{
    uint32_t retAddr = 0;
    struct in_addr addr = {};
    inet_pton(AF_INET, ipString, &addr);
    retAddr = ntohl(addr.s_addr);
    return retAddr;
}

void NET_IpUint32ToString(uint32_t ipUint32, char* outIpString)
{
    struct sockaddr_in sa = {};
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(ipUint32);
    sa.sin_port = 0;

    inet_ntop(AF_INET, &sa.sin_addr, outIpString, INET_ADDRSTRLEN);
}

uint32_t NET_GetIpAddress()
{
    uint32_t retIp = 0;
    struct ifaddrs* addrs = nullptr;
    struct ifaddrs* addr = nullptr;
    getifaddrs(&addrs);
    addr = addrs;

    while (addr)
    {
        if (addr->ifa_addr &&
            addr->ifa_addr->sa_family == AF_INET)
        {
            LogDebug("Network Interface: %s", addr->ifa_name);

            struct sockaddr_in* inAddr = (sockaddr_in*) addr->ifa_addr;
            uint32_t ip = ntohl(inAddr->sin_addr.s_addr);
            if ((ip & 0x7f000000) != 0x7f000000)
            {
                retIp = ip;
                break;
            }
        }

        addr = addr->ifa_next;
    }

    freeifaddrs(addrs);

    return retIp;
}

uint32_t NET_GetSubnetMask()
{
    uint32_t retMask = 0;
    struct ifaddrs* addrs = nullptr;
    struct ifaddrs* addr = nullptr;
    getifaddrs(&addrs);
    addr = addrs;

    while (addr)
    {
        if (addr->ifa_addr &&
            addr->ifa_addr->sa_family == AF_INET)
        {
            struct sockaddr_in* inAddr = (struct sockaddr_in*) addr->ifa_addr;
            uint32_t ip = ntohl(inAddr->sin_addr.s_addr);
            if ((ip & 0x7f000000) != 0x7f000000)
            {
                struct sockaddr_in* netMaskAddr = (struct sockaddr_in*) addr->ifa_netmask;
                retMask = ntohl(netMaskAddr->sin_addr.s_addr);
                break;
            }
        }

        addr = addr->ifa_next;
    }

    freeifaddrs(addrs);

    return retMask;
}

#endif
