#pragma once

#include "Datum.h"
#include <unordered_map>

class Actor;

#define OCT_NET_FUNC_MAX_PARAMS 8

#define ADD_NET_FUNC(OutFuncs, FuncType, Function) OutFuncs.push_back(NetFunc(NetFuncType::FuncType, #Function, Function));
#define ADD_NET_FUNC_RELIABLE(OutFuncs, FuncType, Function) OutFuncs.push_back(NetFunc(NetFuncType::FuncType, #Function, Function, true));

typedef void (*NetFunc0FP)(Actor* actor);
typedef void (*NetFunc1FP)(Actor* actor, Datum& param0);
typedef void (*NetFunc2FP)(Actor* actor, Datum& param0, Datum& param1);
typedef void (*NetFunc3FP)(Actor* actor, Datum& param0, Datum& param1, Datum& param2);
typedef void (*NetFunc4FP)(Actor* actor, Datum& param0, Datum& param1, Datum& param2, Datum& param3);
typedef void (*NetFunc5FP)(Actor* actor, Datum& param0, Datum& param1, Datum& param2, Datum& param3, Datum& param4);
typedef void (*NetFunc6FP)(Actor* actor, Datum& param0, Datum& param1, Datum& param2, Datum& param3, Datum& param4, Datum& param5);
typedef void (*NetFunc7FP)(Actor* actor, Datum& param0, Datum& param1, Datum& param2, Datum& param3, Datum& param4, Datum& param5, Datum& param6);
typedef void (*NetFunc8FP)(Actor* actor, Datum& param0, Datum& param1, Datum& param2, Datum& param3, Datum& param4, Datum& param5, Datum& param6, Datum& param7);

enum NetFuncType : uint8_t
{
    Server,
    Client,
    Multicast,

    Count
};

struct NetFunc
{
    NetFuncType mType = NetFuncType::Count;
    uint8_t mNumParams = 0;
    uint16_t mIndex = 0;
    std::string mName;
    bool mReliable = false;

    // Do we need to store param types for safety checking?
    // I don't think so since the user code will naturally assert if you GetVector() on a float datum for instance.
    //DatumType mParamTypes[OCT_NET_FUNC_MAX_PARAMS] = { DatumType::Count, DatumType::Count, DatumType::Count, DatumType::Count, DatumType::Count, DatumType::Count, DatumType::Count, DatumType::Count};
    
    union
    {
        NetFunc0FP p0;
        NetFunc1FP p1;
        NetFunc2FP p2;
        NetFunc3FP p3;
        NetFunc4FP p4;
        NetFunc5FP p5;
        NetFunc6FP p6;
        NetFunc7FP p7;
        NetFunc8FP p8;
    } mFuncPointer = {};

    NetFunc();
    NetFunc(NetFuncType type, const char* name, NetFunc0FP fp, bool reliable = false);
    NetFunc(NetFuncType type, const char* name, NetFunc1FP fp, bool reliable = false);
    NetFunc(NetFuncType type, const char* name, NetFunc2FP fp, bool reliable = false);
    NetFunc(NetFuncType type, const char* name, NetFunc3FP fp, bool reliable = false);
    NetFunc(NetFuncType type, const char* name, NetFunc4FP fp, bool reliable = false);
    NetFunc(NetFuncType type, const char* name, NetFunc5FP fp, bool reliable = false);
    NetFunc(NetFuncType type, const char* name, NetFunc6FP fp, bool reliable = false);
    NetFunc(NetFuncType type, const char* name, NetFunc7FP fp, bool reliable = false);
    NetFunc(NetFuncType type, const char* name, NetFunc8FP fp, bool reliable = false);
};

//struct ScriptNetFunc
//{
//    uint16_t mIndex = 0;
//    NetFuncType mType = NetFuncType::Count;
//    bool mReliable = false;
//    //uint8_t mNumParams = 0;
//    std::string mName;
//
//    ScriptNetFunc() {}
//};

typedef NetFunc ScriptNetFunc;

bool ShouldExecuteNetFunc(NetFuncType type, Actor* actor);
bool ShouldSendNetFunc(NetFuncType type, Actor* actor);
