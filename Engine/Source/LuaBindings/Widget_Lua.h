#pragma once

#include "EngineTypes.h"
#include "Nodes/Widgets/Widget.h"

#include "LuaBindings/Node_Lua.h"

#if LUA_ENABLED

#define WIDGET_LUA_NAME "Widget"
#define WIDGET_LUA_FLAG "cfWidget"
#define CHECK_WIDGET(L, arg) static_cast<Widget*>(CheckNodeLuaType(L, arg, WIDGET_LUA_NAME, WIDGET_LUA_FLAG));

struct Widget_Lua
{
    static int GetRect(lua_State* L);
    static int UpdateRect(lua_State* L);
    static int SetX(lua_State* L);
    static int SetY(lua_State* L);
    static int SetWidth(lua_State* L);
    static int SetHeight(lua_State* L);
    static int SetXRatio(lua_State* L);
    static int SetYRatio(lua_State* L);
    static int SetWidthRatio(lua_State* L);
    static int SetHeightRatio(lua_State* L);
    static int SetLeftMargin(lua_State* L);
    static int SetTopMargin(lua_State* L);
    static int SetRightMargin(lua_State* L);
    static int SetBottomMargin(lua_State* L);
    static int SetPosition(lua_State* L);
    static int SetDimensions(lua_State* L);
    static int SetRect(lua_State* L);
    static int SetRatios(lua_State* L);
    static int SetMargins(lua_State* L);
    static int SetOffset(lua_State* L);
    static int GetOffset(lua_State* L);
    static int SetSize(lua_State* L);
    static int GetSize(lua_State* L);
    static int GetAnchorMode(lua_State* L);
    static int SetAnchorMode(lua_State* L);
    static int StretchX(lua_State* L);
    static int StretchY(lua_State* L);
    static int GetX(lua_State* L);
    static int GetY(lua_State* L);
    static int GetWidth(lua_State* L);
    static int GetHeight(lua_State* L);
    static int GetPosition(lua_State* L);
    static int GetDimensions(lua_State* L);
    static int GetParentWidth(lua_State* L);
    static int GetParentHeight(lua_State* L);
    static int SetColor(lua_State* L);
    static int ShouldHandleInput(lua_State* L);
    static int MarkDirty(lua_State* L);
    static int IsDirty(lua_State* L);
    static int ContainsMouse(lua_State* L);
    static int ContainsPoint(lua_State* L);
    static int SetRotation(lua_State* L);
    static int GetRotation(lua_State* L);
    static int SetPivot(lua_State* L);
    static int GetPivot(lua_State* L);
    static int SetScale(lua_State* L);
    static int GetScale(lua_State* L);
    static int IsScissorEnabled(lua_State* L);
    static int EnableScissor(lua_State* L);
    static int SetOpacity(lua_State* L);
    static int SetOpacityFloat(lua_State* L);
    static int GetOpacity(lua_State* L);
    static int GetOpacityFloat(lua_State* L);

    static void Bind();
};

#endif