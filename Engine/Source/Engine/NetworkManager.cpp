#include "NetworkManager.h"
#include "Engine.h"
#include "Log.h"
#include "Assets/Level.h"
#include "Assets/Blueprint.h"
#include "World.h"
#include "Profiler.h"
#include "Maths.h"
#include "ScriptEvent.h"

#include "Components/ScriptComponent.h"

#ifdef SendMessage
#undef SendMessage
#endif

#define DEBUG_MSG_STATS 0
#define DEBUG_NETWORK_CONDITIONS 0

// Do we even need sRecvBuffer? We could probably just use stack space for reading/writing packet data.
static char sRecvBuffer[OCT_RECV_BUFFER_SIZE] = {};
static char sSendBuffer[OCT_SEND_BUFFER_SIZE] = {};

#if DEBUG_MSG_STATS
static uint32_t sNumPacketsSent = 0;
static uint32_t sNumPacketsReceived = 0;
#endif

#if DEBUG_NETWORK_CONDITIONS
static float sPacketLossFrac = 0.5f;
static float sLatencyMs = 80.0f;
static float sJitterMs = 10.0f;

struct DelayedPacket
{
    SocketHandle mSocket = {};
    float mTime = 0.0f;
    uint32_t mIpAddress = 0;
    uint16_t mPort = 0;
    uint32_t mSize = 0;
    char mData[OCT_MAX_MSG_SIZE] = {};
};

static std::vector<DelayedPacket> sDelayedPackets;

int32_t DebugSendTo(SocketHandle socket, uint32_t ip, uint16_t port, uint32_t size, const char* data)
{
    float dropRoll = Maths::RandRange(0.0f, 1.0f);

    if (dropRoll >= sPacketLossFrac)
    {

        sDelayedPackets.emplace_back();
        DelayedPacket& packet = sDelayedPackets.back();

        packet.mSocket = socket;
        packet.mTime = (1/1000.0f) * (sLatencyMs + Maths::RandRange(-sJitterMs, sJitterMs));
        packet.mIpAddress = ip;
        packet.mPort = port;
        packet.mSize = size;
        memcpy(packet.mData, data, size);
    }
    else
    {
        //LogWarning("Dropping packet");
    }

    return size;
}

void UpdateDebugPackets(float deltaTime)
{
    for (int32_t i = 0; i < int32_t(sDelayedPackets.size()); ++i)
    {
        sDelayedPackets[i].mTime -= deltaTime;

        if (sDelayedPackets[i].mTime <= 0.0f)
        {
            NET_SocketSendTo(
                sDelayedPackets[i].mSocket,
                sDelayedPackets[i].mData,
                sDelayedPackets[i].mSize,
                sDelayedPackets[i].mIpAddress,
                sDelayedPackets[i].mPort);

            sDelayedPackets.erase(sDelayedPackets.begin() + i);
            --i;
        }
    }
}

#endif

// Avoid dynamic allocations when appropriate. Reuse static messages.
static NetMsgReplicate sMsgReplicate;
static NetMsgReplicateScript sMsgReplicateScript;
static NetMsgInvoke sMsgInvoke;
static NetMsgInvokeScript sMsgInvokeScript;

// Reliable messaging
static float sReliableResendTime = 0.1f;
static uint32_t sMaxReliableResends = 20;
static uint32_t sMaxOutgoingPackets = 100;
static uint32_t sMaxIncomingPackets = 100;

#define NET_MSG_CASE(Type) \
    case NetMsgType::Type: \
    { \
        /*LogDebug("Recv NetMsg %s", #Type);*/ \
        NetMsg##Type netMsg; \
        netMsg.Read(stream); \
        netMsg.Execute(sender); \
        msgHandled = true; \
        break; \
    }

#define NET_MSG_STATIC_CASE(Type) \
    case NetMsgType::Type: \
    { \
        /*LogDebug("Recv NetMsg %s", #Type);*/ \
        sMsg##Type.Read(stream); \
        sMsg##Type.Execute(sender); \
        msgHandled = true; \
        break; \
    }

NetworkManager* NetworkManager::sInstance = nullptr;

bool NetIsClient()
{
    return NetworkManager::Get()->IsClient();
}

bool NetIsServer()
{
    return NetworkManager::Get()->IsServer();
}

bool NetIsLocal()
{
    return NetworkManager::Get()->IsLocal();
}

bool NetIsAuthority()
{
    return NetworkManager::Get()->IsAuthority();
}

NetHostId NetGetHostId()
{
    return NetworkManager::Get()->GetHostId();
}

void NetworkManager::Create()
{
    OCT_ASSERT(sInstance == nullptr);
    sInstance = new NetworkManager();
}

void NetworkManager::Destroy()
{
    if (sInstance != nullptr)
    {
        delete sInstance;
        sInstance = nullptr;
    }
}

NetworkManager* NetworkManager::Get()
{
    return sInstance;
}

NetworkManager::NetworkManager()
{
    
}

void NetworkManager::Initialize()
{
    
}

void NetworkManager::Shutdown()
{
    if (mNetStatus == NetStatus::Server)
    {
        CloseSession();
    }
    else if (mNetStatus == NetStatus::Client ||
             mNetStatus == NetStatus::Connecting)
    {
        Disconnect();
    }
}

void NetworkManager::PreTickUpdate(float deltaTime)
{
    SCOPED_FRAME_STAT("Network");

#if DEBUG_MSG_STATS
    LogDebug("Sent[%d] %d, Recv[%d] %d", sNumPacketsSent, mBytesSent, sNumPacketsReceived, mBytesReceived);

    sNumPacketsSent = 0;
    sNumPacketsReceived = 0;
#endif

    // Update averages and reset counters for this frame.
    float frameUploadRate = (deltaTime > 0.0f) ? (mBytesSent / deltaTime) : 0.0f;
    float frameDownloadRate = (deltaTime > 0.0f) ? (mBytesReceived / deltaTime) : 0.0f;
    mUploadRate = Maths::Damp(mUploadRate, (float)frameUploadRate, 0.05f, deltaTime);
    mDownloadRate = Maths::Damp(mDownloadRate, (float)frameDownloadRate, 0.05f, deltaTime);
    mBytesSent = 0;
    mBytesReceived = 0;

    if (mNetStatus == NetStatus::Connecting)
    {
        int32_t prevTimerInt = (int32_t)mConnectTimer;
        mConnectTimer += deltaTime;
        int32_t currTimerInt = (int32_t)mConnectTimer;

        if (mConnectTimer >= mConnectTimeout)
        {
            LogError("Failed to connect to session (timeout).");
            Disconnect();
        }
        else if (prevTimerInt != currTimerInt)
        {
            // Attempt to send another Connect message if we still haven't heard back.
            NetMsgConnect connectMsg;
            connectMsg.mGameCode = GetEngineState()->mGameCode;
            connectMsg.mVersion = GetEngineState()->mVersion;
            NetworkManager::SendMessage(&connectMsg, &mServer);
        }
    }

    if (mNetStatus != NetStatus::Local)
    {
        // Handle incoming messages from the server or clients
        ProcessIncomingPackets(deltaTime);

        // Handle upkeep of unreliable messages that still need ACKs from recipients
        UpdateReliablePackets(deltaTime);

        // Disconnect from hosts that have timed out.
        UpdateHostConnections(deltaTime);
    }

    if (mSearching)
    {
        UpdateSearch();
    }
}

