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

Actor* CheckActorLuaType(lua_State* L, int arg, const char* className, const char* classFlag)
{
#if LUA_SAFE_ACTOR
    Actor* ret = nullptr;
    Actor_Lua* luaObj = static_cast<Actor_Lua*>(CheckHierarchyLuaType<Actor_Lua>(L, arg, className, classFlag));

    if (luaObj != nullptr)
    {
        ret = luaObj->mActor.Get();
        if (ret == nullptr)
        {
            luaL_error(L, "Attempting to use destroyed actor at arg %d", arg);
        }
    }

    return ret;
#else
    return CheckHierarchyLuaType<Actor_Lua>(L, arg, className, classFlag)->mActor;
#endif
}

Component* CheckComponentLuaType(lua_State* L, int arg, const char* className, const char* classFlag)
{
#if LUA_SAFE_COMPONENT
    Component* ret = nullptr;
    Component_Lua* luaObj = static_cast<Component_Lua*>(CheckHierarchyLuaType<Component_Lua>(L, arg, className, classFlag));

    if (luaObj != nullptr)
    {
        ret = luaObj->mComponent.Get();
        if (ret == nullptr)
        {
            luaL_error(L, "Attempting to use destroyed component at arg %d", arg);
        }
    }

    return ret;
#else
    return CheckHierarchyLuaType<Component_Lua>(L, arg, className, classFlag)->mComponent;
#endif
}

bool CheckClassFlag(lua_State* L, int arg, const char* flag)
{
    bool isClass = false;

    if (lua_type(L, arg) == LUA_TUSERDATA)
    {
        isClass = (lua_getfield(L, arg, flag) != LUA_TNIL);
    }

    return isClass;
}

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
