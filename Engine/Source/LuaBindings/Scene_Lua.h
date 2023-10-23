#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Assets/Scene.h"

#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define SCENE_LUA_NAME "Scene"
#define SCENE_LUA_FLAG "cfScene"
#define CHECK_SCENE(L, arg) CheckAssetLuaType<Scene>(L, arg, SCENE_LUA_NAME, SCENE_LUA_FLAG)

struct Scene_Lua
{
    static int Capture(lua_State* L);
    static int Instantiate(lua_State* L);

    static void Bind();
};

#endif
