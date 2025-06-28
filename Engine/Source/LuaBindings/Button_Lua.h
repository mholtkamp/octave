#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Nodes/Widgets/Button.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define BUTTON_LUA_NAME "Button"
#define BUTTON_LUA_FLAG "cfButton"
#define CHECK_BUTTON(L, arg) (Button*)CheckNodeLuaType(L, arg, BUTTON_LUA_NAME, BUTTON_LUA_FLAG);

struct Button_Lua
{
    static int SetSelected(lua_State* L);
    static int GetSelected(lua_State* L);
    static int EnableMouseHandling(lua_State* L);
    static int IsSelected(lua_State* L);
    static int Activate(lua_State* L);
    static int GetState(lua_State* L);
    static int SetLocked(lua_State* L);
    static int SetTextString(lua_State* L);
    static int GetTextString(lua_State* L);
    static int SetStateTextures(lua_State* L);
    static int SetStateColors(lua_State* L);
    static int GetStateTextures(lua_State* L);
    static int GetStateColors(lua_State* L);
    static int GetText(lua_State* L);
    static int GetQuad(lua_State* L);

    static void Bind();
};

#endif
