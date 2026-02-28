#if EDITOR

#include "CssThemeManager.h"
#include "CssThemeParser.h"
#include "../../JsonSettings.h"

#include "document.h"
#include "prettywriter.h"
#include "stringbuffer.h"

#include "System/System.h"
#include "Utilities.h"
#include "Stream.h"
#include "Log.h"

#include <algorithm>
#include <cstring>

static const std::string sEmptyString;

CssThemeManager& CssThemeManager::Get()
{
    static CssThemeManager sInstance;
    return sInstance;
}

std::string CssThemeManager::GetThemesDirectory() const
{
    return JsonSettings::GetPreferencesDirectory() + "/Themes";
}

std::string CssThemeManager::GetThemeListPath() const
{
    return JsonSettings::GetPreferencesDirectory() + "/Appearance_CustomThemes.json";
}

const std::string& CssThemeManager::GetThemeName(int index) const
{
    if (index >= 0 && index < static_cast<int>(mThemes.size()))
    {
        return mThemes[index].Name;
    }
    return sEmptyString;
}

bool CssThemeManager::ImportTheme(const std::string& filePath, const std::string& name)
{
    // Ensure Themes directory exists
    std::string themesDir = GetThemesDirectory();
    if (!DoesDirExist(themesDir.c_str()))
    {
        JsonSettings::EnsurePreferencesDirectory();
        SYS_CreateDirectory(themesDir.c_str());
    }

    // Generate a unique filename based on the theme name
    std::string safeName = name;
    std::replace(safeName.begin(), safeName.end(), ' ', '_');
    std::string destFileName = safeName + ".css";
    std::string destPath = themesDir + "/" + destFileName;

    // If file already exists, append a number
    int counter = 1;
    while (SYS_DoesFileExist(destPath.c_str(), false))
    {
        destFileName = safeName + "_" + std::to_string(counter) + ".css";
        destPath = themesDir + "/" + destFileName;
        counter++;
    }

    // Copy the CSS file to Preferences/Themes/
    SYS_CopyFile(filePath.c_str(), destPath.c_str());

    // Parse the CSS
    CustomThemeEntry entry;
    entry.Name = name;
    entry.FileName = destFileName;
    entry.IsValid = CssThemeParser::ParseFile(destPath, entry.ParsedData);

    if (!entry.IsValid)
    {
        LogError("CSS Theme: Failed to parse '%s'", filePath.c_str());
        return false;
    }

    mThemes.push_back(entry);
    SaveThemeList();

    LogDebug("CSS Theme: Imported '%s' as '%s'", filePath.c_str(), name.c_str());
    return true;
}

void CssThemeManager::RemoveTheme(int index)
{
    if (index < 0 || index >= static_cast<int>(mThemes.size()))
        return;

    // Delete the CSS file from Preferences/Themes/
    std::string filePath = GetThemesDirectory() + "/" + mThemes[index].FileName;
    SYS_RemoveFile(filePath.c_str());

    mThemes.erase(mThemes.begin() + index);
    SaveThemeList();
}

bool CssThemeManager::ReimportTheme(int index)
{
    if (index < 0 || index >= static_cast<int>(mThemes.size()))
        return false;

    std::string filePath = GetThemesDirectory() + "/" + mThemes[index].FileName;
    CssThemeData newData;
    if (CssThemeParser::ParseFile(filePath, newData))
    {
        mThemes[index].ParsedData = newData;
        mThemes[index].IsValid = true;
        LogDebug("CSS Theme: Reimported '%s'", mThemes[index].Name.c_str());
        return true;
    }

    LogError("CSS Theme: Failed to reimport '%s'", mThemes[index].Name.c_str());
    mThemes[index].IsValid = false;
    return false;
}

bool CssThemeManager::ReimportThemeFromFile(int index, const std::string& filePath)
{
    if (index < 0 || index >= static_cast<int>(mThemes.size()))
        return false;

    // Overwrite the stored CSS file with the new one
    std::string destPath = GetThemesDirectory() + "/" + mThemes[index].FileName;
    SYS_CopyFile(filePath.c_str(), destPath.c_str());

    // Re-parse from the updated file
    CssThemeData newData;
    if (CssThemeParser::ParseFile(destPath, newData))
    {
        mThemes[index].ParsedData = newData;
        mThemes[index].IsValid = true;
        LogDebug("CSS Theme: Reimported '%s' from '%s'", mThemes[index].Name.c_str(), filePath.c_str());
        return true;
    }

    LogError("CSS Theme: Failed to reimport '%s' from '%s'", mThemes[index].Name.c_str(), filePath.c_str());
    mThemes[index].IsValid = false;
    return false;
}

