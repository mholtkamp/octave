#include "LuaBindings/ParticleSystem_Lua.h"
#include "LuaBindings/Asset_Lua.h"

#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Material_Lua.h"

#if LUA_ENABLED

int ParticleSystem_Lua::SetMaterial(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);
    Material* value = nullptr;
    if (!lua_isnil(L, 2)) { value = CHECK_MATERIAL(L, 2); }

    sys->SetMaterial(value);

    return 0;
}

int ParticleSystem_Lua::GetMaterial(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);

    Material* ret = sys->GetMaterial();

    Asset_Lua::Create(L, ret);
    return 1;
}

int ParticleSystem_Lua::SetDuration(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);
    float value = CHECK_NUMBER(L, 2);

    sys->SetDuration(value);

    return 0;
}

int ParticleSystem_Lua::GetDuration(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);

    float ret = sys->GetDuration();

    lua_pushnumber(L, ret);
    return 1;
}

int ParticleSystem_Lua::SetSpawnRate(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);
    float value = CHECK_NUMBER(L, 2);

    sys->SetSpawnRate(value);

    return 0;
}

int ParticleSystem_Lua::GetSpawnRate(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);

    float ret = sys->GetSpawnRate();

    lua_pushnumber(L, ret);
    return 1;
}

int ParticleSystem_Lua::SetBurstCount(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);
    uint32_t value = (uint32_t) CHECK_INTEGER(L, 2);

    sys->SetBurstCount(value);

    return 0;
}

int ParticleSystem_Lua::GetBurstCount(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);

    uint32_t ret = sys->GetBurstCount();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int ParticleSystem_Lua::SetBurstWindow(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);
    float value = CHECK_NUMBER(L, 2);

    sys->SetBurstWindow(value);

    return 0;
}

int ParticleSystem_Lua::GetBurstWindow(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);

    float ret = sys->GetBurstWindow();

    lua_pushnumber(L, ret);
    return 1;
}

int ParticleSystem_Lua::SetMaxParticles(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);
    uint32_t value = (uint32_t) CHECK_INTEGER(L, 2);

    sys->SetMaxParticles(value);

    return 0;
}

int ParticleSystem_Lua::GetMaxParticles(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);

    uint32_t ret = sys->GetMaxParticles();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int ParticleSystem_Lua::SetLoops(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);
    uint32_t value = (uint32_t) CHECK_INTEGER(L, 2);

    sys->SetLoops(value);

    return 0;
}

int ParticleSystem_Lua::GetLoops(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);

    uint32_t ret = sys->GetLoops();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int ParticleSystem_Lua::SetRadialVelocity(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    sys->SetRadialVelocity(value);

    return 0;
}

int ParticleSystem_Lua::IsRadialVelocity(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);

    bool ret = sys->IsRadialVelocity();

    lua_pushboolean(L, ret);
    return 1;
}

int ParticleSystem_Lua::SetRadialSpawn(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    sys->SetRadialSpawn(value);

    return 0;
}

int ParticleSystem_Lua::IsRadialSpawn(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);

    bool ret = sys->IsRadialSpawn();

    lua_pushboolean(L, ret);
    return 1;
}

int ParticleSystem_Lua::SetLockedRatio(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    sys->SetLockedRatio(value);

    return 0;
}

int ParticleSystem_Lua::IsRatioLocked(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);

    bool ret = sys->IsRatioLocked();

    lua_pushboolean(L, ret);
    return 1;
}

int ParticleSystem_Lua::SetLifetime(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);
    float minVal = CHECK_NUMBER(L, 2);
    float maxVal = minVal;
    if (!lua_isnone(L, 3)) { maxVal = CHECK_NUMBER(L, 3); }

    sys->GetParams().mLifetimeMin = minVal;
    sys->GetParams().mLifetimeMax = maxVal;

    return 0;
}

int ParticleSystem_Lua::GetLifetimeMin(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);

    float ret = sys->GetParams().mLifetimeMin;

    lua_pushnumber(L, ret);
    return 1;
}

int ParticleSystem_Lua::GetLifetimeMax(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);

    float ret = sys->GetParams().mLifetimeMax;

    lua_pushnumber(L, ret);
    return 1;
}

int ParticleSystem_Lua::SetPosition(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);
    glm::vec3 minVal = CHECK_VECTOR(L, 2);
    glm::vec3 maxVal = minVal;
    if (!lua_isnone(L, 3)) { maxVal = CHECK_VECTOR(L, 3); }

    sys->GetParams().mPositionMin = minVal;
    sys->GetParams().mPositionMax = maxVal;

    return 0;
}

int ParticleSystem_Lua::GetPositionMin(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);

    glm::vec3 ret = sys->GetParams().mPositionMin;

    Vector_Lua::Create(L, ret);
    return 1;
}

