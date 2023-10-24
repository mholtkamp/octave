#include "NetMsg.h"
#include "Log.h"
#include "Engine.h"
#include "NetworkManager.h"
#include "AssetManager.h"
#include "Assets/Scene.h"
#include "World.h"
#include "Script.h"

void NetSafeStringWrite(Stream& stream, const std::string& string)
{
    // TODO:
    // Ensure that we don't exceed our safe message size.
    // Truncate the string if needed
}

void NetMsg::Read(Stream& stream)
{
    NetMsgType type = (NetMsgType)stream.ReadUint8();

    if (GetType() != type)
    {
        LogError("NetMsg struct/type mismatch");
        OCT_ASSERT(0);
    }
}

void NetMsg::Write(Stream& stream) const
{
    stream.WriteUint8((uint8_t) GetType());
}

void NetMsg::Execute(NetHost sender)
{
    // Does nothing for now.
}

bool NetMsg::IsReliable() const
{
    return false;
}

void NetMsgConnect::Read(Stream& stream)
{
    NetMsg::Read(stream);
    mGameCode = stream.ReadUint32();
    mVersion = stream.ReadUint32();
}

void NetMsgConnect::Write(Stream& stream) const
{
    NetMsg::Write(stream);
    stream.WriteUint32(mGameCode);
    stream.WriteUint32(mVersion);
}

void NetMsgConnect::Execute(NetHost sender)
{
    NetMsg::Execute(sender);
    NetworkManager::Get()->HandleConnect(sender, mGameCode, mVersion);
}

void NetMsgAccept::Read(Stream& stream)
{
    NetMsg::Read(stream);
    mAssignedHostId = (NetHostId) stream.ReadUint8();
}

void NetMsgAccept::Write(Stream& stream) const
{
    NetMsg::Write(stream);
    stream.WriteUint8((uint8_t) mAssignedHostId);
}

void NetMsgAccept::Execute(NetHost sender)
{
    NetMsg::Execute(sender);
    NetworkManager::Get()->HandleAccept(mAssignedHostId);
}

void NetMsgReject::Read(Stream& stream)
{
    NetMsg::Read(stream);
    mReason = (Reason) stream.ReadUint8();
}

void NetMsgReject::Write(Stream& stream) const
{
    NetMsg::Write(stream);
    stream.WriteUint8((uint8_t) mReason);
}

void NetMsgReject::Execute(NetHost sender)
{
    NetMsg::Execute(sender);
    NetworkManager::Get()->HandleReject(mReason);
}

void NetMsgDisconnect::Read(Stream& stream)
{
    NetMsg::Read(stream);
}

void NetMsgDisconnect::Write(Stream& stream) const
{
    NetMsg::Write(stream);
}

void NetMsgDisconnect::Execute(NetHost sender)
{
    NetMsg::Execute(sender);
    NetworkManager::Get()->HandleDisconnect(sender);
}

void NetMsgKick::Read(Stream& stream)
{
    NetMsg::Read(stream);
    mReason = (Reason) stream.ReadUint8();
}

void NetMsgKick::Write(Stream& stream) const
{
    NetMsg::Write(stream);
    stream.WriteUint8((uint8_t) mReason);
}

void NetMsgKick::Execute(NetHost sender)
{
    NetMsg::Execute(sender);
    NetworkManager::Get()->HandleKick(mReason);
}

void NetMsgReady::Read(Stream& stream)
{
    NetMsg::Read(stream);
}

void NetMsgReady::Write(Stream& stream) const
{
    NetMsg::Write(stream);
}

void NetMsgReady::Execute(NetHost sender)
{
    NetMsg::Execute(sender);
    NetworkManager::Get()->HandleReady(sender);
}

void NetMsgPing::Read(Stream& stream)
{
    NetMsg::Read(stream);
}

void NetMsgPing::Write(Stream& stream) const
{
    NetMsg::Write(stream);
}

void NetMsgPing::Execute(NetHost sender)
{
    NetMsg::Execute(sender);
}

void NetMsgSpawn::Read(Stream& stream)
{
    NetMsg::Read(stream);
    mNodeTypeId = stream.ReadUint32();
    mNetId = stream.ReadUint32();
    mParentNetId = stream.ReadUint32();
    stream.ReadString(mSceneName);
}

void NetMsgSpawn::Write(Stream& stream) const
{
    NetMsg::Write(stream);
    stream.WriteUint32(mNodeTypeId);
    stream.WriteUint32(mNetId);
    stream.WriteUint32(mParentNetId);
    stream.WriteString(mSceneName);
}

