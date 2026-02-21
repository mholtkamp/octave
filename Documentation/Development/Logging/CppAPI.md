# Logging - C++ API

Include `Log.h` for all logging functionality.

```cpp
#include "Log.h"
```

## Core Log Functions

### LogDebug(format, ...)

Logs a debug-level message using `printf`-style format strings.

```cpp
LogDebug("Player spawned");
LogDebug("Score: %d", score);
LogDebug("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
```

### LogWarning(format, ...)

Logs a warning-level message.

```cpp
LogWarning("Asset '%s' not found, using fallback", assetName.c_str());
LogWarning("Frame time exceeded threshold: %.1fms", frameTime * 1000.0f);
```

### LogError(format, ...)

Logs an error-level message.

```cpp
LogError("Failed to open file: %s", path.c_str());
LogError("Null pointer in %s at line %d", __FUNCTION__, __LINE__);
```

### LogConsole(color, format, ...)

Logs a message directly to the in-game Console widget with a custom color. Does not trigger callbacks or write to the log file.

```cpp
LogConsole({0.0f, 1.0f, 1.0f, 1.0f}, "Cyan message");
LogConsole({1.0f, 0.5f, 0.0f, 1.0f}, "Score: %d", score);
```

The color is a `glm::vec4` with RGBA components.

> **Note**: The C++ parameter order is `(color, format, ...)`, which differs from the Lua `Log.Console(message, color)`.

## Runtime Enable/Disable

### EnableLog(bool enable)

```cpp
EnableLog(false);  // Suppress all logging
EnableLog(true);   // Resume logging
```

Only effective when `LOGGING_ENABLED` is 1.

### IsLogEnabled()

```cpp
if (IsLogEnabled())
{
    LogDebug("Verbose: %s", expensiveToString().c_str());
}
```

## Thread Safety

### LockLog() / UnlockLog()

The log functions lock internally, so you do not need to call these for normal logging. They are exposed for advanced use cases where you need to group operations under the log mutex.

```cpp
LockLog();
// ... critical section ...
UnlockLog();
```

`LockLog()` auto-initializes the mutex if `InitializeLog()` hasn't been called yet.

## Log Callback System

Register functions to receive all log messages. This is how the editor Debug Log window receives entries.

### Callback Signature

```cpp
typedef void(*LogCallbackFP)(LogSeverity severity, const char* message);
```

The callback receives:
- `severity` - The `LogSeverity` enum value (`Debug`, `Warning`, or `Error`)
- `message` - The fully formatted message string (up to 1024 characters)

### RegisterLogCallback(callback)

```cpp
void MyLogHandler(LogSeverity severity, const char* message)
{
    // Handle log message
    if (severity == LogSeverity::Error)
    {
        // Alert on errors
    }
}

// Register during initialization
RegisterLogCallback(MyLogHandler);
```

### UnregisterLogCallback(callback)

```cpp
// Clean up when done
UnregisterLogCallback(MyLogHandler);
```

**Important**: Callbacks are invoked **while the log mutex is held**. Do not call `LogDebug`/`LogWarning`/`LogError` from within a callback, as this will deadlock. If you need to buffer messages for later processing, copy the data and process it outside the callback (see the `DebugLogWindow` pattern in [Editor Debug Log](EditorDebugLog.md)).

### Callback Buffer Size

Messages passed to callbacks are formatted into a 1024-byte buffer (`vsnprintf`). Messages exceeding this length will be truncated.

## Debug Logs In Build

### SetDebugLogsInBuildEnabled(bool enabled)

Controls whether log messages appear in the in-game Console widget at runtime. When `false`, `LogDebug`/`LogWarning`/`LogError` still output to the platform log, callbacks, and file - but not to the Console widget.

```cpp
SetDebugLogsInBuildEnabled(false);  // Hide console messages in builds
```

### IsDebugLogsInBuildEnabled()

```cpp
bool showInConsole = IsDebugLogsInBuildEnabled();
```

This is tied to the editor preference "Show Debug Logs In Build" (see [Configuration](Configuration.md)).

## Initialization / Shutdown

These are called by the engine automatically. You typically do not need to call them directly.

### InitializeLog()

- Creates the log mutex
- Opens the log file if `EngineConfig::mLogToFile` is true
- Sets initial logging state from `EngineConfig::mLogging`

### ShutdownLog()

- Destroys the mutex
- Closes the log file

## Console Message Limits

Messages written to the in-game Console widget are truncated to **128 characters** (`vsnprintf` with 128-byte buffer in `WriteConsoleMessage`). The platform log and callbacks receive the full message.

## Key Source Files

| File | Purpose |
|------|---------|
| `Engine/Source/Engine/Log.h` | Public API declarations |
| `Engine/Source/Engine/Log.cpp` | Implementation |
| `Engine/Source/Engine/Constants.h` | `LOGGING_ENABLED`, `CONSOLE_ENABLED` macros |
| `Engine/Source/System/SystemTypes.h` | `LogSeverity` enum, `MutexObject` type |
| `Engine/Source/System/System.h` | `SYS_Log()`, `SYS_CreateMutex()`, etc. |
