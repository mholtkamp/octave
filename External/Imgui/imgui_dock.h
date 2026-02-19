// based on https://github.com/nem0/LumixEngine/blob/master/external/imgui/imgui_dock.h
// modified from https://bitbucket.org/duangle/liminal/src/tip/src/liminal/imgui_dock.h
// adapted for ImGui 1.89.9

#pragma once

typedef enum ImGuiDockSlot {
    ImGuiDockSlot_Left,
    ImGuiDockSlot_Right,
    ImGuiDockSlot_Top,
    ImGuiDockSlot_Bottom,
    ImGuiDockSlot_Tab,

    ImGuiDockSlot_Float,
    ImGuiDockSlot_None
} ImGuiDockSlot;

namespace ImGui{

IMGUI_API bool BeginDockspace();
IMGUI_API void EndDockspace();
IMGUI_API void ShutdownDock();
IMGUI_API void SetNextDock(const char* dock_panel, ImGuiDockSlot slot);
IMGUI_API void SetNextDockPos(const char* dock_panel, ImGuiDockSlot slot, float ratio = 0.5f);
IMGUI_API void ResetNextDockParent(const char* panel);
IMGUI_API bool BeginDock(const char* label, bool* opened = nullptr, ImGuiWindowFlags extra_flags = 0);
IMGUI_API void EndDock();
IMGUI_API void DockDebugWindow(const char* dock_panel);
IMGUI_API void UndockAll(const char* panel);
IMGUI_API void DockTo(const char* panel, const char* dockLabel, const char* destLabel, ImGuiDockSlot slot, float ratio = 0.5f);
IMGUI_API void DockToRoot(const char* panel, const char* dockLabel, ImGuiDockSlot slot, float ratio = 0.5f);
IMGUI_API bool HasDockLayout(const char* panel);
IMGUI_API bool HasDock(const char* panel, const char* label);
IMGUI_API void ClearDocks(const char* panel);
IMGUI_API void SetDockTabRounding(float left, float right);
IMGUI_API void GetDockTabRounding(float& left, float& right);
IMGUI_API void InitDock();

};
