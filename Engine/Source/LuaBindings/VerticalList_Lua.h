#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Nodes/Widgets/VerticalList.h"
#include "LuaBindings/Widget_Lua.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define VERTICAL_LIST_LUA_NAME "VerticalList"
#define VERTICAL_LIST_LUA_FLAG "cfVerticalList"
#define CHECK_VERTICAL_LIST(L, arg) (VerticalList*)CheckNodeLuaType(L, arg, VERTICAL_LIST_LUA_NAME, VERTICAL_LIST_LUA_FLAG);

struct VerticalList_Lua
{
    static int AddListItem(lua_State* L);
    static int RemoveListItem(lua_State* L);
    static int GetListItem(lua_State* L);
    static int GetNumListItems(lua_State* L);
    static int SetDisplayCount(lua_State* L);
    static int GetDisplayCount(lua_State* L);
    static int SetDisplayOffset(lua_State* L);
    static int GetDisplayOffset(lua_State* L);
    static int SetFitContents(lua_State* L);

    static void Bind();
};

#endif