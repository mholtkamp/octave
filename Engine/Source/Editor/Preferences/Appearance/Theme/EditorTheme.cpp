#if EDITOR

#include "EditorTheme.h"
#include "imgui.h"

namespace EditorTheme
{

static std::vector<std::string> sThemeNames = {
    "Dark",
    "Light",
    "Future Dark",
    "Classic"
};

const std::vector<std::string>& GetThemeNames()
{
    return sThemeNames;
}

const char* GetThemeName(EditorThemeType type)
{
    int index = static_cast<int>(type);
    if (index >= 0 && index < static_cast<int>(sThemeNames.size()))
    {
        return sThemeNames[index].c_str();
    }
    return "Unknown";
}

EditorThemeType GetThemeTypeFromName(const std::string& name)
{
    for (size_t i = 0; i < sThemeNames.size(); ++i)
    {
        if (sThemeNames[i] == name)
        {
            return static_cast<EditorThemeType>(i);
        }
    }
    return EditorThemeType::Dark;
}

void ApplyTheme(EditorThemeType type)
{
    switch (type)
    {
    case EditorThemeType::Dark:
        ApplyDarkTheme();
        break;
    case EditorThemeType::Light:
        ApplyLightTheme();
        break;
    case EditorThemeType::FutureDark:
        ApplyFutureDarkTheme();
        break;
    case EditorThemeType::Classic:
        ApplyClassicTheme();
        break;
    default:
        ApplyDarkTheme();
        break;
    }
}

void ApplyDarkTheme()
{
    ImGui::StyleColorsDark();

    // Custom overrides for Octave
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Tab] = ImVec4(0.20f, 0.20f, 0.68f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.61f, 0.98f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.12f, 0.50f, 0.47f, 1.00f);
    colors[ImGuiCol_TitleBg] = colors[ImGuiCol_TitleBgActive];
}

void ApplyLightTheme()
{
    ImGui::StyleColorsLight();

    // Custom overrides for Octave
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Tab] = ImVec4(0.40f, 0.40f, 0.78f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.61f, 0.98f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.22f, 0.60f, 0.57f, 1.00f);
    colors[ImGuiCol_TitleBg] = colors[ImGuiCol_TitleBgActive];
}

void ApplyFutureDarkTheme()
{
    // Future Dark style by rewrking from ImThemes
    ImGuiStyle& style = ImGui::GetStyle();

    style.Alpha = 1.0f;
    style.DisabledAlpha = 1.0f;
    style.WindowPadding = ImVec2(12.0f, 12.0f);
    style.WindowRounding = 0.0f;
    style.WindowBorderSize = 0.0f;
    style.WindowMinSize = ImVec2(20.0f, 20.0f);
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_None;
    style.ChildRounding = 0.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupRounding = 0.0f;
    style.PopupBorderSize = 1.0f;
    style.FramePadding = ImVec2(6.0f, 6.0f);
    style.FrameRounding = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.ItemSpacing = ImVec2(12.0f, 6.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 3.0f);
    style.CellPadding = ImVec2(12.0f, 6.0f);
    style.IndentSpacing = 20.0f;
    style.ColumnsMinSpacing = 6.0f;
    style.ScrollbarSize = 12.0f;
    style.ScrollbarRounding = 0.0f;
    style.GrabMinSize = 12.0f;
    style.GrabRounding = 0.0f;
    style.TabRounding = 0.0f;
    style.TabBorderSize = 0.0f;
    style.TabMinWidthForCloseButton = 0.0f;
    style.ColorButtonPosition = ImGuiDir_Right;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

    style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.27450982f, 0.31764707f, 0.4509804f, 1.0f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.078431375f, 0.08627451f, 0.101960786f, 1.0f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.078431375f, 0.08627451f, 0.101960786f, 1.0f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.078431375f, 0.08627451f, 0.101960786f, 1.0f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.15686275f, 0.16862746f, 0.19215687f, 1.0f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.078431375f, 0.08627451f, 0.101960786f, 1.0f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.11764706f, 0.13333334f, 0.14901961f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.15686275f, 0.16862746f, 0.19215687f, 1.0f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.23529412f, 0.21568628f, 0.59607846f, 1.0f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.047058824f, 0.05490196f, 0.07058824f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.047058824f, 0.05490196f, 0.07058824f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.078431375f, 0.08627451f, 0.101960786f, 1.0f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.09803922f, 0.105882354f, 0.12156863f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.047058824f, 0.05490196f, 0.07058824f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.11764706f, 0.13333334f, 0.14901961f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.15686275f, 0.16862746f, 0.19215687f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.11764706f, 0.13333334f, 0.14901961f, 1.0f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.49803922f, 0.5137255f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.49803922f, 0.5137255f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.5372549f, 0.5529412f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.11764706f, 0.13333334f, 0.14901961f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.19607843f, 0.1764706f, 0.54509807f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.23529412f, 0.21568628f, 0.59607846f, 1.0f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.11764706f, 0.13333334f, 0.14901961f, 1.0f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.19607843f, 0.1764706f, 0.54509807f, 1.0f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.23529412f, 0.21568628f, 0.59607846f, 1.0f);
    style.Colors[ImGuiCol_Separator] = ImVec4(0.15686275f, 0.18431373f, 0.2509804f, 1.0f);
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.15686275f, 0.18431373f, 0.2509804f, 1.0f);
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.15686275f, 0.18431373f, 0.2509804f, 1.0f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.11764706f, 0.13333334f, 0.14901961f, 1.0f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.19607843f, 0.1764706f, 0.54509807f, 1.0f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.23529412f, 0.21568628f, 0.59607846f, 1.0f);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.047058824f, 0.05490196f, 0.07058824f, 1.0f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.11764706f, 0.13333334f, 0.14901961f, 1.0f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.09803922f, 0.105882354f, 0.12156863f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.047058824f, 0.05490196f, 0.07058824f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.078431375f, 0.08627451f, 0.101960786f, 1.0f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.52156866f, 0.6f, 0.7019608f, 1.0f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.039215688f, 0.98039216f, 0.98039216f, 1.0f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(1.0f, 0.2901961f, 0.59607846f, 1.0f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.99607843f, 0.4745098f, 0.69803923f, 1.0f);
    style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.047058824f, 0.05490196f, 0.07058824f, 1.0f);
    style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.047058824f, 0.05490196f, 0.07058824f, 1.0f);
    style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.11764706f, 0.13333334f, 0.14901961f, 1.0f);
    style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.09803922f, 0.105882354f, 0.12156863f, 1.0f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.23529412f, 0.21568628f, 0.59607846f, 1.0f);
    style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.49803922f, 0.5137255f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.49803922f, 0.5137255f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.49803922f, 0.5137255f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.19607843f, 0.1764706f, 0.54509807f, 0.5019608f);
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.19607843f, 0.1764706f, 0.54509807f, 0.5019608f);
}

void ApplyClassicTheme()
{
    ImGui::StyleColorsClassic();

    // Custom overrides for Octave
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_TitleBg] = colors[ImGuiCol_TitleBgActive];
}

} // namespace EditorTheme

#endif
