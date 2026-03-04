#pragma once

#if EDITOR

#include <string>
#include <vector>
#include <cstdint>
#include "BuildProfile.h"

/**
 * @brief Singleton manager for project-specific build profiles.
 *
 * PackagingSettings manages the collection of build profiles for the current
 * project. Settings are stored in the project's Settings directory and persist
 * across editor sessions.
 */
class PackagingSettings
{
public:
    /**
     * @brief Creates the singleton instance.
     * Should be called when a project is opened.
     */
    static void Create();

    /**
     * @brief Destroys the singleton instance.
     * Should be called when a project is closed.
     */
    static void Destroy();

    /**
     * @brief Gets the singleton instance.
     * @return Pointer to the PackagingSettings instance, or nullptr if not created
     */
    static PackagingSettings* Get();

    /**
     * @brief Creates a new build profile with the given name.
     * @param name The display name for the new profile
     * @return Pointer to the newly created profile
     */
    BuildProfile* CreateProfile(const std::string& name);

    /**
     * @brief Deletes a profile by its unique ID.
     * @param profileId The ID of the profile to delete
     */
    void DeleteProfile(uint32_t profileId);

    /**
     * @brief Gets a profile by its unique ID.
     * @param profileId The ID of the profile to find
     * @return Pointer to the profile, or nullptr if not found
     */
    BuildProfile* GetProfile(uint32_t profileId);

    /**
     * @brief Gets the currently selected profile.
     * @return Pointer to the selected profile, or nullptr if none selected
     */
    BuildProfile* GetSelectedProfile();

    /**
     * @brief Gets all build profiles.
     * @return Reference to the vector of profiles
     */
    std::vector<BuildProfile>& GetProfiles();

    /**
     * @brief Gets the index of the currently selected profile.
     * @return The selected profile index, or -1 if none selected
     */
    int32_t GetSelectedProfileIndex() const { return mSelectedProfileIndex; }

    /**
     * @brief Sets the selected profile by index.
     * @param index The index of the profile to select
     */
    void SetSelectedProfileIndex(int32_t index);

    /**
     * @brief Loads settings from the project's settings file.
     * Called when a project is opened.
     */
    void LoadSettings();

    /**
     * @brief Saves settings to the project's settings file.
     * Called on changes and when the project is closed.
     */
    void SaveSettings();

    /**
     * @brief Gets the path to the settings file.
     * @return Full path to {ProjectDir}/Settings/BuildProfiles.json
     */
    std::string GetSettingsFilePath() const;

private:
    PackagingSettings();
    ~PackagingSettings();

    static PackagingSettings* sInstance;

    /** @brief Collection of build profiles */
    std::vector<BuildProfile> mProfiles;

    /** @brief Index of the currently selected profile (-1 = none) */
    int32_t mSelectedProfileIndex = -1;

    /** @brief Next unique ID to assign to new profiles */
    uint32_t mNextProfileId = 1;
};

#endif
