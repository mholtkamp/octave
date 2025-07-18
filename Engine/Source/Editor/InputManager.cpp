#if EDITOR

#include "InputManager.h"
#include "ActionManager.h"
#include "InputDevices.h"
#include "EditorUtils.h"
#include "EditorState.h"
#include "Renderer.h"
#include "Viewport3d.h"
#include "Viewport2d.h"
#include "Assets/Scene.h"
#include "AssetManager.h"

#include "Input/Input.h"

#include "imgui.h"

InputManager* InputManager::sInstance = nullptr;

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

}

InputManager::~InputManager()
{

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
            if (GetEditorState()->mNodePropertySelect)
            {
                GetEditorState()->ClearNodePropertySelect();
            }
            else
            {
                GetEditorState()->EndPlayInEditor();
            }
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
        EditorMode editorMode = GetEditorState()->GetEditorMode();
        const bool isScene = (editorMode == EditorMode::Scene) || (editorMode == EditorMode::Scene2D) || (editorMode == EditorMode::Scene3D);

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
        else if (ctrlDown && IsKeyJustDown(KEY_O))
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

        if (ctrlDown &&
            (GetEditorState()->GetViewport3D()->ShouldHandleInput() || 
             GetEditorState()->GetViewport2D()->ShouldHandleInput()))
        {
            if (IsKeyJustDown(KEY_1))
            {
                GetEditorState()->SetEditorMode(EditorMode::Scene);
            }
            else if (IsKeyJustDown(KEY_2))
            {
                GetEditorState()->SetEditorMode(EditorMode::Scene2D);
            }
            else if (IsKeyJustDown(KEY_3))
            {
                GetEditorState()->SetEditorMode(EditorMode::Scene3D);
                GetEditorState()->SetPaintMode(PaintMode::None);
            }
            else if (IsKeyJustDown(KEY_4))
            {
                GetEditorState()->SetEditorMode(EditorMode::Scene3D);
                GetEditorState()->SetPaintMode(PaintMode::Color);
            }
            else if (IsKeyJustDown(KEY_5))
            {
                GetEditorState()->SetEditorMode(EditorMode::Scene3D);
                GetEditorState()->SetPaintMode(PaintMode::Instance);
            }
        }

        if (IsKeyJustDown(KEY_ESCAPE))
        {
            if (GetEditorState()->mNodePropertySelect)
            {
                GetEditorState()->ClearNodePropertySelect();
            }
            else
            {
                GetEditorState()->SetEditorMode(EditorMode::Scene);
            }
        }
    }

    // Clicked but nothing was selected? Just clear node prop select mode.
    if (GetEditorState()->mNodePropertySelect &&
        IsMouseButtonJustDown(MOUSE_LEFT))
    {
        GetEditorState()->ClearNodePropertySelect();
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
