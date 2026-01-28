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

private:
    void DrawSidebar();
    void DrawContent();
    void DrawFooter();
    void DrawModuleTree(PreferencesModule* module);

    bool mIsOpen = false;
    PreferencesModule* mSelectedModule = nullptr;
    bool mPendingClose = false;
};

PreferencesWindow* GetPreferencesWindow();

#endif
