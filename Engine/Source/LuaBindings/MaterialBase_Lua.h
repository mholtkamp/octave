#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Assets/Material.h"

#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define MATERIAL_LUA_NAME "Material"
#define MATERIAL_LUA_FLAG "cfMaterial"
#define CHECK_MATERIAL(L, arg) CheckAssetLuaType<Material>(L, arg, MATERIAL_LUA_NAME, MATERIAL_LUA_FLAG)

struct Material_Lua
{
    static int XXX(lua_State* L);


    static void Bind();
};

#endif
