#pragma once

#include "Network/NetPlatform.h"

#if NET_PLATFORM_EOS

class NetPlatformEpic : public NetPlatform
{
public:

    // Init/Shutdown
    virtual void Create() override;
    virtual void Destroy() override;
    virtual void Update() override;

    // Login
    virtual void Login() override;
    virtual void Logout() override;

    // Matchmaking
    virtual void OpenSession() override;
    virtual void CloseSession() override;
    virtual void BeginSessionSearch() override;
    virtual void EndSessionSearch() override;
    virtual void UpdateSearch() override;
    virtual bool IsSearching() const override;

protected:

    std::string mGameName;
    EOS_HPlatform mPlatformHandle = nullptr;
    bool mInitialized = false;
    bool mShuttingDown = false;
};

#else

class NetPlatformEpic : public NetPlatform
{

};

#endif