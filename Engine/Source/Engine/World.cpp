#include "World.h"
#include "Nodes/3D/Camera3d.h"
#include "Constants.h"
#include "Renderer.h"
#include "Profiler.h"
#include "Utilities.h"
#include "AudioManager.h"
#include "AssetManager.h"
#include "NetworkManager.h"
#include "InputDevices.h"
#include "Assets/Scene.h"
#include "Nodes/3D/StaticMesh3d.h"
#include "Nodes/3D/PointLight3d.h"
#include "Nodes/3D/Particle3d.h"
#include "Nodes/3D/Audio3d.h"

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
    mAmbientLightColor(DEFAULT_AMBIENT_LIGHT_COLOR),
    mShadowColor(DEFAULT_SHADOW_COLOR),
    mActiveCamera(nullptr),
    mAudioReceiver(nullptr)
{
    SCOPED_STAT("World()")

    // Setup physics world
    mCollisionConfig = new btDefaultCollisionConfiguration();
    mCollisionDispatcher = new btCollisionDispatcher(mCollisionConfig);
    mBroadphase = new btDbvtBroadphase();
    mSolver = new btSequentialImpulseConstraintSolver();
    mDynamicsWorld = new btDiscreteDynamicsWorld(mCollisionDispatcher, mBroadphase, mSolver, mCollisionConfig);
    mDynamicsWorld->setGravity(btVector3(0, -10, 0));

    mDefaultDynamicsWorld = mDynamicsWorld;
}

