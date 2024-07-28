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
    virtual bool Create() override;
    virtual void Destroy() override;
    virtual void Update() override;

    // Login
    virtual void Login() override;
    virtual void Logout() override;
    virtual bool IsLoggedIn() const override;

    // Matchmaking
    virtual void OpenSession(const NetSessionOpenOptions& options) override;
    virtual void CloseSession() override;
    virtual void JoinSession(const NetSession& session) override;

    virtual void BeginSessionSearch() override;
    virtual void EndSessionSearch() override;
    virtual void UpdateSearch() override;
    virtual bool IsSearching() const override;

    // Messaging
    virtual void SendMessage(const NetHost& host, const char* buffer, uint32_t size) override;
    virtual int32_t RecvMessage(char* recvBuffer, int32_t bufferSize, NetHost& outHost) override;

protected:

    void OnLobbyCreated(LobbyCreated_t* pCallback, bool bIOFailure);
    CCallResult<NetPlatformSteam, LobbyCreated_t> mLobbyCreateCb;

    void OnLobbyEntered(LobbyEnter_t* pCallback, bool bIOFailure);
    CCallResult<NetPlatformSteam, LobbyEnter_t> mLobbyEnterCb;

    void OnLobbyList(LobbyMatchList_t* pCallback, bool bIOFailure);
    CCallResult<NetPlatformSteam, LobbyMatchList_t> mLobbyListCb;

    STEAM_CALLBACK(NetPlatformSteam, OnLobbyDataUpdated, LobbyDataUpdate_t);
    STEAM_CALLBACK(NetPlatformSteam, OnMessageRequest, SteamNetworkingMessagesSessionRequest_t);
    STEAM_CALLBACK(NetPlatformSteam, OnLobbyJoinRequested, GameLobbyJoinRequested_t);

    CSteamID mLobbyId;
    bool mSearchingForLobbies = false;
    NetSessionOpenOptions mSessionOptions;
};

#else

class NetPlatformSteam : public NetPlatform
{

};

#endif
