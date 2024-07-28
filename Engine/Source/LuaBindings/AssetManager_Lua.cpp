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

int AssetManager_Lua::SaveAsset(lua_State* L)
{
#if EDITOR
    const char* name = CHECK_STRING(L, 1);

    AssetManager::Get()->SaveAsset(name);
#endif
    return 0;
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

int AssetManager_Lua::CreateAndRegisterAsset(lua_State* L)
{
    const char* assetTypeStr = CHECK_STRING(L, 1);
    const char* assetDirStr = CHECK_STRING(L, 2);
    const char* assetNameStr = CHECK_STRING(L, 3);

    Asset* retAsset = nullptr;

    TypeId assetType = Asset::GetTypeIdFromName(assetTypeStr);
    AssetDir* assetDir = AssetManager::Get()->GetAssetDirFromPath(assetDirStr);
    AssetStub* stub = AssetManager::Get()->CreateAndRegisterAsset(assetType, assetDir, assetNameStr, false);

    if (stub != nullptr)
    {
        retAsset = stub->mAsset;
    }

    Asset_Lua::Create(L, retAsset);
    return 1;
}


void AssetManager_Lua::Bind()
{
    lua_State* L = GetLua();
    OCT_ASSERT(lua_gettop(L) == 0);

    lua_newtable(L);
    int tableIdx = lua_gettop(L);

    REGISTER_TABLE_FUNC(L, tableIdx, RefSweep);

    REGISTER_TABLE_FUNC(L, tableIdx, GetAsset);

    REGISTER_TABLE_FUNC(L, tableIdx, LoadAsset);

    REGISTER_TABLE_FUNC(L, tableIdx, SaveAsset);

    REGISTER_TABLE_FUNC(L, tableIdx, AsyncLoadAsset);

    REGISTER_TABLE_FUNC(L, tableIdx, UnloadAsset);

    REGISTER_TABLE_FUNC(L, tableIdx, CreateAndRegisterAsset);

    lua_setglobal(L, ASSET_MANAGER_LUA_NAME);
    OCT_ASSERT(lua_gettop(L) == 0);

    BindGlobalFunctions();
}

void AssetManager_Lua::BindGlobalFunctions()
{
    lua_State* L = GetLua();
    OCT_ASSERT(lua_gettop(L) == 0);

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

    OCT_ASSERT(lua_gettop(L) == 0);
}
