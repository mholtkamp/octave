#include "LuaBindings/PolyRect_Lua.h"
#include "LuaBindings/Widget_Lua.h"

#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Rect_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Texture_Lua.h"

#include "Assets/Texture.h"
#include "AssetManager.h"

#if LUA_ENABLED

int PolyRect_Lua::CreateNew(lua_State* L)
{
    return Widget_Lua::CreateNew(L, POLY_RECT_LUA_NAME);
}

void PolyRect_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        POLY_RECT_LUA_NAME,
        POLY_RECT_LUA_FLAG,
        WIDGET_LUA_NAME);

    Widget_Lua::BindCommon(L, mtIndex);

    lua_pushcfunction(L, CreateNew);
    lua_setfield(L, mtIndex, "Create");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
