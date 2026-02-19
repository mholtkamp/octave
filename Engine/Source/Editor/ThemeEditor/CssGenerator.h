#pragma once

#if EDITOR

#include "imgui.h"
#include <string>

struct CssThemeData;

namespace CssGenerator
{
    // Generate CSS text from an ImGuiStyle + custom properties.
    // Only writes properties that differ from ImGuiStyle() defaults.
    std::string GenerateCss(
        const ImGuiStyle& style,
        bool hasDockTabTextColor,
        const ImVec4& dockTabTextColor,
        const CssThemeData* panelOverrides = nullptr);
}

#endif
