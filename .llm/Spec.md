# Octave Engine - LLM Codebase Specification

## Project Identity

**Octave Engine** is a multi-platform game engine with an ImGui-based editor, Vulkan rendering, Lua scripting, visual node graphs, and timeline animation. It targets Windows, Linux, Android, GameCube/Wii, and Nintendo 3DS.

**Repository root:** The directory containing `Octave.sln`, `CMakeLists.txt`, and `Engine/`.

## Directory Map

```
octave/
├── Engine/                    # Main engine library (vcxproj + CMake)
│   ├── Engine.vcxproj         # Visual Studio project
│   ├── Engine.vcxproj.filters # VS filter groups
│   ├── CMakeLists.txt         # CMake build
│   └── Source/
│       ├── Audio/             # Platform-specific audio (XAudio2, ALSA, DSP)
│       ├── Editor/            # Editor UI (#if EDITOR) — see Editor.md
│       ├── Engine/            # Core engine classes — see Architecture.md
│       │   ├── Assets/        # Asset type implementations — see AssetSystem.md
│       │   ├── Nodes/         # Node types (3D, Widgets) — see NodeSystem.md
│       │   ├── NodeGraph/     # Visual scripting — see NodeGraph.md
│       │   └── Timeline/      # Animation system — see Timeline.md
│       ├── Graphics/          # Rendering backends — see Graphics.md
│       │   ├── Vulkan/        # Primary backend (Windows/Linux)
│       │   ├── GX/            # GameCube/Wii
│       │   └── C3D/           # Nintendo 3DS
│       ├── Input/             # Platform-specific input
│       ├── LuaBindings/       # Lua binding files (*_Lua.h/cpp) — see Scripting.md
│       ├── Network/           # Platform-specific networking
│       ├── Plugins/           # Runtime plugin API — see Addons.md
│       └── System/            # Platform abstraction layer — see Platforms.md
├── External/                  # Third-party: imgui, zep, bullet, imgui-node-editor
├── Plugins/                   # External plugins (e.g., Blender addon)
├── Standalone/                # Standalone launcher (Source/Main.cpp)
├── Template/                  # New project template
├── Tools/                     # Build scripts, Lua stub generator
├── Documentation/             # MkDocs site (see mkdocs.yml)
├── Octave.sln                 # Visual Studio solution
├── CMakeLists.txt             # Root CMake
└── mkdocs.yml                 # Documentation nav structure
```

## Subsystem Index

| Subsystem | Detail File | Key Directory | Summary |
|-----------|-------------|---------------|---------|
| Core Architecture | [Architecture.md](Architecture.md) | `Engine/Source/Engine/` | Object/RTTI system, factory pattern, property reflection, singletons, logging |
| Node Hierarchy | [NodeSystem.md](NodeSystem.md) | `Engine/Source/Engine/Nodes/` | Node3D, Primitives, Lights, Cameras, Widgets, World management |
| Node Graph | [NodeGraph.md](NodeGraph.md) | `Engine/Source/Engine/NodeGraph/` | Visual scripting: domains, graph processor, pins, links, functions |
| Editor | [Editor.md](Editor.md) | `Engine/Source/Editor/` | ImGui panels, undo/redo, preferences, viewport, UI hooks |
| Graphics | [Graphics.md](Graphics.md) | `Engine/Source/Graphics/` | Vulkan pipeline, GX/C3D backends, materials, renderer |
| Scripting | [Scripting.md](Scripting.md) | `Engine/Source/LuaBindings/` | Lua integration, binding macros, auto-registration, stub generator |
| Asset System | [AssetSystem.md](AssetSystem.md) | `Engine/Source/Engine/Assets/` | Versioned serialization, UUID refs, async loading, 14 asset types |
| Platforms | [Platforms.md](Platforms.md) | `Engine/Source/System/` | System API, input, audio, networking, platform abstractions |
| Timeline | [Timeline.md](Timeline.md) | `Engine/Source/Engine/Timeline/` | Keyframe animation: tracks, clips, interpolation, TimelinePlayer |
| Addons | [Addons.md](Addons.md) | `Engine/Source/Plugins/` | Runtime plugin API, editor hooks, native addon system |

