#pragma once

#if EDITOR

#include "../PreferencesModule.h"
#include <string>

/**
 * @brief Preferences module for configuring Docker build settings.
 *
 * This module allows users to configure the Docker image used for
 * building games for various platforms (Linux, GameCube, Wii, 3DS).
 *
 * Settings are stored in user preferences and persist across projects.
 */
class DockerModule : public PreferencesModule
{
public:
    DECLARE_PREFERENCES_MODULE(DockerModule)

    DockerModule();
    virtual ~DockerModule();

    virtual const char* GetName() const override { return GetStaticName(); }
    virtual const char* GetParentPath() const override { return GetStaticParentPath(); }
    virtual void Render() override;
    virtual void LoadSettings(const rapidjson::Document& doc) override;
    virtual void SaveSettings(rapidjson::Document& doc) override;

    /**
     * @brief Gets the configured Docker image name.
     * @return The Docker image to use for builds
     */
    const std::string& GetDockerImage() const { return mDockerImage; }

private:
    /** @brief Docker image name for building */
    std::string mDockerImage = "vltmedia/octavegameengine-linux:dev";
};

#endif
