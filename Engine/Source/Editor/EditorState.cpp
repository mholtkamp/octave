#if EDITOR

#include "EditorState.h"
#include "EditorConstants.h"
#include "ActionManager.h"
#include "Nodes/Node.h"
#include "Asset.h"
#include "AssetManager.h"
#include "Nodes/Node.h"
#include "Nodes/3D/Node3d.h"
#include "Nodes/3D/Camera3d.h"
#include "Nodes/Widgets/Widget.h"
#include "Nodes/Widgets/Text.h"
#include "Engine.h"
#include "Renderer.h"
#include "Grid.h"
#include "World.h"
#include "ScriptUtils.h"
#include "NetworkManager.h"
#include "AssetDir.h"
#include "TimerManager.h"
#include "AudioManager.h"
#include "Assets/Scene.h"
#include "EditorUtils.h"
#include "EditorImgui.h"
#include "Viewport3d.h"
#include "Viewport2d.h"
#include "PaintManager.h"
#include "Input/Input.h"

static EditorState sEditorState;

constexpr const char* kEditorProjectSaveFile = "EditorProject.sav";
constexpr const char* kEditorSaveFile = "Editor.sav";
constexpr int32_t kEditorProjectSaveVersion = 1;
constexpr int32_t kEditorSaveVersion = 1;

constexpr const uint32_t kMaxRecentProjects = 10;

EditorState* GetEditorState()
{
    return &sEditorState;
}

void EditorState::Init()
{
    mEditorCamera = Node::Construct<Camera3D>();
    mEditorCamera->SetName("Editor Camera");
    // TODO-NODE: This is a little sketchy because this will call World::RegisterNode(), but that's probably fine.
    mEditorCamera->SetWorld(GetWorld(0));

    mViewport3D = new Viewport3D();
    mViewport2D = new Viewport2D();
    mPaintManager = new PaintManager();

    mOverlayText = Node::Construct<Text>();
    mOverlayText->SetName("Overlay Text");
    mOverlayText->SetVisible(false);
    mOverlayText->SetAnchorMode(AnchorMode::TopRight);
    mOverlayText->SetHorizontalJustification(Justification::Right);
    mOverlayText->SetPosition(-200, 0);
    mOverlayText->SetDimensions(185, 50);
    mOverlayText->SetTextSize(50);

    ReadEditorSave();
}

void EditorState::Shutdown()
{
    if (IsPlayingInEditor())
    {
        EndPlayInEditor();
    }

    WriteEditorProjectSave();
    WriteEditorSave();

    Node::Destruct(mOverlayText);
    mOverlayText = nullptr;

    delete mViewport3D;
    mViewport3D = nullptr;

    delete mViewport2D;
    mViewport2D = nullptr;

    delete mPaintManager;
    mPaintManager = nullptr;

    mEditorCamera->SetWorld(nullptr);
    Node::Destruct(mEditorCamera);
    mEditorCamera = nullptr;
}

void EditorState::Update(float deltaTime)
{
    // TODO-NODE: Handle Widgets/2D? Maybe split modes to 3D and 2D
    if (!mPlayInEditor || mEjected)
    {
        switch (mMode)
        {
        case EditorMode::Scene:
            mViewport3D->Update(deltaTime);
            break;

        case EditorMode::Scene2D:
            mViewport2D->Update(deltaTime);
            break;

        case EditorMode::Scene3D:
            mViewport3D->Update(deltaTime);
            break;

        default:
            break;
        }
    }

    if (mPlayInEditor && !mEjected)
    {
        mViewportX = 0;
        mViewportY = 0;
        mViewportWidth = GetEngineState()->mWindowWidth;
        mViewportHeight = GetEngineState()->mWindowHeight;
    }
    else
    {
        EditorImguiGetViewport(mViewportX, mViewportY, mViewportWidth, mViewportHeight);
    }

    if (mPrevViewport.x != mViewportX ||
        mPrevViewport.y != mViewportY ||
        mPrevViewport.z != mViewportWidth ||
        mPrevViewport.w != mViewportHeight)
    {
        Renderer::Get()->DirtyAllWidgets();
    }

    mPrevViewport = { mViewportX, mViewportY, mViewportWidth, mViewportHeight };

    // Update overlay text.
    bool isPie = IsPlayingInEditor();
    bool isPaused = GetEditorState()->mPaused;
    bool isEjected = GetEditorState()->mEjected;

    if (isPaused)
    {
        mOverlayText->SetText("PAUSED");
        mOverlayText->SetColor({ 0.7f, 0.7f, 1.0f, 0.5 });
        mOverlayText->SetVisible(true);
        mOverlayText->Tick(deltaTime);
    }
    else if (isPie && isEjected)
    {
        mOverlayText->SetText("PLAYING");
        mOverlayText->SetColor({ 1.0f, 0.7f, 0.7f, 0.5 });
        mOverlayText->SetVisible(true);
        mOverlayText->Tick(deltaTime);
    }
    else
    {
        mOverlayText->SetVisible(false);
    }
}

