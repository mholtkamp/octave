#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"

#include "Actor.h"
#include "Components/Component.h"
#include "Components/PrimitiveComponent.h"
#include "Components/ScriptComponent.h"

#include "LuaBindings/LuaUtils.h"
#include "LuaBindings/Actor_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Component_Lua.h"
#include "LuaBindings/TransformComponent_Lua.h"

#if LUA_ENABLED

int Actor_Lua::Create(lua_State* L, Actor* actor)
{
    if (actor != nullptr)
    {
        Actor_Lua* actorLua = (Actor_Lua*)lua_newuserdata(L, sizeof(Actor_Lua));
        new (actorLua) Actor_Lua();
        actorLua->mActor = actor;

        int udIndex = lua_gettop(L);

        luaL_getmetatable(L, actor->GetClassName());
        if (lua_isnil(L, -1))
        {
            // Could not find this type's metatable, so just use Component
            lua_pop(L, 1);
            luaL_getmetatable(L, ACTOR_LUA_NAME);
        }

        assert(lua_istable(L, -1));
        lua_setmetatable(L, udIndex);

        assert(lua_gettop(L) == udIndex);
    }
    else
    {
        lua_pushnil(L);
    }

    return 1;
}

int Actor_Lua::IsValid(lua_State* L)
{
#if LUA_SAFE_ACTOR
    Actor_Lua* luaObj = static_cast<Actor_Lua*>(CheckHierarchyLuaType<Actor_Lua>(L, 1, ACTOR_LUA_NAME, ACTOR_LUA_FLAG));

    bool ret = (luaObj->mActor.Get() != nullptr);

    lua_pushboolean(L, ret);
    return 1;
#else
    lua_pushboolean(L, true);
    return 1;
#endif
}

int Actor_Lua::CreateComponent(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);
    const char* compClass = CHECK_STRING(L, 2);

    Component* retComp = actor->CreateComponent(compClass);

    Component_Lua::Create(L, retComp);
    return 1;
}

int Actor_Lua::CloneComponent(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);
    Component* srcComp = CHECK_COMPONENT(L, 2);

    Component* retComp = actor->CloneComponent(srcComp);

    Component_Lua::Create(L, retComp);
    return 1;

}

int Actor_Lua::DestroyComponent(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);
    Component* comp = CHECK_COMPONENT(L, 2);

    actor->DestroyComponent(comp);

    return 0;
}

int Actor_Lua::GetName(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);

    const std::string& name = actor->GetName();

    lua_pushstring(L, name.c_str());
    return 1;
}

int Actor_Lua::SetName(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);
    const char* name = CHECK_STRING(L, 2);

    actor->SetName(name);

    return 0;
}

int Actor_Lua::Attach(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);
    luaL_checktype(L, 2, LUA_TUSERDATA);

    if (CheckClassFlag(L, 2, ACTOR_LUA_FLAG))
    {
        Actor* otherActor = CHECK_ACTOR(L, 2);
        actor->Attach(otherActor);
    }
    else
    {
        TransformComponent* comp = CHECK_TRANSFORM_COMPONENT(L, 2);
        actor->Attach(comp);
    }

    return 0;
}

int Actor_Lua::SetPendingDestroy(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);
    bool destroy = CHECK_BOOLEAN(L, 2);

    actor->SetPendingDestroy(destroy);

    return 0;
}

int Actor_Lua::IsPendingDestroy(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);

    bool destroy = actor->IsPendingDestroy();

    lua_pushboolean(L, destroy);
    return 1;
}

int Actor_Lua::EnableTick(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);
    bool enable = CHECK_BOOLEAN(L, 2);

    actor->EnableTick(enable);

    return 0;
}

int Actor_Lua::IsTickEnabled(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);

    bool tickEnabled = actor->IsTickEnabled();

    lua_pushboolean(L, tickEnabled);
    return 1;
}

int Actor_Lua::GetPosition(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);

    glm::vec3 pos = actor->GetPosition();

    Vector_Lua::Create(L, pos);
    return 1;
}

