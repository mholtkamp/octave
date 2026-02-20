# Timeline System Overview

The Timeline system enables sequenced, time-based control of scene nodes. Use it to orchestrate transforms, animations, audio, property changes, function calls, and visibility across multiple nodes from a single authored asset.

## Architecture

```
Timeline (Asset)
  |
  +-- Track 0 (targets Node A via UUID)
  |     +-- Clip 0 [StartTime..EndTime]
  |     |     +-- Keyframe 0 (local time, value)
  |     |     +-- Keyframe 1 (local time, value)
  |     +-- Clip 1 [StartTime..EndTime]
  |
  +-- Track 1 (targets Node B via UUID)
  |     +-- Clip 0 [StartTime..EndTime]
  |
  +-- Track N ...
```

- **Timeline** -- An asset containing tracks, a duration, play rate, and loop/auto-play flags.
- **Track** -- Targets a single scene node (bound by UUID). Contains an ordered list of clips. Each track type defines how it evaluates its clips against the target node.
- **Clip** -- A time range within a track. May contain keyframes for interpolated data or reference an external asset (animation, audio).
- **Keyframe** -- A time/value pair inside a clip. Keyframe times are in clip-local space.

## Runtime Flow

```
TimelinePlayer::Tick(deltaTime)
  |
  +-- Advance mCurrentTime by deltaTime * playRate
  +-- Handle looping (wrap) or stop at end
  +-- EvaluateTimeline(mCurrentTime)
        |
        +-- EnsureInstance()         -- create TimelineInstance if needed
        +-- ResolveBindings(world)   -- map track UUIDs to live Node pointers
        +-- For each track:
              +-- Skip if muted
              +-- track->Evaluate(time, resolvedNode, instance)
                    |
                    +-- Find clip where ContainsTime(time) is true
                    +-- Compute localTime = clip->GetLocalTime(time)
                    +-- Evaluate clip data at localTime
                    +-- Apply result to target node
```

### Key Classes

| Class | Header | Role |
|---|---|---|
| `Timeline` | `Engine/Assets/Timeline.h` | Asset holding tracks and playback settings |
| `TimelineTrack` | `Engine/Timeline/TimelineTrack.h` | Base track -- targets a node, holds clips |
| `TimelineClip` | `Engine/Timeline/TimelineClip.h` | Base clip -- time range, optional keyframes |
| `TimelineInstance` | `Engine/Timeline/TimelineInstance.h` | Runtime state (resolved nodes, pre-play snapshots) |
| `TimelinePlayer` | `Engine/Nodes/TimelinePlayer.h` | Node that drives playback of a Timeline asset |

## Node Binding

Each track stores a **target node UUID** (`mTargetNodeUuid`) and a **target node name** (display only). At play time, `TimelineInstance::ResolveBindings()` looks up each UUID in the current `World` to obtain the live `Node*`. If a node cannot be found, the track is skipped during evaluation.

## Timeline Asset Properties

| Property | Type | Default | Description |
|---|---|---|---|
| Duration | `float` | `5.0` | Total length in seconds |
| Play Rate | `float` | `1.0` | Global speed multiplier |
| Loop | `bool` | `false` | Wrap time back to 0 at the end |
| Auto Play | `bool` | `false` | Begin playback automatically when the scene starts |

## Built-in Track Types

| Track Type | Clip Type | Purpose |
|---|---|---|
| TransformTrack | TransformClip | Animate position, rotation, and scale of a Node3D |
| ScriptValueTrack | ScriptValueClip | Animate any script-exposed property on a node |
| FunctionCallTrack | FunctionCallClip | Fire Lua functions at specific times |
| ActivateTrack | ActivateClip | Toggle node active/visible state over a time range |
| AnimationTrack | AnimationClip | Drive skeletal animations on a SkeletalMesh3D |
| AudioTrack | AudioClip | Play sound waves on an Audio3D node |

See [TracksAndClips.md](TracksAndClips.md) for detailed documentation of each type.

## Further Reading

- [TimelinePlayer Node](TimelinePlayer.md)
- [Tracks and Clips Reference](TracksAndClips.md)
- [Lua Scripting API](LuaAPI.md)
- [Creating Custom Tracks (C++)](CreatingCustomTracks.md)
