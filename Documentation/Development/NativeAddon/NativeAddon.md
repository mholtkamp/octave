# Native Addon Development Guide

This guide explains how to create C++ native addons for the Octave Engine. Native addons allow you to extend the engine with custom C++ code that can be hot-loaded in the editor and compiled into final builds.

---

## Overview

A native addon is an extension of the standard addon format that includes C++ source code. Native addons provide:

- **Hot-loading in Editor**: Compile and reload your C++ code without restarting the editor (Windows/Linux)
- **Final Build Integration**: Your C++ code is compiled directly into console builds (GameCube, Wii, 3DS)
- **Full Engine Access**: Register custom node types, expose Lua functions, and integrate with engine systems

---

## Addon Structure

A native addon has the following directory structure:

```
MyAddon/
    package.json          # Required: Addon metadata
    thumbnail.png         # Optional: Preview image (256x256 recommended)
    Assets/               # Optional: Asset files (textures, meshes, etc.)
    Scripts/              # Optional: Lua scripts
    Source/               # Required for native: C++ source code
        MyAddon.cpp       # Main plugin entry point
        include/          # Optional: Header files
        src/              # Optional: Additional source files
```

---

## package.json Format

The `package.json` file must include a `native` block to enable native code support:

```json
{
    "name": "My Native Addon",
    "author": "Your Name",
    "description": "Description of what your addon does.",
    "version": "1.0.0",
    "url": "https://github.com/yourname/myaddon",
    "updated": "2024-01-15",
    "tags": ["gameplay", "tools"],
    "native": {
        "sourceDir": "Source",
        "binaryName": "myaddon",
        "entrySymbol": "OctavePlugin_GetDesc",
        "apiVersion": 2
    }
}
```

### Native Block Fields

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `target` | string | `"engine"` | Where the code runs (see below) |
| `sourceDir` | string | `"Source"` | Relative path to C++ source directory |
| `binaryName` | string | Required | Output binary name (without .dll/.so extension) |
| `entrySymbol` | string | `"OctavePlugin_GetDesc"` | Export function name |
| `apiVersion` | integer | `1` | Plugin API version for compatibility checking |

### Target Values

| Value | Hot-Load in Editor | Compiled into Final Builds | Use Case |
|-------|-------------------|---------------------------|----------|
| `"engine"` | Yes | Yes | Gameplay code, custom nodes, systems |
| `"editor"` | Yes | **No** | Debug tools, editor extensions, dev utilities |

**Example - Engine addon (default):**
```json
{
    "native": {
        "target": "engine",
        "binaryName": "mygameplay"
    }
}
```

**Example - Editor-only addon:**
```json
{
    "native": {
        "target": "editor",
        "binaryName": "mydebugtool"
    }
}
```

---

## Plugin Entry Point

Every native addon must implement a single exported function that returns a plugin descriptor:

```cpp
// MyAddon.cpp

#include "OctavePluginAPI.h"

// Static registration for built games (only when compiled-in, not DLL)
#if !defined(OCTAVE_PLUGIN_EXPORT)
#include "Plugins/RuntimePluginManager.h"
OCTAVE_REGISTER_PLUGIN(MyAddon, OctavePlugin_GetDesc)
#endif

static OctaveEngineAPI* sAPI = nullptr;

// Called when plugin is loaded
static int OnLoad(OctaveEngineAPI* api)
{
    sAPI = api;
    api->LogDebug("MyAddon: Loaded successfully!");

    // Perform initialization here
    // - Register custom types
    // - Set up resources
    // - Hook into engine systems

    return 0;  // Return 0 for success, non-zero for failure
}

// Called when plugin is about to be unloaded
static void OnUnload()
{
    // Perform cleanup here
    // - Unregister types
    // - Release resources
    // - Remove hooks

    if (sAPI)
    {
        sAPI->LogDebug("MyAddon: Unloading...");
    }
    sAPI = nullptr;

    // IMPORTANT: After this returns, your code will be unloaded!
    // Ensure no dangling pointers or callbacks remain.
}

// Optional: Called every frame during gameplay (PIE or built game)
static void Tick(float deltaTime)
{
    // Gameplay logic, physics, AI, etc.
}

// Optional: Called every frame in editor (regardless of play state)
static void TickEditor(float deltaTime)
{
    // Editor tools, visualization, debug overlays
}

// Optional: Register custom node types
static void RegisterTypes(void* nodeFactory)
{
    // Cast and use the node factory to register types
    // NodeFactory* factory = static_cast<NodeFactory*>(nodeFactory);
    // factory->RegisterType<MyCustomNode>();
}

// Optional: Register Lua script functions
static void RegisterScriptFuncs(void* luaState)
{
    // lua_State* L = static_cast<lua_State*>(luaState);
    // Register your Lua bindings here
}

// Required: Plugin descriptor export
extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "My Addon";
    desc->pluginVersion = "1.0.0";

    // Lifecycle callbacks
    desc->OnLoad = OnLoad;
    desc->OnUnload = OnUnload;

    // Tick callbacks (set to nullptr if not used)
    desc->Tick = Tick;               // Gameplay only
    desc->TickEditor = TickEditor;   // Editor always

    // Registration callbacks (set to nullptr if not used)
    desc->RegisterTypes = RegisterTypes;
    desc->RegisterScriptFuncs = RegisterScriptFuncs;
    desc->RegisterEditorUI = nullptr;  // See "Extending the Editor UI" section

    // Editor lifecycle hooks (API v2+, set to nullptr if not needed)
    desc->OnEditorPreInit = nullptr;   // Before editor ImGui is fully initialized
    desc->OnEditorReady = nullptr;     // After editor is fully initialized

    return 0;  // Success
}
```

