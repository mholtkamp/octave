#if EDITOR

#include "EditorImgui.h"
#include "System/System.h"
#include "Engine.h"
#include "EditorUtils.h"
#include "EditorConstants.h"
#include "Renderer.h"
#include "InputDevices.h"
#include "Log.h"
#include "Grid.h"

#include "Nodes/3D/StaticMesh3d.h"
#include "Nodes/3D/SkeletalMesh3d.h"

#include "Viewport3d.h"
#include "ActionManager.h"
#include "EditorState.h"

#include <functional>

#include "backends/imgui_impl_vulkan.cpp"

#if PLATFORM_WINDOWS
#include "backends/imgui_impl_win32.cpp"
#elif PLATFORM_LINUX
#error Get the linux backend working!
#endif

static const float kSidePaneWidth = 200.0f;
static const ImGuiWindowFlags kPaneWindowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove;

static void DrawSpawnBasicMenu()
{
    ActionManager* am = ActionManager::Get();
    glm::vec3 spawnPos = EditorGetFocusPosition();
    Asset* selAsset = GetEditorState()->GetSelectedAsset();

    if (ImGui::MenuItem(BASIC_NODE_3D))
        am->SpawnBasicNode(BASIC_NODE_3D, spawnPos, selAsset);
    if (ImGui::MenuItem(BASIC_STATIC_MESH))
        am->SpawnBasicNode(BASIC_STATIC_MESH, spawnPos, selAsset);
    if (ImGui::MenuItem(BASIC_SKELETAL_MESH))
        am->SpawnBasicNode(BASIC_SKELETAL_MESH, spawnPos, selAsset);
    if (ImGui::MenuItem(BASIC_POINT_LIGHT))
        am->SpawnBasicNode(BASIC_POINT_LIGHT, spawnPos, selAsset);
    if (ImGui::MenuItem(BASIC_DIRECTIONAL_LIGHT))
        am->SpawnBasicNode(BASIC_DIRECTIONAL_LIGHT, spawnPos, selAsset);
    if (ImGui::MenuItem(BASIC_PARTICLE))
        am->SpawnBasicNode(BASIC_PARTICLE, spawnPos, selAsset);
    if (ImGui::MenuItem(BASIC_AUDIO))
        am->SpawnBasicNode(BASIC_AUDIO, spawnPos, selAsset);
    if (ImGui::MenuItem(BASIC_BOX))
        am->SpawnBasicNode(BASIC_BOX, spawnPos, selAsset);
    if (ImGui::MenuItem(BASIC_SPHERE))
        am->SpawnBasicNode(BASIC_SPHERE, spawnPos, selAsset);
    if (ImGui::MenuItem(BASIC_CAPSULE))
        am->SpawnBasicNode(BASIC_CAPSULE, spawnPos, selAsset);
    if (ImGui::MenuItem(BASIC_SCENE))
        am->SpawnBasicNode(BASIC_SCENE, spawnPos, selAsset);
    if (ImGui::MenuItem(BASIC_CAMERA))
        am->SpawnBasicNode(BASIC_CAMERA, spawnPos, selAsset);
    if (ImGui::MenuItem(BASIC_TEXT_MESH))
        am->SpawnBasicNode(BASIC_TEXT_MESH, spawnPos, selAsset);
}

static void DrawPackageMenu()
{
    ActionManager* am = ActionManager::Get();

    //if (ImGui::BeginPopup("PackagePopup"))
    //{
#if PLATFORM_WINDOWS
    if (ImGui::MenuItem("Windows"))
        am->BuildData(Platform::Windows, false);
#elif PLATFORM_LINUX
    if (ImGui::MenuItem("Linux"))
        am->BuildData(Platform::Linux, false);
#endif
    if (ImGui::MenuItem("Android"))
        am->BuildData(Platform::Android, false);
    if (ImGui::MenuItem("GameCube"))
        am->BuildData(Platform::GameCube, false);
    if (ImGui::MenuItem("Wii"))
        am->BuildData(Platform::Wii, false);
    if (ImGui::MenuItem("3DS"))
        am->BuildData(Platform::N3DS, false);
    if (ImGui::MenuItem("GameCube Embedded"))
        am->BuildData(Platform::GameCube, true);
    if (ImGui::MenuItem("Wii Embedded"))
        am->BuildData(Platform::Wii, true);
    if (ImGui::MenuItem("3DS Embedded"))
        am->BuildData(Platform::N3DS, true);

    //    ImGui::EndPopup();
    //}
}

static void DrawMenu()
{

}

