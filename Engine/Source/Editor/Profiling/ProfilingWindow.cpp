#if EDITOR

#include "ProfilingWindow.h"
#include "Engine.h"
#include "Clock.h"
#include "Profiler.h"
#include "AssetManager.h"
#include "Assets/Texture.h"
#include "Assets/StaticMesh.h"
#include "Assets/SkeletalMesh.h"
#include "Assets/SoundWave.h"
#include "System/System.h"
#include "Packaging/PackagingSettings.h"
#include "GamePreview/GamePreview.h"
#include "imgui.h"
#include <algorithm>
#include <cmath>

static ProfilingWindow sProfilingWindow;

ProfilingWindow* GetProfilingWindow()
{
    return &sProfilingWindow;
}

void ProfilingWindow::Draw()
{
    // This is called if you need a standalone window, but we use docked panels
    DrawContent();
}

void ProfilingWindow::DrawContent()
{
    // Toggle buttons row
    ImGui::Checkbox("FPS", &mShowFPS);
    ImGui::SameLine();
    ImGui::Checkbox("CPU", &mShowCpuStats);
    ImGui::SameLine();
    ImGui::Checkbox("GPU", &mShowGpuStats);
    ImGui::SameLine();
    ImGui::Checkbox("Memory", &mShowMemory);
    ImGui::SameLine();
    ImGui::Checkbox("Graph", &mShowFrameGraph);

    ImGui::Separator();

    // Scrollable content area
    ImGui::BeginChild("ProfilingContent", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);

    if (mShowFPS)
    {
        DrawFPSSection();
    }

    if (mShowCpuStats)
    {
        DrawCpuStatsSection();
    }

    if (mShowGpuStats)
    {
        DrawGpuStatsSection();
    }

    if (mShowMemory)
    {
        DrawMemorySection();
    }

    if (mShowFrameGraph)
    {
        DrawFrameGraph();
    }

    // Custom addon stats
    if (!mCustomStats.empty())
    {
        DrawCustomStatsSection();
    }

    // Custom addon sections
    DrawCustomSections();

    ImGui::EndChild();
}

void ProfilingWindow::Tick()
{
    const Clock* clock = GetAppClock();
    if (clock == nullptr)
        return;

    float deltaTime = clock->DeltaTime();
    float frameTimeMs = deltaTime * 1000.0f;

    // Add to history
    mFrameTimeHistory.push_back(frameTimeMs);
    while (mFrameTimeHistory.size() > kFrameHistorySize)
    {
        mFrameTimeHistory.pop_front();
    }

    // Calculate smoothed values (exponential moving average)
    const float smoothFactor = 0.1f;
    mSmoothedFrameTime = mSmoothedFrameTime * (1.0f - smoothFactor) + frameTimeMs * smoothFactor;
    mSmoothedFPS = (mSmoothedFrameTime > 0.0f) ? (1000.0f / mSmoothedFrameTime) : 0.0f;

    // Calculate min/max/avg from history
    if (!mFrameTimeHistory.empty())
    {
        float minVal = mFrameTimeHistory[0];
        float maxVal = mFrameTimeHistory[0];
        float sum = 0.0f;

        for (float val : mFrameTimeHistory)
        {
            minVal = std::min(minVal, val);
            maxVal = std::max(maxVal, val);
            sum += val;
        }

        mMinFrameTime = minVal;
        mMaxFrameTime = maxVal;
        mAvgFrameTime = sum / (float)mFrameTimeHistory.size();
    }
}

