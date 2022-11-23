#include "LuaBindings/TransformComponent_Lua.h"
#include "LuaBindings/SkeletalMeshComponent_Lua.h"

#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Component_Lua.h"

#if LUA_ENABLED

int TransformComponent_Lua::Attach(lua_State* L)
{
    TransformComponent* comp = CHECK_TRANSFORM_COMPONENT(L, 1);
    TransformComponent* newParent = nullptr;
    bool keepWorldTransform = false;

    if (!lua_isnil(L, 2))
    {
        newParent = CHECK_TRANSFORM_COMPONENT(L, 2);
    }
    if (!lua_isnone(L, 3))
    {
        keepWorldTransform = CHECK_BOOLEAN(L, 3);
    }

    comp->Attach(newParent, keepWorldTransform);

    return 0;
}

int TransformComponent_Lua::AttachToBone(lua_State* L)
{
    TransformComponent* comp = CHECK_TRANSFORM_COMPONENT(L, 1);
    SkeletalMeshComponent* newParent = CHECK_SKELETAL_MESH_COMPONENT(L, 2);
    const char* boneName = CHECK_STRING(L, 3);
    bool keepWorldTransform = false;

    if (!lua_isnone(L, 4))
    {
        keepWorldTransform = CHECK_BOOLEAN(L, 4);
    }

    comp->AttachToBone(newParent, boneName, keepWorldTransform);

    return 0;
}

int TransformComponent_Lua::GetParent(lua_State* L)
{
    TransformComponent* comp = CHECK_TRANSFORM_COMPONENT(L, 1);

    TransformComponent* parentComp = comp->GetParent();

    Component_Lua::Create(L, parentComp);
    return 1;
}

int TransformComponent_Lua::GetChild(lua_State* L)
{
    TransformComponent* comp = CHECK_TRANSFORM_COMPONENT(L, 1);
    TransformComponent* child = nullptr;
    
    if (lua_isinteger(L, 2))
    {
        int32_t childIndex = (int32_t)lua_tointeger(L, 2) - 1;
        child = comp->GetChild(childIndex);
    }
    else
    {
        const char* childName = CHECK_STRING(L, 2);
        child = comp->GetChild(childName);
    }

    Component_Lua::Create(L, child);
    return 1;
}

int TransformComponent_Lua::GetNumChildren(lua_State* L)
{
    TransformComponent* comp = CHECK_TRANSFORM_COMPONENT(L, 1);

    uint32_t numComps = comp->GetNumChildren();

    lua_pushinteger(L, numComps);
    return 1;
}

int TransformComponent_Lua::UpdateTransform(lua_State* L)
{
    TransformComponent* comp = CHECK_TRANSFORM_COMPONENT(L, 1);
    bool updateChildren = false;
    if (lua_gettop(L) == 2 &&
        lua_isboolean(L, 2))
    {
        updateChildren = lua_toboolean(L, 2);
    }

    comp->UpdateTransform(updateChildren);

    return 0;
}

int TransformComponent_Lua::GetPosition(lua_State* L)
{
    TransformComponent* comp = CHECK_TRANSFORM_COMPONENT(L, 1);

    glm::vec3 position = comp->GetPosition();

    Vector_Lua::Create(L, glm::vec4(position, 0.0f));
    return 1;
}

int TransformComponent_Lua::GetRotationEuler(lua_State* L)
{
    TransformComponent* comp = CHECK_TRANSFORM_COMPONENT(L, 1);

    glm::vec3 rotEuler = comp->GetRotationEuler();

    Vector_Lua::Create(L, glm::vec4(rotEuler, 0.0f));
    return 1;
}

int TransformComponent_Lua::GetRotationQuat(lua_State* L)
{
    TransformComponent* comp = CHECK_TRANSFORM_COMPONENT(L, 1);

    glm::quat rotQuat = comp->GetRotationQuat();

    Vector_Lua::Create(L, QuatToVector(rotQuat));
    return 1;
}

int TransformComponent_Lua::GetScale(lua_State* L)
{
    TransformComponent* comp = CHECK_TRANSFORM_COMPONENT(L, 1);

    glm::vec3 scale = comp->GetScale();

    Vector_Lua::Create(L, glm::vec4(scale, 0.0f));
    return 1;
}

