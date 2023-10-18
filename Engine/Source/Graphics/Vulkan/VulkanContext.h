#pragma once

#if API_VULKAN

#include <vulkan/vulkan.h>
#include <stb_image.h>

#include "VulkanUtils.h"
#include "MultiBuffer.h"
#include "DescriptorSet.h"
#include "Pipeline.h"
#include "PipelineConfigs.h"
#include "Allocator.h"
#include "DestroyQueue.h"
#include "Buffer.h"
#include "Image.h"
#include "Line.h"
#include "ResourceArena.h"
#include "ObjectRef.h"
#include "RayTracer.h"
#include "Profiler.h"
#include "MaterialPipelineCache.h"

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
    void EndRenderPass();
    void BindPipeline(PipelineId id, VertexType vertexType);
    void BindPipeline(Pipeline* pipeline, VertexType vertexType);
    void RebindPipeline(VertexType vertexType);
    void DrawLines(const std::vector<Line>& lines);
    void DrawFullscreen();

    VkDevice GetDevice();
    void CreateSwapchain();
    void RecreateSwapchain(bool recreateSurface);

    VkPhysicalDevice GetPhysicalDevice();
    VkDescriptorPool GetDescriptorPool();

    DestroyQueue* GetDestroyQueue();

    VkExtent2D& GetSwapchainExtent();
    VkFormat GetSwapchainFormat();
    VkFormat GetSceneColorFormat();

    Pipeline* GetPipeline(PipelineId id);
    Pipeline* GetCurrentlyBoundPipeline();
    VkPipelineCache GetPipelineCache() const;
    void SavePipelineCacheToFile();

    VkRenderPass GetForwardRenderPass();
    VkRenderPass GetPostprocessRenderPass();
    VkRenderPass GetUIRenderPass();

    void SetViewport(int32_t x, int32_t y, int32_t width, int32_t height, bool handlePrerotation, bool useSceneRes);
    void SetScissor(int32_t x, int32_t y, int32_t width, int32_t height, bool handlePrerotation, bool useSceneRes);

    DescriptorSet* GetGlobalDescriptorSet();

    void CreateCommandBuffers();

    uint32_t GetFrameIndex() const;
    RenderPassId GetCurrentRenderPassId() const;

    Image* GetShadowMapImage();

    VkCommandBuffer GetCommandBuffer();
    VkCommandPool GetCommandPool();
    VkQueue GetGraphicsQueue();

    void UpdateGlobalDescriptorSet();
    void UpdateGlobalUniformData();

    GlobalUniformData& GetGlobalUniformData();

    bool IsValidationEnabled() const;
    bool IsRayTracingSupported() const;
    bool HasFeatureWideLines() const;
    bool HasFeatureFillModeNonSolid() const;

    bool AreMaterialsEnabled() const;
    void EnableMaterials(bool enable);

    DescriptorSetArena& GetMeshDescriptorSetArena();
    UniformBufferArena& GetMeshUniformBufferArena();

    void BeginGpuTimestamp(const char* name);
    void EndGpuTimestamp(const char* name);
    void ReadTimeQueryResults();

    RayTracer* GetRayTracer();

    VkSurfaceTransformFlagBitsKHR GetPreTransformFlag() const;

    void EnableMaterialPipelineCache(bool enable);
    bool IsMaterialPipelineCacheEnabled() const;
    MaterialPipelineCache* GetMaterialPipelineCache();

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
    void CreateImageViews();
    void CreateGlobalUniformBuffer();
    void CreateGlobalDescriptorSet();
    void CreateRenderPass();
    void CreatePipelineCache();
    void DestroyPipelineCache();
    void CreatePipelines();
    void DestroyPipelines();
    void CreateFramebuffers();
    void CreateCommandPool();
    void CreateSemaphores();
    void CreateFences();
    void CreateDescriptorPool();
    void CreateDepthImage();
    void CreatePostProcessDescriptorSet();
    void CreateSceneColorImage();
    void CreateShadowMapImage();
    void CreateQueryPools();
    void DestroyQueryPools();
    void RecreateSurface();

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

    // Pools
    VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
    VkCommandPool mCommandPool = VK_NULL_HANDLE;

    // Command Buffers
    std::vector<VkCommandBuffer> mCommandBuffers;

    // Swapchain
    VkSwapchainKHR mSwapchain = VK_NULL_HANDLE;
    std::vector<VkImage> mSwapchainImages;
    std::vector<VkImageView> mSwapchainImageViews;
    VkFormat mSwapchainImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D mSwapchainExtent = {};

    // Render Passes
    VkRenderPass mShadowRenderPass = VK_NULL_HANDLE;
    VkRenderPass mForwardRenderPass = VK_NULL_HANDLE;
    VkRenderPass mPostprocessRenderPass = VK_NULL_HANDLE;
    VkRenderPass mUIRenderPass = VK_NULL_HANDLE;
    VkRenderPass mClearSwapchainPass = VK_NULL_HANDLE;

    // Framebuffers
    VkFramebuffer mSceneColorFramebuffer = VK_NULL_HANDLE;
    VkFramebuffer mShadowFramebuffer = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> mSwapchainFramebuffers;

    // Images
    Image* mShadowMapImage = nullptr;
    Image* mSceneColorImage = nullptr;
    VkFormat mSceneColorImageFormat;
    Image* mDepthImage = nullptr;
    VkFormat mDepthImageFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;

    // Synchronization
    VkSemaphore mImageAvailableSemaphore = VK_NULL_HANDLE;
    VkSemaphore mRenderFinishedSemaphore = VK_NULL_HANDLE;
    VkFence mWaitFences[MAX_FRAMES] = {};

    // Pipelines
    VkPipelineCache mPipelineCache = VK_NULL_HANDLE;
    Pipeline* mPipelines[(size_t)PipelineId::Count] = {};

    // Shader Data
    UniformBuffer* mGlobalUniformBuffer = nullptr;
    DescriptorSet* mGlobalDescriptorSet = nullptr;
    DescriptorSet* mDebugDescriptorSet = nullptr;
    DescriptorSet* mPostProcessDescriptorSet = nullptr;
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
    DescriptorSetArena mMeshDescriptorSetArena;
    UniformBufferArena mMeshUniformBufferArena;

    // Timestamp Queries
    std::vector<GpuTimespan> mGpuTimespans[MAX_FRAMES];
    VkQueryPool mTimeQueryPools[MAX_FRAMES] = { };
    int32_t mNumTimestamps[MAX_FRAMES] = { };
    float mTimestampPeriod = 0.0f;
    bool mTimestampsSupported = false;

    // Material Pipelines
    MaterialPipelineCache mMaterialPipelineCache;

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
    bool mEnableMaterialPipelineCache = false;
    bool mFeatureWideLines = false;
    bool mFeatureFillModeNonSolid = false;
    EngineState* mEngineState = nullptr;
    Pipeline* mCurrentlyBoundPipeline = nullptr;
    VkSurfaceTransformFlagBitsKHR mPreTransformFlag = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    float mResolutionScale = 1.0f;
    uint32_t mSceneWidth = 0;
    uint32_t mSceneHeight = 0;

#if EDITOR
public:
    class Node3D* ProcessHitCheck(World* world, int32_t pixelX, int32_t pixelY);
    VkRenderPass GetHitCheckRenderPass();
private:
    void CreateHitCheck();
    void DestroyHitCheck();

    VkFramebuffer mHitCheckFramebuffer = VK_NULL_HANDLE;
    VkRenderPass mHitCheckRenderPass = VK_NULL_HANDLE;
    Image* mHitCheckImage = nullptr;
    Buffer* mHitCheckBuffer = nullptr;
#endif

};

void CreateVulkanContext();
void DestroyVulkanContext();
VulkanContext* GetVulkanContext();

#endif