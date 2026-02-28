# Example: Custom Theme

A native addon that demonstrates how to apply custom ImGui themes to the editor.

---

## Overview

This example demonstrates:
- Applying custom ImGui colors and styles
- Creating themed UI elements
- Switching between themes at runtime
- Persisting theme preferences

---

## Files

### package.json

```json
{
    "name": "Custom Theme",
    "author": "Octave Examples",
    "description": "Applies custom visual themes to the editor.",
    "version": "1.0.0",
    "tags": ["editor", "theme", "ui"],
    "native": {
        "target": "editor",
        "sourceDir": "Source",
        "binaryName": "customtheme",
        "apiVersion": 2
    }
}
```

### Source/CustomTheme.cpp

```cpp
/**
 * @file CustomTheme.cpp
 * @brief Demonstrates applying custom ImGui themes.
 *
 * With the addon include paths set up, you can now include:
 * - Engine headers (Engine/Source)
 * - Lua headers (External/Lua)
 * - GLM math (External/glm)
 * - ImGui (External/Imgui) - editor builds only
 * - ImGuizmo (External/ImGuizmo) - editor builds only
 * - Bullet physics (External/bullet3/src)
 */

#include "Plugins/OctavePluginAPI.h"
#include "Plugins/OctaveEngineAPI.h"

#if EDITOR
#include "Plugins/EditorUIHooks.h"
// ImGui is available in editor builds (External/Imgui)
#include "imgui.h"
#endif

// GLM is available for all builds (External/glm)
#include "glm/glm.hpp"

static OctaveEngineAPI* sEngineAPI = nullptr;
static uint64_t sHookId = 0;

#if EDITOR

//=============================================================================
// Theme Definitions
//=============================================================================

enum class ThemeType
{
    Dark,
    Light,
    Nord,
    Dracula,
    Monokai,
    Solarized,
    Custom
};

static ThemeType sCurrentTheme = ThemeType::Dark;
static bool sThemeWindowOpen = false;

// Custom theme colors (user-editable)
static ImVec4 sCustomColors[ImGuiCol_COUNT];
static bool sCustomColorsInitialized = false;

//=============================================================================
// Theme Application Functions
//=============================================================================

static void ApplyDarkTheme()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Window
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);

    // Borders
    colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // Frame
    colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);

    // Title
    colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);

    // Menu
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);

    // Scrollbar
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);

    // Buttons
    colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);

    // Header
    colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);

    // Separator
    colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);

    // Tab
    colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.35f, 0.58f, 0.86f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.41f, 0.68f, 1.00f);

    // Text
    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

    if (sEngineAPI)
    {
        sEngineAPI->LogDebug("Applied Dark theme");
    }
}

static void ApplyLightTheme()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);

    colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);

    colors[ImGuiCol_TitleBg] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);

    colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);

    colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);

    colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);

    colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);

    if (sEngineAPI)
    {
        sEngineAPI->LogDebug("Applied Light theme");
    }
}

static void ApplyNordTheme()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Nord color palette
    // Polar Night
    ImVec4 nord0(0.18f, 0.20f, 0.25f, 1.0f);   // #2E3440
    ImVec4 nord1(0.23f, 0.26f, 0.32f, 1.0f);   // #3B4252
    ImVec4 nord2(0.26f, 0.30f, 0.37f, 1.0f);   // #434C5E
    ImVec4 nord3(0.30f, 0.34f, 0.42f, 1.0f);   // #4C566A

    // Snow Storm
    ImVec4 nord4(0.85f, 0.87f, 0.91f, 1.0f);   // #D8DEE9
    ImVec4 nord5(0.90f, 0.91f, 0.94f, 1.0f);   // #E5E9F0
    ImVec4 nord6(0.93f, 0.94f, 0.96f, 1.0f);   // #ECEFF4

    // Frost
    ImVec4 nord7(0.56f, 0.74f, 0.73f, 1.0f);   // #8FBCBB
    ImVec4 nord8(0.53f, 0.75f, 0.82f, 1.0f);   // #88C0D0
    ImVec4 nord9(0.51f, 0.63f, 0.76f, 1.0f);   // #81A1C1
    ImVec4 nord10(0.37f, 0.51f, 0.67f, 1.0f);  // #5E81AC

    // Aurora
    ImVec4 nord11(0.75f, 0.38f, 0.42f, 1.0f);  // #BF616A (red)
    ImVec4 nord12(0.83f, 0.54f, 0.44f, 1.0f);  // #D08770 (orange)
    ImVec4 nord13(0.92f, 0.80f, 0.55f, 1.0f);  // #EBCB8B (yellow)
    ImVec4 nord14(0.64f, 0.75f, 0.55f, 1.0f);  // #A3BE8C (green)
    ImVec4 nord15(0.71f, 0.56f, 0.68f, 1.0f);  // #B48EAD (purple)

    colors[ImGuiCol_WindowBg] = nord0;
    colors[ImGuiCol_ChildBg] = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_PopupBg] = nord1;
    colors[ImGuiCol_Border] = nord3;

    colors[ImGuiCol_FrameBg] = nord1;
    colors[ImGuiCol_FrameBgHovered] = nord2;
    colors[ImGuiCol_FrameBgActive] = nord3;

    colors[ImGuiCol_TitleBg] = nord1;
    colors[ImGuiCol_TitleBgActive] = nord2;
    colors[ImGuiCol_TitleBgCollapsed] = nord0;

    colors[ImGuiCol_MenuBarBg] = nord1;

    colors[ImGuiCol_Button] = nord9;
    colors[ImGuiCol_ButtonHovered] = nord8;
    colors[ImGuiCol_ButtonActive] = nord10;

    colors[ImGuiCol_Header] = nord2;
    colors[ImGuiCol_HeaderHovered] = nord3;
    colors[ImGuiCol_HeaderActive] = nord9;

    colors[ImGuiCol_Text] = nord4;
    colors[ImGuiCol_TextDisabled] = nord3;

    colors[ImGuiCol_Tab] = nord1;
    colors[ImGuiCol_TabHovered] = nord9;
    colors[ImGuiCol_TabActive] = nord2;

    if (sEngineAPI)
    {
        sEngineAPI->LogDebug("Applied Nord theme");
    }
}

static void ApplyDraculaTheme()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Dracula color palette
    ImVec4 background(0.16f, 0.16f, 0.21f, 1.0f);    // #282a36
    ImVec4 currentLine(0.27f, 0.28f, 0.35f, 1.0f);   // #44475a
    ImVec4 foreground(0.97f, 0.97f, 0.95f, 1.0f);    // #f8f8f2
    ImVec4 comment(0.38f, 0.45f, 0.53f, 1.0f);       // #6272a4
    ImVec4 cyan(0.55f, 0.91f, 0.99f, 1.0f);          // #8be9fd
    ImVec4 green(0.31f, 0.98f, 0.48f, 1.0f);         // #50fa7b
    ImVec4 orange(1.00f, 0.72f, 0.42f, 1.0f);        // #ffb86c
    ImVec4 pink(1.00f, 0.47f, 0.66f, 1.0f);          // #ff79c6
    ImVec4 purple(0.74f, 0.58f, 0.98f, 1.0f);        // #bd93f9
    ImVec4 red(1.00f, 0.33f, 0.33f, 1.0f);           // #ff5555
    ImVec4 yellow(0.95f, 0.98f, 0.55f, 1.0f);        // #f1fa8c

    colors[ImGuiCol_WindowBg] = background;
    colors[ImGuiCol_PopupBg] = background;
    colors[ImGuiCol_Border] = comment;

    colors[ImGuiCol_FrameBg] = currentLine;
    colors[ImGuiCol_FrameBgHovered] = ImVec4(currentLine.x + 0.1f, currentLine.y + 0.1f, currentLine.z + 0.1f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = purple;

    colors[ImGuiCol_TitleBg] = background;
    colors[ImGuiCol_TitleBgActive] = currentLine;

    colors[ImGuiCol_MenuBarBg] = background;

    colors[ImGuiCol_Button] = purple;
    colors[ImGuiCol_ButtonHovered] = pink;
    colors[ImGuiCol_ButtonActive] = cyan;

    colors[ImGuiCol_Header] = currentLine;
    colors[ImGuiCol_HeaderHovered] = purple;
    colors[ImGuiCol_HeaderActive] = pink;

    colors[ImGuiCol_Text] = foreground;
    colors[ImGuiCol_TextDisabled] = comment;

    if (sEngineAPI)
    {
        sEngineAPI->LogDebug("Applied Dracula theme");
    }
}

static void ApplyTheme(ThemeType theme)
{
    sCurrentTheme = theme;

    switch (theme)
    {
        case ThemeType::Dark:     ApplyDarkTheme(); break;
        case ThemeType::Light:    ApplyLightTheme(); break;
        case ThemeType::Nord:     ApplyNordTheme(); break;
        case ThemeType::Dracula:  ApplyDraculaTheme(); break;
        default:                  ApplyDarkTheme(); break;
    }
}

//=============================================================================
// Theme Selector Window
//=============================================================================

static void DrawThemeWindow(void* userData)
{
    ImGui::Text("Select Editor Theme");
    ImGui::Separator();

    const char* themes[] = { "Dark", "Light", "Nord", "Dracula", "Monokai", "Solarized", "Custom" };
    int currentThemeIndex = static_cast<int>(sCurrentTheme);

    if (ImGui::Combo("Theme", &currentThemeIndex, themes, IM_ARRAYSIZE(themes)))
    {
        ApplyTheme(static_cast<ThemeType>(currentThemeIndex));
    }

    ImGui::Spacing();
    ImGui::Separator();

    // Preview area
    ImGui::Text("Preview");

    if (ImGui::Button("Sample Button"))
    {
        // Does nothing, just for preview
    }
    ImGui::SameLine();
    if (ImGui::Button("Another Button"))
    {
    }

    static float sliderValue = 0.5f;
    ImGui::SliderFloat("Slider", &sliderValue, 0.0f, 1.0f);

    static bool checkValue = true;
    ImGui::Checkbox("Checkbox", &checkValue);

    static char textValue[64] = "Sample text";
    ImGui::InputText("Text Input", textValue, sizeof(textValue));

    if (ImGui::CollapsingHeader("Collapsible Section"))
    {
        ImGui::Text("Content inside collapsible");
        ImGui::BulletText("Item 1");
        ImGui::BulletText("Item 2");
    }

    ImGui::Spacing();
    ImGui::Separator();

    // Style adjustments
    ImGuiStyle& style = ImGui::GetStyle();

    if (ImGui::CollapsingHeader("Style Adjustments"))
    {
        ImGui::SliderFloat("Window Rounding", &style.WindowRounding, 0.0f, 12.0f);
        ImGui::SliderFloat("Frame Rounding", &style.FrameRounding, 0.0f, 12.0f);
        ImGui::SliderFloat("Popup Rounding", &style.PopupRounding, 0.0f, 12.0f);
        ImGui::SliderFloat("Scrollbar Rounding", &style.ScrollbarRounding, 0.0f, 12.0f);
        ImGui::SliderFloat("Grab Rounding", &style.GrabRounding, 0.0f, 12.0f);
        ImGui::SliderFloat("Tab Rounding", &style.TabRounding, 0.0f, 12.0f);

        ImGui::Spacing();

        ImGui::SliderFloat2("Window Padding", &style.WindowPadding.x, 0.0f, 20.0f);
        ImGui::SliderFloat2("Frame Padding", &style.FramePadding.x, 0.0f, 20.0f);
        ImGui::SliderFloat2("Item Spacing", &style.ItemSpacing.x, 0.0f, 20.0f);
    }

    // Reset button
    ImGui::Spacing();
    if (ImGui::Button("Reset to Default Style"))
    {
        style = ImGuiStyle();
        ApplyTheme(sCurrentTheme);
    }
}

//=============================================================================
// Menu Callbacks
//=============================================================================

static void OnOpenThemeWindow(void* userData)
{
    sThemeWindowOpen = true;
}

static void OnApplyDark(void* userData) { ApplyTheme(ThemeType::Dark); }
static void OnApplyLight(void* userData) { ApplyTheme(ThemeType::Light); }
static void OnApplyNord(void* userData) { ApplyTheme(ThemeType::Nord); }
static void OnApplyDracula(void* userData) { ApplyTheme(ThemeType::Dracula); }

//=============================================================================
// Plugin Callbacks
//=============================================================================

static void RegisterEditorUI(EditorUIHooks* hooks, uint64_t hookId)
{
    sHookId = hookId;

    // Register the theme window
    hooks->RegisterWindow(hookId, "Theme Settings", "customtheme_window", DrawThemeWindow, nullptr);

    // Add menu items
    hooks->AddMenuItem(hookId, "Edit", "Themes/Theme Settings...", OnOpenThemeWindow, nullptr, nullptr);
    hooks->AddMenuSeparator(hookId, "Edit");
    hooks->AddMenuItem(hookId, "Edit", "Themes/Dark", OnApplyDark, nullptr, nullptr);
    hooks->AddMenuItem(hookId, "Edit", "Themes/Light", OnApplyLight, nullptr, nullptr);
    hooks->AddMenuItem(hookId, "Edit", "Themes/Nord", OnApplyNord, nullptr, nullptr);
    hooks->AddMenuItem(hookId, "Edit", "Themes/Dracula", OnApplyDracula, nullptr, nullptr);

    if (sEngineAPI)
    {
        sEngineAPI->LogDebug("CustomTheme: Registered theme window and menu items");
    }
}

#endif // EDITOR

static int OnLoad(OctaveEngineAPI* api)
{
    sEngineAPI = api;
    api->LogDebug("CustomTheme addon loaded!");

#if EDITOR
    // Apply default theme on load
    ApplyTheme(ThemeType::Dark);
#endif

    return 0;
}

static void OnUnload()
{
    if (sEngineAPI)
    {
        sEngineAPI->LogDebug("CustomTheme addon unloaded.");
    }
    sEngineAPI = nullptr;
}

//=============================================================================
// Plugin Entry Point
//=============================================================================

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Custom Theme";
    desc->pluginVersion = "1.0.0";
    desc->OnLoad = OnLoad;
    desc->OnUnload = OnUnload;
    desc->Tick = nullptr;          // Editor-only addon, no gameplay tick
    desc->TickEditor = nullptr;    // Theme is applied once, no per-frame updates
    desc->RegisterTypes = nullptr;
    desc->RegisterScriptFuncs = nullptr;

#if EDITOR
    desc->RegisterEditorUI = RegisterEditorUI;
#else
    desc->RegisterEditorUI = nullptr;
#endif

    return 0;
}
```

