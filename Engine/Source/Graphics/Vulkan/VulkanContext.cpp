#if API_VULKAN

#include "Graphics/Vulkan/VulkanContext.h"
#include "Log.h"
#include "Engine.h"
#include "Utilities.h"
#include "World.h"
#include "Renderer.h"

#if EDITOR
#include "EditorState.h"
#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#endif

#include "Graphics/GraphicsUtils.h"
#include "System/System.h"

#include "Nodes/3D/Camera3d.h"
#include "Nodes/3D/DirectionalLight3d.h"
#include "Nodes/3D/PointLight3d.h"
#include "Nodes/3D/StaticMesh3d.h"

#include "Assertion.h"
#include <string>
#include <algorithm>
#include <set>

VulkanContext* gVulkanContext = nullptr;

static const char* sValidationLayers[] = { "VK_LAYER_KHRONOS_validation" };
static uint32_t sNumValidationLayers = 1;

static const char* sDeviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
static uint32_t sNumDeviceExtensions = 1;

PFN_vkCmdBeginDebugUtilsLabelEXT CmdBeginDebugUtilsLabelEXT = nullptr;
PFN_vkCmdEndDebugUtilsLabelEXT CmdEndDebugUtilsLabelEXT = nullptr;
PFN_vkCmdInsertDebugUtilsLabelEXT CmdInsertDebugUtilsLabelEXT = nullptr;
PFN_vkSetDebugUtilsObjectNameEXT SetDebugUtilsObjectNameEXT = nullptr;

#define PIPELINE_CACHE_SAVE_NAME "PipelineCache.sav"
#define ENABLE_FULL_VALIDATION 1
#define MAX_GPU_TIMESPANS 64
#define MAX_GPU_TIMESTAMPS (MAX_GPU_TIMESPANS * 2)

void CreateVulkanContext()
{
    OCT_ASSERT(gVulkanContext == nullptr);
    gVulkanContext = new VulkanContext();
}

void DestroyVulkanContext()
{
    OCT_ASSERT(gVulkanContext != nullptr);
    delete gVulkanContext;
    gVulkanContext = nullptr;
}

