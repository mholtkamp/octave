#if PLATFORM_3DS

#include "System/System.h"

#include "Engine.h"
#include "Renderer.h"
#include "Log.h"
#include "Input/Input.h"
#include "Constants.h"

#include <3ds.h>
#include <citro3d.h>

#include <unistd.h>
#include <malloc.h>

#define ENABLE_SYSTEM_CONSOLE 0

void SYS_Initialize()
{
    // Initialize graphics
    gfxInitDefault();
    gfxSet3D(true); // Enable stereoscopic 3D

    EngineState* engine = GetEngineState();

    engine->mWindowWidth = 400;
    engine->mWindowHeight = 240;

    engine->mSecondWindowWidth = 320;
    engine->mSecondWindowHeight = 240;

#if ENABLE_SYSTEM_CONSOLE
    consoleInit(GFX_BOTTOM, &GetEngineState()->mSystem.mPrintConsole);
    consoleSelect(&GetEngineState()->mSystem.mPrintConsole);
    printf("Console initialized");
#endif

    APT_CheckNew3DS(&(GetEngineState()->mSystem.mNew3DS));

    osSetSpeedupEnable(true);
}

void SYS_Shutdown()
{
    gfxExit();
}

void SYS_Update()
{
    SystemState& system = GetEngineState()->mSystem;
    system.mSlider = osGet3DSliderState();

    GetEngineState()->mQuit = !aptMainLoop();
}

// Files
std::string SYS_GetCurrentDirectoryPath()
{
    char path[MAX_PATH_SIZE] = {};
    getcwd(path, MAX_PATH_SIZE);
    return std::string(path) + "/";
}

void SYS_SetWorkingDirectory(const std::string& dirPath)
{
    chdir(dirPath.c_str());
}

bool SYS_CreateDirectory(const char* dirPath)
{
    return (mkdir(dirPath, 0777) == 0);
}

void SYS_RemoveDirectory(const char* dirPath)
{

}

void SYS_RemoveFile(const char* path)
{
    remove(path);
}

bool SYS_Rename(const char* oldPath, const char* newPath)
{
    return (rename(oldPath, newPath) == 0);
}

void SYS_OpenDirectory(const std::string& dirPath, DirEntry& outDirEntry)
{
    strncpy(outDirEntry.mDirectoryPath, dirPath.c_str(), MAX_PATH_SIZE);

    outDirEntry.mDir = opendir(dirPath.c_str());
    if (outDirEntry.mDir == nullptr)
    {
        LogError("Could not open directory.");
        closedir(outDirEntry.mDir);
        return;
    }

    dirent* ent = readdir(outDirEntry.mDir);
    if (ent == nullptr)
    {
        outDirEntry.mValid = false;
    }
    else
    {
        memcpy(outDirEntry.mFilename, ent->d_name, MAX_PATH_SIZE);

        struct stat statbuf;
        std::string fullPath = dirPath + outDirEntry.mFilename;
        stat(fullPath.c_str(), &statbuf);

        outDirEntry.mDirectory = S_ISDIR(statbuf.st_mode);
        outDirEntry.mValid = true;
    }
}

void SYS_IterateDirectory(DirEntry& dirEntry)
{
    dirent* ent = readdir(dirEntry.mDir);
    if (ent == nullptr)
    {
        dirEntry.mValid = false;
    }
    else
    {
        memcpy(dirEntry.mFilename, ent->d_name, MAX_PATH_SIZE);

        struct stat statbuf;
        std::string fullPath = std::string(dirEntry.mDirectoryPath) + dirEntry.mFilename;
        stat(fullPath.c_str(), &statbuf);

        dirEntry.mDirectory = S_ISDIR(statbuf.st_mode);
        dirEntry.mValid = true;
    }
}

void SYS_CloseDirectory(DirEntry& dirEntry)
{
    closedir(dirEntry.mDir);
    dirEntry.mDir = nullptr;
}

std::string SYS_OpenFileDialog()
{
    return "";
}

std::string SYS_SaveFileDialog()
{
    return "";
}

std::string SYS_SelectFolderDialog()
{
    return "";
}

// Threads
ThreadHandle SYS_CreateThread(ThreadFuncFP func, void* arg)
{
    int32_t priority = 0x30;
    svcGetThreadPriority(&priority, CUR_THREAD_HANDLE);
    priority -= 1;
    priority = priority < 0x18 ? 0x18 : priority;
    priority = priority > 0x3F ? 0x3F : priority;

    ThreadHandle retThread = threadCreate(
        func,
        arg,
        16 * 1024,
        priority,
        -1,
        false);

    if (retThread == 0)
    {
        LogError("Failed to create Thread");
    }

    return retThread;
}

void SYS_JoinThread(ThreadHandle thread)
{
    threadJoin(thread, UINT64_MAX);
}

void SYS_DestroyThread(ThreadHandle thread)
{
    threadFree(thread);
}

MutexHandle SYS_CreateMutex()
{
    MutexHandle retMutex = 0;

    int32_t result = svcCreateMutex(&retMutex, false);

    if (result < 0)
    {
        LogError("Failed to create Mutex");
    }

    return retMutex;
}

void SYS_LockMutex(MutexHandle mutex)
{
    int32_t result = svcWaitSynchronization(mutex, UINT64_MAX);

    if (result < 0)
    {
        LogError("Error locking mutex");
    }
}

void SYS_UnlockMutex(MutexHandle mutex)
{
    if (svcReleaseMutex(mutex) < 0)
    {
        LogError("Error releasing mutex");
    }
}

void SYS_DestroyMutex(MutexHandle mutex)
{
    svcCloseHandle(mutex);
}

void SYS_Sleep(uint32_t milliseconds)
{
    svcSleepThread(milliseconds * 1000 * 1000);
}

// Time
uint64_t SYS_GetTimeMicroseconds()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t microseconds = 1000000 * tv.tv_sec + tv.tv_usec;
    return microseconds;
}

// Process
void SYS_Exec(const char* cmd, std::string* output)
{

}

// Memory
void* SYS_AlignedMalloc(uint32_t size, uint32_t alignment)
{
    return memalign(alignment, size);
}

void SYS_AlignedFree(void* pointer)
{
    assert(pointer != nullptr);
    free(pointer);
}

uint64_t SYS_GetNumBytesFree()
{
    return linearSpaceFree();
}

uint64_t SYS_GetNumBytesAllocated()
{
    return 0;
}

// Misc
void SYS_UpdateConsole()
{
#if 0
    // How do you disable the console??
    if (GetEngineState()->mConsoleMode)
    {
        LogDebug("Enabling console");
        consoleSelect(&GetEngineState()->mSystem.mPrintConsole);
    }
    else
    {
        LogDebug("Disabling console");
        consoleSelect(nullptr);
    }
#endif
}

int32_t SYS_GetPlatformTier()
{
    if (GetEngineState()->mSystem.mNew3DS)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void SYS_SetWindowTitle(const char* title)
{

}

#endif