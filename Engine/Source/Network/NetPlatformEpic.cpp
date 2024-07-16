#include "NetPlatformEpic.h"
#include "Log.h"
#include "Engine.h"

#if PLATFORM_WINDOWS
#pragma comment(lib, "EOSSDK-Win64-Shipping.lib")
#endif

/** The product id for the running application, found on the dev portal */
constexpr char kProductId[] = "94952f5034e640eb92e49fced42c1e05";

/** The sandbox id for the running application, found on the dev portal */
constexpr char kSandboxId[] = "3f880f2205cf40c2ba9be1101c7a3790";

/** The deployment id for the running application, found on the dev portal */
constexpr char kDeploymentId[] = "606ef27ef5a6413daf6108ad1a6ba11d";

/** Client id of the service permissions entry, found on the dev portal */
constexpr char kClientCredentialsId[] = "xyza7891nUk0AiCq7QssjxLm2SfYmFtY";

/** Client secret for accessing the set of permissions, found on the dev portal */
constexpr char kClientCredentialsSecret[] = "MDf4zNUN7o7VbreWdHIcc+pJSv/Ieta7ceERk2iF/Ww";

constexpr char kEncryptionKey[] = "1111111111111111111111111111111111111111111111111111111111111111";

//constexpr char kGameName[] = "Octave";

static std::string EpicAccountIDToString(EOS_EpicAccountId InAccountId)
{
	std::string retString;

	if (InAccountId == nullptr)
	{
		return "NULL";
	}

	static char TempBuffer[EOS_EPICACCOUNTID_MAX_LENGTH + 1];
	int32_t TempBufferSize = sizeof(TempBuffer);
	EOS_EResult Result = EOS_EpicAccountId_ToString(InAccountId, TempBuffer, &TempBufferSize);

	if (Result == EOS_EResult::EOS_Success)
	{
		retString = TempBuffer;
	}
	else
	{
		LogError("[EOS SDK] Epic Account Id To String Error: %d", (int32_t)Result);
		retString = "ERROR";
	}

	return retString;
}

void EOS_CALL EOSSDKLoggingCallback(const EOS_LogMessage* InMsg)
{
	if (InMsg->Level != EOS_ELogLevel::EOS_LOG_Off)
	{
		if (InMsg->Level == EOS_ELogLevel::EOS_LOG_Error || InMsg->Level == EOS_ELogLevel::EOS_LOG_Fatal)
		{
			LogError("[EOS SDK] %ls: %ls", InMsg->Category, InMsg->Message);
		}
		else if (InMsg->Level == EOS_ELogLevel::EOS_LOG_Warning)
		{
			LogWarning("[EOS SDK] %ls: %ls", InMsg->Category, InMsg->Message);
		}
		else
		{
			LogDebug("[EOS SDK] %s: %s", InMsg->Category, InMsg->Message);
		}
	}
}

