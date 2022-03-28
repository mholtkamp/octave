#pragma once

#include "Network/NetworkTypes.h"

void NET_Initialize();
void NET_Shutdown();
void NET_Update();

bool NET_IsActive();

SocketHandle NET_SocketCreate();
void NET_SocketBind(SocketHandle socketHandle, uint32_t ipAddr, uint16_t port);
int32_t NET_SocketRecv(SocketHandle socketHandle, char* buffer, uint32_t size);
int32_t NET_SocketRecvFrom(SocketHandle socketHandle, char* buffer, uint32_t size, uint32_t& addr, uint16_t& port);
int32_t NET_SocketSendTo(SocketHandle socketHandle, const char* buffer, uint32_t size, uint32_t addr, uint16_t port);
void NET_SocketClose(SocketHandle socketHandle);
void NET_SocketSetBlocking(SocketHandle socketHandle, bool blocking);
void NET_SocketSetBroadcast(SocketHandle socketHandle, bool broadcast);
void NET_SocketGetIpAndPort(SocketHandle socketHandle, uint32_t& ipAddr, uint16_t& port);

uint32_t NET_IpStringToUint32(const char* ipString);
void NET_IpUint32ToString(uint32_t ipUint32, char* outIpString);

uint32_t NET_GetIpAddress();
uint32_t NET_GetSubnetMask();

// TODO: Update LAN servers on NET_Update()
//void NET_GetLanServers();
