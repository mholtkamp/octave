#if PLATFORM_ANDROID

#include "System/System.h"
#include "Graphics/Graphics.h"

#include "Engine.h"
#include "Renderer.h"
#include "Log.h"
#include "Input/Input.h"

#include <chrono>
#include <malloc.h>
#include <stdlib.h>
#include <string>
#include <assert.h>
#include <signal.h>

#include <android/input.h>
#include <android/window.h>
#include <android/log.h>

void SYS_Initialize()
{
    LogDebug("ANDROID --- SYS_Initialize()!");
    app_dummy();
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
    std::string cmdStr = std::string("rm -rf ") + dirPath;
    SYS_Exec(cmdStr.c_str());
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
    // TODO!
    return "";
}

// Threads
ThreadHandle SYS_CreateThread(ThreadFuncFP func, void* arg)
{
    ThreadHandle retThread = 0;

    int status = pthread_create(
        &retThread,
        nullptr,
        func,
        arg
    );
    
    if (status != 0)
    {
        LogError("Failed to create Thread");
    }

    return retThread;
}

void SYS_JoinThread(ThreadHandle thread)
{
    pthread_join(thread, nullptr);
}

void SYS_DestroyThread(ThreadHandle thread)
{
    // Doesn't appear to be needed. Join is sufficient.
}

MutexHandle SYS_CreateMutex()
{
    MutexHandle retHandle;
    int status = pthread_mutex_init(&retHandle, nullptr);

    if (status != 0)
    {
        LogError("Failed to create Mutex");
    }

    return retHandle;
}

void SYS_LockMutex(MutexHandle mutex)
{
    int status = pthread_mutex_lock(&mutex);

    if (status != 0)
    {
        LogError("Failed to lock mutex");
    }
}

void SYS_UnlockMutex(MutexHandle mutex)
{
    int status = pthread_mutex_unlock(&mutex);

    if (status != 0)
    {
        LogError("Failed to unlock mutex");
    }
}

void SYS_DestroyMutex(MutexHandle mutex)
{
    pthread_mutex_destroy(&mutex);
}

void SYS_Sleep(uint32_t milliseconds)
{
    usleep(milliseconds * 1000);
}

// Time
uint64_t SYS_GetTimeMicroseconds()
{
    // If doing it this way is a problem, also see about returning the 
    // number of micro seconds since the program started. Maybe save off a 
    // static timepoint in SYS_Init().
    auto now = std::chrono::high_resolution_clock::now();
    auto now_us = std::chrono::time_point_cast<std::chrono::microseconds>(now);
    auto epoch = now_us.time_since_epoch();
    auto value = std::chrono::duration_cast<std::chrono::microseconds>(epoch);
    return value.count();
}

// Process
void SYS_Exec(const char* cmd, std::string* output)
{
    LogDebug("[Exec] %s", cmd);
    system(cmd);
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
    // What do?
    return 0;
}

uint64_t SYS_GetNumBytesAllocated()
{
    return 0;
}

// Save Game
bool SYS_ReadSave(const char* saveName, Stream& outStream)
{
    bool success = false;
    if (GetEngineState()->mProjectDirectory != "")
    {
        if (SYS_DoesSaveExist(saveName))
        {
            std::string savePath = GetEngineState()->mProjectDirectory + "Saves/" + saveName;
            outStream.ReadFile(savePath.c_str());
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
            LogDebug("Game Saved: %s (%d bytes)", saveName, stream.GetSize());
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

// Misc
void SYS_Log(LogSeverity severity, const char* format, va_list arg)
{
    int logPrio = ANDROID_LOG_INFO;
    switch (severity)
    {
        case LogSeverity::Warning:
            logPrio = ANDROID_LOG_WARN;
            break;
        case LogSeverity::Error:
            logPrio = ANDROID_LOG_ERROR;
            break;
        default:
            logPrio = ANDROID_LOG_INFO;
            break;
    }

    __android_log_vprint(logPrio, "Octave", format, arg);
}

void SYS_Assert(const char* exprString, const char* fileString, uint32_t lineNumber)
{
    const char* fileName = strrchr(fileString, '/') ? strrchr(fileString, '/') + 1 : fileString;
    LogError("[Assert] %s, %s, line %d", exprString, fileName, lineNumber);
    raise(SIGTRAP);
}

void SYS_Alert(const char* message)
{
    LogError("%s", message);
    raise(SIGTRAP);
}

void SYS_UpdateConsole()
{

}

int32_t SYS_GetPlatformTier()
{
    return 2;
}

void SYS_SetWindowTitle(const char* title)
{
    
}

bool SYS_DoesWindowHaveFocus()
{
    return true;
}

#endif
