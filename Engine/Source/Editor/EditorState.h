#pragma once

#if EDITOR

#include <string>
#include "Maths.h"

#include "ObjectRef.h"

#include "Widgets/TextEntry.h"
#include "Widgets/SceneImportWidget.h"

class Node;
class Asset;
struct AssetStub;
class ActionList;
class Canvas;

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

    Count
};

struct EditScene
{
    SceneRef mSceneAsset;
    Node* mRootNode = nullptr;
};

struct EditorState
{
    EditorMode mMode;
    std::vector<Node*> mSelectedNodes;
    std::vector<EditScene> mEditScenes;
    AssetStub* mSelectedAssetStub = nullptr;
    ControlMode mControlMode = ControlMode::Default;
    TransformLock mTransformLock = TransformLock::None;
    bool mMouseNeedsRecenter = false;
    bool mUiEnabled = true;
    bool mPlayInEditor = false;
    bool mEjected = false;
    bool mPaused = false;
    NodeRef mInjectedCamera = nullptr;
    NodeRef mEjectedCamera = nullptr;
    std::string mStartupSceneName;
    int32_t mEditSceneIndex = -1;
};

void SetEditorMode(EditorMode mode);
EditorMode GetEditorMode();

void InitializeEditorState();
void DestroyEditorState();
EditorState* GetEditorState();

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

void LoadStartupScene();

Node* GetSelectedNode();
const std::vector<Node*>& GetSelectedNodes();
bool IsNodeSelected(Node* node);
void DeselectNode(Node* node);
//void ShowTextPrompt(const char* title, TextFieldHandlerFP confirmHandler, const char* defaultText = nullptr);

void OpenEditScene(Scene* scene);
void CloseEditScene(Scene* scene);
void ShelveEditScene();
EditScene* GetEditScene();

void ShowEditorUi(bool show);

Asset* GetSelectedAsset();
AssetStub* GetSelectedAssetStub();
ControlMode GetControlMode();
glm::vec3 GetTransformLockVector(TransformLock lock);
void SetTransformLock(TransformLock lock);
ActionList* GetActionList();
SceneImportWidget* GetSceneImportWidget();

#endif
