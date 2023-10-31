#if EDITOR

#include "Viewport3d.h"
#include "Nodes/Widgets/Console.h"
#include "Nodes/Widgets/StatsOverlay.h"
#include "InputDevices.h"
#include "EditorState.h"
#include "World.h"
#include "Engine.h"
#include "Grid.h"
#include "Nodes/Widgets/Quad.h"
#include "Nodes/Widgets/Text.h"
#include "Nodes/Widgets/Button.h"
#include "Nodes/Widgets/PolyRect.h"
#include "Log.h"
#include "Renderer.h"
#include "AssetManager.h"
#include "World.h"
#include "Maths.h"
#include "ActionManager.h"
#include "Assets/SkeletalMesh.h"
#include "Constants.h"
#include "EditorConstants.h"
#include "EditorUtils.h"
#include "PanelManager.h"
#include "Nodes/3D/StaticMesh3d.h"
#include "Nodes/3D/PointLight3d.h"
#include "Nodes/3D/DirectionalLight3d.h"
#include "Nodes/3D/Node3d.h"
#include "Nodes/3D/SkeletalMesh3d.h"
#include "Nodes/3D/Box3d.h"
#include "Nodes/3D/Sphere3d.h"
#include "Nodes/3D/Particle3d.h"
#include "Nodes/3D/Audio3d.h"
#include "Nodes/3D/TextMesh3d.h"
#include "System/System.h"

static float sDefaultFocalDistance = 10.0f;
constexpr float sMaxCameraPitch = 89.99f;

#if NODE_CONV_EDITOR
void Viewport3D::HandleFilePressed(Button* button)
{
    std::string buttonText = button->GetTextString();
    ActionManager* am = ActionManager::Get();
    ActionList* actionList = GetActionList();
    bool hideActionList = true;

    if (buttonText == "File")
    {
        std::vector<std::string> actions;
        if (GetEditorMode() == EditorMode::Blueprint)
        {
            actions.push_back("Reload All Scripts");
        }
        else
        {
            actions.push_back("Open Project");
            actions.push_back("New Project");
            actions.push_back("Save Level");
            actions.push_back("Package Project");
            actions.push_back("Recapture Levels");
            actions.push_back("Recapture Blueprints");
            actions.push_back("Resave All Assets");
            actions.push_back("Reload All Scripts");
            actions.push_back("Import Scene");
        }
        actionList->SetActions(actions, HandleFilePressed);
        hideActionList = false;
    }
    else if (buttonText == "Open Project")
    {
        am->OpenProject();
    }
    else if (buttonText == "New Project")
    {
        am->CreateNewProject();
    }
    else if (buttonText == "Save Level")
    {
        am->SaveLevel(true);
    }
    else if (buttonText == "Package Project")
    {
        am->ShowBuildDataPrompt();
        hideActionList = false;
    }
    else if (buttonText == "Recapture Levels")
    {
        am->RecaptureAndSaveAllLevels();
    }
    else if (buttonText == "Recapture Blueprints")
    {
        am->RecaptureAndSaveAllBlueprints();
    }
    else if (buttonText == "Resave All Assets")
    {
        am->ResaveAllAssets();
    }
    else if (buttonText == "Reload All Scripts")
    {
        ReloadAllScripts();
    }
    else if (buttonText == "Import Scene")
    {
        SceneImportWidget* sceneImpWidget = GetSceneImportWidget();
        Renderer::Get()->SetModalWidget(sceneImpWidget);
        sceneImpWidget->MoveToMousePosition();
    }

    if (hideActionList)
    {
        actionList->Hide();
    }
}

void Viewport3D::HandleViewPressed(Button* button)
{
    std::string buttonText = button->GetTextString();
    ActionManager* am = ActionManager::Get();
    Renderer* renderer = Renderer::Get();
    ActionList* actionList = GetActionList();
    bool hideActionList = true;

    if (buttonText == "View")
    {
        std::vector<std::string> actions;
        actions.push_back("Wireframe");
        actions.push_back("Collision");
        actions.push_back("Proxy");
        actions.push_back("Bounds");
        actions.push_back("Grid");
        actions.push_back("Stats");
        actionList->SetActions(actions, HandleViewPressed);
        hideActionList = false;
    }
    else if (buttonText == "Wireframe")
    {
        if (renderer->GetDebugMode() == DEBUG_WIREFRAME)
        {
            renderer->SetDebugMode(DEBUG_NONE);
        }
        else
        {
            renderer->SetDebugMode(DEBUG_WIREFRAME);
        }
    }
    else if (buttonText == "Collision")
    {
        if (renderer->GetDebugMode() == DEBUG_COLLISION)
        {
            renderer->SetDebugMode(DEBUG_NONE);
        }
        else
        {
            renderer->SetDebugMode(DEBUG_COLLISION);
        }
    }
    else if (buttonText == "Proxy")
    {
        bool enabled = renderer->IsProxyRenderingEnabled();
        renderer->EnableProxyRendering(!enabled);
    }
    else if (buttonText == "Bounds")
    {
        uint32_t newMode = (uint32_t(renderer->GetBoundsDebugMode()) + 1) % uint32_t(BoundsDebugMode::Count);
        renderer->SetBoundsDebugMode((BoundsDebugMode)newMode);
    }
    else if (buttonText == "Grid")
    {
        ToggleGrid();
    }
    else if (buttonText == "Stats")
    {
        bool enabled = renderer->IsStatsOverlayEnabled();
        renderer->EnableStatsOverlay(!enabled);
    }

    if (hideActionList)
    {
        actionList->Hide();
    }
}

