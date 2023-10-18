#include "LuaBindings/Sphere3D_Lua.h"
#include "LuaBindings/Primitive3D_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

int Sphere3D_Lua::GetRadius(lua_State* L)
{
    Sphere3D* comp = CHECK_SPHERE_COMPONENT(L, 1);

    float ret = comp->GetRadius();

    lua_pushnumber(L, ret);
    return 1;
}

int Sphere3D_Lua::SetRadius(lua_State* L)
{
    Sphere3D* comp = CHECK_SPHERE_COMPONENT(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetRadius(value);

    return 0;
}

void Sphere3D_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        SPHERE_COMPONENT_LUA_NAME,
        SPHERE_COMPONENT_LUA_FLAG,
        PRIMITIVE_COMPONENT_LUA_NAME);

    Component_Lua::BindCommon(L, mtIndex);

    lua_pushcfunction(L, GetRadius);
    lua_setfield(L, mtIndex, "GetRadius");

    lua_pushcfunction(L, SetRadius);
    lua_setfield(L, mtIndex, "SetRadius");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
