# Tracks and Clips Reference

This document covers the clip timing model, keyframe system, and every built-in track/clip pair.

## Clip Timing Model

Every `TimelineClip` has these timing properties:

| Property | Type | Default | Description |
|---|---|---|---|
| `mStartTime` | `float` | `0.0` | Where the clip begins in the timeline (global time) |
| `mDuration` | `float` | `1.0` | How long the clip lasts |
| `mClipInTime` | `float` | `0.0` | Offset into the clip content (skip the beginning) |
| `mClipOutTime` | `float` | `0.0` | End offset metadata |
| `mSpeed` | `float` | `1.0` | Local playback speed multiplier |

### Key Methods

```cpp
float GetEndTime() const;       // mStartTime + mDuration
bool  ContainsTime(float t) const;  // t >= mStartTime && t < GetEndTime()
float GetLocalTime(float globalTime) const;
    // (globalTime - mStartTime) * mSpeed + mClipInTime
```

### Time Spaces

- **Global time** -- Position in the overall timeline (0 to Duration).
- **Local time** -- Position within a clip, computed by `GetLocalTime()`. Keyframe times are expressed in this space.

## Keyframe System

Clips that support keyframes (`SupportsKeyframes() == true`) store a sorted vector of keyframe structs. Each keyframe has at minimum a `mTime` field (in local time).

### Keyframe API (on TimelineClip)

```cpp
virtual bool     SupportsKeyframes() const;        // false by default
virtual uint32_t GetNumKeyframes() const;
virtual float    GetKeyframeTime(uint32_t index) const;
virtual void     AddKeyframeAtTime(float localTime, Node* targetNode);
virtual void     SetKeyframeTime(uint32_t index, float time);
virtual void     RemoveKeyframe(uint32_t index);
```

### Interpolation Modes

Defined in `TimelineTypes.h`:

```cpp
enum class InterpMode : uint8_t
{
    Linear,  // Smooth linear interpolation
    Step,    // Instant jump to value (no interpolation)
    Cubic,   // Cubic interpolation
    Count
};
```

Each keyframe can specify its own interpolation mode. The mode determines how values are blended between the current keyframe and the next.

### Evaluation Pattern

Keyframed clips use a consistent evaluation approach:

1. If there is one keyframe (or time is before the first), return the first keyframe's value.
2. If time is at or past the last keyframe, return the last keyframe's value.
3. Otherwise, find the two surrounding keyframes via binary search.
4. If the left keyframe's mode is `Step`, return its value directly.
5. Compute `t = (localTime - kf0.mTime) / (kf1.mTime - kf0.mTime)`.
6. Interpolate based on value type (mix for scalars/vectors, slerp for quaternions).

## Track State

All tracks support:

| Property | Description |
|---|---|
| `mMuted` | Muted tracks are skipped during evaluation |
| `mLocked` | Locked tracks cannot be edited in the editor (evaluation is unaffected) |

---

## TransformTrack / TransformClip

Animates the position, rotation, and scale of a `Node3D`.

**Source:** `Engine/Source/Engine/Timeline/Tracks/TransformTrack.h/.cpp`, `TransformClip.h/.cpp`

### Track Info

| Property | Value |
|---|---|
| Type Name | `"Transform"` |
| Track Color | `(0.9, 0.7, 0.2, 1.0)` |
| Target Node | `Node3D` |
| Supports Keyframes | Yes |

### TransformKeyframe

```cpp
struct TransformKeyframe {
    float     mTime = 0.0f;
    glm::vec3 mPosition = glm::vec3(0.0f);
    glm::quat mRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec3 mScale    = glm::vec3(1.0f);
    InterpMode mInterpMode = InterpMode::Linear;
};
```

### Clip Methods

```cpp
const TransformKeyframe& GetKeyframe(uint32_t index) const;
void AddKeyframe(const TransformKeyframe& kf);
void SetKeyframe(uint32_t index, const TransformKeyframe& kf);

void EvaluateAtLocalTime(float localTime,
                         glm::vec3& outPos,
                         glm::quat& outRot,
                         glm::vec3& outScale) const;
```