void NetworkManager::PostTickUpdate(float deltaTime)
{
    SCOPED_FRAME_STAT("Network");

    if (mNetStatus == NetStatus::Server)
    {
        // Server needs to send replicated actor data to clients
        UpdateReplication(deltaTime);

        mBroadcastTimer -= deltaTime;
        if (mBroadcastTimer <= 0.0f)
        {
            BroadcastSession();
            mBroadcastTimer = OCT_BROADCAST_INTERVAL;
        }
    }

    if (mNetStatus == NetStatus::Client)
    {
        mPingTimer += deltaTime;
        if (mPingTimer >= OCT_PING_INTERVAL)
        {
            NetMsgPing pingMsg;
            SendMessage(&pingMsg, &mServer);
            mPingTimer = 0.0f;
        }
    }

    FlushSendBuffers();

#if DEBUG_NETWORK_CONDITIONS
    UpdateDebugPackets(deltaTime);
#endif
}

void NetworkManager::OpenSession(uint16_t port)
{
    if (!NET_IsActive())
        return;

    if (mNetStatus == NetStatus::Local)
    {
        mSocket = NET_SocketCreate();
        NET_SocketSetBlocking(mSocket, false);
        // Broadcast is used for LAN game discovery.
        NET_SocketSetBroadcast(mSocket, true);

        if (mSocket >= 0)
        {
            NET_SocketBind(mSocket, NET_ANY_IP, port);
            mNetStatus = NetStatus::Server;
            mHostId = SERVER_HOST_ID;
            LogDebug("Session opened.");

            // Determine the broadcast IP based on the subnet mask.
            uint32_t subnetMask = NET_GetSubnetMask();
            uint32_t localIp = NET_GetIpAddress();
            uint32_t netIp = localIp & subnetMask;
            mBroadcastIp = netIp | (~subnetMask);
            LogDebug("Broadcast IP: %08x", mBroadcastIp);
        }
        else
        {
            LogError("Failed to create socket.");
        }
    }
    else
    {
        LogError("Failed to open session.");
    }
}

void NetworkManager::CloseSession()
{
    if (mNetStatus == NetStatus::Server)
    {
        while (!mClients.empty())
        {
            Kick(mClients[0].mHost.mId, NetMsgKick::Reason::SessionClose);
        }

        NET_SocketClose(mSocket);
        mSocket = NET_INVALID_SOCKET;
        mNetStatus = NetStatus::Local;
        mHostId = INVALID_HOST_ID;
    }
    else
    {
        LogWarning("NetworkManager::CloseSession() called but no session is open.");
    }
}

void NetworkManager::BeginSessionSearch()
{
    if (!NET_IsActive())
        return;

    mSearching = true;
    mSessions.clear();
    mSessions.shrink_to_fit();
    mSearchSocket = NET_SocketCreate();
    NET_SocketSetBlocking(mSearchSocket, false);

    if (mSearchSocket >= 0)
    {
        NET_SocketBind(mSearchSocket, NET_ANY_IP, OCT_BROADCAST_PORT);
        LogDebug("Searching for sessions on network...");
    }
    else
    {
        LogError("Failed to create search socket.");
    }
}

void NetworkManager::EndSessionSearch()
{
    mSearching = false;

    if (mSearchSocket != NET_INVALID_SOCKET)
    {
        NET_SocketClose(mSearchSocket);
        mSearchSocket = NET_INVALID_SOCKET;
    }
}

void NetworkManager::UpdateSearch()
{
    int32_t bytes = 0;
    uint32_t address = 0;
    uint16_t port = 0;

    while ((bytes = NET_SocketRecvFrom(mSearchSocket, sRecvBuffer, OCT_RECV_BUFFER_SIZE, address, port)) > 0)
    {
        const uint32_t minSize = sizeof(uint16_t) + sizeof(bool) + sizeof(NetMsgType);
        static_assert(minSize == 4, "Unexpected min size for BC packet");

        if (bytes >= int32_t(minSize))
        {
            Stream stream(sRecvBuffer, bytes);

            // Read seq num and reliable flag (unused by broadcast packets).
            stream.ReadUint16();
            stream.ReadBool();

            NetMsgType msgType = (NetMsgType) stream.GetData()[stream.GetPos()];

            if (msgType == NetMsgType::Broadcast)
            {
                NetHost sender;
                sender.mIpAddress = address;
                sender.mPort = port;
                sender.mId = SERVER_HOST_ID;

                LogDebug("Received Session Broadcast");
                NetMsgBroadcast bcMsg;
                bcMsg.Read(stream);
                bcMsg.Execute(sender);
            }
        }

        mBytesReceived += bytes;

#if DEBUG_MSG_STATS
        sNumPacketsReceived++;
#endif
    }
}

bool NetworkManager::IsSearching() const
{
    return mSearching;
}

const std::vector<GameSession>& NetworkManager::GetSessions() const
{
    return mSessions;
}

void NetworkManager::Connect(const char* ipAddress, uint16_t port)
{
    uint32_t ipAddrInt = NET_IpStringToUint32(ipAddress);
    Connect(ipAddrInt, port);
}

void NetworkManager::Connect(uint32_t ipAddress, uint16_t port)
{
    if (!NET_IsActive())
        return;

    if (mNetStatus == NetStatus::Local)
    {
        mSocket = NET_SocketCreate();
        NET_SocketSetBlocking(mSocket, false);

        if (mSocket >= 0)
        {
            LogDebug("Connecting to session...");
            mNetStatus = NetStatus::Connecting;
            mConnectTimer = 0.0f;
            ResetHostProfile(&mServer);
            mServer.mHost.mIpAddress = ipAddress;
            mServer.mHost.mPort = port;
            mServer.mHost.mId = SERVER_HOST_ID;

            NetMsgConnect connectMsg;
            connectMsg.mGameCode = GetEngineState()->mGameCode;
            connectMsg.mVersion = GetEngineState()->mVersion;
            NetworkManager::SendMessage(&connectMsg, &mServer);
        }
        else
        {
            LogError("Failed to create socket.");
        }
    }
    else
    {
        LogError("NetworkManager::Connect() called while already in session or connecting to a session.");
    }
}

void NetworkManager::Disconnect()
{
    if (mNetStatus == NetStatus::Client ||
        mNetStatus == NetStatus::Connecting)
    {
        if (mNetStatus == NetStatus::Client)
        {
            NetMsgDisconnect disconnectMsg;
            SendMessage(&disconnectMsg, &mServer);
            FlushSendBuffers(&mServer);
        }

        ResetToLocalStatus();
    }
    else
    {
        LogWarning("NetworkManager::Disconnect() called but not connected/connecting to a remote session.");
    }
}

