#pragma once

#include "EngineTypes.h"
#include "NetMsg.h"
#include "NetFunc.h"
#include "ScriptFunc.h"

#include "Network/Network.h"
#include "Network/NetworkConstants.h"

// Conflict in WinUser.h
#ifdef SendMessage
#undef SendMessage
#endif

#define OCT_DEFAULT_PORT 5151
#define OCT_BROADCAST_PORT 15151
#define OCT_RECV_BUFFER_SIZE 1024
#define OCT_SEND_BUFFER_SIZE 1024
#define OCT_MAX_MSG_BODY_SIZE 500
#define OCT_SEQ_NUM_SIZE sizeof(uint16_t)
#define OCT_PACKET_HEADER_SIZE (OCT_SEQ_NUM_SIZE + sizeof(bool))
#define OCT_MAX_MSG_SIZE (OCT_PACKET_HEADER_SIZE + OCT_MAX_MSG_BODY_SIZE)
#define OCT_PING_INTERVAL 1.0f
#define OCT_BROADCAST_INTERVAL 5.0f

class Actor;

bool NetIsClient();
bool NetIsServer();
bool NetIsLocal();
bool NetIsAuthority();
NetHostId NetGetHostId();

// Network even callbacks
typedef void(*NetCallbackConnectFP)(NetClient*);
typedef void(*NetCallbackAcceptFP)();
typedef void(*NetCallbackRejectFP)(NetMsgReject::Reason);
typedef void(*NetCallbackDisconnectFP)(NetClient*);
typedef void(*NetCallbackKickFP)(NetMsgKick::Reason);

struct GameSession
{
    NetHost mHost = {};
    char mName[OCT_SESSION_NAME_LEN + 1] = {};
    uint8_t mMaxPlayers = 0;
    uint8_t mNumPlayers = 0;
};

class NetworkManager
{
public:

    static void Create();
    static void Destroy();
    static NetworkManager* Get();
    
    void Initialize();
    void Shutdown();
    void PreTickUpdate(float deltaTime);
    void PostTickUpdate(float deltaTime);

    void OpenSession(uint16_t port = OCT_DEFAULT_PORT);
    void CloseSession();

    void BeginSessionSearch();
    void EndSessionSearch();
    void UpdateSearch();
    bool IsSearching() const;
    const std::vector<GameSession>& GetSessions() const;

    void Connect(const char* ipAddress, uint16_t port = OCT_DEFAULT_PORT);
    void Connect(uint32_t ipAddress, uint16_t port = OCT_DEFAULT_PORT);
    void Disconnect();
    void Kick(NetHostId hostId, NetMsgKick::Reason reason);

    void SetMaxClients(uint32_t maxClients);
    uint32_t GetMaxClients();
    const std::vector<NetClient>& GetClients() const;

    void SendMessage(const NetMsg* netMsg, NetHostId receiverId);
    void SendMessage(const NetMsg* netMsg, NetHostProfile* hostProfile);
    void SendMessageToAllClients(const NetMsg* netMsg);
    void SendMessageImmediate(const NetMsg* netMsg, uint32_t ipAddress, uint16_t port);

    void SendReplicateMsg(NetMsgReplicate& repMsg, uint32_t& numVars, NetHostId hostId);
    void SendInvokeMsg(NetMsgInvoke& msg, Actor* actor, NetFunc* func, uint32_t numParams, Datum** params);
    void SendInvokeMsg(Actor* actor, NetFunc* func, uint32_t numParams, Datum** params);
    void SendInvokeScriptMsg(ScriptComponent* scriptComp, ScriptNetFunc* func, uint32_t numParams, Datum** params);
    void SendSpawnMessage(Actor* actor, NetClient* client);
    void SendDestroyMessage(Actor* actor, NetClient* client);

    void ResendPacket(NetHostProfile* hostProfile, ReliablePacket& packet);
    void ResendOutgoingReliablePackets(NetHostProfile* hostProfile);

    void FlushSendBuffers();

    NetClient* FindNetClient(NetHostId id);

    NetStatus GetNetStatus() const;

    void EnableIncrementalReplication(bool enable);
    bool IsIncrementalReplicationEnabled() const;

    int32_t GetBytesSent() const;
    int32_t GetBytesReceived() const;
    float GetUploadRate() const;
    float GetDownloadRate() const;

    bool IsServer() const;
    bool IsClient() const;
    bool IsLocal() const;
    bool IsAuthority() const;
    NetHostId GetHostId() const;

