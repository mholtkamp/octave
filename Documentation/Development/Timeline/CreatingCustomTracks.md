# Creating Custom Track and Clip Types

This guide walks through creating a new track/clip pair in C++. We use `TransformTrack`/`TransformClip` as a reference and build a hypothetical **ColorTrack/ColorClip** that animates a node's color property.

**Prerequisites:** Familiarity with the [Timeline architecture](Overview.md) and the [built-in tracks](TracksAndClips.md).

## Step 1: Define the Keyframe Struct

If your clip uses keyframes, define a struct with at least a `mTime` field. Add whatever data each keyframe carries.

```cpp
// ColorClip.h
#include "Timeline/TimelineTypes.h"
#include <glm/glm.hpp>

struct ColorKeyframe
{
    float mTime = 0.0f;
    glm::vec4 mColor = glm::vec4(1.0f);
    InterpMode mInterpMode = InterpMode::Linear;
};
```

## Step 2: Create the Clip Header

Inherit from `TimelineClip` and use the `DECLARE_CLIP` macro. Override the keyframe API if your clip uses keyframes.

```cpp
// ColorClip.h
#pragma once

#include "Timeline/TimelineClip.h"
#include <vector>

struct ColorKeyframe;

class ColorClip : public TimelineClip
{
public:
    DECLARE_CLIP(ColorClip, TimelineClip);

    ColorClip();

    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream, uint32_t version) override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;

    // Keyframe support
    virtual bool SupportsKeyframes() const override { return true; }
    virtual uint32_t GetNumKeyframes() const override;
    virtual float GetKeyframeTime(uint32_t index) const override;
    virtual void AddKeyframeAtTime(float localTime, Node* targetNode) override;
    virtual void SetKeyframeTime(uint32_t index, float time) override;
    virtual void RemoveKeyframe(uint32_t index) override;

    // Custom accessors
    const ColorKeyframe& GetKeyframe(uint32_t index) const;
    void AddKeyframe(const ColorKeyframe& kf);

    // Evaluation
    glm::vec4 EvaluateAtLocalTime(float localTime) const;

private:
    std::vector<ColorKeyframe> mKeyframes;
};
```

## Step 3: Implement the Clip

Use `FORCE_LINK_DEF` and `DEFINE_CLIP` at the top of the cpp file. Implement serialization, keyframe management, and evaluation.

```cpp
// ColorClip.cpp
#include "ColorClip.h"
#include "ObjectRef.h"

FORCE_LINK_DEF(ColorClip);
DEFINE_CLIP(ColorClip);

ColorClip::ColorClip()
{
}

void ColorClip::SaveStream(Stream& stream)
{
    TimelineClip::SaveStream(stream);  // Always call base first

    stream.WriteUint32((uint32_t)mKeyframes.size());
    for (const auto& kf : mKeyframes)
    {
        stream.WriteFloat(kf.mTime);
        stream.WriteVec4(kf.mColor);
        stream.WriteUint8((uint8_t)kf.mInterpMode);
    }
}

void ColorClip::LoadStream(Stream& stream, uint32_t version)
{
    TimelineClip::LoadStream(stream, version);  // Always call base first

    uint32_t count = stream.ReadUint32();
    mKeyframes.resize(count);
    for (uint32_t i = 0; i < count; ++i)
    {
        mKeyframes[i].mTime = stream.ReadFloat();
        mKeyframes[i].mColor = stream.ReadVec4();
        mKeyframes[i].mInterpMode = (InterpMode)stream.ReadUint8();
    }
}

void ColorClip::GatherProperties(std::vector<Property>& outProps)
{
    TimelineClip::GatherProperties(outProps);
    // Add any clip-level properties visible in the editor here
}

uint32_t ColorClip::GetNumKeyframes() const
{
    return (uint32_t)mKeyframes.size();
}

float ColorClip::GetKeyframeTime(uint32_t index) const
{
    return mKeyframes[index].mTime;
}

void ColorClip::AddKeyframeAtTime(float localTime, Node* targetNode)
{
    ColorKeyframe kf;
    kf.mTime = localTime;
    kf.mColor = glm::vec4(1.0f);  // Default white; could sample from targetNode
    AddKeyframe(kf);
}

void ColorClip::SetKeyframeTime(uint32_t index, float time)
{
    mKeyframes[index].mTime = time;
}

void ColorClip::RemoveKeyframe(uint32_t index)
{
    mKeyframes.erase(mKeyframes.begin() + index);
}

const ColorKeyframe& ColorClip::GetKeyframe(uint32_t index) const
{
    return mKeyframes[index];
}

void ColorClip::AddKeyframe(const ColorKeyframe& kf)
{
    // Insert in sorted order by time
    auto it = mKeyframes.begin();
    while (it != mKeyframes.end() && it->mTime < kf.mTime)
    {
        ++it;
    }
    mKeyframes.insert(it, kf);
}

glm::vec4 ColorClip::EvaluateAtLocalTime(float localTime) const
{
    if (mKeyframes.empty())
        return glm::vec4(1.0f);

    if (mKeyframes.size() == 1 || localTime <= mKeyframes[0].mTime)
        return mKeyframes[0].mColor;

    if (localTime >= mKeyframes.back().mTime)
        return mKeyframes.back().mColor;

    // Find surrounding keyframes
    uint32_t i = 0;
    for (; i < mKeyframes.size() - 1; ++i)
    {
        if (localTime < mKeyframes[i + 1].mTime)
            break;
    }

    const ColorKeyframe& kf0 = mKeyframes[i];
    const ColorKeyframe& kf1 = mKeyframes[i + 1];

    if (kf0.mInterpMode == InterpMode::Step)
        return kf0.mColor;

    float t = (localTime - kf0.mTime) / (kf1.mTime - kf0.mTime);
    return glm::mix(kf0.mColor, kf1.mColor, t);
}
```

