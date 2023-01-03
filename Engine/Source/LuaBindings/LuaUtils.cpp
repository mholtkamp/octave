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

    return lua_gettop(L);
}

#endif
