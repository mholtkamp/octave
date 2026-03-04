#include "SystemUtils.h"
#include "System.h"
#include "Log.h"

#include <cstdio>
#include <cstdlib>

void ExecCommon(const char* cmd, std::string* output)
{
#if PLATFORM_WINDOWS
#define popen _popen
#define pclose _pclose
#endif

    LogDebug("[Exec] %s", cmd);

    if (output != nullptr)
    {
        FILE* file = popen(cmd, "r");

        if (file != nullptr)
        {
            char buffer[1024];
            if (fgets(buffer, 1024, file))
            {
                *output = buffer;
            }

            // Strip newlines
            std::string& str = *output;
            for (int32_t i = int32_t(str.size()) - 1; i >= 0; --i)
            {
                if (str[i] == '\n' ||
                    str[i] == '\r')
                {
                    str.erase(str.begin() + i);
                }
            }

            LogDebug(" >> %s", output->c_str());

            pclose(file);
            file = nullptr;
        }
        else
        {
            LogError("Failed to run command");
        }
    }
    else
    {
        system(cmd);
    }

#if PLATFORM_WINDOWS
#undef popen
#undef pclose
#endif
}

bool SYS_ExecFull(const char* cmd, std::string* outStdout, std::string* outStderr, int* outExitCode)
{
#if PLATFORM_WINDOWS
#define popen _popen
#define pclose _pclose
#endif

    LogDebug("[ExecFull] %s", cmd);

    // Build command that captures both stdout and stderr
    // On Windows, we redirect stderr to stdout with 2>&1
    // On Linux, same approach works
    std::string fullCmd = cmd;
    fullCmd += " 2>&1";

    FILE* file = popen(fullCmd.c_str(), "r");
    if (file == nullptr)
    {
        LogError("Failed to run command: %s", cmd);
        if (outExitCode != nullptr)
        {
            *outExitCode = -1;
        }
        return false;
    }

    // Read all output
    std::string output;
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), file) != nullptr)
    {
        output += buffer;
    }

    int status = pclose(file);

#if PLATFORM_WINDOWS || PLATFORM_3DS || PLATFORM_DOLPHIN
    int exitCode = status;
#else
    int exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
#endif

    // Set output parameters
    if (outStdout != nullptr)
    {
        *outStdout = output;
    }

    // stderr is combined with stdout due to 2>&1 redirect
    if (outStderr != nullptr)
    {
        outStderr->clear();
    }

    if (outExitCode != nullptr)
    {
        *outExitCode = exitCode;
    }

    LogDebug("[ExecFull] Exit code: %d, Output length: %zu", exitCode, output.size());

    return exitCode == 0;

#if PLATFORM_WINDOWS
#undef popen
#undef pclose
#endif
}
