#include "LuaBindings/LuaTypeCheck.h"

#include "LuaBindings/Actor_Lua.h"
#include "LuaBindings/Component_Lua.h"
#include "LuaBindings/Node3d_Lua.h"
#include "LuaBindings/Widget_Lua.h"
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
        ret = luaObj->mNode.Get();
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

Widget* CheckWidgetLuaType(lua_State* L, int arg, const char* className, const char* classFlag)
{
    Widget* ret = nullptr;
    Widget_Lua* luaObj = static_cast<Widget_Lua*>(CheckHierarchyLuaType<Widget_Lua>(L, arg, className, classFlag));

    if (luaObj != nullptr)
    {
        ret = luaObj->mWidget;
    }

    return ret;
}

RTTI* CheckRttiLuaType(lua_State* L, int arg)
{
    RTTI* rtti = nullptr;
    luaL_checktype(L, arg, LUA_TUSERDATA);

    // There are only two RTTI types that support script-to-native function calls: Actor and Widget
    // Check actor first
    bool isActor = (lua_getfield(L, arg, "cfActor") != LUA_TNIL);
    lua_pop(L, 1);

    if (isActor)
    {
        rtti = ((Actor_Lua*)lua_touserdata(L, arg))->mActor;
    }
    else
    {
        bool isWidget = (lua_getfield(L, arg, "cfWidget") != LUA_TNIL);
        lua_pop(L, 1);

        if (isWidget)
        {
            rtti = ((Widget_Lua*)lua_touserdata(L, arg))->mWidget;
        }
        else
        {
            luaL_error(L, "Error: Arg #%d: Expected Actor or Widget", arg);
        }
    }

    return rtti;
}

Asset* CheckAssetOrNilLuaType(lua_State* L, int arg, const char* className, const char* classFlag)
{
    return lua_isnil(L, arg) ? nullptr : CheckAssetLuaType<Asset>(L, arg, className, classFlag);
}

Actor* CheckActorOrNilLuaType(lua_State* L, int arg, const char* className, const char* classFlag)
{
    return lua_isnil(L, arg) ? nullptr : CheckActorLuaType(L, arg, className, classFlag);
}

Component* CheckComponentOrNilLuaType(lua_State* L, int arg, const char* className, const char* classFlag)
{
    return lua_isnil(L, arg) ? nullptr : CheckComponentLuaType(L, arg, className, classFlag);
}

Widget* CheckWidgetOrNilLuaType(lua_State* L, int arg, const char* className, const char* classFlag)
{
    return lua_isnil(L, arg) ? nullptr : CheckWidgetLuaType(L, arg, className, classFlag);
}

RTTI* CheckRttiOrNilLuaType(lua_State* L, int arg)
{
    return lua_isnil(L, arg) ? nullptr : CheckRttiLuaType(L, arg);
}

const char* CheckTableName(lua_State* L, int arg)
{
    const char* tableName = "";

    if (lua_istable(L, arg))
    {
        lua_getfield(L, arg, "tableName");

        // Script tables must have tableName assigned to them.
        // This is the name that should be used to access it from global space.
        luaL_checktype(L, -1, LUA_TSTRING);
        tableName = lua_tostring(L, -1);

        lua_pop(L, 1);
    }
    else if (lua_isstring(L, arg))
    {
        tableName = lua_tostring(L, arg);
    }

    return tableName;
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