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

static bool sNodesDiscovered = false;
static std::vector<std::string> sNode3dNames;
static std::vector<std::string> sNodeWidgetNames;
static std::vector<std::string> sNodeOtherNames;

static void DiscoverNodeClasses()
{
    sNode3dNames.clear();
    sNodeWidgetNames.clear();

    const std::vector<Factory*>& nodeFactories = Node::GetFactoryList();
    for (uint32_t i = 0; i < nodeFactories.size(); ++i)
    {
        Node* node = Node::CreateInstance(nodeFactories[i]->GetType());
        if (node->As<Node3D>())
        {
            if (strcmp(node->GetClassName(), "Node3D") == 0)
            {
                sNode3dNames.insert(sNode3dNames.begin(), node->GetClassName());
            }
            else
            {
                sNode3dNames.push_back(nodeFactories[i]->GetClassName());
            }
        }
        else if (node->As<Widget>())
        {
            if (strcmp(node->GetClassName(), "Widget") == 0)
            {
                sNodeWidgetNames.insert(sNodeWidgetNames.begin(), node->GetClassName());
            }
            else
            {
                sNodeWidgetNames.push_back(nodeFactories[i]->GetClassName());
            }
        }
        else if (strcmp(node->GetClassName(), "Node") != 0)
        {
            sNodeOtherNames.push_back(nodeFactories[i]->GetClassName());
        }

        delete node;
    }
}

static void DrawPropertyList(RTTI* owner, std::vector<Property>& props)
{
    ActionManager* am = ActionManager::Get();

    PropertyOwnerType ownerType = PropertyOwnerType::Global;
    if (owner != nullptr)
    {
        if (owner->As<Node>())
        {
            ownerType = PropertyOwnerType::Node;
        }
        else if (owner->As<Asset>())
        {
            ownerType = PropertyOwnerType::Asset;
        }
    }

    for (uint32_t p = 0; p < props.size(); ++p)
    {
        ImGui::PushID(p);

        Property& prop = props[p];
        DatumType propType = prop.GetType();

        // Bools handle name on same line after checkbox
        if (propType != DatumType::Bool)
        {
            ImGui::Text(prop.mName.c_str());
        }

        for (uint32_t i = 0; i < prop.GetCount(); ++i)
        {
            ImGui::PushID(i);
            switch (propType)
            {
            case DatumType::Integer:
            {
                static int32_t sOrigVal = 0;
                int32_t propVal = prop.GetInteger(i);
                int32_t preVal = propVal;

                if (prop.mEnumCount > 0)
                {
                    ImGui::Combo("", &propVal, prop.mEnumStrings, prop.mEnumCount);
                }
                else
                {
                    ImGui::DragInt("", &propVal);
                }

                if (ImGui::IsItemActivated())
                {
                    sOrigVal = preVal;
                }

                if (ImGui::IsItemDeactivatedAfterEdit())
                {
                    prop.SetInteger(sOrigVal);
                    am->EXE_EditProperty(owner, ownerType, prop.mName, i, propVal);
                }
                else if (propVal != preVal)
                {
                    prop.SetInteger(propVal, i);
                }
                break;
            }
            case DatumType::Float:
            {
                static float sOrigVal = 0.0f;
                float propVal = prop.GetFloat(i);
                float preVal = propVal;

                ImGui::DragFloat("", &propVal);

                if (ImGui::IsItemActivated())
                {
                    sOrigVal = preVal;
                }

                if (ImGui::IsItemDeactivatedAfterEdit())
                {
                    prop.SetFloat(sOrigVal);
                    am->EXE_EditProperty(owner, ownerType, prop.mName, i, propVal);
                }
                else if (propVal != preVal)
                {
                    prop.SetFloat(propVal, i);
                }
                break;
            }
            case DatumType::Bool:
            {
                bool propVal = prop.GetBool(i);
                if (ImGui::Checkbox("", &propVal))
                {
                    am->EXE_EditProperty(owner, ownerType, prop.mName, i, propVal);
                }

                ImGui::SameLine();
                ImGui::Text(prop.mName.c_str());
                break;
            }
            case DatumType::String:
            {
                static std::string sTempString;
                sTempString = prop.GetString(i);

                if (sTempString != prop.GetString(i))
                {
                    LogDebug("Handle String Edit!");
                    prop.SetString(sTempString, i);
                }
                break;
            }
            case DatumType::Vector2D:

                break;
            case DatumType::Vector:

                break;
            case DatumType::Color:

                break;
            case DatumType::Asset:

                break;
            case DatumType::Byte:

                break;
            case DatumType::Short:

                break;

            default: break;
            }

            ImGui::PopID();
        }

        ImGui::PopID();
    }
}