// Adding this function for Imgui. We can probably remove it.
static void CheckVkResult(VkResult err)
{
    if (err == 0)
        return;
    LogError("[Vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

VulkanContext* GetVulkanContext()
{
    return gVulkanContext;
}


VulkanContext::VulkanContext()
{
    mInstance = 0;
    mCallback = 0;
    mPhysicalDevice = 0;
    mDevice = 0;
    mGraphicsQueue = 0;
    mPresentQueue = 0;
    mSurface = 0;
    mSwapchain = 0;
    mForwardRenderPass = VK_NULL_HANDLE;
    mPostprocessRenderPass = VK_NULL_HANDLE;
    mUIRenderPass = VK_NULL_HANDLE;
    mClearSwapchainPass = VK_NULL_HANDLE;
    mSceneColorFramebuffer = VK_NULL_HANDLE;
    mImageAvailableSemaphore = 0;
    mRenderFinishedSemaphore = 0;
    mSceneColorImageFormat = VK_FORMAT_R8G8B8A8_UNORM;

    mGlobalUniformData.mScreenDimensions = glm::vec2(800.0f, 600.0f);
    mGlobalUniformData.mVisualizationMode = 0;
}

VulkanContext::~VulkanContext()
{

}

void VulkanContext::Initialize()
{
    mEngineState = GetEngineState();

    mValidate = GetEngineConfig()->mValidateGraphics;

    CreateInstance();
    CreateDebugCallback();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapchain();
    CreateImageViews();
    CreateCommandPool();

    CreateShadowMapImage();
    CreateSceneColorImage();
    CreateDepthImage();
    CreateDescriptorPool();
    CreateRenderPass();

#if EDITOR
    CreateHitCheck();
#endif

    CreatePipelineCache();
    CreatePipelines();
    SavePipelineCacheToFile();

    CreateGlobalDescriptorSet();

    mRayTracer.CreateStaticRayTraceResources();
    mRayTracer.CreateDynamicRayTraceResources();

    CreatePostProcessDescriptorSet();
    CreateFramebuffers();
    CreateCommandBuffers();
    CreateSemaphores();
    CreateFences();

    CreateQueryPools();

    // Transition the swapchain image to swapchain present format before hitting render loop
    // or else the image transitions won't use expected initial layout.
    for (uint32_t i = 0; i < mSwapchainImages.size(); ++i)
    {
        TransitionImageLayout(mSwapchainImages[i],
            mSwapchainImageFormat,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            1,
            1);
    }

    mFrameIndex = Renderer::Get()->GetFrameIndex();
    mFrameNumber = Renderer::Get()->GetFrameNumber();

    mMaterialPipelineCache.Create();

#if PLATFORM_ANDROID
    EnableMaterialPipelineCache(true);
#endif

#if EDITOR

    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance = mInstance;
    initInfo.PhysicalDevice = mPhysicalDevice;
    initInfo.Device = mDevice;
    initInfo.QueueFamily = mGraphicsQueueFamily;
    initInfo.Queue = mGraphicsQueue;
    initInfo.PipelineCache = mPipelineCache;
    initInfo.DescriptorPool = mDescriptorPool;
    initInfo.Subpass = 0;
    initInfo.MinImageCount = MAX_FRAMES;
    initInfo.ImageCount = MAX_FRAMES;
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    initInfo.Allocator = nullptr;
    initInfo.CheckVkResultFn = CheckVkResult;
    ImGui_ImplVulkan_Init(&initInfo, mUIRenderPass);

    // Upload Fonts
    {
        VkCommandBuffer cb = BeginCommandBuffer();
        ImGui_ImplVulkan_CreateFontsTexture(cb);
        EndCommandBuffer(cb);
        DeviceWaitIdle();
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }
#endif

    DeviceWaitIdle();
    mInitialized = true;
}

void VulkanContext::Destroy()
{
    DeviceWaitIdle();

#if EDITOR
    ImGui_ImplVulkan_Shutdown();
#endif

    mMaterialPipelineCache.Destroy();

    DestroyQueryPools();

    mMeshDescriptorSetArena.Destroy();
    mMeshUniformBufferArena.Destroy();

    mRayTracer.DestroyStaticRayTraceResources();

    DestroySwapchain();

    DestroyPipelines();
    DestroyPipelineCache();

    mDestroyQueue.FlushAll();

    vkDestroyDescriptorPool(mDevice, mDescriptorPool, nullptr);

    vkDestroySemaphore(mDevice, mRenderFinishedSemaphore, nullptr);
    vkDestroySemaphore(mDevice, mImageAvailableSemaphore, nullptr);

    for (uint32_t i = 0; i < MAX_FRAMES; ++i)
    {
        vkDestroyFence(mDevice, mWaitFences[i], nullptr);
    }

    vkDestroyCommandPool(mDevice, mCommandPool, nullptr);

    DestroyDebugCallback();

    vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
    vkDestroyDevice(mDevice, nullptr);
    vkDestroyInstance(mInstance, nullptr);
}

void VulkanContext::BeginFrame()
{
    // These should always be equivalent, or else we need a way to sync them.
    OCT_ASSERT(uint32_t(mFrameNumber) == Renderer::Get()->GetFrameNumber());
    OCT_ASSERT(uint32_t(mFrameIndex) == Renderer::Get()->GetFrameIndex());

    float resScale = GetEngineState()->mGraphics.mResolutionScale;
    resScale = glm::clamp(resScale, 0.1f, 2.0f);
    GetEngineState()->mGraphics.mResolutionScale = resScale;

    if (mResolutionScale != resScale)
    {
        mResolutionScale = resScale;
        RecreateSwapchain(false);
    }

    VkResult result = vkAcquireNextImageKHR(mDevice, mSwapchain, std::numeric_limits<uint64_t>::max(), mImageAvailableSemaphore, VK_NULL_HANDLE, &mSwapchainImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapchain(false);
    }
    else if (result != VK_SUCCESS &&
             result != VK_SUBOPTIMAL_KHR)
    {
        LogError("Failed to acquire swapchain image");
        OCT_ASSERT(0);
    }

    if (mCommandBuffers.size() == 0)
    {
        CreateCommandBuffers();
    }

    VkCommandBuffer cb = mCommandBuffers[mFrameIndex];

    // Reset our command buffer to record a fresh set of commands for this frame.
    vkResetCommandBuffer(cb, 0);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    vkBeginCommandBuffer(cb, &beginInfo);
    SetDebugObjectName(VK_OBJECT_TYPE_COMMAND_BUFFER, (uint64_t)cb, "FrameCommandBuffer");

    ReadTimeQueryResults();

    mMeshDescriptorSetArena.Reset();
    mMeshUniformBufferArena.Reset();

    if (mEnableMaterialPipelineCache)
    {
        mMaterialPipelineCache.Update();
    }

#if EDITOR
    ImGui_ImplVulkan_NewFrame();
#endif
}

void VulkanContext::EndFrame()
{
    VkCommandBuffer cb = GetCommandBuffer();

    if (vkEndCommandBuffer(cb) != VK_SUCCESS)
    {
        LogError("Failed to record command buffer");
        OCT_ASSERT(0);
    }

    UpdateGlobalUniformData();
    UpdateGlobalDescriptorSet();

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { mImageAvailableSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cb;

    VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphore };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, mWaitFences[mFrameIndex]) != VK_SUCCESS)
    {
        LogError("Failed to submit draw command buffer");
        OCT_ASSERT(0);
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapchains[] = { mSwapchain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &mSwapchainImageIndex;
    presentInfo.pResults = nullptr;

    vkQueuePresentKHR(mPresentQueue, &presentInfo);

    uint32_t nextFrameIndex = (mFrameIndex + 1) % MAX_FRAMES;

    // Ensure this command buffer has finished executing on the GPU
    vkWaitForFences(mDevice, 1, &mWaitFences[nextFrameIndex], VK_TRUE, UINT64_MAX);
    vkResetFences(mDevice, 1, &mWaitFences[nextFrameIndex]);

    // Destroy anything that was queued from two frames ago.
    // It should be safe if we waited to acquire the swapchain image.
    mDestroyQueue.Flush(nextFrameIndex);

    mFrameIndex = nextFrameIndex;
    mFrameNumber++;

    if (mSupportsRayTracing && 
        mRayTracer.GetLightBakePhase() != LightBakePhase::Count)
    {
        mRayTracer.ReadbackLightBakeResults();
    }
}

void VulkanContext::BeginRenderPass(RenderPassId id)
{
    if (mCurrentRenderPassId != RenderPassId::Count)
    {
        LogError("BeginRenderPass called while a render pass is already active.");
        OCT_ASSERT(0);
        return;
    }

    mCurrentRenderPassId = id;

    VkClearValue clearValues[2] = {};
    clearValues[0].color = { 0.0f, 0.0f, 0.0f, 0.0f };
    clearValues[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = mSwapchainExtent;
    renderPassInfo.clearValueCount = OCT_ARRAY_SIZE(clearValues);
    renderPassInfo.pClearValues = clearValues;

    bool barrierNeeded = false;

    switch (id)
    {
    case RenderPassId::Shadows:
        renderPassInfo.renderArea.extent.width = SHADOW_MAP_RESOLUTION;
        renderPassInfo.renderArea.extent.height = SHADOW_MAP_RESOLUTION;
        renderPassInfo.renderPass = mShadowRenderPass;
        renderPassInfo.framebuffer = mShadowFramebuffer;
        clearValues[0].depthStencil = { 1.0f, 0 };
        break;
    case RenderPassId::Forward:
        renderPassInfo.renderArea.extent.width = mSceneWidth;
        renderPassInfo.renderArea.extent.height = mSceneHeight;
        renderPassInfo.renderPass = mForwardRenderPass;
        renderPassInfo.framebuffer = mSceneColorFramebuffer;
        break;
    case RenderPassId::PostProcess:
        renderPassInfo.renderPass = mPostprocessRenderPass;
        renderPassInfo.framebuffer = mSwapchainFramebuffers[mSwapchainImageIndex];
        barrierNeeded = true;
        break;
    case RenderPassId::Ui:
        renderPassInfo.renderPass = mUIRenderPass;
        renderPassInfo.framebuffer = mSwapchainFramebuffers[mSwapchainImageIndex];
        break;
    case RenderPassId::Clear:
        renderPassInfo.renderPass = mClearSwapchainPass;
        renderPassInfo.framebuffer = mSwapchainFramebuffers[mSwapchainImageIndex];
        break;
#if EDITOR
    case RenderPassId::HitCheck:
        renderPassInfo.renderArea.extent.width = mSceneWidth;
        renderPassInfo.renderArea.extent.height = mSceneHeight;
        renderPassInfo.renderPass = mHitCheckRenderPass;
        renderPassInfo.framebuffer = mHitCheckFramebuffer;
        break;
#endif

    default: break;
    }

    if (barrierNeeded)
    {
        // Add a barrier for our attachments to make sure they are fully written to.
        // This barrier is probably overkill. Might need to refine.
        VkMemoryBarrier memoryBarrier = {};
        memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        memoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

        vkCmdPipelineBarrier(
            mCommandBuffers[mFrameIndex],
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            1,
            &memoryBarrier,
            0, nullptr,
            0, nullptr);
    }

    BeginDebugLabel(GetRenderPassName(id));

    if (mCurrentRenderPassId != RenderPassId::HitCheck)
    {
        BeginGpuTimestamp(GetRenderPassName(mCurrentRenderPassId));
    }

    if (mCurrentRenderPassId == RenderPassId::Forward)
    {
        // Forward pass may render to a different resolution than other passes because of Render Scale setting.
        int32_t w = (int32_t)mSceneWidth;
        int32_t h = (int32_t)mSceneHeight;
        SetViewport(0, 0, w, h, false, true);
        SetScissor(0, 0, w, h, false, true);
    }

    vkCmdBeginRenderPass(mCommandBuffers[mFrameIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanContext::EndRenderPass()
{
    // TEMPORARY!!
    // TODO: Remove this after finding out how to avoid double-updating GeometryUniform buffers between shadow and forward passes.
    if (mCurrentRenderPassId == RenderPassId::Shadows)
    {
        //DeviceWaitIdle();
    }

    if (mCurrentRenderPassId == RenderPassId::Forward)
    {
        // Restore the viewport and scissor in case we were rendering at a different resolution scale.
        SetViewport(0, 0, mEngineState->mWindowWidth, mEngineState->mWindowHeight, true, false);
        SetScissor(0, 0, mEngineState->mWindowWidth, mEngineState->mWindowHeight, true, false);
    }

#if EDITOR
    if (mCurrentRenderPassId == RenderPassId::Ui)
    {
        ImDrawData* draw_data = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(draw_data, mCommandBuffers[mFrameIndex]);
    }
#endif

    if (mCurrentRenderPassId != RenderPassId::Count)
    {
        vkCmdEndRenderPass(mCommandBuffers[mFrameIndex]);
        EndDebugLabel();

        if (mCurrentRenderPassId != RenderPassId::HitCheck)
        {
            EndGpuTimestamp(GetRenderPassName(mCurrentRenderPassId));
        }

        mCurrentRenderPassId = RenderPassId::Count;
    }
}

void VulkanContext::BindPipeline(PipelineId id, VertexType vertexType)
{
    Pipeline* pipeline = GetPipeline(id);
    OCT_ASSERT(pipeline != nullptr &&
        pipeline->GetId() == id);
    BindPipeline(pipeline, vertexType);
}

void VulkanContext::BindPipeline(Pipeline* pipeline, VertexType vertexType)
{
    VkCommandBuffer cb = mCommandBuffers[mFrameIndex];
    VkPipelineLayout pipelineLayout = pipeline->GetPipelineLayout();
    
    pipeline->BindPipeline(cb, vertexType);
    mCurrentlyBoundPipeline = pipeline;

    // Always rebind Global Descriptor (might not need to do this)
    VkPipelineBindPoint bindPoint = pipeline->IsComputePipeline() ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS;
    mGlobalDescriptorSet->Bind(cb, (uint32_t)DescriptorSetBinding::Global, pipelineLayout, bindPoint);

    // Handle pipeline-specific functionality
    switch (pipeline->GetId())
    {
    case PipelineId::PostProcess:
    case PipelineId::NullPostProcess:
        mPostProcessDescriptorSet->Bind(cb, (uint32_t)DescriptorSetBinding::PostProcess, pipelineLayout);
        break;

    default: break;
    }
}

void VulkanContext::RebindPipeline(VertexType vertexType)
{
    if (mCurrentlyBoundPipeline != nullptr)
    {
        BindPipeline(mCurrentlyBoundPipeline, vertexType);
    }
}

void VulkanContext::DrawLines(const std::vector<Line>& lines)
{
    if (lines.size() == 0)
        return;

    VkCommandBuffer cb = GetCommandBuffer();

    if (mNumLinesAllocated < int32_t(lines.size()))
    {
        if (mLineVertexBuffer != nullptr)
        {
            GetDestroyQueue()->Destroy(mLineVertexBuffer);
            mLineVertexBuffer = nullptr;
        }


        mLineVertexBuffer = new Buffer(BufferType::Vertex, lines.size() * sizeof(VertexColorSimple) * 2, "Line Vertex Data");
        mNumLinesAllocated = uint32_t(lines.size());
    }

    // Update vertex data
    {
        void* data = mLineVertexBuffer->Map();
        VertexColorSimple* verts = reinterpret_cast<VertexColorSimple*>(data);
        for (int32_t i = 0; i < int32_t(lines.size()); ++i)
        {
            uint32_t color32 = ColorFloat4ToUint32(lines[i].mColor);

            verts[i * 2 + 0].mPosition = lines[i].mStart;
            verts[i * 2 + 0].mColor = color32;

            verts[i * 2 + 1].mPosition = lines[i].mEnd;
            verts[i * 2 + 1].mColor = color32;
        }
        mLineVertexBuffer->Unmap();
    }

    // Render
    {
        BindPipeline(PipelineId::Line, VertexType::VertexColorSimple);

        VkDeviceSize offset = 0;
        VkBuffer lineVertexBuffer = mLineVertexBuffer->Get();
        vkCmdBindVertexBuffers(cb, 0, 1, &lineVertexBuffer, &offset);
        vkCmdDraw(cb, 2 * uint32_t(lines.size()), 1, 0, 0);
    }
}

void VulkanContext::DrawFullscreen()
{
    // TODO: Bind buffer with 4 vertices
    vkCmdDraw(GetCommandBuffer(), 4, 1, 0, 0);
}

void VulkanContext::DestroySwapchain()
{
    for (size_t i = 0; i < mSwapchainFramebuffers.size(); ++i)
    {
        vkDestroyFramebuffer(mDevice, mSwapchainFramebuffers[i], nullptr);
    }
    vkDestroyFramebuffer(mDevice, mSceneColorFramebuffer, nullptr);
    vkDestroyFramebuffer(mDevice, mShadowFramebuffer, nullptr);

    for (uint32_t i = 0; i < mCommandBuffers.size(); ++i)
    {
        GetDestroyQueue()->Destroy(mCommandBuffers[i]);
    }
    mCommandBuffers.clear();

    vkDestroyRenderPass(mDevice, mShadowRenderPass, nullptr);
    vkDestroyRenderPass(mDevice, mForwardRenderPass, nullptr);
    vkDestroyRenderPass(mDevice, mPostprocessRenderPass, nullptr);
    vkDestroyRenderPass(mDevice, mUIRenderPass, nullptr);
    vkDestroyRenderPass(mDevice, mClearSwapchainPass, nullptr);

    mRayTracer.DestroyDynamicRayTraceResources();

    GetDestroyQueue()->Destroy(mShadowMapImage);
    mShadowMapImage = nullptr;

    GetDestroyQueue()->Destroy(mDepthImage);
    mDepthImage = nullptr;

    GetDestroyQueue()->Destroy(mSceneColorImage);
    mSceneColorImage = nullptr;

    GetDestroyQueue()->Destroy(mGlobalUniformBuffer);
    mGlobalUniformBuffer = nullptr;

    GetDestroyQueue()->Destroy(mGlobalDescriptorSet);
    mGlobalDescriptorSet = nullptr;

    for (size_t i = 0; i < mSwapchainImageViews.size(); ++i)
    {
        vkDestroyImageView(mDevice, mSwapchainImageViews[i], nullptr);
    }

    vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);

#if EDITOR
    DestroyHitCheck();
#endif
}

VkDevice VulkanContext::GetDevice()
{
    return mDevice;
}

void VulkanContext::CreateSwapchain()
{
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(mPhysicalDevice);

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

    mPreTransformFlag = swapChainSupport.capabilities.currentTransform;

    uint32_t imageCount = 2;

    if (imageCount < swapChainSupport.capabilities.minImageCount)
        imageCount = swapChainSupport.capabilities.minImageCount;
    if (swapChainSupport.capabilities.maxImageCount != 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount)
        imageCount = swapChainSupport.capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR ciSwapchain = {};
    ciSwapchain.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    ciSwapchain.surface = mSurface;

    ciSwapchain.minImageCount = imageCount;
    ciSwapchain.imageFormat = surfaceFormat.format;
    ciSwapchain.imageColorSpace = surfaceFormat.colorSpace;
    ciSwapchain.imageExtent = extent;
    ciSwapchain.imageArrayLayers = 1;
    ciSwapchain.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = FindQueueFamilies(mPhysicalDevice);
    uint32_t queueFamilyIndices[] = { (uint32_t)indices.mGraphicsFamily, (uint32_t)indices.mPresentFamily };

    mGraphicsQueueFamily = (uint32_t)indices.mGraphicsFamily;
    mPresentQueueFamily = (uint32_t)indices.mPresentFamily;

    if (indices.mGraphicsFamily != indices.mPresentFamily)
    {
        ciSwapchain.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        ciSwapchain.queueFamilyIndexCount = 2;
        ciSwapchain.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        ciSwapchain.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        ciSwapchain.queueFamilyIndexCount = 0;
        ciSwapchain.pQueueFamilyIndices = nullptr;
    }

    VkCompositeAlphaFlagBitsKHR alphaFlags = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    if ((swapChainSupport.capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) == 0)
    {
        alphaFlags = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    }

    ciSwapchain.preTransform = mPreTransformFlag;
    ciSwapchain.compositeAlpha = alphaFlags;
    ciSwapchain.presentMode = presentMode;
    ciSwapchain.clipped = VK_TRUE;
    ciSwapchain.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(mDevice, &ciSwapchain, nullptr, &mSwapchain) != VK_SUCCESS)
    {
        LogError("Failed to create swapchain");
        OCT_ASSERT(0);
    }

    vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, nullptr);
    mSwapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, mSwapchainImages.data());

    mSwapchainImageFormat = surfaceFormat.format;
    mSwapchainExtent = extent;

    // Update scene/width height based on resolution scale.
    float resScale = GetEngineState()->mGraphics.mResolutionScale;
    mSceneWidth = uint32_t(mSwapchainExtent.width * resScale + 0.5f);
    mSceneHeight = uint32_t(mSwapchainExtent.height * resScale + 0.5f);

    mGlobalUniformData.mScreenDimensions = glm::vec2(extent.width, extent.height);
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanContext::DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void* userData)
{
    char prefix[64];
    char* message = (char*)malloc(strlen(callbackData->pMessage) + 500);
    OCT_ASSERT(message);

    bool isError = false;

    // Severity
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
    {
        strcpy(prefix, "[VERBOSE] ");
    }
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
    {
        strcpy(prefix, "[INFO] ");
    }
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        strcpy(prefix, "[WARNING] ");
    }
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        strcpy(prefix, "[ERROR] ");
        isError = true;
    }

    // Type
    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
    {
        strcat(prefix, "General: ");
    }
    else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
    {
        strcat(prefix, "Perf: ");
    }
    else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
    {
        strcat(prefix, "Validation: ");
    }

    sprintf(message,
        "%s - Message ID Number %d, Message ID String %s :\n%s",
        prefix,
        callbackData->messageIdNumber,
        callbackData->pMessageIdName,
        callbackData->pMessage);

    if (callbackData->objectCount > 0)
    {
        char tmp_message[500];
        sprintf(tmp_message, "\n Objects - %d\n", callbackData->objectCount);
        strcat(message, tmp_message);

        for (uint32_t object = 0; object < callbackData->objectCount; ++object)
        {
            sprintf(tmp_message,
                " Object[%d] - Type %d, Value %p, Name \"%s\"\n",
                object,
                callbackData->pObjects[object].objectType,
                (void*)(callbackData->pObjects[object].objectHandle),
                callbackData->pObjects[object].pObjectName);
            strcat(message, tmp_message);
        }
    }
    if (callbackData->cmdBufLabelCount > 0) {
        char tmp_message[500];
        sprintf(tmp_message,
            "\n Command Buffer Labels - %d\n",
            callbackData->cmdBufLabelCount);
        strcat(message, tmp_message);
        for (uint32_t label = 0; label < callbackData->cmdBufLabelCount; ++label) {
            sprintf(tmp_message,
                " Label[%d] - %s { %f, %f, %f, %f}\n",
                label,
                callbackData->pCmdBufLabels[label].pLabelName,
                callbackData->pCmdBufLabels[label].color[0],
                callbackData->pCmdBufLabels[label].color[1],
                callbackData->pCmdBufLabels[label].color[2],
                callbackData->pCmdBufLabels[label].color[3]);
            strcat(message, tmp_message);
        }
    }

    if (isError)
    {
        LogError("%s\n", message);
    }
    else
    {
        LogWarning("%s\n", message);
    }

    fflush(stdout);
    free(message);

    return false;
}

