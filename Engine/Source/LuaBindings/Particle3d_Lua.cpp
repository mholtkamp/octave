#include "LuaBindings/Particle3d_Lua.h"
#include "LuaBindings/Primitive3d_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Material_Lua.h"
#include "LuaBindings/ParticleSystem_Lua.h"
#include "LuaBindings/LuaUtils.h"

#include "AssetManager.h"
#include "Assets/ParticleSystemInstance.h"

#if LUA_ENABLED

int Particle3D_Lua::Reset(lua_State* L)
{
    Particle3D* comp = CHECK_PARTICLE_3D(L, 1);

    comp->Reset();

    return 0;
}

int Particle3D_Lua::EnableEmission(lua_State* L)
{
    Particle3D* comp = CHECK_PARTICLE_3D(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    comp->EnableEmission(value);

    return 0;
}

int Particle3D_Lua::IsEmissionEnabled(lua_State* L)
{
    Particle3D* comp = CHECK_PARTICLE_3D(L, 1);

    bool ret = comp->IsEmissionEnabled();

    lua_pushboolean(L, ret);
    return 1;
}

int Particle3D_Lua::EnableSimulation(lua_State* L)
{
    Particle3D* comp = CHECK_PARTICLE_3D(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    comp->EnableSimulation(value);

    return 0;
}

int Particle3D_Lua::IsSimulationEnabled(lua_State* L)
{
    Particle3D* comp = CHECK_PARTICLE_3D(L, 1);

    bool ret = comp->IsSimulationEnabled();

    lua_pushboolean(L, ret);
    return 1;
}

int Particle3D_Lua::EnableAutoEmit(lua_State* L)
{
    Particle3D* comp = CHECK_PARTICLE_3D(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    comp->EnableAutoEmit(value);

    return 0;
}

int Particle3D_Lua::IsAutoEmitEnabled(lua_State* L)
{
    Particle3D* comp = CHECK_PARTICLE_3D(L, 1);

    bool ret = comp->IsAutoEmitEnabled();

    lua_pushboolean(L, ret);
    return 1;
}

int Particle3D_Lua::GetElapsedTime(lua_State* L)
{
    Particle3D* comp = CHECK_PARTICLE_3D(L, 1);

    float ret = comp->GetElapsedTime();

    lua_pushnumber(L, ret);
    return 1;
}

int Particle3D_Lua::SetParticleSystem(lua_State* L)
{
    Particle3D* comp = CHECK_PARTICLE_3D(L, 1);
    ParticleSystem* system = nullptr;
    if (!lua_isnil(L, 2)) { system = CHECK_PARTICLE_SYSTEM(L, 2); }

    comp->SetParticleSystem(system);

    return 0;
}

int Particle3D_Lua::GetParticleSystem(lua_State* L)
{
    Particle3D* comp = CHECK_PARTICLE_3D(L, 1);

    ParticleSystem* ret = comp->GetParticleSystem();

    Asset_Lua::Create(L, ret);
    return 1;
}

int Particle3D_Lua::SetMaterialOverride(lua_State* L)
{
    Particle3D* comp = CHECK_PARTICLE_3D(L, 1);
    Material* material = nullptr;
    if (!lua_isnil(L, 2)) { material = CHECK_MATERIAL(L, 2); }

    comp->SetMaterialOverride(material);

    return 0;
}

int Particle3D_Lua::GetMaterial(lua_State* L)
{
    Particle3D* comp = CHECK_PARTICLE_3D(L, 1);

    Material* ret = comp->GetMaterial();

    Asset_Lua::Create(L, ret);
    return 1;
}

int Particle3D_Lua::GetMaterialOverride(lua_State* L)
{
    Particle3D* comp = CHECK_PARTICLE_3D(L, 1);

    Material* ret = comp->GetMaterialOverride();

    Asset_Lua::Create(L, ret);
    return 1;
}

int Particle3D_Lua::SetTimeMultiplier(lua_State* L)
{
    Particle3D* comp = CHECK_PARTICLE_3D(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetTimeMultiplier(value);

    return 0;
}

int Particle3D_Lua::GetTimeMultiplier(lua_State* L)
{
    Particle3D* comp = CHECK_PARTICLE_3D(L, 1);

    float ret = comp->GetTimeMultiplier();

    lua_pushnumber(L, ret);
    return 1;
}

int Particle3D_Lua::SetUseLocalSpace(lua_State* L)
{
    Particle3D* comp = CHECK_PARTICLE_3D(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    comp->SetUseLocalSpace(value);

    return 0;
}

int Particle3D_Lua::GetUseLocalSpace(lua_State* L)
{
    Particle3D* comp = CHECK_PARTICLE_3D(L, 1);

    bool ret = comp->GetUseLocalSpace();

    lua_pushboolean(L, ret);
    return 1;
}

int Particle3D_Lua::GetNumParticles(lua_State* L)
{
    Particle3D* comp = CHECK_PARTICLE_3D(L, 1);

    uint32_t ret = comp->GetNumParticles();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int Particle3D_Lua::GetParticleData(lua_State* L)
{
    Particle3D* comp = CHECK_PARTICLE_3D(L, 1);
    int32_t index = CHECK_INDEX(L, 2);

    if (index >= 0 && index < int32_t(comp->GetNumParticles()))
    {
        const Particle& particleData = comp->GetParticles()[index];
        Datum dataTable;
        dataTable.SetColorField("position", (glm::vec4(particleData.mPosition, 0)));
        dataTable.SetColorField("velocity", (glm::vec4(particleData.mVelocity, 0)));
        dataTable.SetColorField("size", (glm::vec4(particleData.mSize, 0, 0)));
        dataTable.SetFloatField("elapsedTime", particleData.mElapsedTime);
        dataTable.SetFloatField("lifeTime", particleData.mLifetime);
        dataTable.SetFloatField("rotationSpeed", particleData.mRotationSpeed);
        dataTable.SetFloatField("rotation", particleData.mRotation);
        LuaPushDatum(L, dataTable);
    }
    else
    {
        lua_pushnil(L);
    }

    return 1;
}

int Particle3D_Lua::SetParticleData(lua_State* L)
{
    Particle3D* comp = CHECK_PARTICLE_3D(L, 1);
    int32_t index = CHECK_INDEX(L, 2);
    Datum dataTable = LuaObjectToDatum(L, 3);

    if (index >= 0 && index < int32_t(comp->GetNumParticles()))
    {
        Particle& particleData = comp->GetParticles()[index];

        if (dataTable.HasField("position"))
            particleData.mPosition = dataTable.GetColorField("position");

        if (dataTable.HasField("velocity"))
            particleData.mVelocity = dataTable.GetColorField("velocity");

        if (dataTable.HasField("size"))
            particleData.mSize = dataTable.GetColorField("size");
        
        if (dataTable.HasField("elapsedTime"))
            particleData.mElapsedTime = dataTable.GetFloatField("elapsedTime");

        if (dataTable.HasField("lifeTime"))
            particleData.mLifetime = dataTable.GetFloatField("lifeTime");

        if (dataTable.HasField("rotationSpeed"))
            particleData.mRotationSpeed = dataTable.GetFloatField("rotationSpeed");

        if (dataTable.HasField("rotation"))
            particleData.mRotation = dataTable.GetFloatField("rotation");
    }

    return 0;
}

int Particle3D_Lua::SetParticleOrientation(lua_State* L)
{
    Particle3D* comp = CHECK_PARTICLE_3D(L, 1);
    ParticleOrientation value = (ParticleOrientation)CHECK_INTEGER(L, 2);

    comp->SetParticleOrientation(value);

    return 0;
}

int Particle3D_Lua::GetParticleOrientation(lua_State* L)
{
    Particle3D* comp = CHECK_PARTICLE_3D(L, 1);

    int32_t ret = (int32_t)comp->GetParticleOrientation();

    lua_pushinteger(L, ret);
    return 1;
}

int Particle3D_Lua::EnableAutoDestroy(lua_State* L)
{
    Particle3D* comp = CHECK_PARTICLE_3D(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    comp->EnableAutoDestroy(value);

    return 0;
}

int Particle3D_Lua::InstantiateParticleSystem(lua_State* L)
{
    Particle3D* comp = CHECK_PARTICLE_3D(L, 1);

    ParticleSystemInstance* ret = comp->InstantiateParticleSystem();

    Asset_Lua::Create(L, ret);
    return 1;
}

void Particle3D_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        PARTICLE_3D_LUA_NAME,
        PARTICLE_3D_LUA_FLAG,
        PRIMITIVE_3D_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, Reset);

    REGISTER_TABLE_FUNC(L, mtIndex, EnableEmission);

    REGISTER_TABLE_FUNC(L, mtIndex, IsEmissionEnabled);

    REGISTER_TABLE_FUNC(L, mtIndex, EnableSimulation);

    REGISTER_TABLE_FUNC(L, mtIndex, IsSimulationEnabled);

    REGISTER_TABLE_FUNC(L, mtIndex, GetElapsedTime);

    REGISTER_TABLE_FUNC(L, mtIndex, SetParticleSystem);

    REGISTER_TABLE_FUNC(L, mtIndex, GetParticleSystem);

    REGISTER_TABLE_FUNC(L, mtIndex, SetMaterialOverride);

    REGISTER_TABLE_FUNC(L, mtIndex, GetMaterial);

    REGISTER_TABLE_FUNC(L, mtIndex, GetMaterialOverride);

    REGISTER_TABLE_FUNC(L, mtIndex, SetTimeMultiplier);

    REGISTER_TABLE_FUNC(L, mtIndex, GetTimeMultiplier);

    REGISTER_TABLE_FUNC(L, mtIndex, SetUseLocalSpace);

    REGISTER_TABLE_FUNC(L, mtIndex, GetUseLocalSpace);

    REGISTER_TABLE_FUNC(L, mtIndex, GetNumParticles);

    REGISTER_TABLE_FUNC(L, mtIndex, GetParticleData);

    REGISTER_TABLE_FUNC(L, mtIndex, SetParticleData);

    REGISTER_TABLE_FUNC(L, mtIndex, EnableAutoEmit);

    REGISTER_TABLE_FUNC(L, mtIndex, IsAutoEmitEnabled);

    REGISTER_TABLE_FUNC(L, mtIndex, SetParticleOrientation);

    REGISTER_TABLE_FUNC(L, mtIndex, GetParticleOrientation);

    REGISTER_TABLE_FUNC(L, mtIndex, EnableAutoDestroy);

    REGISTER_TABLE_FUNC(L, mtIndex, InstantiateParticleSystem);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
