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

#define SCRIPT_FUNC(FuncName)                                                                                   \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ScriptActorAlias* actor = SF_CHECK_ACTOR(L, 1)->As<ScriptActorAlias>();                                 \
        actor->FuncName();                                                                                      \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, "", #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_1(FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ScriptActorAlias* actor = SF_CHECK_ACTOR(L, 1)->As<ScriptActorAlias>();                                 \
        Datum param0; LuaObjectToDatum(L, 2, param0);                                                           \
        actor->FuncName(param0);                                                                                \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, "", #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_2(FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ScriptActorAlias* actor = SF_CHECK_ACTOR(L, 1)->As<ScriptActorAlias>();                                 \
        Datum param0; LuaObjectToDatum(L, 2, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 3, param1);                                                           \
        actor->FuncName(param0, param1);                                                                        \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, "", #FuncName, ScriptFunc_##FuncName);

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
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, "", #FuncName, ScriptFunc_##FuncName);

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
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, "", #FuncName, ScriptFunc_##FuncName);

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
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, "", #FuncName, ScriptFunc_##FuncName);

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
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, "", #FuncName, ScriptFunc_##FuncName);

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
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, "", #FuncName, ScriptFunc_##FuncName);

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
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, "", #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_R(FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ScriptActorAlias* actor = SF_CHECK_ACTOR(L, 1)->As<ScriptActorAlias>();                                 \
        Datum ret = actor->FuncName();                                                                          \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, "", #FuncName, ScriptFunc_##FuncName);

#define SCRIPT_FUNC_R_1(FuncName)                                                                               \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        ScriptActorAlias* actor = SF_CHECK_ACTOR(L, 1)->As<ScriptActorAlias>();                                 \
        Datum param0; LuaObjectToDatum(L, 2, param0);                                                           \
        Datum ret = actor->FuncName(param0);                                                                    \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, "", #FuncName, ScriptFunc_##FuncName);

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
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, "", #FuncName, ScriptFunc_##FuncName);

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
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, "", #FuncName, ScriptFunc_##FuncName);

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
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, "", #FuncName, ScriptFunc_##FuncName);

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
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, "", #FuncName, ScriptFunc_##FuncName);

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
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, "", #FuncName, ScriptFunc_##FuncName);

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
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, "", #FuncName, ScriptFunc_##FuncName);

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
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs, "", #FuncName, ScriptFunc_##FuncName);






#define GLOBAL_SCRIPT_FUNC(TableName, FuncName)                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        FuncName();                                                                                      \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(gAutoRegScriptFuncs, TableName, #FuncName, ScriptFunc_##FuncName);

