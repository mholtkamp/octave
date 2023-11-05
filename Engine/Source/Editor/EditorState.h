#pragma once

#if EDITOR

#include <string>
#include "Maths.h"
#include "ObjectRef.h"

#include "imgui.h"

class Node;
class Widget;
class Asset;
struct AssetStub;
class ActionList;
class Canvas;
class Camera3D;
class Viewport3D;
class Viewport2D;

enum class ControlMode
{
    Default,
    Pilot,
    Translate,
    Rotate,
    Scale,
    Pan,
    Orbit
};

enum class TransformLock
{
    None,
    AxisX,
    AxisY,
    AxisZ,
    PlaneYZ,
    PlaneXZ,
    PlaneXY,
    Count
};

enum class EditorMode
{
    Scene,
    Scene3D,
    Scene2D,

    Count
};

struct EditScene
{
    SceneRef mSceneAsset;
    Node* mRootNode = nullptr;
    glm::mat4 mCameraTransform;
};

struct EditorState
{
    // Data
    EditorMode mMode;
    std::vector<Node*> mSelectedNodes;
    std::vector<EditScene> mEditScenes;
    AssetStub* mSelectedAssetStub = nullptr;
    ControlMode mControlMode = ControlMode::Default;
    TransformLock mTransformLock = TransformLock::None;
    Camera3D* mEditorCamera = nullptr;
    bool mMouseNeedsRecenter = false;
    bool mUiEnabled = true;
    bool mPlayInEditor = false;
    bool mEjected = false;
    bool mPaused = false;
    bool mHasEjectedOnce = false;
    std::string mStartupSceneName;
    int32_t mEditSceneIndex = -1;
    int32_t mPieEditSceneIdx = -1;
    AssetDir* mCurrentDir = nullptr;
    std::vector<AssetDir*> mDirPast;
    std::vector<AssetDir*> mDirFuture;
    std::vector<RTTI*> mInspectPast;
    std::vector<RTTI*> mInspectFuture;
    RTTI* mInspectedObject;
    RTTI* mPrevInspectedObject;
    AssetRef mInspectedAsset;
    bool mInspectLocked = false;
    Viewport3D* mViewport3D = nullptr;
    Viewport2D* mViewport2D = nullptr;
    std::string mAssetFilterStr;
    std::vector<AssetStub*> mFilteredAssetStubs;
    bool mRequestSaveSceneAs = false;
    bool mTrackSelectedAsset = false;
    bool mTrackSelectedNode = false;
    uint32_t mViewportX = 0;
    uint32_t mViewportY = 0;
    uint32_t mViewportWidth = 100;
    uint32_t mViewportHeight = 100;
    glm::uvec4 mPrevViewport = {};
    bool mShowLeftPane = true;
    bool mShowRightPane = true;
    bool mShowInterface = true;
    bool mPreviewLighting = true;

    // Methods
    void Init();
    void Shutdown();
    void Update(float deltaTime);

    void SetEditorMode(EditorMode mode);
    EditorMode GetEditorMode();

    void ReadEditorSave();
    void WriteEditorSave();

    void SetSelectedNode(Node* newNode);
    void AddSelectedNode(Node* node, bool addAllChildren);
    void RemoveSelectedNode(Node* node);
    void SetSelectedAssetStub(AssetStub* newStub);
    void SetControlMode(ControlMode newMode);

    void BeginPlayInEditor();
    void EndPlayInEditor();
    void EjectPlayInEditor();
    void InjectPlayInEditor();
    void SetPlayInEditorPaused(bool paused);
    bool IsPlayInEditorPaused();

    Camera3D* GetEditorCamera();

    void LoadStartupScene();

    Node* GetSelectedNode();
    Widget* GetSelectedWidget();
    const std::vector<Node*>& GetSelectedNodes();
    bool IsNodeSelected(Node* node);
    void DeselectNode(Node* node);
    //void ShowTextPrompt(const char* title, TextFieldHandlerFP confirmHandler, const char* defaultText = nullptr);

    void OpenEditScene(Scene* scene);
    void OpenEditScene(int32_t idx);
    void CloseEditScene(int32_t idx);
    void ShelveEditScene();
    EditScene* GetEditScene(int32_t idx = -1);
    void CloseAllEditScenes();
    void EnsureActiveScene();

    void ShowEditorUi(bool show);

    Asset* GetSelectedAsset();
    AssetStub* GetSelectedAssetStub();
    ControlMode GetControlMode();
    glm::vec3 GetTransformLockVector(TransformLock lock);
    void SetTransformLock(TransformLock lock);

    RTTI* GetInspectedObject();
    Node* GetInspectedNode();
    Asset* GetInspectedAsset();
    void InspectObject(RTTI* obj, bool force = false, bool recordHistory = true);
    void LockInspect(bool lock);
    bool IsInspectLocked();
    void RecordInspectHistory();
    void ClearInspectHistory();
    void ProgressInspectFuture();
    void RegressInspectPast();
    void ClearAssetDirHistory();
    void SetAssetDirectory(AssetDir* assetDir, bool recordHistory);
    AssetDir* GetAssetDirectory();
    void BrowseToAsset(const std::string& name);

    void CaptureAndSaveScene(AssetStub* stub, Node* rootNode);
    void DuplicateAsset(AssetStub* srcStub);

    void ProgressDirFuture();
    void RegressDirPast();

    void RemoveFilteredAssetStub(AssetStub* stub);

    Viewport3D* GetViewport3D();
    Viewport2D* GetViewport2D();
};

EditorState* GetEditorState();

#endif
