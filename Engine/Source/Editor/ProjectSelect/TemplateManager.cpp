#if EDITOR

#include "TemplateManager.h"
#include "System/System.h"
#include "Stream.h"
#include "Utilities.h"
#include "Log.h"

#include "document.h"
#include "prettywriter.h"
#include "stringbuffer.h"

#include <cstdlib>

static std::string ExtractDirName(const std::string& path)
{
    size_t lastSlash = path.find_last_of("/\\");
    if (lastSlash != std::string::npos && lastSlash < path.size() - 1)
    {
        return path.substr(lastSlash + 1);
    }
    else if (lastSlash == path.size() - 1 && path.size() > 1)
    {
        // Path ends with slash, find the previous one
        size_t prevSlash = path.find_last_of("/\\", lastSlash - 1);
        if (prevSlash != std::string::npos)
        {
            return path.substr(prevSlash + 1, lastSlash - prevSlash - 1);
        }
        return path.substr(0, lastSlash);
    }
    return path;
}

TemplateManager* TemplateManager::sInstance = nullptr;

void TemplateManager::Create()
{
    if (sInstance == nullptr)
    {
        sInstance = new TemplateManager();
    }
}

void TemplateManager::Destroy()
{
    if (sInstance != nullptr)
    {
        delete sInstance;
        sInstance = nullptr;
    }
}

TemplateManager* TemplateManager::Get()
{
    return sInstance;
}

TemplateManager::TemplateManager()
{
    LoadTemplates();
}

TemplateManager::~TemplateManager()
{
    SaveTemplates();
}

std::string TemplateManager::GetTemplatesDirectory()
{
    std::string templatesDir;

#if PLATFORM_WINDOWS
    const char* appData = getenv("APPDATA");
    if (appData != nullptr)
    {
        templatesDir = std::string(appData) + "/OctaveEditor/Templates";
    }
    else
    {
        const char* userProfile = getenv("USERPROFILE");
        if (userProfile != nullptr)
        {
            templatesDir = std::string(userProfile) + "/AppData/Roaming/OctaveEditor/Templates";
        }
    }
#else
    const char* home = getenv("HOME");
    if (home != nullptr)
    {
        templatesDir = std::string(home) + "/.config/OctaveEditor/Templates";
    }
#endif

    if (templatesDir.empty())
    {
        templatesDir = "Engine/Saves/Templates";
    }

    return templatesDir;
}

std::string TemplateManager::GetSettingsPath()
{
    std::string settingsDir;

#if PLATFORM_WINDOWS
    const char* appData = getenv("APPDATA");
    if (appData != nullptr)
    {
        settingsDir = std::string(appData) + "/OctaveEditor";
    }
#else
    const char* home = getenv("HOME");
    if (home != nullptr)
    {
        settingsDir = std::string(home) + "/.config/OctaveEditor";
    }
#endif

    if (settingsDir.empty())
    {
        settingsDir = "Engine/Saves";
    }

    return settingsDir + "/templates.json";
}

void TemplateManager::EnsureTemplatesDirectory()
{
    std::string templatesDir = GetTemplatesDirectory();

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

    if (!DoesDirExist(templatesDir.c_str()))
    {
        SYS_CreateDirectory(templatesDir.c_str());
    }
}

void TemplateManager::LoadTemplates()
{
    mTemplates.clear();
    ScanTemplatesDirectory();
}

void TemplateManager::SaveTemplates()
{
    // Templates are discovered from disk, no need to save a list
    // This could be used for caching template order or preferences
}

void TemplateManager::ScanTemplatesDirectory()
{
    EnsureTemplatesDirectory();

    std::string templatesDir = GetTemplatesDirectory();
    DirEntry dirEntry;
    SYS_OpenDirectory(templatesDir, dirEntry);

    while (dirEntry.mValid)
    {
        if (dirEntry.mDirectory && strcmp(dirEntry.mFilename, ".") != 0 && strcmp(dirEntry.mFilename, "..") != 0)
        {
            std::string templatePath = templatesDir + "/" + dirEntry.mFilename;
            Template tmpl;
            if (LoadTemplateMetadata(templatePath, tmpl))
            {
                mTemplates.push_back(tmpl);
            }
        }
        SYS_IterateDirectory(dirEntry);
    }
    SYS_CloseDirectory(dirEntry);
}

