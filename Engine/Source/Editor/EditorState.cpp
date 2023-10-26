#if EDITOR

#include "EditorState.h"
#include "EditorConstants.h"
#include "PanelManager.h"
#include "ActionManager.h"
#include "Nodes/Node.h"
#include "Asset.h"
#include "AssetManager.h"
#include "Nodes/Node.h"
#include "Nodes/3D/Node3d.h"
#include "Engine.h"
#include "Grid.h"
#include "World.h"
#include "TimerManager.h"
#include "AudioManager.h"
#include "Assets/Scene.h"
#include "EditorUtils.h"
#include "Widgets/ActionList.h"
#include "Widgets/TextEntry.h"
#include "Widgets/WidgetViewportPanel.h"
#include "Widgets/PropertiesPanel.h"
#include "Input/Input.h"

static EditorState sEditorState;

constexpr const char* kEditorSaveFile = "Editor.sav";
constexpr int32_t kEditorSaveVersion = 1;

void SetEditorMode(EditorMode mode)
{
    // Only allow scene editing in PIE for now.
    if (IsPlayingInEditor())
    {
        mode = EditorMode::Scene;
    }

    if (sEditorState.mMode != mode)
    {
        EditorMode prevMode = sEditorState.mMode;
        sEditorState.mMode = mode;

        // TODO-NODE: I don't think we need this anymore. Remove commented call after verifying.
        //SetSelectedNode(nullptr);

        PanelManager::Get()->OnEditorModeChanged();

        ActionManager::Get()->ResetUndoRedo();
    }
}

EditorMode GetEditorMode()
{
    return sEditorState.mMode;
}

void InitializeEditorState()
{

}

void DestroyEditorState()
{

}

EditorState* GetEditorState()
{
    return &sEditorState;
}

void ReadEditorSave()
{
    if (SYS_DoesSaveExist(kEditorSaveFile))
    {
        // TODO: Save an ini file instead of a binary file so it can easily be
        // edited by a user, especially if something goes wrong.
        Stream stream;
        SYS_ReadSave(kEditorSaveFile, stream);

        int32_t version = stream.ReadInt32();

        if (version == kEditorSaveVersion)
        {
            stream.ReadString(sEditorState.mStartupSceneName);
        }
        else
        {
            SYS_DeleteSave(kEditorSaveFile);
        }
    }
}

void WriteEditorSave()
{
    Stream stream;
    stream.WriteInt32(kEditorSaveVersion);
    stream.WriteString(sEditorState.mStartupSceneName);

    SYS_WriteSave(kEditorSaveFile, stream);
}

void SetSelectedNode(Node* newNode)
{
    // Check if the component is actually exiled (only exists in the undo history).
    if (newNode != nullptr && newNode->GetWorld() == nullptr)
    {
        return;
    }

    if (sEditorState.mSelectedNodes.size() != 1 ||
        sEditorState.mSelectedNodes[0] != newNode)
    {
        sEditorState.mSelectedNodes.clear();

        if (newNode != nullptr)
        {
            sEditorState.mSelectedNodes.push_back(newNode);
        }

        if (!IsShuttingDown())
        {
            PanelManager::Get()->OnSelectedNodeChanged();
            ActionManager::Get()->OnSelectedNodeChanged();
        }
    }
}

void AddSelectedNode(Node* node, bool addAllChildren)
{
    if (node != nullptr)
    {
        if (addAllChildren)
        {
            for (uint32_t i = 0; i < node->GetNumChildren(); ++i)
            {
                AddSelectedNode(node->GetChild(i), true);
            }
        }

        std::vector<Node*>& nodes = sEditorState.mSelectedNodes;
        auto it = std::find(nodes.begin(), nodes.end(), node);

        if (it != nodes.end())
        {
            // Move the node to the back of the vector so that 
            // it is considered the primary selected node.
            nodes.erase(it);
        }

        nodes.push_back(node);
    }
}

void RemoveSelectedNode(Node* node)
{
    if (node != nullptr)
    {
        std::vector<Node*>& nodes = sEditorState.mSelectedNodes;
        auto it = std::find(nodes.begin(), nodes.end(), node);

        if (it != nodes.end())
        {
            // Move the node to the back of the vector so that 
            // it is considered the primary selected node.
            nodes.erase(it);
        }
    }
}

void SetSelectedAssetStub(AssetStub* newStub)
{
    if (sEditorState.mSelectedAssetStub != newStub)
    {
        sEditorState.mSelectedAssetStub = newStub;
        if (newStub != nullptr &&
            newStub->mAsset == nullptr)
        {
            AssetManager::Get()->LoadAsset(*newStub);
        }

        PanelManager::Get()->OnSelectedAssetChanged();
    }
}

