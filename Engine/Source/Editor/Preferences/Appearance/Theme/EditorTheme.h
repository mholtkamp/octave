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

    Count,

    Custom = 100  // Custom themes use indices 100+
};

// Helpers for custom theme types
inline bool IsCustomTheme(EditorThemeType type) { return static_cast<int>(type) >= static_cast<int>(EditorThemeType::Custom); }
inline int GetCustomThemeIndex(EditorThemeType type) { return static_cast<int>(type) - static_cast<int>(EditorThemeType::Custom); }
inline EditorThemeType MakeCustomThemeType(int customIndex) { return static_cast<EditorThemeType>(static_cast<int>(EditorThemeType::Custom) + customIndex); }

namespace EditorTheme
{
    // Get list of available theme names for UI (built-in + custom)
    const std::vector<std::string>& GetThemeNames();

    // Rebuild theme names list (call after importing/removing custom themes)
    void RefreshThemeNames();

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