## Step 4: Create the Track Header

Inherit from `TimelineTrack` and use the `DECLARE_TRACK` macro.

```cpp
// ColorTrack.h
#pragma once

#include "Timeline/TimelineTrack.h"

class ColorTrack : public TimelineTrack
{
public:
    DECLARE_TRACK(ColorTrack, TimelineTrack);

    ColorTrack();

    virtual void Evaluate(float time, Node* target, TimelineInstance* inst) override;
    virtual void Reset(Node* target, TimelineInstance* inst) override;

    virtual const char* GetTrackTypeName() const override { return "Color"; }
    virtual glm::vec4 GetTrackColor() const override;
    virtual TypeId GetDefaultClipType() const override;
};
```

## Step 5: Implement the Track

```cpp
// ColorTrack.cpp
#include "ColorTrack.h"
#include "ColorClip.h"
#include "Timeline/TimelineInstance.h"
#include "Nodes/Node3D.h"
#include "ObjectRef.h"

FORCE_LINK_DEF(ColorTrack);
DEFINE_TRACK(ColorTrack);

ColorTrack::ColorTrack()
{
}

void ColorTrack::Evaluate(float time, Node* target, TimelineInstance* inst)
{
    if (target == nullptr)
        return;

    for (uint32_t i = 0; i < GetNumClips(); ++i)
    {
        TimelineClip* baseClip = GetClip(i);

        if (baseClip->ContainsTime(time))
        {
            ColorClip* clip = static_cast<ColorClip*>(baseClip);
            float localTime = clip->GetLocalTime(time);
            glm::vec4 color = clip->EvaluateAtLocalTime(localTime);

            // Apply the color to the target node.
            // This depends on your node API -- adjust as needed.
            // Example: target->SetColor(color);
            break;
        }
    }
}

void ColorTrack::Reset(Node* target, TimelineInstance* inst)
{
    // Restore the node's original color from the pre-play snapshot.
    // Implementation depends on what state you captured.
}

glm::vec4 ColorTrack::GetTrackColor() const
{
    return glm::vec4(0.9f, 0.4f, 0.4f, 1.0f);  // Reddish tint in the editor
}

TypeId ColorTrack::GetDefaultClipType() const
{
    return ColorClip::GetStaticType();
}
```

## Step 6: Add to the Visual Studio Project

Add the new `.h` and `.cpp` files to `Engine/Engine.vcxproj`:

```xml
<!-- In an <ItemGroup> with other ClCompile entries -->
<ClCompile Include="Source\Engine\Timeline\Tracks\ColorTrack.cpp" />
<ClCompile Include="Source\Engine\Timeline\Tracks\ColorClip.cpp" />

<!-- In an <ItemGroup> with other ClInclude entries -->
<ClInclude Include="Source\Engine\Timeline\Tracks\ColorTrack.h" />
<ClInclude Include="Source\Engine\Timeline\Tracks\ColorClip.h" />
```

Add to `Engine/Engine.vcxproj.filters`:

```xml
<!-- ClCompile filter entries -->
<ClCompile Include="Source\Engine\Timeline\Tracks\ColorTrack.cpp">
  <Filter>Source Files\Engine\Timeline\Tracks</Filter>
</ClCompile>
<ClCompile Include="Source\Engine\Timeline\Tracks\ColorClip.cpp">
  <Filter>Source Files\Engine\Timeline\Tracks</Filter>
</ClCompile>

<!-- ClInclude filter entries -->
<ClInclude Include="Source\Engine\Timeline\Tracks\ColorTrack.h">
  <Filter>Header Files\Engine\Timeline\Tracks</Filter>
</ClInclude>
<ClInclude Include="Source\Engine\Timeline\Tracks\ColorClip.h">
  <Filter>Header Files\Engine\Timeline\Tracks</Filter>
</ClInclude>
```

If the filter groups do not already exist, add them with a unique GUID:

```xml
<Filter Include="Source Files\Engine\Timeline\Tracks">
  <UniqueIdentifier>{GENERATE-A-NEW-GUID}</UniqueIdentifier>
</Filter>
```

## Summary Checklist

1. **Keyframe struct** -- Define with `mTime` + your data fields + optional `mInterpMode`.
2. **Clip header** -- Inherit `TimelineClip`, use `DECLARE_CLIP`, override keyframe methods.
3. **Clip implementation** -- Use `FORCE_LINK_DEF` + `DEFINE_CLIP`, implement serialization and evaluation.
4. **Track header** -- Inherit `TimelineTrack`, use `DECLARE_TRACK`, override `Evaluate`/`Reset`/metadata.
5. **Track implementation** -- Use `FORCE_LINK_DEF` + `DEFINE_TRACK`, iterate clips and apply to node.
6. **vcxproj** -- Add all `.h` and `.cpp` files to the project and filters.

## Reference: Existing Track Implementations

| Track | File Path | Good Reference For |
|---|---|---|
| TransformTrack | `Timeline/Tracks/TransformTrack.cpp` | Keyframed evaluation with slerp |
| ScriptValueTrack | `Timeline/Tracks/ScriptValueTrack.cpp` | Property-name-based evaluation, Datum types |
| FunctionCallTrack | `Timeline/Tracks/FunctionCallTrack.cpp` | Fire-once keyframe pattern |
| ActivateTrack | `Timeline/Tracks/ActivateTrack.cpp` | Simple non-keyframed clip logic |
| AnimationTrack | `Timeline/Tracks/AnimationTrack.cpp` | Asset-driven evaluation, wrap modes |
| AudioTrack | `Timeline/Tracks/AudioTrack.cpp` | Fade envelopes, end-mode handling |
