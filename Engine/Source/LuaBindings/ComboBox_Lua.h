#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Widgets/ComboBox.h"
#include "LuaBindings/Widget_Lua.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define COMBO_BOX_LUA_NAME "ComboBox"
#define COMBO_BOX_LUA_FLAG "cfComboBox"
#define CHECK_COMBO_BOX(L, arg) (ComboBox*) CheckHierarchyLuaType<Widget_Lua>(L, arg, COMBO_BOX_LUA_NAME, COMBO_BOX_LUA_FLAG)->mWidget;

struct ComboBox_Lua
{
    static int CreateNew(lua_State* L);

    static int GetList(lua_State* L);
    static int ShowList(lua_State* L);

    static void Bind();
};

#endif