void VulkanContext::CreateInstance()
{
    VkResult result;
    VkBool32 surfaceExtFound = false;
    VkBool32 platformSurfaceExtFound = false;
    VkBool32 debugUtilsExtFound = false;
    uint32_t extensionCount = 0;

    if (mValidate &&
        CheckValidationLayerSupport(sValidationLayers, sNumValidationLayers) == false)
    {
        LogError("Validation layers enabled but the configured layers are not supported.");
        OCT_ASSERT(0);
    }

    result = vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
    OCT_ASSERT(result == VK_SUCCESS);

    if (extensionCount > 0)
    {
        VkExtensionProperties* extensions = reinterpret_cast<VkExtensionProperties*>(malloc(sizeof(VkExtensionProperties) * extensionCount));
        
        result = vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);
        OCT_ASSERT(result == VK_SUCCESS);

        for (uint32_t i = 0; i < extensionCount; i++)
        {
            if (!strcmp(VK_KHR_SURFACE_EXTENSION_NAME, extensions[i].extensionName))
            {
                surfaceExtFound = true;
                mEnabledExtensions[mEnabledExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
            }

#if PLATFORM_WINDOWS
            if (!strcmp(VK_KHR_WIN32_SURFACE_EXTENSION_NAME, extensions[i].extensionName))
            {
                platformSurfaceExtFound = 1;
                mEnabledExtensions[mEnabledExtensionCount++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
            }
#elif PLATFORM_LINUX
            if (!strcmp(VK_KHR_XCB_SURFACE_EXTENSION_NAME, extensions[i].extensionName))
            {
                platformSurfaceExtFound = 1;
                mEnabledExtensions[mEnabledExtensionCount++] = VK_KHR_XCB_SURFACE_EXTENSION_NAME;
            }
#elif PLATFORM_ANDROID
            if (!strcmp(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME, extensions[i].extensionName))
            {
                platformSurfaceExtFound = 1;
                mEnabledExtensions[mEnabledExtensionCount++] = VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
            }
#endif

            if (!strcmp(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, extensions[i].extensionName))
            {
                if (mValidate)
                {
                    mEnabledExtensions[mEnabledExtensionCount++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
                    debugUtilsExtFound = true;
                }
            }
            OCT_ASSERT(mEnabledExtensionCount < MAX_ENABLED_EXTENSIONS);
        }

        free(extensions);
        extensions = nullptr;
    }

    if (!surfaceExtFound)
    {
        LogError("Failed to find surface extension");
        OCT_ASSERT(0);
    }
    if (!platformSurfaceExtFound)
    {
        LogError("Failed to find platform surface extension");
        OCT_ASSERT(0);
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = GetEngineState()->mProjectName.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = ENGINE_NAME;
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkValidationFeatureEnableEXT enables[] = { VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT, VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT };
    VkValidationFeaturesEXT features = {};
    features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
    features.enabledValidationFeatureCount = 2;
    features.pEnabledValidationFeatures = enables;

    VkInstanceCreateInfo ciInstance = {};
    ciInstance.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ciInstance.pApplicationInfo = &appInfo;
    ciInstance.enabledExtensionCount = mEnabledExtensionCount;
    ciInstance.ppEnabledExtensionNames = mEnabledExtensions;
    ciInstance.enabledLayerCount = mEnabledLayersCount;
    ciInstance.ppEnabledLayerNames = mEnabledLayers;

#if ENABLE_FULL_VALIDATION
    ciInstance.pNext = &features;
#endif

    result = vkCreateInstance(&ciInstance, nullptr, &mInstance);

    if (result != VK_SUCCESS)
    {
        LogDebug("Failed to create instance.");
        OCT_ASSERT(0);
    }

    // Setup the function pointers
    if (debugUtilsExtFound)
    {
        CmdBeginDebugUtilsLabelEXT =
            (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(
                mInstance,
                "vkCmdBeginDebugUtilsLabelEXT");
        CmdEndDebugUtilsLabelEXT =
            (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(
                mInstance,
                "vkCmdEndDebugUtilsLabelEXT");
        CmdInsertDebugUtilsLabelEXT =
            (PFN_vkCmdInsertDebugUtilsLabelEXT)vkGetInstanceProcAddr(
                mInstance,
                "vkCmdInsertDebugUtilsLabelEXT");
        SetDebugUtilsObjectNameEXT =
            (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(
                mInstance,
                "vkSetDebugUtilsObjectNameEXT");
    }
}

void VulkanContext::CreateDebugCallback()
{
    if (!mValidate)
    {
        return;
    }

    OCT_ASSERT(mDebugMessenger == VK_NULL_HANDLE);

    PFN_vkCreateDebugUtilsMessengerEXT CreateDebugUtilsMessengerEXT =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            mInstance,
            "vkCreateDebugUtilsMessengerEXT");

    OCT_ASSERT(CreateDebugUtilsMessengerEXT != nullptr);

    VkDebugUtilsMessengerCreateInfoEXT ciDebugMessenger;
    ciDebugMessenger.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    ciDebugMessenger.pNext = NULL;
    ciDebugMessenger.flags = 0;
    ciDebugMessenger.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    ciDebugMessenger.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    ciDebugMessenger.pfnUserCallback = DebugCallback;
    ciDebugMessenger.pUserData = NULL;

    VkResult result = CreateDebugUtilsMessengerEXT(mInstance,
        &ciDebugMessenger,
        NULL,
        &mDebugMessenger);

    if (result != VK_SUCCESS)
    {
        LogError("Failed to setup debug callback!");
        OCT_ASSERT(0);
    }
}

void VulkanContext::CreateSurface()
{
#if PLATFORM_WINDOWS
    PFN_vkCreateWin32SurfaceKHR pfnCreateWin32Surface = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(mInstance, "vkCreateWin32SurfaceKHR");

    OCT_ASSERT(pfnCreateWin32Surface != nullptr);

    VkWin32SurfaceCreateInfoKHR ciSurface = {};
    ciSurface.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    ciSurface.hwnd = mEngineState->mSystem.mWindow;
    ciSurface.hinstance = GetModuleHandle(nullptr);

    if (pfnCreateWin32Surface(mInstance, &ciSurface, nullptr, &mSurface) != VK_SUCCESS)
    {
        LogError("Failed to create window surface.");
        OCT_ASSERT(0);
    }
#elif PLATFORM_LINUX
    VkXcbSurfaceCreateInfoKHR ciSurface = {};
    ciSurface.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    ciSurface.connection = GetEngineState()->mSystem.mXcbConnection;
    ciSurface.window = GetEngineState()->mSystem.mXcbWindow;
        
    VkResult res = vkCreateXcbSurfaceKHR(mInstance, &ciSurface, nullptr, &mSurface);
    if (res != VK_SUCCESS)
    {
        LogError("Failed to create window surface.");
        OCT_ASSERT(0);
    }
#elif PLATFORM_ANDROID
    EngineState* engineState = GetEngineState();
    OCT_ASSERT(engineState->mSystem.mWindow != nullptr);

    VkAndroidSurfaceCreateInfoKHR ciSurface = {};
    ciSurface.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    ciSurface.pNext = nullptr;
    ciSurface.flags = 0;
    ciSurface.window = engineState->mSystem.mWindow;

    VkResult res = vkCreateAndroidSurfaceKHR(mInstance, &ciSurface, nullptr, &mSurface);
    if (res != VK_SUCCESS)
    {
        LogError("Failed to create window surface.");
        OCT_ASSERT(0);
    }
#endif
}

void VulkanContext::PickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        LogError("No physical device found.");
        OCT_ASSERT(0);
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());

    for (const auto& device : devices)
    {
        if (IsDeviceSuitable(device))
        {
            mPhysicalDevice = device;
            break;
        }
    }

    if (mPhysicalDevice == VK_NULL_HANDLE)
    {
        LogError("Failed to find a suitable GPU.");
        OCT_ASSERT(0);
    }

    // Check properties to see what features we have. In the future, possibly
    // check these properties while picking a physical device.
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(mPhysicalDevice, &deviceFeatures);
    mFeatureWideLines = deviceFeatures.wideLines;
    mFeatureFillModeNonSolid = deviceFeatures.fillModeNonSolid;

    {
        bool formatFound = false;
        if (!formatFound)
        {
            VkResult res;
            VkImageFormatProperties imageFormatProps;
            res = vkGetPhysicalDeviceImageFormatProperties(
                mPhysicalDevice,
                VK_FORMAT_D32_SFLOAT_S8_UINT,
                VK_IMAGE_TYPE_2D,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                0,
                &imageFormatProps);

            if (res == VK_SUCCESS)
            {
                mDepthImageFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
                formatFound = true;
            }
        }

        if (!formatFound)
        {
            VkResult res;
            VkImageFormatProperties imageFormatProps;
            res = vkGetPhysicalDeviceImageFormatProperties(
                mPhysicalDevice,
                VK_FORMAT_D24_UNORM_S8_UINT,
                VK_IMAGE_TYPE_2D,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                0,
                &imageFormatProps);

            if (res == VK_SUCCESS)
            {
                mDepthImageFormat = VK_FORMAT_D24_UNORM_S8_UINT;
                formatFound = true;
            }
        }

        if (!formatFound)
        {
            mDepthImageFormat = VK_FORMAT_D32_SFLOAT;
        }
    }

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(mPhysicalDevice, &properties);

#if (PLATFORM_WINDOWS || PLATFORM_LINUX)
    if (properties.limits.maxPerStageDescriptorSampledImages >= PATH_TRACE_MAX_TEXTURES)
    {
        mSupportsRayTracing = true;
    }
#endif

    mTimestampPeriod = properties.limits.timestampPeriod;
}

void VulkanContext::CreateLogicalDevice()
{
    QueueFamilyIndices indices = FindQueueFamilies(mPhysicalDevice);

    float priorities = 1.0f;

    const int QUEUE_GRAPHICS = 0;
    const int QUEUE_PRESENT = 1;
    int queueCount = (indices.mGraphicsFamily != indices.mPresentFamily) ? 2 : 1;

    VkDeviceQueueCreateInfo ciDeviceQueues[2];
    memset(ciDeviceQueues, 0, sizeof(VkDeviceQueueCreateInfo)*2);
    ciDeviceQueues[QUEUE_GRAPHICS].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    ciDeviceQueues[QUEUE_GRAPHICS].queueFamilyIndex = indices.mGraphicsFamily;
    ciDeviceQueues[QUEUE_GRAPHICS].queueCount = 1;
    ciDeviceQueues[QUEUE_GRAPHICS].pQueuePriorities = &priorities;

    ciDeviceQueues[QUEUE_PRESENT].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    ciDeviceQueues[QUEUE_PRESENT].queueFamilyIndex = indices.mPresentFamily;
    ciDeviceQueues[QUEUE_PRESENT].queueCount = 1;
    ciDeviceQueues[QUEUE_PRESENT].pQueuePriorities = &priorities;

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.fillModeNonSolid = mFeatureFillModeNonSolid;
    deviceFeatures.wideLines = mFeatureWideLines;

    VkDeviceCreateInfo ciDevice = {};
    ciDevice.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    ciDevice.pQueueCreateInfos = ciDeviceQueues;
    ciDevice.queueCreateInfoCount = queueCount;
    ciDevice.pEnabledFeatures = &deviceFeatures;
    ciDevice.enabledExtensionCount = sNumDeviceExtensions;
    ciDevice.ppEnabledExtensionNames = sDeviceExtensions;

    if (mValidate)
    {
        ciDevice.enabledLayerCount = sNumValidationLayers;
        ciDevice.ppEnabledLayerNames = sValidationLayers;
    }
    else
    {
        ciDevice.enabledLayerCount = 0;
    }

    VkResult result = vkCreateDevice(mPhysicalDevice, &ciDevice, nullptr, &mDevice);

    if (result != VK_SUCCESS)
    {
        LogError("Failed to create logical device.");
        OCT_ASSERT(0);
    }

    vkGetDeviceQueue(mDevice, indices.mGraphicsFamily, 0, &mGraphicsQueue);
    vkGetDeviceQueue(mDevice, indices.mPresentFamily, 0, &mPresentQueue);
}

void VulkanContext::CreateImageViews()
{
    mSwapchainImageViews.resize(mSwapchainImages.size());

    for (size_t i = 0; i < mSwapchainImages.size(); ++i)
    {
        // ImageView
        VkImageViewCreateInfo ciImageView = {};
        ciImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ciImageView.image = mSwapchainImages[i];
        ciImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ciImageView.format = mSwapchainImageFormat;
        ciImageView.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
        ciImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ciImageView.subresourceRange.baseMipLevel = 0;
        ciImageView.subresourceRange.baseArrayLayer = 0;
        ciImageView.subresourceRange.layerCount = 1;
        ciImageView.subresourceRange.levelCount = 1;

        if (vkCreateImageView(mDevice, &ciImageView, nullptr, &mSwapchainImageViews[i]) != VK_SUCCESS)
        {
            LogError("Failed to create swapchain image view");
            OCT_ASSERT(0);
        }

        SetDebugObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)mSwapchainImageViews[i], "Swapchain Image View");
    }
}

void VulkanContext::CreateSceneColorImage()
{
    VkFormat format;
    VkImageUsageFlags usage;
    //VkImageAspectFlags aspect;
    VkImageLayout layout;
    
    format = mSceneColorImageFormat;
    usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    //aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    ImageDesc imageDesc;
    imageDesc.mWidth = mSceneWidth;
    imageDesc.mHeight = mSceneHeight;
    imageDesc.mFormat = format;
    imageDesc.mUsage = usage;

    SamplerDesc samplerDesc;
    samplerDesc.mAddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

    mSceneColorImage = new Image(imageDesc, samplerDesc, "Scene Color");

    mSceneColorImage->Transition(layout);

    DeviceWaitIdle();
}

void VulkanContext::CreateShadowMapImage()
{
    ImageDesc imageDesc;
    imageDesc.mWidth = SHADOW_MAP_RESOLUTION;
    imageDesc.mHeight = SHADOW_MAP_RESOLUTION;
    imageDesc.mFormat = VK_FORMAT_D16_UNORM;
    imageDesc.mUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    SamplerDesc samplerDesc;
    samplerDesc.mMagFilter = VK_FILTER_NEAREST;
    samplerDesc.mMinFilter = VK_FILTER_NEAREST;
    samplerDesc.mAddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerDesc.mBorderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    mShadowMapImage = new Image(imageDesc, samplerDesc, "Shadow Map");

    mShadowMapImage->Transition(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    DeviceWaitIdle();
}

void VulkanContext::CreateQueryPools()
{
#if PROFILING_ENABLED
    if (!mTimestampsSupported)
        return;

    VkCommandBuffer cb = BeginCommandBuffer();

    for (uint32_t i = 0; i < MAX_FRAMES; ++i)
    {
        VkQueryPoolCreateInfo ci = {};
        ci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
        ci.flags = 0;
        ci.queryType = VK_QUERY_TYPE_TIMESTAMP;
        ci.queryCount = MAX_GPU_TIMESTAMPS;

        VkResult res = vkCreateQueryPool(mDevice, &ci, nullptr, &mTimeQueryPools[i]);
        if (res != VK_SUCCESS)
        {
            LogError("Failed to create timestamp query pool.");
            OCT_ASSERT(0);
        }

        vkCmdResetQueryPool(cb, mTimeQueryPools[i], 0, MAX_GPU_TIMESTAMPS);
    }

    EndCommandBuffer(cb);
#endif
}

void VulkanContext::DestroyQueryPools()
{
#if PROFILING_ENABLED
    if (!mTimestampsSupported)
        return;

    for (uint32_t i = 0; i < MAX_FRAMES; ++i)
    {
        vkDestroyQueryPool(mDevice, mTimeQueryPools[i], nullptr);
    }
#endif
}

DestroyQueue* VulkanContext::GetDestroyQueue()
{
    return &mDestroyQueue;
}

VkFormat VulkanContext::GetSwapchainFormat()
{
    return mSwapchainImageFormat;
}

VkFormat VulkanContext::GetSceneColorFormat()
{
    return mSceneColorImageFormat;
}

void VulkanContext::CreateRenderPass()
{
    // I was getting SYNC-HAZARD-READ-AFTER-WRITE validation warnings 
    // (and was getting incorrect rendering results on the RG552). I thought
    // render passes were supposed to automatically handle the transition
    // to the final output (which it seems to do) but unless I add this external 
    // subpass dependency, I get the read after write warnings.
    VkSubpassDependency extDependency = {};
    extDependency.srcSubpass = 0;
    extDependency.dstSubpass = VK_SUBPASS_EXTERNAL;
    extDependency.srcStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    extDependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    extDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    extDependency.dstAccessMask = 0;
    extDependency.dependencyFlags = 0;

    // Shadow Pass
    {
        VkAttachmentDescription attachmentDesc = {};
        attachmentDesc.format = VK_FORMAT_D16_UNORM;
        attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        //attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        //attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference attachmentRef = {};
        attachmentRef.attachment = 0;
        attachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 0;
        subpass.pDepthStencilAttachment = &attachmentRef;

        VkRenderPassCreateInfo ciRenderPass = {};
        ciRenderPass.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        ciRenderPass.attachmentCount = 1;
        ciRenderPass.pAttachments = &attachmentDesc;
        ciRenderPass.subpassCount = 1;
        ciRenderPass.pSubpasses = &subpass;
        ciRenderPass.dependencyCount = 1;
        ciRenderPass.pDependencies = &extDependency;

        if (vkCreateRenderPass(mDevice, &ciRenderPass, nullptr, &mShadowRenderPass) != VK_SUCCESS)
        {
            LogError("Failed to create shadow renderpass");
            OCT_ASSERT(0);
        }

        SetDebugObjectName(VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)mShadowRenderPass, "Shadow RenderPass");
    }

    // Forward Pass
    {
        VkAttachmentDescription attachments[] =
        {
            // Scene Color
            {
                0,
                mSceneColorImageFormat,
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            },

            // Depth Buffer
            {
                0,
				mDepthImageFormat,
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
            }
        };

        VkAttachmentReference colorRef =
        {
            0,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };

        VkAttachmentReference depthRef =
        {
            1,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        };

        VkSubpassDescription subpass =
        {
            0, // flags
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            0, // input attachments
            nullptr,
            1, // color attachments
            &colorRef,
            nullptr,
            &depthRef, // depth attachment
            0,
            nullptr
        };

        VkRenderPassCreateInfo ciRenderPass =
        {
            VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            nullptr,
            0,
            OCT_ARRAY_SIZE(attachments),
            attachments,
            1,
            &subpass,
            1,
            &extDependency
        };

        if (vkCreateRenderPass(mDevice, &ciRenderPass, nullptr, &mForwardRenderPass) != VK_SUCCESS)
        {
            LogError("Failed to create forward render pass");
            OCT_ASSERT(0);
        }

        SetDebugObjectName(VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)mForwardRenderPass, "Forward RenderPass");
    }

    // Postprocess Pass
    {
        VkAttachmentDescription attachments[] =
        {
            // Swapchain Image
            {
                0,
                mSwapchainImageFormat,
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            }
        };

        VkAttachmentReference colorRef =
        {
            0,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };

        VkSubpassDescription subpass =
        {
            0, // flags
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            0, // input attachments
            nullptr,
            1, // color attachments
            &colorRef,
            nullptr,
            nullptr, // depth attachment
            0,
            nullptr
        };

        VkRenderPassCreateInfo ciRenderPass =
        {
            VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            nullptr,
            0,
            OCT_ARRAY_SIZE(attachments),
            attachments,
            1,
            &subpass,
            1,
            &extDependency
        };

        if (vkCreateRenderPass(mDevice, &ciRenderPass, nullptr, &mPostprocessRenderPass) != VK_SUCCESS)
        {
            LogError("Failed to create postprocess render pass");
            OCT_ASSERT(0);
        }

        SetDebugObjectName(VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)mPostprocessRenderPass, "PostProcess RenderPass");
    }

    // UI Pass
    {
        VkAttachmentDescription attachments[] =
        {
            // Swapchain Image
            {
                0,
                mSwapchainImageFormat,
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_LOAD,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
            }
        };

        VkAttachmentReference colorRef =
        {
            0,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };

        VkSubpassDescription subpass =
        {
            0, // flags
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            0, // input attachments
            nullptr,
            1, // color attachments
            &colorRef,
            nullptr,
            nullptr, // depth attachment
            0,
            nullptr
        };

        VkRenderPassCreateInfo ciRenderPass =
        {
            VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            nullptr,
            0,
            OCT_ARRAY_SIZE(attachments),
            attachments,
            1,
            &subpass,
            1,
            &extDependency
        };

        if (vkCreateRenderPass(mDevice, &ciRenderPass, nullptr, &mUIRenderPass) != VK_SUCCESS)
        {
            LogError("Failed to create UI render pass");
            OCT_ASSERT(0);
        }

        SetDebugObjectName(VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)mUIRenderPass, "UI RenderPass");
    }

    // Clear Swapchain Pass
    {
        VkAttachmentDescription attachments[] =
        {
            // Swapchain Image
            {
                0,
                mSwapchainImageFormat,
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            }
        };

        VkAttachmentReference colorRef =
        {
            0,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };

        VkSubpassDescription subpass =
        {
            0, // flags
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            0, // input attachments
            nullptr,
            1, // color attachments
            &colorRef,
            nullptr,
            nullptr, // depth attachment
            0,
            nullptr
        };

        VkRenderPassCreateInfo ciRenderPass =
        {
            VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            nullptr,
            0,
            OCT_ARRAY_SIZE(attachments),
            attachments,
            1,
            &subpass,
            1,
            &extDependency
        };

        if (vkCreateRenderPass(mDevice, &ciRenderPass, nullptr, &mClearSwapchainPass) != VK_SUCCESS)
        {
            LogError("Failed to create ClearSwapchain pass");
            OCT_ASSERT(0);
        }

        SetDebugObjectName(VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)mClearSwapchainPass, "ClearSwapchain RenderPass");
    }
}

