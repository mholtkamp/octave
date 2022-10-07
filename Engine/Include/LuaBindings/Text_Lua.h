#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Widgets/Text.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define TEXT_LUA_NAME "Text"
#define TEXT_LUA_FLAG "cfText"
#define CHECK_TEXT(L, arg) (Text*) CheckHierarchyLuaType<Widget_Lua>(L, arg, TEXT_LUA_NAME, TEXT_LUA_FLAG)->mWidget;

struct Text_Lua
{
    static int CreateNew(lua_State* L);

    static int SetFont(lua_State* L);
    static int GetFont(lua_State* L);
    static int SetOutlineColor(lua_State* L);
    static int GetOutlineColor(lua_State* L);
    static int SetSize(lua_State* L);
    static int GetSize(lua_State* L);
    static int GetScaledSize(lua_State* L);
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

    static void Bind();
};

#endif