#include "NetMsg.h"
#include "Log.h"
#include "Engine.h"
#include "NetworkManager.h"
#include "AssetManager.h"
#include "Assets/Level.h"
#include "Components/ScriptComponent.h"
#include "World.h"

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

void NetMsgLoadLevel::Read(Stream& stream)
{
    NetMsg::Read(stream);
    stream.ReadString(mLevelName);
}

void NetMsgLoadLevel::Write(Stream& stream) const
{
    NetMsg::Write(stream);

    // TODO: Replace with NetSafeStringWrite() 

    if (stream.GetPos() + STREAM_STRING_LEN_BYTES + mLevelName.size() < OCT_MAX_MSG_SIZE)
    {
        stream.WriteString(mLevelName);
    }
    else
    {
        LogWarning("NetMsgLoadLevel: mLevelName too long. Truncating");
        std::string truncatedLevelName = mLevelName.substr(0, 300);
        stream.WriteString(truncatedLevelName);
    }
}

void NetMsgLoadLevel::Execute(NetHost sender)
{
    NetMsg::Execute(sender);
    
    if (NetworkManager::Get()->IsClient())
    {
        AssetStub* levelAsset = FetchAssetStub(mLevelName);

        if (levelAsset != nullptr &&
            levelAsset->mType == Level::GetStaticType())
        {
            Level* level = (Level*) LoadAsset(mLevelName);
            level->LoadIntoWorld(GetWorld());
        }
        else
        {
            LogError("NetMsgLoadLevel: Failed to find level %s", mLevelName.c_str());
        }
    }
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

void NetMsgSpawnActor::Read(Stream& stream)
{
    NetMsg::Read(stream);
    mActorTypeId = stream.ReadUint32();
    mNetId = stream.ReadUint32();
}

void NetMsgSpawnActor::Write(Stream& stream) const
{
    NetMsg::Write(stream);
    stream.WriteUint32(mActorTypeId);
    stream.WriteUint32(mNetId);
}

void NetMsgSpawnActor::Execute(NetHost sender)
{
    NetMsg::Execute(sender);

    if (NetIsClient())
    {
        Actor* spawnedActor = GetWorld()->SpawnActor(mActorTypeId);
        GetWorld()->AddNetActor(spawnedActor, mNetId);
    }
}

void NetMsgSpawnBlueprint::Read(Stream& stream)
{
    NetMsg::Read(stream);
    stream.ReadString(mBlueprintName);
    mNetId = stream.ReadUint32();
}

void NetMsgSpawnBlueprint::Write(Stream& stream) const
{
    NetMsg::Write(stream);
    stream.WriteString(mBlueprintName);
    stream.WriteUint32(mNetId);
}

void NetMsgSpawnBlueprint::Execute(NetHost sender)
{
    NetMsg::Execute(sender);

    if (NetIsClient())
    {
        Actor* spawnedActor = GetWorld()->SpawnBlueprint(mBlueprintName.c_str());
        GetWorld()->AddNetActor(spawnedActor, mNetId);
    }
}

void NetMsgDestroyActor::Read(Stream& stream)
{
    NetMsg::Read(stream);
    mNetId = stream.ReadUint32();
}

void NetMsgDestroyActor::Write(Stream& stream) const
{
    NetMsg::Write(stream);
    stream.WriteUint32(mNetId);
}

void NetMsgDestroyActor::Execute(NetHost sender)
{
    NetMsg::Execute(sender);

    if (NetIsClient())
    {
        Actor* actor = GetWorld()->FindActor(mNetId);

        if (actor != nullptr)
        {
            GetWorld()->DestroyActor(actor);
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
    mActorNetId = stream.ReadUint32();
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
    stream.WriteUint32(mActorNetId);
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
    OCT_ASSERT(stream.GetPos() < OCT_MAX_MSG_SIZE);
}

void NetMsgReplicate::Execute(NetHost sender)
{
    NetMsg::Execute(sender);

    Actor* actor = GetWorld()->FindActor(mActorNetId);

    if (actor != nullptr)
    {
        std::vector<NetDatum>& repData = actor->GetReplicatedData();

        for (uint32_t i = 0; i < mNumVariables; ++i)
        {
            uint16_t dstIndex = mIndices[i];
            OCT_ASSERT(dstIndex < repData.size());

            if (mIndices[i] < repData.size())
            {
                OCT_ASSERT(repData[dstIndex].mType == mData[i].mType);
                OCT_ASSERT(repData[dstIndex].mCount == mData[i].mCount);

                if (repData[dstIndex] != mData[i])
                {
                    repData[dstIndex].SetValue(mData[i].mData.vp, 0, repData[dstIndex].mCount);
                }
            }
        }
    }
    else
    {
        LogWarning("Repicate message received for unknown netid %08x.", mActorNetId);
    }
}

bool NetMsgReplicate::IsReliable() const
{
    return mReliable;
}

void NetMsgReplicateScript::Read(Stream& stream)
{
    NetMsgReplicate::Read(stream);
    stream.ReadString(mScriptName);
}

void NetMsgReplicateScript::Write(Stream& stream) const
{
    NetMsgReplicate::Write(stream);
    stream.WriteString(mScriptName);

    // Multiple replicate messages will need to be sent for an actor
    // if it exceeds the message size limit.
    OCT_ASSERT(stream.GetPos() < OCT_MAX_MSG_SIZE);
}

ScriptComponent* FindScriptComponent(Actor* actor, const std::string& scriptName)
{
    // Find the script component
    ScriptComponent* retComp = nullptr;
    const std::vector<Component*>& comps = actor->GetComponents();
    for (uint32_t i = 0; i < comps.size(); ++i)
    {
        if (comps[i]->GetType() == ScriptComponent::GetStaticType())
        {
            ScriptComponent* scComp = (ScriptComponent*)comps[i];
            if (scComp->GetScriptClassName() == scriptName)
            {
                retComp = scComp;
                break;
            }
        }
    }
    return retComp;
}

void NetMsgReplicateScript::Execute(NetHost sender)
{
    NetMsg::Execute(sender);

    Actor* actor = GetWorld()->FindActor(mActorNetId);

    if (actor != nullptr)
    {
        ScriptComponent* targetComp = FindScriptComponent(actor, mScriptName);

        if (targetComp != nullptr)
        {
            std::vector<ScriptNetDatum>& repData = targetComp->GetReplicatedData();

            for (uint32_t i = 0; i < mNumVariables; ++i)
            {
                uint16_t dstIndex = mIndices[i];
                OCT_ASSERT(dstIndex < repData.size());

                if (mIndices[i] < repData.size())
                {
                    OCT_ASSERT(repData[dstIndex].mType == mData[i].mType);
                    OCT_ASSERT(repData[dstIndex].mCount == mData[i].mCount);

                    if (repData[dstIndex] != mData[i])
                    {
                        repData[dstIndex].SetValue(mData[i].mData.vp, 0, repData[dstIndex].mCount);
                    }
                }
            }
        }
        else
        {
            LogWarning("RepicateScript message received for unregistered script %s.", mScriptName.c_str());
        }
    }
    else
    {
        LogWarning("RepicateScript message received for unknown netid %08x.", mActorNetId);
    }
}

void NetMsgInvoke::Read(Stream& stream)
{
    NetMsg::Read(stream);

    mActorNetId = stream.ReadUint32();
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
    stream.WriteUint32(mActorNetId);
    stream.WriteUint16(mIndex);
    stream.WriteUint8(mNumParams);

    OCT_ASSERT(mParams.size() == mNumParams);

    for (uint32_t i = 0; i < mNumParams; ++i)
    {
        mParams[i].WriteStream(stream);
    }

    OCT_ASSERT(stream.GetPos() < OCT_MAX_MSG_SIZE);
}

void NetMsgInvoke::Execute(NetHost sender)
{
    NetMsg::Execute(sender);

    Actor* actor = GetWorld()->FindActor(mActorNetId);

    if (actor != nullptr)
    {
        NetFunc* netFunc = actor->FindNetFunc(mIndex);

        if (netFunc != nullptr)
        {
            switch (mNumParams)
            {
                case 0: netFunc->mFuncPointer.p0(actor); break;
                case 1: netFunc->mFuncPointer.p1(actor, mParams[0]); break;
                case 2: netFunc->mFuncPointer.p2(actor, mParams[0], mParams[1]); break;
                case 3: netFunc->mFuncPointer.p3(actor, mParams[0], mParams[1], mParams[2]); break;
                case 4: netFunc->mFuncPointer.p4(actor, mParams[0], mParams[1], mParams[2], mParams[3]); break;
                case 5: netFunc->mFuncPointer.p5(actor, mParams[0], mParams[1], mParams[2], mParams[3], mParams[4]); break;
                case 6: netFunc->mFuncPointer.p6(actor, mParams[0], mParams[1], mParams[2], mParams[3], mParams[4], mParams[5]); break;
                case 7: netFunc->mFuncPointer.p7(actor, mParams[0], mParams[1], mParams[2], mParams[3], mParams[4], mParams[5], mParams[6]); break;
                case 8: netFunc->mFuncPointer.p8(actor, mParams[0], mParams[1], mParams[2], mParams[3], mParams[4], mParams[5], mParams[6], mParams[7]); break;
                default: LogWarning("Max param count exceeded in NetMsgInvoke::Execute()"); break;
            }
        }
        else
        {
            LogWarning("Unknown net func index %u received for class %s", mIndex, actor->GetClassName());
        }
    }
    else
    {
        LogWarning("Invoke message received for unknown netid %08x.", mActorNetId);
    }

}

bool NetMsgInvoke::IsReliable() const
{
    return mReliable;
}

void NetMsgInvokeScript::Read(Stream& stream)
{
    NetMsgInvoke::Read(stream);
    stream.ReadString(mScriptName);
}

void NetMsgInvokeScript::Write(Stream& stream) const
{
    NetMsgInvoke::Write(stream);
    stream.WriteString(mScriptName);

    OCT_ASSERT(stream.GetPos() < OCT_MAX_MSG_SIZE);
}

void NetMsgInvokeScript::Execute(NetHost sender)
{
    NetMsg::Execute(sender);

    Actor* actor = GetWorld()->FindActor(mActorNetId);

    if (actor != nullptr)
    {
        // Find Script component
        ScriptComponent* scriptComp = FindScriptComponent(actor, mScriptName);

        if (scriptComp != nullptr)
        {
            scriptComp->ExecuteNetFunc(mIndex, mNumParams, mParams);
        }
        else
        {
            LogWarning("Can't find target script %s on actor for InvokeScript msg.", mScriptName.c_str());
        }
    }
    else
    {
        LogWarning("InvokeScript message received for unknown netid %08x.", mActorNetId);
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
