#pragma once

#include <string>
#include <string.h>

#include "Constants.h"
#include "Maths.h"

#include "System/SystemTypes.h"
#include "Graphics/GraphicsTypes.h"
#include "Input/InputTypes.h"

#include <Bullet/BulletCollision/CollisionDispatch/btCollisionWorld.h>

#if LUA_ENABLED
#include <Lua/lua.hpp>
#endif

class Level;
class Primitive3D;
class Node3D;
class Node;

class StaticMesh;
class Material;

typedef uint32_t TypeId;
typedef uint32_t NetId;
typedef uint64_t RuntimeId;

// Bullet Types
class btDynamicsWorld;
struct btDbvtBroadphase;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btRigidBody;
class btCollisionShape;
class btBvhTriangleMeshShape;
class btTriangleIndexVertexArray;
struct btTriangleInfoMap;

enum class CollisionShape : uint32_t
{
    Empty,
    Box,
    Sphere,
    Capsule,
    TriangleMesh,
    ScaledTriangleMesh,
    ConvexHull,
    Compound,

    Num
};

enum class ShadingModel : uint32_t
{
    Unlit,
    Lit,
    Toon,

    Count
};

enum class BlendMode : uint32_t
{
    Opaque,
    Masked,
    Translucent,
    Additive,

    Count
};

enum class VertexColorMode : uint32_t
{
    None,
    Modulate,
    TextureBlend,

    Count
};

enum class TevMode : uint32_t
{
    Replace,
    Modulate,
    Decal,
    Add,
    SignedAdd,
    Subtract,
    Interpolate,
    Pass,

    Count
};

enum class CullMode : uint8_t
{
    None,
    Back,
    Front,

    Count
};

enum class PropertyOwnerType
{
    Node,
    Asset,
    Global,
    Count
};

enum class LightType : uint8_t
{
    Point,
    Spot,
    Directional,

    Count
};

enum class LightingDomain : uint8_t
{
    Static,
    Dynamic,
    All,

    Count
};

enum class PostProcessPassId : uint8_t
{
    Blur,
    Tonemap,

    Count
};

struct Bounds
{
    glm::vec3 mCenter = { };
    float mRadius = 1.0f;
};

struct DrawData
{
    Node* mNode;
    Material* mMaterial;
    BlendMode mBlendMode;
    glm::vec3 mPosition;
    Bounds mBounds;
    int32_t mSortPriority;
    float mDistance2;
    TypeId mNodeType;
    bool mDepthless;
};

struct LightData
{
    LightType mType;
    LightingDomain mDomain;
    uint8_t mLightingChannels;
    glm::vec3 mPosition;
    glm::vec4 mColor;
    glm::vec3 mDirection;
    float mRadius;
    float mIntensity;
};

struct DebugDraw
{
    StaticMesh* mMesh = nullptr;
    Material* mMaterial = nullptr;
    Node3D* mNode = nullptr;
    glm::mat4 mTransform = glm::mat4(1);
    glm::vec4 mColor = { 0.25f, 0.25f, 1.0f, 1.0f };
    float mLife = 0.0f;
};

struct PrimitivePair
{
    Primitive3D* mPrimitiveA = nullptr;
    Primitive3D* mPrimitiveB = nullptr;

    PrimitivePair() :
        mPrimitiveA(nullptr),
        mPrimitiveB(nullptr)
    {

    }

    PrimitivePair(Primitive3D* compA, Primitive3D* compB)
    {
        mPrimitiveA = compA;
        mPrimitiveB = compB;
    }

    size_t operator()(const PrimitivePair& pairToHash) const
    {
        size_t hash = (size_t)pairToHash.mPrimitiveA + (size_t)pairToHash.mPrimitiveB;
        return hash;
    }

    bool operator==(const PrimitivePair& other) const
    {
        return (mPrimitiveA == other.mPrimitiveA) &&
            (mPrimitiveB == other.mPrimitiveB);
    }

    PrimitivePair(const PrimitivePair& other)
    {
        mPrimitiveA = other.mPrimitiveA;
        mPrimitiveB = other.mPrimitiveB;
    }

    PrimitivePair& operator=(const PrimitivePair& other)
    {
        mPrimitiveA = other.mPrimitiveA;
        mPrimitiveB = other.mPrimitiveB;
        return *this;
    }

    PrimitivePair& operator=(PrimitivePair&& other)
    {
        mPrimitiveA = other.mPrimitiveA;
        mPrimitiveB = other.mPrimitiveB;
        return *this;
    }
};


struct InitOptions
{
    int32_t mWidth = 1280;
    int32_t mHeight = 720;
    std::string mProjectName;
    bool mUseAssetRegistry = false;
    bool mStandalone = false;
    struct EmbeddedFile* mEmbeddedAssets = nullptr;
    uint32_t mEmbeddedAssetCount = 0;
    struct EmbeddedFile* mEmbeddedScripts = nullptr;
    uint32_t mEmbeddedScriptCount = 0;
    std::string mWorkingDirectory;
    uint32_t mGameCode = 0;
    uint32_t mVersion = 0;
    std::string mDefaultScene;
};

struct EngineConfig
{
    EngineConfig()
    {
#if _DEBUG
        mValidateGraphics = true;
#endif
    }

    std::string mProjectPath;
    std::string mDefaultScene;
    int32_t mWindowWidth = 0;
    int32_t mWindowHeight = 0;
    bool mValidateGraphics = false;
    bool mFullscreen = false;
    bool mPackageForSteam = false;
};

