#pragma once

#if EDITOR

#include <string>
#include <glm/glm.hpp>

#include "ObjectRef.h"

#include "Widgets/TextEntry.h"
#include "Widgets/SceneImportWidget.h"

class Actor;
class Component;
class Asset;
class Level;
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
    Level,
    Widget,
    Blueprint,

    Count
};

struct EditorState
{
    EditorMode mMode;
    std::vector<Component*> mSelectedComponents;
    Widget* mSelectedWidget = nullptr;
    AssetStub* mSelectedAssetStub = nullptr;
    LevelRef mActiveLevel;
    WidgetMapRef mActiveWidgetMap;
    ControlMode mControlMode = ControlMode::Default;
    TransformLock mTransformLock = TransformLock::None;
    ActionList* mActionList = nullptr;
    SceneImportWidget* mSceneImportWidget = nullptr;
    TextEntry* mTextEntry = nullptr;
    Canvas* mRootCanvas = nullptr;
    Widget* mEditRootWidget = nullptr;
    bool mMouseNeedsRecenter = false;
    bool mPlayInEditor = false;
    bool mEjected = false;
    bool mPaused = false;
    LevelRef mCachedPieLevel = nullptr;
    ComponentRef mInjectedCamera = nullptr;
    ComponentRef mEjectedCamera = nullptr;
    std::string mStartupLevelName;
};

void SetEditorMode(EditorMode mode);
EditorMode GetEditorMode();

void InitializeEditorState();
void DestroyEditorState();
EditorState* GetEditorState();

void ReadEditorSave();
void WriteEditorSave();

void SetSelectedComponent(Component* newComponent);
void SetSelectedActor(Actor* newActor);
void AddSelectedComponent(Component* component);
void RemoveSelectedComponent(Component* component);
void AddSelectedActor(Actor* actor, bool addAllChildren);
void RemoveSelectedActor(Actor* actor);
void SetSelectedAssetStub(AssetStub* newStub);
void SetActiveLevel(Level* level);
void SetControlMode(ControlMode newMode);

void BeginPlayInEditor();
void EndPlayInEditor();
void EjectPlayInEditor();
void InjectPlayInEditor();
void SetPlayInEditorPaused(bool paused);
bool IsPlayInEditorPaused();

void LoadStartupLevel();

Component* GetSelectedComponent();
Actor* GetSelectedActor();
const std::vector<Component*>& GetSelectedComponents();
std::vector<Actor*> GetSelectedActors();
bool IsComponentSelected(Component* component);
bool IsActorSelected(Actor* actor);
void DeselectComponent(Component* component);
void ShowTextPrompt(const char* title, TextFieldHandlerFP confirmHandler, const char* defaultText = nullptr);
void ShowRootCanvas(bool show);

Widget* GetSelectedWidget();
void SetSelectedWidget(Widget* widget);
Widget* GetEditRootWidget();
void SetEditRootWidget(Widget* widget);
void SetActiveWidgetMap(WidgetMap* widgetMap);
WidgetMap* GetActiveWidgetMap();

Asset* GetSelectedAsset();
AssetStub* GetSelectedAssetStub();
Level* GetActiveLevel();
ControlMode GetControlMode();
glm::vec3 GetTransformLockVector(TransformLock lock);
void SetTransformLock(TransformLock lock);
ActionList* GetActionList();
SceneImportWidget* GetSceneImportWidget();

#endif