### Interpolation

- **Position** -- `glm::mix()` (linear)
- **Rotation** -- `glm::slerp()` (spherical linear)
- **Scale** -- `glm::mix()` (linear)
- **Step mode** -- returns the left keyframe's transform without interpolation

### Evaluation

The track finds the active clip at the current global time, computes local time, calls `EvaluateAtLocalTime()`, and applies position/rotation/scale to the target `Node3D`.

---

## ScriptValueTrack / ScriptValueClip

Animates any script-exposed property on a node by name.

**Source:** `Engine/Source/Engine/Timeline/Tracks/ScriptValueTrack.h/.cpp`, `ScriptValueClip.h/.cpp`

### Track Info

| Property | Value |
|---|---|
| Type Name | `"Script Value"` |
| Track Color | `(0.7, 0.3, 0.9, 1.0)` |
| Target Node | Any node with the named property |
| Supports Keyframes | Yes |

### Track Properties

```cpp
const std::string& GetPropertyName() const;
void SetPropertyName(const std::string& name);
```

The track stores `mPropertyName` -- the name of the property to animate on the target node.

### ScriptValueKeyframe

```cpp
struct ScriptValueKeyframe {
    float      mTime = 0.0f;
    Datum      mValue;
    InterpMode mInterpMode = InterpMode::Linear;
};
```

### Clip Methods

```cpp
const ScriptValueKeyframe& GetKeyframe(uint32_t index) const;
void AddKeyframe(const ScriptValueKeyframe& kf);
Datum EvaluateAtLocalTime(float localTime) const;
```

### Interpolation by Datum Type

| Datum Type | Interpolation |
|---|---|
| `Float` | Linear/cubic mix |
| `Vector` (vec3) | Linear mix |
| `Vector2D` (vec2) | Linear mix |
| `Color` (vec4) | Linear mix |
| All other types | Step (no interpolation) |

### Evaluation

The track finds the active clip, evaluates it at local time, looks up the property by `mPropertyName` on the target node, and sets the interpolated `Datum` value.

---

## FunctionCallTrack / FunctionCallClip

Fires Lua script functions at specific points in time.

**Source:** `Engine/Source/Engine/Timeline/Tracks/FunctionCallTrack.h/.cpp`, `FunctionCallClip.h/.cpp`

### Track Info

| Property | Value |
|---|---|
| Type Name | `"Func Call"` |
| Track Color | `(0.2, 0.8, 0.4, 1.0)` |
| Target Node | Any node with an active Script component |
| Supports Keyframes | Yes |

### FunctionCallKeyframe

```cpp
struct FunctionCallKeyframe {
    float       mTime = 0.0f;
    std::string mFunctionName;
};
```

### Clip Methods

```cpp
const std::string& GetKeyframeFunctionName(uint32_t index) const;
void SetKeyframeFunctionName(uint32_t index, const std::string& name);
```

### Fire-Once Mechanism

The clip tracks `mLastFiredKeyframe` (an index). During evaluation, only keyframes with an index greater than `mLastFiredKeyframe` whose time has been reached will fire. This prevents the same function from being called repeatedly on subsequent frames. `Reset()` sets `mLastFiredKeyframe = -1`.

### Evaluation

For each keyframe where `keyframeTime <= localTime` and `index > mLastFiredKeyframe`:
1. Look up the script on the target node.
2. Call `script->CallFunction(functionName)`.
3. Update `mLastFiredKeyframe`.

---

## ActivateTrack / ActivateClip

Toggles a node's active and visible state over a time range.

**Source:** `Engine/Source/Engine/Timeline/Tracks/ActivateTrack.h/.cpp`, `ActivateClip.h/.cpp`

### Track Info

| Property | Value |
|---|---|
| Type Name | `"Activate"` |
| Track Color | `(0.7, 0.7, 0.7, 1.0)` |
| Target Node | Any Node |
| Supports Keyframes | No |

