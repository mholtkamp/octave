#pragma once

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
    bool mMouseNeedsRecenter = false;
};

void InitializeEditorState();
void DestroyEditorState();
EditorState* GetEditorState();

void SetSelectedComponent(Component* newComponent);
void SetSelectedActor(Actor* newActor);
void AddSelectedComponent(Component* component);
void AddSelectedActor(Actor* actor, bool addAllChildren);
void SetSelectedAssetStub(AssetStub* newStub);
void SetActiveLevel(Level* level);
void SetControlMode(ControlMode newMode);

Component* GetSelectedComponent();
Actor* GetSelectedActor();
const std::vector<Component*>& GetSelectedComponents();
std::vector<Actor*> GetSelectedActors();
bool IsComponentSelected(Component* component);
bool IsActorSelected(Actor* actor);
void DeselectComponent(Component* component);
void ShowTextPrompt(const char* title, TextFieldHandlerFP confirmHandler);

Asset* GetSelectedAsset();
AssetStub* GetSelectedAssetStub();
Level* GetActiveLevel();
ControlMode GetControlMode();
glm::vec3 GetTransformLockVector(TransformLock lock);
void SetTransformLock(TransformLock lock);
ActionList* GetActionList();
