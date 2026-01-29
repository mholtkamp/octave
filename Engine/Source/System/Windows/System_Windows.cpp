#if PLATFORM_WINDOWS

#include "System/System.h"
#include "System/SystemUtils.h"
#include "Graphics/Graphics.h"

#include "Engine.h"
#include "Renderer.h"
#include "Log.h"
#include "Input/Input.h"

#include <direct.h>
#include <chrono>
#include <psapi.h>
#include <Shlobj.h>
#include <assert.h>
#include <errno.h>

#if EDITOR
#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include <sys/stat.h>
#include <string>
#include <fstream>

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // EDITOR

#define OCT_WINDOWED_STYLE_FLAGS (WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_SYSMENU)
#define OCT_FULLSCREEN_STYLE_FLAGS (OCT_WINDOWED_STYLE_FLAGS & ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU))

// MS-Windows event handling function:
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#if EDITOR
    if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true;
#endif

    EngineState* engineState = GetEngineState();

    switch (uMsg) {
    case WM_CLOSE:
        PostQuitMessage(0);
        break;
    case WM_SIZE:
    {
        WORD width = LOWORD(lParam);
        WORD height = HIWORD(lParam);

        ResizeWindow(width, height);
        break;
    }
    case WM_KEYDOWN:
    {
        INP_SetKey((int32_t)wParam);
        return 0;
    }

    case WM_KEYUP:
    {
        INP_ClearKey((int32_t)wParam);
        return 0;
    }

    case WM_LBUTTONDOWN:
    {
        INP_SetMouseButton(MOUSE_LEFT);
        return 0;
    }

    case WM_RBUTTONDOWN:
    {
        INP_SetMouseButton(MOUSE_RIGHT);
        return 0;
    }

    case WM_MBUTTONDOWN:
    {
        INP_SetMouseButton(MOUSE_MIDDLE);
        return 0;
    }

    case WM_XBUTTONDOWN:
    {
        int nButton = HIWORD(wParam);

        if (nButton == 1)
        {
            INP_SetMouseButton(MOUSE_X1);
        }
        else if (nButton == 2)
        {
            INP_SetMouseButton(MOUSE_X2);
        }

        return 0;
    }

    case WM_LBUTTONUP:
    {
        INP_ClearMouseButton(MOUSE_LEFT);
        return 0;
    }

    case WM_RBUTTONUP:
    {
        INP_ClearMouseButton(MOUSE_RIGHT);
        return 0;
    }

    case WM_MBUTTONUP:
    {
        INP_ClearMouseButton(MOUSE_MIDDLE);
        return 0;
    }

    case WM_XBUTTONUP:
    {
        int nButton = HIWORD(wParam);

        if (nButton == 1)
        {
            INP_ClearMouseButton(MOUSE_X1);
        }
        else if (nButton == 2)
        {
            INP_ClearMouseButton(MOUSE_X2);
        }

        return 0;
    }

    case WM_MOUSEWHEEL:
    {
        INP_SetScrollWheelDelta(GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);

        return 0;
    }
    case WM_MOUSEMOVE:
    {
        int nX = LOWORD(lParam);
        int nY = HIWORD(lParam);
        INP_SetMousePosition(nX, nY);

        return 0;
    }

    case WM_INPUT:
    {
        uint8_t buffer[128];

        uint32_t bufferSize = 0;
        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &bufferSize, sizeof(RAWINPUTHEADER));

        OCT_ASSERT(bufferSize < 128);

        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buffer, &bufferSize, sizeof(RAWINPUTHEADER));

        RAWINPUT *raw = (RAWINPUT*)buffer;
        if (raw->header.dwType == RIM_TYPEMOUSE)
        {
            int32_t x = raw->data.mouse.lLastX;
            int32_t y = raw->data.mouse.lLastY;

            if (raw->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE)
            {
                static int32_t prevX = x;
                static int32_t prevY = y;

                engineState->mInput.mMouseDeltaX += (x - prevX);
                engineState->mInput.mMouseDeltaY += (y - prevY);

                prevX = x;
                prevY = y;
            }
            else
            {
                engineState->mInput.mMouseDeltaX += x;
                engineState->mInput.mMouseDeltaY += y;
            }
        }

        return 0;
    }

    case WM_SYSKEYDOWN:
    {
        INP_SetKey((int32_t)wParam);
        return 0;
    }

    case WM_SYSKEYUP:
    {
        INP_ClearKey((int32_t)wParam);
        return 0;
    }

    case WM_MENUCHAR:
    {
        return (MNC_CLOSE << 16);
    }

    case WM_SETFOCUS:
    {
        engineState->mSystem.mWindowHasFocus = true;

        INP_TrapCursor(INP_IsCursorTrapped());

        return 0;
    }

    case WM_KILLFOCUS:
    {
        engineState->mSystem.mWindowHasFocus = false;
        INP_ClearAllKeys();
        INP_ClearAllMouseButtons();

        ClipCursor(nullptr);

        return 0;
    }

    //case WM_PAINT:
    //    // The validation callback calls MessageBox which can generate paint
    //    // events - don't make more Vulkan calls if we got here from the
    //    // callback
    //    if (!sEngineState.mInCallback)
    //    {
    //        if (Renderer::Get() != nullptr)
    //        {
    //            Renderer::Get()->Render();
    //        }
    //    }
    //    break;
    case WM_GETMINMAXINFO:     // set window's minimum size
        ((MINMAXINFO*)lParam)->ptMinTrackSize = engineState->mSystem.mMinSize;
        return 0;
    default:
        break;
    }
    return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

