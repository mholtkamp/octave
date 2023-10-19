#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Nodes/Widgets/ComboBox.h"
#include "LuaBindings/Widget_Lua.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define COMBO_BOX_LUA_NAME "ComboBox"
#define COMBO_BOX_LUA_FLAG "cfComboBox"
#define CHECK_COMBO_BOX(L, arg) (ComboBox*)CheckNodeLuaType(L, arg, COMBO_BOX_LUA_NAME, COMBO_BOX_LUA_FLAG);

struct ComboBox_Lua
{
    static int GetList(lua_State* L);
    static int ShowList(lua_State* L);

    static void Bind();
};

#endif