---

## Plugin API Reference

### OctavePluginDesc

The descriptor structure you must fill in:

```cpp
struct OctavePluginDesc
{
    uint32_t apiVersion;           // Must match OCTAVE_PLUGIN_API_VERSION
    const char* pluginName;        // Display name
    const char* pluginVersion;     // Version string (e.g., "1.0.0")

    // Lifecycle callbacks
    int (*OnLoad)(OctaveEngineAPI* api);   // Called after loading
    void (*OnUnload)();                     // Called before unloading

    // Tick callbacks (set to nullptr if not used)
    void (*Tick)(float deltaTime);         // Called during gameplay (PIE or built game)
    void (*TickEditor)(float deltaTime);   // Called in editor regardless of play state

    // Registration callbacks
    void (*RegisterTypes)(void* nodeFactory);
    void (*RegisterScriptFuncs)(void* luaState);

    // Editor UI extension (editor builds only, set to nullptr otherwise)
    void (*RegisterEditorUI)(EditorUIHooks* hooks, uint64_t hookId);

    // Editor lifecycle callbacks (API version 2+, editor builds only)
    void (*OnEditorPreInit)();   // Called before editor ImGui is fully initialized
    void (*OnEditorReady)();     // Called after editor is fully initialized, before main loop
};
```

> **API Version Note:** The `OnEditorPreInit` and `OnEditorReady` fields were added in API version 2. Plugins built with API version 1 are still compatible - the engine zeros out these fields automatically.

### Tick Callbacks

Native addons can receive frame updates through two separate tick callbacks:

| Callback | When Called | Use Case |
|----------|-------------|----------|
| `Tick` | During gameplay only (PIE or built game) | Gameplay logic, physics, AI |
| `TickEditor` | Every frame in editor (regardless of play state) | Editor tools, visualization, debug overlays |

**Important:** In editor builds, `TickEditor` is always called. `Tick` is only called when Play-In-Editor is active. In built games, only `Tick` is called.

**Example: Rotator addon that only rotates during gameplay:**
```cpp
static Node3D* sTargetNode = nullptr;
static float sRotationSpeed = 45.0f;  // degrees per second
static OctaveEngineAPI* sAPI = nullptr;

static void Tick(float deltaTime)
{
    if (sTargetNode && sAPI)
    {
        sAPI->Node3D_AddRotation(sTargetNode, 0.0f, sRotationSpeed * deltaTime, 0.0f);
    }
}

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Rotator";
    desc->pluginVersion = "1.0.0";
    desc->OnLoad = OnLoad;
    desc->OnUnload = OnUnload;
    desc->Tick = Tick;           // Called during gameplay only
    desc->TickEditor = nullptr;  // No editor tick needed
    // ...
    return 0;
}
```

### OctaveEngineAPI

The engine API provides access to core functionality:

