#include "LuaBindings/Font_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Texture_Lua.h"

#include "LuaBindings/Vector_Lua.h"

#if LUA_ENABLED

int Font_Lua::GetSize(lua_State* L)
{
    Font* font = CHECK_FONT(L, 1);

    int32_t ret = font->GetSize();

    lua_pushinteger(L, ret);
    return 1;
}

int Font_Lua::GetWidth(lua_State* L)
{
    Font* font = CHECK_FONT(L, 1);

    int32_t ret = font->GetWidth();

    lua_pushinteger(L, ret);
    return 1;
}

int Font_Lua::GetHeight(lua_State* L)
{
    Font* font = CHECK_FONT(L, 1);

    int32_t ret = font->GetHeight();

    lua_pushinteger(L, ret);
    return 1;
}

int Font_Lua::GetTexture(lua_State* L)
{
    Font* font = CHECK_FONT(L, 1);

    Texture* ret = font->GetTexture();

    Asset_Lua::Create(L, ret);
    return 1;
}

int Font_Lua::IsBold(lua_State* L)
{
    Font* font = CHECK_FONT(L, 1);

    bool ret = font->IsBold();

    lua_pushboolean(L, ret);
    return 1;
}

int Font_Lua::IsItalic(lua_State* L)
{
    Font* font = CHECK_FONT(L, 1);

    bool ret = font->IsItalic();

    lua_pushboolean(L, ret);
    return 1;
}

void Font_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        FONT_LUA_NAME,
        FONT_LUA_FLAG,
        ASSET_LUA_NAME);

    Asset_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, GetSize);

    REGISTER_TABLE_FUNC(L, mtIndex, GetWidth);

    REGISTER_TABLE_FUNC(L, mtIndex, GetHeight);

    REGISTER_TABLE_FUNC(L, mtIndex, GetTexture);

    REGISTER_TABLE_FUNC(L, mtIndex, IsBold);

    REGISTER_TABLE_FUNC(L, mtIndex, IsItalic);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