void NetworkManager::Kick(NetHostId hostId, NetMsgKick::Reason reason)
{
    for (uint32_t i = 0; i < mClients.size(); ++i)
    {
        if (mClients[i].mHost.mId == hostId)
        {
            NetMsgKick kickMsg;
            kickMsg.mReason = reason;
            SendMessage(&kickMsg, &mClients[i]);
            FlushSendBuffers(&mClients[i]);

            // Run the disconnect callback so the game code can handle the player leaving.
            if (mDisconnectCallback.mFuncPointer != nullptr)
            {
                mDisconnectCallback.mFuncPointer(&mClients[i]);
            }
            if (mDisconnectCallback.mScriptTableName != "")
            {
                ScriptEvent::NetDisconnect(
                    mDisconnectCallback.mScriptTableName,
                    mDisconnectCallback.mScriptFuncName,
                    mClients[i]);
            }

            LogDebug("Kicking client %08x:%u", mClients[i].mHost.mIpAddress, mClients[i].mHost.mPort);
            mClients.erase(mClients.begin() + i);
            break;
        }
    }
}

void NetworkManager::SetMaxClients(uint32_t maxClients)
{
    mMaxClients = maxClients;
}

uint32_t NetworkManager::GetMaxClients()
{
    return mMaxClients;
}

const std::vector<NetClient>& NetworkManager::GetClients() const
{
    return mClients;
}

void NetworkManager::SendMessage(const NetMsg* netMsg, NetHostId receiverId)
{
    NetHostProfile* hostProfile = nullptr;

    if (mNetStatus == NetStatus::Client ||
        mNetStatus == NetStatus::Connecting)
    {
        // Clients can only send messages to the server, not other clients.
        OCT_ASSERT(receiverId == SERVER_HOST_ID);
        hostProfile = &mServer;
    }
    else if (mNetStatus == NetStatus::Server)
    {
        hostProfile = FindNetClient(receiverId);
    }

    if (hostProfile != nullptr)
    {
        SendMessage(netMsg, hostProfile);
    }
    else
    {
        LogError("Failed to send message to NetHostId %u", (uint32_t) receiverId);
    }
}

void NetworkManager::SendMessage(const NetMsg* netMsg, NetHostProfile* hostProfile)
{
    if (hostProfile != nullptr)
    {
        char msgData[OCT_MAX_MSG_SIZE] = {};
        Stream stream(msgData, OCT_MAX_MSG_SIZE);
        netMsg->Write(stream);

        bool reliable = netMsg->IsReliable();
        std::vector<char>& sendBuffer = reliable ? hostProfile->mReliableSendBuffer : hostProfile->mSendBuffer;

        // If this newly serialized message would cause send buffer to exceed max message size,
        // then send out the queued messages first.
        if (sendBuffer.size() + stream.GetPos() > OCT_MAX_MSG_SIZE)
        {
            FlushSendBuffer(hostProfile, reliable);
        }

        uint32_t startByte = (uint32_t)sendBuffer.size();
        sendBuffer.resize(sendBuffer.size() + stream.GetPos());
        memcpy(sendBuffer.data() + startByte, stream.GetData(), stream.GetPos());
    }
}

void NetworkManager::SendMessageToAllClients(const NetMsg* netMsg)
{
    OCT_ASSERT(IsServer());
    for (uint32_t i = 0; i < mClients.size(); ++i)
    {
        SendMessage(netMsg, &mClients[i]);
    }
}

void NetworkManager::SendMessageImmediate(const NetMsg* netMsg, uint32_t ipAddress, uint16_t port)
{
    // Immediate messages don't rely on a sequence number.
    // Connect and Reject should always be seq num 0 since they are the first messages sent between
    // the server and the client. Broadcast uses a totally different socket / recv function.
    uint16_t seqNum = 0;

    Stream stream(sSendBuffer, OCT_SEND_BUFFER_SIZE);
    stream.WriteUint16(seqNum);
    stream.WriteBool(false);
    netMsg->Write(stream);

    // If this newly serialized message would cause send buffer to exceed max message size,
    // then send out the queued messages first.
    if (stream.GetPos() <= OCT_MAX_MSG_SIZE)
    {

#if DEBUG_NETWORK_CONDITIONS
        mBytesSent += DebugSendTo(mSocket, ipAddress, port, stream.GetPos(), stream.GetData());
#else
        mBytesSent += NET_SocketSendTo(
                mSocket,
                stream.GetData(),
                stream.GetPos(),
                ipAddress,
                port);
#endif

#if DEBUG_MSG_STATS
        sNumPacketsSent++;
#endif
    }
    else
    {
        LogError("SendMessageImmediate: message is too large to send. Type = %d", (int) netMsg->GetType());
        OCT_ASSERT(0);
    }
}

NetClient* NetworkManager::FindNetClient(NetHostId id)
{
    NetClient* retClient = nullptr;

    for (uint32_t i = 0; i < mClients.size(); ++i)
    {
        if (mClients[i].mHost.mId == id)
        {
            retClient = &mClients[i];
            break;
        }
    }

    return retClient;
}

NetStatus NetworkManager::GetNetStatus() const
{
    return mNetStatus;
}

void NetworkManager::EnableIncrementalReplication(bool enable)
{
    mIncrementalReplication = enable;
}

bool NetworkManager::IsIncrementalReplicationEnabled() const
{
    return mIncrementalReplication;
}

int32_t NetworkManager::GetBytesSent() const
{
    return mBytesSent;
}

int32_t NetworkManager::GetBytesReceived() const
{
    return mBytesReceived;
}
    
float NetworkManager::GetUploadRate() const
{
    return mUploadRate;
}

    
float NetworkManager::GetDownloadRate() const
{
    return mDownloadRate;
}


bool NetworkManager::IsServer() const
{
    return (mNetStatus == NetStatus::Server);
}

bool NetworkManager::IsClient() const
{
    return (mNetStatus == NetStatus::Client);
}

bool NetworkManager::IsLocal() const
{
    return (mNetStatus == NetStatus::Local);
}

bool NetworkManager::IsAuthority() const
{
    return (mNetStatus == NetStatus::Local ||
            mNetStatus == NetStatus::Server);
}

NetHostId NetworkManager::GetHostId() const
{
    return mHostId;
}

