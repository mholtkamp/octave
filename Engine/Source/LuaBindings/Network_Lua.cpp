#include "NetworkManager.h"
#include "Engine.h"

#include "Components/ScriptComponent.h"

#include "LuaBindings/Network_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

void PushSession(lua_State* L, const GameSession& session)
{
    lua_newtable(L);
    int sessionIdx = lua_gettop(L);

    // IP Address
    char ipString[32] = {};
    NET_IpUint32ToString(session.mHost.mIpAddress, ipString);
    lua_pushstring(L, ipString);
    lua_setfield(L, sessionIdx, "ipAddress");

    // Port
    lua_pushinteger(L, (int)session.mHost.mPort);
    lua_setfield(L, sessionIdx, "port");

    // Name
    lua_pushstring(L, session.mName);
    lua_setfield(L, sessionIdx, "name");

    // MaxPlayers
    lua_pushinteger(L, (int)session.mMaxPlayers);
    lua_setfield(L, sessionIdx, "maxPlayers");

    // NumPlayers
    lua_pushinteger(L, (int)session.mNumPlayers);
    lua_setfield(L, sessionIdx, "numPlayers");
}

void PushNetHostProfile(lua_State* L, const NetHostProfile& profile)
{
    lua_newtable(L);
    int profIdx = lua_gettop(L);

    // IP Address
    char ipString[32] = {};
    NET_IpUint32ToString(profile.mHost.mIpAddress, ipString);
    lua_pushstring(L, ipString);
    lua_setfield(L, profIdx, "ipAddress");

    // Port
    lua_pushinteger(L, (int)profile.mHost.mPort);
    lua_setfield(L, profIdx, "port");

    // Id
    lua_pushinteger(L, (int)profile.mHost.mId);
    lua_setfield(L, profIdx, "id");

    // Ping
    lua_pushnumber(L, (float)profile.mPing);
    lua_setfield(L, profIdx, "ping");

    // Ready
    lua_pushboolean(L, profile.mReady);
    lua_setfield(L, profIdx, "ready");
}


int Network_Lua::OpenSession(lua_State* L)
{
    uint16_t port = OCT_DEFAULT_PORT;
    if (!lua_isnone(L, 1)) { port = (uint16_t)CHECK_INTEGER(L, 1); }

    NetworkManager::Get()->OpenSession(port);

    return 0;
}

int Network_Lua::CloseSession(lua_State* L)
{
    NetworkManager::Get()->CloseSession();

    return 0;
}

int Network_Lua::BeginSessionSearch(lua_State* L)
{
    NetworkManager::Get()->BeginSessionSearch();

    return 0;
}

int Network_Lua::EndSessionSearch(lua_State* L)
{
    NetworkManager::Get()->EndSessionSearch();

    return 0;
}

int Network_Lua::IsSearching(lua_State* L)
{
    bool ret = NetworkManager::Get()->IsSearching();

    lua_pushboolean(L, ret);
    return 1;
}