bool TemplateManager::LoadTemplateMetadata(const std::string& templateDir, Template& outTemplate)
{
    std::string packagePath = templateDir + "/package.json";

    if (!SYS_DoesFileExist(packagePath.c_str(), false))
    {
        // No package.json, use directory name as ID/Name
        outTemplate.mMetadata.mId = ExtractDirName(templateDir);
        outTemplate.mMetadata.mName = outTemplate.mMetadata.mId;
        outTemplate.mPath = templateDir;
        outTemplate.mHasThumbnail = SYS_DoesFileExist((templateDir + "/thumbnail.png").c_str(), false);
        return true;
    }

    Stream stream;
    if (!stream.ReadFile(packagePath.c_str(), false))
    {
        return false;
    }

    std::string jsonStr(stream.GetData(), stream.GetSize());
    rapidjson::Document doc;
    doc.Parse(jsonStr.c_str());

    if (doc.HasParseError())
    {
        LogError("Failed to parse template package.json: %s", packagePath.c_str());
        return false;
    }

    outTemplate.mMetadata.mId = ExtractDirName(templateDir);
    outTemplate.mPath = templateDir;

    if (doc.HasMember("name") && doc["name"].IsString())
    {
        outTemplate.mMetadata.mName = doc["name"].GetString();
    }
    else
    {
        outTemplate.mMetadata.mName = outTemplate.mMetadata.mId;
    }

    if (doc.HasMember("author") && doc["author"].IsString())
    {
        outTemplate.mMetadata.mAuthor = doc["author"].GetString();
    }

    if (doc.HasMember("description") && doc["description"].IsString())
    {
        outTemplate.mMetadata.mDescription = doc["description"].GetString();
    }

    if (doc.HasMember("url") && doc["url"].IsString())
    {
        outTemplate.mMetadata.mUrl = doc["url"].GetString();
    }

    if (doc.HasMember("version") && doc["version"].IsString())
    {
        outTemplate.mMetadata.mVersion = doc["version"].GetString();
    }

    if (doc.HasMember("updated") && doc["updated"].IsString())
    {
        outTemplate.mMetadata.mUpdated = doc["updated"].GetString();
    }

    if (doc.HasMember("cpp") && doc["cpp"].IsBool())
    {
        outTemplate.mMetadata.mIsCpp = doc["cpp"].GetBool();
    }

    if (doc.HasMember("tags") && doc["tags"].IsArray())
    {
        const rapidjson::Value& tags = doc["tags"];
        for (rapidjson::SizeType i = 0; i < tags.Size(); ++i)
        {
            if (tags[i].IsString())
            {
                outTemplate.mMetadata.mTags.push_back(tags[i].GetString());
            }
        }
    }

    outTemplate.mHasThumbnail = SYS_DoesFileExist((templateDir + "/thumbnail.png").c_str(), false);

    return true;
}

const Template* TemplateManager::FindTemplate(const std::string& templateId) const
{
    for (const Template& tmpl : mTemplates)
    {
        if (tmpl.mMetadata.mId == templateId)
        {
            return &tmpl;
        }
    }
    return nullptr;
}

std::string TemplateManager::ConvertGitHubUrlToDownloadUrl(const std::string& gitHubUrl)
{
    // Convert: https://github.com/user/repo
    // To: https://github.com/user/repo/archive/refs/heads/main.zip

    std::string url = gitHubUrl;

    // Remove trailing slash if present
    while (!url.empty() && url.back() == '/')
    {
        url.pop_back();
    }

    // Check if it's already a download URL
    if (url.find("/archive/") != std::string::npos)
    {
        return url;
    }

    // Handle tree URLs (specific folder)
    // https://github.com/user/repo/tree/main/folder
    // -> Download full repo zip for now
    size_t treePos = url.find("/tree/");
    if (treePos != std::string::npos)
    {
        url = url.substr(0, treePos);
    }

    return url + "/archive/refs/heads/main.zip";
}

