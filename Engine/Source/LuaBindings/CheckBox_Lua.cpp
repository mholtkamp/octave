#include "LuaBindings/CheckBox_Lua.h"
#include "LuaBindings/Selector_Lua.h"

#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Rect_Lua.h"

#if LUA_ENABLED

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

    Node_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, IsChecked);

    REGISTER_TABLE_FUNC(L, mtIndex, SetChecked);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
