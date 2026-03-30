#include "ToolTipManager_Lua.h"

#if LUA_ENABLED

#include "LuaBindings/Widget_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Node_Lua.h"
#include "LuaBindings/LuaTypeCheck.h"

#include "ToolTipManager.h"
#include "Nodes/Widgets/ToolTipWidget.h"

// Helper to push ToolTipWidget to Lua
static void PushToolTipWidgetToLua(lua_State* L, ToolTipWidget* widget)
{
    if (widget == nullptr)
    {
        lua_pushnil(L);
        return;
    }

    widget->RegisterScriptFuncs(L);

    Node** ud = (Node**)lua_newuserdata(L, sizeof(Node*));
    *ud = widget;

    luaL_getmetatable(L, "ToolTipWidget");
    if (lua_isnil(L, -1))
    {
        lua_pop(L, 1);
        luaL_getmetatable(L, WIDGET_LUA_NAME);
    }
    lua_setmetatable(L, -2);
}

static void PushWidgetToLua(lua_State* L, Widget* widget)
{
    if (widget == nullptr)
    {
        lua_pushnil(L);
        return;
    }

    widget->RegisterScriptFuncs(L);

    Node** ud = (Node**)lua_newuserdata(L, sizeof(Node*));
    *ud = widget;

    luaL_getmetatable(L, widget->GetClassName());
    if (lua_isnil(L, -1))
    {
        lua_pop(L, 1);
        luaL_getmetatable(L, NODE_LUA_NAME);
    }
    lua_setmetatable(L, -2);
}

// Configuration

int ToolTipManager_Lua::SetShowDelay(lua_State* L)
{
    float value = CHECK_NUMBER(L, 1);

    ToolTipManager* mgr = ToolTipManager::Get();
    if (mgr != nullptr)
    {
        mgr->SetShowDelay(value);
    }

    return 0;
}

int ToolTipManager_Lua::GetShowDelay(lua_State* L)
{
    float ret = 0.5f;

    ToolTipManager* mgr = ToolTipManager::Get();
    if (mgr != nullptr)
    {
        ret = mgr->GetShowDelay();
    }

    lua_pushnumber(L, ret);
    return 1;
}

int ToolTipManager_Lua::SetHideDelay(lua_State* L)
{
    float value = CHECK_NUMBER(L, 1);

    ToolTipManager* mgr = ToolTipManager::Get();
    if (mgr != nullptr)
    {
        mgr->SetHideDelay(value);
    }

    return 0;
}

int ToolTipManager_Lua::GetHideDelay(lua_State* L)
{
    float ret = 0.0f;

    ToolTipManager* mgr = ToolTipManager::Get();
    if (mgr != nullptr)
    {
        ret = mgr->GetHideDelay();
    }

    lua_pushnumber(L, ret);
    return 1;
}

int ToolTipManager_Lua::SetCursorOffset(lua_State* L)
{
    float x = CHECK_NUMBER(L, 1);
    float y = CHECK_NUMBER(L, 2);

    ToolTipManager* mgr = ToolTipManager::Get();
    if (mgr != nullptr)
    {
        mgr->SetCursorOffset(glm::vec2(x, y));
    }

    return 0;
}

int ToolTipManager_Lua::GetCursorOffset(lua_State* L)
{
    glm::vec2 ret(16.0f, 16.0f);

    ToolTipManager* mgr = ToolTipManager::Get();
    if (mgr != nullptr)
    {
        ret = mgr->GetCursorOffset();
    }

    Vector_Lua::Create(L, ret);
    return 1;
}

int ToolTipManager_Lua::SetEnabled(lua_State* L)
{
    bool value = CHECK_BOOLEAN(L, 1);

    ToolTipManager* mgr = ToolTipManager::Get();
    if (mgr != nullptr)
    {
        mgr->SetEnabled(value);
    }

    return 0;
}

int ToolTipManager_Lua::IsEnabled(lua_State* L)
{
    bool ret = true;

    ToolTipManager* mgr = ToolTipManager::Get();
    if (mgr != nullptr)
    {
        ret = mgr->IsEnabled();
    }

    lua_pushboolean(L, ret);
    return 1;
}

// Manual control

int ToolTipManager_Lua::Show(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);

    ToolTipManager* mgr = ToolTipManager::Get();
    if (mgr != nullptr)
    {
        mgr->ShowTooltip(widget);
    }

    return 0;
}

int ToolTipManager_Lua::Hide(lua_State* L)
{
    ToolTipManager* mgr = ToolTipManager::Get();
    if (mgr != nullptr)
    {
        mgr->HideTooltip();
    }

    return 0;
}

int ToolTipManager_Lua::GetWidget(lua_State* L)
{
    ToolTipManager* mgr = ToolTipManager::Get();
    if (mgr != nullptr)
    {
        PushToolTipWidgetToLua(L, mgr->GetToolTipWidget());
    }
    else
    {
        lua_pushnil(L);
    }

    return 1;
}

// State

int ToolTipManager_Lua::GetHoveredWidget(lua_State* L)
{
    ToolTipManager* mgr = ToolTipManager::Get();
    if (mgr != nullptr)
    {
        PushWidgetToLua(L, mgr->GetHoveredWidget());
    }
    else
    {
        lua_pushnil(L);
    }

    return 1;
}

int ToolTipManager_Lua::IsVisible(lua_State* L)
{
    bool ret = false;

    ToolTipManager* mgr = ToolTipManager::Get();
    if (mgr != nullptr)
    {
        ret = mgr->IsTooltipVisible();
    }

    lua_pushboolean(L, ret);
    return 1;
}

// Callbacks

