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
    static int IsBase(lua_State* L);
    static int IsInstance(lua_State* L);
    static int IsLite(lua_State* L);

    static int SetScalarParameter(lua_State* L);
    static int SetVectorParameter(lua_State* L);
    static int SetTextureParameter(lua_State* L);

    static int GetScalarParameter(lua_State* L);
    static int GetVectorParameter(lua_State* L);
    static int GetTextureParameter(lua_State* L);

    static int GetBlendMode(lua_State* L);
    static int GetMaskCutoff(lua_State* L);
    static int GetSortPriority(lua_State* L);
    static int IsDepthTestDisabled(lua_State* L);
    static int ShouldApplyFog(lua_State* L);
    static int GetCullMode(lua_State* L);

    static void Bind();
};

#endif