void VulkanContext::CreatePipelineCache()
{
    const char* initData = nullptr;
    size_t initSize = 0;

    Stream pipelineData;
    if (SYS_DoesSaveExist(PIPELINE_CACHE_SAVE_NAME))
    {
        if (SYS_ReadSave(PIPELINE_CACHE_SAVE_NAME, pipelineData))
        {
            initData = pipelineData.GetData();
            initSize = (size_t)pipelineData.GetSize();
        }
    }

    OCT_ASSERT(mPipelineCache == VK_NULL_HANDLE);
    VkPipelineCacheCreateInfo ciCache = {};
    ciCache.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    ciCache.pInitialData = (void*)initData;
    ciCache.initialDataSize = initSize;
    ciCache.flags = 0;
    
    if (vkCreatePipelineCache(mDevice, &ciCache, nullptr, &mPipelineCache) != VK_SUCCESS)
    {
        LogError("Failed to create pipeline cache");
        OCT_ASSERT(0);
    }
}

void VulkanContext::DestroyPipelineCache()
{
    OCT_ASSERT(mPipelineCache != VK_NULL_HANDLE);
    vkDestroyPipelineCache(mDevice, mPipelineCache, nullptr);
}

void VulkanContext::CreateFramebuffers()
{
    mSwapchainFramebuffers.resize(mSwapchainImageViews.size());

    for (size_t i = 0; i < mSwapchainImageViews.size(); ++i)
    {
        VkImageView attachmentViews[] = { mSwapchainImageViews[i] };

        VkFramebufferCreateInfo ciFramebuffer = {};
        ciFramebuffer.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        ciFramebuffer.renderPass = mUIRenderPass;
        ciFramebuffer.attachmentCount = OCT_ARRAY_SIZE(attachmentViews);
        ciFramebuffer.pAttachments = attachmentViews;
        ciFramebuffer.width = mSwapchainExtent.width;
        ciFramebuffer.height = mSwapchainExtent.height;
        ciFramebuffer.layers = 1;

        if (vkCreateFramebuffer(mDevice, &ciFramebuffer, nullptr, &mSwapchainFramebuffers[i]) != VK_SUCCESS)
        {
            LogError("Failed to create framebuffer.");
            OCT_ASSERT(0);
        }
    }

    {
        OCT_ASSERT(mShadowRenderPass != VK_NULL_HANDLE);

        VkImageView shadowMapImageView = GetShadowMapImage()->GetView();

        VkFramebufferCreateInfo ciFramebuffer = {};
        ciFramebuffer.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        ciFramebuffer.renderPass = mShadowRenderPass;
        ciFramebuffer.attachmentCount = 1;
        ciFramebuffer.pAttachments = &shadowMapImageView;
        ciFramebuffer.width = SHADOW_MAP_RESOLUTION;
        ciFramebuffer.height = SHADOW_MAP_RESOLUTION;
        ciFramebuffer.layers = 1;

        if (vkCreateFramebuffer(mDevice, &ciFramebuffer, nullptr, &mShadowFramebuffer) != VK_SUCCESS)
        {
            LogError("Failed to create shadow framebuffer.");
            OCT_ASSERT(0);
        }
    }

    {
        VkImageView attachmentViews[] = { mSceneColorImage->GetView(), mDepthImage->GetView() };

        VkFramebufferCreateInfo ciFramebuffer = {};
        ciFramebuffer.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        ciFramebuffer.renderPass = mForwardRenderPass;
        ciFramebuffer.attachmentCount = OCT_ARRAY_SIZE(attachmentViews);
        ciFramebuffer.pAttachments = attachmentViews;
        ciFramebuffer.width = mSceneWidth;
        ciFramebuffer.height = mSceneHeight;
        ciFramebuffer.layers = 1;

        if (vkCreateFramebuffer(mDevice, &ciFramebuffer, nullptr, &mSceneColorFramebuffer) != VK_SUCCESS)
        {
            LogError("Failed to create framebuffer.");
            OCT_ASSERT(0);
        }
    }
}

