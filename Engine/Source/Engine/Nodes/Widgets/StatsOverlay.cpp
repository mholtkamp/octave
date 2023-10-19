#include "Nodes/Widgets/StatsOverlay.h"
#include "Assets/Font.h"
#include "AssetManager.h"
#include "Renderer.h"
#include "Profiler.h"
#include "Engine.h"
#include "NetworkManager.h"

#include "System/System.h"

FORCE_LINK_DEF(StatsOverlay);
DEFINE_WIDGET(StatsOverlay, Canvas);

#define DEFAULT_STAT_COLOR glm::vec4(0.4f, 1.0f, 0.4f, 1.0f)

StatsOverlay::StatsOverlay()
{
    const float width = 200.0f;
    const float height = 240.0f;
    float x = -width;
    float y = 10.0f;

    SetAnchorMode(AnchorMode::TopRight);
    SetRect(x, y, width, height);
}

void StatsOverlay::Update()
{
    // (1) Determine the number of stats to display.
    uint32_t numStats = 0;

    switch (mDisplayMode)
    {
    case StatDisplayMode::FrameText:
        numStats = 1;
        break;
    case StatDisplayMode::CpuStatText:
    case StatDisplayMode::CpuStatBars:
        numStats = (uint32_t)GetProfiler()->GetCpuFrameStats().size();
        break;
    case StatDisplayMode::GpuStatText:
    case StatDisplayMode::GpuStatBars:
        numStats = (uint32_t)GetProfiler()->GetGpuStats().size();
        break;
    case StatDisplayMode::AllStatText:
        numStats = (uint32_t)GetProfiler()->GetCpuFrameStats().size();
        numStats += (uint32_t)GetProfiler()->GetGpuStats().size();
        break;
    case StatDisplayMode::Memory:
        numStats = 1;
        break;
    case StatDisplayMode::Network:
        numStats = 2;
        break;
    default:
        numStats = 0;
        break;
    }

    // (2) Hide excess stats
    OCT_ASSERT(mStatKeyTexts.size() == mStatValueTexts.size());
    for (uint32_t i = numStats; i < mStatKeyTexts.size(); ++i)
    {
        mStatKeyTexts[i]->SetVisible(false);
        mStatValueTexts[i]->SetVisible(false);
    }

    // (3) Create widget(s) if needed.
    if (numStats > mStatKeyTexts.size())
    {
        Font* font = LoadAsset<Font>("F_RobotoMono16");
        uint32_t numToAlloc = (numStats - (uint32_t)mStatKeyTexts.size());

        for (uint32_t i = 0; i < numToAlloc; ++i)
        {
            Text* newKeyText = new Text();
            newKeyText->SetColor(DEFAULT_STAT_COLOR);
            newKeyText->SetFont(font);
            AddChild(newKeyText);

            Text* newValueText = new Text();
            newValueText->SetColor(DEFAULT_STAT_COLOR);
            newValueText->SetFont(font);
            AddChild(newValueText);

            mStatKeyTexts.push_back(newKeyText);
            mStatValueTexts.push_back(newValueText);
        }
    }

    float statY = 0.0f;

    if (mDisplayMode == StatDisplayMode::Memory)
    {
#if PLATFORM_WINDOWS || PLATFORM_LINUX
        SetStatText(0, "Used Memory", SYS_GetNumBytesAllocated() / static_cast<float>(1024 * 1024), DEFAULT_STAT_COLOR, statY);
#else
        SetStatText(0, "Free Memory", SYS_GetNumBytesFree() / static_cast<float>(1024 * 1024), DEFAULT_STAT_COLOR, statY);
#endif
    }
    else if (mDisplayMode == StatDisplayMode::Network)
    {
        NetworkManager* netMan = NetworkManager::Get();
        SetStatText(0, "Upload", netMan->GetUploadRate() / 1024, DEFAULT_STAT_COLOR, statY);
        SetStatText(1, "Download", netMan->GetDownloadRate() / 1024, DEFAULT_STAT_COLOR, statY);
    }
    else
    {
        const std::vector<CpuStat>& cpuStats = GetProfiler()->GetCpuFrameStats();
        const std::vector<GpuStat>& gpuStats = GetProfiler()->GetGpuStats();
        OCT_ASSERT(numStats <= (cpuStats.size() + gpuStats.size()));
        uint32_t uStat = 0;

        if (mDisplayMode == StatDisplayMode::CpuStatBars ||
            mDisplayMode == StatDisplayMode::CpuStatText ||
            mDisplayMode == StatDisplayMode::AllStatText)
        {
            // Cpu Stats first
            for (uint32_t i = 0; i < cpuStats.size(); ++i)
            {
                SetStatText(uStat, cpuStats[i].mName, cpuStats[i].mSmoothedTime, DEFAULT_STAT_COLOR, statY);
                ++uStat;
            }
        }

        if (mDisplayMode == StatDisplayMode::GpuStatBars ||
            mDisplayMode == StatDisplayMode::GpuStatText ||
            mDisplayMode == StatDisplayMode::AllStatText)
        {
            // Gpu Stats second
            for (uint32_t i = 0; i < gpuStats.size(); ++i)
            {
                SetStatText(uStat, gpuStats[i].mName, gpuStats[i].mSmoothedTime, glm::vec4(1.0f, 0.4f, 0.4f, 1.0f), statY);
                ++uStat;
            }
        }
    }

    Canvas::Update();
}

void StatsOverlay::SetDisplayMode(StatDisplayMode mode)
{
    mDisplayMode = mode;
}

StatDisplayMode StatsOverlay::GetDisplayMode() const
{
    return mDisplayMode;
}

void StatsOverlay::SetStatText(uint32_t index, const char* key, float value, glm::vec4 color, float& y)
{
    float keyX = 0.0f;
    float valueX = 150.0f;

    Text* keyText = mStatKeyTexts[index];
    Text* valueText = mStatValueTexts[index];
    keyText->SetVisible(true);
    valueText->SetVisible(true);
    keyText->SetColor(color);
    valueText->SetColor(color);

    keyText->SetText(key);
    char valueString[16];
    snprintf(valueString, 16, "%.2f", value);
    valueText->SetText(valueString);

    keyText->SetPosition(keyX, y);
    valueText->SetPosition(valueX, y);
    keyText->SetTextSize(mTextSize);
    valueText->SetTextSize(mTextSize);

    y += mTextSize;
}
