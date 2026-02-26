#if EDITOR

#include "SecondScreenPreview.h"
#include "GamePreview/GamePreview.h"
#include "World.h"
#include "Renderer.h"
#include "Engine.h"
#include "Log.h"
#include "Nodes/Node.h"
#include "Nodes/3D/Camera3d.h"
#include "Nodes/Widgets/Widget.h"
#include "Input/Input.h"
#include "Assets/Scene.h"
#include "EditorState.h"

#include "imgui.h"

#if API_VULKAN
#include "Graphics/Vulkan/Image.h"
#include "Graphics/Vulkan/DestroyQueue.h"
#include "Graphics/Vulkan/VulkanUtils.h"
#include "backends/imgui_impl_vulkan.h"
#endif
#include <EditorIcons.h>

static SecondScreenPreview sSecondScreenPreview;

SecondScreenPreview* GetSecondScreenPreview()
{
    return &sSecondScreenPreview;
}

void SecondScreenPreview::Enable()
{
    GetGamePreview()->Disable();

    if (mEnabled)
        return;

    // Top screen
    mTop.mWorld = new World();
    mTop.mWorld->SpawnDefaultRoot();
    mTop.mWorld->SpawnDefaultCamera();
    CreateScreenTargets(mTop, kTopWidth, kTopHeight, "3DS Top");

    // Bottom screen
    mBottom.mWorld = new World();
    mBottom.mWorld->SpawnDefaultRoot();
    mBottom.mWorld->SpawnDefaultCamera();
    CreateScreenTargets(mBottom, kBottomWidth, kBottomHeight, "3DS Bottom");

    mEnabled = true;

    LogDebug("3DS Preview enabled");
}

void SecondScreenPreview::Disable()
{
    if (!mEnabled)
        return;

    mEnabled = false;

#if API_VULKAN
    DeviceWaitIdle();

    if (mTop.mImGuiTexId != 0)
    {
        ImGui_ImplVulkan_RemoveTexture((VkDescriptorSet)mTop.mImGuiTexId);
        mTop.mImGuiTexId = 0;
    }
    if (mBottom.mImGuiTexId != 0)
    {
        ImGui_ImplVulkan_RemoveTexture((VkDescriptorSet)mBottom.mImGuiTexId);
        mBottom.mImGuiTexId = 0;
    }
#endif

    DestroyScreenTargets(mTop);
    DestroyScreenTargets(mBottom);

    if (mTop.mWorld != nullptr)
    {
        mTop.mWorld->Destroy();
        delete mTop.mWorld;
        mTop.mWorld = nullptr;
    }
    if (mBottom.mWorld != nullptr)
    {
        mBottom.mWorld->Destroy();
        delete mBottom.mWorld;
        mBottom.mWorld = nullptr;
    }

    mTop.mCurrentScene = nullptr;
    mBottom.mCurrentScene = nullptr;

    LogDebug("3DS Preview disabled");
}

