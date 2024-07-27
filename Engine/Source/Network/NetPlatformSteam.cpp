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
void NetPlatformSteam::Create()
{
	if (false /*SteamAPI_RestartAppIfNecessary(k_uAppIdInvalid)*/)
	{
		// if Steam is not running or the game wasn't started through Steam, SteamAPI_RestartAppIfNecessary starts the 
		// local Steam client and also launches this game again.

		// Once you get a public Steam AppID assigned for this game, you need to replace k_uAppIdInvalid with it and
		// removed steam_appid.txt from the game depot.

		return;
	}

	// Initialize SteamAPI, if this fails we bail out since we depend on Steam for lots of stuff.
	// You don't necessarily have to though if you write your code to check whether all the Steam
	// interfaces are NULL before using them and provide alternate paths when they are unavailable.
	//
	// This will also load the in-game steam overlay dll into your process.  That dll is normally
	// injected by steam when it launches games, but by calling this you cause it to always load,
	// even when not launched via steam.
	SteamErrMsg errMsg = { 0 };
	if (SteamAPI_InitEx(&errMsg) != k_ESteamAPIInitResult_OK)
	{
		LogError("SteamAPI_Init() failed: ");
		LogError(errMsg);

		LogError("Fatal Error", "Steam must be running to play this game (SteamAPI_Init() failed).");
		return;
	}

	// set our debug handler
	SteamClient()->SetWarningMessageHook(&SteamAPIDebugTextHook);

	// Ensure that the user has logged into Steam. This will always return true if the game is launched
	// from Steam, but if Steam is at the login prompt when you run your game from the debugger, it
	// will return false.
	if (!SteamUser()->BLoggedOn())
	{
		LogWarning("Steam user is not logged in");
		return;
	}
}

void NetPlatformSteam::Destroy()
{
	StopServer();

	SteamAPI_Shutdown();
}

