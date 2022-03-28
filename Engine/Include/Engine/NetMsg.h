#pragma once

#include "Constants.h"
#include "EngineTypes.h"
#include "Stream.h"
#include "Datum.h"

#define NET_MESSAGE_MAGIC_STR "OCTM"

#define NET_MSG_INTERFACE(Name) \
    virtual void Read(Stream& stream) override; \
    virtual void Write(Stream& stream) const override; \
    virtual void Execute(NetHost sender) override; \
    virtual NetMsgType GetType() const override { return NetMsgType::Name; }

#define NET_MSG_INTERFACE_RELIABLE(Name) \
    NET_MSG_INTERFACE(Name) \
    virtual bool IsReliable() const override { return true; }

enum class NetMsgType : uint8_t
{
    Connect,
    Accept,
    Reject,
    Disconnect,
    Kick,
    LoadLevel,
    Ready,
    SpawnActor,
    SpawnBlueprint,
    DestroyActor,
    Ping,
    Replicate,
    ReplicateScript,
    Invoke,
    InvokeScript,
    Broadcast,
    Ack,

    Count
};

struct NetMsg
{
    virtual void Read(Stream& stream);
    virtual void Write(Stream& stream) const;
    virtual void Execute(NetHost sender);
    virtual bool IsReliable() const;
    virtual NetMsgType GetType() const = 0;
};

struct NetMsgConnect : public NetMsg
{
    NET_MSG_INTERFACE(Connect);

    uint32_t mGameCode = 0;
    uint32_t mVersion = 0;
};

struct NetMsgAccept : public NetMsg
{
    NET_MSG_INTERFACE_RELIABLE(Accept);

    NetHostId mAssignedHostId = INVALID_HOST_ID;
};

struct NetMsgReject : public NetMsg
{
    NET_MSG_INTERFACE(Reject);

    enum class Reason : uint8_t
    {
        InvalidGameCode,
        VersionMismatch,
        SessionFull,

        Count
    };

    Reason mReason = Reason::Count; 
};

struct NetMsgDisconnect : public NetMsg
{
    NET_MSG_INTERFACE(Disconnect);
};

struct NetMsgKick : public NetMsg
{
    NET_MSG_INTERFACE(Kick);

    enum class Reason : uint8_t
    {
        SessionClose,
        Timeout,
        Forced,

        Count
    };

    Reason mReason = Reason::Count;
};

struct NetMsgLoadLevel : public NetMsg
{
    NET_MSG_INTERFACE_RELIABLE(LoadLevel);

    std::string mLevelName;
};

struct NetMsgReady : public NetMsg
{
    NET_MSG_INTERFACE_RELIABLE(Ready);
};

struct NetMsgPing : public NetMsg
{
    NET_MSG_INTERFACE(Ping);
};

struct NetMsgSpawnActor : public NetMsg
{
    NET_MSG_INTERFACE_RELIABLE(SpawnActor);

    TypeId mActorTypeId = INVALID_TYPE_ID;
    NetId mNetId = INVALID_NET_ID;
};

struct NetMsgSpawnBlueprint : public NetMsg
{
    NET_MSG_INTERFACE_RELIABLE(SpawnBlueprint);

    std::string mBlueprintName;
    NetId mNetId = INVALID_NET_ID;
};

struct NetMsgDestroyActor : public NetMsg
{
    NET_MSG_INTERFACE_RELIABLE(DestroyActor);

    NetId mNetId = INVALID_NET_ID;
};

struct NetMsgReplicate : public NetMsg
{
    NET_MSG_INTERFACE(Replicate);

    virtual bool IsReliable() const override;

    NetId mActorNetId = INVALID_TYPE_ID;
    uint16_t mNumVariables = 0;
    std::vector<uint16_t> mIndices;
    std::vector<Datum> mData;
    bool mReliable = false;
};

struct NetMsgReplicateScript : public NetMsgReplicate
{
    NET_MSG_INTERFACE(ReplicateScript);

    std::string mScriptName;
};

struct NetMsgInvoke : public NetMsg
{
    NET_MSG_INTERFACE(Invoke);

    virtual bool IsReliable() const override;

    NetId mActorNetId = INVALID_TYPE_ID;
    uint16_t mIndex = 0;
    uint8_t mNumParams = 0;
    bool mReliable = false;
    std::vector<Datum> mParams;
};

struct NetMsgInvokeScript : public NetMsgInvoke
{
    NET_MSG_INTERFACE(InvokeScript);

    std::string mScriptName;
};

struct NetMsgBroadcast : public NetMsg
{
    NET_MSG_INTERFACE(Broadcast);

    static const uint32_t sMagicNumber;

    uint32_t mMagic = sMagicNumber;
    uint32_t mGameCode = 0;
    uint32_t mVersion = 0;

    char mName[OCT_SESSION_NAME_LEN + 1] = {};
    uint8_t mMaxPlayers = 0;
    uint8_t mNumPlayers = 0;
};

struct NetMsgAck : public NetMsg
{
    NET_MSG_INTERFACE(Ack);

    uint16_t mSequenceNumber = 0;
};
