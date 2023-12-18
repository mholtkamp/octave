#include "LuaBindings/Material_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Texture_Lua.h"

#include "LuaBindings/Vector_Lua.h"

#if LUA_ENABLED

int Material_Lua::IsMaterialInstance(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);

    bool ret = mat->IsMaterialInstance();

    lua_pushboolean(L, ret);
    return 1;
}

int Material_Lua::SetTexture(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);
    int32_t slot = CHECK_INTEGER(L, 2);
    Texture* texture = nullptr;
    if (!lua_isnil(L, 3)) { texture = CHECK_TEXTURE(L, 3); }

    mat->SetTexture((TextureSlot)slot, texture);

    return 0;
}

int Material_Lua::GetTexture(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);
    int32_t slot = CHECK_INTEGER(L, 2);

    Texture* ret = mat->GetTexture((TextureSlot)slot);

    Asset_Lua::Create(L, ret);
    return 1;
}

int Material_Lua::GetShadingModel(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);

    ShadingModel ret = mat->GetShadingModel();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int Material_Lua::SetShadingModel(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);
    ShadingModel value = (ShadingModel)CHECK_INTEGER(L, 2);

    mat->SetShadingModel(value);

    return 0;
}

int Material_Lua::GetBlendMode(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);

    BlendMode ret = mat->GetBlendMode();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int Material_Lua::SetBlendMode(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);
    BlendMode value = (BlendMode)CHECK_INTEGER(L, 2);

    mat->SetBlendMode(value);

    return 0;
}

int Material_Lua::GetUvOffset(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);
    uint32_t uvIndex = 0;
    if (!lua_isnone(L, 2)) { uvIndex = CHECK_INTEGER(L, 2); }

    glm::vec2 ret = mat->GetUvOffset(uvIndex);

    Vector_Lua::Create(L, ret);
    return 1;
}

int Material_Lua::SetUvOffset(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);
    glm::vec2 value = CHECK_VECTOR(L, 2);
    uint32_t uvIndex = 0;
    if (!lua_isnone(L, 3)) { uvIndex = CHECK_INTEGER(L, 3); }

    mat->SetUvOffset(value, uvIndex);

    return 0;
}

int Material_Lua::GetUvScale(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);
    uint32_t uvIndex = 0;
    if (!lua_isnone(L, 2)) { uvIndex = CHECK_INTEGER(L, 2); }

    glm::vec2 ret = mat->GetUvScale(uvIndex);

    Vector_Lua::Create(L, ret);
    return 1;
}

int Material_Lua::SetUvScale(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);
    glm::vec2 value = CHECK_VECTOR(L, 2);
    uint32_t uvIndex = 0;
    if (!lua_isnone(L, 3)) { uvIndex = CHECK_INTEGER(L, 3); }

    mat->SetUvScale(value, uvIndex);

    return 0;
}

int Material_Lua::GetColor(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);

    glm::vec4 ret = mat->GetColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Material_Lua::SetColor(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);
    glm::vec4 value = CHECK_VECTOR(L, 2);

    mat->SetColor(value);

    return 0;
}

int Material_Lua::GetFresnelColor(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);

    glm::vec4 ret = mat->GetFresnelColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Material_Lua::SetFresnelColor(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);
    glm::vec4 value = CHECK_VECTOR(L, 2);

    mat->SetFresnelColor(value);

    return 0;
}

int Material_Lua::GetFresnelPower(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);

    float ret = mat->GetFresnelPower();

    lua_pushnumber(L, ret);
    return 1;
}

int Material_Lua::SetFresnelPower(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);
    float value = CHECK_NUMBER(L, 2);

    mat->SetFresnelPower(value);

    return 0;
}

int Material_Lua::GetEmission(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);

    float ret = mat->GetEmission();

    lua_pushnumber(L, ret);
    return 1;
}

int Material_Lua::SetEmission(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);
    float value = CHECK_NUMBER(L, 2);

    mat->SetEmission(value);

    return 0;
}

int Material_Lua::GetWrapLighting(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);

    float ret = mat->GetWrapLighting();

    lua_pushnumber(L, ret);
    return 1;
}

int Material_Lua::SetWrapLighting(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);
    float value = CHECK_NUMBER(L, 2);

    mat->SetWrapLighting(value);

    return 0;
}

int Material_Lua::GetSpecular(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);

    float ret = mat->GetSpecular();

    lua_pushnumber(L, ret);
    return 1;
}

int Material_Lua::SetSpecular(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);
    float value = CHECK_NUMBER(L, 2);

    mat->SetSpecular(value);

    return 0;
}

int Material_Lua::GetOpacity(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);

    float ret = mat->GetOpacity();

    lua_pushnumber(L, ret);
    return 1;
}

int Material_Lua::SetOpacity(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);
    float value = CHECK_NUMBER(L, 2);

    mat->SetOpacity(value);

    return 0;
}

int Material_Lua::GetMaskCutoff(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);

    float ret = mat->GetMaskCutoff();

    lua_pushnumber(L, ret);
    return 1;
}

