#include "LuaBindings/Node3d_Lua.h"
#include "LuaBindings/SkeletalMesh3d_Lua.h"

#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Node_Lua.h"

#if LUA_ENABLED

int Node3D_Lua::AttachToBone(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);
    SkeletalMesh3D* newParent = CHECK_SKELETAL_MESH_3D(L, 2);
    const char* boneName = CHECK_STRING(L, 3);
    bool keepWorldTransform = false;
    int32_t childIndex = -1;

    if (!lua_isnone(L, 4))
    {
        keepWorldTransform = CHECK_BOOLEAN(L, 4);
    }

    if (!lua_isnone(L, 5))
    {
        childIndex = CHECK_INTEGER(L, 5);
    }

    comp->AttachToBone(newParent, boneName, keepWorldTransform, childIndex);

    return 0;
}

int Node3D_Lua::UpdateTransform(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);
    bool updateChildren = false;
    if (lua_gettop(L) == 2 &&
        lua_isboolean(L, 2))
    {
        updateChildren = lua_toboolean(L, 2);
    }

    comp->UpdateTransform(updateChildren);

    return 0;
}

int Node3D_Lua::GetPosition(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);

    glm::vec3 position = comp->GetPosition();

    Vector_Lua::Create(L, glm::vec4(position, 0.0f));
    return 1;
}

int Node3D_Lua::GetRotationEuler(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);

    glm::vec3 rotEuler = comp->GetRotationEuler();

    Vector_Lua::Create(L, glm::vec4(rotEuler, 0.0f));
    return 1;
}

int Node3D_Lua::GetRotationQuat(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);

    glm::quat rotQuat = comp->GetRotationQuat();

    Vector_Lua::Create(L, LuaQuatToVector(rotQuat));
    return 1;
}

int Node3D_Lua::GetScale(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);

    glm::vec3 scale = comp->GetScale();

    Vector_Lua::Create(L, glm::vec4(scale, 0.0f));
    return 1;
}

int Node3D_Lua::SetPosition(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);
    glm::vec4& pos = CHECK_VECTOR(L, 2);

    comp->SetPosition(glm::vec3(pos));

    return 0;
}

int Node3D_Lua::SetRotationEuler(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);
    glm::vec4& rotEuler = CHECK_VECTOR(L, 2);

    comp->SetRotation(glm::vec3(rotEuler));

    return 0;
}

int Node3D_Lua::SetRotationQuat(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);
    glm::vec4& rotQuat = CHECK_VECTOR(L, 2);

    comp->SetRotation(LuaVectorToQuat(rotQuat));

    return 0;
}

int Node3D_Lua::SetScale(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);
    glm::vec4& scale = CHECK_VECTOR(L, 2);

    comp->SetScale(glm::vec3(scale));

    return 0;
}

int Node3D_Lua::RotateAround(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);
    glm::vec3 pivot = CHECK_VECTOR(L, 2);
    glm::vec3 axis = CHECK_VECTOR(L, 3);
    float degrees = CHECK_NUMBER(L, 4);

    comp->RotateAround(pivot, axis, degrees);

    return 0;
}

int Node3D_Lua::GetWorldPosition(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);

    glm::vec3 absPos = comp->GetWorldPosition();

    Vector_Lua::Create(L, glm::vec4(absPos, 0.0f));
    return 1;
}

int Node3D_Lua::GetWorldRotationEuler(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);

    glm::vec3 absRotEuler = comp->GetWorldRotationEuler();

    Vector_Lua::Create(L, glm::vec4(absRotEuler, 0.0f));
    return 1;
}

int Node3D_Lua::GetWorldRotationQuat(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);

    glm::quat absQuatEuler = comp->GetWorldRotationQuat();

    Vector_Lua::Create(L, LuaQuatToVector(absQuatEuler));
    return 1;
}

int Node3D_Lua::GetWorldScale(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);

    glm::vec3 absScale = comp->GetWorldScale();

    Vector_Lua::Create(L, glm::vec4(absScale, 0.0f));
    return 1;
}

int Node3D_Lua::SetWorldPosition(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);
    glm::vec4& pos = CHECK_VECTOR(L, 2);

    comp->SetWorldPosition(glm::vec3(pos));

    return 0;
}

int Node3D_Lua::SetWorldRotationEuler(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);
    glm::vec4& rotEuler = CHECK_VECTOR(L, 2);

    comp->SetWorldRotation(glm::vec3(rotEuler));

    return 0;
}

int Node3D_Lua::SetWorldRotationQuat(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);
    glm::vec4& rotQuat = CHECK_VECTOR(L, 2);

    comp->SetWorldRotation(LuaVectorToQuat(rotQuat));

    return 0;
}

int Node3D_Lua::SetWorldScale(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);
    glm::vec4& scale = CHECK_VECTOR(L, 2);

    comp->SetWorldScale(glm::vec3(scale));

    return 0;
}

int Node3D_Lua::AddRotationEuler(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);
    glm::vec3 deltaDegrees = CHECK_VECTOR(L, 2);

    comp->AddRotation(deltaDegrees);

    return 0;
}

int Node3D_Lua::AddRotationQuat(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);
    glm::vec4 deltaVec = CHECK_VECTOR(L, 2);

    glm::quat deltaQuat = LuaVectorToQuat(deltaVec);
    comp->AddRotation(deltaQuat);

    return 0;
}

int Node3D_Lua::AddWorldRotationEuler(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);
    glm::vec3 deltaDegrees = CHECK_VECTOR(L, 2);

    comp->AddWorldRotation(deltaDegrees);

    return 0;
}

