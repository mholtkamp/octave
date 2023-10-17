#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Assets/Texture.h"

#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define TEXTURE_LUA_NAME "Texture"
#define TEXTURE_LUA_FLAG "cfTexture"
#define CHECK_TEXTURE(L, arg) CheckAssetLuaType<Texture>(L, arg, TEXTURE_LUA_NAME, TEXTURE_LUA_FLAG)

struct Texture_Lua
{
    static int IsMipmapped(lua_State* L);
    static int IsRenderTarget(lua_State* L);
    static int GetWidth(lua_State* L);
    static int GetHeight(lua_State* L);
    static int GetMipLevels(lua_State* L);
    static int GetLayers(lua_State* L);
    static int GetFormat(lua_State* L);
    static int GetFilterType(lua_State* L);
    static int GetWrapMode(lua_State* L);

    static void Bind();
};

#endif
