#if PLATFORM_DOLPHIN

#include "System/System.h"

#include "Engine.h"
#include "Renderer.h"
#include "Log.h"
#include "Input/Input.h"
#include "Constants.h"
#include "InputDevices.h"

#include <gccore.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <fat.h>
#include <ogc/lwp_watchdog.h>

static bool sFatInit = false;
static void InitFAT()
{
    if (!sFatInit)
    {
        if (fatInitDefault())
        {
            LogDebug("FAT Initialized Successfully.\n");
            sFatInit = true;
        }
        else
        {
            LogDebug("fatInitDefault() failure.\n");
        }
    }
}

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

#if PLATFORM_WII
    CONF_Init();
    int32_t aspectRatio = CONF_GetAspectRatio();
    if (aspectRatio == CONF_ASPECT_16_9)
    {
        // On the Wii, if the TV is a 16:9 aspect ratio, then we still render
        // to the 640x480 framebuffer, but it will be stretched to fill the whole screen.
        // So save off an aspect ratio scale that we can use to adjust the camera's aspect ratio.
        engine.mAspectRatioScale = (16.0f / 9.0f) / (4.0f / 3.0f);
    }
#endif

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

    InitFAT();
}

void SYS_Shutdown()
{

}

void SYS_Update()
{

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
    // Need to init fat in case opening the Engine.ini in OctPreInitialize()
    InitFAT();

    outData = nullptr;
    outSize = 0;

    FILE* file = fopen(path, "rb");

    // TODO: Handle reading from ISO

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

    int32_t createStatus = 0;

    createStatus = LWP_CreateThread(
        retThread,     /* thread handle */
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

void SYS_JoinThread(ThreadObject* thread)
{
    void* retValue = nullptr;
    LWP_JoinThread(*thread, &retValue);
}

void SYS_DestroyThread(ThreadObject* thread)
{
    delete thread;
}

MutexObject* SYS_CreateMutex()
{
    MutexObject* retMutex = new MutexObject();

    // Pass true in second param to allow recursive locking
    int32_t status = LWP_MutexInit(retMutex, true);

    if (status < 0)
    {
        LogError("Failed to create Mutex");
    }

    return retMutex;
}

void SYS_LockMutex(MutexObject* mutex)
{
    LWP_MutexLock(*mutex);
}

void SYS_UnlockMutex(MutexObject* mutex)
{
    LWP_MutexUnlock(*mutex);
}

void SYS_DestroyMutex(MutexObject* mutex)
{
    LWP_MutexDestroy(*mutex);
    delete mutex;
}

void SYS_Sleep(uint32_t milliseconds)
{
    // Uh... not sure how to sleep for a given duration.
    // But this sleep function at least yields to other threads I think...
    lwp_t thisThread = LWP_GetSelf();
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
    OCT_ASSERT(pointer != nullptr);
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

static bool IsMemoryCardMounted()
{
    return GetEngineState()->mSystem.mMemoryCardMounted;
}

#if PLATFORM_GAMECUBE
static void UnmountMemoryCard(int32_t channel, int32_t result)
{
    LogWarning("Memory Card was removed from Slot %c", (channel == 0) ? 'A' : 'B');
    SYS_UnmountMemoryCard();
}

static void MountMemoryCard()
{
    if (!IsMemoryCardMounted())
    {
        LogDebug("Initializing CARD");
        GetEngineState()->mSystem.mMemoryCardMountArea = SYS_AlignedMalloc(CARD_WORKAREA_SIZE, 32);
        CARD_Init("OCTA","00");
        int errorSlotA = CARD_Mount(CARD_SLOTA, GetEngineState()->mSystem.mMemoryCardMountArea, UnmountMemoryCard);
        LogDebug("Memory card code: %d", errorSlotA);

        if (errorSlotA >= 0)
        {
            GetEngineState()->mSystem.mMemoryCardMounted = true;
        }
    }
}
#endif

bool SYS_ReadSave(const char* saveName, Stream& outStream)
{
    // This needs to be different between GameCube and Wii, since GameCube uses memory cards and Wii uses SD cards.
    bool success = false;

#if PLATFORM_WII
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
#else

    LogDebug("READ SAVE");
    MountMemoryCard();

    if (IsMemoryCardMounted())
    {
        uint32_t sectorSize = 0;
        CARD_GetSectorSize(CARD_SLOTA, &sectorSize);

        card_file cardFile;
        int32_t cardError = CARD_Open(CARD_SLOTA, saveName, &cardFile);

        if (cardError >= 0)
        {
            int32_t fileSize = ((cardFile.len + sectorSize - 1) / sectorSize) * sectorSize;

            char* cardBuffer = (char*)SYS_AlignedMalloc(fileSize, 32);
            CARD_Read(&cardFile, cardBuffer, sectorSize, 0);
            success = true;

            outStream.SetPos(0);
            outStream.WriteBytes((uint8_t*) cardBuffer, fileSize);
            outStream.SetPos(0);
            SYS_AlignedFree(cardBuffer);

            CARD_Close(&cardFile);
        }
    }
#endif

    return success;
}

bool SYS_WriteSave(const char* saveName, Stream& stream)
{
    bool success = false;
#if PLATFORM_WII
    
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
#else

    LogDebug("WRITE SAVE");
    MountMemoryCard();

    if (IsMemoryCardMounted())
    {
        uint32_t sectorSize = 0;
        CARD_GetSectorSize(CARD_SLOTA, &sectorSize);

        int32_t fileSize = ((stream.GetSize() + sectorSize - 1) / sectorSize) * sectorSize;

        card_file cardFile;
        int32_t cardError = CARD_Open(CARD_SLOTA, saveName, &cardFile);

        if (cardError < 0)
        {
            // File not found. Create it.
            cardError = CARD_Create(CARD_SLOTA, saveName, fileSize, &cardFile);

            if (cardError < 0)
            {
                LogError("Failed to create save data on memory card. Error code = %d", cardError);
            }
        }

        if (cardError >= 0)
        {
            char* cardBuffer = (char*)SYS_AlignedMalloc(fileSize, 32);

            //LogDebug("fileSize = %d, cardFile.len = %d, stream.GetSize() = %d", fileSize, cardFile.len, stream.GetSize());
            //OCT_ASSERT(fileSize == cardFile.len);
            OCT_ASSERT(fileSize >= (int32_t)stream.GetSize());
            memcpy(cardBuffer, stream.GetData(), stream.GetSize());

            cardError = CARD_Write(&cardFile, cardBuffer, fileSize, 0);
            success = true;

            if (cardError < 0)
            {
                LogError("Failed to write save to memory card. Error code = %d", cardError);
            }

            SYS_AlignedFree(cardBuffer);
            CARD_Close(&cardFile);
        }
    }
#endif

    return success;
}

bool SYS_DoesSaveExist(const char* saveName)
{
    bool exists = false;

#if PLATFORM_WII
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
#else

    LogDebug("CHECK SAVE");
    MountMemoryCard();

    if (IsMemoryCardMounted())
    {
        card_file cardFile;
        int32_t cardError = CARD_Open(CARD_SLOTA, saveName, &cardFile);

        if (cardError >= 0)
        {
            exists = true;
            CARD_Close(&cardFile);
        }
    }
#endif

    return exists;
}

bool SYS_DeleteSave(const char* saveName)
{
    bool success = false;

#if PLATFORM_WII
    if (GetEngineState()->mProjectDirectory != "")
    {
        std::string savePath = GetEngineState()->mProjectDirectory + "Saves/" + saveName;
        SYS_RemoveFile(savePath.c_str());
        success = true;
    }
#else
    LogDebug("DELETE SAVE");
    MountMemoryCard();

    if (IsMemoryCardMounted())
    {
        int32_t cardError = CARD_Delete(CARD_SLOTA, saveName);

        if (cardError >= 0)
        {
            success = true;
        }
    }
#endif

    return success;
}

void SYS_UnmountMemoryCard()
{
    LogDebug("Unmounting Memory Card");
    if (IsMemoryCardMounted())
    {
        CARD_Unmount(CARD_SLOTA);
        GetEngineState()->mSystem.mMemoryCardMounted = false;
        SYS_AlignedFree(GetEngineState()->mSystem.mMemoryCardMountArea);
        GetEngineState()->mSystem.mMemoryCardMountArea = nullptr;
    }
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
#if 1
    vprintf(format, arg);
    printf("\n");
#else
    // Log to file (for easier debugging)
    // DO NOT DO BOTH! arg list can only be used once.
    FILE* file = fopen("Log.txt", "a");

    if (file != nullptr)
    {
        vfprintf(file, format, arg);
        fprintf(file, "\n");
        fclose(file);
        file = nullptr;
    }
#endif
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
    // Display alert message in console view and wait for player to hit A button.
    LogError("%s", message);

    EnableConsole(true);
    SYS_Sleep(500);
    INP_Update();
    while (!IsGamepadButtonJustDown(GAMEPAD_A, 0))
    {
        SYS_Sleep(5);
        INP_Update();
    }

    EnableConsole(false);
}

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