void NetworkManager::HandleConnect(NetHost host, uint32_t gameCode, uint32_t version)
{
    // Only handle connections on the server from unknown senders.
    // Ignore if a second "Connect" message was received from current client.
    if (IsServer() && host.mId == INVALID_HOST_ID)
    {
        bool accept = true;
        NetMsgReject::Reason rejectReason = NetMsgReject::Reason::Count;

        if (gameCode != GetEngineState()->mGameCode)
        {
            // Different game
            accept = false;
            rejectReason = NetMsgReject::Reason::InvalidGameCode;
        }
        else if (version != GetEngineState()->mVersion)
        {
            // Different version
            accept = false;
            rejectReason = NetMsgReject::Reason::VersionMismatch;
        }
        else if (mClients.size() >= mMaxClients)
        {
            accept = false;
            rejectReason = NetMsgReject::Reason::SessionFull;
        }
        else
        {
            // Everything looks ok, so add this new host to our client list.
            // Find the first open NetHostId and assign it to them.
            // We will send back their assigned NetHostId in the Accept message.
            mClients.push_back(NetClient());
            NetClient* newClient = &mClients.back();
            newClient->mHost.mIpAddress = host.mIpAddress;
            newClient->mHost.mPort = host.mPort;
            newClient->mHost.mId = FindAvailableNetHostId();

            NetMsgAccept acceptMsg;
            acceptMsg.mAssignedHostId = newClient->mHost.mId;
            SendMessage(&acceptMsg, newClient);

            // Send LoadLevel messages for each loaded level that is flagged for NetLoad.
            const std::vector<LevelRef>& levels = GetWorld()->GetLoadedLevels();
            for (uint32_t i = 0; i < levels.size(); ++i)
            {
                if (levels[i].Get<Level>()->GetNetLoad())
                {
                    NetMsgLoadLevel levelMsg;
                    levelMsg.mLevelName = levels[i].Get<Level>()->GetName();
                    SendMessage(&levelMsg, newClient);
                }
            }

            // Spawn any replicated actors.
            const std::unordered_map<NetId, Actor*>& netActorMap = GetWorld()->GetNetActorMap();
            for (auto it = netActorMap.begin(); it != netActorMap.end(); ++it)
            {
                SendSpawnMessage(it->second, newClient);
            }

            // Send a message asking for the client to send a response after processing
            NetMsgReady readyMsg;
            SendMessage(&readyMsg, newClient);

            // Mark the client as unready until we get a ReadyConfirm message back
            FlushSendBuffers(newClient);
            newClient->mReady = false;


            if (mConnectCallback.mFuncPointer != nullptr)
            {
                mConnectCallback.mFuncPointer(newClient);
            }
            if (mConnectCallback.mScriptTableName != "")
            {
                ScriptEvent::NetConnect(
                    mConnectCallback.mScriptTableName,
                    mConnectCallback.mScriptFuncName,
                    *newClient);
            }
        }

        if (!accept)
        {
            // Send back a Reject message to let them know why their connection was rejected.
            NetMsgReject rejectMsg;
            rejectMsg.mReason = rejectReason;
            SendMessageImmediate(&rejectMsg, host.mIpAddress, host.mPort);
        }
    }
}

void NetworkManager::HandleAccept(NetHostId assignedId)
{
    if (mNetStatus == NetStatus::Connecting)
    {
        LogDebug("Connected to server %08x:%u", mServer.mHost.mIpAddress, mServer.mHost.mPort);
        mNetStatus = NetStatus::Client;
        mHostId = assignedId;

        if (mAcceptCallback.mFuncPointer != nullptr)
        {
            mAcceptCallback.mFuncPointer();
        }
        if (mAcceptCallback.mScriptTableName != "")
        {
            ScriptEvent::NetAccept(
                mAcceptCallback.mScriptTableName,
                mAcceptCallback.mScriptFuncName);
        }
    }
}

void NetworkManager::HandleReject(NetMsgReject::Reason reason)
{
    if (mNetStatus == NetStatus::Connecting)
    {
        LogWarning("Rejected from server. Reason: %u", (uint32_t) reason);
        ResetToLocalStatus();

        if (mRejectCallback.mFuncPointer != nullptr)
        {
            mRejectCallback.mFuncPointer(reason);
        }
        if (mRejectCallback.mScriptTableName != "")
        {
            ScriptEvent::NetReject(
                mRejectCallback.mScriptTableName,
                mRejectCallback.mScriptFuncName,
                reason);
        }
    }
}

void NetworkManager::HandleDisconnect(NetHost host)
{
    if (IsServer())
    {
        LogDebug("Client disconnected: %08x:%u", host.mIpAddress, host.mPort);

        // Remove the client from the client list.
        bool removed = false;
        for (uint32_t i = 0; i < mClients.size(); ++i)
        {
            if (mClients[i].mHost.mId == host.mId &&
                mClients[i].mHost.mIpAddress == host.mIpAddress &&
                mClients[i].mHost.mPort == host.mPort)
                {
                    if (mDisconnectCallback.mFuncPointer != nullptr)
                    {
                        mDisconnectCallback.mFuncPointer(&mClients[i]);
                    }
                    if (mDisconnectCallback.mScriptTableName != "")
                    {
                        ScriptEvent::NetDisconnect(
                            mDisconnectCallback.mScriptTableName,
                            mDisconnectCallback.mScriptFuncName,
                            mClients[i]);
                    }

                    mClients.erase(mClients.begin() + i);
                    removed = true;
                    break;
                }
        }

        // We should have found a client.
        // ProcessMessages() ensures that only messages from clients are processed.
        OCT_ASSERT(removed);
    }
}

void NetworkManager::HandleKick(NetMsgKick::Reason reason)
{
    if (mNetStatus == NetStatus::Client)
    {
        LogWarning("Kicked from server. Reason: %u", (uint32_t) reason);
        ResetToLocalStatus();

        if (mKickCallback.mFuncPointer != nullptr)
        {
            mKickCallback.mFuncPointer(reason);
        }
        if (mKickCallback.mScriptTableName != "")
        {
            ScriptEvent::NetKick(
                mKickCallback.mScriptTableName,
                mKickCallback.mScriptFuncName,
                reason);
        }
    }
}

void NetworkManager::HandleAck(NetHost host, uint16_t sequenceNumber)
{
    NetHostProfile* profile = NetIsServer() ? FindNetClient(host.mId) : &mServer;

    if (profile != nullptr)
    {
        // Remove the reliable packet with matching sequence number from our outgoing list
        std::vector<ReliablePacket>& packets = profile->mOutgoingPackets;

        for (uint32_t i = 0; i < packets.size(); ++i)
        {
            if (packets[i].mSeq == sequenceNumber)
            {
                //LogDebug("ACK: %d", sequenceNumber);
                packets.erase(packets.begin() + i);
                break;
            }
        }
    }
}

void NetworkManager::HandleReady(NetHost host)
{
    if (NetIsClient())
    {
        // Client needs to confirm that it received the Ready message by sending back a Ready message
        OCT_ASSERT(host.mId == SERVER_HOST_ID);
        NetMsgReady readyMsg;
        SendMessage(&readyMsg, &mServer);
    }
    else if (NetIsServer())
    {
        NetClient* client = FindNetClient(host.mId);

        if (client != nullptr &&
            !client->mReady)
        {
            client->mReady = true;

            // Resend any pending outgoing reliable messages
            ResendOutgoingReliablePackets(client);

            // Now that client has loaded the level(s) and spawned actors,
            // Forcefully replicate the initial state of all actors
            const std::unordered_map<NetId, Actor*>& netActorMap = GetWorld()->GetNetActorMap();
            for (auto it = netActorMap.begin(); it != netActorMap.end(); ++it)
            {
                Actor* actor = it->second;
                ReplicateActor(actor, client->mHost.mId, true, true);
            }
        }
    }
}

