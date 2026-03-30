#pragma once

#include "EngineTypes.h"

#if LUA_ENABLED

#include "LuaBindings/LuaUtils.h"

#define LISTVIEW_LUA_NAME "ListViewWidget"
#define LISTVIEW_LUA_FLAG "cfListView"
#define CHECK_LISTVIEW(L, arg) static_cast<ListViewWidget*>(CheckNodeLuaType(L, arg, LISTVIEW_LUA_NAME, LISTVIEW_LUA_FLAG))

struct ListViewWidget_Lua
{
    // Template
    static int SetItemTemplate(lua_State* L);
    static int GetItemTemplate(lua_State* L);

    // Data Management
    static int SetData(lua_State* L);
    static int AddItem(lua_State* L);
    static int RemoveItem(lua_State* L);
    static int UpdateItem(lua_State* L);
    static int Clear(lua_State* L);
    static int GetItemCount(lua_State* L);
    static int GetItemData(lua_State* L);

    // Layout
    static int SetSpacing(lua_State* L);
    static int GetSpacing(lua_State* L);
    static int SetOrientation(lua_State* L);
    static int GetOrientation(lua_State* L);

    // Item sizing
    static int SetItemWidth(lua_State* L);
    static int GetItemWidth(lua_State* L);
    static int SetItemHeight(lua_State* L);
    static int GetItemHeight(lua_State* L);

    // Selection
    static int SetSelectedIndex(lua_State* L);
    static int GetSelectedIndex(lua_State* L);
    static int GetSelectedData(lua_State* L);
    static int ClearSelection(lua_State* L);

    // Access
    static int GetScrollContainer(lua_State* L);
    static int GetArrayWidget(lua_State* L);
    static int GetItem(lua_State* L);

    // Scroll control
    static int ScrollToItem(lua_State* L);

    static void Bind();
};

#endif
