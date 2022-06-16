#pragma once

#include "Engine.h"
#include "ObjectRef.h"

#if LUA_ENABLED

#define COMPONENT_LUA_NAME "Component"
#define COMPONENT_LUA_FLAG "cfComponent"
#define CHECK_COMPONENT(L, arg) CheckComponentLuaType(L, arg, COMPONENT_LUA_NAME, COMPONENT_LUA_FLAG);

struct Component_Lua
{
#if LUA_SAFE_COMPONENT
    ComponentRef mComponent;
#else
    Component* mComponent = nullptr;
#endif

    static int Create(lua_State* L, Component* component);

    static int IsValid(lua_State* L);

    static int GetOwner(lua_State* L);
    static int GetName(lua_State* L);
    static int SetName(lua_State* L);
    static int SetActive(lua_State* L);
    static int IsActive(lua_State* L);
    static int SetVisible(lua_State* L);
    static int IsVisible(lua_State* L);
    static int GetWorld(lua_State* L);
    static int Equals(lua_State* L);

    static int CheckType(lua_State* L);

    static void Bind();
};

#endif
