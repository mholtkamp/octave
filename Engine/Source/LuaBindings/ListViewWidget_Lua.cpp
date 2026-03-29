#include "LuaBindings/ListViewWidget_Lua.h"

#if LUA_ENABLED

#include "LuaBindings/Widget_Lua.h"
#include "LuaBindings/Node_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/LuaTypeCheck.h"

#include "Nodes/Widgets/ListViewWidget.h"
#include "Nodes/Widgets/ListViewItemWidget.h"
#include "Nodes/Widgets/ScrollContainer.h"
#include "Nodes/Widgets/ArrayWidget.h"
#include "Assets/Scene.h"

// Template

int ListViewWidget_Lua::SetItemTemplate(lua_State* L)
{
    ListViewWidget* listView = CHECK_LISTVIEW(L, 1);
    Scene* scene = nullptr;

    if (!lua_isnil(L, 2))
    {
        scene = CheckAssetLuaType<Scene>(L, 2, "Scene", "cfScene");
    }

    listView->SetItemTemplate(scene);
    return 0;
}

int ListViewWidget_Lua::GetItemTemplate(lua_State* L)
{
    ListViewWidget* listView = CHECK_LISTVIEW(L, 1);
    Asset_Lua::Create(L, listView->GetItemTemplate());
    return 1;
}

// Data Management

int ListViewWidget_Lua::SetData(lua_State* L)
{
    ListViewWidget* listView = CHECK_LISTVIEW(L, 1);

    std::vector<Datum> data;

    if (lua_istable(L, 2))
    {
        lua_pushnil(L);
        while (lua_next(L, 2) != 0)
        {
            data.push_back(LuaObjectToDatum(L, -1));
            lua_pop(L, 1);
        }
    }

    listView->SetData(data);
    return 0;
}

int ListViewWidget_Lua::AddItem(lua_State* L)
{
    ListViewWidget* listView = CHECK_LISTVIEW(L, 1);
    Datum data = LuaObjectToDatum(L, 2);
    int32_t index = -1;

    if (!lua_isnone(L, 3))
    {
        index = (int32_t)lua_tointeger(L, 3);
    }

    listView->AddItem(data, index);
    return 0;
}

int ListViewWidget_Lua::RemoveItem(lua_State* L)
{
    ListViewWidget* listView = CHECK_LISTVIEW(L, 1);
    int32_t index = (int32_t)CHECK_INTEGER(L, 2);

    listView->RemoveItem(index);
    return 0;
}

int ListViewWidget_Lua::UpdateItem(lua_State* L)
{
    ListViewWidget* listView = CHECK_LISTVIEW(L, 1);
    int32_t index = (int32_t)CHECK_INTEGER(L, 2);
    Datum data = LuaObjectToDatum(L, 3);

    listView->UpdateItem(index, data);
    return 0;
}

int ListViewWidget_Lua::Clear(lua_State* L)
{
    ListViewWidget* listView = CHECK_LISTVIEW(L, 1);
    listView->Clear();
    return 0;
}

int ListViewWidget_Lua::GetItemCount(lua_State* L)
{
    ListViewWidget* listView = CHECK_LISTVIEW(L, 1);
    lua_pushinteger(L, listView->GetItemCount());
    return 1;
}

int ListViewWidget_Lua::GetItemData(lua_State* L)
{
    ListViewWidget* listView = CHECK_LISTVIEW(L, 1);
    int32_t index = (int32_t)CHECK_INTEGER(L, 2);

    Datum data = listView->GetItemData(index);
    LuaPushDatum(L, data);
    return 1;
}

// Layout

int ListViewWidget_Lua::SetSpacing(lua_State* L)
{
    ListViewWidget* listView = CHECK_LISTVIEW(L, 1);
    float spacing = CHECK_NUMBER(L, 2);

    listView->SetSpacing(spacing);
    return 0;
}

int ListViewWidget_Lua::GetSpacing(lua_State* L)
{
    ListViewWidget* listView = CHECK_LISTVIEW(L, 1);
    lua_pushnumber(L, listView->GetSpacing());
    return 1;
}

int ListViewWidget_Lua::SetOrientation(lua_State* L)
{
    ListViewWidget* listView = CHECK_LISTVIEW(L, 1);
    ArrayOrientation orientation = ArrayOrientation::Vertical;

    if (lua_isstring(L, 2))
    {
        const char* str = lua_tostring(L, 2);
        if (strcmp(str, "Horizontal") == 0)
        {
            orientation = ArrayOrientation::Horizontal;
        }
    }
    else if (lua_isinteger(L, 2))
    {
        orientation = static_cast<ArrayOrientation>(lua_tointeger(L, 2));
    }

    listView->SetOrientation(orientation);
    return 0;
}

int ListViewWidget_Lua::GetOrientation(lua_State* L)
{
    ListViewWidget* listView = CHECK_LISTVIEW(L, 1);
    ArrayOrientation orientation = listView->GetOrientation();

    if (orientation == ArrayOrientation::Vertical)
    {
        lua_pushstring(L, "Vertical");
    }
    else
    {
        lua_pushstring(L, "Horizontal");
    }
    return 1;
}

// Item sizing

int ListViewWidget_Lua::SetItemWidth(lua_State* L)
{
    ListViewWidget* listView = CHECK_LISTVIEW(L, 1);
    float width = CHECK_NUMBER(L, 2);

    listView->SetItemWidth(width);
    return 0;
}

