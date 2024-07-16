#pragma once

#include "EngineTypes.h"
#include "NetMsg.h"
#include "NetFunc.h"
#include "ScriptFunc.h"
#include "Nodes/Node.h"

#include "Network/Network.h"
#include "Network/NetworkConstants.h"
#include "Network/NetPlatform.h"
#include "Network/NetSession.h"

#include <unordered_map>

// Conflict in WinUser.h
#ifdef SendMessage
#undef SendMessage
#endif

class Node;
class Script;

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

    void Login();
    void Logout();
    bool IsLoggedIn() const;

    void OpenSession(uint16_t port = OCT_DEFAULT_PORT);
    void CloseSession();
    void EnableSessionBroadcast(bool enable);
    bool IsSessionBroadcastEnabled() const;

    void BeginSessionSearch();
    void EndSessionSearch();
    void UpdateSearch();
    bool IsSearching() const;
    const std::vector<NetSession>& GetSessions() const;

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
    void SendInvokeMsg(NetMsgInvoke& msg, Node* node, NetFunc* func, uint32_t numParams, const Datum** params);
    void SendInvokeMsg(Node* node, NetFunc* func, uint32_t numParams, const Datum** params);
    void SendInvokeScriptMsg(Script* script, ScriptNetFunc* func, uint32_t numParams, const Datum** params);
    void SendSpawnMessage(Node* node, NetClient* client);
    void SendDestroyMessage(Node* node, NetClient* client);

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

    void AddNetNode(Node* node, NetId netId);
    void RemoveNetNode(Node* node);
    const std::unordered_map<NetId, Node*>& GetNetNodeMap() const;
    Node* GetNetNode(NetId netId);

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

    void SetScriptConnectCallback(const ScriptFunc& func) { mConnectCallback.mScriptFunc = func; }
    void SetScriptAcceptCallback(const ScriptFunc& func) { mAcceptCallback.mScriptFunc = func; }
    void SetScriptRejectCallback(const ScriptFunc& func) { mRejectCallback.mScriptFunc = func; }
    void SetScriptDisconnectCallback(const ScriptFunc& func) { mDisconnectCallback.mScriptFunc = func; }
    void SetScriptKickCallback(const ScriptFunc& func) { mKickCallback.mScriptFunc = func; }

private:

    static NetworkManager* sInstance;
    NetworkManager();

    void UpdateReplication(float deltaTime);
    bool ReplicateNode(Node* node, NetId hostId, bool force, bool reliable);
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
    std::vector<NetSession> mSessions;
    std::unordered_map<NetId, Node*> mNetNodeMap;
    NetServer mServer;
    uint32_t mBroadcastIp = 0;
    uint32_t mMaxClients = 9;
    NetId mNextNetId = 1;
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
    NetPlatform* mOnlinePlatform = nullptr;
    bool mSearching = false;
    bool mEnableSessionBroadcast = true;
    bool mIncrementalReplication = true;

    ScriptableFP<NetCallbackConnectFP> mConnectCallback;
    ScriptableFP<NetCallbackAcceptFP> mAcceptCallback;
    ScriptableFP<NetCallbackRejectFP> mRejectCallback;
    ScriptableFP<NetCallbackDisconnectFP> mDisconnectCallback;
    ScriptableFP<NetCallbackKickFP> mKickCallback;
};
