#pragma once

// Created this file so that Actor.h could use these type check functions

#include "Utilities.h"
#include "LuaBindings/LuaTypeCheck.h"

#define SF_CHECK_ACTOR(L, arg) CheckActorLuaType(L, arg, "Actor", "cfActor")

//Actor* CheckActorLuaType(lua_State* L, int arg, const char* className, const char* classFlag);

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

#define SCRIPT_FUNC(ClassName, FuncName)                             \
    static int ScriptFunc_##FuncName(lua_State* L)               \
    {                                                       \
        ClassName* actor = SF_CHECK_ACTOR(L, 1)->As<ClassName>();                \
        actor->FuncName();                                \
        return 0;                                           \
    } \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs_##ClassName, #FuncName, ScriptFunc_##FuncName);

//void ConvertLuaToCpp(lua_State* L, int idx, Datum& datum)

#define SCRIPT_FUNC_1(ClassName, FuncName)                        \
    static int ScriptFunc_##FuncName(lua_State* L)               \
    {                                                       \
        ClassName* actor = SF_CHECK_ACTOR(L, 1)->As<ClassName>();                \
        Datum param0; ConvertLuaToCpp(L, 2, param0);        \
        actor->FuncName(param0);                          \
        return 0;                                           \
    } \
    static ScriptFuncAutoReg AutoReg_##FuncName(sAutoRegs_##ClassName, #FuncName, ScriptFunc_##FuncName);

