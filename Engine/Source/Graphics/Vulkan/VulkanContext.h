#pragma once

#if API_VULKAN

#include <vulkan/vulkan.h>
#include <stb_image.h>

#include "VulkanUtils.h"
#include "MultiBuffer.h"
#include "DescriptorSet.h"
#include "Pipeline.h"
#include "PipelineConfigs.h"
#include "VramAllocator.h"
#include "DestroyQueue.h"
#include "Buffer.h"
#include "Image.h"
#include "Line.h"
#include "ObjectRef.h"
#include "RayTracer.h"
#include "Profiler.h"
#include "DescriptorPool.h"
#include "DescriptorLayoutCache.h"
#include "PipelineCache.h"
#include "RenderPassCache.h"
#include "PostProcessChain.h"

#if PLATFORM_LINUX
#include <xcb/xcb.h>
#include <vulkan/vulkan_xcb.h>
#endif

struct EngineState;
class World;

struct QueueFamilyIndices
{
    int32_t mGraphicsFamily = -1;
    int32_t mPresentFamily = -1;

    bool IsComplete()
    {
        return mGraphicsFamily >= 0 &&
            mPresentFamily >= 0;
    }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct GpuTimespan
{
    std::string mName;
    int32_t mStartIndex = -1;
    int32_t mEndIndex = -1;
};

class VulkanContext
{

public:

    VulkanContext();
    ~VulkanContext();

    void Initialize();
    void Destroy();

    void BeginFrame();
    void EndFrame();
    void BeginRenderPass(RenderPassId id);
    void BeginVkRenderPass(const RenderPassSetup& rpSetup, bool insertBarrier);
    void EndRenderPass();
    void EndVkRenderPass();
    void CommitPipeline();
    void DrawLines(const std::vector<Line>& lines);
    void DrawFullscreen();
    void BindFullscreenVertexBuffer(VkCommandBuffer cb);

    VkDevice GetDevice();
    void CreateSwapchain();
    void RecreateSwapchain(bool recreateSurface);

    VkPhysicalDevice GetPhysicalDevice();
    DescriptorPool& GetDescriptorPool();
    DescriptorLayoutCache& GetDescriptorLayoutCache();

    DestroyQueue* GetDestroyQueue();

    VkExtent2D& GetSwapchainExtent();
    VkFormat GetSwapchainFormat();
    VkFormat GetSceneColorFormat();
    Image* GetSceneColorImage();
    Image* GetSwapchainImage();

    Pipeline* GetBoundPipeline();
    PipelineCache& GetPipelineCache();
    void SavePipelineCacheToFile();

    void SetViewport(int32_t x, int32_t y, int32_t width, int32_t height, bool handlePrerotation, bool useSceneRes);
    void SetScissor(int32_t x, int32_t y, int32_t width, int32_t height, bool handlePrerotation, bool useSceneRes);

    void CreateCommandBuffers();

    uint32_t GetFrameIndex() const;
    uint32_t GetFrameNumber() const;
    RenderPassId GetCurrentRenderPassId() const;

    Image* GetShadowMapImage();

    VkCommandBuffer GetCommandBuffer();
    VkCommandPool GetCommandPool();
    VkQueue GetGraphicsQueue();

    void UpdateGlobalDescriptorSet();
    void UpdateGlobalUniformData();

    void BindGlobalDescriptorSet();

    GlobalUniformData& GetGlobalUniformData();

    bool IsValidationEnabled() const;
    bool IsRayTracingSupported() const;
    bool HasFeatureWideLines() const;
    bool HasFeatureFillModeNonSolid() const;

    bool AreMaterialsEnabled() const;
    void EnableMaterials(bool enable);

    void BeginGpuTimestamp(const char* name);
    void EndGpuTimestamp(const char* name);
    void ReadTimeQueryResults();

    RayTracer* GetRayTracer();
    PostProcessChain* GetPostProcessChain();

    void RenderPostProcessChain();

    VkSurfaceTransformFlagBitsKHR GetPreTransformFlag() const;