void World::Destroy()
{
    DestroyRootNode();

    OCT_ASSERT(mRootNode == nullptr);
    mActiveCamera = nullptr;

    mDefaultDynamicsWorld = nullptr;

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

void World::FlushPendingDestroys()
{
    if (mRootNode != nullptr)
    {
        mRootNode->FlushPendingDestroys();

        if (mRootNode->IsPendingDestroy())
        {
            DestroyRootNode();
        }
    }
}

Node* World::GetRootNode()
{
    return mRootNode;
}

void World::SetRootNode(Node* node)
{
    if (mRootNode != node)
    {
        if (mRootNode != nullptr)
        {
            mRootNode->SetWorld(nullptr);
        }

        mRootNode = node;

        if (mRootNode != nullptr)
        {
            mRootNode->SetWorld(this);
        }

        UpdateRenderSettings();
    }
}

void World::DestroyRootNode()
{
    if (mRootNode != nullptr)
    {
        Node::Destruct(mRootNode);
        
        // Node::Destroy should clear the world root node
        // Make sure derived class Destroy() is calling parent.
        OCT_ASSERT(mRootNode == nullptr);
        // Calling SetRootNode() just in case, but it's unnecessary.
        SetRootNode(nullptr);
    }
}

Node* World::FindNode(const std::string& name)
{
    Node* ret = nullptr;

    if (mRootNode != nullptr)
    {
        if (mRootNode->GetName() == name)
        {
            ret = mRootNode;
        }
        else
        {
            ret = mRootNode->FindChild(name, true);
        }
    }

    return ret;
}

Node* World::GetNetNode(NetId netId)
{
    Node* node = NetworkManager::Get()->GetNetNode(netId);

    if (node != nullptr &&
        node->GetWorld() != this)
    {
        // The net node exists, but it's not in the world.
        node = nullptr;
    }

    return node;
}

std::vector<Node*> World::FindNodesWithTag(const char* tag)
{
    std::vector<Node*> retNodes;

    if (mRootNode != nullptr)
    {
        auto gatherNodesWithTag = [&](Node* node) -> bool
        {
            if (node->HasTag(tag))
            {
                retNodes.push_back(node);
            }

            return true;
        };

        mRootNode->Traverse(gatherNodesWithTag);
    }

    return retNodes;
}

std::vector<Node*> World::FindNodesWithName(const char* name)
{
    std::vector<Node*> retNodes;

    if (mRootNode != nullptr)
    {
        auto gatherNodesWithName = [&](Node* node) -> bool
        {
            if (node->GetName() == name)
            {
                retNodes.push_back(node);
            }

            return true;
        };

        mRootNode->Traverse(gatherNodesWithName);
    }

    return retNodes;
}

std::vector<Node*> World::GatherNodes()
{
    // Return a flatted list of all the nodes in the scene.
    std::vector<Node*> nodeList;

    auto gatherNodes = [&](Node* node) -> bool
    {
        nodeList.push_back(node);
        return true;
    };

    if (mRootNode != nullptr)
    {
        mRootNode->Traverse(gatherNodes);
    }

    return nodeList;
}

void World::GatherNodes(std::vector<Node*>& outNodes)
{
    auto gatherNodes = [&](Node* node) -> bool
    {
        outNodes.push_back(node);
        return true;
    };

    if (mRootNode != nullptr)
    {
        mRootNode->Traverse(gatherNodes);
    }
}

void World::Clear()
{
    DestroyRootNode();
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

const std::vector<Light3D*>& World::GetLights()
{
    return mLights;
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
        Primitive3D* primA = mCurrentOverlaps[i].mPrimitiveA;
        Primitive3D* primB = mCurrentOverlaps[i].mPrimitiveB;

        if (primA == prim ||
            primB == prim)
        {
            primA->EndOverlap(primA, primB);
            mCurrentOverlaps.erase(mCurrentOverlaps.begin() + i);
        }
    }
}

void World::RayTest(glm::vec3 start, glm::vec3 end, uint8_t collisionMask, RayTestResult& outResult, uint32_t numIgnoredObjects, btCollisionObject** ignoreObjects)
{
    outResult.mStart = start;
    outResult.mEnd = end;

    btVector3 fromWorld = btVector3(start.x, start.y, start.z);
    btVector3 toWorld = btVector3(end.x, end.y, end.z);

    IgnoreRayResultCallback result(fromWorld, toWorld);
    result.m_collisionFilterGroup = (short)ColGroupAll;
    result.m_collisionFilterMask = collisionMask;
    result.mNumIgnoreObjects = numIgnoredObjects;
    result.mIgnoreObjects = ignoreObjects;

    //mDynamicsWorld->rayTestSingle()
    mDynamicsWorld->rayTest(fromWorld, toWorld, result);

    outResult.mHitPosition = { result.m_hitPointWorld.x(), result.m_hitPointWorld.y(), result.m_hitPointWorld.z() };
    outResult.mHitNormal = { result.m_hitNormalWorld.x(), result.m_hitNormalWorld.y(), result.m_hitNormalWorld.z() };
    outResult.mHitFraction = result.m_closestHitFraction;

    if (result.m_collisionObject != nullptr)
    {
        outResult.mHitNode = reinterpret_cast<Primitive3D*>(result.m_collisionObject->getUserPointer());
    }
    else
    {
        outResult.mHitNode = nullptr;
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
        outResult.mHitNodes.push_back(reinterpret_cast<Primitive3D*>(result.m_collisionObjects[i]->getUserPointer()));
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
        outResult.mHitNode = nullptr;
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
        outResult.mHitNode = reinterpret_cast<Primitive3D*>(result.m_hitCollisionObject->getUserPointer());
    }
    else
    {
        outResult.mHitNode = nullptr;
    }
}

void World::RegisterNode(Node* node)
{
    TypeId nodeType = node->GetType();

    // TODO: Now that components have become nodes, these static type checks don't hold up
    // if the user inherits from these nodes. Won't be a problem for Lua.
    if (nodeType == Audio3D::GetStaticType())
    {
#if _DEBUG
        OCT_ASSERT(std::find(mAudios.begin(), mAudios.end(), (Audio3D*)node) == mAudios.end());
#endif
        mAudios.push_back((Audio3D*)node);
    }
    else if (node->IsLight3D())
    {
#if _DEBUG
        OCT_ASSERT(std::find(mLights.begin(), mLights.end(), (Light3D*)node) == mLights.end());
#endif
        mLights.push_back((Light3D*)node);
    }
    else if (nodeType == Camera3D::GetStaticType())
    {
        if (mActiveCamera == nullptr ||
            mActiveCamera->IsEditorCamera())
        {
            mActiveCamera = node->As<Camera3D>();
        }
    }

    if (node->GetNetId() != INVALID_NET_ID)
    {
        AddNodeToRepVector(node);
    }
}

void World::UnregisterNode(Node* node)
{
    TypeId nodeType = node->GetType();

    if (nodeType == Audio3D::GetStaticType())
    {
        auto it = std::find(mAudios.begin(), mAudios.end(), (Audio3D*)node);
        OCT_ASSERT(it != mAudios.end());
        mAudios.erase(it);
    }
    else if (node->IsLight3D())
    {
        auto it = std::find(mLights.begin(), mLights.end(), (Light3D*)node);
        OCT_ASSERT(it != mLights.end());
        mLights.erase(it);
    }

    if (node == mAudioReceiver)
    {
        SetAudioReceiver(nullptr);
    }

    if (node == mActiveCamera)
    {
        SetActiveCamera(nullptr);
    }

    if (node->GetNetId() != INVALID_NET_ID)
    {
        RemoveNodeFromRepVector(node);
    }
}

const std::vector<Audio3D*>& World::GetAudios() const
{
    return mAudios;
}

std::vector<Node*>& World::GetReplicatedNodeVector(ReplicationRate rate)
{
    OCT_ASSERT(rate != ReplicationRate::Count);
    return mRepNodes[(uint32_t)rate];
}

uint32_t& World::GetReplicatedNodeIndex(ReplicationRate rate)
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

void World::AddNodeToRepVector(Node* node)
{
    std::vector<Node*>& repNodeVector = GetReplicatedNodeVector(node->GetReplicationRate());
    repNodeVector.push_back(node);
}

void World::RemoveNodeFromRepVector(Node* node)
{
    // Remove the destroyed actor from their assigned replication vector.
    std::vector<Node*>& repVector = GetReplicatedNodeVector(node->GetReplicationRate());
    uint32_t& repIndex = GetReplicatedNodeIndex(node->GetReplicationRate());

    for (uint32_t i = 0; i < repVector.size(); ++i)
    {
        if (repVector[i] == node)
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

void World::Update(float deltaTime)
{
    bool gameTickEnabled = IsGameTickEnabled();

    // Load any queued levels.
    if (mQueuedRootNode != nullptr)
    {
        Node* newRoot = mQueuedRootNode.Get<Node>();
        newRoot->Detach();

        DestroyRootNode();

        SetRootNode(newRoot);

        mQueuedRootNode = nullptr;
    }

    // Ensure world root node is set to replicate. (Otherwise clients will see nothing)
    // This might a heavy-handed approach but I don't want a developer to worry about needing to
    // set every one of their root scenes to replicate.
    if (NetIsServer())
    {
        if (mRootNode != nullptr &&
            !mRootNode->IsReplicated())
        {
            mRootNode->SetReplicate(true);
        }
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


                prim0->OnCollision(prim0, prim1, avgContactPoint0, avgNormal, manifold);
                prim1->OnCollision(prim1, prim0, avgContactPoint1, -avgNormal, manifold);
            }

            if (prim0->AreOverlapsEnabled() && prim1->AreOverlapsEnabled() &&
                std::find(mCurrentOverlaps.begin(), mCurrentOverlaps.end(), PrimitivePair(prim0, prim1)) == mCurrentOverlaps.end())
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
                pair.mPrimitiveA->BeginOverlap(pair.mPrimitiveA, pair.mPrimitiveB);
            }
        }

        // Call End Overlaps
        for (auto& pair : mPreviousOverlaps)
        {
            bool endOverlap = std::find(mCurrentOverlaps.begin(), mCurrentOverlaps.end(), pair) == mCurrentOverlaps.end();

            if (endOverlap)
            {
                pair.mPrimitiveA->EndOverlap(pair.mPrimitiveA, pair.mPrimitiveB);
            }
        }
    }

    UpdateLines(deltaTime);

    {
        SCOPED_FRAME_STAT("Tick");
        if (mRootNode != nullptr)
        {
            mRootNode->RecursiveTick(deltaTime, gameTickEnabled);
        }
    }

    {
        // TODO-NODE: Adding this! Make sure it works. I think we need to
        // make sure transforms are updated so that the bullet dynamics world is in sync.
        // But maybe not and we only need to update transforms when getting world pos/rot/scale/transform
        SCOPED_FRAME_STAT("Transforms");
        if (mRootNode != nullptr)
        {
            auto update3dTransform = [](Node* node) -> bool
            {
                if (node->IsNode3D())
                {
                    Node3D* node3d = (Node3D*)node;
                    if (node3d->IsTransformDirty())
                    {
                        node3d->UpdateTransform(false);
                    }
                }

                return true;
            };

            mRootNode->Traverse(update3dTransform);
        }
    }
}

