#pragma once

#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"

#include "Nodes/3D/Particle3d.h"

#include "LuaBindings/Node_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define PARTICLE_3D_LUA_NAME "Particle3D"
#define PARTICLE_3D_LUA_FLAG "cfParticle3D"
#define CHECK_PARTICLE_3D(L, arg) static_cast<Particle3D*>(CheckNodeLuaType(L, arg, PARTICLE_3D_LUA_NAME, PARTICLE_3D_LUA_FLAG));

struct Particle3D_Lua
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

    static int GetParticleData(lua_State* L);
    static int SetParticleData(lua_State* L);
    
    static int SetParticleOrientation(lua_State* L);
    static int GetParticleOrientation(lua_State* L);

    static int EnableAutoDestroy(lua_State* L);

    static int InstantiateParticleSystem(lua_State* L);

    static void Bind();
};

#endif
