#include "World.h"
#include "Nodes/3D/CameraComponent.h"
#include "Constants.h"
#include "Renderer.h"
#include "Profiler.h"
#include "Utilities.h"
#include "AudioManager.h"
#include "AssetManager.h"
#include "NetworkManager.h"
#include "InputDevices.h"
#include "StaticMeshActor.h"
#include "Assets/Level.h"
#include "Assets/Blueprint.h"
#include "Nodes/3D/StaticMeshComponent.h"
#include "Nodes/3D/PointLightComponent.h"
#include "Nodes/3D/AudioComponent.h"

#if EDITOR
#include "Editor/EditorState.h"
#endif

#include <map>
#include <algorithm>

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btInternalEdgeUtility.h>
#include <Bullet/BulletCollision/CollisionShapes/btTriangleShape.h>

using namespace std;

bool ContactAddedHandler(btManifoldPoint& cp,
    const btCollisionObjectWrapper* colObj0Wrap,
    int partId0,
    int index0,
    const btCollisionObjectWrapper* colObj1Wrap,
    int partId1,
    int index1)
{
    btAdjustInternalEdgeContacts(cp, colObj1Wrap, colObj0Wrap, partId1, index1);
    return true;

#if 0
    // This blocked out code was taken from https://pybullet.org/Bullet/phpBB3/viewtopic.php?t=3052
    // but it doesn't seem to be any better than using btAdjustInternalEdgeContacts()
    // There are still some bumps :/
    // Correct the normal
    if (colObj0Wrap->getCollisionShape()->getShapeType() == TRIANGLE_SHAPE_PROXYTYPE)
    {
        btTriangleShape* s0 = ((btTriangleShape*)colObj0Wrap->getCollisionShape());
        cp.m_normalWorldOnB = (s0->m_vertices1[1] - s0->m_vertices1[0]).cross(s0->m_vertices1[2] - s0->m_vertices1[0]);
        cp.m_normalWorldOnB.normalize();
    }
    else if (colObj1Wrap->getCollisionShape()->getShapeType() == TRIANGLE_SHAPE_PROXYTYPE)
    {
        btTriangleShape * s1 = ((btTriangleShape*)colObj1Wrap->getCollisionShape());
        cp.m_normalWorldOnB = (s1->m_vertices1[1] - s1->m_vertices1[0]).cross(s1->m_vertices1[2] - s1->m_vertices1[0]);
        cp.m_normalWorldOnB.normalize();
    }

    return true;
#endif
}

World::World() :
    mDirectionalLight(nullptr),
    mAmbientLightColor(DEFAULT_AMBIENT_LIGHT_COLOR),
    mShadowColor(DEFAULT_SHADOW_COLOR),
    mActiveCamera(nullptr),
    mAudioReceiver(nullptr),
    mDefaultCamera(nullptr),
    mNextNetId(1)
{
    SCOPED_STAT("World()")

    SpawnDefaultCamera();

    // Setup physics world
    mCollisionConfig = new btDefaultCollisionConfiguration();
    mCollisionDispatcher = new btCollisionDispatcher(mCollisionConfig);
    mBroadphase = new btDbvtBroadphase();
    mSolver = new btSequentialImpulseConstraintSolver();
    mDynamicsWorld = new btDiscreteDynamicsWorld(mCollisionDispatcher, mBroadphase, mSolver, mCollisionConfig);
    mDynamicsWorld->setGravity(btVector3(0, -10, 0));
}

void World::Destroy()
{
    while (mActors.size() > 0)
    {
        DestroyActor(int32_t(mActors.size() - 1));
    }

    OCT_ASSERT(mActors.size() == 0);
    mActors.clear();
    mActiveCamera = nullptr;

    delete mDynamicsWorld;
    delete mSolver;
    delete mBroadphase;
    delete mCollisionDispatcher;
    delete mCollisionConfig;

    mDynamicsWorld = nullptr;
    mSolver = nullptr;
    mBroadphase = nullptr;
    mCollisionDispatcher = nullptr;
    mCollisionConfig = nullptr;
}