// Init/Shutdown
void NetPlatformEpic::Create()
{
    LogDebug("Initializing Epic Online Services");

	mGameName = GetEngineState()->mProjectName;

	EOS_InitializeOptions SDKOptions = {};
	SDKOptions.ApiVersion = EOS_INITIALIZE_API_LATEST;
	SDKOptions.AllocateMemoryFunction = nullptr;
	SDKOptions.ReallocateMemoryFunction = nullptr;
	SDKOptions.ReleaseMemoryFunction = nullptr;
	SDKOptions.ProductName = mGameName.c_str();
	SDKOptions.ProductVersion = "1.0";
#if ALLOW_RESERVED_OPTIONS
	SetReservedInitializeOptions(SDKOptions);
#else
	SDKOptions.Reserved = nullptr;
#endif
	SDKOptions.SystemInitializeOptions = nullptr;
	SDKOptions.OverrideThreadAffinity = nullptr;

	EOS_EResult InitResult = EOS_Initialize(&SDKOptions);
	if (InitResult != EOS_EResult::EOS_Success)
	{
		LogError("EOS SDK Init Failed!");
		OCT_ASSERT(0);
		return;
	}

	LogDebug("EOS SDK Initialized. Setting Logging Callback...");
	EOS_EResult SetLogCallbackResult = EOS_Logging_SetCallback(&EOSSDKLoggingCallback);
	if (SetLogCallbackResult != EOS_EResult::EOS_Success)
	{
		LogError("[EOS SDK] Set Logging Callback Failed!");
		OCT_ASSERT(0);
	}
	else
	{
		LogDebug("[EOS SDK] Logging Callback Set");
		EOS_Logging_SetLogLevel(EOS_ELogCategory::EOS_LC_ALL_CATEGORIES, EOS_ELogLevel::EOS_LOG_Verbose);
	}

#ifdef EOS_STEAM_ENABLED
	FSteamManager::GetInstance().Init();
#endif

	// Create platform instance
	EOS_Platform_Options PlatformOptions = {};
	PlatformOptions.ApiVersion = EOS_PLATFORM_OPTIONS_API_LATEST;
	PlatformOptions.bIsServer = false;
	PlatformOptions.EncryptionKey = kEncryptionKey;
	PlatformOptions.OverrideCountryCode = nullptr;
	PlatformOptions.OverrideLocaleCode = nullptr;
	PlatformOptions.Flags = EOS_PF_WINDOWS_ENABLE_OVERLAY_D3D9 | EOS_PF_WINDOWS_ENABLE_OVERLAY_D3D10 | EOS_PF_WINDOWS_ENABLE_OVERLAY_OPENGL; // Enable overlay support for D3D9/10 and OpenGL. This sample uses D3D11 or SDL.
	PlatformOptions.CacheDirectory = nullptr;

	PlatformOptions.ProductId = kProductId;
	PlatformOptions.SandboxId = kSandboxId;
	PlatformOptions.DeploymentId = kDeploymentId;

	std::string ClientId = kClientCredentialsId;
	std::string ClientSecret = kClientCredentialsSecret;

	EOS_Platform_RTCOptions RtcOptions = { 0 };
	RtcOptions.ApiVersion = EOS_PLATFORM_RTCOPTIONS_API_LATEST;
	RtcOptions.BackgroundMode = EOS_ERTCBackgroundMode::EOS_RTCBM_LeaveRooms;
	RtcOptions.PlatformSpecificOptions = NULL;

	PlatformOptions.RTCOptions = &RtcOptions;

	if (!PlatformOptions.IntegratedPlatformOptionsContainerHandle)
	{
		EOS_IntegratedPlatform_CreateIntegratedPlatformOptionsContainerOptions CreateIntegratedPlatformOptionsContainerOptions = {};
		CreateIntegratedPlatformOptionsContainerOptions.ApiVersion = EOS_INTEGRATEDPLATFORM_CREATEINTEGRATEDPLATFORMOPTIONSCONTAINER_API_LATEST;

		EOS_EResult Result = EOS_IntegratedPlatform_CreateIntegratedPlatformOptionsContainer(&CreateIntegratedPlatformOptionsContainerOptions, &PlatformOptions.IntegratedPlatformOptionsContainerHandle);
		if (Result != EOS_EResult::EOS_Success)
		{
			LogDebug("Failed to create integrated platform options container: %s", EOS_EResult_ToString(Result));
		}
	}

#if ALLOW_RESERVED_OPTIONS
	SetReservedPlatformOptions(PlatformOptions);
#else
	PlatformOptions.Reserved = NULL;
#endif // ALLOW_RESERVED_OPTIONS

	mPlatformHandle = EOS_Platform_Create(&PlatformOptions);

	if (PlatformOptions.IntegratedPlatformOptionsContainerHandle)
	{
		EOS_IntegratedPlatformOptionsContainer_Release(PlatformOptions.IntegratedPlatformOptionsContainerHandle);
	}

	if (mPlatformHandle == nullptr)
	{
		LogError("Failed to create EOS platform handle");
		OCT_ASSERT(0);
	}

	mInitialized = true;
}

void NetPlatformEpic::Destroy()
{
	if (mPlatformHandle)
	{
		EOS_Platform_Release(mPlatformHandle);
		mPlatformHandle = nullptr;
	}

	mInitialized = false;
	mShuttingDown = true;
}

void NetPlatformEpic::Update()
{
	if (mPlatformHandle)
	{
		EOS_Platform_Tick(mPlatformHandle);
	}
}


