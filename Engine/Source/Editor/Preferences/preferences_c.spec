Overview

 Implement a modern, extensible Preferences Window system with JSON-based settings and tree-structured sidebar.

 File Structure

 Engine/Source/Editor/Preferences/
     PreferencesManager.h/cpp     # Singleton manager (follows ActionManager pattern)
     PreferencesModule.h/cpp      # Abstract base class for modules
     PreferencesWindow.h/cpp      # ImGui window with tree sidebar
     JsonSettings.h/cpp           # RapidJSON wrapper utilities

     General/
         GeneralModule.h/cpp      # General settings module

     Appearance/
         AppearanceModule.h/cpp   # Appearance settings
         Viewport/
             ViewportModule.h/cpp # Viewport-specific settings (nested under Appearance)

 Implementation Steps

 Step 1: JsonSettings Utility

 Create JsonSettings.h/cpp with RapidJSON helpers:
 - LoadFromFile(path, doc) / SaveToFile(path, doc)
 - Typed getters with defaults: GetBool, GetInt, GetFloat, GetString, GetVec3, GetVec4
 - Typed setters: SetBool, SetInt, SetFloat, SetString, SetVec3, SetVec4
 - Storage path: Engine/Saves/Preferences/
 - Include: External/Assimp/contrib/rapidjson/include/

 Step 2: PreferencesModule Base Class

 Create abstract PreferencesModule.h/cpp:
 class PreferencesModule {
 public:
     virtual const char* GetName() const = 0;
     virtual const char* GetParentPath() const { return ""; }
     virtual void Render() = 0;
     virtual void LoadSettings(const rapidjson::Document& doc);
     virtual void SaveSettings(rapidjson::Document& doc);

     // Hierarchy management
     void AddSubModule(PreferencesModule* sub);
     const std::vector<PreferencesModule*>& GetSubModules() const;
     std::string GetFullPath() const;
     std::string GetSettingsFilePath() const;
     bool IsDirty() const;
     void SetDirty(bool dirty);
 };

 Registration macros (following Factory.h pattern):
 #define DECLARE_PREFERENCES_MODULE(Class) \
     static const char* GetStaticName(); \
     static const char* GetStaticParentPath();

 #define DEFINE_PREFERENCES_MODULE(Class, Name, ParentPath) \
     const char* Class::GetStaticName() { return Name; } \
     const char* Class::GetStaticParentPath() { return ParentPath; }

 Step 3: PreferencesManager Singleton

 Create PreferencesManager.h/cpp following ActionManager pattern:
 class PreferencesManager {
 public:
     static void Create();
     static void Destroy();
     static PreferencesManager* Get();

     void RegisterModule(PreferencesModule* module);
     const std::vector<PreferencesModule*>& GetRootModules() const;
     PreferencesModule* FindModule(const std::string& path) const;

     void LoadAllSettings();
     void SaveAllSettings();
     std::string GetPreferencesDirectory() const;

 protected:
     static PreferencesManager* sInstance;
     std::vector<PreferencesModule*> mRootModules;
 };

 Step 4: PreferencesWindow UI

 Create PreferencesWindow.h/cpp with modern ImGui layout:

 Window Layout:
 +------------------------------------------------------------------+
 |                        Preferences                          [X]  |
 +------------------------------------------------------------------+
 | > General            |  [Selected Module Content]               |
 | v Appearance         |                                           |
 |     Viewport         |  Settings rendered by module's Render()  |
 |                      |                                           |
 +------------------------------------------------------------------+
 |                                    [Apply]  [Cancel]  [OK]       |
 +------------------------------------------------------------------+

 Key implementation:
 - Centered modal window (700x500)
 - Left sidebar (200px) with tree using TreeNodeEx() + ImGuiTreeNodeFlags_OpenOnArrow
 - Right panel renders selected module's Render() method
 - Footer buttons: Apply (save), Cancel (discard), OK (save & close)

 Step 5: Initial Modules

 GeneralModule (General/GeneralModule.h/cpp):
 - Auto-save settings toggle
 - Recent projects limit (integer slider)
 - Show welcome screen toggle

 AppearanceModule (Appearance/AppearanceModule.h/cpp):
 - Theme selection (future)
 - UI scale (migrate from ViewPopup)

 ViewportModule (Appearance/Viewport/ViewportModule.h/cpp):
 - Background color (color picker)
 - Grid visibility toggle
 - Grid color (color picker)
 - Grid size (float slider)

 Step 6: Editor Integration

 EditorMain.cpp - Add lifecycle:
 // After line 53 (InputManager::Create())
 PreferencesManager::Create();

 // Before line 119 (GetEditorState()->Shutdown())
 PreferencesManager::Destroy();

 EditorImgui.cpp - Add menu item and draw call:
 // In ViewPopup (around line 4059, before EndPopup)
 if (ImGui::Selectable("Preferences..."))
     GetPreferencesWindow()->Open();

 // In EditorImguiDraw()
 GetPreferencesWindow()->Draw();

 JSON Settings Schema

 Each module saves to Engine/Saves/Preferences/{ModulePath}.json:
 // General.json
 { "version": 1, "autoSave": true, "recentProjectsLimit": 10 }

 // Appearance.json
 { "version": 1, "uiScale": 1.0 }

 // Appearance_Viewport.json
 { "version": 1, "backgroundColor": [0.1,0.1,0.1,1.0], "showGrid": true }

 Verification

 1. Build the editor and verify no compilation errors
 2. Open editor, click View > Preferences
 3. Verify tree sidebar shows: General, Appearance (expandable), Viewport (under Appearance)
 4. Change settings in each module, click Apply
 5. Close and reopen Preferences - verify settings persisted
 6. Check Engine/Saves/Preferences/ for JSON files

 Key Files to Modify

 - Engine/Source/Editor/EditorMain.cpp:52-53 - Add PreferencesManager lifecycle
 - Engine/Source/Editor/EditorImgui.cpp:4059,4880 - Add menu item and draw call

 Pattern References

 - ActionManager.h:27-35 - Singleton Create/Destroy/Get pattern
 - Factory.h:90-109 - Registration macro pattern
 - EditorImgui.cpp:4011-4062 - ViewPopup menu pattern