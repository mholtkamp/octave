#pragma once

#include "imgui.h"

namespace ImGui
{
    bool OctDragScalarN(const char* label, ImGuiDataType data_type, void* p_data, int components, float v_speed, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags);
    bool OctColorEdit4(const char* label, float col[4], ImGuiColorEditFlags flags);
}
