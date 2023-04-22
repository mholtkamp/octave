#pragma once

#include <glm/glm.hpp>

struct GlobalUniformData
{
    glm::mat4 mViewProjMatrix;
    glm::mat4 mDirectionalLightVP;
    glm::mat4 mViewToWorld;

    glm::vec4 mDirectionalLightDirection;
    glm::vec4 mDirectionalLightColor;
    glm::vec4 mAmbientLightColor;
    glm::vec4 mViewPosition;
    glm::vec4 mViewDirection;
    glm::vec2 mScreenDimensions;
    glm::vec2 mInterfaceResolution;
    glm::vec4 mPointLightPositions[MAX_POINTLIGHTS];
    glm::vec4 mPointLightColors[MAX_POINTLIGHTS];
    glm::vec4 mShadowColor;

    int32_t mNumPointLights;
    int32_t mVisualizationMode;
    float mGameTime;
    float mRealTime;

    glm::vec4 mFogColor;
    int mFogEnabled;
    int mFogDensityFunc;
    float mFogNear;
    float mFogFar;

    float mNearHalfWidth;
    float mNearHalfHeight;
    float mNearDist;
    uint32_t mFrameNumber;

    uint32_t mPathTracingEnabled;
    uint32_t mPad0;
    uint32_t mPad1;
    uint32_t mPad2;
};

struct PathTraceUniforms
{
    uint32_t mNumTriangles;
    uint32_t mNumMeshes;
    uint32_t mNumLights;
    uint32_t mMaxBounces;

    uint32_t mRaysPerThread;
    uint32_t mAccumulatedFrames;
    uint32_t mNumBakeVertices;
    uint32_t mNumBakeTriangles;

    float mShadowBias;
    uint32_t mDiffuseDirect;
    uint32_t mDiffuseIndirect;
    float mPad0;
};

struct GeometryData
{
    glm::mat4 mWVPMatrix;
    glm::mat4 mWorldMatrix;
    glm::mat4 mNormalMatrix;
    glm::mat4 mLightWVPMatrix;
    glm::vec4 mColor;

    uint32_t mHitCheckId;
    uint32_t mHasBakedLighting;
    uint32_t mPadding0;
    uint32_t mPadding1;
};

struct SkinnedGeometryData
{
    GeometryData mBase;

    glm::mat4 mBoneMatrices[MAX_BONES];

    uint32_t mNumBoneInfluences;
    uint32_t mPadding0;
    uint32_t mPadding1;
    uint32_t mPadding2;
};

struct QuadUniformData
{
    glm::mat4 mTransform;
    glm::vec4 mColor;
    glm::vec4 mTint;
};

struct TextUniformData
{
    glm::mat4 mTransform;
    glm::vec4 mColor;

    float mX;
    float mY;
    float mCutoff;
    float mOutlineSize;

    float mScale;
    float mSoftness;
    float mPadding1;
    float mPadding2;

    int32_t mDistanceField;
    int32_t mEffect;
};

struct PolyUniformData
{
    glm::mat4 mTransform;
    glm::vec4 mColor;

    float mX;
    float mY;
    float mPad0;
    float mPad1;
};

struct MaterialData
{
    glm::vec2 mUvOffset0;
    glm::vec2 mUvScale0;

    glm::vec2 mUvOffset1;
    glm::vec2 mUvScale1;

    glm::vec4 mColor;
    glm::vec4 mFresnelColor;

    uint32_t mShadingModel;
    uint32_t mBlendMode;
    uint32_t mToonSteps;
    float mFresnelPower;

    float mSpecular;
    float mOpacity;
    float mMaskCutoff;
    float mShininess;

    uint32_t mFresnelEnabled;
    uint32_t mVertexColorMode;
    uint32_t mApplyFog;
    float mEmission;

    uint32_t mUvMaps[MATERIAL_MAX_TEXTURES];
    uint32_t mTevModes[MATERIAL_MAX_TEXTURES];
};

enum class PathTraceLightType
{
    Point,
    Directional,

    Count
};

struct PathTraceVertex
{
    glm::vec3 mPosition = { 0.0f, 0.0f, 0.0f };
    float mPad0 = 1337.0f;

    glm::vec2 mTexcoord0 = { 0.0f, 0.0f };
    glm::vec2 mTexcoord1 = { 0.0f, 0.0f };

    glm::vec3 mNormal = { 0.0f, 0.0f, 1.0f };
    float mPad1 = 1337.1f;

    glm::vec4 mColor = { 1.0f, 1.0f, 1.0f, 1.0f };
};

struct PathTraceTriangle
{
    PathTraceVertex mVertices[3];
};

struct PathTraceMesh
{
    glm::vec4 mBounds = { 0.0f, 0.0f, 0.0f, 10000.0f };

    uint32_t mStartTriangleIndex = 0;
    uint32_t mNumTriangles = 0;
    uint32_t mCastShadows = 1;
    uint32_t mHasBakedLighting = 0;

    glm::uvec4 mTextures;

    MaterialData mMaterial;
};

struct PathTraceLight
{
    glm::vec3 mPosition = { 0.0f, 0.0f, 0.0f };
    float mRadius = 0.0f;

    glm::vec4 mColor = { 1.0f, 1.0f, 1.0f, 1.0f };

    glm::vec3 mDirection = { 0.0f, 0.0f, -1.0f };
    uint32_t mLightType = uint32_t(PathTraceLightType::Point);

    uint32_t mCastShadows = 1;
    uint32_t mPad0 = 1337;
    uint32_t mPad1 = 1338;
    uint32_t mPad2 = 1339;
};

struct LightBakeVertex
{
    glm::vec3 mPosition = { 0.0f, 0.0f, 0.0f };
    float mPad0 = 1337.0f;

    glm::vec3 mNormal = { 0.0f, 1.0f, 0.0f };
    float mPad1 = 1337.1f;

    glm::vec4 mDirectLight = { 0.0f, 0.0f, 0.0f, 0.0f };
    glm::vec4 mIndirectLight = { 0.0f, 0.0f, 0.0f, 0.0f };
};

struct VertexLightData
{
    glm::vec4 mDirectLight = { 0.0f, 0.0f, 0.0f, 0.0f };
    glm::vec4 mIndirectLight = { 0.0f, 0.0f, 0.0f, 0.0f };
};

struct DiffuseTriangle
{
    glm::uvec3 mVertexIndices = { 0, 0, 0 };
    uint32_t mPad0 = 1337;
};

struct LightBakeResult
{
    std::vector<glm::vec4> mDirectColors;
    std::vector<glm::vec4> mIndirectColors;
};

enum class LightBakePhase : uint8_t
{
    Direct,
    Indirect,
    Diffuse,
    Count
};

enum class DescriptorSetBinding
{
    Global = 0,

    Geometry = 1,
    PostProcess = 1,
    Quad = 1,
    Text = 1,
    Poly = 1,

    Material = 2
};
