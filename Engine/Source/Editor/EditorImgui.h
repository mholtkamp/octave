#pragma once

#if EDITOR

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

void EditorImguiInit();
void EditorImguiDraw();
void EditorImguiShutdown();

void EditorImguiGetViewport(uint32_t& x, uint32_t& y, uint32_t& width, uint32_t& height);
bool EditorIsInterfaceVisible();

#endif