void ProfilingWindow::DrawFPSSection()
{
    if (ImGui::CollapsingHeader("FPS / Frame Time", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Color coding based on FPS
        ImVec4 fpsColor;
        if (mSmoothedFPS >= 55.0f)
            fpsColor = ImVec4(0.2f, 0.9f, 0.2f, 1.0f); // Green
        else if (mSmoothedFPS >= 30.0f)
            fpsColor = ImVec4(0.9f, 0.9f, 0.2f, 1.0f); // Yellow
        else
            fpsColor = ImVec4(0.9f, 0.2f, 0.2f, 1.0f); // Red

        ImGui::TextColored(fpsColor, "FPS: %.1f", mSmoothedFPS);
        ImGui::SameLine(150.0f);
        ImGui::Text("Frame: %.2f ms", mSmoothedFrameTime);

        ImGui::Text("Min: %.2f ms", mMinFrameTime);
        ImGui::SameLine(150.0f);
        ImGui::Text("Max: %.2f ms", mMaxFrameTime);
        ImGui::SameLine(300.0f);
        ImGui::Text("Avg: %.2f ms", mAvgFrameTime);

        ImGui::Spacing();
    }
}

void ProfilingWindow::DrawStatBar(const char* name, float value, float maxValue, const ImVec4& color)
{
    float fraction = (maxValue > 0.0f) ? (value / maxValue) : 0.0f;
    fraction = std::min(fraction, 1.0f);

    ImGui::Text("%-12s", name);
    ImGui::SameLine(100.0f);
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
    char overlay[32];
    snprintf(overlay, sizeof(overlay), "%.2f ms", value);
    ImGui::ProgressBar(fraction, ImVec2(150.0f, 0), overlay);
    ImGui::PopStyleColor();
}

void ProfilingWindow::DrawCpuStatsSection()
{
    if (ImGui::CollapsingHeader("CPU Stats", ImGuiTreeNodeFlags_DefaultOpen))
    {
        Profiler* profiler = GetProfiler();
        if (profiler == nullptr)
        {
            ImGui::TextDisabled("Profiler not available");
            return;
        }

        const std::vector<CpuStat>& frameStats = profiler->GetCpuFrameStats();
        const float maxTime = 16.67f; // 60fps target
        ImVec4 cpuColor(0.3f, 0.7f, 0.9f, 1.0f); // Blue

        if (frameStats.empty())
        {
            ImGui::TextDisabled("No CPU stats recorded");
        }
        else
        {
            for (const CpuStat& stat : frameStats)
            {
                DrawStatBar(stat.mName, stat.mSmoothedTime, maxTime, cpuColor);
            }
        }

        // Show persistent stats in a tree node
        const std::vector<CpuStat>& persistentStats = profiler->GetCpuPersistentStats();
        if (!persistentStats.empty())
        {
            if (ImGui::TreeNode("Persistent Stats"))
            {
                for (const CpuStat& stat : persistentStats)
                {
                    DrawStatBar(stat.mName, stat.mSmoothedTime, maxTime, cpuColor);
                }
                ImGui::TreePop();
            }
        }

        ImGui::Spacing();
    }
}

void ProfilingWindow::DrawGpuStatsSection()
{
    if (ImGui::CollapsingHeader("GPU Stats", ImGuiTreeNodeFlags_DefaultOpen))
    {
        Profiler* profiler = GetProfiler();
        if (profiler == nullptr)
        {
            ImGui::TextDisabled("Profiler not available");
            return;
        }

        const std::vector<GpuStat>& gpuStats = profiler->GetGpuStats();
        const float maxTime = 16.67f; // 60fps target
        ImVec4 gpuColor(0.9f, 0.5f, 0.3f, 1.0f); // Orange/Red

        if (gpuStats.empty())
        {
            ImGui::TextDisabled("No GPU stats recorded");
        }
        else
        {
            for (const GpuStat& stat : gpuStats)
            {
                DrawStatBar(stat.mName, stat.mSmoothedTime, maxTime, gpuColor);
            }
        }

        ImGui::Spacing();
    }
}

void ProfilingWindow::DrawMemoryBar(const char* label, uint64_t used, uint64_t total, const ImVec4& color)
{
    float usedMB = (float)used / (1024.0f * 1024.0f);
    float totalMB = (float)total / (1024.0f * 1024.0f);
    float fraction = (total > 0) ? ((float)used / (float)total) : 0.0f;
    fraction = std::min(fraction, 1.0f);
    float percentage = fraction * 100.0f;

    // Color based on usage
    ImVec4 barColor = color;
    if (percentage >= 80.0f)
        barColor = ImVec4(0.9f, 0.2f, 0.2f, 1.0f); // Red
    else if (percentage >= 60.0f)
        barColor = ImVec4(0.9f, 0.9f, 0.2f, 1.0f); // Yellow

    ImGui::Text("%-10s", label);
    ImGui::SameLine(80.0f);
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, barColor);
    char overlay[64];
    snprintf(overlay, sizeof(overlay), "%.1f / %.1f MB (%.0f%%)", usedMB, totalMB, percentage);
    ImGui::ProgressBar(fraction, ImVec2(-1, 0), overlay);
    ImGui::PopStyleColor();
}

