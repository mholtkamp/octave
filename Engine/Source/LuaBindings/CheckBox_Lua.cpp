#include "LuaBindings/CheckBox_Lua.h"
#include "LuaBindings/Selector_Lua.h"

#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Rect_Lua.h"

#if LUA_ENABLED

int CheckBox_Lua::CreateNew(lua_State* L)
{
    return Widget_Lua::CreateNew(L, CHECK_BOX_LUA_NAME);
}

int CheckBox_Lua::IsChecked(lua_State* L)
{
    CheckBox* cb = CHECK_CHECK_BOX(L, 1);

    bool ret = cb->IsChecked();

    lua_pushboolean(L, ret);
    return 1;
}

int CheckBox_Lua::SetChecked(lua_State* L)
{
    CheckBox* cb = CHECK_CHECK_BOX(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    cb->SetChecked(value);

    return 0;
}

void CheckBox_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        CHECK_BOX_LUA_NAME,
        CHECK_BOX_LUA_FLAG,
        SELECTOR_LUA_NAME);

    Widget_Lua::BindCommon(L, mtIndex);

    lua_pushcfunction(L, CreateNew);
    lua_setfield(L, mtIndex, "Create");

    lua_pushcfunction(L, IsChecked);
    lua_setfield(L, mtIndex, "IsChecked");

    lua_pushcfunction(L, SetChecked);
    lua_setfield(L, mtIndex, "SetChecked");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
