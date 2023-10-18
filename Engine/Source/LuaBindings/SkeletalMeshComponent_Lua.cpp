#include "LuaBindings/SkeletalMeshComponent_Lua.h"
#include "LuaBindings/MeshComponent_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/SkeletalMesh_Lua.h"
#include "LuaBindings/Vector_Lua.h"

#include "Asset.h"
#include "AssetManager.h"
#include "Assets/SkeletalMesh.h"

int SkeletalMeshComponent_Lua::SetSkeletalMesh(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_COMPONENT(L, 1);
    SkeletalMesh* skMesh = nullptr;
    if (!lua_isnil(L, 2))
    {
        skMesh = CHECK_SKELETAL_MESH(L, 2);
    }

    comp->SetSkeletalMesh(skMesh);

    return 0;
}

int SkeletalMeshComponent_Lua::GetSkeletalMesh(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_COMPONENT(L, 1);

    SkeletalMesh* skMesh = comp->GetSkeletalMesh();

    Asset_Lua::Create(L, skMesh);
    return 1;
}

int SkeletalMeshComponent_Lua::PlayAnimation(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_COMPONENT(L, 1);
    const char* animName = CHECK_STRING(L, 2);
    bool loop = false;
    float speed = 1.0f;
    float weight = 1.0f;
    uint8_t priority = 255;

    if (!lua_isnone(L, 3)) { loop = CHECK_BOOLEAN(L, 3); }
    if (!lua_isnone(L, 4)) { speed = CHECK_NUMBER(L, 4); }
    if (!lua_isnone(L, 5)) { weight = CHECK_NUMBER(L, 5); }
    if (!lua_isnone(L, 6)) { priority = (uint8_t) CHECK_INTEGER(L, 6); }

    comp->PlayAnimation(animName, loop, speed, weight, priority);

    return 0;
}

int SkeletalMeshComponent_Lua::StopAnimation(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_COMPONENT(L, 1);
    const char* animName = CHECK_STRING(L, 2);
    bool cancelQueued = false;

    if (!lua_isnone(L, 3)) { cancelQueued = CHECK_BOOLEAN(L, 3); }

    comp->StopAnimation(animName, cancelQueued);

    return 0;
}

int SkeletalMeshComponent_Lua::StopAllAnimations(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_COMPONENT(L, 1);
    bool cancelQueued = false;

    if (!lua_isnone(L, 2)) { cancelQueued = CHECK_BOOLEAN(L, 2); }

    comp->StopAllAnimations(cancelQueued);

    return 0;
}

int SkeletalMeshComponent_Lua::IsAnimationPlaying(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_COMPONENT(L, 1);
    const char* animName = CHECK_STRING(L, 2);

    bool ret = comp->IsAnimationPlaying(animName);

    lua_pushboolean(L, ret);
    return 1;
}

int SkeletalMeshComponent_Lua::QueueAnimation(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_COMPONENT(L, 1);
    const char* animName = CHECK_STRING(L, 2);
    bool loop = CHECK_BOOLEAN(L, 3);
    const char* dependentAnimName = nullptr;

    float speed = 1.0f;
    float weight = 1.0f;
    uint8_t priority = 255;

    if (!lua_isnone(L, 4)) { dependentAnimName = CHECK_STRING(L, 4); }
    if (!lua_isnone(L, 5)) { speed = CHECK_NUMBER(L, 5); }
    if (!lua_isnone(L, 6)) { weight = CHECK_NUMBER(L, 6); }
    if (!lua_isnone(L, 7)) { priority = (uint8_t)CHECK_INTEGER(L, 7); }

    comp->QueueAnimation(animName, loop, dependentAnimName, speed, weight, priority);

    return 0;
}

int SkeletalMeshComponent_Lua::CancelQueuedAnimation(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_COMPONENT(L, 1);
    const char* animName = CHECK_STRING(L, 2);

    comp->CancelQueuedAnimation(animName);

    return 0;
}

int SkeletalMeshComponent_Lua::CancelAllQueuedAnimations(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_COMPONENT(L, 1);

    comp->CancelAllQueuedAnimations();

    return 0;
}

