#pragma once

#include "Engine.h"
#if LUA_ENABLED

#define ENGINE_LUA_NAME "Engine"

struct Engine_Lua
{
    static int IsShuttingDown(lua_State* L);
    static int GetTime(lua_State* L);
    static int GetDeltaTime(lua_State* L);
    static int GetPlatform(lua_State* L);
    static int IsEditor(lua_State* L);

    static void Bind();
};

#endif