void SecondScreenPreview::CreateScreenTargets(ScreenState& screen, uint32_t width, uint32_t height, const char* debugName)
{
#if API_VULKAN
    // Color render target
    {
        ImageDesc desc;
        desc.mWidth = width;
        desc.mHeight = height;
        desc.mFormat = VK_FORMAT_B8G8R8A8_UNORM;
        desc.mUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

        SamplerDesc sampDesc;
        sampDesc.mAddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

        screen.mColorTarget = new Image(desc, sampDesc, debugName);
        screen.mColorTarget->Transition(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    // Depth render target
    {
        ImageDesc desc;
        desc.mWidth = width;
        desc.mHeight = height;
        desc.mFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
        desc.mUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        screen.mDepthTarget = new Image(desc, SamplerDesc(), debugName);
        screen.mDepthTarget->Transition(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    }

    // Create ImGui texture descriptor
    screen.mImGuiTexId = (ImTextureID)ImGui_ImplVulkan_AddTexture(
        screen.mColorTarget->GetSampler(),
        screen.mColorTarget->GetView(),
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    DeviceWaitIdle();
#endif
}

void SecondScreenPreview::DestroyScreenTargets(ScreenState& screen)
{
#if API_VULKAN
    if (screen.mColorTarget != nullptr)
    {
        GetDestroyQueue()->Destroy(screen.mColorTarget);
        screen.mColorTarget = nullptr;
    }

    if (screen.mDepthTarget != nullptr)
    {
        GetDestroyQueue()->Destroy(screen.mDepthTarget);
        screen.mDepthTarget = nullptr;
    }
#endif
}

Scene* SecondScreenPreview::FindSceneForScreen(uint8_t targetScreen)
{
    World* mainWorld = GetWorld(0);
    if (mainWorld == nullptr)
        return nullptr;

    Node* root = mainWorld->GetRootNode();
    if (root == nullptr)
        return nullptr;

    const std::vector<NodePtr>& children = root->GetChildren();
    for (uint32_t i = 0; i < children.size(); ++i)
    {
        Node* child = children[i].Get();
        if (child != nullptr && child->GetTargetScreen() == targetScreen)
        {
            Scene* scene = child->GetScene();
            if (scene != nullptr)
                return scene;
        }
    }

    return nullptr;
}

static Camera3D* FindCameraForScreen(World* world, uint8_t targetScreen)
{
    if (world == nullptr)
        return nullptr;

    std::vector<Camera3D*> cameras;
    world->FindNodes(cameras);

    for (Camera3D* cam : cameras)
    {
        if (cam->GetTargetScreen() == targetScreen)
            return cam;
    }

    return nullptr;
}

void SecondScreenPreview::UpdateScreen(ScreenState& screen, uint8_t targetScreen, float deltaTime)
{
    Scene* scene = FindSceneForScreen(targetScreen);

    if (scene != screen.mCurrentScene)
    {
        screen.mCurrentScene = scene;

        if (screen.mCurrentScene != nullptr)
        {
            screen.mWorld->LoadScene(screen.mCurrentScene->GetName().c_str(), true);
        }
        else
        {
            screen.mWorld->DestroyRootNode();
            screen.mWorld->SpawnDefaultRoot();
            screen.mWorld->SpawnDefaultCamera();
        }
    }

    screen.mWorld->Update(deltaTime);
}

void SecondScreenPreview::Update(float deltaTime)
{
    if (!mEnabled)
        return;

    // During PIE, the game world is already being updated by the engine.
    if (IsPlayingInEditor())
        return;

    UpdateScreen(mTop, 0, deltaTime);
    UpdateScreen(mBottom, 1, deltaTime);
}

void SecondScreenPreview::Render()
{
    if (!mEnabled)
        return;

    // Temporarily override the editor viewport dimensions so that
    // Camera3D::ComputeMatrices() picks up the render target's aspect ratio
    // instead of the editor viewport's aspect ratio.
    EditorState* edState = GetEditorState();
    uint32_t prevVpW = edState->mViewportWidth;
    uint32_t prevVpH = edState->mViewportHeight;

    if (IsPlayingInEditor())
    {
        // During PIE, render the live game world with target-screen cameras
        World* gameWorld = GetWorld(0);
        if (gameWorld != nullptr)
        {
            if (mTop.mColorTarget != nullptr)
            {
                Camera3D* topCam = FindCameraForScreen(gameWorld, 0);
                edState->mViewportWidth = kTopWidth;
                edState->mViewportHeight = kTopHeight;
                Renderer::Get()->RenderSecondScreen(gameWorld, mTop.mColorTarget, mTop.mDepthTarget,
                                                     kTopWidth, kTopHeight, topCam, 0);
            }

            if (mBottom.mColorTarget != nullptr)
            {
                Camera3D* botCam = FindCameraForScreen(gameWorld, 1);
                edState->mViewportWidth = kBottomWidth;
                edState->mViewportHeight = kBottomHeight;
                Renderer::Get()->RenderSecondScreen(gameWorld, mBottom.mColorTarget, mBottom.mDepthTarget,
                                                     kBottomWidth, kBottomHeight, botCam, 1);
            }
        }
    }
    else
    {
        // Normal editor preview — render from separate preview worlds
        if (mTop.mWorld != nullptr && mTop.mColorTarget != nullptr)
        {
            edState->mViewportWidth = kTopWidth;
            edState->mViewportHeight = kTopHeight;
            Renderer::Get()->RenderSecondScreen(mTop.mWorld, mTop.mColorTarget, mTop.mDepthTarget,
                                                kTopWidth, kTopHeight);
        }

        if (mBottom.mWorld != nullptr && mBottom.mColorTarget != nullptr)
        {
            edState->mViewportWidth = kBottomWidth;
            edState->mViewportHeight = kBottomHeight;
            Renderer::Get()->RenderSecondScreen(mBottom.mWorld, mBottom.mColorTarget, mBottom.mDepthTarget,
                                                kBottomWidth, kBottomHeight);
        }
    }

    // Restore original viewport dimensions
    edState->mViewportWidth = prevVpW;
    edState->mViewportHeight = prevVpH;
}

static void DrawScreenImage(ImTextureID texId, uint32_t nativeW, uint32_t nativeH, float maxW)
{
    if (texId == 0)
        return;

    float aspect = (float)nativeW / (float)nativeH;
    float drawW = maxW;
    float drawH = drawW / aspect;

    // Center horizontally
    float offsetX = (ImGui::GetContentRegionAvail().x - drawW) * 0.5f;
    if (offsetX > 0.0f)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);

    ImGui::Image(texId, ImVec2(drawW, drawH));
}

void SecondScreenPreview::DrawPanel()
{
    bool wasEnabled = mEnabled;
    bool enableToggle = mEnabled;
    ImGui::Checkbox(ICON_MDI_EYE, &enableToggle);

    if (enableToggle && !wasEnabled)
        Enable();
    else if (!enableToggle && wasEnabled)
        Disable();

    ImGui::SameLine();
    bool inPie = GetEditorState()->mPlayInEditor;
    if (ImGui::Button(inPie ? "Stop" : "Play"))
    {
        if (inPie)
            GetEditorState()->EndPlayInEditor();
        else
        {
            GetEditorState()->mPlayInGameWindow = true;
            GetEditorState()->BeginPlayInEditor();
        }
    }

    if (!mEnabled)
    {
        ImGui::TextDisabled("Preview is disabled. Enable to see the 3DS screens.");
        return;
    }

    // Both screens are 240px tall natively. Scale uniformly so they
    // look like the real 3DS: top (400x240) wider, bottom (320x240) narrower.
    ImVec2 avail = ImGui::GetContentRegionAvail();
    float spacing = 4.0f;

    // Uniform scale: fit top width to panel width, and both screens + spacing in height
    float scale = avail.x / (float)kTopWidth;
    float totalH = scale * (float)(kTopHeight + kBottomHeight) + spacing;

    if (totalH > avail.y && avail.y > 0.0f)
    {
        scale = (avail.y - spacing) / (float)(kTopHeight + kBottomHeight);
    }

    float topW = scale * (float)kTopWidth;
    float topH = scale * (float)kTopHeight;
    float botW = scale * (float)kBottomWidth;
    float botH = scale * (float)kBottomHeight;
    totalH = topH + botH + spacing;

    // Center vertically
    float offsetY = (avail.y - totalH) * 0.5f;
    if (offsetY > 0.0f)
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offsetY);

    // Top screen (wider)
    DrawScreenImage(mTop.mImGuiTexId, kTopWidth, kTopHeight, topW);

    ImGui::Dummy(ImVec2(0.0f, spacing));

    // Bottom screen (narrower, centered)
    DrawScreenImage(mBottom.mImGuiTexId, kBottomWidth, kBottomHeight, botW);
    mBottomImageMin = ImGui::GetItemRectMin();
    mBottomImageMax = ImGui::GetItemRectMax();
}

void SecondScreenPreview::BeginInputRemap()
{
    mInputRemapActive = false;

    if (!IsPlayingInEditor() || !mEnabled || !GetEditorState()->mPlayInGameWindow)
        return;

    float imgW = mBottomImageMax.x - mBottomImageMin.x;
    float imgH = mBottomImageMax.y - mBottomImageMin.y;
    if (imgW <= 0.0f || imgH <= 0.0f)
        return;

    EditorState* edState = GetEditorState();

    // Save and override viewport to bottom screen resolution
    mSavedVpX = edState->mViewportX;
    mSavedVpY = edState->mViewportY;
    mSavedVpW = edState->mViewportWidth;
    mSavedVpH = edState->mViewportHeight;
    edState->mViewportX = 0;
    edState->mViewportY = 0;
    edState->mViewportWidth = kBottomWidth;
    edState->mViewportHeight = kBottomHeight;

    // Save and remap mouse from window-space to bottom-screen-resolution-space.
    // ImGui rects are in logical (scaled) coords, but INP mouse is in raw pixels,
    // so convert the image rect to raw pixel space first.
    float scale = GetEngineConfig()->mEditorInterfaceScale;
    if (scale <= 0.0f) scale = 1.0f;

    float rawMinX = mBottomImageMin.x * scale;
    float rawMinY = mBottomImageMin.y * scale;
    float rawImgW = imgW * scale;
    float rawImgH = imgH * scale;

    INP_GetMousePosition(mSavedMouseX, mSavedMouseY);
    float relX = ((float)mSavedMouseX - rawMinX) / rawImgW;
    float relY = ((float)mSavedMouseY - rawMinY) / rawImgH;
    int32_t finalX = (int32_t)(relX * (float)kBottomWidth);
    int32_t finalY = (int32_t)(relY * (float)kBottomHeight);
    INP_SetMousePosition(finalX, finalY);

    // Force all widgets to recompute their rects with the bottom screen viewport.
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

void SecondScreenPreview::EndInputRemap()
{
    if (!mInputRemapActive)
        return;

    // Mark widgets dirty so they recompute rects with the editor viewport
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
}

#endif
