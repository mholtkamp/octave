#if EDITOR

#include "CssThemeParser.h"
#include "imgui_dock.h"
#include "System/System.h"
#include "Log.h"

#include <algorithm>
#include <sstream>
#include <cctype>

// ---- Helpers ----

static std::string TrimString(const std::string& s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static std::string ToLower(const std::string& s)
{
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return (char)std::tolower(c); });
    return result;
}

static std::string StripComments(const std::string& css)
{
    std::string result;
    result.reserve(css.size());
    size_t i = 0;
    while (i < css.size())
    {
        if (i + 1 < css.size() && css[i] == '/' && css[i + 1] == '*')
        {
            i += 2;
            while (i + 1 < css.size() && !(css[i] == '*' && css[i + 1] == '/'))
                i++;
            i += 2; // skip */
        }
        else if (i + 1 < css.size() && css[i] == '/' && css[i + 1] == '/')
        {
            // Single-line comments (non-standard CSS but convenient)
            while (i < css.size() && css[i] != '\n')
                i++;
        }
        else
        {
            result += css[i];
            i++;
        }
    }
    return result;
}

// ---- Selector->ImGuiCol Mapping ----

static const SelectorMapping sSelectorMappings[] =
{
    { "window",                 "background",            ImGuiCol_WindowBg },
    { "window",                 "child-background",      ImGuiCol_ChildBg },
    { "popup",                  "background",            ImGuiCol_PopupBg },
    { "border",                 "color",                 ImGuiCol_Border },
    { "border",                 "shadow",                ImGuiCol_BorderShadow },
    { "text",                   "color",                 ImGuiCol_Text },
    { "text",                   "disabled",              ImGuiCol_TextDisabled },
    { "text",                   "selected-background",   ImGuiCol_TextSelectedBg },
    { "frame",                  "background",            ImGuiCol_FrameBg },
    { "frame:hover",            "background",            ImGuiCol_FrameBgHovered },
    { "frame:active",           "background",            ImGuiCol_FrameBgActive },
    { "title",                  "background",            ImGuiCol_TitleBg },
    { "title:active",           "background",            ImGuiCol_TitleBgActive },
    { "title:collapsed",        "background",            ImGuiCol_TitleBgCollapsed },
    { "menubar",                "background",            ImGuiCol_MenuBarBg },
    { "scrollbar",              "background",            ImGuiCol_ScrollbarBg },
    { "scrollbar",              "grab",                  ImGuiCol_ScrollbarGrab },
    { "scrollbar:hover",        "grab",                  ImGuiCol_ScrollbarGrabHovered },
    { "scrollbar:active",       "grab",                  ImGuiCol_ScrollbarGrabActive },
    { "button",                 "background",            ImGuiCol_Button },
    { "button:hover",           "background",            ImGuiCol_ButtonHovered },
    { "button:active",          "background",            ImGuiCol_ButtonActive },
    { "header",                 "background",            ImGuiCol_Header },
    { "header:hover",           "background",            ImGuiCol_HeaderHovered },
    { "header:active",          "background",            ImGuiCol_HeaderActive },
    { "separator",              "color",                 ImGuiCol_Separator },
    { "separator:hover",        "color",                 ImGuiCol_SeparatorHovered },
    { "separator:active",       "color",                 ImGuiCol_SeparatorActive },
    { "resize-grip",            "background",            ImGuiCol_ResizeGrip },
    { "resize-grip:hover",      "background",            ImGuiCol_ResizeGripHovered },
    { "resize-grip:active",     "background",            ImGuiCol_ResizeGripActive },
    { "tab",                    "background",            ImGuiCol_Tab },
    { "tab:hover",              "background",            ImGuiCol_TabHovered },
    { "tab:active",             "background",            ImGuiCol_TabActive },
    { "tab:unfocused",          "background",            ImGuiCol_TabUnfocused },
    { "tab:unfocused-active",   "background",            ImGuiCol_TabUnfocusedActive },
    { "checkbox",               "checkmark",             ImGuiCol_CheckMark },
    { "slider",                 "grab",                  ImGuiCol_SliderGrab },
    { "slider:active",          "grab",                  ImGuiCol_SliderGrabActive },
    { "plot",                   "lines",                 ImGuiCol_PlotLines },
    { "plot:hover",             "lines",                 ImGuiCol_PlotLinesHovered },
    { "plot",                   "histogram",             ImGuiCol_PlotHistogram },
    { "plot:hover",             "histogram",             ImGuiCol_PlotHistogramHovered },
    { "table",                  "header-background",     ImGuiCol_TableHeaderBg },
    { "table",                  "border-strong",         ImGuiCol_TableBorderStrong },
    { "table",                  "border-light",          ImGuiCol_TableBorderLight },
    { "table",                  "row-background",        ImGuiCol_TableRowBg },
    { "table",                  "row-background-alt",    ImGuiCol_TableRowBgAlt },
    { "drag-drop",              "target",                ImGuiCol_DragDropTarget },
    { "nav",                    "highlight",             ImGuiCol_NavHighlight },
    { "nav",                    "windowing-highlight",   ImGuiCol_NavWindowingHighlight },
    { "nav",                    "windowing-dim",         ImGuiCol_NavWindowingDimBg },
    { "modal",                  "dim-background",        ImGuiCol_ModalWindowDimBg },
};

