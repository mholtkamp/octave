#pragma once

#include <stdint.h>
#include "Graphics/GraphicsConstants.h"

#if API_VULKAN
#include "Graphics/Vulkan/Buffer.h"
#include "Graphics/Vulkan/Image.h"
#include "Graphics/Vulkan/UniformBuffer.h"
#include "Graphics/Vulkan/DescriptorSet.h"
#elif API_GX
#include <gccore.h>
#elif API_C3D
#include <3ds.h>
#include <citro3d.h>
#include "Graphics/C3D/DoubleBuffer.h"
#endif

#if API_VULKAN
extern class VulkanContext* gVulkanContext;
#elif API_GX
extern struct GxContext gGxContext;
#elif API_C3D
extern struct C3dContext gC3dContext;
#endif

struct GraphicsState
{
#if API_VULKAN
    class VulkanContext* mVulkanContext = gVulkanContext;
#elif API_GX
    struct GxContext* mGxContext = &gGxContext;
#elif API_C3D
    struct C3dContext* mC3dContext = &gC3dContext;
#endif
};

enum class PixelFormat
{
    LA4,
    RGB565,
    RGBA8,
    CMPR,
    RGBA5551,

    R8,
    R32U,
    R32F,
    RGBA16F,

    Depth24Stencil8,
	Depth32FStencil8,
    Depth16,
    Depth32F,

    Count
};

enum class PipelineId
{
    // Graphics
    Shadow,
    Opaque,
    Translucent,
    Additive,
    DepthlessOpaque,
    DepthlessTranslucent,
    DepthlessAdditive,
    ShadowMeshBack,
    ShadowMeshFront,
    ShadowMeshClear,
    Selected,
    Wireframe,
    Collision,
    BakedLightVis,
    Line,
    PostProcess,
    NullPostProcess,
    Quad,
    Text,
    Poly,

    // Compute
    PathTrace,
    LightBake,

    // Editor
    HitCheck,

    Count
};

enum class RenderPassId
{
    Shadows,
    Forward,
    PostProcess,
    Ui,
    Clear,

    HitCheck,

    Count
};

enum class FilterType
{
    Nearest,
    Linear,

    Count
};

enum class WrapMode
{
    Clamp,
    Repeat,
    Mirror,

    Count
};

#if API_VULKAN
typedef uint32_t IndexType;
#else
typedef uint16_t IndexType;
#endif

struct TextureResource
{
#if API_VULKAN
    Image* mImage = nullptr;
#elif API_GX
    GXTexObj mGxTexObj = {};
    TPLFile mTplFile = {};
    void* mTplData = nullptr;
#elif API_C3D
    C3D_Tex mTex = {};
    void* mT3dsData = nullptr;
#endif
};

struct MaterialResource
{
#if API_VULKAN
    DescriptorSet* mDescriptorSet = nullptr;
    UniformBuffer* mUniformBuffer = nullptr;
#endif
};

struct StaticMeshResource
{
#if API_VULKAN
    Buffer* mVertexBuffer = nullptr;
    Buffer* mIndexBuffer = nullptr;
#elif API_GX
    void* mDisplayList = nullptr;
    uint32_t mDisplayListSize = 0;
#elif API_C3D
    void* mVertexData = nullptr;
    void* mIndexData = nullptr;
#endif
};

struct SkeletalMeshResource
{
#if API_VULKAN
    Buffer* mVertexBuffer = nullptr;
    Buffer* mIndexBuffer = nullptr;
#elif API_C3D
    void* mVertexData = nullptr;
    void* mIndexData = nullptr;
#endif
};

struct StaticMeshCompResource
{
#if API_VULKAN
    DescriptorSet* mDescriptorSet = nullptr;
    UniformBuffer* mUniformBuffer = nullptr;
#endif
};

struct SkeletalMeshCompResource
{
#if API_VULKAN
    DescriptorSet* mDescriptorSet = nullptr;
    UniformBuffer* mUniformBuffer = nullptr;
    Buffer* mVertexBuffer = nullptr;
#elif API_C3D
    DoubleBuffer mVertexData;
#endif
};

struct TextMeshCompResource
{
#if API_VULKAN
    DescriptorSet* mDescriptorSet = nullptr;
    UniformBuffer* mUniformBuffer = nullptr;
    Buffer* mVertexBuffer = nullptr;
#elif API_C3D
    DoubleBuffer mVertexData;
#endif
};

struct QuadResource
{
#if API_VULKAN
    Buffer* mVertexBuffer = nullptr;
    UniformBuffer* mUniformBuffer = nullptr;
    DescriptorSet* mDescriptorSet = nullptr;
#elif API_C3D
    DoubleBuffer mVertexData;
#endif
};

struct PolyResource
{
#if API_VULKAN
    Buffer* mVertexBuffer = nullptr;
    UniformBuffer* mUniformBuffer = nullptr;
    DescriptorSet* mDescriptorSet = nullptr;
    uint32_t mNumVerts = 0;
#elif API_C3D
    DoubleBuffer mVertexData;
#endif
};

struct TextResource
{
#if API_VULKAN
    Buffer* mVertexBuffer = nullptr;
    UniformBuffer* mUniformBuffer = nullptr;
    DescriptorSet* mDescriptorSet = nullptr;
    uint32_t mNumBufferCharsAllocated = 0;
#elif API_C3D
    DoubleBuffer mVertexData;
    uint32_t mNumBufferCharsAllocated = 0;
#endif
};

struct ParticleCompResource
{
#if API_VULKAN
    DescriptorSet* mDescriptorSet = nullptr;
    UniformBuffer* mUniformBuffer = nullptr;
    Buffer* mVertexBuffer = nullptr;
    Buffer* mIndexBuffer = nullptr;
    uint32_t mNumVerticesAllocated = 0;
#elif API_C3D
    DoubleBuffer mVertexData;
    DoubleBuffer mIndexData;
    uint32_t mNumVerticesAllocated = 0;
#endif
};
