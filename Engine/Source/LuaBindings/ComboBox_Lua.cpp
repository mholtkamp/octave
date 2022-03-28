#include "LuaBindings/ComboBox_Lua.h"
#include "LuaBindings/Selector_Lua.h"

#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Rect_Lua.h"

#include "Widgets/VerticalList.h"

#if LUA_ENABLED

int ComboBox_Lua::CreateNew(lua_State* L)
{
    return Widget_Lua::CreateNew(L, COMBO_BOX_LUA_NAME);
}

int ComboBox_Lua::GetList(lua_State* L)
{
    ComboBox* cb = CHECK_COMBO_BOX(L, 1);

    VerticalList* ret = cb->GetList();

    Widget_Lua::Create(L, ret);
    return 1;
}

int ComboBox_Lua::ShowList(lua_State* L)
{
    ComboBox* cb = CHECK_COMBO_BOX(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    cb->ShowList(value);

    return 0;
}

void ComboBox_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        COMBO_BOX_LUA_NAME,
        COMBO_BOX_LUA_FLAG,
        SELECTOR_LUA_NAME);

    lua_pushcfunction(L, CreateNew);
    lua_setfield(L, mtIndex, "Create");

    lua_pushcfunction(L, Widget_Lua::Destroy);
    lua_setfield(L, mtIndex, "__gc");

    lua_pushcfunction(L, GetList);
    lua_setfield(L, mtIndex, "GetList");

    lua_pushcfunction(L, ShowList);
    lua_setfield(L, mtIndex, "ShowList");

    lua_pop(L, 1);
    assert(lua_gettop(L) == 0);
}

#endif
