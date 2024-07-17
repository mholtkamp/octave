#pragma once

#include "Network/NetPlatform.h"

#if NET_PLATFORM_STEAM

#include "steam_api.h"
#include "isteamuserstats.h"
#include "isteamremotestorage.h"
#include "isteammatchmaking.h"
#include "steam_gameserver.h"

class NetPlatformSteam : public NetPlatform
{
public:

    // Init/Shutdown
    virtual void Create() override;
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

    //static void LoginCompleteCb(const EOS_Auth_LoginCallbackInfo* Data);
    //static void LogoutCompleteCb(const EOS_Auth_LogoutCallbackInfo* Data);
};

#else

class NetPlatformSteam : public NetPlatform
{

};

#endif
