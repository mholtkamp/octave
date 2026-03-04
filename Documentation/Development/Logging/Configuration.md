# Logging - Configuration

Logging behavior is controlled at three levels: compile-time macros, engine configuration, and editor preferences.

## Compile-Time Macros

Defined in `Engine/Source/Engine/Constants.h`:

| Macro | Default | Effect |
|-------|---------|--------|
| `LOGGING_ENABLED` | `1` | Master switch. When `0`, all `LogDebug`/`LogWarning`/`LogError` calls compile to nothing. |
| `CONSOLE_ENABLED` | `1` | When `0`, log messages are not written to the in-game Console widget. |

Setting `LOGGING_ENABLED` to `0` removes all logging overhead from the binary. This is useful for final release builds where maximum performance is needed.

## Engine Configuration

The `EngineConfig` struct (in `Engine/Source/Engine/EngineTypes.h`) has two logging-related fields:

### mLogging

```cpp
bool mLogging = true;
```

Controls whether logging is enabled at startup. This is read by `InitializeLog()` to set the initial runtime state. Can be overridden at runtime via `EnableLog()` / `Log.Enable()`.

### mLogToFile

```cpp
bool mLogToFile = false;
```

When `true`, all log messages are also written to a file on disk:

- **File name**: `{ProjectName}.log` (or `Octave.log` if no project name is set)
- **Location**: The working directory
- **Mode**: Write (`"w"`) - the file is overwritten each time the engine starts
- **Format**: One message per line, no timestamps or severity prefixes in the file

Each log message is written as:
```
{formatted message}\n
```

The log file is opened during `InitializeLog()` and closed during `ShutdownLog()`.

## Editor Preferences

The editor's **General** preferences module (under Preferences > General) exposes two logging-related settings:

### Show Debug Log In Editor

- **Default**: `true`
- **JSON key**: `"showDebugInEditor"`
- **Effect**: Toggles visibility of the Debug Log panel in the editor UI

When disabled, the Debug Log window is not drawn. Log messages are still collected in the background.

### Show Debug Logs In Build

- **Default**: `true`
- **JSON key**: `"showDebugLogsInBuild"`
- **Effect**: Controls whether log messages appear in the in-game Console widget at runtime

This maps to `SetDebugLogsInBuildEnabled()` / `IsDebugLogsInBuildEnabled()`. When disabled, `LogDebug`/`LogWarning`/`LogError` still output to the platform log, callbacks, and log file - only the in-game Console widget output is suppressed.

Useful for hiding debug output in published builds while still capturing logs through other channels.

## Summary

| Setting | Scope | Controls |
|---------|-------|----------|
| `LOGGING_ENABLED` | Compile-time | Whether log functions exist at all |
| `CONSOLE_ENABLED` | Compile-time | Whether in-game console output exists |
| `EngineConfig::mLogging` | Startup | Initial runtime enabled state |
| `EngineConfig::mLogToFile` | Startup | Whether to write a .log file |
| `EnableLog()` / `Log.Enable()` | Runtime | Toggle logging on/off |
| "Show Debug Log In Editor" | Editor pref | Debug Log panel visibility |
| "Show Debug Logs In Build" | Editor pref | In-game Console widget output |

## Platform-Specific Logging

The `SYS_Log()` function (declared in `System/System.h`) is implemented per-platform:

```cpp
void SYS_Log(LogSeverity severity, const char* format, va_list arg);
```

Each platform implementation decides where platform-level log output goes (e.g., `OutputDebugString` on Windows, `printf` on Linux/GameCube, etc.).

The mutex used for thread safety (`MutexObject`) is also platform-specific:

| Platform | Mutex Type |
|----------|------------|
| Windows | `HANDLE` (WinAPI) |
| Linux / Android | `pthread_mutex_t` |
| GameCube / Wii | `mutex_t` |
| 3DS | `uint32_t` |