int Network_Lua::GetNumSessions(lua_State* L)
{
    uint32_t ret = (uint32_t) NetworkManager::Get()->GetSessions().size();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int Network_Lua::GetSession(lua_State* L)
{
    int32_t index = (int32_t)CHECK_INTEGER(L, 1);
    index -= 1; // Lua indexes starting from 1

    const std::vector<GameSession>& sessions = NetworkManager::Get()->GetSessions();
    if (index >= 0 &&
        index < int32_t(sessions.size()))
    {
        const GameSession& session = sessions[index];
        PushSession(L, session);
        return 1;
    }
    else
    {
        lua_pushnil(L);
        return 1;
    }
}

int Network_Lua::GetSessions(lua_State* L)
{
    const std::vector<GameSession>& sessions = NetworkManager::Get()->GetSessions();

    lua_newtable(L);
    int tableIdx = lua_gettop(L);

    for (uint32_t i = 0; i < sessions.size(); ++i)
    {
        PushSession(L, sessions[i]);
        lua_seti(L, tableIdx, (int)i + 1);
    }

    // Array of sessions should be on the top of the stack.
    return 1;
}

int Network_Lua::Connect(lua_State* L)
{
    const char* ipAddr = CHECK_STRING(L, 1);
    uint16_t port = OCT_DEFAULT_PORT;
    if (!lua_isnone(L, 2)) { port = (uint16_t)CHECK_INTEGER(L, 2); }

    NetworkManager::Get()->Connect(ipAddr, port);

    return 0;
}

int Network_Lua::Disconnect(lua_State* L)
{
    NetworkManager::Get()->Disconnect();

    return 0;
}

int Network_Lua::Kick(lua_State* L)
{
    NetHostId hostId = CHECK_INTEGER(L, 1);
    NetMsgKick::Reason reason = NetMsgKick::Reason::Forced;

    NetworkManager::Get()->Kick(hostId, reason);

    return 0;
}

int Network_Lua::SetMaxClients(lua_State* L)
{
    uint32_t value = (uint32_t) CHECK_INTEGER(L, 1);

    NetworkManager::Get()->SetMaxClients(value);

    return 0;
}

int Network_Lua::GetMaxClients(lua_State* L)
{
    uint32_t ret = NetworkManager::Get()->GetMaxClients();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int Network_Lua::GetNumClients(lua_State* L)
{
    uint32_t ret = (uint32_t) NetworkManager::Get()->GetClients().size();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int Network_Lua::GetClients(lua_State* L)
{
    lua_newtable(L);
    int arrayIdx = lua_gettop(L);

    const std::vector<NetClient>& clients = NetworkManager::Get()->GetClients();

    for (uint32_t i = 0; i < clients.size(); ++i)
    {
        PushNetHostProfile(L, clients[i]);
        lua_seti(L, arrayIdx, (int)i + 1);
    }

    // The array table should be on top.
    return 1;
}

int Network_Lua::FindNetClient(lua_State* L)
{
    int32_t id = (int32_t)CHECK_INTEGER(L, 1);

    NetClient* prof = NetworkManager::Get()->FindNetClient(id);

    if (prof != nullptr)
    {
        PushNetHostProfile(L, *prof);
        return 1;
    }
    else
    {
        lua_pushnil(L);
        return 1;
    }
}

int Network_Lua::GetNetStatus(lua_State* L)
{
    NetStatus status = NetworkManager::Get()->GetNetStatus();

    const char* statusString = "Local";

    switch (status)
    {
        case NetStatus::Client: statusString = "Client"; break;
        case NetStatus::Server: statusString = "Server"; break;
        case NetStatus::Connecting: statusString = "Connecting"; break;
        case NetStatus::Local: statusString = "Local"; break;

        default:
            break;
    }

    lua_pushstring(L, statusString);
    return 1;
}

int Network_Lua::EnableIncrementalReplication(lua_State* L)
{
    bool value = CHECK_BOOLEAN(L, 1);

    NetworkManager::Get()->EnableIncrementalReplication(value);
    
    return 0;
}

int Network_Lua::IsIncrementalReplicationEnabled(lua_State* L)
{
    bool ret = NetworkManager::Get()->IsIncrementalReplicationEnabled();

    lua_pushboolean(L, ret);
    return 1;
}

int Network_Lua::GetBytesSent(lua_State* L)
{
    int32_t ret = NetworkManager::Get()->GetBytesSent();

    lua_pushinteger(L, ret);
    return 1;
}

int Network_Lua::GetBytesReceived(lua_State* L)
{
    int32_t ret = NetworkManager::Get()->GetBytesReceived();

    lua_pushinteger(L, ret);
    return 1;
}

int Network_Lua::GetUploadRate(lua_State* L)
{
    float ret = NetworkManager::Get()->GetUploadRate();

    lua_pushnumber(L, ret);
    return 1;
}

int Network_Lua::GetDownloadRate(lua_State* L)
{
    float ret = NetworkManager::Get()->GetDownloadRate();

    lua_pushnumber(L, ret);
    return 1;
}

int Network_Lua::IsServer(lua_State* L)
{
    bool ret = NetworkManager::Get()->IsServer();

    lua_pushboolean(L, ret);
    return 1;
}

int Network_Lua::IsClient(lua_State* L)
{
    bool ret = NetworkManager::Get()->IsClient();

    lua_pushboolean(L, ret);
    return 1;
}

int Network_Lua::IsLocal(lua_State* L)
{
    bool ret = NetworkManager::Get()->IsLocal();

    lua_pushboolean(L, ret);
    return 1;
}

int Network_Lua::IsAuthority(lua_State* L)
{
    bool ret = NetworkManager::Get()->IsAuthority();

    lua_pushboolean(L, ret);
    return 1;
}

int Network_Lua::GetHostId(lua_State* L)
{
    NetHostId id = NetworkManager::Get()->GetHostId();

    lua_pushinteger(L, (int) id);
    return 1;
}

// Callbacks
int Network_Lua::SetConnectCallback(lua_State* L)
{
    const char* funcName = CHECK_STRING(L, 1);

    if (strcmp(funcName, "") == 0)
    {
        NetworkManager::Get()->SetScriptConnectCallback("", "");
    }
    else
    {
        NetworkManager::Get()->SetScriptConnectCallback(
            ScriptComponent::GetExecutingScriptTableName(),
            funcName);
    }

    return 0;
}

int Network_Lua::SetAcceptCallback(lua_State* L)
{
    const char* funcName = CHECK_STRING(L, 1);

    if (strcmp(funcName, "") == 0)
    {
        NetworkManager::Get()->SetScriptAcceptCallback("", "");
    }
    else
    {
        NetworkManager::Get()->SetScriptAcceptCallback(
            ScriptComponent::GetExecutingScriptTableName(),
            funcName);
    }

    return 0;
}

int Network_Lua::SetRejectCallback(lua_State* L)
{
    const char* funcName = CHECK_STRING(L, 1);

    if (strcmp(funcName, "") == 0)
    {
        NetworkManager::Get()->SetScriptRejectCallback("", "");
    }
    else
    {
        NetworkManager::Get()->SetScriptRejectCallback(
            ScriptComponent::GetExecutingScriptTableName(),
            funcName);
    }

    return 0;
}

int Network_Lua::SetDisconnectCallback(lua_State* L)
{
    const char* funcName = CHECK_STRING(L, 1);

    if (strcmp(funcName, "") == 0)
    {
        NetworkManager::Get()->SetScriptDisconnectCallback("", "");
    }
    else
    {
        NetworkManager::Get()->SetScriptDisconnectCallback(
            ScriptComponent::GetExecutingScriptTableName(),
            funcName);
    }

    return 0;
}

int Network_Lua::SetKickCallback(lua_State* L)
{
    const char* funcName = CHECK_STRING(L, 1);

    if (strcmp(funcName, "") == 0)
    {
        NetworkManager::Get()->SetScriptKickCallback("", "");
    }
    else
    {
        NetworkManager::Get()->SetScriptKickCallback(
            ScriptComponent::GetExecutingScriptTableName(),
            funcName);
    }

    return 0;
}

void Network_Lua::Bind()
{
    lua_State* L = GetLua();

    lua_newtable(L);
    int tableIdx = lua_gettop(L);

    lua_pushcfunction(L, OpenSession);
    lua_setfield(L, tableIdx, "OpenSession");

    lua_pushcfunction(L, CloseSession);
    lua_setfield(L, tableIdx, "CloseSession");

    lua_pushcfunction(L, BeginSessionSearch);
    lua_setfield(L, tableIdx, "BeginSessionSearch");

    lua_pushcfunction(L, EndSessionSearch);
    lua_setfield(L, tableIdx, "EndSessionSearch");

    lua_pushcfunction(L, IsSearching);
    lua_setfield(L, tableIdx, "IsSearching");

    lua_pushcfunction(L, GetNumSessions);
    lua_setfield(L, tableIdx, "GetNumSessions");

    lua_pushcfunction(L, GetSession);
    lua_setfield(L, tableIdx, "GetSession");

    lua_pushcfunction(L, GetSessions);
    lua_setfield(L, tableIdx, "GetSessions");

    lua_pushcfunction(L, Connect);
    lua_setfield(L, tableIdx, "Connect");

    lua_pushcfunction(L, Disconnect);
    lua_setfield(L, tableIdx, "Disconnect");

    lua_pushcfunction(L, Kick);
    lua_setfield(L, tableIdx, "Kick");

    lua_pushcfunction(L, SetMaxClients);
    lua_setfield(L, tableIdx, "SetMaxClients");

    lua_pushcfunction(L, GetMaxClients);
    lua_setfield(L, tableIdx, "GetMaxClients");

    lua_pushcfunction(L, GetNumClients);
    lua_setfield(L, tableIdx, "GetNumClients");

    lua_pushcfunction(L, GetClients);
    lua_setfield(L, tableIdx, "GetClients");

    lua_pushcfunction(L, FindNetClient);
    lua_setfield(L, tableIdx, "FindNetClient");

    lua_pushcfunction(L, GetNetStatus);
    lua_setfield(L, tableIdx, "GetNetStatus");

    lua_pushcfunction(L, EnableIncrementalReplication);
    lua_setfield(L, tableIdx, "EnableIncrementalReplication");

    lua_pushcfunction(L, IsIncrementalReplicationEnabled);
    lua_setfield(L, tableIdx, "IsIncrementalReplicationEnabled");

    lua_pushcfunction(L, GetBytesSent);
    lua_setfield(L, tableIdx, "GetBytesSent");

    lua_pushcfunction(L, GetBytesReceived);
    lua_setfield(L, tableIdx, "GetBytesReceived");

    lua_pushcfunction(L, GetUploadRate);
    lua_setfield(L, tableIdx, "GetUploadRate");

    lua_pushcfunction(L, GetDownloadRate);
    lua_setfield(L, tableIdx, "GetDownloadRate");

    lua_pushcfunction(L, IsServer);
    lua_setfield(L, tableIdx, "IsServer");

    lua_pushcfunction(L, IsClient);
    lua_setfield(L, tableIdx, "IsClient");

    lua_pushcfunction(L, IsLocal);
    lua_setfield(L, tableIdx, "IsLocal");

    lua_pushcfunction(L, IsAuthority);
    lua_setfield(L, tableIdx, "IsAuthority");

    lua_pushcfunction(L, GetHostId);
    lua_setfield(L, tableIdx, "GetHostId");

    lua_pushcfunction(L, SetConnectCallback);
    lua_setfield(L, tableIdx, "SetConnectCallback");

    lua_pushcfunction(L, SetAcceptCallback);
    lua_setfield(L, tableIdx, "SetAcceptCallback");

    lua_pushcfunction(L, SetRejectCallback);
    lua_setfield(L, tableIdx, "SetRejectCallback");

    lua_pushcfunction(L, SetDisconnectCallback);
    lua_setfield(L, tableIdx, "SetDisconnectCallback");

    lua_pushcfunction(L, SetKickCallback);
    lua_setfield(L, tableIdx, "SetKickCallback");

    lua_setglobal(L, NETWORK_LUA_NAME);

    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