int ToolTipManager_Lua::SetOnShowCallback(lua_State* L)
{
    ToolTipManager* mgr = ToolTipManager::Get();
    if (mgr == nullptr)
    {
        return 0;
    }

    if (lua_isstring(L, 1))
    {
        const char* funcName = lua_tostring(L, 1);
        mgr->SetOnShowCallback(funcName);
    }
    else if (lua_isfunction(L, 1))
    {
        lua_pushvalue(L, 1);
        int ref = luaL_ref(L, LUA_REGISTRYINDEX);
        mgr->SetOnShowCallbackRef(ref);
    }
    else if (lua_isnil(L, 1))
    {
        mgr->ClearOnShowCallback();
    }

    return 0;
}

int ToolTipManager_Lua::SetOnHideCallback(lua_State* L)
{
    ToolTipManager* mgr = ToolTipManager::Get();
    if (mgr == nullptr)
    {
        return 0;
    }

    if (lua_isstring(L, 1))
    {
        const char* funcName = lua_tostring(L, 1);
        mgr->SetOnHideCallback(funcName);
    }
    else if (lua_isfunction(L, 1))
    {
        lua_pushvalue(L, 1);
        int ref = luaL_ref(L, LUA_REGISTRYINDEX);
        mgr->SetOnHideCallbackRef(ref);
    }
    else if (lua_isnil(L, 1))
    {
        mgr->ClearOnHideCallback();
    }

    return 0;
}

// Templates

int ToolTipManager_Lua::RegisterTemplate(lua_State* L)
{
    const char* name = CHECK_STRING(L, 1);
    ToolTipWidget* tooltip = static_cast<ToolTipWidget*>(CheckNodeLuaType(L, 2, "ToolTipWidget", "cfToolTipWidget"));

    ToolTipManager* mgr = ToolTipManager::Get();
    if (mgr != nullptr)
    {
        mgr->RegisterTemplate(name, tooltip);
    }

    return 0;
}

int ToolTipManager_Lua::UnregisterTemplate(lua_State* L)
{
    ToolTipManager* mgr = ToolTipManager::Get();
    if (mgr == nullptr)
    {
        return 0;
    }

    if (lua_isstring(L, 1))
    {
        const char* name = lua_tostring(L, 1);
        mgr->UnregisterTemplate(name);
    }
    else if (lua_isuserdata(L, 1))
    {
        ToolTipWidget* tooltip = static_cast<ToolTipWidget*>(CheckNodeLuaType(L, 1, "ToolTipWidget", "cfToolTipWidget"));
        mgr->UnregisterTemplate(tooltip);
    }

    return 0;
}

int ToolTipManager_Lua::GetTemplate(lua_State* L)
{
    const char* name = CHECK_STRING(L, 1);

    ToolTipManager* mgr = ToolTipManager::Get();
    if (mgr != nullptr)
    {
        ToolTipWidget* tooltip = mgr->GetTemplate(name);
        if (tooltip != nullptr)
        {
            PushToolTipWidgetToLua(L, tooltip);
            return 1;
        }
    }

    lua_pushnil(L);
    return 1;
}

int ToolTipManager_Lua::HasTemplate(lua_State* L)
{
    const char* name = CHECK_STRING(L, 1);

    bool ret = false;
    ToolTipManager* mgr = ToolTipManager::Get();
    if (mgr != nullptr)
    {
        ret = mgr->HasTemplate(name);
    }

    lua_pushboolean(L, ret);
    return 1;
}

void ToolTipManager_Lua::Bind()
{
    lua_State* L = GetLua();

    // Create global "ToolTip" table
    lua_newtable(L);

    // Configuration
    lua_pushcfunction(L, SetShowDelay);
    lua_setfield(L, -2, "SetShowDelay");

    lua_pushcfunction(L, GetShowDelay);
    lua_setfield(L, -2, "GetShowDelay");

    lua_pushcfunction(L, SetHideDelay);
    lua_setfield(L, -2, "SetHideDelay");

    lua_pushcfunction(L, GetHideDelay);
    lua_setfield(L, -2, "GetHideDelay");

    lua_pushcfunction(L, SetCursorOffset);
    lua_setfield(L, -2, "SetCursorOffset");

    lua_pushcfunction(L, GetCursorOffset);
    lua_setfield(L, -2, "GetCursorOffset");

    lua_pushcfunction(L, SetEnabled);
    lua_setfield(L, -2, "SetEnabled");

    lua_pushcfunction(L, IsEnabled);
    lua_setfield(L, -2, "IsEnabled");

    // Manual control
    lua_pushcfunction(L, Show);
    lua_setfield(L, -2, "Show");

    lua_pushcfunction(L, Hide);
    lua_setfield(L, -2, "Hide");

    lua_pushcfunction(L, GetWidget);
    lua_setfield(L, -2, "GetWidget");

    // State
    lua_pushcfunction(L, GetHoveredWidget);
    lua_setfield(L, -2, "GetHoveredWidget");

    lua_pushcfunction(L, IsVisible);
    lua_setfield(L, -2, "IsVisible");

    // Callbacks
    lua_pushcfunction(L, SetOnShowCallback);
    lua_setfield(L, -2, "SetOnShowCallback");

    lua_pushcfunction(L, SetOnHideCallback);
    lua_setfield(L, -2, "SetOnHideCallback");

    // Templates
    lua_pushcfunction(L, RegisterTemplate);
    lua_setfield(L, -2, "RegisterTemplate");

    lua_pushcfunction(L, UnregisterTemplate);
    lua_setfield(L, -2, "UnregisterTemplate");

    lua_pushcfunction(L, GetTemplate);
    lua_setfield(L, -2, "GetTemplate");

    lua_pushcfunction(L, HasTemplate);
    lua_setfield(L, -2, "HasTemplate");

    // Set as global
    lua_setglobal(L, "ToolTip");
}

#endif
