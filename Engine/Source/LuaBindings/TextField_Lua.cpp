#include "LuaBindings/TextField_Lua.h"
#include "LuaBindings/Button_Lua.h"

#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Rect_Lua.h"

#include "Components/ScriptComponent.h"

#if LUA_ENABLED

int TextField_Lua::CreateNew(lua_State* L)
{
    return Widget_Lua::CreateNew(L, TEXT_FIELD_LUA_NAME);
}

int TextField_Lua::SetTextEditHandler(lua_State* L)
{
    TextField* tf = CHECK_TEXT_FIELD(L, 1);
    const char* tableName = CheckTableName(L, 2);
    const char* funcName = CHECK_STRING(L, 3);

    tf->SetScriptTextEditHandler(
        tableName,
        funcName);

    return 0;
}

int TextField_Lua::SetTextConfirmHandler(lua_State* L)
{
    TextField* tf = CHECK_TEXT_FIELD(L, 1);
    const char* tableName = CheckTableName(L, 2);
    const char* funcName = CHECK_STRING(L, 3);

    tf->SetScriptTextConfirmHandler(
        tableName,
        funcName);

    return 0;
}

int TextField_Lua::GetSelectedTextField(lua_State* L)
{
    TextField* tf = TextField::GetSelectedTextField();

    Widget_Lua::Create(L, tf);
    return 1;
}


void TextField_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        TEXT_FIELD_LUA_NAME,
        TEXT_FIELD_LUA_FLAG,
        BUTTON_LUA_NAME);

    Widget_Lua::BindCommon(L, mtIndex);

    lua_pushcfunction(L, CreateNew);
    lua_setfield(L, mtIndex, "Create");

    lua_pushcfunction(L, SetTextEditHandler);
    lua_setfield(L, mtIndex, "SetTextEditHandler");

    lua_pushcfunction(L, SetTextConfirmHandler);
    lua_setfield(L, mtIndex, "SetTextConfirmHandler");

    lua_pushcfunction(L, GetSelectedTextField);
    lua_setfield(L, mtIndex, "GetSelectedTextField");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
