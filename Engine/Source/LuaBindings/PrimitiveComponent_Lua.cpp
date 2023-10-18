#include "LuaBindings/Primitive3D_Lua.h"
#include "LuaBindings/Node3D_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

int Primitive3D_Lua::EnablePhysics(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);
    bool enable = CHECK_BOOLEAN(L, 2);

    prim->EnablePhysics(enable);

    return 0;
}

int Primitive3D_Lua::EnableCollision(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);
    bool enable = CHECK_BOOLEAN(L, 2);

    prim->EnableCollision(enable);

    return 0;
}

int Primitive3D_Lua::EnableOverlaps(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);
    bool enable = CHECK_BOOLEAN(L, 2);

    prim->EnableOverlaps(enable);

    return 0;
}

int Primitive3D_Lua::IsPhysicsEnabled(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);

    bool enabled = prim->IsPhysicsEnabled();

    lua_pushboolean(L, enabled);
    return 1;
}

int Primitive3D_Lua::IsCollisionEnabled(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);

    bool enabled = prim->IsCollisionEnabled();

    lua_pushboolean(L, enabled);
    return 1;
}

int Primitive3D_Lua::AreOverlapsEnabled(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);

    bool enabled = prim->AreOverlapsEnabled();

    lua_pushboolean(L, enabled);
    return 1;
}

int Primitive3D_Lua::GetMass(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);

    float ret = prim->GetMass();

    lua_pushnumber(L, ret);
    return 1;
}

int Primitive3D_Lua::GetLinearDamping(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);

    float ret = prim->GetLinearDamping();

    lua_pushnumber(L, ret);
    return 1;
}

int Primitive3D_Lua::GetAngularDamping(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);

    float ret = prim->GetAngularDamping();

    lua_pushnumber(L, ret);
    return 1;
}

int Primitive3D_Lua::GetRestitution(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);

    float ret = prim->GetRestitution();

    lua_pushnumber(L, ret);
    return 1;
}

int Primitive3D_Lua::GetFriction(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);

    float ret = prim->GetFriction();

    lua_pushnumber(L, ret);
    return 1;
}

int Primitive3D_Lua::GetRollingFriction(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);

    float ret = prim->GetRollingFriction();

    lua_pushnumber(L, ret);
    return 1;
}

int Primitive3D_Lua::GetLinearFactor(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);

    glm::vec3 ret = prim->GetLinearFactor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Primitive3D_Lua::GetAngularFactor(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);

    glm::vec3 ret = prim->GetAngularFactor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Primitive3D_Lua::GetCollisionGroup(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);

    int ret = prim->GetCollisionGroup();

    lua_pushinteger(L, ret);
    return 1;
}

int Primitive3D_Lua::GetCollisionMask(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);

    int ret = prim->GetCollisionMask();

    lua_pushinteger(L, ret);
    return 1;
}

int Primitive3D_Lua::SetMass(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);
    float value = CHECK_NUMBER(L, 2);

    prim->SetMass(value);

    return 0;
}

int Primitive3D_Lua::SetLinearDamping(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);
    float value = CHECK_NUMBER(L, 2);

    prim->SetLinearDamping(value);

    return 0;
}

int Primitive3D_Lua::SetAngularDamping(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);
    float value = CHECK_NUMBER(L, 2);

    prim->SetAngularDamping(value);

    return 0;
}

int Primitive3D_Lua::SetRestitution(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);
    float value = CHECK_NUMBER(L, 2);

    prim->SetRestitution(value);

    return 0;
}

int Primitive3D_Lua::SetFriction(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);
    float value = CHECK_NUMBER(L, 2);

    prim->SetFriction(value);

    return 0;
}

int Primitive3D_Lua::SetRollingFriction(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);
    float value = CHECK_NUMBER(L, 2);

    prim->SetRollingFriction(value);

    return 0;
}

int Primitive3D_Lua::SetLinearFactor(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);
    glm::vec3 value = CHECK_VECTOR(L, 2);

    prim->SetLinearFactor(value);

    return 0;
}

int Primitive3D_Lua::SetAngularFactor(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);
    glm::vec3 value = CHECK_VECTOR(L, 2);

    prim->SetAngularFactor(value);

    return 0;
}

int Primitive3D_Lua::SetCollisionGroup(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);
    uint8_t value = (uint8_t) CHECK_INTEGER(L, 2);

    prim->SetCollisionGroup(value);

    return 0;
}

int Primitive3D_Lua::SetCollisionMask(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);
    uint8_t value = (uint8_t)CHECK_INTEGER(L, 2);

    prim->SetCollisionMask(value);

    return 0;
}

