#pragma once

#if EDITOR
#include "EngineTypes.h"

#include <cstdint>
#include <vector>
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

class Object;
class Property;
struct AssetStub;

typedef void(*FileBrowserCallbackFP)(const std::vector<std::string>& filePaths);


void EditorImguiInit();
void EditorImguiDraw();
void EditorImguiShutdown();
void EditorImguiPreShutdown();

void EditorImguiGetViewport(uint32_t& x, uint32_t& y, uint32_t& width, uint32_t& height);
bool EditorIsInterfaceVisible();
void EditorOpenFileBrowser(FileBrowserCallbackFP callback, bool folderMode);
void EditorSetFileBrowserDir(const std::string& dir);
void EditorShowUnsavedAssetsModal(const std::vector<AssetStub*>& unsavedStubs);

void DrawAssetProperty(Property& prop, uint32_t index, Object* owner, PropertyOwnerType ownerType);

#endif