Camera3D* World::GetActiveCamera()
{
#if EDITOR
    // When in editor, the active camera is the EditorCamera unless
    // we are playing in editor (and not ejected).
    if (!GetEditorState()->mPlayInEditor || 
        GetEditorState()->mEjected)
    {
        Camera3D* editorCam = GetEditorState()->GetEditorCamera();

        if (editorCam != nullptr)
        {
            OCT_ASSERT(editorCam->GetParent() == nullptr);
        }

        return editorCam;
    }
#endif

    return mActiveCamera;
}

Node3D* World::GetAudioReceiver()
{
    if (mAudioReceiver != nullptr)
    {
        return mAudioReceiver;
    }

    Camera3D* activeCam = GetActiveCamera();
    if (activeCam != nullptr)
    {
        return activeCam;
    }

    return nullptr;
}

void World::SetActiveCamera(Camera3D* activeCamera)
{
#if EDITOR
    if (activeCamera != GetEditorState()->mEditorCamera)
    {
        mActiveCamera = activeCamera;
    }
#else
    mActiveCamera = activeCamera;
#endif
}

void World::SetAudioReceiver(Node3D* newReceiver)
{
    mAudioReceiver = newReceiver;
}

void World::PlaceNewlySpawnedNode(Node* node)
{
    if (node != nullptr)
    {
        if (mRootNode != nullptr)
        {
            mRootNode->AddChild(node);
        }
        else
        {
            SetRootNode(node);
        }
    }
}

