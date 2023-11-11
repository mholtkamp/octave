#include "LuaBindings/TextMesh3d_Lua.h"
#include "LuaBindings/StaticMesh3d_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/LuaUtils.h"
#include "LuaBindings/Font_Lua.h"

#if LUA_ENABLED

int TextMesh3D_Lua::SetText(lua_State* L)
{
    TextMesh3D* textComp = CHECK_TEXT_MESH_3D(L, 1);
    const char* text = CHECK_STRING(L, 2);

    textComp->SetText(text);

    return 0;
}

int TextMesh3D_Lua::GetText(lua_State* L)
{
    TextMesh3D* textComp = CHECK_TEXT_MESH_3D(L, 1);

    const char* value = textComp->GetText().c_str();

    lua_pushstring(L, value);
    return 1;
}

int TextMesh3D_Lua::SetFont(lua_State* L)
{
    TextMesh3D* textComp = CHECK_TEXT_MESH_3D(L, 1);
    Font* font = CHECK_FONT(L, 2);

    textComp->SetFont(font);

    return 0;
}

int TextMesh3D_Lua::GetFont(lua_State* L)
{
    TextMesh3D* textComp = CHECK_TEXT_MESH_3D(L, 1);

    Font* font = textComp->GetFont();

    Asset_Lua::Create(L, font);
    return 1;
}

int TextMesh3D_Lua::SetColor(lua_State* L)
{
    TextMesh3D* textComp = CHECK_TEXT_MESH_3D(L, 1);
    glm::vec4 value = CHECK_VECTOR(L, 2);

    textComp->SetColor(value);

    return 0;
}

int TextMesh3D_Lua::GetColor(lua_State* L)
{
    TextMesh3D* textComp = CHECK_TEXT_MESH_3D(L, 1);

    glm::vec4 ret = textComp->GetColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int TextMesh3D_Lua::SetBlendMode(lua_State* L)
{
    TextMesh3D* textComp = CHECK_TEXT_MESH_3D(L, 1);
    int32_t blendMode = CHECK_INTEGER(L, 2);

    textComp->SetBlendMode((BlendMode)blendMode);

    return 0;
}

int TextMesh3D_Lua::GetBlendMode(lua_State* L)
{
    TextMesh3D* textComp = CHECK_TEXT_MESH_3D(L, 1);

    int32_t ret = (int32_t)textComp->GetBlendMode();

    lua_pushinteger(L, ret);
    return 1;
}

int TextMesh3D_Lua::SetHorizontalJustification(lua_State* L)
{
    TextMesh3D* textComp = CHECK_TEXT_MESH_3D(L, 1);
    float just = CHECK_NUMBER(L, 2);

    textComp->SetHorizontalJustification(just);

    return 0;
}

int TextMesh3D_Lua::GetHorizontalJustification(lua_State* L)
{
    TextMesh3D* textComp = CHECK_TEXT_MESH_3D(L, 1);

    float ret = textComp->GetHorizontalJustification();

    lua_pushnumber(L, ret);
    return 1;
}

int TextMesh3D_Lua::SetVerticalJustification(lua_State* L)
{
    TextMesh3D* textComp = CHECK_TEXT_MESH_3D(L, 1);
    float just = CHECK_NUMBER(L, 2);

    textComp->SetVerticalJustification(just);

    return 0;
}

int TextMesh3D_Lua::GetVerticalJustification(lua_State* L)
{
    TextMesh3D* textComp = CHECK_TEXT_MESH_3D(L, 1);

    float ret = textComp->GetVerticalJustification();

    lua_pushnumber(L, ret);
    return 1;
}

void TextMesh3D_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        TEXT_MESH_3D_LUA_NAME,
        TEXT_MESH_3D_LUA_FLAG,
        STATIC_MESH_3D_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, SetText);

    REGISTER_TABLE_FUNC(L, mtIndex, GetText);

    REGISTER_TABLE_FUNC(L, mtIndex, SetFont);

    REGISTER_TABLE_FUNC(L, mtIndex, GetFont);

    REGISTER_TABLE_FUNC(L, mtIndex, SetColor);

    REGISTER_TABLE_FUNC(L, mtIndex, GetColor);

    REGISTER_TABLE_FUNC(L, mtIndex, SetBlendMode);

    REGISTER_TABLE_FUNC(L, mtIndex, GetBlendMode);

    REGISTER_TABLE_FUNC(L, mtIndex, SetHorizontalJustification);

    REGISTER_TABLE_FUNC(L, mtIndex, GetHorizontalJustification);

    REGISTER_TABLE_FUNC(L, mtIndex, SetVerticalJustification);

    REGISTER_TABLE_FUNC(L, mtIndex, GetVerticalJustification);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
