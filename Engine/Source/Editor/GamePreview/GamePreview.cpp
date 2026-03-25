#if EDITOR

#include "GamePreview.h"
#include "SecondScreenPreview/SecondScreenPreview.h"
#include "World.h"
#include "Renderer.h"
#include "Engine.h"
#include "Log.h"
#include "Utilities.h"
#include "System/System.h"
#include "Nodes/3D/Camera3d.h"
#include "Nodes/Widgets/Widget.h"
#include "EditorState.h"
#include "EditorIcons.h"
#include "EditorUIHookManager.h"
#include "Packaging/PackagingSettings.h"
#include "Input/Input.h"

#include "imgui.h"

#if API_VULKAN
#include "Graphics/Vulkan/Image.h"
#include "Graphics/Vulkan/Buffer.h"
#include "Graphics/Vulkan/DestroyQueue.h"
#include "Graphics/Vulkan/VulkanUtils.h"
#include "backends/imgui_impl_vulkan.h"
#endif

#include <stb_image_write.h>
#include <ctime>

static GamePreview sGamePreview;

GamePreview* GetGamePreview()
{
    return &sGamePreview;
}

const std::vector<ResolutionPreset> GamePreview::sBuiltInPresets = {
    { "GameCube  640x480",   640,  480 },
    { "Wii 4:3  640x480",       640,  480 },
    { "3DS Top  400x240",   400,  240 },
    { "3DS Bottom  320x240",320,  240 },
    { "720p  1280x720",    1280,  720 },
    { "1080p  1920x1080",  1920, 1080 },
    { "4K  3840x2160",     3840, 2160 },
};

void GamePreview::Enable()
{
    GetSecondScreenPreview()->Disable();

    if (mEnabled)
        return;

    ResolutionPreset preset = GetCurrentPreset();
    CreateRenderTargets(preset.mWidth, preset.mHeight);

    mEnabled = true;
    LogDebug("Game Preview enabled (%ux%u)", preset.mWidth, preset.mHeight);
}

void GamePreview::Disable()
{
    if (!mEnabled)
        return;

    mEnabled = false;

#if API_VULKAN
    DeviceWaitIdle();

    if (mImGuiTexId != 0)
    {
        ImGui_ImplVulkan_RemoveTexture((VkDescriptorSet)mImGuiTexId);
        mImGuiTexId = 0;
    }
#endif

    DestroyRenderTargets();
    mCachedCameras.clear();

    LogDebug("Game Preview disabled");
}

