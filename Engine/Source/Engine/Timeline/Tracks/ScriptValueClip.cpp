#include "Timeline/Tracks/ScriptValueClip.h"
#include "Utilities.h"

FORCE_LINK_DEF(ScriptValueClip);
DEFINE_CLIP(ScriptValueClip);

ScriptValueClip::ScriptValueClip()
{
}

ScriptValueClip::~ScriptValueClip()
{
}

void ScriptValueClip::SaveStream(Stream& stream)
{
    TimelineClip::SaveStream(stream);

    stream.WriteUint32((uint32_t)mKeyframes.size());
    for (uint32_t i = 0; i < mKeyframes.size(); ++i)
    {
        const ScriptValueKeyframe& kf = mKeyframes[i];
        stream.WriteFloat(kf.mTime);
        stream.WriteUint8((uint8_t)kf.mInterpMode);
        stream.WriteUint8((uint8_t)kf.mValue.GetType());

        switch (kf.mValue.GetType())
        {
        case DatumType::Float: stream.WriteFloat(kf.mValue.GetFloat()); break;
        case DatumType::Integer: stream.WriteInt32(kf.mValue.GetInteger()); break;
        case DatumType::Bool: stream.WriteBool(kf.mValue.GetBool()); break;
        case DatumType::String: stream.WriteString(kf.mValue.GetString()); break;
        case DatumType::Vector: stream.WriteVec3(kf.mValue.GetVector()); break;
        case DatumType::Vector2D: stream.WriteVec2(kf.mValue.GetVector2D()); break;
        case DatumType::Color: stream.WriteVec4(kf.mValue.GetColor()); break;
        default: break;
        }
    }
}

void ScriptValueClip::LoadStream(Stream& stream, uint32_t version)
{
    TimelineClip::LoadStream(stream, version);

    uint32_t numKeyframes = stream.ReadUint32();
    mKeyframes.resize(numKeyframes);
    for (uint32_t i = 0; i < numKeyframes; ++i)
    {
        mKeyframes[i].mTime = stream.ReadFloat();
        mKeyframes[i].mInterpMode = (InterpMode)stream.ReadUint8();
        DatumType type = (DatumType)stream.ReadUint8();

        switch (type)
        {
        case DatumType::Float: mKeyframes[i].mValue = Datum(stream.ReadFloat()); break;
        case DatumType::Integer: mKeyframes[i].mValue = Datum(stream.ReadInt32()); break;
        case DatumType::Bool: mKeyframes[i].mValue = Datum(stream.ReadBool()); break;
        case DatumType::String: { std::string s; stream.ReadString(s); mKeyframes[i].mValue = Datum(s); break; }
        case DatumType::Vector: mKeyframes[i].mValue = Datum(stream.ReadVec3()); break;
        case DatumType::Vector2D: mKeyframes[i].mValue = Datum(stream.ReadVec2()); break;
        case DatumType::Color: mKeyframes[i].mValue = Datum(stream.ReadVec4()); break;
        default: break;
        }
    }
}

void ScriptValueClip::GatherProperties(std::vector<Property>& outProps)
{
    TimelineClip::GatherProperties(outProps);

#if EDITOR
    static const char* sInterpModeStrings[] = { "Linear", "Step", "Cubic" };

    SCOPED_CATEGORY("Keyframes");

    uint32_t startIdx = 0;
    uint32_t endIdx = 0;
    GetKeyframeDisplayRange(startIdx, endIdx);

    for (uint32_t i = startIdx; i < endIdx; ++i)
    {
        char timeName[32];
        snprintf(timeName, sizeof(timeName), "KF %d Time", i);
        outProps.push_back(Property(DatumType::Float, timeName, this, &mKeyframes[i].mTime));

        DatumType valueType = mKeyframes[i].mValue.GetType();
        if (valueType != DatumType::Count)
        {
            char valName[32];
            snprintf(valName, sizeof(valName), "KF %d Value", i);
            void* valuePtr = mKeyframes[i].mValue.GetValue(0);
            outProps.push_back(Property(valueType, valName, this, valuePtr));
        }

        char interpName[32];
        snprintf(interpName, sizeof(interpName), "KF %d Interp", i);
        outProps.push_back(Property(DatumType::Byte, interpName, this, &mKeyframes[i].mInterpMode, 1, nullptr, NULL_DATUM, (int32_t)InterpMode::Count, sInterpModeStrings));
    }
#endif
}