void EditorState::GatherProperties(std::vector<Property>& props)
{

}

void EditorState::SetEditorMode(EditorMode mode)
{
    // Only allow scene editing in PIE for now.
    if (IsPlayingInEditor())
    {
        mode = EditorMode::Scene;
    }

    if (mMode != mode)
    {
        EditorMode prevMode = mMode;
        mMode = mode;

        static_assert(
            (int32_t)EditorMode::Scene == 0 && 
            (int32_t)EditorMode::Scene2D == 1 &&
            (int32_t)EditorMode::Scene3D == 2,
            "Update this check below.");

        // Only reset undo history when changing out of "scene editing".
        if (int32_t(prevMode) > int32_t(EditorMode::Scene3D) ||
            int32_t(mMode) > int32_t(EditorMode::Scene3D))
        {
            ActionManager::Get()->ResetUndoRedo();
        }

        if (mMode != EditorMode::Scene3D)
        {
            SetPaintMode(PaintMode::None);
        }

        if (mMode != EditorMode::Scene2D)
        {
            // Dirty all widgets since they are no longer based off of the wrapper widget
            GetWorld(0)->DirtyAllWidgets();
        }
    }
}

void EditorState::SetPaintMode(PaintMode paintMode)
{
    if (mPaintMode != paintMode)
    {
        mPaintMode = paintMode;

        if (mPaintMode == PaintMode::None)
        {
            // Make sure cursor is visible and unlocked 
            INP_ShowCursor(true);
            INP_LockCursor(false);
        }
    }
}

PaintMode EditorState::GetPaintMode()
{
    return mPaintMode;
}

EditorMode EditorState::GetEditorMode()
{
    return mMode;
}

void EditorState::ReadEditorSave()
{
    std::string saveFilePath = std::string("Engine/Saves/") + kEditorSaveFile;

    if (SYS_DoesFileExist(saveFilePath.c_str(), false))
    {
        Stream stream;
        stream.ReadFile(saveFilePath.c_str(), false);

        int32_t version = stream.ReadInt32();

        if (version == kEditorSaveVersion)
        {
            uint32_t numRecentProjects = stream.ReadUint32();
            mRecentProjects.clear();
            for (uint32_t i = 0; i < numRecentProjects; ++i)
            {
                std::string recentProj;
                stream.ReadString(recentProj);
                mRecentProjects.push_back(recentProj);
            }
        }
        else
        {
            SYS_RemoveFile(saveFilePath.c_str());
        }
    }
}

void EditorState::WriteEditorSave()
{
    Stream stream;
    stream.WriteInt32(kEditorSaveVersion);
    stream.WriteUint32((uint32_t)mRecentProjects.size());
    for (uint32_t i = 0; i < mRecentProjects.size(); ++i)
    {
        stream.WriteString(mRecentProjects[i]);
    }


    bool saveDirExists = DoesDirExist("Engine/Saves");

    if (!saveDirExists)
    {
        saveDirExists = SYS_CreateDirectory("Engine/Saves");
    }

    if (saveDirExists)
    {
        std::string saveFilePath = std::string("Engine/Saves/") + kEditorSaveFile;
        stream.WriteFile(saveFilePath.c_str());
    }
    else
    {
        LogError("Failed to create Engine Saves directory while writing Editor.sav");
    }
}

void EditorState::ReadEditorProjectSave()
{
    const std::string& projDir = GetEngineState()->mProjectDirectory;
    std::string saveFilePath = projDir + "Saves/" + kEditorProjectSaveFile;

    if (projDir != "" &&
        SYS_DoesFileExist(saveFilePath.c_str(), false))
    {
        // TODO: Save an ini file instead of a binary file so it can easily be
        // edited by a user, especially if something goes wrong.
        Stream stream;
        stream.ReadFile(saveFilePath.c_str(), false);

        int32_t version = stream.ReadInt32();

        if (version == kEditorProjectSaveVersion)
        {
            stream.ReadString(mStartupSceneName);

            uint32_t numFavDirs = stream.ReadUint32();
            mFavoritedDirs.clear();
            for (uint32_t i = 0; i < numFavDirs; ++i)
            {
                std::string favDir;
                stream.ReadString(favDir);
                mFavoritedDirs.push_back(favDir);
            }
        }
        else
        {
            SYS_RemoveFile(saveFilePath.c_str());
        }
    }
}

