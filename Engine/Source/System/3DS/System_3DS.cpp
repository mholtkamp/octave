#if PLATFORM_3DS

#include "System/System.h"

#include "Engine.h"
#include "Renderer.h"
#include "Log.h"
#include "Input/Input.h"
#include "InputDevices.h"
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
bool SYS_DoesFileExist(const char* path, bool isAsset)
{
    struct stat info;
    bool exists = false;

    int32_t retStatus = stat(path, &info);

    if (retStatus == 0)
    {
        // If the file is actually a directory, than return false.
        exists = !(info.st_mode & S_IFDIR);
    }

    return exists;
}

void SYS_AcquireFileData(const char* path, bool isAsset, int32_t maxSize, char*& outData, uint32_t& outSize)
{
    outData = nullptr;
    outSize = 0;

    FILE* file = fopen(path, "rb");

    if (file != nullptr)
    {
        int32_t fileSize = 0;
        fseek(file, 0, SEEK_END);
        fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        if (maxSize > 0)
        {
            fileSize = glm::min(fileSize, maxSize);
        }

        outData = (char*)malloc(fileSize);
        outSize = uint32_t(fileSize);
        fread(outData, fileSize, 1, file);

        fclose(file);
        file = nullptr;
    }
    else
    {
        LogError("Failed to open file: %s", path);
    }
}

void SYS_ReleaseFileData(char* data)
{
    if (data != nullptr)
    {
        free(data);
    }
}

std::string SYS_GetCurrentDirectoryPath()
{
    char path[MAX_PATH_SIZE] = {};
    getcwd(path, MAX_PATH_SIZE);
    return std::string(path) + "/";
}