// Helper to get memory limit for a platform
static uint64_t GetMemoryBytesForPlatform(Platform platform)
{
    switch (platform)
    {
        case Platform::Wii:      return 88ULL * 1024 * 1024;  // 24MB MEM1 + 64MB MEM2
        case Platform::GameCube: return 43ULL * 1024 * 1024;  // ~43MB total
        case Platform::N3DS:     return 128ULL * 1024 * 1024; // 128MB
        default:                 return 0; // Windows/Linux: no fixed cap
    }
}

// Helper to get name for a platform
static const char* GetNameForPlatform(Platform platform)
{
    switch (platform)
    {
        case Platform::Windows:  return "Windows";
        case Platform::Linux:    return "Linux";
        case Platform::Android:  return "Android";
        case Platform::GameCube: return "GameCube";
        case Platform::Wii:      return "Wii";
        case Platform::N3DS:     return "3DS";
        default:                 return "Unknown";
    }
}

uint64_t ProfilingWindow::GetPlatformTotalMemoryBytes() const
{
    // Check for current target profile first
    PackagingSettings* settings = PackagingSettings::Get();
    if (settings != nullptr)
    {
        BuildProfile* target = settings->GetCurrentTargetProfile();
        if (target != nullptr)
        {
            return GetMemoryBytesForPlatform(target->mTargetPlatform);
        }
    }

    // Fall back to compile-time platform
#if PLATFORM_WII
    return 88ULL * 1024 * 1024;
#elif PLATFORM_GCN
    return 43ULL * 1024 * 1024;
#elif PLATFORM_3DS
    return 128ULL * 1024 * 1024;
#else
    return 0;
#endif
}

const char* ProfilingWindow::GetPlatformName() const
{
    // Check for current target profile first
    PackagingSettings* settings = PackagingSettings::Get();
    if (settings != nullptr)
    {
        BuildProfile* target = settings->GetCurrentTargetProfile();
        if (target != nullptr)
        {
            return GetNameForPlatform(target->mTargetPlatform);
        }
    }

    // Fall back to compile-time platform
#if PLATFORM_WII
    return "Wii";
#elif PLATFORM_GCN
    return "GameCube";
#elif PLATFORM_3DS
    return "3DS";
#elif PLATFORM_WINDOWS
    return "Windows";
#elif PLATFORM_LINUX
    return "Linux";
#elif PLATFORM_ANDROID
    return "Android";
#else
    return "Unknown";
#endif
}

