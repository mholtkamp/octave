#if EDITOR

#include "PackagingSettings.h"
#include "Preferences/JsonSettings.h"

#include "Engine.h"
#include "Log.h"
#include "System/System.h"
#include "Utilities.h"

#include "document.h"
#include "prettywriter.h"
#include "stringbuffer.h"
#include "Stream.h"

PackagingSettings* PackagingSettings::sInstance = nullptr;

void PackagingSettings::Create()
{
    Destroy();
    sInstance = new PackagingSettings();
}

void PackagingSettings::Destroy()
{
    if (sInstance != nullptr)
    {
        delete sInstance;
        sInstance = nullptr;
    }
}

PackagingSettings* PackagingSettings::Get()
{
    return sInstance;
}

PackagingSettings::PackagingSettings()
{
}

PackagingSettings::~PackagingSettings()
{
}

BuildProfile* PackagingSettings::CreateProfile(const std::string& name)
{
    BuildProfile profile;
    profile.mId = mNextProfileId++;
    profile.mName = name;
    mProfiles.push_back(profile);

    // Select the new profile if none is selected
    if (mSelectedProfileIndex < 0)
    {
        mSelectedProfileIndex = static_cast<int32_t>(mProfiles.size()) - 1;
    }

    SaveSettings();
    return &mProfiles.back();
}

void PackagingSettings::DeleteProfile(uint32_t profileId)
{
    for (size_t i = 0; i < mProfiles.size(); ++i)
    {
        if (mProfiles[i].mId == profileId)
        {
            mProfiles.erase(mProfiles.begin() + i);

            // Adjust selected index
            if (mSelectedProfileIndex >= static_cast<int32_t>(mProfiles.size()))
            {
                mSelectedProfileIndex = static_cast<int32_t>(mProfiles.size()) - 1;
            }

            SaveSettings();
            return;
        }
    }
}

BuildProfile* PackagingSettings::GetProfile(uint32_t profileId)
{
    for (BuildProfile& profile : mProfiles)
    {
        if (profile.mId == profileId)
        {
            return &profile;
        }
    }
    return nullptr;
}

BuildProfile* PackagingSettings::GetSelectedProfile()
{
    if (mSelectedProfileIndex >= 0 && mSelectedProfileIndex < static_cast<int32_t>(mProfiles.size()))
    {
        return &mProfiles[mSelectedProfileIndex];
    }
    return nullptr;
}

std::vector<BuildProfile>& PackagingSettings::GetProfiles()
{
    return mProfiles;
}

void PackagingSettings::SetSelectedProfileIndex(int32_t index)
{
    if (index >= -1 && index < static_cast<int32_t>(mProfiles.size()))
    {
        mSelectedProfileIndex = index;
        SaveSettings();
    }
}

std::string PackagingSettings::GetSettingsFilePath() const
{
    const EngineState* engine = GetEngineState();
    if (engine->mProjectDirectory.empty())
    {
        return "";
    }
    return engine->mProjectDirectory + "Settings/BuildProfiles.json";
}

void PackagingSettings::LoadSettings()
{
    std::string path = GetSettingsFilePath();
    if (path.empty())
    {
        // No project directory, create default profile
        CreateProfile("Default");
        return;
    }

    if (!SYS_DoesFileExist(path.c_str(), false))
    {
        // Settings file doesn't exist, create default profile for new projects
        CreateProfile("Default");
        return;
    }

    Stream stream;
    if (!stream.ReadFile(path.c_str(), false))
    {
        LogError("Failed to read BuildProfiles.json");
        CreateProfile("Default");
        return;
    }

    std::string jsonStr(stream.GetData(), stream.GetSize());
    rapidjson::Document doc;
    doc.Parse(jsonStr.c_str());

    if (doc.HasParseError())
    {
        LogError("Failed to parse BuildProfiles.json");
        CreateProfile("Default");
        return;
    }

    // Load selected profile index
    if (doc.HasMember("selectedProfileIndex") && doc["selectedProfileIndex"].IsInt())
    {
        mSelectedProfileIndex = doc["selectedProfileIndex"].GetInt();
    }

    // Load next profile ID
    if (doc.HasMember("nextProfileId") && doc["nextProfileId"].IsUint())
    {
        mNextProfileId = doc["nextProfileId"].GetUint();
    }

    // Load profiles
    if (doc.HasMember("profiles") && doc["profiles"].IsArray())
    {
        const rapidjson::Value& profiles = doc["profiles"];
        mProfiles.clear();
        mProfiles.reserve(profiles.Size());

        for (rapidjson::SizeType i = 0; i < profiles.Size(); ++i)
        {
            BuildProfile profile;
            profile.LoadFromJson(profiles[i]);
            mProfiles.push_back(profile);
        }
    }

    // Validate selected index
    if (mSelectedProfileIndex >= static_cast<int32_t>(mProfiles.size()))
    {
        mSelectedProfileIndex = mProfiles.empty() ? -1 : 0;
    }

    // Create default profile if none exist
    if (mProfiles.empty())
    {
        CreateProfile("Default");
    }
}

void PackagingSettings::SaveSettings()
{
    std::string path = GetSettingsFilePath();
    if (path.empty())
    {
        return;
    }

    // Ensure Settings directory exists
    const EngineState* engine = GetEngineState();
    std::string settingsDir = engine->mProjectDirectory + "Settings/";
    if (!DoesDirExist(settingsDir.c_str()))
    {
        SYS_CreateDirectory(settingsDir.c_str());
    }

    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

    // Save version
    doc.AddMember("version", 1, allocator);

    // Save selected profile index
    doc.AddMember("selectedProfileIndex", mSelectedProfileIndex, allocator);

    // Save next profile ID
    doc.AddMember("nextProfileId", mNextProfileId, allocator);

    // Save profiles
    rapidjson::Value profilesArray(rapidjson::kArrayType);
    for (const BuildProfile& profile : mProfiles)
    {
        rapidjson::Value profileValue;
        profile.SaveToJson(doc, profileValue);
        profilesArray.PushBack(profileValue, allocator);
    }
    doc.AddMember("profiles", profilesArray, allocator);

    // Write to file
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    Stream stream(buffer.GetString(), static_cast<uint32_t>(buffer.GetSize()));
    if (!stream.WriteFile(path.c_str()))
    {
        LogError("Failed to write BuildProfiles.json");
    }
}

#endif