#define GLOBAL_SCRIPT_FUNC_1(TableName, FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        Datum param0; LuaObjectToDatum(L, 1, param0);                                                           \
        FuncName(param0);                                                                                \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(gAutoRegScriptFuncs, TableName, #FuncName, ScriptFunc_##FuncName);

#define GLOBAL_SCRIPT_FUNC_2(TableName, FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        Datum param0; LuaObjectToDatum(L, 1, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 2, param1);                                                           \
        FuncName(param0, param1);                                                                        \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(gAutoRegScriptFuncs, TableName, #FuncName, ScriptFunc_##FuncName);

#define GLOBAL_SCRIPT_FUNC_3(TableName, FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        Datum param0; LuaObjectToDatum(L, 1, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 2, param1);                                                           \
        Datum param2; LuaObjectToDatum(L, 3, param2);                                                           \
        FuncName(param0, param1, param2);                                                                \
        return 0;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(gAutoRegScriptFuncs, TableName, #FuncName, ScriptFunc_##FuncName);

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
    static ScriptFuncAutoReg AutoReg_##FuncName(gAutoRegScriptFuncs, TableName, #FuncName, ScriptFunc_##FuncName);

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
    static ScriptFuncAutoReg AutoReg_##FuncName(gAutoRegScriptFuncs, TableName, #FuncName, ScriptFunc_##FuncName);

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
    static ScriptFuncAutoReg AutoReg_##FuncName(gAutoRegScriptFuncs, TableName, #FuncName, ScriptFunc_##FuncName);

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
    static ScriptFuncAutoReg AutoReg_##FuncName(gAutoRegScriptFuncs, TableName, #FuncName, ScriptFunc_##FuncName);

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
    static ScriptFuncAutoReg AutoReg_##FuncName(gAutoRegScriptFuncs, TableName, #FuncName, ScriptFunc_##FuncName);

#define GLOBAL_SCRIPT_FUNC_R(TableName, FuncName)                                                                                 \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        Datum ret = FuncName();                                                                          \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(gAutoRegScriptFuncs, TableName, #FuncName, ScriptFunc_##FuncName);

#define GLOBAL_SCRIPT_FUNC_R_1(TableName, FuncName)                                                                               \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        Datum param0; LuaObjectToDatum(L, 1, param0);                                                           \
        Datum ret = FuncName(param0);                                                                    \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(gAutoRegScriptFuncs, TableName, #FuncName, ScriptFunc_##FuncName);

#define GLOBAL_SCRIPT_FUNC_R_2(TableName, FuncName)                                                                               \
    static int ScriptFunc_##FuncName(lua_State* L)                                                              \
    {                                                                                                           \
        Datum param0; LuaObjectToDatum(L, 1, param0);                                                           \
        Datum param1; LuaObjectToDatum(L, 2, param1);                                                           \
        Datum ret = FuncName(param0, param1);                                                            \
        LuaPushDatum(L, ret);                                                                                   \
        return 1;                                                                                               \
    }                                                                                                           \
    static ScriptFuncAutoReg AutoReg_##FuncName(gAutoRegScriptFuncs, TableName, #FuncName, ScriptFunc_##FuncName);

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
    static ScriptFuncAutoReg AutoReg_##FuncName(gAutoRegScriptFuncs, TableName, #FuncName, ScriptFunc_##FuncName);

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
    static ScriptFuncAutoReg AutoReg_##FuncName(gAutoRegScriptFuncs, TableName, #FuncName, ScriptFunc_##FuncName);

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
    static ScriptFuncAutoReg AutoReg_##FuncName(gAutoRegScriptFuncs, TableName, #FuncName, ScriptFunc_##FuncName);

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
    static ScriptFuncAutoReg AutoReg_##FuncName(gAutoRegScriptFuncs, TableName, #FuncName, ScriptFunc_##FuncName);

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
    static ScriptFuncAutoReg AutoReg_##FuncName(gAutoRegScriptFuncs, TableName, #FuncName, ScriptFunc_##FuncName);

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
    static ScriptFuncAutoReg AutoReg_##FuncName(gAutoRegScriptFuncs, TableName, #FuncName, ScriptFunc_##FuncName);

// Actor Script Functions
#define ASF(FuncName)   SCRIPT_FUNC(FuncName)
#define ASF_1(FuncName) SCRIPT_FUNC_1(FuncName)
#define ASF_2(FuncName) SCRIPT_FUNC_2(FuncName)
#define ASF_3(FuncName) SCRIPT_FUNC_3(FuncName)
#define ASF_4(FuncName) SCRIPT_FUNC_4(FuncName)
#define ASF_5(FuncName) SCRIPT_FUNC_5(FuncName)
#define ASF_6(FuncName) SCRIPT_FUNC_6(FuncName)
#define ASF_7(FuncName) SCRIPT_FUNC_7(FuncName)
#define ASF_8(FuncName) SCRIPT_FUNC_8(FuncName)
#define ASF_R(FuncName)   SCRIPT_FUNC_R(FuncName)
#define ASF_R_1(FuncName) SCRIPT_FUNC_R_1(FuncName)
#define ASF_R_2(FuncName) SCRIPT_FUNC_R_2(FuncName)
#define ASF_R_3(FuncName) SCRIPT_FUNC_R_3(FuncName)
#define ASF_R_4(FuncName) SCRIPT_FUNC_R_4(FuncName)
#define ASF_R_5(FuncName) SCRIPT_FUNC_R_5(FuncName)
#define ASF_R_6(FuncName) SCRIPT_FUNC_R_6(FuncName)
#define ASF_R_7(FuncName) SCRIPT_FUNC_R_7(FuncName)
#define ASF_R_8(FuncName) SCRIPT_FUNC_R_8(FuncName)

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

