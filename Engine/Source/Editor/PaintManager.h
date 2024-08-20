#pragma once

#if EDITOR

#include <unordered_map>

#include "Bullet/btBulletDynamicsCommon.h"
#include "Bullet/BulletCollision/CollisionDispatch/btGhostObject.h"

#include "Nodes/3D/StaticMesh3d.h"
#include "ActionManager.h"

struct PaintMeshCollision
{
    btCollisionObject* mCollisionObject = nullptr;
    StaticMesh3D* mNode = nullptr;
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

struct PaintColorOptions
{
    PaintBlendMode mBlendMode = PaintBlendMode::Mix;
    glm::vec4 mColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    bool mOnlyFacingNormals = true;
};

struct PaintInstanceOptions
{
    StaticMeshRef mMesh;
    float mDensity = 1.0f;
    float mMinSeparation = 0.0f;
    glm::vec3 mMinPosition = {};
    glm::vec3 mMaxPosition = {};
    glm::vec3 mMinRotation = {};
    glm::vec3 mMaxRotation = {};
    float mMinScale = 1.0f;
    float mMaxScale = 1.0f;
    bool mAlignWithNormal = false;
    bool mErase = false;
};

struct PendingColorData
{
    ActionSetInstanceColorsData mData;
    ActionSetInstanceColorsData mOriginalData;
    std::vector<uint32_t> mOriginalColors;
    std::vector<float> mVertexDrawAlpha;
    bool mAnyVertexPainted = false;
};

struct PendingInstanceData
{
    InstancedMesh3D* mMeshNode = nullptr;
    std::vector<MeshInstanceData> mData;
    std::vector<MeshInstanceData> mOriginalData;
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
    void UpdateHotkeys();
    void UpdatePaintReticle();
    void UpdatePaintDraw();

    void FinishAdjustment();

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
    glm::vec3 mSphereNormal = { 0.0f, 1.0f, 0.0f };
    float mRadius = 1.0f;
    float mOpacity = 1.0f;
    bool mSphereValid = false;

    float mPreAdjustRadius = 1.0f;
    float mPreAdjustOpacity = 1.0f;

    bool mAdjustRadius = false;
    bool mAdjustOpacity = false;
    bool mAdjustmentFinished = false;
    glm::vec2 mAdjustmentAnchor = {};

    bool mOnlyRenderSelected = false;

    // Spacing is the distance we need to move between paints (in screen space currently)
    float mSpacing = 10.0f;
    glm::vec2 mLastPaintMousePos = {};

    // Options
    PaintColorOptions mColorOptions;
    PaintInstanceOptions mInstanceOptions;

    // Pending data
    std::vector<PendingColorData> mPendingColorData;
    PendingInstanceData mPendingInstanceData;
};

#endif