void ProfilingWindow::DrawMemorySection()
{
    if (ImGui::CollapsingHeader("Memory", ImGuiTreeNodeFlags_DefaultOpen))
    {
        uint64_t platformTotal = GetPlatformTotalMemoryBytes();
        const char* platformName = GetPlatformName();

        // Check if using a target profile
        PackagingSettings* settings = PackagingSettings::Get();
        BuildProfile* target = (settings != nullptr) ? settings->GetCurrentTargetProfile() : nullptr;

        Platform targetPlatform = Platform::Windows;
        if (target != nullptr)
        {
            targetPlatform = target->mTargetPlatform;

            // Show target profile info
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Target: %s", target->mName.c_str());
            if (platformTotal > 0)
            {
                float totalMB = (float)platformTotal / (1024.0f * 1024.0f);
                ImGui::Text("Platform: %s (%.0f MB limit)", platformName, totalMB);
            }
            else
            {
                ImGui::Text("Platform: %s (no memory limit)", platformName);
            }
        }
        else
        {
            ImGui::Text("Platform: %s", platformName);
            ImGui::TextDisabled("Set a Build Profile as Current Target for memory estimates");
            ImGui::Spacing();
            ImGui::TextDisabled("No target set - showing raw process memory");

            // Fall back to showing actual process memory
            std::vector<MemoryStat> memStats = SYS_GetMemoryStats();
            ImVec4 memColor(0.3f, 0.8f, 0.5f, 1.0f);
            for (const MemoryStat& stat : memStats)
            {
                uint64_t used = stat.mBytesAllocated;
                uint64_t total = stat.mBytesAllocated + stat.mBytesFree;
                DrawMemoryBar(stat.mName.c_str(), used, total, memColor);
            }
            ImGui::Spacing();
            return;
        }

        ImGui::Spacing();

        // Calculate estimated memory for target platform
        EstimatedMemory estimated = EstimateMemoryForPlatform(targetPlatform);
        uint64_t totalEstimated = estimated.Total();

        ImVec4 texColor(0.4f, 0.7f, 0.9f, 1.0f);   // Blue
        ImVec4 meshColor(0.5f, 0.9f, 0.5f, 1.0f);  // Green
        ImVec4 audioColor(0.9f, 0.7f, 0.4f, 1.0f); // Orange
        ImVec4 rtColor(0.9f, 0.5f, 0.9f, 1.0f);    // Purple
        ImVec4 totalColor(0.3f, 0.8f, 0.5f, 1.0f); // Green

        // Show breakdown
        if (estimated.mTextures > 0)
            DrawMemoryBar("Textures", estimated.mTextures, platformTotal, texColor);
        if (estimated.mMeshes > 0)
            DrawMemoryBar("Meshes", estimated.mMeshes, platformTotal, meshColor);
        if (estimated.mSkeletalMeshes > 0)
            DrawMemoryBar("Skel Mesh", estimated.mSkeletalMeshes, platformTotal, meshColor);
        if (estimated.mAudio > 0)
            DrawMemoryBar("Audio", estimated.mAudio, platformTotal, audioColor);
        if (estimated.mRenderTargets > 0)
            DrawMemoryBar("RT/FB", estimated.mRenderTargets, platformTotal, rtColor);

        ImGui::Spacing();
        ImGui::Separator();

        // Total bar
        if (platformTotal > 0)
        {
            DrawMemoryBar("TOTAL", totalEstimated, platformTotal, totalColor);

            // Warning if over budget
            float usage = (float)totalEstimated / (float)platformTotal * 100.0f;
            if (usage > 90.0f)
            {
                ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "WARNING: Over 90%% memory budget!");
            }
            else if (usage > 75.0f)
            {
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Caution: Over 75%% memory budget");
            }
        }
        else
        {
            float totalMB = (float)totalEstimated / (1024.0f * 1024.0f);
            ImGui::Text("Estimated Total: %.2f MB", totalMB);
        }

        ImGui::Spacing();
    }
}

void ProfilingWindow::DrawFrameGraph()
{
    if (ImGui::CollapsingHeader("Frame Time Graph", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (mFrameTimeHistory.empty())
        {
            ImGui::TextDisabled("No frame history");
            return;
        }

        // Scale options
        static const char* scaleNames[] = { "Auto", "16.67ms (60fps)", "33.33ms (30fps)", "50ms", "100ms", "200ms", "500ms", "1s", "5s", "10s", "20s" };
        static const float scaleValues[] = { 0.0f, 16.67f, 33.33f, 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 5000.0f, 10000.0f, 20000.0f };
        static const int scaleCount = sizeof(scaleValues) / sizeof(scaleValues[0]);

        // Scale dropdown
        ImGui::SetNextItemWidth(150.0f);
        ImGui::Combo("Scale", &mFrameGraphScaleIndex, scaleNames, scaleCount);
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Zoom out to see larger frame time spikes");
        }

        // Convert deque to vector for ImGui
        std::vector<float> values(mFrameTimeHistory.begin(), mFrameTimeHistory.end());

        // Determine max scale value
        float maxVal;
        if (mFrameGraphScaleIndex == 0)
        {
            // Auto: scale based on actual data
            maxVal = 16.67f;
            for (float v : values)
            {
                maxVal = std::max(maxVal, v);
            }
            maxVal *= 1.2f; // Add headroom
        }
        else
        {
            // Fixed scale
            maxVal = scaleValues[mFrameGraphScaleIndex];
        }

        // Draw the graph
        char overlayText[64];
        snprintf(overlayText, sizeof(overlayText), "Frame Time (%.2f ms)", mSmoothedFrameTime);
        ImGui::PlotLines("##FrameGraph", values.data(), (int)values.size(),
                         0, overlayText, 0.0f, maxVal, ImVec2(-1, 80));

        // Reference lines legend
        ImGui::TextDisabled("60fps = 16.67ms | 30fps = 33.33ms");

        ImGui::Spacing();
    }
}

