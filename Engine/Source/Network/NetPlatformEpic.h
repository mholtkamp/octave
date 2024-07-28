#pragma once

#include "Network/NetPlatform.h"

#if NET_PLATFORM_EOS

#include "eos_sdk.h"
#include "eos_logging.h"
#include "eos_auth.h"

#if PLATFORM_WINDOWS
#include "Windows/eos_Windows.h"
#endif

class NetPlatformEpic : public NetPlatform
{
public:

    // Init/Shutdown
    virtual bool Create() override;
    virtual void Destroy() override;
    virtual void Update() override;

    // Login
    virtual void Login() override;
    virtual void Logout() override;
    virtual bool IsLoggedIn() const override;

    // Matchmaking
    virtual void OpenSession() override;
    virtual void CloseSession() override;
    virtual void BeginSessionSearch() override;
    virtual void EndSessionSearch() override;
    virtual void UpdateSearch() override;
    virtual bool IsSearching() const override;

protected:

    static void LoginCompleteCb(const EOS_Auth_LoginCallbackInfo* Data);
    static void LogoutCompleteCb(const EOS_Auth_LogoutCallbackInfo* Data);

    std::string mGameName;
    EOS_HPlatform mPlatformHandle = nullptr;
    EOS_EpicAccountId mAccountId = {};
    bool mInitialized = false;
    bool mShuttingDown = false;
};

#else

class NetPlatformEpic : public NetPlatform
{

};

#endif