// These are pretty hacky... needed for doing raytests in the paint manager's mesh collision world
void World::OverrideDynamicsWorld(btDiscreteDynamicsWorld* world)
{
    mDynamicsWorld = world;
}

void World::RestoreDynamicsWorld()
{
    mDynamicsWorld = mDefaultDynamicsWorld;
}

Node* World::SpawnNode(TypeId actorType)
{
    Node* newNode = Node::Construct(actorType);

    if (newNode != nullptr)
    {
        PlaceNewlySpawnedNode(newNode);
    }
    else
    {
        LogError("Failed to spawn node with type: %d.", (int)actorType);
    }

    return newNode;
}

Node* World::SpawnNode(const char* typeName)
{
    Node* newNode = Node::Construct(typeName);

    if (newNode != nullptr)
    {
        PlaceNewlySpawnedNode(newNode);
    }
    else
    {
        LogError("Failed to spawn node with type name: %s.", typeName);
    }

    return newNode;
}

Node* World::SpawnScene(const char* sceneName)
{
    Scene* scene = LoadAsset<Scene>(sceneName);
    Node* newNode = scene ? scene->Instantiate() : nullptr;

    if (newNode != nullptr)
    {
        PlaceNewlySpawnedNode(newNode);
    }
    else
    {
        LogError("Failed to spawn scene with type: %s.", sceneName);
    }

    return newNode;
}

Particle3D* World::SpawnParticle(ParticleSystem* sys, glm::vec3 position)
{
    Particle3D* ret = SpawnNode<Particle3D>();
    ret->SetParticleSystem(sys);
    ret->SetPosition(position);
    ret->EnableEmission(true);
    ret->EnableAutoDestroy(true);

    return ret;
}

void World::DestroyNode(Node* node)
{
    if (node->GetWorld() == this)
    {
        Node::Destruct(node);
    }
}

void World::LoadScene(const char* name, bool instant)
{
    if (instant)
    {
        Scene* scene = LoadAsset<Scene>(name);

        if (scene != nullptr)
        {
            DestroyRootNode();

            Node* newRoot = scene->Instantiate();
            SetRootNode(newRoot);
        }
    }
    else
    {
        // Non-instant should be done if a script is loading a scene for instance.
        // We can cause problems if we destroy root tree while iterating.
        QueueRootScene(name);
    }
}

void World::QueueRootScene(const char* name)
{
    Scene* scene = LoadAsset<Scene>(name);

    if (scene != nullptr)
    {
        Node* sceneNode = scene->Instantiate();
        QueueRootNode(sceneNode);
    }
}

void World::QueueRootNode(Node* node)
{
    mQueuedRootNode = node;
}

void World::EnableInternalEdgeSmoothing(bool enable)
{
    gContactAddedCallback = enable ? ContactAddedHandler : nullptr;
}

bool World::IsInternalEdgeSmoothingEnabled() const
{
    return (gContactAddedCallback != nullptr);
}

void World::DirtyAllWidgets()
{
    if (mRootNode != nullptr)
    {
        auto dirtyWidget = [](Node* node) -> bool
        {
            if (node->IsWidget())
            {
                static_cast<Widget*>(node)->MarkDirty();
            }

            return true;
        };

        mRootNode->Traverse(dirtyWidget);
    }
}

void World::UpdateRenderSettings()
{
    Scene* srcScene = mRootNode ? mRootNode->GetScene() : nullptr;

    if (srcScene != nullptr)
    {
        srcScene->ApplyRenderSettings(this);
    }
    else
    {
        // Default render settings
        SetAmbientLightColor(DEFAULT_AMBIENT_LIGHT_COLOR);
        SetShadowColor(DEFAULT_SHADOW_COLOR);
        FogSettings fogSettings;
        SetFogSettings(fogSettings);
    }
}

Camera3D* World::SpawnDefaultCamera()
{
    if (mRootNode == nullptr)
    {
        SpawnDefaultRoot();
    }

    Camera3D* transCam = SpawnNode<Camera3D>();
    transCam->SetName("Default Camera");
    transCam->SetTransient(true);
    transCam->SetPosition(glm::vec3(0.0f, 0.0f, 10.0f));

    return transCam;
}

Node* World::SpawnDefaultRoot()
{
    if (mRootNode == nullptr)
    {
        mRootNode = SpawnNode<Node>();
        mRootNode->SetName("Default Root");
    }

    return mRootNode;
}
