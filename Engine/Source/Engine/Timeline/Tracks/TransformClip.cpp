#include "Timeline/Tracks/TransformClip.h"
#include "Nodes/3D/Node3d.h"
#include "Utilities.h"

FORCE_LINK_DEF(TransformClip);
DEFINE_CLIP(TransformClip);

TransformClip::TransformClip()
{
}

TransformClip::~TransformClip()
{
}

void TransformClip::SaveStream(Stream& stream)
{
    TimelineClip::SaveStream(stream);

    stream.WriteUint32((uint32_t)mKeyframes.size());
    for (uint32_t i = 0; i < mKeyframes.size(); ++i)
    {
        const TransformKeyframe& kf = mKeyframes[i];
        stream.WriteFloat(kf.mTime);
        stream.WriteVec3(kf.mPosition);
        stream.WriteQuat(kf.mRotation);
        stream.WriteVec3(kf.mScale);
        stream.WriteUint8((uint8_t)kf.mInterpMode);
    }
}

void TransformClip::LoadStream(Stream& stream, uint32_t version)
{
    TimelineClip::LoadStream(stream, version);

    uint32_t numKeyframes = stream.ReadUint32();
    mKeyframes.resize(numKeyframes);
    for (uint32_t i = 0; i < numKeyframes; ++i)
    {
        mKeyframes[i].mTime = stream.ReadFloat();
        mKeyframes[i].mPosition = stream.ReadVec3();
        mKeyframes[i].mRotation = stream.ReadQuat();
        mKeyframes[i].mScale = stream.ReadVec3();
        mKeyframes[i].mInterpMode = (InterpMode)stream.ReadUint8();
    }
}

#if EDITOR
static bool HandleTransformClipEulerChange(Datum* datum, uint32_t index, const void* newValue)
{
    // The datum's external data points to mEditorRotationEuler[i]
    // We need to find the TransformClip owner and sync the quat
    Property* prop = static_cast<Property*>(datum);
    TransformClip* clip = static_cast<TransformClip*>(prop->mOwner);

    // Accept the new euler value first (use SetValueRaw to avoid re-triggering this handler)
    datum->SetValueRaw(newValue, index);

    // Find which keyframe this is by parsing the property name "KF N Rot"
    const std::string& name = prop->mName;
    int kfIndex = 0;
    if (name.size() > 3 && name[0] == 'K' && name[1] == 'F' && name[2] == ' ')
    {
        kfIndex = atoi(name.c_str() + 3);
    }

    if ((uint32_t)kfIndex < clip->GetNumKeyframes())
    {
        glm::vec3 euler = *(const glm::vec3*)datum->GetValue(0);
        glm::quat quat = glm::quat(glm::radians(euler));
        TransformKeyframe kf = clip->GetKeyframe(kfIndex);
        kf.mRotation = quat;
        clip->SetKeyframe(kfIndex, kf);
    }

    return true;
}
#endif

void TransformClip::GatherProperties(std::vector<Property>& outProps)
{
    TimelineClip::GatherProperties(outProps);

#if EDITOR
    static const char* sInterpModeStrings[] = { "Linear", "Step", "Cubic" };

    // Sync euler cache size
    mEditorRotationEuler.resize(mKeyframes.size());
    for (uint32_t i = 0; i < mKeyframes.size(); ++i)
    {
        mEditorRotationEuler[i] = glm::degrees(glm::eulerAngles(mKeyframes[i].mRotation));
    }

    SCOPED_CATEGORY("Keyframes");

    uint32_t startIdx = 0;
    uint32_t endIdx = 0;
    GetKeyframeDisplayRange(startIdx, endIdx);

    for (uint32_t i = startIdx; i < endIdx; ++i)
    {
        char timeName[32];
        snprintf(timeName, sizeof(timeName), "KF %d Time", i);
        outProps.push_back(Property(DatumType::Float, timeName, this, &mKeyframes[i].mTime));

        char posName[32];
        snprintf(posName, sizeof(posName), "KF %d Pos", i);
        outProps.push_back(Property(DatumType::Vector, posName, this, &mKeyframes[i].mPosition));

        char rotName[32];
        snprintf(rotName, sizeof(rotName), "KF %d Rot", i);
        outProps.push_back(Property(DatumType::Vector, rotName, this, &mEditorRotationEuler[i], 1, HandleTransformClipEulerChange));

        char scaleName[32];
        snprintf(scaleName, sizeof(scaleName), "KF %d Scale", i);
        outProps.push_back(Property(DatumType::Vector, scaleName, this, &mKeyframes[i].mScale));

        char interpName[32];
        snprintf(interpName, sizeof(interpName), "KF %d Interp", i);
        outProps.push_back(Property(DatumType::Byte, interpName, this, &mKeyframes[i].mInterpMode, 1, nullptr, NULL_DATUM, (int32_t)InterpMode::Count, sInterpModeStrings));
    }
#endif
}

