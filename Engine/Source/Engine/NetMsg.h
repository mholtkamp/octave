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
    Ready,
    Spawn,
    Destroy,
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

struct NetMsgReady : public NetMsg
{
    NET_MSG_INTERFACE_RELIABLE(Ready);
};

struct NetMsgPing : public NetMsg
{
    NET_MSG_INTERFACE(Ping);
};

struct NetMsgSpawn : public NetMsg
{
    NET_MSG_INTERFACE_RELIABLE(Spawn);

    TypeId mNodeTypeId = INVALID_TYPE_ID;
    NetId mNetId = INVALID_NET_ID;
    NetId mParentNetId = INVALID_NET_ID;
    bool mReplicateTransform = INVALID_NET_ID;
    std::string mSceneName;
};

struct NetMsgDestroy : public NetMsg
{
    NET_MSG_INTERFACE_RELIABLE(Destroy);

    NetId mNetId = INVALID_NET_ID;
};

struct NetMsgReplicate : public NetMsg
{
    NET_MSG_INTERFACE(Replicate);

    virtual bool IsReliable() const override;

    NetId mNodeNetId = INVALID_TYPE_ID;
    uint16_t mNumVariables = 0;
    std::vector<uint16_t> mIndices;
    std::vector<Datum> mData;
    bool mReliable = false;
};

struct NetMsgReplicateScript : public NetMsgReplicate
{
    NET_MSG_INTERFACE(ReplicateScript);
};

struct NetMsgInvoke : public NetMsg
{
    NET_MSG_INTERFACE(Invoke);

    virtual bool IsReliable() const override;

    NetId mNodeNetId = INVALID_TYPE_ID;
    uint16_t mIndex = 0;
    uint8_t mNumParams = 0;
    bool mReliable = false;
    std::vector<Datum> mParams;
};

// Creating a separate message for script invoke so we don't need
// to serialize an extra bool for whether the netfunc belongs to the script.
struct NetMsgInvokeScript : public NetMsgInvoke
{
    NET_MSG_INTERFACE(InvokeScript);
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
