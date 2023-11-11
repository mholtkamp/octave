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

    lua_pushcfunction(L, AddSelection);
    lua_setfield(L, mtIndex, "AddSelection");

    lua_pushcfunction(L, RemoveSelection);
    lua_setfield(L, mtIndex, "RemoveSelection");

    lua_pushcfunction(L, RemoveAllSelections);
    lua_setfield(L, mtIndex, "RemoveAllSelections");

    lua_pushcfunction(L, GetNumSelections);
    lua_setfield(L, mtIndex, "GetNumSelections");

    lua_pushcfunction(L, SetSelectionByString);
    lua_setfield(L, mtIndex, "SetSelectionByString");

    lua_pushcfunction(L, SetSelectionIndex);
    lua_setfield(L, mtIndex, "SetSelectionIndex");

    lua_pushcfunction(L, Increment);
    lua_setfield(L, mtIndex, "Increment");

    lua_pushcfunction(L, Decrement);
    lua_setfield(L, mtIndex, "Decrement");

    lua_pushcfunction(L, GetSelectionString);
    lua_setfield(L, mtIndex, "GetSelectionString");

    lua_pushcfunction(L, GetSelectionIndex);
    lua_setfield(L, mtIndex, "GetSelectionIndex");

    lua_pushcfunction(L, SetSelectionChangeHandler);
    lua_setfield(L, mtIndex, "SetSelectionChangeHandler");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
