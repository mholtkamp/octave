#if API_VULKAN

#include "Graphics/Vulkan/VulkanContext.h"
#include "Log.h"
#include "Engine.h"
#include "Utilities.h"
#include "World.h"
#include "Renderer.h"

#include "Graphics/GraphicsUtils.h"

#include "Components/CameraComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"

#include <assert.h>
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

void CreateVulkanContext()
{
    assert(gVulkanContext == nullptr);
    gVulkanContext = new VulkanContext();
}

void DestroyVulkanContext()
{
    assert(gVulkanContext != nullptr);
    delete gVulkanContext;
    gVulkanContext = nullptr;
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
    mSceneColorFramebuffer = VK_NULL_HANDLE;
    mImageAvailableSemaphore = 0;
    mRenderFinishedSemaphore = 0;
    mSceneColorImageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;

    mGlobalUniformData.mDirectionalLightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    mGlobalUniformData.mDirectionalLightDirection = glm::vec4(2.0f, -4.0f, -8.0f, 0.0f);
    mGlobalUniformData.mScreenDimensions = glm::vec2(800.0f, 600.0f);
    mGlobalUniformData.mVisualizationMode = 0;
}

VulkanContext::~VulkanContext()
{

}

void VulkanContext::Initialize()
{
    mEngineState = GetEngineState();

    // TODO: Consider commandline arg for validation
#ifdef _DEBUG
    mValidate = true;
#else
    mValidate = false;
#endif

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

    CreatePipelines();

    CreateGlobalDescriptorSet();
    CreatePostProcessDescriptorSet();
    CreateFramebuffers();
    CreateCommandBuffers();
    CreateSemaphores();
    CreateFences();

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

    DeviceWaitIdle();
    mInitialized = true;
}

