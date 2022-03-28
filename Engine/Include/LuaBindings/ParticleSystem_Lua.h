#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Assets/ParticleSystem.h"

#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define PARTICLE_SYSTEM_LUA_NAME "ParticleSystem"
#define PARTICLE_SYSTEM_LUA_FLAG "cfParticleSystem"
#define CHECK_PARTICLE_SYSTEM(L, arg) CheckAssetLuaType<ParticleSystem>(L, arg, PARTICLE_SYSTEM_LUA_NAME, PARTICLE_SYSTEM_LUA_FLAG)

struct ParticleSystem_Lua
{
    static int SetMaterial(lua_State* L);
    static int GetMaterial(lua_State* L);
    static int SetDuration(lua_State* L);
    static int GetDuration(lua_State* L);
    static int SetSpawnRate(lua_State* L);
    static int GetSpawnRate(lua_State* L);
    static int SetBurstCount(lua_State* L);
    static int GetBurstCount(lua_State* L);
    static int SetBurstWindow(lua_State* L);
    static int GetBurstWindow(lua_State* L);
    static int SetMaxParticles(lua_State* L);
    static int GetMaxParticles(lua_State* L);
    static int SetLoops(lua_State* L);
    static int GetLoops(lua_State* L);
    static int SetRadial(lua_State* L);
    static int IsRadial(lua_State* L);
    static int SetLockedRatio(lua_State* L);
    static int IsRatioLocked(lua_State* L);

    static int SetLifetime(lua_State* L);
    static int GetLifetimeMin(lua_State* L);
    static int GetLifetimeMax(lua_State* L);
    static int SetPosition(lua_State* L);
    static int GetPositionMin(lua_State* L);
    static int GetPositionMax(lua_State* L);
    static int SetVelocity(lua_State* L);
    static int GetVelocityMin(lua_State* L);
    static int GetVelocityMax(lua_State* L);
    static int SetSize(lua_State* L);
    static int GetSizeMin(lua_State* L);
    static int GetSizeMax(lua_State* L);
    static int SetRotation(lua_State* L);
    static int GetRotationMin(lua_State* L);
    static int GetRotationMax(lua_State* L);
    static int SetRotationSpeed(lua_State* L);
    static int GetRotationSpeedMin(lua_State* L);
    static int GetRotationSpeedMax(lua_State* L);
    static int SetAcceleration(lua_State* L);
    static int GetAcceleration(lua_State* L);
    static int SetAlphaEase(lua_State* L);
    static int GetAlphaEase(lua_State* L);
    static int SetScaleEase(lua_State* L);
    static int GetScaleEase(lua_State* L);
    static int SetColorStart(lua_State* L);
    static int GetColorStart(lua_State* L);
    static int SetColorEnd(lua_State* L);
    static int GetColorEnd(lua_State* L);

    static void Bind();
};

#endif