void World::SetTestDirectionalLight()
{
    if (mDirectionalLight == nullptr)
    {
        Actor* dirLightActor = SpawnActor<Actor>();
        mDirectionalLight = dirLightActor->CreateComponent<DirectionalLight3D>();
        dirLightActor->SetRootComponent(mDirectionalLight);
        dirLightActor->SetName("Default Light");
    }

    float lightValue = 5.0f;
    mDirectionalLight->SetActive(true);
    mDirectionalLight->SetCastShadows(true);
    mDirectionalLight->SetColor(glm::vec4(lightValue, 4.0f, 4.0f, 4.0f));
    mDirectionalLight->SetDirection(glm::normalize(glm::vec3(1.0f, -1.0f, 0.2f)));
}

void World::SpawnDefaultCamera()
{
    if (GetActiveCamera() == nullptr)
    {
        Actor* cameraActor = SpawnActor<Actor>();
        mActiveCamera = cameraActor->CreateComponent<Camera3D>();
        cameraActor->SetRootComponent(mActiveCamera);
        cameraActor->SetName("Default Camera");
        cameraActor->SetPersitent(true);
        mActiveCamera->SetPosition(glm::vec3(0.0f, 0.0f, 10.0f));

        mDefaultCamera = mActiveCamera;
    }
}

void World::DestroyActor(Actor* actor)
{
    for (uint32_t i = 0; i < mActors.size(); ++i)
    {
        if (mActors[i] == actor)
        {
            DestroyActor(i);
            break;
        }
    }
}

void World::DestroyActor(uint32_t index)
{
    OCT_ASSERT(index < mActors.size());
    Actor* actor = mActors[index];

    if (actor->GetNetId() != INVALID_NET_ID)
    {
        // Send destroy message
        if (NetIsServer())
        {
            NetworkManager::Get()->SendDestroyMessage(actor, nullptr);
        }

        // This actor was assigned a net id, so it should exist in our net actor map.
        OCT_ASSERT(mNetActorMap.find(actor->GetNetId()) != mNetActorMap.end());
        mNetActorMap.erase(actor->GetNetId());

        // Remove the destroyed actor from their assigned replication vector.
        std::vector<Actor*>& repVector = GetReplicatedActorVector(actor->GetReplicationRate());
        uint32_t& repIndex = GetReplicatedActorIndex(actor->GetReplicationRate());

        for (uint32_t i = 0; i < repVector.size(); ++i)
        {
            if (repVector[i] == actor)
            {
                repVector.erase(repVector.begin() + i);

                // Decrement the rep index so that an actor doesn't get skipped for one cycle.
                if (repIndex > 0 &&
                    repIndex > i)
                {
                    repIndex = repIndex - 1;
                }

                break;
            }
        }
    }

#if DEBUG_DRAW_ENABLED
    Renderer::Get()->RemoveDebugDrawsForActor(actor);
#endif

    actor->Destroy();
    mActors.erase(mActors.begin() + index);
    delete actor;
    actor = nullptr;
}

void World::DestroyAllActors()
{
    for (int32_t i = int32_t(mActors.size()) - 1; i >= 0; --i)
    {
        DestroyActor(i);
    }

    mLoadedLevels.clear();

    mNextNetId = 1;
    SpawnDefaultCamera();
}

void World::FlushPendingDestroys()
{
    for (int32_t i = int32_t(mActors.size()) - 1; i >= 0; --i)
    {
        if (mActors[i]->IsPendingDestroy())
        {
            DestroyActor(i);
        }
    }
}

const std::vector<Actor*>& World::GetActors() const
{
    return mActors;
}

Actor* World::FindActor(const std::string& name)
{
    Actor* foundActor = nullptr;

    for (uint32_t i = 0; i < mActors.size(); ++i)
    {
        if (mActors[i]->GetName() == name)
        {
            foundActor = mActors[i];
            break;
        }
    }

    return foundActor;
}

Actor* World::FindActor(NetId netId)
{
    Actor* retActor = nullptr;
    auto it = mNetActorMap.find(netId);
    if (it != mNetActorMap.end())
    {
        retActor = it->second;
    }
    return retActor;
}

std::vector<Actor*> World::FindActorsByTag(const char* tag)
{
    std::vector<Actor*> retActors;

    for (uint32_t i = 0; i < mActors.size(); ++i)
    {
        if (mActors[i]->HasTag(tag))
        {
            retActors.push_back(mActors[i]);
        }
    }

    return retActors;
}

