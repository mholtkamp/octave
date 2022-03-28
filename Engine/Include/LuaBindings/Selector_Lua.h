#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Widgets/Selector.h"
#include "LuaBindings/Widget_Lua.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define SELECTOR_LUA_NAME "Selector"
#define SELECTOR_LUA_FLAG "cfSelector"
#define CHECK_SELECTOR(L, arg) (Selector*) CheckHierarchyLuaType<Widget_Lua>(L, arg, SELECTOR_LUA_NAME, SELECTOR_LUA_FLAG)->mWidget;

struct Selector_Lua
{
    static int CreateNew(lua_State* L);

    static int AddSelection(lua_State* L);
    static int RemoveSelection(lua_State* L);
    static int RemoveAllSelections(lua_State* L);
    static int GetNumSelections(lua_State* L);

    static int SetSelectionByString(lua_State* L);
    static int SetSelectionIndex(lua_State* L);
    static int Increment(lua_State* L);
    static int Decrement(lua_State* L);

    static int GetSelectionString(lua_State* L);
    static int GetSelectionIndex(lua_State* L);

    static int SetSelectionChangeHandler(lua_State* L);

    static void Bind();
};

#endif