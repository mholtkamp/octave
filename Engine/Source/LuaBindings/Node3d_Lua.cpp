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

int Node3D_Lua::GetAbsolutePosition(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);

    glm::vec3 absPos = comp->GetAbsolutePosition();

    Vector_Lua::Create(L, glm::vec4(absPos, 0.0f));
    return 1;
}

int Node3D_Lua::GetAbsoluteRotationEuler(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);

    glm::vec3 absRotEuler = comp->GetAbsoluteRotationEuler();

    Vector_Lua::Create(L, glm::vec4(absRotEuler, 0.0f));
    return 1;
}

int Node3D_Lua::GetAbsoluteRotationQuat(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);

    glm::quat absQuatEuler = comp->GetAbsoluteRotationQuat();

    Vector_Lua::Create(L, LuaQuatToVector(absQuatEuler));
    return 1;
}

int Node3D_Lua::GetAbsoluteScale(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);

    glm::vec3 absScale = comp->GetAbsoluteScale();

    Vector_Lua::Create(L, glm::vec4(absScale, 0.0f));
    return 1;
}

int Node3D_Lua::SetAbsolutePosition(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);
    glm::vec4& pos = CHECK_VECTOR(L, 2);

    comp->SetAbsolutePosition(glm::vec3(pos));

    return 0;
}

int Node3D_Lua::SetAbsoluteRotationEuler(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);
    glm::vec4& rotEuler = CHECK_VECTOR(L, 2);

    comp->SetAbsoluteRotation(glm::vec3(rotEuler));

    return 0;
}

int Node3D_Lua::SetAbsoluteRotationQuat(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);
    glm::vec4& rotQuat = CHECK_VECTOR(L, 2);

    comp->SetAbsoluteRotation(LuaVectorToQuat(rotQuat));

    return 0;
}

int Node3D_Lua::SetAbsoluteScale(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);
    glm::vec4& scale = CHECK_VECTOR(L, 2);

    comp->SetAbsoluteScale(glm::vec3(scale));

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

int Node3D_Lua::AddAbsoluteRotationEuler(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);
    glm::vec3 deltaDegrees = CHECK_VECTOR(L, 2);

    comp->AddAbsoluteRotation(deltaDegrees);

    return 0;
}

int Node3D_Lua::AddAbsoluteRotationQuat(lua_State* L)
{
    Node3D* comp = CHECK_NODE_3D(L, 1);
    glm::vec4 deltaVec = CHECK_VECTOR(L, 2);

    glm::quat deltaQuat = LuaVectorToQuat(deltaVec);
    comp->AddAbsoluteRotation(deltaQuat);

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

    REGISTER_TABLE_FUNC(L, mtIndex, GetAbsolutePosition);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, GetAbsolutePosition, "GetWorldPosition");

    REGISTER_TABLE_FUNC(L, mtIndex, GetAbsoluteRotationEuler);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, GetAbsoluteRotationEuler, "GetAbsoluteRotation");
    REGISTER_TABLE_FUNC_EX(L, mtIndex, GetAbsoluteRotationEuler, "GetWorldRotation");

    REGISTER_TABLE_FUNC(L, mtIndex, GetAbsoluteRotationQuat);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, GetAbsoluteRotationQuat, "GetWorldRotationQuat");


    REGISTER_TABLE_FUNC(L, mtIndex, GetAbsoluteScale);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, GetAbsoluteScale, "GetWorldScale");

    REGISTER_TABLE_FUNC(L, mtIndex, SetAbsolutePosition);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, SetAbsolutePosition, "SetWorldPosition");

    REGISTER_TABLE_FUNC(L, mtIndex, SetAbsoluteRotationEuler);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, SetAbsoluteRotationEuler, "SetAbsoluteRotation");
    REGISTER_TABLE_FUNC_EX(L, mtIndex, SetAbsoluteRotationEuler, "SetWorldRotation");


    REGISTER_TABLE_FUNC(L, mtIndex, SetAbsoluteRotationQuat);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, SetAbsoluteRotationQuat, "SetWorldRotationQuat");

    REGISTER_TABLE_FUNC(L, mtIndex, SetAbsoluteScale);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, SetAbsoluteScale, "SetWorldScale");

    REGISTER_TABLE_FUNC(L, mtIndex, AddRotationEuler);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, AddRotationEuler, "AddRotation");

    REGISTER_TABLE_FUNC(L, mtIndex, AddRotationQuat);

    REGISTER_TABLE_FUNC(L, mtIndex, AddAbsoluteRotationEuler);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, AddAbsoluteRotationEuler, "AddAbsoluteRotation");
    REGISTER_TABLE_FUNC_EX(L, mtIndex, AddAbsoluteRotationEuler, "AddWorldRotation");

    REGISTER_TABLE_FUNC(L, mtIndex, AddAbsoluteRotationQuat);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, AddAbsoluteRotationQuat, "AddWorldRotationQuat");

    REGISTER_TABLE_FUNC(L, mtIndex, LookAt);

    REGISTER_TABLE_FUNC(L, mtIndex, GetForwardVector);

    REGISTER_TABLE_FUNC(L, mtIndex, GetRightVector);

    REGISTER_TABLE_FUNC(L, mtIndex, GetUpVector);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
