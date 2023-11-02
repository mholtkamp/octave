#if EDITOR

#include "EditorUtils.h"
#include "Engine.h"
#include "Log.h"
#include "World.h"
#include "Nodes/3D/Camera3d.h"
#include "PanelManager.h"
#include "Viewport3d.h"
#include "Viewport2d.h"
#include "Widgets/WidgetHierarchyPanel.h"
#include "AssetManager.h"
#include "Assets/Scene.h"
#include "ActionManager.h"

#include "Input/Input.h"
#include "InputDevices.h"

void EditorCenterCursor()
{
    int32_t centerX = GetEngineState()->mWindowWidth / 2;
    int32_t centerY = GetEngineState()->mWindowHeight / 2;
    INP_SetCursorPos(centerX, centerY);
}

glm::vec3 EditorGetFocusPosition()
{
    Camera3D* camera = GetWorld()->GetActiveCamera();
    float focalDistance = GetEditorState()->GetViewport3D()->GetFocalDistance();
    glm::vec3 focusPos = camera->GetAbsolutePosition() + focalDistance * camera->GetForwardVector();

    return focusPos;
}

AssetStub* EditorAddUniqueAsset(const char* baseName, AssetDir* dir, TypeId assetType, bool autoCreate)
{
    AssetStub* stub = nullptr;
    std::string name = baseName;


    // In editor, we want to make the name more readable.
    const int32_t kMaxRenameTries = 1000;
    int32_t renameTry = 0;
    bool validName = !AssetManager::Get()->DoesAssetExist(name);

    if (!validName)
    {
        // Append an underscore if we don't have one yet.
        size_t lastUnderscore = name.rfind("_");
        if (lastUnderscore == std::string::npos)
        {
            // No underscores at all, just append the underscore.
            name += "_";
        }
        else
        {
            // There is an underscore, so lets see if everything to the right of it is a number.
            // If not, then we need to add another underscore.
            for (size_t i = lastUnderscore + 1; i < name.size(); ++i)
            {
                if (!(name[i] >= '0' && name[i] <= '9'))
                {
                    name += "_";
                    break;
                }
            }
        }
    }

    while (!validName)
    {
        // 1 - Determine number at end of current name and remove it from name string
        size_t lastCharIndex = name.find_last_not_of("0123456789");

        uint32_t num = 1;
        if (lastCharIndex == std::string::npos)
        {
            // The name is entirely numbers?
            num = (uint32_t)std::stoul(name);
            name = "";
        }
        else if (lastCharIndex + 1 < name.size())
        {
            std::string numStr = name.substr(lastCharIndex + 1);
            name = name.substr(0, lastCharIndex + 1);

            num = (uint32_t)std::stoul(numStr);
        }

        num++;
        name = name + std::to_string(num);

        validName = !AssetManager::Get()->DoesAssetExist(name);

        renameTry++;

        // If exceeded max renames, just use sUniqueId to make unique name like in-game.
        if (renameTry > kMaxRenameTries)
        {
            break;
        }
    }

    if (validName)
    {
        if (autoCreate)
        {
            stub = AssetManager::Get()->CreateAndRegisterAsset(assetType, dir, name, false);
        }
        else
        {
            stub = AssetManager::Get()->RegisterAsset(name, assetType, dir, nullptr, false);

            if (stub != nullptr)
            {
                Asset* newAsset = Asset::CreateInstance(assetType);
                stub->mAsset = newAsset;
            }
        }
    }

    return stub;
}

std::string EditorGetAssetNameFromPath(const std::string& path)
{
    const char* lastSlash = strrchr(path.c_str(), '/');
    std::string filename = lastSlash ? (lastSlash + 1) : path;
    int32_t dotIndex = int32_t(filename.find_last_of('.'));
    std::string assetName = filename.substr(0, dotIndex);

    return assetName;
}

// Utility function for removing nodes that have parents already in the list.
// This is probably a pretty slow O(N^2) operation.
void RemoveRedundantDescendants(std::vector<Node*>& nodes)
{
    for (int32_t i = int32_t(nodes.size()) - 1; i >= 0; --i)
    {
        Node* parent = nodes[i]->GetParent();

        if (parent != nullptr)
        {
            for (int32_t j = 0; j < int32_t(nodes.size()); ++j)
            {
                if (parent == nodes[j])
                {
                    // nodes[i] has a parent (nodes[j]), so we can remove it from the list.
                    nodes.erase(nodes.begin() + i);
                    break;
                }
            }
        }
    }
}

#endif
