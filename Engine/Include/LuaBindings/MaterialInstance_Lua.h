#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Assets/MaterialInstance.h"

#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define MATERIAL_INSTANCE_LUA_NAME "MaterialInstance"
#define MATERIAL_INSTANCE_LUA_FLAG "cfMaterialInstance"
#define CHECK_MATERIAL_INSTANCE(L, arg) CheckAssetLuaType<MaterialInstance>(L, arg, MATERIAL_INSTANCE_LUA_NAME, MATERIAL_INSTANCE_LUA_FLAG)

struct MaterialInstance_Lua
{
    static int CreateNew(lua_State* L);

    static void Bind();
};

#endif