std::string SYS_GetAbsolutePath(const std::string& relativePath)
{
    std::string absPath;
    char* resolvedPath = realpath(relativePath.c_str(), nullptr);
    if (resolvedPath != nullptr)
    {
        absPath = resolvedPath;
        free(resolvedPath);
    }

    if (absPath != "" && DoesDirExist(absPath.c_str()))
        absPath += "/";

    return absPath;
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

std::vector<std::string> SYS_OpenFileDialog()
{
    return {};
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
ThreadObject* SYS_CreateThread(ThreadFuncFP func, void* arg)
{
    ThreadObject* retThread = new ThreadObject();

    int32_t priority = 0x30;
    svcGetThreadPriority(&priority, CUR_THREAD_HANDLE);
    priority -= 1;
    priority = priority < 0x18 ? 0x18 : priority;
    priority = priority > 0x3F ? 0x3F : priority;

    *retThread = threadCreate(
        func,
        arg,
        16 * 1024,
        priority,
        -1,
        false);

    if (*retThread == 0)
    {
        LogError("Failed to create Thread");
    }

    return retThread;
}

void SYS_JoinThread(ThreadObject* thread)
{
    threadJoin(*thread, UINT64_MAX);
}

void SYS_DestroyThread(ThreadObject* thread)
{
    threadFree(*thread);
    delete thread;
}

MutexObject* SYS_CreateMutex()
{
    MutexObject* retMutex = new MutexObject();

    int32_t result = svcCreateMutex(retMutex, false);

    if (result < 0)
    {
        LogError("Failed to create Mutex");
    }

    return retMutex;
}

void SYS_LockMutex(MutexObject* mutex)
{
    int32_t result = svcWaitSynchronization(*mutex, UINT64_MAX);

    if (result < 0)
    {
        LogError("Error locking mutex");
    }
}

void SYS_UnlockMutex(MutexObject* mutex)
{
    if (svcReleaseMutex(*mutex) < 0)
    {
        LogError("Error releasing mutex");
    }
}

void SYS_DestroyMutex(MutexObject* mutex)
{
    svcCloseHandle(*mutex);
    delete mutex;
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
    OCT_ASSERT(pointer != nullptr);
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

bool SYS_ReadSave(const char* saveName, Stream& outStream)
{
    bool success = false;

    if (GetEngineState()->mProjectDirectory != "")
    {
        if (SYS_DoesSaveExist(saveName))
        {
            std::string savePath = GetEngineState()->mProjectDirectory + "Saves/" + saveName;
            outStream.ReadFile(savePath.c_str(), false);
            success = true;
        }
        else
        {
            LogError("Failed to read save.");
        }
    }
    else
    {
        LogError("Failed to read save. Project directory is unset.");
    }

    return success;
}

bool SYS_WriteSave(const char* saveName, Stream& stream)
{
    bool success = false;
    
    if (GetEngineState()->mProjectDirectory != "")
    {
        std::string saveDir = GetEngineState()->mProjectDirectory + "Saves";

        // In the embedded case, might need to create a Project directory
        if (!DoesDirExist(GetEngineState()->mProjectDirectory.c_str()))
        {
            SYS_CreateDirectory(GetEngineState()->mProjectDirectory.c_str());
        }
        
        bool saveDirExists = DoesDirExist(saveDir.c_str());

        if (!saveDirExists)
        {
            saveDirExists = SYS_CreateDirectory(saveDir.c_str());
        }

        if (saveDirExists)
        {
            std::string savePath = saveDir + "/" + saveName;
            stream.WriteFile(savePath.c_str());
            success = true;
            LogDebug("Save written: %s (%d bytes)", saveName, stream.GetSize());
        }
        else
        {
            LogError("Failed to open Saves directory");
        }
    }
    else
    {
        LogError("Failed to write save");
    }

    return success;
}

bool SYS_DoesSaveExist(const char* saveName)
{
    bool exists = false;

    if (GetEngineState()->mProjectDirectory != "")
    {
        std::string savePath = GetEngineState()->mProjectDirectory + "Saves/" + saveName;

        FILE* file = fopen(savePath.c_str(), "rb");

        if (file != nullptr)
        {
            exists = true;
            fclose(file);
            file = nullptr;
        }
    }


    return exists;
}

bool SYS_DeleteSave(const char* saveName)
{
    bool success = false;

    if (GetEngineState()->mProjectDirectory != "")
    {
        std::string savePath = GetEngineState()->mProjectDirectory + "Saves/" + saveName;
        SYS_RemoveFile(savePath.c_str());
        success = true;
    }

    return success;
}

void SYS_UnmountMemoryCard()
{

}

// Clipboard
void SYS_SetClipboardText(const std::string& str)
{

}

std::string SYS_GetClipboardText()
{
    return "";
}

// Misc
void SYS_Log(LogSeverity severity, const char* format, va_list arg)
{
    vprintf(format, arg);
    printf("\n");
}

void SYS_Assert(const char* exprString, const char* fileString, uint32_t lineNumber)
{
    const char* fileName = strrchr(fileString, '/') ? strrchr(fileString, '/') + 1 : fileString;
    char str[256];
    snprintf(str, 256, "[Assert] %s, %s, line %d", exprString, fileName, lineNumber);

    SYS_Alert(str);
}

void SYS_Alert(const char* message)
{
    consoleInit(GFX_BOTTOM, &GetEngineState()->mSystem.mPrintConsole);
    consoleSelect(&GetEngineState()->mSystem.mPrintConsole);

    LogError("%s", message);

    // Display alert message in console view and wait for player to hit A button.
    EnableConsole(true);

    SYS_Sleep(500);
    INP_Update();
    while (!IsGamepadButtonJustDown(GAMEPAD_A, 0))
    {
        INP_Update();
    }

    // Add some small feedback to show that 
    // user is attempting to proceed.
    LogError(">>>");
    SYS_Sleep(100);

    EnableConsole(false);
}

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

bool SYS_DoesWindowHaveFocus()
{
    return true;
}

void SYS_SetScreenOrientation(ScreenOrientation orientation)
{

}

ScreenOrientation SYS_GetScreenOrientation()
{
    return ScreenOrientation::Landscape;
}

void SYS_SetFullscreen(bool fullscreen)
{

}

bool SYS_IsFullscreen()
{
    return true;
}

#endif