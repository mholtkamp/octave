#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Nodes/Widgets/CheckBox.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define CHECKBOX_LUA_NAME "CheckBox"
#define CHECKBOX_LUA_FLAG "cfCheckBox"
#define CHECK_CHECKBOX(L, arg) (CheckBox*)CheckNodeLuaType(L, arg, CHECKBOX_LUA_NAME, CHECKBOX_LUA_FLAG);

struct CheckBox_Lua
{
    // State
    static int SetChecked(lua_State* L);
    static int IsChecked(lua_State* L);
    static int Toggle(lua_State* L);

    // Text
    static int SetText(lua_State* L);
    static int GetText(lua_State* L);

    // Visual
    static int SetCheckedColor(lua_State* L);
    static int GetCheckedColor(lua_State* L);
    static int SetUncheckedColor(lua_State* L);
    static int GetUncheckedColor(lua_State* L);
    static int SetTextColor(lua_State* L);
    static int GetTextColor(lua_State* L);
    static int SetBoxSize(lua_State* L);
    static int GetBoxSize(lua_State* L);
    static int SetSpacing(lua_State* L);
    static int GetSpacing(lua_State* L);

    // Children
    static int GetBoxQuad(lua_State* L);
    static int GetTextWidget(lua_State* L);

    static void Bind();
};

#endif
