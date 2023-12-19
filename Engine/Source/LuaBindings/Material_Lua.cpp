#include "LuaBindings/Material_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Texture_Lua.h"

#include "LuaBindings/Vector_Lua.h"

#if LUA_ENABLED

int Material_Lua::IsBase(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);

    bool ret = mat->IsBase();

    lua_pushboolean(L, ret);
    return 1;
}

int Material_Lua::IsInstance(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);

    bool ret = mat->IsInstance();

    lua_pushboolean(L, ret);
    return 1;
}

int Material_Lua::IsLite(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);

    bool ret = mat->IsLite();

    lua_pushboolean(L, ret);
    return 1;
}

int Material_Lua::SetScalarParameter(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);
    const char* name = CHECK_STRING(L, 2);
    float value = CHECK_NUMBER(L, 3);

    mat->SetScalarParameter(name, value);

    return 0;
}

int Material_Lua::SetVectorParameter(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);
    const char* name = CHECK_STRING(L, 2);
    glm::vec4 value = CHECK_VECTOR(L, 3);

    mat->SetVectorParameter(name, value);

    return 0;
}

int Material_Lua::SetTextureParameter(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);
    const char* name = CHECK_STRING(L, 2);
    Texture* value = CHECK_TEXTURE(L, 3);

    mat->SetTextureParameter(name, value);

    return 0;
}

int Material_Lua::GetScalarParameter(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);
    const char* name = CHECK_STRING(L, 2);

    float ret = mat->GetScalarParameter(name);

    lua_pushnumber(L, ret);
    return 1;
}

int Material_Lua::GetVectorParameter(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);
    const char* name = CHECK_STRING(L, 2);

    glm::vec4 ret = mat->GetVectorParameter(name);

    Vector_Lua::Create(L, ret);
    return 1;
}

int Material_Lua::GetTextureParameter(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);
    const char* name = CHECK_STRING(L, 2);

    Texture* ret = mat->GetTextureParameter(name);

    Asset_Lua::Create(L, ret);
    return 1;
}

int Material_Lua::GetBlendMode(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);

    BlendMode ret = mat->GetBlendMode();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int Material_Lua::GetMaskCutoff(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);

    float ret = mat->GetMaskCutoff();

    lua_pushnumber(L, ret);
    return 1;
}

int Material_Lua::GetSortPriority(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);

    int32_t ret = mat->GetSortPriority();

    lua_pushinteger(L, ret);
    return 1;
}

int Material_Lua::IsDepthTestDisabled(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);

    bool ret = mat->IsDepthTestDisabled();

    lua_pushboolean(L, ret);
    return 1;
}

int Material_Lua::ShouldApplyFog(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);

    bool ret = mat->ShouldApplyFog();

    lua_pushboolean(L, ret);
    return 1;
}

int Material_Lua::GetCullMode(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);

    CullMode ret = mat->GetCullMode();

    lua_pushinteger(L, (int)ret);
    return 1;
}

void Material_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        MATERIAL_LUA_NAME,
        MATERIAL_LUA_FLAG,
        ASSET_LUA_NAME);

    Asset_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, IsBase);
    REGISTER_TABLE_FUNC(L, mtIndex, IsInstance);
    REGISTER_TABLE_FUNC(L, mtIndex, IsLite);
    REGISTER_TABLE_FUNC(L, mtIndex, SetScalarParameter);
    REGISTER_TABLE_FUNC(L, mtIndex, SetVectorParameter);
    REGISTER_TABLE_FUNC(L, mtIndex, SetTextureParameter);
    REGISTER_TABLE_FUNC(L, mtIndex, GetScalarParameter);
    REGISTER_TABLE_FUNC(L, mtIndex, GetVectorParameter);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTextureParameter);
    REGISTER_TABLE_FUNC(L, mtIndex, GetBlendMode);
    REGISTER_TABLE_FUNC(L, mtIndex, GetMaskCutoff);
    REGISTER_TABLE_FUNC(L, mtIndex, GetSortPriority);
    REGISTER_TABLE_FUNC(L, mtIndex, IsDepthTestDisabled);
    REGISTER_TABLE_FUNC(L, mtIndex, ShouldApplyFog);
    REGISTER_TABLE_FUNC(L, mtIndex, GetCullMode);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
