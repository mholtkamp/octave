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
#include "PaintManager.h"
#include "Nodes/3D/StaticMesh3d.h"
#include "Nodes/3D/PointLight3d.h"
#include "Nodes/3D/DirectionalLight3d.h"
#include "Nodes/3D/Node3d.h"
#include "Nodes/3D/SkeletalMesh3d.h"
#include "Nodes/3D/InstancedMesh3d.h"
#include "Nodes/3D/Box3d.h"
#include "Nodes/3D/Sphere3d.h"
#include "Nodes/3D/Particle3d.h"
#include "Nodes/3D/Audio3d.h"
#include "Nodes/3D/TextMesh3d.h"
#include "System/System.h"

#include "imgui.h"

constexpr float sMaxCameraPitch = 89.99f;

Viewport3D::Viewport3D()
{

}

Viewport3D::~Viewport3D()
{

}

void Viewport3D::Update(float deltaTime)
{
    ControlMode controlMode = GetEditorState()->GetControlMode();

    if (ShouldHandleInput())
    {
        if (GetEditorState()->mMouseNeedsRecenter)
        {
            EditorCenterCursor();
            GetEditorState()->mMouseNeedsRecenter = false;
        }

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

    if (GetEditorState()->GetPaintMode() != PaintMode::None && controlMode == ControlMode::Default)
    {
        GetEditorState()->mPaintManager->Update();
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
    

    bool handleInput = (modal == nullptr && !imguiAnyWindowHovered && !imguiWantsText && !imguiAnyPopupUp && !imguiWantsMouse);
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
    Camera3D* camera = GetWorld(0)->GetActiveCamera();
    glm::vec3 focus = camera->GetWorldPosition() + camera->GetForwardVector() * mFocalDistance;

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

        if (IsMouseButtonJustDown(MOUSE_LEFT))
        {
            int32_t mouseX = 0;
            int32_t mouseY = 0;
            GetMousePosition(mouseX, mouseY);

            uint32_t selectInstance = 0;
            Node3D* selectNode = Renderer::Get()->ProcessHitCheck(GetWorld(0), mouseX, mouseY, &selectInstance);

            if (shiftDown || controlDown)
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
            else if (GetEditorState()->GetPaintMode() == PaintMode::None)
            {
                if (altDown)
                {
                    GetEditorState()->SetSelectedNode(selectNode);
                    GetEditorState()->mTrackSelectedNode = true;
                }
                else
                {
                    if (GetEditorState()->GetSelectedNode() != selectNode)
                    {
                        GetEditorState()->SetSelectedNode(selectNode);
                    }
                    else
                    {
                        int32_t curSelInstance = GetEditorState()->GetSelectedInstance();
                        InstancedMesh3D* instMesh = selectNode ? selectNode->As<InstancedMesh3D>() : nullptr;
                        if (instMesh != nullptr &&
                            curSelInstance != int32_t(selectInstance))
                        {
                            // We clicked a different instance. Don't deselect, just change the sel instance
                            GetEditorState()->SetSelectedNode(selectNode);
                            GetEditorState()->SetSelectedInstance((int32_t)selectInstance);
                        }
                        else
                        {
                            GetEditorState()->SetSelectedNode(nullptr);
                        }
                    }
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

        if (!cmdKeyDown && IsKeyJustDown(KEY_Z))
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

        if (altDown && IsKeyJustDown(KEY_L))
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

        if (controlDown && IsKeyJustDown(KEY_T))
        {
            ToggleTransformMode();
        }

        if (IsKeyJustDown(KEY_NUMPAD5))
        {
            Camera3D* camera = GetWorld(0)->GetActiveCamera();
            if (camera != nullptr)
            {
                ProjectionMode newMode = (camera->GetProjectionMode() == ProjectionMode::ORTHOGRAPHIC) ? ProjectionMode::PERSPECTIVE : ProjectionMode::ORTHOGRAPHIC;
                camera->SetProjectionMode(newMode);
            }
        }

        if ((IsKeyDown(KEY_F) && GetEditorState()->GetPaintMode() == PaintMode::None) ||
            IsKeyDown(KEY_DECIMAL))
        {
            // Focus on selected object
            Node* node = GetEditorState()->GetSelectedNode();
            Node3D* node3d = (node && node->IsNode3D()) ? static_cast<Node3D*>(node) : nullptr;

            if (node3d != nullptr && node3d != camera)
            {
                glm::vec3 compPos = node3d->GetWorldPosition();

                Primitive3D* prim = node3d->As<Primitive3D>();

                float boundsRadius = 0.0f;
                if (prim != nullptr)
                {
                    Bounds bounds = prim->GetBounds();

                    // Some primitives (like Box,Sphere,Capsule) just have default bounds.
                    // Same for particles that don't have their bounds set.
                    // So just treat the bounds radius as 0.0 instead of blasting the camera to 10000 units away.
                    if (bounds.mRadius < (LARGE_BOUNDS - 1.0f))
                    {
                        boundsRadius = bounds.mRadius;
                    }

                    compPos = bounds.mCenter;
                }

                float focusDist = glm::max(boundsRadius + 1.0f, 2.0f);

                glm::vec3 cameraPos = camera->GetWorldPosition();
                glm::vec3 toCamera = Maths::SafeNormalize(cameraPos - compPos);
                glm::vec3 newCamPos = compPos + toCamera * focusDist;
                camera->SetWorldPosition(newCamPos);

                glm::quat cameraRot = glm::conjugate(glm::toQuat(glm::lookAt(toCamera, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f))));
                camera->SetWorldRotation(cameraRot);

                mFocalDistance = focusDist;
            }
        }

        if (IsKeyJustDown(KEY_NUMPAD1))
        {
            // Back
            if (controlDown)
            {
                camera->SetWorldRotation(glm::vec3(0.0f, 180.0f, 0.0f));
                camera->SetWorldPosition(focus + mFocalDistance * glm::vec3(0.0f, 0.0f, -1.0f));
            }
            // Front
            else
            {
                camera->SetWorldRotation(glm::vec3(0.0f, 0.0f, 0.0f));
                camera->SetWorldPosition(focus + mFocalDistance * glm::vec3(0.0f, 0.0f, 1.0f));
            }
        }

        if (IsKeyJustDown(KEY_NUMPAD3))
        {
            // Left
            if (controlDown)
            {
                camera->SetWorldRotation(glm::vec3(0.0f, 270.0f, 0.0f));
                camera->SetWorldPosition(focus + mFocalDistance * glm::vec3(-1.0f, 0.0f, 0.0f));
            }
            // Right
            else
            {
                camera->SetWorldRotation(glm::vec3(0.0f, 90.0f, 0.0f));
                camera->SetWorldPosition(focus + mFocalDistance * glm::vec3(1.0f, 0.0f, 0.0f));
            }
        }

        if (IsKeyJustDown(KEY_NUMPAD7))
        {
            // Bottom
            if (controlDown)
            {
                camera->SetWorldRotation(glm::vec3(90.0f, 0.0f, 0.0f));
                camera->SetWorldPosition(focus + mFocalDistance * glm::vec3(0.0f, -1.0f, 0.0f));
            }
            // Top
            else
            {
                camera->SetWorldRotation(glm::vec3(-90.0f, 0.0f, 0.0f));
                camera->SetWorldPosition(focus + mFocalDistance * glm::vec3(0.0f, 1.0f, 0.0f));
            }
        }

        glm::vec3 spawnPos = camera->GetWorldPosition() + mFocalDistance * camera->GetForwardVector();
        if (altDown && IsKeyJustDown(KEY_1))
        {
            ActionManager::Get()->SpawnBasicNode(BASIC_STATIC_MESH, nullptr, nullptr, true, spawnPos);
        }
        else if (altDown && IsKeyJustDown(KEY_2))
        {
            ActionManager::Get()->SpawnBasicNode(BASIC_POINT_LIGHT, nullptr, nullptr, true, spawnPos);
        }
        else if (altDown && IsKeyJustDown(KEY_3))
        {
            ActionManager::Get()->SpawnBasicNode(BASIC_NODE_3D, nullptr, nullptr, true, spawnPos);
        }
        else if (altDown && IsKeyJustDown(KEY_4))
        {
            ActionManager::Get()->SpawnBasicNode(BASIC_DIRECTIONAL_LIGHT, nullptr, nullptr, true, spawnPos);
        }
        else if (altDown && IsKeyJustDown(KEY_5))
        {
            ActionManager::Get()->SpawnBasicNode(BASIC_SKELETAL_MESH, nullptr, nullptr, true, spawnPos);
        }
        else if (altDown && IsKeyJustDown(KEY_6))
        {
            ActionManager::Get()->SpawnBasicNode(BASIC_BOX, nullptr, nullptr, true, spawnPos);
        }
        else if (altDown && IsKeyJustDown(KEY_7))
        {
            ActionManager::Get()->SpawnBasicNode(BASIC_SPHERE, nullptr, nullptr, true, spawnPos);
        }
        else if (altDown && IsKeyJustDown(KEY_8))
        {
            ActionManager::Get()->SpawnBasicNode(BASIC_PARTICLE, nullptr, nullptr, true, spawnPos);
        }
        else if (altDown && IsKeyJustDown(KEY_9))
        {
            ActionManager::Get()->SpawnBasicNode(BASIC_AUDIO, nullptr, nullptr, true, spawnPos);
        }
        else if (altDown && IsKeyJustDown(KEY_0))
        {
            ActionManager::Get()->SpawnBasicNode(BASIC_SCENE, nullptr, nullptr, true, spawnPos);
        }


        if (IsKeyJustDown(KEY_DELETE))
        {
            Node* selNode = GetEditorState()->GetSelectedNode();
            int32_t selInstance = GetEditorState()->GetSelectedInstance();
            InstancedMesh3D* instMesh = selNode ? selNode->As<InstancedMesh3D>() : nullptr;

            if (instMesh && selInstance >= 0)
            {
                // If a specific instance is selected, delete that instance
                std::vector<MeshInstanceData> meshInstData = instMesh->GetInstanceData();
                if (selInstance < meshInstData.size())
                {
                    meshInstData.erase(meshInstData.begin() + selInstance);
                    ActionManager::Get()->EXE_SetInstanceData(instMesh, -1, meshInstData);
                }
                else
                {
                    LogError("Can't delete invalid instance index");
                }
            }
            else
            {
                ActionManager::Get()->DeleteSelectedNodes();
            }
        }

        if (controlDown && IsKeyJustDown(KEY_D))
        {
            // Duplicate node
            const std::vector<Node*>& selectedNodes = GetEditorState()->GetSelectedNodes();

            if (selectedNodes.size() > 0)
            {
                ActionManager::Get()->DuplicateNodes(selectedNodes);
                GetEditorState()->SetControlMode(ControlMode::Translate);
                SavePreTransforms();
            }
        }

        if (altDown && IsKeyJustDown(KEY_A))
        {
            GetEditorState()->SetSelectedNode(nullptr);
        }
        if (controlDown && IsKeyJustDown(KEY_A))
        {
            std::vector<Node*> nodes = GetWorld(0)->GatherNodes();

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
                         ActionManager::Get()->EXE_SetWorldRotation(transComp, rotQuat);
                     }
                     else
                     {
                         // Avoid Nans when normal is almost identical to up vector.
                         ActionManager::Get()->EXE_SetWorldRotation(transComp, glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
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
                             float x = mouseX * camera->GetOrthoWidth();
                             float y = -mouseY * camera->GetOrthoHeight();
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
                             startPos = camera->GetWorldPosition();
                             rayDir = Maths::SafeNormalize(nearPos);
                             rayDir = glm::vec3(camera->CalculateInvViewMatrix() * glm::vec4(rayDir, 0.0f));
                         }

                         endPos = startPos + rayDir * farZ;
                     }
                     else
                     {
                         startPos = transComp->GetWorldPosition();
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

                     GetWorld(0)->RayTest(startPos, endPos, 0x02, rayResult);

                     if (primComp)
                     {
                         primComp->EnableCollision(savedCollisionEnabled);
                     }

                     if (rayResult.mHitNode != nullptr)
                     {
                         ActionManager::Get()->EXE_SetWorldPosition(transComp, rayResult.mHitPosition);
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
            Camera3D* camera = GetWorld(0)->GetActiveCamera();

            if (camera->GetProjectionMode() == ProjectionMode::PERSPECTIVE)
            {
                glm::vec3 forwardVector = camera->GetForwardVector();
                glm::vec3 cameraPos = camera->GetWorldPosition();
                float deltaPos = mFocalDistance * 0.05f * scrollDelta;
                camera->SetWorldPosition(cameraPos + forwardVector * deltaPos);
                mFocalDistance = glm::max(0.1f, mFocalDistance - deltaPos);
            }
            else
            {
                // For orthographic, adjust the bounds of the projection.
                const float xySpeed = 0.05f;
                const float zSpeed = 0.05f;

                float orthoWidth = camera->GetOrthoWidth();
                float farZ = camera->GetFarZ();

                orthoWidth *= 1.0f + (xySpeed * -scrollDelta);
                farZ *= 1.0f + (zSpeed * -scrollDelta);

                orthoWidth = glm::max(orthoWidth, 0.05f);
                farZ = glm::max(farZ, 50.0f);

                camera->SetOrthoWidth(orthoWidth);
                camera->SetFarZ(farZ);
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
    Camera3D* camera = GetWorld(0)->GetActiveCamera();
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

    bool instance = false;
    int32_t selInstance = GetEditorState()->GetSelectedInstance();
    InstancedMesh3D* instMesh = nullptr;
    MeshInstanceData instData;

    if (ShouldTransformInstance())
    {
        instMesh = selectedComps[0]->As<InstancedMesh3D>();
        if (selInstance >= 0 && selInstance < int32_t(instMesh->GetNumInstances()))
        {
            instData = instMesh->GetInstanceData(selInstance);
            instance = true;
        }
    }

    Camera3D* camera = GetWorld(0)->GetActiveCamera();
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

            if (instance)
            {
                glm::mat4 invTransform = glm::inverse(instMesh->GetTransform());
                glm::vec3 localDelta = invTransform * glm::vec4(worldDelta.x, worldDelta.y, worldDelta.z, 0.0f);
                instData.mPosition += speed * localDelta;
                instMesh->SetInstanceData(selInstance, instData);
            }
            else
            {
                for (uint32_t i = 0; i < transComps.size(); ++i)
                {
                    glm::vec3 pos = transComps[i]->GetWorldPosition();
                    pos += speed * glm::vec3(worldDelta.x, worldDelta.y, worldDelta.z);
                    transComps[i]->SetWorldPosition(pos);
                }
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

            if (instance)
            {
                glm::mat4 invTransform = glm::inverse(instMesh->GetTransform());
                glm::vec3 localRotateAxis = invTransform * glm::vec4(rotateAxisWS, 0.0f);
                glm::quat localAddQuat = glm::angleAxis(-totalDelta * speed, localRotateAxis);

                glm::quat instRotQuat = glm::quat(instData.mRotation * DEGREES_TO_RADIANS);
                instRotQuat = localAddQuat * instRotQuat;
                glm::vec3 eulerAngles = glm::eulerAngles(instRotQuat) * RADIANS_TO_DEGREES;
                eulerAngles = EnforceEulerRange(eulerAngles);

                instData.mRotation = eulerAngles;
                instMesh->SetInstanceData(selInstance, instData);
            }
            else if (mTransformLocal)
            {
                for (uint32_t i = 0; i < transComps.size(); ++i)
                {
                    transComps[i]->AddWorldRotation(addQuat);
                }
            }
            else
            {
                glm::vec3 pivot = transComp->GetWorldPosition();

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

            if (instance)
            {
                // TODO: Move in world space
                instData.mScale += deltaScale3;
                instMesh->SetInstanceData(selInstance, instData);
            }
            else if (mTransformLocal)
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
                glm::vec3 pivot = transComp->GetWorldPosition();

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

                        glm::vec3 scaledPos = scaleMat * glm::vec4(transComps[i]->GetWorldPosition(), 1.0f);

                        glm::vec3 newScaleRot = newScale;

                        if (GetEditorState()->mTransformLock != TransformLock::None)
                        {
                            glm::quat absRot = transComps[i]->GetWorldRotationQuat();
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

                        transComps[i]->SetWorldPosition(scaledPos);
                        transComps[i]->SetScale(newScaleRot);
                    }
                }
            }
        }
    }

    if (IsControlDown() && IsKeyJustDown(KEY_T))
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

        if (instance)
        {
            ActionManager::Get()->EXE_SetInstanceData(instMesh, selInstance, { instData });
        }
        else
        {
            ActionManager::Get()->EXE_EditTransforms(transComps, newTransforms);
        }

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
    Camera3D* camera = GetWorld(0)->GetActiveCamera();
    glm::vec3 cameraPos = camera->GetWorldPosition();
    glm::vec3 right = camera->GetRightVector();
    glm::vec3 up = camera->GetUpVector();

    glm::vec2 deltaPosVS = HandleLockedCursor();
    glm::vec3 deltaPosWS = 0.002f * -(right * deltaPosVS.x - up * deltaPosVS.y);
    camera->SetWorldPosition(cameraPos + deltaPosWS);

    if (!IsMouseButtonDown(MOUSE_MIDDLE))
    {
        GetEditorState()->SetControlMode(ControlMode::Default);
    }
}

void Viewport3D::HandleOrbitControls()
{
    Camera3D* camera = GetWorld(0)->GetActiveCamera();
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
    int32_t selInstance = GetEditorState()->GetSelectedInstance();

    if (ShouldTransformInstance())
    {
        // Moving an instance
        InstancedMesh3D* instMesh = selNodes[0]->As<InstancedMesh3D>();

        if (selInstance >= 0 &&
            selInstance < (int32_t)instMesh->GetNumInstances())
        {
            mInstancePreTransform = instMesh->GetInstanceData(selInstance);
        }
    }
    else
    {
        for (uint32_t i = 0; i < selNodes.size(); ++i)
        {
            Node3D* transComp = (selNodes[i] && selNodes[i]->IsNode3D()) ? static_cast<Node3D*>(selNodes[i]) : nullptr;

            if (transComp)
            {
                mPreTransforms.push_back(transComp->GetTransform());
            }
        }
    }
}

void Viewport3D::RestorePreTransforms()
{
    const std::vector<Node*>& selNodes = GetEditorState()->GetSelectedNodes();
    int32_t selInstance = GetEditorState()->GetSelectedInstance();

    if (ShouldTransformInstance())
    {
        // Moving an instance
        InstancedMesh3D* instMesh = selNodes[0]->As<InstancedMesh3D>();

        if (selInstance >= 0 &&
            selInstance < (int32_t)instMesh->GetNumInstances())
        {
            instMesh->SetInstanceData(selInstance, mInstancePreTransform);
        }
    }
    else
    {
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

bool Viewport3D::ShouldTransformInstance() const
{
    return GetEditorState()->GetSelectedInstance() != -1 &&
        GetEditorState()->GetSelectedNodes().size() == 1 &&
        GetEditorState()->GetSelectedNodes()[0]->As<InstancedMesh3D>();
}

#endif