### Clip Properties

```cpp
bool GetSetActive() const;    // default: true
bool GetSetVisible() const;   // default: true
```

### Evaluation

- If a clip contains the current time: sets `target->SetActive(clip->GetSetActive())` and `target->SetVisible(clip->GetSetVisible())`.
- If no clip is active at the current time: the node is set to inactive and invisible.
- On `Reset()`: restores the node's pre-play active/visible state from the `TimelineInstance` snapshot.

---

## AnimationTrack / AnimationClip

Drives skeletal animation playback on a `SkeletalMesh3D` node.

**Source:** `Engine/Source/Engine/Timeline/Tracks/AnimationTrack.h/.cpp`, `AnimationClip.h/.cpp`

### Track Info

| Property | Value |
|---|---|
| Type Name | `"Animation"` |
| Track Color | `(0.3, 0.4, 0.9, 1.0)` |
| Target Node | `SkeletalMesh3D` |
| Supports Keyframes | No |

### AnimationWrapMode

```cpp
enum class AnimationWrapMode {
    Loop,      // Loop the animation
    Hold,      // Hold at the last frame
    PingPong,  // Reverse direction at boundaries
    Count
};
```

### Clip Properties

| Property | Type | Default | Description |
|---|---|---|---|
| `mAnimationName` | `string` | `""` | Name of the animation to play |
| `mBlendIn` | `float` | `0.0` | Blend-in duration in seconds |
| `mBlendOut` | `float` | `0.0` | Blend-out duration in seconds |
| `mWeight` | `float` | `1.0` | Animation weight |
| `mWrapMode` | `AnimationWrapMode` | `Loop` | How to handle the animation reaching its end |

### Evaluation

1. Cast target to `SkeletalMesh3D`.
2. Compute local time from the clip.
3. Apply wrap mode to local time:
   - **Loop**: `fmod(localTime, animDuration)`
   - **PingPong**: reverse direction at boundaries
   - **Hold**: clamp (use localTime as-is)
4. Start the animation if not already active.
5. Set `mTime`, `mWeight`, and `mSpeed = 0.0f` on the active animation (speed is 0 because the timeline drives time directly).
6. When outside the clip range: stop the animation.

---

## AudioTrack / AudioClip

Plays a `SoundWave` asset on an `Audio3D` node.

**Source:** `Engine/Source/Engine/Timeline/Tracks/AudioTrack.h/.cpp`, `AudioClip.h/.cpp`

### Track Info

| Property | Value |
|---|---|
| Type Name | `"Audio"` |
| Track Color | `(0.3, 0.8, 0.3, 1.0)` |
| Target Node | `Audio3D` |
| Supports Keyframes | No |

### AudioClipEndMode

```cpp
enum class AudioClipEndMode {
    Stop,      // Stop audio when clip ends
    Continue,  // Keep playing after clip ends
    Count
};
```

### Clip Properties

| Property | Type | Default | Description |
|---|---|---|---|
| `mSoundWave` | `SoundWaveRef` | -- | The audio asset to play |
| `mVolume` | `float` | `1.0` | Volume multiplier |
| `mPitch` | `float` | `1.0` | Pitch multiplier |
| `mFadeInDuration` | `float` | `0.0` | Fade-in time in seconds |
| `mFadeOutDuration` | `float` | `0.0` | Fade-out time in seconds |
| `mEndMode` | `AudioClipEndMode` | `Stop` | What happens when the clip ends |
| `mLoop` | `bool` | `false` | Loop the audio |

### Evaluation

1. Cast target to `Audio3D`.
2. Set sound wave, loop, volume, and pitch on the audio node.
3. Apply fade envelope:
   - Fade-in: `volume *= localTime / fadeInDuration` during the first `fadeInDuration` seconds.
   - Fade-out: `volume *= timeToEnd / fadeOutDuration` during the last `fadeOutDuration` seconds.
4. Start audio if not already playing.
5. When outside clip range: stop audio (unless a clip with `EndMode::Continue` covers the time).
