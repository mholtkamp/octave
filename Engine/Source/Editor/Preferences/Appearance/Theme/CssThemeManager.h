#pragma once

#if EDITOR

#include "CssThemeParser.h"
#include <string>
#include <vector>

struct CustomThemeEntry
{
    std::string Name;
    std::string FileName;       // filename within Preferences/Themes/ directory
    CssThemeData ParsedData;
    bool IsValid = false;
};

class CssThemeManager
{
public:
    static CssThemeManager& Get();

    // Import a CSS file as a new custom theme
    bool ImportTheme(const std::string& filePath, const std::string& name);

    // Remove a custom theme by index
    void RemoveTheme(int index);

    // Re-parse a theme from its saved CSS file
    bool ReimportTheme(int index);

    // Re-import a theme from a new external CSS file (replaces the stored copy)
    bool ReimportThemeFromFile(int index, const std::string& filePath);

    // Apply a custom theme by index
    void ApplyTheme(int index);

    // Persistence
    void LoadThemeList();
    void SaveThemeList();

    // Accessors
    int GetThemeCount() const { return static_cast<int>(mThemes.size()); }
    const std::vector<CustomThemeEntry>& GetThemes() const { return mThemes; }
    const std::string& GetThemeName(int index) const;

private:
    CssThemeManager() = default;

    std::string GetThemesDirectory() const;
    std::string GetThemeListPath() const;

    std::vector<CustomThemeEntry> mThemes;
};

#endif