int Node3D_Lua::AddWorldRotationQuat(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);
    glm::vec4 deltaVec = CHECK_VECTOR(L, 2);

    glm::quat deltaQuat = LuaVectorToQuat(deltaVec);
    comp->AddWorldRotation(deltaQuat);

    return 0;
}

int Node3D_Lua::LookAt(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);
    glm::vec3 worldPos = CHECK_VECTOR(L, 2);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    if (lua_gettop(L) == 3)
    {
        up = CHECK_VECTOR(L, 3);
    }

    comp->LookAt(worldPos, up);

    return 0;
}

int Node3D_Lua::GetForwardVector(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);

    glm::vec3 fwd = comp->GetForwardVector();

    Vector_Lua::Create(L, glm::vec4(fwd, 0.0f));
    return 1;
}

int Node3D_Lua::GetRightVector(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);

    glm::vec3 right = comp->GetRightVector();

    Vector_Lua::Create(L, glm::vec4(right, 0.0f));
    return 1;
}

int Node3D_Lua::GetUpVector(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);

    glm::vec3 up = comp->GetUpVector();

    Vector_Lua::Create(L, glm::vec4(up, 0.0f));
    return 1;
}

int Node3D_Lua::GetInheritTransform(lua_State* L)
{
    Node3D* node = CHECK_NODE_3D(L, 1);

    bool ret = node->GetInheritTransform();

    lua_pushboolean(L, ret);
    return 1;
}

int Node3D_Lua::SetInheritTransform(lua_State* L)
{
    Node3D* node = CHECK_NODE_3D(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    node->SetInheritTransform(value);

    return 0;
}

void Node3D_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        NODE_3D_LUA_NAME,
        NODE_3D_LUA_FLAG,
        NODE_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, AttachToBone);

    REGISTER_TABLE_FUNC(L, mtIndex, UpdateTransform);

    REGISTER_TABLE_FUNC(L, mtIndex, GetPosition);

    REGISTER_TABLE_FUNC(L, mtIndex, GetRotationEuler);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, GetRotationEuler, "GetRotation");

    REGISTER_TABLE_FUNC(L, mtIndex, GetRotationQuat);

    REGISTER_TABLE_FUNC(L, mtIndex, GetScale);

    REGISTER_TABLE_FUNC(L, mtIndex, SetPosition);

    REGISTER_TABLE_FUNC(L, mtIndex, SetRotationEuler);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, SetRotationEuler, "SetRotation");

    REGISTER_TABLE_FUNC(L, mtIndex, SetRotationQuat);

    REGISTER_TABLE_FUNC(L, mtIndex, SetScale);

    REGISTER_TABLE_FUNC(L, mtIndex, RotateAround);

    REGISTER_TABLE_FUNC(L, mtIndex, GetWorldPosition);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, GetWorldPosition, "GetAbsolutePosition");

    REGISTER_TABLE_FUNC(L, mtIndex, GetWorldRotationEuler);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, GetWorldRotationEuler, "GetWorldRotation");
    REGISTER_TABLE_FUNC_EX(L, mtIndex, GetWorldRotationEuler, "GetAbsoluteRotation");

    REGISTER_TABLE_FUNC(L, mtIndex, GetWorldRotationQuat);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, GetWorldRotationQuat, "GetAbsoluteRotationQuat");


    REGISTER_TABLE_FUNC(L, mtIndex, GetWorldScale);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, GetWorldScale, "GetAbsoluteScale");

    REGISTER_TABLE_FUNC(L, mtIndex, SetWorldPosition);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, SetWorldPosition, "SetAbsolutePosition");

    REGISTER_TABLE_FUNC(L, mtIndex, SetWorldRotationEuler);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, SetWorldRotationEuler, "SetWorldRotation");
    REGISTER_TABLE_FUNC_EX(L, mtIndex, SetWorldRotationEuler, "SetAbsoluteRotation");


    REGISTER_TABLE_FUNC(L, mtIndex, SetWorldRotationQuat);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, SetWorldRotationQuat, "SetAbsoluteRotationQuat");

    REGISTER_TABLE_FUNC(L, mtIndex, SetWorldScale);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, SetWorldScale, "SetAbsoluteScale");

    REGISTER_TABLE_FUNC(L, mtIndex, AddRotationEuler);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, AddRotationEuler, "AddRotation");

    REGISTER_TABLE_FUNC(L, mtIndex, AddRotationQuat);

    REGISTER_TABLE_FUNC(L, mtIndex, AddWorldRotationEuler);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, AddWorldRotationEuler, "AddWorldRotation");
    REGISTER_TABLE_FUNC_EX(L, mtIndex, AddWorldRotationEuler, "AddAbsoluteRotation");

    REGISTER_TABLE_FUNC(L, mtIndex, AddWorldRotationQuat);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, AddWorldRotationQuat, "AddAbsoluteRotationQuat");

    REGISTER_TABLE_FUNC(L, mtIndex, LookAt);

    REGISTER_TABLE_FUNC(L, mtIndex, GetForwardVector);

    REGISTER_TABLE_FUNC(L, mtIndex, GetRightVector);

    REGISTER_TABLE_FUNC(L, mtIndex, GetUpVector);

    REGISTER_TABLE_FUNC(L, mtIndex, GetInheritTransform);
    REGISTER_TABLE_FUNC(L, mtIndex, SetInheritTransform);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
