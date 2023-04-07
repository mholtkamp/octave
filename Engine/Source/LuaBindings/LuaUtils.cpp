#include "LuaBindings/LuaUtils.h"

#include "Log.h"
#include "Constants.h"
#include "AssetManager.h"

#include "Assertion.h"

#include "LuaBindings/Actor_Lua.h"
#include "LuaBindings/Component_Lua.h"
#include "LuaBindings/TransformComponent_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Vector_Lua.h"

#if LUA_ENABLED

int CreateClassMetatable(
    const char* className,
    const char* classFlag,
    const char* parentClassName)
{
    lua_State* L = GetLua();
    int retMtIdx = 0;

    // Check if the metatable already exists. If it does, do nothing.
    luaL_getmetatable(L, className);

    if (lua_isnil(L, -1))
    {
        // Pop nil
        lua_pop(L, 1);

        luaL_newmetatable(L, className);
        int mtIndex = lua_gettop(L);

        // Set the class identifier field for hierarhical type checks.
        lua_pushboolean(L, true);
        lua_setfield(L, mtIndex, classFlag);

        // Set the __index metamethod to itself
        lua_pushvalue(L, mtIndex);
        lua_setfield(L, mtIndex, "__index");

        if (parentClassName != nullptr)
        {
            // Set this metatable's metatable to the parent class.
            luaL_getmetatable(L, parentClassName);
            lua_setmetatable(L, mtIndex);
        }

        lua_pushvalue(L, mtIndex);
        lua_setglobal(L, className);
    }

    // The metatable should be on top of the stack now.
    retMtIdx = lua_gettop(L);

    return retMtIdx;
}

#endif
