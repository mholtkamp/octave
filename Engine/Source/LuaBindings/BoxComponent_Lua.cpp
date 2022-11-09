#include "LuaBindings/BoxComponent_Lua.h"
#include "LuaBindings/PrimitiveComponent_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

int BoxComponent_Lua::GetExtents(lua_State* L)
{
    BoxComponent* comp = CHECK_BOX_COMPONENT(L, 1);

    glm::vec3 ret = comp->GetExtents();

    Vector_Lua::Create(L, ret);
    return 1;
}

int BoxComponent_Lua::SetExtents(lua_State* L)
{
    BoxComponent* comp = CHECK_BOX_COMPONENT(L, 1);
    glm::vec3 value = CHECK_VECTOR(L, 2);

    comp->SetExtents(value);

    return 1;
}

void BoxComponent_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        BOX_COMPONENT_LUA_NAME,
        BOX_COMPONENT_LUA_FLAG,
        PRIMITIVE_COMPONENT_LUA_NAME);

    lua_pushcfunction(L, Component_Lua::Destroy);
    lua_setfield(L, mtIndex, "__gc");

    lua_pushcfunction(L, GetExtents);
    lua_setfield(L, mtIndex, "GetExtents");

    lua_pushcfunction(L, SetExtents);
    lua_setfield(L, mtIndex, "SetExtents");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