bool TemplateManager::DownloadFile(const std::string& url, const std::string& destPath, std::string& outError)
{
    std::string output;

#if PLATFORM_WINDOWS
    // Try curl first (available on Windows 10+)
    std::string cmd = "curl -L -o \"" + destPath + "\" \"" + url + "\" 2>&1";
    SYS_Exec(cmd.c_str(), &output);

    if (!SYS_DoesFileExist(destPath.c_str(), false))
    {
        // Fallback to PowerShell
        cmd = "powershell -Command \"Invoke-WebRequest -Uri '" + url + "' -OutFile '" + destPath + "'\" 2>&1";
        SYS_Exec(cmd.c_str(), &output);
    }
#else
    std::string cmd = "curl -L -o \"" + destPath + "\" \"" + url + "\" 2>&1";
    SYS_Exec(cmd.c_str(), &output);
#endif

    if (!SYS_DoesFileExist(destPath.c_str(), false))
    {
        outError = "Failed to download file: " + output;
        return false;
    }

    return true;
}

bool TemplateManager::ExtractZip(const std::string& zipPath, const std::string& destDir, std::string& outError)
{
    std::string output;

    if (!DoesDirExist(destDir.c_str()))
    {
        SYS_CreateDirectory(destDir.c_str());
    }

#if PLATFORM_WINDOWS
    // Use tar on Windows 10+ (built-in)
    std::string cmd = "tar -xf \"" + zipPath + "\" -C \"" + destDir + "\" 2>&1";
    SYS_Exec(cmd.c_str(), &output);
#else
    std::string cmd = "unzip -o \"" + zipPath + "\" -d \"" + destDir + "\" 2>&1";
    SYS_Exec(cmd.c_str(), &output);
#endif

    return true;
}

bool TemplateManager::AddTemplateFromZip(const std::string& zipPath, std::string& outError)
{
    if (!SYS_DoesFileExist(zipPath.c_str(), false))
    {
        outError = "Zip file not found: " + zipPath;
        return false;
    }

    EnsureTemplatesDirectory();
    std::string templatesDir = GetTemplatesDirectory();

    // Create temp directory for extraction
    std::string tempDir = templatesDir + "/_temp_extract";
    if (DoesDirExist(tempDir.c_str()))
    {
        SYS_RemoveDirectory(tempDir.c_str());
    }
    SYS_CreateDirectory(tempDir.c_str());

    // Extract zip
    if (!ExtractZip(zipPath, tempDir, outError))
    {
        SYS_RemoveDirectory(tempDir.c_str());
        return false;
    }

    // Find the extracted folder (GitHub zips have repo-name-branch structure)
    DirEntry dirEntry;
    SYS_OpenDirectory(tempDir, dirEntry);
    std::string extractedFolder;

    while (dirEntry.mValid)
    {
        if (dirEntry.mDirectory && strcmp(dirEntry.mFilename, ".") != 0 && strcmp(dirEntry.mFilename, "..") != 0)
        {
            extractedFolder = tempDir + "/" + dirEntry.mFilename;
            break;
        }
        SYS_IterateDirectory(dirEntry);
    }
    SYS_CloseDirectory(dirEntry);

    if (extractedFolder.empty())
    {
        // Files extracted directly, use temp as source
        extractedFolder = tempDir;
    }

    // Determine template name from package.json or folder name
    std::string templateName;
    std::string packagePath = extractedFolder + "/package.json";

    if (SYS_DoesFileExist(packagePath.c_str(), false))
    {
        Stream stream;
        if (stream.ReadFile(packagePath.c_str(), false))
        {
            std::string jsonStr(stream.GetData(), stream.GetSize());
            rapidjson::Document doc;
            doc.Parse(jsonStr.c_str());

            if (!doc.HasParseError() && doc.HasMember("name") && doc["name"].IsString())
            {
                templateName = doc["name"].GetString();
                // Sanitize name for directory
                for (char& c : templateName)
                {
                    if (c == ' ' || c == '/' || c == '\\' || c == ':')
                    {
                        c = '_';
                    }
                }
            }
        }
    }

    if (templateName.empty())
    {
        templateName = ExtractDirName(zipPath);
        // Remove .zip extension
        size_t dotPos = templateName.find_last_of('.');
        if (dotPos != std::string::npos)
        {
            templateName = templateName.substr(0, dotPos);
        }
    }

    // Move to templates directory
    std::string finalPath = templatesDir + "/" + templateName;
    if (DoesDirExist(finalPath.c_str()))
    {
        SYS_RemoveDirectory(finalPath.c_str());
    }
    SYS_MoveDirectory(extractedFolder.c_str(), finalPath.c_str());

    // Clean up temp directory
    if (DoesDirExist(tempDir.c_str()))
    {
        SYS_RemoveDirectory(tempDir.c_str());
    }

    // Reload templates
    LoadTemplates();

    return true;
}

