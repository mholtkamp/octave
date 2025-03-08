#pragma once

#include "Engine.h"
#include "Signals.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define SIGNAL_LUA_NAME "Signal"
#define CHECK_SIGNAL(L, arg) CheckLuaType<Signal_Lua>(L, arg, SIGNAL_LUA_NAME)->mSignal;

struct Signal_Lua
{
    Signal mSignal;

    static int Create(lua_State* L);
    static int Destroy(lua_State* L);

    static int Emit(lua_State* L);
    static int Connect(lua_State* L);
    static int Disconnect(lua_State* L);

    static void Bind();
};

#endif
