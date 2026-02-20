# Custom Scene Type Registration

Register a custom scene type that appears in the "New Scene" dialog. When the user selects your scene type and clicks Create, your callback populates the root node with a pre-configured hierarchy.

## API

```cpp
// EditorUIHooks (passed to your addon via RegisterEditorUI)
void (*RegisterSceneType)(
    HookId hookId,
    const char* typeName,              // Display name on the radio button
    SceneCreationCallback createFunc,
    void* userData
);

void (*UnregisterSceneType)(HookId hookId, const char* typeName);
```

`SceneCreationCallback` signature:

```cpp
typedef void (*SceneCreationCallback)(
    const char* sceneName,  // Asset name entered by user
    void* rootNode,         // Pointer to root Node* -- add children under it
    void* userData
);
```

Your callback receives the root `Node*` (cast from `void*`). Create child nodes under it to populate the scene. The engine handles asset creation, capture, and saving.

## Example

```cpp
#include "Plugins/OctavePluginAPI.h"
#include "Plugins/OctaveEngineAPI.h"

#if EDITOR
#include "Plugins/EditorUIHooks.h"
#endif

static OctaveEngineAPI* sEngineAPI = nullptr;

#if EDITOR
#include "Nodes/Node.h"
#include "Nodes/3D/Node3d.h"
#include "Nodes/3D/Camera3d.h"
#include "Nodes/3D/DirectionalLight3d.h"
#include "Nodes/3D/StaticMesh3d.h"

/**
 * @brief Creates a pre-configured isometric scene.
 *
 * Hierarchy:
 *   Root (Node3D)
 *     Camera3D  -- positioned at isometric angle
 *     Sun       -- directional light
 *     Ground    -- placeholder static mesh
 */
static void CreateIsometricScene(const char* sceneName, void* rootNode, void* userData)
{
    Node* root = static_cast<Node*>(rootNode);

    // Isometric camera at 45-degree angle
    Camera3D* cam = root->CreateChild<Camera3D>("Camera3D");
    cam->SetPosition(glm::vec3(10.0f, 10.0f, 10.0f));

    // Directional light
    DirectionalLight3D* sun = root->CreateChild<DirectionalLight3D>("Sun");
    sun->SetRotation(glm::vec3(-45.0f, 30.0f, 0.0f));

    // Ground plane placeholder
    root->CreateChild<Node3D>("Ground");
}

/**
 * @brief Creates a top-down 2D scene template.
 *
 * Hierarchy:
 *   Root (Node3D)
 *     Camera3D  -- looking straight down
 *     World     -- container for game objects
 */
static void CreateTopDownScene(const char* sceneName, void* rootNode, void* userData)
{
    Node* root = static_cast<Node*>(rootNode);

    Camera3D* cam = root->CreateChild<Camera3D>("Camera3D");
    cam->SetPosition(glm::vec3(0.0f, 20.0f, 0.0f));
    cam->SetRotation(glm::vec3(-90.0f, 0.0f, 0.0f));

    root->CreateChild<Node3D>("World");
}
#endif

static int OnLoad(OctaveEngineAPI* api)
{
    sEngineAPI = api;
    return 0;
}

static void OnUnload()
{
    sEngineAPI = nullptr;
}

#if EDITOR
static void RegisterEditorUI(EditorUIHooks* hooks, uint64_t hookId)
{
    hooks->RegisterSceneType(hookId, "Isometric", CreateIsometricScene, nullptr);
    hooks->RegisterSceneType(hookId, "Top-Down", CreateTopDownScene, nullptr);
}
#endif

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Scene Templates Addon";
    desc->pluginVersion = "1.0.0";
    desc->OnLoad = OnLoad;
    desc->OnUnload = OnUnload;
    desc->Tick = nullptr;
    desc->TickEditor = nullptr;
    desc->RegisterTypes = nullptr;
    desc->RegisterScriptFuncs = nullptr;
#if EDITOR
    desc->RegisterEditorUI = RegisterEditorUI;
#else
    desc->RegisterEditorUI = nullptr;
#endif
    desc->OnEditorPreInit = nullptr;
    desc->OnEditorReady = nullptr;
    return 0;
}
```

## Result

The New Scene dialog will show additional radio buttons:

```
Name: [SC_Scene          ]

(o) 2D  (o) 3D  (o) Isometric  (o) Top-Down
                      ^^^^^^^       ^^^^^^^^
                      addon types

[x] Create Camera

[ Create ]
```

Selecting "Isometric" and clicking Create produces:

```
Root (Node3D)
  Camera3D        -- position (10, 10, 10)
  Sun             -- DirectionalLight3D
  Ground          -- Node3D placeholder
```

## API Reference

### RegisterSceneType

```cpp
void (*RegisterSceneType)(HookId hookId, const char* typeName,
    SceneCreationCallback createFunc, void* userData);
```

**Parameters:**
- `hookId` - The hook identifier for cleanup
- `typeName` - Display label for the radio button (e.g., `"Isometric"`, `"Top-Down"`)
- `createFunc` - Callback: `void createFunc(const char* sceneName, void* rootNode, void* userData)`
- `userData` - Optional user data passed to the callback

### UnregisterSceneType

```cpp
void (*UnregisterSceneType)(HookId hookId, const char* typeName);
```

Removes a previously registered scene type.

## Notes

- The root node passed to your callback is a `Node3D`. Cast `rootNode` to `Node*` and create children under it.
- The engine handles creating the Scene asset, calling `Capture()`, and saving -- your callback only needs to build the node tree.
- The "Create Camera" checkbox only applies to the built-in 2D/3D types. Plugin scene types control their own hierarchy entirely.
- Call `RemoveAllHooks` in `OnUnload` to cleanly unregister all scene types when your addon is unloaded.
- Scene type names should be short to fit on the radio button row.