void NetPlatformSteam::Update()
{
	SteamAPI_RunCallbacks();

	if (mServerRunning)
	{
		SteamGameServer_RunCallbacks();
	}
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

	// record which lobby we're in
	if (pCallback->m_eResult == k_EResultOK)
	{
		//StartServer();

		// success
		mLobbyId = pCallback->m_ulSteamIDLobby;

		// set the name of the lobby if it's ours
		char rgchLobbyName[256];
		snprintf(rgchLobbyName, 256, "%s's lobby", SteamFriends()->GetPersonaName());
		SteamMatchmaking()->SetLobbyData(mLobbyId, "name", rgchLobbyName);
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

#if 0
	// Query game server
	uint32_t ipAddress = 0;
	uint16_t port = 0;
	CSteamID serverId;
	bool hasServer = SteamMatchmaking()->GetLobbyGameServer(mLobbyId, &ipAddress, &port, &serverId);

	if (hasServer && serverId.IsValid())
	{
		ConnectToServer(serverId);
	}
#else

	// Get lobby owner
	CSteamID serverId = SteamMatchmaking()->GetLobbyOwner(mLobbyId);

	if (serverId.IsValid())
	{
		ConnectToServer(serverId);
	}
#endif
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

void NetPlatformSteam::OnLobbyGameCreated(LobbyGameCreated_t* pCallback)
{
	if (!mLobbyId.IsValid())
		return;

	// join the game server specified, via whichever method we can
	if (CSteamID(pCallback->m_ulSteamIDGameServer).IsValid())
	{
		ConnectToServer(CSteamID(pCallback->m_ulSteamIDGameServer));
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

void NetPlatformSteam::OpenSession()
{
	if (!mLobbyCreateCb.IsActive())
	{
		int32_t numPlayers = NetworkManager::Get()->GetMaxClients();
		SteamAPICall_t hSteamAPICall = SteamMatchmaking()->CreateLobby(k_ELobbyTypePublic, numPlayers);
		mLobbyCreateCb.Set(hSteamAPICall, this, &NetPlatformSteam::OnLobbyCreated);
	}

	SteamFriends()->SetRichPresence("status", "Creating a lobby");
}

void NetPlatformSteam::CloseSession()
{
	if (mLobbyId.IsValid())
	{
		SteamMatchmaking()->LeaveLobby(mLobbyId);
		mLobbyId.Clear();
	}

	StopServer();
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
	SteamNetworkingMessages()->ReceiveMessagesOnChannel(0, &steamMsg, 1);

	if (bytes > 0 && 
		steamMsg != nullptr)
	{
		int32_t msgSize = (int32_t)steamMsg->GetSize();
		if (msgSize < OCT_MAX_MSG_SIZE)
		{
			memcpy(recvBuffer, steamMsg->GetData(), msgSize);
			steamMsg->Release();
			bytes = msgSize;
		}
		else
		{
			LogError("Large steam message received. (Greater than OCT_MAX_MSG_SIZE). Shouldn't happen.");
		}
	}

	return bytes;
}

void NetPlatformSteam::StartServer()
{
	if (mServerRunning)
	{
		LogWarning("Server is already running, ignoring call to NetPlatformSteam::StartServer()");
		return;
	}

	const char* pchGameDir = "Octave";
	const char* serverVersion = "1.0.0.0";
	uint32_t unIP = INADDR_ANY;
	uint16_t usMasterServerUpdaterPort = 27016;
	uint16_t serverPort = 27015;

	// Don't let Steam do authentication
	EServerMode eMode = eServerModeNoAuthentication;

	// Initialize the SteamGameServer interface, we tell it some info about us, and we request support
	// for both Authentication (making sure users own games) and secure mode, VAC running in our game
	// and kicking users who are VAC banned

	// !FIXME! We need a way to pass the dedicated server flag here!

	SteamErrMsg errMsg = { 0 };
	if (SteamGameServer_InitEx(unIP, serverPort, usMasterServerUpdaterPort, eMode, serverVersion, &errMsg) != k_ESteamAPIInitResult_OK)
	{
		LogError("SteamGameServer_Init call failed: %s", errMsg);
	}

	if (SteamGameServer())
	{

		// Set the "game dir".
		// This is currently required for all games.  However, soon we will be
		// using the AppID for most purposes, and this string will only be needed
		// for mods.  it may not be changed after the server has logged on
		SteamGameServer()->SetModDir(pchGameDir);

		// These fields are currently required, but will go away soon.
		// See their documentation for more info
		SteamGameServer()->SetProduct("SteamworksExample");
		SteamGameServer()->SetGameDescription("Steamworks Example");

		// We don't support specators in our game.
		// .... but if we did:
		//SteamGameServer()->SetSpectatorPort( ... );
		//SteamGameServer()->SetSpectatorServerName( ... );

		// Initiate Anonymous logon.
		// Coming soon: Logging into authenticated, persistent game server account
		SteamGameServer()->LogOnAnonymous();

		// Initialize the peer to peer connection process.  This is not required, but we do it
		// because we cannot accept connections until this initialization completes, and so we
		// want to start it as soon as possible.
		SteamNetworkingUtils()->InitRelayNetworkAccess();

		mListenSocket = SteamGameServerNetworkingSockets()->CreateListenSocketP2P(0, 0, nullptr);
		mPollGroup = SteamGameServerNetworkingSockets()->CreatePollGroup();

		// server is up; tell everyone else to connect
		SteamMatchmaking()->SetLobbyGameServer(mLobbyId, 0, 0, mServerId);
	}
	else
	{
		LogError("SteamGameServer() interface is invalid");
	}
}

void NetPlatformSteam::StopServer()
{
	if (mServerRunning)
	{
		SteamGameServerNetworkingSockets()->CloseListenSocket(mListenSocket);
		SteamGameServerNetworkingSockets()->DestroyPollGroup(mPollGroup);

		mListenSocket = k_HSteamListenSocket_Invalid;
		mPollGroup = k_HSteamNetPollGroup_Invalid;

		// Disconnect from the steam servers
		SteamGameServer()->LogOff();

		// release our reference to the steam client library
		SteamGameServer_Shutdown();
	}
}

void NetPlatformSteam::ConnectToServer(CSteamID serverId)
{
	//if (mLobbyId.IsValid())
	//{
	//	SteamMatchmaking()->LeaveLobby(mLobbyId);
	//}

	mConnectingToServer = true;

	mServerId = serverId;

	SteamNetworkingIdentity identity;
	identity.SetSteamID(serverId);
	mServerIdentity = identity;

	NetHost serverHost;
	serverHost.mOnlineId = serverId.ConvertToUint64();
	NetworkManager::Get()->Connect(serverHost);

	//mServerConnection = SteamNetworkingSockets()->ConnectP2P(identity, 0, 0, nullptr);

	//SteamNetworkingIdentity identity;
	//identity.SetSteamID(mServerId);

	//mServerConnection = SteamNetworkingSockets()->ConnectP2P(identity, 0, 0, nullptr);

	// Update when we last retried the connection, as well as the last packet received time so we won't timeout too soon,
	// and so we will retry at appropriate intervals if packets drop
	//m_ulLastNetworkDataReceivedTime = m_ulLastConnectionAttemptRetryTime = m_pGameEngine->GetGameTickCount();
}

void NetPlatformSteam::DisconnectFromServer()
{
	CloseSession();
}

#endif