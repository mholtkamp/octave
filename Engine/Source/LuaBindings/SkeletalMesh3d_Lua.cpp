#include "LuaBindings/SkeletalMesh3d_Lua.h"
#include "LuaBindings/Mesh3d_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/SkeletalMesh_Lua.h"
#include "LuaBindings/Vector_Lua.h"

#include "Asset.h"
#include "AssetManager.h"
#include "Assets/SkeletalMesh.h"

int SkeletalMesh3D_Lua::SetSkeletalMesh(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_3D(L, 1);
    SkeletalMesh* skMesh = nullptr;
    if (!lua_isnil(L, 2))
    {
        skMesh = CHECK_SKELETAL_MESH(L, 2);
    }

    comp->SetSkeletalMesh(skMesh);

    return 0;
}

int SkeletalMesh3D_Lua::GetSkeletalMesh(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_3D(L, 1);

    SkeletalMesh* skMesh = comp->GetSkeletalMesh();

    Asset_Lua::Create(L, skMesh);
    return 1;
}

int SkeletalMesh3D_Lua::PlayAnimation(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_3D(L, 1);
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

int SkeletalMesh3D_Lua::StopAnimation(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_3D(L, 1);
    const char* animName = CHECK_STRING(L, 2);
    bool cancelQueued = false;

    if (!lua_isnone(L, 3)) { cancelQueued = CHECK_BOOLEAN(L, 3); }

    comp->StopAnimation(animName, cancelQueued);

    return 0;
}

int SkeletalMesh3D_Lua::StopAllAnimations(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_3D(L, 1);
    bool cancelQueued = false;

    if (!lua_isnone(L, 2)) { cancelQueued = CHECK_BOOLEAN(L, 2); }

    comp->StopAllAnimations(cancelQueued);

    return 0;
}

int SkeletalMesh3D_Lua::IsAnimationPlaying(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_3D(L, 1);
    const char* animName = CHECK_STRING(L, 2);

    bool ret = comp->IsAnimationPlaying(animName);

    lua_pushboolean(L, ret);
    return 1;
}

int SkeletalMesh3D_Lua::QueueAnimation(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_3D(L, 1);
    const char* animName = CHECK_STRING(L, 2);
    bool loop = CHECK_BOOLEAN(L, 3);
    const char* dependentAnimName = nullptr;

    float speed = 1.0f;
    float weight = 1.0f;
    uint8_t priority = 255;

    if (!lua_isnone(L, 4) && !lua_isnil(L, 4)) { dependentAnimName = CHECK_STRING(L, 4); }
    if (!lua_isnone(L, 5)) { speed = CHECK_NUMBER(L, 5); }
    if (!lua_isnone(L, 6)) { weight = CHECK_NUMBER(L, 6); }
    if (!lua_isnone(L, 7)) { priority = (uint8_t)CHECK_INTEGER(L, 7); }

    comp->QueueAnimation(animName, loop, dependentAnimName, speed, weight, priority);

    return 0;
}

int SkeletalMesh3D_Lua::CancelQueuedAnimation(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_3D(L, 1);
    const char* animName = CHECK_STRING(L, 2);

    comp->CancelQueuedAnimation(animName);

    return 0;
}

int SkeletalMesh3D_Lua::CancelAllQueuedAnimations(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_3D(L, 1);

    comp->CancelAllQueuedAnimations();

    return 0;
}

int SkeletalMesh3D_Lua::SetInheritPose(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_3D(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    comp->SetInheritPose(value);

    return 0;
}

int SkeletalMesh3D_Lua::IsInheritPoseEnabled(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_3D(L, 1);

    bool ret = comp->IsInheritPoseEnabled();

    lua_pushboolean(L, ret);
    return 1;
}

int SkeletalMesh3D_Lua::ResetAnimation(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_3D(L, 1);

    comp->ResetAnimation();

    return 0;
}

int SkeletalMesh3D_Lua::GetAnimationSpeed(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_3D(L, 1);

    float ret = comp->GetAnimationSpeed();

    lua_pushnumber(L, ret);
    return 1;
}

int SkeletalMesh3D_Lua::SetAnimationSpeed(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_3D(L, 1);
    float speed = CHECK_NUMBER(L, 2);

    comp->SetAnimationSpeed(speed);

    return 0;
}

int SkeletalMesh3D_Lua::SetAnimationPaused(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_3D(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    comp->SetAnimationPaused(value);

    return 0;
}

int SkeletalMesh3D_Lua::IsAnimationPaused(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_3D(L, 1);

    bool ret = comp->IsAnimationPaused();

    lua_pushboolean(L, ret);
    return 1;
}

int SkeletalMesh3D_Lua::GetBonePosition(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_3D(L, 1);
    const char* boneName = CHECK_STRING(L, 2);

    glm::vec3 ret = comp->GetBonePosition(boneName);

    Vector_Lua::Create(L, ret);
    return 1;
}

int SkeletalMesh3D_Lua::GetBoneRotation(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_3D(L, 1);
    const char* boneName = CHECK_STRING(L, 2);

    glm::vec3 ret = comp->GetBoneRotationEuler(boneName);

    Vector_Lua::Create(L, ret);
    return 1;
}

int SkeletalMesh3D_Lua::GetBoneScale(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_3D(L, 1);
    const char* boneName = CHECK_STRING(L, 2);

    glm::vec3 ret = comp->GetBoneScale(boneName);

    Vector_Lua::Create(L, ret);
    return 1;
}

int SkeletalMesh3D_Lua::SetBonePosition(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_3D(L, 1);
    const char* boneName = CHECK_STRING(L, 2);
    glm::vec3 value = CHECK_VECTOR(L, 3);

    int32_t boneIndex = comp->FindBoneIndex(boneName);
    comp->SetBonePosition(boneIndex, value);

    return 0;
}

int SkeletalMesh3D_Lua::SetBoneRotation(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_3D(L, 1);
    const char* boneName = CHECK_STRING(L, 2);
    glm::vec3 value = CHECK_VECTOR(L, 3);

    int32_t boneIndex = comp->FindBoneIndex(boneName);
    comp->SetBoneRotation(boneIndex, value);

    return 0;
}

int SkeletalMesh3D_Lua::SetBoneScale(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_3D(L, 1);
    const char* boneName = CHECK_STRING(L, 2);
    glm::vec3 value = CHECK_VECTOR(L, 3);

    int32_t boneIndex = comp->FindBoneIndex(boneName);
    comp->SetBoneScale(boneIndex, value);

    return 0;
}

int SkeletalMesh3D_Lua::GetNumBones(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_3D(L, 1);

    uint32_t numBones = comp->GetNumBones();

    lua_pushinteger(L, (int)numBones);
    return 1;
}

int SkeletalMesh3D_Lua::SetAnimEventHandler(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_3D(L, 1);
    CHECK_FUNCTION(L, 2);
    ScriptFunc func(L, 2);

    comp->SetScriptAnimEventHandler(func);

    return 0;
}

int SkeletalMesh3D_Lua::SetBoundsRadiusOverride(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_3D(L, 1);
    float radius = CHECK_NUMBER(L, 2);

    comp->SetBoundsRadiusOverride(radius);

    return 0;
}

int SkeletalMesh3D_Lua::GetBoundsRadiusOverride(lua_State* L)
{
    SkeletalMesh3D* comp = CHECK_SKELETAL_MESH_3D(L, 1);

    float radius = comp->GetBoundsRadiusOverride();

    lua_pushnumber(L, radius);
    return 1;
}

void SkeletalMesh3D_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        SKELETAL_MESH_3D_LUA_NAME,
        SKELETAL_MESH_3D_LUA_FLAG,
        MESH_3D_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, SetSkeletalMesh);

    REGISTER_TABLE_FUNC(L, mtIndex, GetSkeletalMesh);

    REGISTER_TABLE_FUNC(L, mtIndex, PlayAnimation);

    REGISTER_TABLE_FUNC(L, mtIndex, StopAnimation);

    REGISTER_TABLE_FUNC(L, mtIndex, StopAllAnimations);

    REGISTER_TABLE_FUNC(L, mtIndex, IsAnimationPlaying);

    REGISTER_TABLE_FUNC(L, mtIndex, QueueAnimation);

    REGISTER_TABLE_FUNC(L, mtIndex, CancelQueuedAnimation);

    REGISTER_TABLE_FUNC(L, mtIndex, CancelAllQueuedAnimations);

    REGISTER_TABLE_FUNC(L, mtIndex, SetInheritPose);

    REGISTER_TABLE_FUNC(L, mtIndex, IsInheritPoseEnabled);

    REGISTER_TABLE_FUNC(L, mtIndex, ResetAnimation);

    REGISTER_TABLE_FUNC(L, mtIndex, GetAnimationSpeed);

    REGISTER_TABLE_FUNC(L, mtIndex, SetAnimationSpeed);

    REGISTER_TABLE_FUNC(L, mtIndex, SetAnimationPaused);

    REGISTER_TABLE_FUNC(L, mtIndex, IsAnimationPaused);

    REGISTER_TABLE_FUNC(L, mtIndex, GetBonePosition);

    REGISTER_TABLE_FUNC(L, mtIndex, GetBoneRotation);

    REGISTER_TABLE_FUNC(L, mtIndex, GetBoneScale);

    REGISTER_TABLE_FUNC(L, mtIndex, SetBonePosition);

    REGISTER_TABLE_FUNC(L, mtIndex, SetBoneRotation);

    REGISTER_TABLE_FUNC(L, mtIndex, SetBoneScale);

    REGISTER_TABLE_FUNC(L, mtIndex, GetNumBones);

    REGISTER_TABLE_FUNC(L, mtIndex, SetAnimEventHandler);

    REGISTER_TABLE_FUNC(L, mtIndex, SetBoundsRadiusOverride);

    REGISTER_TABLE_FUNC(L, mtIndex, GetBoundsRadiusOverride);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}