int Primitive3D_Lua::GetLinearVelocity(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);

    glm::vec3 ret = prim->GetLinearVelocity();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Primitive3D_Lua::GetAngularVelocity(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);

    glm::vec3 ret = prim->GetAngularVelocity();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Primitive3D_Lua::AddLinearVelocity(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);
    glm::vec3 delta = CHECK_VECTOR(L, 2);

    prim->AddLinearVelocity(delta);

    return 0;
}

int Primitive3D_Lua::AddAngularVelocity(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);
    glm::vec3 delta = CHECK_VECTOR(L, 2);

    prim->AddAngularVelocity(delta);

    return 0;
}

int Primitive3D_Lua::SetLinearVelocity(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);
    glm::vec3 value = CHECK_VECTOR(L, 2);

    prim->SetLinearVelocity(value);

    return 0;
}

int Primitive3D_Lua::SetAngularVelocity(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);
    glm::vec3 value = CHECK_VECTOR(L, 2);

    prim->SetAngularVelocity(value);

    return 0;
}

int Primitive3D_Lua::AddForce(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);
    glm::vec3 delta = CHECK_VECTOR(L, 2);

    prim->AddForce(delta);

    return 0;
}

int Primitive3D_Lua::AddImpulse(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);
    glm::vec3 delta = CHECK_VECTOR(L, 2);

    prim->AddImpulse(delta);

    return 0;
}

int Primitive3D_Lua::ClearForces(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);

    prim->ClearForces();

    return 0;
}

int Primitive3D_Lua::EnableCastShadows(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);
    bool enable = CHECK_BOOLEAN(L, 2);

    prim->EnableCastShadows(enable);

    return 0;
}

int Primitive3D_Lua::ShouldCastShadows(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);

    bool ret = prim->ShouldCastShadows();

    lua_pushboolean(L, ret);
    return 1;
}

int Primitive3D_Lua::EnableReceiveShadows(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);
    bool enable = CHECK_BOOLEAN(L, 2);

    prim->EnableReceiveShadows(enable);

    return 0;
}

int Primitive3D_Lua::ShouldReceiveShadows(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);

    bool ret = prim->ShouldReceiveShadows();

    lua_pushboolean(L, ret);
    return 1;
}

int Primitive3D_Lua::EnableReceiveSimpleShadows(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);
    bool enable = CHECK_BOOLEAN(L, 2);

    prim->EnableReceiveSimpleShadows(enable);

    return 0;
}

int Primitive3D_Lua::ShouldReceiveSimpleShadows(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);

    bool ret = prim->ShouldReceiveSimpleShadows();

    lua_pushboolean(L, ret);
    return 1;
}