void VulkanContext::Destroy()
{
    DeviceWaitIdle();

    mMeshDescriptorSetArena.Destroy();
    mMeshUniformBufferArena.Destroy();

    DestroySwapchain();

    DestroyPipelines();

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
    assert(uint32_t(mFrameNumber) == Renderer::Get()->GetFrameNumber());
    assert(uint32_t(mFrameIndex) == Renderer::Get()->GetFrameIndex());

    if (mCommandBuffers.size() == 0)
    {
        CreateCommandBuffers();
    }

    VkResult result = vkAcquireNextImageKHR(mDevice, mSwapchain, std::numeric_limits<uint64_t>::max(), mImageAvailableSemaphore, VK_NULL_HANDLE, &mSwapchainImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapchain();
        return;
    }
    else if (result != VK_SUCCESS &&
             result != VK_SUBOPTIMAL_KHR)
    {
        LogError("Failed to acquire swapchain image");
        assert(0);
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

    mMeshDescriptorSetArena.Reset();
    mMeshUniformBufferArena.Reset();
}

void VulkanContext::EndFrame()
{
    VkCommandBuffer cb = GetCommandBuffer();

    if (vkEndCommandBuffer(cb) != VK_SUCCESS)
    {
        LogError("Failed to record command buffer");
        assert(0);
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
        assert(0);
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
}

void VulkanContext::BeginRenderPass(RenderPassId id)
{
    if (mCurrentRenderPassId != RenderPassId::Count)
    {
        LogError("BeginRenderPass called while a render pass is already active.");
        assert(0);
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
#if EDITOR
    case RenderPassId::HitCheck:
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
    vkCmdBeginRenderPass(mCommandBuffers[mFrameIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanContext::EndRenderPass()
{
    // TEMPORARY!!
    // TODO: Remove this after finding out how to avoid double-updating GeometryUniform buffers between shadow and forward passes.
    if (mCurrentRenderPassId == RenderPassId::Shadows)
    {
        DeviceWaitIdle();
    }

    if (mCurrentRenderPassId != RenderPassId::Count)
    {
        vkCmdEndRenderPass(mCommandBuffers[mFrameIndex]);
        EndDebugLabel();
        mCurrentRenderPassId = RenderPassId::Count;
    }
}

void VulkanContext::BindPipeline(PipelineId id, VertexType vertexType)
{
    Pipeline* pipeline = GetPipeline(id);
    assert(pipeline != nullptr &&
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
    mGlobalDescriptorSet->Bind(cb, (uint32_t)DescriptorSetBinding::Global, pipelineLayout);

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

    ciSwapchain.preTransform = swapChainSupport.capabilities.currentTransform;
    ciSwapchain.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    ciSwapchain.presentMode = presentMode;
    ciSwapchain.clipped = VK_TRUE;
    ciSwapchain.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(mDevice, &ciSwapchain, nullptr, &mSwapchain) != VK_SUCCESS)
    {
        LogError("Failed to create swapchain");
        assert(0);
    }

    vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, nullptr);
    mSwapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, mSwapchainImages.data());

    mSwapchainImageFormat = surfaceFormat.format;
    mSwapchainExtent = extent;

    mGlobalUniformData.mScreenDimensions = glm::vec2(extent.width, extent.height);
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanContext::DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void* userData)
{
    char prefix[64];
    char* message = (char*) malloc(strlen(callbackData->pMessage) + 500);
    assert(message);

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

    LogDebug("%s\n", message);

#if PLATFORM_WINDOWS
    OutputDebugString(message);
#endif

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
        assert(0);
    }

    result = vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
    assert(result == VK_SUCCESS);

    if (extensionCount > 0)
    {
        VkExtensionProperties* extensions = reinterpret_cast<VkExtensionProperties*>(malloc(sizeof(VkExtensionProperties) * extensionCount));
        
        result = vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);
        assert(result == VK_SUCCESS);

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
#endif

            if (!strcmp(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, extensions[i].extensionName))
            {
                if (mValidate)
                {
                    mEnabledExtensions[mEnabledExtensionCount++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
                    debugUtilsExtFound = true;
                }
            }
            assert(mEnabledExtensionCount < MAX_ENABLED_EXTENSIONS);
        }

        free(extensions);
        extensions = nullptr;
    }

    if (!surfaceExtFound)
    {
        LogError("Failed to find surface extension");
        assert(0);
    }
    if (!platformSurfaceExtFound)
    {
        LogError("Failed to find platform surface extension");
        assert(0);
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = GetEngineState()->mProjectName.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = ENGINE_NAME;
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo ciInstance = {};
    ciInstance.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ciInstance.pApplicationInfo = &appInfo;
    ciInstance.enabledExtensionCount = mEnabledExtensionCount;
    ciInstance.ppEnabledExtensionNames = mEnabledExtensions;
    ciInstance.enabledLayerCount = mEnabledLayersCount;
    ciInstance.ppEnabledLayerNames = mEnabledLayers;
    ciInstance.pNext = nullptr;

    result = vkCreateInstance(&ciInstance, nullptr, &mInstance);

    if (result != VK_SUCCESS)
    {
        LogDebug("Failed to create instance.");
        assert(0);
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

    assert(mDebugMessenger == VK_NULL_HANDLE);

    PFN_vkCreateDebugUtilsMessengerEXT CreateDebugUtilsMessengerEXT =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            mInstance,
            "vkCreateDebugUtilsMessengerEXT");

    assert(CreateDebugUtilsMessengerEXT != nullptr);

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
        assert(0);
    }
}

void VulkanContext::CreateSurface()
{
#if PLATFORM_WINDOWS
    PFN_vkCreateWin32SurfaceKHR pfnCreateWin32Surface = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(mInstance, "vkCreateWin32SurfaceKHR");

    assert(pfnCreateWin32Surface != nullptr);

    VkWin32SurfaceCreateInfoKHR ciSurface = {};
    ciSurface.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    ciSurface.hwnd = mEngineState->mSystem.mWindow;
    ciSurface.hinstance = GetModuleHandle(nullptr);

    if (pfnCreateWin32Surface(mInstance, &ciSurface, nullptr, &mSurface) != VK_SUCCESS)
    {
        LogError("Failed to create window surface.");
        assert(0);
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
        assert(0);
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
        assert(0);
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
        assert(0);
    }
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
    deviceFeatures.fillModeNonSolid = true;
    deviceFeatures.wideLines = true;

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
        assert(0);
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
            assert(0);
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
    imageDesc.mWidth = mSwapchainExtent.width;
    imageDesc.mHeight = mSwapchainExtent.height;
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

DestroyQueue* VulkanContext::GetDestroyQueue()
{
    return &mDestroyQueue;
}

VkFormat VulkanContext::GetSwapchainFormat()
{
    return mSwapchainImageFormat;
}

void VulkanContext::CreateRenderPass()
{
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

        if (vkCreateRenderPass(mDevice, &ciRenderPass, nullptr, &mShadowRenderPass) != VK_SUCCESS)
        {
            LogError("Failed to create shadow renderpass");
            assert(0);
        }
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
                VK_FORMAT_D24_UNORM_S8_UINT,
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
            0,
            nullptr
        };

        if (vkCreateRenderPass(mDevice, &ciRenderPass, nullptr, &mForwardRenderPass) != VK_SUCCESS)
        {
            LogError("Failed to create forward render pass");
            assert(0);
        }
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
            },

            // Depth Buffer
            {
                0,
                VK_FORMAT_D24_UNORM_S8_UINT,
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_LOAD,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_LOAD,
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
            0,
            nullptr
        };

        if (vkCreateRenderPass(mDevice, &ciRenderPass, nullptr, &mPostprocessRenderPass) != VK_SUCCESS)
        {
            LogError("Failed to create postprocess render pass");
            assert(0);
        }
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
            },

            // Depth Buffer
            {
                0,
                VK_FORMAT_D24_UNORM_S8_UINT,
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_LOAD,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_LOAD,
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
            0,
            nullptr
        };

        if (vkCreateRenderPass(mDevice, &ciRenderPass, nullptr, &mUIRenderPass) != VK_SUCCESS)
        {
            LogError("Failed to create forward  render pass");
            assert(0);
        }
    }
}

