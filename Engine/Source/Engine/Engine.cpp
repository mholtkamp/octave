#include <stdio.h>

#include "Renderer.h"
#include "World.h"
#include "InputDevices.h"
#include "Engine.h"
#include "Log.h"
#include "Assets/Level.h"
#include "AssetManager.h"
#include "NetworkManager.h"
#include "AudioManager.h"
#include "Constants.h"
#include "Utilities.h"
#include "Profiler.h"
#include "Maths.h"
#include "ScriptAutoReg.h"
#include "ScriptFunc.h"
#include "TimerManager.h"
#include "Components/ScriptComponent.h"

#include "System/System.h"
#include "Graphics/Graphics.h"
#include "Input/Input.h"
#include "Audio/Audio.h"

#if PLATFORM_WINDOWS
// I think this is needed for the WinMain parameters
#include <Windows.h>
#endif


#if EDITOR
#include "EditorState.h"

// Located in Editor/EditorMain.cpp
void EditorMain(int32_t argc, char** argv);
#endif

static EngineState sEngineState;
static World* sWorld = nullptr;
static Clock sClock;

void ForceLinkage()
{
    // Actor Types
    FORCE_LINK_CALL(Actor);
    FORCE_LINK_CALL(StaticMeshActor);

    // Component Types
    FORCE_LINK_CALL(AudioComponent);
    FORCE_LINK_CALL(BoxComponent);
    FORCE_LINK_CALL(CameraComponent);
    FORCE_LINK_CALL(Component);
    FORCE_LINK_CALL(DirectionalLightComponent);
    FORCE_LINK_CALL(ParticleComponent);
    FORCE_LINK_CALL(PointLightComponent);
    FORCE_LINK_CALL(SkeletalMeshComponent);
    FORCE_LINK_CALL(SphereComponent);
    FORCE_LINK_CALL(StaticMeshComponent);
    FORCE_LINK_CALL(TransformComponent);
    FORCE_LINK_CALL(CapsuleComponent);
    FORCE_LINK_CALL(ShadowMeshComponent);
    FORCE_LINK_CALL(TextMeshComponent);

    // Asset Types
    FORCE_LINK_CALL(Level);
    FORCE_LINK_CALL(Material);
    FORCE_LINK_CALL(ParticleSystem);
    FORCE_LINK_CALL(SkeletalMesh);
    FORCE_LINK_CALL(SoundWave);
    FORCE_LINK_CALL(StaticMesh);
    FORCE_LINK_CALL(Texture);
    FORCE_LINK_CALL(Blueprint);
    FORCE_LINK_CALL(WidgetMap);

    // Widget Types
    FORCE_LINK_CALL(ArrayWidget);
    FORCE_LINK_CALL(Button);
    FORCE_LINK_CALL(Canvas);
    FORCE_LINK_CALL(CheckBox);
    FORCE_LINK_CALL(ComboBox);
    FORCE_LINK_CALL(Console);
    FORCE_LINK_CALL(ModalList);
    FORCE_LINK_CALL(Quad);
    FORCE_LINK_CALL(PolyRect);
    FORCE_LINK_CALL(Poly);
    FORCE_LINK_CALL(Selector);
    FORCE_LINK_CALL(StatsOverlay);
    FORCE_LINK_CALL(Text);
    FORCE_LINK_CALL(TextField);
    FORCE_LINK_CALL(VerticalList);
    FORCE_LINK_CALL(ScriptWidget);
    FORCE_LINK_CALL(Widget);
}

CommandLineOptions gCommandLineOptions;

void ReadCommandLineArgs(int32_t argc, char** argv)
{
    for (int32_t i = 0; i < argc; ++i)
    {
        if (strcmp(argv[i], "-project") == 0)
        {
            OCT_ASSERT(i + 1 < argc);
            gCommandLineOptions.mProjectPath = argv[i + 1];
            ++i;
        }

        if (strcmp(argv[i], "-level") == 0)
        {
            OCT_ASSERT(i + 1 < argc);
            gCommandLineOptions.mDefaultLevel = argv[i + 1];
            ++i;
        }

        if (strcmp(argv[i], "-res") == 0)
        {
            OCT_ASSERT(i + 2 < argc);
            int32_t width = atoi(argv[i + 1]);
            int32_t height = atoi(argv[i + 2]);

            width = glm::clamp<int32_t>(width, 100, 3840);
            height = glm::clamp<int32_t>(height, 100, 2160);

            gCommandLineOptions.mWindowWidth = width;
            gCommandLineOptions.mWindowHeight = height;
        }
    }
}

