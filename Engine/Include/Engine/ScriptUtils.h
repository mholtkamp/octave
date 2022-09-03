#pragma once

// These macros are ugly, but it makes it easy to expose C++ functions to Lua
// In your Actor class, you just need drop one of these macros based on the function return/params
// Example in Unit.cpp:
// SCRIPT_FUNC_1(AddHealth)
// SCRIPT_FUNC_R(GetHealth)

#include "Utilities.h"
#include "LuaBindings/LuaTypeCheck.h"

#define SF_CHECK_ACTOR(L, arg) CheckActorLuaType(L, arg, "Actor", "cfActor")

struct AutoRegData
{
    AutoRegData(const char* name, lua_CFunction func) :
        mName(name),
        mFunc(func)
    {

    }

    const char* mName;
    lua_CFunction mFunc;
};

struct ScriptFuncAutoReg
{
    ScriptFuncAutoReg(std::vector<AutoRegData>& autoRegs, const char* name, lua_CFunction func)
    {
        autoRegs.emplace_back(name, func);
    }
};

#define SCRIPT_FUNC(FuncName)                                                                                   \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ScriptActorAlias* actor = SF_CHECK_ACTOR(L, 1)->As<ScriptActorAlias>();                                 \
        actor->FuncName();                                                                                      \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_1(FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ScriptActorAlias* actor = SF_CHECK_ACTOR(L, 1)->As<ScriptActorAlias>();                                 \
        Datum param0; LuaObjectToDatum(L, 2, param0);                                                           \
        actor->FuncName(param0);                                                                                \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_2(FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ScriptActorAlias* actor = SF_CHECK_ACTOR(L, 1)->As<ScriptActorAlias>();                                 \
        Datum param0; LuaObjectToDatum(L, 2, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 3, param1);                                                           \
        actor->FuncName(param0, param1);                                                                        \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_3(FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ScriptActorAlias* actor = SF_CHECK_ACTOR(L, 1)->As<ScriptActorAlias>();                                 \
        Datum param0; LuaObjectToDatum(L, 2, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 3, param1);                                                           \
        Datum param2; LuaObjectToDatum(L, 4, param2);                                                           \
        actor->FuncName(param0, param1, param2);                                                                \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_4(FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ScriptActorAlias* actor = SF_CHECK_ACTOR(L, 1)->As<ScriptActorAlias>();                                 \
        Datum param0; LuaObjectToDatum(L, 2, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 3, param1);                                                           \
        Datum param2; LuaObjectToDatum(L, 4, param2);                                                           \
        Datum param3; LuaObjectToDatum(L, 5, param3);                                                           \
        actor->FuncName(param0, param1, param2, param3);                                                        \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_5(FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ScriptActorAlias* actor = SF_CHECK_ACTOR(L, 1)->As<ScriptActorAlias>();                                 \
        Datum param0; LuaObjectToDatum(L, 2, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 3, param1);                                                           \
        Datum param2; LuaObjectToDatum(L, 4, param2);                                                           \
        Datum param3; LuaObjectToDatum(L, 5, param3);                                                           \
        Datum param4; LuaObjectToDatum(L, 6, param4);                                                           \
        actor->FuncName(param0, param1, param2, param3, param4);                                                \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_6(FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ScriptActorAlias* actor = SF_CHECK_ACTOR(L, 1)->As<ScriptActorAlias>();                                 \
        Datum param0; LuaObjectToDatum(L, 2, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 3, param1);                                                           \
        Datum param2; LuaObjectToDatum(L, 4, param2);                                                           \
        Datum param3; LuaObjectToDatum(L, 5, param3);                                                           \
        Datum param4; LuaObjectToDatum(L, 6, param4);                                                           \
        Datum param5; LuaObjectToDatum(L, 7, param5);                                                           \
        actor->FuncName(param0, param1, param2, param3, param4, param5);                                        \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_7(FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ScriptActorAlias* actor = SF_CHECK_ACTOR(L, 1)->As<ScriptActorAlias>();                                 \
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
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_8(FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ScriptActorAlias* actor = SF_CHECK_ACTOR(L, 1)->As<ScriptActorAlias>();                                 \
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
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_R(FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ScriptActorAlias* actor = SF_CHECK_ACTOR(L, 1)->As<ScriptActorAlias>();                                 \
        Datum ret = actor->FuncName();                                                                          \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_R_1(FuncName)                                                                               \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ScriptActorAlias* actor = SF_CHECK_ACTOR(L, 1)->As<ScriptActorAlias>();                                 \
        Datum param0; LuaObjectToDatum(L, 2, param0);                                                           \
        Datum ret = actor->FuncName(param0);                                                                    \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_R_2(FuncName)                                                                               \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ScriptActorAlias* actor = SF_CHECK_ACTOR(L, 1)->As<ScriptActorAlias>();                                 \
        Datum param0; LuaObjectToDatum(L, 2, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 3, param1);                                                           \
        Datum ret = actor->FuncName(param0, param1);                                                            \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_R_3(FuncName)                                                                               \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ScriptActorAlias* actor = SF_CHECK_ACTOR(L, 1)->As<ScriptActorAlias>();                                 \
        Datum param0; LuaObjectToDatum(L, 2, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 3, param1);                                                           \
        Datum param2; LuaObjectToDatum(L, 4, param2);                                                           \
        Datum ret = actor->FuncName(param0, param1, param2);                                                    \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_R_4(FuncName)                                                                               \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ScriptActorAlias* actor = SF_CHECK_ACTOR(L, 1)->As<ScriptActorAlias>();                                 \
        Datum param0; LuaObjectToDatum(L, 2, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 3, param1);                                                           \
        Datum param2; LuaObjectToDatum(L, 4, param2);                                                           \
        Datum param3; LuaObjectToDatum(L, 5, param3);                                                           \
        Datum ret = actor->FuncName(param0, param1, param2, param3);                                            \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_R_5(FuncName)                                                                               \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ScriptActorAlias* actor = SF_CHECK_ACTOR(L, 1)->As<ScriptActorAlias>();                                 \
        Datum param0; LuaObjectToDatum(L, 2, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 3, param1);                                                           \
        Datum param2; LuaObjectToDatum(L, 4, param2);                                                           \
        Datum param3; LuaObjectToDatum(L, 5, param3);                                                           \
        Datum param4; LuaObjectToDatum(L, 6, param4);                                                           \
        Datum ret = actor->FuncName(param0, param1, param2, param3, param4);                                    \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_R_6(FuncName)                                                                               \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ScriptActorAlias* actor = SF_CHECK_ACTOR(L, 1)->As<ScriptActorAlias>();                                 \
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
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_R_7(FuncName)                                                                               \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ScriptActorAlias* actor = SF_CHECK_ACTOR(L, 1)->As<ScriptActorAlias>();                                 \
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
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_R_8(FuncName)                                                                               \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ScriptActorAlias* actor = SF_CHECK_ACTOR(L, 1)->As<ScriptActorAlias>();                                 \
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
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, #FuncName, ScriptFunc_##FuncName);


