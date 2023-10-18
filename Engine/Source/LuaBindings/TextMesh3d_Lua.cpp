#include "LuaBindings/TextMesh3d_Lua.h"
#include "LuaBindings/StaticMesh3d_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/LuaUtils.h"
#include "LuaBindings/Font_Lua.h"

#if LUA_ENABLED

int TextMesh3D_Lua::SetText(lua_State* L)
{
    TextMesh3D* textComp = CHECK_TEXT_MESH_COMPONENT(L, 1);
    const char* text = CHECK_STRING(L, 2);

    textComp->SetText(text);

    return 0;
}

int TextMesh3D_Lua::GetText(lua_State* L)
{
    TextMesh3D* textComp = CHECK_TEXT_MESH_COMPONENT(L, 1);

    const char* value = textComp->GetText().c_str();

    lua_pushstring(L, value);
    return 1;
}

int TextMesh3D_Lua::SetFont(lua_State* L)
{
    TextMesh3D* textComp = CHECK_TEXT_MESH_COMPONENT(L, 1);
    Font* font = CHECK_FONT(L, 2);

    textComp->SetFont(font);

    return 0;
}

int TextMesh3D_Lua::GetFont(lua_State* L)
{
    TextMesh3D* textComp = CHECK_TEXT_MESH_COMPONENT(L, 1);

    Font* font = textComp->GetFont();

    Asset_Lua::Create(L, font);
    return 1;
}

int TextMesh3D_Lua::SetColor(lua_State* L)
{
    TextMesh3D* textComp = CHECK_TEXT_MESH_COMPONENT(L, 1);
    glm::vec4 value = CHECK_VECTOR(L, 2);

    textComp->SetColor(value);

    return 0;
}

int TextMesh3D_Lua::GetColor(lua_State* L)
{
    TextMesh3D* textComp = CHECK_TEXT_MESH_COMPONENT(L, 1);

    glm::vec4 ret = textComp->GetColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int TextMesh3D_Lua::SetBlendMode(lua_State* L)
{
    TextMesh3D* textComp = CHECK_TEXT_MESH_COMPONENT(L, 1);
    int32_t blendMode = CHECK_INTEGER(L, 2);

    textComp->SetBlendMode((BlendMode)blendMode);

    return 0;
}

int TextMesh3D_Lua::GetBlendMode(lua_State* L)
{
    TextMesh3D* textComp = CHECK_TEXT_MESH_COMPONENT(L, 1);

    int32_t ret = (int32_t)textComp->GetBlendMode();

    lua_pushinteger(L, ret);
    return 1;
}

int TextMesh3D_Lua::SetHorizontalJustification(lua_State* L)
{
    TextMesh3D* textComp = CHECK_TEXT_MESH_COMPONENT(L, 1);
    float just = CHECK_NUMBER(L, 2);

    textComp->SetHorizontalJustification(just);

    return 0;
}

int TextMesh3D_Lua::GetHorizontalJustification(lua_State* L)
{
    TextMesh3D* textComp = CHECK_TEXT_MESH_COMPONENT(L, 1);

    float ret = textComp->GetHorizontalJustification();

    lua_pushnumber(L, ret);
    return 1;
}

int TextMesh3D_Lua::SetVerticalJustification(lua_State* L)
{
    TextMesh3D* textComp = CHECK_TEXT_MESH_COMPONENT(L, 1);
    float just = CHECK_NUMBER(L, 2);

    textComp->SetVerticalJustification(just);

    return 0;
}

int TextMesh3D_Lua::GetVerticalJustification(lua_State* L)
{
    TextMesh3D* textComp = CHECK_TEXT_MESH_COMPONENT(L, 1);

    float ret = textComp->GetVerticalJustification();

    lua_pushnumber(L, ret);
    return 1;
}

void TextMesh3D_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        TEXT_MESH_COMPONENT_LUA_NAME,
        TEXT_MESH_COMPONENT_LUA_FLAG,
        STATIC_MESH_COMPONENT_LUA_NAME);

    Component_Lua::BindCommon(L, mtIndex);

    lua_pushcfunction(L, SetText);
    lua_setfield(L, mtIndex, "SetText");

    lua_pushcfunction(L, GetText);
    lua_setfield(L, mtIndex, "GetText");

    lua_pushcfunction(L, SetFont);
    lua_setfield(L, mtIndex, "SetFont");

    lua_pushcfunction(L, GetFont);
    lua_setfield(L, mtIndex, "GetFont");

    lua_pushcfunction(L, SetColor);
    lua_setfield(L, mtIndex, "SetColor");

    lua_pushcfunction(L, GetColor);
    lua_setfield(L, mtIndex, "GetColor");

    lua_pushcfunction(L, SetBlendMode);
    lua_setfield(L, mtIndex, "SetBlendMode");

    lua_pushcfunction(L, GetBlendMode);
    lua_setfield(L, mtIndex, "GetBlendMode");

    lua_pushcfunction(L, SetHorizontalJustification);
    lua_setfield(L, mtIndex, "SetHorizontalJustification");

    lua_pushcfunction(L, GetHorizontalJustification);
    lua_setfield(L, mtIndex, "GetHorizontalJustification");

    lua_pushcfunction(L, SetVerticalJustification);
    lua_setfield(L, mtIndex, "SetVerticalJustification");

    lua_pushcfunction(L, GetVerticalJustification);
    lua_setfield(L, mtIndex, "GetVerticalJustification");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