int ParticleSystem_Lua::GetPositionMax(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);

    glm::vec3 ret = sys->GetParams().mPositionMax;

    Vector_Lua::Create(L, ret);
    return 1;
}

int ParticleSystem_Lua::SetVelocity(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);
    glm::vec3 minVal = CHECK_VECTOR(L, 2);
    glm::vec3 maxVal = minVal;
    if (!lua_isnone(L, 3)) { maxVal = CHECK_VECTOR(L, 3); }

    sys->GetParams().mVelocityMin = minVal;
    sys->GetParams().mVelocityMax = maxVal;

    return 0;
}

int ParticleSystem_Lua::GetVelocityMin(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);

    glm::vec3 ret = sys->GetParams().mVelocityMin;

    Vector_Lua::Create(L, ret);
    return 1;
}

int ParticleSystem_Lua::GetVelocityMax(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);

    glm::vec3 ret = sys->GetParams().mVelocityMax;

    Vector_Lua::Create(L, ret);
    return 1;
}

int ParticleSystem_Lua::SetSize(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);
    glm::vec2 minVal = CHECK_VECTOR(L, 2);
    glm::vec2 maxVal = minVal;
    if (!lua_isnone(L, 3)) { maxVal = CHECK_VECTOR(L, 3); }

    sys->GetParams().mSizeMin = minVal;
    sys->GetParams().mSizeMax = maxVal;

    return 0;
}

int ParticleSystem_Lua::GetSizeMin(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);

    glm::vec2 ret = sys->GetParams().mSizeMin;

    Vector_Lua::Create(L, ret);
    return 1;
}

int ParticleSystem_Lua::GetSizeMax(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);

    glm::vec2 ret = sys->GetParams().mSizeMax;

    Vector_Lua::Create(L, ret);
    return 1;
}

int ParticleSystem_Lua::SetRotation(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);
    float minVal = CHECK_NUMBER(L, 2);
    float maxVal = minVal;
    if (!lua_isnone(L, 3)) { maxVal = CHECK_NUMBER(L, 3); }

    sys->GetParams().mRotationMin = minVal;
    sys->GetParams().mRotationMax = maxVal;

    return 0;
}

int ParticleSystem_Lua::GetRotationMin(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);

    float ret = sys->GetParams().mRotationMin;

    lua_pushnumber(L, ret);
    return 1;
}

int ParticleSystem_Lua::GetRotationMax(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);

    float ret = sys->GetParams().mRotationMax;

    lua_pushnumber(L, ret);
    return 1;
}

int ParticleSystem_Lua::SetRotationSpeed(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);
    float minVal = CHECK_NUMBER(L, 2);
    float maxVal = minVal;
    if (!lua_isnone(L, 3)) { maxVal = CHECK_NUMBER(L, 3); }

    sys->GetParams().mRotationSpeedMin = minVal;
    sys->GetParams().mRotationSpeedMax = maxVal;

    return 0;
}

int ParticleSystem_Lua::GetRotationSpeedMin(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);

    float ret = sys->GetParams().mRotationSpeedMin;

    lua_pushnumber(L, ret);
    return 1;
}

int ParticleSystem_Lua::GetRotationSpeedMax(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);

    float ret = sys->GetParams().mRotationSpeedMax;

    lua_pushnumber(L, ret);
    return 1;
}

int ParticleSystem_Lua::SetAcceleration(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);
    glm::vec3 value = CHECK_VECTOR(L, 2);

    sys->GetParams().mAcceleration = value;

    return 0;
}

int ParticleSystem_Lua::GetAcceleration(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);

    glm::vec3 ret = sys->GetParams().mAcceleration;

    Vector_Lua::Create(L, ret);
    return 1;
}

int ParticleSystem_Lua::SetAlphaEase(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);
    float value = CHECK_NUMBER(L, 2);

    sys->GetParams().mAlphaEase = value;

    return 0;
}

int ParticleSystem_Lua::GetAlphaEase(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);

    float ret = sys->GetParams().mAlphaEase;

    lua_pushnumber(L, ret);
    return 1;
}

int ParticleSystem_Lua::SetScaleEase(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);
    float value = CHECK_NUMBER(L, 2);

    sys->GetParams().mScaleEase = value;

    return 0;
}

int ParticleSystem_Lua::GetScaleEase(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);

    float ret = sys->GetParams().mScaleEase;

    lua_pushnumber(L, ret);
    return 1;
}

int ParticleSystem_Lua::SetColorStart(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);
    glm::vec4 value = CHECK_VECTOR(L, 2);

    sys->GetParams().mColorStart = value;

    return 0;
}

int ParticleSystem_Lua::GetColorStart(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);

    glm::vec4 ret = sys->GetParams().mColorStart;

    Vector_Lua::Create(L, ret);
    return 1;
}

