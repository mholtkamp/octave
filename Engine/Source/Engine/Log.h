#pragma once

#include <stdarg.h>
#include <stdio.h>
#include "Maths.h"
#include "Constants.h"

void InitializeLog();
void ShutdownLog();

void EnableLog(bool enable);
bool IsLogEnabled();

void LockLog();
void UnlockLog();

void LogDebug(const char* format, ...);
void LogWarning(const char* format, ...);
void LogError(const char* format, ...);
void LogConsole(glm::vec4 color, const char* format, ...);

#include "System/SystemTypes.h"
typedef void(*LogCallbackFP)(LogSeverity severity, const char* message);
void RegisterLogCallback(LogCallbackFP callback);
void UnregisterLogCallback(LogCallbackFP callback);
void SetDebugLogsInBuildEnabled(bool enabled);
bool IsDebugLogsInBuildEnabled();
