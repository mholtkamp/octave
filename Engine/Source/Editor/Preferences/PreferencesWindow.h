#pragma once

#if EDITOR

#include <string>

class PreferencesModule;

class PreferencesWindow
{
public:
    PreferencesWindow();
    ~PreferencesWindow();

    void Open();
    void Close();
    void Draw();

    bool IsOpen() const { return mIsOpen; }

    /**
     * @brief Selects a module by its full path.
     * @param path The full path to the module (e.g., "External/Launchers")
     */
    void SelectModule(const std::string& path);

private:
    void DrawSidebar();
    void DrawContent();
    void DrawFooter();
    void DrawModuleTree(PreferencesModule* module);

    bool mIsOpen = false;
    PreferencesModule* mSelectedModule = nullptr;
    bool mPendingClose = false;
    std::string mSelectedAddonPanel;  // Non-empty if an addon panel is selected
};

PreferencesWindow* GetPreferencesWindow();

#endif
