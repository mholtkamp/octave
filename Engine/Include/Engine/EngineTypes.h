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
class PrimitiveComponent;
class TransformComponent;
class Actor;

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

enum class CullMode : uint8_t
{
    None,
    Back,
    Front,

    Count
};

enum class PropertyOwnerType
{
    Component,
    Actor,
    Asset,
    Widget,
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

struct Bounds
{
    glm::vec3 mCenter = { };
    float mRadius = 1.0f;
};

struct DrawData
{
    PrimitiveComponent* mComponent;
    Material* mMaterial;
    ShadingModel mShadingModel;
    BlendMode mBlendMode;
    glm::vec3 mPosition;
    Bounds mBounds;
    int32_t mSortPriority;
    TypeId mComponentType;
    bool mDepthless;
};

struct LightData
{
    LightType mType;
    LightingDomain mDomain;
    glm::vec3 mPosition;
    glm::vec4 mColor;
    glm::vec3 mDirection;
    float mRadius;
};

struct DebugDraw
{
    StaticMesh* mMesh = nullptr;
    Material* mMaterial = nullptr;
    Actor* mActor = nullptr;
    TransformComponent* mComponent = nullptr;
    glm::mat4 mTransform = glm::mat4(1);
    glm::vec4 mColor = { 0.25f, 0.25f, 1.0f, 1.0f };
    float mLife = 0.0f;
};

struct ComponentPair
{
    PrimitiveComponent* mComponentA = nullptr;
    PrimitiveComponent* mComponentB = nullptr;

    ComponentPair() :
        mComponentA(nullptr),
        mComponentB(nullptr)
    {

    }

    ComponentPair(PrimitiveComponent* compA, PrimitiveComponent* compB)
    {
        mComponentA = compA;
        mComponentB = compB;
    }

    size_t operator()(const ComponentPair& pairToHash) const
    {
        size_t hash = (size_t)pairToHash.mComponentA + (size_t)pairToHash.mComponentB;
        return hash;
    }

    bool operator==(const ComponentPair& other) const
    {
        return (mComponentA == other.mComponentA) &&
            (mComponentB == other.mComponentB);
    }

    ComponentPair(const ComponentPair& other)
    {
        mComponentA = other.mComponentA;
        mComponentB = other.mComponentB;
    }

    ComponentPair& operator=(const ComponentPair& other)
    {
        mComponentA = other.mComponentA;
        mComponentB = other.mComponentB;
        return *this;
    }

    ComponentPair& operator=(ComponentPair&& other)
    {
        mComponentA = other.mComponentA;
        mComponentB = other.mComponentB;
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
    std::string mDefaultLevel;
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
    std::string mDefaultLevel;
    int32_t mWindowWidth = 0;
    int32_t mWindowHeight = 0;
    bool mValidateGraphics = false;
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
    float mGameDeltaTime = 0.0f;
    float mRealDeltaTime = 0.0f;
    float mGameElapsedTime = 0.0f;
    float mRealElapsedTime = 0.0f;
    float mTimeDilation = 1.0f;
    float mAspectRatioScale = 1.0f;
    bool mPaused = false;
    bool mFrameStep = false;
    bool mInitialized = false;

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
    PrimitiveComponent* mHitComponent = {};
    glm::vec3 mHitNormal = {};
    glm::vec3 mHitPosition = {};
    float mHitFraction = 0.0f;
};

struct RayTestMultiResult
{
    glm::vec3 mStart = {};
    glm::vec3 mEnd = {};
    uint32_t mNumHits = 0;
    std::vector<PrimitiveComponent*> mHitComponents;
    std::vector<glm::vec3> mHitNormals;
    std::vector<glm::vec3> mHitPositions;
    std::vector<float> mHitFractions;
};

struct SweepTestResult
{
    glm::vec3 mStart = {};
    glm::vec3 mEnd = {};
    PrimitiveComponent* mHitComponent = {};
    glm::vec3 mHitNormal = {};
    glm::vec3 mHitPosition = {};
    float mHitFraction = 0.0f;
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
    GameCube,
    Wii,
    N3DS,

    Count
};
