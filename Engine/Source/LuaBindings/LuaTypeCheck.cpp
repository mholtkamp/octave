#include "LuaBindings/LuaTypeCheck.h"

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
        lua_pop(L, 1);
    }

    return isClass;
}

#endif