#include "NetworkManager.h"
#include "Engine.h"
#include "Log.h"
#include "Nodes/Node.h"
#include "Assets/Scene.h"
#include "World.h"
#include "Profiler.h"
#include "Maths.h"
#include "Script.h"

#include "LuaBindings/Network_Lua.h"

#include "Network/NetPlatformEpic.h"
#include "Network/NetPlatformSteam.h"

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
    uint32_t mSize = 0;
    char mData[OCT_MAX_MSG_BODY_SIZE] = {};
    NetHost mHost;
};

static std::vector<DelayedPacket> sDelayedPackets;

void DebugSendTo(const NetHost& host, uint32_t size, const char* data)
{
    float dropRoll = Maths::RandRange(0.0f, 1.0f);

    if (dropRoll >= sPacketLossFrac)
    {
        sDelayedPackets.emplace_back();
        DelayedPacket& packet = sDelayedPackets.back();

        packet.mTime = (1/1000.0f) * (sLatencyMs + Maths::RandRange(-sJitterMs, sJitterMs));
        packet.mHost = host;
        packet.mSize = size;
        memcpy(packet.mData, data, size);
    }
    else
    {
        //LogWarning("Dropping packet");
    }
}

void UpdateDebugPackets(float deltaTime)
{
    for (int32_t i = 0; i < int32_t(sDelayedPackets.size()); ++i)
    {
        sDelayedPackets[i].mTime -= deltaTime;

        if (sDelayedPackets[i].mTime <= 0.0f)
        {
            NetworkManager::Get()->SendTo(
                sDelayedPackets[i].mHost,
                sDelayedPackets[i].mData,
                sDelayedPackets[i].mSize);

            sDelayedPackets.erase(sDelayedPackets.begin() + i);
            --i;
        }
    }
}

#endif

// Avoid dynamic allocations when appropriate. Reuse static messages.
static NetMsgReplicate sMsgReplicate;
static NetMsgReplicateScript sMsgReplicateScript;

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
#if NET_PLATFORM_STEAM
    mOnlinePlatform = new NetPlatformSteam();
#elif NET_PLATFORM_EPIC
    mOnlinePlatform = new NetPlatformEpic();
