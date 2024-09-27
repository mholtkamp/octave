#pragma once

#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"

#include "Nodes/3D/Primitive3d.h"

#include "LuaBindings/Node_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define PRIMITIVE_3D_LUA_NAME "Primitive3D"
#define PRIMITIVE_3D_LUA_FLAG "cfPrimitive3D"
#define CHECK_PRIMITIVE_3D(L, arg) static_cast<Primitive3D*>(CheckNodeLuaType(L, arg, PRIMITIVE_3D_LUA_NAME, PRIMITIVE_3D_LUA_FLAG));

struct Primitive3D_Lua
{
    static int EnablePhysics(lua_State* L);
    static int EnableCollision(lua_State* L);
    static int EnableOverlaps(lua_State* L);
    static int IsPhysicsEnabled(lua_State* L);
    static int IsCollisionEnabled(lua_State* L);
    static int AreOverlapsEnabled(lua_State* L);

    static int GetMass(lua_State* L);
    static int GetLinearDamping(lua_State* L);
    static int GetAngularDamping(lua_State* L);
    static int GetRestitution(lua_State* L);
    static int GetFriction(lua_State* L);
    static int GetRollingFriction(lua_State* L);
    static int GetLinearFactor(lua_State* L);
    static int GetAngularFactor(lua_State* L);
    static int GetCollisionGroup(lua_State* L);
    static int GetCollisionMask(lua_State* L);

    static int SetMass(lua_State* L);
    static int SetLinearDamping(lua_State* L);
    static int SetAngularDamping(lua_State* L);
    static int SetRestitution(lua_State* L);
    static int SetFriction(lua_State* L);
    static int SetRollingFriction(lua_State* L);
    static int SetLinearFactor(lua_State* L);
    static int SetAngularFactor(lua_State* L);
    static int SetCollisionGroup(lua_State* L);
    static int SetCollisionMask(lua_State* L);

    static int GetLinearVelocity(lua_State* L);
    static int GetAngularVelocity(lua_State* L);

    static int AddLinearVelocity(lua_State* L);
    static int AddAngularVelocity(lua_State* L);
    static int SetLinearVelocity(lua_State* L);
    static int SetAngularVelocity(lua_State* L);
    static int AddForce(lua_State* L);
    static int AddImpulse(lua_State* L);
    static int ClearForces(lua_State* L);

    static int EnableCastShadows(lua_State* L);
    static int ShouldCastShadows(lua_State* L);

    static int EnableReceiveShadows(lua_State* L);
    static int ShouldReceiveShadows(lua_State* L);

    static int EnableReceiveSimpleShadows(lua_State* L);
    static int ShouldReceiveSimpleShadows(lua_State* L);

    static int GetLightingChannels(lua_State* L);
    static int SetLightingChannels(lua_State* L);

    static int SweepToWorldPosition(lua_State* L);

    static void Bind();
};

#endif
