#include "LuaBindings/ParticleSystemInstance_Lua.h"
#include "LuaBindings/ParticleSystem_Lua.h"

#include "LuaBindings/Vector_Lua.h"

#if LUA_ENABLED

int ParticleSystemInstance_Lua::CreateNew(lua_State* L)
{
    ParticleSystem* src = nullptr;
    if (!lua_isnone(L, 1)) { src = CHECK_PARTICLE_SYSTEM(L, 1); }

    ParticleSystemInstance* ret = ParticleSystemInstance::New(src);

    Asset_Lua::Create(L, ret);
    return 1;
}

void ParticleSystemInstance_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        PARTICLE_SYSTEM_INSTANCE_LUA_NAME,
        PARTICLE_SYSTEM_INSTANCE_LUA_FLAG,
        PARTICLE_SYSTEM_LUA_NAME);

    lua_pushcfunction(L, CreateNew);
    lua_setfield(L, mtIndex, "Create");

    Asset_Lua::BindCommon(L, mtIndex);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