```cpp
struct OctaveEngineAPI
{
    // ===== Logging =====
    void (*LogDebug)(const char* fmt, ...);
    void (*LogWarning)(const char* fmt, ...);
    void (*LogError)(const char* fmt, ...);

    // ===== Lua Access =====
    lua_State* (*GetLua)();

    // ===== Lua Wrappers =====
    // These wrap Lua C API functions so plugins don't need to link against Lua.
    // Use sEngineAPI->Lua_pushnumber() instead of lua_pushnumber()
    void (*Lua_settop)(lua_State* L, int idx);
    void (*Lua_pushvalue)(lua_State* L, int idx);
    void (*Lua_pop)(lua_State* L, int n);
    int (*Lua_gettop)(lua_State* L);
    int (*Lua_type)(lua_State* L, int idx);
    int (*Lua_isfunction)(lua_State* L, int idx);
    int (*Lua_istable)(lua_State* L, int idx);
    int (*Lua_toboolean)(lua_State* L, int idx);
    double (*Lua_tonumber)(lua_State* L, int idx);
    const char* (*Lua_tostring)(lua_State* L, int idx);
    void* (*Lua_touserdata)(lua_State* L, int idx);
    void (*Lua_pushnil)(lua_State* L);
    void (*Lua_pushboolean)(lua_State* L, int b);
    void (*Lua_pushnumber)(lua_State* L, double n);
    void (*Lua_pushstring)(lua_State* L, const char* s);
    void (*Lua_createtable)(lua_State* L, int narr, int nrec);
    void (*Lua_setfield)(lua_State* L, int idx, const char* k);
    void (*Lua_getfield)(lua_State* L, int idx, const char* k);
    void (*Lua_setglobal)(lua_State* L, const char* name);
    void (*Lua_getglobal)(lua_State* L, const char* name);
    // ... and more Lua wrappers (see OctaveEngineAPI.h for full list)

    // ===== World Management =====
    World* (*GetWorld)(int32_t index);
    int32_t (*GetNumWorlds)();

    // ===== Node Operations =====
    Node* (*SpawnNode)(World* world, const char* typeName);
    void (*DestroyNode)(Node* node);
    Node* (*FindNode)(World* world, const char* name);

    // ===== Node3D Operations =====
    void (*Node3D_GetRotation)(Node3D* node, float* outX, float* outY, float* outZ);
    void (*Node3D_SetRotation)(Node3D* node, float x, float y, float z);
    void (*Node3D_AddRotation)(Node3D* node, float x, float y, float z);
    void (*Node3D_GetPosition)(Node3D* node, float* outX, float* outY, float* outZ);
    void (*Node3D_SetPosition)(Node3D* node, float x, float y, float z);
    void (*Node3D_GetScale)(Node3D* node, float* outX, float* outY, float* outZ);
    void (*Node3D_SetScale)(Node3D* node, float x, float y, float z);

    // ===== Asset System =====
    Asset* (*LoadAsset)(const char* name);
    Asset* (*FetchAsset)(const char* name);
    void (*UnloadAsset)(const char* name);

    // ===== Audio =====
    void (*PlaySound2D)(SoundWave* sound, float volume, float pitch);
    void (*StopAllSounds)();
    void (*SetMasterVolume)(float volume);
    float (*GetMasterVolume)();

    // ===== Input =====
    bool (*IsKeyDown)(int32_t key);
    bool (*IsKeyJustPressed)(int32_t key);
    bool (*IsKeyJustReleased)(int32_t key);
    bool (*IsMouseButtonDown)(int32_t button);
    bool (*IsMouseButtonJustPressed)(int32_t button);
    void (*GetMousePosition)(int32_t* x, int32_t* y);
    void (*GetMouseDelta)(int32_t* deltaX, int32_t* deltaY);
    int32_t (*GetScrollWheelDelta)();

    // ===== Time =====
    float (*GetDeltaTime)();
    float (*GetElapsedTime)();

    // ===== Editor UI Hooks (Editor builds only) =====
    EditorUIHooks* editorUI;  // nullptr in game builds
};
```

See `OctaveEngineAPI.h` for the complete API with documentation comments.

---

## Export Macros

Use these macros for cross-platform compatibility:

```cpp
#include "OctavePluginAPI.h"

// OCTAVE_PLUGIN_API - marks functions for export
// Expands to:
//   Windows: __declspec(dllexport) or __declspec(dllimport)
//   Linux:   __attribute__((visibility("default")))

// OCTAVE_PLUGIN_API_VERSION - current API version number
// Check this matches the engine version you're targeting
```

When building your addon, define `OCTAVE_PLUGIN_EXPORT` to export symbols:

```cpp
// This is done automatically by the build system
#define OCTAVE_PLUGIN_EXPORT
#include "OctavePluginAPI.h"
```

---

## Development Workflow

There are two ways to develop native addons:

### Option A: Local Development (Recommended for Development)

Create your addon directly in your project's `Packages/` folder:

```
{YourProject}/
    Packages/
        MyAddon/
            package.json
            Source/
                MyAddon.cpp
```

**Steps:**
1. Create `{ProjectDir}/Packages/MyAddon/` directory
2. Add `package.json` with native configuration
3. Add your C++ source files in `Source/`
4. Use **Developer > Reload Native Addons**
5. Your addon is automatically discovered, built, and loaded

**Advantages:**
- No installation step required
- Immediate hot-reload workflow
- Easy to iterate during development
- Files stay in your project folder

### Option B: Installed Addons (For Distribution)

Use the Addons window to install from a repository:

**Steps:**
1. Go to **Edit > Addons...**
2. Add your addon repository or install from local ZIP
3. The addon assets/scripts are merged into your project
4. Native source stays in the addon cache
5. Enable native code in the **Installed** tab

### Building and Reloading

**From the Addons Window:**
- Find your addon in the **Installed** tab
- Click **Build** to compile
- Click **Reload** to hot-load changes
- Check the build log for errors

**From the Developer Menu:**

| Menu Item | Description |
|-----------|-------------|
| **Developer > Reload Native Addons** | Discovers, builds changed addons, and reloads all native addons |
| **Developer > Discover Native Addons** | Scans for native addons without building or loading |
| **Developer > Regenerate Native Addon Dependencies** | Rewrites IDE configs (.vcxproj, .vscode/c_cpp_properties.json) for all addons |
| **Developer > Create Native Addon...** | Opens dialog to create a new native addon with template files |
| **Developer > Package Native Addon...** | Opens dialog to package an addon as a distributable ZIP |

**From the File Menu:**

| Menu Item | Description |
|-----------|-------------|
| **File > Reload All Scripts** | Reloads Lua scripts AND regenerates native addon dependencies AND reloads native addons |

### Iterate Quickly

