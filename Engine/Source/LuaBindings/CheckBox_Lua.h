#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Nodes/Widgets/CheckBox.h"
#include "LuaBindings/Widget_Lua.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define CHECK_BOX_LUA_NAME "CheckBox"
#define CHECK_BOX_LUA_FLAG "cfCheckBox"
#define CHECK_CHECK_BOX(L, arg) (CheckBox*)CheckNodeLuaType(L, arg, CHECK_BOX_LUA_NAME, CHECK_BOX_LUA_FLAG);

struct CheckBox_Lua
{
    static int IsChecked(lua_State* L);
    static int SetChecked(lua_State* L);

    static void Bind();
};

#endif