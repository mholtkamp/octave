#include "LuaBindings/DirectionalLightComponent_Lua.h"
#include "LuaBindings/LightComponent_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

int DirectionalLightComponent_Lua::GetDirection(lua_State* L)
{
    DirectionalLightComponent* comp = CHECK_DIRECTIONAL_LIGHT_COMPONENT(L, 1);

    glm::vec3 ret = comp->GetDirection();

    Vector_Lua::Create(L, ret);
    return 1;
}

int DirectionalLightComponent_Lua::SetDirection(lua_State* L)
{
    DirectionalLightComponent* comp = CHECK_DIRECTIONAL_LIGHT_COMPONENT(L, 1);
    glm::vec3 value = CHECK_VECTOR(L, 2);

    comp->SetDirection(value);

    return 0;
}

void DirectionalLightComponent_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        DIRECTIONAL_LIGHT_COMPONENT_LUA_NAME,
        DIRECTIONAL_LIGHT_COMPONENT_LUA_FLAG,
        LIGHT_COMPONENT_LUA_NAME);

    lua_pushcfunction(L, GetDirection);
    lua_setfield(L, mtIndex, "GetDirection");

    lua_pushcfunction(L, SetDirection);
    lua_setfield(L, mtIndex, "SetDirection");

    lua_pop(L, 1);
    assert(lua_gettop(L) == 0);
}

#endif
