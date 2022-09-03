#include "LuaBindings/LuaUtils.h"

#include "Log.h"
#include "Constants.h"
#include "AssetManager.h"

#include <assert.h>

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

void PushDatumValue(lua_State* L, DatumType datumType, const void* value)
{
#if LUA_ENABLED
    switch (datumType)
    {
    case DatumType::Integer: lua_pushinteger(L, *reinterpret_cast<const int32_t*>(value)); break;
    case DatumType::Float: lua_pushnumber(L, *reinterpret_cast<const float*>(value)); break;
    case DatumType::Bool: lua_pushboolean(L, *reinterpret_cast<const bool*>(value)); break;
    case DatumType::String: lua_pushstring(L, reinterpret_cast<const std::string*>(value)->c_str()); break;
    case DatumType::Vector2D: Vector_Lua::Create(L, *reinterpret_cast<const glm::vec2*>(value)); break;
    case DatumType::Vector: Vector_Lua::Create(L, *reinterpret_cast<const glm::vec3*>(value)); break;
    case DatumType::Color: Vector_Lua::Create(L, *reinterpret_cast<const glm::vec4*>(value)); break;
    case DatumType::Asset: Asset_Lua::Create(L, *reinterpret_cast<const Asset* const*>(value)); break;

    default:
        LogError("Invalid script datum type");
        break;
    }
#endif
}


#endif