void ProfilingWindow::DrawCustomStatsSection()
{
    if (ImGui::CollapsingHeader("Custom Stats"))
    {
        ImVec4 customColor(0.8f, 0.5f, 0.8f, 1.0f); // Purple

        for (const CustomProfilingStat& stat : mCustomStats)
        {
            if (stat.mShowAsBar)
            {
                DrawStatBar(stat.mName.c_str(), stat.mValue, stat.mMaxValue, customColor);
            }
            else
            {
                ImGui::Text("%-12s: %.2f", stat.mName.c_str(), stat.mValue);
            }
        }

        ImGui::Spacing();
    }
}

void ProfilingWindow::DrawCustomSections()
{
    for (const CustomProfilingSection& section : mCustomSections)
    {
        if (ImGui::CollapsingHeader(section.mName.c_str()))
        {
            if (section.mDrawCallback)
            {
                section.mDrawCallback(section.mUserData);
            }
        }
    }
}

// Addon extension API implementation

void ProfilingWindow::RegisterCustomStat(uint64_t hookId, const char* name, const char* category, float maxValue, bool showAsBar)
{
    // Check if already registered
    for (CustomProfilingStat& stat : mCustomStats)
    {
        if (stat.mName == name)
        {
            stat.mHookId = hookId;
            stat.mCategory = category ? category : "";
            stat.mMaxValue = maxValue;
            stat.mShowAsBar = showAsBar;
            return;
        }
    }

    CustomProfilingStat stat;
    stat.mHookId = hookId;
    stat.mName = name ? name : "";
    stat.mCategory = category ? category : "";
    stat.mMaxValue = maxValue;
    stat.mShowAsBar = showAsBar;
    stat.mValue = 0.0f;
    mCustomStats.push_back(stat);
}

void ProfilingWindow::UnregisterCustomStat(uint64_t hookId, const char* name)
{
    std::string n = name ? name : "";
    mCustomStats.erase(
        std::remove_if(mCustomStats.begin(), mCustomStats.end(),
            [hookId, &n](const CustomProfilingStat& stat) {
                return stat.mHookId == hookId && stat.mName == n;
            }),
        mCustomStats.end());
}

void ProfilingWindow::SetCustomStatValue(const char* name, float value)
{
    for (CustomProfilingStat& stat : mCustomStats)
    {
        if (stat.mName == name)
        {
            stat.mValue = value;
            return;
        }
    }
}

void ProfilingWindow::RegisterCustomSection(uint64_t hookId, const char* name, void (*drawFunc)(void*), void* userData)
{
    // Check if already registered
    for (CustomProfilingSection& section : mCustomSections)
    {
        if (section.mName == name)
        {
            section.mHookId = hookId;
            section.mDrawCallback = drawFunc;
            section.mUserData = userData;
            return;
        }
    }

    CustomProfilingSection section;
    section.mHookId = hookId;
    section.mName = name ? name : "";
    section.mDrawCallback = drawFunc;
    section.mUserData = userData;
    mCustomSections.push_back(section);
}

void ProfilingWindow::UnregisterCustomSection(uint64_t hookId, const char* name)
{
    std::string n = name ? name : "";
    mCustomSections.erase(
        std::remove_if(mCustomSections.begin(), mCustomSections.end(),
            [hookId, &n](const CustomProfilingSection& section) {
                return section.mHookId == hookId && section.mName == n;
            }),
        mCustomSections.end());
}

void ProfilingWindow::RemoveAllHooks(uint64_t hookId)
{
    mCustomStats.erase(
        std::remove_if(mCustomStats.begin(), mCustomStats.end(),
            [hookId](const CustomProfilingStat& stat) {
                return stat.mHookId == hookId;
            }),
        mCustomStats.end());

    mCustomSections.erase(
        std::remove_if(mCustomSections.begin(), mCustomSections.end(),
            [hookId](const CustomProfilingSection& section) {
                return section.mHookId == hookId;
            }),
        mCustomSections.end());
}

// Platform-specific memory helpers

