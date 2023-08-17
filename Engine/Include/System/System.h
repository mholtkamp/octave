#pragma once

#include "System/SystemTypes.h"

#include <string>
#include <stdarg.h>

class Stream;

void SYS_Initialize();
void SYS_Shutdown();
void SYS_Update();

// Files
bool SYS_DoesFileExist(const char* path, bool isAsset);
void SYS_AcquireFileData(const char* path, bool isAsset, int32_t maxSize, char*& outData, uint32_t& outSize);
void SYS_ReleaseFileData(char* data);
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
ThreadObject* SYS_CreateThread(ThreadFuncFP func, void* arg);
void SYS_JoinThread(ThreadObject* thread);
void SYS_DestroyThread(ThreadObject* thread);
MutexObject* SYS_CreateMutex();
void SYS_LockMutex(MutexObject* mutex);
void SYS_UnlockMutex(MutexObject* mutex);
void SYS_DestroyMutex(MutexObject* mutex);
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
bool SYS_ReadSave(const char* saveName, Stream& outStream);
bool SYS_WriteSave(const char* saveName, Stream& stream);
bool SYS_DoesSaveExist(const char* saveName);
bool SYS_DeleteSave(const char* saveName);
void SYS_UnmountMemoryCard();

// Misc
void SYS_Log(LogSeverity severity, const char* format, va_list arg);
void SYS_Assert(const char* exprString, const char* fileString, uint32_t lineNumber);
void SYS_Alert(const char* message);
void SYS_UpdateConsole();
int32_t SYS_GetPlatformTier();
void SYS_SetWindowTitle(const char* title);
bool SYS_DoesWindowHaveFocus();
void SYS_SetScreenOrientation(ScreenOrientation orientation);
ScreenOrientation SYS_GetScreenOrientation();
void SYS_SetFullscreen(bool fullscreen);

struct ScopedLock
{
    ScopedLock(MutexObject* mutex)
    {
        mMutex = mutex;
        SYS_LockMutex(mMutex);
    }

    ~ScopedLock()
    {
        SYS_UnlockMutex(mMutex);
    }

    MutexObject* mMutex = nullptr;
};

#define SCOPED_LOCK(mutex) ScopedLock scopedLock(mutex)

