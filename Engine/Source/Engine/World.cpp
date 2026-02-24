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
#include "Engine.h"
#include "Assets/Scene.h"
#include "Assets/StaticMesh.h"
#include "Nodes/3D/StaticMesh3d.h"
#include "Nodes/3D/NavMesh3d.h"
#include "Nodes/3D/PointLight3d.h"
#include "Nodes/3D/Particle3d.h"
#include "Nodes/3D/Audio3d.h"

#if EDITOR
#include "Editor/EditorState.h"
#endif

#include <map>
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <cctype>
#include <memory>

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btInternalEdgeUtility.h>
#include <Bullet/BulletCollision/CollisionShapes/btTriangleShape.h>

#include "../../../External/recastnavigation/Recast/Include/Recast.h"
#include "../../../External/recastnavigation/Detour/Include/DetourNavMesh.h"
#include "../../../External/recastnavigation/Detour/Include/DetourNavMeshBuilder.h"
#include "../../../External/recastnavigation/Detour/Include/DetourNavMeshQuery.h"

using namespace std;

namespace
{
    struct RecastNavData
    {
        dtNavMesh* mNavMesh = nullptr;
        dtNavMeshQuery* mQuery = nullptr;

        ~RecastNavData()
        {
            if (mQuery) dtFreeNavMeshQuery(mQuery);
            if (mNavMesh) dtFreeNavMesh(mNavMesh);
            mQuery = nullptr;
            mNavMesh = nullptr;
        }
    };

    static bool BuildRecastNavData(World* world, RecastNavData& outData);

    static std::unordered_map<World*, std::unique_ptr<RecastNavData>> sWorldNavCache;\n    static void InvalidateWorldNavCache(World* world)
    {
        sWorldNavCache.erase(world);
    }

    static RecastNavData* GetOrBuildWorldNav(World* world)
    {
        auto it = sWorldNavCache.find(world);
        if (it != sWorldNavCache.end() && it->second && it->second->mQuery)
        {
            return it->second.get();
        }

        std::unique_ptr<RecastNavData> nav = std::make_unique<RecastNavData>();
        if (!BuildRecastNavData(world, *nav))
        {
            return nullptr;
        }

        RecastNavData* ret = nav.get();
        sWorldNavCache[world] = std::move(nav);
        return ret;
    }

