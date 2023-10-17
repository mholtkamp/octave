#pragma once

#include "Engine.h"
#include "Rect.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define RECT_LUA_NAME "Rect"
#define CHECK_RECT(L, Arg) CheckLuaType<Rect_Lua>(L, Arg, RECT_LUA_NAME)->mRect;

struct Rect_Lua
{
    Rect mRect;

    static int Create(lua_State* L);
    static int Create(lua_State* L, Rect value);
    static int Destroy(lua_State* L);

    static int Index(lua_State* L);
    static int NewIndex(lua_State* L);
    static int ToString(lua_State* L);

    static int ContainsPoint(lua_State* L);
    static int OverlapsRect(lua_State* L);
    static int Clamp(lua_State* L);
    static int Top(lua_State* L);
    static int Bottom(lua_State* L);
    static int Left(lua_State* L);
    static int Right(lua_State* L);

    static void Bind();
};

#endif
