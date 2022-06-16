#pragma once

#include "Engine.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define VECTOR_LUA_NAME "Vector"
#define CHECK_VECTOR(L, Arg) CheckLuaType<Vector_Lua>(L, Arg, VECTOR_LUA_NAME)->mVector;

struct Vector_Lua
{
    glm::vec4 mVector;

    Vector_Lua() { mVector = { 0.0f, 0.0f, 0.0f, 0.0f }; }
    ~Vector_Lua() { }

    static int Create(lua_State* L);
    static int Create(lua_State* L, glm::vec4 value);
    static int Create(lua_State* L, glm::vec3 value);
    static int Create(lua_State* L, glm::vec2 value);
    static int Destroy(lua_State* L);

    static int Index(lua_State* L);
    static int NewIndex(lua_State* L);
    static int ToString(lua_State* L);

    static int Set(lua_State* L);
    static int Clone(lua_State* L);
    static int Add(lua_State* L);
    static int Subtract(lua_State* L);
    static int Multiply(lua_State* L);
    static int Divide(lua_State* L);
    static int Equals(lua_State* L);
    static int Dot(lua_State* L);
    static int Dot3(lua_State* L);
    static int Cross(lua_State* L);
    static int Lerp(lua_State* L);
    static int Max(lua_State* L);
    static int Min(lua_State* L);
    static int Clamp(lua_State* L);
    static int Normalize(lua_State* L);
    static int Normalize3(lua_State* L);
    static int Reflect(lua_State* L);
    static int Damp(lua_State* L);
    static int Rotate(lua_State* L);
    static int Length(lua_State* L);
    static int Angle(lua_State* L);
    static int SignedAngle(lua_State* L);

    static void Bind();
};

#endif