void Viewport3D::HandleWorldPressed(Button* button)
{
    std::string buttonText = button->GetTextString();
    ActionManager* am = ActionManager::Get();
    ActionList* actionList = GetActionList();
    Viewport3D* vp = PanelManager::Get()->GetViewportPanel();
    bool hideActionList = true;

    if (buttonText == "World")
    {
        std::vector<std::string> actions;

        if (GetEditorMode() == EditorMode::Level)
        {
            actions.push_back("Spawn Actor");
            actions.push_back("Spawn Basic");
            actions.push_back("Delete All Actors");
            actions.push_back("Bake Lighting");
            actions.push_back("Clear Baked Lighting");
        }

        actions.push_back("Undo");
        actions.push_back("Redo");
        actions.push_back("Toggle Transform Mode");

        actionList->SetActions(actions, HandleWorldPressed);
        hideActionList = false;
    }
    else if (buttonText == "Spawn Actor")
    {
        ShowSpawnActorPrompt(false);
        hideActionList = false;
    }
    else if (buttonText == "Spawn Basic")
    {
        ShowSpawnActorPrompt(true);
        hideActionList = false;
    }
    else if (buttonText == "Undo")
    {
        ActionManager::Get()->Undo();
    }
    else if (buttonText == "Redo")
    {
        ActionManager::Get()->Redo();
    }
    else if (buttonText == "Toggle Transform Mode")
    {
        vp->ToggleTransformMode();
    }
    else if (buttonText == "Delete All Actors")
    {
        ActionManager::Get()->DeleteAllActors();
    }
    else if (buttonText == "Bake Lighting")
    {
        Renderer::Get()->BeginLightBake();
    }
    else if (buttonText == "Clear Baked Lighting")
    {
        const std::vector<Actor*>& actors = GetWorld()->GetActors();
        for (uint32_t a = 0; a < actors.size(); ++a)
        {
            for (uint32_t c = 0; c < actors[a]->GetNumComponents(); ++c)
            {
                StaticMesh3D* meshComp = actors[a]->GetComponent((uint32_t)c)->As<StaticMesh3D>();
                if (meshComp != nullptr)
                {
                    meshComp->ClearInstanceColors();
                }
            }
        }
    }

    if (hideActionList)
    {
        actionList->Hide();
    }
}

void Viewport3D::HandlePlayPressed(Button* button)
{
    if (GetEditorState()->mPlayInEditor)
    {
        bool pause = IsPlayInEditorPaused();
        SetPlayInEditorPaused(!pause);
    }
    else
    {
        BeginPlayInEditor();
    }
}

void Viewport3D::HandleStopPressed(Button* button)
{
    if (GetEditorState()->mPlayInEditor)
    {
        EndPlayInEditor();
    }
}

void HandleSpawnActorPressed(Button* button)
{
    const std::string& actorTypeName = button->GetTextString();

    // Find actorid we need to spawn.
    bool spawnedActor = false;
    const std::vector<Factory*>& factories =  Actor::GetFactoryList();
    for (uint32_t i = 0; i < factories.size(); ++i)
    {
        if (strncmp(factories[i]->GetClassName(), actorTypeName.c_str(), MAX_PATH_SIZE) == 0)
        {
            TypeId actorType = factories[i]->GetType();
            Camera3D* camera = GetWorld()->GetActiveCamera();
            float focalDistance = PanelManager::Get()->GetViewportPanel()->GetFocalDistance();
            glm::vec3 spawnPos = camera->GetAbsolutePosition() + focalDistance * camera->GetForwardVector();

            ActionManager::Get()->SpawnActor(actorType, spawnPos);

            spawnedActor = true;
            break;
        }
    }
    
    if (!spawnedActor)
    {
        LogError("Failed to spawn actor of type %s", actorTypeName.c_str());
    }

    // Hide the modal spawn list
    Renderer::Get()->SetModalWidget(nullptr);
}

void HandleSpawnBasicPressed(Button* button)
{
    glm::vec3 spawnPos = EditorGetFocusPosition();

    const std::string& basicName = button->GetTextString();
    
    ActionManager::Get()->SpawnBasicActor(basicName, spawnPos);

    Renderer::Get()->SetModalWidget(nullptr);
}

#endif


Viewport3D::Viewport3D() :
    mFirstPersonMoveSpeed(10.0f),
    mFirstPersonRotationSpeed(0.07f),
    mFocalDistance(sDefaultFocalDistance)
{
#if CONSOLE_ENABLED
    // Move the console into  viewport region
    // TODO-NODE: If we can slide out the left panels, then make the log slide over too.
    const float kDefaultWidth = 180.0f;
    Renderer::Get()->GetConsoleWidget()->SetRect(kDefaultWidth + 5.0f, 30, 1280 - kDefaultWidth, 720);
#endif

    StatsOverlay* statsWidget = Renderer::Get()->GetStatsWidget();
    statsWidget->SetPosition(
        -statsWidget->GetWidth() - kDefaultWidth,
        statsWidget->GetY());
    statsWidget->SetVisible(true);
}

Viewport3D::~Viewport3D()
{

}

void Viewport3D::Update(float deltaTime)
{
    if (ShouldHandleInput())
    {
        if (GetEditorState()->mMouseNeedsRecenter)
        {
            EditorCenterCursor();
            GetEditorState()->mMouseNeedsRecenter = false;
        }

        ControlMode controlMode = GetEditorState()->GetControlMode();

        switch (controlMode)
        {
        case ControlMode::Default: HandleDefaultControls(); break;
        case ControlMode::Pilot: HandlePilotControls(); break;
        case ControlMode::Translate: HandleTransformControls(); break;
        case ControlMode::Rotate: HandleTransformControls(); break;
        case ControlMode::Scale: HandleTransformControls(); break;
        case ControlMode::Pan: HandlePanControls(); break;
        case ControlMode::Orbit: HandleOrbitControls(); break;
        }
    }

    INP_GetMousePosition(mPrevMouseX, mPrevMouseY);
}

bool Viewport3D::ShouldHandleInput() const
{
    Widget* modal = Renderer::Get()->GetModalWidget();
    bool imguiWantsKeyboard = ImGui::GetIO().WantCaptureKeyboard;
    bool imguiWantsMouse = ImGui::GetIO().WantCaptureMouse;
    bool imguiWantsText = ImGui::GetIO().WantTextInput;
    bool imguiAnyItemHovered = ImGui::IsAnyItemHovered();
    bool imguiAnyWindowHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
    bool imguiAnyPopupUp = ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup);
    

    bool handleInput = (modal == nullptr && !imguiAnyWindowHovered && !imguiWantsText && !imguiAnyPopupUp);
    return handleInput;
}