std::vector<Actor*> World::FindActorsByName(const char* name)
{
    std::vector<Actor*> retActors;

    for (uint32_t i = 0; i < mActors.size(); ++i)
    {
        if (mActors[i]->GetName() == name)
        {
            retActors.push_back(mActors[i]);
        }
    }

    return retActors;
}

Component* World::FindComponent(const std::string& name)
{
    Component* foundComponent = nullptr;

    for (uint32_t i = 0; i < mActors.size(); ++i)
    {
        const std::vector<Component*>& components = mActors[i]->GetComponents();
        for (uint32_t c = 0; c < components.size(); ++c)
        {
            if (components[c]->GetName() == name)
            {
                foundComponent = components[c];
                break;
            }
        }
    }

    return foundComponent;
}

void World::PrioritizeActorTick(Actor* actor)
{
    // This function simply moves the requested actor to the front of the mActors vector.
    // In the future maybe we could implement some tick priority system or something.
    auto it = std::find(mActors.begin(), mActors.end(), actor);

    if (it != mActors.end())
    {
        Actor* target = *it;
        mActors.erase(it);
        mActors.insert(mActors.begin(), target);
    }
}

void World::AddNetActor(Actor* actor, NetId netId)
{
    OCT_ASSERT(actor != nullptr);
    OCT_ASSERT(actor->GetNetId() == INVALID_NET_ID);

    if (actor->IsReplicated())
    {
        // Gather net functions (even if local)
        Actor::RegisterNetFuncs(actor);

        if (!NetIsLocal())
        {
            actor->GatherReplicatedData(actor->GetReplicatedData());

            if (NetIsServer() &&
                netId == INVALID_NET_ID)
            {
                netId = mNextNetId;
                ++mNextNetId;
            }

            if (netId != INVALID_NET_ID)
            {
                actor->SetNetId(netId);
                mNetActorMap.insert({ netId, actor });

                std::vector<Actor*>& repActorVector = GetReplicatedActorVector(actor->GetReplicationRate());
                repActorVector.push_back(actor);

                // The server needs to send Spawn messages for newly added network actors.
                if (NetIsServer())
                {
                    NetworkManager::Get()->SendSpawnMessage(actor, nullptr);
                }
            }
        }
    }
}

const std::unordered_map<NetId, Actor*>& World::GetNetActorMap() const
{
    return mNetActorMap;
}

void World::Clear(bool clearPersistent)
{
    if (clearPersistent)
    {
        DestroyAllActors();
    }
    else
    {
        // Unload all levels
        UnloadAllLevels();

        // Destroy all non-persistent actors.
        for (int32_t i = int32_t(mActors.size()) - 1; i >= 0; --i)
        {
            if (!mActors[i]->IsPersistent())
            {
                DestroyActor(i);
            }
        }

        mNextNetId = 1;
        SpawnDefaultCamera();
    }
}

void World::AddLine(const Line& line)
{
    // Add unique
    for (uint32_t i = 0; i < mLines.size(); ++i)
    {
        if (mLines[i] == line)
        {
            // If the same line already exist, choose the larger lifetime.
            if (mLines[i].mLifetime < 0.0f || line.mLifetime < 0.0f)
            {
                mLines[i].mLifetime = -1.0f;
            }
            else
            {
                float newLifetime = glm::max(mLines[i].mLifetime, line.mLifetime);
                mLines[i].mLifetime = newLifetime;
            }

            return;
        }
    }

    mLines.push_back(line);
}

void World::RemoveLine(const Line& line)
{
    for (uint32_t i = 0; i < mLines.size(); ++i)
    {
        if (mLines[i] == line)
        {
            mLines.erase(mLines.begin() + i);
            break;
        }
    }
}

void World::RemoveAllLines()
{
    mLines.clear();
}

const std::vector<Line>& World::GetLines() const
{
    return mLines;
}

const std::vector<Light3D*>& World::GetLightComponents()
{
    return mLightComponents;
}

void World::SetAmbientLightColor(glm::vec4 color)
{
    mAmbientLightColor = color;
}

glm::vec4 World::GetAmbientLightColor() const
{
    return mAmbientLightColor;
}

void World::SetShadowColor(glm::vec4 shadowColor)
{
    mShadowColor = shadowColor;
}

glm::vec4 World::GetShadowColor() const
{
    return mShadowColor;
}

void World::SetFogSettings(const FogSettings& settings)
{
    mFogSettings = settings;
}

