#if EDITOR

// WinSock2 must be included before Windows.h to avoid winsock.h conflict with ASIO
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <Windows.h>
#endif

#include "ControllerServer.h"
#include "ControllerServerTypes.h"
#include "ControllerServerRoutes.h"

#include "Log.h"
#include "EditorUIHookManager.h"

#if defined(_MSC_VER)
#pragma warning(push, 0)
#endif

#include "crow.h"

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

// PIMPL implementation — owns the crow::SimpleApp instance
struct ControllerServerImpl
{
    std::unique_ptr<crow::SimpleApp> mApp;
};

ControllerServer* ControllerServer::sInstance = nullptr;

ControllerServer::ControllerServer()
    : mImpl(std::make_unique<ControllerServerImpl>())
{
}

ControllerServer::~ControllerServer()
{
    Stop();
}

void ControllerServer::Create()
{
    Destroy();
    sInstance = new ControllerServer();
}

void ControllerServer::Destroy()
{
    if (sInstance != nullptr)
    {
        delete sInstance;
        sInstance = nullptr;
    }
}

ControllerServer* ControllerServer::Get()
{
    return sInstance;
}

void ControllerServer::Start(int port)
{
    if (mRunning.load())
    {
        return;
    }

    mPort = port;
    mImpl->mApp = std::make_unique<crow::SimpleApp>();

    // Don't let Crow register signal handlers (would interfere with editor)
    mImpl->mApp->signal_clear();
    mImpl->mApp->port(static_cast<uint16_t>(mPort));
    mImpl->mApp->loglevel(crow::LogLevel::Warning);

    // Register all REST endpoints
    RegisterRoutes(mImpl->mApp.get(), this);

    mRunning.store(true);
    mServerFuture = mImpl->mApp->run_async();

    LogDebug("Controller Server started on port %d", mPort);

    EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
    if (hookMgr)
    {
        hookMgr->FireOnControllerServerStateChanged(0); // Started
    }
}

void ControllerServer::Stop()
{
    if (!mRunning.load())
    {
        return;
    }

    mRunning.store(false);

    if (mImpl->mApp)
    {
        mImpl->mApp->stop();
    }

    if (mServerFuture.valid())
    {
        mServerFuture.wait();
    }

    mImpl->mApp.reset();

    LogDebug("Controller Server stopped");

    EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
    if (hookMgr)
    {
        hookMgr->FireOnControllerServerStateChanged(1); // Stopped
    }
}

void ControllerServer::Restart(int port)
{
    Stop();
    Start(port);
}

bool ControllerServer::IsRunning() const
{
    return mRunning.load();
}

void ControllerServer::SetLogRequests(bool log)
{
    mLogRequests.store(log);
}

bool ControllerServer::GetLogRequests() const
{
    return mLogRequests.load();
}

std::future<std::string> ControllerServer::QueueCommand(std::function<std::string()> fn)
{
    auto cmd = std::make_unique<ControllerCommand>();
    cmd->mFunction = std::move(fn);
    std::future<std::string> future = cmd->mPromise.get_future();

    {
        std::lock_guard<std::mutex> lock(mQueueMutex);
        mCommandQueue.push(std::move(cmd));
    }

    return future;
}

void ControllerServer::Tick()
{
    std::queue<std::unique_ptr<ControllerCommand>> batch;

    {
        std::lock_guard<std::mutex> lock(mQueueMutex);
        std::swap(batch, mCommandQueue);
    }

    while (!batch.empty())
    {
        auto cmd = std::move(batch.front());
        batch.pop();

        try
        {
            std::string result = cmd->mFunction();
            cmd->mPromise.set_value(std::move(result));
        }
        catch (...)
        {
            try
            {
                cmd->mPromise.set_exception(std::current_exception());
            }
            catch (...) {}
        }
    }
}

#endif
