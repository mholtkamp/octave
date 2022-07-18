#if EDITOR

#include "EditorUtils.h"
#include "Engine.h"
#include "Log.h"
#include "World.h"
#include "Components/CameraComponent.h"
#include "PanelManager.h"
#include "Widgets/ViewportPanel.h"
#include "AssetManager.h"

#include "Input/Input.h"
#include "InputDevices.h"

void EditorShowCursor(bool show)
{
    INP_ShowCursor(show);
}

void EditorCenterCursor()
{
    int32_t centerX = GetEngineState()->mWindowWidth / 2;
    int32_t centerY = GetEngineState()->mWindowHeight / 2;
    EditorSetCursorPos(centerX, centerY);
}

void EditorSetCursorPos(int32_t x, int32_t y)
{
    INP_SetCursorPos(x, y);
}

void EditorGetWindowCenter(int32_t& x, int32_t& y)
{
    x = GetEngineState()->mWindowWidth / 2;
    y = GetEngineState()->mWindowHeight / 2;
}

glm::vec3 EditorGetFocusPosition()
{
    CameraComponent* camera = GetWorld()->GetActiveCamera();
    float focalDistance = PanelManager::Get()->GetViewportPanel()->GetFocalDistance();
    glm::vec3 focusPos = camera->GetAbsolutePosition() + focalDistance * camera->GetForwardVector();

    return focusPos;
}

AssetStub* EditorAddUniqueAsset(const char* baseName, AssetDir* dir, TypeId assetType, bool autoCreate)
{
    AssetStub* stub = nullptr;
    std::string assetName;
    for (int32_t i = 0; i < 99; ++i)
    {
        assetName = baseName;

        if (i > 0)
        {
            assetName += "_";
            assetName += std::to_string(i);
        }

        if (!AssetManager::Get()->DoesAssetExist(assetName))
        {
            if (autoCreate)
            {
                stub = AssetManager::Get()->CreateAndRegisterAsset(assetType, dir, assetName, false);
            }
            else
            {
                stub = AssetManager::Get()->RegisterAsset(assetName, assetType, dir, nullptr, false);

                if (stub != nullptr)
                {
                    Asset* newAsset = Asset::CreateInstance(assetType);
                    stub->mAsset = newAsset;
                }
            }
            break;
        }
    }

    return stub;
}


#endif
