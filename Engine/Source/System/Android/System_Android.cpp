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
#include <android/asset_manager.h>

static GamepadButtonCode GetGamepadButtonCodeFromKey(int key)
{
    // Setting default value (if somehow the key doesn't match) to C?
    GamepadButtonCode code = GamepadButtonCode::GAMEPAD_C;

    switch (key)
    {
        case 96: code = GamepadButtonCode::GAMEPAD_A; break;
        case 97: code = GamepadButtonCode::GAMEPAD_B; break;
        case 98: code = GamepadButtonCode::GAMEPAD_C; break;
        case 99: code = GamepadButtonCode::GAMEPAD_X; break;
        case 100: code = GamepadButtonCode::GAMEPAD_Y; break;
        case 101: code = GamepadButtonCode::GAMEPAD_Z; break;
        case 102: code = GamepadButtonCode::GAMEPAD_L1; break;
        case 103: code = GamepadButtonCode::GAMEPAD_R1; break;
        case 104: code = GamepadButtonCode::GAMEPAD_L2; break;
        case 105: code = GamepadButtonCode::GAMEPAD_R2; break;
        case 106: code = GamepadButtonCode::GAMEPAD_THUMBL; break;
        case 107: code = GamepadButtonCode::GAMEPAD_THUMBR; break;
        case 108: code = GamepadButtonCode::GAMEPAD_START; break;
        case 109: code = GamepadButtonCode::GAMEPAD_SELECT; break;

        default: break;
    }

    return code;
}

static void HandleCommand(struct android_app* app,
    int cmd)
{
    switch (cmd)
    {
    case APP_CMD_INIT_WINDOW:
    {
        LogDebug("APP_CMD_INIT_WINDOW");
        // The window is being shown, get it ready.
        if (app->window != NULL)
        {
            //InitializeGraphics(app->window);
            GetEngineState()->mSystem.mWindowInitialized = true;
            //Render();
        }
        break;
    }
    case APP_CMD_TERM_WINDOW:
    {
        LogDebug("APP_CMD_TERM_WINDOW");
        // The window is being hidden or closed, clean it up.
        //Shutdown();
        break;
    }
    case APP_CMD_GAINED_FOCUS:
    {
        LogDebug("APP_CMD_GAINED_FOCUS");
        // Enable rendering again.
        GetEngineState()->mSystem.mWindowHasFocus = true;
        break;
    }
    case APP_CMD_LOST_FOCUS:
    {
        LogDebug("APP_CMD_LOST_FOCUS");
        // Disable rendering.
        GetEngineState()->mSystem.mWindowHasFocus = false;
        break;
    }
    case APP_CMD_START:
    {
        LogDebug("APP_CMD_START");
        break;
    }
    case APP_CMD_RESUME:
    {
        LogDebug("APP_CMD_RESUME");
        break;
    }
    case APP_CMD_PAUSE:
    {
        LogDebug("APP_CMD_PAUSE");
        break;
    }
    case APP_CMD_STOP:
    {
        LogDebug("APP_CMD_STOP");
        GetEngineState()->mQuit = true;
        break;
    }
    case APP_CMD_DESTROY:
    {
        LogDebug("APP_CMD_DESTROY");
        GetEngineState()->mQuit = true;
        break;
    }

    }
}

