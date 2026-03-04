#include "Timeline/Tracks/ScriptValueTrack.h"
#include "Timeline/Tracks/ScriptValueClip.h"
#include "Timeline/TimelineInstance.h"
#include "Nodes/Node.h"
#include "Property.h"
#include "Utilities.h"

FORCE_LINK_DEF(ScriptValueTrack);
DEFINE_TRACK(ScriptValueTrack);

ScriptValueTrack::ScriptValueTrack()
{
}

ScriptValueTrack::~ScriptValueTrack()
{
}

void ScriptValueTrack::SaveStream(Stream& stream)
{
    TimelineTrack::SaveStream(stream);

    stream.WriteString(mPropertyName);
}

void ScriptValueTrack::LoadStream(Stream& stream, uint32_t version)
{
    TimelineTrack::LoadStream(stream, version);

    stream.ReadString(mPropertyName);
}

void ScriptValueTrack::GatherProperties(std::vector<Property>& outProps)
{
    TimelineTrack::GatherProperties(outProps);

    SCOPED_CATEGORY("Script Value");

    outProps.push_back(Property(DatumType::String, "Property Name", this, &mPropertyName));
}

void ScriptValueTrack::Evaluate(float time, Node* target, TimelineInstance* inst)
{
    if (target == nullptr || mPropertyName.empty())
        return;

    std::vector<Property> props;
    target->GatherProperties(props);

    Property* targetProp = nullptr;
    for (uint32_t p = 0; p < props.size(); ++p)
    {
        if (props[p].mName == mPropertyName)
        {
            targetProp = &props[p];
            break;
        }
    }

    if (targetProp == nullptr)
        return;

    for (uint32_t i = 0; i < mClips.size(); ++i)
    {
        if (mClips[i]->GetType() != ScriptValueClip::GetStaticType())
            continue;

        ScriptValueClip* clip = static_cast<ScriptValueClip*>(mClips[i]);

        if (clip->ContainsTime(time))
        {
            float localTime = clip->GetLocalTime(time);
            Datum value = clip->EvaluateAtLocalTime(localTime);

            if (value.GetType() != DatumType::Count)
            {
                targetProp->SetValue(value.mData.vp, 0, 1);
            }
            break;
        }
    }
}

void ScriptValueTrack::Reset(Node* target, TimelineInstance* inst)
{
}

glm::vec4 ScriptValueTrack::GetTrackColor() const
{
    return glm::vec4(0.7f, 0.3f, 0.9f, 1.0f);
}

TypeId ScriptValueTrack::GetDefaultClipType() const
{
    return ScriptValueClip::GetStaticType();
}
