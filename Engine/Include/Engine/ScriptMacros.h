#pragma once

// These macros are ugly, but it makes it easy to expose C++ functions to Lua
// In your Actor class, you just need drop one of these macros based on the function return/params
// Example in Unit.cpp:
// SCRIPT_FUNC_1(AddHealth)
// SCRIPT_FUNC_R(GetHealth)

#include "Utilities.h"
#include "LuaBindings/LuaTypeCheck.h"
#include "ScriptAutoReg.h"

#define SF_CHECK_ACTOR(L, arg) CheckActorLuaType(L, arg, "Actor", "cfActor")
#define SF_CHECK_COMPONENT(L, arg) CheckComponentLuaType(L, arg, "Component", "cfComponent")

#define SfActor(idx) SF_CHECK_ACTOR(L, idx)
#define SfComponent(idx) SF_CHECK_COMPONENT(L, idx)
#define SfAsset(idx) LuaObjectToDatum(L, idx)
#define SfVector(idx) LuaObjectToDatum(L, idx)

#define SfInt(idx) lua_tointeger(L, idx)
#define SfFloat(idx) lua_tonumber(L, idx)
#define SfString(idx) lua_tostring(L, idx)
#define SfBool(idx) lua_toboolean(L, idx)

#define SCRIPT_FUNC_X1(ClassName, FuncName, P0)                                                                        \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ClassName* actor = SF_CHECK_ACTOR(L, 1)->As<ClassName>();                                 \
        actor->FuncName(P0(2));                                                             \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs_##ClassName, "", #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_X2(ClassName, FuncName, P0, P1)                                                                        \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ClassName* actor = SF_CHECK_ACTOR(L, 1)->As<ClassName>();                                 \
        actor->FuncName(P0(2), P1(3));                                                             \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs_##ClassName, "", #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_X3(ClassName, FuncName, P0, P1, P2)                                                                        \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ClassName* actor = SF_CHECK_ACTOR(L, 1)->As<ClassName>();                                 \
        actor->FuncName(P0(2), P1(3), P2(4));                                                             \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs_##ClassName, "", #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_X4(ClassName, FuncName, P0, P1, P2, P3)                                                                        \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ClassName* actor = SF_CHECK_ACTOR(L, 1)->As<ClassName>();                                 \
        actor->FuncName(P0(2), P1(3), P2(4), P3(5));                                                             \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs_##ClassName, "", #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_R_X1(ClassName, FuncName, P0)                                                                        \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ClassName* actor = SF_CHECK_ACTOR(L, 1)->As<ClassName>();                                 \
        Datum ret = actor->FuncName(P0(2));                                                             \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs_##ClassName, "", #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_R_X2(ClassName, FuncName, P0, P1)                                                                        \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ClassName* actor = SF_CHECK_ACTOR(L, 1)->As<ClassName>();                                 \
        Datum ret = actor->FuncName(P0(2), P1(3));                                                             \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs_##ClassName, "", #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_R_X3(ClassName, FuncName, P0, P1, P2)                                                                  \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ClassName* actor = SF_CHECK_ACTOR(L, 1)->As<ClassName>();                                 \
        Datum ret = actor->FuncName(P0(2), P1(3), P2(4));                                                       \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs_##ClassName, "", #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_R_X4(ClassName, FuncName, P0, P1, P2, P3)                                                              \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ClassName* actor = SF_CHECK_ACTOR(L, 1)->As<ClassName>();                                 \
        Datum ret = actor->FuncName(P0(2), P1(3), P2(4), P3(5));                                                \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs_##ClassName, "", #FuncName, ScriptFunc_##FuncName);


#define GLOBAL_SCRIPT_FUNC_X1(TableName, FuncName, P0)                                                                        \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        FuncName(P0(1));                                                             \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(GetGlobalAutoRegArray(), TableName, #FuncName, ScriptFunc_##FuncName);

#define GLOBAL_SCRIPT_FUNC_X2(TableName, FuncName, P0, P1)                                                                        \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        FuncName(P0(1), P1(2));                                                             \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(GetGlobalAutoRegArray(), TableName, #FuncName, ScriptFunc_##FuncName);

#define GLOBAL_SCRIPT_FUNC_X3(TableName, FuncName, P0, P1, P2)                                                                        \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        FuncName(P0(1), P1(2), P2(3));                                                             \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(GetGlobalAutoRegArray(), TableName, #FuncName, ScriptFunc_##FuncName);

