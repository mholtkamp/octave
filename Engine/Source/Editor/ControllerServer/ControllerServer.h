#pragma once

#if EDITOR

#include "ControllerServerTypes.h"

#include <atomic>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

// Forward-declared PIMPL — hides crow::SimpleApp from the header to avoid WinSock conflicts
struct ControllerServerImpl;

class ControllerServer
{
public:
    static void Create();
    static void Destroy();
    static ControllerServer* Get();

    void Start(int port);
    void Stop();
    void Restart(int port);
    bool IsRunning() const;

    void Tick();

    std::future<std::string> QueueCommand(std::function<std::string()> fn);

    void SetLogRequests(bool log);
    bool GetLogRequests() const;

private:
    ControllerServer();
    ~ControllerServer();

    static ControllerServer* sInstance;

    std::unique_ptr<ControllerServerImpl> mImpl;
    std::future<void> mServerFuture;
    std::atomic<bool> mRunning{ false };
    std::atomic<bool> mLogRequests{ false };
    int mPort = 7890;

    std::mutex mQueueMutex;
    std::queue<std::unique_ptr<ControllerCommand>> mCommandQueue;
};

#endif
