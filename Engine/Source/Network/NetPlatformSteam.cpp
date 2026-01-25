#include "NetPlatformSteam.h"
#include "Assertion.h"
#include "Log.h"

#include "NetworkManager.h"

#if NET_PLATFORM_STEAM

#if PLATFORM_WINDOWS
#pragma comment(lib, "steam_api64.lib")
#pragma comment(lib, "sdkencryptedappticket64.lib")
#endif

void SteamAPIDebugTextHook(int nSeverity, const char* pchDebugText)
{
    // if you're running in the debugger, only warnings (nSeverity >= 1) will be sent
    // if you add -debug_steamapi to the command-line, a lot of extra informational messages will also be sent
    if (nSeverity >= 1)
    {
        LogError(pchDebugText);
        OCT_ASSERT(0);
    }
    else
    {
        LogDebug(pchDebugText);
    }
}

// Lifecycle
bool NetPlatformSteam::Create()
{
    LogDebug("Initializing NetPlatformSteam");

    SteamErrMsg errMsg = { 0 };
    if (SteamAPI_InitEx(&errMsg) != k_ESteamAPIInitResult_OK)
    {
        LogError("SteamAPI_Init() failed: ");
        LogError(errMsg);

        return false;
    }

    // set our debug handler
    SteamClient()->SetWarningMessageHook(&SteamAPIDebugTextHook);

    // Ensure that the user has logged into Steam. This will always return true if the game is launched
    // from Steam, but if Steam is at the login prompt when you run your game from the debugger, it
    // will return false.
    if (!SteamUser()->BLoggedOn())
    {
        LogWarning("Steam user is not logged in");
        return false;
    }

    return true;
}

void NetPlatformSteam::Destroy()
{
    SteamAPI_Shutdown();
}

void NetPlatformSteam::Update()
{
    SteamAPI_RunCallbacks();
}


// Login
void NetPlatformSteam::Login()
{

}

void NetPlatformSteam::Logout()
{

}

bool NetPlatformSteam::IsLoggedIn() const
{
    return SteamUser()->BLoggedOn();
}


// Matchmaking
void NetPlatformSteam::OnLobbyCreated(LobbyCreated_t* pCallback, bool bIOFailure)
{
    if (mLobbyId.IsValid())
    {
        LogWarning("OnLobbyCreated: Already in lobby.");
        return;
    }

    // Record which lobby we're in
    if (pCallback->m_eResult == k_EResultOK)
    {
        // Success
        mLobbyId = pCallback->m_ulSteamIDLobby;

        // Set the name of the lobby if it's ours
        char rgchLobbyName[256];
        snprintf(rgchLobbyName, 256, "%s", mSessionOptions.mName.c_str());
        SteamMatchmaking()->SetLobbyData(mLobbyId, "name", rgchLobbyName);

        char code[32];
        snprintf(code, 32, "%d", GetEngineState()->mGameCode);
        SteamMatchmaking()->SetLobbyData(mLobbyId, "code", code);

        char version[32];
        snprintf(version, 32, "%d", GetEngineState()->mVersion);
        SteamMatchmaking()->SetLobbyData(mLobbyId, "version", version);
    }
    else
    {
        LogError("Failed to create lobby");
    }
}

void NetPlatformSteam::OnLobbyEntered(LobbyEnter_t* pCallback, bool bIOFailure)
{
    if (pCallback->m_EChatRoomEnterResponse != k_EChatRoomEnterResponseSuccess)
    {
        LogError("Failed to enter lobby");
        return;
    }

    // Success!
    mLobbyId = pCallback->m_ulSteamIDLobby;

    // Get lobby owner
    CSteamID serverId = SteamMatchmaking()->GetLobbyOwner(mLobbyId);

    if (serverId.IsValid())
    {
        NetHost serverHost;
        serverHost.mOnlineId = serverId.ConvertToUint64();

        // Before connecting, attempt to ready all messages (that may have been previously sent from the server 
        // before we left the session and then rejoined)
        char tempBuffer[OCT_MAX_MSG_SIZE];
        NetHost tempHost;
        while (RecvMessage(tempBuffer, OCT_MAX_MSG_SIZE, tempHost) > 0)
        {
            LogDebug("Ignoring old message");
        }

        NetworkManager::Get()->Connect(serverHost);
    }
}

void NetPlatformSteam::OnLobbyList(LobbyMatchList_t* pCallback, bool bIOFailure)
{
    mSessions.clear();
    mSearchingForLobbies = false;

    for (uint32 iLobby = 0; iLobby < pCallback->m_nLobbiesMatching; iLobby++)
    {
        CSteamID steamIDLobby = SteamMatchmaking()->GetLobbyByIndex(iLobby);

        // add the lobby to the list
        NetSession session;
        session.mLan = false;
        session.mLobbyId = steamIDLobby.ConvertToUint64();

        // pull the name from the lobby metadata
        const char* pchLobbyName = SteamMatchmaking()->GetLobbyData(steamIDLobby, "name");
        if (pchLobbyName && pchLobbyName[0])
        {
            // set the lobby name
            strncpy(session.mName, pchLobbyName, OCT_SESSION_NAME_LEN);
        }
        else
        {
            // we don't have info about the lobby yet, request it
            SteamMatchmaking()->RequestLobbyData(steamIDLobby);
            // results will be returned via LobbyDataUpdate_t callback
            snprintf(session.mName, OCT_SESSION_NAME_LEN, "Lobby %d", steamIDLobby.GetAccountID());
        }

        mSessions.push_back(session);
    }
}