bool Initialize(InitOptions& initOptions)
{
    // Override initOptions with commandline options
    if (gCommandLineOptions.mDefaultLevel != "")
    {
        initOptions.mDefaultLevel = gCommandLineOptions.mDefaultLevel;
    }

    if (gCommandLineOptions.mWindowWidth > 0 &&
        gCommandLineOptions.mWindowHeight > 0)
    {
        initOptions.mWidth = gCommandLineOptions.mWindowWidth;
        initOptions.mHeight = gCommandLineOptions.mWindowHeight;
    }

    InitializeLog();

    CreateProfiler();
    SCOPED_STAT("Initialize");

    Renderer::Create();
    AssetManager::Create();
    NetworkManager::Create();

    Renderer* renderer = Renderer::Get();

    renderer->SetEngineState(&sEngineState);

    sEngineState.mStandalone = initOptions.mStandalone;
    sEngineState.mWindowWidth = initOptions.mWidth;
    sEngineState.mWindowHeight = initOptions.mHeight;
    sEngineState.mProjectName = (initOptions.mProjectName != "") ? initOptions.mProjectName : DEFAULT_GAME_NAME;
    sEngineState.mGameCode = initOptions.mGameCode;
    sEngineState.mVersion = initOptions.mVersion;

    {
        SCOPED_STAT("SYS_Initialize");
        SYS_Initialize();
    }

    if (initOptions.mWorkingDirectory != "")
    {
        SYS_SetWorkingDirectory(initOptions.mWorkingDirectory);
    }

    AssetManager::Get()->Initialize();

    if (gCommandLineOptions.mProjectPath != "")
    {
#if !EDITOR
        // Editor uses ActionManager::OpenProject()
        LoadProject(gCommandLineOptions.mProjectPath, !initOptions.mUseAssetRegistry);
#endif
    }
    else if (initOptions.mProjectName != "")
    {
        std::string projectName = initOptions.mProjectName;
        std::string projectPath = projectName + "/" + projectName + ".octp";
        LoadProject(projectPath, !initOptions.mUseAssetRegistry);
    }

#if !EDITOR
    if (GetEngineState()->mProjectDirectory != "" &&
        initOptions.mUseAssetRegistry)
    {
        AssetManager::Get()->DiscoverAssetRegistry((GetEngineState()->mProjectDirectory + "AssetRegistry.txt").c_str());
    }
#endif

    if (initOptions.mEmbeddedAssetCount > 0 &&
        initOptions.mEmbeddedAssets != nullptr)
    {
        AssetManager::Get()->DiscoverEmbeddedAssets(initOptions.mEmbeddedAssets, initOptions.mEmbeddedAssetCount);
    }

    if (initOptions.mEmbeddedScriptCount > 0 &&
        initOptions.mEmbeddedScripts != nullptr)
    {
        ScriptUtils::SetEmbeddedScripts(initOptions.mEmbeddedScripts, initOptions.mEmbeddedScriptCount);
    }

#if !EDITOR
    // In editor, it's expected that all engine assets are imported manually...
    // At least for now. This is to prevent breaking the editor when a file format changes.
    // Building Data (Ctrl+B) in editor will regenerate .oct files from the source data.
    if (!initOptions.mUseAssetRegistry)
    {
        AssetManager::Get()->Discover("Engine", "Engine/Assets/");
    }
#endif

    {
        SCOPED_STAT("GFX_Initialize");
        GFX_Initialize();
    }
    {
        SCOPED_STAT("INP_Initialize");
        INP_Initialize();
    }
    {
        SCOPED_STAT("AUD_Initialize");
        AUD_Initialize();
    }
    {
        SCOPED_STAT("NET_Initialize");
        NET_Initialize();
    }

    renderer->Initialize();
    NetworkManager::Get()->Initialize();

    sClock.Start();

    // There is only ever one world right now?
    sWorld = new World();

    Maths::SeedRand((uint32_t)SYS_GetTimeMicroseconds());

#if LUA_ENABLED
    {
        SCOPED_STAT("Lua Init")

        extern void BindLuaInterface();
        extern void SetupLuaPath();

        sEngineState.mLua = luaL_newstate();
        luaL_openlibs(sEngineState.mLua);

        BindLuaInterface();
        SetupLuaPath();
        InitAutoRegScripts();
        ScriptFunc::CreateRefTable();

        // Run Startup.lua if it exists.
        ScriptUtils::RunScript("Startup.lua");
    }
#endif

    // We need to force linkage of any class that uses the factory pattern
    // Because we need to register all of the classes by their constructors.
    ForceLinkage();

#if !EDITOR
    if (initOptions.mDefaultLevel != "")
    {
        Asset* levelAsset = LoadAsset(initOptions.mDefaultLevel);

        if (levelAsset != nullptr &&
            levelAsset->GetType() == Level::GetStaticType())
        {
            Level* level = (Level*)levelAsset;
            level->LoadIntoWorld(sWorld);
        }
    }
#endif 

    sEngineState.mInitialized = true;

    return true;
}

