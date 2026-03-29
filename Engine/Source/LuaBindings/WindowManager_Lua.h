#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "WindowManager.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define WINDOW_MANAGER_LUA_NAME "WindowManager"

struct WindowManager_Lua
{
    static int FindWindow(lua_State* L);
    static int HasWindow(lua_State* L);
    static int ShowWindow(lua_State* L);
    static int HideWindow(lua_State* L);
    static int CloseWindow(lua_State* L);
    static int BringToFront(lua_State* L);

    static void Bind();
};

#endif