void NetPlatformSteam::OnLobbyDataUpdated(LobbyDataUpdate_t* pCallback)
{
    for (uint32_t i = 0; i < mSessions.size(); ++i)
    {
        NetSession& session = mSessions[i];

        if (session.mLobbyId == pCallback->m_ulSteamIDLobby)
        {
            const char* pchLobbyName = SteamMatchmaking()->GetLobbyData(session.mLobbyId, "name");
            if (pchLobbyName[0])
            {
                strncpy(session.mName, pchLobbyName, OCT_SESSION_NAME_LEN);
                LogDebug("Lobby Name: %s", pchLobbyName);
            }

            return;
        }
    }
}

void NetPlatformSteam::OnMessageRequest(SteamNetworkingMessagesSessionRequest_t* pCallback)
{
    bool accepted = SteamNetworkingMessages()->AcceptSessionWithUser(pCallback->m_identityRemote);
    LogDebug("Accepted Steam connection!");
}

void NetPlatformSteam::OnLobbyJoinRequested(GameLobbyJoinRequested_t* pCallback)
{
    LogDebug("Received lobby join request");

    NetSession session;
    session.mLan = false;
    session.mLobbyId = pCallback->m_steamIDLobby.ConvertToUint64();
    NetworkManager::Get()->JoinSession(session);
}

void NetPlatformSteam::OpenSession(const NetSessionOpenOptions& options)
{
    if (!mLobbyCreateCb.IsActive())
    {
        mSessionOptions = options;

        ELobbyType lobbyType = options.mPrivate ? k_ELobbyTypeFriendsOnly : k_ELobbyTypePublic;
        int32_t numPlayers = options.mMaxPlayers;
        SteamAPICall_t hSteamAPICall = SteamMatchmaking()->CreateLobby(lobbyType, numPlayers);
        mLobbyCreateCb.Set(hSteamAPICall, this, &NetPlatformSteam::OnLobbyCreated);
    }
}

void NetPlatformSteam::CloseSession()
{
    if (mLobbyId.IsValid())
    {
        SteamMatchmaking()->LeaveLobby(mLobbyId);
        mLobbyId.Clear();
    }
}

void NetPlatformSteam::JoinSession(const NetSession& session)
{
    SteamAPICall_t hSteamAPICall = SteamMatchmaking()->JoinLobby(session.mLobbyId);

    // set the function to call when this API completes
    mLobbyEnterCb.Set(hSteamAPICall, this, &NetPlatformSteam::OnLobbyEntered);
}

void NetPlatformSteam::BeginSessionSearch()
{
    mSessions.clear();

    SteamMatchmaking()->AddRequestLobbyListNumericalFilter("code", GetEngineState()->mGameCode, k_ELobbyComparisonEqual);
    SteamMatchmaking()->AddRequestLobbyListNumericalFilter("version", GetEngineState()->mVersion, k_ELobbyComparisonEqual);

    SteamAPICall_t hSteamAPICall = SteamMatchmaking()->RequestLobbyList();
    mLobbyListCb.Set(hSteamAPICall, this, &NetPlatformSteam::OnLobbyList);
}

void NetPlatformSteam::EndSessionSearch()
{
    mSearchingForLobbies = false;
}

void NetPlatformSteam::UpdateSearch()
{

}

bool NetPlatformSteam::IsSearching() const
{
    return mSearchingForLobbies;
}

void NetPlatformSteam::SendMessage(const NetHost& host, const char* buffer, uint32_t size)
{
    SteamNetworkingIdentity identity;
    identity.SetSteamID(host.mOnlineId);

    EResult result = SteamNetworkingMessages()->SendMessageToUser(identity, buffer, size, 0, 0);

    if (result != k_EResultOK)
    {
        LogWarning("Failed to send message: result = %d", result);
    }
}

int32_t NetPlatformSteam::RecvMessage(char* recvBuffer, int32_t bufferSize, NetHost& outHost)
{
    int32_t bytes = 0;

    SteamNetworkingMessage_t* steamMsg = nullptr;
    int numMessages = SteamNetworkingMessages()->ReceiveMessagesOnChannel(0, &steamMsg, 1);

    if (numMessages > 0 &&
        steamMsg != nullptr)
    {
        int32_t msgSize = (int32_t)steamMsg->GetSize();
        if (msgSize <= bufferSize)
        {
            memcpy(recvBuffer, steamMsg->GetData(), msgSize);
            steamMsg->Release();
            bytes = msgSize;
            outHost.mOnlineId = steamMsg->m_identityPeer.GetSteamID64();
        }
        else
        {
            LogError("Large steam message received. (Greater than OCT_MAX_MSG_SIZE). Shouldn't happen.");
        }
    }

    return bytes;
}

#endif