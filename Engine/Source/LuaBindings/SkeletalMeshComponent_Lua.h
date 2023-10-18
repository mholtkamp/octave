#pragma once

#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"

#include "Nodes/3D/SkeletalMeshComponent.h"

#include "LuaBindings/Component_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define SKELETAL_MESH_COMPONENT_LUA_NAME "SkeletalMesh3D"
#define SKELETAL_MESH_COMPONENT_LUA_FLAG "cfSkeletalMeshComponent"
#define CHECK_SKELETAL_MESH_COMPONENT(L, arg) static_cast<SkeletalMesh3D*>(CheckComponentLuaType(L, arg, SKELETAL_MESH_COMPONENT_LUA_NAME, SKELETAL_MESH_COMPONENT_LUA_FLAG));

struct SkeletalMeshComponent_Lua
{
    static int SetSkeletalMesh(lua_State* L);
    static int GetSkeletalMesh(lua_State* L);
    static int PlayAnimation(lua_State* L);
    static int StopAnimation(lua_State* L);
    static int StopAllAnimations(lua_State* L);
    static int IsAnimationPlaying(lua_State* L);
    static int QueueAnimation(lua_State* L);
    static int CancelQueuedAnimation(lua_State* L);
    static int CancelAllQueuedAnimations(lua_State* L);
    static int SetInheritPose(lua_State* L);
    static int IsInheritPoseEnabled(lua_State* L);
    static int ResetAnimation(lua_State* L);
    static int GetAnimationSpeed(lua_State* L);
    static int SetAnimationSpeed(lua_State* L);
    static int SetAnimationPaused(lua_State* L);
    static int IsAnimationPaused(lua_State* L);
    static int GetBonePosition(lua_State* L);
    static int GetBoneRotation(lua_State* L);
    static int GetBoneScale(lua_State* L);
    static int SetBonePosition(lua_State* L);
    static int SetBoneRotation(lua_State* L);
    static int SetBoneScale(lua_State* L);
    static int GetNumBones(lua_State* L);
    static int SetAnimEventHandler(lua_State* L);
    static int SetBoundsRadiusOverride(lua_State* L);
    static int GetBoundsRadiusOverride(lua_State* L);

    static void Bind();
};

#endif