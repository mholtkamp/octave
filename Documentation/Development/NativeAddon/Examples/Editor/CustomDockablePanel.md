# Custom Dockable Panel

Register a dockable editor panel from a native C++ addon. The panel participates in the dock layout alongside built-in panels (Scene, Assets, Properties, etc.) and can be dragged, tabbed, and resized by the user.

## API

```cpp
// EditorUIHooks (passed to your addon via OnLoad)
void (*RegisterWindow)(
    HookId hookId,
    const char* windowName,   // Display name shown on the dock tab
    const char* windowId,     // Unique ID for persistence / open-close tracking
    WindowDrawCallback drawFunc,
    void* userData
);

void (*UnregisterWindow)(HookId hookId, const char* windowId);
void (*OpenWindow)(const char* windowId);
void (*CloseWindow)(const char* windowId);
bool (*IsWindowOpen)(const char* windowId);
```

`WindowDrawCallback` signature:

```cpp
typedef void (*WindowDrawCallback)(void* userData);
```

Your callback is invoked inside `BeginDock` / `EndDock`, so you can use any ImGui widgets directly -- no need to call `Begin`/`End` yourself.

## Example

```cpp
#include "Plugins/EditorUIHooks.h"
#include "imgui.h"

// Unique hook identifier for this addon
static HookId sHookId = 0;

// State for the panel content
struct MyPanelState
{
    float mValue = 0.5f;
    char  mText[128] = "Hello from addon!";
};

static MyPanelState sState;

// Draw callback -- called every frame while the panel is open
static void DrawMyPanel(void* userData)
{
    MyPanelState* state = static_cast<MyPanelState*>(userData);

    ImGui::Text("My Custom Panel");
    ImGui::Separator();
    ImGui::SliderFloat("Value", &state->mValue, 0.0f, 1.0f);
    ImGui::InputText("Label", state->mText, sizeof(state->mText));

    if (ImGui::Button("Reset"))
    {
        state->mValue = 0.5f;
        snprintf(state->mText, sizeof(state->mText), "Hello from addon!");
    }
}

// Called by the engine when the addon is loaded
extern "C" void OnLoad(EditorUIHooks* hooks)
{
    sHookId = GenerateHookId("com.example.my-panel");

    // Register the dockable panel
    hooks->RegisterWindow(
        sHookId,
        "My Panel",       // Tab label
        "my_panel",       // Unique ID
        DrawMyPanel,
        &sState
    );

    // Open it immediately (optional)
    hooks->OpenWindow("my_panel");
}

// Called by the engine when the addon is unloaded
extern "C" void OnUnload(EditorUIHooks* hooks)
{
    hooks->RemoveAllHooks(sHookId);
}
```

## Opening / Closing from a Menu

You can combine `RegisterWindow` with `AddMenuItem` to give users a toggle in the View menu:

```cpp
static void ToggleMyPanel(void* userData)
{
    EditorUIHooks* hooks = static_cast<EditorUIHooks*>(userData);
    if (hooks->IsWindowOpen("my_panel"))
        hooks->CloseWindow("my_panel");
    else
        hooks->OpenWindow("my_panel");
}

// Inside OnLoad:
hooks->AddMenuItem(sHookId, "View", "My Panel", ToggleMyPanel, hooks, "Ctrl+Shift+P");
```

## Notes

- The panel is rendered inside the dock layout, so users can drag its tab to dock it next to any built-in panel (Scene, Assets, etc.).
- The `windowId` is used internally to track open/close state. Keep it stable across versions.
- Call `RemoveAllHooks` in `OnUnload` to cleanly unregister the panel when your addon is unloaded.
- Your draw callback receives the `userData` pointer you passed during registration.
