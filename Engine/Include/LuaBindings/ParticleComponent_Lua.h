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
    static int EnableSimulation(lua_State* L);
    static int IsSimulationEnabled(lua_State* L);
    static int EnableAutoEmit(lua_State* L);
    static int IsAutoEmitEnabled(lua_State* L);
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

    static int SetParticleVelocity(lua_State* L);
    static int GetParticleVelocity(lua_State* L);
    static int SetParticlePosition(lua_State* L);
    static int GetParticlePosition(lua_State* L);
    static int SetParticleSpeed(lua_State* L);
    static int SetParticleOrientation(lua_State* L);
    static int GetParticleOrientation(lua_State* L);

    static int InstantiateParticleSystem(lua_State* L);

    static void Bind();
};

#endif
