#include "World.h"
#include "Components/CameraComponent.h"
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
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/AudioComponent.h"

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
    mAmbientLightColor(0.1f, 0.1f, 0.1f, 1.0f),
    mShadowColor(0.0f, 0.0f, 0.0f, 0.8f),
    mActiveCamera(nullptr),
    mAudioReceiver(nullptr),
    mNextNetId(1)
{
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
    for (uint32_t i = 0; i < mActors.size(); ++i)
    {
        // Actor destructor calls Destroy(). Might change that in the future.
        //mActors[i]->Destroy();
        delete mActors[i];
        mActors[i] = nullptr;
    }

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
        mDirectionalLight = dirLightActor->CreateComponent<DirectionalLightComponent>();
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
    // TODO: Active camera should point to one of the
    // cameras loaded from the .dae file.
    Actor* cameraActor = SpawnActor<Actor>();
    mActiveCamera = cameraActor->CreateComponent<CameraComponent>();
    cameraActor->SetRootComponent(mActiveCamera);
    cameraActor->SetName("Default Camera");
    mActiveCamera->SetPosition(glm::vec3(0.0f, 0.0f, 10.0f));
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
    assert(index < mActors.size());
    Actor* actor = mActors[index];

    if (actor->GetNetId() != INVALID_NET_ID)
    {
        // This actor was assigned a net id, so it should exist in our net actor map.
        assert(mNetActorMap.find(actor->GetNetId()) != mNetActorMap.end());
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

    mNextNetId = 1;
    SpawnDefaultCamera();
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
    assert(actor != nullptr);
    assert(actor->GetNetId() == INVALID_NET_ID);

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

const std::vector<PointLightComponent*>& World::GetPointLights()
{
    return mPointLights;
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

void World::PurgeOverlaps(PrimitiveComponent* prim)
{
    for (int32_t i = (int32_t)mCurrentOverlaps.size() - 1; i >= 0; --i)
    {
        PrimitiveComponent* compA = mCurrentOverlaps[i].mComponentA;
        PrimitiveComponent* compB = mCurrentOverlaps[i].mComponentB;

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
        outResult.mHitComponent = reinterpret_cast<PrimitiveComponent*>(result.m_collisionObject->getUserPointer());
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
        outResult.mHitComponents.push_back(reinterpret_cast<PrimitiveComponent*>(result.m_collisionObjects[i]->getUserPointer()));
    }
}

void World::SweepTest(PrimitiveComponent* primComp, glm::vec3 start, glm::vec3 end, uint8_t collisionMask, SweepTestResult& outResult)
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
        outResult.mHitComponent = reinterpret_cast<PrimitiveComponent*>(result.m_hitCollisionObject->getUserPointer());
    }
    else
    {
        outResult.mHitComponent = nullptr;
    }
}

void World::RegisterComponent(Component* comp)
{
    TypeId compType = comp->GetType();

    if (compType == AudioComponent::GetStaticType())
    {
#if _DEBUG
        assert(std::find(mAudioComponents.begin(), mAudioComponents.end(), (AudioComponent*)comp) == mAudioComponents.end());
#endif
        mAudioComponents.push_back((AudioComponent*) comp);
    }
    else if (compType == PointLightComponent::GetStaticType())
    {
#if _DEBUG
        assert(std::find(mPointLights.begin(), mPointLights.end(), (PointLightComponent*)comp) == mPointLights.end());
#endif
        mPointLights.push_back((PointLightComponent*)comp);
    }
}

void World::UnregisterComponent(Component* comp)
{
    TypeId compType = comp->GetType();

    if (compType == AudioComponent::GetStaticType())
    {
        auto it = std::find(mAudioComponents.begin(), mAudioComponents.end(), (AudioComponent*)comp);
        assert(it != mAudioComponents.end());
        mAudioComponents.erase(it);
    }
    else if (compType == PointLightComponent::GetStaticType())
    {
        auto it = std::find(mPointLights.begin(), mPointLights.end(), (PointLightComponent*)comp);
        assert(it != mPointLights.end());
        mPointLights.erase(it);
    }
}

const std::vector<AudioComponent*>& World::GetAudioComponents() const
{
    return mAudioComponents;
}

std::vector<Actor*>& World::GetReplicatedActorVector(ReplicationRate rate)
{
    assert(rate != ReplicationRate::Count);
    return mRepActors[(uint32_t)rate];
}

uint32_t& World::GetReplicatedActorIndex(ReplicationRate rate)
{
    assert(rate != ReplicationRate::Count);
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
    for (uint32_t i = 0; i < mLoadedLevels.size(); ++i)
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

DirectionalLightComponent* World::GetDirectionalLight()
{
    return mDirectionalLight;
}

void World::SetDirectionalLight(DirectionalLightComponent* directionalLight)
{
    mDirectionalLight = directionalLight;
}

void World::Update(float deltaTime)
{
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
                level->LoadIntoWorld(this, offset, rotation);
            }
        }

        mQueuedLevels.clear();
    }

    {
        SCOPED_CPU_STAT("Physics");
        mDynamicsWorld->stepSimulation(deltaTime, 2);
    }

    {
        SCOPED_CPU_STAT("Collisions");
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

            PrimitiveComponent* prim0 = reinterpret_cast<PrimitiveComponent*>(object0->getUserPointer());
            PrimitiveComponent* prim1 = reinterpret_cast<PrimitiveComponent*>(object1->getUserPointer());

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
        SCOPED_CPU_STAT("Tick");
        for (int32_t i = 0; i < (int32_t)mActors.size(); ++i)
        {
#if !EDITOR
            if (!mActors[i]->HasBegunPlay())
            {
                mActors[i]->BeginPlay();
            }
#endif

            if (mActors[i]->IsTickEnabled()
#if EDITOR
                 && mActors[i]->ShouldTickInEditor()
#endif
                )

            {
                mActors[i]->Tick(deltaTime);
            }

            if (mActors[i]->IsPendingDestroy())
            {
                DestroyActor(i);
                --i;
            }
        }
    }

    if (mPendingDestroyAllActors)
    {
        DestroyAllActors();
        mPendingDestroyAllActors = false;
    }
}

CameraComponent* World::GetActiveCamera()
{
    return mActiveCamera;
}

TransformComponent* World::GetAudioReceiver()
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

void World::SetActiveCamera(CameraComponent* activeCamera)
{
    mActiveCamera = activeCamera;
}

void World::SetAudioReceiver(TransformComponent* newReceiver)
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
        LogError("Failed to load blueprint.");
    }

    return ret;
}

void World::LoadLevel(const char* name, glm::vec3 offset, glm::vec3 rotation)
{
    Level* level = LoadAsset<Level>(name);

    if (level != nullptr)
    {
        level->LoadIntoWorld(this, offset, rotation);
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
        mPendingDestroyAllActors = true;
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
