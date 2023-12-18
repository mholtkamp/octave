#pragma once

#if 0
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
    static int IsMaterialInstance(lua_State* L);
    static int SetTexture(lua_State* L);
    static int GetTexture(lua_State* L);
    static int GetShadingModel(lua_State* L);
    static int SetShadingModel(lua_State* L);
    static int GetBlendMode(lua_State* L);
    static int SetBlendMode(lua_State* L);
    static int GetUvOffset(lua_State* L);
    static int SetUvOffset(lua_State* L);
    static int GetUvScale(lua_State* L);
    static int SetUvScale(lua_State* L);
    static int GetColor(lua_State* L);
    static int SetColor(lua_State* L);
    static int GetFresnelColor(lua_State* L);
    static int SetFresnelColor(lua_State* L);
    static int GetFresnelPower(lua_State* L);
    static int SetFresnelPower(lua_State* L);
    static int GetEmission(lua_State* L);
    static int SetEmission(lua_State* L);
    static int GetWrapLighting(lua_State* L);
    static int SetWrapLighting(lua_State* L);
    static int GetSpecular(lua_State* L);
    static int SetSpecular(lua_State* L);
    static int GetOpacity(lua_State* L);
    static int SetOpacity(lua_State* L);
    static int GetMaskCutoff(lua_State* L);
    static int SetMaskCutoff(lua_State* L);
    static int GetSortPriority(lua_State* L);
    static int SetSortPriority(lua_State* L);
    static int IsDepthTestDisabled(lua_State* L);
    static int SetDepthTestDisabled(lua_State* L);
    static int GetUvMap(lua_State* L);
    static int SetUvMap(lua_State* L);
    static int GetTevMode(lua_State* L);
    static int SetTevMode(lua_State* L);

    static void Bind();
};

#endif

#endif