int SkeletalMeshComponent_Lua::SetInheritPose(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_COMPONENT(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    comp->SetInheritPose(value);

    return 0;
}

int SkeletalMeshComponent_Lua::IsInheritPoseEnabled(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_COMPONENT(L, 1);

    bool ret = comp->IsInheritPoseEnabled();

    lua_pushboolean(L, ret);
    return 1;
}

int SkeletalMeshComponent_Lua::ResetAnimation(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_COMPONENT(L, 1);

    comp->ResetAnimation();

    return 0;
}

int SkeletalMeshComponent_Lua::GetAnimationSpeed(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_COMPONENT(L, 1);

    float ret = comp->GetAnimationSpeed();

    lua_pushnumber(L, ret);
    return 1;
}

int SkeletalMeshComponent_Lua::SetAnimationSpeed(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_COMPONENT(L, 1);
    float speed = CHECK_NUMBER(L, 2);

    comp->SetAnimationSpeed(speed);

    return 0;
}

int SkeletalMeshComponent_Lua::SetAnimationPaused(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_COMPONENT(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    comp->SetAnimationPaused(value);

    return 0;
}

int SkeletalMeshComponent_Lua::IsAnimationPaused(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_COMPONENT(L, 1);

    bool ret = comp->IsAnimationPaused();

    lua_pushboolean(L, ret);
    return 1;
}

int SkeletalMeshComponent_Lua::GetBonePosition(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_COMPONENT(L, 1);
    const char* boneName = CHECK_STRING(L, 2);

    glm::vec3 ret = comp->GetBonePosition(boneName);

    Vector_Lua::Create(L, ret);
    return 1;
}

int SkeletalMeshComponent_Lua::GetBoneRotation(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_COMPONENT(L, 1);
    const char* boneName = CHECK_STRING(L, 2);

    glm::vec3 ret = comp->GetBoneRotationEuler(boneName);

    Vector_Lua::Create(L, ret);
    return 1;
}

int SkeletalMeshComponent_Lua::GetBoneScale(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_COMPONENT(L, 1);
    const char* boneName = CHECK_STRING(L, 2);

    glm::vec3 ret = comp->GetBoneScale(boneName);

    Vector_Lua::Create(L, ret);
    return 1;
}

int SkeletalMeshComponent_Lua::SetBonePosition(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_COMPONENT(L, 1);
    const char* boneName = CHECK_STRING(L, 2);
    glm::vec3 value = CHECK_VECTOR(L, 3);

    int32_t boneIndex = comp->FindBoneIndex(boneName);
    comp->SetBonePosition(boneIndex, value);

    return 0;
}

int SkeletalMeshComponent_Lua::SetBoneRotation(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_COMPONENT(L, 1);
    const char* boneName = CHECK_STRING(L, 2);
    glm::vec3 value = CHECK_VECTOR(L, 3);

    int32_t boneIndex = comp->FindBoneIndex(boneName);
    comp->SetBoneRotation(boneIndex, value);

    return 0;
}

int SkeletalMeshComponent_Lua::SetBoneScale(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_COMPONENT(L, 1);
    const char* boneName = CHECK_STRING(L, 2);
    glm::vec3 value = CHECK_VECTOR(L, 3);

    int32_t boneIndex = comp->FindBoneIndex(boneName);
    comp->SetBoneScale(boneIndex, value);

    return 0;
}

int SkeletalMeshComponent_Lua::GetNumBones(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_COMPONENT(L, 1);

    uint32_t numBones = comp->GetNumBones();

    lua_pushinteger(L, (int)numBones);
    return 1;
}

int SkeletalMeshComponent_Lua::SetAnimEventHandler(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_COMPONENT(L, 1);
    const char* tableName = CheckTableName(L, 2);
    const char* funcName = CHECK_STRING(L, 3);

    comp->SetScriptAnimEventHandler(
        tableName,
        funcName);

    return 0;
}

int SkeletalMeshComponent_Lua::SetBoundsRadiusOverride(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_COMPONENT(L, 1);
    float radius = CHECK_NUMBER(L, 2);

    comp->SetBoundsRadiusOverride(radius);

    return 0;
}

int SkeletalMeshComponent_Lua::GetBoundsRadiusOverride(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_COMPONENT(L, 1);

    float radius = comp->GetBoundsRadiusOverride();

    lua_pushnumber(L, radius);
    return 1;
}

void SkeletalMeshComponent_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        SKELETAL_MESH_COMPONENT_LUA_NAME,
        SKELETAL_MESH_COMPONENT_LUA_FLAG,
        MESH_COMPONENT_LUA_NAME);

    Component_Lua::BindCommon(L, mtIndex);

    lua_pushcfunction(L, SetSkeletalMesh);
    lua_setfield(L, mtIndex, "SetSkeletalMesh");

    lua_pushcfunction(L, GetSkeletalMesh);
    lua_setfield(L, mtIndex, "GetSkeletalMesh");

    lua_pushcfunction(L, PlayAnimation);
    lua_setfield(L, mtIndex, "PlayAnimation");

    lua_pushcfunction(L, StopAnimation);
    lua_setfield(L, mtIndex, "StopAnimation");

    lua_pushcfunction(L, StopAllAnimations);
    lua_setfield(L, mtIndex, "StopAllAnimations");

    lua_pushcfunction(L, IsAnimationPlaying);
    lua_setfield(L, mtIndex, "IsAnimationPlaying");

    lua_pushcfunction(L, QueueAnimation);
    lua_setfield(L, mtIndex, "QueueAnimation");

    lua_pushcfunction(L, CancelQueuedAnimation);
    lua_setfield(L, mtIndex, "CancelQueuedAnimation");

    lua_pushcfunction(L, CancelAllQueuedAnimations);
    lua_setfield(L, mtIndex, "CancelAllQueuedAnimations");

    lua_pushcfunction(L, SetInheritPose);
    lua_setfield(L, mtIndex, "SetInheritPose");

    lua_pushcfunction(L, IsInheritPoseEnabled);
    lua_setfield(L, mtIndex, "IsInheritPoseEnabled");

    lua_pushcfunction(L, ResetAnimation);
    lua_setfield(L, mtIndex, "ResetAnimation");

    lua_pushcfunction(L, GetAnimationSpeed);
    lua_setfield(L, mtIndex, "GetAnimationSpeed");

    lua_pushcfunction(L, SetAnimationSpeed);
    lua_setfield(L, mtIndex, "SetAnimationSpeed");

    lua_pushcfunction(L, SetAnimationPaused);
    lua_setfield(L, mtIndex, "SetAnimationPaused");

    lua_pushcfunction(L, IsAnimationPaused);
    lua_setfield(L, mtIndex, "IsAnimationPaused");

    lua_pushcfunction(L, GetBonePosition);
    lua_setfield(L, mtIndex, "GetBonePosition");

    lua_pushcfunction(L, GetBoneRotation);
    lua_setfield(L, mtIndex, "GetBoneRotation");

    lua_pushcfunction(L, GetBoneScale);
    lua_setfield(L, mtIndex, "GetBoneScale");

    lua_pushcfunction(L, SetBonePosition);
    lua_setfield(L, mtIndex, "SetBonePosition");

    lua_pushcfunction(L, SetBoneRotation);
    lua_setfield(L, mtIndex, "SetBoneRotation");

    lua_pushcfunction(L, SetBoneScale);
    lua_setfield(L, mtIndex, "SetBoneScale");

    lua_pushcfunction(L, GetNumBones);
    lua_setfield(L, mtIndex, "GetNumBones");

    lua_pushcfunction(L, SetAnimEventHandler);
    lua_setfield(L, mtIndex, "SetAnimEventHandler");

    lua_pushcfunction(L, SetBoundsRadiusOverride);
    lua_setfield(L, mtIndex, "SetBoundsRadiusOverride");

    lua_pushcfunction(L, GetBoundsRadiusOverride);
    lua_setfield(L, mtIndex, "GetBoundsRadiusOverride");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}
