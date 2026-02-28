# Example: Debug Log Window

A native addon that creates a resizable debug log window at the bottom of the editor, similar to Unity's Console or Unreal's Output Log.

---

## Overview

This example demonstrates:
- Creating a dockable window that defaults to the bottom
- Capturing and displaying log messages
- Filtering logs by severity (Debug, Warning, Error)
- Searchable log history
- Vertical resizing
- Copy/clear functionality

---

## Files

### package.json

```json
{
    "name": "Debug Log",
    "author": "Octave Examples",
    "description": "A dockable debug log window with filtering and search.",
    "version": "1.0.0",
    "tags": ["editor", "debug", "logging"],
    "native": {
        "target": "editor",
        "sourceDir": "Source",
        "binaryName": "debuglog",
        "apiVersion": 2
    }
}
```

### Source/DebugLog.cpp

```cpp
/**
 * @file DebugLog.cpp
 * @brief Debug log window for the editor.
 *
 * This example demonstrates:
 * - Creating a custom editor window with ImGui
 * - Direct access to engine subsystems via OctaveEngineAPI
 * - Proper #if EDITOR guards for editor-only functionality
 *
 * Available engine includes for addons:
 * - Engine/Source - Core engine headers
 * - External/Lua - Lua scripting (lua.h, lauxlib.h)
 * - External/glm - GLM math library
 * - External/Imgui - ImGui UI (editor builds only)
 * - External/bullet3/src - Physics headers
 */

#include "Plugins/OctavePluginAPI.h"
#include "Plugins/OctaveEngineAPI.h"

#if EDITOR
#include "Plugins/EditorUIHooks.h"
// ImGui is available via External/Imgui
#include "imgui.h"
#endif

// GLM available via External/glm
#include "glm/glm.hpp"

#include <vector>
#include <string>
#include <cstring>
#include <ctime>
#include <mutex>

static OctaveEngineAPI* sEngineAPI = nullptr;
static uint64_t sHookId = 0;

#if EDITOR

//=============================================================================
// Log Entry Structure
//=============================================================================

enum class LogLevel
{
    Debug,
    Warning,
    Error,
    Info
};

struct LogEntry
{
    std::string message;
    std::string timestamp;
    LogLevel level;
    int count;  // For collapsing duplicate messages
};

//=============================================================================
// Log State
//=============================================================================

static std::vector<LogEntry> sLogEntries;
static std::mutex sLogMutex;
static bool sLogWindowOpen = true;
static char sSearchBuffer[256] = "";
static bool sAutoScroll = true;
static bool sShowTimestamps = true;
static bool sCollapseDuplicates = true;

// Filters
static bool sShowDebug = true;
static bool sShowWarnings = true;
static bool sShowErrors = true;
static bool sShowInfo = true;

// Counts for toolbar
static int sDebugCount = 0;
static int sWarningCount = 0;
static int sErrorCount = 0;

// Window sizing
static float sWindowHeight = 200.0f;
static float sMinHeight = 100.0f;
static float sMaxHeight = 500.0f;

//=============================================================================
// Helper Functions
//=============================================================================

static std::string GetTimestamp()
{
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    char buffer[32];
    strftime(buffer, sizeof(buffer), "[%H:%M:%S]", timeinfo);
    return std::string(buffer);
}

static ImVec4 GetLevelColor(LogLevel level)
{
    switch (level)
    {
        case LogLevel::Debug:   return ImVec4(0.7f, 0.7f, 0.7f, 1.0f);  // Gray
        case LogLevel::Info:    return ImVec4(0.8f, 0.8f, 1.0f, 1.0f);  // Light blue
        case LogLevel::Warning: return ImVec4(1.0f, 0.8f, 0.0f, 1.0f);  // Yellow
        case LogLevel::Error:   return ImVec4(1.0f, 0.3f, 0.3f, 1.0f);  // Red
    }
    return ImVec4(1, 1, 1, 1);
}

static const char* GetLevelPrefix(LogLevel level)
{
    switch (level)
    {
        case LogLevel::Debug:   return "[DBG]";
        case LogLevel::Info:    return "[INF]";
        case LogLevel::Warning: return "[WRN]";
        case LogLevel::Error:   return "[ERR]";
    }
    return "[???]";
}

static const char* GetLevelIcon(LogLevel level)
{
    switch (level)
    {
        case LogLevel::Debug:   return " ";
        case LogLevel::Info:    return "i";
        case LogLevel::Warning: return "!";
        case LogLevel::Error:   return "X";
    }
    return "?";
}

static bool PassesFilter(const LogEntry& entry)
{
    // Check level filter
    switch (entry.level)
    {
        case LogLevel::Debug:   if (!sShowDebug) return false; break;
        case LogLevel::Info:    if (!sShowInfo) return false; break;
        case LogLevel::Warning: if (!sShowWarnings) return false; break;
        case LogLevel::Error:   if (!sShowErrors) return false; break;
    }

    // Check search filter
    if (strlen(sSearchBuffer) > 0)
    {
        // Case-insensitive search
        std::string msgLower = entry.message;
        std::string searchLower = sSearchBuffer;

        for (auto& c : msgLower) c = tolower(c);
        for (auto& c : searchLower) c = tolower(c);

        if (msgLower.find(searchLower) == std::string::npos)
        {
            return false;
        }
    }

    return true;
}

//=============================================================================
// Public Logging API
//=============================================================================

static void AddLogEntry(const char* message, LogLevel level)
{
    std::lock_guard<std::mutex> lock(sLogMutex);

    // Update counts
    switch (level)
    {
        case LogLevel::Debug:   sDebugCount++; break;
        case LogLevel::Warning: sWarningCount++; break;
        case LogLevel::Error:   sErrorCount++; break;
        default: break;
    }

    // Check for duplicate (collapse)
    if (sCollapseDuplicates && !sLogEntries.empty())
    {
        LogEntry& last = sLogEntries.back();
        if (last.message == message && last.level == level)
        {
            last.count++;
            last.timestamp = GetTimestamp();
            return;
        }
    }

    LogEntry entry;
    entry.message = message;
    entry.timestamp = GetTimestamp();
    entry.level = level;
    entry.count = 1;

    sLogEntries.push_back(entry);

    // Limit log size
    const size_t maxEntries = 1000;
    if (sLogEntries.size() > maxEntries)
    {
        sLogEntries.erase(sLogEntries.begin());
    }
}

// Lua-callable functions
static void Log_Debug(const char* msg) { AddLogEntry(msg, LogLevel::Debug); }
static void Log_Info(const char* msg) { AddLogEntry(msg, LogLevel::Info); }
static void Log_Warning(const char* msg) { AddLogEntry(msg, LogLevel::Warning); }
static void Log_Error(const char* msg) { AddLogEntry(msg, LogLevel::Error); }

//=============================================================================
// Window Drawing
//=============================================================================

static void DrawDebugLogWindow(void* userData)
{
    // Toolbar
    {
        // Clear button
        if (ImGui::Button("Clear"))
        {
            std::lock_guard<std::mutex> lock(sLogMutex);
            sLogEntries.clear();
            sDebugCount = 0;
            sWarningCount = 0;
            sErrorCount = 0;
        }

        ImGui::SameLine();

        // Collapse toggle
        ImGui::Checkbox("Collapse", &sCollapseDuplicates);

        ImGui::SameLine();

        // Auto-scroll toggle
        ImGui::Checkbox("Auto-scroll", &sAutoScroll);

        ImGui::SameLine();

        // Timestamp toggle
        ImGui::Checkbox("Time", &sShowTimestamps);

        ImGui::SameLine();
        ImGui::Separator();
        ImGui::SameLine();

        // Filter buttons with counts
        ImGui::PushStyleColor(ImGuiCol_Text, GetLevelColor(LogLevel::Debug));
        if (ImGui::Checkbox("Debug", &sShowDebug)) {}
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::TextDisabled("(%d)", sDebugCount);

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Text, GetLevelColor(LogLevel::Warning));
        if (ImGui::Checkbox("Warn", &sShowWarnings)) {}
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::TextDisabled("(%d)", sWarningCount);

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Text, GetLevelColor(LogLevel::Error));
        if (ImGui::Checkbox("Error", &sShowErrors)) {}
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::TextDisabled("(%d)", sErrorCount);

        ImGui::SameLine();
        ImGui::Separator();
        ImGui::SameLine();

        // Search box
        ImGui::SetNextItemWidth(150);
        ImGui::InputText("##Search", sSearchBuffer, sizeof(sSearchBuffer));
        ImGui::SameLine();
        if (ImGui::Button("X##ClearSearch"))
        {
            sSearchBuffer[0] = '\0';
        }
    }

    ImGui::Separator();

    // Log content area
    ImGui::BeginChild("LogScrollRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    std::lock_guard<std::mutex> lock(sLogMutex);

    ImGuiListClipper clipper;
    clipper.Begin((int)sLogEntries.size());

    while (clipper.Step())
    {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
        {
            const LogEntry& entry = sLogEntries[i];

            if (!PassesFilter(entry))
            {
                continue;
            }

            ImGui::PushID(i);

            // Icon/prefix
            ImVec4 color = GetLevelColor(entry.level);
            ImGui::TextColored(color, "%s", GetLevelIcon(entry.level));
            ImGui::SameLine();

            // Timestamp
            if (sShowTimestamps)
            {
                ImGui::TextDisabled("%s", entry.timestamp.c_str());
                ImGui::SameLine();
            }

            // Count badge (if collapsed)
            if (entry.count > 1)
            {
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f), "(%d)", entry.count);
                ImGui::SameLine();
            }

            // Message
            ImGui::TextColored(color, "%s", entry.message.c_str());

            // Context menu
            if (ImGui::BeginPopupContextItem("LogEntryContext"))
            {
                if (ImGui::MenuItem("Copy"))
                {
                    std::string fullMsg = entry.timestamp + " " + GetLevelPrefix(entry.level) + " " + entry.message;
                    ImGui::SetClipboardText(fullMsg.c_str());
                }
                if (ImGui::MenuItem("Copy Message Only"))
                {
                    ImGui::SetClipboardText(entry.message.c_str());
                }
                ImGui::EndPopup();
            }

            ImGui::PopID();
        }
    }

    clipper.End();

    // Auto-scroll to bottom
    if (sAutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();
}

//=============================================================================
// Menu Callbacks
//=============================================================================

static void OnOpenLogWindow(void* userData)
{
    sLogWindowOpen = true;
}

static void OnClearLog(void* userData)
{
    std::lock_guard<std::mutex> lock(sLogMutex);
    sLogEntries.clear();
    sDebugCount = 0;
    sWarningCount = 0;
    sErrorCount = 0;
}

static void OnTestDebug(void* userData) { Log_Debug("This is a debug message"); }
static void OnTestInfo(void* userData) { Log_Info("This is an info message"); }
static void OnTestWarning(void* userData) { Log_Warning("This is a warning message"); }
static void OnTestError(void* userData) { Log_Error("This is an error message"); }

static void OnTestBurst(void* userData)
{
    for (int i = 0; i < 100; i++)
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "Burst message %d", i + 1);
        Log_Debug(buf);
    }
}

//=============================================================================
// Plugin Callbacks
//=============================================================================

static void RegisterEditorUI(EditorUIHooks* hooks, uint64_t hookId)
{
    sHookId = hookId;

    // Register the log window
    hooks->RegisterWindow(hookId, "Debug Log", "debuglog_window", DrawDebugLogWindow, nullptr);

    // Add menu items
    hooks->AddMenuItem(hookId, "View", "Debug Log", OnOpenLogWindow, nullptr, "Ctrl+Shift+L");

    hooks->AddMenuSeparator(hookId, "Developer");
    hooks->AddMenuItem(hookId, "Developer", "Debug Log/Clear Log", OnClearLog, nullptr, nullptr);
    hooks->AddMenuItem(hookId, "Developer", "Debug Log/Test Debug", OnTestDebug, nullptr, nullptr);
    hooks->AddMenuItem(hookId, "Developer", "Debug Log/Test Info", OnTestInfo, nullptr, nullptr);
    hooks->AddMenuItem(hookId, "Developer", "Debug Log/Test Warning", OnTestWarning, nullptr, nullptr);
    hooks->AddMenuItem(hookId, "Developer", "Debug Log/Test Error", OnTestError, nullptr, nullptr);
    hooks->AddMenuItem(hookId, "Developer", "Debug Log/Test Burst (100 messages)", OnTestBurst, nullptr, nullptr);

    // Open the window by default
    hooks->OpenWindow("debuglog_window");

    // Add some initial messages
    Log_Info("Debug Log addon initialized");
    Log_Debug("Ready to capture log messages");

    if (sEngineAPI)
    {
        sEngineAPI->LogDebug("DebugLog: Registered UI hooks");
    }
}

#endif // EDITOR

static int OnLoad(OctaveEngineAPI* api)
{
    sEngineAPI = api;
    api->LogDebug("DebugLog addon loaded!");
    return 0;
}

static void OnUnload()
{
    if (sEngineAPI)
    {
        sEngineAPI->LogDebug("DebugLog addon unloaded.");
    }
    sEngineAPI = nullptr;

#if EDITOR
    std::lock_guard<std::mutex> lock(sLogMutex);
    sLogEntries.clear();
#endif
}

//=============================================================================
// Lua Bindings - Use sEngineAPI->Lua_* wrappers!
//=============================================================================

// Lua headers for type definitions only
extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

static int Lua_DebugLog_Debug(lua_State* L)
{
    // Use sEngineAPI wrapper instead of direct luaL_checkstring
    const char* msg = sEngineAPI->LuaL_checkstring(L, 1);
#if EDITOR
    Log_Debug(msg);
#endif
    return 0;
}

static int Lua_DebugLog_Info(lua_State* L)
{
    const char* msg = sEngineAPI->LuaL_checkstring(L, 1);
#if EDITOR
    Log_Info(msg);
#endif
    return 0;
}

static int Lua_DebugLog_Warning(lua_State* L)
{
    const char* msg = sEngineAPI->LuaL_checkstring(L, 1);
#if EDITOR
    Log_Warning(msg);
#endif
    return 0;
}

static int Lua_DebugLog_Error(lua_State* L)
{
    const char* msg = sEngineAPI->LuaL_checkstring(L, 1);
#if EDITOR
    Log_Error(msg);
#endif
    return 0;
}

static int Lua_DebugLog_Clear(lua_State* L)
{
#if EDITOR
    std::lock_guard<std::mutex> lock(sLogMutex);
    sLogEntries.clear();
    sDebugCount = 0;
    sWarningCount = 0;
    sErrorCount = 0;
#endif
    return 0;
}

static const luaL_Reg sDebugLogFuncs[] = {
    {"Debug", Lua_DebugLog_Debug},
    {"Info", Lua_DebugLog_Info},
    {"Warning", Lua_DebugLog_Warning},
    {"Error", Lua_DebugLog_Error},
    {"Clear", Lua_DebugLog_Clear},
    {nullptr, nullptr}
};

static void RegisterScriptFuncs(lua_State* L)
{
    // Use sEngineAPI wrappers for table creation
    sEngineAPI->LuaL_setfuncs(L, sDebugLogFuncs, 0);
    sEngineAPI->Lua_createtable(L, 0, 5);
    sEngineAPI->LuaL_setfuncs(L, sDebugLogFuncs, 0);
    sEngineAPI->Lua_setglobal(L, "DebugLog");
}

//=============================================================================
// Plugin Entry Point
//=============================================================================

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Debug Log";
    desc->pluginVersion = "1.0.0";
    desc->OnLoad = OnLoad;
    desc->OnUnload = OnUnload;
    desc->Tick = nullptr;          // No gameplay tick needed
    desc->TickEditor = nullptr;    // Window updates via ImGui, no tick needed
    desc->RegisterTypes = nullptr;
    desc->RegisterScriptFuncs = RegisterScriptFuncs;

#if EDITOR
    desc->RegisterEditorUI = RegisterEditorUI;
#else
    desc->RegisterEditorUI = nullptr;
#endif

    return 0;
}
```

