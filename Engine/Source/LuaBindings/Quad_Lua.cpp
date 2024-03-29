#include "LuaBindings/Quad_Lua.h"
#include "LuaBindings/Widget_Lua.h"

#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Rect_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Texture_Lua.h"

#include "Assets/Texture.h"
#include "AssetManager.h"

#if LUA_ENABLED

int Quad_Lua::SetTexture(lua_State* L)
{
    Quad* quad = CHECK_QUAD(L, 1);
    Texture* texture = nullptr;
    if (!lua_isnil(L, 2)) { texture = CHECK_TEXTURE(L, 2); };

    quad->SetTexture(texture);

    return 0;
}

int Quad_Lua::GetTexture(lua_State* L)
{
    Quad* quad = CHECK_QUAD(L, 1);

    Texture* ret = quad->GetTexture();

    Asset_Lua::Create(L, ret);
    return 1;
}

int Quad_Lua::SetVertexColors(lua_State* L)
{
    Quad* quad = CHECK_QUAD(L, 1);
    glm::vec4 tl = CHECK_VECTOR(L, 2);
    glm::vec4 tr = CHECK_VECTOR(L, 3);
    glm::vec4 bl = CHECK_VECTOR(L, 4);
    glm::vec4 br = CHECK_VECTOR(L, 5);

    quad->SetColor(tl, tr, bl, br);

    return 0;
}

int Quad_Lua::SetUvScale(lua_State* L)
{
    Quad* quad = CHECK_QUAD(L, 1);
    glm::vec2 value = CHECK_VECTOR(L, 2);

    quad->SetUvScale(value);

    return 0;
}

int Quad_Lua::GetUvScale(lua_State* L)
{
    Quad* quad = CHECK_QUAD(L, 1);

    glm::vec2 ret = quad->GetUvScale();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Quad_Lua::SetUvOffset(lua_State* L)
{
    Quad* quad = CHECK_QUAD(L, 1);
    glm::vec2 value = CHECK_VECTOR(L, 2);

    quad->SetUvOffset(value);

    return 0;
}

int Quad_Lua::GetUvOffset(lua_State* L)
{
    Quad* quad = CHECK_QUAD(L, 1);

    glm::vec2 ret = quad->GetUvOffset();

    Vector_Lua::Create(L, ret);
    return 1;
}


void Quad_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        QUAD_LUA_NAME,
        QUAD_LUA_FLAG,
        WIDGET_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, SetTexture);

    REGISTER_TABLE_FUNC(L, mtIndex, GetTexture);

    REGISTER_TABLE_FUNC(L, mtIndex, SetVertexColors);

    REGISTER_TABLE_FUNC(L, mtIndex, SetUvScale);

    REGISTER_TABLE_FUNC(L, mtIndex, GetUvScale);

    REGISTER_TABLE_FUNC(L, mtIndex, SetUvOffset);

    REGISTER_TABLE_FUNC(L, mtIndex, GetUvOffset);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
