#include "LuaBindings/WidgetMap_Lua.h"
#include "LuaBindings/Asset_Lua.h"

#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Widget_Lua.h"

#if LUA_ENABLED

int WidgetMap_Lua::Instantiate(lua_State* L)
{
    // Can call like WidgetMap.Instantiate('W_TestWidget')
    // Or use it on an object testWidgetMap:Instantiate()
    // The widget will be deallocated on garbage collect.
    WidgetMap* widgetMap = CHECK_WIDGET_MAP(L, 1);

    Widget* ret = widgetMap->Instantiate();
    ret->SetScriptOwned(true);

    Node_Lua::Create(L, ret);
    Widget_Lua* widgetLua = (Widget_Lua*)lua_touserdata(L, -1);
    widgetLua->mAlloced = true;

    return 1;
}

void WidgetMap_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        WIDGET_MAP_LUA_NAME,
        WIDGET_MAP_LUA_FLAG,
        ASSET_LUA_NAME);

    Asset_Lua::BindCommon(L, mtIndex);

    lua_pushcfunction(L, Instantiate);
    lua_setfield(L, mtIndex, "Instantiate");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