const FogSettings& World::GetFogSettings() const
{
    return mFogSettings;
}

void World::SetGravity(glm::vec3 gravity)
{
    if (mDynamicsWorld)
    {
        btVector3 btGrav = GlmToBullet(gravity);
        mDynamicsWorld->setGravity(btGrav);
    }
}

glm::vec3 World::GetGravity() const
{
    glm::vec3 ret = {};

    if (mDynamicsWorld)
    {
        ret = BulletToGlm(mDynamicsWorld->getGravity());
    }

    return ret;
}

btDynamicsWorld* World::GetDynamicsWorld()
{
    return mDynamicsWorld;
}

btDbvtBroadphase* World::GetBroadphase()
{
    return mBroadphase;
}

void World::PurgeOverlaps(Primitive3D* prim)
{
    for (int32_t i = (int32_t)mCurrentOverlaps.size() - 1; i >= 0; --i)
    {
        Primitive3D* compA = mCurrentOverlaps[i].mComponentA;
        Primitive3D* compB = mCurrentOverlaps[i].mComponentB;

        if (compA == prim ||
            compB == prim)
        {
            compA->GetOwner()->EndOverlap(compA, compB);
            mCurrentOverlaps.erase(mCurrentOverlaps.begin() + i);
        }
    }
}

void World::RayTest(glm::vec3 start, glm::vec3 end, uint8_t collisionMask, RayTestResult& outResult)
{
    outResult.mStart = start;
    outResult.mEnd = end;

    btVector3 fromWorld = btVector3(start.x, start.y, start.z);
    btVector3 toWorld = btVector3(end.x, end.y, end.z);

    btCollisionWorld::ClosestRayResultCallback result(fromWorld, toWorld);
    result.m_collisionFilterGroup = (short)ColGroupAll;
    result.m_collisionFilterMask = collisionMask;

    //mDynamicsWorld->rayTestSingle()
    mDynamicsWorld->rayTest(fromWorld, toWorld, result);

    outResult.mHitPosition = { result.m_hitPointWorld.x(), result.m_hitPointWorld.y(), result.m_hitPointWorld.z() };
    outResult.mHitNormal = { result.m_hitNormalWorld.x(), result.m_hitNormalWorld.y(), result.m_hitNormalWorld.z() };
    outResult.mHitFraction = result.m_closestHitFraction;

    if (result.m_collisionObject != nullptr)
    {
        outResult.mHitComponent = reinterpret_cast<Primitive3D*>(result.m_collisionObject->getUserPointer());
    }
    else
    {
        outResult.mHitComponent = nullptr;
    }
}

void World::RayTestMulti(glm::vec3 start, glm::vec3 end, uint8_t collisionMask, RayTestMultiResult& outResult)
{
    outResult.mStart = start;
    outResult.mEnd = end;

    btVector3 fromWorld = btVector3(start.x, start.y, start.z);
    btVector3 toWorld = btVector3(end.x, end.y, end.z);

    btCollisionWorld::AllHitsRayResultCallback result(fromWorld, toWorld);
    result.m_collisionFilterGroup = (short)ColGroupAll;
    result.m_collisionFilterMask = collisionMask;

    mDynamicsWorld->rayTest(fromWorld, toWorld, result);

    outResult.mNumHits = uint32_t(result.m_collisionObjects.size());

    for (uint32_t i = 0; i < outResult.mNumHits; ++i)
    {
        outResult.mHitPositions.push_back({ result.m_hitPointWorld[i].x(), result.m_hitPointWorld[i].y(), result.m_hitPointWorld[i].z() });
        outResult.mHitNormals.push_back({ result.m_hitNormalWorld[i].x(), result.m_hitNormalWorld[i].y(), result.m_hitNormalWorld[i].z() });
        outResult.mHitFractions.push_back(result.m_hitFractions[i]);
        outResult.mHitComponents.push_back(reinterpret_cast<Primitive3D*>(result.m_collisionObjects[i]->getUserPointer()));
    }
}

