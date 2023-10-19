#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Nodes/Widgets/Text.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define TEXT_LUA_NAME "Text"
#define TEXT_LUA_FLAG "cfText"
#define CHECK_TEXT(L, arg) (Text*)CheckNodeLuaType(L, arg, TEXT_LUA_NAME, TEXT_LUA_FLAG);

struct Text_Lua
{
    static int SetFont(lua_State* L);
    static int GetFont(lua_State* L);
    static int SetOutlineColor(lua_State* L);
    static int GetOutlineColor(lua_State* L);
    static int SetTextSize(lua_State* L);
    static int GetTextSize(lua_State* L);
    static int GetScaledTextSize(lua_State* L);
    static int GetOutlineSize(lua_State* L);
    static int GetSoftness(lua_State* L);
    static int GetCutoff(lua_State* L);
    static int SetText(lua_State* L);
    static int GetText(lua_State* L);
    static int GetTextWidth(lua_State* L);
    static int GetTextHeight(lua_State* L);
    static int GetScaledMinExtent(lua_State* L);
    static int GetScaledMaxExtent(lua_State* L);
    static int SetHorizontalJustification(lua_State* L);
    static int GetHorizontalJustification(lua_State* L);
    static int SetVerticalJustification(lua_State* L);
    static int GetVerticalJustification(lua_State* L);
    static int EnableWordWrap(lua_State* L);
    static int IsWordWrapEnabled(lua_State* L);

    static void Bind();
};

#endif