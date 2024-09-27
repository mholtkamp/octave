#include "LuaBindings/Primitive3d_Lua.h"
#include "LuaBindings/Node3d_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

int Primitive3D_Lua::EnablePhysics(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);
    bool enable = CHECK_BOOLEAN(L, 2);

    prim->EnablePhysics(enable);

    return 0;
}

int Primitive3D_Lua::EnableCollision(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);
    bool enable = CHECK_BOOLEAN(L, 2);

    prim->EnableCollision(enable);

    return 0;
}

int Primitive3D_Lua::EnableOverlaps(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);
    bool enable = CHECK_BOOLEAN(L, 2);

    prim->EnableOverlaps(enable);

    return 0;
}

int Primitive3D_Lua::IsPhysicsEnabled(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);

    bool enabled = prim->IsPhysicsEnabled();

    lua_pushboolean(L, enabled);
    return 1;
}

int Primitive3D_Lua::IsCollisionEnabled(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);

    bool enabled = prim->IsCollisionEnabled();

    lua_pushboolean(L, enabled);
    return 1;
}

int Primitive3D_Lua::AreOverlapsEnabled(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);

    bool enabled = prim->AreOverlapsEnabled();

    lua_pushboolean(L, enabled);
    return 1;
}

int Primitive3D_Lua::GetMass(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);

    float ret = prim->GetMass();

    lua_pushnumber(L, ret);
    return 1;
}

int Primitive3D_Lua::GetLinearDamping(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);

    float ret = prim->GetLinearDamping();

    lua_pushnumber(L, ret);
    return 1;
}

int Primitive3D_Lua::GetAngularDamping(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);

    float ret = prim->GetAngularDamping();

    lua_pushnumber(L, ret);
    return 1;
}

int Primitive3D_Lua::GetRestitution(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);

    float ret = prim->GetRestitution();

    lua_pushnumber(L, ret);
    return 1;
}

int Primitive3D_Lua::GetFriction(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);

    float ret = prim->GetFriction();

    lua_pushnumber(L, ret);
    return 1;
}

int Primitive3D_Lua::GetRollingFriction(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);

    float ret = prim->GetRollingFriction();

    lua_pushnumber(L, ret);
    return 1;
}

int Primitive3D_Lua::GetLinearFactor(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);

    glm::vec3 ret = prim->GetLinearFactor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Primitive3D_Lua::GetAngularFactor(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);

    glm::vec3 ret = prim->GetAngularFactor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Primitive3D_Lua::GetCollisionGroup(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);

    int ret = prim->GetCollisionGroup();

    lua_pushinteger(L, ret);
    return 1;
}

int Primitive3D_Lua::GetCollisionMask(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);

    int ret = prim->GetCollisionMask();

    lua_pushinteger(L, ret);
    return 1;
}

int Primitive3D_Lua::SetMass(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);
    float value = CHECK_NUMBER(L, 2);

    prim->SetMass(value);

    return 0;
}

int Primitive3D_Lua::SetLinearDamping(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);
    float value = CHECK_NUMBER(L, 2);

    prim->SetLinearDamping(value);

    return 0;
}

int Primitive3D_Lua::SetAngularDamping(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);
    float value = CHECK_NUMBER(L, 2);

    prim->SetAngularDamping(value);

    return 0;
}

int Primitive3D_Lua::SetRestitution(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);
    float value = CHECK_NUMBER(L, 2);

    prim->SetRestitution(value);

    return 0;
}

int Primitive3D_Lua::SetFriction(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);
    float value = CHECK_NUMBER(L, 2);

    prim->SetFriction(value);

    return 0;
}

int Primitive3D_Lua::SetRollingFriction(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);
    float value = CHECK_NUMBER(L, 2);

    prim->SetRollingFriction(value);

    return 0;
}

int Primitive3D_Lua::SetLinearFactor(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);
    glm::vec3 value = CHECK_VECTOR(L, 2);

    prim->SetLinearFactor(value);

    return 0;
}

int Primitive3D_Lua::SetAngularFactor(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);
    glm::vec3 value = CHECK_VECTOR(L, 2);

    prim->SetAngularFactor(value);

    return 0;
}

int Primitive3D_Lua::SetCollisionGroup(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);
    uint8_t value = (uint8_t) CHECK_INTEGER(L, 2);

    prim->SetCollisionGroup(value);

    return 0;
}