    uint32_t GetSceneWidth();
    uint32_t GetSceneHeight();

    const VkPhysicalDeviceProperties& GetDeviceProperties() const;
    UniformBuffer* GetFrameUniformBuffer();

    Shader* GetGlobalShader(const std::string& name);

    // Pipeline State
    const PipelineState& GetPipelineState() const;
    void SetPipelineState(const PipelineState& state);
    void SetVertexShader(Shader* shader);
    void SetFragmentShader(Shader* shader);
    void SetComputeShader(Shader* shader);
    void SetVertexShader(const std::string& globalName);
    void SetFragmentShader(const std::string& globalName);
    void SetComputeShader(const std::string& globalName);
    void SetRenderPass(VkRenderPass renderPass);
    void SetVertexType(VertexType vertexType);
    void SetRasterizerDiscard(bool discard);
    void SetPrimitiveTopology(VkPrimitiveTopology primitiveToplogy);
    void SetPolygonMode(VkPolygonMode polygonMode);
    void SetLineWidth(float lineWidth);
    void SetDynamicLineWidth(bool dynamicLineWidth);
    void SetCullMode(VkCullModeFlags cullMode);
    void SetFrontFace(VkFrontFace frontFace);
    void SetDepthBias(float depthBias);
    void SetDepthTestEnabled(bool enabled);
    void SetDepthWriteEnabled(bool enabled);
    void SetDepthCompareOp(VkCompareOp compareOp);
    void SetBlendState(VkPipelineColorBlendAttachmentState blendState, uint32_t index = 0);
    void SetBlendState(BasicBlendState basicBlendState, uint32_t index = 0);
    void SetBlendEnable(bool enable, uint32_t index = 0);
    void SetBlendColorOp(VkBlendFactor src, VkBlendFactor dst, VkBlendOp op, uint32_t index = 0);
    void SetBlendAlphaOp(VkBlendFactor src, VkBlendFactor dst, VkBlendOp op, uint32_t index = 0);
    void SetColorWriteMask(VkColorComponentFlags writeMask, uint32_t index = 0);


private:

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
        void* userData);

    void CreateInstance();
    void CreateDebugCallback();
    void CreateSurface();
    void CreateLogicalDevice();
    void CreateFrameUniformBuffer();
    void DestroyFrameUniformBuffer();
    void CreateRenderPasses();
    void DestroyRenderPasses();
    void CreateCommandPool();
    void CreateSemaphores();
    void CreateFences();
    void CreateDescriptorPools();
    void DestroyDescriptorPools();
    void CreateDepthImage();
    void CreateSceneColorImage();
    void CreateShadowMapImage();
    void CreateQueryPools();
    void DestroyQueryPools();
    void RecreateSurface();
    void CreateGlobalShaders();
    void DestroyGlobalShaders();
    void CreateMisc();
    void DestroyMisc();

    void PickPhysicalDevice();
    bool IsDeviceSuitable(VkPhysicalDevice device);
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availableModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

    bool CheckValidationLayerSupport(
        const char** layers,
        uint32_t count);
    bool CheckDeviceExtensionSupport(
        VkPhysicalDevice device,
        const char** extensions,
        uint32_t count);

    void DestroySwapchain();
    void DestroyDebugCallback();

private:

    // Core
    VkInstance mInstance = VK_NULL_HANDLE;
    VkDebugReportCallbackEXT mCallback = VK_NULL_HANDLE;
    VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
    VkDevice mDevice = VK_NULL_HANDLE;
    VkQueue mGraphicsQueue = VK_NULL_HANDLE;
    VkQueue mPresentQueue = VK_NULL_HANDLE;
    VkSurfaceKHR mSurface = VK_NULL_HANDLE;
    uint32_t mGraphicsQueueFamily = 0;
    uint32_t mPresentQueueFamily = 0;

    // Descriptors
    DescriptorPool mDescriptorPools[MAX_FRAMES];
    DescriptorLayoutCache mDescriptorLayoutCache;
    VkDescriptorPool mImguiDescriptorPool = VK_NULL_HANDLE;