void NetworkManager::HandleBroadcast(
    NetHost host,
    uint32_t gameCode,
    uint32_t version,
    const char* name,
    uint8_t maxPlayers,
    uint8_t numPlayers)
{
    if (mSearching &&
        gameCode == GetEngineState()->mGameCode &&
        version == GetEngineState()->mVersion)
    {
        GameSession* session = nullptr;

        for (uint32_t i = 0; i < mSessions.size(); ++i)
        {
            if (mSessions[i].mHost.mIpAddress == host.mIpAddress &&
                mSessions[i].mHost.mPort == host.mPort)
            {
                session = &mSessions[i];
                break;
            }
        }

        if (session == nullptr &&
            mSessions.size() < OCT_MAX_SESSION_LIST_SIZE)
        {
            LogDebug("Found new session");
            mSessions.push_back(GameSession());
            session = &mSessions.back();
        }

        // Copy the new session data.
        if (session != nullptr)
        {
            session->mHost = host;
            memcpy(session->mName, name, OCT_SESSION_NAME_LEN);
            session->mName[OCT_SESSION_NAME_LEN] = 0;
            session->mMaxPlayers = maxPlayers;
            session->mNumPlayers = numPlayers;

            LogDebug("Session: %08x:%u - %s", session->mHost.mIpAddress, session->mHost.mPort, session->mName);
        }
    }
}

static const uint32_t RepMsgHeaderSize = 
    sizeof(NetMsgReplicate::mActorNetId) +
    sizeof(NetMsgReplicate::mNumVariables);

static const uint32_t MaxDatumNetSerializeSize = 
    OCT_MAX_MSG_SIZE - 
    RepMsgHeaderSize -
    sizeof(uint16_t); // 1 index

void NetworkManager::SendReplicateMsg(NetMsgReplicate& repMsg, uint32_t& numVars, NetHostId hostId)
{
    OCT_ASSERT(numVars > 0);

    repMsg.mNumVariables = numVars;

    if (hostId == INVALID_HOST_ID)
    {
        SendMessageToAllClients(&repMsg);
    }
    else
    {
        SendMessage(&repMsg, hostId);
    }

    repMsg.mIndices.clear();
    repMsg.mData. clear();
    repMsg.mNumVariables = 0;
    numVars = 0;
}

void NetworkManager::SendInvokeMsg(NetMsgInvoke& msg, Actor* actor, NetFunc* func, uint32_t numParams, Datum** params)
{
    NetFuncType type = func->mType;
    bool scriptMsg = msg.GetType() == NetMsgType::InvokeScript;
    OCT_ASSERT(scriptMsg || numParams == func->mNumParams); // Script NetFuncs do not setup num params.

    msg.mActorNetId = actor->GetNetId();
    msg.mIndex = func->mIndex;
    msg.mNumParams = numParams;
    msg.mReliable = func->mReliable;

    msg.mParams.clear();

    for (uint32_t i = 0; i < numParams; ++i)
    {
        msg.mParams.push_back(Datum(*params[i]));
    }

    switch (type)
    {
    case NetFuncType::Server:
    {
        OCT_ASSERT(mNetStatus == NetStatus::Client);
        SendMessage(&msg, &mServer);
        break;
    }
    case NetFuncType::Client:
    {
        OCT_ASSERT(mNetStatus == NetStatus::Server);
        OCT_ASSERT(actor->GetOwningHost() != INVALID_HOST_ID);
        OCT_ASSERT(actor->GetOwningHost() != SERVER_HOST_ID);
        SendMessage(&msg, actor->GetOwningHost());
        break;
    }
    case NetFuncType::Multicast:
    {
        SendMessageToAllClients(&msg);
        break;
    }

    case NetFuncType::Count: OCT_ASSERT(0); break;
    }
}

void NetworkManager::SendInvokeMsg(Actor* actor, NetFunc* func, uint32_t numParams, Datum** params)
{
    SendInvokeMsg(sMsgInvoke, actor, func, numParams, params);
}

void NetworkManager::SendInvokeScriptMsg(ScriptComponent* scriptComp, ScriptNetFunc* func, uint32_t numParams, Datum** params)
{
    Actor* actor = scriptComp->GetOwner();
    sMsgInvokeScript.mScriptName = scriptComp->GetScriptClassName();
    SendInvokeMsg(sMsgInvokeScript, actor, func, numParams, params);
}

void NetworkManager::SendSpawnMessage(Actor* actor, NetClient* client)
{
    Blueprint* bpSource = actor->GetBlueprintSource();

    if (bpSource != nullptr)
    {
        NetMsgSpawnBlueprint spawnMsg;
        spawnMsg.mBlueprintName = bpSource->GetName();
        spawnMsg.mNetId = actor->GetNetId();

        if (client == nullptr)
        {
            SendMessageToAllClients(&spawnMsg);
        }
        else
        {
            SendMessage(&spawnMsg, client);
        }
    }
    else
    {
        NetMsgSpawnActor spawnMsg;
        spawnMsg.mActorTypeId = actor->GetType();
        spawnMsg.mNetId = actor->GetNetId();

        if (client == nullptr)
        {
            SendMessageToAllClients(&spawnMsg);
        }
        else
        {
            SendMessage(&spawnMsg, client);
        }
    }
}

void NetworkManager::SendDestroyMessage(Actor* actor, NetClient* client)
{
    OCT_ASSERT(actor != nullptr);
    NetMsgDestroyActor destroyMsg;
    destroyMsg.mNetId = actor->GetNetId();

    if (client == nullptr)
    {
        SendMessageToAllClients(&destroyMsg);
    }
    else
    {
        SendMessage(&destroyMsg, client);
    }
}

void NetworkManager::ResendPacket(NetHostProfile* hostProfile, ReliablePacket& packet)
{
    // Resend the packet
    mBytesSent += NET_SocketSendTo(
        mSocket,
        packet.mData.data(),
        (uint32_t)packet.mData.size(),
        hostProfile->mHost.mIpAddress,
        hostProfile->mHost.mPort);

    packet.mTimeSinceSend = 0.0f;
    packet.mNumSends++;
}

void NetworkManager::ResendOutgoingReliablePackets(NetHostProfile* hostProfile)
{
    if (hostProfile != nullptr &&
        hostProfile->mReady)
    {
        std::vector<ReliablePacket>& packets = hostProfile->mOutgoingPackets;

        for (uint32_t i = 0; i < packets.size(); ++i)
        {
            // TODO: UHHH WHAT? This was empty? NEED to implement this??? I would think so?
            // Or maybe we don't need it because we hold on to all reliable messages until the client is ready
            // and then they get sent after. See FlushSendBuffer() and UpdateReliablePackets(NetHostProfile*, float))
        }
    }
}

void NetworkManager::FlushSendBuffers()
{
    if (mNetStatus == NetStatus::Server)
    {
        for (uint32_t i = 0; i < mClients.size(); ++i)
        {
            FlushSendBuffers(&mClients[i]);
        }
    }
    else if (mNetStatus == NetStatus::Client ||
            mNetStatus == NetStatus::Connecting)
    {
        FlushSendBuffers(&mServer);
    }
}

