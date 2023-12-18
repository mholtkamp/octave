#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Assets/MaterialBase.h"

#include "LuaBindings/Material_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define MATERIAL_BASE_LUA_NAME "MaterialBase"
#define MATERIAL_BASE_LUA_FLAG "cfMaterialBase"
#define CHECK_MATERIAL_BASE(L, arg) CheckAssetLuaType<MaterialBase>(L, arg, MATERIAL_BASE_LUA_NAME, MATERIAL_BASE_LUA_FLAG)

struct MaterialBase_Lua
{
    static void Bind();
};

#endif
