#include "NetworkManager.h"
#include "Engine.h"

#include "LuaBindings/Network_Lua.h"
#include "LuaBindings/LuaUtils.h"

#include "TableDatum.h"

#if LUA_ENABLED

void PushSession(lua_State* L, const NetSession& session)
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

    char lobbyId[64];
    snprintf(lobbyId, 64, "%llu", session.mLobbyId);
    lua_pushstring(L, lobbyId);
    lua_setfield(L, sessionIdx, "lobbyId");

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

    // Id
    char onlineId[64];
    snprintf(onlineId, 64, "%llu", profile.mHost.mOnlineId);
    lua_pushstring(L, onlineId);
    lua_setfield(L, profIdx, "onlineId");

    // Ping
    lua_pushnumber(L, (float)profile.mPing);
    lua_setfield(L, profIdx, "ping");

    // Ready
    lua_pushboolean(L, profile.mReady);
    lua_setfield(L, profIdx, "ready");
}

void WriteNetHostProfile(const NetHostProfile& profile, Datum& table)
{
    char ipString[32] = {};
    NET_IpUint32ToString(profile.mHost.mIpAddress, ipString);

    char onlineId[64];
    snprintf(onlineId, 64, "%llu", profile.mHost.mOnlineId);

    table.SetStringField("ipAddress", ipString);
    table.SetIntegerField("port", (int)profile.mHost.mPort);
    table.SetIntegerField("id", (int)profile.mHost.mId);
    table.SetStringField("onlineId", onlineId);
    table.SetFloatField("ping", profile.mPing);
    table.SetBoolField("ready", profile.mReady);
}


int Network_Lua::OpenSession(lua_State* L)
{
    bool lan = CHECK_BOOLEAN(L, 1);
    uint16_t port = OCT_DEFAULT_PORT;
    if (!lua_isnone(L, 1)) { port = (uint16_t)CHECK_INTEGER(L, 2); }

    NetworkManager::Get()->OpenSession(lan, port);

    return 0;
}

int Network_Lua::CloseSession(lua_State* L)
{
    NetworkManager::Get()->CloseSession();

    return 0;
}

int Network_Lua::JoinSession(lua_State* L)
{
    CHECK_TABLE(L, 1);
    Datum table = LuaObjectToDatum(L, 1);

    NetSession session;
    session.mHost.mIpAddress = NET_IpStringToUint32(table.GetStringField("ipAddress").c_str());
    session.mHost.mPort = (uint16_t)table.GetIntegerField("port");
    session.mHost.mId = table.GetIntegerField("id");
    session.mLobbyId = (uint64_t)std::stoll(table.GetStringField("onlineId"));


    NetworkManager::Get()->JoinSession(session);

    return 0;
}