void VulkanContext::CreateDepthImage()
{
    ImageDesc imageDesc;
    imageDesc.mWidth = mSceneWidth;
    imageDesc.mHeight = mSceneHeight;
    imageDesc.mFormat = mDepthImageFormat;
    imageDesc.mUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    mDepthImage = new Image(imageDesc, SamplerDesc(), "Depth");

    mDepthImage->Transition(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    DeviceWaitIdle();
}

void VulkanContext::CreateGlobalUniformBuffer()
{
    mGlobalUniformBuffer = new UniformBuffer(sizeof(GlobalUniformData), "Global Uniforms");
}

void VulkanContext::UpdateGlobalUniformData()
{
    // Update the camera
    World* world = GetWorld();

    if (world != nullptr &&
        world->GetActiveCamera() != nullptr)
    {
        Camera3D* camera = world->GetActiveCamera();
        EngineState* engineState = GetEngineState();

        mGlobalUniformData.mViewProjMatrix = camera->GetViewProjectionMatrix();
        mGlobalUniformData.mViewPosition = glm::vec4(camera->GetAbsolutePosition(), 1.0f);
        mGlobalUniformData.mViewDirection = glm::vec4(camera->GetForwardVector(), 0.0f);
        mGlobalUniformData.mViewToWorld = glm::inverse(camera->GetViewMatrix());

        // Determine pre-rotation matrix. This matrix is used in widget shaders to rotate them.
        // 3D objects already have their prerotation baked into their WVP matrix.
        glm::mat3 preRotateMat = glm::mat3(1.0f);
        glm::vec3 rotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);

        if (mPreTransformFlag & VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR) 
        {
            preRotateMat = glm::rotate(preRotateMat, glm::radians(90.0f));
        }
        else if (mPreTransformFlag & VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR)
        {
            preRotateMat = glm::rotate(preRotateMat, glm::radians(270.0f));
        }
        else if (mPreTransformFlag & VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR)
        {
            preRotateMat = glm::rotate(preRotateMat, glm::radians(180.0f));
        }

        mGlobalUniformData.mPreRotationMatrix = glm::mat4(preRotateMat);

        mGlobalUniformData.mAmbientLightColor = world->GetAmbientLightColor();

        const std::vector<LightData>& lightData = Renderer::Get()->GetLightData();
        mGlobalUniformData.mNumLights = glm::min<uint32_t>(uint32_t(lightData.size()), MAX_LIGHTS_PER_FRAME);
        
        for (uint32_t i = 0; i < MAX_LIGHTS_PER_FRAME; ++i)
        {
            LightUniformData& lightUni = mGlobalUniformData.mLights[i];

            if (i < lightData.size())
            {
                const LightData& light = lightData[i];
                lightUni.mPosition = light.mPosition;
                lightUni.mRadius = light.mRadius;
                lightUni.mColor = light.mColor;
                lightUni.mDirection = light.mDirection;
                lightUni.mType = (uint32_t)light.mType;
            }
            else
            {
                lightUni.mPosition = glm::vec3(0,0,0);
                lightUni.mRadius = 0.0f;
                lightUni.mColor = glm::vec4(0, 0, 0, 1);
                lightUni.mDirection = glm::vec3(1.0f, 0.0f, 0.0f);
                lightUni.mType = (uint32_t)LightType::Count;
            }
        }

        mGlobalUniformData.mShadowColor = world->GetShadowColor();

        const FogSettings& fog = world->GetFogSettings();
        mGlobalUniformData.mFogColor = fog.mColor;
        mGlobalUniformData.mFogEnabled = fog.mEnabled;
        mGlobalUniformData.mFogDensityFunc = int32_t(fog.mDensityFunc);
        mGlobalUniformData.mFogNear = fog.mNear;
        mGlobalUniformData.mFogFar = fog.mFar;

        mGlobalUniformData.mNearHalfWidth = 0.5f * camera->GetNearWidth();
        mGlobalUniformData.mNearHalfHeight = 0.5f * camera->GetNearHeight();
        mGlobalUniformData.mNearDist = camera->GetNearZ();

        mGlobalUniformData.mGameTime = engineState->mGameElapsedTime;
        mGlobalUniformData.mRealTime = engineState->mRealElapsedTime;
        mGlobalUniformData.mFrameNumber = Renderer::Get()->GetFrameNumber();

        mGlobalUniformData.mPathTracingEnabled = Renderer::Get()->IsPathTracingEnabled();
    }

    mGlobalUniformData.mInterfaceResolution = Renderer::Get()->GetScreenResolution();
}

VkPhysicalDevice VulkanContext::GetPhysicalDevice()
{
    return mPhysicalDevice;
}

GlobalUniformData& VulkanContext::GetGlobalUniformData()
{
    return mGlobalUniformData;
}

bool VulkanContext::IsValidationEnabled() const
{
    return mValidate;
}

bool VulkanContext::IsRayTracingSupported() const
{
    return mSupportsRayTracing;
}

bool VulkanContext::HasFeatureWideLines() const
{
    return mFeatureWideLines;
}

bool VulkanContext::HasFeatureFillModeNonSolid() const
{
    return mFeatureFillModeNonSolid;
}

bool VulkanContext::AreMaterialsEnabled() const
{
    return mEnableMaterials;
}

void VulkanContext::EnableMaterials(bool enable)
{
    mEnableMaterials = enable;
}

void VulkanContext::UpdateGlobalDescriptorSet()
{
    mGlobalUniformBuffer->Update(&mGlobalUniformData, sizeof(GlobalUniformData));
    mGlobalDescriptorSet->UpdateImageDescriptor(GLD_SHADOW_MAP, mShadowMapImage);
}

void VulkanContext::CreateGlobalDescriptorSet()
{
    CreateGlobalUniformBuffer();

    VkDescriptorSetLayout layout = GetPipeline(PipelineId::Opaque)->GetDescriptorSetLayout(0);
    mGlobalDescriptorSet = new DescriptorSet(layout);
    mGlobalDescriptorSet->UpdateUniformDescriptor(GLD_UNIFORM_BUFFER, mGlobalUniformBuffer);
    mGlobalDescriptorSet->UpdateImageDescriptor(GLD_SHADOW_MAP, mShadowMapImage);

    UpdateGlobalDescriptorSet();
}

void VulkanContext::CreatePostProcessDescriptorSet()
{
    VkDescriptorSetLayout layout = GetPipeline(PipelineId::PostProcess)->GetDescriptorSetLayout(1);
    mPostProcessDescriptorSet = new DescriptorSet(layout);
    mPostProcessDescriptorSet->UpdateImageDescriptor(0, mSceneColorImage);
    mPostProcessDescriptorSet->UpdateImageDescriptor(1, mSupportsRayTracing ? mRayTracer.GetPathTraceImage() : mSceneColorImage);
}

void VulkanContext::CreateCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(mPhysicalDevice);

    VkCommandPoolCreateInfo ciCommandPool = {};
    ciCommandPool.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    ciCommandPool.queueFamilyIndex = queueFamilyIndices.mGraphicsFamily;
    ciCommandPool.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(mDevice, &ciCommandPool, nullptr, &mCommandPool))
    {
        LogError("Failed to create command pool");
        OCT_ASSERT(0);
    }
}

void VulkanContext::CreateCommandBuffers()
{
    if (mCommandBuffers.size() == 0)
    {
        mCommandBuffers.resize(mSwapchainFramebuffers.size());

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = mCommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)mCommandBuffers.size();

        if (vkAllocateCommandBuffers(mDevice, &allocInfo, mCommandBuffers.data()) != VK_SUCCESS)
        {
            LogError("Failed to create command buffers");
            OCT_ASSERT(0);
        }
    }
}

