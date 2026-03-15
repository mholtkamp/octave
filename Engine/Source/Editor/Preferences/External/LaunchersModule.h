#pragma once

#if EDITOR

#include "../PreferencesModule.h"
#include "EngineTypes.h"
#include <string>

/**
 * @brief Preferences module for configuring external emulator and tool paths.
 *
 * This module allows users to configure paths and command-line arguments
 * for emulators (Dolphin, Azahar/Citra) and tools (3dslink) used in
 * the Build & Run workflow.
 *
 * Settings are stored in user preferences and persist across projects.
 */
class LaunchersModule : public PreferencesModule
{
public:
    DECLARE_PREFERENCES_MODULE(LaunchersModule)

    LaunchersModule();
    virtual ~LaunchersModule();

    virtual const char* GetName() const override { return GetStaticName(); }
    virtual const char* GetParentPath() const override { return GetStaticParentPath(); }
    virtual void Render() override;
    virtual void LoadSettings(const rapidjson::Document& doc) override;
    virtual void SaveSettings(rapidjson::Document& doc) override;

    /** @brief Path to Dolphin emulator executable (GameCube/Wii) */
    std::string mDolphinPath;

    /** @brief Command-line arguments for Dolphin. Supports placeholders: {emulator}, {output}, {outputdir} */
    std::string mDolphinArgs = "{emulator} --batch -e {output}";

    /** @brief Path to Azahar/Citra emulator executable (3DS) */
    std::string mAzaharPath;

    /** @brief Command-line arguments for Azahar. Supports placeholders: {emulator}, {output}, {outputdir} */
    std::string mAzaharArgs = "{emulator} {output}";

    /**
     * @brief Checks if an emulator is configured for the given platform.
     * @param platform The target platform
     * @return True if the emulator path is set for that platform
     */
    bool IsEmulatorConfigured(Platform platform) const;

    /**
     * @brief Checks if 3dslink is configured.
     * @return True if 3dslink can be used
     */
    bool Is3dsLinkConfigured() const;

    /**
     * @brief Builds the full launch command with placeholder substitution.
     * @param platform The target platform
     * @param outputPath Full path to the built executable
     * @return The complete command string ready for SYS_Exec
     */
    std::string BuildLaunchCommand(Platform platform, const std::string& outputPath) const;

    /**
     * @brief Builds the 3dslink command for sending to hardware.
     * On Windows, uses devkitPro MSYS2. On Linux, runs directly.
     * @param outputPath Full path to the built .3dsx file
     * @return The complete command string ready for SYS_Exec
     */
    std::string Build3dsLinkCommand(const std::string& outputPath) const;

private:
    /**
     * @brief Helper to draw a path input with browse button.
     * @param label The input label
     * @param path Reference to the path string
     * @param dialogTitle Title for the file dialog
     * @return True if the value changed
     */
    bool DrawPathInput(const char* label, std::string& path, const char* dialogTitle);

    /**
     * @brief Replaces all occurrences of a substring.
     * @param str The string to modify
     * @param from The substring to find
     * @param to The replacement string
     */
    static void ReplaceAll(std::string& str, const std::string& from, const std::string& to);
};

#endif
