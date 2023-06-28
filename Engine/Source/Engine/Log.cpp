#include "Log.h"
#include "Renderer.h"
#include "Widgets/Console.h"

#include "System/System.h"

#include "EngineTypes.h"

static bool sInitialized = false;
static MutexObject* sMutex = nullptr;

void InitializeLog()
{
    sMutex = SYS_CreateMutex();
    sInitialized = true;
}

void ShutdownLog()
{
    sInitialized = false;
    SYS_DestroyMutex(sMutex);
    sMutex = nullptr;
}

void LockLog()
{
    OCT_ASSERT(sInitialized);
    SYS_LockMutex(sMutex);
}

void UnlockLog()
{
    OCT_ASSERT(sInitialized);
    SYS_UnlockMutex(sMutex);
}

void WriteConsoleMessage(glm::vec4 color, const char* format, va_list args)
{
#if CONSOLE_ENABLED
    char msg[128] = {};
    vsnprintf(msg, 128, format, args);

    Renderer* renderer = Renderer::Get();
    Console* console = renderer ? renderer->GetConsoleWidget() : nullptr;

    if (console != nullptr)
    {
        console->WriteOutput(msg, color);
    }
#endif
}


void LogDebug(const char* format, ...)
{
#if LOGGING_ENABLED
    LockLog();

    {
        va_list argptr;
        va_start(argptr, format);

        // Pass to SYS interface
        SYS_Log(LogSeverity::Debug, format, argptr);
        va_end(argptr);
    }

    {
        va_list argptr;
        va_start(argptr, format);

        // Write to in-game console
        WriteConsoleMessage({0.5f, 1.0f, 0.5f, 1.0f}, format, argptr);

        va_end(argptr);
    }
    
    UnlockLog();
#endif
}


void LogWarning(const char* format, ...)
{
#if LOGGING_ENABLED
    LockLog();

    {
        va_list argptr;
        va_start(argptr, format);

        // Pass to SYS interface
        SYS_Log(LogSeverity::Warning, format, argptr);
        va_end(argptr);
    }

    {
        va_list argptr;
        va_start(argptr, format);

        // Write to in-game console
        WriteConsoleMessage({1.0f, 1.0f, 0.5f, 1.0f}, format, argptr);

        va_end(argptr);
    }
    
    UnlockLog();
#endif
}

void LogError(const char* format, ...)
{
#if LOGGING_ENABLED

    LockLog();

    {
        va_list argptr;
        va_start(argptr, format);

        // Pass to SYS interface
        SYS_Log(LogSeverity::Error, format, argptr);
        va_end(argptr);
    }

    {
        va_list argptr;
        va_start(argptr, format);

        // Write to in-game console
        WriteConsoleMessage({1.0f, 0.5f, 0.5f, 1.0f}, format, argptr);

        va_end(argptr);
    }
    
    UnlockLog();

#endif
}

void LogConsole(glm::vec4 color, const char* format, ...)
{
#if LOGGING_ENABLED

    LockLog();

    va_list argptr;
    va_start(argptr, format);

    // Write to in-game console
    WriteConsoleMessage(color, format, argptr);

    va_end(argptr);

    UnlockLog();

#endif
}
