#include "LuaBindings/ComboBox_Lua.h"
#include "LuaBindings/Selector_Lua.h"

#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Rect_Lua.h"

#include "Nodes/Widgets/VerticalList.h"

#if LUA_ENABLED

int ComboBox_Lua::GetList(lua_State* L)
{
    ComboBox* cb = CHECK_COMBO_BOX(L, 1);

    VerticalList* ret = cb->GetList();

    Node_Lua::Create(L, ret);
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

    Node_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, GetList);

    REGISTER_TABLE_FUNC(L, mtIndex, ShowList);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
