#if EDITOR

#include "EditorImgui.h"
#include "System/System.h"
#include "Engine.h"
#include "Renderer.h"
#include "Log.h"
#include "Grid.h"

#include "Nodes/3D/StaticMesh3d.h"

#include "Viewport3d.h"
#include "ActionManager.h"
#include "EditorState.h"

#include "backends/imgui_impl_vulkan.cpp"

#if PLATFORM_WINDOWS
#include "backends/imgui_impl_win32.cpp"
#elif PLATFORM_LINUX
#error Get the linux backend working!
#endif

static const float kSidePaneWidth = 200.0f;
static const ImGuiWindowFlags kPaneWindowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove;

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
        if (ImGui::Selectable("Package Project", false, ImGuiSelectableFlags_DontClosePopups))
        {
            ImGui::OpenPopup("PackagePopup");
        }

        if (ImGui::BeginPopup("PackagePopup"))
        {
#if PLATFORM_WINDOWS
            if (ImGui::Selectable("Windows"))
                am->BuildData(Platform::Windows, false);
#elif PLATFORM_LINUX
            if (ImGui::Selectable("Linux"))
                am->BuildData(Platform::Linux, false);
#endif
            if (ImGui::Selectable("Android"))
                am->BuildData(Platform::Android, false);
            if (ImGui::Selectable("GameCube"))
                am->BuildData(Platform::GameCube, false);
            if (ImGui::Selectable("Wii"))
                am->BuildData(Platform::Wii, false);
            if (ImGui::Selectable("3DS"))
                am->BuildData(Platform::N3DS, false);
            if (ImGui::Selectable("GameCube Embedded"))
                am->BuildData(Platform::GameCube, true);
            if (ImGui::Selectable("Wii Embedded"))
                am->BuildData(Platform::Wii, true);
            if (ImGui::Selectable("3DS Embedded"))
                am->BuildData(Platform::N3DS, true);

            ImGui::EndPopup();
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
        if (ImGui::Selectable("Spawn Basic"))
            LogDebug("Spawn Basic!");
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
