#include "LuaBindings/Canvas_Lua.h"
#include "LuaBindings/Widget_Lua.h"

#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Rect_Lua.h"

#if LUA_ENABLED

int Canvas_Lua::CreateNew(lua_State* L)
{
    return Widget_Lua::CreateNew(L, CANVAS_LUA_NAME);
}

void Canvas_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        CANVAS_LUA_NAME,
        CANVAS_LUA_FLAG,
        WIDGET_LUA_NAME);

    lua_pushcfunction(L, CreateNew);
    lua_setfield(L, mtIndex, "Create");

    lua_pushcfunction(L, Widget_Lua::Destroy);
    lua_setfield(L, mtIndex, "__gc");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