void NetMsgSpawn::Execute(NetHost sender)
{
    NetMsg::Execute(sender);

    if (NetIsClient())
    {
        Node* newNode = nullptr;
        
        if (mSceneName != "")
        {
            // Spawning a scene
            Scene* scene = LoadAsset<Scene>(mSceneName);

            if (scene != nullptr)
            {
                newNode = scene->Instantiate();
            }
            else
            {
                LogError("Failed to load scene %s in NetMsgSpawnNode.", mSceneName.c_str());
            }
        }
        else
        {
            // Spawning a single native node
            newNode = Node::Construct(mNodeTypeId);

            if (newNode == nullptr)
            {
                LogError("Failed to instantiate node of type %d in NetMsgSpawnNode.", (int)mNodeTypeId);
            }
        }

        if (newNode != nullptr)
        {
            NetworkManager::Get()->AddNetNode(newNode, mNetId);

            if (mParentNetId == INVALID_NET_ID)
            {
                // This is the root
                // TODO: Handle multiple worlds.
                GetWorld()->SetRootNode(newNode);
            }
            else
            {
                Node* parent = nullptr;

                parent = NetworkManager::Get()->GetNetNode(mParentNetId);

                if (parent != nullptr)
                {
                    parent->AddChild(newNode);
                }
                else
                {
                    LogError("Failed to find parent net node, attaching new net node to world root.");
                    Node* rootNode = GetWorld()->GetRootNode();
                    if (rootNode)
                    {
                        rootNode->AddChild(newNode);
                    }
                    else
                    {
                        // Hmm okay, well I guess this node will be the new world root.
                        GetWorld()->SetRootNode(newNode);
                    }
                }
            }

            // TODO-NODE: Do we want to Start() the node here??
            // If not, then the client might receive replication messages with script data and the client node's 
            // script won't be started. On the other hand, calling Start() here before all of the initial data has been
            // replicated may not initialize the node properly on the client.
            // Node::mScript isn't replicated (at least as of now), and usually you will be spawning a scene with a script...
            // So maybe... at this point, if the node has a script, call mScript->StartScript()
            // so that we can populate the netdata / netfuncs. And then later it's Start() will be called appropriately.
            
            //newNode->StartScript();
        }
    }
}

void NetMsgDestroy::Read(Stream& stream)
{
    NetMsg::Read(stream);
    mNetId = stream.ReadUint32();
}

void NetMsgDestroy::Write(Stream& stream) const
{
    NetMsg::Write(stream);
    stream.WriteUint32(mNetId);
}

void NetMsgDestroy::Execute(NetHost sender)
{
    NetMsg::Execute(sender);

    if (NetIsClient())
    {
        Node* node = NetworkManager::Get()->GetNetNode(mNetId);

        if (node != nullptr)
        {
            Node::Destruct(node);
            node = nullptr;
        }
        else
        {
            LogWarning("NetMsgDestroyActor::Execute() - Failed to find actor with netid %d", mNetId);
        }
    }
}

void NetMsgReplicate::Read(Stream& stream)
{
    NetMsg::Read(stream);
    mNodeNetId = stream.ReadUint32();
    mNumVariables = stream.ReadUint16();

    // Limiting max variables per message to 64.
    OCT_ASSERT(mNumVariables <= 64);

    mIndices.clear();
    mData.clear();

    mIndices.resize(mNumVariables);
    mData.resize(mNumVariables);

    for (uint32_t i = 0; i < mNumVariables; ++i)
    {
        mIndices[i] = stream.ReadUint16();

        // We are assuming the stream data will persist for the duration
        // of this message's life cycle (until Execute() is called).
        // This avoids allocation and copying data.
        mData[i].ReadStream(stream, false);
    }
}

void NetMsgReplicate::Write(Stream& stream) const
{
    NetMsg::Write(stream);
    stream.WriteUint32(mNodeNetId);
    stream.WriteUint16(mNumVariables);

    OCT_ASSERT(mIndices.size() == mNumVariables);
    OCT_ASSERT(mData.size() == mNumVariables);

    for (uint32_t i = 0; i < mNumVariables; ++i)
    {
        stream.WriteUint16(mIndices[i]);
        mData[i].WriteStream(stream);
    }

    // Multiple replicate messages will need to be send for an actor
    // if it exceeds the message size limit.
    OCT_ASSERT(stream.GetPos() < OCT_MAX_MSG_BODY_SIZE);
}