void GamePreview::CreateRenderTargets(uint32_t w, uint32_t h)
{
#if API_VULKAN
    // Clean up old targets first
    if (mImGuiTexId != 0)
    {
        DeviceWaitIdle();
        ImGui_ImplVulkan_RemoveTexture((VkDescriptorSet)mImGuiTexId);
        mImGuiTexId = 0;
    }
    DestroyRenderTargets();

    // Color render target
    {
        ImageDesc desc;
        desc.mWidth = w;
        desc.mHeight = h;
        desc.mFormat = VK_FORMAT_B8G8R8A8_UNORM;
        desc.mUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

        SamplerDesc sampDesc;
        sampDesc.mAddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

        mColorTarget = new Image(desc, sampDesc, "GamePreview Color");
        mColorTarget->Transition(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    // Depth render target
    {
        ImageDesc desc;
        desc.mWidth = w;
        desc.mHeight = h;
        desc.mFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
        desc.mUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        mDepthTarget = new Image(desc, SamplerDesc(), "GamePreview Depth");
        mDepthTarget->Transition(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    }

    // Create ImGui texture descriptor
    mImGuiTexId = (ImTextureID)ImGui_ImplVulkan_AddTexture(
        mColorTarget->GetSampler(),
        mColorTarget->GetView(),
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    mCurrentWidth = w;
    mCurrentHeight = h;

    DeviceWaitIdle();
#endif
}

void GamePreview::DestroyRenderTargets()
{
#if API_VULKAN
    if (mColorTarget != nullptr)
    {
        GetDestroyQueue()->Destroy(mColorTarget);
        mColorTarget = nullptr;
    }

    if (mDepthTarget != nullptr)
    {
        GetDestroyQueue()->Destroy(mDepthTarget);
        mDepthTarget = nullptr;
    }
#endif

    mCurrentWidth = 0;
    mCurrentHeight = 0;
}

void GamePreview::RefreshCameraList()
{
    mCachedCameras.clear();

    World* world = GetWorld(0);
    if (world == nullptr)
        return;

    world->FindNodes(mCachedCameras);
}

ResolutionPreset GamePreview::GetCurrentPreset()
{
    std::vector<ResolutionPreset> all = GetAllPresets();
    if (mSelectedPresetIndex >= 0 && mSelectedPresetIndex < (int32_t)all.size())
        return all[mSelectedPresetIndex];

    if (!all.empty())
        return all[0];

    return { "Default  640x480", 640, 480 };
}

// Helper to get resolution for a platform
static void GetPlatformResolution(Platform platform, uint32_t& outWidth, uint32_t& outHeight, const char*& outName)
{
    switch (platform)
    {
        case Platform::GameCube:
            outWidth = 640;
            outHeight = 480;
            outName = "GameCube";
            break;
        case Platform::Wii:
            outWidth = 640;
            outHeight = 480;
            outName = "Wii";
            break;
        case Platform::N3DS:
            outWidth = 400;
            outHeight = 240;
            outName = "3DS Top";
            break;
        case Platform::Windows:
            outWidth = 1280;
            outHeight = 720;
            outName = "Windows";
            break;
        case Platform::Linux:
            outWidth = 1280;
            outHeight = 720;
            outName = "Linux";
            break;
        default:
            outWidth = 640;
            outHeight = 480;
            outName = "Default";
            break;
    }
}

std::vector<ResolutionPreset> GamePreview::GetAllPresets()
{
    std::vector<ResolutionPreset> all;

    // Add "Target Platform" preset at top if a current target is set
    PackagingSettings* pkgSettings = PackagingSettings::Get();
    if (pkgSettings != nullptr)
    {
        BuildProfile* target = pkgSettings->GetCurrentTargetProfile();
        if (target != nullptr)
        {
            uint32_t w, h;
            const char* platformName;
            GetPlatformResolution(target->mTargetPlatform, w, h, platformName);

            char presetName[128];
            snprintf(presetName, sizeof(presetName), "* %s [%s]  %ux%u",
                     target->mName.c_str(), platformName, w, h);
            all.push_back({ presetName, w, h });
        }
    }

    // Add built-in presets
    for (const auto& preset : sBuiltInPresets)
    {
        all.push_back(preset);
    }

    // Append addon presets from hook manager
    EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
    if (hookMgr != nullptr)
    {
        const auto& hookPresets = hookMgr->GetGamePreviewResolutions();
        for (const auto& hp : hookPresets)
        {
            all.push_back({ hp.mName, hp.mWidth, hp.mHeight });
        }
    }

    // Append locally-added addon presets
    for (const auto& p : mAddonPresets)
    {
        all.push_back(p);
    }

    return all;
}

void GamePreview::AddResolutionPreset(const std::string& name, uint32_t w, uint32_t h)
{
    // Avoid duplicates
    for (const auto& p : mAddonPresets)
    {
        if (p.mName == name)
            return;
    }
    mAddonPresets.push_back({ name, w, h });
}

void GamePreview::RemoveResolutionPreset(const std::string& name)
{
    for (auto it = mAddonPresets.begin(); it != mAddonPresets.end(); ++it)
    {
        if (it->mName == name)
        {
            mAddonPresets.erase(it);
            return;
        }
    }
}

void GamePreview::Render()
{
    if (!mEnabled)
        return;

    // Lazy-create render targets (needed when mEnabled defaults to true)
    if (mColorTarget == nullptr || mDepthTarget == nullptr)
    {
        ResolutionPreset preset = GetCurrentPreset();
        CreateRenderTargets(preset.mWidth, preset.mHeight);
    }

    World* world = GetWorld(0);
    if (world == nullptr || mColorTarget == nullptr || mDepthTarget == nullptr)
        return;

    // Refresh camera list to avoid dangling pointers
    // (e.g., after PIE ends and the game world's cameras are destroyed)
    RefreshCameraList();

    // Determine the selected camera to pass as override
    Camera3D* selectedCamera = nullptr;
    if (mSelectedCameraIndex >= 0 && mSelectedCameraIndex < (int32_t)mCachedCameras.size())
    {
        selectedCamera = mCachedCameras[mSelectedCameraIndex];
    }

    // Temporarily override the editor viewport dimensions so that
    // Camera3D::ComputeMatrices() picks up the render target's aspect ratio
    // instead of the editor viewport's aspect ratio.
    EditorState* edState = GetEditorState();
    uint32_t prevVpW = edState->mViewportWidth;
    uint32_t prevVpH = edState->mViewportHeight;
    edState->mViewportWidth = mCurrentWidth;
    edState->mViewportHeight = mCurrentHeight;

    // Toggle proxy (gizmo) rendering
    bool prevProxy = Renderer::Get()->IsProxyRenderingEnabled();
    Renderer::Get()->EnableProxyRendering(mShowGizmos);

    Renderer::Get()->RenderSecondScreen(world, mColorTarget, mDepthTarget,
                                         mCurrentWidth, mCurrentHeight, selectedCamera);

    // Restore state
    edState->mViewportWidth = prevVpW;
    edState->mViewportHeight = prevVpH;
    Renderer::Get()->EnableProxyRendering(prevProxy);

    // Handle screenshot after render
    if (mScreenshotRequested)
    {
        mScreenshotRequested = false;
        CaptureScreenshot();
    }
}

void GamePreview::CaptureScreenshot()
{
#if API_VULKAN
    if (mColorTarget == nullptr || mCurrentWidth == 0 || mCurrentHeight == 0)
        return;

    uint32_t w = mCurrentWidth;
    uint32_t h = mCurrentHeight;
    size_t bufSize = (size_t)w * h * 4;

    Buffer* stagingBuffer = new Buffer(
        BufferType::Transfer, bufSize, "GamePreview Screenshot", nullptr, true);

    // Transition color target to transfer src
    mColorTarget->Transition(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    // Copy image to buffer
    VkCommandBuffer cb = BeginCommandBuffer();
    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { w, h, 1 };

    vkCmdCopyImageToBuffer(cb, mColorTarget->Get(),
                            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                            stagingBuffer->Get(), 1, &region);
    EndCommandBuffer(cb);

    // Transition color target back
    mColorTarget->Transition(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    DeviceWaitIdle();

    // Read pixels
    void* mapped = stagingBuffer->Map();
    if (mapped != nullptr)
    {
        // BGRA -> RGBA conversion
        uint8_t* pixels = (uint8_t*)mapped;
        std::vector<uint8_t> rgbaPixels(bufSize);
        for (size_t i = 0; i < bufSize; i += 4)
        {
            rgbaPixels[i + 0] = pixels[i + 2]; // R
            rgbaPixels[i + 1] = pixels[i + 1]; // G
            rgbaPixels[i + 2] = pixels[i + 0]; // B
            rgbaPixels[i + 3] = pixels[i + 3]; // A
        }

        // Build path: ProjectDir/Screenshots/GamePreview_YYYYMMDD_HHMMSS.png
        std::string projDir = GetEngineState()->mProjectDirectory;
        std::string screenshotDir = projDir + "Screenshots";

        if (!DoesDirExist(screenshotDir.c_str()))
        {
            SYS_CreateDirectory(screenshotDir.c_str());
        }

        time_t now = time(nullptr);
        struct tm timeInfo;
#if PLATFORM_WINDOWS
        localtime_s(&timeInfo, &now);
#else
        localtime_r(&now, &timeInfo);
#endif

        char filename[256];
        snprintf(filename, sizeof(filename),
                 "%s/GamePreview_%04d%02d%02d_%02d%02d%02d.png",
                 screenshotDir.c_str(),
                 timeInfo.tm_year + 1900, timeInfo.tm_mon + 1, timeInfo.tm_mday,
                 timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);

        int result = stbi_write_png(filename, (int)w, (int)h, 4, rgbaPixels.data(), (int)(w * 4));
        if (result)
        {
            LogDebug("Game Preview screenshot saved: %s", filename);
        }
        else
        {
            LogError("Failed to save Game Preview screenshot");
        }

        stagingBuffer->Unmap();
    }

    GetDestroyQueue()->Destroy(stagingBuffer);
#endif
}

void GamePreview::DrawPanel()
{
    // Enable/Disable toggle
    bool wasEnabled = mEnabled;
    bool enableToggle = mEnabled;
    ImGui::Checkbox(ICON_MDI_EYE, &enableToggle);

    if (enableToggle && !wasEnabled)
        Enable();
    else if (!enableToggle && wasEnabled)
        Disable();

    if (!mEnabled)
    {
        ImGui::TextDisabled("Preview is disabled. Enable to see the game through a scene camera.");
        return;
    }

    // Resolution dropdown
    {
        std::vector<ResolutionPreset> allPresets = GetAllPresets();

        // Build combo preview string
        std::string previewLabel = "No presets";
        if (mSelectedPresetIndex >= 0 && mSelectedPresetIndex < (int32_t)allPresets.size())
        {
            previewLabel = allPresets[mSelectedPresetIndex].mName;
        }

        ImGui::SetNextItemWidth(200.0f);
        if (ImGui::BeginCombo("Resolution", previewLabel.c_str()))
        {
            for (int32_t i = 0; i < (int32_t)allPresets.size(); ++i)
            {
                bool isSelected = (mSelectedPresetIndex == i);
                if (ImGui::Selectable(allPresets[i].mName.c_str(), isSelected))
                {
                    if (mSelectedPresetIndex != i)
                    {
                        mSelectedPresetIndex = i;
                        ResolutionPreset preset = allPresets[i];
                        if (preset.mWidth != mCurrentWidth || preset.mHeight != mCurrentHeight)
                        {
                            CreateRenderTargets(preset.mWidth, preset.mHeight);
                        }
                    }
                }
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }

    // Camera dropdown
    {
        RefreshCameraList();

        std::string camPreview = "(none)";
        if (mSelectedCameraIndex >= 0 && mSelectedCameraIndex < (int32_t)mCachedCameras.size())
        {
            camPreview = mCachedCameras[mSelectedCameraIndex]->GetName();
        }

        ImGui::SameLine();
        ImGui::SetNextItemWidth(180.0f);
        if (ImGui::BeginCombo("Camera", camPreview.c_str()))
        {
            for (int32_t i = 0; i < (int32_t)mCachedCameras.size(); ++i)
            {
                const std::string& camName = mCachedCameras[i]->GetName();
                bool isSelected = (mSelectedCameraIndex == i);
                if (ImGui::Selectable(camName.c_str(), isSelected))
                {
                    mSelectedCameraIndex = i;
                }
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }

    // Toolbar: Gizmos | Screenshot | Play/Stop
    {
        ImGui::Checkbox("Gizmos", &mShowGizmos);

        ImGui::SameLine();
        if (ImGui::Button("Screenshot"))
        {
            mScreenshotRequested = true;
        }

        ImGui::SameLine();
        bool inPie = GetEditorState()->mPlayInEditor;
        if (ImGui::Button(inPie ? "Stop" : "Play"))
        {
            if (inPie)
            {
                GetEditorState()->EndPlayInEditor();
            }
            else
            {
                GetEditorState()->mPlayInGameWindow = true;
                GetEditorState()->BeginPlayInEditor();
            }
        }

    }

    // Rendered image, aspect-ratio preserved, centered
    if (mImGuiTexId != 0 && mCurrentWidth > 0 && mCurrentHeight > 0)
    {
        ImVec2 avail = ImGui::GetContentRegionAvail();
        if (avail.x < 1.0f) avail.x = 1.0f;
        if (avail.y < 1.0f) avail.y = 1.0f;

        float imgAspect = (float)mCurrentWidth / (float)mCurrentHeight;
        float panelAspect = avail.x / avail.y;

        float drawW, drawH;
        if (imgAspect > panelAspect)
        {
            // Image is wider than panel -> fit to width, letterbox top/bottom
            drawW = avail.x;
            drawH = drawW / imgAspect;
        }
        else
        {
            // Image is taller than panel -> fit to height, pillarbox left/right
            drawH = avail.y;
            drawW = drawH * imgAspect;
        }

        // Center horizontally
        float offsetX = (avail.x - drawW) * 0.5f;
        if (offsetX > 0.0f)
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);

        // Center vertically
        float offsetY = (avail.y - drawH) * 0.5f;
        if (offsetY > 0.0f)
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offsetY);

        ImGui::Image(mImGuiTexId, ImVec2(drawW, drawH));
        mImageMin = ImGui::GetItemRectMin();
        mImageMax = ImGui::GetItemRectMax();

        // Click-to-recapture: if PIE is active in game window but cursor is not captured, recapture on click
        EditorState* es = GetEditorState();
        if (ImGui::IsItemClicked() && es->mPlayInEditor && es->mPlayInGameWindow && !es->mGamePreviewCaptured)
        {
            es->mGamePreviewCaptured = true;
        }

        // Apply cursor confinement to the image rect
        if (es->mGamePreviewCaptured)
        {
            float scale = GetEngineConfig()->mEditorInterfaceScale;
            if (scale <= 0.0f) scale = 1.0f;

            int32_t rx = (int32_t)(mImageMin.x * scale);
            int32_t ry = (int32_t)(mImageMin.y * scale);
            int32_t rw = (int32_t)((mImageMax.x - mImageMin.x) * scale);
            int32_t rh = (int32_t)((mImageMax.y - mImageMin.y) * scale);
            INP_TrapCursorToRect(rx, ry, rw, rh);
        }
    }
}

void GamePreview::BeginInputRemap()
{
    mInputRemapActive = false;

    if (!IsPlayingInEditor() || !GetEditorState()->mPlayInGameWindow)
        return;
    if (mCurrentWidth == 0 || mCurrentHeight == 0)
        return;

    float imgW = mImageMax.x - mImageMin.x;
    float imgH = mImageMax.y - mImageMin.y;
    if (imgW <= 0.0f || imgH <= 0.0f)
        return;

    EditorState* edState = GetEditorState();

    // Save and override viewport to game resolution
    mSavedVpX = edState->mViewportX;
    mSavedVpY = edState->mViewportY;
    mSavedVpW = edState->mViewportWidth;
    mSavedVpH = edState->mViewportHeight;
    edState->mViewportX = 0;
    edState->mViewportY = 0;
    edState->mViewportWidth = mCurrentWidth;
    edState->mViewportHeight = mCurrentHeight;

    // Save and remap mouse from window-space to game-resolution-space.
    // ImGui rects are in logical (scaled) coords, but INP mouse is in raw pixels,
    // so convert the image rect to raw pixel space first.
    float scale = GetEngineConfig()->mEditorInterfaceScale;
    if (scale <= 0.0f) scale = 1.0f;

    float rawMinX = mImageMin.x * scale;
    float rawMinY = mImageMin.y * scale;
    float rawImgW = imgW * scale;
    float rawImgH = imgH * scale;

    INP_GetMousePosition(mSavedMouseX, mSavedMouseY);
    float relX = ((float)mSavedMouseX - rawMinX) / rawImgW;
    float relY = ((float)mSavedMouseY - rawMinY) / rawImgH;
    int32_t finalX = (int32_t)(relX * (float)mCurrentWidth);
    int32_t finalY = (int32_t)(relY * (float)mCurrentHeight);
    INP_SetMousePosition(finalX, finalY);

    // Force all widgets to recompute their rects with the game viewport.
    // UpdateRect() is normally only called from PreRender() (during rendering),
    // but Button::Tick() needs correct rects during World::Update() for hit-testing.
    World* world = GetWorld(0);
    if (world != nullptr && world->GetRootNode() != nullptr)
    {
        world->GetRootNode()->Traverse([](Node* node) -> bool {
            if (node->IsWidget())
            {
                static_cast<Widget*>(node)->UpdateRect();
            }
            return true;
        });
    }

    mInputRemapActive = true;
}

void GamePreview::EndInputRemap()
{
    if (!mInputRemapActive)
        return;

    // Mark widgets dirty so they recompute rects with the editor viewport
    // during the next PreRender() call.
    World* world = GetWorld(0);
    if (world != nullptr && world->GetRootNode() != nullptr)
    {
        world->GetRootNode()->Traverse([](Node* node) -> bool {
            if (node->IsWidget())
            {
                static_cast<Widget*>(node)->MarkDirty();
            }
            return true;
        });
    }

    EditorState* edState = GetEditorState();
    edState->mViewportX = mSavedVpX;
    edState->mViewportY = mSavedVpY;
    edState->mViewportWidth = mSavedVpW;
    edState->mViewportHeight = mSavedVpH;

    INP_SetMousePosition(mSavedMouseX, mSavedMouseY);
    mInputRemapActive = false;

    // Re-apply cursor confinement after the game update, in case the game called
    // INP_TrapCursor(true) which would confine to the full window instead.
    if (GetEditorState()->mGamePreviewCaptured)
    {
        float scale = GetEngineConfig()->mEditorInterfaceScale;
        if (scale <= 0.0f) scale = 1.0f;

        int32_t rx = (int32_t)(mImageMin.x * scale);
        int32_t ry = (int32_t)(mImageMin.y * scale);
        int32_t rw = (int32_t)((mImageMax.x - mImageMin.x) * scale);
        int32_t rh = (int32_t)((mImageMax.y - mImageMin.y) * scale);
        INP_TrapCursorToRect(rx, ry, rw, rh);
    }
}

#endif