int TransformComponent_Lua::SetPosition(lua_State* L)
{
    TransformComponent* comp = CHECK_TRANSFORM_COMPONENT(L, 1);
    glm::vec4& pos = CHECK_VECTOR(L, 2);

    comp->SetPosition(glm::vec3(pos));

    return 0;
}

int TransformComponent_Lua::SetRotationEuler(lua_State* L)
{
    TransformComponent* comp = CHECK_TRANSFORM_COMPONENT(L, 1);
    glm::vec4& rotEuler = CHECK_VECTOR(L, 2);

    comp->SetRotation(glm::vec3(rotEuler));

    return 0;
}

int TransformComponent_Lua::SetRotationQuat(lua_State* L)
{
    TransformComponent* comp = CHECK_TRANSFORM_COMPONENT(L, 1);
    glm::vec4& rotQuat = CHECK_VECTOR(L, 2);

    comp->SetRotation(VectorToQuat(rotQuat));

    return 0;
}

int TransformComponent_Lua::SetScale(lua_State* L)
{
    TransformComponent* comp = CHECK_TRANSFORM_COMPONENT(L, 1);
    glm::vec4& scale = CHECK_VECTOR(L, 2);

    comp->SetScale(glm::vec3(scale));

    return 0;
}

int TransformComponent_Lua::RotateAround(lua_State* L)
{
    TransformComponent* comp = CHECK_TRANSFORM_COMPONENT(L, 1);
    glm::vec3 pivot = CHECK_VECTOR(L, 2);
    glm::vec3 axis = CHECK_VECTOR(L, 3);
    float degrees = CHECK_NUMBER(L, 4);

    comp->RotateAround(pivot, axis, degrees);

    return 0;
}

int TransformComponent_Lua::GetAbsolutePosition(lua_State* L)
{
    TransformComponent* comp = CHECK_TRANSFORM_COMPONENT(L, 1);

    glm::vec3 absPos = comp->GetAbsolutePosition();

    Vector_Lua::Create(L, glm::vec4(absPos, 0.0f));
    return 1;
}

int TransformComponent_Lua::GetAbsoluteRotationEuler(lua_State* L)
{
    TransformComponent* comp = CHECK_TRANSFORM_COMPONENT(L, 1);

    glm::vec3 absRotEuler = comp->GetAbsoluteRotationEuler();

    Vector_Lua::Create(L, glm::vec4(absRotEuler, 0.0f));
    return 1;
}

int TransformComponent_Lua::GetAbsoluteRotationQuat(lua_State* L)
{
    TransformComponent* comp = CHECK_TRANSFORM_COMPONENT(L, 1);

    glm::quat absQuatEuler = comp->GetAbsoluteRotationQuat();

    Vector_Lua::Create(L, QuatToVector(absQuatEuler));
    return 1;
}

int TransformComponent_Lua::GetAbsoluteScale(lua_State* L)
{
    TransformComponent* comp = CHECK_TRANSFORM_COMPONENT(L, 1);

    glm::vec3 absScale = comp->GetAbsoluteScale();

    Vector_Lua::Create(L, glm::vec4(absScale, 0.0f));
    return 1;
}

int TransformComponent_Lua::SetAbsolutePosition(lua_State* L)
{
    TransformComponent* comp = CHECK_TRANSFORM_COMPONENT(L, 1);
    glm::vec4& pos = CHECK_VECTOR(L, 2);

    comp->SetAbsolutePosition(glm::vec3(pos));

    return 0;
}

int TransformComponent_Lua::SetAbsoluteRotationEuler(lua_State* L)
{
    TransformComponent* comp = CHECK_TRANSFORM_COMPONENT(L, 1);
    glm::vec4& rotEuler = CHECK_VECTOR(L, 2);

    comp->SetAbsoluteRotation(glm::vec3(rotEuler));

    return 0;
}

int TransformComponent_Lua::SetAbsoluteRotationQuat(lua_State* L)
{
    TransformComponent* comp = CHECK_TRANSFORM_COMPONENT(L, 1);
    glm::vec4& rotQuat = CHECK_VECTOR(L, 2);

    comp->SetAbsoluteRotation(VectorToQuat(rotQuat));

    return 0;
}

int TransformComponent_Lua::SetAbsoluteScale(lua_State* L)
{
    TransformComponent* comp = CHECK_TRANSFORM_COMPONENT(L, 1);
    glm::vec4& scale = CHECK_VECTOR(L, 2);

    comp->SetAbsoluteScale(glm::vec3(scale));

    return 0;
}