#define GLOBAL_SCRIPT_FUNC_X4(TableName, FuncName, P0, P1, P2, P3)                                                                        \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        FuncName(P0(1), P1(2), P2(3), P3(4));                                                             \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(GetGlobalAutoRegArray(), TableName, #FuncName, ScriptFunc_##FuncName);

#define GLOBAL_SCRIPT_FUNC_R_X1(TableName, FuncName, P0)                                                                        \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        Datum ret = FuncName(P0(1));                                                             \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(GetGlobalAutoRegArray(), TableName, #FuncName, ScriptFunc_##FuncName);

#define GLOBAL_SCRIPT_FUNC_R_X2(TableName, FuncName, P0, P1)                                                                        \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        Datum ret = FuncName(P0(1), P1(2));                                                             \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(GetGlobalAutoRegArray(), TableName, #FuncName, ScriptFunc_##FuncName);

#define GLOBAL_SCRIPT_FUNC_R_X3(TableName, FuncName, P0, P1, P2)                                                                  \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        Datum ret = FuncName(P0(1), P1(2), P2(3));                                                       \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(GetGlobalAutoRegArray(), TableName, #FuncName, ScriptFunc_##FuncName);

#define GLOBAL_SCRIPT_FUNC_R_X4(TableName, FuncName, P0, P1, P2, P3)                                                              \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        Datum ret = FuncName(P0(1), P1(2), P2(3), P3(4));                                                \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(GetGlobalAutoRegArray(), TableName, #FuncName, ScriptFunc_##FuncName);



