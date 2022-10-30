#include "Maths.h"
#include "Engine.h"

#include "LuaBindings/LuaUtils.h"
#include "LuaBindings/Maths_Lua.h"
#include "LuaBindings/Vector_Lua.h"

#if LUA_ENABLED

int Maths_Lua::Damp(lua_State* L)
{
    if (lua_isnumber(L, 1))
    {
        float source = CHECK_NUMBER(L, 1);
        float target = CHECK_NUMBER(L, 2);
        float smoothing = CHECK_NUMBER(L, 3);
        float deltaTime = CHECK_NUMBER(L, 4);

        float ret = Maths::Damp(source, target, smoothing, deltaTime);

        lua_pushnumber(L, ret);
        return 1;
    }
    else
    {
        glm::vec4 source = CHECK_VECTOR(L, 1);
        glm::vec4 target = CHECK_VECTOR(L, 2);
        float smoothing = CHECK_NUMBER(L, 3);
        float deltaTime = CHECK_NUMBER(L, 4);

        glm::vec4 ret = Maths::Damp(source, target, smoothing, deltaTime);

        Vector_Lua::Create(L, ret);
        return 1;
    }
}

int Maths_Lua::DampAngle(lua_State* L)
{
    float source = CHECK_NUMBER(L, 1);
    float target = CHECK_NUMBER(L, 2);
    float smoothing = CHECK_NUMBER(L, 3);
    float deltaTime = CHECK_NUMBER(L, 4);

    float ret = Maths::DampAngle(source, target, smoothing, deltaTime);

    lua_pushnumber(L, ret);
    return 1;
}

int Maths_Lua::Approach(lua_State* L)
{
    float source = CHECK_NUMBER(L, 1);
    float target = CHECK_NUMBER(L, 2);
    float speed = CHECK_NUMBER(L, 3);
    float deltaTime = CHECK_NUMBER(L, 4);

    float ret = Maths::Approach(source, target, speed, deltaTime);

    lua_pushnumber(L, ret);
    return 1;
}

int Maths_Lua::ApproachAngle(lua_State* L)
{
    float source = CHECK_NUMBER(L, 1);
    float target = CHECK_NUMBER(L, 2);
    float speed = CHECK_NUMBER(L, 3);
    float deltaTime = CHECK_NUMBER(L, 4);

    float ret = Maths::ApproachAngle(source, target, speed, deltaTime);

    lua_pushnumber(L, ret);
    return 1;
}

int Maths_Lua::IsPowerOfTwo(lua_State* L)
{
    bool ret = false;
    int arg = CHECK_INTEGER(L, 1);

    if (arg >= 0)
    {
        ret = Maths::IsPowerOfTwo((uint32_t)arg);
    }

    lua_pushboolean(L, ret);
    return 1;
}

int Maths_Lua::Min(lua_State* L)
{
    float a = CHECK_NUMBER(L, 1);
    float b = CHECK_NUMBER(L, 2);

    float ret = glm::min(a, b);

    lua_pushnumber(L, ret);
    return 1;
}

int Maths_Lua::Max(lua_State* L)
{
    float a = CHECK_NUMBER(L, 1);
    float b = CHECK_NUMBER(L, 2);

    float ret = glm::max(a, b);

    lua_pushnumber(L, ret);
    return 1;
}

int Maths_Lua::Clamp(lua_State* L)
{
    float x = CHECK_NUMBER(L, 1);
    float min = CHECK_NUMBER(L, 2);
    float max = CHECK_NUMBER(L, 3);

    float ret = glm::clamp(x, min, max);

    lua_pushnumber(L, ret);
    return 1;
}

int Maths_Lua::Lerp(lua_State* L)
{
    float a = CHECK_NUMBER(L, 1);
    float b = CHECK_NUMBER(L, 2);
    float alpha = CHECK_NUMBER(L, 3);

    float ret = glm::mix(a, b, alpha);

    lua_pushnumber(L, ret);
    return 1;
}

int Maths_Lua::Sign(lua_State* L)
{
    float number = CHECK_NUMBER(L, 1);

    float ret = (number >= 0.0f) ? 1.0f : -1.0f;

    lua_pushnumber(L, ret);
    return 1;
}