int ParticleSystem_Lua::SetColorEnd(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);
    glm::vec4 value = CHECK_VECTOR(L, 2);

    sys->GetParams().mColorEnd = value;

    return 0;
}

int ParticleSystem_Lua::GetColorEnd(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);

    glm::vec4 ret = sys->GetParams().mColorEnd;

    Vector_Lua::Create(L, ret);
    return 1;
}

void ParticleSystem_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        PARTICLE_SYSTEM_LUA_NAME,
        PARTICLE_SYSTEM_LUA_FLAG,
        ASSET_LUA_NAME);

    Asset_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, SetMaterial);

    REGISTER_TABLE_FUNC(L, mtIndex, GetMaterial);

    REGISTER_TABLE_FUNC(L, mtIndex, SetDuration);

    REGISTER_TABLE_FUNC(L, mtIndex, GetDuration);

    REGISTER_TABLE_FUNC(L, mtIndex, SetSpawnRate);

    REGISTER_TABLE_FUNC(L, mtIndex, GetSpawnRate);

    REGISTER_TABLE_FUNC(L, mtIndex, SetBurstCount);

    REGISTER_TABLE_FUNC(L, mtIndex, GetBurstCount);

    REGISTER_TABLE_FUNC(L, mtIndex, SetBurstWindow);

    REGISTER_TABLE_FUNC(L, mtIndex, GetBurstWindow);

    REGISTER_TABLE_FUNC(L, mtIndex, SetMaxParticles);

    REGISTER_TABLE_FUNC(L, mtIndex, GetMaxParticles);

    REGISTER_TABLE_FUNC(L, mtIndex, SetLoops);

    REGISTER_TABLE_FUNC(L, mtIndex, GetLoops);

    REGISTER_TABLE_FUNC(L, mtIndex, SetRadialVelocity);

    REGISTER_TABLE_FUNC(L, mtIndex, IsRadialVelocity);

    REGISTER_TABLE_FUNC(L, mtIndex, SetRadialSpawn);

    REGISTER_TABLE_FUNC(L, mtIndex, IsRadialSpawn);

    REGISTER_TABLE_FUNC(L, mtIndex, SetLockedRatio);

    REGISTER_TABLE_FUNC(L, mtIndex, IsRatioLocked);

    REGISTER_TABLE_FUNC(L, mtIndex, SetLifetime);

    REGISTER_TABLE_FUNC(L, mtIndex, GetLifetimeMin);

    REGISTER_TABLE_FUNC(L, mtIndex, GetLifetimeMax);

    REGISTER_TABLE_FUNC(L, mtIndex, SetPosition);

    REGISTER_TABLE_FUNC(L, mtIndex, GetPositionMin);

    REGISTER_TABLE_FUNC(L, mtIndex, GetPositionMax);

    REGISTER_TABLE_FUNC(L, mtIndex, SetVelocity);

    REGISTER_TABLE_FUNC(L, mtIndex, GetVelocityMin);

    REGISTER_TABLE_FUNC(L, mtIndex, GetVelocityMax);

    REGISTER_TABLE_FUNC(L, mtIndex, SetSize);

    REGISTER_TABLE_FUNC(L, mtIndex, GetSizeMin);

    REGISTER_TABLE_FUNC(L, mtIndex, GetSizeMax);

    REGISTER_TABLE_FUNC(L, mtIndex, SetRotation);

    REGISTER_TABLE_FUNC(L, mtIndex, GetRotationMin);

    REGISTER_TABLE_FUNC(L, mtIndex, GetRotationMax);

    REGISTER_TABLE_FUNC(L, mtIndex, SetRotationSpeed);

    REGISTER_TABLE_FUNC(L, mtIndex, GetRotationSpeedMin);

    REGISTER_TABLE_FUNC(L, mtIndex, GetRotationSpeedMax);

    REGISTER_TABLE_FUNC(L, mtIndex, SetAcceleration);

    REGISTER_TABLE_FUNC(L, mtIndex, GetAcceleration);

    REGISTER_TABLE_FUNC(L, mtIndex, SetAlphaEase);

    REGISTER_TABLE_FUNC(L, mtIndex, GetAlphaEase);

    REGISTER_TABLE_FUNC(L, mtIndex, SetScaleEase);

    REGISTER_TABLE_FUNC(L, mtIndex, GetScaleEase);

    REGISTER_TABLE_FUNC(L, mtIndex, SetColorStart);

    REGISTER_TABLE_FUNC(L, mtIndex, GetColorStart);

    REGISTER_TABLE_FUNC(L, mtIndex, SetColorEnd);

    REGISTER_TABLE_FUNC(L, mtIndex, GetColorEnd);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
