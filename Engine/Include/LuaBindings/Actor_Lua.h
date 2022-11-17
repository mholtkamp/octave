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

    static int CreateComponent(lua_State* L);
    static int CloneComponent(lua_State* L);
    static int DestroyComponent(lua_State* L);
    
    static int GetName(lua_State* L);
    static int SetName(lua_State* L);
    static int Attach(lua_State* L);
    static int Detach(lua_State* L);
    static int SetPendingDestroy(lua_State* L);
    static int IsPendingDestroy(lua_State* L);
    static int EnableTick(lua_State* L);
    static int IsTickEnabled(lua_State* L);

    static int GetPosition(lua_State* L);
    static int GetRotationQuat(lua_State* L);
    static int GetRotationEuler(lua_State* L);
    static int GetScale(lua_State* L);

    static int SetPosition(lua_State* L);
    static int SetRotationQuat(lua_State* L);
    static int SetRotationEuler(lua_State* L);
    static int SetScale(lua_State* L);

    static int GetForwardVector(lua_State* L);
    static int GetRightVector(lua_State* L);
    static int GetUpVector(lua_State* L);

    static int SweepToPosition(lua_State* L);
    
    static int GetNetId(lua_State* L);
    static int GetOwningHost(lua_State* L);
    static int SetOwningHost(lua_State* L);

    static int SetReplicate(lua_State* L);
    static int IsReplicated(lua_State* L);
    static int ForceReplication(lua_State* L);

    static int GetComponent(lua_State* L);
    static int GetNumComponents(lua_State* L);
    static int AddComponent(lua_State* L);
    static int RemoveComponent(lua_State* L);
    
    static int SetRootComponent(lua_State* L);
    static int GetRootComponent(lua_State* L);

    static int UpdateComponentTransforms(lua_State* L);

    static int GetScript(lua_State* L);

    static int HasTag(lua_State* L);
    static int AddTag(lua_State* L);
    static int RemoveTag(lua_State* L);

    static int SetPersistent(lua_State* L);
    static int IsPersistent(lua_State* L);

    // TODO-LUA: Add calls to InvokeNetFunc()

    static void Bind();
};

#endif