static void DrawScene()
{
    const float halfHeight = (float)GetEngineState()->mWindowHeight / 2.0f;

    static float f = 0.0f;
    static int counter = 0;

    bool show_demo_window = true;
    static glm::vec4 clear_color = {};
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(kSidePaneWidth, halfHeight));

    ImGui::Begin("Scene", nullptr, kPaneWindowFlags);

    ImGuiTreeNodeFlags treeNodeFlags = 
        ImGuiTreeNodeFlags_OpenOnArrow 
        | ImGuiTreeNodeFlags_OpenOnDoubleClick 
        | ImGuiTreeNodeFlags_SpanAvailWidth
        | ImGuiTreeNodeFlags_DefaultOpen;

    World* world = GetWorld();
    Node* rootNode = world ? world->GetRootNode() : nullptr;

    std::function<void(Node*)> drawTree = [&](Node* node)
    {
        bool nodeSelected = GetEditorState()->IsNodeSelected(node);

        ImGuiTreeNodeFlags nodeFlags = treeNodeFlags;
        if (nodeSelected)
        {
            nodeFlags |= ImGuiTreeNodeFlags_Selected;
        }

        if (node->GetNumChildren() == 0)
        {
            nodeFlags |= ImGuiTreeNodeFlags_Leaf;
        }
        
        bool nodeOpen = ImGui::TreeNodeEx(node->GetName().c_str(), nodeFlags);
        bool nodeClicked = ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen();

        if (ImGui::BeginPopupContextItem()) // <-- use last item id as popup id
        {
            if (ImGui::Selectable("Rename"))
            {

            }
            if (ImGui::Selectable("Duplicate"))
            {

            }
            if (ImGui::Selectable("Attach Selected"))
            {

            }

            if (node->As<SkeletalMesh3D>())
            {
                if (ImGui::Selectable("Attach To Bone"))
                {

                }
            }

            if (ImGui::Selectable("Set Root Node"))
            {

            }

            if (ImGui::Selectable("Delete"))
            {

            }
            if (ImGui::Selectable("Merge"))
            {

            }
            if (ImGui::Selectable("Add Node..."))
            {

            }

            if (ImGui::Selectable("Add Scene..."))
            {

            }

            ImGui::EndPopup();
        }

        if (nodeOpen)
        {
            for (uint32_t i = 0; i < node->GetNumChildren(); ++i)
            {
                Node* child = node->GetChild(i);
                drawTree(child);
            }

            ImGui::TreePop();
        }

        if (nodeClicked)
        {
            if (nodeSelected)
            {
                GetEditorState()->DeselectNode(node);
            }
            else
            {
                if (ImGui::GetIO().KeyCtrl)
                {
                    GetEditorState()->AddSelectedNode(node, false);
                }
                else
                {
                    GetEditorState()->SetSelectedNode(node);
                }
            }
        }

    };

    if (rootNode != nullptr)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 6.0f);
        drawTree(rootNode);
        ImGui::PopStyleVar();
    }

    ImGui::End();
}

static void DrawAssets()
{
    const float halfHeight = (float)GetEngineState()->mWindowHeight / 2.0f;

    ImGui::SetNextWindowPos(ImVec2(0.0f, halfHeight));
    ImGui::SetNextWindowSize(ImVec2(kSidePaneWidth, halfHeight));

    ImGui::Begin("Assets", nullptr, kPaneWindowFlags);

    ImGui::End();

}