void NetworkManager::UpdateReplication(float deltaTime)
{
    OCT_ASSERT(mNetStatus == NetStatus::Server);

    Actor* incRepActor = nullptr;

    if (mIncrementalReplication)
    {
        uint32_t& incActorTier = GetWorld()->GetIncrementalRepTier();
        uint32_t& incActorIndex = GetWorld()->GetIncrementalRepIndex();
        std::vector<Actor*>& repVector = GetWorld()->GetReplicatedActorVector((ReplicationRate)incActorTier);

        if (incActorIndex < repVector.size())
        {
            incRepActor = repVector[incActorIndex];
        }

        ++incActorIndex;

        // If we've iterated over all these actors in this rep tier, move on to the next.
        if (incActorIndex > repVector.size())
        {
            incActorIndex = 0;
            incActorTier++;
            if (incActorTier == (uint32_t)ReplicationRate::Count)
            {
                incActorTier = (uint32_t)ReplicationRate::Low;
            }
        }
    }


    uint32_t numActorsReplicated = 0;

    auto replicateTier = [this, incRepActor, &numActorsReplicated](const std::vector<Actor*>& repVector, uint32_t& repIndex, uint32_t count)
    {
        // Loop back around if the index is somehow past the vector size already
        if (repIndex >= repVector.size())
        {
            repIndex = 0;
        }

        for (uint32_t i = 0; i < count; ++i)
        {
            Actor* actor = repVector[repIndex];
            bool forceRep = (actor == incRepActor);
            bool actorReplicated = ReplicateActor(actor, INVALID_HOST_ID, forceRep, false);

            if (actorReplicated)
            {
                numActorsReplicated++;
            }

            ++repIndex;

            if (repIndex >= repVector.size())
            {
                repIndex = 0;
            }
        }
    };

    // High Priority
    {
        const std::vector<Actor*>& repVector = GetWorld()->GetReplicatedActorVector(ReplicationRate::High);
        uint32_t& repIndex = GetWorld()->GetReplicatedActorIndex(ReplicationRate::High);
        uint32_t vectorSize = (uint32_t) repVector.size();
        uint32_t count = vectorSize / 1;
        replicateTier(repVector, repIndex, count);
    }

    // Medium Priority
    {
        const std::vector<Actor*>& repVector = GetWorld()->GetReplicatedActorVector(ReplicationRate::Medium);
        uint32_t& repIndex = GetWorld()->GetReplicatedActorIndex(ReplicationRate::Medium);
        uint32_t vectorSize = (uint32_t) repVector.size();
        uint32_t count = (vectorSize + 1) / 2;
        replicateTier(repVector, repIndex, count);
    }
    // Low Priority
    {
        const std::vector<Actor*>& repVector = GetWorld()->GetReplicatedActorVector(ReplicationRate::Low);
        uint32_t& repIndex = GetWorld()->GetReplicatedActorIndex(ReplicationRate::Low);
        uint32_t vectorSize = (uint32_t) repVector.size();
        uint32_t count = (vectorSize + 3) / 4;
        replicateTier(repVector, repIndex, count);
    }
}

template<typename T>
bool ReplicateData(std::vector<T>& repData, NetMsgReplicate& msg, NetId hostId, bool force, bool reliable)
{
    // msg.mNetId and msg.mScriptName should already be set by caller.
    msg.mIndices.clear();
    msg.mData.clear();
    msg.mReliable = reliable;

    bool scriptRep = (msg.GetType() == NetMsgType::ReplicateScript);
    bool replicated = false;
    uint32_t numVars = 0;
    uint32_t scriptNameBytes = 0;

    // Strings are serialized with a 4 byte count, followed by the bytes.
    if (scriptRep)
    {
        scriptNameBytes = sizeof(uint32_t) + uint32_t(static_cast<NetMsgReplicateScript&>(msg).mScriptName.size());
    }

    uint32_t msgSerializedSize = RepMsgHeaderSize + scriptNameBytes;

    for (uint32_t i = 0; i < repData.size(); ++i)
    {
        if (force || repData[i].ShouldReplicate())
        {
            // First check if the replicated variable will fit into the message.
            // If not, we will need to send a message for all of the replicated vars
            // that have been processed to this point, and then begin a new message.
            uint32_t datumSerializeSize = repData[i].GetSerializationSize();

            // If the replicated variable is too large, then skip it.
            if (datumSerializeSize > MaxDatumNetSerializeSize)
            {
                LogWarning("Replicated variable too large to replicate. Most likely a big string.");
                continue;
            }
            else if (msgSerializedSize + datumSerializeSize > OCT_MAX_MSG_SIZE)
            {
                // Send what we have until now
                NetworkManager::Get()->SendReplicateMsg(msg, numVars, hostId);
                msgSerializedSize = RepMsgHeaderSize + scriptNameBytes;
                replicated = true;
            }

            msg.mIndices.push_back((uint16_t)i);
            msg.mData.push_back(Datum(repData[i]));

            numVars++;
            msgSerializedSize += datumSerializeSize;
            repData[i].PostReplicate();
        }
    }

    if (numVars > 0)
    {
        NetworkManager::Get()->SendReplicateMsg(msg, numVars, hostId);
        msgSerializedSize = RepMsgHeaderSize;
        replicated = true;
    }

    return replicated;
}

bool NetworkManager::ReplicateActor(Actor* actor, NetId hostId, bool force, bool reliable)
{
    bool actorReplicated = false;
    bool needsForcedRep = actor->NeedsForcedReplication();
    force = (force || needsForcedRep);
    reliable = (reliable || needsForcedRep);
    sMsgReplicate.mActorNetId = actor->GetNetId();

    std::vector<NetDatum>& repData = actor->GetReplicatedData();

    actorReplicated = ReplicateData<NetDatum>(repData, sMsgReplicate, hostId, force, reliable);

    if (actor->GetNumComponents() > 0)
    {
        const std::vector<Component*>& comps = actor->GetComponents();
        for (uint32_t i = 0; i < comps.size(); ++i)
        {
            if (comps[i]->GetType() == ScriptComponent::GetStaticType() &&
                static_cast<ScriptComponent*>(comps[i])->GetTableName() != "")
            {
                ScriptComponent* scriptComp = static_cast<ScriptComponent*>(comps[i]);
                sMsgReplicateScript.mScriptName = scriptComp->GetScriptClassName();
                sMsgReplicateScript.mActorNetId = actor->GetNetId();

                std::vector<ScriptNetDatum>& scriptRepData = scriptComp->GetReplicatedData();
                actorReplicated = ReplicateData<ScriptNetDatum>(scriptRepData, sMsgReplicateScript, hostId, force, reliable);
            }
        }
    }

    actor->ClearForcedReplication();

    return actorReplicated;
}

void NetworkManager::UpdateHostConnections(float deltaTime)
{
    if (IsServer())
    {
        for (int32_t i = int32_t(mClients.size()) - 1; i >= 0; --i)
        {
            mClients[i].mTimeSinceLastMsg += deltaTime;

            if (mClients[i].mTimeSinceLastMsg >= mInactiveTimeout ||
                mClients[i].mOutgoingPackets.size() > sMaxOutgoingPackets)
            {
                Kick(mClients[i].mHost.mId, NetMsgKick::Reason::Timeout);
            }
        }
    }
    else if (IsClient())
    {
        mServer.mTimeSinceLastMsg += deltaTime;
        if (mServer.mTimeSinceLastMsg >= mInactiveTimeout ||
            mServer.mOutgoingPackets.size() > sMaxOutgoingPackets)
        {
            Disconnect();

            if (mKickCallback.mFuncPointer != nullptr)
            {
                mKickCallback.mFuncPointer(NetMsgKick::Reason::Timeout);
            }
            if (mKickCallback.mScriptTableName != "")
            {
                ScriptEvent::NetKick(
                    mKickCallback.mScriptTableName,
                    mKickCallback.mScriptFuncName,
                    NetMsgKick::Reason::Timeout);
            }
        }
    }
}

