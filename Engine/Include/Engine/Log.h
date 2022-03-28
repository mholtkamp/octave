#pragma once

#include <stdio.h>
#include <glm/glm.hpp>
#include "Constants.h"

void InitializeLog();
void ShutdownLog();

void LockLog();
void UnlockLog();

void WriteConsoleMessage(const char* message, glm::vec4 color = { 1, 1, 1, 1 });

#if LOGGING_ENABLED
#if CONSOLE_ENABLED
#define LogDebug(...) LockLog(); printf( __VA_ARGS__ ); printf("\n"); { char msg[128] = {}; snprintf(msg, 128, __VA_ARGS__); WriteConsoleMessage(msg, {0.5f, 1.0f, 0.5f, 1.0f}); } UnlockLog();
#define LogWarning(...) LockLog(); printf( __VA_ARGS__ ); printf("\n"); { char msg[128] = {}; snprintf(msg, 128, __VA_ARGS__); WriteConsoleMessage(msg, {1.0f, 1.0f, 0.5f, 1.0f}); } UnlockLog();
#define LogError(...) LockLog(); printf( __VA_ARGS__ ); printf("\n"); { char msg[128] = {}; snprintf(msg, 128, __VA_ARGS__); WriteConsoleMessage(msg, {1.0f, 0.5f, 0.5f, 1.0f}); } UnlockLog();
#else // !CONSOLE_ENABLED
#define LogDebug(...) LockLog(); printf( __VA_ARGS__ ); printf("\n"); UnlockLog();
#define LogWarning(...) LockLog(); printf( __VA_ARGS__ ); printf("\n"); UnlockLog();
#define LogError(...) LockLog(); printf( __VA_ARGS__ ); printf("\n"); UnlockLog();
#endif // CONSOLE_ENABLED
#else // !LOGGING_ENABLED
#define LogDebug(...) 
#define LogWarning(...) 
#define LogError(...)
#endif // LOGGING_ENABLED

#if CONSOLE_ENABLED
#define LogConsole(color, ...) { LockLog(); char msg[128] = {}; snprintf(msg, 128, __VA_ARGS__); WriteConsoleMessage(msg, color); UnlockLog(); }
#else
#define LogConsole(...)
#endif