The hot-reload workflow:
1. Edit your C++ code
2. Press **Developer > Reload Native Addons**
3. Changes take effect immediately - no editor restart!

**Tip:** If you also have Lua scripts, use **File > Reload All Scripts** to reload everything at once.

### IDE Configuration

Native addons generate IDE configuration files for IntelliSense support:

- `.vscode/c_cpp_properties.json` - VS Code C++ configuration
- `{AddonName}.vcxproj` - Visual Studio project file
- `CMakeLists.txt` - CMake configuration

These files include the correct engine include paths. If the engine is moved or paths change, use **Developer > Regenerate Native Addon Dependencies** to update them.

---

## Hot-Reload Best Practices

Hot-reloading replaces your code while the editor is running. Follow these guidelines to avoid crashes:

### Do

- Keep plugin state minimal
- Use the engine's systems for persistent data
- Clean up all resources in `OnUnload()`
- Store configuration in project files, not static variables

### Don't

- Store raw pointers to plugin data in engine objects
- Use static variables that persist across reloads
- Register callbacks without a way to unregister them
- Create threads that outlive the plugin

### Example: Safe Callback Registration

```cpp
static void* sMyCallbackHandle = nullptr;

static int OnLoad(OctaveEngineAPI* api)
{
    // Register with a handle we can use to unregister
    sMyCallbackHandle = SomeSystem::RegisterCallback(MyCallback);
    return 0;
}

static void OnUnload()
{
    // Always unregister before unload!
    if (sMyCallbackHandle)
    {
        SomeSystem::UnregisterCallback(sMyCallbackHandle);
        sMyCallbackHandle = nullptr;
    }
}
```

---

## Registering Custom Node Types

To add custom nodes that appear in the editor:

```cpp
#include "OctavePluginAPI.h"
#include "Node.h"  // From engine includes

class MyCustomNode : public Node3D
{
    DECLARE_NODE(MyCustomNode, Node3D);

public:
    virtual void Tick(float deltaTime) override
    {
        // Custom behavior
    }
};

DEFINE_NODE(MyCustomNode, Node3D);

static void RegisterTypes(void* nodeFactory)
{
    // The factory handles registration via the DEFINE_NODE macro
    // Just ensure the symbol is linked
    FORCE_LINK_CALL(MyCustomNode);
}
```

**Note:** Custom node types require more careful handling during hot-reload. Existing instances of your node type will become invalid after unload.

---

## Extending the Editor UI

Native addons can extend the editor interface with custom menus, windows, and inspectors.

### Adding Menu Items

```cpp
static void RegisterEditorUI(EditorUIHooks* hooks, uint64_t hookId)
{
    // Add a menu item under Developer menu
    hooks->AddMenuItem(
        hookId,
        "Developer",              // Menu to add to
        "My Addon/Do Something",  // Item path (supports submenus)
        [](void* userData) {
            LogDebug("Menu item clicked!");
        },
        nullptr,                  // User data
        "Ctrl+Shift+D"            // Optional shortcut
    );

    // Add to a custom top-level menu
    hooks->AddMenuItem(hookId, "My Addon", "Open Settings", MySettingsCallback, nullptr, nullptr);
}
```

### Creating Custom Windows

```cpp
static void DrawMyWindow(void* userData)
{
    ImGui::Text("Hello from my addon!");

    if (ImGui::Button("Click Me"))
    {
        LogDebug("Button clicked!");
    }

    static float myValue = 0.5f;
    ImGui::SliderFloat("Value", &myValue, 0.0f, 1.0f);
}

static void RegisterEditorUI(EditorUIHooks* hooks, uint64_t hookId)
{
    // Register a dockable window
    hooks->RegisterWindow(
        hookId,
        "My Addon Window",        // Display name
        "myaddon_main_window",    // Unique ID for docking persistence
        DrawMyWindow,
        nullptr
    );

    // Optionally open it immediately
    hooks->OpenWindow("myaddon_main_window");
}
```

### Custom Inspectors

```cpp
static void DrawMyNodeInspector(void* node, void* userData)
{
    MyCustomNode* myNode = static_cast<MyCustomNode*>(node);

    ImGui::Text("Custom Inspector for MyCustomNode");

    // Draw custom properties
    float value = myNode->GetCustomValue();
    if (ImGui::DragFloat("Custom Value", &value))
    {
        myNode->SetCustomValue(value);
    }
}

static void RegisterEditorUI(EditorUIHooks* hooks, uint64_t hookId)
{
    hooks->RegisterInspector(hookId, "MyCustomNode", DrawMyNodeInspector, nullptr);
}
```

### Context Menu Extensions

```cpp
static void RegisterEditorUI(EditorUIHooks* hooks, uint64_t hookId)
{
    // Add to node right-click menu
    hooks->AddNodeContextItem(hookId, "My Addon/Do Something", MyNodeAction, nullptr);

    // Add to asset right-click menu (for textures only)
    hooks->AddAssetContextItem(hookId, "Process Texture", "Texture", MyTextureAction, nullptr);
}
```

### Cleanup on Unload

**Important:** Always clean up your hooks when the plugin unloads!

