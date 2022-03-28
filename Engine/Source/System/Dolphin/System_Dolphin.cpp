#if PLATFORM_DOLPHIN

#include "System/System.h"

#include "Engine.h"
#include "Renderer.h"
#include "Log.h"
#include "Input/Input.h"
#include "Constants.h"

#include <gccore.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <fat.h>
#include <ogc/lwp_watchdog.h>

void SYS_Initialize()
{
    EngineState& engine = *GetEngineState();
    SystemState& system = engine.mSystem;

    system.mFrameIndex = 0;

    VIDEO_Init();
    GXRModeObj* rmode = VIDEO_GetPreferredMode(NULL);
    system.mGxrMode = rmode;
    engine.mWindowWidth = rmode->fbWidth;
    engine.mWindowHeight = rmode->efbHeight;

    // Initialize the console, required for printf
    system.mConsoleBuffer = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

    // allocate 2 framebuffers for double buffering
    system.mFrameBuffers[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    system.mFrameBuffers[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

    VIDEO_Configure(system.mGxrMode);
    VIDEO_SetNextFramebuffer(system.mFrameBuffers[system.mFrameIndex]);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();

    if (system.mGxrMode->viTVMode & VI_NON_INTERLACE)
    {
        VIDEO_WaitVSync();
    }

    console_init(system.mConsoleBuffer, 20, 20, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);

    //printf("\x1b[2;0H");

    if (fatInitDefault())
    {
        LogDebug("FAT Initialized Successfully.\n");
    }
    else
    {
        LogDebug("fatInitDefault() failure.\n");
    }
}

void SYS_Shutdown()
{

}

void SYS_Update()
{

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
    ThreadHandle retThread;

    int32_t createStatus = 0;

    createStatus = LWP_CreateThread(
        &retThread,     /* thread handle */
        func,           /* code */
        arg,            /* arg pointer for thread */
        nullptr,        /* stack base */
        16 * 1024,      /* stack size */
        50              /* thread priority */);

    if (createStatus < 0)
    {
        LogError("Failed to create Thread");
    }

    return retThread;
}

void SYS_JoinThread(ThreadHandle thread)
{
    void* retValue = nullptr;
    LWP_JoinThread(thread, &retValue);
}

void SYS_DestroyThread(ThreadHandle thread)
{
    // Not sure if this is needed?
}

MutexHandle SYS_CreateMutex()
{
    MutexHandle retHandle;

    // Pass true in second param to allow recursive locking
    int32_t status = LWP_MutexInit(&retHandle, true);

    if (status < 0)
    {
        LogError("Failed to create Mutex");
    }

    return retHandle;
}

void SYS_LockMutex(MutexHandle mutex)
{
    LWP_MutexLock(mutex);
}

void SYS_UnlockMutex(MutexHandle mutex)
{
    LWP_MutexUnlock(mutex);
}

void SYS_DestroyMutex(MutexHandle mutex)
{
    LWP_MutexDestroy(mutex);
}

void SYS_Sleep(uint32_t milliseconds)
{
    // Uh... not sure how to sleep for a given duration.
    // But this sleep function at least yields to other threads I think...
    ThreadHandle thisThread = LWP_GetSelf();
    LWP_ThreadSleep(thisThread);

    OCT_UNUSED(milliseconds);
}

// Time
uint64_t SYS_GetTimeMicroseconds()
{
    // There is another function called gettick(), and im not quite sure
    // what the difference is, but gettick() returns only a u32 and I got really 
    // weird results when I used it instead of gettime().
    return ticks_to_microsecs(gettime());
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
    uint64_t retBytes = SYS_GetArena1Size();

#if PLATFORM_WII
    retBytes += SYS_GetArena2Size();
#endif

    return retBytes;
}

uint64_t SYS_GetNumBytesAllocated()
{
    uint64_t totalMem = 24 * 1024 * 1024;
    return totalMem - SYS_GetNumBytesFree();
}

// Misc
void SYS_UpdateConsole()
{
    SystemState& system = GetEngineState()->mSystem;
    void* fb = GetEngineState()->mConsoleMode ? system.mConsoleBuffer : system.mFrameBuffers[system.mFrameIndex];

    VIDEO_SetNextFramebuffer(fb);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();
}

int32_t SYS_GetPlatformTier()
{
    return 1;
}

void SYS_SetWindowTitle(const char* title)
{
    
}

#endif