bool Viewport3D::IsMouseInside() const
{
    // TODO-NODE: Check if mouse is outside of all imgui windows?
    //   Or at least check if mouse is within the kDefaultWidth area.
    return true;
}

float Viewport3D::GetFocalDistance() const
{
    return mFocalDistance;
}

void Viewport3D::HandleDefaultControls()
{
    Renderer* renderer = Renderer::Get();
    Camera3D* camera = GetWorld()->GetActiveCamera();
    glm::vec3 focus = camera->GetAbsolutePosition() + camera->GetForwardVector() * mFocalDistance;

    if (IsMouseInside())
    {
        const int32_t scrollDelta = GetScrollWheelDelta();
        const bool controlDown = IsControlDown();
        const bool shiftDown = IsShiftDown();
        const bool altDown = IsAltDown();
        const bool cmdKeyDown = (controlDown || shiftDown || altDown);

        if (IsMouseButtonJustDown(MOUSE_RIGHT))
        {
            GetEditorState()->SetControlMode(ControlMode::Pilot);
        }

        if (IsMouseButtonJustDown(MOUSE_LEFT) &&
            !IsMouseOnAnyButton())
        {
            int32_t mouseX = 0;
            int32_t mouseY = 0;
            GetMousePosition(mouseX, mouseY);
            Node3D* selectNode = Renderer::Get()->ProcessHitCheck(GetWorld(), mouseX, mouseY);

            if (shiftDown)
            {
                if (selectNode != nullptr)
                {
                    if (GetEditorState()->IsNodeSelected(selectNode))
                    {
                        GetEditorState()->RemoveSelectedNode(selectNode);
                    }
                    else
                    {
                        GetEditorState()->AddSelectedNode(selectNode, false);
                    }
                }
            }
            else
            {
                if (GetEditorState()->GetSelectedNode() != selectNode)
                {
                    GetEditorState()->SetSelectedNode(selectNode);
                }
                else
                {
                    GetEditorState()->SetSelectedNode(nullptr);
                }
            }
        }

        if (GetEditorState()->GetSelectedNode() != nullptr &&
            GetEditorState()->GetSelectedNode()->IsNode3D())
        {
            if (!controlDown && !altDown && IsKeyJustDown(KEY_G))
            {
                GetEditorState()->SetControlMode(ControlMode::Translate);
                SavePreTransforms();
            }

            if (!controlDown && !altDown && IsKeyJustDown(KEY_R))
            {
                GetEditorState()->SetControlMode(ControlMode::Rotate);
                SavePreTransforms();
            }

            if (!controlDown && !altDown && IsKeyJustDown(KEY_S))
            {
                GetEditorState()->SetControlMode(ControlMode::Scale);
                SavePreTransforms();
            }
        }

        if (!controlDown && !shiftDown && IsKeyJustDown(KEY_Z))
        {
            renderer->SetDebugMode((renderer->GetDebugMode() != DEBUG_WIREFRAME) ? DEBUG_WIREFRAME : DEBUG_NONE);
        }

        if (IsKeyJustDown(KEY_K))
        {
            renderer->SetDebugMode((renderer->GetDebugMode() != DEBUG_COLLISION) ? DEBUG_COLLISION : DEBUG_NONE);
        }

        if (!controlDown && !altDown && IsKeyJustDown(KEY_P))
        {
            renderer->EnableProxyRendering(!renderer->IsProxyRenderingEnabled());
        }

        if (IsKeyJustDown(KEY_L))
        {
            renderer->EnablePathTracing(!renderer->IsPathTracingEnabled());
        }

        if (!controlDown && IsKeyJustDown(KEY_B))
        {
            uint32_t boundsMode = (uint32_t) renderer->GetBoundsDebugMode();
            boundsMode = (boundsMode + 1) % uint32_t(BoundsDebugMode::Count);
            renderer->SetBoundsDebugMode(BoundsDebugMode(boundsMode));
        }

        if (controlDown && IsKeyJustDown(KEY_G))
        {
            ToggleGrid();
        }

        if (!controlDown && IsKeyJustDown(KEY_T))
        {
            ToggleTransformMode();
        }

        if (IsKeyJustDown(KEY_NUMPAD5))
        {
            Camera3D* camera = GetWorld()->GetActiveCamera();
            if (camera != nullptr)
            {
                ProjectionMode newMode = (camera->GetProjectionMode() == ProjectionMode::ORTHOGRAPHIC) ? ProjectionMode::PERSPECTIVE : ProjectionMode::ORTHOGRAPHIC;
                camera->SetProjectionMode(newMode);
            }
        }

        if (IsKeyJustDown(KEY_F) ||
            IsKeyJustDown(KEY_DECIMAL))
        {
            // Focus on selected object
            Node* node = GetEditorState()->GetSelectedNode();
            Node3D* node3d = (node && node->IsNode3D()) ? static_cast<Node3D*>(node) : nullptr;

            if (node3d != nullptr && node3d != camera)
            {
                glm::vec3 cameraPos = camera->GetAbsolutePosition();
                glm::vec3 compPos = node3d->GetAbsolutePosition();
                glm::vec3 toCamera = glm::normalize(cameraPos - compPos);
                camera->SetAbsolutePosition(compPos + toCamera * sDefaultFocalDistance);

                glm::quat cameraRot = glm::conjugate(glm::toQuat(glm::lookAt(toCamera, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f))));
                camera->SetAbsoluteRotation(cameraRot);

                mFocalDistance = sDefaultFocalDistance;
            }
        }

        if (IsKeyJustDown(KEY_NUMPAD1))
        {
            // Back
            if (controlDown)
            {
                camera->SetAbsoluteRotation(glm::vec3(0.0f, 180.0f, 0.0f));
                camera->SetAbsolutePosition(focus + mFocalDistance * glm::vec3(0.0f, 0.0f, -1.0f));
            }
            // Front
            else
            {
                camera->SetAbsoluteRotation(glm::vec3(0.0f, 0.0f, 0.0f));
                camera->SetAbsolutePosition(focus + mFocalDistance * glm::vec3(0.0f, 0.0f, 1.0f));
            }
        }

        if (IsKeyJustDown(KEY_NUMPAD3))
        {
            // Left
            if (controlDown)
            {
                camera->SetAbsoluteRotation(glm::vec3(0.0f, 270.0f, 0.0f));
                camera->SetAbsolutePosition(focus + mFocalDistance * glm::vec3(-1.0f, 0.0f, 0.0f));
            }
            // Right
            else
            {
                camera->SetAbsoluteRotation(glm::vec3(0.0f, 90.0f, 0.0f));
                camera->SetAbsolutePosition(focus + mFocalDistance * glm::vec3(1.0f, 0.0f, 0.0f));
            }
        }

        if (IsKeyJustDown(KEY_NUMPAD7))
        {
            // Bottom
            if (controlDown)
            {
                camera->SetAbsoluteRotation(glm::vec3(90.0f, 0.0f, 0.0f));
                camera->SetAbsolutePosition(focus + mFocalDistance * glm::vec3(0.0f, -1.0f, 0.0f));
            }
            // Top
            else
            {
                camera->SetAbsoluteRotation(glm::vec3(-90.0f, 0.0f, 0.0f));
                camera->SetAbsolutePosition(focus + mFocalDistance * glm::vec3(0.0f, 1.0f, 0.0f));
            }
        }

        glm::vec3 spawnPos = camera->GetAbsolutePosition() + mFocalDistance * camera->GetForwardVector();
        if (controlDown && IsKeyJustDown(KEY_1))
        {
            ActionManager::Get()->SpawnBasicNode(BASIC_STATIC_MESH, nullptr, nullptr, true, spawnPos);
        }
        else if (controlDown && IsKeyJustDown(KEY_2))
        {
            ActionManager::Get()->SpawnBasicNode(BASIC_POINT_LIGHT, nullptr, nullptr, true, spawnPos);
        }
        else if (controlDown && IsKeyJustDown(KEY_3))
        {
            ActionManager::Get()->SpawnBasicNode(BASIC_NODE_3D, nullptr, nullptr, true, spawnPos);
        }
        else if (controlDown && IsKeyJustDown(KEY_4))
        {
            ActionManager::Get()->SpawnBasicNode(BASIC_DIRECTIONAL_LIGHT, nullptr, nullptr, true, spawnPos);
        }
        else if (controlDown && IsKeyJustDown(KEY_5))
        {
            ActionManager::Get()->SpawnBasicNode(BASIC_SKELETAL_MESH, nullptr, nullptr, true, spawnPos);
        }
        else if (controlDown && IsKeyJustDown(KEY_6))
        {
            ActionManager::Get()->SpawnBasicNode(BASIC_BOX, nullptr, nullptr, true, spawnPos);
        }
        else if (controlDown && IsKeyJustDown(KEY_7))
        {
            ActionManager::Get()->SpawnBasicNode(BASIC_SPHERE, nullptr, nullptr, true, spawnPos);
        }
        else if (controlDown && IsKeyJustDown(KEY_8))
        {
            ActionManager::Get()->SpawnBasicNode(BASIC_PARTICLE, nullptr, nullptr, true, spawnPos);
        }
        else if (controlDown && IsKeyJustDown(KEY_9))
        {
            ActionManager::Get()->SpawnBasicNode(BASIC_AUDIO, nullptr, nullptr, true, spawnPos);
        }
        else if (controlDown && IsKeyJustDown(KEY_0))
        {
            ActionManager::Get()->SpawnBasicNode(BASIC_SCENE, nullptr, nullptr, true, spawnPos);
        }


        if (IsKeyJustDown(KEY_DELETE))
        {
            ActionManager::Get()->DeleteSelectedNodes();
        }

        if (controlDown && IsKeyJustDown(KEY_D))
        {
            // Duplicate node
            Node* selectedNode = GetEditorState()->GetSelectedNode();

            if (selectedNode != nullptr)
            {
                ActionManager::Get()->DuplicateNode(selectedNode);
                GetEditorState()->SetControlMode(ControlMode::Translate);
                SavePreTransforms();
            }
        }

        if (shiftDown && (IsKeyJustDown(KEY_A) || IsKeyJustDown(KEY_Q)))
        {
            // Set the spawn actor list as the modal widget.
            const bool basic = IsKeyJustDown(KEY_Q);
            ShowSpawnActorPrompt(basic);
        }


        if (altDown && IsKeyJustDown(KEY_A))
        {
            GetEditorState()->SetSelectedNode(nullptr);
        }
        if (controlDown && IsKeyJustDown(KEY_A))
        {
            std::vector<Node*> nodes = GetWorld()->GatherNodes();

            for (uint32_t i = 0; i < nodes.size(); ++i)
            {
                GetEditorState()->AddSelectedNode(nodes[i], false);
            }
        }

        // Actor placement hotkeys
        if (IsKeyJustDown(KEY_END) || IsKeyJustDown(KEY_INSERT))
        {
            static glm::vec3 sLastNormal = { 0.0f, 1.0f, 0.0f };
            static float sLastPressedTime = 0.0f;
            float pressedTime = GetAppClock()->GetTime();
            float deltaPressTime = pressedTime - sLastPressedTime;
            sLastPressedTime = pressedTime;

            bool orient = deltaPressTime < 0.5f;

             Node* selNode = GetEditorState()->GetSelectedNode();
             Node3D* transComp = selNode ? selNode->As<Node3D>() : nullptr;
             Primitive3D* primComp = selNode ? selNode->As<Primitive3D>() : nullptr;

             if (transComp)
             {
                 if (orient)
                 {
                     // If this is the second tap then orient the component to the surface normal.
                     if (glm::dot(sLastNormal, glm::vec3(0.0f, 1.0f, 0.0f)) < 0.99999f)
                     {
                         glm::mat4 rotMat = glm::orientation(sLastNormal, glm::vec3(0.0f, 1.0f, 0.0f));
                         glm::quat rotQuat = glm::quat(rotMat);
                         ActionManager::Get()->EXE_SetAbsoluteRotation(transComp, rotQuat);
                     }
                     else
                     {
                         // Avoid Nans when normal is almost identical to up vector.
                         ActionManager::Get()->EXE_SetAbsoluteRotation(transComp, glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
                     }

                     sLastPressedTime = 0.0f;
                 }
                 else
                 {
                     glm::vec3 startPos = {};
                     glm::vec3 endPos = {};

                     if (IsKeyJustDown(KEY_INSERT))
                     {
                         float mouseX = 0.0f;
                         float mouseY = 0.0f;
                         INP_GetPointerPositionNormalized(mouseX, mouseY);

                         float nearZ = camera->GetNearZ();
                         float farZ = camera->GetFarZ();

                         glm::vec3 rayDir = {};
                         if (camera->GetProjectionMode() == ProjectionMode::ORTHOGRAPHIC)
                         {
                             float x = mouseX * camera->GetOrthoSettings().mWidth;
                             float y = -mouseY * camera->GetOrthoSettings().mHeight;
                             glm::vec3 nearPos = glm::vec3(x, y, -nearZ);
                             startPos = glm::vec3(camera->CalculateInvViewMatrix() * glm::vec4(nearPos, 1.0f));
                             rayDir = camera->GetForwardVector();
                         }
                         else
                         {
                             float aspect = camera->GetAspectRatio();
                             float fovY = DEGREES_TO_RADIANS * camera->GetFieldOfViewY();
                             float fovX = DEGREES_TO_RADIANS * camera->GetFieldOfViewX();
                             glm::vec3 camFwd = camera->GetForwardVector();

                             float dx = nearZ * tanf(fovX / 2.0f);
                             float dy = nearZ * tanf(fovY / 2.0f);

                             glm::vec3 nearPos = glm::vec3(dx * mouseX, dy * -mouseY, -nearZ);
                             startPos = camera->GetAbsolutePosition();
                             rayDir = Maths::SafeNormalize(nearPos);
                             rayDir = glm::vec3(camera->CalculateInvViewMatrix() * glm::vec4(rayDir, 0.0f));
                         }

                         endPos = startPos + rayDir * farZ;
                     }
                     else
                     {
                         startPos = transComp->GetAbsolutePosition();
                         endPos = startPos + glm::vec3(0.0f, -1000.0f, 0.0f);
                     }

                     RayTestResult rayResult;
                     // A convention that I've been using in the engine is that 0x02 collision group
                     // should be used for static environment pieces.

                     // Don't intersect with self
                     bool savedCollisionEnabled = false;
                     if (primComp)
                     {
                         savedCollisionEnabled = primComp->IsCollisionEnabled();
                         primComp->EnableCollision(false);
                     }

                     GetWorld()->RayTest(startPos, endPos, 0x02, rayResult);

                     if (primComp)
                     {
                         primComp->EnableCollision(savedCollisionEnabled);
                     }

                     if (rayResult.mHitComponent != nullptr)
                     {
                         ActionManager::Get()->EXE_SetAbsolutePosition(transComp, rayResult.mHitPosition);
                         sLastNormal = rayResult.mHitNormal;
                     }
                 }
             }
        }

        // Position selected component at camera transform
        if (IsKeyJustDown(KEY_NUMPAD0))
        {
            glm::mat4 camTransform = camera->GetTransform();

            Node* selNode = GetEditorState()->GetSelectedNode();
            Node3D* transComp = selNode ? selNode->As<Node3D>() : nullptr;

            if (transComp)
            {
                ActionManager::Get()->EXE_EditTransform(transComp, camTransform);
            }
        }

        // Handle zoom
        if (scrollDelta != 0)
        {
            Camera3D* camera = GetWorld()->GetActiveCamera();

            if (camera->GetProjectionMode() == ProjectionMode::PERSPECTIVE)
            {
                glm::vec3 forwardVector = camera->GetForwardVector();
                glm::vec3 cameraPos = camera->GetAbsolutePosition();
                float deltaPos = mFocalDistance * 0.05f * scrollDelta;
                camera->SetAbsolutePosition(cameraPos + forwardVector * deltaPos);
                mFocalDistance = glm::max(0.1f, mFocalDistance - deltaPos);
            }
            else
            {
                // For orthographic, adjust the bounds of the projection.
                const float xySpeed = 0.05f;
                const float zSpeed = 0.05f;
                OrthoSettings settings = camera->GetOrthoSettings();
                settings.mWidth *= 1.0f + (xySpeed * -scrollDelta);
                settings.mHeight *= 1.0f + (xySpeed * -scrollDelta);
                settings.mFar *= 1.0f + (zSpeed * -scrollDelta);

                settings.mWidth = glm::max(settings.mWidth, 0.05f);
                settings.mHeight = glm::max(settings.mHeight, 0.05f);
                settings.mFar = glm::max(settings.mFar, 50.0f);
                camera->SetOrthoSettings(settings.mWidth, settings.mHeight, settings.mNear, settings.mFar);
            }
        }

        const bool middleMouseDown = IsMouseButtonDown(MOUSE_MIDDLE);
        if (middleMouseDown)
        {
            if (shiftDown)
            {
                GetEditorState()->SetControlMode(ControlMode::Pan);
            }
            else
            {
                GetEditorState()->SetControlMode(ControlMode::Orbit);
            }
        }
    }
}