```cpp
static uint64_t sHookId = 0;

static int OnLoad(OctaveEngineAPI* api)
{
    // hookId will be provided by RegisterEditorUI
    return 0;
}

static void OnUnload()
{
    // Hooks are automatically cleaned up by the engine when a plugin unloads
    // But you can also manually remove specific items if needed
}

static void RegisterEditorUI(EditorUIHooks* hooks, uint64_t hookId)
{
    sHookId = hookId;  // Save for later if needed

    hooks->AddMenuItem(hookId, "Developer", "My Tool", MyCallback, nullptr, nullptr);
    hooks->RegisterWindow(hookId, "My Window", "my_window", DrawMyWindow, nullptr);

    // All hooks registered with this hookId are automatically removed
    // when the plugin is unloaded
}
```

---

## Editor Lifecycle Hooks

In addition to the UI extension hooks above, addons can register for editor lifecycle events. These hooks fire when specific editor actions occur (project open/close, scene changes, packaging, etc.), enabling addons to react to editor state changes without polling.

All lifecycle hooks are registered through the `EditorUIHooks` struct in your `RegisterEditorUI` callback. Each registration takes a `HookId` for automatic cleanup on unload.

### Editor Init Hooks (`OctavePluginDesc`)

These two hooks are set directly in `OctavePluginDesc`, not via `EditorUIHooks`:

| Callback | When Fired | Use Case |
|----------|-----------|----------|
| `OnEditorPreInit` | Before editor ImGui is fully initialized | Custom fonts, ImGui config flags |
| `OnEditorReady` | After editor is fully initialized, before main loop | Query project state, open windows |

```cpp
static void OnEditorPreInit()
{
    // ImGui context exists but UI is not yet fully configured
    // Add custom fonts, set ImGui config flags, etc.
}

static void OnEditorReady()
{
    // Editor is fully up - safe to query project state, open windows, etc.
}

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    // ... other fields ...
    desc->OnEditorPreInit = OnEditorPreInit;
    desc->OnEditorReady = OnEditorReady;
    return 0;
}
```

### Project Lifecycle Events

| Hook | Callback Type | Parameter | When Fired |
|------|--------------|-----------|-----------|
| `RegisterOnProjectOpen` | `StringEventCallback` | Project path | After a project is opened |
| `RegisterOnProjectClose` | `StringEventCallback` | Project path | Before a project is closed |
| `RegisterOnProjectSave` | `StringEventCallback` | File path | After a project/scene is saved |

```cpp
static void RegisterEditorUI(EditorUIHooks* hooks, uint64_t hookId)
{
    hooks->RegisterOnProjectOpen(hookId, [](const char* path, void*) {
        LogDebug("Project opened: %s", path);
    }, nullptr);

    hooks->RegisterOnProjectClose(hookId, [](const char* path, void*) {
        LogDebug("Project closing: %s", path);
    }, nullptr);

    hooks->RegisterOnProjectSave(hookId, [](const char* path, void*) {
        LogDebug("Project saved: %s", path);
    }, nullptr);
}
```

### Scene Lifecycle Events

| Hook | Callback Type | Parameter | When Fired |
|------|--------------|-----------|-----------|
| `RegisterOnSceneOpen` | `StringEventCallback` | Scene name | After a scene is opened for editing |
| `RegisterOnSceneClose` | `StringEventCallback` | Scene name | Before a scene is closed |

```cpp
hooks->RegisterOnSceneOpen(hookId, [](const char* scene, void*) {
    LogDebug("Scene opened: %s", scene);
}, nullptr);

hooks->RegisterOnSceneClose(hookId, [](const char* scene, void*) {
    LogDebug("Scene closing: %s", scene);
}, nullptr);
```

### Packaging/Build Events

| Hook | Callback Type | Parameters | When Fired |
|------|--------------|-----------|-----------|
| `RegisterOnPackageStarted` | `PlatformEventCallback` | Platform (int32_t) | Before build begins |
| `RegisterOnPackageFinished` | `PackageFinishedCallback` | Platform, success (bool) | After build completes or fails |

```cpp
hooks->RegisterOnPackageStarted(hookId, [](int32_t platform, void*) {
    LogDebug("Build starting for platform %d", platform);
    // Pre-build validation here
}, nullptr);

hooks->RegisterOnPackageFinished(hookId, [](int32_t platform, bool success, void*) {
    if (success)
        LogDebug("Build succeeded for platform %d", platform);
    else
        LogError("Build failed for platform %d", platform);
}, nullptr);
```

### Editor State Events

| Hook | Callback Type | Parameter | When Fired |
|------|--------------|-----------|-----------|
| `RegisterOnSelectionChanged` | `EventCallback` | (none) | When the selected node changes |
| `RegisterOnPlayModeChanged` | `PlayModeCallback` | State: 0=Enter, 1=Exit, 2=Eject | When PIE state changes |
| `RegisterOnEditorShutdown` | `EventCallback` | (none) | Before editor shuts down |