uint32_t VulkanContext::GetFrameIndex() const
{
    return mFrameIndex;
}

RenderPassId VulkanContext::GetCurrentRenderPassId() const
{
    return mCurrentRenderPassId;
}

Image* VulkanContext::GetShadowMapImage()
{
    return mShadowMapImage;
}

VkCommandBuffer VulkanContext::GetCommandBuffer()
{
    return mCommandBuffers[mFrameIndex];
}

VkCommandPool VulkanContext::GetCommandPool()
{
    return mCommandPool;
}

VkQueue VulkanContext::GetGraphicsQueue()
{
    return mGraphicsQueue;
}

void VulkanContext::CreateSemaphores()
{
    VkSemaphoreCreateInfo ciSemaphore = {};
    ciSemaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if (vkCreateSemaphore(mDevice, &ciSemaphore, nullptr, &mImageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(mDevice, &ciSemaphore, nullptr, &mRenderFinishedSemaphore) != VK_SUCCESS)
    {
        LogError("Failed to create semaphores");
        OCT_ASSERT(0);
    }
}

void VulkanContext::CreateFences()
{
    VkFenceCreateInfo ciFence = {};
    ciFence.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    ciFence.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (uint32_t i = 0; i < MAX_FRAMES; ++i)
    {
        vkCreateFence(mDevice, &ciFence, nullptr, &mWaitFences[i]);
    }

    // The first frame's fence should start unsignaled.
    vkResetFences(mDevice, 1, &mWaitFences[0]);
}

void VulkanContext::CreateDescriptorPool()
{
    VkDescriptorPoolSize poolSizes[4] = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = MAX_UNIFORM_BUFFER_DESCRIPTORS;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = MAX_SAMPLER_DESCRIPTORS;
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[2].descriptorCount = MAX_STORAGE_BUFFER_DESCRIPTORS;
    poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    poolSizes[3].descriptorCount = MAX_STORAGE_IMAGE_DESCRIPTORS;

    VkDescriptorPoolCreateInfo ciPool = {};
    ciPool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    ciPool.poolSizeCount = 4;
    ciPool.pPoolSizes = poolSizes;
    ciPool.maxSets = MAX_DESCRIPTOR_SETS;
    ciPool.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    if (vkCreateDescriptorPool(mDevice, &ciPool, nullptr, &mDescriptorPool) != VK_SUCCESS)
    {
        LogError("Failed to create descriptor pool");
        OCT_ASSERT(0);
    }
}

void VulkanContext::RecreateSwapchain(bool recreateSurface)
{
    if (!mInitialized)
    {
        return;
    }

    if (mEnableMaterialPipelineCache)
    {
        // Need to stop thread temporarily while resizing to avoid data access hazards.
        mMaterialPipelineCache.Enable(false);
    }

    DeviceWaitIdle();

    DestroySwapchain();

    if (recreateSurface)
    {
        RecreateSurface();
    }

    CreateSwapchain();
    CreateImageViews();
    CreateDepthImage();
    CreateSceneColorImage();
    CreateShadowMapImage();
    CreateRenderPass();
    CreateFramebuffers();
    CreateGlobalDescriptorSet();
    mRayTracer.CreateDynamicRayTraceResources();
    CreatePostProcessDescriptorSet();

#if EDITOR
    CreateHitCheck();
#endif

    // Transition the swapchain image to swapchain present format before hitting render loop
    // or else the image transitions won't use expected initial layout.
    for (uint32_t i = 0; i < mSwapchainImages.size(); ++i)
    {
        TransitionImageLayout(mSwapchainImages[i],
            mSwapchainImageFormat,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            1,
            1);
    }

    // If we don't flush the destroy queue, then constantly resizing the window 
    // may lead to an OOM crash in the VRAM allocator.
    DeviceWaitIdle();
    GetDestroyQueue()->FlushAll();

    if (mEnableMaterialPipelineCache)
    {
        mMaterialPipelineCache.Enable(true);
    }
}

void VulkanContext::RecreateSurface()
{
    if (mSurface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
    }

    CreateSurface();
}

VkDescriptorPool VulkanContext::GetDescriptorPool()
{
    return mDescriptorPool;
}

DescriptorSet* VulkanContext::GetGlobalDescriptorSet()
{
    return mGlobalDescriptorSet;
}

bool VulkanContext::IsDeviceSuitable(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    bool extensionsSupported = CheckDeviceExtensionSupport(device, sDeviceExtensions, sNumDeviceExtensions);

    QueueFamilyIndices indices = FindQueueFamilies(device);

    bool swapChainAdequate = false;

    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return indices.IsComplete() && extensionsSupported && swapChainAdequate;
}

VkSurfaceFormatKHR VulkanContext::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    if (availableFormats.size() == 0)
    {
        LogError("No available formats for swap surface.");
        OCT_ASSERT(0);
    }

    if (availableFormats.size() == 1 &&
        availableFormats[0].format == VK_FORMAT_UNDEFINED)
    {
        return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
    }

    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR VulkanContext::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availableModes)
{
    std::vector<VkPresentModeKHR> preferredModes = { VK_PRESENT_MODE_FIFO_KHR,
                                                     VK_PRESENT_MODE_IMMEDIATE_KHR,
                                                     VK_PRESENT_MODE_FIFO_RELAXED_KHR,
                                                     VK_PRESENT_MODE_MAILBOX_KHR };

    for (VkPresentModeKHR mode : preferredModes)
    {
        if (std::find(availableModes.begin(), availableModes.end(), mode) != availableModes.end())
        {
            return mode;
        }
    }

    LogError("Could not find a valid present mode for swapchain.");
    OCT_ASSERT(0);
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanContext::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    VkExtent2D retExtent = capabilities.currentExtent;

#if PLATFORM_ANDROID
    retExtent.width = capabilities.currentExtent.width;
    retExtent.height = capabilities.currentExtent.height;

    if (capabilities.currentTransform & VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR ||
        capabilities.currentTransform & VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR)
    {
        // Swap to get identity width and height
        uint32_t oldWidth = retExtent.width;
        uint32_t oldHeight = retExtent.height;
        retExtent.height = oldWidth;
        retExtent.width = oldHeight;
}
#else

    // Not entirely sure what this if statement is doing yet.
    if (retExtent.width == std::numeric_limits<uint32_t>::max())
    {
        retExtent = { mEngineState->mWindowWidth, mEngineState->mWindowHeight };

        retExtent.width = glm::clamp(retExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        retExtent.height = glm::clamp(retExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    }
#endif

    return retExtent;
}

QueueFamilyIndices VulkanContext::FindQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int32_t i = 0;
    
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueCount > 0 &&
            (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) /*&&
            (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)*/)
        {
            indices.mGraphicsFamily = i;

            if (queueFamily.timestampValidBits > 0)
            {
                mTimestampsSupported = true;
            }
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface, &presentSupport);

        if (queueFamily.queueCount > 0 &&
            presentSupport)
        {
            indices.mPresentFamily = i;
        }

        if (indices.IsComplete())
        {
            break;
        }

        i++;
    }

    return indices;
}

SwapChainSupportDetails VulkanContext::QuerySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, mSurface, &details.capabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

bool VulkanContext::CheckValidationLayerSupport(const char** layers, uint32_t count)
{
    uint32_t numLayers = 0;
    vkEnumerateInstanceLayerProperties(&numLayers, nullptr);

    VkLayerProperties* layerProperties = (VkLayerProperties*) malloc(numLayers * sizeof(VkLayerProperties));
    vkEnumerateInstanceLayerProperties(&numLayers, layerProperties);

    bool supported = true;
    
    for (uint32_t j = 0; j < count; j++)
    {
        bool layerFound = false;

        for (uint32_t i = 0; i < numLayers; i++)
        {
            if (strcmp(layers[j], layerProperties[i].layerName) == 0)
            {
                layerFound = true;
                mEnabledLayers[mEnabledLayersCount++] = layers[j];
                break;
            }
        }

        if (!layerFound)
        {
            supported = false;
            break;
        }
    }

    free(layerProperties);
    return supported;
}

bool VulkanContext::CheckDeviceExtensionSupport(VkPhysicalDevice device,
                                           const char** extensions,
                                           uint32_t count)
{
    uint32_t availableExtensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &availableExtensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &availableExtensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions;

    for (uint32_t i = 0; i < sNumDeviceExtensions; ++i)
    {
        requiredExtensions.insert(sDeviceExtensions[i]);
    }

    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

void VulkanContext::DestroyDebugCallback()
{
    if (mDebugMessenger != VK_NULL_HANDLE)
    {
        PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessengerEXT =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
                mInstance,
                "vkDestroyDebugUtilsMessengerEXT");

        if (DestroyDebugUtilsMessengerEXT != nullptr)
        {
            DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
        }
    }
}

DescriptorSetArena& VulkanContext::GetMeshDescriptorSetArena()
{
    return mMeshDescriptorSetArena;
}

UniformBufferArena& VulkanContext::GetMeshUniformBufferArena()
{
    return mMeshUniformBufferArena;
}

void VulkanContext::BeginGpuTimestamp(const char* name)
{
#if PROFILING_ENABLED

    if (!mTimestampsSupported)
        return;

    GpuTimespan* timeSpan = nullptr;
    for (uint32_t i = 0; i < mGpuTimespans[mFrameIndex].size(); ++i)
    {
        if (mGpuTimespans[mFrameIndex][i].mName == name)
        {
            timeSpan = &mGpuTimespans[mFrameIndex][i];
            break;
        }
    }

    if (timeSpan == nullptr)
    {
        mGpuTimespans[mFrameIndex].push_back(GpuTimespan());
        timeSpan = &(mGpuTimespans[mFrameIndex].back());
        timeSpan->mName = name;
    }

    timeSpan->mStartIndex = mNumTimestamps[mFrameIndex];
    mNumTimestamps[mFrameIndex]++;

    OCT_ASSERT(timeSpan->mStartIndex < MAX_GPU_TIMESTAMPS);

    vkCmdWriteTimestamp(
        GetCommandBuffer(),
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        mTimeQueryPools[mFrameIndex],
        (uint32_t) timeSpan->mStartIndex);
#endif
}

void VulkanContext::EndGpuTimestamp(const char* name)
{
#if PROFILING_ENABLED

    if (!mTimestampsSupported)
        return;

    GpuTimespan* timeSpan = nullptr;
    for (uint32_t i = 0; i < mGpuTimespans[mFrameIndex].size(); ++i)
    {
        if (mGpuTimespans[mFrameIndex][i].mName == name)
        {
            timeSpan = &mGpuTimespans[mFrameIndex][i];
            break;
        }
    }

    // A BeginGpuTimestamp() call should have been made.
    OCT_ASSERT(timeSpan != nullptr);

    timeSpan->mEndIndex = mNumTimestamps[mFrameIndex];
    mNumTimestamps[mFrameIndex]++;

    OCT_ASSERT(timeSpan->mEndIndex < MAX_GPU_TIMESTAMPS);

    vkCmdWriteTimestamp(
        GetCommandBuffer(),
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        mTimeQueryPools[mFrameIndex],
        (uint32_t)timeSpan->mEndIndex);
#endif
}