int Primitive3D_Lua::SetCollisionMask(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);
    uint8_t value = (uint8_t)CHECK_INTEGER(L, 2);

    prim->SetCollisionMask(value);

    return 0;
}

int Primitive3D_Lua::GetLinearVelocity(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);

    glm::vec3 ret = prim->GetLinearVelocity();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Primitive3D_Lua::GetAngularVelocity(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);

    glm::vec3 ret = prim->GetAngularVelocity();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Primitive3D_Lua::AddLinearVelocity(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);
    glm::vec3 delta = CHECK_VECTOR(L, 2);

    prim->AddLinearVelocity(delta);

    return 0;
}

int Primitive3D_Lua::AddAngularVelocity(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);
    glm::vec3 delta = CHECK_VECTOR(L, 2);

    prim->AddAngularVelocity(delta);

    return 0;
}

int Primitive3D_Lua::SetLinearVelocity(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);
    glm::vec3 value = CHECK_VECTOR(L, 2);

    prim->SetLinearVelocity(value);

    return 0;
}

int Primitive3D_Lua::SetAngularVelocity(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);
    glm::vec3 value = CHECK_VECTOR(L, 2);

    prim->SetAngularVelocity(value);

    return 0;
}

int Primitive3D_Lua::AddForce(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);
    glm::vec3 delta = CHECK_VECTOR(L, 2);

    prim->AddForce(delta);

    return 0;
}

int Primitive3D_Lua::AddImpulse(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);
    glm::vec3 delta = CHECK_VECTOR(L, 2);

    prim->AddImpulse(delta);

    return 0;
}

int Primitive3D_Lua::ClearForces(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);

    prim->ClearForces();

    return 0;
}

int Primitive3D_Lua::EnableCastShadows(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);
    bool enable = CHECK_BOOLEAN(L, 2);

    prim->EnableCastShadows(enable);

    return 0;
}

int Primitive3D_Lua::ShouldCastShadows(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);

    bool ret = prim->ShouldCastShadows();

    lua_pushboolean(L, ret);
    return 1;
}

int Primitive3D_Lua::EnableReceiveShadows(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);
    bool enable = CHECK_BOOLEAN(L, 2);

    prim->EnableReceiveShadows(enable);

    return 0;
}

int Primitive3D_Lua::ShouldReceiveShadows(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);

    bool ret = prim->ShouldReceiveShadows();

    lua_pushboolean(L, ret);
    return 1;
}

int Primitive3D_Lua::EnableReceiveSimpleShadows(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);
    bool enable = CHECK_BOOLEAN(L, 2);

    prim->EnableReceiveSimpleShadows(enable);

    return 0;
}

int Primitive3D_Lua::ShouldReceiveSimpleShadows(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);

    bool ret = prim->ShouldReceiveSimpleShadows();

    lua_pushboolean(L, ret);
    return 1;
}

int Primitive3D_Lua::GetLightingChannels(lua_State* L)
{
    Primitive3D* comp = CHECK_PRIMITIVE_3D(L, 1);

    int32_t ret = (int32_t)comp->GetLightingChannels();

    lua_pushinteger(L, ret);
    return 1;
}

int Primitive3D_Lua::SetLightingChannels(lua_State* L)
{
    Primitive3D* comp = CHECK_PRIMITIVE_3D(L, 1);
    int32_t value = CHECK_INTEGER(L, 2);

    comp->SetLightingChannels((uint8_t)value);

    return 0;
}

int Primitive3D_Lua::SweepToWorldPosition(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_3D(L, 1);
    glm::vec3 pos = CHECK_VECTOR(L, 2);
    uint8_t mask = (lua_gettop(L) >= 3) ? (uint8_t)lua_tointeger(L, 3) : 0;

    SweepTestResult result;
    prim->SweepToWorldPosition(pos, result, mask);

    lua_newtable(L);
    Vector_Lua::Create(L, result.mStart);
    lua_setfield(L, -2, "start");
    Vector_Lua::Create(L, result.mEnd);
    lua_setfield(L, -2, "end");
    Node_Lua::Create(L, result.mHitNode);
    lua_setfield(L, -2, "hitNode");
    Vector_Lua::Create(L, result.mHitNormal);
    lua_setfield(L, -2, "hitNormal");
    Vector_Lua::Create(L, result.mHitPosition);
    lua_setfield(L, -2, "hitPosition");
    lua_pushnumber(L, result.mHitFraction);
    lua_setfield(L, -2, "hitFraction");
    return 1;
}