int ListViewWidget_Lua::GetItemWidth(lua_State* L)
{
    ListViewWidget* listView = CHECK_LISTVIEW(L, 1);
    lua_pushnumber(L, listView->GetItemWidth());
    return 1;
}

int ListViewWidget_Lua::SetItemHeight(lua_State* L)
{
    ListViewWidget* listView = CHECK_LISTVIEW(L, 1);
    float height = CHECK_NUMBER(L, 2);

    listView->SetItemHeight(height);
    return 0;
}

int ListViewWidget_Lua::GetItemHeight(lua_State* L)
{
    ListViewWidget* listView = CHECK_LISTVIEW(L, 1);
    lua_pushnumber(L, listView->GetItemHeight());
    return 1;
}

// Selection

int ListViewWidget_Lua::SetSelectedIndex(lua_State* L)
{
    ListViewWidget* listView = CHECK_LISTVIEW(L, 1);
    int32_t index = (int32_t)CHECK_INTEGER(L, 2);

    listView->SetSelectedIndex(index);
    return 0;
}

int ListViewWidget_Lua::GetSelectedIndex(lua_State* L)
{
    ListViewWidget* listView = CHECK_LISTVIEW(L, 1);
    lua_pushinteger(L, listView->GetSelectedIndex());
    return 1;
}

int ListViewWidget_Lua::GetSelectedData(lua_State* L)
{
    ListViewWidget* listView = CHECK_LISTVIEW(L, 1);
    Datum data = listView->GetSelectedData();
    LuaPushDatum(L, data);
    return 1;
}

int ListViewWidget_Lua::ClearSelection(lua_State* L)
{
    ListViewWidget* listView = CHECK_LISTVIEW(L, 1);
    listView->ClearSelection();
    return 0;
}

// Access

int ListViewWidget_Lua::GetScrollContainer(lua_State* L)
{
    ListViewWidget* listView = CHECK_LISTVIEW(L, 1);
    ScrollContainer* scrollContainer = listView->GetScrollContainer();

    if (scrollContainer != nullptr)
    {
        Node_Lua::Create(L, scrollContainer);
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

int ListViewWidget_Lua::GetArrayWidget(lua_State* L)
{
    ListViewWidget* listView = CHECK_LISTVIEW(L, 1);
    ArrayWidget* arrayWidget = listView->GetArrayWidget();

    if (arrayWidget != nullptr)
    {
        Node_Lua::Create(L, arrayWidget);
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

int ListViewWidget_Lua::GetItem(lua_State* L)
{
    ListViewWidget* listView = CHECK_LISTVIEW(L, 1);
    int32_t index = (int32_t)CHECK_INTEGER(L, 2);

    ListViewItemWidget* item = listView->GetItem(index);
    if (item != nullptr)
    {
        Node_Lua::Create(L, item);
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

// Scroll control

int ListViewWidget_Lua::ScrollToItem(lua_State* L)
{
    ListViewWidget* listView = CHECK_LISTVIEW(L, 1);
    int32_t index = (int32_t)CHECK_INTEGER(L, 2);

    listView->ScrollToItem(index);
    return 0;
}

void ListViewWidget_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        LISTVIEW_LUA_NAME,
        LISTVIEW_LUA_FLAG,
        WIDGET_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    // Template
    REGISTER_TABLE_FUNC(L, mtIndex, SetItemTemplate);
    REGISTER_TABLE_FUNC(L, mtIndex, GetItemTemplate);

    // Data Management
    REGISTER_TABLE_FUNC(L, mtIndex, SetData);
    REGISTER_TABLE_FUNC(L, mtIndex, AddItem);
    REGISTER_TABLE_FUNC(L, mtIndex, RemoveItem);
    REGISTER_TABLE_FUNC(L, mtIndex, UpdateItem);
    REGISTER_TABLE_FUNC(L, mtIndex, Clear);
    REGISTER_TABLE_FUNC(L, mtIndex, GetItemCount);
    REGISTER_TABLE_FUNC(L, mtIndex, GetItemData);

    // Layout
    REGISTER_TABLE_FUNC(L, mtIndex, SetSpacing);
    REGISTER_TABLE_FUNC(L, mtIndex, GetSpacing);
    REGISTER_TABLE_FUNC(L, mtIndex, SetOrientation);
    REGISTER_TABLE_FUNC(L, mtIndex, GetOrientation);

    // Item sizing
    REGISTER_TABLE_FUNC(L, mtIndex, SetItemWidth);
    REGISTER_TABLE_FUNC(L, mtIndex, GetItemWidth);
    REGISTER_TABLE_FUNC(L, mtIndex, SetItemHeight);
    REGISTER_TABLE_FUNC(L, mtIndex, GetItemHeight);

    // Selection
    REGISTER_TABLE_FUNC(L, mtIndex, SetSelectedIndex);
    REGISTER_TABLE_FUNC(L, mtIndex, GetSelectedIndex);
    REGISTER_TABLE_FUNC(L, mtIndex, GetSelectedData);
    REGISTER_TABLE_FUNC(L, mtIndex, ClearSelection);

    // Access
    REGISTER_TABLE_FUNC(L, mtIndex, GetScrollContainer);
    REGISTER_TABLE_FUNC(L, mtIndex, GetArrayWidget);
    REGISTER_TABLE_FUNC(L, mtIndex, GetItem);

    // Scroll control
    REGISTER_TABLE_FUNC(L, mtIndex, ScrollToItem);

    lua_pop(L, 1);
}

#endif
