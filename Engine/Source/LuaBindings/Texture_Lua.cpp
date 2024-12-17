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

    lua_pushinteger(L, (int)ret);
    return 1;
}

int Texture_Lua::GetHeight(lua_State* L)
{
    Texture* texture = CHECK_TEXTURE(L, 1);

    uint32_t ret = texture->GetHeight();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int Texture_Lua::GetMipLevels(lua_State* L)
{
    Texture* texture = CHECK_TEXTURE(L, 1);

    uint32_t ret = texture->GetMipLevels();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int Texture_Lua::GetLayers(lua_State* L)
{
    Texture* texture = CHECK_TEXTURE(L, 1);

    uint32_t ret = texture->GetLayers();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int Texture_Lua::GetFormat(lua_State* L)
{
    Texture* texture = CHECK_TEXTURE(L, 1);

    PixelFormat ret = texture->GetFormat();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int Texture_Lua::GetFilterType(lua_State* L)
{
    Texture* texture = CHECK_TEXTURE(L, 1);

    FilterType ret = texture->GetFilterType();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int Texture_Lua::GetWrapMode(lua_State* L)
{
    Texture* texture = CHECK_TEXTURE(L, 1);

    WrapMode ret = texture->GetWrapMode();

    lua_pushinteger(L, (int)ret);
    return 1;
}

void Texture_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        TEXTURE_LUA_NAME,
        TEXTURE_LUA_FLAG,
        ASSET_LUA_NAME);

    Asset_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, IsMipmapped);

    REGISTER_TABLE_FUNC(L, mtIndex, IsRenderTarget);

    REGISTER_TABLE_FUNC(L, mtIndex, GetWidth);

    REGISTER_TABLE_FUNC(L, mtIndex, GetHeight);

    REGISTER_TABLE_FUNC(L, mtIndex, GetMipLevels);

    REGISTER_TABLE_FUNC(L, mtIndex, GetLayers);

    REGISTER_TABLE_FUNC(L, mtIndex, GetFormat);

    REGISTER_TABLE_FUNC(L, mtIndex, GetFilterType);

    REGISTER_TABLE_FUNC(L, mtIndex, GetWrapMode);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