---

## Features

### Log Levels
- **Debug** (Gray): Verbose debugging information
- **Info** (Light Blue): General information
- **Warning** (Yellow): Potential issues
- **Error** (Red): Errors that need attention

### Toolbar
- **Clear**: Remove all log entries
- **Collapse**: Combine duplicate messages with a count
- **Auto-scroll**: Keep scrolled to newest messages
- **Time**: Show/hide timestamps
- **Filters**: Toggle visibility of each log level
- **Search**: Filter messages by text

### Context Menu
Right-click any log entry to:
- Copy full message (with timestamp and level)
- Copy message only

---

## Usage

### From the Editor
1. Open via **View > Debug Log** (or Ctrl+Shift+L)
2. The window docks at the bottom by default
3. Use filter checkboxes to show/hide message types
4. Search to find specific messages

### From Lua Scripts

```lua
-- Write to the debug log
DebugLog.Debug("This is a debug message")
DebugLog.Info("Player spawned at position (0, 0, 0)")
DebugLog.Warning("Health is low!")
DebugLog.Error("Failed to load asset: missing_texture.png")

-- Clear all messages
DebugLog.Clear()
```

### Test Messages
Use **Developer > Debug Log > Test ...** menu items to test different message types.

---

## Lua API Reference

### DebugLog.Debug(message)
Logs a debug message (gray).

