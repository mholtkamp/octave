#pragma once

#if EDITOR

#include <unordered_map>

#include <Bullet/btBulletDynamicsCommon.h>

#include "Nodes/3D/StaticMesh3d.h"


struct PaintMeshCollision
{
    btCollisionObject* mCollisionObject = nullptr;
    StaticMeshRef mMesh;
    glm::vec3 mPosition = {};
    glm::quat mRotation = {};
    glm::vec3 mScale = {};
};

class PaintManager
{
public:

    PaintManager();
    ~PaintManager();
    void Update();
    void HandleNodeDestroy(Node* node);

    void AddPaintMeshCollision(const PaintMeshCollision& col);
    void RemovePaintMeshCollision(const PaintMeshCollision& col);

    void UpdateDynamicsWorld();
    void UpdatePaintReticle();

    bool mInitialized = false;

    btDefaultCollisionConfiguration* mCollisionConfig = nullptr;
    btCollisionDispatcher* mCollisionDispatcher = nullptr;
    btDbvtBroadphase* mBroadphase = nullptr;
    btSequentialImpulseConstraintSolver* mSolver = nullptr;
    btDiscreteDynamicsWorld* mDynamicsWorld = nullptr;

    std::unordered_map<StaticMesh3D*, PaintMeshCollision> mMeshCollisionMap;
};

#endif