void CssThemeManager::ApplyTheme(int index)
{
    if (index < 0 || index >= static_cast<int>(mThemes.size()))
        return;

    if (mThemes[index].IsValid)
    {
        CssThemeParser::ApplyTheme(mThemes[index].ParsedData);
    }
}

static std::string DeriveBundledThemeName(const std::string& filename)
{
    // Strip .css extension
    std::string name = filename;
    size_t dot = name.find_last_of('.');
    if (dot != std::string::npos)
        name = name.substr(0, dot);

    // Convert hyphens/underscores to spaces and capitalize each word
    std::string result;
    bool capitalizeNext = true;
    for (char c : name)
    {
        if (c == '-' || c == '_')
        {
            result += ' ';
            capitalizeNext = true;
        }
        else
        {
            if (capitalizeNext && c >= 'a' && c <= 'z')
                result += (char)(c - 32);
            else
                result += c;
            capitalizeNext = false;
        }
    }
    return result;
}

void CssThemeManager::LoadBundledThemes()
{
    std::string themesDir = SYS_GetOctavePath() + "Engine/Assets/Themes/";

    DirEntry dirEntry;
    SYS_OpenDirectory(themesDir, dirEntry);

    while (dirEntry.mValid)
    {
        if (!dirEntry.mDirectory &&
            strcmp(dirEntry.mFilename, ".") != 0 &&
            strcmp(dirEntry.mFilename, "..") != 0)
        {
            std::string filename = dirEntry.mFilename;
            size_t dot = filename.find_last_of('.');
            if (dot != std::string::npos && filename.substr(dot) == ".css")
            {
                std::string fullPath = themesDir + filename;

                CustomThemeEntry entry;
                entry.Name = DeriveBundledThemeName(filename);
                entry.FileName = filename;
                entry.IsBundled = true;
                entry.IsValid = CssThemeParser::ParseFile(fullPath, entry.ParsedData);

                if (!entry.IsValid)
                {
                    LogWarning("CSS Theme: Could not parse bundled theme '%s'", filename.c_str());
                }

                mThemes.push_back(entry);
            }
        }

        SYS_IterateDirectory(dirEntry);
    }

    SYS_CloseDirectory(dirEntry);
}

void CssThemeManager::LoadThemeList()
{
    mThemes.clear();

    // Load bundled themes first (from Engine/Assets/Themes/)
    LoadBundledThemes();

    rapidjson::Document doc;
    std::string listPath = GetThemeListPath();

    if (!JsonSettings::LoadFromFile(listPath, doc))
        return;

    if (!doc.HasMember("themes") || !doc["themes"].IsArray())
        return;

    const rapidjson::Value& arr = doc["themes"];
    std::string themesDir = GetThemesDirectory();

    for (rapidjson::SizeType i = 0; i < arr.Size(); ++i)
    {
        const rapidjson::Value& entry = arr[i];
        if (!entry.IsObject()) continue;
        if (!entry.HasMember("name") || !entry["name"].IsString()) continue;
        if (!entry.HasMember("file") || !entry["file"].IsString()) continue;

        CustomThemeEntry theme;
        theme.Name = entry["name"].GetString();
        theme.FileName = entry["file"].GetString();

        std::string fullPath = themesDir + "/" + theme.FileName;
        theme.IsValid = CssThemeParser::ParseFile(fullPath, theme.ParsedData);

        if (!theme.IsValid)
        {
            LogWarning("CSS Theme: Could not parse saved theme '%s' from '%s'",
                theme.Name.c_str(), theme.FileName.c_str());
        }

        mThemes.push_back(theme);
    }
}

void CssThemeManager::SaveThemeList()
{
    JsonSettings::EnsurePreferencesDirectory();

    rapidjson::Document doc;
    doc.SetObject();
    auto& allocator = doc.GetAllocator();

    rapidjson::Value arr(rapidjson::kArrayType);

    for (const auto& theme : mThemes)
    {
        if (theme.IsBundled)
            continue;

        rapidjson::Value entry(rapidjson::kObjectType);
        entry.AddMember("name",
            rapidjson::Value(theme.Name.c_str(), allocator),
            allocator);
        entry.AddMember("file",
            rapidjson::Value(theme.FileName.c_str(), allocator),
            allocator);
        arr.PushBack(entry, allocator);
    }

    doc.AddMember("themes", arr, allocator);

    JsonSettings::SaveToFile(GetThemeListPath(), doc);
}

#endif