int Primitive3D_Lua::SweepToWorldPosition(lua_State* L)
{
    Primitive3D* prim = CHECK_PRIMITIVE_COMPONENT(L, 1);
    glm::vec3 pos = CHECK_VECTOR(L, 2);
    uint8_t mask = (lua_gettop(L) >= 3) ? (uint8_t)lua_tointeger(L, 3) : 0;

    SweepTestResult result;
    prim->SweepToWorldPosition(pos, result, mask);

    lua_newtable(L);
    Vector_Lua::Create(L, result.mStart);
    lua_setfield(L, -2, "start");
    Vector_Lua::Create(L, result.mEnd);
    lua_setfield(L, -2, "end");
    Component_Lua::Create(L, result.mHitComponent);
    lua_setfield(L, -2, "hitComponent");
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
        PRIMITIVE_COMPONENT_LUA_NAME,
        PRIMITIVE_COMPONENT_LUA_FLAG,
        TRANSFORM_COMPONENT_LUA_NAME);

    Component_Lua::BindCommon(L, mtIndex);

    lua_pushcfunction(L, EnablePhysics);
    lua_setfield(L, mtIndex, "EnablePhysics");

    lua_pushcfunction(L, EnableCollision);
    lua_setfield(L, mtIndex, "EnableCollision");

    lua_pushcfunction(L, EnableOverlaps);
    lua_setfield(L, mtIndex, "EnableOverlaps");

    lua_pushcfunction(L, IsPhysicsEnabled);
    lua_setfield(L, mtIndex, "IsPhysicsEnabled");

    lua_pushcfunction(L, IsCollisionEnabled);
    lua_setfield(L, mtIndex, "IsCollisionEnabled");

    lua_pushcfunction(L, AreOverlapsEnabled);
    lua_setfield(L, mtIndex, "AreOverlapsEnabled");

    lua_pushcfunction(L, GetMass);
    lua_setfield(L, mtIndex, "GetMass");

    lua_pushcfunction(L, GetLinearDamping);
    lua_setfield(L, mtIndex, "GetLinearDamping");

    lua_pushcfunction(L, GetAngularDamping);
    lua_setfield(L, mtIndex, "GetAngularDamping");

    lua_pushcfunction(L, GetRestitution);
    lua_setfield(L, mtIndex, "GetRestitution");

    lua_pushcfunction(L, GetFriction);
    lua_setfield(L, mtIndex, "GetFriction");

    lua_pushcfunction(L, GetRollingFriction);
    lua_setfield(L, mtIndex, "GetRollingFriction");

    lua_pushcfunction(L, GetLinearFactor);
    lua_setfield(L, mtIndex, "GetLinearFactor");

    lua_pushcfunction(L, GetAngularFactor);
    lua_setfield(L, mtIndex, "GetAngularFactor");

    lua_pushcfunction(L, GetCollisionGroup);
    lua_setfield(L, mtIndex, "GetCollisionGroup");

    lua_pushcfunction(L, GetCollisionMask);
    lua_setfield(L, mtIndex, "GetCollisionMask");

    lua_pushcfunction(L, SetMass);
    lua_setfield(L, mtIndex, "SetMass");

    lua_pushcfunction(L, SetLinearDamping);
    lua_setfield(L, mtIndex, "SetLinearDamping");

    lua_pushcfunction(L, SetAngularDamping);
    lua_setfield(L, mtIndex, "SetAngularDamping");

    lua_pushcfunction(L, SetRestitution);
    lua_setfield(L, mtIndex, "SetRestitution");

    lua_pushcfunction(L, SetFriction);
    lua_setfield(L, mtIndex, "SetFriction");

    lua_pushcfunction(L, SetRollingFriction);
    lua_setfield(L, mtIndex, "SetRollingFriction");

    lua_pushcfunction(L, SetLinearFactor);
    lua_setfield(L, mtIndex, "SetLinearFactor");

    lua_pushcfunction(L, SetAngularFactor);
    lua_setfield(L, mtIndex, "SetAngularFactor");

    lua_pushcfunction(L, SetCollisionGroup);
    lua_setfield(L, mtIndex, "SetCollisionGroup");

    lua_pushcfunction(L, SetCollisionMask);
    lua_setfield(L, mtIndex, "SetCollisionMask");

    lua_pushcfunction(L, GetLinearVelocity);
    lua_setfield(L, mtIndex, "GetLinearVelocity");

    lua_pushcfunction(L, GetAngularVelocity);
    lua_setfield(L, mtIndex, "GetAngularVelocity");

    lua_pushcfunction(L, AddLinearVelocity);
    lua_setfield(L, mtIndex, "AddLinearVelocity");

    lua_pushcfunction(L, AddAngularVelocity);
    lua_setfield(L, mtIndex, "AddAngularVelocity");

    lua_pushcfunction(L, SetLinearVelocity);
    lua_setfield(L, mtIndex, "SetLinearVelocity");

    lua_pushcfunction(L, SetAngularVelocity);
    lua_setfield(L, mtIndex, "SetAngularVelocity");

    lua_pushcfunction(L, AddForce);
    lua_setfield(L, mtIndex, "AddForce");

    lua_pushcfunction(L, AddImpulse);
    lua_setfield(L, mtIndex, "AddImpulse");

    lua_pushcfunction(L, ClearForces);
    lua_setfield(L, mtIndex, "ClearForces");

    lua_pushcfunction(L, EnableCastShadows);
    lua_setfield(L, mtIndex, "EnableCastShadows");

    lua_pushcfunction(L, ShouldCastShadows);
    lua_setfield(L, mtIndex, "ShouldCastShadows");

    lua_pushcfunction(L, EnableReceiveShadows);
    lua_setfield(L, mtIndex, "EnableReceiveShadows");

    lua_pushcfunction(L, ShouldReceiveShadows);
    lua_setfield(L, mtIndex, "ShouldReceiveShadows");

    lua_pushcfunction(L, EnableReceiveSimpleShadows);
    lua_setfield(L, mtIndex, "EnableReceiveSimpleShadows");

    lua_pushcfunction(L, ShouldReceiveSimpleShadows);
    lua_setfield(L, mtIndex, "ShouldReceiveSimpleShadows");

    lua_pushcfunction(L, SweepToWorldPosition);
    lua_setfield(L, mtIndex, "SweepToWorldPosition");
    lua_pushcfunction(L, SweepToWorldPosition);
    lua_setfield(L, mtIndex, "SweepToPosition"); // Alias

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
