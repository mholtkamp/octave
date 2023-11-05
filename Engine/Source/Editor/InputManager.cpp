#if EDITOR

#include "InputManager.h"
#include "ActionManager.h"
#include "InputDevices.h"
#include "Nodes/Widgets/TextField.h"
#include "Nodes/Widgets/Button.h"
#include "EditorUtils.h"
#include "EditorState.h"
#include "Renderer.h"
#include "Assets/Scene.h"
#include "AssetManager.h"

#include "Input/Input.h"

#include "imgui.h"

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
    addPlatform("Android", Platform::Android, false);
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
    bool modKeyDown = ctrlDown || shiftDown || altDown;

    const bool textFieldActive = ImGui::GetIO().WantTextInput;

    bool popupOpen = ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup);

    if (GetEditorState()->mPlayInEditor)
    {
        if (IsKeyJustDown(KEY_ESCAPE))
        {
            GetEditorState()->EndPlayInEditor();
        }
        else if (IsKeyJustDown(KEY_P) && altDown)
        {
            bool pause = !GetEditorState()->IsPlayInEditorPaused();
            GetEditorState()->SetPlayInEditorPaused(pause);
        }
        else if (IsKeyJustDown(KEY_F8))
        {
            if (GetEditorState()->mEjected)
            {
                GetEditorState()->InjectPlayInEditor();
            }
            else
            {
                GetEditorState()->EjectPlayInEditor();
            }
        }
        else if (IsKeyJustDown(KEY_F10))
        {
            FrameStep();
        }
    }
    else if (!popupOpen)
    {
        const bool isScene = (GetEditorState()->GetEditorMode() == EditorMode::Scene);

        if (ctrlDown && IsKeyJustDown(KEY_P) && isScene)
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
        else if (altDown && IsKeyJustDown(KEY_P) && isScene)
        {
            GetEditorState()->BeginPlayInEditor();
        }
        else if (ctrlDown && IsKeyJustDown(KEY_S))
        {
            if (isScene)
            {
                const bool saveAs = IsShiftDown();
                ActionManager::Get()->SaveScene(saveAs);
                ClearControlDown();
                ClearShiftDown();
                INP_ClearKey(KEY_S);
            }
        }
        else if (shiftDown && IsKeyJustDown(KEY_S) && !textFieldActive)
        {
            ActionManager::Get()->SaveSelectedAsset();
        }
        else if (ctrlDown && IsKeyJustDown(KEY_O) && isScene)
        {
            ActionManager::Get()->OpenScene();
            ClearControlDown();
            INP_ClearKey(KEY_O);
        }
        else if (ctrlDown && IsKeyJustDown(KEY_I))
        {
            ActionManager::Get()->ImportAsset();
            ClearControlDown();
            INP_ClearKey(KEY_I);
        }
        else if (shiftDown && ctrlDown && IsKeyJustDown(KEY_Z) && !textFieldActive)
        {
            ActionManager::Get()->Redo();
        }
        else if (ctrlDown && IsKeyJustDown(KEY_Z) && !textFieldActive)
        {
            ActionManager::Get()->Undo();
        }
        else if (altDown && IsKeyJustDown(KEY_R) && !textFieldActive)
        {
            ReloadAllScripts();
        }

        if (IsKeyJustDown(KEY_ESCAPE) &&
            !isScene)
        {
            // TODO: Show save prompt if edited asset has unsaved changes?
            GetEditorState()->SetEditorMode(EditorMode::Scene);
        }
    }

    if (!IsPlaying() || GetEditorState()->mEjected)
    {
        if (!modKeyDown && IsKeyJustDown(KEY_T) && !textFieldActive)
        {
            GetEditorState()->mShowLeftPane = !GetEditorState()->mShowLeftPane;
        }
        else if (!modKeyDown && IsKeyJustDown(KEY_N) && !textFieldActive)
        {
            GetEditorState()->mShowRightPane = !GetEditorState()->mShowRightPane;
        }
        else if (altDown && IsKeyJustDown(KEY_Z) && !textFieldActive)
        {
            GetEditorState()->mShowInterface = !GetEditorState()->mShowInterface;
        }
    }
}

#endif