    static bool GatherNavTriangles(World* world, std::vector<float>& outVerts, std::vector<int>& outTris)
    {
        std::vector<StaticMesh3D*> navMeshes;
        world->FindNodes<StaticMesh3D>(navMeshes);

        std::vector<NavMesh3D*> navBounds;
        world->FindNodes<NavMesh3D>(navBounds);

        struct NavBoundsEntry
        {
            glm::mat4 invTransform;
            glm::vec3 halfExtents;
            bool overlay = false;
            bool negator = false;
            bool cullWalls = false;
            float wallCullThreshold = 0.2f;
        };
        std::vector<NavBoundsEntry> activeBounds;

        for (NavMesh3D* navBox : navBounds)
        {
            if (navBox == nullptr || !navBox->IsNavBounds())
            {
                continue;
            }

            NavBoundsEntry e;
            e.invTransform = glm::inverse(navBox->GetTransform());
            e.halfExtents = navBox->GetExtents() * 0.5f;
            e.overlay = navBox->IsNavOverlayEnabled();
            e.negator = navBox->IsNavNegatorEnabled();
            e.cullWalls = navBox->IsCullWallsEnabled();
            e.wallCullThreshold = navBox->GetWallCullThreshold();
            activeBounds.push_back(e);
        }

        auto pointInsideAnyBounds = [&](const glm::vec3& p) -> bool
        {
            if (activeBounds.empty())
            {
                return false;
            }

            bool hasPositiveBounds = false;
            bool insidePositiveBounds = false;
            bool insideNegator = false;

            for (const NavBoundsEntry& e : activeBounds)
            {
                glm::vec4 lp4 = e.invTransform * glm::vec4(p, 1.0f);
                glm::vec3 lp(lp4.x, lp4.y, lp4.z);
                const bool inside = (fabsf(lp.x) <= e.halfExtents.x &&
                                     fabsf(lp.y) <= e.halfExtents.y &&
                                     fabsf(lp.z) <= e.halfExtents.z);
                if (!inside)
                {
                    continue;
                }

                if (e.negator)
                {
                    insideNegator = true;
                }
                else
                {
                    insidePositiveBounds = true;
                }
            }

            for (const NavBoundsEntry& e : activeBounds)
            {
                if (!e.negator)
                {
                    hasPositiveBounds = true;
                    break;
                }
            }

            if (insideNegator)
            {
                return false;
            }

            if (hasPositiveBounds)
            {
                return insidePositiveBounds;
            }

            // No positive NavMesh3D bounds available yet.
            return false;
        };`r`n
        auto pointInsideCullWallsBounds = [&](const glm::vec3& p, float& outThreshold) -> bool
        {
            if (activeBounds.empty())
            {
                return false;
            }

            bool found = false;
            float bestThreshold = 0.2f;

            for (const NavBoundsEntry& e : activeBounds)
            {
                if (!e.cullWalls)
                {
                    continue;
                }

                glm::vec4 lp4 = e.invTransform * glm::vec4(p, 1.0f);
                glm::vec3 lp(lp4.x, lp4.y, lp4.z);
                if (fabsf(lp.x) <= e.halfExtents.x &&
                    fabsf(lp.y) <= e.halfExtents.y &&
                    fabsf(lp.z) <= e.halfExtents.z)
                {
                    if (!found || e.wallCullThreshold > bestThreshold)
                    {
                        bestThreshold = e.wallCullThreshold;
                    }
                    found = true;
                }
            }

            if (found)
            {
                outThreshold = bestThreshold;
            }
            return found;
        };

        for (StaticMesh3D* meshNode : navMeshes)
        {
            if (meshNode == nullptr || !meshNode->IsNavmeshReady())
            {
                continue;
            }

            StaticMesh* staticMesh = meshNode->GetStaticMesh();
            if (staticMesh == nullptr)
            {
                continue;
            }

            Vertex* vertices = staticMesh->GetVertices();
            IndexType* indices = staticMesh->GetIndices();
            const uint32_t numVertices = staticMesh->GetNumVertices();
            const uint32_t numIndices = staticMesh->GetNumIndices();

            if (!vertices || !indices || numVertices == 0 || numIndices < 3)
            {
                continue;
            }

            const glm::mat4 transform = meshNode->GetTransform();

            // Build world-space vertices for this mesh and only emit triangles that
            // fall inside at least one Nav Bounds box (if any are enabled).
            std::vector<glm::vec3> worldVerts;
            worldVerts.resize(numVertices);
            for (uint32_t i = 0; i < numVertices; ++i)
            {
                glm::vec4 wp = transform * glm::vec4(vertices[i].mPosition, 1.0f);
                worldVerts[i] = glm::vec3(wp.x, wp.y, wp.z);
            }

            for (uint32_t i = 0; i + 2 < numIndices; i += 3)
            {
                const uint32_t ia = (uint32_t)indices[i + 0];
                const uint32_t ib = (uint32_t)indices[i + 1];
                const uint32_t ic = (uint32_t)indices[i + 2];
                if (ia >= numVertices || ib >= numVertices || ic >= numVertices)
                {
                    continue;
                }

                const glm::vec3& a = worldVerts[ia];
                const glm::vec3& b = worldVerts[ib];
                const glm::vec3& c = worldVerts[ic];
                const glm::vec3 centroid = (a + b + c) / 3.0f;

                if (!pointInsideAnyBounds(centroid))
                {
                    continue;
                }

                float wallCullThreshold = 0.2f;
                if (pointInsideCullWallsBounds(centroid, wallCullThreshold))
                {
                    glm::vec3 n = glm::cross(b - a, c - a);
                    float nlen2 = glm::dot(n, n);
                    if (nlen2 > 1e-8f)
                    {
                        n = glm::normalize(n);
                        // Cull near-vertical faces ("90 degree walls")
                        if (fabsf(n.y) < wallCullThreshold)
                        {
                            continue;
                        }
                    }
                }

                const int baseVert = (int)(outVerts.size() / 3);
                outVerts.push_back(a.x); outVerts.push_back(a.y); outVerts.push_back(a.z);
                outVerts.push_back(b.x); outVerts.push_back(b.y); outVerts.push_back(b.z);
                outVerts.push_back(c.x); outVerts.push_back(c.y); outVerts.push_back(c.z);

                outTris.push_back(baseVert + 0);
                outTris.push_back(baseVert + 1);
                outTris.push_back(baseVert + 2);`r`n            }
        }

        return !outVerts.empty() && !outTris.empty();
    }

