#pragma once

#include "OctaveAPI.h"
#include <stdarg.h>
#include <stdio.h>
#include "Maths.h"
#include "Constants.h"

void InitializeLog();
void ShutdownLog();

OCTAVE_API void EnableLog(bool enable);
OCTAVE_API bool IsLogEnabled();

void LockLog();
void UnlockLog();

OCTAVE_API void LogDebug(const char* format, ...);
OCTAVE_API void LogWarning(const char* format, ...);
OCTAVE_API void LogError(const char* format, ...);
OCTAVE_API void LogConsole(glm::vec4 color, const char* format, ...);

#include "System/SystemTypes.h"
typedef void(*LogCallbackFP)(LogSeverity severity, const char* message);
void RegisterLogCallback(LogCallbackFP callback);
void UnregisterLogCallback(LogCallbackFP callback);
void SetDebugLogsInBuildEnabled(bool enabled);
bool IsDebugLogsInBuildEnabled();
