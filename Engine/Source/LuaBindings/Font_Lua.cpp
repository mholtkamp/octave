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

    lua_pushinteger(L, ret);
    return 1;
}

int Font_Lua::IsItalic(lua_State* L)
{
    Font* font = CHECK_FONT(L, 1);

    bool ret = font->IsItalic();

    lua_pushinteger(L, ret);
    return 1;
}

void Font_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        FONT_LUA_NAME,
        FONT_LUA_FLAG,
        ASSET_LUA_NAME);

    lua_pushcfunction(L, Asset_Lua::Destroy);
    lua_setfield(L, mtIndex, "__gc");

    lua_pushcfunction(L, GetSize);
    lua_setfield(L, mtIndex, "GetSize");

    lua_pushcfunction(L, GetWidth);
    lua_setfield(L, mtIndex, "GetWidth");

    lua_pushcfunction(L, GetHeight);
    lua_setfield(L, mtIndex, "GetHeight");

    lua_pushcfunction(L, GetTexture);
    lua_setfield(L, mtIndex, "GetTexture");

    lua_pushcfunction(L, IsBold);
    lua_setfield(L, mtIndex, "IsBold");

    lua_pushcfunction(L, IsItalic);
    lua_setfield(L, mtIndex, "IsItalic");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
