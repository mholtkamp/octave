#pragma once

#if EDITOR

#include <string>
#include "Maths.h"
#include "AssetRef.h"
#include "SmartPointer.h"
#include "Property.h"
#include "Nodes/Widgets/Text.h"

class Node;
class Scene;
class Widget;
class Text;
class Asset;
struct AssetStub;
class ActionList;
class Canvas;
class Camera3D;
class Viewport3D;
class Viewport2D;
class PaintManager;
struct SubSceneOverride;

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
    Scene2D,
    Scene3D,

    Count
};

enum class PaintMode
{
    None,
    Color,
    Instance,

    Count
};

struct LinkedSceneProps
{
    Node* mNode = nullptr;
    std::vector<Property> mProps;
    std::vector<SubSceneOverride> mSubSceneOverrides;
};

struct EditScene
{
    SceneRef mSceneAsset;
    NodePtr mRootNode;
    glm::mat4 mCameraTransform;
    std::vector<LinkedSceneProps> mLinkedSceneProps;
};

struct EditorState
{
    // Data
    EditorMode mMode;
    std::vector<Node*> mSelectedNodes;
    int32_t mSelectedInstance = -1;
    std::vector<EditScene> mEditScenes;
    AssetStub* mSelectedAssetStub = nullptr;
    ControlMode mControlMode = ControlMode::Default;
    TransformLock mTransformLock = TransformLock::None;
    SharedPtr<Camera3D> mEditorCamera;
    bool mMouseNeedsRecenter = false;
    bool mUiEnabled = true;
    bool mPlayInEditor = false;
    bool mEjected = false;
    bool mPaused = false;
    bool mHasEjectedOnce = false;
    int32_t mEditSceneIndex = -1;
    int32_t mPieEditSceneIdx = -1;
    AssetDir* mCurrentDir = nullptr;
    std::vector<AssetDir*> mDirPast;
    std::vector<AssetDir*> mDirFuture;
    std::vector<Object*> mInspectPast;
    std::vector<Object*> mInspectFuture;
    Object* mInspectedObject;
    Object* mPrevInspectedObject;
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
    SharedPtr<Text> mOverlayText = nullptr;
    std::vector<std::string> mFavoritedDirs;
    std::vector<std::string> mRecentProjects;
    PaintMode mPaintMode = PaintMode::None;
    PaintManager* mPaintManager = nullptr;
    bool mNodePropertySelect = false;
    int32_t mNodePropertySelectIndex = 0;
    std::string mNodePropertySelectName = "";
    std::string mPendingSceneImportPath = "";

    // Methods
    void Init();
    void Shutdown();
    void Update(float deltaTime);

    void GatherProperties(std::vector<Property>& props);

    void SetEditorMode(EditorMode mode);
    EditorMode GetEditorMode();

    void SetPaintMode(PaintMode paintMode);
    PaintMode GetPaintMode();

    void ReadEditorSave();
    void WriteEditorSave();

    void ReadEditorProjectSave();
    void WriteEditorProjectSave();

    void HandleNodeDestroy(Node* node);

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

    void SetNodePropertySelect(bool enable, int32_t index, const std::string& propName);
    void ClearNodePropertySelect();
    void AssignNodePropertySelect(Node* targetNode);

    Camera3D* GetEditorCamera();

    void LoadStartupScene();

    Node* GetSelectedNode();
    Widget* GetSelectedWidget();
    const std::vector<Node*>& GetSelectedNodes();
    bool IsNodeSelected(Node* node);
    void DeselectNode(Node* node);
    int32_t GetSelectedInstance();
    void SetSelectedInstance(int32_t instance);
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

    Object* GetInspectedObject();
    Node* GetInspectedNode();
    Asset* GetInspectedAsset();
    void InspectObject(Object* obj, bool force = false, bool recordHistory = true);
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

    bool IsDirFavorited(const std::string& dirPath);
    void AddFavoriteDir(const std::string& dirPath);
    void RemoveFavoriteDir(const std::string& dirPath);

    void AddRecentProject(const std::string& projPath);
};

EditorState* GetEditorState();

#endif