void World::SweepTest(Primitive3D* primComp, glm::vec3 start, glm::vec3 end, uint8_t collisionMask, SweepTestResult& outResult)
{
    if (primComp->GetCollisionShape() == nullptr ||
        primComp->GetCollisionShape()->isCompound() ||
        !primComp->GetCollisionShape()->isConvex())
    {
        LogError("SweepTest is only supported for non-compound convex shapes.");
        return;
    }

    btConvexShape* convexShape = static_cast<btConvexShape*>(primComp->GetCollisionShape());
    glm::quat rotation = primComp->GetRotationQuat();
    btCollisionObject* compColObj = primComp->GetRigidBody();
    SweepTest(convexShape, start, end, rotation, collisionMask, outResult, 1, &compColObj);
}

void World::SweepTest(
    btConvexShape* convexShape,
    glm::vec3 start,
    glm::vec3 end,
    glm::quat rotation,
    uint8_t collisionMask,
    SweepTestResult& outResult,
    uint32_t numIgnoreObjects,
    btCollisionObject** ignoreObjects)
{
    if (start == end)
    {
        outResult.mStart = start;
        outResult.mEnd = end;
        outResult.mHitFraction = 1.0f;
        outResult.mHitComponent = nullptr;
        return;
    }

    outResult.mStart = start;
    outResult.mEnd = end;


    btVector3 startPos = btVector3(start.x, start.y, start.z);
    btVector3 endPos = btVector3(end.x, end.y, end.z);
    btQuaternion rot = btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w);

    btTransform startTransform(rot, startPos);
    btTransform endTransform(rot, endPos);

    IgnoreConvexResultCallback result(startPos, endPos);
    result.m_collisionFilterGroup = (short)ColGroupAll;
    result.m_collisionFilterMask = collisionMask;
    result.mNumIgnoreObjects = numIgnoreObjects;
    result.mIgnoreObjects = ignoreObjects;

    mDynamicsWorld->convexSweepTest(
        convexShape,
        startTransform,
        endTransform,
        result);

    outResult.mHitPosition = { result.m_hitPointWorld.x(), result.m_hitPointWorld.y(), result.m_hitPointWorld.z() };
    outResult.mHitNormal = { result.m_hitNormalWorld.x(), result.m_hitNormalWorld.y(), result.m_hitNormalWorld.z() };
    outResult.mHitFraction = result.m_closestHitFraction;

    if (result.m_hitCollisionObject != nullptr)
    {
        outResult.mHitComponent = reinterpret_cast<Primitive3D*>(result.m_hitCollisionObject->getUserPointer());
    }
    else
    {
        outResult.mHitComponent = nullptr;
    }
}

void World::RegisterComponent(Component* comp)
{
    TypeId compType = comp->GetType();

    if (compType == Audio3D::GetStaticType())
    {
#if _DEBUG
        OCT_ASSERT(std::find(mAudioComponents.begin(), mAudioComponents.end(), (Audio3D*)comp) == mAudioComponents.end());
#endif
        mAudioComponents.push_back((Audio3D*) comp);
    }
    else if (comp->IsLightComponent())
    {
#if _DEBUG
        OCT_ASSERT(std::find(mLightComponents.begin(), mLightComponents.end(), (Light3D*)comp) == mLightComponents.end());
#endif
        mLightComponents.push_back((Light3D*)comp);
    }
}

void World::UnregisterComponent(Component* comp)
{
    TypeId compType = comp->GetType();

    if (compType == Audio3D::GetStaticType())
    {
        auto it = std::find(mAudioComponents.begin(), mAudioComponents.end(), (Audio3D*)comp);
        OCT_ASSERT(it != mAudioComponents.end());
        mAudioComponents.erase(it);
    }
    else if (comp->IsLightComponent())
    {
        auto it = std::find(mLightComponents.begin(), mLightComponents.end(), (Light3D*)comp);
        OCT_ASSERT(it != mLightComponents.end());
        mLightComponents.erase(it);
    }
}

const std::vector<Audio3D*>& World::GetAudioComponents() const
{
    return mAudioComponents;
}

std::vector<Actor*>& World::GetReplicatedActorVector(ReplicationRate rate)
{
    OCT_ASSERT(rate != ReplicationRate::Count);
    return mRepActors[(uint32_t)rate];
}

uint32_t& World::GetReplicatedActorIndex(ReplicationRate rate)
{
    OCT_ASSERT(rate != ReplicationRate::Count);
    return mRepIndices[(uint32_t)rate];
}

uint32_t& World::GetIncrementalRepTier()
{
    return mIncrementalRepTier;
}

