#include "LuaBindings/CapsuleComponent_Lua.h"
#include "LuaBindings/PrimitiveComponent_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

int CapsuleComponent_Lua::GetHeight(lua_State* L)
{
    CapsuleComponent* comp = CHECK_CAPSULE_COMPONENT(L, 1);

    float ret = comp->GetHeight();

    lua_pushnumber(L, ret);
    return 1;
}

int CapsuleComponent_Lua::SetHeight(lua_State* L)
{
    CapsuleComponent* comp = CHECK_CAPSULE_COMPONENT(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetHeight(value);

    return 0;
}

int CapsuleComponent_Lua::GetRadius(lua_State* L)
{
    CapsuleComponent* comp = CHECK_CAPSULE_COMPONENT(L, 1);

    float ret = comp->GetRadius();

    lua_pushnumber(L, ret);
    return 1;
}

int CapsuleComponent_Lua::SetRadius(lua_State* L)
{
    CapsuleComponent* comp = CHECK_CAPSULE_COMPONENT(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetRadius(value);

    return 0;
}

void CapsuleComponent_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        CAPSULE_COMPONENT_LUA_NAME,
        CAPSULE_COMPONENT_LUA_FLAG,
        PRIMITIVE_COMPONENT_LUA_NAME);

    lua_pushcfunction(L, GetHeight);
    lua_setfield(L, mtIndex, "GetHeight");

    lua_pushcfunction(L, SetHeight);
    lua_setfield(L, mtIndex, "SetHeight");

    lua_pushcfunction(L, GetRadius);
    lua_setfield(L, mtIndex, "GetRadius");

    lua_pushcfunction(L, SetRadius);
    lua_setfield(L, mtIndex, "SetRadius");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
