#include "LuaBindings/Texture_Lua.h"
#include "LuaBindings/Asset_Lua.h"

#include "LuaBindings/Vector_Lua.h"

#if LUA_ENABLED

int Texture_Lua::IsMipmapped(lua_State* L)
{
    Texture* texture = CHECK_TEXTURE(L, 1);

    bool ret = texture->IsMipmapped();

    lua_pushboolean(L, ret);
    return 1;
}

int Texture_Lua::IsRenderTarget(lua_State* L)
{
    Texture* texture = CHECK_TEXTURE(L, 1);

    bool ret = texture->IsRenderTarget();

    lua_pushboolean(L, ret);
    return 1;
}

int Texture_Lua::GetWidth(lua_State* L)
{
    Texture* texture = CHECK_TEXTURE(L, 1);

    uint32_t ret = texture->GetWidth();

    lua_pushboolean(L, (int)ret);
    return 1;
}

int Texture_Lua::GetHeight(lua_State* L)
{
    Texture* texture = CHECK_TEXTURE(L, 1);

    uint32_t ret = texture->GetHeight();

    lua_pushboolean(L, (int)ret);
    return 1;
}

int Texture_Lua::GetMipLevels(lua_State* L)
{
    Texture* texture = CHECK_TEXTURE(L, 1);

    uint32_t ret = texture->GetMipLevels();

    lua_pushboolean(L, (int)ret);
    return 1;
}

int Texture_Lua::GetLayers(lua_State* L)
{
    Texture* texture = CHECK_TEXTURE(L, 1);

    uint32_t ret = texture->GetLayers();

    lua_pushboolean(L, (int)ret);
    return 1;
}

int Texture_Lua::GetFormat(lua_State* L)
{
    Texture* texture = CHECK_TEXTURE(L, 1);

    PixelFormat ret = texture->GetFormat();

    lua_pushboolean(L, (int)ret);
    return 1;
}

int Texture_Lua::GetFilterType(lua_State* L)
{
    Texture* texture = CHECK_TEXTURE(L, 1);

    FilterType ret = texture->GetFilterType();

    lua_pushboolean(L, (int)ret);
    return 1;
}

int Texture_Lua::GetWrapMode(lua_State* L)
{
    Texture* texture = CHECK_TEXTURE(L, 1);

    WrapMode ret = texture->GetWrapMode();

    lua_pushboolean(L, (int)ret);
    return 1;
}

void Texture_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        TEXTURE_LUA_NAME,
        TEXTURE_LUA_FLAG,
        ASSET_LUA_NAME);

    lua_pushcfunction(L, Asset_Lua::Destroy);
    lua_setfield(L, mtIndex, "__gc");

    lua_pushcfunction(L, IsMipmapped);
    lua_setfield(L, mtIndex, "IsMipmapped");

    lua_pushcfunction(L, IsRenderTarget);
    lua_setfield(L, mtIndex, "IsRenderTarget");

    lua_pushcfunction(L, GetWidth);
    lua_setfield(L, mtIndex, "GetWidth");

    lua_pushcfunction(L, GetHeight);
    lua_setfield(L, mtIndex, "GetHeight");

    lua_pushcfunction(L, GetMipLevels);
    lua_setfield(L, mtIndex, "GetMipLevels");

    lua_pushcfunction(L, GetLayers);
    lua_setfield(L, mtIndex, "GetLayers");

    lua_pushcfunction(L, GetFormat);
    lua_setfield(L, mtIndex, "GetFormat");

    lua_pushcfunction(L, GetFilterType);
    lua_setfield(L, mtIndex, "GetFilterType");

    lua_pushcfunction(L, GetWrapMode);
    lua_setfield(L, mtIndex, "GetWrapMode");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