uint32_t& World::GetIncrementalRepIndex()
{
    return mIncrementalRepIndex;
}

std::vector<LevelRef>& World::GetLoadedLevels()
{
    return mLoadedLevels;
}

void World::UnloadAllLevels()
{
    for (int32_t i = int32_t(mLoadedLevels.size()) - 1; i >= 0; --i)
    {
        mLoadedLevels[i].Get<Level>()->UnloadFromWorld(this);
    }

    mLoadedLevels.clear();
}

void World::UpdateLines(float deltaTime)
{
    for (int32_t i = (int32_t)mLines.size() - 1; i >= 0; --i)
    {
        // Only "update" lines with >= 0.0f lifetime.
        // Anything with negative lifetime is meant to be persistent.
        if (mLines[i].mLifetime >= 0.0f)
        {
            mLines[i].mLifetime -= deltaTime;

            if (mLines[i].mLifetime <= 0.0f)
            {
                mLines.erase(mLines.begin() + i);
            }
        }
    }
}

void World::AddActor(Actor* actor)
{
#if !EDITOR
    // This should really only be used in Editor for Undo/Redo reasons.
    OCT_ASSERT(0);
#endif

    OCT_ASSERT(std::find(mActors.begin(), mActors.end(), actor) == mActors.end());
    mActors.push_back(actor);
    actor->SetWorld(this);

    // Unregister components from world
    for (uint32_t i = 0; i < actor->GetNumComponents(); ++i)
    {
        GetWorld()->RegisterComponent(actor->GetComponent(i));
    }
}

void World::RemoveActor(Actor* actor)
{
#if !EDITOR
    // This should really only be used in Editor for Undo/Redo reasons.
    OCT_ASSERT(0);
#endif

    for (uint32_t i = 0; i < mActors.size(); ++i)
    {
        if (mActors[i] == actor)
        {
            mActors.erase(mActors.begin() + i);
            break;
        }
    }

    // Unregister components from world
    for (uint32_t i = 0; i < actor->GetNumComponents(); ++i)
    {
        GetWorld()->UnregisterComponent(actor->GetComponent(i));
    }

    actor->SetWorld(nullptr);
}

