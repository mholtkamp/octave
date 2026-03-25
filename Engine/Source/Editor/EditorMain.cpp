#if EDITOR

#include <stdint.h>
#include <stdio.h>

#if PLATFORM_WINDOWS
#include <Windows.h>
#endif

#undef min
#undef max

#include "Engine.h"
#include "World.h"
#include "Clock.h"
#include "Renderer.h"
#include "Log.h"

#include "Nodes/Widgets/Quad.h"
#include "Nodes/Widgets/Text.h"
#include "Nodes/Widgets/Canvas.h"

#include "Nodes/3D/StaticMesh3d.h"
#include "Nodes/3D/PointLight3d.h"
#include "Nodes/3D/TestSpinner.h"

#include "ActionManager.h"
#include "InputManager.h"
#include "Preferences/PreferencesManager.h"
#include "Preferences/General/GeneralModule.h"
#include "ProjectSelect/TemplateManager.h"
#include "ProjectSelect/ProjectSelectWindow.h"
#include "Addons/AddonManager.h"
#include "Addons/NativeAddonManager.h"
#include "EditorUIHookManager.h"
#include "ControllerServer/ControllerServer.h"
#include "Preferences/Network/NetworkModule.h"
#include "Profiling/ProfilingWindow.h"
#include "Grid.h"
#include "Assets/StaticMesh.h"
#include "Assets/Font.h"
#include "EditorState.h"
#include "EditorImgui.h"
#include "BuildDependencyWindow.h"
#include "Utilities.h"

void OctPreInitialize(EngineConfig& config);

