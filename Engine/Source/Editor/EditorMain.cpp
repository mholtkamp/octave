#if EDITOR

#include <stdint.h>

#if PLATFORM_WINDOWS
#include <Windows.h>
#endif

#undef min
#undef max

#include "Engine.h"
#include "World.h"
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
#include "Grid.h"
#include "Assets/StaticMesh.h"
#include "Assets/Font.h"
#include "EditorState.h"

void OctPreInitialize(EngineConfig& config);

void EditorMain(int32_t argc, char** argv)
{
    GetEngineState()->mArgC = argc;
    GetEngineState()->mArgV = argv;
    ReadCommandLineArgs(argc, argv);
    ReadEngineConfig();

    {
        EngineConfig* mutableConfig = GetMutableEngineConfig();
        OctPreInitialize(*mutableConfig);
    }

    Initialize();

    GetEditorState()->Init();

    ActionManager::Create();
    InputManager::Create();

    InitializeGrid();

    const EngineConfig* engineConfig = GetEngineConfig();
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

    Renderer::Get()->EnableConsole(true);
    Renderer::Get()->EnableStatsOverlay(false);

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

        if (playInEditor)
        {
            OctPostUpdate();
        }
    }
    
    GetEditorState()->Shutdown();
    Shutdown();
}

#endif