#endif

    if (mOnlinePlatform)
    {
        bool success = mOnlinePlatform->Create();

        if (!success)
        {
            mOnlinePlatform->Destroy();
            delete mOnlinePlatform;
            mOnlinePlatform = nullptr;
        }
    }
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

    if (mOnlinePlatform)
    {
        mOnlinePlatform->Destroy();
        delete mOnlinePlatform;
        mOnlinePlatform = nullptr;
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

    if (mOnlinePlatform)
    {
        mOnlinePlatform->Update();
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

void NetworkManager::Login()
{
    if (mOnlinePlatform)
    {
        mOnlinePlatform->Login();
    }
}

void NetworkManager::Logout()
{
    if (mOnlinePlatform)
    {
        mOnlinePlatform->Logout();
    }
}

bool NetworkManager::IsLoggedIn() const
{
    bool loggedIn = mOnlinePlatform ? mOnlinePlatform->IsLoggedIn() : false;
    return loggedIn;
}

void NetworkManager::OpenSession(const NetSessionOpenOptions& options)
{
    if (!NET_IsActive())
        return;

    if (mNetStatus == NetStatus::Local)
    {
        bool sessionOpened = false;

        if (!options.mLan && mOnlinePlatform != nullptr)
        {
            mOnlinePlatform->OpenSession(options);
            mInOnlineSession = true;
            sessionOpened = true;
        }
        else
        {
            mSocket = NET_SocketCreate();
            NET_SocketSetBlocking(mSocket, false);
            // Broadcast is used for LAN game discovery.
            NET_SocketSetBroadcast(mSocket, true);

            if (mSocket >= 0)
            {
                NET_SocketBind(mSocket, NET_ANY_IP, options.mPort);

                // Broadcasting using subnet mask wasn't working on android
                // (Probably because the subnet mask was incorrect)
                // Need to check this on other platforms, but 255.255.255.255 works fine?
#if PLATFORM_ANDROID
                mBroadcastIp = NET_IpStringToUint32("255.255.255.255");
#else
                // Determine the broadcast IP based on the subnet mask.
                uint32_t subnetMask = NET_GetSubnetMask();
                uint32_t localIp = NET_GetIpAddress();
                uint32_t netIp = localIp & subnetMask;
                mBroadcastIp = netIp | (~subnetMask);
#endif

                LogDebug("Broadcast IP: %08x", mBroadcastIp);
                sessionOpened = true;
            }
            else
            {
                LogError("Failed to create socket.");
            }
        }

        if (sessionOpened)
        {
            mNetStatus = NetStatus::Server;
            mHostId = SERVER_HOST_ID;
            LogDebug("Session opened.");

            mSessionName = options.mName;
            mMaxClients = (uint32_t)glm::clamp<int32_t>(options.mMaxPlayers - 1, 0, 256);

            // Iterate through world and assign Net IDs
            // TODO: Support multiple worlds
            Node* rootNode = GetWorld(0)->GetRootNode();
            if (rootNode)
            {
                rootNode->Traverse([](Node* node) -> bool
                    {
                        NetworkManager::Get()->AddNetNode(node, INVALID_NET_ID);
                        return true;
                    });
            }
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

        ResetToLocalStatus();
    }
    else if (mNetStatus == NetStatus::Client ||
             mNetStatus == NetStatus::Connecting)
    {
        Disconnect();
    }
    else
    {
        LogWarning("NetworkManager::CloseSession() called but there is no active session.");
    }

    if (mOnlinePlatform != nullptr)
    {
        mOnlinePlatform->CloseSession();
    }
}

void NetworkManager::JoinSession(const NetSession& session)
{
    if (mNetStatus == NetStatus::Local)
    {
        if (session.mLan)
        {
            Connect(session.mHost.mIpAddress, session.mHost.mPort);
            mInOnlineSession = false;
        }
        else if (mOnlinePlatform)
        {
            mOnlinePlatform->JoinSession(session);
            mInOnlineSession = true;
        }
    }
}

const std::string& NetworkManager::GetSessionName() const
{
    return mSessionName;
}

void NetworkManager::EnableSessionBroadcast(bool enable)
{
    mEnableSessionBroadcast = enable;
}

bool NetworkManager::IsSessionBroadcastEnabled() const
{
    return mEnableSessionBroadcast;
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
    NET_SocketSetBroadcast(mSearchSocket, true);

    if (mSearchSocket >= 0)
    {
        NET_SocketBind(mSearchSocket, NET_ANY_IP, OCT_BROADCAST_PORT);
        LogDebug("Searching for sessions on network...");
    }
    else
    {
        LogError("Failed to create search socket.");
    }

    if (mOnlinePlatform)
    {
        mOnlinePlatform->BeginSessionSearch();
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

    if (mOnlinePlatform)
    {
        mOnlinePlatform->EndSessionSearch();
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

    if (mOnlinePlatform)
    {
        mOnlinePlatform->UpdateSearch();
    }
}

bool NetworkManager::IsSearching() const
{
    return mSearching;
}

const std::vector<NetSession>& NetworkManager::GetSessions() const
{
    static std::vector<NetSession> sSessions;

    // Start with LAN sessions
    sSessions = mSessions;

    // Add Online sessions
    if (mOnlinePlatform)
    {
        const std::vector<NetSession>& onlineSessions = mOnlinePlatform->GetSessions();

        for (uint32_t i = 0; i < onlineSessions.size(); ++i)
        {
            sSessions.push_back(onlineSessions[i]);
        }
    }

    return sSessions;
}

void NetworkManager::Connect(const char* ipAddress, uint16_t port)
{
    uint32_t ipAddrInt = NET_IpStringToUint32(ipAddress);
    Connect(ipAddrInt, port);
}

void NetworkManager::Connect(uint32_t ipAddress, uint16_t port)
{
    NetHost host;
    host.mIpAddress = ipAddress;
    host.mPort = port;
    host.mId = SERVER_HOST_ID;

    Connect(host);
}

void NetworkManager::Connect(const NetHost& host)
{
    if (!NET_IsActive())
        return;

    if (mNetStatus == NetStatus::Local)
    {
        LogDebug("Connecting to session...");
        mNetStatus = NetStatus::Connecting;
        mConnectTimer = 0.0f;
        ResetHostProfile(&mServer);
        mServer.mHost.mIpAddress = host.mIpAddress;
        mServer.mHost.mPort = host.mPort;
        mServer.mHost.mId = SERVER_HOST_ID;

        NetMsgConnect connectMsg;
        connectMsg.mGameCode = GetEngineState()->mGameCode;
        connectMsg.mVersion = GetEngineState()->mVersion;

        if (host.mOnlineId != 0 && mOnlinePlatform)
        {
            mServer.mHost.mOnlineId = host.mOnlineId;
            NetworkManager::SendMessage(&connectMsg, &mServer);
        }
        else
        {
            mSocket = NET_SocketCreate();
            NET_SocketSetBlocking(mSocket, false);

            if (mSocket >= 0)
            {
                NetworkManager::SendMessage(&connectMsg, &mServer);
            }
            else
            {
                LogError("Failed to create socket.");
                ResetToLocalStatus();
            }
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
    else if (mNetStatus == NetStatus::Server)
    {
        CloseSession();
    }
    else
    {
        LogWarning("NetworkManager::Disconnect() called but not connected to a session.");
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
            if (mDisconnectCallback.mScriptFunc.IsValid())
            {
                Datum clientTable;
                WriteNetHostProfile(mClients[i], clientTable);
                mDisconnectCallback.mScriptFunc.Call(1, &clientTable);
            }

            LogDebug("Kicking client %08x:%u", mClients[i].mHost.mIpAddress, mClients[i].mHost.mPort);
            mClients.erase(mClients.begin() + i);
            break;
        }
    }
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
        char msgData[OCT_MAX_MSG_BODY_SIZE] = {};
        Stream stream(msgData, OCT_MAX_MSG_BODY_SIZE);
        netMsg->Write(stream);

        bool reliable = netMsg->IsReliable();
        std::vector<char>& sendBuffer = reliable ? hostProfile->mReliableSendBuffer : hostProfile->mSendBuffer;

        // If this newly serialized message would cause send buffer to exceed max message size,
        // then send out the queued messages first.
        if (sendBuffer.size() + stream.GetPos() > OCT_MAX_MSG_BODY_SIZE)
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

void NetworkManager::SendMessageImmediate(const NetHost& host, const NetMsg* netMsg)
{
    // Immediate messages don't rely on a sequence number.
    // Connect and Reject should always be seq num 0 since they are the first messages sent between
    // the server and the client. Broadcast uses a totally different socket / recv function.
    uint16_t seqNum = 0;

    Stream stream(sSendBuffer, OCT_SEND_BUFFER_SIZE);
    stream.WriteUint16(seqNum);
    stream.WriteBool(false);
    netMsg->Write(stream);

    if (stream.GetPos() <= OCT_MAX_MSG_BODY_SIZE)
    {

#if DEBUG_NETWORK_CONDITIONS
        DebugSendTo(host, stream.GetPos(), stream.GetData());
#else
        SendTo(host,
               stream.GetData(),
               stream.GetPos());
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

void NetworkManager::AddNetNode(Node* node, NetId netId)
{
    OCT_ASSERT(node != nullptr);
    OCT_ASSERT(node->GetNetId() == INVALID_NET_ID);

    if (node->IsReplicated())
    {
        // Gather net functions (even if local)
        Node::RegisterNetFuncs(node);

        if (!NetIsLocal())
        {
            node->GatherReplicatedData(node->GetReplicatedData());

            if (NetIsServer() &&
                netId == INVALID_NET_ID)
            {
                netId = mNextNetId;
                ++mNextNetId;
            }

            if (netId != INVALID_NET_ID)
            {
                node->SetNetId(netId);
                mNetNodeMap.insert({ netId, node });

                // The server needs to send Spawn messages for newly added network actors.
                if (NetIsServer())
                {
                    NetworkManager::Get()->SendSpawnMessage(node, nullptr);
                }
            }
        }

    }

}

void NetworkManager::RemoveNetNode(Node* node)
{
    NetId netId = node->GetNetId();

    if (netId != INVALID_NET_ID)
    {
        // Send destroy message
        if (NetIsServer())
        {
            NetworkManager::Get()->SendDestroyMessage(node, nullptr);
        }

        // This node was assigned a net id, so it should exist in our net actor map.
        OCT_ASSERT(mNetNodeMap.find(netId) != mNetNodeMap.end());
        mNetNodeMap.erase(netId);

        node->SetNetId(INVALID_NET_ID);
    }
}

const std::unordered_map<NetId, Node*>& NetworkManager::GetNetNodeMap() const
{
    return mNetNodeMap;
}

Node* NetworkManager::GetNetNode(NetId netId)
{
    Node* retNode = nullptr;
    auto it = mNetNodeMap.find(netId);
    if (it != mNetNodeMap.end())
    {
        retNode = it->second;
    }
    return retNode;
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
            newClient->mHost.mOnlineId = host.mOnlineId;

            NetMsgAccept acceptMsg;
            acceptMsg.mAssignedHostId = newClient->mHost.mId;
            SendMessage(&acceptMsg, newClient);

            // Spawn any replicated actors.
            auto spawnNode = [&](Node* node) -> bool
            {
                if (node->IsReplicated())
                {
                    SendSpawnMessage(node, newClient);
                    return true;
                }

                // Do not spawn nodes with non-replicated parents.
                // At least I think this is the behavior we want...
                return false;
            };

            World* world = GetWorld(0);
            Node* worldRoot = world ? world->GetRootNode() : nullptr;
            if (worldRoot != nullptr)
            {
                // Make sure to traverse non-inverted because the parents need to be replicated first.
                worldRoot->Traverse(spawnNode, false);
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
            if (mConnectCallback.mScriptFunc.IsValid())
            {
                Datum clientTable;
                WriteNetHostProfile(*newClient, clientTable);

                mConnectCallback.mScriptFunc.Call(1, &clientTable);
            }
        }

        if (!accept)
        {
            // Send back a Reject message to let them know why their connection was rejected.
            NetMsgReject rejectMsg;
            rejectMsg.mReason = rejectReason;
            SendMessageImmediate(host, &rejectMsg);
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
        if (mAcceptCallback.mScriptFunc.IsValid())
        {
            mAcceptCallback.mScriptFunc.Call();
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
        if (mRejectCallback.mScriptFunc.IsValid())
        {
            Datum reasonArg = (int32_t)reason;
            mRejectCallback.mScriptFunc.Call(1, &reasonArg);
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
                    if (mDisconnectCallback.mScriptFunc.IsValid())
                    {
                        Datum clientTable;
                        WriteNetHostProfile(mClients[i], clientTable);

                        mDisconnectCallback.mScriptFunc.Call(1, &clientTable);
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
        if (mKickCallback.mScriptFunc.IsValid())
        {
            Datum reasonArg = (int32_t)reason;
            mKickCallback.mScriptFunc.Call(1, &reasonArg);
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
            auto repNode = [&](Node* node) -> bool
            {
                if (node->IsReplicated())
                {
                    ReplicateNode(node, client->mHost.mId, true, true);
                    return true;
                }

                // Do not replicate nodes with non-replicated parents.
                // At least I think this is the behavior we want...
                return false;
            };

            World* world = GetWorld(0);
            Node* worldRoot = world ? world->GetRootNode() : nullptr;
            if (worldRoot != nullptr)
            {
                // Make sure to traverse non-inverted because the parents need to be replicated first.
                worldRoot->Traverse(repNode, false);
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
        NetSession* session = nullptr;

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
            mSessions.push_back(NetSession());
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
    sizeof(NetMsgReplicate::mNodeNetId) +
    sizeof(NetMsgReplicate::mNumVariables);

static const uint32_t MaxDatumNetSerializeSize = 
    OCT_MAX_MSG_BODY_SIZE - 
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

void NetworkManager::SendInvokeMsg(NetMsgInvoke& msg, Node* node, NetFunc* func, uint32_t numParams, const Datum** params)
{
    NetFuncType type = func->mType;
    bool scriptMsg = msg.GetType() == NetMsgType::InvokeScript;
    OCT_ASSERT(scriptMsg || numParams == func->mNumParams); // Script NetFuncs do not setup num params.

    msg.mNodeNetId = node->GetNetId();
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
        OCT_ASSERT(node->GetOwningHost() != INVALID_HOST_ID);
        OCT_ASSERT(node->GetOwningHost() != SERVER_HOST_ID);
        SendMessage(&msg, node->GetOwningHost());
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

void NetworkManager::SendInvokeMsg(Node* actor, NetFunc* func, uint32_t numParams, const Datum** params)
{
    NetMsgInvoke netMsgInvoke;
    SendInvokeMsg(netMsgInvoke, actor, func, numParams, params);
}

void NetworkManager::SendInvokeScriptMsg(Script* script, ScriptNetFunc* func, uint32_t numParams, const Datum** params)
{
    NetMsgInvokeScript netMsgInvokeScript;
    Node* node = script->GetOwner();
    SendInvokeMsg(netMsgInvokeScript, node, func, numParams, params);
}

void NetworkManager::SendSpawnMessage(Node* node, NetClient* client)
{
    NetMsgSpawn spawnMsg;
    spawnMsg.mNodeTypeId = node->GetType();
    spawnMsg.mNetId = node->GetNetId();
    spawnMsg.mParentNetId = INVALID_NET_ID;
    spawnMsg.mReplicateTransform = node->IsTransformReplicated();

    Node* parent = node->GetParent();
    if (parent != nullptr)
    {
        spawnMsg.mParentNetId = parent->GetNetId();
    }

    Scene* scene = node->GetScene();
    if (scene != nullptr)
    {
        spawnMsg.mSceneName = scene->GetName();
    }

    if (client == nullptr)
    {
        SendMessageToAllClients(&spawnMsg);
    }
    else
    {
        SendMessage(&spawnMsg, client);
    }
}

void NetworkManager::SendDestroyMessage(Node* node, NetClient* client)
{
    OCT_ASSERT(node != nullptr);
    NetMsgDestroy destroyMsg;
    destroyMsg.mNetId = node->GetNetId();

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
    SendTo(hostProfile->mHost,
        packet.mData.data(),
        (uint32_t)packet.mData.size());

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

    // TODO: Handle multiple worlds. Pass world into update replication.

    Node* incRepNode = nullptr;
    World* world = GetWorld(0);

    if (mIncrementalReplication)
    {
        uint32_t& incTier = world->GetIncrementalRepTier();
        uint32_t& incIndex = world->GetIncrementalRepIndex();
        std::vector<Node*>& repVector = world->GetReplicatedNodeVector((ReplicationRate)incTier);

        if (incIndex < repVector.size())
        {
            incRepNode = repVector[incIndex];
        }

        ++incIndex;

        // If we've iterated over all these actors in this rep tier, move on to the next.
        if (incIndex > repVector.size())
        {
            incIndex = 0;
            incTier++;
            if (incTier == (uint32_t)ReplicationRate::Count)
            {
                incTier = (uint32_t)ReplicationRate::Low;
            }
        }
    }

    uint32_t numNodesReplicated = 0;

    auto replicateTier = [this, incRepNode, &numNodesReplicated](const std::vector<Node*>& repVector, uint32_t& repIndex, uint32_t count)
    {
        // Loop back around if the index is somehow past the vector size already
        if (repIndex >= repVector.size())
        {
            repIndex = 0;
        }

        for (uint32_t i = 0; i < count; ++i)
        {
            Node* node = repVector[repIndex];
            bool forceRep = (node == incRepNode);
            bool nodeReplicated = ReplicateNode(node, INVALID_HOST_ID, forceRep, false);

            if (nodeReplicated)
            {
                numNodesReplicated++;
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
        const std::vector<Node*>& repVector = world->GetReplicatedNodeVector(ReplicationRate::High);
        uint32_t& repIndex = world->GetReplicatedNodeIndex(ReplicationRate::High);
        uint32_t vectorSize = (uint32_t) repVector.size();
        uint32_t count = vectorSize / 1;
        replicateTier(repVector, repIndex, count);
    }

    // Medium Priority
    {
        const std::vector<Node*>& repVector = world->GetReplicatedNodeVector(ReplicationRate::Medium);
        uint32_t& repIndex = world->GetReplicatedNodeIndex(ReplicationRate::Medium);
        uint32_t vectorSize = (uint32_t) repVector.size();
        uint32_t count = (vectorSize + 1) / 2;
        replicateTier(repVector, repIndex, count);
    }
    // Low Priority
    {
        const std::vector<Node*>& repVector = world->GetReplicatedNodeVector(ReplicationRate::Low);
        uint32_t& repIndex = world->GetReplicatedNodeIndex(ReplicationRate::Low);
        uint32_t vectorSize = (uint32_t) repVector.size();
        uint32_t count = (vectorSize + 3) / 4;
        replicateTier(repVector, repIndex, count);
    }
}

template<typename T>
bool ReplicateData(std::vector<T>& repData, NetMsgReplicate& msg, NetId hostId, bool force, bool reliable)
{
    // msg.mNetId should already be set by caller.
    msg.mIndices.clear();
    msg.mData.clear();
    msg.mReliable = reliable;

    bool replicated = false;
    uint32_t numVars = 0;

    uint32_t msgSerializedSize = RepMsgHeaderSize;

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
            else if (msgSerializedSize + datumSerializeSize > OCT_MAX_MSG_BODY_SIZE)
            {
                // Send what we have until now
                NetworkManager::Get()->SendReplicateMsg(msg, numVars, hostId);
                msgSerializedSize = RepMsgHeaderSize;
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

bool NetworkManager::ReplicateNode(Node* node, NetId hostId, bool force, bool reliable)
{
    bool nodeReplicated = false;
    bool needsForcedRep = node->NeedsForcedReplication();
    force = (force || needsForcedRep);
    reliable = (reliable || needsForcedRep);
    sMsgReplicate.mNodeNetId = node->GetNetId();

    std::vector<NetDatum>& repData = node->GetReplicatedData();

    nodeReplicated = ReplicateData<NetDatum>(repData, sMsgReplicate, hostId, force, reliable);

    Script* script = node->GetScript();
    if (script != nullptr && script->IsActive())
    {
        sMsgReplicateScript.mNodeNetId = node->GetNetId();

        std::vector<ScriptNetDatum>& scriptRepData = script->GetReplicatedData();
        nodeReplicated = ReplicateData<ScriptNetDatum>(scriptRepData, sMsgReplicateScript, hostId, force, reliable);
    }

    node->ClearForcedReplication();

    return nodeReplicated;
}

void NetworkManager::UpdateHostConnections(float deltaTime)
{
    float clampedDeltaTime = glm::min(deltaTime, 0.333f);

    if (IsServer())
    {
        for (int32_t i = int32_t(mClients.size()) - 1; i >= 0; --i)
        {
            mClients[i].mTimeSinceLastMsg += clampedDeltaTime;

            if (mClients[i].mTimeSinceLastMsg >= mInactiveTimeout ||
                mClients[i].mOutgoingPackets.size() > sMaxOutgoingPackets)
            {
                Kick(mClients[i].mHost.mId, NetMsgKick::Reason::Timeout);
            }
        }
    }
    else if (IsClient())
    {
        mServer.mTimeSinceLastMsg += clampedDeltaTime;
        if (mServer.mTimeSinceLastMsg >= mInactiveTimeout ||
            mServer.mOutgoingPackets.size() > sMaxOutgoingPackets)
        {
            Disconnect();

            if (mKickCallback.mFuncPointer != nullptr)
            {
                mKickCallback.mFuncPointer(NetMsgKick::Reason::Timeout);
            }
            if (mKickCallback.mScriptFunc.IsValid())
            {
                Datum reasonArg = (int32_t)NetMsgKick::Reason::Timeout;
                mKickCallback.mScriptFunc.Call(1, &reasonArg);
            }
        }
    }
}

int32_t NetworkManager::RecvFrom(char* buffer, uint32_t size, NetHost& outHost)
{
    int32_t bytes = 0;

    if (mInOnlineSession && mOnlinePlatform)
    {
        bytes = mOnlinePlatform->RecvMessage(buffer, size, outHost);
    }
    else
    {
        bytes = NET_SocketRecvFrom(mSocket, buffer, size, outHost.mIpAddress, outHost.mPort);
    }

    return bytes;
}

void NetworkManager::SendTo(const NetHost& host, const char* buffer, uint32_t size)
{
    if (mInOnlineSession && mOnlinePlatform)
    {
        mOnlinePlatform->SendMessage(host, buffer, size);
        mBytesSent += size;
    }
    else
    {
        mBytesSent += NET_SocketSendTo(
            mSocket,
            buffer,
            size,
            host.mIpAddress,
            host.mPort);
    }
}

void NetworkManager::ProcessIncomingPackets(float deltaTime)
{
    int32_t bytes = 0;
    NetHost sender;

    while ((bytes = RecvFrom(sRecvBuffer, OCT_RECV_BUFFER_SIZE, sender)) > 0)
    {   
        Stream stream(sRecvBuffer, bytes);
        NetMsgType msgType = (NetMsgType) sRecvBuffer[OCT_PACKET_HEADER_SIZE];

        // Find which NetHost the message was from.
        // if there is no matching NetHost then ignore this message (unless it is a "Connect" message)
        sender.mId = INVALID_HOST_ID;

        NetHostProfile* senderProfile = nullptr;

        // Connect messages are only executed on the Server
        bool connectMsg = mNetStatus == NetStatus::Server && 
                          msgType == NetMsgType::Connect;

        if (mNetStatus == NetStatus::Server)
        {
            for (uint32_t i = 0; i < mClients.size(); ++i)
            {
                if ((mInOnlineSession && mClients[i].mHost.mOnlineId == sender.mOnlineId) || 
                    (mClients[i].mHost.mIpAddress == sender.mIpAddress &&
                    mClients[i].mHost.mPort == sender.mPort))
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
            if ((mInOnlineSession && mServer.mHost.mOnlineId == sender.mOnlineId) ||
                (mServer.mHost.mIpAddress == sender.mIpAddress &&
                mServer.mHost.mPort == sender.mPort))
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
            LogDebug("Unrecognized host: %08x:%u", sender.mIpAddress, sender.mPort);
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
            NET_MSG_CASE(Ready)
            NET_MSG_CASE(Spawn)
            NET_MSG_CASE(Destroy)
            NET_MSG_CASE(Ping)
            NET_MSG_STATIC_CASE(Replicate)
            NET_MSG_STATIC_CASE(ReplicateScript)
            NET_MSG_CASE(Invoke)
            NET_MSG_CASE(InvokeScript)
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
        if (mSocket != NET_INVALID_SOCKET)
        {
            NET_SocketClose(mSocket);
        }

        if (mOnlinePlatform)
        {
            mOnlinePlatform->CloseSession();
        }

        // Invalidate all net IDs
        // TODO: Support multiple worlds
        Node* rootNode = GetWorld(0)->GetRootNode();
        if (rootNode)
        {
            rootNode->Traverse([](Node* node) -> bool
                {
                    NetworkManager::Get()->RemoveNetNode(node);
                    return true;
                });
        }

        mSocket = NET_INVALID_SOCKET;
        mNetStatus = NetStatus::Local;
        mHostId = INVALID_HOST_ID;
        mServer = NetServer();
        mInOnlineSession = false;
    }
}

void NetworkManager::BroadcastSession()
{
    if (mEnableSessionBroadcast && !mInOnlineSession)
    {
        NetMsgBroadcast bcMsg;
        bcMsg.mMagic = NetMsgBroadcast::sMagicNumber;
        bcMsg.mGameCode = GetEngineState()->mGameCode;
        bcMsg.mVersion = GetEngineState()->mVersion;

        // For now, use project name for session name.
        strncpy(bcMsg.mName, mSessionName.c_str(), OCT_SESSION_NAME_LEN);
        bcMsg.mName[OCT_SESSION_NAME_LEN] = 0;

        bcMsg.mMaxPlayers = 1 + mMaxClients;
        bcMsg.mNumPlayers = 1 + uint8_t(mClients.size());

        NetHost host;
        host.mIpAddress = mBroadcastIp;
        host.mPort = OCT_BROADCAST_PORT;
        SendMessageImmediate(host, &bcMsg);
    }
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
        OCT_ASSERT(sendBuffer.size() <= OCT_MAX_MSG_BODY_SIZE);

        if (sendBuffer.size() <= OCT_MAX_MSG_BODY_SIZE)
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
                DebugSendTo(hostProfile->mHost,
                    packetSize,
                    sSendBuffer);
#else
                SendTo(hostProfile->mHost, sSendBuffer, packetSize);
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
