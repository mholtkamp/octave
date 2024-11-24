#include <stdio.h>

#include "Renderer.h"
#include "World.h"
#include "InputDevices.h"
#include "Engine.h"
#include "Log.h"
#include "Script.h"
#include "Assets/Scene.h"
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
#include "Nodes/Widgets/TextField.h"

#include "System/System.h"
#include "Graphics/Graphics.h"
#include "Input/Input.h"
#include "Audio/Audio.h"

#if PLATFORM_WINDOWS
// I think this is needed for the WinMain parameters
#include <Windows.h>
#endif

#define OCT_LUA_DEBUGGING (PLATFORM_WINDOWS)

#if OCT_LUA_DEBUGGING
#include "LuaSocket/luasocket.h"
#endif

#if EDITOR
#include "EditorState.h"
#include "EditorImgui.h"

// Located in Editor/EditorMain.cpp
void EditorMain(int32_t argc, char** argv);
#endif

static EngineState sEngineState;
static EngineConfig sEngineConfig;

static std::vector<World*> sWorlds;
static Clock sClock;

void ForceLinkage()
{
    // Node Types
    FORCE_LINK_CALL(Node);
    FORCE_LINK_CALL(Node3D);
    FORCE_LINK_CALL(Audio3D);
    FORCE_LINK_CALL(Box3D);
    FORCE_LINK_CALL(Camera3D);
    FORCE_LINK_CALL(DirectionalLight3D);
    FORCE_LINK_CALL(Particle3D);
    FORCE_LINK_CALL(PointLight3D);
    FORCE_LINK_CALL(SkeletalMesh3D);
    FORCE_LINK_CALL(Sphere3D);
    FORCE_LINK_CALL(StaticMesh3D);
    FORCE_LINK_CALL(Capsule3D);
    FORCE_LINK_CALL(ShadowMesh3D);
    FORCE_LINK_CALL(TextMesh3D);
    FORCE_LINK_CALL(InstancedMesh3D);

    // Asset Types
    FORCE_LINK_CALL(Scene);
    FORCE_LINK_CALL(Material);
    FORCE_LINK_CALL(MaterialBase);
    FORCE_LINK_CALL(MaterialInstance);
    FORCE_LINK_CALL(MaterialLite);
    FORCE_LINK_CALL(ParticleSystem);
    FORCE_LINK_CALL(ParticleSystemInstance);
    FORCE_LINK_CALL(SkeletalMesh);
    FORCE_LINK_CALL(SoundWave);
    FORCE_LINK_CALL(StaticMesh);
    FORCE_LINK_CALL(Texture);
    FORCE_LINK_CALL(Font);

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
    FORCE_LINK_CALL(Widget);
}

void ReadCommandLineArgs(int32_t argc, char** argv)
{
    for (int32_t i = 0; i < argc; ++i)
    {
        if (strcmp(argv[i], "-project") == 0)
        {
            OCT_ASSERT(i + 1 < argc);
            sEngineConfig.mProjectPath = argv[i + 1];
            ++i;
        }
        else if (strcmp(argv[i], "-level") == 0 || strcmp(argv[i], "-scene") == 0)
        {
            OCT_ASSERT(i + 1 < argc);
            sEngineConfig.mDefaultScene = argv[i + 1];
            ++i;
        }
        else if (strcmp(argv[i], "-res") == 0)
        {
            OCT_ASSERT(i + 2 < argc);
            int32_t width = atoi(argv[i + 1]);
            int32_t height = atoi(argv[i + 2]);

            width = glm::clamp<int32_t>(width, 100, 3840);
            height = glm::clamp<int32_t>(height, 100, 2160);

            sEngineConfig.mWindowWidth = width;
            sEngineConfig.mWindowHeight = height;
            i += 2;
        }
        else if (strcmp(argv[i], "-fullscreen") == 0)
        {
            sEngineConfig.mFullscreen = true;
        }
        else if (strcmp(argv[i], "-validate") == 0)
        {
            OCT_ASSERT(i + 1 < argc);
            int32_t validate = atoi(argv[i + 1]);
            sEngineConfig.mValidateGraphics = (validate != 0);
            ++i;
        }
        else if (strcmp(argv[i], "-packageForSteam"))
        {
            sEngineConfig.mPackageForSteam = true;
        }
    }
}

