#pragma once

#if EDITOR

#include "../PreferencesModule.h"
#include <string>

/**
 * @brief Parent preferences module for external tools configuration.
 *
 * This module serves as a container for external tool settings such as
 * emulator paths, launcher configurations, and build tool paths.
 */
class ExternalModule : public PreferencesModule
{
public:
    DECLARE_PREFERENCES_MODULE(ExternalModule)

    ExternalModule();
    virtual ~ExternalModule();

    virtual const char* GetName() const override { return GetStaticName(); }
    virtual const char* GetParentPath() const override { return GetStaticParentPath(); }
    virtual void Render() override;
    virtual void LoadSettings(const rapidjson::Document& doc) override;
    virtual void SaveSettings(rapidjson::Document& doc) override;

    const std::string& GetDockerPath() const { return mDockerPath; }
    const std::string& GetGradlePath() const { return mGradlePath; }
    bool GetUseWSL() const { return mUseWSL; }

    /** Returns the resolved Docker invocation prefix (e.g. "docker", "wsl docker", "wsl /usr/bin/docker") */
    std::string GetDockerCommand() const;

    /** Returns the resolved Gradle command (custom path or "gradle") */
    std::string GetGradleCommand() const;

private:
    std::string mDockerPath;   // Custom Docker executable path (empty = "docker")
    std::string mGradlePath;   // Custom Gradle executable path (empty = "gradle")
    bool mUseWSL = false;      // Windows only: prefix Docker commands with "wsl"
};

#endif
