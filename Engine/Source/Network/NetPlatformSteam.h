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
    virtual void JoinSession(uint64_t sessionId) override;
    virtual void BeginSessionSearch() override;
    virtual void EndSessionSearch() override;
    virtual void UpdateSearch() override;
    virtual bool IsSearching() const override;

protected:

    void OnLobbyCreated(LobbyCreated_t* pCallback, bool bIOFailure);
    CCallResult<NetPlatformSteam, LobbyCreated_t> mLobbyCreateCb;

    uint64_t mLobbyId = 0;
    HSteamNetConnection mNetConnection = 0;
    HSteamListenSocket mListenSocket = 0;
    HSteamNetPollGroup mPollGroup = 0;

};

#else

class NetPlatformSteam : public NetPlatform
{

};

#endif
