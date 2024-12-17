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
    if (lua_isnumber(L, 1))
    {
        float source = CHECK_NUMBER(L, 1);
        float target = CHECK_NUMBER(L, 2);
        float speed = CHECK_NUMBER(L, 3);
        float deltaTime = CHECK_NUMBER(L, 4);

        float ret = Maths::Approach(source, target, speed, deltaTime);

        lua_pushnumber(L, ret);
        return 1;
    }
    else
    {
        glm::vec4 source = CHECK_VECTOR(L, 1);
        glm::vec4 target = CHECK_VECTOR(L, 2);
        float speed = CHECK_NUMBER(L, 3);
        float deltaTime = CHECK_NUMBER(L, 4);

        glm::vec4 ret = Maths::Approach(source, target, speed, deltaTime);

        Vector_Lua::Create(L, ret);
        return 1;
    }
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

int Maths_Lua::NormalizeRange(lua_State* L)
{
    float value = CHECK_NUMBER(L, 1);
    float start = CHECK_NUMBER(L, 2);
    float end = CHECK_NUMBER(L, 3);

    float ret = Maths::NormalizeRange(value, start, end);

    lua_pushnumber(L, ret);
    return 1;
}

int Maths_Lua::Map(lua_State* L)
{
    float inX = CHECK_NUMBER(L, 1);
    float inMin = CHECK_NUMBER(L, 2);
    float inMax = CHECK_NUMBER(L, 3);
    float outMin = CHECK_NUMBER(L, 4);
    float outMax = CHECK_NUMBER(L, 5);

    float outX = Maths::Map(inX, inMin, inMax, outMin, outMax);

    lua_pushnumber(L, outX);
    return 1;
}

int Maths_Lua::MapClamped(lua_State* L)
{
    float inX = CHECK_NUMBER(L, 1);
    float inMin = CHECK_NUMBER(L, 2);
    float inMax = CHECK_NUMBER(L, 3);
    float outMin = CHECK_NUMBER(L, 4);
    float outMax = CHECK_NUMBER(L, 5);

    float outX = Maths::MapClamped(inX, inMin, inMax, outMin, outMax);

    lua_pushnumber(L, outX);
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
    if (lua_isinteger(L, 1))
    {
        int32_t a = CHECK_INTEGER(L, 1);
        int32_t b = CHECK_INTEGER(L, 2);

        int32_t ret = glm::min(a, b);

        lua_pushinteger(L, ret);
    }
    else
    {
        float a = CHECK_NUMBER(L, 1);
        float b = CHECK_NUMBER(L, 2);

        float ret = glm::min(a, b);

        lua_pushnumber(L, ret);
    }

    return 1;
}

int Maths_Lua::Max(lua_State* L)
{
    if (lua_isinteger(L, 1))
    {
        int32_t a = CHECK_INTEGER(L, 1);
        int32_t b = CHECK_INTEGER(L, 2);

        int32_t ret = glm::max(a, b);

        lua_pushinteger(L, ret);
    }
    else
    {
        float a = CHECK_NUMBER(L, 1);
        float b = CHECK_NUMBER(L, 2);

        float ret = glm::max(a, b);

        lua_pushnumber(L, ret);
    }

    return 1;
}