enum class ConsoleMode
{
    Off,
    Overlay,
    Full
};

struct EngineState
{
    uint32_t mWindowWidth = DEFAULT_WINDOW_WIDTH;
    uint32_t mWindowHeight = DEFAULT_WINDOW_HEIGHT;
    uint32_t mSecondWindowWidth = DEFAULT_WINDOW_WIDTH;
    uint32_t mSecondWindowHeight = DEFAULT_WINDOW_HEIGHT;
    uint32_t mGameCode = 0;
    uint32_t mVersion = 0;
    std::string mProjectPath;
    std::string mProjectDirectory;
    std::string mProjectName;
    std::string mAssetDirectory;
    std::string mSolutionPath;
    int32_t mArgC = 0;
    char** mArgV = nullptr;
    float mGameDeltaTime = 0.0f;
    float mRealDeltaTime = 0.0f;
    float mGameElapsedTime = 0.0f;
    float mRealElapsedTime = 0.0f;
    float mTimeDilation = 1.0f;
    float mAspectRatioScale = 1.0f;
    bool mPaused = false;
    bool mFrameStep = false;
    bool mInitialized = false;
    bool mSuspended = false;

#if LUA_ENABLED
    lua_State* mLua = nullptr;
#endif
    
    bool mConsoleMode = false;
    bool mQuit = false;
    bool mWindowMinimized = false;
    bool mStandalone = false;

    SystemState mSystem;
    GraphicsState mGraphics;
    InputState mInput;
};

struct RayTestResult
{
    glm::vec3 mStart = {};
    glm::vec3 mEnd = {};
    Primitive3D* mHitNode = {};
    glm::vec3 mHitNormal = {};
    glm::vec3 mHitPosition = {};
    float mHitFraction = 0.0f;
};

struct RayTestMultiResult
{
    glm::vec3 mStart = {};
    glm::vec3 mEnd = {};
    uint32_t mNumHits = 0;
    std::vector<Primitive3D*> mHitNodes;
    std::vector<glm::vec3> mHitNormals;
    std::vector<glm::vec3> mHitPositions;
    std::vector<float> mHitFractions;
};

struct SweepTestResult
{
    glm::vec3 mStart = {};
    glm::vec3 mEnd = {};
    Primitive3D* mHitNode = {};
    glm::vec3 mHitNormal = {};
    glm::vec3 mHitPosition = {};
    float mHitFraction = 0.0f;
};

struct IgnoreRayResultCallback : btCollisionWorld::ClosestRayResultCallback
{
    IgnoreRayResultCallback(const btVector3& rayFromWorld, const btVector3& rayToWorld);
    virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult & rayResult, bool normalInWorldSpace) override;

    uint32_t mNumIgnoreObjects = 0;
    btCollisionObject** mIgnoreObjects = nullptr;
};

struct IgnoreConvexResultCallback : btCollisionWorld::ClosestConvexResultCallback
{
    IgnoreConvexResultCallback(
        const btVector3& convexFromWorld, 
        const btVector3& convexToWorld);
    virtual bool needsCollision(btBroadphaseProxy* proxy0) const;

    uint32_t mNumIgnoreObjects = 0;
    btCollisionObject** mIgnoreObjects = nullptr;
};

enum class FogDensityFunc : uint8_t
{
    Linear,
    Exponential,

    Count
};

struct FogSettings
{
    bool mEnabled = false;
    glm::vec4 mColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    FogDensityFunc mDensityFunc = FogDensityFunc::Linear;
    float mNear = 0.0f;
    float mFar = 100.0f;
};

enum CollisionGroup
{
    ColGroup0 = 1,
    ColGroup1 = 2,
    ColGroup2 = 4,
    ColGroup3 = 8,
    ColGroup4 = 16,
    ColGroup5 = 32,
    ColGroup6 = 64,
    ColGroup7 = 128,

    ColGroupAll = -1
};

enum class AttenuationFunc
{
    Constant,
    Linear,

    Count
};

enum class NetStatus
{
    Local,
    Connecting,
    Client,
    Server,
    
    Count
};

typedef uint8_t NetHostId;

struct NetHost
{
    uint32_t mIpAddress = 0;
    uint16_t mPort = 0;
    NetHostId mId = INVALID_HOST_ID;
    uint64_t mOnlineId = 0;
};

struct ReliablePacket
{
    ReliablePacket(uint16_t seqNum, const char* data, uint32_t size);

    float mTimeSinceSend = 0.0f;
    uint32_t mNumSends = 0;

    std::vector<char> mData;
    uint16_t mSeq = 0;
};

struct NetHostProfile
{
    static const uint32_t sSendBufferSize = 512;

    NetHost mHost;
    float mPing = 0.0f;
    float mTimeSinceLastMsg = 0.0f;
    std::vector<char> mSendBuffer;
    std::vector<char> mReliableSendBuffer;
    std::vector<ReliablePacket> mOutgoingPackets;
    std::vector<ReliablePacket> mIncomingPackets;
    uint16_t mOutgoingReliableSeq = 0;
    uint16_t mIncomingReliableSeq = 0;
    uint16_t mOutgoingUnreliableSeq = 0;
    uint16_t mIncomingUnreliableSeq = 0;
    bool mReady = true;
};

typedef NetHostProfile NetClient;
typedef NetHostProfile NetServer;

enum class ReplicationRate : uint8_t
{
    Low,
    Medium,
    High,

    Count
};

enum class Platform
{
    Windows,
    Linux,
    Android,
    GameCube,
    Wii,
    N3DS,

    Count
};