void VulkanContext::CreateFramebuffers()
{
    mSwapchainFramebuffers.resize(mSwapchainImageViews.size());

    for (size_t i = 0; i < mSwapchainImageViews.size(); ++i)
    {
        VkImageView attachmentViews[] = { mSwapchainImageViews[i], mDepthImage->GetView() };

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
            assert(0);
        }
    }

    {
        assert(mShadowRenderPass != VK_NULL_HANDLE);

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
            assert(0);
        }
    }

    {
        VkImageView attachmentViews[] = { mSceneColorImage->GetView(), mDepthImage->GetView() };

        VkFramebufferCreateInfo ciFramebuffer = {};
        ciFramebuffer.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        ciFramebuffer.renderPass = mForwardRenderPass;
        ciFramebuffer.attachmentCount = OCT_ARRAY_SIZE(attachmentViews);
        ciFramebuffer.pAttachments = attachmentViews;
        ciFramebuffer.width = mSwapchainExtent.width;
        ciFramebuffer.height = mSwapchainExtent.height;
        ciFramebuffer.layers = 1;

        if (vkCreateFramebuffer(mDevice, &ciFramebuffer, nullptr, &mSceneColorFramebuffer) != VK_SUCCESS)
        {
            LogError("Failed to create framebuffer.");
            assert(0);
        }
    }
}