    static bool BuildRecastNavData(World* world, RecastNavData& outData)
    {
        std::vector<float> verts;
        std::vector<int> tris;
        if (!GatherNavTriangles(world, verts, tris))
        {
            return false;
        }

        float bmin[3], bmax[3];
        rcCalcBounds(verts.data(), (int)verts.size() / 3, bmin, bmax);

        rcConfig cfg{};
        cfg.cs = 0.3f;
        cfg.ch = 0.2f;
        cfg.walkableSlopeAngle = 55.0f;
        cfg.walkableHeight = (int)ceilf(2.0f / cfg.ch);
        cfg.walkableClimb = (int)floorf(0.9f / cfg.ch);
        cfg.walkableRadius = (int)ceilf(0.4f / cfg.cs);
        cfg.maxEdgeLen = (int)(12.0f / cfg.cs);
        cfg.maxSimplificationError = 1.3f;
        cfg.minRegionArea = (int)rcSqr(8);
        cfg.mergeRegionArea = (int)rcSqr(20);
        cfg.maxVertsPerPoly = 6;
        cfg.detailSampleDist = 6.0f;
        cfg.detailSampleMaxError = 1.0f;

        rcVcopy(cfg.bmin, bmin);
        rcVcopy(cfg.bmax, bmax);
        rcCalcGridSize(cfg.bmin, cfg.bmax, cfg.cs, &cfg.width, &cfg.height);

        rcContext ctx(false);
        rcHeightfield* solid = rcAllocHeightfield();
        if (!solid) return false;

        bool ok = false;
        unsigned char* triAreas = nullptr;
        rcCompactHeightfield* chf = nullptr;
        rcContourSet* cset = nullptr;
        rcPolyMesh* pmesh = nullptr;
        rcPolyMeshDetail* dmesh = nullptr;

        do {
            if (!rcCreateHeightfield(&ctx, *solid, cfg.width, cfg.height, cfg.bmin, cfg.bmax, cfg.cs, cfg.ch)) break;

            const int ntris = (int)tris.size() / 3;
            triAreas = new unsigned char[ntris];
            memset(triAreas, 0, ntris * sizeof(unsigned char));
            rcMarkWalkableTriangles(&ctx, cfg.walkableSlopeAngle, verts.data(), (int)verts.size() / 3, tris.data(), ntris, triAreas);
            if (!rcRasterizeTriangles(&ctx, verts.data(), (int)verts.size() / 3, tris.data(), triAreas, ntris, *solid, cfg.walkableClimb)) break;

            rcFilterLowHangingWalkableObstacles(&ctx, cfg.walkableClimb, *solid);
            rcFilterLedgeSpans(&ctx, cfg.walkableHeight, cfg.walkableClimb, *solid);
            rcFilterWalkableLowHeightSpans(&ctx, cfg.walkableHeight, *solid);

            chf = rcAllocCompactHeightfield();
            if (!chf) break;
            if (!rcBuildCompactHeightfield(&ctx, cfg.walkableHeight, cfg.walkableClimb, *solid, *chf)) break;

            if (!rcErodeWalkableArea(&ctx, cfg.walkableRadius, *chf)) break;
            if (!rcBuildDistanceField(&ctx, *chf)) break;
            if (!rcBuildRegions(&ctx, *chf, 0, cfg.minRegionArea, cfg.mergeRegionArea)) break;

            cset = rcAllocContourSet();
            if (!cset) break;
            if (!rcBuildContours(&ctx, *chf, cfg.maxSimplificationError, cfg.maxEdgeLen, *cset)) break;

            pmesh = rcAllocPolyMesh();
            if (!pmesh) break;
            if (!rcBuildPolyMesh(&ctx, *cset, cfg.maxVertsPerPoly, *pmesh)) break;

            dmesh = rcAllocPolyMeshDetail();
            if (!dmesh) break;
            if (!rcBuildPolyMeshDetail(&ctx, *pmesh, *chf, cfg.detailSampleDist, cfg.detailSampleMaxError, *dmesh)) break;

            for (int i = 0; i < pmesh->npolys; ++i)
            {
                pmesh->flags[i] = 1;
                pmesh->areas[i] = 0;
            }

            dtNavMeshCreateParams params{};
            params.verts = pmesh->verts;
            params.vertCount = pmesh->nverts;
            params.polys = pmesh->polys;
            params.polyAreas = pmesh->areas;
            params.polyFlags = pmesh->flags;
            params.polyCount = pmesh->npolys;
            params.nvp = pmesh->nvp;
            params.detailMeshes = dmesh->meshes;
            params.detailVerts = dmesh->verts;
            params.detailVertsCount = dmesh->nverts;
            params.detailTris = dmesh->tris;
            params.detailTriCount = dmesh->ntris;
            params.walkableHeight = 2.0f;
            params.walkableRadius = 0.4f;
            params.walkableClimb = 0.9f;
            rcVcopy(params.bmin, pmesh->bmin);
            rcVcopy(params.bmax, pmesh->bmax);
            params.cs = cfg.cs;
            params.ch = cfg.ch;
            params.buildBvTree = true;

            unsigned char* navData = nullptr;
            int navDataSize = 0;
            if (!dtCreateNavMeshData(&params, &navData, &navDataSize)) break;

            outData.mNavMesh = dtAllocNavMesh();
            if (!outData.mNavMesh) break;
            if (dtStatusFailed(outData.mNavMesh->init(navData, navDataSize, DT_TILE_FREE_DATA))) break;

            outData.mQuery = dtAllocNavMeshQuery();
            if (!outData.mQuery) break;
            if (dtStatusFailed(outData.mQuery->init(outData.mNavMesh, 2048))) break;

            ok = true;
        } while (false);

        delete[] triAreas;
        rcFreePolyMeshDetail(dmesh);
        rcFreePolyMesh(pmesh);
        rcFreeContourSet(cset);
        rcFreeCompactHeightfield(chf);
        rcFreeHeightField(solid);

        return ok;
    }
}

