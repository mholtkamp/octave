#pragma once

#include "Widgets/Canvas.h"
#include "Widgets/Text.h"

enum class StatDisplayMode
{
    None,
    FrameText,
    CpuStatText,
    CpuStatBars,
    Memory,
    Network,

    Count
};

class StatsOverlay : public Canvas
{
public:

    DECLARE_FACTORY(StatsOverlay, Widget);

    StatsOverlay();

    virtual void Update();

    void SetDisplayMode(StatDisplayMode mode);
    StatDisplayMode GetDisplayMode() const;

    void SetStatText(uint32_t index, const char* key, float value, float& y);

    float mTextSize = 14.0f;

    std::vector<Text*> mStatKeyTexts;
    std::vector<Text*> mStatValueTexts;
    StatDisplayMode mDisplayMode = StatDisplayMode::CpuStatText;
};
