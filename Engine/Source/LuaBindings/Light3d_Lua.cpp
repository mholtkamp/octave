#include "LuaBindings/Light3d_Lua.h"
#include "LuaBindings/Node3d_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

int Light3D_Lua::SetColor(lua_State* L)
{
    Light3D* comp = CHECK_LIGHT_3D(L, 1);
    glm::vec4 value = CHECK_VECTOR(L, 2);

    comp->SetColor(value);

    return 0;
}

int Light3D_Lua::GetColor(lua_State* L)
{
    Light3D* comp = CHECK_LIGHT_3D(L, 1);

    glm::vec4 ret = comp->GetColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Light3D_Lua::SetIntensity(lua_State* L)
{
    Light3D* comp = CHECK_LIGHT_3D(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetIntensity(value);

    return 0;
}

int Light3D_Lua::GetIntensity(lua_State* L)
{
    Light3D* comp = CHECK_LIGHT_3D(L, 1);

    float ret = comp->GetIntensity();

    lua_pushnumber(L, ret);
    return 1;
}

int Light3D_Lua::SetCastShadows(lua_State* L)
{
    Light3D* comp = CHECK_LIGHT_3D(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    comp->SetCastShadows(value);

    return 0;
}

int Light3D_Lua::ShouldCastShadows(lua_State* L)
{
    Light3D* comp = CHECK_LIGHT_3D(L, 1);

    bool ret = comp->ShouldCastShadows();

    lua_pushboolean(L, ret);
    return 1;
}

int Light3D_Lua::GetDomain(lua_State* L)
{
    Light3D* comp = CHECK_LIGHT_3D(L, 1);

    int32_t ret = (int32_t)comp->GetLightingDomain();

    lua_pushinteger(L, ret);
    return 1;
}

int Light3D_Lua::GetLightingChannels(lua_State* L)
{
    Light3D* comp = CHECK_LIGHT_3D(L, 1);

    int32_t ret = (int32_t)comp->GetLightingChannels();

    lua_pushinteger(L, ret);
    return 1;
}

int Light3D_Lua::SetLightingChannels(lua_State* L)
{
    Light3D* comp = CHECK_LIGHT_3D(L, 1);
    int32_t value = CHECK_INTEGER(L, 2);

    comp->SetLightingChannels((uint8_t)value);

    return 0;
}

void Light3D_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        LIGHT_3D_LUA_NAME,
        LIGHT_3D_LUA_FLAG,
        NODE_3D_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, SetColor);

    REGISTER_TABLE_FUNC(L, mtIndex, GetColor);

    REGISTER_TABLE_FUNC(L, mtIndex, SetIntensity);

    REGISTER_TABLE_FUNC(L, mtIndex, GetIntensity);

    REGISTER_TABLE_FUNC(L, mtIndex, SetCastShadows);

    REGISTER_TABLE_FUNC(L, mtIndex, ShouldCastShadows);

    REGISTER_TABLE_FUNC(L, mtIndex, GetDomain);

    REGISTER_TABLE_FUNC(L, mtIndex, GetLightingChannels);

    REGISTER_TABLE_FUNC(L, mtIndex, SetLightingChannels);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