void Viewport3D::HandlePilotControls()
{
    Camera3D* camera = GetWorld()->GetActiveCamera();
    float deltaTime = GetAppClock()->DeltaTime();

    int32_t scrollDelta = GetScrollWheelDelta();
    if (scrollDelta != 0)
    {
        const float deltaSpeed = 0.1f;
        float totalDelta = 1.0f + (scrollDelta > 0 ? deltaSpeed : -deltaSpeed);
        totalDelta = pow(totalDelta, fabs((float)scrollDelta));
        mFirstPersonMoveSpeed *= totalDelta;
        mFirstPersonMoveSpeed = glm::clamp(mFirstPersonMoveSpeed, 0.1f, 1000.0f);
    }

    glm::vec2 deltaRot = HandleLockedCursor();

    glm::vec3 cameraPosition = camera->GetPosition();
    glm::vec3 cameraRotation = camera->GetRotationEuler();

    float angleY = glm::radians(cameraRotation.y);
    float c = cos(angleY);
    float s = sin(angleY);
    float angleX = glm::radians(cameraRotation.x);
    float xc = cos(angleX);
    float xs = sin(angleX);

    if (IsKeyDown(KEY_A))
    {
        cameraPosition.x -= c * (mFirstPersonMoveSpeed * deltaTime);
        cameraPosition.z += s * (mFirstPersonMoveSpeed * deltaTime);
    }

    if (IsKeyDown(KEY_D))
    {
        cameraPosition.x += c * (mFirstPersonMoveSpeed * deltaTime);
        cameraPosition.z -= s * (mFirstPersonMoveSpeed * deltaTime);
    }

    if (IsKeyDown(KEY_W))
    {
        cameraPosition.z -= xc * (c * (mFirstPersonMoveSpeed * deltaTime));
        cameraPosition.x -= xc * (s * (mFirstPersonMoveSpeed * deltaTime));
        cameraPosition.y += xs * (mFirstPersonMoveSpeed * deltaTime);
    }

    if (IsKeyDown(KEY_S))
    {
        cameraPosition.z += xc * (c * (mFirstPersonMoveSpeed * deltaTime));
        cameraPosition.x += xc * (s * (mFirstPersonMoveSpeed * deltaTime));
        cameraPosition.y -= xs * (mFirstPersonMoveSpeed * deltaTime);
    }

    if (IsKeyDown(KEY_E))
    {
        cameraPosition.y += (mFirstPersonMoveSpeed * deltaTime);
    }

    if (IsKeyDown(KEY_Q))
    {
        cameraPosition.y -= (mFirstPersonMoveSpeed * deltaTime);
    }

    cameraRotation.y -= deltaRot.x * mFirstPersonRotationSpeed;
    cameraRotation.x -= deltaRot.y * mFirstPersonRotationSpeed;

    // Clamp x rotation to -90/90
    cameraRotation.x = glm::clamp(cameraRotation.x, -sMaxCameraPitch, sMaxCameraPitch);

    camera->SetPosition(cameraPosition);
    camera->SetRotation(cameraRotation);

    if (!IsMouseButtonDown(MOUSE_RIGHT))
    {
        GetEditorState()->SetControlMode(ControlMode::Default);
    }
}

