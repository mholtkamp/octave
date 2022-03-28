
#include "LuaBindings/World_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Actor_Lua.h"
#include "LuaBindings/Component_Lua.h"
#include "LuaBindings/TransformComponent_Lua.h"
#include "LuaBindings/PrimitiveComponent_Lua.h"
#include "LuaBindings/CameraComponent_Lua.h"
#include "LuaBindings/DirectionalLightComponent_Lua.h"

#include "Components/PrimitiveComponent.h"

#if LUA_ENABLED

int World_Lua::Create(lua_State* L, World* world)
{
    if (world != nullptr)
    {
        World_Lua* worldLua = (World_Lua*)lua_newuserdata(L, sizeof(World_Lua));
        worldLua->mWorld = world;

        int udIndex = lua_gettop(L);
        luaL_getmetatable(L, WORLD_LUA_NAME);
        assert(lua_istable(L, -1));
        lua_setmetatable(L, udIndex);
    }
    else
    {
        lua_pushnil(L);
    }

    return 1;
}

int World_Lua::GetActiveCamera(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);
    
    CameraComponent* cameraComp = world->GetActiveCamera();

    Component_Lua::Create(L, cameraComp);
    return 1;
}

int World_Lua::GetAudioReceiver(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);

    TransformComponent* receiverComp = world->GetAudioReceiver();

    Component_Lua::Create(L, receiverComp);
    return 1;
}

int World_Lua::GetDirectionalLight(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);

    DirectionalLightComponent* dirLightComp = world->GetDirectionalLight();

    Component_Lua::Create(L, dirLightComp);
    return 1;
}

int World_Lua::SetActiveCamera(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);
    CameraComponent* cameraComp = CHECK_CAMERA_COMPONENT(L, 2);

    world->SetActiveCamera(cameraComp);

    return 0;
}

int World_Lua::SetAudioReceiver(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);
    TransformComponent* transformComp = nullptr;
    if (!lua_isnil(L, 2))
    {
        transformComp = CHECK_TRANSFORM_COMPONENT(L, 2);
    }

    world->SetAudioReceiver(transformComp);

    return 0;
}

int World_Lua::SetDirectionalLight(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);
    DirectionalLightComponent* lightComp = nullptr;
    if (!lua_isnil(L, 2))
    {
        lightComp = CHECK_DIRECTIONAL_LIGHT_COMPONENT(L, 2);
    }

    world->SetDirectionalLight(lightComp);
 
    return 0;
}

int World_Lua::SpawnActor(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);
    const char* actorClass = "Actor";
    if (lua_gettop(L) >= 2 &&
        lua_isstring(L, 2))
    {
        actorClass = lua_tostring(L, 2);
    }

    Actor* spawnedActor = world->SpawnActor(actorClass);

    Actor_Lua::Create(L, spawnedActor);
    return 1;
}

int World_Lua::CloneActor(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);
    Actor* srcActor = CHECK_ACTOR(L, 2);

    Actor* clonedActor = world->CloneActor(srcActor);

    Actor_Lua::Create(L, clonedActor);
    return 1;
}

int World_Lua::DestroyActor(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);
    Actor* actor = CHECK_ACTOR(L, 2);

    world->DestroyActor(actor);
    
    return 0;
}

int World_Lua::DestroyAllActors(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);

    world->DestroyAllActors();

    return 0;
}

int World_Lua::FindActor(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);
    const char* name = CHECK_STRING(L, 2);

    Actor* actor = world->FindActor(name);

    Actor_Lua::Create(L, actor);
    return 1;
}

int World_Lua::FindComponent(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);
    const char* name = CHECK_STRING(L, 2);

    Component* comp = world->FindComponent(name);

    Component_Lua::Create(L, comp);
    return 1;
}

int World_Lua::PrioritizeActorTick(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);
    Actor* actor = CHECK_ACTOR(L, 2);

    world->PrioritizeActorTick(actor);

    return 0;
}