void EditorState::WriteEditorProjectSave()
{
    const std::string& projDir = GetEngineState()->mProjectDirectory;
    if (projDir != "")
    {
        Stream stream;
        stream.WriteInt32(kEditorProjectSaveVersion);
        stream.WriteString(mStartupSceneName);
        stream.WriteUint32((uint32_t)mFavoritedDirs.size());
        for (uint32_t i = 0; i < mFavoritedDirs.size(); ++i)
        {
            stream.WriteString(mFavoritedDirs[i]);
        }

        std::string projSavesDir = projDir + "Saves";
        bool saveDirExists = DoesDirExist(projSavesDir.c_str());

        if (!saveDirExists)
        {
            saveDirExists = SYS_CreateDirectory(projSavesDir.c_str());
        }

        if (saveDirExists)
        {
            std::string saveFilePath = projSavesDir + "/" + kEditorProjectSaveFile;
            stream.WriteFile(saveFilePath.c_str());
        }
        else
        {
            LogError("Failed to create Project Saves directory while writing EditorProject.sav");
        }
    }
}

void EditorState::HandleNodeDestroy(Node* node)
{
    DeselectNode(node);
    if (GetInspectedObject() == node)
    {
        InspectObject(nullptr, true, false);
    }

    if (mPaintManager)
    {
        mPaintManager->HandleNodeDestroy(node);
    }
}


void EditorState::SetSelectedNode(Node* newNode)
{
    if (newNode != nullptr && (newNode->GetWorld() == nullptr || newNode->IsForeign()))
    {
        return;
    }

    if (mSelectedNodes.size() != 1 ||
        mSelectedNodes[0] != newNode)
    {
        mSelectedNodes.clear();
        mSelectedInstance = -1;

        if (newNode != nullptr)
        {
            mSelectedNodes.push_back(newNode);

            if (!IsPlayingInEditor())
            {
                if (newNode->IsWidget())
                {
                    SetEditorMode(EditorMode::Scene2D);
                }
                else
                {
                    SetEditorMode(EditorMode::Scene3D);
                }
            }
        }

        if (!IsShuttingDown())
        {
            if (newNode || GetSelectedNode())
            {
                InspectObject(newNode);
            }

            ActionManager::Get()->OnSelectedNodeChanged();
        }
    }
}

void EditorState::AddSelectedNode(Node* node, bool addAllChildren)
{
    if (node != nullptr && !node->IsForeign())
    {
        if (addAllChildren)
        {
            for (uint32_t i = 0; i < node->GetNumChildren(); ++i)
            {
                AddSelectedNode(node->GetChild(i), true);
            }
        }

        std::vector<Node*>& nodes = mSelectedNodes;
        auto it = std::find(nodes.begin(), nodes.end(), node);

        if (it != nodes.end())
        {
            // Move the node to the back of the vector so that 
            // it is considered the primary selected node.
            nodes.erase(it);
        }

        nodes.push_back(node);
        InspectObject(node);

        mSelectedInstance = -1;
    }
}

void EditorState::RemoveSelectedNode(Node* node)
{
    if (node != nullptr)
    {
        std::vector<Node*>& nodes = mSelectedNodes;
        auto it = std::find(nodes.begin(), nodes.end(), node);

        if (it != nodes.end())
        {
            // Move the node to the back of the vector so that 
            // it is considered the primary selected node.
            nodes.erase(it);
        }

        if (nodes.size() > 0 && GetInspectedAsset() == nullptr)
        {
            InspectObject(nodes.back());
        }
        else if (GetInspectedNode() == node)
        {
            InspectObject(nullptr);
        }

        mSelectedInstance = -1;
    }
}

void EditorState::SetSelectedAssetStub(AssetStub* newStub)
{
    if (mSelectedAssetStub != newStub)
    {
        mSelectedAssetStub = newStub;
        if (newStub != nullptr &&
            newStub->mAsset == nullptr)
        {
            AssetManager::Get()->LoadAsset(*newStub);
        }
    }
}

void EditorState::SetControlMode(ControlMode newMode)
{
    // Don't do anything if new mode is same as current mode or there is no component selected.
    if (mControlMode == newMode)
    {
        return;
    }

    ControlMode prevMode = mControlMode;

    mControlMode = newMode;

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
        mMouseNeedsRecenter = true;
    }

    // Always reset transform lock when switching control modes.
    SetTransformLock(TransformLock::None);
}

