#pragma once

#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"

#include "Nodes/3D/Node3d.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define NODE_3D_LUA_NAME "Node3D"
#define NODE_3D_LUA_FLAG "cfNode3D"
#define CHECK_NODE_3D(L, arg) static_cast<Node3D*>(CheckNodeLuaType(L, arg, NODE_3D_LUA_NAME, NODE_3D_LUA_FLAG));

struct Node3D_Lua
{
    static int AttachToBone(lua_State* L);
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

    static int GetWorldPosition(lua_State* L);
    static int GetWorldRotationEuler(lua_State* L);
    static int GetWorldRotationQuat(lua_State* L);
    static int GetWorldScale(lua_State* L);

    static int SetWorldPosition(lua_State* L);
    static int SetWorldRotationEuler(lua_State* L);
    static int SetWorldRotationQuat(lua_State* L);
    static int SetWorldScale(lua_State* L);

    static int AddRotationEuler(lua_State* L);
    static int AddRotationQuat(lua_State* L);
    static int AddWorldRotationEuler(lua_State* L);
    static int AddWorldRotationQuat(lua_State* L);

    static int LookAt(lua_State* L);
    static int GetForwardVector(lua_State* L);
    static int GetRightVector(lua_State* L);
    static int GetUpVector(lua_State* L);

    static void Bind();
};

#endif
