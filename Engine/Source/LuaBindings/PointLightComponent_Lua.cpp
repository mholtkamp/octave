#include "LuaBindings/PointLightComponent_Lua.h"
#include "LuaBindings/LightComponent_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

int PointLightComponent_Lua::SetRadius(lua_State* L)
{
    PointLightComponent* comp = CHECK_POINT_LIGHT_COMPONENT(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetRadius(value);

    return 0;
}

int PointLightComponent_Lua::GetRadius(lua_State* L)
{
    PointLightComponent* comp = CHECK_POINT_LIGHT_COMPONENT(L, 1);

    float ret = comp->GetRadius();

    lua_pushnumber(L, ret);
    return 1;
}

void PointLightComponent_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        POINT_LIGHT_COMPONENT_LUA_NAME,
        POINT_LIGHT_COMPONENT_LUA_FLAG,
        LIGHT_COMPONENT_LUA_NAME);

    lua_pushcfunction(L, SetRadius);
    lua_setfield(L, mtIndex, "SetRadius");

    lua_pushcfunction(L, GetRadius);
    lua_setfield(L, mtIndex, "GetRadius");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
