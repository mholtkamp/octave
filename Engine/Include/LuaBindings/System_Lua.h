#pragma once

#include "Engine.h"

#if LUA_ENABLED

#define SYSTEM_LUA_NAME "System"

struct System_Lua
{
    static int WriteSave(lua_State* L);
    static int ReadSave(lua_State* L);
    static int DoesSaveExist(lua_State* L);
    static int DeleteSave(lua_State* L);
    static int UnmountMemoryCard(lua_State* L);

    static int SetScreenOrientation(lua_State* L);
    static int GetScreenOrientation(lua_State* L);

    static void Bind();
};

#endif