void NetMsgReplicate::Execute(NetHost sender)
{
    NetMsg::Execute(sender);

    Node* node = NetworkManager::Get()->GetNetNode(mNodeNetId);

    if (node != nullptr)
    {
        std::vector<NetDatum>& repData = node->GetReplicatedData();

        for (uint32_t i = 0; i < mNumVariables; ++i)
        {
            uint16_t dstIndex = mIndices[i];
            OCT_ASSERT(dstIndex < repData.size());

            if (dstIndex < repData.size())
            {
                // Native net datum
                OCT_ASSERT(repData[dstIndex].mType == mData[i].mType);
                OCT_ASSERT(repData[dstIndex].mCount == mData[i].mCount);

                if (repData[dstIndex] != mData[i])
                {
                    repData[dstIndex].SetValue(mData[i].mData.vp, 0, repData[dstIndex].mCount);
                }
            }
            else
            {
                OCT_ASSERT(0);
                LogError("Replicated index out of range.");
            }
        }
    }
    else
    {
        LogWarning("Repicate message received for unknown netid %08x.", mNodeNetId);
    }
}

bool NetMsgReplicate::IsReliable() const
{
    return mReliable;
}

void NetMsgReplicateScript::Read(Stream& stream)
{
    NetMsgReplicate::Read(stream);
}

void NetMsgReplicateScript::Write(Stream& stream) const
{
    NetMsgReplicate::Write(stream);
}

void NetMsgReplicateScript::Execute(NetHost sender)
{
    NetMsg::Execute(sender);

    // Override NetMsgReplicate

    Node* node = NetworkManager::Get()->GetNetNode(mNodeNetId);

    if (node != nullptr)
    {
        Script* script = node->GetScript();

        if (script != nullptr)
        {
            std::vector<ScriptNetDatum>& repData = script->GetReplicatedData();

            for (uint32_t i = 0; i < mNumVariables; ++i)
            {
                uint16_t dstIndex = mIndices[i];
                OCT_ASSERT(dstIndex < repData.size());

                if (dstIndex < repData.size())
                {
                    OCT_ASSERT(repData[dstIndex].mType == mData[i].mType);
                    OCT_ASSERT(repData[dstIndex].mCount == mData[i].mCount);

                    if (repData[dstIndex] != mData[i])
                    {
                        repData[dstIndex].SetValue(mData[i].mData.vp, 0, repData[dstIndex].mCount);
                    }
                }
                else
                {
                    OCT_ASSERT(0);
                    LogError("Replicated index out of range.");
                }
            }
        }
        else
        {
            LogWarning("ReplicateScript message received for unregistered script on %s.", node->GetName().c_str());
        }
    }
    else
    {
        LogWarning("ReplicateScript message received for unknown netid %08x.", mNodeNetId);
    }
}

void NetMsgInvoke::Read(Stream& stream)
{
    NetMsg::Read(stream);

    mNodeNetId = stream.ReadUint32();
    mIndex = stream.ReadUint16();
    mNumParams = stream.ReadUint8();

    mParams.clear();
    mParams.resize(mNumParams);

    for (uint32_t i = 0; i < mNumParams; ++i)
    {
        mParams[i].ReadStream(stream, false);
    }
}

void NetMsgInvoke::Write(Stream& stream) const
{
    NetMsg::Write(stream);
    stream.WriteUint32(mNodeNetId);
    stream.WriteUint16(mIndex);
    stream.WriteUint8(mNumParams);

    OCT_ASSERT(mParams.size() == mNumParams);

    for (uint32_t i = 0; i < mNumParams; ++i)
    {
        mParams[i].WriteStream(stream);
    }

    OCT_ASSERT(stream.GetPos() < OCT_MAX_MSG_BODY_SIZE);
}

void NetMsgInvoke::Execute(NetHost sender)
{
    NetMsg::Execute(sender);

    Node* node = NetworkManager::Get()->GetNetNode(mNodeNetId);

    if (node != nullptr)
    {
        NetFunc* netFunc = node->FindNetFunc(mIndex);

        if (netFunc != nullptr)
        {
            switch (mNumParams)
            {
                case 0: netFunc->mFuncPointer.p0(node); break;
                case 1: netFunc->mFuncPointer.p1(node, mParams[0]); break;
                case 2: netFunc->mFuncPointer.p2(node, mParams[0], mParams[1]); break;
                case 3: netFunc->mFuncPointer.p3(node, mParams[0], mParams[1], mParams[2]); break;
                case 4: netFunc->mFuncPointer.p4(node, mParams[0], mParams[1], mParams[2], mParams[3]); break;
                case 5: netFunc->mFuncPointer.p5(node, mParams[0], mParams[1], mParams[2], mParams[3], mParams[4]); break;
                case 6: netFunc->mFuncPointer.p6(node, mParams[0], mParams[1], mParams[2], mParams[3], mParams[4], mParams[5]); break;
                case 7: netFunc->mFuncPointer.p7(node, mParams[0], mParams[1], mParams[2], mParams[3], mParams[4], mParams[5], mParams[6]); break;
                case 8: netFunc->mFuncPointer.p8(node, mParams[0], mParams[1], mParams[2], mParams[3], mParams[4], mParams[5], mParams[6], mParams[7]); break;
                default: LogWarning("Max param count exceeded in NetMsgInvoke::Execute()"); break;
            }
        }
        else
        {
            LogWarning("Unknown net func index %u received for class %s", mIndex, node->GetClassName());
        }
    }
    else
    {
        LogWarning("Invoke message received for unknown netid %08x.", mNodeNetId);
    }

}

