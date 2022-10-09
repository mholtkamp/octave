#pragma once

#if EDITOR

#include <string>
#include <glm/glm.hpp>

#include "ObjectRef.h"

#include "Widgets/TextEntry.h"

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

struct EditorState
{
    std::vector<Component*> mSelectedComponents;
    AssetStub* mSelectedAssetStub = nullptr;
    Level* mActiveLevel = nullptr;
    ControlMode mControlMode = ControlMode::Default;
    TransformLock mTransformLock = TransformLock::None;
    ActionList* mActionList = nullptr;
    TextEntry* mTextEntry = nullptr;
    Canvas* mRootCanvas = nullptr;
    bool mMouseNeedsRecenter = false;
    bool mPlayInEditor = false;
    bool mEjected = false;
    LevelRef mCachedPieLevel = nullptr;
    ComponentRef mInjectedCamera = nullptr;
    ComponentRef mEjectedCamera = nullptr;
};

void InitializeEditorState();
void DestroyEditorState();
EditorState* GetEditorState();

void SetSelectedComponent(Component* newComponent);
void SetSelectedActor(Actor* newActor);
void AddSelectedComponent(Component* component);
void RemoveSelectedComponent(Component* component);
void AddSelectedActor(Actor* actor, bool addAllChildren);
void SetSelectedAssetStub(AssetStub* newStub);
void SetActiveLevel(Level* level);
void SetControlMode(ControlMode newMode);

void BeginPlayInEditor();
void EndPlayInEditor();
void EjectPlayInEditor();
void InjectPlayInEditor();

Component* GetSelectedComponent();
Actor* GetSelectedActor();
const std::vector<Component*>& GetSelectedComponents();
std::vector<Actor*> GetSelectedActors();
bool IsComponentSelected(Component* component);
bool IsActorSelected(Actor* actor);
void DeselectComponent(Component* component);
void ShowTextPrompt(const char* title, TextFieldHandlerFP confirmHandler);
void ShowRootCanvas(bool show);

Asset* GetSelectedAsset();
AssetStub* GetSelectedAssetStub();
Level* GetActiveLevel();
ControlMode GetControlMode();
glm::vec3 GetTransformLockVector(TransformLock lock);
void SetTransformLock(TransformLock lock);
ActionList* GetActionList();

#endif
