#if EDITOR

#include "EditorState.h"
#include "PanelManager.h"
#include "ActionManager.h"
#include "Actor.h"
#include "Asset.h"
#include "AssetManager.h"
#include "Components/Component.h"
#include "Components/TransformComponent.h"
#include "Engine.h"
#include "World.h"
#include "Assets/Level.h"
#include "EditorUtils.h"
#include "Widgets/ActionList.h"
#include "Widgets/TextEntry.h"

static EditorState sEditorState;

void InitializeEditorState()
{
    sEditorState.mTextEntry = new TextEntry();
}

void DestroyEditorState()
{
    delete sEditorState.mTextEntry;
    sEditorState.mTextEntry = nullptr;
}

EditorState* GetEditorState()
{
    return &sEditorState;
}

void SetSelectedComponent(Component* newComponent)
{
    if (sEditorState.mSelectedComponents.size() != 1 ||
        sEditorState.mSelectedComponents[0] != newComponent)
    {
        sEditorState.mSelectedComponents.clear();

        if (newComponent != nullptr)
        {
            sEditorState.mSelectedComponents.push_back(newComponent);
        }

        PanelManager::Get()->OnSelectedComponentChanged();
        ActionManager::Get()->OnSelectedComponentChanged();
    }
}

void SetSelectedActor(Actor* newActor)
{
    SetSelectedComponent(newActor ? newActor->GetRootComponent() : nullptr);
}

void AddSelectedComponent(Component* component)
{
    if (component != nullptr)
    {
        std::vector<Component*>& comps = sEditorState.mSelectedComponents;
        auto it = std::find(comps.begin(), comps.end(), component);

        if (it != comps.end())
        {
            // Move the component to the back of the vector so that 
            // it is considered the primary selected component.
            comps.erase(it);
        }

        comps.push_back(component);
    }
}

