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
        case 19: code = GamepadButtonCode::GAMEPAD_UP; break;
        case 20: code = GamepadButtonCode::GAMEPAD_DOWN; break;
        case 21: code = GamepadButtonCode::GAMEPAD_LEFT; break;
        case 22: code = GamepadButtonCode::GAMEPAD_RIGHT; break;
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
        static bool sGraphicsInit = false;
        LogDebug("APP_CMD_INIT_WINDOW");
        // The window is being shown, get it ready.
        if (app->window != nullptr)
        {
            EngineState* engineState = GetEngineState();

            if (sGraphicsInit)
            {
                engineState->mSystem.mWindow = app->window;
                GFX_Reset();
            }
            else
            {
                engineState->mSystem.mWindow = app->window;
                engineState->mWindowWidth = ANativeWindow_getWidth(app->window);
                engineState->mWindowHeight = ANativeWindow_getHeight(app->window);

                LogDebug("Window Width = %d", engineState->mWindowWidth);
                LogDebug("Window Height = %d", engineState->mWindowHeight);

                GFX_Initialize();
                GetEngineState()->mSystem.mWindowInitialized = true;

                sGraphicsInit = true;
            }

            GetEngineState()->mWindowMinimized = false;
        }
        break;
    }
    case APP_CMD_TERM_WINDOW:
    {
        LogDebug("APP_CMD_TERM_WINDOW");
        GetEngineState()->mWindowMinimized = true;
        break;
    }
    case APP_CMD_GAINED_FOCUS:
    {
        LogDebug("APP_CMD_GAINED_FOCUS");
        GetEngineState()->mSystem.mWindowHasFocus = true;
        break;
    }
    case APP_CMD_LOST_FOCUS:
    {
        LogDebug("APP_CMD_LOST_FOCUS");
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
        GetEngineState()->mSuspended = false;
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
        GetEngineState()->mSuspended = true;
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

static int HandleInput(struct android_app* app, AInputEvent* inputEvent)
{
    int device = 0;
    int source = 0;

    device = AInputEvent_getDeviceId(inputEvent);
    source = AInputEvent_getSource(inputEvent) & 0xffffff00;

    EngineState* engineState = GetEngineState();

    if (AInputEvent_getType(inputEvent) == AINPUT_EVENT_TYPE_MOTION)
    {
        int action = AMotionEvent_getAction(inputEvent);
        int pointer = (0xff00 & action) >> 8;
        action = 0x00ff & action;

        if (action == AMOTION_EVENT_ACTION_DOWN)
        {
            INP_SetTouch(pointer);
            INP_SetTouchPosition(static_cast<int>(AMotionEvent_getX(inputEvent, pointer)),
                static_cast<int>(AMotionEvent_getY(inputEvent, pointer)),
                pointer);
            return 1;
        }
        else if (action == AMOTION_EVENT_ACTION_UP)
        {
            INP_ClearTouch(pointer);
            INP_SetTouchPosition(static_cast<int>(AMotionEvent_getX(inputEvent, pointer)),
                static_cast<int>(AMotionEvent_getY(inputEvent, pointer)),
                pointer);
            return 1;
        }
        else if (action == AMOTION_EVENT_ACTION_POINTER_DOWN)
        {
            INP_SetTouch(pointer);
            INP_SetTouchPosition(static_cast<int>(AMotionEvent_getX(inputEvent, pointer)),
                static_cast<int>(AMotionEvent_getY(inputEvent, pointer)),
                pointer);
            return 1;
        }
        else if (action == AMOTION_EVENT_ACTION_POINTER_UP)
        {
            // This used to be called but it is useless because INP_ClearTouch() will have to
            // propagate higher pointer positions downward, which overwrites this position.
            //INP_SetTouchPosition(static_cast<int>(AMotionEvent_getX(inputEvent, pointer)),
            //                     static_cast<int>(AMotionEvent_getY(inputEvent, pointer)),
            //                     pointer);

            INP_ClearTouch(pointer);
            return 1;
        }
        else if (action == AMOTION_EVENT_ACTION_MOVE)
        {
            int numPointers = AMotionEvent_getPointerCount(inputEvent);
            for (int ptr = 0; ptr < numPointers && ptr < INPUT_MAX_TOUCHES; ptr++)
            {
                int x = static_cast<int>(AMotionEvent_getX(inputEvent, ptr));
                int y = static_cast<int>(AMotionEvent_getY(inputEvent, ptr));

                INP_SetTouchPosition(x, y, ptr);
            }

            if (source & AINPUT_SOURCE_JOYSTICK)
            {
                int gamepadIndex = INP_GetGamepadIndex(device);

                float axisX = AMotionEvent_getAxisValue(inputEvent,
                    AMOTION_EVENT_AXIS_X,
                    0);
                float axisY = AMotionEvent_getAxisValue(inputEvent,
                    AMOTION_EVENT_AXIS_Y,
                    0);
                float axisZ = AMotionEvent_getAxisValue(inputEvent,
                    AMOTION_EVENT_AXIS_Z,
                    0);
                float axisRZ = AMotionEvent_getAxisValue(inputEvent,
                    AMOTION_EVENT_AXIS_RZ,
                    0);
                float axisHatX = AMotionEvent_getAxisValue(inputEvent,
                    AMOTION_EVENT_AXIS_HAT_X,
                    0);
                float axisHatY = AMotionEvent_getAxisValue(inputEvent,
                    AMOTION_EVENT_AXIS_HAT_Y,
                    0);
                float axisTriggerL = AMotionEvent_getAxisValue(inputEvent,
                    AMOTION_EVENT_AXIS_LTRIGGER,
                    0);
                float axisTriggerR = AMotionEvent_getAxisValue(inputEvent,
                    AMOTION_EVENT_AXIS_RTRIGGER,
                    0);
                float axisGas = AMotionEvent_getAxisValue(inputEvent,
                    AMOTION_EVENT_AXIS_GAS,
                    0);
                float axisBrake = AMotionEvent_getAxisValue(inputEvent,
                    AMOTION_EVENT_AXIS_BRAKE,
                    0);

                axisTriggerL = glm::max(axisTriggerL, axisBrake);
                axisTriggerR = glm::max(axisTriggerR, axisGas);
                
                INP_SetGamepadAxisValue(GamepadAxisCode::GAMEPAD_AXIS_LTHUMB_X, axisX, gamepadIndex);
                INP_SetGamepadAxisValue(GamepadAxisCode::GAMEPAD_AXIS_LTHUMB_Y, -axisY, gamepadIndex);
                INP_SetGamepadAxisValue(GamepadAxisCode::GAMEPAD_AXIS_RTHUMB_X, axisZ, gamepadIndex);
                INP_SetGamepadAxisValue(GamepadAxisCode::GAMEPAD_AXIS_RTHUMB_Y, -axisRZ, gamepadIndex);
                INP_SetGamepadAxisValue(GamepadAxisCode::GAMEPAD_AXIS_LTRIGGER, axisTriggerL, gamepadIndex);
                INP_SetGamepadAxisValue(GamepadAxisCode::GAMEPAD_AXIS_RTRIGGER, axisTriggerR, gamepadIndex);

                // Convert hat axis to DPad buttons
                if (axisHatY < 0.0)
                    INP_SetGamepadButton(GamepadButtonCode::GAMEPAD_UP, gamepadIndex);
                else 
                    INP_ClearGamepadButton(GamepadButtonCode::GAMEPAD_UP, gamepadIndex);

                if (axisHatY > 0.0)
                    INP_SetGamepadButton(GamepadButtonCode::GAMEPAD_DOWN, gamepadIndex);
                else
                    INP_ClearGamepadButton(GamepadButtonCode::GAMEPAD_DOWN, gamepadIndex);

                if (axisHatX < 0.0)
                    INP_SetGamepadButton(GamepadButtonCode::GAMEPAD_LEFT, gamepadIndex);
                else
                    INP_ClearGamepadButton(GamepadButtonCode::GAMEPAD_LEFT, gamepadIndex);

                if (axisHatX > 0.0)
                    INP_SetGamepadButton(GamepadButtonCode::GAMEPAD_RIGHT, gamepadIndex);
                else
                    INP_ClearGamepadButton(GamepadButtonCode::GAMEPAD_RIGHT, gamepadIndex);

            }
            return 1;
        }
    }
    else if (AInputEvent_getType(inputEvent) == AINPUT_EVENT_TYPE_KEY)
    {
        int action = AKeyEvent_getAction(inputEvent);

        int key = AKeyEvent_getKeyCode(inputEvent);

        if (key == AKEYCODE_VOLUME_DOWN ||
            key == AKEYCODE_VOLUME_UP ||
            key == AKEYCODE_VOLUME_MUTE ||
            key == AKEYCODE_BACK ||
            key == AKEYCODE_HOME ||
            key == AKEYCODE_MENU)
        {
            // Let the system handle these.
            return 0;
        }

        if (action == AKEY_EVENT_ACTION_DOWN)
        {
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

void HandleResize(ANativeActivity *activity, ANativeWindow *window)
{
    SystemState& system = GetEngineState()->mSystem;
    system.mOrientationChanged = true;

    if (ANativeWindow_getWidth(window) >= ANativeWindow_getHeight(window))
    {
        system.mActiveOrientation = ScreenOrientation::Landscape;
    }
    else
    {
        system.mActiveOrientation = ScreenOrientation::Portrait;
    }
}

void SYS_Initialize()
{
    LogDebug("ANDROID --- SYS_Initialize()!");
    app_dummy();

    EngineState* engineState = GetEngineState();
    SystemState& system = engineState->mSystem;
    android_app* state = system.mState;
    system.mActivity = state->activity;

    state->onAppCmd = HandleCommand;
    state->onInputEvent = HandleInput;
    state->activity->callbacks->onNativeWindowResized = HandleResize;

    system.mInternalDataPath = system.mActivity->internalDataPath;
    system.mInternalDataPath += "/";

    // Keep processing events until window is initialized.
    while (!system.mWindowInitialized)
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
}

void SYS_Shutdown()
{

}

void SYS_Update()
{
    SystemState& system = GetEngineState()->mSystem;

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
            source->process(system.mState, source);
        }
    }

    InputPostUpdate();

    if (system.mOrientationChanged)
    {
        uint32_t newWidth = ANativeWindow_getWidth(system.mWindow);
        uint32_t newHeight = ANativeWindow_getHeight(system.mWindow);

        ResizeWindow(newWidth, newHeight);
        system.mOrientationChanged = false;
    }
}

// Files
bool SYS_DoesFileExist(const char* path, bool isAsset)
{
    bool exists = false;

    if (isAsset)
    {
        AAssetManager* assetManager = GetEngineState()->mSystem.mState->activity->assetManager;
        AAsset* asset = AAssetManager_open(assetManager, path, AASSET_MODE_BUFFER);

        if (asset != nullptr)
        {
            exists = true;
            AAsset_close(asset);
        }
    }
    else
    {
        struct stat info;

        int32_t retStatus = stat(path, &info);

        if (retStatus == 0)
        {
            // If the file is actually a directory, than return false.
            exists = !(info.st_mode & S_IFDIR);
        }
    }
    return exists;
}

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

static std::vector<std::string> sDirIterFiles;

static std::vector<std::string> ConvertStringList(JNIEnv* env, jobject arrayList)
{
    static jclass java_util_ArrayList      = static_cast<jclass>(env->NewGlobalRef(env->FindClass("java/util/ArrayList")));
    static jmethodID java_util_ArrayList_     = env->GetMethodID(java_util_ArrayList, "<init>", "(I)V");
    static jmethodID java_util_ArrayList_size = env->GetMethodID (java_util_ArrayList, "size", "()I");
    static jmethodID java_util_ArrayList_get  = env->GetMethodID(java_util_ArrayList, "get", "(I)Ljava/lang/Object;");
    static jmethodID java_util_ArrayList_add  = env->GetMethodID(java_util_ArrayList, "add", "(Ljava/lang/Object;)Z");

    std::vector<std::string> vecList;

    jint arrayListSize = env->CallIntMethod(arrayList, java_util_ArrayList_size);

    for (jint i = 0; i < arrayListSize; ++i)
    {
        jstring jStr = static_cast<jstring>(env->CallObjectMethod(arrayList, java_util_ArrayList_get, i));

        const char* cStr = env->GetStringUTFChars(jStr, nullptr);
        vecList.push_back(cStr);

        env->ReleaseStringUTFChars(jStr, cStr);
        env->DeleteLocalRef(jStr);
    }

    return vecList;
}

void SYS_OpenDirectory(const std::string& dirPath, DirEntry& outDirEntry)
{
    SystemState& system = GetEngineState()->mSystem;

    sDirIterFiles.clear();

    strncpy(outDirEntry.mDirectoryPath, dirPath.c_str(), MAX_PATH_SIZE);

    // Call JNI to get list of files and dirs
    JNIEnv* env = nullptr;
    JavaVM* vm = system.mActivity->vm;
    vm->AttachCurrentThread(&env, nullptr);

    {
        jobject octActivity = system.mActivity->clazz;
        jclass octClass = env->GetObjectClass(octActivity);
        jmethodID javaMeth = env->GetMethodID(octClass, "iterateDirFiles",
                                                       "(Ljava/lang/String;)Ljava/util/ArrayList;");

        jstring jDir = env->NewStringUTF(dirPath.c_str());
        jobject jArray = env->CallObjectMethod(octActivity, javaMeth, jDir);

        sDirIterFiles = ConvertStringList(env, jArray);
    }

    vm->DetachCurrentThread();

    if (sDirIterFiles.size() == 0)
    {
        outDirEntry.mValid = false;
    }
    else
    {
        memcpy(outDirEntry.mFilename, sDirIterFiles.back().c_str(), MAX_PATH_SIZE);

        outDirEntry.mDirectory = false;
        outDirEntry.mValid = true;
    }
}

void SYS_IterateDirectory(DirEntry& dirEntry)
{
    sDirIterFiles.pop_back();

    if (sDirIterFiles.size() == 0)
    {
        dirEntry.mValid = false;
    }
    else
    {
        memcpy(dirEntry.mFilename, sDirIterFiles.back().c_str(), MAX_PATH_SIZE);

        dirEntry.mDirectory = true;
        dirEntry.mValid = true;
    }
}

void SYS_CloseDirectory(DirEntry& dirEntry)
{
    sDirIterFiles.clear();
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
    // TODO!
    return "";
}

// Threads
ThreadObject* SYS_CreateThread(ThreadFuncFP func, void* arg)
{
    ThreadObject* retThread = new ThreadObject();

    int status = pthread_create(
        retThread,
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

void SYS_JoinThread(ThreadObject* thread)
{
    pthread_join(*thread, nullptr);
}

void SYS_DestroyThread(ThreadObject* thread)
{
    delete thread;
}

MutexObject* SYS_CreateMutex()
{
    MutexObject* retMutex = new MutexObject();
    int status = pthread_mutex_init(retMutex, nullptr);

    if (status != 0)
    {
        LogError("Failed to create Mutex");
    }

    return retMutex;
}

void SYS_LockMutex(MutexObject* mutex)
{
    int status = pthread_mutex_lock(mutex);

    if (status != 0)
    {
        LogError("Failed to lock mutex");
    }
}

void SYS_UnlockMutex(MutexObject* mutex)
{
    int status = pthread_mutex_unlock(mutex);

    if (status != 0)
    {
        LogError("Failed to unlock mutex");
    }
}

void SYS_DestroyMutex(MutexObject* mutex)
{
    pthread_mutex_destroy(mutex);
    delete mutex;
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


std::string SYS_GetFileName(const std::string& relativePath)
{
 size_t slash = relativePath.find_last_of("/\\");
    size_t start = (slash == std::string::npos) ? 0 : slash + 1;

    // Strip extension (last '.' after the last slash)
    size_t dot = relativePath.find_last_of('.');
    if (dot == std::string::npos || dot < start) {
        dot = relativePath.size(); // no extension
    }

    return relativePath.substr(start, dot - start);

}
void SYS_CopyDirectory(const char* sourceDir, const char* destDir)
{
    std::string cmd = std::string("cp -r \"") + sourceDir + "\" \"" + destDir + "\"";
    SYS_Exec(cmd.c_str());
}

void SYS_CopyFile(const char* sourcePath, const char* destPath)
{
    std::string cmd = std::string("cp \"") + sourcePath + "\" \"" + destPath + "\"";
    SYS_Exec(cmd.c_str());
}

void SYS_MoveDirectory(const char* sourceDir, const char* destDir)
{
    std::string cmd = std::string("mv \"") + sourceDir + "\" \"" + destDir + "\"";
    SYS_Exec(cmd.c_str());
}

void SYS_MoveFile(const char* sourcePath, const char* destPath)
{
    std::string cmd = std::string("mv \"") + sourcePath + "\" \"" + destPath + "\"";
    SYS_Exec(cmd.c_str());
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

std::vector<MemoryStat> SYS_GetMemoryStats()
{
    // What do?
    return {};
}

// Save Game
bool SYS_ReadSave(const char* saveName, Stream& outStream)
{
    bool success = false;
    
    const std::string& dataPath = GetEngineState()->mSystem.mInternalDataPath;

    if (dataPath != "")
    {
        if (SYS_DoesSaveExist(saveName))
        {
            std::string savePath = dataPath + "Saves/" + saveName;
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

    const std::string& dataPath = GetEngineState()->mSystem.mInternalDataPath;

    if (dataPath != "")
    {
        std::string saveDir = dataPath + "Saves";
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

    const std::string& dataPath = GetEngineState()->mSystem.mInternalDataPath;

    if (dataPath != "")
    {
        std::string savePath = dataPath + "Saves/" + saveName;

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

    const std::string& dataPath = GetEngineState()->mSystem.mInternalDataPath;

    if (dataPath != "")
    {
        std::string savePath = dataPath + "Saves/" + saveName;
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

void SYS_SetScreenOrientation(ScreenOrientation orientation)
{
    SystemState& system = GetEngineState()->mSystem;

    system.mOrientationMode = orientation;

    JNIEnv* env = nullptr;
    JavaVM* vm = system.mActivity->vm;
    vm->AttachCurrentThread(&env, nullptr);

    jobject octActivity = system.mActivity->clazz;
    jclass octClass = env->GetObjectClass(octActivity);
    jmethodID orientationMethod = env->GetMethodID(octClass, "setSystemOrientation", "(I)V");

    env->CallVoidMethod(octActivity, orientationMethod, (int32_t)orientation);

    vm->DetachCurrentThread();
}

ScreenOrientation SYS_GetScreenOrientation()
{
    ScreenOrientation orientation = ScreenOrientation::Count;
    SystemState& system = GetEngineState()->mSystem;
    if (system.mOrientationMode != ScreenOrientation::Auto)
    {
        orientation = system.mOrientationMode;
    }
    else
    {
        orientation = system.mActiveOrientation;
    }

    return orientation;
}

void SYS_SetFullscreen(bool fullscreen)
{

}

bool SYS_IsFullscreen()
{
    return true;
}

#endif