void SYS_Initialize()
{
    // Without the CoInitialize call I'm getting heap corruption??
    // Apparently this is needed for anything using COM objects?? 
    // I think XAudio and XInput require this.
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    EngineState* engineState = GetEngineState();
    HINSTANCE hInst = GetModuleHandle(NULL); // hInstance
    engineState->mSystem.mConnection = hInst;

    WNDCLASSEX win_class;

    char exeName[1024];
    GetModuleFileName(hInst, exeName, 1024);
    HICON icon = ExtractIcon(hInst, exeName, 0);

    // Initialize the window class structure:
    win_class.cbSize = sizeof(WNDCLASSEX);
    win_class.style = CS_HREDRAW | CS_VREDRAW;
    win_class.lpfnWndProc = WndProc;
    win_class.cbClsExtra = 0;
    win_class.cbWndExtra = 0;
    win_class.hInstance = hInst;
    win_class.hIcon = icon;
    win_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    win_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    win_class.lpszMenuName = NULL;
    win_class.lpszClassName = engineState->mProjectName.c_str();
    win_class.hIconSm = icon;

    // Register window class:
    if (!RegisterClassEx(&win_class)) {
        // It didn't work, so try to give a useful error:
        printf("Unexpected error trying to start the application!\n");
        fflush(stdout);
        exit(1);
    }
    // Create window with the registered class:
    uint32_t winWidth = GetEngineConfig()->mWindowWidth;
    uint32_t winHeight = GetEngineConfig()->mWindowHeight;
    engineState->mWindowWidth = winWidth;
    engineState->mWindowHeight = winHeight;

    RECT wr = { 0, 0, LONG(winWidth), LONG(winHeight) };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    engineState->mSystem.mWindow = CreateWindowEx(0,
        engineState->mProjectName.c_str(), // class name
        engineState->mProjectName.c_str(), // app name
        OCT_WINDOWED_STYLE_FLAGS,
        100, 100,           // x/y coordinates
        wr.right - wr.left, // width
        wr.bottom - wr.top, // height
        NULL,               // handle to parent
        NULL,               // handle to menu
        hInst,   // hInstance
        NULL);              // no extra parameters
    if (!engineState->mSystem.mWindow)
    {
        // It didn't work, so try to give a useful error:
        LogError("Cannot create a window in which to draw!\n");
        fflush(stdout);
        exit(1);
    }
    // Window client area size must be at least 1 pixel high, to prevent crash.
    engineState->mSystem.mMinSize.x = GetSystemMetrics(SM_CXMINTRACK);
    engineState->mSystem.mMinSize.y = GetSystemMetrics(SM_CYMINTRACK) + 1;

    if (GetEngineConfig()->mFullscreen)
    {
        SYS_SetFullscreen(true);
    }

#if EDITOR
    ImGui_ImplWin32_Init(engineState->mSystem.mWindow);
#endif
}

void SYS_Shutdown()
{
#if EDITOR
    ImGui_ImplWin32_Shutdown();
#endif
}