bool Update()
{
    GetProfiler()->BeginFrame();

    BEGIN_FRAME_STAT("Frame");

    {
        SCOPED_FRAME_STAT("Audio");
        AUD_Update();
    }

    INP_Update();
    SYS_Update();

    if (sEngineState.mQuit)
    {
        return false;
    }

    sClock.Update();
    AudioManager::Update(sClock.DeltaTime());

    NetworkManager::Get()->PreTickUpdate(sClock.DeltaTime());

    // Limit delta time in World::Update(). Prevent crazy issues.
    float realDeltaTime = sClock.DeltaTime();
    float gameDeltaTime = glm::min(realDeltaTime, 0.33333f);

    gameDeltaTime *= GetTimeDilation();

    if (IsPaused())
    {
        gameDeltaTime = 0.0f;
    }

#if EDITOR
    if (IsPlayingInEditor() && IsPlayInEditorPaused())
    {
        gameDeltaTime = 0.0f;
    }
#endif

    bool doFrameStep = sEngineState.mFrameStep;
    if (gameDeltaTime == 0.0f && doFrameStep)
    {
        // Force a 60 fps frame
        gameDeltaTime = 0.016f;
    }

    sEngineState.mRealDeltaTime = realDeltaTime;
    sEngineState.mGameDeltaTime = gameDeltaTime;
    sEngineState.mElapsedTime = sClock.GetTime();

    GetTimerManager()->Update(gameDeltaTime);

    sWorld->Update(gameDeltaTime);

    NetworkManager::Get()->PostTickUpdate(realDeltaTime);

    Renderer::Get()->Render(sWorld);

    AssetManager::Get()->Update(realDeltaTime);

    END_FRAME_STAT("Frame");

    GetProfiler()->EndFrame();

    if (doFrameStep)
    {
        sEngineState.mFrameStep = false;
    }

    return !sEngineState.mQuit;
}

void Shutdown()
{
    NetworkManager::Get()->Shutdown();

    sWorld->Destroy();
    delete sWorld;
    sWorld = nullptr;

#if LUA_ENABLED
    lua_close(sEngineState.mLua);
    sEngineState.mLua = nullptr;
#endif

    NetworkManager::Destroy();
    Renderer::Destroy();
    AssetManager::Destroy();

    NET_Shutdown();
    AUD_Shutdown();
    INP_Shutdown();
    GFX_Shutdown();
    SYS_Shutdown();

    DestroyProfiler();

    LogDebug("Shutdown Complete");
    ShutdownLog();

    sEngineState.mInitialized = false;
}

void Quit()
{
    sEngineState.mQuit = true;
}

World* GetWorld()
{
    return sWorld;
}

EngineState* GetEngineState()
{
    return &sEngineState;
}

const Clock* GetAppClock()
{
    return &sClock;
}

bool IsShuttingDown()
{
    return sEngineState.mQuit;
}

void LoadProject(const std::string& path, bool discoverAssets)
{
    SCOPED_STAT("LoadProject");

    // Reset asset manager??
    //AssetManager::Get()->Purge();

    sEngineState.mProjectPath = path;
    sEngineState.mProjectDirectory = path.substr(0, path.find_last_of("/\\") + 1);

    FILE* file = fopen(path.c_str(), "r");

    if (file != nullptr)
    {
        char key[MAX_PATH_SIZE] = {};
        char value[MAX_PATH_SIZE] = {};

        while (fscanf(file, "%[^=]=%s\n", key, value) != -1)
        {
            if (strncmp(key, "name", MAX_PATH_SIZE) == 0)
            {
                sEngineState.mProjectName = value;
            }
            else if (strncmp(key, "assets", MAX_PATH_SIZE) == 0)
            {
                sEngineState.mAssetDirectory = value;
            }
            else if (strncmp(key, "solution", MAX_PATH_SIZE) == 0)
            {
                sEngineState.mSolutionPath = sEngineState.mProjectDirectory + value;
            }
        }

        fclose(file);
        file = nullptr;
    }

    if (discoverAssets &&
        sEngineState.mProjectName != "")
    {
        AssetManager::Get()->Discover(sEngineState.mProjectName.c_str(), (sEngineState.mProjectDirectory + "Assets/").c_str());
    }

    char windowName[1024] = {};
    sprintf(windowName, "%s", GetEngineState()->mProjectName.c_str());
    SYS_SetWindowTitle(windowName);

#if LUA_ENABLED
    extern void UpdateLuaPath();
    UpdateLuaPath();

    if (GetEngineState()->mInitialized)
    {
        // If the engine is already initialized, then run the new startup script.
        // Otherwise, it will get run on Initialize().
        ScriptUtils::RunScript("Startup.lua");
    }
#endif

#if EDITOR
    ReadEditorSave();
    LoadStartupLevel();
#endif
}