int Actor_Lua::GetRotationQuat(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);

    glm::quat rotQuat = actor->GetRotationQuat();

    Vector_Lua::Create(L, QuatToVector(rotQuat));
    return 1;
}

int Actor_Lua::GetRotationEuler(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);

    glm::vec3 rotEuler = actor->GetRotationEuler();

    Vector_Lua::Create(L, rotEuler);
    return 1;
}

int Actor_Lua::GetScale(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);

    glm::vec3 scale = actor->GetScale();

    Vector_Lua::Create(L, scale);
    return 1;
}

int Actor_Lua::SetPosition(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);
    glm::vec3 pos = CHECK_VECTOR(L, 2);

    actor->SetPosition(pos);

    return 0;
}

int Actor_Lua::SetRotationQuat(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);
    glm::vec4 rotVec = CHECK_VECTOR(L, 2);

    glm::quat rotQuat = VectorToQuat(rotVec);
    actor->SetRotation(rotQuat);

    return 0;
}

int Actor_Lua::SetRotationEuler(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);
    glm::vec3 rotEuler = CHECK_VECTOR(L, 2);

    actor->SetRotation(rotEuler);

    return 0;
}

int Actor_Lua::SetScale(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);
    glm::vec3 scale = CHECK_VECTOR(L, 2);

    actor->SetScale(scale);

    return 0;
}

int Actor_Lua::GetForwardVector(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);

    glm::vec3 fwd = actor->GetForwardVector();

    Vector_Lua::Create(L, fwd);
    return 1;
}

int Actor_Lua::GetRightVector(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);

    glm::vec3 right = actor->GetRightVector();

    Vector_Lua::Create(L, right);
    return 1;
}

int Actor_Lua::GetUpVector(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);

    glm::vec3 up = actor->GetUpVector();

    Vector_Lua::Create(L, up);
    return 1;
}

int Actor_Lua::SweepToPosition(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);
    glm::vec3 pos = CHECK_VECTOR(L, 2);
    uint8_t mask = (lua_gettop(L) >= 3) ? (uint8_t)lua_tointeger(L, 3) : 0;

    SweepTestResult result;
    actor->SweepToPosition(pos, result, mask);

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

int Actor_Lua::GetNetId(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);

    NetId netId = actor->GetNetId();

    lua_pushinteger(L, (int)netId);
    return 1;
}

int Actor_Lua::GetOwningHost(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);

    NetHostId netHostId = actor->GetOwningHost();

    lua_pushinteger(L, (int)netHostId);
    return 1;
}

int Actor_Lua::SetOwningHost(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);
    int netHostId = CHECK_INTEGER(L, 2);

    actor->SetOwningHost(netHostId);

    return 0;
}

int Actor_Lua::SetReplicate(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);
    bool replicate = CHECK_BOOLEAN(L, 2);

    actor->SetReplicate(replicate);

    return 0;
}

int Actor_Lua::IsReplicated(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);

    bool replicated = actor->IsReplicated();

    lua_pushboolean(L, replicated);
    return 1;
}

int Actor_Lua::ForceReplication(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);

    actor->ForceReplication();

    return 0;
}

int Actor_Lua::GetComponent(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);
    assert(lua_isstring(L, 2) || lua_isnumber(L, 2));

    Component* targetComp = nullptr;

    if (lua_isinteger(L, 2))
    {
        int32_t index = int32_t(lua_tointeger(L, 2) - 1);
        targetComp = actor->GetComponent(index);
    }
    else if (lua_isstring(L, 2))
    {
        const char* compName = lua_tostring(L, 2);
        targetComp = actor->GetComponent(compName);
    }

    Component_Lua::Create(L, targetComp);
    return 1;
}

int Actor_Lua::GetNumComponents(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);

    uint32_t numComps = actor->GetNumComponents();

    lua_pushinteger(L, numComps);
    return 1;
}

int Actor_Lua::AddComponent(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);
    Component* comp = CHECK_COMPONENT(L, 2);

    actor->AddComponent(comp);

    return 0;
}

