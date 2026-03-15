#if EDITOR

#include "BuildProfile.h"

void BuildProfile::LoadFromJson(const rapidjson::Value& value)
{
    if (value.HasMember("id") && value["id"].IsUint())
    {
        mId = value["id"].GetUint();
    }

    if (value.HasMember("name") && value["name"].IsString())
    {
        mName = value["name"].GetString();
    }

    if (value.HasMember("platform") && value["platform"].IsInt())
    {
        mTargetPlatform = static_cast<Platform>(value["platform"].GetInt());
    }

    if (value.HasMember("embedded") && value["embedded"].IsBool())
    {
        mEmbedded = value["embedded"].GetBool();
    }

    if (value.HasMember("outputDirectory") && value["outputDirectory"].IsString())
    {
        mOutputDirectory = value["outputDirectory"].GetString();
    }

    if (value.HasMember("useDocker") && value["useDocker"].IsBool())
    {
        mUseDocker = value["useDocker"].GetBool();
    }

    if (value.HasMember("openDirectoryOnFinish") && value["openDirectoryOnFinish"].IsBool())
    {
        mOpenDirectoryOnFinish = value["openDirectoryOnFinish"].GetBool();
    }
}

void BuildProfile::SaveToJson(rapidjson::Document& doc, rapidjson::Value& value) const
{
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

    value.SetObject();
    value.AddMember("id", mId, allocator);
    value.AddMember("name", rapidjson::Value(mName.c_str(), allocator), allocator);
    value.AddMember("platform", static_cast<int>(mTargetPlatform), allocator);
    value.AddMember("embedded", mEmbedded, allocator);
    value.AddMember("outputDirectory", rapidjson::Value(mOutputDirectory.c_str(), allocator), allocator);
    value.AddMember("useDocker", mUseDocker, allocator);
    value.AddMember("openDirectoryOnFinish", mOpenDirectoryOnFinish, allocator);
}

const char* GetPlatformOutputExtension(Platform platform)
{
    switch (platform)
    {
        case Platform::Windows:  return ".exe";
        case Platform::Linux:    return ".elf";
        case Platform::Android:  return ".apk";
        case Platform::GameCube: return ".dol";
        case Platform::Wii:      return ".dol";
        case Platform::N3DS:     return ".3dsx";
        default:                 return "";
    }
}

bool PlatformRequiresDockerOnWindows(Platform platform)
{
    return false;
    switch (platform)
    {
        case Platform::GameCube:
        case Platform::Wii:
            return true;
        default:
            return false;
    }
}

bool PlatformSupportsRun(Platform platform)
{
    switch (platform)
    {
        case Platform::GameCube:
        case Platform::Wii:
        case Platform::N3DS:
            return true;
        default:
            return false;
    }
}

#endif
