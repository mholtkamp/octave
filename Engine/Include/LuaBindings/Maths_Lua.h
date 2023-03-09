#pragma once

#include "Maths.h"
#include "EngineTypes.h"

#if LUA_ENABLED

#define MATHS_LUA_NAME "Maths"

struct Maths_Lua
{
    static int Damp(lua_State* L);
    static int DampAngle(lua_State* L);
    static int Approach(lua_State* L);
    static int ApproachAngle(lua_State* L);
    static int NormalizeRange(lua_State* L);
    static int IsPowerOfTwo(lua_State* L);
    static int Min(lua_State* L);
    static int Max(lua_State* L);
    static int Clamp(lua_State* L);
    static int Lerp(lua_State* L);
    static int Sign(lua_State* L);
    static int RotateYawTowardDirection(lua_State* L);
    static int Reflect(lua_State* L);

    static int SeedRand(lua_State* L);
    static int RandRange(lua_State* L);
    static int RandRangeInt(lua_State* L);
    static int RandRangeVec(lua_State* L);
    static int Rand(lua_State* L);

    static void Bind();
};

#endif
