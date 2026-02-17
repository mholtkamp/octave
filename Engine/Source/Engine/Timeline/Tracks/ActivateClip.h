#pragma once

#include "Timeline/TimelineClip.h"

class ActivateClip : public TimelineClip
{
public:

    DECLARE_CLIP(ActivateClip, TimelineClip);

    ActivateClip();
    virtual ~ActivateClip();

    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream, uint32_t version) override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;

    bool GetSetActive() const { return mSetActive; }
    bool GetSetVisible() const { return mSetVisible; }

protected:

    bool mSetActive = true;
    bool mSetVisible = true;
};
