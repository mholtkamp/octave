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
#include "Nodes/Widgets/Button.h"
#include "Nodes/Widgets/Selector.h"
#include "Nodes/Widgets/CheckBox.h"
#include "Nodes/Widgets/Text.h"
#include "Nodes/Widgets/TextField.h"
#include "Nodes/Widgets/Canvas.h"

#include "Widgets/Panel.h"
#include "Widgets/AssetsPanel.h"

#include "Nodes/3D/StaticMesh3d.h"
#include "Nodes/3D/PointLight3d.h"
#include "Nodes/3D/TestSpinner.h"

#include "ActionManager.h"
#include "InputManager.h"
#include "PanelManager.h"
#include "Grid.h"
#include "Assets/StaticMesh.h"
#include "Assets/Font.h"
#include "EditorState.h"

InitOptions OctPreInitialize();
extern void ReadCommandLineArgs(int32_t argc, char** argv);

void EditorMain(int32_t argc, char** argv)
{
    GetEngineState()->mArgC = argc;
    GetEngineState()->mArgV = argv;
    ReadCommandLineArgs(argc, argv);
    InitOptions initOptions = OctPreInitialize();
    Initialize(initOptions);

    GetEditorState()->Init();

    ActionManager::Create();
    InputManager::Create();



    InitializeGrid();

    //if (initOptions.mProjectName != nullptr)
    //{
    //    std::string projectFile = std::string() + initOptions.mProjectName + "/" + initOptions.mProjectName + ".octp";
    //    ActionManager::Get()->OpenProject(projectFile.c_str());
    //}

    EngineConfig* engineConfig = GetEngineConfig();
    if (engineConfig->mProjectPath != "")
    {
        ActionManager::Get()->OpenProject(engineConfig->mProjectPath.c_str());
    }

    // Spawn starting scene if a default wasn't loaded
    if (GetEditorState()->GetEditScene() == nullptr)
    {
        GetEditorState()->OpenEditScene(nullptr);
        GetWorld()->SpawnNode<TestSpinner>();
    }

    GetEditorState()->GetEditorCamera()->SetPosition(glm::vec3(0, 0, 10.0f));
    GetEditorState()->GetEditorCamera()->SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));

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
