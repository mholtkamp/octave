#include "LuaBindings/Canvas_Lua.h"
#include "LuaBindings/Widget_Lua.h"
#include "LuaBindings/Asset_Lua.h"

#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Rect_Lua.h"

#include "UI/UIDocument.h"

#if LUA_ENABLED

int Canvas_Lua::GetUIDocument(lua_State* L)
{
    Canvas* canvas = CHECK_CANVAS(L, 1);
    UIDocument* doc = canvas->GetRuntimeDocument();

    if (doc)
    {
        Asset_Lua::Create(L, doc);
    }
    else
    {
        lua_pushnil(L);
    }

    return 1;
}

void Canvas_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        CANVAS_LUA_NAME,
        CANVAS_LUA_FLAG,
        WIDGET_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, GetUIDocument);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
