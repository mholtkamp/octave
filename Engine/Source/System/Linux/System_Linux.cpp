#if PLATFORM_LINUX

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

extern bool gWarpCursor;
extern int32_t gWarpCursorX;
extern int32_t gWarpCursorY;

void HandleXcbEvent(xcb_generic_event_t* event)
{
    EngineState& engine = *GetEngineState();
    SystemState& system = engine.mSystem;

    switch (event->response_type & 0x7f)
    {
    case XCB_CLIENT_MESSAGE:
    {
        if ((*(xcb_client_message_event_t*)event).data.data32[0] == (*system.mAtomDeleteWindow).atom)
        {
        	engine.mQuit = true;
        }
        break;
    }
    case XCB_MOTION_NOTIFY:
    {
        xcb_motion_notify_event_t *motion = (xcb_motion_notify_event_t*) event;
        INP_SetMousePosition((int32_t)motion->event_x, (int32_t)motion->event_y);
        break;
    }
    case XCB_BUTTON_PRESS:
    {
        xcb_button_press_event_t* press = (xcb_button_press_event_t*) event;
        if (press->detail == XCB_BUTTON_INDEX_1)
            INP_SetMouseButton(MouseCode::MOUSE_LEFT);
        if (press->detail == XCB_BUTTON_INDEX_2)
            INP_SetMouseButton(MouseCode::MOUSE_MIDDLE);
        if (press->detail == XCB_BUTTON_INDEX_3)
            INP_SetMouseButton(MouseCode::MOUSE_RIGHT);
        if (press->detail == XCB_BUTTON_INDEX_4)
            INP_SetScrollWheelDelta(INP_GetScrollWheelDelta() + 1);
        if (press->detail == XCB_BUTTON_INDEX_5)
            INP_SetScrollWheelDelta(INP_GetScrollWheelDelta() - 1);
            
        break;
    }
    case XCB_BUTTON_RELEASE:
    {
        xcb_button_press_event_t* press = (xcb_button_press_event_t*) event;
        if (press->detail == XCB_BUTTON_INDEX_1)
            INP_ClearMouseButton(MouseCode::MOUSE_LEFT);
        if (press->detail == XCB_BUTTON_INDEX_2)
            INP_ClearMouseButton(MouseCode::MOUSE_MIDDLE);
        if (press->detail == XCB_BUTTON_INDEX_3)
            INP_ClearMouseButton(MouseCode::MOUSE_RIGHT);
        break;
    }
    case XCB_KEY_PRESS:
    {
        const xcb_key_release_event_t* keyEvent = (const xcb_key_release_event_t*)event;
        INP_SetKey(keyEvent->detail);
        //LogDebug("Key %d", keyEvent->detail);
        break;
    }
    case XCB_KEY_RELEASE:
    {
        const xcb_key_release_event_t* keyEvent = (const xcb_key_release_event_t*) event;
        INP_ClearKey(keyEvent->detail);
        break;
    }
    case XCB_FOCUS_OUT:
    {
        INP_ClearAllKeys();
        INP_ClearAllMouseButtons();
        break;
    }
    case XCB_DESTROY_NOTIFY:
    {
        GetEngineState()->mQuit = true;
        break;
    }
    case XCB_CONFIGURE_NOTIFY:
    {
        const xcb_configure_notify_event_t* cfgEvent = (const xcb_configure_notify_event_t*) event;

        EngineState* engineState = GetEngineState();
        uint32_t width = cfgEvent->width;
        uint32_t height = cfgEvent->height;

        ResizeWindow(width, height);
        break;
    }
    
    default:
        break;
    }
}

