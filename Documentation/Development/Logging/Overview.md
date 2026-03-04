# Logging System Overview

The Octave Engine provides a thread-safe logging system with three severity levels, multiple output channels, and both Lua and C++ interfaces.

## Severity Levels

| Level     | Color                | Purpose                                     |
|-----------|----------------------|---------------------------------------------|
| **Debug** | Green (0.5, 1, 0.5)  | General information, tracing, status updates |
| **Warning** | Yellow (1, 1, 0.5) | Potential issues that don't stop execution   |
| **Error** | Red (1, 0.5, 0.5)    | Failures that may cause incorrect behavior   |

The `LogSeverity` enum is defined in `System/SystemTypes.h`:

```cpp
enum class LogSeverity : uint32_t
{
    Debug,
    Warning,
    Error,
    Count
};
```

## Quick Start

### Lua

```lua
Log.Debug("Player spawned at position " .. tostring(pos))
Log.Warning("Asset not found, using fallback")
Log.Error("Failed to load level: " .. levelName)
```

### C++

```cpp
#include "Log.h"

LogDebug("Player spawned at (%f, %f, %f)", pos.x, pos.y, pos.z);
LogWarning("Asset not found: %s", assetName.c_str());
LogError("Failed to load level: %s", levelName.c_str());
```

## Output Channels

When you call a log function, the message is sent to up to four destinations:

1. **Platform log** - OS-specific output (console window, debug output, etc.) via `SYS_Log()`
2. **In-game console** - The `Console` widget overlay visible at runtime (if `CONSOLE_ENABLED` and debug logs in build are enabled)
3. **Registered callbacks** - Any functions registered via `RegisterLogCallback()` (e.g., the editor's Debug Log window)
4. **Log file** - Written to `{ProjectName}.log` on disk (if `mLogToFile` is enabled in config)

All four outputs happen within a single mutex lock, so messages are delivered atomically.

```
LogDebug/Warning/Error()
    |
    +---> SYS_Log()             --> Platform console/debug output
    +---> WriteConsoleMessage()  --> In-game Console widget
    +---> NotifyLogCallbacks()   --> Registered callbacks (editor, addons)
    +---> LogToFile()            --> {ProjectName}.log file
```

## Thread Safety

All logging functions are **thread-safe**. A global mutex (`sMutex`) is acquired before any output and released after all channels have been written.

```cpp
// These are safe to call from any thread:
LockLog();   // Acquire mutex (auto-initializes if needed)
UnlockLog(); // Release mutex
```

The log functions call `LockLog()` / `UnlockLog()` internally, so you don't need to lock manually when using `LogDebug`, `LogWarning`, or `LogError`.

## Compile-Time Control

Logging can be entirely compiled out by setting `LOGGING_ENABLED` to 0 in `Engine/Source/Engine/Constants.h`:

```cpp
#define LOGGING_ENABLED 1   // Set to 0 to compile out all logging
#define CONSOLE_ENABLED 1   // Set to 0 to compile out console output
```

When `LOGGING_ENABLED` is 0, all `LogDebug()`, `LogWarning()`, and `LogError()` calls become no-ops with zero runtime cost.

## Runtime Control

Even when compiled in, logging can be toggled at runtime:

```lua
Log.Enable(false)  -- Disable all logging
Log.Enable(true)   -- Re-enable logging

if Log.IsEnabled() then
    Log.Debug("Logging is active")
end
```

```cpp
EnableLog(false);
EnableLog(true);
bool active = IsLogEnabled();
```

The initial runtime state is read from `EngineConfig::mLogging` (defaults to `true`).

## Further Reading

- [Lua API Reference](LuaAPI.md) - All Lua logging functions
- [C++ API Reference](CppAPI.md) - All C++ logging functions, callbacks, thread safety
- [Editor Debug Log Window](EditorDebugLog.md) - The editor's log viewer panel
- [Configuration](Configuration.md) - Engine config, preferences, and log file settings