std::unordered_set<NodePtrWeak> World::sNewlyRegisteredNodes;

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
    InvalidateWorldNavCache(this);
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

Node* World::GetRootNode()
{
    return mRootNode.Get();
}

NodePtr World::GetRootNodePtr()
{
    return mRootNode;
}

void World::SetRootNode(Node* node)
{
    if (mRootNode != node)
    {
        InvalidateWorldNavCache(this);
        if (mRootNode != nullptr)
        {
            if (IsPlaying())
            {
                ExtractPersistingNodes();
            }

            mRootNode->SetWorld(nullptr, true);
        }

        mRootNode = ResolvePtr(node);

        if (mRootNode != nullptr && mRootNode->IsDestroyed())
        {
            LogWarning("Failed to set root node. Node is destroyed.");
            mRootNode = nullptr;
        }

        if (mRootNode != nullptr)
        {
            // The new root node should not be attached to any other node.
            if (mRootNode->GetParent() != nullptr)
            {
                mRootNode->Detach();
            }

            // If this node is already a root node for a different world, first
            // clear it as the root node in that world.
            World* prevWorld = mRootNode->GetWorld();

            // This shouldn't be possible.
            OCT_ASSERT(prevWorld != this);

            if (prevWorld &&
                prevWorld->GetRootNode() == mRootNode.Get())
            {
                prevWorld->SetRootNode(nullptr);
            }

            // The new root node should no longer be associated with a world.
            OCT_ASSERT(mRootNode->GetWorld() == nullptr);

            mRootNode->SetWorld(this, true);
        }

        if (mRootNode != nullptr)
        {
            if (IsPlaying())
            {
                for (auto& persNode : mPersistingNodes)
                {
                    mRootNode->AddChild(persNode);
                }
            }

            mPersistingNodes.clear();
        }

        UpdateRenderSettings();
    }
}