void TransformClip::AddKeyframe(const TransformKeyframe& kf)
{
    // Insert sorted by time
    bool inserted = false;
    for (uint32_t i = 0; i < mKeyframes.size(); ++i)
    {
        if (kf.mTime < mKeyframes[i].mTime)
        {
            mKeyframes.insert(mKeyframes.begin() + i, kf);
            inserted = true;
            break;
        }
    }

    if (!inserted)
    {
        mKeyframes.push_back(kf);
    }
}

void TransformClip::AddKeyframeAtTime(float localTime, Node* targetNode)
{
    TransformKeyframe kf;
    kf.mTime = localTime;

    if (targetNode != nullptr && targetNode->IsNode3D())
    {
        Node3D* node3d = static_cast<Node3D*>(targetNode);
        kf.mPosition = node3d->GetPosition();
        kf.mRotation = node3d->GetRotationQuat();
        kf.mScale = node3d->GetScale();
    }

    AddKeyframe(kf);
}

void TransformClip::SetKeyframeTime(uint32_t index, float time)
{
    if (index < mKeyframes.size())
    {
        mKeyframes[index].mTime = time;
    }
}

void TransformClip::RemoveKeyframe(uint32_t index)
{
    if (index < mKeyframes.size())
    {
        mKeyframes.erase(mKeyframes.begin() + index);
    }
}

void TransformClip::SetKeyframe(uint32_t index, const TransformKeyframe& kf)
{
    if (index < mKeyframes.size())
    {
        mKeyframes[index] = kf;
    }
}

void TransformClip::EvaluateAtLocalTime(float localTime, glm::vec3& outPos, glm::quat& outRot, glm::vec3& outScale) const
{
    if (mKeyframes.empty())
        return;

    if (mKeyframes.size() == 1 || localTime <= mKeyframes[0].mTime)
    {
        outPos = mKeyframes[0].mPosition;
        outRot = mKeyframes[0].mRotation;
        outScale = mKeyframes[0].mScale;
        return;
    }

    if (localTime >= mKeyframes.back().mTime)
    {
        outPos = mKeyframes.back().mPosition;
        outRot = mKeyframes.back().mRotation;
        outScale = mKeyframes.back().mScale;
        return;
    }

    // Binary search for the surrounding keyframes
    uint32_t low = 0;
    uint32_t high = (uint32_t)mKeyframes.size() - 1;
    while (low < high - 1)
    {
        uint32_t mid = (low + high) / 2;
        if (mKeyframes[mid].mTime <= localTime)
        {
            low = mid;
        }
        else
        {
            high = mid;
        }
    }

    const TransformKeyframe& kf0 = mKeyframes[low];
    const TransformKeyframe& kf1 = mKeyframes[high];

    if (kf0.mInterpMode == InterpMode::Step)
    {
        outPos = kf0.mPosition;
        outRot = kf0.mRotation;
        outScale = kf0.mScale;
        return;
    }

    float t = (localTime - kf0.mTime) / (kf1.mTime - kf0.mTime);
    t = glm::clamp(t, 0.0f, 1.0f);

    outPos = glm::mix(kf0.mPosition, kf1.mPosition, t);
    outRot = glm::slerp(kf0.mRotation, kf1.mRotation, t);
    outScale = glm::mix(kf0.mScale, kf1.mScale, t);
}
