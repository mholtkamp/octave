#pragma once

#include "System/SystemTypes.h"

#include <string>

class Stream;

void SYS_Initialize();
void SYS_Shutdown();
void SYS_Update();

// Files
std::string SYS_GetCurrentDirectoryPath();
void SYS_SetWorkingDirectory(const std::string& dirPath);
bool SYS_CreateDirectory(const char* dirPath);
void SYS_RemoveDirectory(const char* dirPath);
void SYS_OpenDirectory(const std::string& dirPath, DirEntry& outDirEntry);
void SYS_IterateDirectory(DirEntry& dirEntry);
void SYS_CloseDirectory(DirEntry& dirEntry);
void SYS_RemoveFile(const char* path);
bool SYS_Rename(const char* oldPath, const char* newPath);
std::string SYS_OpenFileDialog();
std::string SYS_SaveFileDialog();
std::string SYS_SelectFolderDialog();

// Threading
ThreadHandle SYS_CreateThread(ThreadFuncFP func, void* arg);
void SYS_JoinThread(ThreadHandle thread);
void SYS_DestroyThread(ThreadHandle thread);
MutexHandle SYS_CreateMutex();
void SYS_LockMutex(MutexHandle mutex);
void SYS_UnlockMutex(MutexHandle mutex);
void SYS_DestroyMutex(MutexHandle mutex);
void SYS_Sleep(uint32_t milliseconds);

// Time
uint64_t SYS_GetTimeMicroseconds();

// Process
void SYS_Exec(const char* cmd, std::string* output = nullptr);

// Memory
void* SYS_AlignedMalloc(uint32_t size, uint32_t alignment);
void SYS_AlignedFree(void* pointer);
uint64_t SYS_GetNumBytesFree();
uint64_t SYS_GetNumBytesAllocated();

// Save / Memcard
void SYS_ReadSave(const char* saveName, Stream& outStream);
void SYS_WriteSave(const char* saveName, Stream& stream);
bool SYS_DoesSaveExist(const char* saveName);

// Misc
void SYS_UpdateConsole();
int32_t SYS_GetPlatformTier();
void SYS_SetWindowTitle(const char* title);

struct ScopedLock
{
    ScopedLock(MutexHandle mutex)
    {
        mMutex = mutex;
        SYS_LockMutex(mMutex);
    }

    ~ScopedLock()
    {
        SYS_UnlockMutex(mMutex);
    }

    MutexHandle mMutex;
};

#define SCOPED_LOCK(mutex) ScopedLock scopedLock(mutex)