void VulkanContext::ReadTimeQueryResults()
{
#if PROFILING_ENABLED

    if (!mTimestampsSupported)
        return;

    if (mNumTimestamps[mFrameIndex] <= 0)
        return;

    uint64_t buffer[MAX_GPU_TIMESTAMPS];

    VkResult res = vkGetQueryPoolResults(mDevice, mTimeQueryPools[mFrameIndex], 0, mNumTimestamps[mFrameIndex], MAX_GPU_TIMESTAMPS * sizeof(uint64_t), buffer, sizeof(uint64_t), VK_QUERY_RESULT_64_BIT);
    if (res == VK_NOT_READY)
    {
        LogWarning("Timestamps not ready");
    }
    else if (res == VK_SUCCESS)
    {
        // We have the timestamp values in buffer. Now we just need to determine the timespans and send it to the Profiler
        for (int32_t i = 0; i < (int32_t)mGpuTimespans[mFrameIndex].size(); ++i)
        {
            uint64_t start = buffer[mGpuTimespans[mFrameIndex][i].mStartIndex];
            uint64_t end = buffer[mGpuTimespans[mFrameIndex][i].mEndIndex];
            float timeNs = (end - start) * mTimestampPeriod;
            float timeMs = timeNs / 1000000.0f;

            GetProfiler()->SetGpuStatTime(mGpuTimespans[mFrameIndex][i].mName.c_str(), timeMs);
            //LogDebug("[%s] %.3f", mGpuTimespans[mFrameIndex][i].mName.c_str(), timeMs);
        }
    }
    else
    {
        LogError("Failed to read timestamp queries");
        OCT_ASSERT(0);
    }

    vkCmdResetQueryPool(GetCommandBuffer(), mTimeQueryPools[mFrameIndex], 0, MAX_GPU_TIMESTAMPS);
    mNumTimestamps[mFrameIndex] = 0;

#endif
}

RayTracer* VulkanContext::GetRayTracer()
{
    return &mRayTracer;
}

VkSurfaceTransformFlagBitsKHR VulkanContext::GetPreTransformFlag() const
{
    return mPreTransformFlag;
}

void VulkanContext::EnableMaterialPipelineCache(bool enable)
{
    if (mEnableMaterialPipelineCache != enable)
    {
        mEnableMaterialPipelineCache = enable;
        mMaterialPipelineCache.Enable(enable);
    }
}

bool VulkanContext::IsMaterialPipelineCacheEnabled() const
{
    return mEnableMaterialPipelineCache;
}

MaterialPipelineCache* VulkanContext::GetMaterialPipelineCache()
{
    return &mMaterialPipelineCache;
}

VkExtent2D& VulkanContext::GetSwapchainExtent()
{
    return mSwapchainExtent;
}

Pipeline* VulkanContext::GetPipeline(PipelineId id)
{
    uint32_t index = uint32_t(id);
    OCT_ASSERT(index < (uint32_t)PipelineId::Count);
    return mPipelines[index];
}

Pipeline* VulkanContext::GetCurrentlyBoundPipeline()
{
    return mCurrentlyBoundPipeline;
}

VkPipelineCache VulkanContext::GetPipelineCache() const
{
    return mPipelineCache;
}

void VulkanContext::SavePipelineCacheToFile()
{
    if (mPipelineCache != VK_NULL_HANDLE)
    {
        size_t cacheSize = 0;
        VkResult res = vkGetPipelineCacheData(mDevice, mPipelineCache, &cacheSize, nullptr);
        OCT_ASSERT(res == VK_SUCCESS);

        if (cacheSize > 0)
        {
            char* cacheData = (char*)malloc(sizeof(char) * cacheSize);
            OCT_ASSERT(cacheData);

            res = vkGetPipelineCacheData(mDevice, mPipelineCache, &cacheSize, cacheData);
            OCT_ASSERT(res == VK_SUCCESS);

            Stream stream;
            stream.WriteBytes((uint8_t*)cacheData, (uint32_t)cacheSize);

            SYS_WriteSave(PIPELINE_CACHE_SAVE_NAME, stream);
        }
    }
}

VkRenderPass VulkanContext::GetForwardRenderPass()
{
    return mForwardRenderPass;
}

VkRenderPass VulkanContext::GetPostprocessRenderPass()
{
    return mPostprocessRenderPass;
}

VkRenderPass VulkanContext::GetUIRenderPass()
{
    return mUIRenderPass;
}

static ThreadFuncRet CreatePipelineThread(void* arg)
{
    PipelineCreateJobArgs* jobArgs = (PipelineCreateJobArgs*)arg;

    std::vector<Pipeline*>& pipelines = *(jobArgs->mPipelines);
    MutexObject* mutex = jobArgs->mMutex;

    while (true)
    {
        Pipeline* pipeline = nullptr;

        SYS_LockMutex(mutex);

        if (pipelines.size() > 0)
        {
            pipeline = pipelines.back();
            pipelines.pop_back();
        }

        SYS_UnlockMutex(mutex);

        if (pipeline != nullptr)
        {
            pipeline->Create();
        }
        else
        {
            break;
        }
    }


    THREAD_RETURN();
}

void VulkanContext::CreatePipelines()
{
    mPipelines[(size_t)PipelineId::Shadow] = new ShadowPipeline();
    mPipelines[(size_t)PipelineId::Opaque] = new OpaquePipeline();
    mPipelines[(size_t)PipelineId::Translucent] = new TranslucentPipeline();
    mPipelines[(size_t)PipelineId::Additive] = new AdditivePipeline();
    mPipelines[(size_t)PipelineId::DepthlessOpaque] = new OpaquePipeline();
    mPipelines[(size_t)PipelineId::DepthlessTranslucent] = new TranslucentPipeline();
    mPipelines[(size_t)PipelineId::DepthlessAdditive] = new AdditivePipeline();
    mPipelines[(size_t)PipelineId::CullFrontOpaque] = new OpaquePipeline();
    mPipelines[(size_t)PipelineId::CullFrontTranslucent] = new TranslucentPipeline();
    mPipelines[(size_t)PipelineId::CullFrontAdditive] = new AdditivePipeline();
    mPipelines[(size_t)PipelineId::CullNoneOpaque] = new OpaquePipeline();
    mPipelines[(size_t)PipelineId::CullNoneTranslucent] = new TranslucentPipeline();
    mPipelines[(size_t)PipelineId::CullNoneAdditive] = new AdditivePipeline();
    mPipelines[(size_t)PipelineId::ShadowMeshBack] = new ShadowMeshBackPipeline();
    mPipelines[(size_t)PipelineId::ShadowMeshFront] = new ShadowMeshFrontPipeline();
    mPipelines[(size_t)PipelineId::ShadowMeshClear] = new ShadowMeshClearPipeline();
    mPipelines[(size_t)PipelineId::Selected] = new SelectedGeometryPipeline();
    mPipelines[(size_t)PipelineId::Wireframe] = new WireframeGeometryPipeline();
    mPipelines[(size_t)PipelineId::Collision] = new CollisionGeometryPipeline();
    mPipelines[(size_t)PipelineId::Line] = new LineGeometryPipeline();
    mPipelines[(size_t)PipelineId::PostProcess] = new PostProcessPipeline();
    mPipelines[(size_t)PipelineId::NullPostProcess] = new NullPostProcessPipeline();
    mPipelines[(size_t)PipelineId::Quad] = new QuadPipeline();
    mPipelines[(size_t)PipelineId::Text] = new TextPipeline();
    mPipelines[(size_t)PipelineId::Poly] = new PolyPipeline();

    if (mSupportsRayTracing)
    {
        mPipelines[(size_t)PipelineId::PathTrace] = new PathTracePipeline();
        mPipelines[(size_t)PipelineId::LightBakeDirect] = new LightBakeDirectPipeline();
        mPipelines[(size_t)PipelineId::LightBakeIndirect] = new LightBakeIndirectPipeline();
        mPipelines[(size_t)PipelineId::LightBakeAverage] = new LightBakeAveragePipeline();
        mPipelines[(size_t)PipelineId::LightBakeDiffuse] = new LightBakeDiffusePipeline();
    }

#if EDITOR
    mPipelines[(size_t)PipelineId::HitCheck] = new HitCheckPipeline();
#endif

    // Modified Foward Pipelines
    for (uint32_t i = 0; i < 3; ++i)
    {
        uint32_t pipelineIdx = (uint32_t)PipelineId::DepthlessOpaque + i;
        mPipelines[pipelineIdx]->mPipelineId = PipelineId(pipelineIdx);
        mPipelines[pipelineIdx]->mDepthTestEnabled = false;
        mPipelines[pipelineIdx]->mDepthWriteEnabled = false;
    }
    for (uint32_t i = 0; i < 3; ++i)
    {
        uint32_t pipelineIdx = (uint32_t)PipelineId::CullFrontOpaque + i;
        mPipelines[pipelineIdx]->mPipelineId = PipelineId(pipelineIdx);
        mPipelines[pipelineIdx]->mCullMode = VK_CULL_MODE_FRONT_BIT;
    }
    for (uint32_t i = 0; i < 3; ++i)
    {
        uint32_t pipelineIdx = (uint32_t)PipelineId::CullNoneOpaque + i;
        mPipelines[pipelineIdx]->mPipelineId = PipelineId(pipelineIdx);
        mPipelines[pipelineIdx]->mCullMode = VK_CULL_MODE_NONE;
    }

    // Create Pipelines
    mPipelines[(size_t)PipelineId::Shadow]->SetRenderPass(mShadowRenderPass);
    mPipelines[(size_t)PipelineId::Opaque]->SetRenderPass(mForwardRenderPass);
    mPipelines[(size_t)PipelineId::Translucent]->SetRenderPass(mForwardRenderPass);
    mPipelines[(size_t)PipelineId::Additive]->SetRenderPass(mForwardRenderPass);
    mPipelines[(size_t)PipelineId::DepthlessOpaque]->SetRenderPass(mForwardRenderPass);
    mPipelines[(size_t)PipelineId::DepthlessTranslucent]->SetRenderPass(mForwardRenderPass);
    mPipelines[(size_t)PipelineId::DepthlessAdditive]->SetRenderPass(mForwardRenderPass);
    mPipelines[(size_t)PipelineId::CullFrontOpaque]->SetRenderPass(mForwardRenderPass);
    mPipelines[(size_t)PipelineId::CullFrontTranslucent]->SetRenderPass(mForwardRenderPass);
    mPipelines[(size_t)PipelineId::CullFrontAdditive]->SetRenderPass(mForwardRenderPass);
    mPipelines[(size_t)PipelineId::CullNoneOpaque]->SetRenderPass(mForwardRenderPass);
    mPipelines[(size_t)PipelineId::CullNoneTranslucent]->SetRenderPass(mForwardRenderPass);
    mPipelines[(size_t)PipelineId::CullNoneAdditive]->SetRenderPass(mForwardRenderPass);
    mPipelines[(size_t)PipelineId::ShadowMeshBack]->SetRenderPass(mForwardRenderPass);
    mPipelines[(size_t)PipelineId::ShadowMeshFront]->SetRenderPass(mForwardRenderPass);
    mPipelines[(size_t)PipelineId::ShadowMeshClear]->SetRenderPass(mForwardRenderPass);
    mPipelines[(size_t)PipelineId::Selected]->SetRenderPass(mPostprocessRenderPass);
    mPipelines[(size_t)PipelineId::Wireframe]->SetRenderPass(mForwardRenderPass);
    mPipelines[(size_t)PipelineId::Collision]->SetRenderPass(mForwardRenderPass);
    mPipelines[(size_t)PipelineId::Line]->SetRenderPass(mForwardRenderPass);
    mPipelines[(size_t)PipelineId::PostProcess]->SetRenderPass(mPostprocessRenderPass);
    mPipelines[(size_t)PipelineId::NullPostProcess]->SetRenderPass(mPostprocessRenderPass);
    mPipelines[(size_t)PipelineId::Quad]->SetRenderPass(mUIRenderPass);
    mPipelines[(size_t)PipelineId::Text]->SetRenderPass(mUIRenderPass);
    mPipelines[(size_t)PipelineId::Poly]->SetRenderPass(mUIRenderPass);

    // Compute pipelines don't need to set a RenderPass.

#if EDITOR
    mPipelines[(size_t)PipelineId::HitCheck]->SetRenderPass(mHitCheckRenderPass);
#endif

    // Create pipelines across multiple threads to use all CPU cores.
    std::vector<Pipeline*> pipelines;
    pipelines.reserve((size_t)PipelineId::Count);
    MutexObject* mutex = SYS_CreateMutex();

    PipelineCreateJobArgs jobArgs;
    jobArgs.mPipelines = &pipelines;
    jobArgs.mMutex = mutex;

    // Gather pipelines that we need to create.
    for (uint32_t i = 0; i < (uint32_t)PipelineId::Count; ++i)
    {
        if (mPipelines[i] != nullptr)
        {
            pipelines.push_back(mPipelines[i]);
        }
    }

    // Dispatch threads
    constexpr uint32_t kNumThreads = 8;
    ThreadObject* threads[kNumThreads] = {};

    for (uint32_t i = 0; i < kNumThreads; ++i)
    {
        threads[i] = SYS_CreateThread(CreatePipelineThread, &jobArgs);
    }

    // Join threads
    for (uint32_t i = 0; i < kNumThreads; ++i)
    {
        SYS_JoinThread(threads[i]);
        SYS_DestroyThread(threads[i]);
        threads[i] = nullptr;
    }

    // All pipeslines should be created now. Delete the shader modules we used.
    OCT_ASSERT(pipelines.size() == 0);
    SYS_DestroyMutex(mutex);
    mutex = nullptr;
}