bool Initialize(InitOptions& initOptions)
{
    // Override initOptions with commandline options
    if (sEngineConfig.mDefaultScene != "")
    {
        initOptions.mDefaultScene = sEngineConfig.mDefaultScene;
    }

    if (sEngineConfig.mWindowWidth > 0 &&
        sEngineConfig.mWindowHeight > 0)
    {
        initOptions.mWidth = sEngineConfig.mWindowWidth;
        initOptions.mHeight = sEngineConfig.mWindowHeight;
    }

    if (GetPlatform() == Platform::Android ||
        GetPlatform() == Platform::GameCube ||
        GetPlatform() == Platform::Wii ||
        GetPlatform() == Platform::N3DS)
    {
        // Use the asset registry to make loading faster. On consoles, scanning the SD card directories 
        // can be extremely slow. Android used to require the asset registry, but I did add support for 
        // iterating over the assets directory via Java. It's still probably faster to use the asset registry though.
        initOptions.mUseAssetRegistry = true;
    }

    InitializeLog();

    CreateProfiler();
    SCOPED_STAT("Initialize");

    Renderer::Create();
    AssetManager::Create();
    NetworkManager::Create();

#if EDITOR
    EditorImguiInit();
#endif

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

    if (sEngineConfig.mProjectPath != "")
    {
#if EDITOR
        // Even though we aren't loading the project immediately, set the project path so 
        // graphics initialization can load shader cache correctly.
        const std::string& path = sEngineConfig.mProjectPath;
        sEngineState.mProjectPath = path;
        sEngineState.mProjectDirectory = path.substr(0, path.find_last_of("/\\") + 1);
#else
        // Editor uses ActionManager::OpenProject()
        LoadProject(sEngineConfig.mProjectPath, !initOptions.mUseAssetRegistry);
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

    sWorlds.push_back(new World());

#if PLATFORM_3DS
    // So far only 3DS can support a second screen and we have a one-world-per-screen setup.
    sWorlds.push_back(new World());
#endif


    Maths::SeedRand((uint32_t)SYS_GetTimeMicroseconds());

#if LUA_ENABLED
    {
        SCOPED_STAT("Lua Init")

        extern void BindLuaInterface();
        extern void SetupLuaPath();

        sEngineState.mLua = luaL_newstate();
        luaL_openlibs(sEngineState.mLua);

#if OCT_LUA_DEBUGGING
        luaopen_socket_core(sEngineState.mLua);
        lua_setglobal(sEngineState.mLua, "socket");
#endif

        BindLuaInterface();
        SetupLuaPath();
        InitAutoRegScripts();
        ScriptFunc::CreateRefTable();

#if OCT_LUA_DEBUGGING
        ScriptUtils::RunScript("StartLuaPanda.lua");
#endif

        // Run Startup.lua if it exists.
        ScriptUtils::RunScript("EngineStartup.lua");
        ScriptUtils::RunScript("Startup.lua");
    }
#endif

    // We need to force linkage of any class that uses the factory pattern
    // Because we need to register all of the classes by their constructors.
    ForceLinkage();

#if !EDITOR
    if (initOptions.mDefaultScene != "")
    {
        Asset* sceneAsset = LoadAsset(initOptions.mDefaultScene);
        GetWorld(0)->LoadScene(initOptions.mDefaultScene.c_str(), true);
    }
#endif 

    sEngineState.mInitialized = true;

    return true;
}

bool Update()
{
    if (sEngineState.mSuspended)
    {
        SYS_Update();
        return !sEngineState.mQuit;
    }

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
    if (IsPlayingInEditor() && 
        GetEditorState()->IsPlayInEditorPaused())
    {
        gameDeltaTime = 0.0f;
    }

    GetEditorState()->Update(realDeltaTime);
#endif

    bool doFrameStep = sEngineState.mFrameStep;
    if (gameDeltaTime == 0.0f && doFrameStep)
    {
        // Force a 60 fps frame
        gameDeltaTime = 0.016f;
    }

    sEngineState.mRealDeltaTime = realDeltaTime;
    sEngineState.mGameDeltaTime = gameDeltaTime;
    sEngineState.mGameElapsedTime += gameDeltaTime;
    sEngineState.mRealElapsedTime += realDeltaTime;

    GetTimerManager()->Update(gameDeltaTime);

    for (uint32_t i = 0; i < sWorlds.size(); ++i)
    {
        sWorlds[i]->Update(gameDeltaTime);
    }

    TextField::StaticUpdate();

    NetworkManager::Get()->PostTickUpdate(realDeltaTime);

#if EDITOR
    EditorImguiDraw();
#endif

    for (int32_t i = 0; i < int32_t(sWorlds.size()); ++i)
    {
        Renderer::Get()->Render(sWorlds[i], i);
    }

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

    for (uint32_t i = 0; i < sWorlds.size(); ++i)
    {
        sWorlds[i]->Destroy();
        delete sWorlds[i];
        sWorlds[i] = nullptr;
    }

    sWorlds.clear();

#if LUA_ENABLED
    lua_close(sEngineState.mLua);
    sEngineState.mLua = nullptr;
#endif

#if EDITOR
    EditorImguiPreShutdown();
#endif

    NetworkManager::Destroy();
    Renderer::Destroy();
    AssetManager::Destroy();

    NET_Shutdown();
    AUD_Shutdown();
    INP_Shutdown();
    GFX_Shutdown();
    SYS_Shutdown();

#if EDITOR
    EditorImguiShutdown();
#endif

    DestroyProfiler();

    LogDebug("Shutdown Complete");
    ShutdownLog();

    sEngineState.mInitialized = false;
}

void Quit()
{
    sEngineState.mQuit = true;
}

World* GetWorld(int32_t index)
{
    // Right now only supports two worlds
    index = glm::clamp<int32_t>(index, 0, int32_t(sWorlds.size()) - 1);
    return  sWorlds[index];
}

int32_t GetNumWorlds()
{
    return int32_t(sWorlds.size());
}

EngineState* GetEngineState()
{
    return &sEngineState;
}

EngineConfig* GetEngineConfig()
{
    return &sEngineConfig;
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

#if EDITOR
    GetEditorState()->EndPlayInEditor();
    GetEditorState()->CloseAllEditScenes();

    // Reset asset manager??
    if (sEngineState.mProjectDirectory != "")
    {
        GetEditorState()->WriteEditorProjectSave();
        AssetManager::Get()->Purge(false);
        AssetManager::Get()->UnloadProjectDirectory();

        sEngineState.mProjectName = "";
        sEngineState.mProjectPath = "";
        sEngineState.mProjectDirectory = "";
    }
#endif

    sEngineState.mProjectPath = path;
    sEngineState.mProjectDirectory = path.substr(0, path.find_last_of("/\\") + 1);

    Stream projFileStream;
    projFileStream.ReadFile(path.c_str(), true);

    if (projFileStream.GetData() != nullptr)
    {
        char key[MAX_PATH_SIZE] = {};
        char value[MAX_PATH_SIZE] = {};

        while (projFileStream.Scan("%[^=]=%s\n", key, value) != -1)
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
    GetEditorState()->ReadEditorProjectSave();
    GetEditorState()->LoadStartupScene();
    if (sEngineState.mProjectPath != "")
    {
        GetEditorState()->AddRecentProject(sEngineState.mProjectPath);
    }
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
    std::vector<Script*> scripts;
    std::vector<std::vector<Property> > scriptProps;

    std::vector<Node*> nodes;

    for (uint32_t i = 0; i < sWorlds.size(); ++i)
    {
        sWorlds[i]->GatherNodes(nodes);
    }

    if (restartComponents)
    {
        for (uint32_t i = 0; i < nodes.size(); ++i)
        {
            Script* script = nodes[i]->GetScript();

            if (script != nullptr)
            {
                scripts.push_back(script);
                scriptProps.push_back(script->GetScriptProperties());
            }
        }

        // Stop the script instances
        for (uint32_t i = 0; i < scripts.size(); ++i)
        {
            scripts[i]->StopScript();
        }
    }

    // Reload script files
    ScriptUtils::ReloadAllScriptFiles();

    if (restartComponents)
    {
        // Start script instances again
        for (uint32_t i = 0; i < scripts.size(); ++i)
        {
            scripts[i]->StartScript();
            scripts[i]->SetScriptProperties(scriptProps[i]);
        }
    }

    LogDebug("--Reloaded All Scripts--");

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

void GatherGlobalProperties(std::vector<Property>& props)
{
    Renderer::Get()->GatherProperties(props);

#if EDITOR
    GetEditorState()->GatherProperties(props);
#endif
}

void SetScreenOrientation(ScreenOrientation mode)
{
    SYS_SetScreenOrientation(mode);
}

ScreenOrientation GetScreenOrientation()
{
    return SYS_GetScreenOrientation();
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
    sEngineState.mArgC = argc;
    sEngineState.mArgV = argv;
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
#elif PLATFORM_ANDROID
void android_main(struct android_app* state)
{
    sEngineState.mSystem.mState = state;

    int32_t argc = 0;
    char** argv = nullptr;

    // For now, just enable validation
    //GetEngineConfig()->mValidateGraphics = true;

#else
int main(int argc, char** argv)
{
#endif

#if EDITOR
    EditorMain(argc, argv);
#else
    GameMain(argc, argv);
#endif

#if PLATFORM_ANDROID
    exit(0);
#else
    return 0;
#endif
}