bool NetMsgInvoke::IsReliable() const
{
    return mReliable;
}

void NetMsgInvokeScript::Read(Stream& stream)
{
    NetMsgInvoke::Read(stream);
}

void NetMsgInvokeScript::Write(Stream& stream) const
{
    NetMsgInvoke::Write(stream);
}

void NetMsgInvokeScript::Execute(NetHost sender)
{
    NetMsg::Execute(sender);
    
    // Override NetMsgInvoke

    Node* node = NetworkManager::Get()->GetNetNode(mNodeNetId);

    if (node != nullptr)
    {
        Script* script = node->GetScript();

        if (script != nullptr)
        {
            // Convert the vector to a "const Datum**" param
            const Datum* params[OCT_NET_FUNC_MAX_PARAMS] = {};
            OCT_ASSERT(mNumParams <= OCT_NET_FUNC_MAX_PARAMS);
            for (uint32_t i = 0; i < mNumParams; ++i)
            {
                params[i] = &(mParams[i]);
            }

            script->ExecuteNetFunc(mIndex, mNumParams, params);
        }
        else
        {
            LogWarning("No script is on %s in NetMsgInvokeScript::Execture().", node->GetName().c_str());
        }
    }
    else
    {
        LogWarning("Invoke script message received for unknown netid %08x.", mNodeNetId);
    }
}

const uint32_t NetMsgBroadcast::sMagicNumber = 0x4f435421;

void NetMsgBroadcast::Read(Stream& stream)
{
    NetMsg::Read(stream);

    // Do an extra safety check since we can accept broadcast type messages from any sender.
    int32_t remainingSize = int32_t(stream.GetSize()) - int32_t(stream.GetPos());

    if (remainingSize == 
            sizeof(mMagic) + 
            sizeof(mGameCode) + 
            sizeof(mVersion) + 
            (OCT_SESSION_NAME_LEN + 1) + 
            sizeof(mMaxPlayers) + 
            sizeof(mNumPlayers))
    {
        mMagic = stream.ReadUint32();
        mGameCode = stream.ReadUint32();
        mVersion = stream.ReadUint32();

        stream.ReadBytes((uint8_t*)mName, OCT_SESSION_NAME_LEN + 1);
        mMaxPlayers = stream.ReadUint8();
        mNumPlayers = stream.ReadUint8();
    }
    else
    {
        LogWarning("Invalid broadcast message.");

        mMagic = 0;
        mGameCode = 0;
        mVersion = 0;
    }
}

void NetMsgBroadcast::Write(Stream& stream) const
{
    NetMsg::Write(stream);
    stream.WriteUint32(sMagicNumber);
    stream.WriteUint32(mGameCode);
    stream.WriteUint32(mVersion);

    stream.WriteBytes((uint8_t*)mName, OCT_SESSION_NAME_LEN + 1);
    stream.WriteUint8(mMaxPlayers);
    stream.WriteUint8(mNumPlayers);
}

void NetMsgBroadcast::Execute(NetHost sender)
{
    NetMsg::Execute(sender);

    if (mMagic == sMagicNumber)
    {
        NetworkManager::Get()->HandleBroadcast(sender, mGameCode, mVersion, mName, mMaxPlayers, mNumPlayers);
    }
}

void NetMsgAck::Read(Stream& stream)
{
    NetMsg::Read(stream);
    mSequenceNumber = stream.ReadUint16();
}

void NetMsgAck::Write(Stream& stream) const
{
    NetMsg::Write(stream);
    stream.WriteUint16(mSequenceNumber);
}

void NetMsgAck::Execute(NetHost sender)
{
    NetMsg::Execute(sender);
    NetworkManager::Get()->HandleAck(sender, mSequenceNumber);
}
