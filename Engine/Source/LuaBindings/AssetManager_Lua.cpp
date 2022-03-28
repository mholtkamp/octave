#include "LuaBindings/AssetManager_Lua.h"
#include "LuaBindings/Asset_Lua.h"

int AssetManager_Lua::RefSweep(lua_State* L)
{
    AssetManager::Get()->RefSweep();
    return 0;
}

int AssetManager_Lua::GetAsset(lua_State* L)
{
    const char* name = CHECK_STRING(L, 1);

    Asset* asset = AssetManager::Get()->GetAsset(name);

    Asset_Lua::Create(L, asset);
    return 1;
}

int AssetManager_Lua::LoadAsset(lua_State* L)
{
    const char* name = CHECK_STRING(L, 1);

    Asset* asset = AssetManager::Get()->LoadAsset(name);

    Asset_Lua::Create(L, asset);
    return 1;
}

int AssetManager_Lua::AsyncLoadAsset(lua_State* L)
{
    const char* name = CHECK_STRING(L, 1);

    // Create an Asset_Lua object with a null mAsset member.
    // The async load functionality will fill in the null member after the load as finished.
    // If the asset is already loaded, then the value will be set immediately.
    Asset_Lua::Create(L, nullptr, true);
    Asset_Lua* assetLua = (Asset_Lua*) lua_touserdata(L, -1);

    AssetManager::Get()->AsyncLoadAsset(name, &assetLua->mAsset);

    // The newly created Asset_Lua userdata should be on top of the stack.
    return 1;
}

int AssetManager_Lua::UnloadAsset(lua_State* L)
{
    const char* name = CHECK_STRING(L, 1);

    AssetManager::Get()->UnloadAsset(name);

    return 0;
}


void AssetManager_Lua::Bind()
{
    lua_State* L = GetLua();
    assert(lua_gettop(L) == 0);

    lua_newtable(L);
    int tableIdx = lua_gettop(L);

    lua_pushcfunction(L, RefSweep);
    lua_setfield(L, tableIdx, "RefSweep");

    lua_pushcfunction(L, GetAsset);
    lua_setfield(L, tableIdx, "GetAsset");

    lua_pushcfunction(L, LoadAsset);
    lua_setfield(L, tableIdx, "LoadAsset");

    lua_pushcfunction(L, AsyncLoadAsset);
    lua_setfield(L, tableIdx, "AsyncLoadAsset");

    lua_pushcfunction(L, UnloadAsset);
    lua_setfield(L, tableIdx, "UnloadAsset");

    lua_setglobal(L, ASSET_MANAGER_LUA_NAME);
    assert(lua_gettop(L) == 0);

    BindGlobalFunctions();
}

void AssetManager_Lua::BindGlobalFunctions()
{
    lua_State* L = GetLua();
    assert(lua_gettop(L) == 0);

    lua_pushcfunction(L, RefSweep);
    lua_setglobal(L, "RefSweep");

    lua_pushcfunction(L, GetAsset);
    lua_setglobal(L, "GetAsset");

    lua_pushcfunction(L, LoadAsset);
    lua_setglobal(L, "LoadAsset");

    lua_pushcfunction(L, AsyncLoadAsset);
    lua_setglobal(L, "AsyncLoadAsset");

    lua_pushcfunction(L, UnloadAsset);
    lua_setglobal(L, "UnloadAsset");

    assert(lua_gettop(L) == 0);
}
