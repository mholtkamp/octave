#pragma once

#if EDITOR

#include <deque>
#include <vector>
#include <string>
#include "System/SystemTypes.h"
#include "EngineTypes.h"
#include "imgui.h"

// Frame history buffer size (~2 seconds at 60fps)
constexpr uint32_t kFrameHistorySize = 120;

// Custom stat registered by addons
struct CustomProfilingStat
{
    uint64_t mHookId = 0;
    std::string mName;
    std::string mCategory;
    float mValue = 0.0f;
    float mMaxValue = 16.67f;
    bool mShowAsBar = true;
};

// Custom section registered by addons
struct CustomProfilingSection
{
    uint64_t mHookId = 0;
    std::string mName;
    void (*mDrawCallback)(void*) = nullptr;
    void* mUserData = nullptr;
};

class ProfilingWindow
{
public:
    void Draw();
    void DrawContent();
    void Tick();

    // Addon extension API
    void RegisterCustomStat(uint64_t hookId, const char* name, const char* category, float maxValue, bool showAsBar);
    void UnregisterCustomStat(uint64_t hookId, const char* name);
    void SetCustomStatValue(const char* name, float value);

    void RegisterCustomSection(uint64_t hookId, const char* name, void (*drawFunc)(void*), void* userData);
    void UnregisterCustomSection(uint64_t hookId, const char* name);

    void RemoveAllHooks(uint64_t hookId);

    // Section visibility toggles
    bool mShowFPS = true;
    bool mShowCpuStats = true;
    bool mShowGpuStats = true;
    bool mShowMemory = true;
    bool mShowFrameGraph = true;

private:
    // Frame time history for graph
    std::deque<float> mFrameTimeHistory;

    // Smoothed values
    float mSmoothedFPS = 60.0f;
    float mSmoothedFrameTime = 16.67f;
    float mMinFrameTime = 16.67f;
    float mMaxFrameTime = 16.67f;
    float mAvgFrameTime = 16.67f;

    // Custom addon stats and sections
    std::vector<CustomProfilingStat> mCustomStats;
    std::vector<CustomProfilingSection> mCustomSections;

    // Drawing helpers
    void DrawFPSSection();
    void DrawCpuStatsSection();
    void DrawGpuStatsSection();
    void DrawMemorySection();
    void DrawFrameGraph();
    void DrawCustomStatsSection();
    void DrawCustomSections();

    void DrawStatBar(const char* name, float value, float maxValue, const ImVec4& color);
    void DrawMemoryBar(const char* label, uint64_t used, uint64_t total, const ImVec4& color);

    uint64_t GetPlatformTotalMemoryBytes() const;
    const char* GetPlatformName() const;

    // Estimated memory calculation for target platform
    struct EstimatedMemory
    {
        uint64_t mTextures = 0;
        uint64_t mMeshes = 0;
        uint64_t mSkeletalMeshes = 0;
        uint64_t mAudio = 0;
        uint64_t mRenderTargets = 0;
        uint64_t mOther = 0;

        uint64_t Total() const { return mTextures + mMeshes + mSkeletalMeshes + mAudio + mRenderTargets + mOther; }
    };

    EstimatedMemory EstimateMemoryForPlatform(Platform platform) const;
    uint64_t GetBytesPerPixelForPlatform(Platform platform) const;
    uint32_t GetIndexSizeForPlatform(Platform platform) const;
};

ProfilingWindow* GetProfilingWindow();

#endif
