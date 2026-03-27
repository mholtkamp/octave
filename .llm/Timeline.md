# Timeline / Animation System

## Overview

The timeline system provides keyframe-based animation for node properties, transforms, script values, audio, and skeletal animations. A `Timeline` asset contains tracks, each targeting a node. A `TimelinePlayer` node drives playback at runtime.

## Key Files

All paths relative to `Engine/Source/`:

| File | Purpose |
|------|---------|
| `Engine/Assets/Timeline.h/.cpp` | Timeline asset class |
| `Engine/Timeline/TimelineTrack.h/.cpp` | Base track class |
| `Engine/Timeline/TimelineClip.h/.cpp` | Base clip class |
| `Engine/Timeline/TimelineTypes.h` | InterpMode enum |
| `Engine/Timeline/TimelineInstance.h/.cpp` | Runtime state per track |
| `Engine/Nodes/TimelinePlayer.h/.cpp` | Playback node |
| `Engine/Timeline/Tracks/` | All track + clip implementations |
| `Editor/Timeline/TimelinePanel.h/.cpp` | Editor UI |
| `Editor/Timeline/TimelineActions.h/.cpp` | Undo/redo actions |

## Timeline Asset

```cpp
class Timeline : public Asset {
    float GetDuration() const;            // Default 5.0f
    void SetDuration(float duration);
    float GetPlayRate() const;            // Default 1.0f
    bool IsLooping() const;
    bool IsAutoPlay() const;

    uint32_t GetNumTracks() const;
    TimelineTrack* GetTrack(uint32_t index) const;
    TimelineTrack* AddTrack(TypeId trackType);
    void RemoveTrack(uint32_t index);
    void MoveTrack(uint32_t from, uint32_t to);
};
```

## TimelinePlayer Node

Inherits from `Node`. Attaches to the scene tree and drives a Timeline asset:

```cpp
class TimelinePlayer : public Node {
    void SetTimeline(Timeline* timeline);
    void Play();
    void Pause();
    void StopPlayback();
    void SetTime(float time);
    float GetTime() const;
    float GetProgress() const;    // Returns 0.0-1.0
    bool IsPlaying() const;
    void SetPlayOnStart(bool);    // Auto-play on game start
};
```

**Signals:**
- `OnStarted` — when Play() is called
- `OnFinished` — when non-looping timeline reaches end
- `OnStopped` — when StopPlayback() is called
- `OnStateChanged` — on any state change

Pattern: `EmitSignal("SignalName", {})` + `CallFunction("SignalName")` for script callbacks.

**Tick flow:**
1. Advance `mCurrentTime` by `deltaTime * playRate`
2. Handle looping (wrap) or stop at end
3. `EvaluateTimeline(mCurrentTime)`:
   - Ensure runtime instance exists
   - Resolve track bindings (UUID → live Node*)
   - For each non-muted track: `track->Evaluate(time, resolvedNode, instance)`

## Track Types

Located in `Engine/Source/Engine/Timeline/Tracks/`:

| Track | Clip | Purpose |
|-------|------|---------|
| `TransformTrack` | `TransformClip` | Animate position/rotation/scale of a Node3D |
| `ScriptValueTrack` | `ScriptValueClip` | Animate any script-exposed property |
| `FunctionCallTrack` | `FunctionCallClip` | Fire Lua functions at specific times |
| `ActivateTrack` | `ActivateClip` | Toggle node active/visible state |
| `AnimationTrack` | `AnimationClip` | Drive skeletal animations on SkeletalMesh3D |
| `AudioTrack` | `AudioClip` | Play sound waves on Audio3D nodes |

All use `DECLARE_TRACK(Class, Parent)` / `DEFINE_TRACK(Class)` macros.

## TimelineTrack Base

```cpp
class TimelineTrack : public Object {
    virtual void Evaluate(float time, Node* target, TimelineInstance* inst);
    virtual void Reset(Node* target, TimelineInstance* inst);

    uint64_t GetTargetNodeUuid() const;     // Binding to scene node
    void SetTargetNodeName(const std::string& name);

    uint32_t GetNumClips() const;
    TimelineClip* GetClip(uint32_t index) const;
    void AddClip(TimelineClip* clip);
    void RemoveClip(uint32_t index);

    bool IsMuted() const;
    bool IsLocked() const;
    virtual TypeId GetDefaultClipType() const;
};
```

## TimelineClip Base

```cpp
class TimelineClip : public Object {
    float GetStartTime() const;
    float GetDuration() const;
    float GetEndTime() const;       // mStartTime + mDuration
    float GetSpeed() const;

    float GetClipInTime() const;    // Offset into clip content
    bool ContainsTime(float globalTime) const;
    float GetLocalTime(float globalTime) const;

    virtual bool SupportsKeyframes() const;
    virtual uint32_t GetNumKeyframes() const;
    virtual void AddKeyframeAtTime(float localTime, Node* targetNode);
    virtual void SetKeyframeTime(uint32_t index, float time);
    virtual void RemoveKeyframe(uint32_t index);
};
```

## Interpolation

```cpp
enum class InterpMode : uint8_t { Linear, Step, Cubic, Count };
```

## Keyframe Types

**TransformKeyframe**: `{ float mTime; vec3 mPosition; quat mRotation; vec3 mScale; InterpMode mInterpMode; }`

**ScriptValueKeyframe**: `{ float mTime; Datum mValue; InterpMode mInterpMode; }`

**FunctionCallKeyframe**: `{ float mTime; std::string mFunctionName; }` — fire-once tracking via `mLastFiredKeyframe`.

## TimelineInstance (Runtime State)

Holds per-track runtime data:
```cpp
struct TrackInstanceData {
    Node* mResolvedNode;          // Resolved from UUID at runtime
    bool mBindingResolved;
    // Pre-play snapshots for undo
    vec3 mPrePlayPosition; quat mPrePlayRotation; vec3 mPrePlayScale;
    bool mPrePlayActive; bool mPrePlayVisible;
    bool mStateSnapshotted;
};
```

`ResolveBindings()` maps track UUIDs to live Node pointers. `CapturePrePlayState()` / `RestorePrePlayState()` enable editor preview with undo.

## Editor Integration

**Files:** `Editor/Timeline/TimelinePanel.h/.cpp`, `TimelineActions.h/.cpp`

API: `DrawTimelinePanel()`, `OpenTimelineForEditing(Timeline*)`, `CloseTimelinePanel()`.

State in EditorState: `mEditedTimelineRef`, `mTimelinePreviewInstance`, `mTimelinePlayheadTime`, `mTimelineZoom`, `mTimelineScrollX`, `mTimelineSnapInterval`, selection indices.

Features: track management, clip editing, keyframe editing, playback preview with scrubbing, zoom/pan, snap-to-grid, undo/redo via TimelineActions.

## Documentation

User-facing docs in `Documentation/Development/Timeline/`: Overview, TimelinePlayer, TracksAndClips, LuaAPI, CreatingCustomTracks.