void Primitive3D_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        PRIMITIVE_3D_LUA_NAME,
        PRIMITIVE_3D_LUA_FLAG,
        NODE_3D_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, EnablePhysics);

    REGISTER_TABLE_FUNC(L, mtIndex, EnableCollision);

    REGISTER_TABLE_FUNC(L, mtIndex, EnableOverlaps);

    REGISTER_TABLE_FUNC(L, mtIndex, IsPhysicsEnabled);

    REGISTER_TABLE_FUNC(L, mtIndex, IsCollisionEnabled);

    REGISTER_TABLE_FUNC(L, mtIndex, AreOverlapsEnabled);

    REGISTER_TABLE_FUNC(L, mtIndex, GetMass);

    REGISTER_TABLE_FUNC(L, mtIndex, GetLinearDamping);

    REGISTER_TABLE_FUNC(L, mtIndex, GetAngularDamping);

    REGISTER_TABLE_FUNC(L, mtIndex, GetRestitution);

    REGISTER_TABLE_FUNC(L, mtIndex, GetFriction);

    REGISTER_TABLE_FUNC(L, mtIndex, GetRollingFriction);

    REGISTER_TABLE_FUNC(L, mtIndex, GetLinearFactor);

    REGISTER_TABLE_FUNC(L, mtIndex, GetAngularFactor);

    REGISTER_TABLE_FUNC(L, mtIndex, GetCollisionGroup);

    REGISTER_TABLE_FUNC(L, mtIndex, GetCollisionMask);

    REGISTER_TABLE_FUNC(L, mtIndex, SetMass);

    REGISTER_TABLE_FUNC(L, mtIndex, SetLinearDamping);

    REGISTER_TABLE_FUNC(L, mtIndex, SetAngularDamping);

    REGISTER_TABLE_FUNC(L, mtIndex, SetRestitution);

    REGISTER_TABLE_FUNC(L, mtIndex, SetFriction);

    REGISTER_TABLE_FUNC(L, mtIndex, SetRollingFriction);

    REGISTER_TABLE_FUNC(L, mtIndex, SetLinearFactor);

    REGISTER_TABLE_FUNC(L, mtIndex, SetAngularFactor);

    REGISTER_TABLE_FUNC(L, mtIndex, SetCollisionGroup);

    REGISTER_TABLE_FUNC(L, mtIndex, SetCollisionMask);

    REGISTER_TABLE_FUNC(L, mtIndex, GetLinearVelocity);

    REGISTER_TABLE_FUNC(L, mtIndex, GetAngularVelocity);

    REGISTER_TABLE_FUNC(L, mtIndex, AddLinearVelocity);

    REGISTER_TABLE_FUNC(L, mtIndex, AddAngularVelocity);

    REGISTER_TABLE_FUNC(L, mtIndex, SetLinearVelocity);

    REGISTER_TABLE_FUNC(L, mtIndex, SetAngularVelocity);

    REGISTER_TABLE_FUNC(L, mtIndex, AddForce);

    REGISTER_TABLE_FUNC(L, mtIndex, AddImpulse);

    REGISTER_TABLE_FUNC(L, mtIndex, ClearForces);

    REGISTER_TABLE_FUNC(L, mtIndex, EnableCastShadows);

    REGISTER_TABLE_FUNC(L, mtIndex, ShouldCastShadows);

    REGISTER_TABLE_FUNC(L, mtIndex, EnableReceiveShadows);

    REGISTER_TABLE_FUNC(L, mtIndex, ShouldReceiveShadows);

    REGISTER_TABLE_FUNC(L, mtIndex, EnableReceiveSimpleShadows);

    REGISTER_TABLE_FUNC(L, mtIndex, ShouldReceiveSimpleShadows);

    REGISTER_TABLE_FUNC(L, mtIndex, GetLightingChannels);

    REGISTER_TABLE_FUNC(L, mtIndex, SetLightingChannels);

    REGISTER_TABLE_FUNC(L, mtIndex, SweepToWorldPosition);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, SweepToWorldPosition, "SweepToPosition"); // Alias

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
