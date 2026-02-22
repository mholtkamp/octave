# Editor System

## Overview

The editor is built on ImGui with a docking layout. All editor code is guarded by `#if EDITOR`. The main rendering happens in `EditorImgui.cpp` (~7600 lines), with specialized panels in subdirectories.

## Key Files

All paths relative to `Engine/Source/Editor/`:

| File | Purpose |
|------|---------|
| `EditorImgui.h/.cpp` | Main UI rendering: `EditorImguiInit()`, `EditorImguiDraw()` |
| `EditorState.h/.cpp` | Global editor state singleton (`GetEditorState()`) |
| `ActionManager.h/.cpp` | Undo/redo system |
| `EditorUIHookManager.h/.cpp` | Plugin extension hooks |
| `EditorTypes.h` | Enums: ControlMode, TransformLock, EditorMode, PaintMode |
| `EditorConstants.h` | Drag-drop types, basic node names |
| `EditorUtils.h/.cpp` | Editor utility functions |
| `InputManager.h/.cpp` | Hotkey handling |
| `Viewport3d.h/.cpp` | 3D viewport camera (pilot, orbit, pan) |
| `Viewport2d.h/.cpp` | 2D widget viewport |
| `PaintManager.h/.cpp` | Mesh instance/color painting |
| `Grid.h/.cpp` | Grid rendering |
| `CustomImgui.h/.cpp` | ImGui extensions |

## Major Panels

Rendered in `EditorImguiDraw()` via docked ImGui windows:

| Panel | Location | Purpose |
|-------|----------|---------|
| **Viewport** | `EditorImgui.cpp` | 3D/2D scene view with gizmos |
| **Scene Hierarchy** | `EditorImgui.cpp` (`DrawScenePanel()`) | Node tree, drag-drop, context menus |
| **Properties** | `EditorImgui.cpp` (`DrawPropertiesPanel()`) | Inspector for selected node/asset |
| **Assets** | `EditorImgui.cpp` | Asset browser (Project/Addons tabs) |
| **Debug Log** | `DebugLog/DebugLogWindow.h/.cpp` | Log viewer with severity filtering |
| **Scripts** | `EditorImgui.cpp` (`DrawScriptsPanel()`) | Script browser |
| **Script Editor** | `ScriptEditor/ScriptEditorWindow.h/.cpp` | Zep-based code editor |
| **Node Graph** | `NodeGraph/NodeGraphPanel.h/.cpp` | Visual scripting editor (Ctrl+C/V/D, export/import) |
| **Timeline** | `Timeline/TimelinePanel.h/.cpp` | Animation timeline editor |
| **Game Preview** | `GamePreview/GamePreview.h/.cpp` | Device resolution preview |
| **3DS Preview** | `EditorImgui.cpp` | 3DS hardware preview |
| **Theme Editor** | `ThemeEditor/ThemeEditorWindow.h/.cpp` | Theme customization |
| **Packaging** | `Packaging/PackagingWindow.h/.cpp` | Build & deploy system |

## EditorState

**Singleton accessor:** `GetEditorState()`

Key state groups:
- **Selection**: `mSelectedNodes`, `mSelectedAssetStub`, `mInspectedObject`
- **Mode**: `mMode` (Scene/Scene2D/Scene3D), `mControlMode` (Default/Pilot/Translate/Rotate/Scale/Pan/Orbit)
- **Camera**: `mEditorCamera`, perspective/ortho settings
- **Viewport**: `mViewportX/Y/Width/Height`, pane visibility toggles
- **Play In Editor**: `mPlayInEditor`, `mPaused`, `mEjected`
- **Transform**: `mTransformLock` (axis/plane locking)
- **Gizmo**: `mGizmoOperation` (Translate/Rotate/Scale), `mGizmoMode` (World/Local)
- **Paint**: `mPaintMode` (None/Color/Instance)
- **Timeline**: playhead time, zoom, scroll, selection indices
- **Asset Browser**: current directory per tab, filter strings, navigation history

## ActionManager (Undo/Redo)

**Pattern:** Every undoable operation creates an `Action` subclass with `Execute()` and `Reverse()` methods, pushed to the history stack.

