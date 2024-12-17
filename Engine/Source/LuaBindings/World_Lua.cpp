
#include "LuaBindings/World_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Node_Lua.h"
#include "LuaBindings/Node3d_Lua.h"
#include "LuaBindings/Primitive3d_Lua.h"
#include "LuaBindings/Camera3d_Lua.h"
#include "LuaBindings/DirectionalLight3d_Lua.h"
#include "LuaBindings/ParticleSystem_Lua.h"

#include "Nodes/3D/Primitive3d.h"
#include "Nodes/3D/Particle3d.h"

#if LUA_ENABLED

int World_Lua::Create(lua_State* L, World* world)
{
    if (world != nullptr)
    {
        World_Lua* worldLua = (World_Lua*)lua_newuserdata(L, sizeof(World_Lua));
        worldLua->mWorld = world;

        int udIndex = lua_gettop(L);
        luaL_getmetatable(L, WORLD_LUA_NAME);
        OCT_ASSERT(lua_istable(L, -1));
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
    
    Camera3D* cameraComp = world->GetActiveCamera();

    Node_Lua::Create(L, cameraComp);
    return 1;
}

int World_Lua::GetAudioReceiver(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);

    Node3D* receiverComp = world->GetAudioReceiver();

    Node_Lua::Create(L, receiverComp);
    return 1;
}

int World_Lua::SetActiveCamera(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);
    Camera3D* cameraComp = CHECK_CAMERA_3D(L, 2);

    world->SetActiveCamera(cameraComp);

    return 0;
}

int World_Lua::SetAudioReceiver(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);
    Node3D* transformComp = nullptr;
    if (!lua_isnil(L, 2))
    {
        transformComp = CHECK_NODE_3D(L, 2);
    }

    world->SetAudioReceiver(transformComp);

    return 0;
}

int World_Lua::SpawnNode(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);
    const char* nodeClass = CHECK_STRING(L, 2);

    Node* spawnedNode = world->SpawnNode(nodeClass);

    Node_Lua::Create(L, spawnedNode);
    return 1;
}

int World_Lua::SpawnScene(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);
    const char* sceneName = CHECK_STRING(L, 2);

    Node* spawnedNode = world->SpawnScene(sceneName);

    Node_Lua::Create(L, spawnedNode);
    return 1;
}

int World_Lua::GetRootNode(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);

    Node* ret = world->GetRootNode();

    Node_Lua::Create(L, ret);
    return 1;
}

int World_Lua::SetRootNode(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);
    Node* node = CHECK_NODE(L, 2);

    world->SetRootNode(node);

    return 0;
}

int World_Lua::DestroyRootNode(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);

    world->DestroyRootNode();

    return 0;
}

int World_Lua::FindNode(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);
    const char* name = CHECK_STRING(L, 2);

    Node* node = world->FindNode(name);

    Node_Lua::Create(L, node);
    return 1;
}

int World_Lua::FindNodesWithTag(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);
    const char* tag = CHECK_STRING(L, 2);

    std::vector<Node*> nodes = world->FindNodesWithTag(tag);

    lua_newtable(L);
    int arrayIdx = lua_gettop(L);

    for (uint32_t i = 0; i < nodes.size(); ++i)
    {
        lua_pushinteger(L, (int)i + 1);
        Node_Lua::Create(L, nodes[i]);
        lua_settable(L, arrayIdx);
    }

    return 1;
}

int World_Lua::FindNodesWithName(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);
    const char* name = CHECK_STRING(L, 2);

    std::vector<Node*> nodes = world->FindNodesWithName(name);

    lua_newtable(L);
    int arrayIdx = lua_gettop(L);

    for (uint32_t i = 0; i < nodes.size(); ++i)
    {
        lua_pushinteger(L, (int)i + 1);
        Node_Lua::Create(L, nodes[i]);
        lua_settable(L, arrayIdx);
    }

    return 1;
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
    std::vector<btCollisionObject*> ignoreObjects;

    if (!lua_isnoneornil(L, 5))
    {
        CHECK_TABLE(L, 5);
        Datum ignoreTable = LuaObjectToDatum(L, 5);

        for (uint32_t i = 1; i <= ignoreTable.GetCount(); ++i)
        {
            RTTI* rtti = ignoreTable.GetPointerField(i);
            Primitive3D* prim = rtti ? rtti->As<Primitive3D>() : nullptr;

            if (prim && prim->GetRigidBody())
            {
                ignoreObjects.push_back(prim->GetRigidBody());
            }
        }
    }

    RayTestResult result;
    world->RayTest(start, end, colMask, result, uint32_t(ignoreObjects.size()), ignoreObjects.data());

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
        Node_Lua::Create(L, result.mHitNodes[i]);
        lua_setfield(L, hitTableIdx, "node");
        Vector_Lua::Create(L, result.mHitNormals[i]);
        lua_setfield(L, hitTableIdx, "normal");
        Vector_Lua::Create(L, result.mHitPositions[i]);
        lua_setfield(L, hitTableIdx, "position");
        lua_pushnumber(L, result.mHitFractions[i]);
        lua_setfield(L, hitTableIdx, "fraction");

        lua_pushinteger(L, (int)i + 1);
        lua_pushvalue(L, hitTableIdx);
        lua_settable(L, hitListIdx);

        // the hit instance table is still on the stack.
        lua_pop(L, 1);
    }

    // the hit list table is still on the stack
    lua_pop(L, 1);

    // The top table should be on the top of the stack
    OCT_ASSERT(lua_gettop(L) == 5);
    return 1;
}

