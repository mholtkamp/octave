# Addon / Plugin System

## Overview

Octave has two plugin systems: a **RuntimePluginManager** for game/runtime plugins and a **NativeAddonManager** (editor-only) for editor extensions. Both share the same `EditorUIHooks` API for editor integration.

## Key Files

| File | Purpose |
|------|---------|
| `Engine/Source/Plugins/RuntimePluginManager.h/.cpp` | Runtime plugin lifecycle |
| `Engine/Source/Plugins/OctavePluginAPI.h` | Plugin descriptor struct |
| `Engine/Source/Plugins/OctaveEngineAPI.h` | Engine API exposed to plugins |
| `Engine/Source/Plugins/EditorUIHooks.h` | Editor UI extension API (~940 lines) |
| `Engine/Source/Editor/Addons/NativeAddonManager.cpp` | Editor addon hot-reload system |
| `Engine/Source/Editor/Addons/AddonManager.cpp` | Addon discovery and management |
| `Engine/Source/Editor/Addons/AddonCreator.cpp` | Addon project scaffolding |
| `Engine/Source/Editor/Addons/AddonsWindow.cpp` | Addons UI panel |
| `Plugins/` | External plugins (e.g., Blender addon) |

## Plugin Descriptor

```cpp
#define OCTAVE_PLUGIN_API_VERSION 2

struct OctavePluginDesc {
    uint32_t apiVersion;           // Must match OCTAVE_PLUGIN_API_VERSION
    const char* pluginName;
    const char* pluginVersion;     // e.g., "1.0.0"

    // Lifecycle
    int (*OnLoad)(OctaveEngineAPI* api);    // Return 0 on success
    void (*OnUnload)();

    // Per-frame
    void (*Tick)(float deltaTime);          // Game tick
    void (*TickEditor)(float deltaTime);    // Editor tick (EDITOR only)

    // Registration
    void (*RegisterTypes)(void* nodeFactory);
    void (*RegisterScriptFuncs)(lua_State* L);

    // Editor (EDITOR only, nullptr for game-only plugins)
    void (*RegisterEditorUI)(EditorUIHooks* hooks, uint64_t hookId);
    void (*OnEditorPreInit)();
    void (*OnEditorReady)();
};
```

## Plugin Registration

Static registration via macro:
```cpp
OCTAVE_REGISTER_PLUGIN(myPlugin, MyPlugin_GetDesc)
```

This creates a static registrar that calls `QueuePluginRegistration()` before `main()`. The `RuntimePluginManager` processes queued registrations during `Create()`.

## Plugin Lifecycle

1. **Static init**: `OCTAVE_REGISTER_PLUGIN` queues the plugin
2. **Create()**: RuntimePluginManager processes queued registrations
3. **Initialize()**: Calls `OnLoad()`, `RegisterTypes()`, `RegisterScriptFuncs()` for each plugin
4. **Main loop**: `TickAllPlugins(deltaTime)` calls `Tick()` (and `TickEditor()` in editor builds)
5. **Shutdown()**: Calls `OnUnload()` for each plugin

## Engine API (OctaveEngineAPI)

Exposed to plugins via `OctaveEngineAPI*` passed to `OnLoad()`:

**Logging**: `LogDebug()`, `LogWarning()`, `LogError()`

**Lua**: Full Lua C API wrappers — stack manipulation, type checking, table operations, metatables

**World**: `GetWorld(index)`, `GetNumWorlds()`

**Nodes**: `SpawnNode(world, typeName)`, `DestroyNode(node)`, `FindNode(world, name)`

**Node3D**: Get/Set Position, Rotation, Scale

**Assets**: `LoadAsset(name)`, `FetchAsset(name)`, `UnloadAsset(name)`

**Audio**: `PlaySound2D()`, `StopAllSounds()`, `SetMasterVolume()`

**Input**: Key, mouse button, position, delta, scroll queries

**Time**: `GetDeltaTime()`, `GetElapsedTime()`

**Editor UI**: `editorUI` pointer (nullptr in game builds)

## Editor UI Hooks (EditorUIHooks)

The `EditorUIHooks` struct provides function pointers for extending the editor. All hooks are keyed by a `HookId` (uint64) for cleanup.

**Categories:**

| Category | Key Functions | Purpose |
|----------|---------------|---------|
| Menus | `AddMenuItem()`, `AddTopLevelMenuItem()` | Extend menu bar and context menus |
| Windows | `RegisterWindow()`, `OpenWindow()`, `CloseWindow()` | Custom dockable panels |
| Inspectors | `RegisterInspector()` | Per-node-type custom property UI |
| Context Menus | `AddNodeContextItem()`, `AddAssetContextItem()`, `AddViewportContextItem()` | Right-click menus |
| Toolbar | `AddToolbarItem()` | Custom toolbar buttons |
| Node Menus | `AddNodeMenuItems()`, `AddSpawnBasic3dItems()` | Extend "Add Node" menus |
| Asset Creation | `AddCreateAssetItems()` | Extend "Create Asset" menu |
| Scene Types | `RegisterSceneType()` | Custom scene templates |
| Overlays | `RegisterViewportOverlay()` | Viewport overlays |
| Preferences | `RegisterPreferencesPanel()` | Custom settings pages |
| Shortcuts | `RegisterShortcut()` | Keyboard shortcuts |
| Property Drawers | `RegisterPropertyDrawer()` | Custom property UI |
| Hierarchy GUI | `RegisterHierarchyItemGUI()` | Per-row hierarchy overlay |
| Import | `RegisterAssetImporter()` | Custom file importers |
| Drag-Drop | `RegisterDragDropHandler()` | Custom drag-drop handling |
| Gizmo Tools | `RegisterGizmoTool()` | Custom transform tools |
| Play Targets | `AddPlayTarget()` | Custom launch targets |
| Preview | `AddGamePreviewResolution()` | Device resolution presets |

**Events:**
- Project: `RegisterOnProjectOpen/Close/Save()`
- Scene: `RegisterOnSceneOpen/Close()`
- Packaging: `RegisterOnPackageStarted/Finished()`, `RegisterOnPreBuild/PostBuild()`
- Editor: `RegisterOnSelectionChanged()`, `RegisterOnPlayModeChanged()`, `RegisterOnEditorShutdown()`, `RegisterOnEditorModeChanged()`
- Assets: `RegisterOnAssetImported/Deleted/Saved/Open()`
- Hierarchy: `RegisterOnHierarchyChanged()`

**Cleanup:** `RemoveAllHooks(hookId)` removes all hooks for a given addon.

## NativeAddonManager (Editor)

Located in `Engine/Source/Editor/Addons/`. Manages editor-side addon loading, including hot-reload support for DLL-based addons. Uses the `EditorUIHookManager` internally to register hooks.

## Example: Blender Addon

**Location:** `Plugins/Blender/octave-gameengine-connect/`

A Python Blender addon that exports glTF with Octave metadata (mesh type, asset reference, script, material type). Demonstrates external tooling integration rather than runtime plugin API usage.

## Documentation

User-facing docs:
- `Documentation/Info/Addons.md` — General addon info
- `Documentation/Development/NativeAddon/NativeAddon.md` — Native addon overview
- `Documentation/Development/NativeAddon/Examples/` — 30+ examples covering all hook types