void World::Update(float deltaTime)
{
    bool gameTickEnabled = IsGameTickEnabled();

    if (mPendingClear)
    {
        Clear();
        mPendingClear = false;
    }

    // Load any queued levels.
    if (mQueuedLevels.size() > 0)
    {
        for (uint32_t i = 0; i < mQueuedLevels.size(); ++i)
        {
            Level* level = mQueuedLevels[i].mLevel.Get<Level>();
            glm::vec3 offset = mQueuedLevels[i].mOffset;
            glm::vec3 rotation = mQueuedLevels[i].mRotation;

            if (level)
            {
                level->LoadIntoWorld(this, false, offset, rotation);
            }
        }

        mQueuedLevels.clear();
    }

    if (gameTickEnabled)
    {
        SCOPED_FRAME_STAT("Physics");
        mDynamicsWorld->stepSimulation(deltaTime, 2);
    }

    if (gameTickEnabled)
    {
        SCOPED_FRAME_STAT("Collisions");
        mCollisionDispatcher->dispatchAllCollisionPairs(
            mBroadphase->getOverlappingPairCache(),
            mDynamicsWorld->getDispatchInfo(),
            mCollisionDispatcher);

        // Update collisions
        mPreviousOverlaps = mCurrentOverlaps;
        mCurrentOverlaps.clear();

        int32_t numManifolds = mDynamicsWorld->getDispatcher()->getNumManifolds();

        for (int32_t i = 0; i < numManifolds; ++i)
        {
            btPersistentManifold* manifold = mDynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
            int32_t numPoints = manifold->getNumContacts();

            if (numPoints == 0)
                continue;

            const btCollisionObject* object0 = manifold->getBody0();
            const btCollisionObject* object1 = manifold->getBody1();

            Primitive3D* prim0 = reinterpret_cast<Primitive3D*>(object0->getUserPointer());
            Primitive3D* prim1 = reinterpret_cast<Primitive3D*>(object1->getUserPointer());

            if (prim0 == nullptr || prim1 == nullptr || prim0 == prim1)
                continue;

            if (prim0->IsCollisionEnabled() && prim1->IsCollisionEnabled())
            {
                glm::vec3 avgNormal = {};
                glm::vec3 avgContactPoint0 = {};
                glm::vec3 avgContactPoint1 = {};

                for (uint32_t i = 0; i < (uint32_t)numPoints; ++i)
                {
                    btManifoldPoint& point = manifold->getContactPoint(i);
                    glm::vec3 normal = BulletToGlm(point.m_normalWorldOnB);
                    glm::vec3 contactPoint0 = BulletToGlm(point.m_positionWorldOnA);
                    glm::vec3 contactPoint1 = BulletToGlm(point.m_positionWorldOnB);

                    avgNormal += normal;
                    avgContactPoint0 += contactPoint0;
                    avgContactPoint1 += contactPoint1;
                }

                avgNormal = glm::normalize(avgNormal);
                avgContactPoint0 /= numPoints;
                avgContactPoint1 /= numPoints;


                prim0->GetOwner()->OnCollision(prim0, prim1, avgContactPoint0, avgNormal, manifold);
                prim1->GetOwner()->OnCollision(prim1, prim0, avgContactPoint1, -avgNormal, manifold);
            }

            if (prim0->AreOverlapsEnabled() && prim1->AreOverlapsEnabled() &&
                std::find(mCurrentOverlaps.begin(), mCurrentOverlaps.end(), ComponentPair(prim0, prim1)) == mCurrentOverlaps.end())
            {
                mCurrentOverlaps.push_back({ prim0, prim1 });
                mCurrentOverlaps.push_back({ prim1, prim0 });
            }
        }

        // Call Begin Overlaps
        for (auto& pair : mCurrentOverlaps)
        {
            bool beginOverlap = std::find(mPreviousOverlaps.begin(), mPreviousOverlaps.end(), pair) == mPreviousOverlaps.end();

            if (beginOverlap)
            {
                pair.mComponentA->GetOwner()->BeginOverlap(pair.mComponentA, pair.mComponentB);
            }
        }

        // Call End Overlaps
        for (auto& pair : mPreviousOverlaps)
        {
            bool endOverlap = std::find(mCurrentOverlaps.begin(), mCurrentOverlaps.end(), pair) == mCurrentOverlaps.end();

            if (endOverlap)
            {
                pair.mComponentA->GetOwner()->EndOverlap(pair.mComponentA, pair.mComponentB);
            }
        }
    }

    UpdateLines(deltaTime);

    {
        SCOPED_FRAME_STAT("Tick");
        mRootNode->RecursiveTick(deltaTime, gameTickEnabled);

    //    for (int32_t i = 0; i < (int32_t)mActors.size(); ++i)
    //    {
    //        if (gameTickEnabled)
    //        {
    //            if (!mActors[i]->HasBegunPlay())
    //            {
    //                mActors[i]->BeginPlay();
    //            }

    //            if (!mActors[i]->IsPendingDestroy() && 
    //                mActors[i]->IsTickEnabled())
    //            {
    //                mActors[i]->Tick(deltaTime);
    //            }
    //        }
    //        else
    //        {
    //            if (!mActors[i]->IsPendingDestroy() && 
    //                mActors[i]->IsTickEnabled())
    //            {
    //                mActors[i]->EditorTick(deltaTime);
    //            }
    //        }

    //        if (mActors[i]->IsPendingDestroy())
    //        {
    //            DestroyActor(i);
    //            --i;
    //        }
    //    }
    }

    {
        // TODO-NODE: Adding this! Make sure it works. I think we need to
        // make sure transforms are updated so that the bullet dynamics world is in sync.
        // But maybe not and we only need to update transforms when getting world pos/rot/scale/transform
        SCOPED_FRAME_STAT("Transforms");
        mRootNode->UpdateTransform(true);
    }
}

Camera3D* World::GetActiveCamera()
{
    return mActiveCamera;
}

Camera3D* World::GetDefaultCamera()
{
    return mDefaultCamera.Get<Camera3D>();
}

Node3D* World::GetAudioReceiver()
{
    if (mAudioReceiver != nullptr)
    {
        return mAudioReceiver;
    }

    if (mActiveCamera != nullptr)
    {
        return mActiveCamera;
    }

    return nullptr;
}

void World::SetActiveCamera(Camera3D* activeCamera)
{
    mActiveCamera = activeCamera;
}