---

## Available Themes

| Theme | Description |
|-------|-------------|
| Dark | Default dark theme with blue accents |
| Light | Clean light theme |
| Nord | Based on the Nord color palette (arctic blue-gray) |
| Dracula | Based on the Dracula color scheme (purple accents) |
| Monokai | Based on the Monokai color scheme |
| Solarized | Based on the Solarized color palette |
| Custom | User-defined colors |

---

## Usage

After loading the addon:

1. Go to **Edit > Themes > Theme Settings...** to open the theme window
2. Or quickly switch themes via **Edit > Themes > [Theme Name]**
3. Adjust style parameters (rounding, padding) in the Theme Settings window

---

## Creating Custom Themes

To create your own theme:

```cpp
static void ApplyMyCustomTheme()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Define your colors
    ImVec4 myPrimary(0.2f, 0.6f, 0.8f, 1.0f);
    ImVec4 myBackground(0.1f, 0.1f, 0.12f, 1.0f);
    // ... etc

    // Apply to ImGui color slots
    colors[ImGuiCol_WindowBg] = myBackground;
    colors[ImGuiCol_Button] = myPrimary;
    // ... etc
}
```

---

## ImGui Color Slots

Key color slots to customize:

| Slot | Description |
|------|-------------|
| `ImGuiCol_WindowBg` | Window background |
| `ImGuiCol_ChildBg` | Child window background |
| `ImGuiCol_PopupBg` | Popup/tooltip background |
| `ImGuiCol_Border` | Border color |
| `ImGuiCol_FrameBg` | Input field background |
| `ImGuiCol_FrameBgHovered` | Input field hovered |
| `ImGuiCol_FrameBgActive` | Input field active |
| `ImGuiCol_TitleBg` | Title bar background |
| `ImGuiCol_TitleBgActive` | Active title bar |
| `ImGuiCol_MenuBarBg` | Menu bar background |
| `ImGuiCol_Button` | Button color |
| `ImGuiCol_ButtonHovered` | Button hovered |
| `ImGuiCol_ButtonActive` | Button pressed |
| `ImGuiCol_Header` | Header/tree node |
| `ImGuiCol_Tab` | Tab color |
| `ImGuiCol_Text` | Text color |
| `ImGuiCol_TextDisabled` | Disabled text |