int Maths_Lua::RotateYawTowardDirection(lua_State* L)
{
    float srcYaw = CHECK_NUMBER(L, 1);
    glm::vec3 dir = CHECK_VECTOR(L, 2);
    float speed = CHECK_NUMBER(L, 3);
    float deltaTime = CHECK_NUMBER(L, 4);

    float ret = Maths::RotateYawTowardDirection(srcYaw, dir, speed, deltaTime);

    lua_pushnumber(L, ret);
    return 1;
}

int Maths_Lua::Reflect(lua_State* L)
{
    glm::vec3 i = CHECK_VECTOR(L, 1);
    glm::vec3 n = CHECK_VECTOR(L, 2);

    glm::vec3 ret = glm::reflect(i, n);

    Vector_Lua::Create(L, ret);
    return 1;
}

int Maths_Lua::SeedRand(lua_State* L)
{
    int32_t seed = CHECK_INTEGER(L, 1);

    Maths::SeedRand(seed);

    return 0;
}

int Maths_Lua::RandRange(lua_State* L)
{
    float fMin = CHECK_NUMBER(L, 1);
    float fMax = CHECK_NUMBER(L, 2);

    float ret = Maths::RandRange(fMin, fMax);

    lua_pushnumber(L, ret);
    return 1;
}

int Maths_Lua::RandRangeInt(lua_State* L)
{
    int32_t iMin = CHECK_INTEGER(L, 1);
    int32_t iMax = CHECK_INTEGER(L, 2);

    int32_t ret = Maths::RandRange(iMin, iMax);

    lua_pushinteger(L, ret);
    return 1;
}

int Maths_Lua::RandRangeVec(lua_State* L)
{
    glm::vec4 vMin;
    glm::vec4 vMax;

    if (lua_isnumber(L, 1))
    {
        float fMin = CHECK_NUMBER(L, 1);
        vMin = { fMin, fMin, fMin, fMin };
    }
    else
    {
        vMin = CHECK_VECTOR(L, 1);
    }

    if (lua_isnumber(L, 2))
    {
        float fMax = CHECK_NUMBER(L, 2);
        vMax = { fMax, fMax, fMax, fMax };
    }
    else
    {
        vMax = CHECK_VECTOR(L, 2);
    }

    glm::vec4 ret = Maths::RandRange(vMin, vMax);

    Vector_Lua::Create(L, ret);
    return 1;
}

int Maths_Lua::Rand(lua_State* L)
{
    float ret = Maths::RandRange(0.0f, 1.0f);

    lua_pushnumber(L, ret);
    return 1;
}


void Maths_Lua::Bind()
{
    lua_State* L = GetLua();

    lua_newtable(L);
    int tableIdx = lua_gettop(L);

    lua_pushcfunction(L, Damp);
    lua_setfield(L, tableIdx, "Damp");

    lua_pushcfunction(L, DampAngle);
    lua_setfield(L, tableIdx, "DampAngle");

    lua_pushcfunction(L, Approach);
    lua_setfield(L, tableIdx, "Approach");

    lua_pushcfunction(L, ApproachAngle);
    lua_setfield(L, tableIdx, "ApproachAngle");

    lua_pushcfunction(L, IsPowerOfTwo);
    lua_setfield(L, tableIdx, "IsPowerOfTwo");

    lua_pushcfunction(L, Min);
    lua_setfield(L, tableIdx, "Min");

    lua_pushcfunction(L, Max);
    lua_setfield(L, tableIdx, "Max");

    lua_pushcfunction(L, Clamp);
    lua_setfield(L, tableIdx, "Clamp");

    lua_pushcfunction(L, Lerp);
    lua_setfield(L, tableIdx, "Lerp");

    lua_pushcfunction(L, Sign);
    lua_setfield(L, tableIdx, "Sign");

    lua_pushcfunction(L, RotateYawTowardDirection);
    lua_setfield(L, tableIdx, "RotateYawTowardDirection");

    lua_pushcfunction(L, Reflect);
    lua_setfield(L, tableIdx, "Reflect");

    lua_pushcfunction(L, SeedRand);
    lua_setfield(L, tableIdx, "SeedRand");

    lua_pushcfunction(L, RandRange);
    lua_setfield(L, tableIdx, "RandRange");

    lua_pushcfunction(L, RandRangeInt);
    lua_setfield(L, tableIdx, "RandRangeInt");

    lua_pushcfunction(L, RandRangeVec);
    lua_setfield(L, tableIdx, "RandRangeVec");

    lua_pushcfunction(L, Rand);
    lua_setfield(L, tableIdx, "Rand");

    lua_setglobal(L, MATHS_LUA_NAME);

    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