void SYS_Initialize()
{
    EngineState& engine = *GetEngineState();
    SystemState& system = engine.mSystem;

    // Create a window with XCB
    system.mXcbConnection = xcb_connect(NULL, NULL);

    int32_t error = xcb_connection_has_error(system.mXcbConnection);
    if (error != 0)
    {
        LogError("Failed to open XCB connection");
        OCT_ASSERT(0);
    }

    LogDebug("XCB connection opened");

    const xcb_setup_t* setup = xcb_get_setup(system.mXcbConnection);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    system.mXcbScreen = iter.data;

    LogDebug("XCB using screen %p", system.mXcbScreen);

    system.mXcbWindow = xcb_generate_id(system.mXcbConnection);

    uint32_t valueMask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;;
    uint32_t valueList[32] = {};
    valueList[0] = 0x00000000;
	valueList[1] =
		XCB_EVENT_MASK_KEY_RELEASE |
		XCB_EVENT_MASK_KEY_PRESS |
		XCB_EVENT_MASK_EXPOSURE |
		XCB_EVENT_MASK_STRUCTURE_NOTIFY |
		XCB_EVENT_MASK_POINTER_MOTION |
		XCB_EVENT_MASK_BUTTON_PRESS |
		XCB_EVENT_MASK_BUTTON_RELEASE | 
        XCB_EVENT_MASK_FOCUS_CHANGE;

    xcb_create_window(
                    system.mXcbConnection, 
                    XCB_COPY_FROM_PARENT, 
                    system.mXcbWindow,
                    system.mXcbScreen->root, 
                    0, 
                    0, 
                    engine.mWindowWidth, 
                    engine.mWindowHeight, 
                    0,
                    XCB_WINDOW_CLASS_INPUT_OUTPUT, 
                    system.mXcbScreen->root_visual, 
                    valueMask, 
                    valueList);

    // BEGIN - Code taken from Sascha Willems' vulkan samples
    // https://github.com/SaschaWillems/Vulkan/blob/master/base/vulkanexamplebase.cpp

	/* Magic code that will send notification when window is destroyed */
    auto intern_atom_helper = [](xcb_connection_t *conn, bool only_if_exists, const char *str) -> xcb_intern_atom_reply_t*
    {
        xcb_intern_atom_cookie_t cookie = xcb_intern_atom(conn, only_if_exists, strlen(str), str);
        return xcb_intern_atom_reply(conn, cookie, NULL);
    };

	xcb_intern_atom_reply_t* reply = intern_atom_helper(system.mXcbConnection, true, "WM_PROTOCOLS");
	system.mAtomDeleteWindow = intern_atom_helper(system.mXcbConnection, false, "WM_DELETE_WINDOW");

	xcb_change_property(system.mXcbConnection, XCB_PROP_MODE_REPLACE,
		system.mXcbWindow, (*reply).atom, 4, 32, 1,
		&(*system.mAtomDeleteWindow).atom);

	// std::string windowTitle = getWindowTitle();
	// xcb_change_property(system.mXcbConnection, XCB_PROP_MODE_REPLACE,
	// 	system.mXcbWindow, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
	// 	title.size(), windowTitle.c_str());

	free(reply);

	// /**
	//  * Set the WM_CLASS property to display
	//  * title in dash tooltip and application menu
	//  * on GNOME and other desktop environments
	//  */
	// std::string wm_class;
	// wm_class = wm_class.insert(0, name);
	// wm_class = wm_class.insert(name.size(), 1, '\0');
	// wm_class = wm_class.insert(name.size() + 1, title);
	// wm_class = wm_class.insert(wm_class.size(), 1, '\0');
	// xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, XCB_ATOM_WM_CLASS, XCB_ATOM_STRING, 8, wm_class.size() + 2, wm_class.c_str());

    // END - Code taken from Sascha Willems' vulkan samples


    xcb_map_window(system.mXcbConnection, system.mXcbWindow);   
    xcb_flush(system.mXcbConnection);

    LogDebug("XCB window created (%x)", system.mXcbWindow);

    // Create a blank cursor so we can hide it if needed.
    const xcb_font_t cursorFont = xcb_generate_id(system.mXcbConnection);
    xcb_void_cookie_t openFontCookie = xcb_open_font_checked(system.mXcbConnection, cursorFont, strlen("fixed"), "fixed");
    system.mNullCursor = xcb_generate_id(system.mXcbConnection);

    xcb_generic_error_t* fontError = xcb_request_check(system.mXcbConnection, openFontCookie);
    if (fontError != nullptr)
    {
            LogError("ERROR: Could not open font: %d", fontError->error_code);
            LogError("You can check available fonts on the system by using 'xlsfonts' command in the terminal.");
            exit(1);
    }

    xcb_create_glyph_cursor (
        system.mXcbConnection,
        system.mNullCursor,
        cursorFont, /* font for the source glyph */
        cursorFont, /* font for the mask glyph or XCB_NONE */
        ' ',        /* character glyph for the source */
        ' ',        /* character glyph for the mask */
        0,          /* red value for the foreground of the source */
        0,          /* green value for the foreground of the source */
        0,          /* blue value for the foreground of the source */
        0,          /* red value for the background of the source */
        0,          /* green value for the background of the source */
        0 );        /* blue value for the background of the source */

}