Key high-level methods (all `EXE_*` prefixed):
- `EXE_SpawnNode()`, `EXE_DeleteNodes()`, `EXE_AttachNode()`
- `EXE_EditTransform()`, `EXE_SetWorldPosition/Rotation/Scale()`
- `EXE_EditProperty()` — generic property editing
- `EXE_TimelineAddTrack/RemoveTrack/AddClip/RemoveClip()`

Also: `CreateNewProject()`, `OpenProject()`, `SaveScene()`, `ImportAsset()`, `BuildData()`, `DuplicateNodes()`.

## EditorUIHookManager (Extension System)

Enables addons/plugins to extend the editor UI. Categories of hooks:

- **Menus**: `RegisteredMenuItem`, top-level menus, context menus
- **Windows**: Custom dockable windows
- **Inspectors**: Per-node-type custom property UI
- **Toolbar**: Custom toolbar items
- **Node/Asset Creation**: Extend "Add Node" and "Create Asset" menus
- **Scene Types**: Custom scene creation templates
- **Viewport Overlays**: Render over the viewport
- **Preferences Panels**: Custom settings pages
- **Shortcuts**: Keyboard shortcuts with key parsing
- **Property Drawers**: Custom UI for specific property types
- **Drag-Drop & Import**: Custom handlers and importers
- **Gizmo Tools**: Custom transform tools
- **Play Targets**: Custom launch targets (emulators, devices)

Events: `FireOnProjectOpen/Close()`, `FireOnSceneOpen/Close()`, `FireOnSelectionChanged()`, `FireOnPlayModeChanged()`, `FireOnAssetImported/Deleted/Saved()`, etc.

Cleanup: `RemoveAllHooks(hookId)` removes all hooks for a given addon.

## Preferences System

**Files:** `Preferences/PreferencesManager.h/.cpp`, `Preferences/PreferencesModule.h/.cpp`

Hierarchical module system. Each module: `Render()`, `LoadSettings()`, `SaveSettings()`.

Modules: General (autosave, debug flags), Appearance > Theme (CSS themes, font) + Viewport, External > Editors + Launchers, Packaging > Docker.

`PreferencesManager::Get()->RegisterModule(module)`, `LoadAllSettings()`, `SaveAllSettings()`.

## Packaging System

**Files:** `Packaging/PackagingWindow.h/.cpp`, `PackagingSettings.h`, `BuildProfile.h`

Multi-platform build system with build profiles. Supports Docker for console builds. Targets: Windows, Linux, GameCube, Wii, 3DS. Features: asset embedding, emulator launching, 3dslink support.

## ImGuizmo Integration

**File:** `ImGuizmo/ImGuizmo.cpp`

3D/2D transform gizmos: Translate, Rotate, Scale in World/Local space. Configured via `EditorState::mGizmoOperation` and `mGizmoMode`.

## Key Constants

```cpp
// EditorConstants.h
DRAGDROP_NODE "DND_NODE"
DRAGDROP_ASSET "DND_ASSET"
BASIC_STATIC_MESH "Static Mesh"
BASIC_BOX "Box"
// etc.

// EditorTypes.h
enum class ControlMode { Default, Pilot, Translate, Rotate, Scale, Pan, Orbit };
enum class EditorMode { Scene, Scene2D, Scene3D };
enum class PaintMode { None, Color, Instance };
```

## Editor Subdirectories

| Directory | Contents |
|-----------|----------|
| `Addons/` | AddonManager, AddonCreator, NativeAddonManager, AddonsWindow |
| `DebugLog/` | DebugLogWindow (bottom panel log viewer) |
| `GamePreview/` | Device resolution preview |
| `ImGuizmo/` | Transform gizmo library |
| `NodeGraph/` | Node graph editor panel |
| `Packaging/` | Build profiles and deployment |
| `Preferences/` | Settings hierarchy (General, Appearance, External, Packaging) |
| `ProjectSelect/` | Project creation and selection |
| `ScriptCreator/` | Lua/C++ script creation dialogs |
| `ScriptEditor/` | Zep-based code editor |
| `SecondScreenPreview/` | Second display preview |
| `ThemeEditor/` | Theme editor with CSS export |
| `Timeline/` | Timeline animation editor |
