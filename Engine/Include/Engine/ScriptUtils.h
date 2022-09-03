#pragma once

// Created this file so that Actor.h could use these type check functions

#include "Utilities.h"
#include "LuaBindings/LuaTypeCheck.h"

#define SF_CHECK_ACTOR(L, arg) CheckActorLuaType(L, arg, "Actor", "cfActor");

Actor* CheckActorLuaType(lua_State* L, int arg, const char* className, const char* classFlag);


#define SCRIPT_FUNC(NativeName)                             \
    int ScriptFunc_##NativeName(lua_State* L)               \
    {                                                       \
        Actor* actor = SF_CHECK_ACTOR(L, 1);                \
        actor->NativeName();                                \
        return 0;                                           \
    }

//void ConvertLuaToCpp(lua_State* L, int idx, Datum& datum)

#define SCRIPT_FUNC_1(NativeName)                         \
    int ScriptFunc_##NativeName(lua_State* L)               \
    {                                                       \
        Actor* actor = SF_CHECK_ACTOR(L, 1);                \
        Datum param0; ConvertLuaToCpp(L, 2, param0);        \
        actor->NativeName(param0);                          \
        return 0;                                           \
    }