void EditorState::BeginPlayInEditor()
{
    if (mPlayInEditor)
        return;

    SetSelectedNode(nullptr);
    SetSelectedAssetStub(nullptr);
    InspectObject(nullptr, true);

    ActionManager::Get()->ResetUndoRedo();

    // Save the current scene we want to play (and later restore)
    mPieEditSceneIdx = mEditSceneIndex;
    ShelveEditScene();

    // TODO-NODE: This is overkill since the root node of the scene should have been removed in ShelveEditScene()
    //   Maybe we just want to assert that the root node is null.
    GetWorld(0)->Clear();
    OCT_ASSERT(GetWorld(0)->GetRootNode() == nullptr);

    ShowEditorUi(false);
    Renderer::Get()->EnableProxyRendering(false);

    SetEditorMode(EditorMode::Scene);

    mPlayInEditor = true;
    mHasEjectedOnce = false;

    // Fake-Initialize the Game
    //OctPreInitialize();
    OctPostInitialize();

    EditScene* editScene = GetEditScene(mPieEditSceneIdx);
    if (editScene != nullptr &&
        editScene->mRootNode != nullptr)
    {
        Node* clonedRoot = editScene->mRootNode->Clone(true, false);
        GetWorld(0)->SetRootNode(clonedRoot);
    }
}

void EditorState::EndPlayInEditor()
{
    if (!mPlayInEditor)
        return;

    glm::mat4 cameraTransform(1);
    if (GetWorld(0)->GetActiveCamera())
    {
        cameraTransform = GetWorld(0)->GetActiveCamera()->GetTransform();
    }

    GetWorld(0)->DestroyRootNode();
    GetTimerManager()->ClearAllTimers();

    AudioManager::StopAllSounds();

    if (!NetIsLocal())
    {
        NetworkManager::Get()->Disconnect();
    }

    // Fake Shutdown
    OctPreShutdown();
    OctPostShutdown();

    SetSelectedNode(nullptr);
    SetSelectedAssetStub(nullptr);
    InspectObject(nullptr, true);

    ActionManager::Get()->ResetUndoRedo();

    ShowEditorUi(true);
    Renderer::Get()->EnableProxyRendering(true);

    mPlayInEditor = false;
    mEjected = false;
    mPaused = false;

    ScriptUtils::GarbageCollect();

    // Restore the scene we were working on
    OpenEditScene(mPieEditSceneIdx);

    if (GetWorld(0)->GetActiveCamera())
    {
        GetWorld(0)->GetActiveCamera()->SetTransform(cameraTransform);
    }
}

void EditorState::EjectPlayInEditor()
{
    if (mPlayInEditor &&
        !mEjected)
    {
        SetSelectedNode(nullptr);

        // Get the game's active camera. This should return the camera spawned in game, and not the editor camera,
        // as long as we get it here and not once mEjected has been set.
        Camera3D* activeCam = GetWorld(0)->GetActiveCamera();

        if (!mHasEjectedOnce && activeCam)
        {
            mEditorCamera->SetTransform(activeCam->GetTransform());
        }

        ShowEditorUi(true);
        mEjected = true;
        mHasEjectedOnce = true;
    }
}

void EditorState::InjectPlayInEditor()
{
    if (mPlayInEditor &&
        mEjected)
    {
        SetSelectedNode(nullptr);

        ShowEditorUi(false);
        mEjected = false;
    }
}

void EditorState::SetPlayInEditorPaused(bool paused)
{
    mPaused = paused;
}

bool EditorState::IsPlayInEditorPaused()
{
    return mPaused;
}

Camera3D* EditorState::GetEditorCamera()
{
    return mEditorCamera;
}

void EditorState::LoadStartupScene()
{
    if (mStartupSceneName != "")
    {
        Scene* scene = LoadAsset<Scene>(mStartupSceneName);

        if (scene != nullptr)
        {
            ActionManager::Get()->OpenScene(scene);
        }
    }
}

Node* EditorState::GetSelectedNode()
{
    return (mSelectedNodes.size() > 0) ?
        mSelectedNodes.back() :
        nullptr;
}

Widget* EditorState::GetSelectedWidget()
{
    Node* selNode = GetSelectedNode();
    Widget* selWidget = selNode ? selNode->As<Widget>() : nullptr;

    return selWidget;
}

const std::vector<Node*>& EditorState::GetSelectedNodes()
{
    return mSelectedNodes;
}

bool EditorState::IsNodeSelected(Node* node)
{
    for (uint32_t i = 0; i < mSelectedNodes.size(); ++i)
    {
        if (mSelectedNodes[i] == node)
        {
            return true;
        }
    }
    return false;
}

void EditorState::DeselectNode(Node* node)
{
    bool erased = false;
    for (uint32_t i = 0; i < mSelectedNodes.size(); ++i)
    {
        if (mSelectedNodes[i] == node)
        {
            mSelectedNodes.erase(mSelectedNodes.begin() + i);
            erased = true;
            break;
        }
    }

    if (erased && !IsShuttingDown())
    {
        ActionManager::Get()->OnSelectedNodeChanged();
    }
}

int32_t EditorState::GetSelectedInstance()
{
    return mSelectedInstance;
}