### DebugLog.Info(message)
Logs an info message (light blue).

### DebugLog.Warning(message)
Logs a warning message (yellow).

### DebugLog.Error(message)
Logs an error message (red).

### DebugLog.Clear()
Clears all log entries.

---

## Customization

### Changing Maximum Log Entries

```cpp
const size_t maxEntries = 1000;  // Increase for more history
```

### Adding Custom Log Levels

```cpp
enum class LogLevel
{
    Debug,
    Warning,
    Error,
    Info,
    Verbose,  // Add new level
    Critical  // Add new level
};
```

### Hooking into Engine Logging

To capture all engine log messages:

```cpp
static int OnLoad(OctaveEngineAPI* api)
{
    // Replace or wrap engine logging functions
    // to capture messages to our log window
}
```

---

## Best Practices

### 1. Use Appropriate Log Levels

```lua
-- Good
DebugLog.Debug("Player position updated")     -- Verbose info
DebugLog.Info("Level loaded: forest_01")      -- Notable events
DebugLog.Warning("Frame rate dropped below 30") -- Potential issues
DebugLog.Error("Failed to save game!")         -- Actual errors

-- Bad
DebugLog.Error("Player moved")  -- Not an error
DebugLog.Debug("CRITICAL FAILURE")  -- Wrong level
```

### 2. Include Context

```lua
-- Good
DebugLog.Error("Asset not found: textures/grass.png")

-- Bad
DebugLog.Error("Asset not found")
```

### 3. Avoid Log Spam

```lua
-- Bad: Logs every frame
function Tick(deltaTime)
    DebugLog.Debug("Ticking...")
end

-- Good: Log only on changes
local lastState = nil
function Tick(deltaTime)
    if state ~= lastState then
        DebugLog.Debug("State changed to: " .. state)
        lastState = state
    end
end
```