void EnableConsole(bool enable)
{
    sEngineState.mConsoleMode = enable;
    SYS_UpdateConsole();
}

void ResizeWindow(uint32_t width, uint32_t height)
{
    if (width != 0 &&
        height != 0)
    {
        sEngineState.mWindowMinimized = false;
        sEngineState.mWindowWidth = width;
        sEngineState.mWindowHeight = height;
        GFX_ResizeWindow();
    }
    else
    {
        sEngineState.mWindowMinimized = true;
    }

    Renderer::Get()->DirtyAllWidgets();
}

bool IsPlayingInEditor()
{
#if EDITOR
    return GetEditorState()->mPlayInEditor;
#else
    return false;
#endif
}

bool IsPlaying()
{
#if EDITOR
    return GetEditorState()->mPlayInEditor;
#else
    return true;
#endif
}

bool IsGameTickEnabled()
{
#if EDITOR
    return (IsPlayingInEditor() && (!GetEditorState()->mPaused || GetEngineState()->mFrameStep));
#else
    return true;
#endif
}

void ReloadAllScripts(bool restartComponents)
{
#if LUA_ENABLED
    std::vector<ScriptComponent*> scriptComps;
    const std::vector<Actor*>& actors = GetWorld()->GetActors();

    if (restartComponents)
    {
        for (uint32_t i = 0; i < actors.size(); ++i)
        {
            for (uint32_t c = 0; c < actors[i]->GetNumComponents(); ++c)
            {
                Component* comp = actors[i]->GetComponent(c);
                if (comp->Is(ScriptComponent::ClassRuntimeId()))
                {
                    scriptComps.push_back(comp->As<ScriptComponent>());
                }
            }
        }

        // Stop the script instances
        for (uint32_t i = 0; i < scriptComps.size(); ++i)
        {
            scriptComps[i]->StopScript();
        }
    }

    // Reload script files
    ScriptUtils::ReloadAllScriptFiles();

    if (restartComponents)
    {
        // Start script instances again
        for (uint32_t i = 0; i < scriptComps.size(); ++i)
        {
            scriptComps[i]->StartScript();
        }
    }

    LogDebug("--Reloaded All Scripts--")

#endif
}

void SetPaused(bool paused)
{
    sEngineState.mPaused = paused;
}

bool IsPaused()
{
    return sEngineState.mPaused;
}

void FrameStep()
{
    sEngineState.mFrameStep = true;
}

void SetTimeDilation(float timeDilation)
{
    sEngineState.mTimeDilation = timeDilation;
}

float GetTimeDilation()
{
    return sEngineState.mTimeDilation;
}

void GarbageCollect()
{
    ScriptUtils::GarbageCollect();
    AssetManager::Get()->RefSweep();
}

#if LUA_ENABLED
lua_State* GetLua()
{
    return sEngineState.mLua;
}
#endif

#if !EDITOR
void GameMain(int32_t argc, char** argv)
{
    ReadCommandLineArgs(argc, argv);
    InitOptions initOptions = OctPreInitialize();
    Initialize(initOptions);
    OctPostInitialize();

    EnableConsole(true);
    EnableConsole(false);

    bool loop = true;
    while (loop)
    {
        OctPreUpdate();
        loop = Update();
        OctPostUpdate();
    }

    OctPreShutdown();
    Shutdown();
    OctPostShutdown();
}
#endif

// Octave main function
#if PLATFORM_WINDOWS && !_DEBUG && !EDITOR
int32_t WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int32_t nCmdShow)
{
    int32_t argc = __argc;
    char** argv = __argv;
#else
int main(int argc, char** argv)
{
#endif

#if EDITOR
    EditorMain(argc, argv);
#else
    GameMain(argc, argv);
#endif

    return 0;
}