void World::SetAudioReceiver(Node3D* newReceiver)
{
    mAudioReceiver = newReceiver;
}

Actor* World::SpawnActor(TypeId actorType, bool addNetwork)
{
    Actor* retActor = Actor::CreateInstance(actorType);

    if (retActor != nullptr)
    {
        mActors.push_back(retActor);
        retActor->SetWorld(this);
        retActor->Create();

        for (uint32_t i = 0; i < retActor->GetNumComponents(); ++i)
        {
            retActor->GetComponent(i)->SetDefault(true);
        }

#if _DEBUG && (PLATFORM_WINDOWS || PLATFORM_LINUX)
        OCT_ASSERT(retActor->DoComponentsHaveUniqueNames());
#endif

        if (addNetwork)
        {
            AddNetActor(retActor, INVALID_NET_ID);
        }
    }
    
    return retActor;
}

Actor* World::SpawnActor(const char* typeName)
{
    Actor* retActor = nullptr;
    const std::vector<Factory*>& factories = Actor::GetFactoryList();
    for (uint32_t i = 0; i < factories.size(); ++i)
    {
        if (strncmp(typeName, factories[i]->GetClassName(), MAX_PATH_SIZE) == 0)
        {
            retActor = SpawnActor(factories[i]->GetType());
            break;
        }
    }

    return retActor;
}

Actor* World::CloneActor(Actor* srcActor)
{
    Actor* retActor = SpawnActor(srcActor->GetType());

    if (retActor != nullptr)
    {
        retActor->Copy(srcActor);
    }

    return retActor;   
}

Actor* World::SpawnBlueprint(const char* name)
{
    Actor* ret = nullptr;
    Blueprint* bp = LoadAsset<Blueprint>(name);

    if (bp != nullptr)
    {
        ret = bp->Instantiate(this);
    }
    else
    {
        LogError("Failed to load blueprint: %s", name);
    }

    return ret;
}

void World::LoadLevel(const char* name, bool clear, glm::vec3 offset, glm::vec3 rotation)
{
    Level* level = LoadAsset<Level>(name);

    if (level != nullptr)
    {
        level->LoadIntoWorld(this, clear, offset, rotation);
    }
    else
    {
        LogError("Failed to load level.");
    }
}

void World::QueueLevelLoad(const char* name, bool clearWorld, glm::vec3 offset, glm::vec3 rotation)
{
    // Load level at the beginning of next frame
    if (clearWorld)
    {
        mPendingClear = true;
    }

    Level* level = LoadAsset<Level>(name);

    if (level != nullptr)
    {
        QueuedLevel queuedLevel;
        queuedLevel.mLevel = level;
        queuedLevel.mOffset = offset;
        queuedLevel.mRotation = rotation;
        mQueuedLevels.push_back(queuedLevel);
    }
}

void World::UnloadLevel(const char* name)
{
    for (uint32_t i = 0; i < mLoadedLevels.size(); ++i)
    {
        if (mLoadedLevels[i].Get()->GetName() == name)
        {
            mLoadedLevels[i].Get<Level>()->UnloadFromWorld(this);
            mLoadedLevels.erase(mLoadedLevels.begin() + i);
        }
    }
}

bool World::IsLevelLoaded(const char* levelName)
{
    bool loaded = false;

    for (uint32_t i = 0; i < mLoadedLevels.size(); ++i)
    {
        if (mLoadedLevels[i].Get()->GetName() == levelName)
        {
            loaded = true;
            break;
        }
    }

    return loaded;
}

void World::EnableInternalEdgeSmoothing(bool enable)
{
    gContactAddedCallback = enable ? ContactAddedHandler : nullptr;
}

bool World::IsInternalEdgeSmoothingEnabled() const
{
    return (gContactAddedCallback != nullptr);
}

#if EDITOR

bool World::IsComponentSelected(Component* comp) const
{
    return ::IsComponentSelected(comp);
}

Component* World::GetSelectedComponent()
{
    return ::GetSelectedComponent();
}

const std::vector<Component*>& World::GetSelectedComponents()
{
    return ::GetSelectedComponents();
}

std::vector<Actor*> World::GetSelectedActors()
{
    return ::GetSelectedActors();
}

void World::DeselectComponent(Component* comp)
{
    ::DeselectComponent(comp);
}

#endif
