## Plan: Preferences Window with JSON Settings & Tree Hierarchy

Update the spec for a Preferences Window with a JSON-based settings system, tree-structured sidebar (sub-tabs only in the tree), and initial modules: General, Appearance, and Appearance/Viewport.

### Steps
1. **Preferences Core & Base Class**
   - In [Preferences.h/cpp](Engine/Source/Editor/Preferences/Preferences.h), define `PreferencesWindow` and an abstract `PreferencesModule` base class.
   - Base class provides: `GetName()`, `GetSubModules()`, `Render()`, `LoadSettings()`, `SaveSettings()`.
   - Include JSON helpers for per-module settings (e.g., using nlohmann/json or similar).

2. **Tree-View Sidebar**
   - Implement a tree-view sidebar (ImGui) for modules and submodules.
   - Only the sidebar supports nesting; main panel shows the selected module’s UI.

3. **Module Structure & Registration**
   - Each module in its own subfolder, e.g., [Preferences/General/](Engine/Source/Editor/Preferences/General/), [Preferences/Appearance/](Engine/Source/Editor/Preferences/Appearance/), [Preferences/Appearance/Viewport/](Engine/Source/Editor/Preferences/Appearance/Viewport/).
   - Modules extend `PreferencesModule`, implement required methods, and register via macro/static function.

4. **Per-Module JSON Settings**
   - Each module loads/saves its settings to a dedicated JSON file (e.g., Preferences/General/settings.json).
   - Base class provides JSON helpers for common operations.

5. **Initial Modules**
   - Implement General, Appearance, and Appearance/Viewport modules as examples/templates.

### Further Considerations
1. Use a consistent JSON schema for settings files.
2. Allow easy extension for future modules/submodules.
3. Document the registration and settings API for developers.