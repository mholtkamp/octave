#pragma once

#include "Maths.h"
#include "System/SystemTypes.h"
#include "Vertex.h"
#include "VulkanConstants.h"

#include <vulkan/vulkan.h>
#include <vector>

class Shader;
class Image;

struct PipelineCreateJobArgs
{
    std::vector<class Pipeline*>* mPipelines;
    MutexObject* mMutex = nullptr;
};

enum class BasicBlendState
{
    Opaque,
    Translucent,
    Additive,

    Count
};

struct PipelineState
{
    PipelineState()
    {
        // Default write masks to RGBA
        for (uint32_t i = 0; i < MAX_RENDER_TARGETS; ++i)
        {
            mBlendStates[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        }
    }

    Shader* mVertexShader = nullptr;
    Shader* mFragmentShader = nullptr;
    Shader* mComputeShader = nullptr;

    VkRenderPass mRenderPass = VK_NULL_HANDLE;
    VertexType mVertexType = VertexType::Vertex;

    // Rasterizer
    bool mRasterizerDiscard = false;
    VkPrimitiveTopology mPrimitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkPolygonMode mPolygonMode = VK_POLYGON_MODE_FILL;
    float mLineWidth = 1.0f;
    bool mDynamicLineWidth = false;
    VkCullModeFlags mCullMode = VK_CULL_MODE_BACK_BIT;
    VkFrontFace mFrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    float mDepthBias = 0.0f;

    // Depth Stencil
    bool mDepthTestEnabled = true;
    bool mDepthWriteEnabled = true;
    VkCompareOp mDepthCompareOp = VK_COMPARE_OP_LESS;
    bool mDepthBoundsEnabled = false;
    float mMinDepthBounds = 0.0f;
    float mMaxDepthBounds = 1.0f;
    bool mStencilTestEnabled = false;
    VkStencilOpState mStencilFront = {};
    VkStencilOpState mStencilBack = {};

    // Blending
    VkPipelineColorBlendAttachmentState mBlendStates[MAX_RENDER_TARGETS] = {};

    bool operator==(const PipelineState& other) const;

    size_t Hash() const;
};

struct PipelineStateHasher
{
    size_t operator()(const PipelineState& k) const
    {
        return k.Hash();
    }
};

struct RenderPassSetup
{
    Image* mDepthImage = nullptr;
    Image* mColorImages[MAX_RENDER_TARGETS] = {};
    VkAttachmentLoadOp mLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    VkAttachmentStoreOp mStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    VkAttachmentLoadOp mDepthLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    VkAttachmentStoreOp mDepthStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    VkImageLayout mPreLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkImageLayout mPostLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    const char* mDebugName = "";
};

struct LightUniformData
{
    glm::vec4 mColor;

    glm::vec3 mPosition;
    float mRadius;

    glm::vec3 mDirection;
    uint32_t mType;

    float mIntensity;
    float mPad0;
    float mPad1;
    float mPad2;
};

struct MeshInstanceBufferData
{
    glm::mat4 mTransform;
};

struct GlobalUniformData
{
    glm::mat4 mViewProjMatrix;
    glm::mat4 mViewToWorld;
    glm::mat4 mShadowViewProj;
    glm::mat4 mPreRotationMatrix;

    glm::vec4 mAmbientLightColor;
    glm::vec4 mViewPosition;
    glm::vec4 mViewDirection;
    glm::vec2 mScreenDimensions;
    glm::vec2 mInterfaceResolution;
    glm::vec4 mShadowColor;
    glm::vec4 mViewport;
    glm::vec4 mSceneViewport;

    uint32_t mFrameNumber;
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
    uint32_t mPathTracingEnabled;

    uint32_t mNumLights;
    int32_t mSelectedInstance;
    uint32_t mPad0;
    uint32_t mPad1;

    LightUniformData mLights[MAX_LIGHTS_PER_FRAME];
};

struct RayTraceUniforms
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
    int32_t mBakeMeshIndex;

    uint32_t mReceiveShadows;
    uint32_t mPad0;
    uint32_t mPad1;
    uint32_t mPad2;

    glm::vec4 mSkyZenithColor;
    glm::vec4 mSkyHorizonColor;
    glm::vec4 mGroundColor;
};

struct GeometryData
{
    glm::mat4 mWVPMatrix;
    glm::mat4 mWorldMatrix;
    glm::mat4 mNormalMatrix;
    glm::vec4 mColor;

    uint32_t mHitCheckId;
    uint32_t mHasBakedLighting;
    uint32_t mPad0;
    uint32_t mPad1;

    uint32_t mNumLights;
    uint32_t mLights0;
    uint32_t mLights1;
    uint32_t mPad2;
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

    float mWrapLighting;
    float mPad0;
    float mPad1;
    float mPad2;

    uint32_t mUvMaps[MATERIAL_LITE_MAX_TEXTURES];
    uint32_t mTevModes[MATERIAL_LITE_MAX_TEXTURES];
};

enum class RayTraceLightType
{
    Point,
    Directional,

    Count
};

struct RayTraceVertex
{
    glm::vec3 mPosition = { 0.0f, 0.0f, 0.0f };
    float mPad0 = 1337.0f;

    glm::vec2 mTexcoord0 = { 0.0f, 0.0f };
    glm::vec2 mTexcoord1 = { 0.0f, 0.0f };

    glm::vec3 mNormal = { 0.0f, 0.0f, 1.0f };
    float mPad1 = 1337.1f;

    glm::vec4 mColor = { 1.0f, 1.0f, 1.0f, 1.0f };
};

struct RayTraceTriangle
{
    RayTraceVertex mVertices[3];
};

struct RayTraceMesh
{
    glm::vec4 mBounds = { 0.0f, 0.0f, 0.0f, 10000.0f };

    uint32_t mStartTriangleIndex = 0;
    uint32_t mNumTriangles = 0;
    uint32_t mCastShadows = 1;
    uint32_t mHasBakedLighting = 0;

    glm::uvec4 mTextures;

    MaterialData mMaterial;
};

struct RayTraceLight
{
    glm::vec3 mPosition = { 0.0f, 0.0f, 0.0f };
    float mRadius = 0.0f;

    glm::vec4 mColor = { 1.0f, 1.0f, 1.0f, 1.0f };

    glm::vec3 mDirection = { 0.0f, 0.0f, -1.0f };
    uint32_t mLightType = uint32_t(RayTraceLightType::Point);

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
