#pragma once

#if EDITOR

#include "../PreferencesModule.h"
#include <string>

/**
 * @brief Preferences module for configuring external code editors.
 *
 * Allows users to configure paths and command-line arguments for
 * Lua script editors and C++ IDE/editors used to open addon source.
 *
 * Settings are stored in user preferences and persist across projects.
 */
class EditorsModule : public PreferencesModule
{
public:
    DECLARE_PREFERENCES_MODULE(EditorsModule)

    EditorsModule();
    virtual ~EditorsModule();

    virtual const char* GetName() const override { return GetStaticName(); }
    virtual const char* GetParentPath() const override { return GetStaticParentPath(); }
    virtual void Render() override;
    virtual void LoadSettings(const rapidjson::Document& doc) override;
    virtual void SaveSettings(rapidjson::Document& doc) override;

    /** @brief Path to Lua editor executable */
    std::string mLuaEditorPath;

    /** @brief Command-line arguments for Lua editor. Supports: {editor}, {file}, {filedir} */
    std::string mLuaEditorArgs = "{editor} {file}";

    /** @brief Path to C++ editor/IDE executable */
    std::string mCppEditorPath;

    /** @brief Command-line arguments for C++ editor. Supports: {editor}, {file}, {filedir}, {project} */
    std::string mCppEditorArgs = "{editor} {project}";

    /** @brief Use internal editor (WIP, non-functional) */
    bool mUseInternalEditor = false;

    /** @brief Check if a Lua editor path is configured. */
    bool IsLuaEditorConfigured() const;

    /** @brief Check if a C++ editor path is configured. */
    bool IsCppEditorConfigured() const;

    /**
     * @brief Build the command string for opening a Lua script.
     * @param filePath Full path to the .lua file
     * @return The complete command string ready for SYS_Exec
     */
    std::string BuildLuaOpenCommand(const std::string& filePath) const;

    /**
     * @brief Build the command string for opening a C++ file/project.
     * @param filePath Full path to the source file
     * @param projectPath Full path to the .vcxproj file
     * @return The complete command string ready for SYS_Exec
     */
    std::string BuildCppOpenCommand(const std::string& filePath, const std::string& projectPath) const;

    /**
     * @brief Open a Lua script in the configured editor.
     * @param filePath Full path to the .lua file
     */
    void OpenLuaScript(const std::string& filePath);

    /**
     * @brief Open a C++ file/project in the configured editor.
     * @param filePath Full path to the source file
     * @param vcxprojPath Full path to the .vcxproj file
     */
    void OpenCppFile(const std::string& filePath, const std::string& vcxprojPath);

private:
    bool DrawPathInput(const char* label, std::string& path, const char* dialogTitle);
    static void ReplaceAll(std::string& str, const std::string& from, const std::string& to);
};

#endif