void NetworkManager::ProcessIncomingPackets(float deltaTime)
{
    int32_t bytes = 0;
    uint32_t address = 0;
    uint16_t port = 0;

    while ((bytes =  NET_SocketRecvFrom(mSocket, sRecvBuffer, OCT_RECV_BUFFER_SIZE, address, port)) > 0)
    {   
        Stream stream(sRecvBuffer, bytes);
        NetMsgType msgType = (NetMsgType) sRecvBuffer[OCT_PACKET_HEADER_SIZE];

        // Find which NetHost the message was from.
        // if there is no matching NetHost then ignore this message (unless it is a "Connect" message)
        NetHost sender;
        sender.mIpAddress = address;
        sender.mPort = port;
        sender.mId = INVALID_HOST_ID;

        NetHostProfile* senderProfile = nullptr;

        // Connect messages are only executed on the Server
        bool connectMsg = mNetStatus == NetStatus::Server && 
                          msgType == NetMsgType::Connect;

        if (mNetStatus == NetStatus::Server)
        {
            for (uint32_t i = 0; i < mClients.size(); ++i)
            {
                if (mClients[i].mHost.mIpAddress == sender.mIpAddress &&
                    mClients[i].mHost.mPort == sender.mPort)
                {
                    OCT_ASSERT(mClients[i].mHost.mId != INVALID_HOST_ID);
                    sender.mId = mClients[i].mHost.mId;
                    mClients[i].mTimeSinceLastMsg = 0.0f;

                    senderProfile = &mClients[i];

                    break;
                }
            }
        }
        else
        {
            if (mServer.mHost.mIpAddress == sender.mIpAddress &&
                mServer.mHost.mPort == sender.mPort)
            {
                OCT_ASSERT(mServer.mHost.mId == SERVER_HOST_ID);
                sender.mId = mServer.mHost.mId;
                mServer.mTimeSinceLastMsg = 0.0f;

                senderProfile = &mServer;
            }
        }

        if (!connectMsg &&
            (sender.mId == INVALID_HOST_ID || senderProfile == nullptr))
        {
            LogDebug("Unrecognized host: %08x:%u", address, port);
            continue;
        }

        uint16_t seq = stream.ReadUint16();
        bool reliable = stream.ReadBool();

        bool processMsg = false;

        if (connectMsg)
        {
            processMsg = true;
        }
        else if (reliable)
        {
            uint16_t& curSeq = senderProfile->mIncomingReliableSeq;
            bool ack = false;

            if (seq == curSeq)
            {
                // We received the next expected packet, so process it.
                processMsg = true;
                ack = true;
                curSeq++;
            }
            else if (SeqNumLess(seq, curSeq))
            {
                // If the received seq is less than the current seq, don't process the packet, as it should
                // have already been processed previously. Send an Ack back saying that the message has been acknowledged.
                processMsg = false;
                ack = true;
            }
            else
            {
                if (senderProfile->mIncomingPackets.size() < sMaxIncomingPackets &&
                    !HostProfileHasIncomingPacket(senderProfile, seq))
                {
                    //LogError("Queuing reliable packet %d - Waiting on %d", seq, curSeq);
                    // The received seq number is ahead of our current expected seq num, so we need to queue it up.
                    const char* data = &(stream.GetData()[stream.GetPos()]);
                    uint32_t size = bytes - stream.GetPos();
                    OCT_ASSERT(size > 0);
                    senderProfile->mIncomingPackets.emplace_back(seq, data, size);
                    ack = true;
                }

                processMsg = false;
            }

            if (ack)
            {
                NetMsgAck ackMsg;
                ackMsg.mSequenceNumber = seq;
                SendMessage(&ackMsg, senderProfile);
            }
        }
        else
        {
            uint16_t& curSeq = senderProfile->mIncomingUnreliableSeq;

            // If the received seq is less than the current seq, ignore the packet.
            if (SeqNumLess(seq, curSeq))
            {
                //LogDebug("Ignoring out of sequence unreliable packet");
                processMsg = false;
            }
            else
            {
                processMsg = true;
                curSeq = seq + 1;
            }
        }

        if (processMsg)
        {
            ProcessMessages(sender, stream);

            if (reliable)
            {
                // Send back the Ack
                NetMsgAck ackMsg;
                ackMsg.mSequenceNumber = seq;
                SendMessage(&ackMsg, senderProfile);

                // Process pending reliable packets first before processing any more messages.
                ProcessPendingReliablePackets(senderProfile);
            }
        }

        mBytesReceived += bytes;

#if DEBUG_MSG_STATS
        sNumPacketsReceived++;
#endif
    }
}

void NetworkManager::ProcessMessages(NetHost sender, Stream& stream)
{
    // Handle multiple messages in the same UDP packet.
    while (stream.GetPos() < stream.GetSize())
    {
        NetMsgType msgType = (NetMsgType)stream.GetData()[stream.GetPos()];
        bool msgHandled = false;

        switch (msgType)
        {
            NET_MSG_CASE(Connect)
            NET_MSG_CASE(Accept)
            NET_MSG_CASE(Reject)
            NET_MSG_CASE(Disconnect)
            NET_MSG_CASE(Kick)
            NET_MSG_CASE(LoadLevel)
            NET_MSG_CASE(Ready)
            NET_MSG_CASE(Ping)
            NET_MSG_CASE(SpawnActor)
            NET_MSG_CASE(SpawnBlueprint)
            NET_MSG_CASE(DestroyActor)
            NET_MSG_STATIC_CASE(Replicate)
            NET_MSG_STATIC_CASE(ReplicateScript)
            NET_MSG_STATIC_CASE(Invoke)
            NET_MSG_STATIC_CASE(InvokeScript)
            //NET_MSG_CASE(Broadcast)
            NET_MSG_CASE(Ack)

        default: break;
        }

        if (!msgHandled)
        {
            LogWarning("Unknown message received: %u", (uint32_t)msgType);
        }
    }
}

void NetworkManager::ProcessPendingReliablePackets(NetHostProfile* profile)
{
    std::vector<ReliablePacket>& packets = profile->mIncomingPackets;
    bool processedPacket = true;
    while (processedPacket)
    {
        processedPacket = false;

        for (uint32_t i = 0; i < packets.size(); ++i)
        {
            if (packets[i].mSeq == profile->mIncomingReliableSeq)
            {
                Stream stream(packets[i].mData.data(), (uint32_t)packets[i].mData.size());
                ProcessMessages(profile->mHost, stream);

                profile->mIncomingReliableSeq++;
                packets.erase(packets.begin() + i);
                processedPacket = true;
                break;
            }
        }
    }
}