void VulkanContext::DestroyPipelines()
{
    for (uint32_t i = 0; i < (uint32_t)PipelineId::Count; ++i)
    {
        if (mPipelines[i] != nullptr)
        {
            mPipelines[i]->Destroy();
            delete mPipelines[i];
            mPipelines[i] = nullptr;
        }
    }
}

void VulkanContext::SetViewport(int32_t x, int32_t y, int32_t width, int32_t height, bool handlePrerotation, bool useSceneRes)
{
    float bufferWidth = (float)(useSceneRes ? mSceneWidth : mSwapchainExtent.width);
    float bufferHeight = (float)(useSceneRes ? mSceneHeight : mSwapchainExtent.height);

    float fX = static_cast<float>(x);
    float fY = static_cast<float>(y);
    float fW = static_cast<float>(width);
    float fH = static_cast<float>(height);

    glm::vec4 viewportData;

    if (!handlePrerotation)
    {
        viewportData = { fX, fY, fW, fH };
    }
    else
    {
        switch (mPreTransformFlag)
        {
        case VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR:
            viewportData = { bufferWidth - fH - fY, fX, fH, fW };
            break;
        case VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR:
            viewportData = { bufferWidth - fW - fX, bufferHeight - fH - fY, fW, fH };
            break;
        case VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR:
            viewportData = { fY, bufferHeight - fW - fX, fH, fW };
            break;
        default:
            viewportData = { fX, fY, fW, fH };
            break;
        }
    }

    VkViewport viewport = {};
    viewport.x = viewportData.x;
    viewport.y = viewportData.y;
    viewport.width = viewportData.z;
    viewport.height = viewportData.w;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(GetCommandBuffer(), 0, 1, &viewport);
}

void VulkanContext::SetScissor(int32_t x, int32_t y, int32_t width, int32_t height, bool handlePrerotation, bool useSceneRes)
{
    float bufferWidth = (float)(useSceneRes ? mSceneWidth : mSwapchainExtent.width);
    float bufferHeight = (float)(useSceneRes ? mSceneHeight : mSwapchainExtent.height);

    float fX = static_cast<float>(x);
    float fY = static_cast<float>(y);
    float fW = static_cast<float>(width);
    float fH = static_cast<float>(height);

    glm::vec4 scissorData;

    if (!handlePrerotation)
    {
        scissorData = { fX, fY, fW, fH };
    }
    else
    {
        switch (mPreTransformFlag)
        {
        case VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR:
            scissorData = { bufferWidth - fH - fY, fX, fH, fW };
            break;
        case VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR:
            scissorData = { bufferWidth - fW - fX, bufferHeight - fH - fY, fW, fH };
            break;
        case VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR:
            scissorData = { fY, bufferHeight - fW - fX, fH, fW };
            break;
        default:
            scissorData = { fX, fY, fW, fH };
            break;
        }
    }

    // Should the offset and extent be rounded to nearest integer?
    // Or just rewrite the transform code to use integer math?
    VkRect2D scissorRect = {};
    scissorRect.offset = { int32_t(scissorData.x), int32_t(scissorData.y )};
    scissorRect.extent = { uint32_t(scissorData.z), uint32_t(scissorData.w )};
    vkCmdSetScissor(GetCommandBuffer(), 0, 1, &scissorRect);
}

#if EDITOR
Node3D* VulkanContext::ProcessHitCheck(World* world, int32_t pixelX, int32_t pixelY)
{
    if (world == nullptr)
    {
        return nullptr;
    }

    DeviceWaitIdle();

    std::vector<Node3D*> nodes;

    // Convert pixelX and pixelY to scene-resolution coordinates
    pixelX = (int32_t)(pixelX * mResolutionScale + 0.5f);
    pixelY = (int32_t)(pixelY * mResolutionScale + 0.5f);
    pixelX = glm::clamp<int32_t>(pixelX, 0, int32_t(mSceneWidth) - 1);
    pixelY = glm::clamp<int32_t>(pixelY, 0, int32_t(mSceneHeight) - 1);

    // Render to image
    {
        VkCommandBuffer cb = BeginCommandBuffer();
        // HACK - since I'm not passing the CB to Render calls()
        // replace the "current" CB with our temp CB
        VkCommandBuffer realCb = mCommandBuffers[mFrameIndex];
        mCommandBuffers[mFrameIndex] = cb;

        UpdateGlobalUniformData();
        UpdateGlobalDescriptorSet();

        SetViewport(0, 0, mSceneWidth, mSceneHeight, false, true);
        SetScissor(0, 0, mSceneWidth, mSceneHeight, false, true);

        BeginRenderPass(RenderPassId::HitCheck);
        std::vector<DebugDraw> debugDraws;

        BindPipeline(PipelineId::HitCheck, VertexType::Vertex);

        uint32_t i = 1; // Start hit check id at 1, 0 = no hit.
        auto renderHitChecks = [&](Node* node) -> bool
        {
            if (!node->IsVisible())
                return false;

            Node3D* node3d = node->As<Node3D>();

            if (node3d)
            {
                nodes.push_back(node3d);
                node3d->SetHitCheckId(i);
                node3d->Render(PipelineId::HitCheck);
                ++i;

                if (Renderer::Get()->IsProxyRenderingEnabled())
                {
                    node3d->GatherProxyDraws(debugDraws);
                }
            }

            return true;
        };

        Node* root = world->GetRootNode();
        if (root != nullptr)
        {
            root->Traverse(renderHitChecks);
        }

        // Issue proxy draws
        for (uint32_t i = 0; i < debugDraws.size(); ++i)
        {
            if (debugDraws[i].mNode != nullptr)
            {
                DrawStaticMesh(
                    debugDraws[i].mMesh,
                    nullptr,
                    debugDraws[i].mTransform,
                    { 1.0f, 1.0f, 1.0f, 1.0f },
                    debugDraws[i].mNode->GetHitCheckId());
            }
        }

        EndRenderPass();

        EndCommandBuffer(cb);
        mCommandBuffers[mFrameIndex] = realCb; // HACK, see beginning of this CB recording block.

        // Ensure that this CB executes so that we can make sure the image is updated.
        DeviceWaitIdle();
    }

    // Copy to buffer
    {
        VkCommandBuffer cb = BeginCommandBuffer();
        VkBufferImageCopy copy = {};
        copy.bufferOffset = 0;
        copy.bufferRowLength = mSceneWidth;
        copy.bufferImageHeight = mSceneHeight;
        copy.imageExtent = { mSceneWidth, mSceneHeight, 1 };
        copy.imageOffset = { 0, 0, 0 };
        copy.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};

        vkCmdCopyImageToBuffer(cb, mHitCheckImage->Get(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, mHitCheckBuffer->Get(), 1, &copy);
        EndCommandBuffer(cb);
    }

    // Ensure the copy is completed before mapping the buffer.
    DeviceWaitIdle();

    // Read buffer at pixel index.
    uint32_t hitId = 0;
    void* bufferData = mHitCheckBuffer->Map();
    uint32_t* hitData = reinterpret_cast<uint32_t*>(bufferData);
    hitId = hitData[pixelX + pixelY * mSceneWidth];
    mHitCheckBuffer->Unmap();

    Node3D* hitNode = (hitId != 0) ? nodes[hitId - 1] : nullptr;

    return hitNode;
}

VkRenderPass VulkanContext::GetHitCheckRenderPass()
{
    return mHitCheckRenderPass;
}

void VulkanContext::CreateHitCheck()
{
    // Create Image
    ImageDesc imageDesc;
    imageDesc.mWidth = mSceneWidth;
    imageDesc.mHeight = mSceneHeight;
    imageDesc.mFormat = VK_FORMAT_R32_UINT;
    imageDesc.mUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    mHitCheckImage = new Image(imageDesc, SamplerDesc(), "Hit Check");

    mHitCheckImage->Transition(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    // Create Buffer
    uint32_t bufferSize = 4 * mSceneWidth * mSceneHeight; // 4 bytes per pixel. 32 uint format.
    mHitCheckBuffer = new Buffer(BufferType::Transfer, bufferSize, "Hit Check");

    // Create Render Pass
    VkAttachmentDescription attachments[] =
    {
        // Color attachment
        {
            0,
            VK_FORMAT_R32_UINT,
            VK_SAMPLE_COUNT_1_BIT,
            VK_ATTACHMENT_LOAD_OP_CLEAR,
            VK_ATTACHMENT_STORE_OP_STORE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
        },

        // Depth attachment
        {
            0,
			mDepthImageFormat,
            VK_SAMPLE_COUNT_1_BIT,
            VK_ATTACHMENT_LOAD_OP_CLEAR,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        }
    };

    // Light output attachment reference
    VkAttachmentReference colorAttachmentReference =
    {
        0,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkAttachmentReference depthAttachmentReference =
    {
        1,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpass =
    {
        0, // flags
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        0, // input attachments
        nullptr,
        1, // color attachments
        &colorAttachmentReference,
        nullptr,
        &depthAttachmentReference, // depth attachment
        0,
        nullptr
    };

    VkRenderPassCreateInfo ciRenderPass =
    {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        nullptr,
        0,
        2,
        attachments,
        1,
        &subpass,
        0,
        nullptr
    };

    if (vkCreateRenderPass(mDevice, &ciRenderPass, nullptr, &mHitCheckRenderPass) != VK_SUCCESS)
    {
        LogError("Failed to create renderpass");
        OCT_ASSERT(0);
    }

    // Create Framebuffer
    VkImageView imageAttachments[] = { mHitCheckImage->GetView(), mDepthImage->GetView() };

    VkFramebufferCreateInfo ciFramebuffer = {};
    ciFramebuffer.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    ciFramebuffer.renderPass = mHitCheckRenderPass;
    ciFramebuffer.attachmentCount = 2;
    ciFramebuffer.pAttachments = imageAttachments;
    ciFramebuffer.width = mSceneWidth;
    ciFramebuffer.height = mSceneHeight;
    ciFramebuffer.layers = 1;

    if (vkCreateFramebuffer(mDevice, &ciFramebuffer, nullptr, &mHitCheckFramebuffer) != VK_SUCCESS)
    {
        LogError("Failed to create framebuffer.");
        OCT_ASSERT(0);
    }
}

void VulkanContext::DestroyHitCheck()
{
    if (mHitCheckImage != nullptr)
    {
        vkDestroyFramebuffer(mDevice, mHitCheckFramebuffer, nullptr);
        vkDestroyRenderPass(mDevice, mHitCheckRenderPass, nullptr);

        GetDestroyQueue()->Destroy(mHitCheckBuffer);
        mHitCheckBuffer = nullptr;

        GetDestroyQueue()->Destroy(mHitCheckImage);
        mHitCheckImage = nullptr;

        mHitCheckFramebuffer = VK_NULL_HANDLE;
        mHitCheckBuffer = VK_NULL_HANDLE;
        mHitCheckRenderPass = VK_NULL_HANDLE;
    }
}
#endif // EDITOR

#endif