```cpp
hooks->RegisterOnSelectionChanged(hookId, [](void*) {
    LogDebug("Selection changed");
    // Query current selection via editor API
}, nullptr);

hooks->RegisterOnPlayModeChanged(hookId, [](int32_t state, void*) {
    const char* names[] = { "Enter", "Exit", "Eject" };
    LogDebug("Play mode: %s", names[state]);
}, nullptr);

hooks->RegisterOnEditorShutdown(hookId, [](void*) {
    LogDebug("Editor shutting down - saving addon state");
}, nullptr);
```

### Asset Pipeline Events

| Hook | Callback Type | Parameter | When Fired |
|------|--------------|-----------|-----------|
| `RegisterOnAssetImported` | `StringEventCallback` | Asset name | After an asset is imported |
| `RegisterOnAssetDeleted` | `StringEventCallback` | Asset name | After an asset is deleted |
| `RegisterOnAssetSaved` | `StringEventCallback` | Asset name | After an asset is saved |

```cpp
hooks->RegisterOnAssetImported(hookId, [](const char* name, void*) {
    LogDebug("Asset imported: %s", name);
}, nullptr);

hooks->RegisterOnAssetDeleted(hookId, [](const char* name, void*) {
    LogDebug("Asset deleted: %s", name);
}, nullptr);

hooks->RegisterOnAssetSaved(hookId, [](const char* name, void*) {
    LogDebug("Asset saved: %s", name);
}, nullptr);
```

### Undo/Redo Events

| Hook | Callback Type | Parameter | When Fired |
|------|--------------|-----------|-----------|
| `RegisterOnUndoRedo` | `EventCallback` | (none) | After an undo or redo operation |

```cpp
hooks->RegisterOnUndoRedo(hookId, [](void*) {
    LogDebug("Undo/Redo performed - refreshing addon state");
}, nullptr);
```

### Top-Level Menus

Add a custom top-level menu to the editor viewport bar alongside File, Edit, View, etc.

| Hook | When Called |
|------|-----------|
| `AddTopLevelMenuItem` | Draws custom menu contents inside ImGui popup |
| `RemoveTopLevelMenuItem` | Removes a previously added top-level menu |

```cpp
hooks->AddTopLevelMenuItem(hookId, "My Addon", [](void*) {
    if (ImGui::MenuItem("Open Dashboard")) { /* ... */ }
    if (ImGui::MenuItem("Settings")) { /* ... */ }
    ImGui::Separator();
    if (ImGui::MenuItem("About")) { /* ... */ }
}, nullptr);
```

### Toolbar Items

Add custom items to the editor viewport toolbar (next to the Play button).

| Hook | When Called |
|------|-----------|
| `AddToolbarItem` | Draws custom toolbar widgets inline |
| `RemoveToolbarItem` | Removes a previously added toolbar item |

```cpp
hooks->AddToolbarItem(hookId, "MyToolbarButton", [](void*) {
    if (ImGui::Button("Quick Build"))
    {
        // Trigger build action
    }
}, nullptr);
```

### Callback Type Reference

| Type | Signature | Used By |
|------|-----------|---------|
| `EventCallback` | `void(void* userData)` | Selection, Shutdown, UndoRedo |
| `StringEventCallback` | `void(const char* str, void* userData)` | Project, Scene, Asset events |
| `PlatformEventCallback` | `void(int32_t platform, void* userData)` | PackageStarted |
| `PackageFinishedCallback` | `void(int32_t platform, bool success, void* userData)` | PackageFinished |
| `PlayModeCallback` | `void(int32_t state, void* userData)` | PlayModeChanged |
| `TopLevelMenuDrawCallback` | `void(void* userData)` | Top-level menus |
| `ToolbarDrawCallback` | `void(void* userData)` | Toolbar items |

---

## Exposing Lua Functions

Add custom functions callable from Lua scripts:

```cpp
#include "OctavePluginAPI.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

static int Lua_MyFunction(lua_State* L)
{
    // Get arguments
    const char* message = luaL_checkstring(L, 1);

    // Do something
    LogDebug("MyAddon: %s", message);

    // Return values
    lua_pushboolean(L, true);
    return 1;  // Number of return values
}

static void RegisterScriptFuncs(void* luaState)
{
    lua_State* L = static_cast<lua_State*>(luaState);

    // Create a table for your addon's functions
    lua_newtable(L);

    lua_pushcfunction(L, Lua_MyFunction);
    lua_setfield(L, -2, "MyFunction");

    // Register the table globally
    lua_setglobal(L, "MyAddon");
}
```

Usage from Lua:

```lua
-- In your Lua scripts
MyAddon.MyFunction("Hello from Lua!")
```

---

## Lua-Based Editor Extensions

You can also extend the editor UI using pure Lua scripts (no C++ required).

### Lua UI Hooks

