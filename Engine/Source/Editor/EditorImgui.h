#pragma once

#if EDITOR
#include <cstdint>
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

typedef void(*FileBrowserCallbackFP)(const std::string& filePath);


void EditorImguiInit();
void EditorImguiDraw();
void EditorImguiShutdown();
void EditorImguiPreShutdown();

void EditorImguiGetViewport(uint32_t& x, uint32_t& y, uint32_t& width, uint32_t& height);
bool EditorIsInterfaceVisible();
void EditorOpenFileBrowser(FileBrowserCallbackFP callback, bool folderMode);
void EditorSetFileBrowserDir(const std::string& dir);

#endif