void SetControlMode(ControlMode newMode)
{
    // Don't do anything if new mode is same as current mode or there is no component selected.
    if (sEditorState.mControlMode == newMode)
    {
        return;
    }

    ControlMode prevMode = sEditorState.mControlMode;

    sEditorState.mControlMode = newMode;

    if (prevMode == ControlMode::Pilot ||
        prevMode == ControlMode::Translate ||
        prevMode == ControlMode::Rotate ||
        prevMode == ControlMode::Scale ||
        prevMode == ControlMode::Pan ||
        prevMode == ControlMode::Orbit)
    {
        INP_ShowCursor(true);
        INP_LockCursor(false);
    }

    if (newMode == ControlMode::Pilot ||
        newMode == ControlMode::Translate ||
        newMode == ControlMode::Rotate ||
        newMode == ControlMode::Scale ||
        newMode == ControlMode::Pan ||
        newMode == ControlMode::Orbit)
    {
        INP_ShowCursor(false);
        INP_LockCursor(true);

        // But because of the event loop processing, we might get a bogus mouse motion event even after
        // we have just forced the position. So set a flag to let the viewport panel know that we need to
        // recenter the mouse next frame.
        sEditorState.mMouseNeedsRecenter = true;
    }

    // Always reset transform lock when switching control modes.
    SetTransformLock(TransformLock::None);
}

void BeginPlayInEditor()
{
    if (sEditorState.mPlayInEditor)
        return;

    SetSelectedNode(nullptr);
    SetSelectedAssetStub(nullptr);
    PanelManager::Get()->GetPropertiesPanel()->InspectAsset(nullptr);

    ActionManager::Get()->ResetUndoRedo();

    // Save the current scene we want to play (and later restore)
    ShelveEditScene();

    // TODO-NODE: This is overkill since the root node of the scene should have been removed in ShelveEditScene()
    //   Maybe we just want to assert that the root node is null.
    GetWorld()->Clear();
    OCT_ASSERT(GetWorld()->GetRootNode() == nullptr);

    ShowEditorUi(false);
    Renderer::Get()->EnableProxyRendering(false);

    sEditorState.mPlayInEditor = true;

    // Fake-Initialize the Game
    //OctPreInitialize();
    OctPostInitialize();

    EditScene* editScene = GetEditScene();
    if (editScene != nullptr &&
        editScene->mRootNode != nullptr)
    {
        Node* clonedRoot = editScene->mRootNode->Clone(true, false);
        GetWorld()->SetRootNode(clonedRoot);
    }
}

void EndPlayInEditor()
{
    if (!sEditorState.mPlayInEditor)
        return;

    glm::mat4 cameraTransform(1);
    if (GetWorld()->GetActiveCamera())
    {
        cameraTransform = GetWorld()->GetActiveCamera()->GetTransform();
    }

    GetWorld()->DestroyRootNode();
    GetTimerManager()->ClearAllTimers();

    AudioManager::StopAllSounds();

    // Fake Shutdown
    OctPreShutdown();
    OctPostShutdown();

    SetSelectedNode(nullptr);
    SetSelectedAssetStub(nullptr);
    PanelManager::Get()->GetPropertiesPanel()->InspectAsset(nullptr);

    ActionManager::Get()->ResetUndoRedo();

    ShowEditorUi(true);
    Renderer::Get()->EnableProxyRendering(true);

    sEditorState.mPlayInEditor = false;
    sEditorState.mEjected = false;
    sEditorState.mPaused = false;

    // Restore the scene we were working on
    EditScene* editScene = GetEditScene();
    if (editScene != nullptr)
    {
        GetWorld()->SetRootNode(editScene->mRootNode);
    }

    if (GetWorld()->GetActiveCamera())
    {
        GetWorld()->GetActiveCamera()->SetTransform(cameraTransform);
    }
}

void EjectPlayInEditor()
{
    if (sEditorState.mPlayInEditor &&
        !sEditorState.mEjected)
    {
        SetSelectedNode(nullptr);
        sEditorState.mInjectedCamera = GetWorld()->GetActiveCamera();

        if (sEditorState.mEjectedCamera == nullptr)
        {
            Camera3D* ejectedCamera = GetWorld()->SpawnNode<Camera3D>();
            ejectedCamera->SetName("Ejected Camera");
            sEditorState.mEjectedCamera = ejectedCamera;

            // Set its transform to match the PIE camera
            if (GetWorld()->GetActiveCamera())
            {
                ejectedCamera->SetTransform(GetWorld()->GetActiveCamera()->GetTransform());
            }
        }

        GetWorld()->SetActiveCamera(sEditorState.mEjectedCamera.Get<Camera3D>());
        ShowEditorUi(true);
        sEditorState.mEjected = true;
    }
}

void InjectPlayInEditor()
{
    if (sEditorState.mPlayInEditor &&
        sEditorState.mEjected)
    {
        SetSelectedNode(nullptr);

        if (sEditorState.mInjectedCamera != nullptr)
        {
            GetWorld()->SetActiveCamera(sEditorState.mInjectedCamera.Get<Camera3D>());
        }

        ShowEditorUi(false);
        sEditorState.mEjected = false;
    }
}

void SetPlayInEditorPaused(bool paused)
{
    sEditorState.mPaused = paused;
}

bool IsPlayInEditorPaused()
{
    return sEditorState.mPaused;
}