void SYS_Update()
{
    MSG     msg;
    BOOL    done = FALSE;

    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            GetEngineState()->mQuit = true;
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    if (INP_IsKeyDown(KEY_ALT_L) || INP_IsKeyDown(KEY_ALT_R))
    {
        if (INP_IsKeyJustDown(KEY_ENTER))
        {
            SYS_SetFullscreen(!GetEngineState()->mSystem.mFullscreen);
        }

        if (INP_IsKeyJustDown(KEY_F4))
        {
            Quit();
        }
    }

#if EDITOR
    ImGui_ImplWin32_NewFrame();
#endif
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
std::string SYS_GetOctavePath()
{
    std::string octaveDirectory = SYS_GetCurrentDirectoryPath();
    if(SYS_DoesFileExist((octaveDirectory + "Octave/imgui.ini").c_str(), false)){
        octaveDirectory = octaveDirectory + "Octave/";
        }
    if(!SYS_DoesFileExist((octaveDirectory + "Standalone/Standalone.rc").c_str(), false)){
        std::string octaveEXE = SYS_GetExecutablePath();
        size_t lastSlash = octaveEXE.find_last_of("\\/");
        octaveDirectory = octaveEXE.substr(0, lastSlash + 1);

    }
    return octaveDirectory;
}
std::string SYS_GetExecutablePath()
{
    char path[MAX_PATH_SIZE] = {};
    GetModuleFileName(NULL, path, MAX_PATH_SIZE);
    return std::string(path);
}

std::string SYS_GetCurrentDirectoryPath()
{
    char path[MAX_PATH_SIZE] = {};
    _getcwd(path, MAX_PATH_SIZE);
    return std::string(path) + "/";
}

std::string SYS_GetAbsolutePath(const std::string& relativePath)
{
    char absPath[MAX_PATH_SIZE];
    GetFullPathName(relativePath.c_str(), MAX_PATH_SIZE, absPath, nullptr);
    return absPath;
}


std::string SYS_GetFileName(const std::string& path)
{
    char fileName[MAX_PATH_SIZE];
    _splitpath_s(
        path.c_str(),
        nullptr, 0,
        nullptr, 0,
        fileName, MAX_PATH_SIZE,
        nullptr, 0);
    return std::string(fileName);
    
}

void SYS_SetWorkingDirectory(const std::string& dirPath)
{
    _chdir(dirPath.c_str());
}

bool SYS_CreateDirectory(const char* dirPath)
{
    int32_t ret = _mkdir(dirPath);

    if (ret < 0)
    {
        LogWarning("_mkdir error: %s", strerror(errno));
    }

    return (ret == 0);
}

void SYS_RemoveDirectory(const char* dirPath)
{
    std::string path = dirPath;
    for (uint32_t i = 0; i < path.length(); ++i)
    {
        if (path[i] == '/')
        {
            path[i] = '\\';
        }
    }

    std::string cmd = std::string("rmdir ") + path + " /s /q";
    SYS_Exec(cmd.c_str());
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

    std::string dirString = dirPath + "*";
    outDirEntry.mFindHandle = FindFirstFile(dirString.c_str(), &outDirEntry.mFindData);
    if (outDirEntry.mFindHandle == INVALID_HANDLE_VALUE)
    {
        LogError("Invalid first file");
        return;
    }

    // Init first DirEntry
    memcpy(outDirEntry.mFilename, outDirEntry.mFindData.cFileName, MAX_PATH_SIZE);
    outDirEntry.mDirectory = outDirEntry.mFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
    outDirEntry.mValid = true;
}

void SYS_IterateDirectory(DirEntry& dirEntry)
{
    if (FindNextFile(dirEntry.mFindHandle, &dirEntry.mFindData))
    {
        memcpy(dirEntry.mFilename, dirEntry.mFindData.cFileName, MAX_PATH_SIZE);
        dirEntry.mDirectory = dirEntry.mFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
        dirEntry.mValid = true;
    }
    else
    {
        dirEntry.mValid = false;
    }
}

void SYS_CloseDirectory(DirEntry& dirEntry)
{
    FindClose(dirEntry.mFindHandle);
    dirEntry.mFindHandle = nullptr;
}

std::vector<std::string> SYS_OpenFileDialog()
{
    std::vector<std::string> retPaths;

    OPENFILENAME ofn; // common dialog box structure
    char szFile[1024 * 16]; // buffer for file name
    HWND hwnd = GetActiveWindow(); // owner window

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    // Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
    // use the contents of szFile to initialize itself.
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = nullptr;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_ALLOWMULTISELECT | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_EXPLORER;

    // Display the Open dialog box. 
    if (GetOpenFileName(&ofn) != 0)
    {
        // First string contains directory path
        char* str = ofn.lpstrFile;
        std::string firstStr = str;
        str += (firstStr.length() + 1);

        if (*str != '\0')
        {
            std::string dirStr = firstStr;

            // There are multiple files. First string was dir name.
            while (*str)
            {
                std::string fileStr = str;
                str += (fileStr.length() + 1);

                fileStr = dirStr + "/" + fileStr;
                retPaths.push_back(fileStr);
            }
        }
        else
        {
            // There was only one file
            retPaths.push_back(firstStr);
        }
    }

    // Convert backslash to forward slashes.
    for (uint32_t i = 0; i < retPaths.size(); ++i)
    {
        std::string& retPath = retPaths[i];
        for (uint32_t i = 0; i < retPath.size(); ++i)
        {
            if (retPath[i] == '\\')
            {
                retPath[i] = '/';
            }
        }
    }

    return retPaths;
}

std::string SYS_SaveFileDialog()
{
    std::string retPath = "";

    OPENFILENAME ofn; // common dialog box structure
    char szFile[260]; // buffer for file name
    HWND hwnd = GetActiveWindow(); // owner window

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    // Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
    // use the contents of szFile to initialize itself.
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = nullptr;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

    if (GetSaveFileName(&ofn) != 0)
    {
        retPath = ofn.lpstrFile;
    }

    // Convert backslash to forward slashes.
    for (uint32_t i = 0; i < retPath.size(); ++i)
    {
        if (retPath[i] == '\\')
        {
            retPath[i] = '/';
        }
    }

    return retPath;
}

static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    if (uMsg == BFFM_INITIALIZED)
    {
        std::string tmp = (const char *)lpData;
        LogDebug("path: %s", tmp.c_str());
        SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
    }

    return 0;
}