void World::DestroyRootNode()
{
    if (mRootNode != nullptr)
    {
        if (IsPlaying())
        {
            ExtractPersistingNodes();
        }

        mRootNode->Destroy();
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
            ret = mRootNode.Get();
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

bool World::FindNavPath(glm::vec3 start, glm::vec3 end, std::vector<glm::vec3>& outPath)
{
    outPath.clear();

    RecastNavData* nav = GetOrBuildWorldNav(this);
    if (!nav || !nav->mQuery)
    {
        return false;
    }

    const float ext[3] = { 2.0f, 4.0f, 2.0f };
    const float startPt[3] = { start.x, start.y, start.z };
    const float endPt[3] = { end.x, end.y, end.z };

    dtQueryFilter filter;
    filter.setIncludeFlags(0xffff);
    filter.setExcludeFlags(0);

    dtPolyRef startRef = 0;
    dtPolyRef endRef = 0;
    float nearestStart[3];
    float nearestEnd[3];

    if (dtStatusFailed(nav->mQuery->findNearestPoly(startPt, ext, &filter, &startRef, nearestStart)) || startRef == 0)
    {
        return false;
    }

    if (dtStatusFailed(nav->mQuery->findNearestPoly(endPt, ext, &filter, &endRef, nearestEnd)) || endRef == 0)
    {
        return false;
    }

    dtPolyRef polys[2048];
    int npolys = 0;
    if (dtStatusFailed(nav->mQuery->findPath(startRef, endRef, nearestStart, nearestEnd, &filter, polys, &npolys, 2048)) || npolys <= 0)
    {
        return false;
    }

    float straight[2048 * 3];
    unsigned char straightFlags[2048];
    dtPolyRef straightPolys[2048];
    int nstraight = 0;

    if (dtStatusFailed(nav->mQuery->findStraightPath(nearestStart, nearestEnd, polys, npolys,
        straight, straightFlags, straightPolys, &nstraight, 2048, DT_STRAIGHTPATH_ALL_CROSSINGS)) || nstraight <= 0)
    {
        return false;
    }

    outPath.reserve((size_t)nstraight);
    for (int i = 0; i < nstraight; ++i)
    {
        outPath.push_back(glm::vec3(straight[i * 3 + 0], straight[i * 3 + 1], straight[i * 3 + 2]));
    }

    return !outPath.empty();
}

bool World::FindRandomNavPoint(glm::vec3& outPoint)
{
    RecastNavData* nav = GetOrBuildWorldNav(this);
    if (!nav || !nav->mQuery)
    {
        return false;
    }

    dtQueryFilter filter;
    filter.setIncludeFlags(0xffff);
    filter.setExcludeFlags(0);

    dtPolyRef ref = 0;
    float pt[3] = {};
    auto frand = []() -> float { return Maths::RandRange(0.0f, 1.0f); };

    if (dtStatusFailed(nav->mQuery->findRandomPoint(&filter, frand, &ref, pt)) || ref == 0)
    {
        return false;
    }

    outPoint = glm::vec3(pt[0], pt[1], pt[2]);
    return true;
}

bool World::FindClosestNavPoint(glm::vec3 inPoint, glm::vec3& outPoint)
{
    RecastNavData* nav = GetOrBuildWorldNav(this);
    if (!nav || !nav->mQuery)
    {
        return false;
    }

    dtQueryFilter filter;
    filter.setIncludeFlags(0xffff);
    filter.setExcludeFlags(0);

    const float ext[3] = { 2.0f, 4.0f, 2.0f };
    const float inPt[3] = { inPoint.x, inPoint.y, inPoint.z };
    dtPolyRef ref = 0;
    float outPt[3] = {};

    if (dtStatusFailed(nav->mQuery->findNearestPoly(inPt, ext, &filter, &ref, outPt)) || ref == 0)
    {
        return false;
    }

    outPoint = glm::vec3(outPt[0], outPt[1], outPt[2]);
    return true;
}



void World::Clear()
{
    InvalidateWorldNavCache(this);
    DestroyRootNode();
}

int32_t World::GetIndex() const
{
    int32_t worldIdx = -1;
    int32_t numWorlds = GetNumWorlds();

    for (int32_t i = 0; i < numWorlds; ++i)
    {
        if (::GetWorld(i) == this)
        {
            worldIdx = i;
            break;
        }
    }

    return worldIdx;
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

std::vector<FadingLight>& World::GetFadingLights()
{
    return mFadingLights;
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

void World::RayTest(glm::vec3 start, glm::vec3 end, uint8_t collisionMask, RayTestResult& outResult, uint32_t numIgnoredObjects, btCollisionObject** ignoreObjects, bool ignorePureOverlap)
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
    result.mIgnorePureOverlap = ignorePureOverlap;

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

void World::RayTestMulti(glm::vec3 start, glm::vec3 end, uint8_t collisionMask, bool ignorePureOverlap, RayTestMultiResult& outResult)
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
        if (ignorePureOverlap && (result.m_collisionObjects[i]->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE))
            continue;

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

void World::RegisterNode(Node* node, bool subRoot)
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

    if (subRoot)
    {
        sNewlyRegisteredNodes.insert(ResolveWeakPtr(node));
    }
}

void World::UnregisterNode(Node* node, bool subRoot)
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

    if (subRoot)
    {
        sNewlyRegisteredNodes.erase(ResolveWeakPtr(node));
    }
}

const std::vector<Audio3D*>& World::GetAudios() const
{
    return mAudios;
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

void World::ExtractPersistingNodes()
{
    // Extract persistent nodes that will be added to world when next root node is set
    mRootNode->Traverse(
        [this](Node* node) -> bool
        {
            if (node->IsPersistent())
            {
                mPersistingNodes.push_back(ResolvePtr(node));
                node->Detach();
                return false;
            }

            return true;
        },
        true);
}

void World::Update(float deltaTime)
{
    bool gameTickEnabled = IsGameTickEnabled();

    // Load any queued levels.
    if (mQueuedRootNode != nullptr)
    {
        mQueuedRootNode->Detach();

        DestroyRootNode();

        SetRootNode(mQueuedRootNode.Get());

        mQueuedRootNode.Reset();
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

        // Check the number of manifolds each loop iteration, since an overlap/collision callbacks
        // may reduce the number of manifolds if an collision object gets removed from the dynamics world.
        btDispatcher* dispatcher = mDynamicsWorld->getDispatcher();
        for (int32_t i = 0; i < dispatcher->getNumManifolds(); ++i)
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

                prim0->EmitSignal("OnCollision", {prim0, prim1, avgContactPoint0, avgNormal});
                prim1->EmitSignal("OnCollision", {prim1, prim0, avgContactPoint1, -avgNormal});
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
                pair.mPrimitiveA->EmitSignal("BeginOverlap", {pair.mPrimitiveA, pair.mPrimitiveB});
            }
        }

        // Call End Overlaps
        for (auto& pair : mPreviousOverlaps)
        {
            bool endOverlap = std::find(mCurrentOverlaps.begin(), mCurrentOverlaps.end(), pair) == mCurrentOverlaps.end();

            if (endOverlap)
            {
                pair.mPrimitiveA->EndOverlap(pair.mPrimitiveA, pair.mPrimitiveB);
                pair.mPrimitiveA->EmitSignal("EndOverlap", {pair.mPrimitiveA, pair.mPrimitiveB});
            }
        }
    }

    UpdateLines(deltaTime);

    {
        SCOPED_FRAME_STAT("Tick");
        if (mRootNode != nullptr)
        {
            // To allow dynamically removing / reparenting nodes, gather up all the nodes that
            // need to tick first, then iterate over them and call tick if they are valid / not destroyed.
            static std::vector<NodePtrWeak> sNodesToTick;
            sNodesToTick.clear();

            mRootNode->PrepareTick(sNodesToTick, gameTickEnabled, true);

            // Setting a limit of 10 iterations. If we go over this, there is
            // likely an infinite chain of node creation
            const int32_t kMaxTickIterations = 10;
            int32_t tickIteration = 0;
            uint32_t currentFrame = GetEngineState()->mFrameNumber;

            // Tick all of the nodes that need to be ticked, and then keep iterating
            // until all newly spawned nodes / added nodes have ticked (and maybe start)
            while (sNodesToTick.size() > 0 && tickIteration < kMaxTickIterations)
            {
                for (uint32_t i = 0; i < sNodesToTick.size(); ++i)
                {
                    Node* node = sNodesToTick[i].Get();

                    // Node may have been destroyed or removed from the world
                    if (node &&
                        node->GetWorld() == this &&
                        node->GetLastTickedFrame() != currentFrame)
                    {
                        if (gameTickEnabled)
                        {
                            node->Tick(deltaTime);
                        }
                        else
                        {
                            node->EditorTick(deltaTime);
                        }
                    }
                }

                tickIteration++;
                sNodesToTick.clear();

                if (sNewlyRegisteredNodes.size() > 0)
                {
                    // Make a copy otherwise sNewlyRegisteredNodes might get altered while 
                    // we are calling PrepareTick()
                    std::unordered_set<NodePtrWeak> newNodes = sNewlyRegisteredNodes;
                    sNewlyRegisteredNodes.clear();

                    for (const NodePtrWeak& nodePtr : newNodes)
                    {
                        if (nodePtr.IsValid() &&
                            nodePtr->GetWorld() == this &&
                            nodePtr->GetLastTickedFrame() != currentFrame)
                        {
                            nodePtr->PrepareTick(sNodesToTick, gameTickEnabled, true);
                        }
                    }
                }


                if (tickIteration == kMaxTickIterations)
                {
                    LogWarning("Reached tick iteration limit");
                }
            }
        }
    }

    Node::ProcessPendingDestroys();

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

Camera3D* World::GetMainCamera()
{
    std::vector<Camera3D*> cams;
    FindNodes(cams);
	Camera3D* matchCamera = nullptr;
	Camera3D* highestPriority = nullptr;
    for (Camera3D* cam : cams)
    {
        if (highestPriority == nullptr || cam->GetPriority() > highestPriority->GetPriority()) {
            if (cam->GetIsMainCamera() && matchCamera == nullptr)
            {
                matchCamera = cam;
            }
        }
    }
    if (matchCamera != nullptr)
    {
        return matchCamera;
	}
	return highestPriority;
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
    if (GetEditorState()->mEditorCamera != activeCamera)
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

void World::PlaceNewlySpawnedNode(NodePtr node, glm::vec3 position)
{
    if (node != nullptr)
    {
        if (mRootNode != nullptr)
        {
            mRootNode->AddChild(node.Get());

            if (position != glm::vec3(0.0f, 0.0f, 0.0f))
            {
                Node3D* node3d = Cast<Node3D>(node.Get());
                if (node3d)
                {
                    node3d->SetWorldPosition(position);
                    node3d->UpdateTransform(true);
                }
            }
        }
        else
        {
            SetRootNode(node.Get());
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

Node* World::SpawnNode(TypeId actorType, glm::vec3 position)
{
    NodePtr newNode = Node::Construct(actorType);

    if (newNode != nullptr)
    {
        PlaceNewlySpawnedNode(newNode, position);
    }
    else
    {
        LogError("Failed to spawn node with type: %d.", (int)actorType);
    }

    // Safe to return a pointer here since the newly constructed
    // node should be a child in the world's node tree now.
    return newNode.Get();
}

Node* World::SpawnNode(const char* typeName, glm::vec3 position)
{
    NodePtr newNode = Node::Construct(typeName);

    if (newNode != nullptr)
    {
        PlaceNewlySpawnedNode(newNode, position);
    }
    else
    {
        LogError("Failed to spawn node with type name: %s.", typeName);
    }

    return newNode.Get();
}

Node* World::SpawnScene(const char* sceneName, glm::vec3 position)
{
    Scene* scene = LoadAsset<Scene>(sceneName);
    return SpawnScene(scene, position);
}

Node* World::SpawnScene(Scene* scene, glm::vec3 position)
{
    NodePtr newNode = scene ? scene->Instantiate() : nullptr;

    if (newNode != nullptr)
    {
        PlaceNewlySpawnedNode(newNode, position);
    }
    else
    {
        LogError("Failed to spawn scene with type: %s.", scene->GetName());
    }

    return newNode.Get();
}

Particle3D* World::SpawnParticle(ParticleSystem* sys, glm::vec3 position)
{
    Particle3D* ret = nullptr;

    if (sys != nullptr)
    {
        ret = SpawnNode<Particle3D>();
        ret->SetParticleSystem(sys);
        ret->SetPosition(position);
        ret->EnableEmission(true);
        ret->EnableAutoDestroy(true);
    }

    return ret;
}

void World::LoadScene(const char* name, bool instant)
{
    if (instant)
    {
        Scene* scene = LoadAsset<Scene>(name);

        if (scene != nullptr)
        {
            DestroyRootNode();

            NodePtr newRoot = scene->Instantiate();
            SetRootNode(newRoot.Get());
        }
        else
        {
            LogError("Failed to load scene %s", name);
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
        NodePtr sceneNode = scene->Instantiate();
        QueueRootNode(sceneNode.Get());
    }
    else
    {
        LogError("Failed to load scene %s", name);
    }
}

void World::QueueRootNode(Node* node)
{
    mQueuedRootNode = ResolvePtr(node);
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

void World::AddNewlyRegisteredNode(Node* node)
{
    if (node != nullptr)
    {
        // This should really only be called for Widgets when they are first made visible
        sNewlyRegisteredNodes.insert(ResolveWeakPtr(node));
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
        mRootNode = ResolvePtr(SpawnNode<Node>());
        mRootNode->SetName("Default Root");
    }

    return mRootNode.Get();
}



