void LoadStartupScene()
{
    if (sEditorState.mStartupSceneName != "")
    {
        Scene* scene = LoadAsset<Scene>(sEditorState.mStartupSceneName);

        if (scene != nullptr)
        {
            ActionManager::Get()->OpenScene(scene);
        }
    }
}

Node* GetSelectedNode()
{
    return (sEditorState.mSelectedNodes.size() > 0) ?
        sEditorState.mSelectedNodes.back() :
        nullptr;
}

const std::vector<Node*>& GetSelectedNodes()
{
    return sEditorState.mSelectedNodes;
}

bool IsNodeSelected(Node* node)
{
    for (uint32_t i = 0; i < sEditorState.mSelectedNodes.size(); ++i)
    {
        if (sEditorState.mSelectedNodes[i] == node)
        {
            return true;
        }
    }
    return false;
}

void DeselectNode(Node* node)
{
    bool erased = false;
    for (uint32_t i = 0; i < sEditorState.mSelectedNodes.size(); ++i)
    {
        if (sEditorState.mSelectedNodes[i] == node)
        {
            sEditorState.mSelectedNodes.erase(sEditorState.mSelectedNodes.begin() + i);
            erased = true;
            break;
        }
    }

    if (erased && !IsShuttingDown())
    {
        PanelManager::Get()->OnSelectedNodeChanged();
        ActionManager::Get()->OnSelectedNodeChanged();
    }
}

void OpenEditScene(Scene* scene)
{

}

void CloseEditScene(Scene* scene)
{

}

void ShelveEditScene()
{

}

EditScene* GetEditScene()
{

}

void ShowEditorUi(bool show)
{
    sEditorState.mUiEnabled = show;
}

Asset* GetSelectedAsset()
{
    return  sEditorState.mSelectedAssetStub ? sEditorState.mSelectedAssetStub->mAsset : nullptr;
}

AssetStub* GetSelectedAssetStub()
{
    return sEditorState.mSelectedAssetStub;
}

ControlMode GetControlMode()
{
    return sEditorState.mControlMode;
}

glm::vec3 GetTransformLockVector(TransformLock lock)
{
    glm::vec3 ret = glm::vec3(1.0, 1.0, 1.0);

    switch (lock)
    {
    case TransformLock::AxisX: ret = glm::vec3(1.0f, 0.0f, 0.0f); break;
    case TransformLock::AxisY: ret = glm::vec3(0.0f, 1.0f, 0.0f); break;
    case TransformLock::AxisZ: ret = glm::vec3(0.0f, 0.0f, 1.0f); break;
    case TransformLock::PlaneYZ: ret = glm::vec3(0.0f, 1.0f, 1.0f); break;
    case TransformLock::PlaneXZ: ret = glm::vec3(1.0f, 0.0f, 1.0f); break;
    case TransformLock::PlaneXY: ret = glm::vec3(1.0f, 1.0f, 0.0f); break;
    default: ret = glm::vec3(1.0, 1.0, 1.0); break;
    }

    return ret;
}

void SetTransformLock(TransformLock lock)
{
    static Line lineX = Line({ 0,0,0 }, { 10, 0, 0 }, { 1.0f, 0.4f, 0.4f, 1.0f }, -1.0f);
    static Line lineY = Line({ 0,0,0 }, { 0, 10, 0 }, { 0.4f, 1.0f ,0.4f, 1.0f }, -1.0f);
    static Line lineZ = Line({ 0,0,0 }, { 0, 0, 10 }, { 0.4f, 0.4f, 1.0f, 1.0f }, -1.0f);

    sEditorState.mTransformLock = lock;

    World* world = GetWorld();

    if (world != nullptr)
    {
        world->RemoveLine(lineX);
        world->RemoveLine(lineY);
        world->RemoveLine(lineZ);

        Node* node = GetSelectedNode();
        if (node != nullptr && node->IsNode3D())
        {
            glm::vec3 pos = static_cast<Node3D*>(node)->GetAbsolutePosition();
            lineX.mStart = pos - glm::vec3(10000, 0, 0);;
            lineY.mStart = pos - glm::vec3(0, 10000, 0);;
            lineZ.mStart = pos - glm::vec3(0, 0, 10000);;
            lineX.mEnd = pos + glm::vec3(10000,0,0);
            lineY.mEnd = pos + glm::vec3(0, 10000, 0);
            lineZ.mEnd = pos + glm::vec3(0, 0, 10000);

            switch (lock)
            {
            case TransformLock::AxisX:
                world->AddLine(lineX);
                break;
            case TransformLock::AxisY:
                world->AddLine(lineY);
                break;
            case TransformLock::AxisZ:
                world->AddLine(lineZ);
                break;
            case TransformLock::PlaneYZ:
                world->AddLine(lineY);
                world->AddLine(lineZ);
                break;
            case TransformLock::PlaneXZ:
                world->AddLine(lineX);
                world->AddLine(lineZ);
                break;
            case TransformLock::PlaneXY:
                world->AddLine(lineX);
                world->AddLine(lineY);
                break;
            default:
                break;
            }

        }
    }
}

#endif