void EditorState::SetSelectedInstance(int32_t instance)
{
    mSelectedInstance = instance;
}

void CacheEditSceneLinkedProps(EditScene& editScene)
{
    editScene.mLinkedSceneProps.clear();
    auto saveSceneLinkedProps = [&](Node* node) -> bool
    {
        if (node->IsSceneLinked())
        {
            Scene* linkedScene = node->GetScene();
            if (linkedScene != nullptr)
            {
                editScene.mLinkedSceneProps.push_back(LinkedSceneProps());
                LinkedSceneProps& nonDefProps = editScene.mLinkedSceneProps.back();
                nonDefProps.mNode = node;
                GatherNonDefaultProperties(node, nonDefProps.mProps);
            }

            return false;
        }

        return true;
    };

    if (editScene.mRootNode != nullptr)
    {
        editScene.mRootNode->Traverse(saveSceneLinkedProps);
    }
}

void EditorState::OpenEditScene(Scene* scene)
{
    int32_t editSceneIdx = -1;
    EditScene* editScene = nullptr;

    // Allow opening multiple null scenes.
    if (scene != nullptr)
    {
        for (uint32_t i = 0; i < mEditScenes.size(); ++i)
        {
            if (mEditScenes[i].mSceneAsset == scene)
            {
                editScene = &mEditScenes[i];
                editSceneIdx = (int32_t)i;
                break;
            }
        }
    }

    if (editScene == nullptr)
    {
        // The scene isn't open yet,
        mEditScenes.push_back(EditScene());
        EditScene& newEditScene = mEditScenes.back();
        newEditScene.mSceneAsset = scene;
        if (scene != nullptr)
        {
            newEditScene.mRootNode = scene->Instantiate();
        }

        newEditScene.mCameraTransform = glm::translate(glm::vec3(0.0f, 2.5f, 10.0f));

        editScene = &newEditScene;
        editSceneIdx = int32_t(mEditScenes.size()) - 1;

        CacheEditSceneLinkedProps(*editScene);
    }

    OCT_ASSERT(editScene != nullptr);
    OCT_ASSERT(editSceneIdx != -1);

    OpenEditScene(editSceneIdx);
}

void EditorState::OpenEditScene(int32_t idx)
{
    // Lock scene open/close during PIE
    if (mPlayInEditor)
        return;

    // Shelve whatever we are working on.
    ShelveEditScene();
    
    if (GetWorld(0)->GetRootNode() != nullptr)
    {
        LogWarning("Destroying nodes without associated EditScene.");
        GetWorld(0)->DestroyRootNode();
    }

    if (idx >= 0 && idx < int32_t(mEditScenes.size()))
    {
        const EditScene& editScene = mEditScenes[idx];
        mEditSceneIndex = idx;
        GetWorld(0)->SetRootNode(editScene.mRootNode); // could be nullptr.
        GetEditorCamera()->SetTransform(editScene.mCameraTransform);

        // Reinstantiate scene-linked nodes
        auto respawnSceneLinks = [&](Node* node) -> bool
        {
            if (node->IsSceneLinked())
            {
                const std::vector<Property>* nonDefProps = nullptr;
                for (uint32_t i = 0; i < editScene.mLinkedSceneProps.size(); ++i)
                {
                    if (editScene.mLinkedSceneProps[i].mNode == node)
                    {
                        nonDefProps = &editScene.mLinkedSceneProps[i].mProps;
                        break;
                    }
                }

                // Replace this scene-linked node with a newly instantiated version
                // to make sure we grab new changes that have been made to the scene.
                Node* newNode = node->GetScene()->Instantiate(); // node->Clone(true);
                Node* parent = node->GetParent();
                int32_t nodeIdx = parent->FindChildIndex(node);

                parent->RemoveChild(node);
                parent->AddChild(newNode, nodeIdx);

                // Copy non-default props.
                if (nonDefProps != nullptr)
                {
                    std::vector<Property> dstProps;
                    newNode->GatherProperties(dstProps);
                    CopyPropertyValues(dstProps, *nonDefProps);
                }

                Node::Destruct(node);
                node = nullptr;
            }

            return true;
        };

        if (editScene.mRootNode != nullptr)
        {
            editScene.mRootNode->Traverse(respawnSceneLinks);

            if (editScene.mRootNode->IsNode3D())
                GetEditorState()->SetEditorMode(EditorMode::Scene3D);
            else if (editScene.mRootNode->IsWidget())
                GetEditorState()->SetEditorMode(EditorMode::Scene2D);
        }

        ActionManager::Get()->ResetUndoRedo();
    }
}