    // Command Buffers
    VkCommandPool mCommandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> mCommandBuffers;

    // Swapchain
    VkSwapchainKHR mSwapchain = VK_NULL_HANDLE;
    std::vector<VkImage> mSwapchainImages;
    std::vector<VkImageView> mSwapchainImageViews;
    VkFormat mSwapchainImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D mSwapchainExtent = {};
    std::vector<Image*> mExtSwapchainImages;

    // RenderPasses
    RenderPassCache mRenderPassCache;
    VkRenderPass mImguiRenderPass = VK_NULL_HANDLE;

    // Images
    Image* mShadowMapImage = nullptr;
    Image* mSceneColorImage = nullptr;
    VkFormat mSceneColorImageFormat;
    Image* mDepthImage = nullptr;
    VkFormat mDepthImageFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;

    // Synchronization
    VkSemaphore mImageAvailableSemaphore[MAX_FRAMES] = {};
    VkSemaphore mRenderFinishedSemaphore[MAX_FRAMES] = {};
    VkFence mWaitFences[MAX_FRAMES] = {};

    // Pipelines
    PipelineCache mPipelineCache;
    Pipeline* mBoundPipeline = nullptr;

    // Shader Data
    std::unordered_map<std::string, Shader*> mGlobalShaders;
    DescriptorSet mGlobalDescriptorSet;
    DescriptorSet mDebugDescriptorSet;
    DescriptorSet mPostProcessDescriptorSet;
    UniformBuffer* mFrameUniformBuffer = nullptr;
    GlobalUniformData mGlobalUniformData;

    // Destroy Queue
    DestroyQueue mDestroyQueue;

    // Ray Tracer
    RayTracer mRayTracer;

    // Debug
    bool mValidate;
    VkDebugUtilsMessengerEXT mDebugMessenger = VK_NULL_HANDLE;
    uint32_t mEnabledExtensionCount = 0;
    const char* mEnabledExtensions[MAX_ENABLED_EXTENSIONS] = { };
    uint32_t mEnabledLayersCount = 0;
    const char* mEnabledLayers[MAX_ENABLED_LAYERS] = { };

    // Timestamp Queries
    std::vector<GpuTimespan> mGpuTimespans[MAX_FRAMES];
    VkQueryPool mTimeQueryPools[MAX_FRAMES] = { };
    int32_t mNumTimestamps[MAX_FRAMES] = { };
    float mTimestampPeriod = 0.0f;
    bool mTimestampsSupported = false;

    //Pipeline State
    PipelineState mPipelineState;

    // PostProcess
    PostProcessChain mPostProcessChain;

    // Misc
    int32_t mFrameIndex = 0;
    int32_t mFrameNumber = 0;
    uint32_t mSwapchainImageIndex = 0;
    RenderPassId mCurrentRenderPassId = RenderPassId::Count;
    int32_t mNumLinesAllocated = 0;
    Buffer* mLineVertexBuffer = nullptr;
    bool mInitialized = false;
    bool mEnableMaterials = false;
    bool mSupportsRayTracing = false;
    bool mFeatureWideLines = false;
    bool mFeatureFillModeNonSolid = false;
    EngineState* mEngineState = nullptr;
    VkSurfaceTransformFlagBitsKHR mPreTransformFlag = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    float mResolutionScale = 1.0f;
    uint32_t mSceneWidth = 0;
    uint32_t mSceneHeight = 0;
    VkPhysicalDeviceProperties mDeviceProperties;
    Buffer* mFullScreenVertexBuffer = nullptr;

#if EDITOR
public:
    class Node3D* ProcessHitCheck(World* world, int32_t pixelX, int32_t pixelY, uint32_t* outInstance = nullptr);
private:
    void CreateHitCheck();
    void DestroyHitCheck();

    Image* mHitCheckImage = nullptr;
    Buffer* mHitCheckBuffer = nullptr;
#endif

};

void CreateVulkanContext();
void DestroyVulkanContext();
VulkanContext* GetVulkanContext();

#endif