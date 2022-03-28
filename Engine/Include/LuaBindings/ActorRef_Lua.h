#pragma once

#include "Engine.h"
#include "ObjectRef.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define ACTOR_REF_LUA_NAME "ActorRef"
#define CHECK_ACTOR_REF(L, arg) CheckLuaType<ActorRef_Lua>(L, arg, ACTOR_REF_LUA_NAME)->mActorRef;

struct ActorRef_Lua
{
    ActorRef mActorRef;

    static int Create(lua_State* L);
    static int Destroy(lua_State* L);
    static int Get(lua_State* L);
    static int Set(lua_State* L);

    static void Bind();
};

#endif