static void DrawProperties()
{
    const float dispWidth = (float)GetEngineState()->mWindowWidth;
    const float dispHeight = (float)GetEngineState()->mWindowHeight;

    ImGui::SetNextWindowPos(ImVec2(dispWidth - kSidePaneWidth, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(kSidePaneWidth, dispHeight));

    ImGui::Begin("Properties", nullptr, kPaneWindowFlags);

    ImGui::End();
}

static void DrawViewport()
{
    Renderer* renderer = Renderer::Get();
    ActionManager* am = ActionManager::Get();

    const float viewportWidth = (float)GetEngineState()->mWindowWidth - kSidePaneWidth * 2.0f;
    const float viewportHeight = 32.0f;

    const ImGuiWindowFlags kViewportWindowFlags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings; /* | ImGuiWindowFlags_AlwaysAutoResize*/ /*| ImGuiWindowFlags_NoBackground*/;

    ImGui::SetNextWindowPos(ImVec2(kSidePaneWidth, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(viewportWidth, viewportHeight));

    ImGui::Begin("Viewport", nullptr, kViewportWindowFlags);

    // (1) Draw File / View / World / Play buttons below
    if (ImGui::Button("File"))
        ImGui::OpenPopup("FilePopup");

    ImGui::SameLine();
    if (ImGui::Button("Edit"))
        ImGui::OpenPopup("EditPopup");

    ImGui::SameLine();
    if (ImGui::Button("View"))
        ImGui::OpenPopup("ViewPopup");

    ImGui::SameLine();
    if (ImGui::Button("World"))
        ImGui::OpenPopup("WorldPopup");

    ImGui::SameLine();
    if (ImGui::Button("Play"))
    {
        LogError("TODO: Play in Editor!");
    }

    if (ImGui::BeginPopup("FilePopup"))
    {
        if (ImGui::Selectable("Open Project"))
            am->OpenProject();
        if (ImGui::Selectable("New Project"))
            am->CreateNewProject();
        if (ImGui::Selectable("Save Scene"))
            am->SaveScene(true);
        if (ImGui::Selectable("Recapture All Scenes"))
            am->RecaptureAndSaveAllScenes();
        if (ImGui::Selectable("Resave All Assets"))
            am->ResaveAllAssets();
        if (ImGui::Selectable("Reload All Scripts"))
            ReloadAllScripts();
        //if (ImGui::Selectable("Import Scene"))
        //    YYY;
        if (ImGui::BeginMenu("Package Project"))
        {
            DrawPackageMenu();
            ImGui::EndMenu();
        }

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("EditPopup"))
    {
        if (ImGui::Selectable("Undo"))
            am->Undo();
        if (ImGui::Selectable("Redo"))
            am->Redo();

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("ViewPopup"))
    {
        if (ImGui::Selectable("Wireframe"))
            renderer->SetDebugMode(renderer->GetDebugMode() == DEBUG_WIREFRAME ? DEBUG_NONE : DEBUG_WIREFRAME);
        if (ImGui::Selectable("Collision"))
            renderer->SetDebugMode(renderer->GetDebugMode() == DEBUG_COLLISION ? DEBUG_NONE : DEBUG_COLLISION);
        if (ImGui::Selectable("Proxy"))
            renderer->EnableProxyRendering(!renderer->IsProxyRenderingEnabled());
        if (ImGui::Selectable("Bounds"))
        {
            uint32_t newMode = (uint32_t(renderer->GetBoundsDebugMode()) + 1) % uint32_t(BoundsDebugMode::Count);
            renderer->SetBoundsDebugMode((BoundsDebugMode)newMode);
        }
        if (ImGui::Selectable("Grid"))
            ToggleGrid();
        if (ImGui::Selectable("Stats"))
            renderer->EnableStatsOverlay(!renderer->IsStatsOverlayEnabled());

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("WorldPopup"))
    {
        if (ImGui::Selectable("Spawn Node"))
            LogDebug("Spawn Node!");
        if (ImGui::BeginMenu("Spawn Basic"))
        {
            DrawSpawnBasicMenu();
            ImGui::EndMenu();
        }
        if (ImGui::Selectable("Clear World"))
            am->DeleteAllNodes();
        if (ImGui::Selectable("Bake Lighting"))
            renderer->BeginLightBake();
        if (ImGui::Selectable("Clear Baked Lighting"))
        {
            const std::vector<Node*>& nodes = GetWorld()->GatherNodes();
            for (uint32_t a = 0; a < nodes.size(); ++a)
            {
                StaticMesh3D* meshNode = nodes[a]->As<StaticMesh3D>();
                if (meshNode != nullptr)
                {
                    meshNode->ClearInstanceColors();
                }
            }
        }
        if (ImGui::Selectable("Toggle Transform Mode"))
            GetEditorState()->GetViewport3D()->ToggleTransformMode();

        ImGui::EndPopup();
    }

    // (2) Draw Scene tabs on top
    const ImGuiTabBarFlags kSceneTabBarFlags = ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_FittingPolicyScroll;
    ImGui::SameLine(0.0f, 20.0f);
    //ImGui::PushStyleColor(ImGuiCol_Tab, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
    static int32_t sActiveScene = 0;
    if (ImGui::BeginTabBar("SceneTabBar", kSceneTabBarFlags))
    {
        for (int32_t n = 0; n < 5; n++)
        {
            bool opened = true;
            char tabName[128];
            snprintf(tabName, 128, "Scene %d", n);
            if (ImGui::BeginTabItem(tabName, &opened, ImGuiTabItemFlags_None))
            {
                if (n != sActiveScene)
                {
                    LogDebug("TODO: Switch to scene!");
                    sActiveScene = n;
                }
                ImGui::EndTabItem();
            }

            if (!opened)
            {
                LogError("TODO: Close scene!");
            }
        }
        ImGui::EndTabBar();
    }
    //ImGui::PopStyleColor();


    // Draw 3D / 2D / Material combo box on top right corner.


    // Hotkey Menus
    if (GetEditorState()->GetViewport3D()->ShouldHandleInput())
    {
        const bool ctrlDown = IsControlDown();
        bool shiftDown = IsShiftDown();
        const bool altDown = IsAltDown();

        //ImGuiIO& io = ImGui::GetIO();
        //ImGui::SetNextWindowPos(io.MousePos);

        if (shiftDown && IsKeyJustDown(KEY_A))
        {
            ImGui::OpenPopup("Spawn Basic");
        }
    }

    if (ImGui::BeginPopup("Spawn Basic"))
    {
        DrawSpawnBasicMenu();
        ImGui::EndPopup();
    }

    ImGui::End();

}

void EditorImguiInit()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Override theme
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Tab] = ImVec4(0.20f, 0.20f, 0.68f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.61f, 0.98f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.12f, 0.50f, 0.47f, 1.00f);



    // Set unactive window title bg equal to active title.
    colors[ImGuiCol_TitleBg] = colors[ImGuiCol_TitleBgActive];

}

void EditorImguiDraw()
{
    ImGui::NewFrame();

    DrawMenu();
    DrawScene();
    DrawAssets();
    DrawProperties();
    DrawViewport();

    ImGui::Render();
}

void EditorImguiShutdown()
{
    ImGui::DestroyContext();
}

#endif
