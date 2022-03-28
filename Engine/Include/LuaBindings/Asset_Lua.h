#pragma once

#include "Engine.h"
#include "Asset.h"
#include "AssetRef.h"

#if LUA_ENABLED

#define ASSET_LUA_NAME "Asset"
#define ASSET_LUA_FLAG "cfAsset"
#define CHECK_ASSET(L, Arg) CheckAssetLuaType<Asset>(L, Arg, ASSET_LUA_NAME, ASSET_LUA_FLAG);
#define CHECK_ASSET_USERDATA(L, Arg) CheckHierarchyLuaType<Asset_Lua>(L, Arg, ASSET_LUA_NAME, ASSET_LUA_FLAG)->mAsset.Get();

struct Asset_Lua
{
    AssetRef mAsset;

    Asset_Lua() { }
    ~Asset_Lua() { }

    static int Create(lua_State* L, const Asset* asset, bool allowNull = false);
    static int Destroy(lua_State* L);

    static int GetName(lua_State* L);
    static int IsRefCounted(lua_State* L);
    static int GetRefCount(lua_State* L);

    static int GetTypeName(lua_State* L);
    static int IsTransient(lua_State* L);

    static int IsLoaded(lua_State* L);

    static void Bind();
};

#endif