```lua
-- MyEditorTool.lua (in your addon's Scripts/ folder)

-- Only runs in editor
if not Editor then return end

-- Use this script's UUID as the hook identifier
local hookId = Script.GetUUID()

-- Add a menu item
Editor.AddMenuItem(hookId, "Developer", "My Lua Tool", function()
    Log.Debug("Lua tool activated!")
    Editor.OpenWindow("my_lua_window")
end)

-- Register a custom window
Editor.RegisterWindow(hookId, "My Lua Window", "my_lua_window", function()
    ImGui.Text("Hello from Lua!")

    if ImGui.Button("Do Something") then
        Log.Debug("Button clicked from Lua!")
    end

    -- Access engine state
    local world = Engine.GetWorld(0)
    if world then
        ImGui.Text("World: " .. tostring(world))
    end
end)

-- Clean up when script is unloaded/reloaded
function OnUnload()
    Editor.RemoveAllHooks(hookId)
end
```

### Available Lua Editor Functions

```lua
-- Menu items
Editor.AddMenuItem(hookId, menuPath, itemPath, callback, shortcut)
Editor.AddMenuSeparator(hookId, menuPath)
Editor.RemoveMenuItem(hookId, menuPath, itemPath)

-- Windows
Editor.RegisterWindow(hookId, displayName, windowId, drawCallback)
Editor.UnregisterWindow(hookId, windowId)
Editor.OpenWindow(windowId)
Editor.CloseWindow(windowId)
Editor.IsWindowOpen(windowId)

-- Context menus
Editor.AddNodeContextItem(hookId, itemPath, callback)
Editor.AddAssetContextItem(hookId, itemPath, assetTypeFilter, callback)

-- Cleanup
Editor.RemoveAllHooks(hookId)
```

### IMPORTANT: Game Build Safety

The `Editor` table **does not exist** in game builds. Always check before using:

```lua
-- CORRECT - Safe for game builds
if Editor then
    Editor.AddMenuItem(...)
end

-- CORRECT - Guard entire script
if not Editor then return end

-- WRONG - Will crash in game builds!
Editor.AddMenuItem(...)  -- Editor is nil in game!
```

This ensures your addon's Lua scripts work in both editor and game builds.

### ImGui Bindings for Lua

Common ImGui functions are exposed to Lua:

```lua
-- Layout
ImGui.Text("Hello")
ImGui.TextColored(1, 0, 0, 1, "Red text")
ImGui.Separator()
ImGui.SameLine()
ImGui.NewLine()

-- Input
if ImGui.Button("Click") then ... end
local changed, value = ImGui.DragFloat("Speed", currentValue, 0.1, 0, 100)
local changed, value = ImGui.SliderInt("Count", currentValue, 0, 10)
local changed, value = ImGui.Checkbox("Enabled", currentValue)
local changed, text = ImGui.InputText("Name", currentText, 256)

-- Containers
if ImGui.CollapsingHeader("Section") then ... end
if ImGui.TreeNode("Node") then ... ImGui.TreePop() end
if ImGui.BeginChild("Scroll", 0, 200) then ... ImGui.EndChild() end
```

---

## Final Build Integration

When building for consoles or release:

1. Enable your addon in the project
2. Ensure "Enable Native" is checked
3. Build via **File > Build Data > [Platform]**

The build system will:
- Add your `Source/` directory to include paths
- Compile all `.cpp` files directly into the executable
- No dynamic loading on consoles - code is statically linked

### Static Plugin Registration

In built games (non-editor), plugins are statically linked and must register themselves at startup. Use the `OCTAVE_REGISTER_PLUGIN` macro:

```cpp
#include "OctavePluginAPI.h"

// Forward declaration
extern "C" int OctavePlugin_GetDesc(OctavePluginDesc* desc);

// Static registration for built games (only when compiled-in, not DLL)
#if !defined(OCTAVE_PLUGIN_EXPORT)
#include "Plugins/RuntimePluginManager.h"
OCTAVE_REGISTER_PLUGIN(MyAddon, OctavePlugin_GetDesc)
#endif

// ... rest of plugin code ...
```

**Important:** The `#if !defined(OCTAVE_PLUGIN_EXPORT)` guard ensures this code only runs when the plugin is compiled into the game, not when it's built as a DLL for hot-loading in the editor.

### How It Works

| Context | Loading Method | `OCTAVE_PLUGIN_EXPORT` Defined |
|---------|----------------|-------------------------------|
| Editor (hot-load) | Dynamic DLL loading | Yes |
| Built game | Static registration | No |

In the editor, plugins are loaded as DLLs and `OctavePlugin_GetDesc` is called via `GetProcAddress`/`dlsym`. In built games, the `OCTAVE_REGISTER_PLUGIN` macro registers the plugin at static initialization time, and the `RuntimePluginManager` calls `OnLoad` during engine startup.

### Platform-Specific Code

Use preprocessor checks for platform-specific code:

```cpp
#if PLATFORM_WINDOWS
    // Windows-specific code
#elif PLATFORM_LINUX
    // Linux-specific code
#elif PLATFORM_GCN
    // GameCube-specific code
#elif PLATFORM_WII
    // Wii-specific code
#elif PLATFORM_3DS
    // 3DS-specific code
#endif

#if EDITOR
    // Editor-only code (hot-reload support, debug UI, etc.)
#endif
```

### Tick in Built Games

The `Tick` callback works in both editor (during Play-In-Editor) and built games:

```cpp
static void Tick(float deltaTime)
{
    // This runs every frame during gameplay
    // Works in both editor PIE and built games
}
```

