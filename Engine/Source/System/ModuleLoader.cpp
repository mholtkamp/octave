#if EDITOR

#include "ModuleLoader.h"
#include "Log.h"

#include <string>

#if PLATFORM_WINDOWS

#include <Windows.h>

static std::string sLastError;

void* MOD_Load(const char* path)
{
    sLastError.clear();

    HMODULE module = LoadLibraryA(path);
    if (module == nullptr)
    {
        DWORD error = GetLastError();
        char buffer[256];
        FormatMessageA(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            buffer,
            sizeof(buffer),
            nullptr
        );
        sLastError = buffer;
        LogError("MOD_Load failed for '%s': %s", path, sLastError.c_str());
    }

    return static_cast<void*>(module);
}

void* MOD_Symbol(void* handle, const char* name)
{
    sLastError.clear();

    if (handle == nullptr)
    {
        sLastError = "Invalid module handle";
        return nullptr;
    }

    FARPROC proc = GetProcAddress(static_cast<HMODULE>(handle), name);
    if (proc == nullptr)
    {
        DWORD error = GetLastError();
        char buffer[256];
        FormatMessageA(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            buffer,
            sizeof(buffer),
            nullptr
        );
        sLastError = buffer;
    }

    return reinterpret_cast<void*>(proc);
}

void MOD_Unload(void* handle)
{
    sLastError.clear();

    if (handle != nullptr)
    {
        if (!FreeLibrary(static_cast<HMODULE>(handle)))
        {
            DWORD error = GetLastError();
            char buffer[256];
            FormatMessageA(
                FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr,
                error,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                buffer,
                sizeof(buffer),
                nullptr
            );
            sLastError = buffer;
            LogWarning("MOD_Unload failed: %s", sLastError.c_str());
        }
    }
}

const char* MOD_GetError()
{
    return sLastError.c_str();
}

#elif PLATFORM_LINUX

#include <dlfcn.h>

static std::string sLastError;

void* MOD_Load(const char* path)
{
    sLastError.clear();

    void* handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (handle == nullptr)
    {
        const char* error = dlerror();
        sLastError = error ? error : "Unknown error";
        LogError("MOD_Load failed for '%s': %s", path, sLastError.c_str());
    }

    return handle;
}

void* MOD_Symbol(void* handle, const char* name)
{
    sLastError.clear();

    if (handle == nullptr)
    {
        sLastError = "Invalid module handle";
        return nullptr;
    }

    // Clear any existing error
    dlerror();

    void* symbol = dlsym(handle, name);

    const char* error = dlerror();
    if (error != nullptr)
    {
        sLastError = error;
        symbol = nullptr;
    }

    return symbol;
}

void MOD_Unload(void* handle)
{
    sLastError.clear();

    if (handle != nullptr)
    {
        if (dlclose(handle) != 0)
        {
            const char* error = dlerror();
            sLastError = error ? error : "Unknown error";
            LogWarning("MOD_Unload failed: %s", sLastError.c_str());
        }
    }
}

const char* MOD_GetError()
{
    return sLastError.c_str();
}

#else

// Stub implementations for unsupported platforms
static const char* sUnsupportedError = "Module loading not supported on this platform";

void* MOD_Load(const char* path)
{
    LogError("%s", sUnsupportedError);
    return nullptr;
}

void* MOD_Symbol(void* handle, const char* name)
{
    return nullptr;
}

void MOD_Unload(void* handle)
{
}

const char* MOD_GetError()
{
    return sUnsupportedError;
}

#endif // Platform

#endif // EDITOR