bool TemplateManager::AddTemplateFromGitHub(const std::string& gitHubUrl, std::string& outError)
{
    EnsureTemplatesDirectory();
    std::string templatesDir = GetTemplatesDirectory();

    // Download zip
    std::string downloadUrl = ConvertGitHubUrlToDownloadUrl(gitHubUrl);
    std::string tempZipPath = templatesDir + "/_temp_download.zip";

    if (!DownloadFile(downloadUrl, tempZipPath, outError))
    {
        return false;
    }

    // Extract and install
    bool result = AddTemplateFromZip(tempZipPath, outError);

    // Clean up temp zip
    if (SYS_DoesFileExist(tempZipPath.c_str(), false))
    {
        SYS_RemoveFile(tempZipPath.c_str());
    }

    return result;
}

bool TemplateManager::RemoveTemplate(const std::string& templateId)
{
    const Template* tmpl = FindTemplate(templateId);
    if (tmpl == nullptr)
    {
        return false;
    }

    if (DoesDirExist(tmpl->mPath.c_str()))
    {
        SYS_RemoveDirectory(tmpl->mPath.c_str());
    }

    // Reload templates
    LoadTemplates();

    return true;
}

bool TemplateManager::CreateProjectFromTemplate(const std::string& templateId,
                                                 const std::string& projectPath,
                                                 const std::string& projectName,
                                                 std::string& outError)
{
    const Template* tmpl = FindTemplate(templateId);
    if (tmpl == nullptr)
    {
        outError = "Template not found: " + templateId;
        return false;
    }

    // Create project directory
    if (!DoesDirExist(projectPath.c_str()))
    {
        SYS_CreateDirectory(projectPath.c_str());
    }

    // Copy template contents to project directory
    if (!SYS_CopyDirectoryRecursive(tmpl->mPath, projectPath))
    {
        outError = "Failed to copy template files";
        return false;
    }

    // Rename .octp file if exists
    DirEntry dirEntry;
    SYS_OpenDirectory(projectPath, dirEntry);

    while (dirEntry.mValid)
    {
        if (!dirEntry.mDirectory)
        {
            std::string filename = dirEntry.mFilename;
            size_t octpPos = filename.find(".octp");
            if (octpPos != std::string::npos)
            {
                std::string oldPath = projectPath + "/" + filename;
                std::string newPath = projectPath + "/" + projectName + ".octp";
                SYS_Rename(oldPath.c_str(), newPath.c_str());
                break;
            }
        }
        SYS_IterateDirectory(dirEntry);
    }
    SYS_CloseDirectory(dirEntry);

    // Remove package.json and thumbnail.png from project
    std::string packagePath = projectPath + "/package.json";
    if (SYS_DoesFileExist(packagePath.c_str(), false))
    {
        SYS_RemoveFile(packagePath.c_str());
    }

    std::string thumbnailPath = projectPath + "/thumbnail.png";
    if (SYS_DoesFileExist(thumbnailPath.c_str(), false))
    {
        SYS_RemoveFile(thumbnailPath.c_str());
    }

    return true;
}

#endif