static const int sSelectorMappingCount = sizeof(sSelectorMappings) / sizeof(sSelectorMappings[0]);

// ---- Color Parsing ----

static int HexCharToInt(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

namespace CssThemeParser
{

bool ParseHexColor(const std::string& hex, ImVec4& outColor)
{
    std::string h = hex;
    if (!h.empty() && h[0] == '#')
        h = h.substr(1);

    if (h.size() == 3)
    {
        // #RGB -> #RRGGBB
        int r = HexCharToInt(h[0]);
        int g = HexCharToInt(h[1]);
        int b = HexCharToInt(h[2]);
        outColor = ImVec4(
            (r * 17) / 255.0f,
            (g * 17) / 255.0f,
            (b * 17) / 255.0f,
            1.0f);
        return true;
    }
    else if (h.size() == 6)
    {
        int r = HexCharToInt(h[0]) * 16 + HexCharToInt(h[1]);
        int g = HexCharToInt(h[2]) * 16 + HexCharToInt(h[3]);
        int b = HexCharToInt(h[4]) * 16 + HexCharToInt(h[5]);
        outColor = ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
        return true;
    }
    else if (h.size() == 8)
    {
        int r = HexCharToInt(h[0]) * 16 + HexCharToInt(h[1]);
        int g = HexCharToInt(h[2]) * 16 + HexCharToInt(h[3]);
        int b = HexCharToInt(h[4]) * 16 + HexCharToInt(h[5]);
        int a = HexCharToInt(h[6]) * 16 + HexCharToInt(h[7]);
        outColor = ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
        return true;
    }

    return false;
}

bool ParseRgbColor(const std::string& rgb, ImVec4& outColor)
{
    // rgb(r, g, b)
    size_t start = rgb.find('(');
    size_t end = rgb.find(')');
    if (start == std::string::npos || end == std::string::npos)
        return false;

    std::string inner = rgb.substr(start + 1, end - start - 1);
    std::istringstream ss(inner);
    std::string token;
    float values[3] = { 0, 0, 0 };
    int idx = 0;

    while (std::getline(ss, token, ',') && idx < 3)
    {
        values[idx++] = std::stof(TrimString(token)) / 255.0f;
    }

    if (idx == 3)
    {
        outColor = ImVec4(values[0], values[1], values[2], 1.0f);
        return true;
    }
    return false;
}

bool ParseRgbaColor(const std::string& rgba, ImVec4& outColor)
{
    // rgba(r, g, b, a)
    size_t start = rgba.find('(');
    size_t end = rgba.find(')');
    if (start == std::string::npos || end == std::string::npos)
        return false;

    std::string inner = rgba.substr(start + 1, end - start - 1);
    std::istringstream ss(inner);
    std::string token;
    float values[4] = { 0, 0, 0, 1 };
    int idx = 0;

    while (std::getline(ss, token, ',') && idx < 4)
    {
        std::string trimmed = TrimString(token);
        if (idx < 3)
            values[idx] = std::stof(trimmed) / 255.0f;
        else
            values[idx] = std::stof(trimmed); // alpha is 0-1
        idx++;
    }

    if (idx == 4)
    {
        outColor = ImVec4(values[0], values[1], values[2], values[3]);
        return true;
    }
    return false;
}

float ParsePxValue(const std::string& value)
{
    std::string v = TrimString(value);
    // Remove "px" suffix if present
    if (v.size() > 2 && v.substr(v.size() - 2) == "px")
        v = v.substr(0, v.size() - 2);
    v = TrimString(v);
    try { return std::stof(v); }
    catch (...) { return 0.0f; }
}

// ---- Internal Parsing ----

static bool ResolveColor(const std::string& value,
    const std::unordered_map<std::string, std::string>& variables,
    ImVec4& outColor)
{
    std::string v = TrimString(value);
    std::string lower = ToLower(v);

    if (lower == "transparent")
    {
        outColor = ImVec4(0, 0, 0, 0);
        return true;
    }

    // Resolve var() references
    if (lower.substr(0, 4) == "var(")
    {
        size_t end = v.find(')');
        if (end != std::string::npos)
        {
            std::string varName = TrimString(v.substr(4, end - 4));
            auto it = variables.find(varName);
            if (it != variables.end())
            {
                return ResolveColor(it->second, variables, outColor);
            }
            else
            {
                LogWarning("CSS Theme: Unknown variable '%s'", varName.c_str());
                return false;
            }
        }
        return false;
    }

    if (v[0] == '#')
        return ParseHexColor(v, outColor);

    if (lower.substr(0, 5) == "rgba(")
        return ParseRgbaColor(v, outColor);

    if (lower.substr(0, 4) == "rgb(")
        return ParseRgbColor(v, outColor);

    return false;
}

static ImVec2 ParseVec2Value(const std::string& value)
{
    std::string v = TrimString(value);
    // "6px 4px" or "6px"
    std::istringstream ss(v);
    std::string token;
    float values[2] = { 0, 0 };
    int idx = 0;
    while (ss >> token && idx < 2)
    {
        values[idx++] = ParsePxValue(token);
    }
    if (idx == 1)
        values[1] = values[0];
    return ImVec2(values[0], values[1]);
}

static void ParseStyleBlock(const std::string& body, CssThemeData& theme)
{
    std::istringstream ss(body);
    std::string line;
    while (std::getline(ss, line, ';'))
    {
        line = TrimString(line);
        if (line.empty()) continue;

        size_t colon = line.find(':');
        if (colon == std::string::npos) continue;

        std::string prop = TrimString(ToLower(line.substr(0, colon)));
        std::string val = TrimString(line.substr(colon + 1));

        if (prop == "alpha")                  { theme.Alpha = std::stof(val);                 theme.hasAlpha = true; }
        else if (prop == "disabled-alpha")     { theme.DisabledAlpha = std::stof(val);          theme.hasDisabledAlpha = true; }
        else if (prop == "window-padding")     { theme.WindowPadding = ParseVec2Value(val);     theme.hasWindowPadding = true; }
        else if (prop == "window-rounding")    { theme.WindowRounding = ParsePxValue(val);      theme.hasWindowRounding = true; }
        else if (prop == "window-border-size") { theme.WindowBorderSize = ParsePxValue(val);    theme.hasWindowBorderSize = true; }
        else if (prop == "window-min-size")    { theme.WindowMinSize = ParseVec2Value(val);     theme.hasWindowMinSize = true; }
        else if (prop == "window-title-align") { theme.WindowTitleAlign = ParseVec2Value(val);  theme.hasWindowTitleAlign = true; }
        else if (prop == "child-rounding")     { theme.ChildRounding = ParsePxValue(val);       theme.hasChildRounding = true; }
        else if (prop == "child-border-size")  { theme.ChildBorderSize = ParsePxValue(val);     theme.hasChildBorderSize = true; }
        else if (prop == "popup-rounding")     { theme.PopupRounding = ParsePxValue(val);       theme.hasPopupRounding = true; }
        else if (prop == "popup-border-size")  { theme.PopupBorderSize = ParsePxValue(val);     theme.hasPopupBorderSize = true; }
        else if (prop == "frame-padding")      { theme.FramePadding = ParseVec2Value(val);      theme.hasFramePadding = true; }
        else if (prop == "frame-rounding")     { theme.FrameRounding = ParsePxValue(val);       theme.hasFrameRounding = true; }
        else if (prop == "frame-border-size")  { theme.FrameBorderSize = ParsePxValue(val);     theme.hasFrameBorderSize = true; }
        else if (prop == "item-spacing")       { theme.ItemSpacing = ParseVec2Value(val);       theme.hasItemSpacing = true; }
        else if (prop == "item-inner-spacing") { theme.ItemInnerSpacing = ParseVec2Value(val);  theme.hasItemInnerSpacing = true; }
        else if (prop == "cell-padding")       { theme.CellPadding = ParseVec2Value(val);       theme.hasCellPadding = true; }
        else if (prop == "indent-spacing")     { theme.IndentSpacing = ParsePxValue(val);       theme.hasIndentSpacing = true; }
        else if (prop == "columns-min-spacing"){ theme.ColumnsMinSpacing = ParsePxValue(val);   theme.hasColumnsMinSpacing = true; }
        else if (prop == "scrollbar-size")     { theme.ScrollbarSize = ParsePxValue(val);       theme.hasScrollbarSize = true; }
        else if (prop == "scrollbar-rounding") { theme.ScrollbarRounding = ParsePxValue(val);   theme.hasScrollbarRounding = true; }
        else if (prop == "grab-min-size")      { theme.GrabMinSize = ParsePxValue(val);         theme.hasGrabMinSize = true; }
        else if (prop == "grab-rounding")      { theme.GrabRounding = ParsePxValue(val);        theme.hasGrabRounding = true; }
        else if (prop == "tab-rounding")       { theme.TabRounding = ParsePxValue(val);         theme.hasTabRounding = true; }
        else if (prop == "tab-border-size")    { theme.TabBorderSize = ParsePxValue(val);       theme.hasTabBorderSize = true; }
        else if (prop == "button-text-align")  { theme.ButtonTextAlign = ParseVec2Value(val);   theme.hasButtonTextAlign = true; }
        else if (prop == "selectable-text-align") { theme.SelectableTextAlign = ParseVec2Value(val); theme.hasSelectableTextAlign = true; }
    }
}

static void ParseRootBlock(const std::string& body,
    std::unordered_map<std::string, std::string>& variables)
{
    std::istringstream ss(body);
    std::string line;
    while (std::getline(ss, line, ';'))
    {
        line = TrimString(line);
        if (line.empty()) continue;

        size_t colon = line.find(':');
        if (colon == std::string::npos) continue;

        std::string name = TrimString(line.substr(0, colon));
        std::string value = TrimString(line.substr(colon + 1));

        if (!name.empty() && name.substr(0, 2) == "--")
        {
            variables[name] = value;
        }
    }
}

static void ParseSelectorBlock(const std::string& selector, const std::string& body,
    const std::unordered_map<std::string, std::string>& variables,
    CssThemeData& theme)
{
    std::string sel = TrimString(ToLower(selector));

    // Parse properties in the block
    std::istringstream ss(body);
    std::string line;
    while (std::getline(ss, line, ';'))
    {
        line = TrimString(line);
        if (line.empty()) continue;

        size_t colon = line.find(':');
        if (colon == std::string::npos) continue;

        std::string prop = TrimString(ToLower(line.substr(0, colon)));
        std::string val = TrimString(line.substr(colon + 1));

        // Special: tab { color } sets dock tab text color
        if (sel == "tab" && prop == "color")
        {
            ImVec4 color;
            if (ResolveColor(val, variables, color))
            {
                theme.DockTabTextColor = color;
                theme.hasDockTabTextColor = true;
            }
            continue;
        }

        // Dock splitter colors
        if (sel == "panel-border" && prop == "color")
        {
            ImVec4 color;
            if (ResolveColor(val, variables, color))
            {
                theme.DockSplitterColor = color;
                theme.hasDockSplitterColor = true;
            }
            continue;
        }
        if (sel == "panel-border:hover" && prop == "color")
        {
            ImVec4 color;
            if (ResolveColor(val, variables, color))
            {
                theme.DockSplitterHoverColor = color;
                theme.hasDockSplitterHoverColor = true;
            }
            continue;
        }

        // Dock tab bar background
        if (sel == "dock-tabbar" && prop == "background")
        {
            ImVec4 color;
            if (ResolveColor(val, variables, color))
            {
                theme.DockTabBarBg = color;
                theme.hasDockTabBarBg = true;
            }
            continue;
        }

        // Panel background overrides
        if (prop == "background")
        {
            ImVec4 color;
            if (sel == "panel-assets" && ResolveColor(val, variables, color))
            {
                theme.PanelAssetsBg = color;
                theme.hasPanelAssetsBg = true;
                continue;
            }
            else if (sel == "panel-scene" && ResolveColor(val, variables, color))
            {
                theme.PanelSceneBg = color;
                theme.hasPanelSceneBg = true;
                continue;
            }
            else if (sel == "panel-properties" && ResolveColor(val, variables, color))
            {
                theme.PanelPropertiesBg = color;
                theme.hasPanelPropertiesBg = true;
                continue;
            }
            else if (sel == "panel-debug-log" && ResolveColor(val, variables, color))
            {
                theme.PanelDebugLogBg = color;
                theme.hasPanelDebugLogBg = true;
                continue;
            }
        }

        // Find matching selector+property in our mappings
        for (int i = 0; i < sSelectorMappingCount; ++i)
        {
            if (sel == sSelectorMappings[i].selector && prop == sSelectorMappings[i].property)
            {
                ImVec4 color;
                if (ResolveColor(val, variables, color))
                {
                    theme.Colors[sSelectorMappings[i].imguiCol] = color;
                }
                else
                {
                    LogWarning("CSS Theme: Could not parse color '%s' for %s { %s }",
                        val.c_str(), sel.c_str(), prop.c_str());
                }
                break;
            }
        }
    }
}

// ---- Global Panel Color Storage ----

static ImVec4 sPanelAssetsBg;
static ImVec4 sPanelSceneBg;
static ImVec4 sPanelPropertiesBg;
static ImVec4 sPanelDebugLogBg;
static bool sHasPanelAssetsBg = false;
static bool sHasPanelSceneBg = false;
static bool sHasPanelPropertiesBg = false;
static bool sHasPanelDebugLogBg = false;

// ---- Public API ----

bool ParseFile(const std::string& filePath, CssThemeData& outTheme)
{
    char* data = nullptr;
    uint32_t size = 0;
    SYS_AcquireFileData(filePath.c_str(), false, -1, data, size);

    if (data == nullptr || size == 0)
    {
        LogError("CSS Theme: Failed to read file '%s'", filePath.c_str());
        return false;
    }

    std::string css(data, size);
    delete[] data;

    return ParseString(css, outTheme);
}

bool ParseString(const std::string& css, CssThemeData& outTheme)
{
    outTheme = CssThemeData();

    std::string cleaned = StripComments(css);
    std::unordered_map<std::string, std::string> variables;

    // Parse blocks: selector { body }
    size_t pos = 0;
    while (pos < cleaned.size())
    {
        // Skip whitespace
        while (pos < cleaned.size() && std::isspace((unsigned char)cleaned[pos]))
            pos++;

        if (pos >= cleaned.size())
            break;

        // Find opening brace
        size_t braceOpen = cleaned.find('{', pos);
        if (braceOpen == std::string::npos)
            break;

        std::string selector = TrimString(cleaned.substr(pos, braceOpen - pos));

        // Find matching closing brace
        size_t braceClose = cleaned.find('}', braceOpen);
        if (braceClose == std::string::npos)
            break;

        std::string body = cleaned.substr(braceOpen + 1, braceClose - braceOpen - 1);
        pos = braceClose + 1;

        std::string selectorLower = ToLower(selector);

        if (selectorLower == ":root")
        {
            ParseRootBlock(body, variables);
        }
        else if (selectorLower == "style")
        {
            ParseStyleBlock(body, outTheme);
        }
        else
        {
            ParseSelectorBlock(selector, body, variables, outTheme);
        }
    }

    return true;
}

void ApplyTheme(const CssThemeData& themeData)
{
    // Reset all style properties to defaults, then apply Dark colors as base.
    // StyleColorsDark() only sets colors, NOT style vars like rounding/padding,
    // so without this reset, stale values from a previously applied theme persist.
    ImGui::GetStyle() = ImGuiStyle();
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();

    // Apply style overrides (only those that were specified)
    if (themeData.hasAlpha)              style.Alpha = themeData.Alpha;
    if (themeData.hasDisabledAlpha)      style.DisabledAlpha = themeData.DisabledAlpha;
    if (themeData.hasWindowPadding)      style.WindowPadding = themeData.WindowPadding;
    if (themeData.hasWindowRounding)     style.WindowRounding = themeData.WindowRounding;
    if (themeData.hasWindowBorderSize)   style.WindowBorderSize = themeData.WindowBorderSize;
    if (themeData.hasWindowMinSize)      style.WindowMinSize = themeData.WindowMinSize;
    if (themeData.hasWindowTitleAlign)   style.WindowTitleAlign = themeData.WindowTitleAlign;
    if (themeData.hasChildRounding)      style.ChildRounding = themeData.ChildRounding;
    if (themeData.hasChildBorderSize)    style.ChildBorderSize = themeData.ChildBorderSize;
    if (themeData.hasPopupRounding)      style.PopupRounding = themeData.PopupRounding;
    if (themeData.hasPopupBorderSize)    style.PopupBorderSize = themeData.PopupBorderSize;
    if (themeData.hasFramePadding)       style.FramePadding = themeData.FramePadding;
    if (themeData.hasFrameRounding)      style.FrameRounding = themeData.FrameRounding;
    if (themeData.hasFrameBorderSize)    style.FrameBorderSize = themeData.FrameBorderSize;
    if (themeData.hasItemSpacing)        style.ItemSpacing = themeData.ItemSpacing;
    if (themeData.hasItemInnerSpacing)   style.ItemInnerSpacing = themeData.ItemInnerSpacing;
    if (themeData.hasCellPadding)        style.CellPadding = themeData.CellPadding;
    if (themeData.hasIndentSpacing)      style.IndentSpacing = themeData.IndentSpacing;
    if (themeData.hasColumnsMinSpacing)  style.ColumnsMinSpacing = themeData.ColumnsMinSpacing;
    if (themeData.hasScrollbarSize)      style.ScrollbarSize = themeData.ScrollbarSize;
    if (themeData.hasScrollbarRounding)  style.ScrollbarRounding = themeData.ScrollbarRounding;
    if (themeData.hasGrabMinSize)        style.GrabMinSize = themeData.GrabMinSize;
    if (themeData.hasGrabRounding)       style.GrabRounding = themeData.GrabRounding;
    if (themeData.hasTabRounding)        style.TabRounding = themeData.TabRounding;
    if (themeData.hasTabBorderSize)      style.TabBorderSize = themeData.TabBorderSize;
    if (themeData.hasButtonTextAlign)    style.ButtonTextAlign = themeData.ButtonTextAlign;
    if (themeData.hasSelectableTextAlign) style.SelectableTextAlign = themeData.SelectableTextAlign;

    // Overlay parsed colors
    for (const auto& pair : themeData.Colors)
    {
        if (pair.first >= 0 && pair.first < ImGuiCol_COUNT)
        {
            style.Colors[pair.first] = pair.second;
        }
    }

    // Dock tab text color
    ImGui::ClearDockTabTextColor();
    if (themeData.hasDockTabTextColor)
    {
        ImGui::SetDockTabTextColor(ImGui::ColorConvertFloat4ToU32(themeData.DockTabTextColor));
    }

    // Dock splitter and tab bar colors
    ImGui::ClearDockSplitterColors();
    ImGui::ClearDockTabBarBg();
    if (themeData.hasDockSplitterColor)
        ImGui::SetDockSplitterColor(ImGui::ColorConvertFloat4ToU32(themeData.DockSplitterColor));
    if (themeData.hasDockSplitterHoverColor)
        ImGui::SetDockSplitterHoverColor(ImGui::ColorConvertFloat4ToU32(themeData.DockSplitterHoverColor));
    if (themeData.hasDockTabBarBg)
        ImGui::SetDockTabBarBg(ImGui::ColorConvertFloat4ToU32(themeData.DockTabBarBg));

    // Panel background overrides
    sPanelAssetsBg = themeData.PanelAssetsBg;         sHasPanelAssetsBg = themeData.hasPanelAssetsBg;
    sPanelSceneBg = themeData.PanelSceneBg;           sHasPanelSceneBg = themeData.hasPanelSceneBg;
    sPanelPropertiesBg = themeData.PanelPropertiesBg; sHasPanelPropertiesBg = themeData.hasPanelPropertiesBg;
    sPanelDebugLogBg = themeData.PanelDebugLogBg;     sHasPanelDebugLogBg = themeData.hasPanelDebugLogBg;
}

const SelectorMapping* GetSelectorMappings()
{
    return sSelectorMappings;
}

int GetSelectorMappingCount()
{
    return sSelectorMappingCount;
}

bool GetPanelAssetsBg(ImVec4& outColor)
{
    if (sHasPanelAssetsBg) { outColor = sPanelAssetsBg; return true; }
    return false;
}

bool GetPanelSceneBg(ImVec4& outColor)
{
    if (sHasPanelSceneBg) { outColor = sPanelSceneBg; return true; }
    return false;
}

bool GetPanelPropertiesBg(ImVec4& outColor)
{
    if (sHasPanelPropertiesBg) { outColor = sPanelPropertiesBg; return true; }
    return false;
}

bool GetPanelDebugLogBg(ImVec4& outColor)
{
    if (sHasPanelDebugLogBg) { outColor = sPanelDebugLogBg; return true; }
    return false;
}

void SetPanelColors(const CssThemeData& data)
{
    sPanelAssetsBg = data.PanelAssetsBg;         sHasPanelAssetsBg = data.hasPanelAssetsBg;
    sPanelSceneBg = data.PanelSceneBg;           sHasPanelSceneBg = data.hasPanelSceneBg;
    sPanelPropertiesBg = data.PanelPropertiesBg; sHasPanelPropertiesBg = data.hasPanelPropertiesBg;
    sPanelDebugLogBg = data.PanelDebugLogBg;     sHasPanelDebugLogBg = data.hasPanelDebugLogBg;
}

void ClearPanelColors()
{
    sHasPanelAssetsBg = false;
    sHasPanelSceneBg = false;
    sHasPanelPropertiesBg = false;
    sHasPanelDebugLogBg = false;
}

} // namespace CssThemeParser

#endif