// Login
void NetPlatformEpic::LoginCompleteCb(const EOS_Auth_LoginCallbackInfo* Data)
{
	OCT_ASSERT(Data != nullptr);

	LogDebug("Epic Login Complete: User ID = %s", Data->LocalUserId);

	NetPlatformEpic* Epic = (NetPlatformEpic*)Data->ClientData;
	OCT_ASSERT(Epic);

	EOS_HAuth AuthHandle = EOS_Platform_GetAuthInterface(Epic->mPlatformHandle);
	OCT_ASSERT(AuthHandle);

	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		const int32_t AccountsCount = EOS_Auth_GetLoggedInAccountsCount(AuthHandle);
		for (int32_t AccountIdx = 0; AccountIdx < AccountsCount; ++AccountIdx)
		{
			EOS_EpicAccountId AccountId = EOS_Auth_GetLoggedInAccountByIndex(AuthHandle, AccountIdx);
			std::string AccountIdStr = EpicAccountIDToString(AccountId);

			EOS_ELoginStatus LoginStatus;
			LoginStatus = EOS_Auth_GetLoginStatus(AuthHandle, Data->LocalUserId);

			LogDebug("[EOS SDK] [%d] - Account ID: %s, Status: %d", AccountIdx, AccountIdStr.c_str(), (int32_t)LoginStatus);
		}
	}
	else if (Data->ResultCode == EOS_EResult::EOS_Auth_MFARequired)
	{
		LogWarning("[EOS SDK] MFA Code needs to be entered before logging in");
	}
	else if (Data->ResultCode == EOS_EResult::EOS_InvalidUser)
	{
		if (Data->ContinuanceToken != NULL)
		{
			LogWarning("[EOS SDK] Login failed, external account not found");
		}
		else
		{
			LogError("[EOS SDK] Continuation Token is Invalid!");
		}
	}
	else if (EOS_EResult_IsOperationComplete(Data->ResultCode))
	{
		LogError("[EOS SDK] Login Failed - Error Code: %d", Data->ResultCode);

		if (Epic)
		{
			// TODO: Persistent Auth?
			//if (Epic->CurrentLoginMode == ELoginMode::PersistentAuth)
			//{
			//	// Delete saved persistent auth token if token has expired or auth is invalid
			//	// Don't delete for other errors (e.g. EOS_EResult::EOS_NoConnection), the auth token may still be valid in these cases
			//	if (Data->ResultCode == EOS_EResult::EOS_Auth_Expired ||
			//		Data->ResultCode == EOS_EResult::EOS_InvalidAuth)
			//	{
			//		ThisAuth->DeletePersistentAuth();
			//	}
			//}
		}
	}

}

void NetPlatformEpic::Login()
{
	if (!mInitialized)
	{
		LogError("Cannot login, NetPlatformEpic not initialized.");
		return;
	}

	EOS_HAuth AuthHandle = EOS_Platform_GetAuthInterface(mPlatformHandle);
	OCT_ASSERT(AuthHandle != nullptr);

	EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(mPlatformHandle);
	OCT_ASSERT(ConnectHandle != nullptr);

	EOS_Auth_Credentials Credentials = {};
	Credentials.ApiVersion = EOS_AUTH_CREDENTIALS_API_LATEST;
	Credentials.Type = EOS_ELoginCredentialType::EOS_LCT_AccountPortal;

	EOS_Auth_LoginOptions LoginOptions = {};
	memset(&LoginOptions, 0, sizeof(LoginOptions));
	LoginOptions.ApiVersion = EOS_AUTH_LOGIN_API_LATEST;
	LoginOptions.ScopeFlags = EOS_EAuthScopeFlags::EOS_AS_BasicProfile | EOS_EAuthScopeFlags::EOS_AS_FriendsList | EOS_EAuthScopeFlags::EOS_AS_Presence;
	LoginOptions.Credentials = &Credentials;

	EOS_Auth_Login(AuthHandle, &LoginOptions, this, LoginCompleteCb);
}

void NetPlatformEpic::LogoutCompleteCb(const EOS_Auth_LogoutCallbackInfo* Data)
{
	OCT_ASSERT(Data != NULL);

	NetPlatformEpic* Epic = (NetPlatformEpic*)Data->ClientData;
	OCT_ASSERT(Epic);

	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		std::string AccountIdStr = EpicAccountIDToString(Data->LocalUserId);
		LogDebug("[EOS SDK] Logout Complete - User: %s", AccountIdStr.c_str());

		Epic->mAccountId = nullptr;
	}
	else
	{
		std::string AccountIdStr = EpicAccountIDToString(Data->LocalUserId);
		LogWarning("[EOS SDK] Logout Failed - User: %ls, Result: %ls", AccountIdStr.c_str(), EOS_EResult_ToString(Data->ResultCode));
	}
}

void NetPlatformEpic::Logout()
{
		LogDebug("[EOS SDK] Logging Out");

		EOS_HAuth AuthHandle = EOS_Platform_GetAuthInterface(mPlatformHandle);
		OCT_ASSERT(AuthHandle != nullptr);

		EOS_Auth_LogoutOptions LogoutOptions = {};
		LogoutOptions.ApiVersion = EOS_AUTH_LOGOUT_API_LATEST;
		LogoutOptions.LocalUserId = mAccountId;

		OCT_ASSERT(AuthHandle != nullptr);
		EOS_Auth_Logout(AuthHandle, &LogoutOptions, NULL, LogoutCompleteCb);
}

bool NetPlatformEpic::IsLoggedIn() const
{
	return (mAccountId != nullptr);
}

// Matchmaking
void NetPlatformEpic::OpenSession()
{

}

void NetPlatformEpic::CloseSession()
{

}

void NetPlatformEpic::BeginSessionSearch()
{

}

void NetPlatformEpic::EndSessionSearch()
{

}

void NetPlatformEpic::UpdateSearch()
{

}

bool NetPlatformEpic::IsSearching() const
{
	return false;
}