void Viewport3D::HandleTransformControls()
{
    ControlMode controlMode = GetEditorState()->GetControlMode();
    Node* node = GetEditorState()->GetSelectedNode();
    const std::vector<Node*>& selectedComps = GetEditorState()->GetSelectedNodes();

    if (node == nullptr || !node->IsNode3D())
        return;

    Node3D* transComp = static_cast<Node3D*>(node);

    std::vector<Node3D*> transComps;
    for (uint32_t i = 0; i < selectedComps.size(); ++i)
    {
        if (selectedComps[i]->IsNode3D())
        {
            transComps.push_back((Node3D*)selectedComps[i]);
        }
    }

    Camera3D* camera = GetWorld()->GetActiveCamera();
    glm::mat4 invViewMat = camera->CalculateInvViewMatrix();

    HandleAxisLocking();

    glm::vec2 delta = HandleLockedCursor();
    delta.y *= -1.0f;

    const bool shiftDown = IsShiftDown();
    const float shiftSpeedMult = 0.1f;

    if (delta != glm::vec2(0.0f, 0.0f))
    {
        if (controlMode == ControlMode::Translate)
        {
            const float translateSpeed = 0.025f;
            glm::vec3 worldDelta = glm::vec3(delta.x, delta.y, 0.0f);
            worldDelta = invViewMat * glm::vec4(worldDelta, 0.0);
            worldDelta = GetLockedTranslationDelta(worldDelta);

            float speed = shiftDown ? (shiftSpeedMult * translateSpeed) : translateSpeed;

            for (uint32_t i = 0; i < transComps.size(); ++i)
            {
                glm::vec3 pos = transComps[i]->GetAbsolutePosition();
                pos += speed * glm::vec3(worldDelta.x, worldDelta.y, worldDelta.z);
                transComps[i]->SetAbsolutePosition(pos);
            }
        }
        else if (controlMode == ControlMode::Rotate)
        {
            const float rotateSpeed = 0.025f;

            float speed = shiftDown ? (shiftSpeedMult * rotateSpeed) : rotateSpeed;
            glm::vec3 rotateAxisWS = invViewMat * glm::vec4(0, 0, 1, 0);
            float totalDelta = delta.x - delta.y;

            if (GetEditorState()->mTransformLock != TransformLock::None)
            {
                rotateAxisWS = GetLockedRotationAxis();
            }

            glm::quat addQuat = glm::angleAxis(-totalDelta * speed, rotateAxisWS);

            if (mTransformLocal)
            {
                for (uint32_t i = 0; i < transComps.size(); ++i)
                {
                    transComps[i]->AddAbsoluteRotation(addQuat);
                }
            }
            else
            {
                glm::vec3 pivot = transComp->GetAbsolutePosition();

                for (uint32_t i = 0; i < transComps.size(); ++i)
                {
                    glm::mat4 rotMat = glm::translate(pivot);
                    rotMat *= glm::rotate(-totalDelta * speed, rotateAxisWS);
                    rotMat *= glm::translate(-pivot);

                    glm::mat4 transform = transComps[i]->GetTransform();
                    transComps[i]->SetTransform(rotMat * transform);
                }
            }
        }
        else if (controlMode == ControlMode::Scale)
        {
            // Only enable uniform scaling for now
            const float scaleSpeed = 0.025f;

            float speed = shiftDown ? (shiftSpeedMult * scaleSpeed) : scaleSpeed;
            float totalDelta = delta.x - delta.y;
            float deltaScale = speed * totalDelta;
            glm::vec3 deltaScale3 = glm::vec3(deltaScale, deltaScale, deltaScale);

            if (GetEditorState()->mTransformLock != TransformLock::None)
            {
                deltaScale3 *= GetLockedScaleDelta();
            }

            if (mTransformLocal)
            {
                for (uint32_t i = 0; i < transComps.size(); ++i)
                {
                    glm::vec3 scale = transComps[i]->GetScale();
                    scale += deltaScale;
                    transComps[i]->SetScale(scale);
                }
            }
            else
            {
                glm::vec3 pivot = transComp->GetAbsolutePosition();

                for (uint32_t i = 0; i < transComps.size(); ++i)
                {
                    glm::vec3 scale = transComps[i]->GetScale();
                    glm::vec3 newScale = (deltaScale3 + scale);

                    if (glm::length(scale) != 0.0f &&
                        glm::length(newScale) != 0.0f)
                    {
                        glm::mat4 scaleMat = glm::translate(pivot);
                        scaleMat *= glm::scale(newScale / scale);
                        scaleMat *= glm::translate(-pivot);

                        glm::vec3 scaledPos = scaleMat * glm::vec4(transComps[i]->GetAbsolutePosition(), 1.0f);

                        glm::vec3 newScaleRot = newScale;

                        if (GetEditorState()->mTransformLock != TransformLock::None)
                        {
                            glm::quat absRot = transComps[i]->GetAbsoluteRotationQuat();
                            glm::mat4 rotMat = glm::mat4(absRot);
                            glm::vec3 deltaScaleRot = rotMat * glm::vec4(deltaScale3, 0.0f);

                            if (deltaScale >= 0.0f)
                            {
                                deltaScaleRot = glm::abs(deltaScaleRot);
                            }
                            else
                            {
                                deltaScaleRot = -glm::abs(deltaScaleRot);
                            }

                            newScaleRot = scale + deltaScaleRot;
                        }

                        transComps[i]->SetAbsolutePosition(scaledPos);
                        transComps[i]->SetScale(newScaleRot);
                    }
                }
            }
        }
    }

    if (!IsControlDown() && IsKeyJustDown(KEY_T))
    {
        ToggleTransformMode();
    }

    if (IsMouseButtonDown(MOUSE_LEFT))
    {
        std::vector<glm::mat4> newTransforms;

        for (uint32_t i = 0; i < transComps.size(); ++i)
        {
            // Make sure transform isn't dirty.
            transComps[i]->UpdateTransform(false);
            newTransforms.push_back(transComps[i]->GetTransform());
        }

        RestorePreTransforms();

        for (uint32_t i = 0; i < transComps.size(); ++i)
        {
            // Make sure transform isn't dirty.
            transComps[i]->UpdateTransform(false);
        }

        ActionManager::Get()->EXE_EditTransforms(transComps, newTransforms);
        GetEditorState()->SetControlMode(ControlMode::Default);
    }

    if (IsMouseButtonDown(MOUSE_RIGHT))
    {
        // Cancel transform operation
        RestorePreTransforms();
        GetEditorState()->SetControlMode(ControlMode::Default);
    }
}

