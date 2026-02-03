#pragma once

#if EDITOR

#include <string>
#include <vector>

enum class EditorThemeType
{
    Dark,
    Light,
    FutureDark,
    Classic,

    Count
};

namespace EditorTheme
{
    // Get list of available theme names for UI
    const std::vector<std::string>& GetThemeNames();

    // Apply a theme by type
    void ApplyTheme(EditorThemeType type);

    // Get theme type from name
    EditorThemeType GetThemeTypeFromName(const std::string& name);

    // Get theme name from type
    const char* GetThemeName(EditorThemeType type);

    // Individual theme application functions (for adding new themes)
    void ApplyDarkTheme();
    void ApplyLightTheme();
    void ApplyFutureDarkTheme();
    void ApplyClassicTheme();
}

#endif
