# Claude Prompt: Implement Timeline System (Unity Parity) for Octave

You are a senior C++ engine/editor engineer. You are working inside the **Octave Game Engine** (cross-platform: GameCube, Wii, 3DS, Windows, Linux, Android).

---

## Goal

Design and plan a **full Timeline system** with parity to **Unity Timeline**, integrated into Octave’s Editor and runtime playback.

We need:

* A **Timeline Asset** (or equivalent) that contains a list of **Tracks**.
* Tracks can be added, removed, reordered, and each Track can be **bound to a scene object** (Node / Node3D / Audio3D / SkeletalMesh, etc.).
* Tracks contain **Clips** that can be moved on a time ruler, snapped, trimmed, cut (in/out), duplicated, and dragged between tracks.
* Clips have **properties** (editable in the Properties panel) and can hold **references** (assets, scene nodes, etc.).
* Support **overlapping clips** with **fade / overlap handles** (at minimum for audio; ideally also animation and value blending).
* Provide **built-in track types** and a **plugin/extensibility layer** so developers can create **new Track types and Clip types** using Octave’s **Native C++ System**.
* Needs to feel modern.
* Needs theming hooks.

---

## Required Built‑in Track Types

### 1. Transform Track

* Animates a bound scene object’s transform (position / rotation / scale).
* Clips contain keyframes or curves (engine-appropriate representation).
* Overlapping clips should blend when possible.

### 2. Script Value Track

* Animates or drives a script-exposed property on a bound scene object.
* Supports continuous values (curves) and/or discrete triggers/events.
* Integrates with Octave’s property and scripting systems.
* Needs to expose public properties on the node and it's script to edit just like the normal inspector does.


### 3. Audio Track

* Clips reference an audio asset.
* Supports move, trim, cut (in/out), looping (optional).
* Allows selecting which object emits the audio (e.g., Audio3D node or bound node).
* Supports overlapping clips with fade-in / fade-out handles (crossfading).

### 4. Animation Track (Skeletal Mesh)

* Bound to a **Skeletal Mesh** object.
* Clips select which animation to play on the SkeletalMesh.
* Supports trimming, cutting, and repositioning.
* Overlapping clips should blend (animation blending in/out).

### 5. Activate Track

* Handle the active and visible state of a node/object.
* When a Clip exists, that means the node/object is visible and active.

---

## Extensibility (Native C++ System)

The Timeline system must expose hooks so developers can:

* Register **custom Track types** in C++.
* Register **custom Clip types** with their own data and evaluation logic.
* Expose clip/track properties to the Editor Inspector.
* Execute custom evaluation logic during Timeline playback.
* Serialize and deserialize custom tracks/clips with scenes or assets.

This should integrate cleanly with the existing **Native C++ System** and not require engine modification for each new track type.

We need to create an example for a Custom Track and Custom Clip type at `Documentation\Development\NativeAddon\Examples\CustomAnimationTrack.md` 

---

## Node & Object Referencing (Critical Requirement)

All scene object references used by the Timeline system **must use stable Node IDs, not names**.

Requirements:

* Every scene node has a persistent `.id` (UUID) assigned at creation time.
* Needs to be backwards compatible if possible, so if a node doesn't have a `.id` applied, auto generate then save the scene, then send log messages about what was updated.
* Timeline Tracks and Clips **store only the Node `.id`** for bindings and references.
* Node names, mesh names, or hierarchy paths **must never be relied on** for Timeline bindings.
* Renaming a node, mesh, or Skeletal Mesh must **not break Timeline references**.
* Timeline evaluation must resolve the Node at runtime via the Scene’s Node-ID lookup system.
* Missing or deleted nodes must fail gracefully (no crashes, clear editor warning state).

This applies to:

* Track bindings to scene objects
* Audio emitter targets
* Skeletal Mesh animation targets
* Script value/property targets
* Any custom Track or Clip created via the Native C++ System

Custom Track and Clip APIs must therefore expose:

* `GetBoundNodeId()` / `SetBoundNodeId(UUID)` or equivalent
* Runtime resolution hooks (e.g. `ResolveNode(Scene&)`)

---

## Deliverables

### 1. Codebase Discovery Notes

Identify and document:

* Existing animation playback system (especially Skeletal Mesh animation).
* Existing audio playback and Audio node systems.
* How Editor property inspection/editing is implemented.
* How assets are represented and serialized.
* How undo/redo is implemented (if present).
* Best place to add Timeline (Engine module vs Editor module vs hybrid).

---

### 2. Proposed Architecture

Cover the following:

#### Core Runtime

* TimelineAsset
* TimelinePlayer / TimelineInstance
* Track base class
* Clip base class
* Track–scene-object binding model

#### Editor

* Timeline Editor window/panel
* Track list UI + time ruler
* Clip UI model (selection, drag, trim, overlap)
* Undo/redo commands for Timeline edits

#### Evaluation Model

* Playhead time handling
* Scrubbing and seeking
* Paused evaluation (editor preview)
* Runtime playback
* Sampling model (per-frame vs event-based)

#### Blending Model

* Audio fades and crossfades
* Animation blend weights
* Script/value interpolation

#### Clip Data Model

* Start time
* Duration
* In / Out offsets
* Time scaling
* Looping (if applicable)

---

### 3. Serialization Format

Decide and document:

* Whether Timeline data is stored as:

  * A standalone Timeline Asset, or
  * Embedded in Scene data
* How bindings to scene objects are stored.
* Versioning and backward compatibility strategy.

---

### 4. Editor UX Plan

Describe:

* Timeline layout (tracks, ruler, clips).
* Clip operations: drag, trim, cut/split, duplicate, multi-select.
* Snapping behavior.
* Overlap UI with fade handles.
* Binding UI (assign scene object to track).
* Inspector integration for Track and Clip properties.

---

### 5. Phased Implementation Plan

Suggested phases:

**Phase 0** – Minimal Timeline player + basic Track/Clip system

**Phase 1** – Timeline Editor UI + clip manipulation + serialization

**Phase 2** – Audio Track + fades and crossfades

**Phase 3** – Skeletal Mesh Animation Track + blending

**Phase 4** – Script Value Track

**Phase 5** – Native C++ System extensibility, documentation, and examples

---

### 6. Risks and Platform Constraints

Call out:

* Cross-platform constraints (console vs desktop).
* Floating-point precision and long timelines.
* Performance considerations.
* Undo/redo complexity.
* Animation blending edge cases.
* Audio crossfade implementation risks.

---

## First Step (Required)

Before proposing architecture, scan the Octave codebase and report:

* Where Skeletal Mesh animation playback lives.
* Where audio playback and Audio nodes are implemented.
* How Editor property inspection works.
* How assets and scenes are serialized.
* How undo/redo is handled.

Then proceed with the full architecture and phased implementation plan.
