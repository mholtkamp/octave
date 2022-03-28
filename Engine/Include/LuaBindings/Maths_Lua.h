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
    static int IsPowerOfTwo(lua_State* L);

    static void Bind();
};

#endif
