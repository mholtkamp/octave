#pragma once

#include "Engine.h"
#include "ObjectRef.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define ACTOR_LUA_NAME "Actor"
#define ACTOR_LUA_FLAG "cfActor"
#define CHECK_ACTOR(L, arg) CheckActorLuaType(L, arg, ACTOR_LUA_NAME, ACTOR_LUA_FLAG);

struct Actor_Lua
{
#if LUA_SAFE_ACTOR
    ActorRef mActor;
#else
    Actor* mActor = nullptr;
#endif

    static int Create(lua_State* L, Actor* actor);
    static int Destroy(lua_State* L);

    static int IsValid(lua_State* L);












    

    static int GetScript(lua_State* L);










    static void BindCommon(lua_State* L, int mtIndex);
    static void Bind();
};

#endif