void EditorState::CloseEditScene(int32_t idx)
{
    // Lock scene open/close during PIE
    if (mPlayInEditor)
        return;

    if (idx >= 0 && idx < int32_t(mEditScenes.size()))
    {
        if (idx == mEditSceneIndex)
        {
            // Is this the active EditScene? If so, shelve it first.
            ShelveEditScene();
        }

        // Destroy the root node
        Node::Destruct(mEditScenes[idx].mRootNode);

        // Remove this EditScene entry.
        mEditScenes.erase(mEditScenes.begin() + idx);

        // If we removed an editscene below the active editscene, we need to adjust the active editscene index.
        if (idx < mEditSceneIndex)
        {
            --mEditSceneIndex;
        }

        // If that was the active edit scene, then load the next one it's place.
        if (mEditSceneIndex == -1 && 
            mEditScenes.size() > 0)
        {
            if (idx >= int32_t(mEditScenes.size()))
            {
                idx = int32_t(mEditScenes.size() - 1);
            }

            OpenEditScene(idx);
        }

#if 0
        // Do we want to always have a scene open?
        if (mEditScenes.size() == 0)
        {
            // Open a blank edit scene.
            OpenEditScene(nullptr);
        }
#endif
    }
}

//void EditorState::OpenDefaultEditScene()
//{
//    // The scene isn't open yet,
//    mEditScenes.push_back(EditScene());
//    EditScene& newEditScene = mEditScenes.back();
//    newEditScene.mCameraTransform = glm::mat4(1);
//
//    OpenEditScene(int32_t(mEditScenes.size()) - 1);
//}

void EditorState::ShelveEditScene()
{
    if (mEditSceneIndex >= 0)
    {
        EditScene& editScene = mEditScenes[mEditSceneIndex];
        editScene.mRootNode = GetWorld(0)->GetRootNode();
        editScene.mCameraTransform = GetEditorCamera()->GetTransform();
        GetWorld(0)->SetRootNode(nullptr);

        CacheEditSceneLinkedProps(editScene);

        mEditSceneIndex = -1;

        ActionManager::Get()->ResetUndoRedo();
    }
}

EditScene* EditorState::GetEditScene(int32_t idx)
{
    EditScene* ret = nullptr;

    if (idx == -1)
    {
        // -1 means the current edit scene index.
        idx = mEditSceneIndex;
    }

    if (idx >= 0 &&
        idx < int32_t(mEditScenes.size()))
    {
        ret = &mEditScenes[idx];
    }

    return ret;
}

void EditorState::CloseAllEditScenes()
{
    while (mEditScenes.size() > 0)
    {
        CloseEditScene(0);
    }
}

void EditorState::EnsureActiveScene()
{
    if (!IsPlayingInEditor() && 
        GetEditorState()->GetEditScene() == nullptr)
    {
        // Save whatever is in the world and move it to the new scene.
        // For instance, the user has no open editscene, but spawns a Node3D.
        Node* curWorldRoot = GetWorld(0)->GetRootNode();
        GetWorld(0)->SetRootNode(nullptr);

        GetEditorState()->OpenEditScene(nullptr);

        GetWorld(0)->SetRootNode(curWorldRoot);
    }
}

void EditorState::ShowEditorUi(bool show)
{
    mUiEnabled = show;
}

Asset* EditorState::GetSelectedAsset()
{
    return  mSelectedAssetStub ? mSelectedAssetStub->mAsset : nullptr;
}

AssetStub* EditorState::GetSelectedAssetStub()
{
    return mSelectedAssetStub;
}

ControlMode EditorState::GetControlMode()
{
    return mControlMode;
}

glm::vec3 EditorState::GetTransformLockVector(TransformLock lock)
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