static void DrawAddNodeMenu(Node* node)
{
    ActionManager* am = ActionManager::Get();

    if (!sNodesDiscovered)
    {
        DiscoverNodeClasses();
        sNodesDiscovered = true;
    }

    if (ImGui::MenuItem("Node"))
    {
        Node* newNode = node ? node->CreateChild(Node::GetStaticType()) : GetWorld()->SpawnNode(Node::GetStaticType());
        am->EXE_SpawnNode(newNode);
        GetEditorState()->SetSelectedNode(newNode);
    }

    if (ImGui::BeginMenu("3D"))
    {
        for (uint32_t i = 0; i < sNode3dNames.size(); ++i)
        {
            if (ImGui::MenuItem(sNode3dNames[i].c_str()))
            {
                const char* nodeName = sNode3dNames[i].c_str();
                Node* newNode = node ? node->CreateChild(nodeName) : GetWorld()->SpawnNode(nodeName);
                am->EXE_SpawnNode(newNode);
                GetEditorState()->SetSelectedNode(newNode);
            }
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Widget"))
    {
        for (uint32_t i = 0; i < sNodeWidgetNames.size(); ++i)
        {
            if (ImGui::MenuItem(sNodeWidgetNames[i].c_str()))
            {
                const char* nodeName = sNodeWidgetNames[i].c_str();
                Node* newNode = node ? node->CreateChild(nodeName) : GetWorld()->SpawnNode(nodeName);
                am->EXE_SpawnNode(newNode);
                GetEditorState()->SetSelectedNode(newNode);
            }
        }

        ImGui::EndMenu();
    }

    if (sNodeOtherNames.size() > 0 &&
        ImGui::BeginMenu("Other"))
    {
        for (uint32_t i = 0; i < sNodeOtherNames.size(); ++i)
        {
            if (ImGui::MenuItem(sNodeOtherNames[i].c_str()))
            {
                const char* nodeName = sNodeOtherNames[i].c_str();
                Node* newNode = node ? node->CreateChild(nodeName) : GetWorld()->SpawnNode(nodeName);
                am->EXE_SpawnNode(newNode);
                GetEditorState()->SetSelectedNode(newNode);
            }
        }

        ImGui::EndMenu();
    }
}

static void DrawSpawnBasic3dMenu(Node* node, bool setFocusPos)
{
    ActionManager* am = ActionManager::Get();
    glm::vec3 spawnPos = EditorGetFocusPosition();
    Asset* selAsset = GetEditorState()->GetSelectedAsset();

    if (ImGui::MenuItem(BASIC_NODE_3D))
        am->SpawnBasicNode(BASIC_NODE_3D, node, selAsset, setFocusPos, spawnPos);
    if (ImGui::MenuItem(BASIC_STATIC_MESH))
        am->SpawnBasicNode(BASIC_STATIC_MESH, node, selAsset, setFocusPos, spawnPos);
    if (ImGui::MenuItem(BASIC_SKELETAL_MESH))
        am->SpawnBasicNode(BASIC_SKELETAL_MESH, node, selAsset, setFocusPos, spawnPos);
    if (ImGui::MenuItem(BASIC_POINT_LIGHT))
        am->SpawnBasicNode(BASIC_POINT_LIGHT, node, selAsset, setFocusPos, spawnPos);
    if (ImGui::MenuItem(BASIC_DIRECTIONAL_LIGHT))
        am->SpawnBasicNode(BASIC_DIRECTIONAL_LIGHT, node, selAsset, setFocusPos, spawnPos);
    if (ImGui::MenuItem(BASIC_PARTICLE))
        am->SpawnBasicNode(BASIC_PARTICLE, node, selAsset, setFocusPos, spawnPos);
    if (ImGui::MenuItem(BASIC_AUDIO))
        am->SpawnBasicNode(BASIC_AUDIO, node, selAsset, setFocusPos, spawnPos);
    if (ImGui::MenuItem(BASIC_BOX))
        am->SpawnBasicNode(BASIC_BOX, node, selAsset, setFocusPos, spawnPos);
    if (ImGui::MenuItem(BASIC_SPHERE))
        am->SpawnBasicNode(BASIC_SPHERE, node, selAsset, setFocusPos, spawnPos);
    if (ImGui::MenuItem(BASIC_CAPSULE))
        am->SpawnBasicNode(BASIC_CAPSULE, node, selAsset, setFocusPos, spawnPos);
    if (ImGui::MenuItem(BASIC_SCENE))
        am->SpawnBasicNode(BASIC_SCENE, node, selAsset, setFocusPos, spawnPos);
    if (ImGui::MenuItem(BASIC_CAMERA))
        am->SpawnBasicNode(BASIC_CAMERA, node, selAsset, setFocusPos, spawnPos);
    if (ImGui::MenuItem(BASIC_TEXT_MESH))
        am->SpawnBasicNode(BASIC_TEXT_MESH, node, selAsset, setFocusPos, spawnPos);
}
static void DrawSpawnBasicWidgetMenu(Node* node)
{
    ActionManager* am = ActionManager::Get();
    Asset* selAsset = GetEditorState()->GetSelectedAsset();

    const char* widgetTypeName = nullptr;

    if (ImGui::MenuItem("Widget"))
        widgetTypeName = "Widget";
    if (ImGui::MenuItem("Quad"))
        widgetTypeName = "Quad";
    if (ImGui::MenuItem("Text"))
        widgetTypeName = "Text";

    if (widgetTypeName != nullptr)
    {
        Node* newWidget = nullptr;

        if (node == nullptr)
        {
            // Spawning initial node.
            newWidget = GetWorld()->SpawnNode(widgetTypeName);
        }
        else
        {
            newWidget = node->CreateChild(widgetTypeName);
        }

        OCT_ASSERT(newWidget);
        if (newWidget)
        {
            am->EXE_SpawnNode(newWidget);
            GetEditorState()->SetSelectedNode(newWidget);
        }
    }
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
    ActionManager* am = ActionManager::Get();

    const float halfHeight = (float)GetEngineState()->mWindowHeight / 2.0f;

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
    bool sNodeContextActive = false;

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

        if (ImGui::BeginPopupContextItem())
        {
            static char sTextInputBuffer[256] = {};
            static bool sSetTextInputFocus = true;

            sNodeContextActive = true;

            if (node->GetParent() != nullptr &&
                ImGui::BeginMenu("Move"))
            {
                Node* parent = node->GetParent();
                int32_t childSlot = parent->FindChildIndex(node);

                if (ImGui::Selectable("Top"))
                    am->EXE_AttachNode(node, parent, 0, -1);
                if (ImGui::Selectable("Up"))
                    am->EXE_AttachNode(node, parent, glm::max<int32_t>(childSlot - 1, 0), -1);
                if (ImGui::Selectable("Down"))
                    am->EXE_AttachNode(node, parent, childSlot + 1, -1);
                if (ImGui::Selectable("Bottom"))
                    am->EXE_AttachNode(node, parent, -1, -1);

                ImGui::EndMenu();
            }
            if (ImGui::Selectable("Rename", false, ImGuiSelectableFlags_DontClosePopups))
            {
                ImGui::OpenPopup("Rename Node");
                strncpy(sTextInputBuffer, node->GetName().c_str(), 256);
                sSetTextInputFocus = true;
            }
            if (ImGui::Selectable("Duplicate"))
            {
                am->DuplicateNodes({ node });
            }
            if (ImGui::Selectable("Attach Selected"))
            {
                am->AttachSelectedNodes(node, -1);
            }
            if (node->As<SkeletalMesh3D>())
            {
                if (ImGui::Selectable("Attach Selected To Bone"))
                {
                    ImGui::OpenPopup("Attach Selected To Bone");
                    sSetTextInputFocus = true;
                }
            }
            if (ImGui::Selectable("Set Root Node"))
            {
                am->EXE_SetRootNode(node);
            }
            if (ImGui::Selectable("Delete"))
            {
                am->EXE_DeleteNode(node);
            }
            if (node->As<StaticMesh3D>() &&
                ImGui::Selectable("Merge"))
            {
                LogDebug("TODO: Implement Merge for static meshes.");
            }
            if (ImGui::BeginMenu("Add Node"))
            {
                DrawAddNodeMenu(node);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Add Basic 3D"))
            {
                DrawSpawnBasic3dMenu(node, false);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Add Basic Widget"))
            {
                DrawSpawnBasicWidgetMenu(node);
                ImGui::EndMenu();
            }
            //if (ImGui::Selectable("Add Scene..."))
            //{

            //}

            // Sub Popups

            if (ImGui::BeginPopup("Rename Node"))
            {
                if (sSetTextInputFocus)
                {
                    ImGui::SetKeyboardFocusHere();
                    sSetTextInputFocus = false;
                }

                if (ImGui::InputText("Node Name", sTextInputBuffer, 256, ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    node->SetName(sTextInputBuffer);
                }

                ImGui::EndPopup();
            }

            if (ImGui::BeginPopup("Attach Selected To Bone"))
            {
                if (sSetTextInputFocus)
                {
                    ImGui::SetKeyboardFocusHere();
                    sSetTextInputFocus = false;
                }

                if (ImGui::InputText("Bone Name", sTextInputBuffer, 256, ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    SkeletalMesh3D* skNode = node->As<SkeletalMesh3D>();
                    if (skNode)
                    {
                        int32_t boneIdx = skNode->FindBoneIndex(sTextInputBuffer);
                        am->AttachSelectedNodes(skNode, boneIdx);
                    }
                }

                ImGui::EndPopup();
            }

            sSetTextInputFocus = false;

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

    // If no popup was opened and we right clicked somehwere...
    if (!ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup) &&
        ImGui::IsWindowHovered() &&
        !sNodeContextActive)
    {
        const bool ctrlDown = IsControlDown();
        const bool shiftDown = IsShiftDown();
        const bool altDown = IsAltDown();

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
        {
            ImGui::OpenPopup("Null Node Context");
        }

        const std::vector<Node*>& selNodes = GetEditorState()->GetSelectedNodes();

        // Move Up/Down selected node.
        if (selNodes.size() == 1 &&
            selNodes[0]->GetParent() != nullptr)
        {
            Node* node = selNodes[0];
            Node* parent = node->GetParent();
            int32_t childIndex = parent->FindChildIndex(node);

            if (IsKeyJustDown(KEY_MINUS))
            {
                am->EXE_AttachNode(node, parent, glm::max<int32_t>(childIndex - 1, 0), -1);
            }
            else if (IsKeyJustDown(KEY_PLUS))
            {
                am->EXE_AttachNode(node, parent, childIndex + 1, -1);
            }
        }

        if (selNodes.size() > 0)
        {
            if (IsKeyJustDown(KEY_DELETE))
            {
                am->EXE_DeleteNodes(selNodes);
            }
            else if (ctrlDown && IsKeyJustDown(KEY_D))
            {
                am->DuplicateNodes(selNodes);
            }
        }

    }

    if (ImGui::BeginPopup("Null Node Context"))
    {
        if (ImGui::BeginMenu("Spawn Node"))
        {
            DrawAddNodeMenu(nullptr);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Spawn Basic 3D"))
        {
            DrawSpawnBasic3dMenu(nullptr, false);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Spawn Basic Widget"))
        {
            DrawSpawnBasicWidgetMenu(nullptr);
            ImGui::EndMenu();
        }

        ImGui::EndPopup();
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

    if (ImGui::BeginTabBar("PropertyModeTabs"))
    {
        if (ImGui::BeginTabItem("Object"))
        {
            RTTI* obj = GetEditorState()->GetInspectedObject();

            if (obj != nullptr)
            {
                std::vector<Property> props;
                obj->GatherProperties(props);

                DrawPropertyList(obj, props);
            }

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Global"))
        {
            std::vector<Property> globalProps;
            GatherGlobalProperties(globalProps);

            DrawPropertyList(nullptr, globalProps);

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

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
        if (ImGui::BeginMenu("Spawn Node"))
        {
            DrawAddNodeMenu(nullptr);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Spawn Basic 3D"))
        {
            DrawSpawnBasic3dMenu(nullptr, true);
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

        if (shiftDown && IsKeyJustDown(KEY_Q))
        {
            ImGui::OpenPopup("Spawn Basic 3D");
        }

        if (shiftDown && IsKeyJustDown(KEY_W))
        {
            ImGui::OpenPopup("Spawn Basic Widget");
        }

        if (shiftDown && IsKeyJustDown(KEY_A))
        {
            ImGui::OpenPopup("Spawn Node");
        }
    }

    if (ImGui::BeginPopup("Spawn Basic 3D"))
    {
        DrawSpawnBasic3dMenu(nullptr, true);
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Spawn Basic Widget"))
    {
        DrawSpawnBasicWidgetMenu(GetEditorState()->GetSelectedWidget());
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Spawn Node"))
    {
        DrawAddNodeMenu(nullptr);
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