NetHostId NetworkManager::FindAvailableNetHostId()
{
    uint8_t id = INVALID_HOST_ID;

    for (id = SERVER_HOST_ID + 1; id < 255; ++id)
    {
        bool conflict = false;

        for (uint32_t c = 0; c < mClients.size(); ++c)
        {
            if (mClients[c].mHost.mId == id)
            {
                conflict = true;
                break;
            }
        }

        if (!conflict)
        {
            break;
        }
    }

    OCT_ASSERT(id < 255);
    return id;
}

void NetworkManager::ResetToLocalStatus()
{
    if (mNetStatus != NetStatus::Local)
    {
        NET_SocketClose(mSocket);
        mSocket = NET_INVALID_SOCKET;
        mNetStatus = NetStatus::Local;
        mHostId = INVALID_HOST_ID;
        mServer.mHost.mIpAddress = 0;
        mServer.mHost.mPort = 0;
        mServer.mHost.mId = INVALID_HOST_ID;
        mServer.mTimeSinceLastMsg = 0.0f;
    }
}

void NetworkManager::BroadcastSession()
{
    NetMsgBroadcast bcMsg;
    bcMsg.mMagic = NetMsgBroadcast::sMagicNumber;
    bcMsg.mGameCode = GetEngineState()->mGameCode;
    bcMsg.mVersion = GetEngineState()->mVersion;

    // For now, use project name for session name.
    const char* sessionName = GetEngineState()->mProjectName.c_str();
    if (sessionName != nullptr)
    {
        strncpy(bcMsg.mName, sessionName, OCT_SESSION_NAME_LEN);
        bcMsg.mName[OCT_SESSION_NAME_LEN] = 0;
    }

    bcMsg.mMaxPlayers = 1 + mMaxClients;
    bcMsg.mNumPlayers = 1 + uint8_t(mClients.size());

    SendMessageImmediate(&bcMsg, mBroadcastIp, OCT_BROADCAST_PORT);
}

void NetworkManager::FlushSendBuffers(NetHostProfile* hostProfile)
{
    FlushSendBuffer(hostProfile, false);
    FlushSendBuffer(hostProfile, true);
}

void NetworkManager::FlushSendBuffer(NetHostProfile* hostProfile, bool reliable)
{
    std::vector<char>& sendBuffer = reliable ? hostProfile->mReliableSendBuffer : hostProfile->mSendBuffer;
    uint16_t& outgoingSeq = reliable ? hostProfile->mOutgoingReliableSeq : hostProfile->mOutgoingUnreliableSeq;

    if (sendBuffer.size() > 0)
    {
        // Stackoverflow says that 508 is the maximum safe udp payload.
        // Currently the max msg size is 500 for extra safety and to leave 2 bytes for the sequence number.
        OCT_ASSERT(sendBuffer.size() <= OCT_MAX_MSG_SIZE);

        if (sendBuffer.size() <= OCT_MAX_MSG_SIZE)
        {
            Stream stream(sSendBuffer, OCT_SEND_BUFFER_SIZE);
            static_assert(OCT_SEQ_NUM_SIZE == sizeof(uint16_t), "Seq num size mismatch");
            stream.WriteUint16(outgoingSeq);
            stream.WriteBool(reliable);
            stream.WriteBytes((uint8_t*)sendBuffer.data(), (uint32_t)sendBuffer.size());
            uint32_t packetSize = stream.GetPos();
            OCT_ASSERT(packetSize == OCT_PACKET_HEADER_SIZE + uint32_t(sendBuffer.size()));

            // If the client isn't ready yet, then don't send the message.
            // Reliable messages will still be queued and sent once the client is ready.
            if (hostProfile->mReady)
            {
#if DEBUG_NETWORK_CONDITIONS
                mBytesSent += DebugSendTo(
                    mSocket,
                    hostProfile->mHost.mIpAddress,
                    hostProfile->mHost.mPort,
                    packetSize,
                    sSendBuffer);
#else
                mBytesSent += NET_SocketSendTo(
                    mSocket,
                    sSendBuffer,
                    packetSize,
                    hostProfile->mHost.mIpAddress,
                    hostProfile->mHost.mPort);
#endif

#if DEBUG_MSG_STATS
                sNumPacketsSent++;
#endif
            }

            if (reliable)
            {
                hostProfile->mOutgoingPackets.emplace_back(outgoingSeq, sSendBuffer, packetSize);
            }

            outgoingSeq++;
        }
        else
        {
            LogError("Send buffer overflow");
            OCT_ASSERT(0);
        }

        sendBuffer.clear();
    }
}

void NetworkManager::UpdateReliablePackets(float deltaTime)
{
    if (mNetStatus == NetStatus::Server)
    {
        for (int32_t i = 0; i < int32_t(mClients.size()); ++i)
        {
            if (!UpdateReliablePackets(&mClients[i], deltaTime))
            {
                LogWarning("Kicking client because of undeliverable reliable message.");
                Kick(mClients[i].mHost.mId, NetMsgKick::Reason::Timeout);
                --i;
            }
        }
    }
    else if (mNetStatus == NetStatus::Client ||
             mNetStatus == NetStatus::Connecting)
    {
        if (!UpdateReliablePackets(&mServer, deltaTime))
        {
            LogWarning("Disconnecting from server because of undeliverable reliable message.");
            Disconnect();
        }
    }
}

bool NetworkManager::UpdateReliablePackets(NetHostProfile* profile, float deltaTime)
{
    bool retSuccess = true;

    if (profile != nullptr &&
        profile->mReady)
    {
        std::vector<ReliablePacket>& packets = profile->mOutgoingPackets;

        for (uint32_t i = 0; i < packets.size(); ++i)
        {
            packets[i].mTimeSinceSend += deltaTime;

            if (packets[i].mTimeSinceSend >= sReliableResendTime)
            {
                ResendPacket(profile, packets[i]);

                if (packets[i].mNumSends > sMaxReliableResends)
                {
                    retSuccess = false;
                    break;
                }
            }
        }
    }

    return retSuccess;
}

void NetworkManager::ResetHostProfile(NetHostProfile* profile)
{
    *profile = NetHostProfile();
}

bool NetworkManager::HostProfileHasIncomingPacket(NetHostProfile* profile, uint16_t seq)
{
    bool hasPacket = false;
    std::vector<ReliablePacket>& packets = profile->mIncomingPackets;

    for (uint32_t i = 0; i < packets.size(); ++i)
    {
        if (packets[i].mSeq == seq)
        {
            hasPacket = true;
            break;
        }
    }

    return hasPacket;
}

bool NetworkManager::SeqNumLess(uint16_t s1, uint16_t s2)
{
    // https://datatracker.ietf.org/doc/html/rfc1982
    static_assert(OCT_SEQ_NUM_SIZE == sizeof(uint16_t), "Need to adjust limit constant");
    const int32_t limit = 32768;

    int32_t i1 = int32_t(s1);
    int32_t i2 = int32_t(s2);

    return
        ((i1 < i2) && (i2 - i1 < limit)) ||
        ((i1 > i2) && (i1 - i2 > limit));
}