int World_Lua::SetAmbientLightColor(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);
    glm::vec4& color = CHECK_VECTOR(L, 2);

    world->SetAmbientLightColor(color);

    return 0;
}

int World_Lua::GetAmbientLightColor(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);

    glm::vec4 color = world->GetAmbientLightColor();

    Vector_Lua::Create(L, color);
    return 1;
}

int World_Lua::SetShadowColor(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);
    glm::vec4& color = CHECK_VECTOR(L, 2);

    world->SetShadowColor(color);

    return 0;
}

int World_Lua::GetShadowColor(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);

    glm::vec4 color = world->GetShadowColor();

    Vector_Lua::Create(L, color);
    return 1;
}

int World_Lua::SetFogSettings(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);
    bool enabled = CHECK_BOOLEAN(L, 2);
    glm::vec4 color = CHECK_VECTOR(L, 3);
    bool exponential = CHECK_BOOLEAN(L, 4);
    float zNear = CHECK_NUMBER(L, 5);
    float zFar = CHECK_NUMBER(L, 6);

    FogSettings settings;
    settings.mEnabled = enabled;
    settings.mColor = color;
    settings.mDensityFunc = exponential ? FogDensityFunc::Exponential : FogDensityFunc::Linear;
    settings.mNear = zNear;
    settings.mFar = zFar;
    world->SetFogSettings(settings);

    return 0;
}

int World_Lua::GetFogSettings(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);

    const FogSettings& settings = world->GetFogSettings();

    // Should we convert this into a table?
    lua_pushboolean(L, settings.mEnabled);
    Vector_Lua::Create(L, settings.mColor);
    lua_pushboolean(L, settings.mDensityFunc == FogDensityFunc::Exponential);
    lua_pushnumber(L, settings.mNear);
    lua_pushnumber(L, settings.mFar);

    return 5;
}

int World_Lua::SetGravity(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);
    glm::vec3 value = CHECK_VECTOR(L, 2);

    world->SetGravity(value);

    return 0;
}

int World_Lua::GetGravity(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);

    glm::vec3 ret = world->GetGravity();

    Vector_Lua::Create(L, ret);
    return 1;
}

int World_Lua::RayTest(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);
    glm::vec3 start = CHECK_VECTOR(L, 2);
    glm::vec3 end = CHECK_VECTOR(L, 3);
    uint8_t colMask = (uint8_t) CHECK_INTEGER(L, 4);

    RayTestResult result;
    world->RayTest(start, end, colMask, result);

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

int World_Lua::RayTestMulti(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);
    glm::vec3 start = CHECK_VECTOR(L, 2);
    glm::vec3 end = CHECK_VECTOR(L, 3);
    uint8_t colMask = (uint8_t)CHECK_INTEGER(L, 4);

    RayTestMultiResult result;
    world->RayTestMulti(start, end, colMask, result);

    // Setup Top Table
    lua_newtable(L);
    int topTableIdx = lua_gettop(L);
    Vector_Lua::Create(L, result.mStart);
    lua_setfield(L, topTableIdx, "start");
    Vector_Lua::Create(L, result.mEnd);
    lua_setfield(L, topTableIdx, "end");
    lua_pushinteger(L, (int) result.mNumHits);
    lua_setfield(L, topTableIdx, "numHits");
    lua_newtable(L);
    int hitListIdx = lua_gettop(L);
    lua_pushvalue(L, hitListIdx);
    lua_setfield(L, topTableIdx, "hits");

    // Setup each hit in the hits table
    for (uint32_t i = 0; i < result.mNumHits; ++i)
    {
        lua_newtable(L);
        int hitTableIdx = lua_gettop(L);
        Component_Lua::Create(L, result.mHitComponents[i]);
        lua_setfield(L, hitTableIdx, "component");
        Vector_Lua::Create(L, result.mHitNormals[i]);
        lua_setfield(L, hitTableIdx, "normal");
        Vector_Lua::Create(L, result.mHitPositions[i]);
        lua_setfield(L, hitTableIdx, "position");
        lua_pushnumber(L, result.mHitFractions[i]);
        lua_setfield(L, hitTableIdx, "fraction");

        lua_pushinteger(L, (int)i);
        lua_pushvalue(L, hitTableIdx);
        lua_settable(L, hitListIdx);

        // the hit instance table is still on the stack.
        lua_pop(L, 1);
    }

    // the hit list table is still on the stack
    lua_pop(L, 1);

    // The top table should be on the top of the stack
    assert(lua_gettop(L) == 5);
    return 1;
}