static int HandleInput(struct android_app* app, AInputEvent* event)
{
    int device = 0;
    int source = 0;

    device = AInputEvent_getDeviceId(event);
    source = AInputEvent_getSource(event) & 0xffffff00;

    EngineState* engineState = GetEngineState();

    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
    {
        int action = AMotionEvent_getAction(event);
        int pointer = (0xff00 & action) >> 8;
        action = 0x00ff & action;

        if (action == AMOTION_EVENT_ACTION_DOWN)
        {
            INP_SetTouch(pointer);
            INP_SetTouchPosition(static_cast<int>(AMotionEvent_getX(event, pointer)),
                (engineState->mWindowHeight - 1) - static_cast<int>(AMotionEvent_getY(event, pointer)),
                pointer);
            return 1;
        }
        else if (action == AMOTION_EVENT_ACTION_UP)
        {
            INP_ClearTouch(pointer);
            INP_SetTouchPosition(static_cast<int>(AMotionEvent_getX(event, pointer)),
                (engineState->mWindowHeight - 1) - static_cast<int>(AMotionEvent_getY(event, pointer)),
                pointer);
            return 1;
        }
        else if (action == AMOTION_EVENT_ACTION_POINTER_DOWN)
        {
            INP_SetTouch(pointer);
            INP_SetTouchPosition(static_cast<int>(AMotionEvent_getX(event, pointer)),
                (engineState->mWindowHeight - 1) - static_cast<int>(AMotionEvent_getY(event, pointer)),
                pointer);
            return 1;
        }
        else if (action == AMOTION_EVENT_ACTION_POINTER_UP)
        {
            INP_ClearTouch(pointer);
            INP_SetTouchPosition(static_cast<int>(AMotionEvent_getX(event, pointer)),
                (engineState->mWindowHeight - 1) - static_cast<int>(AMotionEvent_getY(event, pointer)),
                pointer);
            return 1;
        }
        else if (action == AMOTION_EVENT_ACTION_MOVE)
        {
            for (int ptr = 0; ptr < INPUT_MAX_TOUCHES; ptr++)
            {
                INP_SetTouchPosition(static_cast<int>(AMotionEvent_getX(event, ptr)),
                    (engineState->mWindowHeight - 1) - static_cast<int>(AMotionEvent_getY(event, ptr)),
                    ptr);
            }

            if (source & AINPUT_SOURCE_JOYSTICK)
            {
                int gamepadIndex = INP_GetGamepadIndex(device);

                float axisX = AMotionEvent_getAxisValue(event,
                    AMOTION_EVENT_AXIS_X,
                    0);
                float axisY = AMotionEvent_getAxisValue(event,
                    AMOTION_EVENT_AXIS_Y,
                    0);
                float axisZ = AMotionEvent_getAxisValue(event,
                    AMOTION_EVENT_AXIS_Z,
                    0);
                float axisRZ = AMotionEvent_getAxisValue(event,
                    AMOTION_EVENT_AXIS_RZ,
                    0);
                //float axisHatX = AMotionEvent_getAxisValue(event,
                //    AMOTION_EVENT_AXIS_HAT_X,
                //    0);
                //float axisHatY = AMotionEvent_getAxisValue(event,
                //    AMOTION_EVENT_AXIS_HAT_Y,
                //    0);
                float axisTriggerL = AMotionEvent_getAxisValue(event,
                    AMOTION_EVENT_AXIS_LTRIGGER,
                    0);
                float axisTriggerR = AMotionEvent_getAxisValue(event,
                    AMOTION_EVENT_AXIS_RTRIGGER,
                    0);

                INP_SetGamepadAxisValue(GamepadAxisCode::GAMEPAD_AXIS_LTHUMB_X, axisX, gamepadIndex);
                INP_SetGamepadAxisValue(GamepadAxisCode::GAMEPAD_AXIS_LTHUMB_Y, axisY, gamepadIndex);
                INP_SetGamepadAxisValue(GamepadAxisCode::GAMEPAD_AXIS_RTHUMB_X, axisZ, gamepadIndex);
                INP_SetGamepadAxisValue(GamepadAxisCode::GAMEPAD_AXIS_RTHUMB_Y, axisRZ, gamepadIndex);
                INP_SetGamepadAxisValue(GamepadAxisCode::GAMEPAD_AXIS_LTRIGGER, axisTriggerL, gamepadIndex);
                INP_SetGamepadAxisValue(GamepadAxisCode::GAMEPAD_AXIS_RTRIGGER, axisTriggerR, gamepadIndex);
            }
            return 1;
        }
    }
    else if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY)
    {
        int action = AKeyEvent_getAction(event);

        if (action == AKEY_EVENT_ACTION_DOWN)
        {
            int key = AKeyEvent_getKeyCode(event);

            if (source & AINPUT_SOURCE_GAMEPAD)
            {
                int gamepadIndex = INP_GetGamepadIndex(device);
                INP_SetGamepadButton(GetGamepadButtonCodeFromKey(key), gamepadIndex);
            }
            else
            {
                INP_SetKey(key);
            }

            return 1;
        }
        else if (action == AKEY_EVENT_ACTION_UP)
        {
            int key = AKeyEvent_getKeyCode(event);

            if (source & AINPUT_SOURCE_GAMEPAD)
            {
                int gamepadIndex = INP_GetGamepadIndex(device);
                INP_ClearGamepadButton(GetGamepadButtonCodeFromKey(key), gamepadIndex);
            }
            else
            {
                INP_ClearKey(key);
            }

            return 1;
        }
    }

    return 0;
}

void SYS_Initialize()
{
    LogDebug("ANDROID --- SYS_Initialize()!");
    app_dummy();

    EngineState* engineState = GetEngineState();
    SystemState& system = engineState->mSystem;
    android_app* state = system.mState;
    system.mWindow = state->window;
    system.mActivity = state->activity;

    engineState->mWindowWidth = ANativeWindow_getWidth(state->window);
    engineState->mWindowHeight = ANativeWindow_getHeight(state->window);

    state->onAppCmd = HandleCommand;
    state->onInputEvent = HandleInput;
    
#if 0
    // Keep processing events until window is initialized.
    while (true)
    {
        int ident;
        int events;
        android_poll_source* source;
        while ((ident = ALooper_pollAll(system.mWindowInitialized ? 0 : -1, nullptr, &events, (void **)&source)) >= 0)
        {
            if (source != nullptr)
            {
                source->process(state, source);
            }
        }
    }
#endif

}

void SYS_Shutdown()
{

}

void SYS_Update()
{
    // Read all pending events.
    int ident;
    int events;
    android_poll_source* source;

    // If not animating, we will block forever waiting for events.
    // If animating, we loop until all events are read, then continue
    // to draw the next frame of animation.
    while ((ident = ALooper_pollAll(0, nullptr, &events, (void**)&source)) >= 0)
    {
        // Process this event.
        if (source != nullptr)
        {
            source->process(GetEngineState()->mSystem.mState, source);
        }
    }
}

// Files
void SYS_AcquireFileData(const char* path, bool isAsset, int32_t maxSize, char*& outData, uint32_t& outSize)
{
    outData = nullptr;
    outSize = 0;

    if (isAsset)
    {
        AAsset* asset = nullptr;
        int32_t fileSize = 0;

        AAssetManager* assetManager = GetEngineState()->mSystem.mState->activity->assetManager;
        asset = AAssetManager_open(assetManager, path, AASSET_MODE_BUFFER);

        if (asset != nullptr)
        {
            fileSize = AAsset_getLength(asset);

            if (maxSize > 0)
            {
                fileSize = glm::min(fileSize, maxSize);
            }

            outData = (char*)malloc(fileSize);
            outSize = uint32_t(fileSize);

            AAsset_read(asset, outData, fileSize);
            AAsset_close(asset);
        }
        else
        {
            LogError("Could not open asset: %s", path);
        }
    }
    else
    {
        FILE* file = fopen(path, "rb");
        OCT_ASSERT(file != nullptr);

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
