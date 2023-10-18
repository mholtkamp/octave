#pragma once

#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"

#include "Nodes/3D/Node3d.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define TRANSFORM_COMPONENT_LUA_NAME "Node3D"
#define TRANSFORM_COMPONENT_LUA_FLAG "cfNode3D"
#define CHECK_TRANSFORM_COMPONENT(L, arg) static_cast<Node3D*>(CheckComponentLuaType(L, arg, TRANSFORM_COMPONENT_LUA_NAME, TRANSFORM_COMPONENT_LUA_FLAG));

struct Node3D_Lua
{
    static int Attach(lua_State* L);
    static int AttachToBone(lua_State* L);
    static int GetParent(lua_State* L);
    static int GetChild(lua_State* L);
    static int GetNumChildren(lua_State* L);
    static int UpdateTransform(lua_State* L);

    static int GetPosition(lua_State* L);
    static int GetRotationEuler(lua_State* L);
    static int GetRotationQuat(lua_State* L);
    static int GetScale(lua_State* L);

    static int SetPosition(lua_State* L);
    static int SetRotationEuler(lua_State* L);
    static int SetRotationQuat(lua_State* L);
    static int SetScale(lua_State* L);

    static int RotateAround(lua_State* L);

    static int GetAbsolutePosition(lua_State* L);
    static int GetAbsoluteRotationEuler(lua_State* L);
    static int GetAbsoluteRotationQuat(lua_State* L);
    static int GetAbsoluteScale(lua_State* L);

    static int SetAbsolutePosition(lua_State* L);
    static int SetAbsoluteRotationEuler(lua_State* L);
    static int SetAbsoluteRotationQuat(lua_State* L);
    static int SetAbsoluteScale(lua_State* L);

    static int AddRotationEuler(lua_State* L);
    static int AddRotationQuat(lua_State* L);
    static int AddAbsoluteRotationEuler(lua_State* L);
    static int AddAbsoluteRotationQuat(lua_State* L);

    static int LookAt(lua_State* L);
    static int GetForwardVector(lua_State* L);
    static int GetRightVector(lua_State* L);
    static int GetUpVector(lua_State* L);

    static void Bind();
};

#endif
