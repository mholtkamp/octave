#if EDITOR

#include "JsonSettings.h"

#include "document.h"
#include "prettywriter.h"
#include "stringbuffer.h"

#include "Stream.h"
#include "Utilities.h"
#include "System/System.h"
#include "Log.h"

#include <cstdlib>

namespace JsonSettings
{

static std::string sPreferencesDir;

std::string GetPreferencesDirectory()
{
    if (sPreferencesDir.empty())
    {
        // Build the path to AppData/Roaming/OctaveEditor/Preferences on Windows
        // or ~/.config/OctaveEditor/Preferences on Linux
#if PLATFORM_WINDOWS
        const char* appData = getenv("APPDATA");
        if (appData != nullptr)
        {
            sPreferencesDir = std::string(appData) + "/OctaveEditor/Preferences";
        }
        else
        {
            // Fallback to USERPROFILE if APPDATA is not set
            const char* userProfile = getenv("USERPROFILE");
            if (userProfile != nullptr)
            {
                sPreferencesDir = std::string(userProfile) + "/AppData/Roaming/OctaveEditor/Preferences";
            }
        }
#else
        const char* home = getenv("HOME");
        if (home != nullptr)
        {
            sPreferencesDir = std::string(home) + "/.config/OctaveEditor/Preferences";
        }
#endif

        // Final fallback to local directory
        if (sPreferencesDir.empty())
        {
            sPreferencesDir = "Engine/Saves/Preferences";
        }
    }

    return sPreferencesDir;
}

void EnsurePreferencesDirectory()
{
    std::string prefsDir = GetPreferencesDirectory();

    // Create parent directories as needed
#if PLATFORM_WINDOWS
    const char* appData = getenv("APPDATA");
    if (appData != nullptr)
    {
        std::string octaveDir = std::string(appData) + "/OctaveEditor";
        if (!DoesDirExist(octaveDir.c_str()))
        {
            SYS_CreateDirectory(octaveDir.c_str());
        }
    }
#else
    const char* home = getenv("HOME");
    if (home != nullptr)
    {
        std::string configDir = std::string(home) + "/.config";
        if (!DoesDirExist(configDir.c_str()))
        {
            SYS_CreateDirectory(configDir.c_str());
        }
        std::string octaveDir = configDir + "/OctaveEditor";
        if (!DoesDirExist(octaveDir.c_str()))
        {
            SYS_CreateDirectory(octaveDir.c_str());
        }
    }
#endif

    if (!DoesDirExist(prefsDir.c_str()))
    {
        SYS_CreateDirectory(prefsDir.c_str());
    }
}

bool LoadFromFile(const std::string& path, rapidjson::Document& doc)
{
    Stream stream;
    if (!stream.ReadFile(path.c_str(), false))
    {
        return false;
    }

    // Null-terminate the data for parsing
    std::string jsonStr(stream.GetData(), stream.GetSize());
    doc.Parse(jsonStr.c_str());

    if (doc.HasParseError())
    {
        LogError("Failed to parse JSON file: %s", path.c_str());
        return false;
    }

    return true;
}

bool SaveToFile(const std::string& path, rapidjson::Document& doc)
{
    EnsurePreferencesDirectory();

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    Stream stream(buffer.GetString(), (uint32_t)buffer.GetSize());
    return stream.WriteFile(path.c_str());
}

bool GetBool(const rapidjson::Document& doc, const char* key, bool defaultValue)
{
    if (doc.HasMember(key) && doc[key].IsBool())
    {
        return doc[key].GetBool();
    }
    return defaultValue;
}

int GetInt(const rapidjson::Document& doc, const char* key, int defaultValue)
{
    if (doc.HasMember(key) && doc[key].IsInt())
    {
        return doc[key].GetInt();
    }
    return defaultValue;
}

float GetFloat(const rapidjson::Document& doc, const char* key, float defaultValue)
{
    if (doc.HasMember(key) && doc[key].IsNumber())
    {
        return doc[key].GetFloat();
    }
    return defaultValue;
}

std::string GetString(const rapidjson::Document& doc, const char* key, const std::string& defaultValue)
{
    if (doc.HasMember(key) && doc[key].IsString())
    {
        return doc[key].GetString();
    }
    return defaultValue;
}

glm::vec3 GetVec3(const rapidjson::Document& doc, const char* key, const glm::vec3& defaultValue)
{
    if (doc.HasMember(key) && doc[key].IsArray())
    {
        const rapidjson::Value& arr = doc[key];
        if (arr.Size() >= 3)
        {
            return glm::vec3(
                arr[0].GetFloat(),
                arr[1].GetFloat(),
                arr[2].GetFloat()
            );
        }
    }
    return defaultValue;
}

glm::vec4 GetVec4(const rapidjson::Document& doc, const char* key, const glm::vec4& defaultValue)
{
    if (doc.HasMember(key) && doc[key].IsArray())
    {
        const rapidjson::Value& arr = doc[key];
        if (arr.Size() >= 4)
        {
            return glm::vec4(
                arr[0].GetFloat(),
                arr[1].GetFloat(),
                arr[2].GetFloat(),
                arr[3].GetFloat()
            );
        }
    }
    return defaultValue;
}

void SetBool(rapidjson::Document& doc, const char* key, bool value)
{
    if (!doc.IsObject())
    {
        doc.SetObject();
    }

    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

    if (doc.HasMember(key))
    {
        doc[key].SetBool(value);
    }
    else
    {
        rapidjson::Value k(key, allocator);
        doc.AddMember(k, value, allocator);
    }
}

void SetInt(rapidjson::Document& doc, const char* key, int value)
{
    if (!doc.IsObject())
    {
        doc.SetObject();
    }

    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

    if (doc.HasMember(key))
    {
        doc[key].SetInt(value);
    }
    else
    {
        rapidjson::Value k(key, allocator);
        doc.AddMember(k, value, allocator);
    }
}

void SetFloat(rapidjson::Document& doc, const char* key, float value)
{
    if (!doc.IsObject())
    {
        doc.SetObject();
    }

    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

    if (doc.HasMember(key))
    {
        doc[key].SetFloat(value);
    }
    else
    {
        rapidjson::Value k(key, allocator);
        doc.AddMember(k, value, allocator);
    }
}

void SetString(rapidjson::Document& doc, const char* key, const std::string& value)
{
    if (!doc.IsObject())
    {
        doc.SetObject();
    }

    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

    if (doc.HasMember(key))
    {
        doc[key].SetString(value.c_str(), allocator);
    }
    else
    {
        rapidjson::Value k(key, allocator);
        rapidjson::Value v(value.c_str(), allocator);
        doc.AddMember(k, v, allocator);
    }
}

void SetVec3(rapidjson::Document& doc, const char* key, const glm::vec3& value)
{
    if (!doc.IsObject())
    {
        doc.SetObject();
    }

    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

    rapidjson::Value arr(rapidjson::kArrayType);
    arr.PushBack(value.x, allocator);
    arr.PushBack(value.y, allocator);
    arr.PushBack(value.z, allocator);

    if (doc.HasMember(key))
    {
        doc[key] = arr;
    }
    else
    {
        rapidjson::Value k(key, allocator);
        doc.AddMember(k, arr, allocator);
    }
}

void SetVec4(rapidjson::Document& doc, const char* key, const glm::vec4& value)
{
    if (!doc.IsObject())
    {
        doc.SetObject();
    }

    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

    rapidjson::Value arr(rapidjson::kArrayType);
    arr.PushBack(value.x, allocator);
    arr.PushBack(value.y, allocator);
    arr.PushBack(value.z, allocator);
    arr.PushBack(value.w, allocator);

    if (doc.HasMember(key))
    {
        doc[key] = arr;
    }
    else
    {
        rapidjson::Value k(key, allocator);
        doc.AddMember(k, arr, allocator);
    }
}

} // namespace JsonSettings

#endif
