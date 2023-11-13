#pragma once

#include "Constants.h"
#include "Maths.h"
#include <string>

#if PLATFORM_WINDOWS
#include <Windows.h>
#elif PLATFORM_LINUX
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <xcb/xcb.h>
#include <pthread.h>
#elif PLATFORM_ANDROID
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <android/native_window.h>
#include <android/native_activity.h>
#include <android_native_app_glue.h>
#endif

#if PLATFORM_WINDOWS
typedef HANDLE ThreadObject;
typedef HANDLE MutexObject;
typedef DWORD ThreadFuncRet;
#elif (PLATFORM_LINUX || PLATFORM_ANDROID)
typedef pthread_t ThreadObject;
typedef pthread_mutex_t MutexObject;
typedef void* ThreadFuncRet;
#endif

typedef ThreadFuncRet(*ThreadFuncFP)(void*);

#define THREAD_RETURN() return 0;

enum class ScreenOrientation : uint8_t
{
    Landscape,
    Portrait,
    Auto,

    Count
};

struct DirEntry
{
    char mDirectoryPath[MAX_PATH_SIZE + 1] = { };
    char mFilename[MAX_PATH_SIZE + 1] = { };
    bool mDirectory = false;
    bool mValid = false;

#if PLATFORM_WINDOWS
    WIN32_FIND_DATA mFindData = { };
    HANDLE mFindHandle = nullptr;
#elif (PLATFORM_LINUX || PLATFORM_ANDROID)
    DIR* mDir = nullptr;
#endif
};

struct SystemState
{
#if PLATFORM_WINDOWS
    HINSTANCE mConnection = nullptr;
    HWND mWindow = nullptr;
    POINT mMinSize = {};
    bool mWindowHasFocus = true;
    bool mFullscreen = false;
#elif PLATFORM_LINUX
    xcb_connection_t* mXcbConnection = nullptr;
    xcb_screen_t* mXcbScreen = nullptr;
    xcb_window_t mXcbWindow = 0;
    xcb_intern_atom_reply_t* mAtomDeleteWindow = nullptr;
    xcb_cursor_t mNullCursor = XCB_NONE;
    bool mWindowHasFocus = false;
    bool mFullscreen = false;
#elif PLATFORM_ANDROID
    android_app* mState = nullptr;
    ANativeWindow* mWindow = nullptr;
    ANativeActivity* mActivity = nullptr;
    //EGLDisplay mDisplay = ??? is this a pointer?;
    //EGLSurface mSurface = ???;
    //EGLContext mContext = ???;
    std::string mInternalDataPath;
    int32_t mWidth= 100;
    int32_t mHeight = 100;
    bool mWindowInitialized = false;
    bool mWindowHasFocus = false;
    bool mOrientationChanged = false;
    ScreenOrientation mOrientationMode = ScreenOrientation::Landscape;
    ScreenOrientation mActiveOrientation = ScreenOrientation::Landscape;
#endif
};

enum class LogSeverity : uint32_t
{
    Debug,
    Warning,
    Error,

    Count
};