uint64_t ProfilingWindow::GetBytesPerPixelForPlatform(Platform platform) const
{
    // Most platforms use 4 bytes (RGBA8), but consoles often use 2 bytes (RGB565/RGBA5551)
    switch (platform)
    {
        case Platform::GameCube:
        case Platform::Wii:
        case Platform::N3DS:
            return 2; // RGB565 or RGBA5551
        default:
            return 4; // RGBA8
    }
}

uint32_t ProfilingWindow::GetIndexSizeForPlatform(Platform platform) const
{
    switch (platform)
    {
        case Platform::GameCube:
        case Platform::Wii:
        case Platform::N3DS:
            return 2; // 16-bit indices
        default:
            return 4; // 32-bit indices
    }
}

ProfilingWindow::EstimatedMemory ProfilingWindow::EstimateMemoryForPlatform(Platform platform) const
{
    EstimatedMemory mem;

    uint64_t bytesPerPixel = GetBytesPerPixelForPlatform(platform);
    uint32_t indexSize = GetIndexSizeForPlatform(platform);

    // Estimate texture memory from loaded assets
    AssetManager* assetMgr = AssetManager::Get();
    if (assetMgr != nullptr)
    {
        auto& assetMap = assetMgr->GetAssetMap();
        for (auto& pair : assetMap)
        {
            AssetStub* stub = pair.second;
            if (stub == nullptr || stub->mAsset == nullptr || !stub->mAsset->IsLoaded())
                continue;

            Asset* asset = stub->mAsset;

            // Texture memory
            if (asset->GetType() == Texture::GetStaticType())
            {
                Texture* tex = static_cast<Texture*>(asset);
                uint64_t w = tex->GetWidth();
                uint64_t h = tex->GetHeight();
                uint64_t baseSize = w * h * bytesPerPixel;

                // Add ~33% for mipmaps if mipmapped
                if (tex->IsMipmapped())
                {
                    baseSize = (baseSize * 4) / 3;
                }

                mem.mTextures += baseSize;
            }
            // Static mesh memory
            else if (asset->GetType() == StaticMesh::GetStaticType())
            {
                StaticMesh* mesh = static_cast<StaticMesh*>(asset);
                uint64_t vertexMem = mesh->GetNumVertices() * mesh->GetVertexSize();
                uint64_t indexMem = mesh->GetNumIndices() * indexSize;
                mem.mMeshes += vertexMem + indexMem;
            }
            // Skeletal mesh memory
            else if (asset->GetType() == SkeletalMesh::GetStaticType())
            {
                SkeletalMesh* mesh = static_cast<SkeletalMesh*>(asset);
                uint64_t vertexMem = mesh->GetNumVertices() * 60; // VertexSkinned = 60 bytes
                uint64_t indexMem = mesh->GetNumIndices() * indexSize;
                uint64_t boneMem = mesh->GetNumBones() * 160; // Approximate bone struct size
                mem.mSkeletalMeshes += vertexMem + indexMem + boneMem;
            }
            // Audio memory
            else if (asset->GetType() == SoundWave::GetStaticType())
            {
                SoundWave* sound = static_cast<SoundWave*>(asset);
                mem.mAudio += sound->GetWaveDataSize();
            }
        }
    }

    // Estimate render target memory based on Game Preview resolution
    GamePreview* preview = GetGamePreview();
    if (preview != nullptr && preview->IsEnabled())
    {
        uint32_t w = preview->GetCurrentWidth();
        uint32_t h = preview->GetCurrentHeight();

        // Color buffer (double buffered)
        uint64_t colorBuffer = w * h * bytesPerPixel * 2;

        // Depth buffer (typically 2 or 4 bytes)
        uint64_t depthBuffer = w * h * 2;

        mem.mRenderTargets = colorBuffer + depthBuffer;
    }
    else
    {
        // Default estimate if Game Preview not active - use a reasonable resolution
        // For target platforms like Wii (640x480) or 3DS (400x240 + 320x240)
        uint32_t w = 640;
        uint32_t h = 480;

        if (platform == Platform::N3DS)
        {
            // 3DS has two screens
            w = 400;
            h = 240 + 240;
        }

        uint64_t colorBuffer = w * h * bytesPerPixel * 2;
        uint64_t depthBuffer = w * h * 2;
        mem.mRenderTargets = colorBuffer + depthBuffer;
    }

    return mem;
}

#endif
