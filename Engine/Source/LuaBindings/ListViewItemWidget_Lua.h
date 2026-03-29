#pragma once

#include "EngineTypes.h"

#if LUA_ENABLED

#include "LuaBindings/LuaUtils.h"

#define LISTVIEW_ITEM_LUA_NAME "ListViewItemWidget"
#define LISTVIEW_ITEM_LUA_FLAG "cfListViewItem"
#define CHECK_LISTVIEW_ITEM(L, arg) static_cast<ListViewItemWidget*>(CheckNodeLuaType(L, arg, LISTVIEW_ITEM_LUA_NAME, LISTVIEW_ITEM_LUA_FLAG))

struct ListViewItemWidget_Lua
{
    static int GetIndex(lua_State* L);
    static int GetListView(lua_State* L);
    static int GetContentWidget(lua_State* L);
    static int SetSelected(lua_State* L);
    static int IsSelected(lua_State* L);
    static int IsHovered(lua_State* L);

    static void Bind();
};

#endif
