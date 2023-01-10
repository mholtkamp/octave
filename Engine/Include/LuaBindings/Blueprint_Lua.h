#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Assets/Blueprint.h"

#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define BLUEPRINT_LUA_NAME "Blueprint"
#define BLUEPRINT_LUA_FLAG "cfBlueprint"
#define CHECK_BLUEPRINT(L, arg) CheckAssetLuaType<Blueprint>(L, arg, BLUEPRINT_LUA_NAME, BLUEPRINT_LUA_FLAG)

struct Blueprint_Lua
{
    static void Bind();
};

#endif