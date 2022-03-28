#pragma once

#include "Constants.h"

#include <glm/glm.hpp>

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
#elif PLATFORM_DOLPHIN
#include <gccore.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#elif PLATFORM_3DS
#include <3ds.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#if PLATFORM_WINDOWS
typedef HANDLE ThreadHandle;
typedef HANDLE MutexHandle;
typedef DWORD ThreadFuncRet;
#elif PLATFORM_LINUX
typedef pthread_t ThreadHandle;
typedef pthread_mutex_t MutexHandle;
typedef void* ThreadFuncRet;
#elif PLATFORM_DOLPHIN
typedef lwp_t ThreadHandle;
typedef uint32_t MutexHandle;
typedef void* ThreadFuncRet;
#elif PLATFORM_3DS
typedef Thread ThreadHandle;
typedef uint32_t MutexHandle;
typedef void ThreadFuncRet;
#endif

typedef ThreadFuncRet(*ThreadFuncFP)(void*);

#if PLATFORM_3DS
#define THREAD_RETURN() return;
#else
#define THREAD_RETURN() return 0;
#endif

struct DirEntry
{
    char mDirectoryPath[MAX_PATH_SIZE + 1] = { };
    char mFilename[MAX_PATH_SIZE + 1] = { };
    bool mDirectory = false;
    bool mValid = false;

#if PLATFORM_WINDOWS
    WIN32_FIND_DATA mFindData = { };
    HANDLE mFindHandle = nullptr;
#elif PLATFORM_LINUX
    DIR* mDir = nullptr;
#elif PLATFORM_DOLPHIN
    DIR* mDir = nullptr;
#elif PLATFORM_3DS
    DIR* mDir = nullptr;
#endif
};

struct SystemState
{
#if PLATFORM_WINDOWS
    HINSTANCE mConnection = nullptr;
    HWND mWindow = nullptr;
    POINT mMinSize = {};
    glm::ivec2 mCursorSet = { -1, -1 };
#elif PLATFORM_LINUX
    xcb_connection_t* mXcbConnection = nullptr;
    xcb_screen_t* mXcbScreen = nullptr;
    xcb_window_t mXcbWindow = 0;
    xcb_intern_atom_reply_t* mAtomDeleteWindow = nullptr;
    xcb_cursor_t mNullCursor = XCB_NONE;
#elif PLATFORM_DOLPHIN
    void* mFrameBuffers[2] = { };
    void* mConsoleBuffer = nullptr;
    GXRModeObj* mGxrMode = nullptr;
    uint32_t mFrameIndex = 0;
#elif PLATFORM_3DS
    PrintConsole mPrintConsole = {};
    float mSlider = 0.0f;
    bool mNew3DS = false;
#endif
};