int Network_Lua::SetSessionName(lua_State* L)
{
    const char* value = CHECK_STRING(L, 1);

    NetworkManager::Get()->SetSessionName(value);

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

int Network_Lua::EnableSessionBroadcast(lua_State* L)
{
    bool value = CHECK_BOOLEAN(L, 1);

    NetworkManager::Get()->EnableSessionBroadcast(value);

    return 0;
}

int Network_Lua::IsSessionBroadcastEnabled(lua_State* L)
{
    bool ret = NetworkManager::Get()->IsSessionBroadcastEnabled();

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
    int32_t index = (int32_t)CHECK_INDEX(L, 1);

    const std::vector<NetSession>& sessions = NetworkManager::Get()->GetSessions();
    if (index >= 0 &&
        index < int32_t(sessions.size()))
    {
        const NetSession& session = sessions[index];
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
    const std::vector<NetSession>& sessions = NetworkManager::Get()->GetSessions();

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
    CHECK_FUNCTION(L, 1);
    ScriptFunc func(L, 1);

    NetworkManager::Get()->SetScriptConnectCallback(func);

    return 0;
}

int Network_Lua::SetAcceptCallback(lua_State* L)
{
    CHECK_FUNCTION(L, 1);
    ScriptFunc func(L, 1);

    NetworkManager::Get()->SetScriptAcceptCallback(func);

    return 0;
}

int Network_Lua::SetRejectCallback(lua_State* L)
{
    CHECK_FUNCTION(L, 1);
    ScriptFunc func(L, 1);

    NetworkManager::Get()->SetScriptRejectCallback(func);

    return 0;
}

int Network_Lua::SetDisconnectCallback(lua_State* L)
{
    CHECK_FUNCTION(L, 1);
    ScriptFunc func(L, 1);

    NetworkManager::Get()->SetScriptDisconnectCallback(func);

    return 0;
}

int Network_Lua::SetKickCallback(lua_State* L)
{
    CHECK_FUNCTION(L, 1);
    ScriptFunc func(L, 1);

    NetworkManager::Get()->SetScriptKickCallback(func);

    return 0;
}

void Network_Lua::Bind()
{
    lua_State* L = GetLua();

    lua_newtable(L);
    int tableIdx = lua_gettop(L);

    REGISTER_TABLE_FUNC(L, tableIdx, OpenSession);

    REGISTER_TABLE_FUNC(L, tableIdx, CloseSession);

    REGISTER_TABLE_FUNC(L, tableIdx, BeginSessionSearch);

    REGISTER_TABLE_FUNC(L, tableIdx, EndSessionSearch);

    REGISTER_TABLE_FUNC(L, tableIdx, IsSearching);

    REGISTER_TABLE_FUNC(L, tableIdx, GetNumSessions);

    REGISTER_TABLE_FUNC(L, tableIdx, GetSession);

    REGISTER_TABLE_FUNC(L, tableIdx, GetSessions);

    REGISTER_TABLE_FUNC(L, tableIdx, Connect);

    REGISTER_TABLE_FUNC(L, tableIdx, Disconnect);

    REGISTER_TABLE_FUNC(L, tableIdx, Kick);

    REGISTER_TABLE_FUNC(L, tableIdx, SetMaxClients);

    REGISTER_TABLE_FUNC(L, tableIdx, GetMaxClients);

    REGISTER_TABLE_FUNC(L, tableIdx, GetNumClients);

    REGISTER_TABLE_FUNC(L, tableIdx, GetClients);

    REGISTER_TABLE_FUNC(L, tableIdx, FindNetClient);

    REGISTER_TABLE_FUNC(L, tableIdx, GetNetStatus);

    REGISTER_TABLE_FUNC(L, tableIdx, EnableIncrementalReplication);

    REGISTER_TABLE_FUNC(L, tableIdx, IsIncrementalReplicationEnabled);

    REGISTER_TABLE_FUNC(L, tableIdx, GetBytesSent);

    REGISTER_TABLE_FUNC(L, tableIdx, GetBytesReceived);

    REGISTER_TABLE_FUNC(L, tableIdx, GetUploadRate);

    REGISTER_TABLE_FUNC(L, tableIdx, GetDownloadRate);

    REGISTER_TABLE_FUNC(L, tableIdx, IsServer);

    REGISTER_TABLE_FUNC(L, tableIdx, IsClient);

    REGISTER_TABLE_FUNC(L, tableIdx, IsLocal);

    REGISTER_TABLE_FUNC(L, tableIdx, IsAuthority);

    REGISTER_TABLE_FUNC(L, tableIdx, GetHostId);

    REGISTER_TABLE_FUNC(L, tableIdx, SetConnectCallback);

    REGISTER_TABLE_FUNC(L, tableIdx, SetAcceptCallback);

    REGISTER_TABLE_FUNC(L, tableIdx, SetRejectCallback);

    REGISTER_TABLE_FUNC(L, tableIdx, SetDisconnectCallback);

    REGISTER_TABLE_FUNC(L, tableIdx, SetKickCallback);

    lua_setglobal(L, NETWORK_LUA_NAME);

    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