void EditorState::SetTransformLock(TransformLock lock)
{
    static Line lineX = Line({ 0,0,0 }, { 10, 0, 0 }, { 1.0f, 0.4f, 0.4f, 1.0f }, -1.0f);
    static Line lineY = Line({ 0,0,0 }, { 0, 10, 0 }, { 0.4f, 1.0f ,0.4f, 1.0f }, -1.0f);
    static Line lineZ = Line({ 0,0,0 }, { 0, 0, 10 }, { 0.4f, 0.4f, 1.0f, 1.0f }, -1.0f);

    mTransformLock = lock;

    World* world = GetWorld(0);

    if (world != nullptr)
    {
        world->RemoveLine(lineX);
        world->RemoveLine(lineY);
        world->RemoveLine(lineZ);

        Node* node = GetSelectedNode();
        if (node != nullptr && node->IsNode3D())
        {
            glm::vec3 pos = static_cast<Node3D*>(node)->GetWorldPosition();

            if (node->As<InstancedMesh3D>() &&
                mSelectedInstance != -1)
            {
                InstancedMesh3D* instMesh = node->As<InstancedMesh3D>();
                if (mSelectedInstance >= 0 &&
                    mSelectedInstance < (int32_t)instMesh->GetNumInstances())
                {
                    MeshInstanceData instData = instMesh->GetInstanceData(mSelectedInstance);
                    glm::mat4 instTransform = MakeTransform(instData.mPosition, instData.mRotation, instData.mScale);
                    glm::mat4 totalTransform = instMesh->GetTransform() * instTransform;
                    pos = totalTransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
                }
            }

            lineX.mStart = pos - glm::vec3(10000, 0, 0);
            lineY.mStart = pos - glm::vec3(0, 10000, 0);
            lineZ.mStart = pos - glm::vec3(0, 0, 10000);
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

RTTI* EditorState::GetInspectedObject()
{
    return mInspectedObject;
}

Node* EditorState::GetInspectedNode()
{
    return mInspectedObject ? mInspectedObject->As<Node>() : nullptr;
}

Asset* EditorState::GetInspectedAsset()
{
    return mInspectedObject ? mInspectedObject->As<Asset>() : nullptr;
}

void EditorState::InspectObject(RTTI* obj, bool force, bool recordHistory)
{
    if (force || !mInspectLocked)
    {
        mInspectedObject = obj;
        mInspectedAsset = obj ? obj->As<Asset>() : nullptr;
        mInspectLocked = false;
    }

    if (recordHistory)
    {
        RecordInspectHistory();
    }
}

void EditorState::LockInspect(bool lock)
{
    // Don't allow locking inspection if nothing is inspected.
    lock = (mInspectedObject != nullptr) ? lock : false;

    mInspectLocked = lock;
}

bool EditorState::IsInspectLocked()
{
    return mInspectLocked;
}

void EditorState::RecordInspectHistory()
{
    if (mPrevInspectedObject != mInspectedObject)
    {
        if (mPrevInspectedObject != nullptr)
        {
            mInspectPast.push_back(mPrevInspectedObject);
            mInspectFuture.clear();
        }

        mPrevInspectedObject = mInspectedObject;
    }
}

void EditorState::ClearInspectHistory()
{
    mInspectPast.clear();
    mInspectFuture.clear();
    mPrevInspectedObject = nullptr;
    mInspectedObject = nullptr;
}

void EditorState::ProgressInspectFuture()
{
    if (mInspectFuture.size() > 0)
    {
        if (mInspectedObject != nullptr)
        {
            mInspectPast.push_back(mInspectedObject);
        }

        RTTI* futureObj = mInspectFuture.back();
        OCT_ASSERT(futureObj != nullptr);
        mInspectFuture.pop_back();

        // Should we prevent moving through history (at a higher level) if the object properties are locked?
        InspectObject(futureObj, true, false);

        mPrevInspectedObject = mInspectedObject;
    }
}

void EditorState::RegressInspectPast()
{
    if (mInspectPast.size() > 0)
    {
        if (mInspectedObject != nullptr)
        {
            mInspectFuture.push_back(mInspectedObject);
        }

        RTTI* pastObj = mInspectPast.back();
        OCT_ASSERT(pastObj != nullptr);
        mInspectPast.pop_back();

        // Should we prevent moving through history (at a higher level) if the object properties are locked?
        InspectObject(pastObj, true, false);

        mPrevInspectedObject = mInspectedObject;
    }
}

void EditorState::ClearAssetDirHistory()
{
    mDirPast.clear();
    mDirFuture.clear();
}

void EditorState::SetAssetDirectory(AssetDir* assetDir, bool recordHistory)
{
    if (mCurrentDir != assetDir)
    {
        if (recordHistory && mCurrentDir != nullptr)
        {
            mDirPast.push_back(mCurrentDir);
            mDirFuture.clear();
        }

        mCurrentDir = assetDir;

        if (mCurrentDir != nullptr)
        {
            mCurrentDir->SortChildrenAlphabetically();
        }

        GetEditorState()->SetSelectedAssetStub(nullptr);
    }
}

AssetDir* EditorState::GetAssetDirectory()
{
    return mCurrentDir;
}

void EditorState::BrowseToAsset(const std::string& name)
{
    AssetStub* stub = AssetManager::Get()->GetAssetStub(name);

    if (stub != nullptr)
    {
        SetAssetDirectory(stub->mDirectory, true);
        SetSelectedAssetStub(stub);

        const std::vector<AssetDir*>& dirs = mCurrentDir->mChildDirs;
        const std::vector<AssetStub*>& assets = mCurrentDir->mAssetStubs;
        const int32_t parentDirCount = (mCurrentDir->mParentDir != nullptr) ? 1 : 0;
        const int32_t numDirs = int32_t(dirs.size());
        const int32_t numAssets = int32_t(assets.size());

        mTrackSelectedAsset = true;
    }
}

void EditorState::CaptureAndSaveScene(AssetStub* stub, Node* rootNode)
{
    if (stub == nullptr)
    {
        stub = EditorAddUniqueAsset("SC_Scene", mCurrentDir, Scene::GetStaticType(), true);
    }

    if (stub->mAsset == nullptr)
    {
        AssetManager::Get()->LoadAsset(*stub);
    }

    if (rootNode == nullptr)
    {
        rootNode = GetWorld(0)->GetRootNode();
    }

    if (rootNode == nullptr)
    {
        rootNode = GetWorld(0)->SpawnNode<Node>();
        rootNode->SetName(stub ? stub->mName : "Root");
    }

    bool worldRoot = (rootNode == GetWorld(0)->GetRootNode());

    Scene* scene = (Scene*)stub->mAsset;
    scene->Capture(rootNode);
    rootNode->SetScene(scene);

    if (worldRoot)
    {
        EditScene* editScene = GetEditScene();
        if (editScene)
        {
            editScene->mSceneAsset = scene;
        }
    }

    AssetManager::Get()->SaveAsset(*stub);
}

void EditorState::DuplicateAsset(AssetStub* srcStub)
{
    if (srcStub != nullptr)
    {
        Asset* srcAsset = nullptr;
        if (srcStub->mAsset == nullptr)
        {
            AssetManager::Get()->LoadAsset(*srcStub);
        }

        srcAsset = srcStub->mAsset;

        if (srcAsset != nullptr)
        {
            AssetStub* stub = EditorAddUniqueAsset(srcAsset->GetName().c_str(), mCurrentDir, srcAsset->GetType(), false);

            if (stub != nullptr)
            {
                stub->mAsset->Copy(srcAsset);
                stub->mAsset->SetName(stub->mName);
                stub->mAsset->Create();
                AssetManager::Get()->SaveAsset(*stub);
            }
        }
    }
}

void EditorState::ProgressDirFuture()
{
    if (mDirFuture.size() > 0)
    {
        if (mCurrentDir != nullptr)
        {
            mDirPast.push_back(mCurrentDir);
        }

        AssetDir* dir = mDirFuture.back();
        OCT_ASSERT(dir);
        mDirFuture.pop_back();

        SetAssetDirectory(dir, false);
    }
}

void EditorState::RegressDirPast()
{
    if (mDirPast.size() > 0)
    {
        if (mCurrentDir != nullptr)
        {
            // Record the current dir.
            mDirFuture.push_back(mCurrentDir);
        }

        AssetDir* dir = mDirPast.back();
        OCT_ASSERT(dir);
        mDirPast.pop_back();

        SetAssetDirectory(dir, false);
    }
}

void EditorState::RemoveFilteredAssetStub(AssetStub* stub)
{
    for (int32_t i = int32_t(mFilteredAssetStubs.size()) - 1; i >= 0; --i)
    {
        if (mFilteredAssetStubs[i] == stub)
        {
            mFilteredAssetStubs.erase(mFilteredAssetStubs.begin() + i);
            break;
        }
    }
}

Viewport3D* EditorState::GetViewport3D()
{
    return mViewport3D;
}

Viewport2D* EditorState::GetViewport2D()
{
    return mViewport2D;
}

bool EditorState::IsDirFavorited(const std::string& dirPath)
{
    bool favorited = false;

    for (uint32_t i = 0; i < mFavoritedDirs.size(); ++i)
    {
        if (mFavoritedDirs[i] == dirPath)
        {
            favorited = true;
        }
    }

    return favorited;
}

void EditorState::AddFavoriteDir(const std::string& dirPath)
{
    if (std::find(mFavoritedDirs.begin(), mFavoritedDirs.end(), dirPath) == mFavoritedDirs.end())
    {
        mFavoritedDirs.push_back(dirPath);
    }
}

void EditorState::RemoveFavoriteDir(const std::string& dirPath)
{
    auto it = std::find(mFavoritedDirs.begin(), mFavoritedDirs.end(), dirPath);
    if (it != mFavoritedDirs.end())
    {
        mFavoritedDirs.erase(it);
    }
}

void EditorState::AddRecentProject(const std::string& projPath)
{
    std::string fullPath = SYS_GetAbsolutePath(projPath);

    // Remove if already in the recent project list.
    for (uint32_t i = 0; i < mRecentProjects.size(); ++i)
    {
        if (mRecentProjects[i] == fullPath)
        {
            mRecentProjects.erase(mRecentProjects.begin() + i);
            break;
        }
    }

    // If recent project list size >= kMaxRecentProjects, pop_back()
    while (mRecentProjects.size() >= kMaxRecentProjects)
    {
        mRecentProjects.pop_back();
    }

    // Insert this project at the front (most-recent).
    mRecentProjects.insert(mRecentProjects.begin(), fullPath);
}


#endif