void SYS_Shutdown()
{
    SystemState& system = GetEngineState()->mSystem;

    xcb_free_cursor (system.mXcbConnection, system.mNullCursor);

    if (system.mXcbWindow != 0)
    {
        xcb_destroy_window(system.mXcbConnection, system.mXcbWindow);
    }
    
    if (system.mXcbConnection != nullptr)
    {
        xcb_disconnect(system.mXcbConnection);
    }
}

void SYS_Update()
{
    int32_t prevMouseX = 0;
    int32_t prevMouseY = 0;
    INP_GetMousePosition(prevMouseX, prevMouseY);

    SystemState& system = GetEngineState()->mSystem;
    xcb_generic_event_t* event;
    while ((event = xcb_poll_for_event(system.mXcbConnection)))
    {
        HandleXcbEvent(event);
        free(event);
    }

    static bool sPrevWarped = false;
    bool warped = false;

    if (gWarpCursor)
    {
        warped = true;
        SystemState& system = GetEngineState()->mSystem;
        xcb_warp_pointer(
            system.mXcbConnection,
            XCB_NONE,
            system.mXcbWindow,
            0,
            0,
            0,
            0,
            gWarpCursorX,
            gWarpCursorY);

        xcb_flush(system.mXcbConnection);


        gWarpCursor = false;
    }

    if (warped != sPrevWarped)
    {
        GetEngineState()->mInput.mMouseDeltaX = 0;
        GetEngineState()->mInput.mMouseDeltaY = 0;
    }
    else
    {
        int32_t newMouseX = 0;
        int32_t newMouseY = 0;
        INP_GetMousePosition(newMouseX, newMouseY);

        GetEngineState()->mInput.mMouseDeltaX = (newMouseX - prevMouseX);
        GetEngineState()->mInput.mMouseDeltaY = (newMouseY - prevMouseY);
    }

    sPrevWarped = warped;
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
    char filename[1024] = {};
    char command[1024] = {};

    const std::string& projDir = SYS_GetCurrentDirectoryPath() + GetEngineState()->mProjectDirectory;
    snprintf(command, 1024, "zenity --file-selection --filename=%s", projDir.c_str());
    
    FILE *f = popen(command, "r");
    fgets(filename, 1024, f);

    if (filename[0] != 0)
    {
        char* newLineChar = strrchr(filename, '\n');

        if (newLineChar != nullptr)
        {
            *newLineChar = 0;
        }
    }

    pclose(f);

    return std::string(filename);
}

std::string SYS_SaveFileDialog()
{
    char filename[1024];
    char command[1024];

    const std::string& projDir = GetEngineState()->mProjectDirectory;
    snprintf(command, 1024, "zenity --file-selection --save --filename=%s", projDir.c_str());

    FILE *f = popen(command, "r");
    fgets(filename, 1024, f);

    char* newLineChar = strrchr(filename, '\n');
    *newLineChar = 0;

    return std::string(filename);
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
    vprintf(format, arg);
    printf("\n");
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
