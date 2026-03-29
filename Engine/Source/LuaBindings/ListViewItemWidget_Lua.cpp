#include "LuaBindings/ListViewItemWidget_Lua.h"

#if LUA_ENABLED

#include "LuaBindings/Widget_Lua.h"
#include "LuaBindings/Node_Lua.h"
#include "LuaBindings/LuaTypeCheck.h"

#include "Nodes/Widgets/ListViewItemWidget.h"
#include "Nodes/Widgets/ListViewWidget.h"

int ListViewItemWidget_Lua::GetIndex(lua_State* L)
{
    ListViewItemWidget* item = CHECK_LISTVIEW_ITEM(L, 1);
    lua_pushinteger(L, item->GetIndex());
    return 1;
}

int ListViewItemWidget_Lua::GetListView(lua_State* L)
{
    ListViewItemWidget* item = CHECK_LISTVIEW_ITEM(L, 1);
    ListViewWidget* listView = item->GetListView();

    if (listView != nullptr)
    {
        Node_Lua::Create(L, listView);
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

int ListViewItemWidget_Lua::GetContentWidget(lua_State* L)
{
    ListViewItemWidget* item = CHECK_LISTVIEW_ITEM(L, 1);
    Widget* content = item->GetContentWidget();

    if (content != nullptr)
    {
        Node_Lua::Create(L, content);
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

int ListViewItemWidget_Lua::SetSelected(lua_State* L)
{
    ListViewItemWidget* item = CHECK_LISTVIEW_ITEM(L, 1);
    bool selected = CHECK_BOOLEAN(L, 2);

    item->SetSelected(selected);
    return 0;
}

int ListViewItemWidget_Lua::IsSelected(lua_State* L)
{
    ListViewItemWidget* item = CHECK_LISTVIEW_ITEM(L, 1);
    lua_pushboolean(L, item->IsSelected());
    return 1;
}

int ListViewItemWidget_Lua::IsHovered(lua_State* L)
{
    ListViewItemWidget* item = CHECK_LISTVIEW_ITEM(L, 1);
    lua_pushboolean(L, item->IsHovered());
    return 1;
}

void ListViewItemWidget_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        LISTVIEW_ITEM_LUA_NAME,
        LISTVIEW_ITEM_LUA_FLAG,
        WIDGET_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, GetIndex);
    REGISTER_TABLE_FUNC(L, mtIndex, GetListView);
    REGISTER_TABLE_FUNC(L, mtIndex, GetContentWidget);
    REGISTER_TABLE_FUNC(L, mtIndex, SetSelected);
    REGISTER_TABLE_FUNC(L, mtIndex, IsSelected);
    REGISTER_TABLE_FUNC(L, mtIndex, IsHovered);

    lua_pop(L, 1);
}

#endif