void Viewport3D::HandlePanControls()
{
    // Pan the camera
    Camera3D* camera = GetWorld()->GetActiveCamera();
    glm::vec3 cameraPos = camera->GetAbsolutePosition();
    glm::vec3 right = camera->GetRightVector();
    glm::vec3 up = camera->GetUpVector();

    glm::vec2 deltaPosVS = HandleLockedCursor();
    glm::vec3 deltaPosWS = 0.002f * -(right * deltaPosVS.x - up * deltaPosVS.y);
    camera->SetAbsolutePosition(cameraPos + deltaPosWS);

    if (!IsMouseButtonDown(MOUSE_MIDDLE))
    {
        GetEditorState()->SetControlMode(ControlMode::Default);
    }
}

void Viewport3D::HandleOrbitControls()
{
    Camera3D* camera = GetWorld()->GetActiveCamera();
    glm::vec2 delta = HandleLockedCursor();
    glm::vec3 cameraPos = camera->GetPosition();
    glm::vec3 cameraRot = camera->GetRotationEuler();
    glm::vec3 focus = cameraPos + camera->GetForwardVector() * mFocalDistance;

    // Transform the camera into focus space
    float deltaYaw = -delta.x * 0.005f;
    float deltaPitch = -delta.y * 0.005f;

    if (cameraRot.x > sMaxCameraPitch)
        cameraRot.x = sMaxCameraPitch;
    else if (cameraRot.x < -sMaxCameraPitch)
        cameraRot.x = -sMaxCameraPitch;

    // clamp pitch changes
    if (cameraRot.x > 0.0f)
    {
        deltaPitch = glm::min(deltaPitch, (sMaxCameraPitch - cameraRot.x) * DEGREES_TO_RADIANS);
    }
    else
    {
        deltaPitch = glm::max(deltaPitch, (-sMaxCameraPitch - cameraRot.x) * DEGREES_TO_RADIANS);
    }

    glm::mat4 matYaw = glm::rotate(glm::mat4(1), deltaYaw, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 matPitch = glm::rotate(glm::mat4(1), deltaPitch, camera->GetRightVector());
    glm::vec4 rotPoint = glm::vec4(cameraPos - focus, 1.0f);

    glm::vec3 newPos = focus + glm::vec3(matPitch * matYaw * rotPoint);
    glm::vec3 newRot = cameraRot + glm::vec3(deltaPitch, deltaYaw, 0.0f) * RADIANS_TO_DEGREES;

    camera->SetPosition(newPos);
    camera->SetRotation(newRot);

    if (!IsMouseButtonDown(MOUSE_MIDDLE))
    {
        GetEditorState()->SetControlMode(ControlMode::Default);
    }
}

glm::vec2 Viewport3D::HandleLockedCursor()
{
    int32_t dX = 0;
    int32_t dY = 0;
    INP_GetMouseDelta(dX, dY);

    return glm::vec2((float)dX, (float)dY);
}

void Viewport3D::HandleAxisLocking()
{
    EditorState* state = GetEditorState();
    TransformLock currentLock = state->mTransformLock;
    TransformLock newLock = TransformLock::None;

    // Plane locking
    if (IsShiftDown())
    {
        if (IsKeyJustDown(KEY_X))
        {
            newLock = TransformLock::PlaneYZ;
        }

        if (IsKeyJustDown(KEY_Y))
        {
            newLock = TransformLock::PlaneXZ;
        }

        if (IsKeyJustDown(KEY_Z))
        {
            newLock = TransformLock::PlaneXY;
        }
    }
    // Axis locking
    else
    {
        if (IsKeyJustDown(KEY_X))
        {
            newLock = TransformLock::AxisX;
        }

        if (IsKeyJustDown(KEY_Y))
        {
            newLock = TransformLock::AxisY;
        }

        if (IsKeyJustDown(KEY_Z))
        {
            newLock = TransformLock::AxisZ;
        }
    }

    if (newLock != TransformLock::None)
    {
        RestorePreTransforms();

        if (newLock != currentLock)
        {
            GetEditorState()->SetTransformLock(newLock);
        }
        else
        {
            // If hitting the same lock key, unlock. (reset to None transform lock).
            GetEditorState()->SetTransformLock(TransformLock::None);
        }
    }
}

glm::vec2 Viewport3D::GetTransformDelta() const
{
    glm::vec2 delta = glm::vec2(0.0f, 0.0f);

    int32_t x = 0;
    int32_t y = 0;
    INP_GetMousePosition(x, y);

    delta.x = static_cast<float>(x - mPrevMouseX);
    delta.y = -static_cast<float>(y - mPrevMouseY);

    return delta;
}

void Viewport3D::SavePreTransforms()
{
    const std::vector<Node*>& selNodes = GetEditorState()->GetSelectedNodes();
    mPreTransforms.clear();

    for (uint32_t i = 0; i < selNodes.size(); ++i)
    {
        Node3D* transComp = (selNodes[i] && selNodes[i]->IsNode3D()) ? static_cast<Node3D*>(selNodes[i]) : nullptr;

        if (transComp)
        {
            mPreTransforms.push_back(transComp->GetTransform());
        }
    }
}

void Viewport3D::RestorePreTransforms()
{
    const std::vector<Node*>& selNodes = GetEditorState()->GetSelectedNodes();

    for (uint32_t i = 0; i < selNodes.size(); ++i)
    {
        if (i >= mPreTransforms.size())
        {
            LogError("Component/Transform array mismatch?");
            break;
        }

        Node3D* transComp = (selNodes[i] && selNodes[i]->IsNode3D()) ? static_cast<Node3D*>(selNodes[i]) : nullptr;

        if (transComp)
        {
            transComp->SetTransform(mPreTransforms[i]);
        }
    }
}

void Viewport3D::ToggleTransformMode()
{
    mTransformLocal = !mTransformLocal;
    LogDebug("Transform Mode = %s", mTransformLocal ? "Local" : "Pivot");
}

glm::vec3 Viewport3D::GetLockedTranslationDelta(glm::vec3 deltaWS) const
{
    glm::vec3 retDelta = deltaWS;
    retDelta *= GetEditorState()->GetTransformLockVector(GetEditorState()->mTransformLock);
    return retDelta;
}

glm::vec3 Viewport3D::GetLockedRotationAxis() const
{
    glm::vec3 ret = glm::vec3(0.0, 1.0, 0.0);

    switch (GetEditorState()->mTransformLock)
    {
    case TransformLock::AxisX:
    case TransformLock::PlaneYZ:
        ret = glm::vec3(1.0, 0.0, 0.0);
        break;
    case TransformLock::AxisY:
    case TransformLock::PlaneXZ:
        ret = glm::vec3(0.0, 1.0, 0.0);
        break;
    case TransformLock::AxisZ:
    case TransformLock::PlaneXY:
        ret = glm::vec3(0.0, 0.0, 1.0);
        break;

    default: break;
    }

    return ret;
}

glm::vec3 Viewport3D::GetLockedScaleDelta()
{
    glm::vec3 retDelta = glm::vec3(1, 1, 1);
    retDelta *= GetEditorState()->GetTransformLockVector(GetEditorState()->mTransformLock);
    return retDelta;
}

bool Viewport3D::IsMouseOnAnyButton() const
{
    bool ret = false;

    for (uint32_t i = 0; i < mButtons.size(); ++i)
    {
        if (mButtons[i]->IsVisible() &&
            mButtons[i]->ContainsMouse())
        {
            ret = true;
            break;
        }
    }

    return ret;
}

void Viewport3D::ShowSpawnActorPrompt(bool basic)
{
#if NODE_CONV_EDITOR

    ActionList* actionList = GetActionList();

    std::vector<std::string> actions;

    if (basic)
    {
        actions.push_back(BASIC_BLUEPRINT);
        actions.push_back(BASIC_STATIC_MESH);
        actions.push_back(BASIC_POINT_LIGHT);
        actions.push_back(BASIC_TRANSFORM);
        actions.push_back(BASIC_DIRECTIONAL_LIGHT);
        actions.push_back(BASIC_SKELETAL_MESH);
        actions.push_back(BASIC_BOX);
        actions.push_back(BASIC_SPHERE);
        actions.push_back(BASIC_CAPSULE);
        actions.push_back(BASIC_PARTICLE);
        actions.push_back(BASIC_AUDIO);
        actions.push_back(BASIC_CAMERA);
        actions.push_back(BASIC_TEXT_MESH);

        actionList->SetActions(actions, HandleSpawnBasicPressed);
    }
    else
    {
        GatherAllClassNames(actions);
        actionList->SetActions(actions, HandleSpawnActorPressed);
    }

#endif
}

#endif