int Material_Lua::SetMaskCutoff(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);
    float value = CHECK_NUMBER(L, 2);

    mat->SetMaskCutoff(value);

    return 0;
}

int Material_Lua::GetSortPriority(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);

    int32_t ret = mat->GetSortPriority();

    lua_pushinteger(L, ret);
    return 1;
}

int Material_Lua::SetSortPriority(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);
    int32_t value = CHECK_INTEGER(L, 2);

    mat->SetSortPriority(value);

    return 0;
}

int Material_Lua::IsDepthTestDisabled(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);

    bool ret = mat->IsDepthTestDisabled();

    lua_pushboolean(L, ret);
    return 1;
}

int Material_Lua::SetDepthTestDisabled(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    mat->SetDepthTestDisabled(value);

    return 0;
}

int Material_Lua::GetUvMap(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);
    int32_t slot = CHECK_INTEGER(L, 2);

    int32_t ret = mat->GetUvMap((TextureSlot)slot);

    lua_pushinteger(L, ret);
    return 1;
}

int Material_Lua::SetUvMap(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);
    int32_t slot = CHECK_INTEGER(L, 2);
    int32_t uvMapIdx = CHECK_INTEGER(L, 3);

    mat->SetUvMap((TextureSlot)slot, uvMapIdx);

    return 0;
}

int Material_Lua::GetTevMode(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);
    int32_t slot = CHECK_INTEGER(L, 2);

    TevMode ret = mat->GetTevMode((TextureSlot)slot);

    lua_pushinteger(L, (int32_t)ret);
    return 1;
}

int Material_Lua::SetTevMode(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);
    int32_t slot = CHECK_INTEGER(L, 2);
    int32_t tevMode = CHECK_INTEGER(L, 3);

    mat->SetTevMode((TextureSlot)slot, (TevMode)tevMode);

    return 0;
}

void Material_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        MATERIAL_LUA_NAME,
        MATERIAL_LUA_FLAG,
        ASSET_LUA_NAME);

    Asset_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, IsMaterialInstance);

    REGISTER_TABLE_FUNC(L, mtIndex, SetTexture);

    REGISTER_TABLE_FUNC(L, mtIndex, GetTexture);

    REGISTER_TABLE_FUNC(L, mtIndex, GetShadingModel);

    REGISTER_TABLE_FUNC(L, mtIndex, SetShadingModel);

    REGISTER_TABLE_FUNC(L, mtIndex, GetBlendMode);

    REGISTER_TABLE_FUNC(L, mtIndex, SetBlendMode);

    REGISTER_TABLE_FUNC(L, mtIndex, GetUvOffset);

    REGISTER_TABLE_FUNC(L, mtIndex, SetUvOffset);

    REGISTER_TABLE_FUNC(L, mtIndex, GetUvScale);

    REGISTER_TABLE_FUNC(L, mtIndex, SetUvScale);

    REGISTER_TABLE_FUNC(L, mtIndex, GetColor);

    REGISTER_TABLE_FUNC(L, mtIndex, SetColor);

    REGISTER_TABLE_FUNC(L, mtIndex, GetFresnelColor);

    REGISTER_TABLE_FUNC(L, mtIndex, SetFresnelColor);

    REGISTER_TABLE_FUNC(L, mtIndex, GetFresnelPower);

    REGISTER_TABLE_FUNC(L, mtIndex, SetFresnelPower);

    REGISTER_TABLE_FUNC(L, mtIndex, GetEmission);

    REGISTER_TABLE_FUNC(L, mtIndex, SetEmission);

    REGISTER_TABLE_FUNC(L, mtIndex, GetWrapLighting);

    REGISTER_TABLE_FUNC(L, mtIndex, SetWrapLighting);

    REGISTER_TABLE_FUNC(L, mtIndex, GetSpecular);

    REGISTER_TABLE_FUNC(L, mtIndex, SetSpecular);

    REGISTER_TABLE_FUNC(L, mtIndex, GetOpacity);

    REGISTER_TABLE_FUNC(L, mtIndex, SetOpacity);

    REGISTER_TABLE_FUNC(L, mtIndex, GetMaskCutoff);

    REGISTER_TABLE_FUNC(L, mtIndex, SetMaskCutoff);

    REGISTER_TABLE_FUNC(L, mtIndex, GetSortPriority);

    REGISTER_TABLE_FUNC(L, mtIndex, SetSortPriority);

    REGISTER_TABLE_FUNC(L, mtIndex, IsDepthTestDisabled);

    REGISTER_TABLE_FUNC(L, mtIndex, SetDepthTestDisabled);

    REGISTER_TABLE_FUNC(L, mtIndex, GetUvMap);

    REGISTER_TABLE_FUNC(L, mtIndex, SetUvMap);

    REGISTER_TABLE_FUNC(L, mtIndex, GetTevMode);

    REGISTER_TABLE_FUNC(L, mtIndex, SetTevMode);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
