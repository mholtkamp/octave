#pragma once

#include "EngineTypes.h"
#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <mutex>

#if PLATFORM_WINDOWS
#include <Windows.h>
#endif

enum class FileAction
{
    Added,
    Modified,
    Removed,
    Renamed
};

struct FileChangeEvent
{
    std::string filePath;
    FileAction action;
    std::string oldPath; // For rename events
};

using FileChangeCallback = std::function<void(const FileChangeEvent&)>;

class FileWatcher
{
public:
    FileWatcher();
    ~FileWatcher();

    // Initialize the file watcher
    bool Initialize();
    
    // Shutdown the file watcher
    void Shutdown();
    
    // Add a directory to watch
    bool WatchDirectory(const std::string& directory, bool recursive = true);
    
    // Remove a directory from watching
    void UnwatchDirectory(const std::string& directory);
    
    // Set callback for file change events
    void SetFileChangeCallback(FileChangeCallback callback);
    
    // Update function to process events (called from main thread)
    void Update();
    
    // Enable/disable the file watcher
    void SetEnabled(bool enabled);
    bool IsEnabled() const { return mEnabled; }

private:
    void WatcherThread();
    void ProcessEvents();
    
#if PLATFORM_WINDOWS
    struct WatchInfo
    {
        HANDLE directoryHandle;
        OVERLAPPED overlapped;
        char buffer[8192];
        std::string path;
        bool recursive;
    };
    
    std::vector<WatchInfo> mWatchInfos;
    HANDLE mCompletionPort;
#endif

    std::thread mWatcherThread;
    std::atomic<bool> mRunning;
    std::atomic<bool> mEnabled;
    
    FileChangeCallback mCallback;
    
    std::vector<FileChangeEvent> mPendingEvents;
    std::mutex mEventsMutex;
    
    // Track last modification times to avoid duplicate events
    std::unordered_map<std::string, uint64_t> mLastModifyTimes;
};

// Global file watcher instance
FileWatcher* GetFileWatcher();
void CreateFileWatcher();
void DestroyFileWatcher();
