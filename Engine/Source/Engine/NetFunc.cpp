#include "NetFunc.h"
#include "Actor.h"
#include "NetworkManager.h"
#include <string.h>

NetFunc::NetFunc()
{
    
}

NetFunc::NetFunc(NetFuncType type, const char* name, NetFunc0FP fp, bool reliable)
{
    mType = type;
    mName = name;
    mFuncPointer.p0 = fp;
    mNumParams = 0;
    mReliable = reliable;
}

NetFunc::NetFunc(NetFuncType type, const char* name, NetFunc1FP fp, bool reliable)
{
    mType = type;
    mName = name;
    mFuncPointer.p1 = fp;
    mNumParams = 1;
    mReliable = reliable;
}

NetFunc::NetFunc(NetFuncType type, const char* name, NetFunc2FP fp, bool reliable)
{
    mType = type;
    mName = name;
    mFuncPointer.p2 = fp;
    mNumParams = 2;
    mReliable = reliable;
}

NetFunc::NetFunc(NetFuncType type, const char* name, NetFunc3FP fp, bool reliable)
{
    mType = type;
    mName = name;
    mFuncPointer.p3 = fp;
    mNumParams = 3;
    mReliable = reliable;
}

NetFunc::NetFunc(NetFuncType type, const char* name, NetFunc4FP fp, bool reliable)
{
    mType = type;
    mName = name;
    mFuncPointer.p4 = fp;
    mNumParams = 4;
    mReliable = reliable;
}

NetFunc::NetFunc(NetFuncType type, const char* name, NetFunc5FP fp, bool reliable)
{
    mType = type;
    mName = name;
    mFuncPointer.p5 = fp;
    mNumParams = 5;
    mReliable = reliable;
}

NetFunc::NetFunc(NetFuncType type, const char* name, NetFunc6FP fp, bool reliable)
{
    mType = type;
    mName = name;
    mFuncPointer.p6 = fp;
    mNumParams = 6;
    mReliable = reliable;
}

NetFunc::NetFunc(NetFuncType type, const char* name, NetFunc7FP fp, bool reliable)
{
    mType = type;
    mName = name;
    mFuncPointer.p7 = fp;
    mNumParams = 7;
    mReliable = reliable;
}

NetFunc::NetFunc(NetFuncType type, const char* name, NetFunc8FP fp, bool reliable)
{
    mType = type;
    mName = name;
    mFuncPointer.p8 = fp;
    mNumParams = 8;
    mReliable = reliable;
}


bool ShouldExecuteNetFunc(NetFuncType type, Node* node)
{
    bool execute = false;
    NetStatus netStatus = NetworkManager::Get()->GetNetStatus();
    NetHostId owningHost = node->GetOwningHost();

    switch (type)
    {
    case NetFuncType::Server:
        execute = (netStatus == NetStatus::Local ||
            netStatus == NetStatus::Server);
        break;

    case NetFuncType::Client:
        execute = (netStatus == NetStatus::Local ||
            netStatus == NetStatus::Client ||
            (netStatus == NetStatus::Server && (owningHost == SERVER_HOST_ID || owningHost == INVALID_HOST_ID)));
        break;

    case NetFuncType::Multicast:
        execute = (netStatus == NetStatus::Local ||
            netStatus == NetStatus::Server ||
            netStatus == NetStatus::Client);
        break;

    default: OCT_ASSERT(0); break;
    }

    return execute;
}

bool ShouldSendNetFunc(NetFuncType type, Node* node)
{
    bool send = false;

    NetStatus netStatus = NetworkManager::Get()->GetNetStatus();
    NetHostId hostId = NetworkManager::Get()->GetHostId();
    NetHostId owningHost = node->GetOwningHost();

    switch (type)
    {
    case NetFuncType::Server:
        send = (netStatus == NetStatus::Client &&
            owningHost == hostId);
        break;

    case NetFuncType::Client:
        send = (netStatus == NetStatus::Server &&
            owningHost != hostId &&
            owningHost != INVALID_HOST_ID);
        break;

    case NetFuncType::Multicast:
        send = (netStatus == NetStatus::Server);
        break;

    default: OCT_ASSERT(0); break;
    }

    return send;
}