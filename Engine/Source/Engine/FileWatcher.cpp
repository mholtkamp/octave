#include "FileWatcher.h"
#include "Log.h"
#include "System/System.h"
#include "EngineTypes.h"
#include "Engine.h"

#if PLATFORM_WINDOWS
#include <Windows.h>
#include <winbase.h>
#endif

static FileWatcher* sFileWatcher = nullptr;

FileWatcher* GetFileWatcher()
{
    return sFileWatcher;
}

void CreateFileWatcher()
{
    if (sFileWatcher == nullptr)
    {
        sFileWatcher = new FileWatcher();
    }
}

void DestroyFileWatcher()
{
    if (sFileWatcher != nullptr)
    {
        delete sFileWatcher;
        sFileWatcher = nullptr;
    }
}

FileWatcher::FileWatcher()
    : mRunning(false)
    , mEnabled(true)
#if PLATFORM_WINDOWS
    , mCompletionPort(INVALID_HANDLE_VALUE)
#endif
{
}

FileWatcher::~FileWatcher()
{
    Shutdown();
}

bool FileWatcher::Initialize()
{
#if PLATFORM_WINDOWS
    // Create I/O completion port
    mCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (mCompletionPort == INVALID_HANDLE_VALUE)
    {
        LogError("Failed to create I/O completion port for FileWatcher");
        return false;
    }
    
    mRunning = true;
    mWatcherThread = std::thread(&FileWatcher::WatcherThread, this);
    
    return true;
#else
    LogWarning("FileWatcher not implemented for this platform");
    return false;
#endif
}

void FileWatcher::Shutdown()
{
    if (mRunning)
    {
        mRunning = false;
        
#if PLATFORM_WINDOWS
        // Signal completion port to wake up the thread
        if (mCompletionPort != INVALID_HANDLE_VALUE)
        {
            PostQueuedCompletionStatus(mCompletionPort, 0, 0, NULL);
        }
#endif

        if (mWatcherThread.joinable())
        {
            mWatcherThread.join();
        }

#if PLATFORM_WINDOWS
        // Clean up watch infos
        for (auto& watchInfo : mWatchInfos)
        {
            if (watchInfo.directoryHandle != INVALID_HANDLE_VALUE)
            {
                CloseHandle(watchInfo.directoryHandle);
            }
        }
        mWatchInfos.clear();
        
        if (mCompletionPort != INVALID_HANDLE_VALUE)
        {
            CloseHandle(mCompletionPort);
            mCompletionPort = INVALID_HANDLE_VALUE;
        }
#endif
    }
}

bool FileWatcher::WatchDirectory(const std::string& directory, bool recursive)
{
#if PLATFORM_WINDOWS
    if (!mRunning)
    {
        LogError("FileWatcher not initialized");
        return false;
    }
    
    LogDebug("Attempting to watch directory: %s (recursive: %s)", directory.c_str(), recursive ? "true" : "false");
    
    // Convert to wide string
    std::wstring wDirectory = std::wstring(directory.begin(), directory.end());
    
    // Open directory handle
    HANDLE dirHandle = CreateFileW(
        wDirectory.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        NULL
    );
    
    if (dirHandle == INVALID_HANDLE_VALUE)
    {
        LogError("Failed to open directory for watching: %s", directory.c_str());
        return false;
    }
    
    // Associate with completion port
    if (CreateIoCompletionPort(dirHandle, mCompletionPort, (ULONG_PTR)mWatchInfos.size(), 0) == NULL)
    {
        LogError("Failed to associate directory with completion port: %s", directory.c_str());
        CloseHandle(dirHandle);
        return false;
    }
    
    // Create watch info
    WatchInfo watchInfo = {};
    watchInfo.directoryHandle = dirHandle;
    watchInfo.path = directory;
    watchInfo.recursive = recursive;
    
    mWatchInfos.push_back(watchInfo);
    size_t watchIndex = mWatchInfos.size() - 1;
    
    // Start watching
    DWORD notifyFilter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION;
    
    BOOL result = ReadDirectoryChangesW(
        dirHandle,
        mWatchInfos[watchIndex].buffer,
        sizeof(mWatchInfos[watchIndex].buffer),
        recursive ? TRUE : FALSE,
        notifyFilter,
        NULL,
        &mWatchInfos[watchIndex].overlapped,
        NULL
    );
    
    if (!result)
    {
        LogError("Failed to start watching directory: %s", directory.c_str());
        CloseHandle(dirHandle);
        mWatchInfos.pop_back();
        return false;
    }
    
    return true;
#else
    return false;
#endif
}

void FileWatcher::UnwatchDirectory(const std::string& directory)
{
#if PLATFORM_WINDOWS
    for (auto it = mWatchInfos.begin(); it != mWatchInfos.end(); ++it)
    {
        if (it->path == directory)
        {
            CloseHandle(it->directoryHandle);
            mWatchInfos.erase(it);
            break;
        }
    }
#endif
}

