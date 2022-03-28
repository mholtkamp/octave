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

    glm::vec2 ret = mat->GetUvOffset();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Material_Lua::SetUvOffset(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);
    glm::vec2 value = CHECK_VECTOR(L, 2);

    mat->SetUvOffset(value);

    return 0;
}

int Material_Lua::GetUvScale(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);

    glm::vec2 ret = mat->GetUvScale();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Material_Lua::SetUvScale(lua_State* L)
{
    Material* mat = CHECK_MATERIAL(L, 1);
    glm::vec2 value = CHECK_VECTOR(L, 2);

    mat->SetUvScale(value);

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


void Material_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        MATERIAL_LUA_NAME,
        MATERIAL_LUA_FLAG,
        ASSET_LUA_NAME);

    lua_pushcfunction(L, IsMaterialInstance);
    lua_setfield(L, mtIndex, "IsMaterialInstance");

    lua_pushcfunction(L, SetTexture);
    lua_setfield(L, mtIndex, "SetTexture");

    lua_pushcfunction(L, GetTexture);
    lua_setfield(L, mtIndex, "GetTexture");

    lua_pushcfunction(L, GetShadingModel);
    lua_setfield(L, mtIndex, "GetShadingModel");

    lua_pushcfunction(L, SetShadingModel);
    lua_setfield(L, mtIndex, "SetShadingModel");

    lua_pushcfunction(L, GetBlendMode);
    lua_setfield(L, mtIndex, "GetBlendMode");

    lua_pushcfunction(L, SetBlendMode);
    lua_setfield(L, mtIndex, "SetBlendMode");

    lua_pushcfunction(L, GetUvOffset);
    lua_setfield(L, mtIndex, "GetUvOffset");

    lua_pushcfunction(L, SetUvOffset);
    lua_setfield(L, mtIndex, "SetUvOffset");

    lua_pushcfunction(L, GetUvScale);
    lua_setfield(L, mtIndex, "GetUvScale");

    lua_pushcfunction(L, SetUvScale);
    lua_setfield(L, mtIndex, "SetUvScale");

    lua_pushcfunction(L, GetColor);
    lua_setfield(L, mtIndex, "GetColor");

    lua_pushcfunction(L, SetColor);
    lua_setfield(L, mtIndex, "SetColor");

    lua_pushcfunction(L, GetFresnelColor);
    lua_setfield(L, mtIndex, "GetFresnelColor");

    lua_pushcfunction(L, SetFresnelColor);
    lua_setfield(L, mtIndex, "SetFresnelColor");

    lua_pushcfunction(L, GetFresnelPower);
    lua_setfield(L, mtIndex, "GetFresnelPower");

    lua_pushcfunction(L, SetFresnelPower);
    lua_setfield(L, mtIndex, "SetFresnelPower");

    lua_pushcfunction(L, GetSpecular);
    lua_setfield(L, mtIndex, "GetSpecular");

    lua_pushcfunction(L, SetSpecular);
    lua_setfield(L, mtIndex, "SetSpecular");

    lua_pushcfunction(L, GetOpacity);
    lua_setfield(L, mtIndex, "GetOpacity");

    lua_pushcfunction(L, SetOpacity);
    lua_setfield(L, mtIndex, "SetOpacity");

    lua_pushcfunction(L, GetMaskCutoff);
    lua_setfield(L, mtIndex, "GetMaskCutoff");

    lua_pushcfunction(L, SetMaskCutoff);
    lua_setfield(L, mtIndex, "SetMaskCutoff");

    lua_pushcfunction(L, GetSortPriority);
    lua_setfield(L, mtIndex, "GetSortPriority");

    lua_pushcfunction(L, SetSortPriority);
    lua_setfield(L, mtIndex, "SetSortPriority");

    lua_pushcfunction(L, IsDepthTestDisabled);
    lua_setfield(L, mtIndex, "IsDepthTestDisabled");

    lua_pushcfunction(L, SetDepthTestDisabled);
    lua_setfield(L, mtIndex, "SetDepthTestDisabled");

    lua_pop(L, 1);
    assert(lua_gettop(L) == 0);
}

#endif
