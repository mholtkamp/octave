# TimelinePlayer Node

`TimelinePlayer` is a `Node` subclass that drives playback of a `Timeline` asset. Add one to your scene, assign a Timeline, and it handles time advancement, track evaluation, and lifecycle management.

**Source:** `Engine/Source/Engine/Nodes/TimelinePlayer.h`, `TimelinePlayer.cpp`

## Editor Properties

| Property | Type | Default | Description |
|---|---|---|---|
| Timeline | `TimelineRef` | `nullptr` | The Timeline asset to play |
| Play On Start | `bool` | `false` | Automatically call `Play()` when the node starts |

## C++ API

### Playback Control

```cpp
void Play();
```
Start (or resume) playback from the current time position.

```cpp
void Pause();
```
Pause playback. Time stops advancing but the current position is preserved.

```cpp
void StopPlayback();
```
Stop playback and reset time to 0. Calls `Reset()` on all tracks to restore nodes to their pre-play state.

```cpp
bool IsPlaying() const;
```
Returns `true` if the timeline is playing and **not** paused.

```cpp
bool IsPaused() const;
```
Returns `true` if playback is paused.

### Time

```cpp
void SetTime(float time);
```
Seek to the given time position (in seconds).

```cpp
float GetTime() const;
```
Returns the current playback time in seconds.

```cpp
float GetDuration() const;
```
Returns the duration of the assigned Timeline asset (or 0 if none).

### Asset

```cpp
void SetTimeline(Timeline* timeline);
```
Assign a Timeline asset. Pass `nullptr` to clear.

```cpp
Timeline* GetTimeline() const;
```
Returns the currently assigned Timeline asset.

```cpp
TimelineRef GetTimelineRef() const;
```
Returns the asset reference (for serialization).

### Settings

```cpp
void SetPlayOnStart(bool playOnStart);
```
Enable or disable auto-play on scene start.

```cpp
bool GetPlayOnStart() const;
```
Returns the auto-play setting.

### Progress

```cpp
float GetProgress() const;
```
Returns the playback progress as a value from 0.0 to 1.0 (currentTime / duration).

### Signals

TimelinePlayer emits the following signals that scripts can connect to:

| Signal | Emitted When |
|--------|--------------|
| `OnStarted` | `Play()` is called |
| `OnFinished` | Timeline reaches the end (non-looping only) |
| `OnStopped` | `StopPlayback()` is called |
| `OnStateChanged` | Any playback state change |

Scripts can also define callback functions with the same names (e.g., `OnFinished()`) which will be called automatically.

### Evaluation

```cpp
void EvaluateTimeline(float time);
```
Evaluate all tracks at the given time. Called internally by `Tick()`, but can be called manually for scrubbing.

```cpp
void EnsureInstance();
```
Create the internal `TimelineInstance` if it does not exist. Called automatically before evaluation.

```cpp
TimelineInstance* GetInstance();
```
Access the internal instance (used by the editor for preview).

## Lifecycle

1. **Create** -- Node is instantiated. No playback occurs.
2. **Start** -- If `mPlayOnStart` is `true`, `Play()` is called automatically.
3. **Tick** -- Each frame while playing:
   - Advance `mCurrentTime` by `deltaTime * timeline->GetPlayRate()`.
   - If looping is enabled and time exceeds duration, wrap back to 0.
   - If not looping and time exceeds duration, stop playback.
   - Call `EvaluateTimeline(mCurrentTime)`.
4. **Stop** -- `StopPlayback()` resets time to 0 and calls `Reset()` on every track, which restores target nodes to their pre-play state via the `TimelineInstance` snapshot.

## Time Advancement

```
newTime = mCurrentTime + deltaTime * timeline->GetPlayRate()

if looping:
    newTime = fmod(newTime, duration)   // wrap
else if newTime >= duration:
    StopPlayback()                       // stop at end
```

The `PlayRate` on the Timeline asset acts as a global speed multiplier. Individual clips also have a `Speed` property that affects their local time calculation independently.

## Member Variables

| Variable | Type | Default | Description |
|---|---|---|---|
| `mTimeline` | `TimelineRef` | -- | Asset reference |
| `mInstance` | `TimelineInstance*` | `nullptr` | Runtime evaluation state |
| `mCurrentTime` | `float` | `0.0f` | Current playback position |
| `mPlaying` | `bool` | `false` | Whether playback is active |
| `mPaused` | `bool` | `false` | Whether playback is paused |
| `mPlayOnStart` | `bool` | `false` | Auto-play flag |