int World_Lua::SweepTest(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);
    PrimitiveComponent* primComp = CHECK_PRIMITIVE_COMPONENT(L, 2);
    glm::vec3 start = CHECK_VECTOR(L, 3);
    glm::vec3 end = CHECK_VECTOR(L, 4);
    uint8_t colMask = (uint8_t)CHECK_INTEGER(L, 5);

    SweepTestResult result;
    world->SweepTest(primComp, start, end, colMask, result);

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

int World_Lua::GetLoadedLevels(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);

    const std::vector<LevelRef>& loadedLevels = world->GetLoadedLevels();

    lua_newtable(L);
    int levelListIdx = lua_gettop(L);
    for (uint32_t i = 0; i < loadedLevels.size(); ++i)
    {
        lua_pushinteger(L, (int)i);
        lua_pushstring(L, loadedLevels[i].Get()->GetName().c_str());
        lua_settable(L, levelListIdx);
    }
    return 1;
}

int World_Lua::UnloadAllLevels(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);

    world->UnloadAllLevels();

    return 0;
}

int World_Lua::SpawnBlueprint(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);
    const char* name = CHECK_STRING(L, 2);

    Actor* ret = world->SpawnBlueprint(name);

    Actor_Lua::Create(L, ret);
    return 1;
}

int World_Lua::LoadLevel(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);
    const char* name = CHECK_STRING(L, 2);

    world->LoadLevel(name);

    return 0;
}

int World_Lua::QueueLevelLoad(lua_State* L)
{
    int numArgs = lua_gettop(L);
    World* world = CHECK_WORLD(L, 1);
    const char* levelName = CHECK_STRING(L, 2);
    bool clearWorld = false;
    if (numArgs >= 3) { clearWorld = CHECK_BOOLEAN(L, 3); }

    world->QueueLevelLoad(levelName, clearWorld);

    return 0;
}

int World_Lua::UnloadLevel(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);
    const char* name = CHECK_STRING(L, 2);

    world->UnloadLevel(name);

    return 0;
}

int World_Lua::EnableInternalEdgeSmoothing(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    world->EnableInternalEdgeSmoothing(value);

    return 0;
}

int World_Lua::IsInternalEdgeSmoothingEnabled(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);

    bool ret = world->IsInternalEdgeSmoothingEnabled();

    lua_pushboolean(L, ret);
    return 1;
}

