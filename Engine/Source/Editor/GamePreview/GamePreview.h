#pragma once

#if EDITOR

#include <cstdint>
#include <string>
#include <vector>
#include "imgui.h"

class Image;
class Buffer;
class Camera3D;

struct ResolutionPreset
{
    std::string mName;
    uint32_t mWidth;
    uint32_t mHeight;
};

class GamePreview
{
public:
    void Enable();
    void Disable();
    void Render();
    void DrawPanel();

    bool IsEnabled() const { return mEnabled; }
    uint32_t GetCurrentWidth() const { return mCurrentWidth; }
    uint32_t GetCurrentHeight() const { return mCurrentHeight; }

    void BeginInputRemap();
    void EndInputRemap();

    // Hook support for addon resolution presets
    void AddResolutionPreset(const std::string& name, uint32_t w, uint32_t h);
    void RemoveResolutionPreset(const std::string& name);
    const std::vector<ResolutionPreset>& GetAddonPresets() const { return mAddonPresets; }

private:
    bool mEnabled = true;
    int32_t mSelectedPresetIndex = 0;
    int32_t mSelectedCameraIndex = 0;
    bool mShowGizmos = true;

    // Render targets (recreated on resolution change)
    Image* mColorTarget = nullptr;
    Image* mDepthTarget = nullptr;
    ImTextureID mImGuiTexId = 0;
    uint32_t mCurrentWidth = 0;
    uint32_t mCurrentHeight = 0;

    // Screenshot
    bool mScreenshotRequested = false;

    // Built-in presets
    static const std::vector<ResolutionPreset> sBuiltInPresets;

    // Addon presets (populated by hooks)
    std::vector<ResolutionPreset> mAddonPresets;

    // Cached camera list
    std::vector<Camera3D*> mCachedCameras;

    // Image display rect in window coords (captured by DrawPanel, used next frame)
    ImVec2 mImageMin = {0, 0};
    ImVec2 mImageMax = {0, 0};

    // Saved state for input remap
    int32_t mSavedMouseX = 0;
    int32_t mSavedMouseY = 0;
    uint32_t mSavedVpX = 0;
    uint32_t mSavedVpY = 0;
    uint32_t mSavedVpW = 0;
    uint32_t mSavedVpH = 0;
    bool mInputRemapActive = false;

    void CreateRenderTargets(uint32_t w, uint32_t h);
    void DestroyRenderTargets();
    void RefreshCameraList();
    void CaptureScreenshot();
    ResolutionPreset GetCurrentPreset();
    std::vector<ResolutionPreset> GetAllPresets();
};

GamePreview* GetGamePreview();

#endif