## Key Entry Points

| Entry Point | File | Purpose |
|-------------|------|---------|
| Application main | `Standalone/Source/Main.cpp` | Calls `Initialize()`, runs main loop |
| Engine init | `Engine/Source/Engine/Engine.h` | `Initialize()` / `Update()` / `Shutdown()` |
| Editor init | `Engine/Source/Editor/EditorImgui.h` | `EditorImguiInit()` / `EditorImguiDraw()` |
| Lifecycle hooks | `Engine/Source/Engine/Engine.h` | `OctPreInitialize()`, `OctPostInitialize()`, `OctPreUpdate()`, etc. |

## Singletons & Global Accessors

| Accessor | Header | Returns |
|----------|--------|---------|
| `GetEngineState()` | `Engine.h` | `EngineState&` — frame number, delta time, window size |
| `GetEngineConfig()` | `Engine.h` | `EngineConfig&` — read-only configuration |
| `GetEditorState()` | `EditorState.h` | `EditorState*` — selection, mode, camera (EDITOR only) |
| `Renderer::Get()` | `Renderer.h` | `Renderer*` — rendering orchestration |
| `AssetManager::Get()` | `AssetManager.h` | `AssetManager*` — asset registry and loading |
| `PreferencesManager::Get()` | `PreferencesManager.h` | `PreferencesManager*` — editor settings |
| `GetAppClock()` | `Engine.h` | `Clock*` — `GetTime()` returns float seconds |

## Naming Conventions

| Element | Convention | Example |
|---------|-----------|---------|
| Classes | PascalCase | `Box3D`, `AssetManager` |
| Member variables | `m` prefix | `mExtents`, `mChildren` |
| Static variables | `s` prefix | `sInstance`, `sClock` |
| Constants | `k` prefix or UPPER_SNAKE | `kSidePaneWidth`, `INVALID_NODE_ID` |
| Functions | PascalCase (public) | `GetName()`, `SetActive()` |
| Files | PascalCase | `Box3D.h`, `EditorImgui.cpp` |
| Lua bindings | `*_Lua.h/cpp` | `Box3d_Lua.cpp` |
| Macros | UPPER_CASE | `DECLARE_NODE`, `EDITOR` |

## Important Macros

| Macro | Purpose |
|-------|---------|
| `EDITOR` (0/1) | Guards editor-only code |
| `PLATFORM_WINDOWS`, `PLATFORM_LINUX`, `PLATFORM_ANDROID`, `PLATFORM_DOLPHIN`, `PLATFORM_3DS` | Platform selection |
| `API_VULKAN`, `API_GX`, `API_C3D` | Graphics backend selection |
| `LOGGING_ENABLED` | Logging system toggle |
| `DECLARE_NODE(Class, Parent)` / `DEFINE_NODE(Class, Parent)` | Node RTTI + factory registration |
| `DECLARE_ASSET(Class, Parent)` / `DEFINE_ASSET(Class)` | Asset RTTI + factory registration |
| `DECLARE_GRAPH_NODE(Class, Parent)` / `DEFINE_GRAPH_NODE(Class)` | Graph node registration |
| `DECLARE_SCRIPT_LINK(Type, Parent, Top)` | Lua auto-registration |

## Build Configurations

**Visual Studio** (`Engine.vcxproj`): Debug, DebugEditor, Release, ReleaseEditor, ReleaseSteam — for Win32, x64, Android-arm64-v8a.

**CMake** (`CMakeLists.txt`): Cross-platform support.

Editor builds define `EDITOR=1`; game builds define `EDITOR=0`.

## Asset Versioning

Current: `ASSET_VERSION_CURRENT = 16`. Key milestones: v12 (UUID support), v14 (persistent node UUIDs), v16 (node graph functions). See [AssetSystem.md](AssetSystem.md).

## Maintenance

See [HowToKeepUpToDate.md](HowToKeepUpToDate.md) for instructions on maintaining these docs.
