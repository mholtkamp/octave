#pragma once

#include "Log.h"
#include "EngineTypes.h"

#if LUA_ENABLED

#define LOG_LUA_NAME "Log"

struct Log_Lua
{
    static int Debug(lua_State* L);
    static int Warning(lua_State* L);
    static int Error(lua_State* L);
    static int Console(lua_State* L);
    static int Enable(lua_State* L);
    static int IsEnabled(lua_State* L);

    static void Bind();
};

#endif
