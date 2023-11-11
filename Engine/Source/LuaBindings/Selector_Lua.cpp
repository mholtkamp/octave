#include "LuaBindings/Selector_Lua.h"
#include "LuaBindings/Button_Lua.h"

#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Rect_Lua.h"

#if LUA_ENABLED

int Selector_Lua::AddSelection(lua_State* L)
{
    Selector* sel = CHECK_SELECTOR(L, 1);
    const char* stringSel = CHECK_STRING(L, 2);

    sel->AddSelection(stringSel);

    return 0;
}

int Selector_Lua::RemoveSelection(lua_State* L)
{
    Selector* sel = CHECK_SELECTOR(L, 1);
    const char* stringSel = CHECK_STRING(L, 2);

    sel->RemoveSelection(stringSel);

    return 0;
}

int Selector_Lua::RemoveAllSelections(lua_State* L)
{
    Selector* sel = CHECK_SELECTOR(L, 1);

    sel->RemoveAllSelections();

    return 0;
}

int Selector_Lua::GetNumSelections(lua_State* L)
{
    Selector* sel = CHECK_SELECTOR(L, 1);

    uint32_t num = sel->GetNumSelections();

    lua_pushinteger(L, (int)num);
    return 1;
}

int Selector_Lua::SetSelectionByString(lua_State* L)
{
    Selector* sel = CHECK_SELECTOR(L, 1);
    const char* selString = CHECK_STRING(L, 2);

    sel->SetSelectionByString(selString);

    return 0;
}

int Selector_Lua::SetSelectionIndex(lua_State* L)
{
    Selector* sel = CHECK_SELECTOR(L, 1);
    int32_t index = CHECK_INTEGER(L, 2);
    index--; // Convert from lua to c

    sel->SetSelectionIndex(index);

    return 0;
}

int Selector_Lua::Increment(lua_State* L)
{
    Selector* sel = CHECK_SELECTOR(L, 1);

    sel->Increment();

    return 0;
}

int Selector_Lua::Decrement(lua_State* L)
{
    Selector* sel = CHECK_SELECTOR(L, 1);

    sel->Decrement();

    return 0;
}

int Selector_Lua::GetSelectionString(lua_State* L)
{
    Selector* sel = CHECK_SELECTOR(L, 1);

    std::string selString = sel->GetSelectionString();
    const char* ret = selString.c_str();

    lua_pushstring(L, ret);
    return 1;
}

int Selector_Lua::GetSelectionIndex(lua_State* L)
{
    Selector* sel = CHECK_SELECTOR(L, 1);

    int32_t ret = sel->GetSelectionIndex();
    ret++; // Convert from c to lua

    lua_pushinteger(L, ret);
    return 1;
}


int Selector_Lua::SetSelectionChangeHandler(lua_State* L)
{
    Selector* sel = CHECK_SELECTOR(L, 1);
    CHECK_FUNCTION(L, 2);
    ScriptFunc func(L, 2);

    sel->SetScriptSelectionChangeHandler(func);

    return 0;
}

void Selector_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        SELECTOR_LUA_NAME,
        SELECTOR_LUA_FLAG,
        BUTTON_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, AddSelection);

    REGISTER_TABLE_FUNC(L, mtIndex, RemoveSelection);

    REGISTER_TABLE_FUNC(L, mtIndex, RemoveAllSelections);

    REGISTER_TABLE_FUNC(L, mtIndex, GetNumSelections);

    REGISTER_TABLE_FUNC(L, mtIndex, SetSelectionByString);

    REGISTER_TABLE_FUNC(L, mtIndex, SetSelectionIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, Increment);

    REGISTER_TABLE_FUNC(L, mtIndex, Decrement);

    REGISTER_TABLE_FUNC(L, mtIndex, GetSelectionString);

    REGISTER_TABLE_FUNC(L, mtIndex, GetSelectionIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, SetSelectionChangeHandler);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
