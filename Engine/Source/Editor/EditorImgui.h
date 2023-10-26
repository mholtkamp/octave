#pragma once

#if EDITOR

#include "imgui.h"

void EditorImguiInit();
void EditorImguiNewFrame();
void EditorImguiRenderDrawData(ImDrawData* draw_data);
void EditorImguiShutdown();

#endif