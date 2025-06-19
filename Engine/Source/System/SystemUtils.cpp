#include "SystemUtils.h"
#include "Log.h"

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