void EditorMain(int32_t argc, char** argv)
{
    GetEngineState()->mArgC = argc;
    GetEngineState()->mArgV = argv;

    ReadCommandLineArgs(argc, argv);

    {
        EngineConfig* mutableConfig = GetMutableEngineConfig();
        OctPreInitialize(*mutableConfig);
    }

    ReadEngineConfig();

    Initialize();

    const EngineConfig* engineConfig = GetEngineConfig();

    // Headless build mode - minimal initialization, run build, and exit
    if (IsHeadless())
    {
        LogDebug("Headless mode: Starting");
        LogDebug("Headless mode: Project path = %s", engineConfig->mProjectPath.c_str());
        LogDebug("Headless mode: Build platform = %d", (int)engineConfig->mBuildPlatform);

        ActionManager::Create();

        // Load project directly without editor state
        if (engineConfig->mProjectPath != "")
        {
            LoadProject(engineConfig->mProjectPath);

            // Check and auto-upgrade assets to new UUID format
            if (ActionManager::Get()->CheckProjectNeedsUpgrade())
            {
                LogDebug("Headless mode: Auto-upgrading assets to new UUID format...");
                ActionManager::Get()->UpgradeProject();
            }
        }

        if (engineConfig->mBuildPlatform != Platform::Count)
        {
            LogDebug("Headless mode: Building for %s (embedded=%d)",
                     GetPlatformString(engineConfig->mBuildPlatform),
                     engineConfig->mBuildEmbedded ? 1 : 0);

            ActionManager::Get()->BuildData(engineConfig->mBuildPlatform, engineConfig->mBuildEmbedded);

            LogDebug("Headless mode: Build complete");
        }
        else
        {
            LogError("Headless mode: No build platform specified. Use -build <platform>");
        }

        // Cleanup and exit
        ActionManager::Destroy();
        Shutdown();
        return;
    }

    // Normal editor initialization
    GetEditorState()->Init();

    ActionManager::Create();
    InputManager::Create();
    PreferencesManager::Create();

    // Auto-start controller server if enabled in preferences (must be after PreferencesManager::Create)
    NetworkModule* netModule = NetworkModule::Get();
    if (netModule && netModule->GetControllerServerEnabled())
    {
        ControllerServer::Get()->Start(netModule->GetPort());
        ControllerServer::Get()->SetLogRequests(netModule->GetLogRequests());
    }

    TemplateManager::Create();
    AddonManager::Create();
    EditorUIHookManager::Create();
    NativeAddonManager::Create();

    // Connect EditorUIHooks to NativeAddonManager's engine API
    if (NativeAddonManager::Get() && EditorUIHookManager::Get())
    {
        NativeAddonManager::Get()->GetEngineAPI()->editorUI = EditorUIHookManager::Get()->GetHooks();
    }

    InitializeGrid();

    if (engineConfig->mProjectPath != "")
    {
        // Clear the project path so we don't overwrite the EditorProject.sav file with default data.
        // This would have been set earlier in Initialize() to ensure that shader cache is loaded correctly.
        // TODO: Seems like we don't need to be storing shader cache in project folder when running the editor?
        GetEngineState()->mProjectName = "";
        GetEngineState()->mProjectPath = "";
        GetEngineState()->mProjectDirectory = "";

        ActionManager::Get()->OpenProject(engineConfig->mProjectPath.c_str());
    }

    // Spawn starting scene if a default wasn't loaded
    if (GetEditorState()->GetEditScene() == nullptr)
    {
        GetEditorState()->OpenEditScene(nullptr);
        GetWorld(0)->SpawnNode<TestSpinner>();
    }

    // Show Project Select window if no project is loaded
    if (GetEngineState()->mProjectPath.empty())
    {
        GetProjectSelectWindow()->Open();
    }

    // Fire OnEditorReady on all loaded plugins
    if (NativeAddonManager::Get() != nullptr)
    {
        NativeAddonManager::Get()->CallOnEditorReady();
    }

    Renderer::Get()->EnableConsole(true);
    Renderer::Get()->EnableStatsOverlay(false);

    GeneralModule* generalModule = static_cast<GeneralModule*>(
        PreferencesManager::Get()->FindModule("General"));
    if (generalModule != nullptr && generalModule->GetCheckBuildDepsOnStartup())
    {
        GetBuildDependencyWindow()->RunChecks();
        if (GetBuildDependencyWindow()->HasMissing())
        {
            GetBuildDependencyWindow()->Open();
        }
    }

    bool ret = true;

    while (ret)
    {
        InputManager::Get()->Update();
        ActionManager::Get()->Update();

        bool playInEditor = GetEditorState()->mPlayInEditor;

        if (playInEditor)
        {
            OctPreUpdate();
        }

        ret = Update();

        // Tick native addon plugins
        if (NativeAddonManager::Get() != nullptr)
        {
            float deltaTime = GetAppClock()->DeltaTime();

            // TickEditor runs every frame in editor (regardless of play state)
            NativeAddonManager::Get()->TickEditorAllPlugins(deltaTime);

            // Tick only runs during gameplay (Play In Editor)
            if (playInEditor)
            {
                NativeAddonManager::Get()->TickAllPlugins(deltaTime);
            }
        }

        // Tick profiling window to record frame time history
        GetProfilingWindow()->Tick();

        if (GetEditorState()->mEndPieAtEndOfFrame)
        {
            GetEditorState()->EndPlayInEditor();
        }

        // We are trying to quit, and haven't done the shutdown check yet
        if (!ret && !GetEditorState()->mShutdownUnsavedCheck)
        {
            GetEditorState()->mShutdownUnsavedCheck = true;
            std::vector<AssetStub*> unsavedAssets = AssetManager::Get()->GatherDirtyAssets();

            if (unsavedAssets.size() > 0)
            {
                // Need to wait on user response.
                ret = true;
                GetEngineState()->mQuit = false;

                // Have the imgui callbacks set / clear the Quit and Shutdown check flags as appropriate.
                EditorShowUnsavedAssetsModal(unsavedAssets);
            }
        }

        if (playInEditor)
        {
            OctPostUpdate();
        }
    }

    // Fire OnEditorShutdown before cleanup
    if (EditorUIHookManager::Get() != nullptr)
    {
        EditorUIHookManager::Get()->FireOnEditorShutdown();
    }

    NativeAddonManager::Destroy();
    EditorUIHookManager::Destroy();
    AddonManager::Destroy();
    TemplateManager::Destroy();
    PreferencesManager::Destroy();
    GetEditorState()->Shutdown();
    Shutdown();
}

#endif
