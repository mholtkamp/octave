#include "LuaBindings/Poly_Lua.h"
#include "LuaBindings/Widget_Lua.h"

#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Rect_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Texture_Lua.h"

#include "Assets/Texture.h"
#include "AssetManager.h"
#include "Utilities.h"

#if LUA_ENABLED

int Poly_Lua::AddVertex(lua_State* L)
{
    Poly* poly = CHECK_POLY(L, 1);
    glm::vec2 pos = CHECK_VECTOR(L, 2);
    glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
    glm::vec2 uv = { 0.0f, 0.0f };
    if (!lua_isnone(L, 3)) { color = CHECK_VECTOR(L, 3); }
    if (!lua_isnone(L, 4)) { uv = CHECK_VECTOR(L, 4); }

    poly->AddVertex(pos, color, uv);

    return 0;
}

int Poly_Lua::ClearVertices(lua_State* L)
{
    Poly* poly = CHECK_POLY(L, 1);

    poly->ClearVertices();

    return 0;
}

int Poly_Lua::GetNumVertices(lua_State* L)
{
    Poly* poly = CHECK_POLY(L, 1);

    uint32_t ret = poly->GetNumVertices();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int Poly_Lua::GetVertex(lua_State* L)
{
    Poly* poly = CHECK_POLY(L, 1);
    int32_t index = CHECK_INTEGER(L, 2);
    index--; // Convert to 0 based

    VertexUI vert = poly->GetVertex((uint32_t)index);

    Datum table;
    table.SetVectorField("position", glm::vec4(vert.mPosition, 0.0f, 0.0f));
    table.SetVectorField("color", ColorUint32ToFloat4(vert.mColor));
    table.SetVectorField("uv", glm::vec4(vert.mTexcoord, 0.0f, 0.0f));
    LuaPushDatum(L, table);

    return 1;
}

int Poly_Lua::SetTexture(lua_State* L)
{
    Poly* poly = CHECK_POLY(L, 1);
    Texture* texture = CHECK_TEXTURE(L, 2);

    poly->SetTexture(texture);

    return 0;
}

int Poly_Lua::GetTexture(lua_State* L)
{
    Poly* poly = CHECK_POLY(L, 1);

    Texture* ret = poly->GetTexture();

    Asset_Lua::Create(L, ret);
    return 1;
}


int Poly_Lua::GetLineWidth(lua_State* L)
{
    Poly* poly = CHECK_POLY(L, 1);

    float ret = poly->GetLineWidth();

    lua_pushnumber(L, ret);
    return 1;
}

int Poly_Lua::SetLineWidth(lua_State* L)
{
    Poly* poly = CHECK_POLY(L, 1);
    float value = CHECK_NUMBER(L, 2);

    poly->SetLineWidth(value);

    return 0;
}


void Poly_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        POLY_LUA_NAME,
        POLY_LUA_FLAG,
        WIDGET_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    lua_pushcfunction(L, AddVertex);
    lua_setfield(L, mtIndex, "AddVertex");

    lua_pushcfunction(L, ClearVertices);
    lua_setfield(L, mtIndex, "ClearVertices");

    lua_pushcfunction(L, GetNumVertices);
    lua_setfield(L, mtIndex, "GetNumVertices");

    lua_pushcfunction(L, GetVertex);
    lua_setfield(L, mtIndex, "GetVertex");

    lua_pushcfunction(L, SetTexture);
    lua_setfield(L, mtIndex, "SetTexture");

    lua_pushcfunction(L, GetTexture);
    lua_setfield(L, mtIndex, "GetTexture");

    lua_pushcfunction(L, GetLineWidth);
    lua_setfield(L, mtIndex, "GetLineWidth");

    lua_pushcfunction(L, SetLineWidth);
    lua_setfield(L, mtIndex, "SetLineWidth");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
