#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Assets/Level.h"

#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define LEVEL_LUA_NAME "Level"
#define LEVEL_LUA_FLAG "cfLevel"
#define CHECK_LEVEL(L, arg) CheckAssetLuaType<Level>(L, arg, LEVEL_LUA_NAME, LEVEL_LUA_FLAG)

struct Level_Lua
{
    static void Bind();
};

#endif