#define SCRIPT_FUNC(ClassName, FuncName)                                                                                   \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ClassName* actor = SF_CHECK_ACTOR(L, 1)->As<ClassName>();                                 \
        actor->FuncName();                                                                                      \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs_##ClassName, "", #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_1(ClassName, FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ClassName* actor = SF_CHECK_ACTOR(L, 1)->As<ClassName>();                                 \
        Datum param0; LuaObjectToDatum(L, 2, param0);                                                           \
        actor->FuncName(param0);                                                                                \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs_##ClassName, "", #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_2(ClassName, FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ClassName* actor = SF_CHECK_ACTOR(L, 1)->As<ClassName>();                                 \
        Datum param0; LuaObjectToDatum(L, 2, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 3, param1);                                                           \
        actor->FuncName(param0, param1);                                                                        \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs_##ClassName, "", #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_3(ClassName, FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ClassName* actor = SF_CHECK_ACTOR(L, 1)->As<ClassName>();                                 \
        Datum param0; LuaObjectToDatum(L, 2, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 3, param1);                                                           \
        Datum param2; LuaObjectToDatum(L, 4, param2);                                                           \
        actor->FuncName(param0, param1, param2);                                                                \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs_##ClassName, "", #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_4(ClassName, FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ClassName* actor = SF_CHECK_ACTOR(L, 1)->As<ClassName>();                                 \
        Datum param0; LuaObjectToDatum(L, 2, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 3, param1);                                                           \
        Datum param2; LuaObjectToDatum(L, 4, param2);                                                           \
        Datum param3; LuaObjectToDatum(L, 5, param3);                                                           \
        actor->FuncName(param0, param1, param2, param3);                                                        \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs_##ClassName, "", #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_5(ClassName, FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ClassName* actor = SF_CHECK_ACTOR(L, 1)->As<ClassName>();                                 \
        Datum param0; LuaObjectToDatum(L, 2, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 3, param1);                                                           \
        Datum param2; LuaObjectToDatum(L, 4, param2);                                                           \
        Datum param3; LuaObjectToDatum(L, 5, param3);                                                           \
        Datum param4; LuaObjectToDatum(L, 6, param4);                                                           \
        actor->FuncName(param0, param1, param2, param3, param4);                                                \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs_##ClassName, "", #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_6(ClassName, FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ClassName* actor = SF_CHECK_ACTOR(L, 1)->As<ClassName>();                                 \
        Datum param0; LuaObjectToDatum(L, 2, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 3, param1);                                                           \
        Datum param2; LuaObjectToDatum(L, 4, param2);                                                           \
        Datum param3; LuaObjectToDatum(L, 5, param3);                                                           \
        Datum param4; LuaObjectToDatum(L, 6, param4);                                                           \
        Datum param5; LuaObjectToDatum(L, 7, param5);                                                           \
        actor->FuncName(param0, param1, param2, param3, param4, param5);                                        \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs_##ClassName, "", #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_7(ClassName, FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ClassName* actor = SF_CHECK_ACTOR(L, 1)->As<ClassName>();                                 \
        Datum param0; LuaObjectToDatum(L, 2, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 3, param1);                                                           \
        Datum param2; LuaObjectToDatum(L, 4, param2);                                                           \
        Datum param3; LuaObjectToDatum(L, 5, param3);                                                           \
        Datum param4; LuaObjectToDatum(L, 6, param4);                                                           \
        Datum param5; LuaObjectToDatum(L, 7, param5);                                                           \
        Datum param6; LuaObjectToDatum(L, 8, param6);                                                           \
        actor->FuncName(param0, param1, param2, param3, param4, param5, param6);                                \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs_##ClassName, "", #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_8(ClassName, FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ClassName* actor = SF_CHECK_ACTOR(L, 1)->As<ClassName>();                                 \
        Datum param0; LuaObjectToDatum(L, 2, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 3, param1);                                                           \
        Datum param2; LuaObjectToDatum(L, 4, param2);                                                           \
        Datum param3; LuaObjectToDatum(L, 5, param3);                                                           \
        Datum param4; LuaObjectToDatum(L, 6, param4);                                                           \
        Datum param5; LuaObjectToDatum(L, 7, param5);                                                           \
        Datum param6; LuaObjectToDatum(L, 8, param6);                                                           \
        Datum param7; LuaObjectToDatum(L, 9, param7);                                                           \
        actor->FuncName(param0, param1, param2, param3, param4, param5, param6, param7);                        \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs_##ClassName, "", #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_R(ClassName, FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ClassName* actor = SF_CHECK_ACTOR(L, 1)->As<ClassName>();                                 \
        Datum ret = actor->FuncName();                                                                          \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs_##ClassName, "", #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_R_1(ClassName, FuncName)                                                                               \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ClassName* actor = SF_CHECK_ACTOR(L, 1)->As<ClassName>();                                 \
        Datum param0; LuaObjectToDatum(L, 2, param0);                                                           \
        Datum ret = actor->FuncName(param0);                                                                    \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs_##ClassName, "", #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_R_2(ClassName, FuncName)                                                                               \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ClassName* actor = SF_CHECK_ACTOR(L, 1)->As<ClassName>();                                 \
        Datum param0; LuaObjectToDatum(L, 2, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 3, param1);                                                           \
        Datum ret = actor->FuncName(param0, param1);                                                            \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs_##ClassName, "", #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_R_3(ClassName, FuncName)                                                                               \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ClassName* actor = SF_CHECK_ACTOR(L, 1)->As<ClassName>();                                 \
        Datum param0; LuaObjectToDatum(L, 2, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 3, param1);                                                           \
        Datum param2; LuaObjectToDatum(L, 4, param2);                                                           \
        Datum ret = actor->FuncName(param0, param1, param2);                                                    \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs_##ClassName, "", #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_R_4(ClassName, FuncName)                                                                               \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ClassName* actor = SF_CHECK_ACTOR(L, 1)->As<ClassName>();                                 \
        Datum param0; LuaObjectToDatum(L, 2, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 3, param1);                                                           \
        Datum param2; LuaObjectToDatum(L, 4, param2);                                                           \
        Datum param3; LuaObjectToDatum(L, 5, param3);                                                           \
        Datum ret = actor->FuncName(param0, param1, param2, param3);                                            \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs_##ClassName, "", #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_R_5(ClassName, FuncName)                                                                               \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ClassName* actor = SF_CHECK_ACTOR(L, 1)->As<ClassName>();                                 \
        Datum param0; LuaObjectToDatum(L, 2, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 3, param1);                                                           \
        Datum param2; LuaObjectToDatum(L, 4, param2);                                                           \
        Datum param3; LuaObjectToDatum(L, 5, param3);                                                           \
        Datum param4; LuaObjectToDatum(L, 6, param4);                                                           \
        Datum ret = actor->FuncName(param0, param1, param2, param3, param4);                                    \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs_##ClassName, "", #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_R_6(ClassName, FuncName)                                                                               \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ClassName* actor = SF_CHECK_ACTOR(L, 1)->As<ClassName>();                                 \
        Datum param0; LuaObjectToDatum(L, 2, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 3, param1);                                                           \
        Datum param2; LuaObjectToDatum(L, 4, param2);                                                           \
        Datum param3; LuaObjectToDatum(L, 5, param3);                                                           \
        Datum param4; LuaObjectToDatum(L, 6, param4);                                                           \
        Datum param5; LuaObjectToDatum(L, 7, param5);                                                           \
        Datum ret = actor->FuncName(param0, param1, param2, param3, param4, param5);                            \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs_##ClassName, "", #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_R_7(ClassName, FuncName)                                                                               \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ClassName* actor = SF_CHECK_ACTOR(L, 1)->As<ClassName>();                                 \
        Datum param0; LuaObjectToDatum(L, 2, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 3, param1);                                                           \
        Datum param2; LuaObjectToDatum(L, 4, param2);                                                           \
        Datum param3; LuaObjectToDatum(L, 5, param3);                                                           \
        Datum param4; LuaObjectToDatum(L, 6, param4);                                                           \
        Datum param5; LuaObjectToDatum(L, 7, param5);                                                           \
        Datum param6; LuaObjectToDatum(L, 8, param6);                                                           \
        Datum ret = actor->FuncName(param0, param1, param2, param3, param4, param5, param6);                    \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs_##ClassName, "", #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_R_8(ClassName, FuncName)                                                                               \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ClassName* actor = SF_CHECK_ACTOR(L, 1)->As<ClassName>();                                 \
        Datum param0; LuaObjectToDatum(L, 2, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 3, param1);                                                           \
        Datum param2; LuaObjectToDatum(L, 4, param2);                                                           \
        Datum param3; LuaObjectToDatum(L, 5, param3);                                                           \
        Datum param4; LuaObjectToDatum(L, 6, param4);                                                           \
        Datum param5; LuaObjectToDatum(L, 7, param5);                                                           \
        Datum param6; LuaObjectToDatum(L, 8, param6);                                                           \
        Datum param7; LuaObjectToDatum(L, 9, param7);                                                           \
        Datum ret = actor->FuncName(param0, param1, param2, param3, param4, param5, param6, param7);            \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs_##ClassName, "", #FuncName, ScriptFunc_##FuncName);






