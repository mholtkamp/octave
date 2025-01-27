#pragma once

#include "NetworkManager.h"
#include "EngineTypes.h"

#if LUA_ENABLED

#define NETWORK_LUA_NAME "Network"

void PushSession(lua_State* L, const NetSession& session);
void PushNetHostProfile(lua_State* L, const NetHostProfile& profile);
void WriteNetHostProfile(const NetHostProfile& profile, Datum& table);

struct Network_Lua
{
    static int OpenSession(lua_State* L);
    static int CloseSession(lua_State* L);
    static int JoinSession(lua_State* L);
    static int SetSessionName(lua_State* L);
    static int BeginSessionSearch(lua_State* L);
    static int EndSessionSearch(lua_State* L);
    static int IsSearching(lua_State* L);
    static int EnableSessionBroadcast(lua_State* L);
    static int IsSessionBroadcastEnabled(lua_State* L);
    static int GetNumSessions(lua_State* L);
    static int GetSession(lua_State* L);
    static int GetSessions(lua_State* L);
    static int Connect(lua_State* L);
    static int Disconnect(lua_State* L);
    static int Kick(lua_State* L);
    static int GetMaxClients(lua_State* L);
    static int GetNumClients(lua_State* L);
    static int GetClients(lua_State* L);
    static int FindNetClient(lua_State* L);
    static int GetNetStatus(lua_State* L);
    static int EnableIncrementalReplication(lua_State* L);
    static int IsIncrementalReplicationEnabled(lua_State* L);
    static int GetBytesSent(lua_State* L);
    static int GetBytesReceived(lua_State* L);
    static int GetUploadRate(lua_State* L);
    static int GetDownloadRate(lua_State* L);
    static int IsServer(lua_State* L);
    static int IsClient(lua_State* L);
    static int IsLocal(lua_State* L);
    static int IsAuthority(lua_State* L);
    static int GetHostId(lua_State* L);
    static int SetRelevancyDistance(lua_State* L);
    static int SetPawn(lua_State* L);
    static int GetPawn(lua_State* L);
    static int EnableNetRelevancy(lua_State* L);

    // Callbacks
    static int SetConnectCallback(lua_State* L);
    static int SetAcceptCallback(lua_State* L);
    static int SetRejectCallback(lua_State* L);
    static int SetDisconnectCallback(lua_State* L);
    static int SetKickCallback(lua_State* L);

    static void Bind();
};

#endif