int World_Lua::SweepTest(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);
    Primitive3D* primComp = CHECK_PRIMITIVE_3D(L, 2);
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

int World_Lua::LoadScene(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);
    const char* newScene = CHECK_STRING(L, 2);
    bool instant = false;
    if (!lua_isnone(L, 3)) { instant = CHECK_BOOLEAN(L, 3); }

    world->LoadScene(newScene, instant);

    return 0;
}

int World_Lua::QueueRootNode(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);
    Node* newRoot = CHECK_NODE(L, 2);

    world->QueueRootNode(newRoot);

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

int World_Lua::SpawnParticle(lua_State* L)
{
    World* world = CHECK_WORLD(L, 1);
    ParticleSystem* particleSys = CHECK_PARTICLE_SYSTEM(L, 2);
    glm::vec3 pos = CHECK_VECTOR(L, 3);

    Particle3D* ret = world->SpawnParticle(particleSys, pos);

    Node_Lua::Create(L, ret);
    return 1;
}

void World_Lua::Bind()
{
    lua_State* L = GetLua();
    OCT_ASSERT(lua_gettop(L) == 0);

    luaL_newmetatable(L, WORLD_LUA_NAME);
    int mtIndex = lua_gettop(L);

    REGISTER_TABLE_FUNC(L, mtIndex, GetActiveCamera);

    REGISTER_TABLE_FUNC(L, mtIndex, GetAudioReceiver);

    REGISTER_TABLE_FUNC(L, mtIndex, SetActiveCamera);

    REGISTER_TABLE_FUNC(L, mtIndex, SetAudioReceiver);

    REGISTER_TABLE_FUNC(L, mtIndex, SpawnNode);

    REGISTER_TABLE_FUNC(L, mtIndex, SpawnScene);

    REGISTER_TABLE_FUNC(L, mtIndex, GetRootNode);

    REGISTER_TABLE_FUNC(L, mtIndex, SetRootNode);

    REGISTER_TABLE_FUNC(L, mtIndex, DestroyRootNode);

    REGISTER_TABLE_FUNC(L, mtIndex, FindNode);

    REGISTER_TABLE_FUNC(L, mtIndex, FindNodesWithTag);

    REGISTER_TABLE_FUNC(L, mtIndex, FindNodesWithName);

    REGISTER_TABLE_FUNC(L, mtIndex, SetAmbientLightColor);

    REGISTER_TABLE_FUNC(L, mtIndex, GetAmbientLightColor);

    REGISTER_TABLE_FUNC(L, mtIndex, SetShadowColor);

    REGISTER_TABLE_FUNC(L, mtIndex, GetShadowColor);

    REGISTER_TABLE_FUNC(L, mtIndex, SetFogSettings);

    REGISTER_TABLE_FUNC(L, mtIndex, GetFogSettings);

    REGISTER_TABLE_FUNC(L, mtIndex, SetGravity);

    REGISTER_TABLE_FUNC(L, mtIndex, GetGravity);

    REGISTER_TABLE_FUNC(L, mtIndex, RayTest);

    REGISTER_TABLE_FUNC(L, mtIndex, RayTestMulti);

    REGISTER_TABLE_FUNC(L, mtIndex, SweepTest);

    REGISTER_TABLE_FUNC(L, mtIndex, LoadScene);

    REGISTER_TABLE_FUNC(L, mtIndex, QueueRootNode);

    REGISTER_TABLE_FUNC(L, mtIndex, EnableInternalEdgeSmoothing);

    REGISTER_TABLE_FUNC(L, mtIndex, IsInternalEdgeSmoothingEnabled);

    REGISTER_TABLE_FUNC(L, mtIndex, SpawnParticle);

    // Set the __index metamethod to itself
    lua_pushvalue(L, mtIndex);
    lua_setfield(L, mtIndex, "__index");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
