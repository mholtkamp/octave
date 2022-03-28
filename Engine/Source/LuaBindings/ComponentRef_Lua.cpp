#include "LuaBindings/ComponentRef_Lua.h"
#include "LuaBindings/Component_Lua.h"

#include "Log.h"

#if LUA_ENABLED

int ComponentRef_Lua::Create(lua_State* L)
{
    Component* srcComponent = nullptr;

    if (lua_isuserdata(L, 1))
    {
        srcComponent = CHECK_COMPONENT(L, 1);
    }


    ComponentRef_Lua* newRef = (ComponentRef_Lua*)lua_newuserdata(L, sizeof(ComponentRef_Lua));
    new (newRef) ComponentRef_Lua();
    luaL_getmetatable(L, COMPONENT_REF_LUA_NAME);
    assert(lua_istable(L, -1));
    lua_setmetatable(L, -2);

    if (srcComponent != nullptr)
    {
        newRef->mComponentRef = srcComponent;
    }

    return 1;
}

int ComponentRef_Lua::Destroy(lua_State* L)
{
    CHECK_COMPONENT_REF(L, 1);
    ComponentRef_Lua* refLua = (ComponentRef_Lua*)lua_touserdata(L, 1);
    refLua->~ComponentRef_Lua();
    return 0;
}

int ComponentRef_Lua::Get(lua_State* L)
{
    ComponentRef& ref = CHECK_COMPONENT_REF(L, 1);

    Component_Lua::Create(L, ref.Get());
    return 1;
}

int ComponentRef_Lua::Set(lua_State* L)
{
    ComponentRef& ref = CHECK_COMPONENT_REF(L, 1);

    Component* actor = nullptr;

    if (!lua_isnil(L, 2))
    {
        actor = CHECK_COMPONENT(L, 2);
    }

    ref.Set(actor);

    return 0;
}

void ComponentRef_Lua::Bind()
{
    lua_State* L = GetLua();
    assert(lua_gettop(L) == 0);

    luaL_newmetatable(L, COMPONENT_REF_LUA_NAME);
    int mtIndex = lua_gettop(L);

    lua_pushcfunction(L, Create);
    lua_setfield(L, mtIndex, "Create");

    lua_pushcfunction(L, Destroy);
    lua_setfield(L, mtIndex, "__gc");

    lua_pushcfunction(L, Get);
    lua_setfield(L, mtIndex, "Get");

    lua_pushcfunction(L, Set);
    lua_setfield(L, mtIndex, "Set");

    // Set the __index metamethod to itself
    lua_pushvalue(L, mtIndex);
    lua_setfield(L, mtIndex, "__index");

    lua_setglobal(L, COMPONENT_REF_LUA_NAME);

    assert(lua_gettop(L) == 0);
}

#endif