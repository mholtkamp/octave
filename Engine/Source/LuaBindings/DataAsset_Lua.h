#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Assets/DataAsset.h"

#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define DATA_ASSET_LUA_NAME "DataAsset"
#define DATA_ASSET_LUA_FLAG "cfDataAsset"
#define CHECK_DATA_ASSET(L, arg) CheckAssetLuaType<DataAsset>(L, arg, DATA_ASSET_LUA_NAME, DATA_ASSET_LUA_FLAG)

struct DataAsset_Lua
{
    static int Get(lua_State* L);
    static int Set(lua_State* L);
    static int GetScriptFile(lua_State* L);
    static int SetScriptFile(lua_State* L);
    static int ReloadProperties(lua_State* L);

    static void Bind();
};

#endif