In built games, `TickEditor` is NOT called (it's editor-only). Only `Tick` is called during the game loop.

---

## Troubleshooting

### Build Fails

- Check the build log in the Addons window
- Verify all includes are correct
- Ensure `OctavePluginAPI.h` is in your include path

### Plugin Won't Load

- Check that `OctavePlugin_GetDesc` is exported correctly
- Verify `apiVersion` matches `OCTAVE_PLUGIN_API_VERSION`
- Look for errors in the editor console

### Crash on Reload

- Ensure `OnUnload()` cleans up all resources
- Check for dangling pointers in engine objects
- Verify no static state persists incorrectly

### Missing Symbols on Console Build

- Add `FORCE_LINK_CALL()` for all custom types
- Check that source files are in the `sourceDir` path

---

## Example: Complete Addon

Here's a complete native addon example with tick callbacks:

**package.json:**
```json
{
    "name": "Hello Native",
    "author": "Octave Team",
    "description": "A minimal native addon example.",
    "version": "1.0.0",
    "tags": ["example"],
    "native": {
        "target": "engine",
        "sourceDir": "Source",
        "binaryName": "hellonative",
        "apiVersion": 2
    }
}
```

Use `"target": "editor"` if your addon should only run in the editor and not be compiled into final builds.

**Source/HelloNative.cpp:**
```cpp
#include "OctavePluginAPI.h"

// Static registration for built games (only when compiled-in, not DLL)
#if !defined(OCTAVE_PLUGIN_EXPORT)
#include "Plugins/RuntimePluginManager.h"
OCTAVE_REGISTER_PLUGIN(HelloNative, OctavePlugin_GetDesc)
#endif

static OctaveEngineAPI* sAPI = nullptr;
static float sTotalTime = 0.0f;

static int OnLoad(OctaveEngineAPI* api)
{
    sAPI = api;
    sTotalTime = 0.0f;
    api->LogDebug("Hello Native: Plugin loaded!");
    return 0;
}

static void OnUnload()
{
    if (sAPI)
    {
        sAPI->LogDebug("Hello Native: Plugin unloading...");
    }
    sAPI = nullptr;
}

// Called every frame during gameplay (PIE or built game)
static void Tick(float deltaTime)
{
    sTotalTime += deltaTime;
    // Gameplay logic here...
}

// Called every frame in editor (regardless of play state)
static void TickEditor(float deltaTime)
{
    // Editor visualization, debug overlays, etc.
}

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Hello Native";
    desc->pluginVersion = "1.0.0";

    // Lifecycle
    desc->OnLoad = OnLoad;
    desc->OnUnload = OnUnload;

    // Tick callbacks
    desc->Tick = Tick;               // Gameplay only
    desc->TickEditor = TickEditor;   // Editor always (set to nullptr if not needed)

    // Registration
    desc->RegisterTypes = nullptr;
    desc->RegisterScriptFuncs = nullptr;
    desc->RegisterEditorUI = nullptr;

    // Editor lifecycle (API v2+)
    desc->OnEditorPreInit = nullptr;
    desc->OnEditorReady = nullptr;

    return 0;
}
```

---

## See Also

- [Addons Documentation](../Info/Addons.md) - General addon system
- [Templates Documentation](../Info/Templates.md) - Project templates
- [Lua Scripting Guide](../Scripting/LuaGuide.md) - Lua API reference

### Examples

- [Custom Menu Item](Examples/CustomMenuItem.md) - Adding menu items to existing menus
- [Custom Debug Window](Examples/CustomDebugWindow.md) - Creating dockable windows
- [Custom Inspector](Examples/CustomScriptInspector.md) - Custom node inspectors
- [Custom Context Menu](Examples/CustomContextMenuItem.md) - Right-click context menus
- [Rotator3D](Examples/Rotator3D.md) - Gameplay tick example

### Editor Hook Examples

- [Top Level Menu](Examples/Editor/TopLevelMenu.md) - Custom top-level menu in the viewport bar
- [Packaging Hooks](Examples/Editor/PackagingHooks.md) - Pre/post build validation
- [Project Lifecycle](Examples/Editor/ProjectLifecycle.md) - Project open/close/save events
- [Editor Init Hooks](Examples/Editor/EditorInitHooks.md) - OnEditorPreInit and OnEditorReady
- [Selection Handler](Examples/Editor/SelectionHandler.md) - Reacting to selection changes
- [Play Mode Hooks](Examples/Editor/PlayModeHooks.md) - PIE state change events
- [Scene Lifecycle](Examples/Editor/SceneLifecycle.md) - Scene open/close events
- [Asset Pipeline Hooks](Examples/Editor/AssetPipelineHooks.md) - Asset import/delete/save events
- [Toolbar Extension](Examples/Editor/ToolbarExtension.md) - Custom toolbar buttons
- [Undo/Redo Hook](Examples/Editor/UndoRedoHook.md) - Syncing state with undo/redo
- [Editor Shutdown Hook](Examples/Editor/EditorShutdownHook.md) - Cleanup before editor exit
