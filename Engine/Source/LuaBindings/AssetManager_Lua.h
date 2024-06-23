#pragma once

#include "Engine.h"
#include "AssetManager.h"
#include "LuaBindings/LuaUtils.h"

#define ASSET_MANAGER_LUA_NAME "AssetManager"

struct AssetManager_Lua
{
    static int RefSweep(lua_State* L);
    static int GetAsset(lua_State* L);
    static int LoadAsset(lua_State* L);
    static int SaveAsset(lua_State* L);
    static int AsyncLoadAsset(lua_State* L);
    static int UnloadAsset(lua_State* L);

    static void Bind();
    static void BindGlobalFunctions();
};

