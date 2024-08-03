#pragma once

#if EDITOR

#include <unordered_map>

#include "Bullet/btBulletDynamicsCommon.h"
#include "Bullet/BulletCollision/CollisionDispatch/btGhostObject.h"

#include "Nodes/3D/StaticMesh3d.h"


struct PaintMeshCollision
{
    btCollisionObject* mCollisionObject = nullptr;
    StaticMeshRef mMesh;
    glm::vec3 mPosition = {};
    glm::quat mRotation = {};
    glm::vec3 mScale = {};
    bool mActive = false;
};

enum PaintBlendMode
{
    Mix,
    Add,
    Subtract,
    Multiply,
    AddAlpha,
    SubtractAlpha,

    Count
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
    void UpdatePaintDraw();

    bool mInitialized = false;

    btDefaultCollisionConfiguration* mCollisionConfig = nullptr;
    btCollisionDispatcher* mCollisionDispatcher = nullptr;
    btDbvtBroadphase* mBroadphase = nullptr;
    btSequentialImpulseConstraintSolver* mSolver = nullptr;
    btDiscreteDynamicsWorld* mDynamicsWorld = nullptr;

    std::unordered_map<StaticMesh3D*, PaintMeshCollision> mMeshCollisionMap;

    btPairCachingGhostObject* mSphereGhost = nullptr;
    btSphereShape* mSphereGhostShape = nullptr;
    btGhostPairCallback* mGhostPairCallback = nullptr;

    StaticMeshRef mSphereMesh;
    MaterialRef mSphereMaterial;

    glm::vec3 mSpherePosition = {};
    float mRadius = 1.0f;
    float mOpacity = 1.0f;

    float mPreAdjustRadius = 1.0f;
    float mPreAdjustOpacity = 1.0f;

    bool mAdjustRadius = false;
    bool mAdjustOpacity = false;
    bool mAdjustmentFinished = false;
    glm::vec2 mAdjustmentAnchor = {};

    // Spacing is the distance we need to move between paints (in world space currently)
    float mSpacing = 1.0f;
    glm::vec3 mLastPaintPosition = {};

    // Vertex Color Options
    PaintBlendMode mBlendMode = PaintBlendMode::Mix;
    glm::vec4 mColor = { 0.0f, 1.0f, 0.0f, 1.0f };
};

#endif