int Maths_Lua::Clamp(lua_State* L)
{
    if (lua_isinteger(L, 1))
    {
        int32_t x = CHECK_INTEGER(L, 1);
        int32_t min = CHECK_INTEGER(L, 2);
        int32_t max = CHECK_INTEGER(L, 3);

        int32_t ret = glm::clamp(x, min, max);

        lua_pushinteger(L, ret);
    }
    else
    {
        float x = CHECK_NUMBER(L, 1);
        float min = CHECK_NUMBER(L, 2);
        float max = CHECK_NUMBER(L, 3);

        float ret = glm::clamp(x, min, max);

        lua_pushnumber(L, ret);
    }

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
    if (lua_isinteger(L, 1))
    {
        int32_t number = CHECK_INTEGER(L, 1);

        int32_t ret = (number >= 0) ? 1 : -1;

        lua_pushinteger(L, ret);
    }
    else
    {
        float number = CHECK_NUMBER(L, 1);

        float ret = (number >= 0.0f) ? 1.0f : -1.0f;

        lua_pushnumber(L, ret);
    }
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

int Maths_Lua::VectorToRotation(lua_State* L)
{
    glm::vec3 vect = CHECK_VECTOR(L, 1);

    glm::vec3 rot = Maths::VectorToRotation(vect);

    Vector_Lua::Create(L, rot);
    return 1;
}

int Maths_Lua::VectorToQuat(lua_State* L)
{
    glm::vec3 vect = CHECK_VECTOR(L, 1);

    glm::quat quat = Maths::VectorToQuat(vect);

    Vector_Lua::Create(L, LuaQuatToVector(quat));
    return 1;
}

int Maths_Lua::RotationToVector(lua_State* L)
{
    glm::vec3 rotVec = CHECK_VECTOR(L, 1);

    glm::vec3 retVec = Maths::RotationToVector(rotVec);

    Vector_Lua::Create(L, retVec);
    return 1;
}

int Maths_Lua::QuatToVector(lua_State* L)
{
    glm::vec4 quatVect = CHECK_VECTOR(L, 1);
    glm::quat quat = LuaVectorToQuat(quatVect);

    glm::vec3 retVec = Maths::QuatToVector(quat);

    Vector_Lua::Create(L, retVec);
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

    REGISTER_TABLE_FUNC(L, tableIdx, Damp);

    REGISTER_TABLE_FUNC(L, tableIdx, DampAngle);

    REGISTER_TABLE_FUNC(L, tableIdx, Approach);

    REGISTER_TABLE_FUNC(L, tableIdx, ApproachAngle);

    REGISTER_TABLE_FUNC(L, tableIdx, NormalizeRange);
    REGISTER_TABLE_FUNC_EX(L, tableIdx, NormalizeRange, "Wrap");

    REGISTER_TABLE_FUNC(L, tableIdx, Map);

    REGISTER_TABLE_FUNC(L, tableIdx, MapClamped);

    REGISTER_TABLE_FUNC(L, tableIdx, IsPowerOfTwo);

    REGISTER_TABLE_FUNC(L, tableIdx, Min);

    REGISTER_TABLE_FUNC(L, tableIdx, Max);

    REGISTER_TABLE_FUNC(L, tableIdx, Clamp);

    REGISTER_TABLE_FUNC(L, tableIdx, Lerp);
    REGISTER_TABLE_FUNC_EX(L, tableIdx, Lerp, "Mix");

    REGISTER_TABLE_FUNC(L, tableIdx, Sign);

    REGISTER_TABLE_FUNC(L, tableIdx, RotateYawTowardDirection);

    REGISTER_TABLE_FUNC(L, tableIdx, Reflect);

    REGISTER_TABLE_FUNC(L, tableIdx, VectorToRotation);

    REGISTER_TABLE_FUNC(L, tableIdx, VectorToQuat);

    REGISTER_TABLE_FUNC(L, tableIdx, RotationToVector);

    REGISTER_TABLE_FUNC(L, tableIdx, QuatToVector);

    REGISTER_TABLE_FUNC(L, tableIdx, SeedRand);

    REGISTER_TABLE_FUNC(L, tableIdx, RandRange);

    REGISTER_TABLE_FUNC(L, tableIdx, RandRangeInt);

    REGISTER_TABLE_FUNC(L, tableIdx, RandRangeVec);

    REGISTER_TABLE_FUNC(L, tableIdx, Rand);

    lua_pushvalue(L, -1);
    lua_setglobal(L, MATHS_LUA_NAME);

    lua_setglobal(L, "Math");

    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
