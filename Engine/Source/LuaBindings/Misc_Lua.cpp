#include "LuaBindings/Misc_Lua.h"

#include "Widgets/Widget.h"
#include "Widgets/Button.h"

#if LUA_ENABLED

void BindAnchorMode()
{
    lua_State* L = GetLua();
    assert(lua_gettop(L) == 0);

    lua_newtable(L);
    int tableIdx = lua_gettop(L);

    lua_pushinteger(L, (int)AnchorMode::TopLeft);
    lua_setfield(L, tableIdx, "TopLeft");

    lua_pushinteger(L, (int)AnchorMode::TopMid);
    lua_setfield(L, tableIdx, "TopMid");

    lua_pushinteger(L, (int)AnchorMode::TopRight);
    lua_setfield(L, tableIdx, "TopRight");

    lua_pushinteger(L, (int)AnchorMode::MidLeft);
    lua_setfield(L, tableIdx, "MidLeft");

    lua_pushinteger(L, (int)AnchorMode::Mid);
    lua_setfield(L, tableIdx, "Mid");

    lua_pushinteger(L, (int)AnchorMode::MidRight);
    lua_setfield(L, tableIdx, "MidRight");

    lua_pushinteger(L, (int)AnchorMode::BottomLeft);
    lua_setfield(L, tableIdx, "BottomLeft");

    lua_pushinteger(L, (int)AnchorMode::BottomMid);
    lua_setfield(L, tableIdx, "BottomMid");

    lua_pushinteger(L, (int)AnchorMode::BottomRight);
    lua_setfield(L, tableIdx, "BottomRight");

    lua_pushinteger(L, (int)AnchorMode::TopStretch);
    lua_setfield(L, tableIdx, "TopStretch");

    lua_pushinteger(L, (int)AnchorMode::MidHorizontalStretch);
    lua_setfield(L, tableIdx, "MidHorizontalStretch");

    lua_pushinteger(L, (int)AnchorMode::BottomStretch);
    lua_setfield(L, tableIdx, "BottomStretch");

    lua_pushinteger(L, (int)AnchorMode::LeftStretch);
    lua_setfield(L, tableIdx, "LeftStretch");

    lua_pushinteger(L, (int)AnchorMode::MidVerticalStretch);
    lua_setfield(L, tableIdx, "MidVerticalStretch");

    lua_pushinteger(L, (int)AnchorMode::RightStretch);
    lua_setfield(L, tableIdx, "RightStretch");

    lua_pushinteger(L, (int)AnchorMode::FullStretch);
    lua_setfield(L, tableIdx, "FullStretch");

    lua_setglobal(L, "AnchorMode");

    assert(lua_gettop(L) == 0);
}

void BindButtonState()
{
    lua_State* L = GetLua();
    assert(lua_gettop(L) == 0);

    lua_newtable(L);
    int tableIdx = lua_gettop(L);

    lua_pushinteger(L, (int)ButtonState::Normal);
    lua_setfield(L, tableIdx, "Normal");

    lua_pushinteger(L, (int)ButtonState::Hovered);
    lua_setfield(L, tableIdx, "Hovered");

    lua_pushinteger(L, (int)ButtonState::Pressed);
    lua_setfield(L, tableIdx, "Pressed");

    lua_pushinteger(L, (int)ButtonState::Disabled);
    lua_setfield(L, tableIdx, "Disabled");

    lua_pushinteger(L, (int)ButtonState::Num);
    lua_setfield(L, tableIdx, "Num");

    lua_setglobal(L, "ButtonState");

    assert(lua_gettop(L) == 0);
}


void BindDatumType()
{
    lua_State* L = GetLua();
    assert(lua_gettop(L) == 0);

    lua_newtable(L);
    int tableIdx = lua_gettop(L);

    lua_pushinteger(L, (int)DatumType::Integer);
    lua_setfield(L, tableIdx, "Integer");

    lua_pushinteger(L, (int)DatumType::Float);
    lua_setfield(L, tableIdx, "Float");

    lua_pushinteger(L, (int)DatumType::Bool);
    lua_setfield(L, tableIdx, "Bool");

    lua_pushinteger(L, (int)DatumType::String);
    lua_setfield(L, tableIdx, "String");

    lua_pushinteger(L, (int)DatumType::Vector2D);
    lua_setfield(L, tableIdx, "Vector2D");

    lua_pushinteger(L, (int)DatumType::Vector);
    lua_setfield(L, tableIdx, "Vector");

    lua_pushinteger(L, (int)DatumType::Color);
    lua_setfield(L, tableIdx, "Color");

    lua_pushinteger(L, (int)DatumType::Asset);
    lua_setfield(L, tableIdx, "Asset");

    lua_pushinteger(L, (int)DatumType::Enum);
    lua_setfield(L, tableIdx, "Enum");

    lua_pushinteger(L, (int)DatumType::Byte);
    lua_setfield(L, tableIdx, "Byte");

    lua_pushinteger(L, (int)DatumType::Count);
    lua_setfield(L, tableIdx, "Count");

    lua_setglobal(L, "DatumType");

    assert(lua_gettop(L) == 0);
}

void BindNetFuncType()
{
    lua_State* L = GetLua();
    assert(lua_gettop(L) == 0);

    lua_newtable(L);
    int tableIdx = lua_gettop(L);

    lua_pushinteger(L, (int)NetFuncType::Server);
    lua_setfield(L, tableIdx, "Server");

    lua_pushinteger(L, (int)NetFuncType::Client);
    lua_setfield(L, tableIdx, "Client");

    lua_pushinteger(L, (int)NetFuncType::Multicast);
    lua_setfield(L, tableIdx, "Multicast");

    lua_pushinteger(L, (int)NetFuncType::Count);
    lua_setfield(L, tableIdx, "Count");

    lua_setglobal(L, "NetFuncType");

    assert(lua_gettop(L) == 0);
}

void Misc_Lua::BindMisc()
{
    BindAnchorMode();
    BindButtonState();
    BindDatumType();
    BindNetFuncType();
}

#endif
