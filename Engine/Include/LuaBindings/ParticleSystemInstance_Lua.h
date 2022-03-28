#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Assets/ParticleSystemInstance.h"

#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define PARTICLE_SYSTEM_INSTANCE_LUA_NAME "ParticleSystemInstance"
#define PARTICLE_SYSTEM_INSTANCE_LUA_FLAG "cfParticleSystemInstance"
#define CHECK_PARTICLE_SYSTEM_INSTANCE(L, arg) CheckAssetLuaType<ParticleSystemInstance>(L, arg, PARTICLE_SYSTEM_INSTANCE_LUA_NAME, PARTICLE_SYSTEM_INSTANCE_LUA_FLAG)

struct ParticleSystemInstance_Lua
{
    static int CreateNew(lua_State* L);

    static void Bind();
};

#endif