void ScriptValueClip::AddKeyframe(const ScriptValueKeyframe& kf)
{
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

void ScriptValueClip::AddKeyframeAtTime(float localTime, Node* targetNode)
{
    ScriptValueKeyframe kf;
    kf.mTime = localTime;

    // Match existing keyframe type, default to Float
    if (!mKeyframes.empty())
    {
        DatumType existingType = mKeyframes[0].mValue.GetType();
        switch (existingType)
        {
        case DatumType::Float:   kf.mValue = Datum(0.0f); break;
        case DatumType::Integer: kf.mValue = Datum((int32_t)0); break;
        case DatumType::Bool:    kf.mValue = Datum(false); break;
        case DatumType::String:  kf.mValue = Datum(std::string("")); break;
        case DatumType::Vector:  kf.mValue = Datum(glm::vec3(0.0f)); break;
        case DatumType::Vector2D:kf.mValue = Datum(glm::vec2(0.0f)); break;
        case DatumType::Color:   kf.mValue = Datum(glm::vec4(1.0f)); break;
        default:                 kf.mValue = Datum(0.0f); break;
        }
    }
    else
    {
        kf.mValue = Datum(0.0f);
    }

    AddKeyframe(kf);
}

void ScriptValueClip::SetKeyframeTime(uint32_t index, float time)
{
    if (index < mKeyframes.size())
    {
        mKeyframes[index].mTime = time;
    }
}

void ScriptValueClip::RemoveKeyframe(uint32_t index)
{
    if (index < mKeyframes.size())
    {
        mKeyframes.erase(mKeyframes.begin() + index);
    }
}

Datum ScriptValueClip::EvaluateAtLocalTime(float localTime) const
{
    if (mKeyframes.empty())
        return Datum();

    if (mKeyframes.size() == 1 || localTime <= mKeyframes[0].mTime)
    {
        return mKeyframes[0].mValue;
    }

    if (localTime >= mKeyframes.back().mTime)
    {
        return mKeyframes.back().mValue;
    }

    // Find surrounding keyframes
    uint32_t low = 0;
    uint32_t high = (uint32_t)mKeyframes.size() - 1;
    while (low < high - 1)
    {
        uint32_t mid = (low + high) / 2;
        if (mKeyframes[mid].mTime <= localTime)
            low = mid;
        else
            high = mid;
    }

    const ScriptValueKeyframe& kf0 = mKeyframes[low];
    const ScriptValueKeyframe& kf1 = mKeyframes[high];

    if (kf0.mInterpMode == InterpMode::Step)
    {
        return kf0.mValue;
    }

    float t = (localTime - kf0.mTime) / (kf1.mTime - kf0.mTime);
    t = glm::clamp(t, 0.0f, 1.0f);

    // Interpolate based on type
    DatumType type = kf0.mValue.GetType();
    switch (type)
    {
    case DatumType::Float:
        return Datum(glm::mix(kf0.mValue.GetFloat(), kf1.mValue.GetFloat(), t));
    case DatumType::Vector:
        return Datum(glm::mix(kf0.mValue.GetVector(), kf1.mValue.GetVector(), t));
    case DatumType::Vector2D:
        return Datum(glm::mix(kf0.mValue.GetVector2D(), kf1.mValue.GetVector2D(), t));
    case DatumType::Color:
        return Datum(glm::mix(kf0.mValue.GetColor(), kf1.mValue.GetColor(), t));
    default:
        // Non-interpolatable types use step
        return kf0.mValue;
    }
}
