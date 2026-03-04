#include "Log.h"
#include "Renderer.h"
#include "Nodes/Widgets/Console.h"

#include "System/System.h"

#include "EngineTypes.h"

#include <vector>
#include <algorithm>

static bool sInitialized = false;
static MutexObject* sMutex = nullptr;
static bool sLoggingEnabled = false;

static std::vector<LogCallbackFP> sLogCallbacks;
static bool sDebugLogsInBuildEnabled = true;

static void OpenLogFile()
{
    EngineState* engineState = GetEngineState();

    if (engineState->mLogFile == nullptr)
    {
        std::string projName = engineState->mProjectName;
        if (projName == "")
        {
            projName = "Octave";
        }
        std::string logName = projName + ".log";
        engineState->mLogFile = fopen(logName.c_str(), "w");
    }
}

static void CloseLogFile()
{
    EngineState* engineState = GetEngineState();
    if (engineState->mLogFile != nullptr)
    {
        fclose(engineState->mLogFile);
        engineState->mLogFile = nullptr;
    }
}

void InitializeLog()
{
    if (!sInitialized)
    {
        sMutex = SYS_CreateMutex();

        if (GetEngineConfig()->mLogToFile)
        {
            OpenLogFile();
        }

        sInitialized = true;
    }

#if LOGGING_ENABLED
    sLoggingEnabled = GetEngineConfig()->mLogging;
#else
    sLoggingEnabled = false;
#endif

}

void ShutdownLog()
{
    if (sInitialized)
    {
        sInitialized = false;

        SYS_DestroyMutex(sMutex);
        sMutex = nullptr;

        CloseLogFile();
    }
}

void LogToFile(const char* format, va_list arg)
{
    FILE* logFile = GetEngineState()->mLogFile;
    if (logFile)
    {
        vfprintf(logFile, format, arg);
        fprintf(logFile, "\n");
    }
}

#define LOG_TO_FILE() \
    if (GetEngineConfig()->mLogToFile) \
    { \
        va_list argptr; \
        va_start(argptr, format); \
        LogToFile(format, argptr); \
        va_end(argptr); \
    }

void EnableLog(bool enable)
{
#if LOGGING_ENABLED
    sLoggingEnabled = enable;
#endif
}

bool IsLogEnabled()
{
    return sLoggingEnabled;
}

void LockLog()
{
    if (!sInitialized)
    {
        InitializeLog();
    }

    SYS_LockMutex(sMutex);
}

void UnlockLog()
{
    OCT_ASSERT(sInitialized);
    SYS_UnlockMutex(sMutex);
}

void RegisterLogCallback(LogCallbackFP callback)
{
    if (callback != nullptr)
    {
        sLogCallbacks.push_back(callback);
    }
}

void UnregisterLogCallback(LogCallbackFP callback)
{
    auto it = std::find(sLogCallbacks.begin(), sLogCallbacks.end(), callback);
    if (it != sLogCallbacks.end())
    {
        sLogCallbacks.erase(it);
    }
}

void SetDebugLogsInBuildEnabled(bool enabled)
{
    sDebugLogsInBuildEnabled = enabled;
}

bool IsDebugLogsInBuildEnabled()
{
    return sDebugLogsInBuildEnabled;
}

static void NotifyLogCallbacks(LogSeverity severity, const char* format, va_list args)
{
    if (sLogCallbacks.empty())
        return;

    char buffer[1024] = {};
    vsnprintf(buffer, sizeof(buffer), format, args);

    for (LogCallbackFP cb : sLogCallbacks)
    {
        cb(severity, buffer);
    }
}

void WriteConsoleMessage(glm::vec4 color, const char* format, va_list args)
{
#if CONSOLE_ENABLED
    if (!IsDebugLogsInBuildEnabled())
        return;
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

    if (!sLoggingEnabled)
        return;

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

    {
        va_list argptr;
        va_start(argptr, format);
        NotifyLogCallbacks(LogSeverity::Debug, format, argptr);
        va_end(argptr);
    }

    LOG_TO_FILE();

    UnlockLog();
#endif
}


void LogWarning(const char* format, ...)
{
#if LOGGING_ENABLED

    if (!sLoggingEnabled)
        return;

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

    {
        va_list argptr;
        va_start(argptr, format);
        NotifyLogCallbacks(LogSeverity::Warning, format, argptr);
        va_end(argptr);
    }

    LOG_TO_FILE();

    UnlockLog();
#endif
}

void LogError(const char* format, ...)
{
#if LOGGING_ENABLED

    if (!sLoggingEnabled)
        return;

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

    {
        va_list argptr;
        va_start(argptr, format);
        NotifyLogCallbacks(LogSeverity::Error, format, argptr);
        va_end(argptr);
    }

    LOG_TO_FILE();

    UnlockLog();

#endif
}

void LogConsole(glm::vec4 color, const char* format, ...)
{
#if LOGGING_ENABLED

    if (!sLoggingEnabled)
        return;

    LockLog();

    va_list argptr;
    va_start(argptr, format);

    // Write to in-game console
    WriteConsoleMessage(color, format, argptr);

    va_end(argptr);

    UnlockLog();

#endif
}
