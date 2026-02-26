#pragma once

#if EDITOR

#include <cstdint>
#include "imgui.h"

class World;
class Image;
class Scene;

class SecondScreenPreview
{
public:
    void Enable();
    void Disable();
    void Update(float deltaTime);
    void Render();
    void DrawPanel();

    void BeginInputRemap();
    void EndInputRemap();

    bool IsEnabled() const { return mEnabled; }

private:
    struct ScreenState
    {
        World* mWorld = nullptr;
        Image* mColorTarget = nullptr;
        Image* mDepthTarget = nullptr;
        ImTextureID mImGuiTexId = 0;
        Scene* mCurrentScene = nullptr;
    };

    bool mEnabled = false;
    ScreenState mTop;
    ScreenState mBottom;

    // Bottom screen image rect (captured by DrawPanel, used by BeginInputRemap)
    ImVec2 mBottomImageMin = {0, 0};
    ImVec2 mBottomImageMax = {0, 0};

    // Saved state for input remap
    int32_t mSavedMouseX = 0;
    int32_t mSavedMouseY = 0;
    uint32_t mSavedVpX = 0;
    uint32_t mSavedVpY = 0;
    uint32_t mSavedVpW = 0;
    uint32_t mSavedVpH = 0;
    bool mInputRemapActive = false;

    static constexpr uint32_t kTopWidth = 400;
    static constexpr uint32_t kTopHeight = 240;
    static constexpr uint32_t kBottomWidth = 320;
    static constexpr uint32_t kBottomHeight = 240;

    void CreateScreenTargets(ScreenState& screen, uint32_t width, uint32_t height, const char* debugName);
    void DestroyScreenTargets(ScreenState& screen);
    Scene* FindSceneForScreen(uint8_t targetScreen);
    void UpdateScreen(ScreenState& screen, uint8_t targetScreen, float deltaTime);
};

SecondScreenPreview* GetSecondScreenPreview();

#endif