void World_Lua::Bind()
{
    lua_State* L = GetLua();
    assert(lua_gettop(L) == 0);

    luaL_newmetatable(L, WORLD_LUA_NAME);
    int mtIndex = lua_gettop(L);

    lua_pushcfunction(L, World_Lua::GetActiveCamera);
    lua_setfield(L, mtIndex, "GetActiveCamera");

    lua_pushcfunction(L, World_Lua::GetAudioReceiver);
    lua_setfield(L, mtIndex, "GetAudioReceiver");

    lua_pushcfunction(L, World_Lua::GetDirectionalLight);
    lua_setfield(L, mtIndex, "GetDirectionalLight");

    lua_pushcfunction(L, World_Lua::SetActiveCamera);
    lua_setfield(L, mtIndex, "SetActiveCamera");

    lua_pushcfunction(L, World_Lua::SetAudioReceiver);
    lua_setfield(L, mtIndex, "SetAudioReceiver");

    lua_pushcfunction(L, World_Lua::SetDirectionalLight);
    lua_setfield(L, mtIndex, "SetDirectionalLight");

    lua_pushcfunction(L, World_Lua::SpawnActor);
    lua_setfield(L, mtIndex, "SpawnActor");

    lua_pushcfunction(L, World_Lua::CloneActor);
    lua_setfield(L, mtIndex, "CloneActor");

    lua_pushcfunction(L, World_Lua::DestroyActor);
    lua_setfield(L, mtIndex, "DestroyActor");

    lua_pushcfunction(L, World_Lua::DestroyAllActors);
    lua_setfield(L, mtIndex, "DestroyAllActors");

    lua_pushcfunction(L, World_Lua::FindActor);
    lua_setfield(L, mtIndex, "FindActor");

    lua_pushcfunction(L, World_Lua::FindComponent);
    lua_setfield(L, mtIndex, "FindComponent");

    lua_pushcfunction(L, World_Lua::PrioritizeActorTick);
    lua_setfield(L, mtIndex, "PrioritizeActorTick");

    lua_pushcfunction(L, World_Lua::SetAmbientLightColor);
    lua_setfield(L, mtIndex, "SetAmbientLightColor");

    lua_pushcfunction(L, World_Lua::GetAmbientLightColor);
    lua_setfield(L, mtIndex, "GetAmbientLightColor");

    lua_pushcfunction(L, World_Lua::SetShadowColor);
    lua_setfield(L, mtIndex, "SetShadowColor");

    lua_pushcfunction(L, World_Lua::GetShadowColor);
    lua_setfield(L, mtIndex, "GetShadowColor");

    lua_pushcfunction(L, World_Lua::SetFogSettings);
    lua_setfield(L, mtIndex, "SetFogSettings");

    lua_pushcfunction(L, World_Lua::GetFogSettings);
    lua_setfield(L, mtIndex, "GetFogSettings");

    lua_pushcfunction(L, World_Lua::SetGravity);
    lua_setfield(L, mtIndex, "SetGravity");

    lua_pushcfunction(L, World_Lua::GetGravity);
    lua_setfield(L, mtIndex, "GetGravity");

    lua_pushcfunction(L, World_Lua::RayTest);
    lua_setfield(L, mtIndex, "RayTest");

    lua_pushcfunction(L, World_Lua::RayTestMulti);
    lua_setfield(L, mtIndex, "RayTestMulti");

    lua_pushcfunction(L, World_Lua::SweepTest);
    lua_setfield(L, mtIndex, "SweepTest");

    lua_pushcfunction(L, World_Lua::GetLoadedLevels);
    lua_setfield(L, mtIndex, "GetLoadedLevels");

    lua_pushcfunction(L, World_Lua::UnloadAllLevels);
    lua_setfield(L, mtIndex, "UnloadAllLevels");

    lua_pushcfunction(L, World_Lua::SpawnBlueprint);
    lua_setfield(L, mtIndex, "SpawnBlueprint");

    lua_pushcfunction(L, World_Lua::LoadLevel);
    lua_setfield(L, mtIndex, "LoadLevel");

    lua_pushcfunction(L, World_Lua::QueueLevelLoad);
    lua_setfield(L, mtIndex, "QueueLevelLoad");

    lua_pushcfunction(L, World_Lua::UnloadLevel);
    lua_setfield(L, mtIndex, "UnloadLevel");

    lua_pushcfunction(L, World_Lua::EnableInternalEdgeSmoothing);
    lua_setfield(L, mtIndex, "EnableInternalEdgeSmoothing");

    lua_pushcfunction(L, World_Lua::IsInternalEdgeSmoothingEnabled);
    lua_setfield(L, mtIndex, "IsInternalEdgeSmoothingEnabled");

    // Set the __index metamethod to itself
    lua_pushvalue(L, mtIndex);
    lua_setfield(L, mtIndex, "__index");

    lua_pop(L, 1);
    assert(lua_gettop(L) == 0);
}

#endif
