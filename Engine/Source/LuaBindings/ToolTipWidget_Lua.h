#pragma once

#include "EngineTypes.h"

#if LUA_ENABLED

#include "LuaBindings/Widget_Lua.h"
#include "LuaBindings/LuaUtils.h"

#define TOOLTIPWIDGET_LUA_NAME "ToolTipWidget"
#define TOOLTIPWIDGET_LUA_FLAG "cfToolTipWidget"
#define CHECK_TOOLTIPWIDGET(L, arg) static_cast<ToolTipWidget*>(CheckNodeLuaType(L, arg, TOOLTIPWIDGET_LUA_NAME, TOOLTIPWIDGET_LUA_FLAG));

struct ToolTipWidget_Lua
{
    // Content
    static int SetTooltipTitle(lua_State* L);
    static int GetTooltipTitle(lua_State* L);
    static int SetTooltipText(lua_State* L);
    static int GetTooltipText(lua_State* L);
    static int SetContent(lua_State* L);
    static int ConfigureFromWidget(lua_State* L);

    // Styling - Background
    static int SetBackgroundColor(lua_State* L);
    static int GetBackgroundColor(lua_State* L);
    static int SetCornerRadius(lua_State* L);
    static int GetCornerRadius(lua_State* L);

    // Styling - Text
    static int SetTitleFontSize(lua_State* L);
    static int GetTitleFontSize(lua_State* L);
    static int SetTextFontSize(lua_State* L);
    static int GetTextFontSize(lua_State* L);
    static int SetTitleColor(lua_State* L);
    static int GetTitleColor(lua_State* L);
    static int SetTextColor(lua_State* L);
    static int GetTextColor(lua_State* L);

    // Styling - Layout
    static int SetPadding(lua_State* L);
    static int GetPaddingLeft(lua_State* L);
    static int GetPaddingTop(lua_State* L);
    static int GetPaddingRight(lua_State* L);
    static int GetPaddingBottom(lua_State* L);
    static int SetMaxWidth(lua_State* L);
    static int GetMaxWidth(lua_State* L);
    static int SetTitleTextSpacing(lua_State* L);
    static int GetTitleTextSpacing(lua_State* L);

    static void Bind();
};

#endif