#define GLOBAL_SCRIPT_FUNC(TableName, FuncName)                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        FuncName();                                                                                      \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(GetGlobalAutoRegArray(), TableName, #FuncName, ScriptFunc_##FuncName);

#define GLOBAL_SCRIPT_FUNC_1(TableName, FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        Datum param0; LuaObjectToDatum(L, 1, param0);                                                           \
        FuncName(param0);                                                                                \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(GetGlobalAutoRegArray(), TableName, #FuncName, ScriptFunc_##FuncName);

#define GLOBAL_SCRIPT_FUNC_2(TableName, FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        Datum param0; LuaObjectToDatum(L, 1, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 2, param1);                                                           \
        FuncName(param0, param1);                                                                        \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(GetGlobalAutoRegArray(), TableName, #FuncName, ScriptFunc_##FuncName);

#define GLOBAL_SCRIPT_FUNC_3(TableName, FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        Datum param0; LuaObjectToDatum(L, 1, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 2, param1);                                                           \
        Datum param2; LuaObjectToDatum(L, 3, param2);                                                           \
        FuncName(param0, param1, param2);                                                                \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(GetGlobalAutoRegArray(), TableName, #FuncName, ScriptFunc_##FuncName);

#define GLOBAL_SCRIPT_FUNC_4(TableName, FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        Datum param0; LuaObjectToDatum(L, 1, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 2, param1);                                                           \
        Datum param2; LuaObjectToDatum(L, 3, param2);                                                           \
        Datum param3; LuaObjectToDatum(L, 4, param3);                                                           \
        FuncName(param0, param1, param2, param3);                                                        \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(GetGlobalAutoRegArray(), TableName, #FuncName, ScriptFunc_##FuncName);

#define GLOBAL_SCRIPT_FUNC_5(TableName, FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        Datum param0; LuaObjectToDatum(L, 1, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 2, param1);                                                           \
        Datum param2; LuaObjectToDatum(L, 3, param2);                                                           \
        Datum param3; LuaObjectToDatum(L, 4, param3);                                                           \
        Datum param4; LuaObjectToDatum(L, 5, param4);                                                           \
        FuncName(param0, param1, param2, param3, param4);                                                \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(GetGlobalAutoRegArray(), TableName, #FuncName, ScriptFunc_##FuncName);

#define GLOBAL_SCRIPT_FUNC_6(TableName, FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        Datum param0; LuaObjectToDatum(L, 1, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 2, param1);                                                           \
        Datum param2; LuaObjectToDatum(L, 3, param2);                                                           \
        Datum param3; LuaObjectToDatum(L, 4, param3);                                                           \
        Datum param4; LuaObjectToDatum(L, 5, param4);                                                           \
        Datum param5; LuaObjectToDatum(L, 6, param5);                                                           \
        FuncName(param0, param1, param2, param3, param4, param5);                                        \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(GetGlobalAutoRegArray(), TableName, #FuncName, ScriptFunc_##FuncName);

#define GLOBAL_SCRIPT_FUNC_7(TableName, FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        Datum param0; LuaObjectToDatum(L, 1, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 2, param1);                                                           \
        Datum param2; LuaObjectToDatum(L, 3, param2);                                                           \
        Datum param3; LuaObjectToDatum(L, 4, param3);                                                           \
        Datum param4; LuaObjectToDatum(L, 5, param4);                                                           \
        Datum param5; LuaObjectToDatum(L, 6, param5);                                                           \
        Datum param6; LuaObjectToDatum(L, 7, param6);                                                           \
        FuncName(param0, param1, param2, param3, param4, param5, param6);                                \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(GetGlobalAutoRegArray(), TableName, #FuncName, ScriptFunc_##FuncName);

#define GLOBAL_SCRIPT_FUNC_8(TableName, FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        Datum param0; LuaObjectToDatum(L, 1, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 2, param1);                                                           \
        Datum param2; LuaObjectToDatum(L, 3, param2);                                                           \
        Datum param3; LuaObjectToDatum(L, 4, param3);                                                           \
        Datum param4; LuaObjectToDatum(L, 5, param4);                                                           \
        Datum param5; LuaObjectToDatum(L, 6, param5);                                                           \
        Datum param6; LuaObjectToDatum(L, 7, param6);                                                           \
        Datum param7; LuaObjectToDatum(L, 8, param7);                                                           \
        FuncName(param0, param1, param2, param3, param4, param5, param6, param7);                        \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(GetGlobalAutoRegArray(), TableName, #FuncName, ScriptFunc_##FuncName);

#define GLOBAL_SCRIPT_FUNC_R(TableName, FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        Datum ret = FuncName();                                                                          \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(GetGlobalAutoRegArray(), TableName, #FuncName, ScriptFunc_##FuncName);

#define GLOBAL_SCRIPT_FUNC_R_1(TableName, FuncName)                                                                               \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        Datum param0; LuaObjectToDatum(L, 1, param0);                                                           \
        Datum ret = FuncName(param0);                                                                    \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(GetGlobalAutoRegArray(), TableName, #FuncName, ScriptFunc_##FuncName);

#define GLOBAL_SCRIPT_FUNC_R_2(TableName, FuncName)                                                                               \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        Datum param0; LuaObjectToDatum(L, 1, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 2, param1);                                                           \
        Datum ret = FuncName(param0, param1);                                                            \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(GetGlobalAutoRegArray(), TableName, #FuncName, ScriptFunc_##FuncName);

#define GLOBAL_SCRIPT_FUNC_R_3(TableName, FuncName)                                                                               \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        Datum param0; LuaObjectToDatum(L, 1, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 2, param1);                                                           \
        Datum param2; LuaObjectToDatum(L, 3, param2);                                                           \
        Datum ret = FuncName(param0, param1, param2);                                                    \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(GetGlobalAutoRegArray(), TableName, #FuncName, ScriptFunc_##FuncName);

#define GLOBAL_SCRIPT_FUNC_R_4(TableName, FuncName)                                                                               \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        Datum param0; LuaObjectToDatum(L, 1, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 2, param1);                                                           \
        Datum param2; LuaObjectToDatum(L, 3, param2);                                                           \
        Datum param3; LuaObjectToDatum(L, 4, param3);                                                           \
        Datum ret = FuncName(param0, param1, param2, param3);                                            \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(GetGlobalAutoRegArray(), TableName, #FuncName, ScriptFunc_##FuncName);

#define GLOBAL_SCRIPT_FUNC_R_5(TableName, FuncName)                                                                               \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        Datum param0; LuaObjectToDatum(L, 1, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 2, param1);                                                           \
        Datum param2; LuaObjectToDatum(L, 3, param2);                                                           \
        Datum param3; LuaObjectToDatum(L, 4, param3);                                                           \
        Datum param4; LuaObjectToDatum(L, 5, param4);                                                           \
        Datum ret = FuncName(param0, param1, param2, param3, param4);                                    \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(GetGlobalAutoRegArray(), TableName, #FuncName, ScriptFunc_##FuncName);

#define GLOBAL_SCRIPT_FUNC_R_6(TableName, FuncName)                                                                               \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        Datum param0; LuaObjectToDatum(L, 1, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 2, param1);                                                           \
        Datum param2; LuaObjectToDatum(L, 3, param2);                                                           \
        Datum param3; LuaObjectToDatum(L, 4, param3);                                                           \
        Datum param4; LuaObjectToDatum(L, 5, param4);                                                           \
        Datum param5; LuaObjectToDatum(L, 6, param5);                                                           \
        Datum ret = FuncName(param0, param1, param2, param3, param4, param5);                            \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(GetGlobalAutoRegArray(), TableName, #FuncName, ScriptFunc_##FuncName);

#define GLOBAL_SCRIPT_FUNC_R_7(TableName, FuncName)                                                                               \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        Datum param0; LuaObjectToDatum(L, 1, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 2, param1);                                                           \
        Datum param2; LuaObjectToDatum(L, 3, param2);                                                           \
        Datum param3; LuaObjectToDatum(L, 4, param3);                                                           \
        Datum param4; LuaObjectToDatum(L, 5, param4);                                                           \
        Datum param5; LuaObjectToDatum(L, 6, param5);                                                           \
        Datum param6; LuaObjectToDatum(L, 7, param6);                                                           \
        Datum ret = FuncName(param0, param1, param2, param3, param4, param5, param6);                    \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(GetGlobalAutoRegArray(), TableName, #FuncName, ScriptFunc_##FuncName);

#define GLOBAL_SCRIPT_FUNC_R_8(TableName, FuncName)                                                                               \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        Datum param0; LuaObjectToDatum(L, 1, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 2, param1);                                                           \
        Datum param2; LuaObjectToDatum(L, 3, param2);                                                           \
        Datum param3; LuaObjectToDatum(L, 4, param3);                                                           \
        Datum param4; LuaObjectToDatum(L, 5, param4);                                                           \
        Datum param5; LuaObjectToDatum(L, 6, param5);                                                           \
        Datum param6; LuaObjectToDatum(L, 7, param6);                                                           \
        Datum param7; LuaObjectToDatum(L, 8, param7);                                                           \
        Datum ret = FuncName(param0, param1, param2, param3, param4, param5, param6, param7);            \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(GetGlobalAutoRegArray(), TableName, #FuncName, ScriptFunc_##FuncName);

// Actor Script Functions
#define ASF(ClassName, FuncName)   SCRIPT_FUNC(ClassName, FuncName)
#define ASF_1(ClassName, FuncName) SCRIPT_FUNC_1(ClassName, FuncName)
#define ASF_2(ClassName, FuncName) SCRIPT_FUNC_2(ClassName, FuncName)
#define ASF_3(ClassName, FuncName) SCRIPT_FUNC_3(ClassName, FuncName)
#define ASF_4(ClassName, FuncName) SCRIPT_FUNC_4(ClassName, FuncName)
#define ASF_5(ClassName, FuncName) SCRIPT_FUNC_5(ClassName, FuncName)
#define ASF_6(ClassName, FuncName) SCRIPT_FUNC_6(ClassName, FuncName)
#define ASF_7(ClassName, FuncName) SCRIPT_FUNC_7(ClassName, FuncName)
#define ASF_8(ClassName, FuncName) SCRIPT_FUNC_8(ClassName, FuncName)
#define ASF_R(ClassName, FuncName)   SCRIPT_FUNC_R(ClassName, FuncName)
#define ASF_R_1(ClassName, FuncName) SCRIPT_FUNC_R_1(ClassName, FuncName)
#define ASF_R_2(ClassName, FuncName) SCRIPT_FUNC_R_2(ClassName, FuncName)
#define ASF_R_3(ClassName, FuncName) SCRIPT_FUNC_R_3(ClassName, FuncName)
#define ASF_R_4(ClassName, FuncName) SCRIPT_FUNC_R_4(ClassName, FuncName)
#define ASF_R_5(ClassName, FuncName) SCRIPT_FUNC_R_5(ClassName, FuncName)
#define ASF_R_6(ClassName, FuncName) SCRIPT_FUNC_R_6(ClassName, FuncName)
#define ASF_R_7(ClassName, FuncName) SCRIPT_FUNC_R_7(ClassName, FuncName)
#define ASF_R_8(ClassName, FuncName) SCRIPT_FUNC_R_8(ClassName, FuncName)

#define TSF(TableName, FuncName)   GLOBAL_SCRIPT_FUNC(TableName, FuncName)
#define TSF_1(TableName, FuncName) GLOBAL_SCRIPT_FUNC_1(TableName, FuncName)
#define TSF_2(TableName, FuncName) GLOBAL_SCRIPT_FUNC_2(TableName, FuncName)
#define TSF_3(TableName, FuncName) GLOBAL_SCRIPT_FUNC_3(TableName, FuncName)
#define TSF_4(TableName, FuncName) GLOBAL_SCRIPT_FUNC_4(TableName, FuncName)
#define TSF_5(TableName, FuncName) GLOBAL_SCRIPT_FUNC_5(TableName, FuncName)
#define TSF_6(TableName, FuncName) GLOBAL_SCRIPT_FUNC_6(TableName, FuncName)
#define TSF_7(TableName, FuncName) GLOBAL_SCRIPT_FUNC_7(TableName, FuncName)
#define TSF_8(TableName, FuncName) GLOBAL_SCRIPT_FUNC_8(TableName, FuncName)
#define TSF_R(TableName, FuncName)   GLOBAL_SCRIPT_FUNC_R(TableName, FuncName)
#define TSF_R_1(TableName, FuncName) GLOBAL_SCRIPT_FUNC_R_1(TableName, FuncName)
#define TSF_R_2(TableName, FuncName) GLOBAL_SCRIPT_FUNC_R_2(TableName, FuncName)
#define TSF_R_3(TableName, FuncName) GLOBAL_SCRIPT_FUNC_R_3(TableName, FuncName)
#define TSF_R_4(TableName, FuncName) GLOBAL_SCRIPT_FUNC_R_4(TableName, FuncName)
#define TSF_R_5(TableName, FuncName) GLOBAL_SCRIPT_FUNC_R_5(TableName, FuncName)
#define TSF_R_6(TableName, FuncName) GLOBAL_SCRIPT_FUNC_R_6(TableName, FuncName)
#define TSF_R_7(TableName, FuncName) GLOBAL_SCRIPT_FUNC_R_7(TableName, FuncName)
#define TSF_R_8(TableName, FuncName) GLOBAL_SCRIPT_FUNC_R_8(TableName, FuncName)

#define GSF(FuncName)   GLOBAL_SCRIPT_FUNC("", FuncName)
#define GSF_1(FuncName) GLOBAL_SCRIPT_FUNC_1("", FuncName)
#define GSF_2(FuncName) GLOBAL_SCRIPT_FUNC_2("", FuncName)
#define GSF_3(FuncName) GLOBAL_SCRIPT_FUNC_3("", FuncName)
#define GSF_4(FuncName) GLOBAL_SCRIPT_FUNC_4("", FuncName)
#define GSF_5(FuncName) GLOBAL_SCRIPT_FUNC_5("", FuncName)
#define GSF_6(FuncName) GLOBAL_SCRIPT_FUNC_6("", FuncName)
#define GSF_7(FuncName) GLOBAL_SCRIPT_FUNC_7("", FuncName)
#define GSF_8(FuncName) GLOBAL_SCRIPT_FUNC_8("", FuncName)
#define GSF_R(FuncName)   GLOBAL_SCRIPT_FUNC_R("", FuncName)
#define GSF_R_1(FuncName) GLOBAL_SCRIPT_FUNC_R_1("", FuncName)
#define GSF_R_2(FuncName) GLOBAL_SCRIPT_FUNC_R_2("", FuncName)
#define GSF_R_3(FuncName) GLOBAL_SCRIPT_FUNC_R_3("", FuncName)
#define GSF_R_4(FuncName) GLOBAL_SCRIPT_FUNC_R_4("", FuncName)
#define GSF_R_5(FuncName) GLOBAL_SCRIPT_FUNC_R_5("", FuncName)
#define GSF_R_6(FuncName) GLOBAL_SCRIPT_FUNC_R_6("", FuncName)
#define GSF_R_7(FuncName) GLOBAL_SCRIPT_FUNC_R_7("", FuncName)
#define GSF_R_8(FuncName) GLOBAL_SCRIPT_FUNC_R_8("", FuncName)

// Explicit (needed for pointer conversions)
#define ASF_X1(ClassName, FuncName, P0) SCRIPT_FUNC_X1(ClassName, FuncName, P0)
#define ASF_X2(ClassName, FuncName, P0, P1) SCRIPT_FUNC_X2(ClassName, FuncName, P0, P1)
#define ASF_X3(ClassName, FuncName, P0, P1, P2) SCRIPT_FUNC_X3(ClassName, FuncName, P0, P1, P2)
#define ASF_X4(ClassName, FuncName, P0, P1, P2, P3) SCRIPT_FUNC_X4(ClassName, FuncName, P0, P1, P2, P3)
#define ASF_R_X1(ClassName, FuncName, P0) SCRIPT_FUNC_R_X1(ClassName, FuncName, P0)
#define ASF_R_X2(ClassName, FuncName, P0, P1) SCRIPT_FUNC_R_X2(ClassName, FuncName, P0, P1)
#define ASF_R_X3(ClassName, FuncName, P0, P1, P2) SCRIPT_FUNC_R_X3(ClassName, FuncName,P0, P1, P2)
#define ASF_R_X4(ClassName, FuncName, P0, P1, P2, P3) SCRIPT_FUNC_R_X4(ClassName, FuncName, P0, P1, P2, P3)

#define TSF_X1(TableName, FuncName, P0) GLOBAL_SCRIPT_FUNC_X1(TableName, FuncName, P0)
#define TSF_X2(TableName, FuncName, P0, P1) GLOBAL_SCRIPT_FUNC_X2(TableName, FuncName, P0, P1)
#define TSF_X3(TableName, FuncName, P0, P1, P2) GLOBAL_SCRIPT_FUNC_X3(TableName, FuncName, P0, P1, P2)
#define TSF_X4(TableName, FuncName, P0, P1, P2, P3) GLOBAL_SCRIPT_FUNC_X4(TableName, FuncName, P0, P1, P2, P3)
#define TSF_R_X1(TableName, FuncName, P0) GLOBAL_SCRIPT_FUNC_R_X1(TableName, FuncName, P0)
#define TSF_R_X2(TableName, FuncName, P0, P1) GLOBAL_SCRIPT_FUNC_R_X2(TableName, FuncName, P0, P1)
#define TSF_R_X3(TableName, FuncName, P0, P1, P2) GLOBAL_SCRIPT_FUNC_R_X3(TableName, FuncName,P0, P1, P2)
#define TSF_R_X4(TableName, FuncName, P0, P1, P2, P3) GLOBAL_SCRIPT_FUNC_R_X4(TableName, FuncName, P0, P1, P2, P3)

#define GSF_X1(FuncName, P0) GLOBAL_SCRIPT_FUNC_X1("", FuncName, P0)
#define GSF_X2(FuncName, P0, P1) GLOBAL_SCRIPT_FUNC_X2("", FuncName, P0, P1)
#define GSF_X3(FuncName, P0, P1, P2) GLOBAL_SCRIPT_FUNC_X3("", FuncName, P0, P1, P2)
#define GSF_X4(FuncName, P0, P1, P2, P3) GLOBAL_SCRIPT_FUNC_X4("", FuncName, P0, P1, P2, P3)
#define GSF_R_X1(FuncName, P0) GLOBAL_SCRIPT_FUNC_R_X1("", FuncName, P0)
#define GSF_R_X2(FuncName, P0, P1) GLOBAL_SCRIPT_FUNC_R_X2("", FuncName, P0, P1)
#define GSF_R_X3(FuncName, P0, P1, P2) GLOBAL_SCRIPT_FUNC_R_X3("", FuncName,P0, P1, P2)
#define GSF_R_X4(FuncName, P0, P1, P2, P3) GLOBAL_SCRIPT_FUNC_R_X4("", FuncName, P0, P1, P2, P3)
