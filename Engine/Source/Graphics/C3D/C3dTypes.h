#pragma once

#if API_C3D

#include <3ds.h>
#include <citro3d.h>
#include <vector>

#include "Graphics/GraphicsConstants.h"
#include "EngineTypes.h"

#define C3D_DYNAMIC_LIGHT_SCALE 4.0f

class World;

enum class ShaderId
{
    StaticMesh,
    SkeletalMesh,
    Particle,
    Quad,
    Text,

    Count
};

enum Shininess
{
    Shininess2,
    Shininess4,
    Shininess8,
    Shininess16,
    Shininess32,
    Shininess64,

    NumShininessLevels
};

enum FresnelPower
{
    FresnelPower1,
    FresnelPower1_5,
    FresnelPower2,
    FresnelPower4,

    NumFresnelPowerLevels
};

enum ToonLevel
{
    ToonLevel2,
    ToonLevel3,

    NumToonLevels
};

// Uniform Location structs.
// Prefetch the locations for possibly better performance.
// TODO: Fill in with other primitive uniforms (staticmesh, skeletalmesh, etc)
struct StaticMeshUniformLocations
{
    int8_t mWorldViewMtx = -1;
    int8_t mNormalMtx = -1;
    int8_t mProjMtx = -1;
    int8_t mUvOffsetScale0 = -1;
    int8_t mUvOffsetScale1 = -1;
    int8_t mUvMaps = -1;
};

struct SkeletalMeshUniformLocations
{
    int8_t mWorldViewMtx = -1;
    int8_t mNormalMtx = -1;
    int8_t mProjMtx = -1;
    int8_t mBoneMtx = -1;
    int8_t mUvOffsetScale0 = -1;
    int8_t mUvOffsetScale1 = -1;
    int8_t mUvMaps = -1;
};

struct ParticleUniformLocations
{
    int8_t mWorldViewMtx = -1;
    int8_t mProjMtx = -1;
    int8_t mUvOffsetScale = -1;
};

struct QuadUniformLocations
{
    int8_t mProjMtx = -1;
};

struct TextUniformLocations
{
    int8_t mWorldViewMtx = -1;
    int8_t mProjMtx = -1;
};

struct LightEnv
{
    C3D_LightEnv mLightEnv = {};
    C3D_LightLutDA mLightAttenuationLuts[8] = {};
    float mLightRadii[8] = {};
    C3D_Light mLights[8] = {};
    uint8_t mLightingChannels = 0x01;
    bool mBakedLighting = false;
};

struct C3dContext
{
    World* mWorld = nullptr;
    C3D_RenderTarget* mRenderTargetLeft = nullptr;
    C3D_RenderTarget* mRenderTargetRight = nullptr;
    C3D_RenderTarget* mRenderTargetBottom = nullptr;
    float mIod = 0.0f;
    uint32_t mCurrentView = 0;
    uint32_t mFrameIndex = 0;
    uint32_t mCurrentScreen = 0;

    DVLB_s* mShaderDvlbs[uint32_t(ShaderId::Count)] = {};
    shaderProgram_s mShaderPrograms[uint32_t(ShaderId::Count)] = {};

    LightEnv mLightEnv;
    C3D_LightEnv mNoLightEnv = {};
    C3D_LightLut mLightLut[NumShininessLevels] = {};
    C3D_LightLut mFresnelLut[NumFresnelPowerLevels] = {};
    C3D_LightLut mToonLut[NumToonLevels] = {};

    C3D_FogLut mFogLut = {};
    float mFogNear = 0.0f;
    float mFogFar = 0.0f;
    FogDensityFunc mFogDensityFunc = FogDensityFunc::Count;
    bool mMaterialApplyFog = true;
    bool mFogEnabled = false;

    ShaderId mLastBoundShaderId = ShaderId::Count;
    Material* mLastBoundMaterial = nullptr;

    std::vector<void*> mLinearDestroyQueue[MAX_FRAMES];
    std::vector<C3D_Tex> mTexDestroyQueue[MAX_FRAMES];

    StaticMeshUniformLocations mStaticMeshLocs;
    SkeletalMeshUniformLocations mSkeletalMeshLocs;
    ParticleUniformLocations mParticleLocs;
    QuadUniformLocations mQuadLocs;
    TextUniformLocations mTextLocs;
};

extern C3dContext gC3dContext;

#endif
