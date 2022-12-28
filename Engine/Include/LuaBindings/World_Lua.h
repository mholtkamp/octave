#pragma once

#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"
#include "World.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define WORLD_LUA_NAME "World"
#define CHECK_WORLD(L, arg) static_cast<World*>(CheckLuaType<World_Lua>(L, arg, WORLD_LUA_NAME)->mWorld);

struct World_Lua
{
    World* mWorld = nullptr;

    static int Create(lua_State* L, World* world);

    static int GetActiveCamera(lua_State* L);
    static int GetAudioReceiver(lua_State* L);
    static int GetDirectionalLight(lua_State* L);

    static int SetActiveCamera(lua_State* L);
    static int SetAudioReceiver(lua_State* L);
    static int SetDirectionalLight(lua_State* L);

    static int SpawnActor(lua_State* L);
    static int CloneActor(lua_State* L);
    static int DestroyActor(lua_State* L);
    static int DestroyAllActors(lua_State* L);
    static int FindActor(lua_State* L);
    static int FindActorsByTag(lua_State* L);
    static int FindActorsByName(lua_State* L);
    static int FindComponent(lua_State* L);
    static int PrioritizeActorTick(lua_State* L);

    static int SetAmbientLightColor(lua_State* L);
    static int GetAmbientLightColor(lua_State* L);
    static int SetShadowColor(lua_State* L);
    static int GetShadowColor(lua_State* L);
    static int SetFogSettings(lua_State* L);
    static int GetFogSettings(lua_State* L);

    static int SetGravity(lua_State* L);
    static int GetGravity(lua_State* L);

    static int RayTest(lua_State* L);
    static int RayTestMulti(lua_State* L);
    static int SweepTest(lua_State* L);

    static int GetLoadedLevels(lua_State* L);
    static int UnloadAllLevels(lua_State* L);

    static int SpawnBlueprint(lua_State* L);
    static int LoadLevel(lua_State* L);
    static int QueueLevelLoad(lua_State* L);
    static int UnloadLevel(lua_State* L);

    static int EnableInternalEdgeSmoothing(lua_State* L);
    static int IsInternalEdgeSmoothingEnabled(lua_State* L);

    static int SpawnParticle(lua_State* L);

    static void Bind();
};

#endif
