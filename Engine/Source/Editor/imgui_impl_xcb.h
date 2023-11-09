#pragma once

#if PLATFORM_LINUX

#include "System/System.h"

#include "imgui.h"      // IMGUI_IMPL_API
#ifndef IMGUI_DISABLE

IMGUI_IMPL_API bool     ImGui_ImplXcb_Init(xcb_window_t window);
IMGUI_IMPL_API void     ImGui_ImplXcb_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplXcb_NewFrame();

IMGUI_IMPL_API int32_t ImGui_ImplXcb_EventHandler(xcb_generic_event_t* event);

#endif // #ifndef IMGUI_DISABLE

#endif