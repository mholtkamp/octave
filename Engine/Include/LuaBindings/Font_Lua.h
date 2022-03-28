#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Assets/Font.h"

#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define FONT_LUA_NAME "Font"
#define FONT_LUA_FLAG "cfFont"
#define CHECK_FONT(L, arg) CheckAssetLuaType<Font>(L, arg, FONT_LUA_NAME, FONT_LUA_FLAG)

struct Font_Lua
{
    static int GetSize(lua_State* L);
    static int GetWidth(lua_State* L);
    static int GetHeight(lua_State* L);
    static int GetTexture(lua_State* L);
    static int IsBold(lua_State* L);
    static int IsItalic(lua_State* L);

    static void Bind();
};

#endif