std::string SYS_SelectFolderDialog()
{
    std::string retString = "";
    IFileDialog* fileDialog = nullptr;

    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&fileDialog));

    if (SUCCEEDED(hr))
    {
        hr = fileDialog->SetOptions(FOS_PICKFOLDERS);

        if (SUCCEEDED(hr))
        {
            hr = fileDialog->Show(NULL);

            if (SUCCEEDED(hr))
            {
                IShellItem *selectedShellItem;
                hr = fileDialog->GetResult(&selectedShellItem);

                if (SUCCEEDED(hr))
                {
                    PWSTR wName;
                    hr = selectedShellItem->GetDisplayName(SIGDN_FILESYSPATH, &wName);

                    if (SUCCEEDED(hr))
                    {
                        std::wstring wideString = wName;
                        for (uint32_t i = 0; i < wideString.length(); ++i)
                        {
                            char ansiChar = (char) wideString.at(i);
                            retString.push_back(ansiChar);
                        }

                        CoTaskMemFree(wName);
                    }

                    selectedShellItem->Release();
                }
            }
        }

        fileDialog->Release();
    }

    return retString;
}


// Threads
ThreadObject* SYS_CreateThread(ThreadFuncFP func, void* arg)
{
    ThreadObject* retThread = new ThreadObject();

    *retThread = CreateThread(
        NULL,                   // default security attributes
        0,                      // use default stack size  
        func,                   // thread function name
        arg,                    // argument to thread function 
        0,                      // use default creation flags 
        nullptr);               // returns the thread identifier 

    if (*retThread == 0)
    {
        LogError("Failed to create Thread");
    }

    return retThread;
}

void SYS_JoinThread(ThreadObject* thread)
{
    WaitForSingleObject(*thread, INFINITE);
}

void SYS_DestroyThread(ThreadObject* thread)
{
    CloseHandle(*thread);
    delete thread;
}

MutexObject* SYS_CreateMutex()
{
    MutexObject* retMutex = new MutexObject();

    *retMutex = CreateMutex(
        NULL,              // default security attributes
        FALSE,             // initially not owned
        NULL);             // unnamed mutex

    if (*retMutex == 0)
    {
        LogError("Failed to create Mutex");
    }

    return retMutex;
}

bool SYS_CopyDirectoryRecursive(const std::string& sourceDir,
                                const std::string& destDir)
{
    std::string source = sourceDir;
    std::string dest = destDir;

    for (char& c : source) if (c == '/') c = '\\';
    for (char& c : dest)   if (c == '/') c = '\\';

    std::string cmd =
        "xcopy \"" + source + "\\*\" \"" + dest + "\\\" /E /I /Y /Q";

    SYS_Exec(cmd.c_str());
	return true;
}


