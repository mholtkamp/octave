#include "LuaBindings/ParticleComponent_Lua.h"
#include "LuaBindings/PrimitiveComponent_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Material_Lua.h"
#include "LuaBindings/ParticleSystem_Lua.h"
#include "LuaBindings/LuaUtils.h"

#include "AssetManager.h"
#include "Assets/ParticleSystemInstance.h"

#if LUA_ENABLED

int ParticleComponent_Lua::Reset(lua_State* L)
{
    ParticleComponent* comp = CHECK_PARTICLE_COMPONENT(L, 1);

    comp->Reset();

    return 0;
}

int ParticleComponent_Lua::EnableEmission(lua_State* L)
{
    ParticleComponent* comp = CHECK_PARTICLE_COMPONENT(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    comp->EnableEmission(value);

    return 0;
}

int ParticleComponent_Lua::IsEmissionEnabled(lua_State* L)
{
    ParticleComponent* comp = CHECK_PARTICLE_COMPONENT(L, 1);

    bool ret = comp->IsEmissionEnabled();

    lua_pushboolean(L, ret);
    return 1;
}

int ParticleComponent_Lua::EnableSimulation(lua_State* L)
{
    ParticleComponent* comp = CHECK_PARTICLE_COMPONENT(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    comp->EnableSimulation(value);

    return 0;
}

int ParticleComponent_Lua::IsSimulationEnabled(lua_State* L)
{
    ParticleComponent* comp = CHECK_PARTICLE_COMPONENT(L, 1);

    bool ret = comp->IsSimulationEnabled();

    lua_pushboolean(L, ret);
    return 1;
}

int ParticleComponent_Lua::EnableAutoEmit(lua_State* L)
{
    ParticleComponent* comp = CHECK_PARTICLE_COMPONENT(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    comp->EnableAutoEmit(value);

    return 0;
}

int ParticleComponent_Lua::IsAutoEmitEnabled(lua_State* L)
{
    ParticleComponent* comp = CHECK_PARTICLE_COMPONENT(L, 1);

    bool ret = comp->IsAutoEmitEnabled();

    lua_pushboolean(L, ret);
    return 1;
}

int ParticleComponent_Lua::GetElapsedTime(lua_State* L)
{
    ParticleComponent* comp = CHECK_PARTICLE_COMPONENT(L, 1);

    float ret = comp->GetElapsedTime();

    lua_pushnumber(L, ret);
    return 1;
}

int ParticleComponent_Lua::SetParticleSystem(lua_State* L)
{
    ParticleComponent* comp = CHECK_PARTICLE_COMPONENT(L, 1);
    ParticleSystem* system = nullptr;
    if (!lua_isnil(L, 2)) { system = CHECK_PARTICLE_SYSTEM(L, 2); }

    comp->SetParticleSystem(system);

    return 0;
}

int ParticleComponent_Lua::GetParticleSystem(lua_State* L)
{
    ParticleComponent* comp = CHECK_PARTICLE_COMPONENT(L, 1);

    ParticleSystem* ret = comp->GetParticleSystem();

    Asset_Lua::Create(L, ret);
    return 1;
}

int ParticleComponent_Lua::SetMaterialOverride(lua_State* L)
{
    ParticleComponent* comp = CHECK_PARTICLE_COMPONENT(L, 1);
    Material* material = nullptr;
    if (!lua_isnil(L, 2)) { material = CHECK_MATERIAL(L, 2); }

    comp->SetMaterialOverride(material);

    return 0;
}

int ParticleComponent_Lua::GetMaterial(lua_State* L)
{
    ParticleComponent* comp = CHECK_PARTICLE_COMPONENT(L, 1);

    Material* ret = comp->GetMaterial();

    Asset_Lua::Create(L, ret);
    return 1;
}

int ParticleComponent_Lua::GetMaterialOverride(lua_State* L)
{
    ParticleComponent* comp = CHECK_PARTICLE_COMPONENT(L, 1);

    Material* ret = comp->GetMaterialOverride();

    Asset_Lua::Create(L, ret);
    return 1;
}

int ParticleComponent_Lua::SetTimeMultiplier(lua_State* L)
{
    ParticleComponent* comp = CHECK_PARTICLE_COMPONENT(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetTimeMultiplier(value);

    return 0;
}

int ParticleComponent_Lua::GetTimeMultiplier(lua_State* L)
{
    ParticleComponent* comp = CHECK_PARTICLE_COMPONENT(L, 1);

    float ret = comp->GetTimeMultiplier();

    lua_pushnumber(L, ret);
    return 1;
}

int ParticleComponent_Lua::SetUseLocalSpace(lua_State* L)
{
    ParticleComponent* comp = CHECK_PARTICLE_COMPONENT(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    comp->SetUseLocalSpace(value);

    return 0;
}

int ParticleComponent_Lua::GetUseLocalSpace(lua_State* L)
{
    ParticleComponent* comp = CHECK_PARTICLE_COMPONENT(L, 1);

    bool ret = comp->GetUseLocalSpace();

    lua_pushboolean(L, ret);
    return 1;
}

int ParticleComponent_Lua::GetNumParticles(lua_State* L)
{
    ParticleComponent* comp = CHECK_PARTICLE_COMPONENT(L, 1);

    uint32_t ret = comp->GetNumParticles();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int ParticleComponent_Lua::SetParticleVelocity(lua_State* L)
{
    ParticleComponent* comp = CHECK_PARTICLE_COMPONENT(L, 1);
    int32_t index = CHECK_INTEGER(L, 2);
    glm::vec3 velocity = CHECK_VECTOR(L, 3);

    comp->SetParticleVelocity(index, velocity);

    return 0;
}

int ParticleComponent_Lua::GetParticleVelocity(lua_State* L)
{
    ParticleComponent* comp = CHECK_PARTICLE_COMPONENT(L, 1);
    int32_t index = CHECK_INTEGER(L, 2);

    glm::vec3 ret = comp->GetParticleVelocity(index);

    Vector_Lua::Create(L, ret);
    return 1;
}

int ParticleComponent_Lua::SetParticlePosition(lua_State* L)
{
    ParticleComponent* comp = CHECK_PARTICLE_COMPONENT(L, 1);
    int32_t index = CHECK_INTEGER(L, 2);
    glm::vec3 position = CHECK_VECTOR(L, 3);

    comp->SetParticlePosition(index, position);

    return 0;
}

int ParticleComponent_Lua::GetParticlePosition(lua_State* L)
{
    ParticleComponent* comp = CHECK_PARTICLE_COMPONENT(L, 1);
    int32_t index = CHECK_INTEGER(L, 2);

    glm::vec3 ret = comp->GetParticlePosition(index);

    Vector_Lua::Create(L, ret);
    return 1;
}

int ParticleComponent_Lua::SetParticleSpeed(lua_State* L)
{
    ParticleComponent* comp = CHECK_PARTICLE_COMPONENT(L, 1);
    int32_t index = CHECK_INTEGER(L, 2);
    float speed = CHECK_NUMBER(L, 3);

    comp->SetParticleSpeed(index, speed);

    return 0;
}

int ParticleComponent_Lua::InstantiateParticleSystem(lua_State* L)
{
    ParticleComponent* comp = CHECK_PARTICLE_COMPONENT(L, 1);

    ParticleSystemInstance* ret = comp->InstantiateParticleSystem();

    Asset_Lua::Create(L, ret);
    return 1;
}

void ParticleComponent_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        PARTICLE_COMPONENT_LUA_NAME,
        PARTICLE_COMPONENT_LUA_FLAG,
        PRIMITIVE_COMPONENT_LUA_NAME);

    Component_Lua::BindCommon(L, mtIndex);

    lua_pushcfunction(L, Reset);
    lua_setfield(L, mtIndex, "Reset");

    lua_pushcfunction(L, EnableEmission);
    lua_setfield(L, mtIndex, "EnableEmission");

    lua_pushcfunction(L, IsEmissionEnabled);
    lua_setfield(L, mtIndex, "IsEmissionEnabled");

    lua_pushcfunction(L, EnableSimulation);
    lua_setfield(L, mtIndex, "EnableSimulation");

    lua_pushcfunction(L, IsSimulationEnabled);
    lua_setfield(L, mtIndex, "IsSimulationEnabled");

    lua_pushcfunction(L, GetElapsedTime);
    lua_setfield(L, mtIndex, "GetElapsedTime");

    lua_pushcfunction(L, SetParticleSystem);
    lua_setfield(L, mtIndex, "SetParticleSystem");

    lua_pushcfunction(L, GetParticleSystem);
    lua_setfield(L, mtIndex, "GetParticleSystem");

    lua_pushcfunction(L, SetMaterialOverride);
    lua_setfield(L, mtIndex, "SetMaterialOverride");

    lua_pushcfunction(L, GetMaterial);
    lua_setfield(L, mtIndex, "GetMaterial");

    lua_pushcfunction(L, GetMaterialOverride);
    lua_setfield(L, mtIndex, "GetMaterialOverride");

    lua_pushcfunction(L, SetTimeMultiplier);
    lua_setfield(L, mtIndex, "SetTimeMultiplier");

    lua_pushcfunction(L, GetTimeMultiplier);
    lua_setfield(L, mtIndex, "GetTimeMultiplier");

    lua_pushcfunction(L, SetUseLocalSpace);
    lua_setfield(L, mtIndex, "SetUseLocalSpace");

    lua_pushcfunction(L, GetUseLocalSpace);
    lua_setfield(L, mtIndex, "GetUseLocalSpace");

    lua_pushcfunction(L, GetNumParticles);
    lua_setfield(L, mtIndex, "GetNumParticles");

    lua_pushcfunction(L, SetParticleVelocity);
    lua_setfield(L, mtIndex, "SetParticleVelocity");

    lua_pushcfunction(L, GetParticleVelocity);
    lua_setfield(L, mtIndex, "GetParticleVelocity");

    lua_pushcfunction(L, SetParticlePosition);
    lua_setfield(L, mtIndex, "SetParticlePosition");

    lua_pushcfunction(L, GetParticlePosition);
    lua_setfield(L, mtIndex, "GetParticlePosition");

    lua_pushcfunction(L, SetParticleSpeed);
    lua_setfield(L, mtIndex, "SetParticleSpeed");

    lua_pushcfunction(L, EnableAutoEmit);
    lua_setfield(L, mtIndex, "EnableAutoEmit");

    lua_pushcfunction(L, IsAutoEmitEnabled);
    lua_setfield(L, mtIndex, "IsAutoEmitEnabled");

    lua_pushcfunction(L, InstantiateParticleSystem);
    lua_setfield(L, mtIndex, "InstantiateParticleSystem");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