int Actor_Lua::RemoveComponent(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);
    Component* comp = CHECK_COMPONENT(L, 2);

    actor->RemoveComponent(comp);

    return 0;
}

int Actor_Lua::SetRootComponent(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);
    TransformComponent* comp = CHECK_TRANSFORM_COMPONENT(L, 2);

    actor->SetRootComponent(comp);

    return 0;
}

int Actor_Lua::GetRootComponent(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);

    TransformComponent* rootComp = actor->GetRootComponent();

    Component_Lua::Create(L, rootComp);
    return 1;
}

int Actor_Lua::UpdateComponentTransforms(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);

    actor->UpdateComponentTransforms();

    return 0;
}

int Actor_Lua::GetScript(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);
    const char* name = CHECK_STRING(L, 2);
    bool found = false;

    const std::vector<Component*>& comps = actor->GetComponents();
    for (uint32_t i = 0; i < comps.size(); ++i)
    {
        if (comps[i]->GetType() == ScriptComponent::GetStaticType())
        {
            ScriptComponent* sc = static_cast<ScriptComponent*>(comps[i]);

            if (sc->GetScriptClassName() == name)
            {
                lua_getglobal(L, sc->GetTableName().c_str());
                if (lua_istable(L, -1))
                {
                    found = true;
                    break;
                }
                else
                {
                    // Should not happen? The component has a table registered
                    // but it doesn't exist in the lua state.
                    assert(0);
                    lua_pop(L, 1);
                }
            }
        }
    }

    if (!found)
    {
        lua_pushnil(L);
    }

    return 1;
}

int Actor_Lua::HasTag(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);
    const char* tag = CHECK_STRING(L, 2);

    bool ret = actor->HasTag(tag);

    lua_pushboolean(L, ret);
    return 1;
}

int Actor_Lua::AddTag(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);
    const char* tag = CHECK_STRING(L, 2);

    actor->AddTag(tag);

    return 0;
}

int Actor_Lua::RemoveTag(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);
    const char* tag = CHECK_STRING(L, 2);

    actor->RemoveTag(tag);

    return 0;
}