int TransformComponent_Lua::AddRotationEuler(lua_State* L)
{
    TransformComponent* comp = CHECK_TRANSFORM_COMPONENT(L, 1);
    glm::vec3 deltaDegrees = CHECK_VECTOR(L, 2);

    comp->AddRotation(deltaDegrees);

    return 0;
}

int TransformComponent_Lua::AddRotationQuat(lua_State* L)
{
    TransformComponent* comp = CHECK_TRANSFORM_COMPONENT(L, 1);
    glm::vec4 deltaVec = CHECK_VECTOR(L, 2);

    glm::quat deltaQuat = VectorToQuat(deltaVec);
    comp->AddRotation(deltaQuat);

    return 0;
}

int TransformComponent_Lua::AddAbsoluteRotationEuler(lua_State* L)
{
    TransformComponent* comp = CHECK_TRANSFORM_COMPONENT(L, 1);
    glm::vec3 deltaDegrees = CHECK_VECTOR(L, 2);

    comp->AddAbsoluteRotation(deltaDegrees);

    return 0;
}

int TransformComponent_Lua::AddAbsoluteRotationQuat(lua_State* L)
{
    TransformComponent* comp = CHECK_TRANSFORM_COMPONENT(L, 1);
    glm::vec4 deltaVec = CHECK_VECTOR(L, 2);

    glm::quat deltaQuat = VectorToQuat(deltaVec);
    comp->AddAbsoluteRotation(deltaQuat);

    return 0;
}

int TransformComponent_Lua::LookAt(lua_State* L)
{
    TransformComponent* comp = CHECK_TRANSFORM_COMPONENT(L, 1);
    glm::vec3 worldPos = CHECK_VECTOR(L, 2);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    if (lua_gettop(L) == 3)
    {
        up = CHECK_VECTOR(L, 3);
    }

    comp->LookAt(worldPos, up);

    return 0;
}

int TransformComponent_Lua::GetForwardVector(lua_State* L)
{
    TransformComponent* comp = CHECK_TRANSFORM_COMPONENT(L, 1);

    glm::vec3 fwd = comp->GetForwardVector();

    Vector_Lua::Create(L, glm::vec4(fwd, 0.0f));
    return 1;
}

int TransformComponent_Lua::GetRightVector(lua_State* L)
{
    TransformComponent* comp = CHECK_TRANSFORM_COMPONENT(L, 1);

    glm::vec3 right = comp->GetRightVector();

    Vector_Lua::Create(L, glm::vec4(right, 0.0f));
    return 1;
}

int TransformComponent_Lua::GetUpVector(lua_State* L)
{
    TransformComponent* comp = CHECK_TRANSFORM_COMPONENT(L, 1);

    glm::vec3 up = comp->GetUpVector();

    Vector_Lua::Create(L, glm::vec4(up, 0.0f));
    return 1;
}