void AddSelectedActor(Actor* actor, bool addAllChildren)
{
    if (addAllChildren)
    {
        const std::vector<Component*>& comps = actor->GetComponents();

        for (uint32_t i = 0; i < comps.size(); ++i)
        {
            if (comps[i]->IsTransformComponent())
            {
                AddSelectedComponent(comps[i]);
            }
        }
    }
    else
    {
        AddSelectedComponent(actor ? actor->GetRootComponent() : nullptr);
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

void SetActiveLevel(Level* level)
{
    sEditorState.mActiveLevel = level;
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
        EditorShowCursor(true);
    }

    if (newMode == ControlMode::Pilot ||
        newMode == ControlMode::Translate ||
        newMode == ControlMode::Rotate ||
        newMode == ControlMode::Scale ||
        newMode == ControlMode::Pan ||
        newMode == ControlMode::Orbit)
    {
        EditorShowCursor(false);

        // Center the cursor before any movement so the camera rotation doesn't jump at first.
        EditorCenterCursor();

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

    SetSelectedComponent(nullptr);
    SetSelectedAssetStub(nullptr);

    sEditorState.mPlayInEditor = true;

    if (sEditorState.mActiveLevel != nullptr)
    {
        assert(sEditorState.mCachedPieLevel == nullptr);
        Level* cachedLevel = new Level();
        cachedLevel->Create();
        cachedLevel->CaptureWorld(GetWorld());
        AssetManager::Get()->RegisterTransientAsset(cachedLevel);
        sEditorState.mCachedPieLevel = cachedLevel;
    }

    GetWorld()->DestroyAllActors();

    ShowRootCanvas(false);
    Renderer::Get()->EnableProxyRendering(false);

    // Fake-Initialize the Game
    //OctPreInitialize();
    OctPostInitialize();

    if (sEditorState.mCachedPieLevel != nullptr)
    {
        sEditorState.mCachedPieLevel.Get<Level>()->LoadIntoWorld(GetWorld());
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

    // Fake Shutdown
    OctPreShutdown();
    OctPostShutdown();

    SetSelectedComponent(nullptr);
    SetSelectedAssetStub(nullptr);

    GetWorld()->DestroyAllActors();

    ShowRootCanvas(true);
    Renderer::Get()->EnableProxyRendering(true);

    sEditorState.mPlayInEditor = false;

    // Restore cached editor level
    if (sEditorState.mCachedPieLevel.Get() != nullptr)
    {
        assert(sEditorState.mActiveLevel != nullptr);
        sEditorState.mCachedPieLevel.Get<Level>()->LoadIntoWorld(GetWorld());
        sEditorState.mCachedPieLevel = nullptr;
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
        sEditorState.mInjectedCamera = GetWorld()->GetActiveCamera();

        if (sEditorState.mEjectedCamera == nullptr)
        {
            Actor* cameraActor = GetWorld()->SpawnActor<Actor>();
            cameraActor->SetName("Ejected Camera");
            sEditorState.mEjectedCamera = cameraActor->CreateComponent<CameraComponent>();

            // Set its transform to match the PIE camera
            if (GetWorld()->GetActiveCamera())
            {
                cameraActor->GetRootComponent()->SetTransform(GetWorld()->GetActiveCamera()->GetTransform());
            }
        }

        GetWorld()->SetActiveCamera(sEditorState.mEjectedCamera.Get<CameraComponent>());
        ShowRootCanvas(true);
        sEditorState.mEjected = true;
    }
}

void InjectPlayInEditor()
{
    if (sEditorState.mPlayInEditor &&
        sEditorState.mEjected)
    {
        if (sEditorState.mInjectedCamera != nullptr)
        {
            GetWorld()->SetActiveCamera(sEditorState.mInjectedCamera.Get<CameraComponent>());
        }

        ShowRootCanvas(false);
        sEditorState.mEjected = false;
    }
}

Component* GetSelectedComponent()
{
    return (sEditorState.mSelectedComponents.size() > 0) ?
        sEditorState.mSelectedComponents.back() :
        nullptr;
}

Actor* GetSelectedActor()
{
    Actor* actor = nullptr;
    Component* selComp = (sEditorState.mSelectedComponents.size() > 0) ?
        sEditorState.mSelectedComponents.back() :
        nullptr;

    if (selComp != nullptr)
    {
        actor = selComp->GetOwner();
    }

    return actor;
}

const std::vector<Component*>& GetSelectedComponents()
{
    // Return copy
    return sEditorState.mSelectedComponents;
}

std::vector<Actor*> GetSelectedActors()
{
    std::vector<Actor*> selActors;

    for (uint32_t i = 0; i < sEditorState.mSelectedComponents.size(); ++i)
    {
        Actor* actor = sEditorState.mSelectedComponents[i]->GetOwner();

        if (actor != nullptr &&
            std::find(selActors.begin(), selActors.end(), actor) == selActors.end())
        {
            selActors.push_back(actor);
        }
    }

    return selActors;
}

bool IsComponentSelected(Component* component)
{
    for (uint32_t i = 0; i < sEditorState.mSelectedComponents.size(); ++i)
    {
        if (sEditorState.mSelectedComponents[i] == component)
        {
            return true;
        }
    }
    return false;
}

bool IsActorSelected(Actor* actor)
{
    for (uint32_t i = 0; i < sEditorState.mSelectedComponents.size(); ++i)
    {
        if (sEditorState.mSelectedComponents[i]->GetOwner() == actor)
        {
            return true;
        }
    }
    return false;
}

void DeselectComponent(Component* component)
{
    for (uint32_t i = 0; i < sEditorState.mSelectedComponents.size(); ++i)
    {
        if (sEditorState.mSelectedComponents[i] == component)
        {
            sEditorState.mSelectedComponents.erase(sEditorState.mSelectedComponents.begin() + i);
            break;
        }
    }
}

void ShowTextPrompt(const char* title, TextFieldHandlerFP confirmHandler)
{
    sEditorState.mTextEntry->Prompt(title, confirmHandler);
}

void ShowRootCanvas(bool show)
{
    if (sEditorState.mRootCanvas)
    {
        if (show)
        {
            Renderer::Get()->AddWidget(sEditorState.mRootCanvas);
        }
        else
        {
            Renderer::Get()->RemoveWidget(sEditorState.mRootCanvas);
        }
    }
}

Asset* GetSelectedAsset()
{
    return  sEditorState.mSelectedAssetStub ? sEditorState.mSelectedAssetStub->mAsset : nullptr;
}

AssetStub* GetSelectedAssetStub()
{
    return sEditorState.mSelectedAssetStub;
}

Level* GetActiveLevel()
{
    return sEditorState.mActiveLevel;
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

        Component* comp = GetSelectedComponent();
        if (comp != nullptr && comp->IsTransformComponent())
        {
            glm::vec3 pos = static_cast<TransformComponent*>(comp)->GetAbsolutePosition();
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

ActionList* GetActionList()
{
    if (sEditorState.mActionList == nullptr)
    {
        sEditorState.mActionList = new ActionList();
    }

    return sEditorState.mActionList;
}

#endif