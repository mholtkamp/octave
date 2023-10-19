#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Nodes/Widgets/Button.h"
#include "LuaBindings/Widget_Lua.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define BUTTON_LUA_NAME "Button"
#define BUTTON_LUA_FLAG "cfButton"
#define CHECK_BUTTON(L, arg) (Button*) CheckHierarchyLuaType<Widget_Lua>(L, arg, BUTTON_LUA_NAME, BUTTON_LUA_FLAG)->mWidget;

struct Button_Lua
{
    static int CreateNew(lua_State* L);

    static int GetState(lua_State* L);
    static int SetState(lua_State* L);
    static int SetNormalTexture(lua_State* L);
    static int SetHoveredTexture(lua_State* L);
    static int SetPressedTexture(lua_State* L);
    static int SetDisabledTexture(lua_State* L);
    static int SetNormalColor(lua_State* L);
    static int SetHoveredColor(lua_State* L);
    static int SetPressedColor(lua_State* L);
    static int SetDisabledColor(lua_State* L);
    static int SetUseQuadStateColor(lua_State* L);
    static int SetUseTextStateColor(lua_State* L);
    static int SetHandleMouseInput(lua_State* L);
    static int SetHoverHandler(lua_State* L);
    static int SetPressedHandler(lua_State* L);
    static int SetTextString(lua_State* L);
    static int GetTextString(lua_State* L);
    static int GetText(lua_State* L);
    static int GetQuad(lua_State* L);

    static void Bind();
};

#endif