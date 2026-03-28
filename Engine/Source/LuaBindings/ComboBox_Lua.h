#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Nodes/Widgets/ComboBox.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define COMBOBOX_LUA_NAME "ComboBox"
#define COMBOBOX_LUA_FLAG "cfComboBox"
#define CHECK_COMBOBOX(L, arg) (ComboBox*)CheckNodeLuaType(L, arg, COMBOBOX_LUA_NAME, COMBOBOX_LUA_FLAG);

struct ComboBox_Lua
{
    // Options
    static int AddOption(lua_State* L);
    static int RemoveOption(lua_State* L);
    static int ClearOptions(lua_State* L);
    static int SetOptions(lua_State* L);
    static int GetOptions(lua_State* L);
    static int GetOptionCount(lua_State* L);

    // Selection
    static int SetSelectedIndex(lua_State* L);
    static int GetSelectedIndex(lua_State* L);
    static int GetSelectedOption(lua_State* L);

    // State
    static int IsOpen(lua_State* L);
    static int Open(lua_State* L);
    static int Close(lua_State* L);
    static int Toggle(lua_State* L);

    // Visual
    static int SetBackgroundColor(lua_State* L);
    static int GetBackgroundColor(lua_State* L);
    static int SetTextColor(lua_State* L);
    static int GetTextColor(lua_State* L);
    static int SetDropdownColor(lua_State* L);
    static int GetDropdownColor(lua_State* L);
    static int SetHoveredColor(lua_State* L);
    static int GetHoveredColor(lua_State* L);
    static int SetMaxVisibleItems(lua_State* L);
    static int GetMaxVisibleItems(lua_State* L);

    // Children
    static int GetBackground(lua_State* L);
    static int GetTextWidget(lua_State* L);

    static void Bind();
};

#endif
