#include "Timeline/Tracks/ActivateClip.h"
#include "Utilities.h"

FORCE_LINK_DEF(ActivateClip);
DEFINE_CLIP(ActivateClip);

ActivateClip::ActivateClip()
{
}

ActivateClip::~ActivateClip()
{
}

void ActivateClip::SaveStream(Stream& stream)
{
    TimelineClip::SaveStream(stream);

    stream.WriteBool(mSetActive);
    stream.WriteBool(mSetVisible);
}

void ActivateClip::LoadStream(Stream& stream, uint32_t version)
{
    TimelineClip::LoadStream(stream, version);

    mSetActive = stream.ReadBool();
    mSetVisible = stream.ReadBool();
}

void ActivateClip::GatherProperties(std::vector<Property>& outProps)
{
    TimelineClip::GatherProperties(outProps);

    SCOPED_CATEGORY("Activate");

    outProps.push_back(Property(DatumType::Bool, "Set Active", this, &mSetActive));
    outProps.push_back(Property(DatumType::Bool, "Set Visible", this, &mSetVisible));
}
