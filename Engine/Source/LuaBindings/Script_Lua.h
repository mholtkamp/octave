#pragma once

#include "Engine.h"
#if LUA_ENABLED

#define SCRIPT_LUA_NAME "Script"

struct Script_Lua
{
    static int Load(lua_State* L);
    static int Run(lua_State* L);
    static int Inherit(lua_State* L);
    static int New(lua_State* L);
    static int GarbageCollect(lua_State* L);
    static int LoadDirectory(lua_State* L);

    static void Bind();
};

#endif