void Actor_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        ACTOR_LUA_NAME,
        ACTOR_LUA_FLAG, 
        nullptr);

    lua_pushcfunction(L, Actor_Lua::CreateComponent);
    lua_setfield(L, mtIndex, "CreateComponent");

    lua_pushcfunction(L, Actor_Lua::IsValid);
    lua_setfield(L, mtIndex, "IsValid");

    lua_pushcfunction(L, Actor_Lua::CloneComponent);
    lua_setfield(L, mtIndex, "CloneComponent");

    lua_pushcfunction(L, Actor_Lua::DestroyComponent);
    lua_setfield(L, mtIndex, "DestroyComponent");

    lua_pushcfunction(L, Actor_Lua::GetName);
    lua_setfield(L, mtIndex, "GetName");

    lua_pushcfunction(L, Actor_Lua::SetName);
    lua_setfield(L, mtIndex, "SetName");

    lua_pushcfunction(L, Actor_Lua::Attach);
    lua_setfield(L, mtIndex, "Attach");

    lua_pushcfunction(L, Actor_Lua::SetPendingDestroy);
    lua_setfield(L, mtIndex, "SetPendingDestroy");

    lua_pushcfunction(L, Actor_Lua::IsPendingDestroy);
    lua_setfield(L, mtIndex, "IsPendingDestroy");

    lua_pushcfunction(L, Actor_Lua::EnableTick);
    lua_setfield(L, mtIndex, "EnableTick");

    lua_pushcfunction(L, Actor_Lua::IsTickEnabled);
    lua_setfield(L, mtIndex, "IsTickEnabled");

    lua_pushcfunction(L, Actor_Lua::GetPosition);
    lua_setfield(L, mtIndex, "GetPosition");

    lua_pushcfunction(L, Actor_Lua::GetRotationQuat);
    lua_setfield(L, mtIndex, "GetRotationQuat");

    lua_pushcfunction(L, Actor_Lua::GetRotationEuler);
    lua_setfield(L, mtIndex, "GetRotationEuler");

    lua_pushcfunction(L, Actor_Lua::GetScale);
    lua_setfield(L, mtIndex, "GetScale");

    lua_pushcfunction(L, Actor_Lua::SetPosition);
    lua_setfield(L, mtIndex, "SetPosition");

    lua_pushcfunction(L, Actor_Lua::SetRotationQuat);
    lua_setfield(L, mtIndex, "SetRotationQuat");

    lua_pushcfunction(L, Actor_Lua::SetRotationEuler);
    lua_pushcfunction(L, Actor_Lua::SetRotationEuler);
    lua_setfield(L, mtIndex, "SetRotationEuler");
    lua_setfield(L, mtIndex, "SetRotation");

    lua_pushcfunction(L, Actor_Lua::SetScale);
    lua_setfield(L, mtIndex, "SetScale");

    lua_pushcfunction(L, Actor_Lua::GetForwardVector);
    lua_setfield(L, mtIndex, "GetForwardVector");

    lua_pushcfunction(L, Actor_Lua::GetRightVector);
    lua_setfield(L, mtIndex, "GetRightVector");

    lua_pushcfunction(L, Actor_Lua::GetUpVector);
    lua_setfield(L, mtIndex, "GetUpVector");

    lua_pushcfunction(L, Actor_Lua::SweepToPosition);
    lua_setfield(L, mtIndex, "SweepToPosition");

    lua_pushcfunction(L, Actor_Lua::GetNetId);
    lua_setfield(L, mtIndex, "GetNetId");

    lua_pushcfunction(L, Actor_Lua::GetOwningHost);
    lua_setfield(L, mtIndex, "GetOwningHost");

    lua_pushcfunction(L, Actor_Lua::SetOwningHost);
    lua_setfield(L, mtIndex, "SetOwningHost");

    lua_pushcfunction(L, Actor_Lua::SetReplicate);
    lua_setfield(L, mtIndex, "SetReplicate");

    lua_pushcfunction(L, Actor_Lua::IsReplicated);
    lua_setfield(L, mtIndex, "IsReplicated");

    lua_pushcfunction(L, Actor_Lua::ForceReplication);
    lua_setfield(L, mtIndex, "ForceReplication");

    lua_pushcfunction(L, Actor_Lua::GetComponent);
    lua_setfield(L, mtIndex, "GetComponent");

    lua_pushcfunction(L, Actor_Lua::GetNumComponents);
    lua_setfield(L, mtIndex, "GetNumComponents");

    lua_pushcfunction(L, Actor_Lua::AddComponent);
    lua_setfield(L, mtIndex, "AddComponent");

    lua_pushcfunction(L, Actor_Lua::RemoveComponent);
    lua_setfield(L, mtIndex, "RemoveComponent");

    lua_pushcfunction(L, Actor_Lua::SetRootComponent);
    lua_setfield(L, mtIndex, "SetRootComponent");

    lua_pushcfunction(L, Actor_Lua::GetRootComponent);
    lua_setfield(L, mtIndex, "GetRootComponent");

    lua_pushcfunction(L, Actor_Lua::UpdateComponentTransforms);
    lua_setfield(L, mtIndex, "UpdateComponentTransforms");

    lua_pushcfunction(L, Actor_Lua::GetScript);
    lua_setfield(L, mtIndex, "GetScript");

    lua_pushcfunction(L, Actor_Lua::HasTag);
    lua_setfield(L, mtIndex, "HasTag");

    lua_pushcfunction(L, Actor_Lua::AddTag);
    lua_setfield(L, mtIndex, "AddTag");

    lua_pushcfunction(L, Actor_Lua::RemoveTag);
    lua_setfield(L, mtIndex, "RemoveTag");

    lua_pop(L, 1);
    assert(lua_gettop(L) == 0);
}

#endif