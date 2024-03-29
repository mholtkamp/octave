#include "LuaBindings/TextField_Lua.h"
#include "LuaBindings/Button_Lua.h"

#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Rect_Lua.h"

#if LUA_ENABLED

int TextField_Lua::SetTextEditHandler(lua_State* L)
{
    TextField* tf = CHECK_TEXT_FIELD(L, 1);
    CHECK_FUNCTION(L, 2);
    ScriptFunc func(L, 2);

    tf->SetScriptTextEditHandler(func);

    return 0;
}

int TextField_Lua::SetTextConfirmHandler(lua_State* L)
{
    TextField* tf = CHECK_TEXT_FIELD(L, 1);
    CHECK_FUNCTION(L, 2);
    ScriptFunc func(L, 2);

    tf->SetScriptTextConfirmHandler(func);

    return 0;
}

int TextField_Lua::GetSelectedTextField(lua_State* L)
{
    TextField* tf = TextField::GetSelectedTextField();

    Node_Lua::Create(L, tf);
    return 1;
}


void TextField_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        TEXT_FIELD_LUA_NAME,
        TEXT_FIELD_LUA_FLAG,
        BUTTON_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, SetTextEditHandler);

    REGISTER_TABLE_FUNC(L, mtIndex, SetTextConfirmHandler);

    REGISTER_TABLE_FUNC(L, mtIndex, GetSelectedTextField);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
