#include "LuaBindings/WindowManager_Lua.h"
#include "LuaBindings/Window_Lua.h"
#include "LuaBindings/Node_Lua.h"

#include "Nodes/Widgets/Window.h"

#if LUA_ENABLED

int WindowManager_Lua::FindWindow(lua_State* L)
{
    const char* id = CHECK_STRING(L, 1);

    Window* window = nullptr;
    if (WindowManager::Get() != nullptr)
    {
        window = WindowManager::Get()->FindWindow(id);
    }

    Node_Lua::Create(L, window);
    return 1;
}

int WindowManager_Lua::HasWindow(lua_State* L)
{
    const char* id = CHECK_STRING(L, 1);

    bool ret = false;
    if (WindowManager::Get() != nullptr)
    {
        ret = WindowManager::Get()->HasWindow(id);
    }

    lua_pushboolean(L, ret);
    return 1;
}

int WindowManager_Lua::ShowWindow(lua_State* L)
{
    const char* id = CHECK_STRING(L, 1);

    if (WindowManager::Get() != nullptr)
    {
        WindowManager::Get()->ShowWindow(id);
    }

    return 0;
}

int WindowManager_Lua::HideWindow(lua_State* L)
{
    const char* id = CHECK_STRING(L, 1);

    if (WindowManager::Get() != nullptr)
    {
        WindowManager::Get()->HideWindow(id);
    }

    return 0;
}

int WindowManager_Lua::CloseWindow(lua_State* L)
{
    const char* id = CHECK_STRING(L, 1);

    if (WindowManager::Get() != nullptr)
    {
        WindowManager::Get()->CloseWindow(id);
    }

    return 0;
}

int WindowManager_Lua::BringToFront(lua_State* L)
{
    const char* id = CHECK_STRING(L, 1);

    if (WindowManager::Get() != nullptr)
    {
        WindowManager::Get()->BringToFront(id);
    }

    return 0;
}

void WindowManager_Lua::Bind()
{
    lua_State* L = GetLua();

    lua_newtable(L);
    int tableIdx = lua_gettop(L);

    REGISTER_TABLE_FUNC(L, tableIdx, FindWindow);
    REGISTER_TABLE_FUNC(L, tableIdx, HasWindow);
    REGISTER_TABLE_FUNC(L, tableIdx, ShowWindow);
    REGISTER_TABLE_FUNC(L, tableIdx, HideWindow);
    REGISTER_TABLE_FUNC(L, tableIdx, CloseWindow);
    REGISTER_TABLE_FUNC(L, tableIdx, BringToFront);

    lua_setglobal(L, WINDOW_MANAGER_LUA_NAME);

    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