void VulkanContext::CreateDepthImage()
{
    ImageDesc imageDesc;
    imageDesc.mWidth = mSwapchainExtent.width;
    imageDesc.mHeight = mSwapchainExtent.height;
    imageDesc.mFormat = VK_FORMAT_D24_UNORM_S8_UINT;
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
        DirectionalLightComponent* dirLight = world->GetDirectionalLight();
        CameraComponent* camera = world->GetActiveCamera();

        mGlobalUniformData.mViewProjMatrix = camera->GetViewProjectionMatrix();
        mGlobalUniformData.mViewPosition = glm::vec4(camera->GetAbsolutePosition(), 1.0f);
        mGlobalUniformData.mViewDirection = glm::vec4(camera->GetForwardVector(), 0.0f);

        if (dirLight && dirLight->IsVisible())
        {
            mGlobalUniformData.mDirectionalLightDirection = glm::vec4(dirLight->GetDirection(), 0.0f);
            mGlobalUniformData.mDirectionalLightColor = dirLight->GetColor();
            mGlobalUniformData.mDirectionalLightVP = dirLight->GetViewProjectionMatrix();
            //mGlobalUniformData.mShadowIntensity = 0.0f; // (GetShadowMapImageView() != VK_NULL_HANDLE && world->GetDirectionalLight()->ShouldCastShadows()) ? 1.0f : 0.0f;
        }
        else
        {
            mGlobalUniformData.mDirectionalLightDirection = glm::vec4(1);
            mGlobalUniformData.mDirectionalLightColor = glm::vec4(0);
            mGlobalUniformData.mDirectionalLightVP = glm::mat4(1);
            //mGlobalUniformData.mShadowIntensity = 0.0;
        }

        mGlobalUniformData.mAmbientLightColor = world->GetAmbientLightColor();

        const std::vector<PointLightComponent*>& pointLights = world->GetPointLights();
        mGlobalUniformData.mNumPointLights = int32_t(pointLights.size());
        
        for (uint32_t i = 0; i < MAX_POINTLIGHTS; ++i)
        {
            PointLightComponent* pointLight = (i < pointLights.size()) ? pointLights[i] : nullptr;

            if (pointLight != nullptr)
            {
                mGlobalUniformData.mPointLightPositions[i] = glm::vec4(pointLight->GetAbsolutePosition(), pointLight->GetRadius());
                mGlobalUniformData.mPointLightColors[i] = pointLight->GetColor();
            }
            else
            {
                mGlobalUniformData.mPointLightPositions[i] = glm::vec4(0,0,0,0);
                mGlobalUniformData.mPointLightColors[i] = glm::vec4(0,0,0,1);
            }
        }

        mGlobalUniformData.mShadowColor = world->GetShadowColor();

        const FogSettings& fog = world->GetFogSettings();
        mGlobalUniformData.mFogColor = fog.mColor;
        mGlobalUniformData.mFogEnabled = fog.mEnabled;
        mGlobalUniformData.mFogDensityFunc = int32_t(fog.mDensityFunc);
        mGlobalUniformData.mFogNear = fog.mNear;
        mGlobalUniformData.mFogFar = fog.mFar;
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
        assert(0);
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
            assert(0);
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
        assert(0);
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
        assert(0);
    }
}

