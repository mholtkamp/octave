# Editor Debug Log Window

The Debug Log is an editor panel that displays engine log messages in real time with filtering, searching, and copy support. It is only available in editor builds (`#if EDITOR`).

## Overview

The Debug Log window connects to the engine's logging system via `RegisterLogCallback()` and displays all `LogDebug`, `LogWarning`, and `LogError` messages in a scrollable, filterable list.

- **Max entries**: 2048 (oldest entries are discarded when the limit is reached)
- **Thread-safe**: Log messages from any thread are buffered and drained on the main thread
- **Color-coded**: Debug (light gray), Warning (yellow), Error (red)
- **Timestamps**: Displayed as `[HH:MM:SS]` based on seconds since engine start

## Toolbar

The toolbar at the top of the Debug Log window provides:

| Button | Action |
|--------|--------|
| Trash icon | Clear all log entries |
| Copy icon | Copy all visible entries to clipboard |
| Search icon | Toggle the search bar |
| Bug icon (green) | Toggle Debug message visibility |
| Warning icon (yellow) | Toggle Warning message visibility |
| Error icon (red) | Toggle Error message visibility |
| Arrow icon | Toggle auto-scroll to latest messages |

## Filtering

Click the severity toggle checkboxes to show or hide messages by level:

- **Debug** (green bug icon) - General debug messages, on by default
- **Warning** (yellow warning icon) - Warning messages, on by default
- **Error** (red error icon) - Error messages, on by default

Filtering applies to both the log display and the clipboard copy. Hidden messages are excluded from "Copy All".

## Search

Click the search icon (or toggle it) to open the search bar. Search is **case-insensitive** and matches against the message text.

- Type in the search field to find matching entries
- Use **<** and **>** buttons to navigate between matches
- The match counter shows `N/M` (current match / total matches)
- The current match is highlighted with a brighter background
- Other matches get a dimmer highlight
- Press **Escape** or click **X** to close the search bar

## Context Menu

Right-click any log entry for a context menu with:

- **Copy** - Copy the selected entry text to clipboard
- **Copy All** - Copy all visible entries to clipboard
- Additional items from registered editor UI hooks (via `EditorUIHookManager`)

## Threading Model

The Debug Log uses a double-buffered approach for thread safety:

1. `DebugLogWindow::LogCallback()` is called from whatever thread produces the log message (under the engine's log mutex)
2. The callback adds entries to `mPendingEntries` protected by its own `std::mutex`
3. On the main thread, `DrainPendingEntries()` moves pending entries to the display list `mEntries`
4. The display list is only accessed from the main thread

This means the callback never blocks on ImGui rendering, and the main thread never blocks on log production.

## Visibility Control

The Debug Log panel's visibility is controlled by the editor preference **"Show Debug Log In Editor"** in the General preferences module. See [Configuration](Configuration.md).

## Key Source Files

| File | Purpose |
|------|---------|
| `Engine/Source/Editor/DebugLog/DebugLogWindow.h` | Class declaration, `DebugLogEntry` struct |
| `Engine/Source/Editor/DebugLog/DebugLogWindow.cpp` | Full implementation |
| `Engine/Source/Editor/EditorImgui.cpp` | Registers the log callback during editor init |

## Access from Code

```cpp
#include "Editor/DebugLog/DebugLogWindow.h"

// Get the singleton window instance
DebugLogWindow* logWindow = GetDebugLogWindow();

// Programmatically clear
logWindow->Clear();

// Change filter state
logWindow->mShowDebug = false;    // Hide debug messages
logWindow->mShowWarnings = true;
logWindow->mShowErrors = true;
logWindow->mAutoScroll = true;
```