void FileWatcher::SetFileChangeCallback(FileChangeCallback callback)
{
    mCallback = callback;
}

void FileWatcher::Update()
{
    if (!mEnabled)
        return;
        
    ProcessEvents();
}

void FileWatcher::SetEnabled(bool enabled)
{
    mEnabled = enabled;
}

void FileWatcher::WatcherThread()
{
#if PLATFORM_WINDOWS
    while (mRunning)
    {
        DWORD bytesTransferred;
        ULONG_PTR completionKey;
        LPOVERLAPPED overlapped;
        
        BOOL result = GetQueuedCompletionStatus(
            mCompletionPort,
            &bytesTransferred,
            &completionKey,
            &overlapped,
            INFINITE
        );
        
        if (!mRunning)
            break;
            
        if (result && overlapped != nullptr)
        {
            size_t watchIndex = completionKey;
            if (watchIndex < mWatchInfos.size())
            {
                WatchInfo& watchInfo = mWatchInfos[watchIndex];
                
                if (bytesTransferred > 0)
                {
                    // Process file change notifications
                    FILE_NOTIFY_INFORMATION* notify = (FILE_NOTIFY_INFORMATION*)watchInfo.buffer;
                    
                    while (true)
                    {
                        // Convert filename to narrow string
                        int filenameLength = notify->FileNameLength / sizeof(WCHAR);
                        std::wstring wFilename(notify->FileName, filenameLength);
                        
                        // Convert wide string to narrow string properly
                        std::string filename;
                        if (!wFilename.empty())
                        {
                            int size = WideCharToMultiByte(CP_UTF8, 0, wFilename.c_str(), -1, nullptr, 0, nullptr, nullptr);
                            if (size > 0)
                            {
                                filename.resize(size - 1); // -1 to exclude null terminator
                                WideCharToMultiByte(CP_UTF8, 0, wFilename.c_str(), -1, &filename[0], size, nullptr, nullptr);
                            }
                        }
                        
                        std::string fullPath = watchInfo.path + "/" + filename;
                        
                        // Determine action
                        FileAction action;
                        switch (notify->Action)
                        {
                            case FILE_ACTION_ADDED:
                                action = FileAction::Added;
                                break;
                            case FILE_ACTION_REMOVED:
                                action = FileAction::Removed;
                                break;
                            case FILE_ACTION_MODIFIED:
                                action = FileAction::Modified;
                                break;
                            case FILE_ACTION_RENAMED_OLD_NAME:
                            case FILE_ACTION_RENAMED_NEW_NAME:
                                action = FileAction::Renamed;
                                break;
                            default:
                                action = FileAction::Modified;
                                break;
                        }
                        
                        // Add to pending events
                        {
                            std::lock_guard<std::mutex> lock(mEventsMutex);
                            FileChangeEvent event;
                            event.filePath = fullPath;
                            event.action = action;
                            mPendingEvents.push_back(event);
                        }
                        
                        if (notify->NextEntryOffset == 0)
                            break;
                            
                        notify = (FILE_NOTIFY_INFORMATION*)((char*)notify + notify->NextEntryOffset);
                    }
                }
                
                // Continue watching
                DWORD notifyFilter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION;
                
                ZeroMemory(&watchInfo.overlapped, sizeof(OVERLAPPED));
                ReadDirectoryChangesW(
                    watchInfo.directoryHandle,
                    watchInfo.buffer,
                    sizeof(watchInfo.buffer),
                    watchInfo.recursive ? TRUE : FALSE,
                    notifyFilter,
                    NULL,
                    &watchInfo.overlapped,
                    NULL
                );
            }
        }
    }
#endif
}

void FileWatcher::ProcessEvents()
{
    std::vector<FileChangeEvent> eventsToProcess;
    
    {
        std::lock_guard<std::mutex> lock(mEventsMutex);
        eventsToProcess = std::move(mPendingEvents);
        mPendingEvents.clear();
    }
    
    for (const auto& event : eventsToProcess)
    {
        // Check if this is a script file (.lua extension)
        if (event.filePath.size() >= 4 && 
            event.filePath.substr(event.filePath.size() - 4) == ".lua")
        {
            // Check for duplicate events by comparing modification times
            uint64_t currentTime = SYS_GetTimeMicroseconds();
            auto it = mLastModifyTimes.find(event.filePath);
            
            if (it != mLastModifyTimes.end())
            {
                // If the last modification was less than 100ms ago, skip this event
                if (currentTime - it->second < 100000) // 100ms in microseconds
                {
                    continue;
                }
            }
            
            mLastModifyTimes[event.filePath] = currentTime;
            
            if (mCallback)
            {
                mCallback(event);
            }
        }
    }
}