void VulkanContext::RecreateSwapchain()
{
    if (!mInitialized)
    {
        return;
    }

    DeviceWaitIdle();

    DestroySwapchain();

    CreateSwapchain();
    CreateImageViews();
    CreateDepthImage();
    CreateSceneColorImage();
    CreateShadowMapImage();
    CreateRenderPass();
    CreateFramebuffers();
    CreateGlobalDescriptorSet();
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
        assert(0);
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
    assert(0);
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanContext::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    VkExtent2D retExtent = capabilities.currentExtent;

    // Not entirely sure what this if statement is doing yet.
    if (retExtent.width == std::numeric_limits<uint32_t>::max())
    {
        retExtent = { mEngineState->mWindowWidth, mEngineState->mWindowHeight };

        retExtent.width = glm::clamp(retExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        retExtent.height = glm::clamp(retExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    }

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

VkExtent2D& VulkanContext::GetSwapchainExtent()
{
    return mSwapchainExtent;
}

Pipeline* VulkanContext::GetPipeline(PipelineId id)
{
    uint32_t index = uint32_t(id);
    assert(index < (uint32_t)PipelineId::Count);
    return mPipelines[index];
}

Pipeline* VulkanContext::GetCurrentlyBoundPipeline()
{
    return mCurrentlyBoundPipeline;
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

void VulkanContext::CreatePipelines()
{
    mPipelines[(size_t)PipelineId::Shadow] = new ShadowPipeline();
    mPipelines[(size_t)PipelineId::Opaque] = new OpaquePipeline();
    mPipelines[(size_t)PipelineId::Translucent] = new TranslucentPipeline();
    mPipelines[(size_t)PipelineId::Additive] = new AdditivePipeline();
    mPipelines[(size_t)PipelineId::DepthlessOpaque] = new DepthlessOpaquePipeline();
    mPipelines[(size_t)PipelineId::DepthlessTranslucent] = new DepthlessTranslucentPipeline();
    mPipelines[(size_t)PipelineId::DepthlessAdditive] = new DepthlessAdditivePipeline();
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

#if EDITOR
    mPipelines[(size_t)PipelineId::HitCheck] = new HitCheckPipeline();
#endif

    mPipelines[(size_t)PipelineId::Shadow]->Create(mShadowRenderPass);
    mPipelines[(size_t)PipelineId::Opaque]->Create(mForwardRenderPass);
    mPipelines[(size_t)PipelineId::Translucent]->Create(mForwardRenderPass);
    mPipelines[(size_t)PipelineId::Additive]->Create(mForwardRenderPass);
    mPipelines[(size_t)PipelineId::DepthlessOpaque]->Create(mForwardRenderPass);
    mPipelines[(size_t)PipelineId::DepthlessTranslucent]->Create(mForwardRenderPass);
    mPipelines[(size_t)PipelineId::DepthlessAdditive]->Create(mForwardRenderPass);
    mPipelines[(size_t)PipelineId::ShadowMeshBack]->Create(mForwardRenderPass);
    mPipelines[(size_t)PipelineId::ShadowMeshFront]->Create(mForwardRenderPass);
    mPipelines[(size_t)PipelineId::ShadowMeshClear]->Create(mForwardRenderPass);
    mPipelines[(size_t)PipelineId::Selected]->Create(mPostprocessRenderPass);
    mPipelines[(size_t)PipelineId::Wireframe]->Create(mPostprocessRenderPass);
    mPipelines[(size_t)PipelineId::Collision]->Create(mPostprocessRenderPass);
    mPipelines[(size_t)PipelineId::Line]->Create(mPostprocessRenderPass);
    mPipelines[(size_t)PipelineId::PostProcess]->Create(mPostprocessRenderPass);
    mPipelines[(size_t)PipelineId::NullPostProcess]->Create(mPostprocessRenderPass);
    mPipelines[(size_t)PipelineId::Quad]->Create(mUIRenderPass);
    mPipelines[(size_t)PipelineId::Text]->Create(mUIRenderPass);

#if EDITOR
    mPipelines[(size_t)PipelineId::HitCheck]->Create(mHitCheckRenderPass);
#endif
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

void VulkanContext::SetViewport(int32_t x, int32_t y, int32_t width, int32_t height)
{
    VkViewport viewport = {};
    viewport.x = static_cast<float>(x);
    viewport.y = static_cast<float>(y);
    viewport.width = static_cast<float>(width);
    viewport.height = static_cast<float>(height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(GetCommandBuffer(), 0, 1, &viewport);
}

void VulkanContext::SetScissor(int32_t x, int32_t y, int32_t width, int32_t height)
{
    VkRect2D scissorRect = {};
    scissorRect.offset = { x, y };
    scissorRect.extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
    vkCmdSetScissor(GetCommandBuffer(), 0, 1, &scissorRect);
}

#if EDITOR
Actor* VulkanContext::ProcessHitCheck(World* world, int32_t pixelX, int32_t pixelY)
{
    if (world == nullptr)
    {
        return nullptr;
    }

    DeviceWaitIdle();

    // Render to image
    {
        VkCommandBuffer cb = BeginCommandBuffer();
        // HACK - since I'm not passing the CB to Render calls()
        // replace the "current" CB with our temp CB
        VkCommandBuffer realCb = mCommandBuffers[mFrameIndex];
        mCommandBuffers[mFrameIndex] = cb;

        UpdateGlobalUniformData();
        UpdateGlobalDescriptorSet();

        SetViewport(0, 0, mSwapchainExtent.width, mSwapchainExtent.height);
        SetScissor(0, 0, mSwapchainExtent.width, mSwapchainExtent.height);

        BeginRenderPass(RenderPassId::HitCheck);
        std::vector<DebugDraw> debugDraws;

        const std::vector<Actor*>& actors = world->GetActors();
        for (uint32_t i = 0; i < actors.size(); ++i)
        {
            actors[i]->SetHitCheckId(i + 1); // Reserve 0 for no actor
            actors[i]->Render(PipelineId::HitCheck);

            if (Renderer::Get()->IsProxyRenderingEnabled())
            {
                for (Component* comp : actors[i]->GetComponents())
                {
                    if (comp->IsTransformComponent())
                    {
                        static_cast<TransformComponent*>(comp)->GatherProxyDraws(debugDraws);
                    }
                }
            }
        }

        // Issue proxy draws
        for (uint32_t i = 0; i < debugDraws.size(); ++i)
        {
            if (debugDraws[i].mActor != nullptr)
            {
                DrawStaticMesh(
                    debugDraws[i].mMesh,
                    nullptr,
                    debugDraws[i].mTransform,
                    { 1.0f, 1.0f, 1.0f, 1.0f },
                    debugDraws[i].mActor->GetHitCheckId());
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
        copy.bufferRowLength = mSwapchainExtent.width;
        copy.bufferImageHeight = mSwapchainExtent.height;
        copy.imageExtent = { mSwapchainExtent.width, mSwapchainExtent.height, 1 };
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
    hitId = hitData[pixelX + pixelY * mSwapchainExtent.width];
    mHitCheckBuffer->Unmap();

    Actor* retActor = (hitId != 0) ? world->GetActors()[hitId - 1] : nullptr;
    return retActor;
}

VkRenderPass VulkanContext::GetHitCheckRenderPass()
{
    return mHitCheckRenderPass;
}

void VulkanContext::CreateHitCheck()
{
    // Create Image
    ImageDesc imageDesc;
    imageDesc.mWidth = mSwapchainExtent.width;
    imageDesc.mHeight = mSwapchainExtent.height;
    imageDesc.mFormat = VK_FORMAT_R32_UINT;
    imageDesc.mUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    mHitCheckImage = new Image(imageDesc, SamplerDesc(), "Hit Check");

    mHitCheckImage->Transition(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    // Create Buffer
    uint32_t bufferSize = 4 * mSwapchainExtent.width * mSwapchainExtent.height; // 4 bytes per pixel. 32 uint format.
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
            VK_FORMAT_D24_UNORM_S8_UINT,
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
        assert(0);
    }

    // Create Framebuffer
    VkImageView imageAttachments[] = { mHitCheckImage->GetView(), mDepthImage->GetView() };

    VkFramebufferCreateInfo ciFramebuffer = {};
    ciFramebuffer.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    ciFramebuffer.renderPass = mHitCheckRenderPass;
    ciFramebuffer.attachmentCount = 2;
    ciFramebuffer.pAttachments = imageAttachments;
    ciFramebuffer.width = mSwapchainExtent.width;
    ciFramebuffer.height = mSwapchainExtent.height;
    ciFramebuffer.layers = 1;

    if (vkCreateFramebuffer(mDevice, &ciFramebuffer, nullptr, &mHitCheckFramebuffer) != VK_SUCCESS)
    {
        LogError("Failed to create framebuffer.");
        assert(0);
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