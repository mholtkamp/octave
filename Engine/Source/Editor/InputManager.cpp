#if EDITOR

#include "InputManager.h"
#include "ActionManager.h"
#include "InputDevices.h"
#include "Widgets/TextField.h"
#include "Widgets/Button.h"
#include "EditorUtils.h"
#include "EditorState.h"
#include "Renderer.h"

#include "Input/Input.h"

InputManager* InputManager::sInstance = nullptr;

class PlatformBuildButton : public Button
{
public:
    Platform mPlatform = Platform::Count;
    bool mEmbedded = false;
};

void HandleBuildPlatformSelect(Button* button)
{
    PlatformBuildButton* platformButton = (PlatformBuildButton*) button;

    Platform platform = platformButton->mPlatform;
    bool embedded = platformButton->mEmbedded;

    ActionManager::Get()->BuildData(platform, embedded);

    // Hide the modal platform build list
    Renderer::Get()->SetModalWidget(nullptr);
}

void InputManager::Create()
{
    Destroy();
    sInstance = new InputManager();
}

void InputManager::Destroy()
{
    if (sInstance != nullptr)
    {
        delete sInstance;
        sInstance = nullptr;
    }
}

InputManager* InputManager::Get()
{
    return sInstance;
}

InputManager::InputManager()
{
    mBuildPlatformList = new ModalList();

    auto addPlatform = [this](const char* name, Platform platform, bool embedded)
    {
        PlatformBuildButton* item = new PlatformBuildButton();
        item->SetDimensions(150,22);
        item->SetTextString(name);
        item->SetPressedHandler(HandleBuildPlatformSelect);
        item->mPlatform = platform;
        item->mEmbedded = embedded;
        mBuildPlatformList->AddListItem(item);
    };

#if PLATFORM_WINDOWS
    addPlatform("Windows", Platform::Windows, false);
#elif PLATFORM_LINUX
    addPlatform("Linux", Platform::Linux, false);
#endif
    addPlatform("GameCube", Platform::GameCube, false);
    addPlatform("Wii", Platform::Wii, false);
    addPlatform("3DS", Platform::N3DS, false);
    addPlatform("GameCube Embedded", Platform::GameCube, true);
    addPlatform("Wii Embedded", Platform::Wii, true);
    addPlatform("3DS Embedded", Platform::N3DS, true);
}

InputManager::~InputManager()
{

    if (mBuildPlatformList != nullptr)
    {
        if (Renderer::Get()->GetModalWidget() == mBuildPlatformList)
        {
            Renderer::Get()->SetModalWidget(nullptr);
        }

        delete mBuildPlatformList;
        mBuildPlatformList = nullptr;
    }
}

void InputManager::Update()
{
    UpdateHotkeys();
}

void InputManager::UpdateHotkeys()
{
    const bool ctrlDown = IsControlDown();
    const bool shiftDown = IsShiftDown();
    const bool altDown = IsAltDown();

    if (GetEditorState()->mPlayInEditor)
    {
        if (IsKeyJustDown(KEY_ESCAPE))
        {
            EndPlayInEditor();
        }
        else if (IsKeyJustDown(KEY_P) && altDown)
        {
            bool pause = !IsPlayInEditorPaused();
            SetPlayInEditorPaused(pause);
        }
        else if (IsKeyJustDown(KEY_F8))
        {
            if (GetEditorState()->mEjected)
            {
                InjectPlayInEditor();
            }
            else
            {
                EjectPlayInEditor();
            }
        }
    }
    else if (Renderer::Get()->GetModalWidget() == nullptr)
    {
        const bool textFieldActive = (TextField::GetSelectedTextField() != nullptr);

        if (ctrlDown && IsKeyJustDown(KEY_P))
        {
            if (shiftDown)
            {
                ActionManager::Get()->CreateNewProject();
            }
            else
            {
                ActionManager::Get()->OpenProject();
            }

            // Fix issue where keys CTRL and P are considered held down still.
            ClearControlDown();
            ClearShiftDown();
            INP_ClearKey(KEY_P);
        }
        else if (altDown && IsKeyJustDown(KEY_P))
        {
            BeginPlayInEditor();
        }
        else if (ctrlDown && IsKeyJustDown(KEY_S))
        {
            const bool saveAs = IsShiftDown();
            ActionManager::Get()->SaveLevel(saveAs);
            ClearControlDown();
            ClearShiftDown();
            INP_ClearKey(KEY_S);
        }
        else if (shiftDown && IsKeyJustDown(KEY_S) && !textFieldActive)
        {
            ActionManager::Get()->SaveSelectedAsset();
        }
        else if (ctrlDown && IsKeyJustDown(KEY_O))
        {
            ActionManager::Get()->OpenLevel();
            ClearControlDown();
            INP_ClearKey(KEY_O);
        }
        else if (ctrlDown && IsKeyJustDown(KEY_I))
        {
            ActionManager::Get()->ImportAsset();
            ClearControlDown();
            INP_ClearKey(KEY_I);
        }
        else if (ctrlDown && IsKeyJustDown(KEY_B))
        {
            Renderer::Get()->SetModalWidget(mBuildPlatformList);
            mBuildPlatformList->MoveToMousePosition();
        }
        else if (ctrlDown && IsKeyJustDown(KEY_K))
        {
            ActionManager::Get()->RecaptureAndSaveAllLevels();
        }
        else if (shiftDown && ctrlDown && IsKeyJustDown(KEY_Z))
        {
            ActionManager::Get()->Redo();
        }
        else if (ctrlDown && IsKeyJustDown(KEY_Z))
        {
            ActionManager::Get()->Undo();
        }
        else if (altDown && IsKeyJustDown(KEY_R))
        {
            ReloadAllScripts();
        }
    }
}

#endif
