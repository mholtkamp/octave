#include "LuaBindings/LightComponent_Lua.h"
#include "LuaBindings/TransformComponent_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

int LightComponent_Lua::SetColor(lua_State* L)
{
    LightComponent* comp = CHECK_LIGHT_COMPONENT(L, 1);
    glm::vec4 value = CHECK_VECTOR(L, 2);

    comp->SetColor(value);

    return 0;
}

int LightComponent_Lua::GetColor(lua_State* L)
{
    LightComponent* comp = CHECK_LIGHT_COMPONENT(L, 1);

    glm::vec4 ret = comp->GetColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int LightComponent_Lua::SetIntensity(lua_State* L)
{
    LightComponent* comp = CHECK_LIGHT_COMPONENT(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetIntensity(value);

    return 0;
}

int LightComponent_Lua::GetIntensity(lua_State* L)
{
    LightComponent* comp = CHECK_LIGHT_COMPONENT(L, 1);

    float ret = comp->GetIntensity();

    lua_pushnumber(L, ret);
    return 1;
}

int LightComponent_Lua::SetCastShadows(lua_State* L)
{
    LightComponent* comp = CHECK_LIGHT_COMPONENT(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    comp->SetCastShadows(value);

    return 0;
}

int LightComponent_Lua::ShouldCastShadows(lua_State* L)
{
    LightComponent* comp = CHECK_LIGHT_COMPONENT(L, 1);

    bool ret = comp->ShouldCastShadows();

    lua_pushboolean(L, ret);
    return 1;
}

void LightComponent_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        LIGHT_COMPONENT_LUA_NAME,
        LIGHT_COMPONENT_LUA_FLAG,
        TRANSFORM_COMPONENT_LUA_NAME);

    Component_Lua::BindCommon(L, mtIndex);

    lua_pushcfunction(L, SetColor);
    lua_setfield(L, mtIndex, "SetColor");

    lua_pushcfunction(L, GetColor);
    lua_setfield(L, mtIndex, "GetColor");

    lua_pushcfunction(L, SetIntensity);
    lua_setfield(L, mtIndex, "SetIntensity");

    lua_pushcfunction(L, GetIntensity);
    lua_setfield(L, mtIndex, "GetIntensity");

    lua_pushcfunction(L, SetCastShadows);
    lua_setfield(L, mtIndex, "SetCastShadows");

    lua_pushcfunction(L, ShouldCastShadows);
    lua_setfield(L, mtIndex, "ShouldCastShadows");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
