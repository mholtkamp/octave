#pragma once

#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"

#include "Components/ParticleComponent.h"

#include "LuaBindings/Component_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define PARTICLE_COMPONENT_LUA_NAME "ParticleComponent"
#define PARTICLE_COMPONENT_LUA_FLAG "cfParticleComponent"
#define CHECK_PARTICLE_COMPONENT(L, arg) static_cast<ParticleComponent*>(CheckComponentLuaType(L, arg, PARTICLE_COMPONENT_LUA_NAME, PARTICLE_COMPONENT_LUA_FLAG));

struct ParticleComponent_Lua
{
    static int Reset(lua_State* L);
    static int EnableEmission(lua_State* L);
    static int IsEmissionEnabled(lua_State* L);
    static int GetElapsedTime(lua_State* L);
    static int SetParticleSystem(lua_State* L);
    static int GetParticleSystem(lua_State* L);
    static int SetMaterialOverride(lua_State* L);
    static int GetMaterial(lua_State* L);
    static int GetMaterialOverride(lua_State* L);
    static int SetTimeMultiplier(lua_State* L);
    static int GetTimeMultiplier(lua_State* L);
    static int SetUseLocalSpace(lua_State* L);
    static int GetUseLocalSpace(lua_State* L);
    static int GetNumParticles(lua_State* L);

    static void Bind();
};

#endif