void SYS_CopyDirectory(const char* sourceDir, const char* destDir)
{
    std::string source = sourceDir;
    std::string dest = destDir;
    for (uint32_t i = 0; i < source.length(); ++i)
    {
        if (source[i] == '/')
        {
            source[i] = '\\';
        }
    }
    for (uint32_t i = 0; i < dest.length(); ++i)
    {
        if (dest[i] == '/')
        {
            dest[i] = '\\';
        }
    }
    std::string cmd = std::string("xcopy \"") + source + "\" \"" + dest + "\" /E /I /Y";
    SYS_Exec(cmd.c_str());
}

void SYS_CopyFile(const char* sourcePath, const char* destPath)
{
    std::string source = sourcePath;
    std::string dest = destPath;
    for (uint32_t i = 0; i < source.length(); ++i)
    {
        if (source[i] == '/')
        {
            source[i] = '\\';
        }
    }
    for (uint32_t i = 0; i < dest.length(); ++i)
    {
        if (dest[i] == '/')
        {
            dest[i] = '\\';
        }
    }
    std::string cmd = std::string("copy \"") + source + "\" \"" + dest + "\" /Y";
    SYS_Exec(cmd.c_str());
}

void SYS_MoveDirectory(const char* sourceDir, const char* destDir)
{
    std::string source = sourceDir;
    std::string dest = destDir;
    for (uint32_t i = 0; i < source.length(); ++i)
    {
        if (source[i] == '/')
        {
            source[i] = '\\';
        }
    }
    for (uint32_t i = 0; i < dest.length(); ++i)
    {
        if (dest[i] == '/')
        {
            dest[i] = '\\';
        }
    }
    std::string cmd = std::string("move \"") + source + "\" \"" + dest + "\"";
    SYS_Exec(cmd.c_str());
}

void SYS_MoveFile(const char* sourcePath, const char* destPath)
{
    std::string source = sourcePath;
    std::string dest = destPath;
    for (uint32_t i = 0; i < source.length(); ++i)
    {
        if (source[i] == '/')
        {
            source[i] = '\\';
        }
    }
    for (uint32_t i = 0; i < dest.length(); ++i)
    {
        if (dest[i] == '/')
        {
            dest[i] = '\\';
        }
    }
    std::string cmd = std::string("move \"") + source + "\" \"" + dest + "\"";
    SYS_Exec(cmd.c_str());
}

void SYS_LockMutex(MutexObject* mutex)
{
    DWORD dwWaitResult = WaitForSingleObject(
        *mutex,      // handle to mutex
        INFINITE);  // no time-out interval

    OCT_UNUSED(dwWaitResult);
}

void SYS_UnlockMutex(MutexObject* mutex)
{
    if (!ReleaseMutex(*mutex))
    {
        LogError("Error releasing mutex");
    }
}

void SYS_DestroyMutex(MutexObject* mutex)
{
    CloseHandle(*mutex);
    delete mutex;
}

void SYS_Sleep(uint32_t milliseconds)
{
    Sleep(milliseconds);
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
    ExecCommon(cmd, output);
}

// Memory
void* SYS_AlignedMalloc(uint32_t size, uint32_t alignment)
{
    return _aligned_malloc(size, alignment);
}

void SYS_AlignedFree(void* pointer)
{
    OCT_ASSERT(pointer != nullptr);
    _aligned_free(pointer);
}

std::vector<MemoryStat> SYS_GetMemoryStats()
{
    std::vector<MemoryStat> stats;

    HANDLE procHandle = GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS counters;
    GetProcessMemoryInfo(procHandle, &counters, sizeof(PROCESS_MEMORY_COUNTERS));
    
    {
        MemoryStat stat;
        stat.mName = "Main";
        stat.mBytesFree = 0;
        stat.mBytesAllocated = counters.WorkingSetSize;
        stats.push_back(stat);
    }

    return stats;
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
    SystemState& system = GetEngineState()->mSystem;

    if (!OpenClipboard(system.mWindow))
    {
        LogError("Failed to open clipboard.");
        return;
    }

    EmptyClipboard();

    HGLOBAL hglb = GlobalAlloc(GMEM_MOVEABLE, str.size() + 1);
    if (hglb == nullptr)
    {
        CloseClipboard();
        LogError("Failed to allocate clipboard data.");
        return;
    }

    char* copyStr = (char*) GlobalLock(hglb);
    memcpy(copyStr, str.data(), str.size());
    copyStr[(int32_t)str.size()] = 0;
    GlobalUnlock(hglb);

    SetClipboardData(CF_TEXT, hglb);

    CloseClipboard();
}