    // Message Handling
    void HandleConnect(NetHost host, uint32_t gameCode, uint32_t version);
    void HandleAccept(NetHostId assignedId);
    void HandleReject(NetMsgReject::Reason reason);
    void HandleDisconnect(NetHost host);
    void HandleKick(NetMsgKick::Reason reason);
    void HandleAck(NetHost host, uint16_t sequenceNumber);
    void HandleReady(NetHost host);
    void HandleBroadcast(
        NetHost host,
        uint32_t gameCode,
        uint32_t version,
        const char* name,
        uint8_t maxPlayers,
        uint8_t numPlayers);

    // Callback Setters
    void SetConnectCallback(NetCallbackConnectFP cb) { mConnectCallback.mFuncPointer = cb; }
    void SetAcceptCallback(NetCallbackAcceptFP cb) { mAcceptCallback.mFuncPointer = cb; }
    void SetRejectCallback(NetCallbackRejectFP cb) { mRejectCallback.mFuncPointer = cb; }
    void SetDisconnectCallback(NetCallbackDisconnectFP cb) { mDisconnectCallback.mFuncPointer = cb; }
    void SetKickCallback(NetCallbackKickFP cb) { mKickCallback.mFuncPointer = cb; }

    void SetScriptConnectCallback(const char* tableName, const char* funcName) { mConnectCallback.mScriptTableName = tableName; mConnectCallback.mScriptFuncName = funcName; }
    void SetScriptAcceptCallback(const char* tableName, const char* funcName) { mAcceptCallback.mScriptTableName = tableName; mAcceptCallback.mScriptFuncName = funcName; }
    void SetScriptRejectCallback(const char* tableName, const char* funcName) { mRejectCallback.mScriptTableName = tableName; mRejectCallback.mScriptFuncName = funcName; }
    void SetScriptDisconnectCallback(const char* tableName, const char* funcName) { mDisconnectCallback.mScriptTableName = tableName; mDisconnectCallback.mScriptFuncName = funcName; }
    void SetScriptKickCallback(const char* tableName, const char* funcName) { mKickCallback.mScriptTableName = tableName; mKickCallback.mScriptFuncName = funcName; }

private:

    static NetworkManager* sInstance;
    NetworkManager();

    void UpdateReplication(float deltaTime);
    bool ReplicateActor(Actor* actor, NetId hostId, bool force, bool reliable);
    void UpdateHostConnections(float deltaTime);
    void ProcessIncomingPackets(float deltaTime);
    void ProcessMessages(NetHost sender, Stream& stream);
    void ProcessPendingReliablePackets(NetHostProfile* profile);
    NetHostId FindAvailableNetHostId();
    void ResetToLocalStatus();
    void BroadcastSession();
    void FlushSendBuffers(NetHostProfile* hostProfile);
    void FlushSendBuffer(NetHostProfile* hostProfile, bool reliable);
    void UpdateReliablePackets(float deltaTime);
    bool UpdateReliablePackets(NetHostProfile* profile, float deltaTime);
    void ResetHostProfile(NetHostProfile* profile);
    bool HostProfileHasIncomingPacket(NetHostProfile* profile, uint16_t seq);
    bool SeqNumLess(uint16_t s1, uint16_t s2);


    NetStatus mNetStatus = NetStatus::Local;
    std::vector<NetClient> mClients;
    std::vector<GameSession> mSessions;
    NetServer mServer;
    uint32_t mBroadcastIp = 0;
    uint32_t mMaxClients = 9;
    float mConnectTimer = 0.0f;
    float mBroadcastTimer = 0.0f;
    float mPingTimer = 0.0f;
    float mConnectTimeout = 5.0f;
    float mInactiveTimeout = 15.0f;
    float mUploadRate = 0;
    float mDownloadRate = 0;
    int32_t mBytesSent = 0;
    int32_t mBytesReceived = 0;
    NetHostId mHostId = INVALID_HOST_ID;
    SocketHandle mSocket = NET_INVALID_SOCKET;
    SocketHandle mSearchSocket = NET_INVALID_SOCKET;
    bool mSearching = false;
    bool mIncrementalReplication = true;

    ScriptableFP<NetCallbackConnectFP> mConnectCallback;
    ScriptableFP<NetCallbackAcceptFP> mAcceptCallback;
    ScriptableFP<NetCallbackRejectFP> mRejectCallback;
    ScriptableFP<NetCallbackDisconnectFP> mDisconnectCallback;
    ScriptableFP<NetCallbackKickFP> mKickCallback;
};