void TransformComponent_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        TRANSFORM_COMPONENT_LUA_NAME,
        TRANSFORM_COMPONENT_LUA_FLAG,
        COMPONENT_LUA_NAME);

    Component_Lua::BindCommon(L, mtIndex);

    lua_pushcfunction(L, TransformComponent_Lua::Attach);
    lua_setfield(L, mtIndex, "Attach");

    lua_pushcfunction(L, TransformComponent_Lua::AttachToBone);
    lua_setfield(L, mtIndex, "AttachToBone");

    lua_pushcfunction(L, TransformComponent_Lua::GetParent);
    lua_setfield(L, mtIndex, "GetParent");

    lua_pushcfunction(L, TransformComponent_Lua::GetChild);
    lua_setfield(L, mtIndex, "GetChild");

    lua_pushcfunction(L, TransformComponent_Lua::GetNumChildren);
    lua_setfield(L, mtIndex, "GetNumChildren");

    lua_pushcfunction(L, TransformComponent_Lua::UpdateTransform);
    lua_setfield(L, mtIndex, "UpdateTransform");

    lua_pushcfunction(L, TransformComponent_Lua::GetPosition);
    lua_setfield(L, mtIndex, "GetPosition");

    lua_pushcfunction(L, TransformComponent_Lua::GetRotationEuler);
    lua_pushvalue(L, -1);
    lua_setfield(L, mtIndex, "GetRotationEuler");
    lua_setfield(L, mtIndex, "GetRotation");

    lua_pushcfunction(L, TransformComponent_Lua::GetRotationQuat);
    lua_setfield(L, mtIndex, "GetRotationQuat");

    lua_pushcfunction(L, TransformComponent_Lua::GetScale);
    lua_setfield(L, mtIndex, "GetScale");

    lua_pushcfunction(L, TransformComponent_Lua::SetPosition);
    lua_setfield(L, mtIndex, "SetPosition");

    lua_pushcfunction(L, TransformComponent_Lua::SetRotationEuler);
    lua_pushvalue(L, -1);
    lua_setfield(L, mtIndex, "SetRotationEuler");
    lua_setfield(L, mtIndex, "SetRotation");

    lua_pushcfunction(L, TransformComponent_Lua::SetRotationQuat);
    lua_setfield(L, mtIndex, "SetRotationQuat");

    lua_pushcfunction(L, TransformComponent_Lua::SetScale);
    lua_setfield(L, mtIndex, "SetScale");

    lua_pushcfunction(L, TransformComponent_Lua::RotateAround);
    lua_setfield(L, mtIndex, "RotateAround");

    lua_pushcfunction(L, TransformComponent_Lua::GetAbsolutePosition);
    lua_setfield(L, mtIndex, "GetAbsolutePosition");

    lua_pushcfunction(L, TransformComponent_Lua::GetAbsoluteRotationEuler);
    lua_pushvalue(L, -1);
    lua_setfield(L, mtIndex, "GetAbsoluteRotationEuler");
    lua_setfield(L, mtIndex, "GetAbsoluteRotation");

    lua_pushcfunction(L, TransformComponent_Lua::GetAbsoluteRotationQuat);
    lua_setfield(L, mtIndex, "GetAbsoluteRotationQuat");

    lua_pushcfunction(L, TransformComponent_Lua::GetAbsoluteScale);
    lua_setfield(L, mtIndex, "GetAbsoluteScale");

    lua_pushcfunction(L, TransformComponent_Lua::SetAbsolutePosition);
    lua_setfield(L, mtIndex, "SetAbsolutePosition");

    lua_pushcfunction(L, TransformComponent_Lua::SetAbsoluteRotationEuler);
    lua_pushvalue(L, -1);
    lua_setfield(L, mtIndex, "SetAbsoluteRotationEuler");
    lua_setfield(L, mtIndex, "SetAbsoluteRotation");

    lua_pushcfunction(L, TransformComponent_Lua::SetAbsoluteRotationQuat);
    lua_setfield(L, mtIndex, "SetAbsoluteRotationQuat");

    lua_pushcfunction(L, TransformComponent_Lua::SetAbsoluteScale);
    lua_setfield(L, mtIndex, "SetAbsoluteScale");

    lua_pushcfunction(L, TransformComponent_Lua::AddRotationEuler);
    lua_pushvalue(L, -1);
    lua_setfield(L, mtIndex, "AddRotationEuler");
    lua_setfield(L, mtIndex, "AddRotation");

    lua_pushcfunction(L, TransformComponent_Lua::AddRotationQuat);
    lua_setfield(L, mtIndex, "AddRotationQuat");

    lua_pushcfunction(L, TransformComponent_Lua::AddAbsoluteRotationEuler);
    lua_pushvalue(L, -1);
    lua_setfield(L, mtIndex, "AddAbsoluteRotationEuler");
    lua_setfield(L, mtIndex, "AddAbsoluteRotation");

    lua_pushcfunction(L, TransformComponent_Lua::AddAbsoluteRotationQuat);
    lua_setfield(L, mtIndex, "AddAbsoluteRotationQuat");

    lua_pushcfunction(L, TransformComponent_Lua::LookAt);
    lua_setfield(L, mtIndex, "LookAt");

    lua_pushcfunction(L, TransformComponent_Lua::GetForwardVector);
    lua_setfield(L, mtIndex, "GetForwardVector");

    lua_pushcfunction(L, TransformComponent_Lua::GetRightVector);
    lua_setfield(L, mtIndex, "GetRightVector");

    lua_pushcfunction(L, TransformComponent_Lua::GetUpVector);
    lua_setfield(L, mtIndex, "GetUpVector");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
