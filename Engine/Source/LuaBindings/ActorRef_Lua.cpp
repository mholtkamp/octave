#include "LuaBindings/ActorRef_Lua.h"
#include "LuaBindings/Actor_Lua.h"

#include "Log.h"

#if LUA_ENABLED

int ActorRef_Lua::Create(lua_State* L)
{
    Actor* srcActor = nullptr;

    if (lua_isuserdata(L, 1))
    {
        srcActor = CHECK_ACTOR(L, 1);
    }


    ActorRef_Lua* newRef = (ActorRef_Lua*)lua_newuserdata(L, sizeof(ActorRef_Lua));
    new (newRef) ActorRef_Lua();
    luaL_getmetatable(L, ACTOR_REF_LUA_NAME);
    OCT_ASSERT(lua_istable(L, -1));
    lua_setmetatable(L, -2);

    if (srcActor != nullptr)
    {
        newRef->mActorRef = srcActor;
    }

    return 1;
}

int ActorRef_Lua::Destroy(lua_State* L)
{
    CHECK_ACTOR_REF(L, 1);
    ActorRef_Lua* refLua = (ActorRef_Lua*)lua_touserdata(L, 1);
    refLua->~ActorRef_Lua();
    return 0;
}

int ActorRef_Lua::Get(lua_State* L)
{
    ActorRef& ref = CHECK_ACTOR_REF(L, 1);

    Actor_Lua::Create(L, ref.Get());
    return 1;
}

int ActorRef_Lua::Set(lua_State* L)
{
    ActorRef& ref = CHECK_ACTOR_REF(L, 1);

    Actor* actor = nullptr;

    if (!lua_isnil(L, 2))
    {
        actor = CHECK_ACTOR(L, 2);
    }

    ref.Set(actor);

    return 0;
}

void ActorRef_Lua::Bind()
{
    lua_State* L = GetLua();
    OCT_ASSERT(lua_gettop(L) == 0);

    luaL_newmetatable(L, ACTOR_REF_LUA_NAME);
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

    lua_setglobal(L, ACTOR_REF_LUA_NAME);

    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif