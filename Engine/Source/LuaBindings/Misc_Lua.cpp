#include "LuaBindings/Misc_Lua.h"

#include "Widgets/Widget.h"
#include "Widgets/Button.h"
#include "Widgets/Text.h"

#if LUA_ENABLED

void BindBlendMode()
{
    lua_State* L = GetLua();
    assert(lua_gettop(L) == 0);

    lua_newtable(L);
    int tableIdx = lua_gettop(L);

    lua_pushinteger(L, (int)BlendMode::Opaque);
    lua_setfield(L, tableIdx, "Opaque");

    lua_pushinteger(L, (int)BlendMode::Masked);
    lua_setfield(L, tableIdx, "Masked");

    lua_pushinteger(L, (int)BlendMode::Translucent);
    lua_setfield(L, tableIdx, "Translucent");

    lua_pushinteger(L, (int)BlendMode::Additive);
    lua_setfield(L, tableIdx, "Additive");

    lua_pushinteger(L, (int)BlendMode::Count);
    lua_setfield(L, tableIdx, "Count");

    lua_setglobal(L, "BlendMode");

    assert(lua_gettop(L) == 0);
}

void BindShadingModel()
{
    lua_State* L = GetLua();
    assert(lua_gettop(L) == 0);

    lua_newtable(L);
    int tableIdx = lua_gettop(L);

    lua_pushinteger(L, (int)ShadingModel::Unlit);
    lua_setfield(L, tableIdx, "Unlit");

    lua_pushinteger(L, (int)ShadingModel::Lit);
    lua_setfield(L, tableIdx, "Lit");

    lua_pushinteger(L, (int)ShadingModel::Toon);
    lua_setfield(L, tableIdx, "Toon");

    lua_pushinteger(L, (int)ShadingModel::Count);
    lua_setfield(L, tableIdx, "Count");

    lua_setglobal(L, "ShadingModel");

    assert(lua_gettop(L) == 0);
}

void BindTevMode()
{
    lua_State* L = GetLua();
    assert(lua_gettop(L) == 0);

    lua_newtable(L);
    int tableIdx = lua_gettop(L);

    lua_pushinteger(L, (int)TevMode::Replace);
    lua_setfield(L, tableIdx, "Replace");

    lua_pushinteger(L, (int)TevMode::Modulate);
    lua_setfield(L, tableIdx, "Modulate");

    lua_pushinteger(L, (int)TevMode::Decal);
    lua_setfield(L, tableIdx, "Decal");

    lua_pushinteger(L, (int)TevMode::Add);
    lua_setfield(L, tableIdx, "Add");

    lua_pushinteger(L, (int)TevMode::SignedAdd);
    lua_setfield(L, tableIdx, "SignedAdd");

    lua_pushinteger(L, (int)TevMode::Subtract);
    lua_setfield(L, tableIdx, "Subtract");

    lua_pushinteger(L, (int)TevMode::Interpolate);
    lua_setfield(L, tableIdx, "Interpolate");

    lua_pushinteger(L, (int)TevMode::Pass);
    lua_setfield(L, tableIdx, "Pass");

    lua_pushinteger(L, (int)TevMode::Count);
    lua_setfield(L, tableIdx, "Count");

    lua_setglobal(L, "TevMode");

    assert(lua_gettop(L) == 0);
}

void BindTextureSlot()
{
    lua_State* L = GetLua();
    assert(lua_gettop(L) == 0);

    lua_newtable(L);
    int tableIdx = lua_gettop(L);

    lua_pushinteger(L, (int)TextureSlot::TEXTURE_0);
    lua_setfield(L, tableIdx, "Slot0");

    lua_pushinteger(L, (int)TextureSlot::TEXTURE_1);
    lua_setfield(L, tableIdx, "Slot1");

    lua_pushinteger(L, (int)TextureSlot::TEXTURE_2);
    lua_setfield(L, tableIdx, "Slot2");

    lua_pushinteger(L, (int)TextureSlot::TEXTURE_3);
    lua_setfield(L, tableIdx, "Slot3");

    lua_setglobal(L, "TextureSlot");

    assert(lua_gettop(L) == 0);
}

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

    lua_pushinteger(L, (int)DatumType::Table);
    lua_setfield(L, tableIdx, "Table");

    lua_pushinteger(L, (int)DatumType::Pointer);
    lua_setfield(L, tableIdx, "Pointer");

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

void BindJustification()
{
    lua_State* L = GetLua();
    assert(lua_gettop(L) == 0);

    lua_newtable(L);
    int tableIdx = lua_gettop(L);

    lua_pushinteger(L, (int)Justification::Left);
    lua_setfield(L, tableIdx, "Left");

    lua_pushinteger(L, (int)Justification::Right);
    lua_setfield(L, tableIdx, "Right");

    lua_pushinteger(L, (int)Justification::Center);
    lua_setfield(L, tableIdx, "Center");

    lua_pushinteger(L, (int)Justification::Top);
    lua_setfield(L, tableIdx, "Top");

    lua_pushinteger(L, (int)Justification::Bottom);
    lua_setfield(L, tableIdx, "Bottom");

    lua_setglobal(L, "Justification");

    assert(lua_gettop(L) == 0);
}

void Misc_Lua::BindMisc()
{
    BindBlendMode();
    BindShadingModel();
    BindTevMode();
    BindTextureSlot();
    BindAnchorMode();
    BindButtonState();
    BindDatumType();
    BindNetFuncType();
}

#endif
