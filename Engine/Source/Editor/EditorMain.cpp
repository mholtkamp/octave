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

#include "Widgets/Quad.h"
#include "Widgets/Button.h"
#include "Widgets/Selector.h"
#include "Widgets/CheckBox.h"
#include "Widgets/Text.h"
#include "Widgets/TextField.h"
#include "Widgets/Canvas.h"

#include "Widgets/Panel.h"
#include "Widgets/AssetsPanel.h"

#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"

#include "ActionManager.h"
#include "InputManager.h"
#include "PanelManager.h"
#include "TestActor.h"
#include "Grid.h"
#include "Assets/StaticMesh.h"
#include "Assets/Font.h"
#include "EditorState.h"

InitOptions OctPreInitialize();
extern void ReadCommandLineArgs(int32_t argc, char** argv);

void EditorMain(int32_t argc, char** argv)
{
    ReadCommandLineArgs(argc, argv);
    InitOptions initOptions = OctPreInitialize();
    Initialize(initOptions);

    InitializeEditorState();

    ActionManager::Create();
    InputManager::Create();
    PanelManager::Create();

    // Spawn Default Light
    Actor* defaultLight = GetWorld()->SpawnActor<Actor>();
    DirectionalLightComponent* dirLightComp = defaultLight->CreateComponent<DirectionalLightComponent>();
    dirLightComp->SetDirection(glm::normalize(glm::vec3(1.0f, -1.0f, -1.0f)));
    dirLightComp->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    defaultLight->SetName("Default Light");
    defaultLight->SetRootComponent(dirLightComp);

    // Spawn Test Actor
    GetWorld()->SpawnActor<TestActor>();

    Canvas* rootCanvas = new Canvas();
    rootCanvas->SetName("Root Canvas");
    rootCanvas->SetAnchorMode(AnchorMode::FullStretch);
    rootCanvas->SetRatios(0.0f, 0.0f, 1.0f, 1.0f);
    PanelManager::Get()->AttachPanels(rootCanvas);
    Renderer::Get()->AddWidget(rootCanvas);
    GetEditorState()->mRootCanvas = rootCanvas;

    GetWorld()->GetActiveCamera()->SetPosition(glm::vec3(0, 0, 10.0f));
    GetWorld()->GetActiveCamera()->SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));

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

    // Update asset panel to reflect our current project.
    PanelManager::Get()->GetAssetsPanel()->OnProjectDirectorySet();

    Renderer::Get()->EnableStatsOverlay(false);

    bool ret = true;

    while (ret)
    {
        InputManager::Get()->Update();
        ActionManager::Get()->Update();
        PanelManager::Get()->Update();

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
    
    PanelManager::Destroy();
    DestroyEditorState();
    Shutdown();
}

#endif