std::string SYS_GetClipboardText()
{
    std::string retStr;

    SystemState& system = GetEngineState()->mSystem;

    if (!IsClipboardFormatAvailable(CF_TEXT))
    {
        return retStr;
    }

    if (!OpenClipboard(system.mWindow))
    {
        LogError("Failed to open clipboard.");
        return retStr;
    }

    HGLOBAL hglb = GetClipboardData(CF_TEXT);
    if (hglb != nullptr)
    {
        char* srcStr = (char*) GlobalLock(hglb);
        if (srcStr != nullptr)
        {
            retStr = srcStr;
            GlobalUnlock(hglb);
        }
    }

    CloseClipboard();

    return retStr;
}

// Misc

void SYS_Log(LogSeverity severity, const char* format, va_list arg)
{
    vprintf(format, arg);
    printf("\n");

    // Also print to visual studio
    char msg[1024];
    vsnprintf(msg, 1024, format, arg);
    OutputDebugString(msg);
    OutputDebugString("\n");
}

void SYS_Assert(const char* exprString, const char* fileString, uint32_t lineNumber)
{
    const char* fileName = strrchr(fileString, '\\') ? strrchr(fileString, '\\') + 1 : fileString;
    LogError("[Assert] %s, %s, line %d", exprString, fileName, lineNumber);

    if (IsDebuggerPresent())
    {
        DebugBreak();
    }
}

void SYS_Alert(const char* message)
{
    LogError("%s", message);

    if (IsDebuggerPresent())
    {
        DebugBreak();
    }
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
    SetWindowText(GetEngineState()->mSystem.mWindow, title);
}

bool SYS_DoesWindowHaveFocus()
{
    return GetEngineState()->mSystem.mWindowHasFocus;
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
    static int sSavedX = 0;
    static int sSavedY = 0;
    static int sSavedWidth = 0;
    static int sSavedHeight = 0;
    static int sSavedClientWidth = 0;
    static int sSavedClientHeight = 0;

    SystemState& system = GetEngineState()->mSystem;
    if (system.mFullscreen != fullscreen)
    {
        system.mFullscreen = fullscreen;

        if (fullscreen)
        {
            // First, save the windowed pos and size so we can restore it to the correct location.
            RECT winRect;
            GetWindowRect(system.mWindow, &winRect);

            sSavedX = winRect.left;
            sSavedY = winRect.top;
            sSavedWidth = winRect.right - winRect.left;
            sSavedHeight = winRect.bottom - winRect.top;
            sSavedClientWidth = GetEngineState()->mWindowWidth;
            sSavedClientHeight = GetEngineState()->mWindowHeight;

            int width = GetSystemMetrics(SM_CXSCREEN);
            int height = GetSystemMetrics(SM_CYSCREEN);

            SetWindowLong(system.mWindow, GWL_STYLE, OCT_FULLSCREEN_STYLE_FLAGS);
            SetWindowPos(system.mWindow, HWND_TOP, 0, 0, width, height, 0);
            ResizeWindow(width, height);
        }
        else
        {
            SetWindowLong(system.mWindow, GWL_STYLE, OCT_WINDOWED_STYLE_FLAGS);
            SetWindowPos(system.mWindow, HWND_TOP, sSavedX, sSavedY, sSavedWidth, sSavedHeight, 0);
            ResizeWindow(sSavedClientWidth, sSavedClientHeight);
        }
    }
}

bool SYS_IsFullscreen()
{
    return GetEngineState()->mSystem.mFullscreen;
}

void SYS_SetWindowRect(int32_t x, int32_t y, int32_t width, int32_t height)
{
    if (!SYS_IsFullscreen())
    {
        SystemState& system = GetEngineState()->mSystem;

        SetWindowPos(system.mWindow, HWND_TOP, x, y, width, height, 0);
    }
}

void SYS_GetWindowRect(int32_t& outX, int32_t& outY, int32_t& outWidth, int32_t& outHeight)
{
    SystemState& system = GetEngineState()->mSystem;

    RECT winRect;
    GetWindowRect(system.mWindow, &winRect);

    outX = winRect.left;
    outY = winRect.top;
    outWidth = winRect.right - winRect.left;
    outHeight = winRect.bottom - winRect.top;
}

void SYS_ExplorerOpenDirectory(const std::string& dirPath)
{
    ShellExecute(NULL, "open", dirPath.c_str(), NULL, NULL, SW_SHOWDEFAULT);
}

#endif