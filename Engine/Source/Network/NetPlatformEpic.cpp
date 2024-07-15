#include "NetPlatformEpic.h"
#include "Log.h"
#include "Engine.h"

#include "eos_sdk.h"
#include "eos_logging.h"


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
	mInitialized = false;
	mPlatformHandle = nullptr;
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
void NetPlatformEpic::Login()
{

}

void NetPlatformEpic::Logout()
{

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

}
