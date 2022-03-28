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

int ParticleSystem_Lua::SetRadial(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    sys->SetRadial(value);

    return 0;
}

int ParticleSystem_Lua::IsRadial(lua_State* L)
{
    ParticleSystem* sys = CHECK_PARTICLE_SYSTEM(L, 1);

    bool ret = sys->IsRadial();

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

    lua_pushcfunction(L, SetMaterial);
    lua_setfield(L, mtIndex, "SetMaterial");

    lua_pushcfunction(L, GetMaterial);
    lua_setfield(L, mtIndex, "GetMaterial");

    lua_pushcfunction(L, SetDuration);
    lua_setfield(L, mtIndex, "SetDuration");

    lua_pushcfunction(L, GetDuration);
    lua_setfield(L, mtIndex, "GetDuration");

    lua_pushcfunction(L, SetSpawnRate);
    lua_setfield(L, mtIndex, "SetSpawnRate");

    lua_pushcfunction(L, GetSpawnRate);
    lua_setfield(L, mtIndex, "GetSpawnRate");

    lua_pushcfunction(L, SetBurstCount);
    lua_setfield(L, mtIndex, "SetBurstCount");

    lua_pushcfunction(L, GetBurstCount);
    lua_setfield(L, mtIndex, "GetBurstCount");

    lua_pushcfunction(L, SetBurstWindow);
    lua_setfield(L, mtIndex, "SetBurstWindow");

    lua_pushcfunction(L, GetBurstWindow);
    lua_setfield(L, mtIndex, "GetBurstWindow");

    lua_pushcfunction(L, SetMaxParticles);
    lua_setfield(L, mtIndex, "SetMaxParticles");

    lua_pushcfunction(L, GetMaxParticles);
    lua_setfield(L, mtIndex, "GetMaxParticles");

    lua_pushcfunction(L, SetLoops);
    lua_setfield(L, mtIndex, "SetLoops");

    lua_pushcfunction(L, GetLoops);
    lua_setfield(L, mtIndex, "GetLoops");

    lua_pushcfunction(L, SetRadial);
    lua_setfield(L, mtIndex, "SetRadial");

    lua_pushcfunction(L, IsRadial);
    lua_setfield(L, mtIndex, "IsRadial");

    lua_pushcfunction(L, SetLockedRatio);
    lua_setfield(L, mtIndex, "SetLockedRatio");

    lua_pushcfunction(L, IsRatioLocked);
    lua_setfield(L, mtIndex, "IsRatioLocked");

    lua_pushcfunction(L, SetLifetime);
    lua_setfield(L, mtIndex, "SetLifetime");

    lua_pushcfunction(L, GetLifetimeMin);
    lua_setfield(L, mtIndex, "GetLifetimeMin");

    lua_pushcfunction(L, GetLifetimeMax);
    lua_setfield(L, mtIndex, "GetLifetimeMax");

    lua_pushcfunction(L, SetPosition);
    lua_setfield(L, mtIndex, "SetPosition");

    lua_pushcfunction(L, GetPositionMin);
    lua_setfield(L, mtIndex, "GetPositionMin");

    lua_pushcfunction(L, GetPositionMax);
    lua_setfield(L, mtIndex, "GetPositionMax");

    lua_pushcfunction(L, SetVelocity);
    lua_setfield(L, mtIndex, "SetVelocity");

    lua_pushcfunction(L, GetVelocityMin);
    lua_setfield(L, mtIndex, "GetVelocityMin");

    lua_pushcfunction(L, GetVelocityMax);
    lua_setfield(L, mtIndex, "GetVelocityMax");

    lua_pushcfunction(L, SetSize);
    lua_setfield(L, mtIndex, "SetSize");

    lua_pushcfunction(L, GetSizeMin);
    lua_setfield(L, mtIndex, "GetSizeMin");

    lua_pushcfunction(L, GetSizeMax);
    lua_setfield(L, mtIndex, "GetSizeMax");

    lua_pushcfunction(L, SetRotation);
    lua_setfield(L, mtIndex, "SetRotation");

    lua_pushcfunction(L, GetRotationMin);
    lua_setfield(L, mtIndex, "GetRotationMin");

    lua_pushcfunction(L, GetRotationMax);
    lua_setfield(L, mtIndex, "GetRotationMax");

    lua_pushcfunction(L, SetRotationSpeed);
    lua_setfield(L, mtIndex, "SetRotationSpeed");

    lua_pushcfunction(L, GetRotationSpeedMin);
    lua_setfield(L, mtIndex, "GetRotationSpeedMin");

    lua_pushcfunction(L, GetRotationSpeedMax);
    lua_setfield(L, mtIndex, "GetRotationSpeedMax");

    lua_pushcfunction(L, SetAcceleration);
    lua_setfield(L, mtIndex, "SetAcceleration");

    lua_pushcfunction(L, GetAcceleration);
    lua_setfield(L, mtIndex, "GetAcceleration");

    lua_pushcfunction(L, SetAlphaEase);
    lua_setfield(L, mtIndex, "SetAlphaEase");

    lua_pushcfunction(L, GetAlphaEase);
    lua_setfield(L, mtIndex, "GetAlphaEase");

    lua_pushcfunction(L, SetScaleEase);
    lua_setfield(L, mtIndex, "SetScaleEase");

    lua_pushcfunction(L, GetScaleEase);
    lua_setfield(L, mtIndex, "GetScaleEase");

    lua_pushcfunction(L, SetColorStart);
    lua_setfield(L, mtIndex, "SetColorStart");

    lua_pushcfunction(L, GetColorStart);
    lua_setfield(L, mtIndex, "GetColorStart");

    lua_pushcfunction(L, SetColorEnd);
    lua_setfield(L, mtIndex, "SetColorEnd");

    lua_pushcfunction(L, GetColorEnd);
    lua_setfield(L, mtIndex, "GetColorEnd");

    lua_pop(L, 1);
    assert(lua_gettop(L) == 0);
}

#endif
