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

    lua_setglobal(L, MATHS_LUA_NAME);

    assert(lua_gettop(L) == 0);
}

#endif
