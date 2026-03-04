#if EDITOR

#include "AddonCreator.h"
#include "AddonManager.h"
#include "NativeAddonManager.h"
#include "System/System.h"
#include "Engine.h"
#include "Log.h"
#include "Stream.h"
#include "Utilities.h"

#include "document.h"
#include "prettywriter.h"
#include "stringbuffer.h"

#include <cctype>
#include <sstream>
#include <cstring>

// Helper: create directories recursively (same pattern as NativeAddonManager)
static bool CreateDirRecursive(const std::string& path)
{
    if (path.empty())
    {
        return false;
    }

    std::string normalized = path;
    for (char& c : normalized)
    {
        if (c == '\\')
        {
            c = '/';
        }
    }

    if (normalized.back() == '/')
    {
        normalized.pop_back();
    }

    if (DoesDirExist(normalized.c_str()))
    {
        return true;
    }

    size_t lastSlash = normalized.find_last_of('/');
    if (lastSlash != std::string::npos && lastSlash > 0)
    {
        std::string parentPath = normalized.substr(0, lastSlash);
        bool isDriveRoot = (parentPath.length() == 2 && parentPath[1] == ':');
        if (!isDriveRoot && !DoesDirExist(parentPath.c_str()))
        {
            if (!CreateDirRecursive(parentPath))
            {
                return false;
            }
        }
    }

    return SYS_CreateDirectory(normalized.c_str());
}

// Helper: escape a string for JSON output
static std::string EscapeJsonString(const std::string& input)
{
    std::string result;
    result.reserve(input.size());
    for (char c : input)
    {
        switch (c)
        {
        case '"':  result += "\\\""; break;
        case '\\': result += "\\\\"; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        default:   result += c; break;
        }
    }
    return result;
}

std::string AddonCreator::GenerateIdFromName(const std::string& name)
{
    std::string id;
    id.reserve(name.size());

    bool lastWasHyphen = false;
    for (char c : name)
    {
        if (std::isalnum(c))
        {
            id += static_cast<char>(std::tolower(c));
            lastWasHyphen = false;
        }
        else if (c == ' ' || c == '_' || c == '-')
        {
            if (!lastWasHyphen && !id.empty())
            {
                id += '-';
                lastWasHyphen = true;
            }
        }
    }

    while (!id.empty() && id.back() == '-')
    {
        id.pop_back();
    }

    return id;
}

bool AddonCreator::CreateTemplate(const AddonCreateInfo& info, const std::string& targetDir, std::string& outError)
{
    if (info.mName.empty())
    {
        outError = "Template name is required.";
        return false;
    }

    std::string addonId = info.mId.empty() ? GenerateIdFromName(info.mName) : info.mId;
    if (addonId.empty())
    {
        outError = "Could not generate valid ID from name.";
        return false;
    }

    // Ensure target directory exists
    if (!DoesDirExist(targetDir.c_str()))
    {
        CreateDirRecursive(targetDir);
    }

    // Create addon directory
    std::string addonPath = targetDir;
    if (addonPath.back() != '/' && addonPath.back() != '\\')
    {
        addonPath += '/';
    }
    addonPath += addonId + "/";

    if (DoesDirExist(addonPath.c_str()))
    {
        outError = "Template folder already exists: " + addonPath;
        return false;
    }

    SYS_CreateDirectory(addonPath.c_str());

    // Create Assets and Scripts directories
    SYS_CreateDirectory((addonPath + "Assets/").c_str());
    SYS_CreateDirectory((addonPath + "Scripts/").c_str());

    // Write package.json
    std::stringstream ss;
    ss << "{\n";
    ss << "    \"name\": \"" << EscapeJsonString(info.mName) << "\",\n";
    ss << "    \"author\": \"" << EscapeJsonString(info.mAuthor) << "\",\n";
    ss << "    \"description\": \"" << EscapeJsonString(info.mDescription) << "\",\n";
    ss << "    \"version\": \"" << EscapeJsonString(info.mVersion) << "\",\n";
    ss << "    \"type\": \"template\"\n";
    ss << "}\n";

    std::string content = ss.str();
    Stream stream(content.c_str(), (uint32_t)content.size());
    if (!stream.WriteFile((addonPath + "package.json").c_str()))
    {
        outError = "Failed to write package.json.";
        return false;
    }

    LogDebug("Created template addon: %s at %s", addonId.c_str(), addonPath.c_str());
    return true;
}

bool AddonCreator::CreateAddon(const AddonCreateInfo& info, const std::string& targetDir, std::string& outError)
{
    if (info.mName.empty())
    {
        outError = "Addon name is required.";
        return false;
    }

    std::string addonId = info.mId.empty() ? GenerateIdFromName(info.mName) : info.mId;
    if (addonId.empty())
    {
        outError = "Could not generate valid ID from name.";
        return false;
    }

    // Ensure target directory exists
    if (!DoesDirExist(targetDir.c_str()))
    {
        CreateDirRecursive(targetDir);
    }

    // Create addon directory
    std::string addonPath = targetDir;
    if (addonPath.back() != '/' && addonPath.back() != '\\')
    {
        addonPath += '/';
    }
    addonPath += addonId + "/";

    if (DoesDirExist(addonPath.c_str()))
    {
        outError = "Addon folder already exists: " + addonPath;
        return false;
    }

    SYS_CreateDirectory(addonPath.c_str());

    // Create Assets and Scripts directories
    SYS_CreateDirectory((addonPath + "Assets/").c_str());
    SYS_CreateDirectory((addonPath + "Scripts/").c_str());

    // Write package.json
    std::stringstream ss;
    ss << "{\n";
    ss << "    \"name\": \"" << EscapeJsonString(info.mName) << "\",\n";
    ss << "    \"author\": \"" << EscapeJsonString(info.mAuthor) << "\",\n";
    ss << "    \"description\": \"" << EscapeJsonString(info.mDescription) << "\",\n";
    ss << "    \"version\": \"" << EscapeJsonString(info.mVersion) << "\",\n";
    ss << "    \"type\": \"addon\"\n";
    ss << "}\n";

    std::string content = ss.str();
    Stream stream(content.c_str(), (uint32_t)content.size());
    if (!stream.WriteFile((addonPath + "package.json").c_str()))
    {
        outError = "Failed to write package.json.";
        return false;
    }

    LogDebug("Created script addon: %s at %s", addonId.c_str(), addonPath.c_str());
    return true;
}

bool AddonCreator::CreateNativeAddon(const NativeAddonCreateInfo& info, const std::string& targetDir,
                                     std::string& outError, std::string* outPath)
{
    NativeAddonManager* nam = NativeAddonManager::Get();
    if (nam == nullptr)
    {
        outError = "NativeAddonManager not initialized.";
        return false;
    }

    return nam->CreateNativeAddonAtPath(info, targetDir, outError, outPath);
}

static bool WriteInitPackageJson(const std::string& path, const std::string& type, std::string& outError)
{
    // If package.json already exists, read it and merge the type field
    if (SYS_DoesFileExist(path.c_str(), false))
    {
        Stream readStream;
        if (!readStream.ReadFile(path.c_str(), false))
        {
            outError = "Failed to read existing package.json.";
            return false;
        }

        std::string jsonStr(readStream.GetData(), readStream.GetSize());
        rapidjson::Document doc;
        doc.Parse(jsonStr.c_str());

        if (doc.HasParseError())
        {
            outError = "Failed to parse existing package.json.";
            return false;
        }

        auto& allocator = doc.GetAllocator();

        // Set or update the type field
        if (doc.HasMember("type"))
        {
            doc["type"].SetString(type.c_str(), (rapidjson::SizeType)type.size(), allocator);
        }
        else
        {
            doc.AddMember("type", rapidjson::Value(type.c_str(), (rapidjson::SizeType)type.size(), allocator), allocator);
        }

        // Write back
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);

        std::string output = buffer.GetString();
        output += "\n";
        Stream writeStream(output.c_str(), (uint32_t)output.size());
        if (!writeStream.WriteFile(path.c_str()))
        {
            outError = "Failed to write package.json.";
            return false;
        }

        return true;
    }

    // Create new package.json
    std::stringstream ss;
    ss << "{\n";
    ss << "    \"name\": \"\",\n";
    ss << "    \"author\": \"\",\n";
    ss << "    \"description\": \"\",\n";
    ss << "    \"version\": \"1.0.0\",\n";
    ss << "    \"type\": \"" << type << "\"\n";
    ss << "}\n";

    std::string content = ss.str();
    Stream stream(content.c_str(), (uint32_t)content.size());
    if (!stream.WriteFile(path.c_str()))
    {
        outError = "Failed to write package.json.";
        return false;
    }

    return true;
}

bool AddonCreator::InitializeAsTemplate(const std::string& projectDir, std::string& outError)
{
    if (projectDir.empty())
    {
        outError = "No project directory specified.";
        return false;
    }

    std::string packagePath = projectDir + "package.json";
    if (!WriteInitPackageJson(packagePath, "template", outError))
    {
        return false;
    }

    LogDebug("Initialized project as template: %s", projectDir.c_str());
    return true;
}

bool AddonCreator::InitializeAsAddon(const std::string& projectDir, std::string& outError)
{
    if (projectDir.empty())
    {
        outError = "No project directory specified.";
        return false;
    }

    std::string packagePath = projectDir + "package.json";
    if (!WriteInitPackageJson(packagePath, "addon", outError))
    {
        return false;
    }

    LogDebug("Initialized project as addon: %s", projectDir.c_str());
    return true;
}

bool AddonCreator::InitializeAsNativeAddon(const std::string& projectDir, std::string& outError)
{
    if (projectDir.empty())
    {
        outError = "No project directory specified.";
        return false;
    }

    std::string packagePath = projectDir + "package.json";

    // For native addons, add the native section
    if (SYS_DoesFileExist(packagePath.c_str(), false))
    {
        Stream readStream;
        if (!readStream.ReadFile(packagePath.c_str(), false))
        {
            outError = "Failed to read existing package.json.";
            return false;
        }

        std::string jsonStr(readStream.GetData(), readStream.GetSize());
        rapidjson::Document doc;
        doc.Parse(jsonStr.c_str());

        if (doc.HasParseError())
        {
            outError = "Failed to parse existing package.json.";
            return false;
        }

        auto& allocator = doc.GetAllocator();

        // Add type
        if (doc.HasMember("type"))
        {
            doc["type"].SetString("native", allocator);
        }
        else
        {
            doc.AddMember("type", "native", allocator);
        }

        // Add native section if not present
        if (!doc.HasMember("native"))
        {
            rapidjson::Value nativeObj(rapidjson::kObjectType);
            nativeObj.AddMember("target", "engine", allocator);
            nativeObj.AddMember("sourceDir", "Source", allocator);
            nativeObj.AddMember("binaryName", "", allocator);
            nativeObj.AddMember("entrySymbol", "OctavePlugin_GetDesc", allocator);
            nativeObj.AddMember("apiVersion", 1, allocator);
            doc.AddMember("native", nativeObj, allocator);
        }

        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);

        std::string output = buffer.GetString();
        output += "\n";
        Stream writeStream(output.c_str(), (uint32_t)output.size());
        if (!writeStream.WriteFile(packagePath.c_str()))
        {
            outError = "Failed to write package.json.";
            return false;
        }
    }
    else
    {
        std::stringstream ss;
        ss << "{\n";
        ss << "    \"name\": \"\",\n";
        ss << "    \"author\": \"\",\n";
        ss << "    \"description\": \"\",\n";
        ss << "    \"version\": \"1.0.0\",\n";
        ss << "    \"type\": \"native\",\n";
        ss << "    \"native\": {\n";
        ss << "        \"target\": \"engine\",\n";
        ss << "        \"sourceDir\": \"Source\",\n";
        ss << "        \"binaryName\": \"\",\n";
        ss << "        \"entrySymbol\": \"OctavePlugin_GetDesc\",\n";
        ss << "        \"apiVersion\": 1\n";
        ss << "    }\n";
        ss << "}\n";

        std::string content = ss.str();
        Stream stream(content.c_str(), (uint32_t)content.size());
        if (!stream.WriteFile(packagePath.c_str()))
        {
            outError = "Failed to write package.json.";
            return false;
        }
    }

    LogDebug("Initialized project as native addon: %s", projectDir.c_str());
    return true;
}

bool AddonCreator::IsGitAvailable()
{
    int exitCode = -1;
    std::string output;
    SYS_ExecFull("git --version", &output, nullptr, &exitCode);
    return exitCode == 0;
}

bool AddonCreator::HasGitRepo(const std::string& dir)
{
    std::string gitDir = dir;
    if (gitDir.back() != '/' && gitDir.back() != '\\')
    {
        gitDir += '/';
    }
    gitDir += ".git";
    return DoesDirExist(gitDir.c_str());
}

bool AddonCreator::InitGitRepo(const std::string& dir, std::string& outError)
{
    std::string cmd = "git -C \"" + dir + "\" init";
    int exitCode = -1;
    std::string stdOut, stdErr;
    SYS_ExecFull(cmd.c_str(), &stdOut, &stdErr, &exitCode);

    if (exitCode != 0)
    {
        outError = "git init failed: " + stdErr;
        return false;
    }

    return true;
}

bool AddonCreator::HasGitRemote(const std::string& dir)
{
    std::string cmd = "git -C \"" + dir + "\" remote";
    int exitCode = -1;
    std::string stdOut;
    SYS_ExecFull(cmd.c_str(), &stdOut, nullptr, &exitCode);

    if (exitCode != 0)
    {
        return false;
    }

    // Check if output has any content (a remote name)
    for (char c : stdOut)
    {
        if (!std::isspace(c))
        {
            return true;
        }
    }
    return false;
}

bool AddonCreator::PublishViaGit(const std::string& addonDir, const std::string& commitMessage,
                                  bool push, std::string& outError, std::string& outLog)
{
    if (!IsGitAvailable())
    {
        outError = "Git is not installed or not in PATH.";
        return false;
    }

    if (!HasGitRepo(addonDir))
    {
        outError = "No git repository found. Initialize one first.";
        return false;
    }

    if (commitMessage.empty())
    {
        outError = "Commit message is required.";
        return false;
    }

    int exitCode = -1;
    std::string stdOut, stdErr;

    // git add .
    std::string addCmd = "git -C \"" + addonDir + "\" add .";
    SYS_ExecFull(addCmd.c_str(), &stdOut, &stdErr, &exitCode);
    outLog += "$ git add .\n" + stdOut;
    if (exitCode != 0)
    {
        outError = "git add failed: " + stdErr;
        outLog += "ERROR: " + stdErr + "\n";
        return false;
    }

    // git commit
    std::string commitCmd = "git -C \"" + addonDir + "\" commit -m \"" + commitMessage + "\"";
    stdOut.clear();
    stdErr.clear();
    SYS_ExecFull(commitCmd.c_str(), &stdOut, &stdErr, &exitCode);
    outLog += "$ git commit -m \"...\"\n" + stdOut;
    if (exitCode != 0)
    {
        // Exit code 1 with "nothing to commit" is not a hard error
        if (stdOut.find("nothing to commit") != std::string::npos)
        {
            outLog += "Nothing to commit.\n";
        }
        else
        {
            outError = "git commit failed: " + stdErr;
            outLog += "ERROR: " + stdErr + "\n";
            return false;
        }
    }

    // git push (if requested)
    if (push)
    {
        if (!HasGitRemote(addonDir))
        {
            outError = "No git remote configured. Add a remote first.";
            outLog += "WARNING: No remote configured, skipping push.\n";
            return false;
        }

        std::string pushCmd = "git -C \"" + addonDir + "\" push";
        stdOut.clear();
        stdErr.clear();
        SYS_ExecFull(pushCmd.c_str(), &stdOut, &stdErr, &exitCode);
        outLog += "$ git push\n" + stdOut;
        if (exitCode != 0)
        {
            outError = "git push failed: " + stdErr;
            outLog += "ERROR: " + stdErr + "\n";
            return false;
        }
    }

    outLog += "Done.\n";
    return true;
}

bool AddonCreator::PublishAsZip(const std::string& addonDir, const std::string& outputPath, std::string& outError)
{
    if (!DoesDirExist(addonDir.c_str()))
    {
        outError = "Addon directory not found: " + addonDir;
        return false;
    }

    if (outputPath.empty())
    {
        outError = "Output path is required.";
        return false;
    }

    // Ensure output directory exists
    std::string outputDir = outputPath;
    size_t lastSlash = outputDir.find_last_of("/\\");
    if (lastSlash != std::string::npos)
    {
        outputDir = outputDir.substr(0, lastSlash);
        if (!DoesDirExist(outputDir.c_str()))
        {
            CreateDirRecursive(outputDir);
        }
    }

    std::string cmd;
#if PLATFORM_WINDOWS
    // Use PowerShell Compress-Archive
    cmd = "powershell -Command \"Compress-Archive -Path '";
    cmd += addonDir;
    cmd += "*' -DestinationPath '";
    cmd += outputPath;
    cmd += "' -Force\"";
#else
    // Use zip command on Linux
    cmd = "cd \"" + addonDir + "\" && zip -r \"" + outputPath + "\" .";
#endif

    int exitCode = -1;
    std::string stdOut, stdErr;
    SYS_ExecFull(cmd.c_str(), &stdOut, &stdErr, &exitCode);

    if (exitCode != 0)
    {
        outError = "Zip failed: " + stdErr;
        return false;
    }

    LogDebug("Packaged addon to: %s", outputPath.c_str());
    return true;
}

bool AddonCreator::ReadPackageJson(const std::string& path, PackageJsonData& outData, std::string& outError)
{
    if (!SYS_DoesFileExist(path.c_str(), false))
    {
        outError = "File not found: " + path;
        return false;
    }

    Stream stream;
    if (!stream.ReadFile(path.c_str(), false))
    {
        outError = "Failed to read file.";
        return false;
    }

    std::string jsonStr(stream.GetData(), stream.GetSize());
    rapidjson::Document doc;
    doc.Parse(jsonStr.c_str());

    if (doc.HasParseError())
    {
        outError = "JSON parse error.";
        return false;
    }

    // Read known fields
    if (doc.HasMember("name") && doc["name"].IsString())
        outData.mName = doc["name"].GetString();
    if (doc.HasMember("author") && doc["author"].IsString())
        outData.mAuthor = doc["author"].GetString();
    if (doc.HasMember("description") && doc["description"].IsString())
        outData.mDescription = doc["description"].GetString();
    if (doc.HasMember("version") && doc["version"].IsString())
        outData.mVersion = doc["version"].GetString();
    if (doc.HasMember("url") && doc["url"].IsString())
        outData.mUrl = doc["url"].GetString();
    if (doc.HasMember("type") && doc["type"].IsString())
        outData.mType = doc["type"].GetString();

    // Read tags
    if (doc.HasMember("tags") && doc["tags"].IsArray())
    {
        const rapidjson::Value& tags = doc["tags"];
        std::string tagStr;
        for (rapidjson::SizeType i = 0; i < tags.Size(); ++i)
        {
            if (tags[i].IsString())
            {
                if (!tagStr.empty())
                    tagStr += ", ";
                tagStr += tags[i].GetString();
            }
        }
        outData.mTags = tagStr;
    }

    // Read native section
    if (doc.HasMember("native") && doc["native"].IsObject())
    {
        outData.mHasNative = true;
        const rapidjson::Value& native = doc["native"];

        if (native.HasMember("target") && native["target"].IsString())
            outData.mNativeTarget = native["target"].GetString();
        if (native.HasMember("sourceDir") && native["sourceDir"].IsString())
            outData.mSourceDir = native["sourceDir"].GetString();
        if (native.HasMember("binaryName") && native["binaryName"].IsString())
            outData.mBinaryName = native["binaryName"].GetString();
        if (native.HasMember("entrySymbol") && native["entrySymbol"].IsString())
            outData.mEntrySymbol = native["entrySymbol"].GetString();
        if (native.HasMember("apiVersion") && native["apiVersion"].IsUint())
            outData.mApiVersion = native["apiVersion"].GetUint();
    }

    return true;
}

bool AddonCreator::WritePackageJson(const std::string& path, const PackageJsonData& data, std::string& outError)
{
    // Read existing document to preserve unknown fields
    rapidjson::Document doc;

    if (SYS_DoesFileExist(path.c_str(), false))
    {
        Stream readStream;
        if (readStream.ReadFile(path.c_str(), false))
        {
            std::string jsonStr(readStream.GetData(), readStream.GetSize());
            doc.Parse(jsonStr.c_str());
        }
    }

    if (!doc.IsObject())
    {
        doc.SetObject();
    }

    auto& allocator = doc.GetAllocator();

    // Helper to set or add a string member
    auto setString = [&](const char* key, const std::string& value) {
        if (doc.HasMember(key))
        {
            doc[key].SetString(value.c_str(), (rapidjson::SizeType)value.size(), allocator);
        }
        else
        {
            rapidjson::Value k(key, allocator);
            rapidjson::Value v(value.c_str(), (rapidjson::SizeType)value.size(), allocator);
            doc.AddMember(k, v, allocator);
        }
    };

    setString("name", data.mName);
    setString("author", data.mAuthor);
    setString("description", data.mDescription);
    setString("version", data.mVersion);
    setString("url", data.mUrl);

    if (!data.mType.empty())
    {
        setString("type", data.mType);
    }

    // Write tags as array
    if (!data.mTags.empty())
    {
        rapidjson::Value tagsArray(rapidjson::kArrayType);
        std::stringstream ss(data.mTags);
        std::string tag;
        while (std::getline(ss, tag, ','))
        {
            // Trim whitespace
            size_t start = tag.find_first_not_of(" \t");
            size_t end = tag.find_last_not_of(" \t");
            if (start != std::string::npos)
            {
                tag = tag.substr(start, end - start + 1);
                tagsArray.PushBack(rapidjson::Value(tag.c_str(), (rapidjson::SizeType)tag.size(), allocator), allocator);
            }
        }

        if (doc.HasMember("tags"))
        {
            doc["tags"] = tagsArray;
        }
        else
        {
            doc.AddMember("tags", tagsArray, allocator);
        }
    }

    // Write native section
    if (data.mHasNative)
    {
        rapidjson::Value nativeObj(rapidjson::kObjectType);

        // If existing native section, start from that to preserve unknown fields
        if (doc.HasMember("native") && doc["native"].IsObject())
        {
            nativeObj.CopyFrom(doc["native"], allocator);
        }

        auto setNativeString = [&](const char* key, const std::string& value) {
            if (nativeObj.HasMember(key))
            {
                nativeObj[key].SetString(value.c_str(), (rapidjson::SizeType)value.size(), allocator);
            }
            else
            {
                rapidjson::Value k(key, allocator);
                rapidjson::Value v(value.c_str(), (rapidjson::SizeType)value.size(), allocator);
                nativeObj.AddMember(k, v, allocator);
            }
        };

        setNativeString("target", data.mNativeTarget);
        setNativeString("sourceDir", data.mSourceDir);
        setNativeString("binaryName", data.mBinaryName);
        setNativeString("entrySymbol", data.mEntrySymbol);

        if (nativeObj.HasMember("apiVersion"))
        {
            nativeObj["apiVersion"].SetInt(data.mApiVersion);
        }
        else
        {
            nativeObj.AddMember("apiVersion", data.mApiVersion, allocator);
        }

        if (doc.HasMember("native"))
        {
            doc["native"] = nativeObj;
        }
        else
        {
            doc.AddMember("native", nativeObj, allocator);
        }
    }
    else
    {
        // Remove native section if unchecked
        if (doc.HasMember("native"))
        {
            doc.RemoveMember("native");
        }
    }

    // Write to file
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    std::string output = buffer.GetString();
    output += "\n";
    Stream writeStream(output.c_str(), (uint32_t)output.size());
    if (!writeStream.WriteFile(path.c_str()))
    {
        outError = "Failed to write file.";
        return false;
    }

    return true;
}

void AddonCreator::OpenInExternalEditor(const std::string& path)
{
#if PLATFORM_WINDOWS
    SYS_Exec(("start \"\" \"" + path + "\"").c_str());
#elif PLATFORM_LINUX
    SYS_Exec(("xdg-open \"" + path + "\" &").c_str());
#endif
}

std::vector<UserAddonInfo> AddonCreator::GetUserCreatedAddons(const std::string& projectDir)
{
    std::vector<UserAddonInfo> result;

    if (projectDir.empty())
    {
        return result;
    }

    std::string packagesDir = projectDir + "Packages/";
    if (!DoesDirExist(packagesDir.c_str()))
    {
        return result;
    }

    // Build set of installed addon IDs (downloaded from repos)
    std::vector<std::string> installedIds;
    AddonManager* am = AddonManager::Get();
    if (am != nullptr)
    {
        am->LoadInstalledAddons();
        const std::vector<InstalledAddon>& installed = am->GetInstalledAddons();
        for (const InstalledAddon& inst : installed)
        {
            installedIds.push_back(inst.mId);
        }
    }

    // Scan Packages/ directory
    DirEntry dirEntry;
    SYS_OpenDirectory(packagesDir, dirEntry);

    while (dirEntry.mValid)
    {
        if (dirEntry.mDirectory &&
            strcmp(dirEntry.mFilename, ".") != 0 &&
            strcmp(dirEntry.mFilename, "..") != 0)
        {
            std::string addonId = dirEntry.mFilename;
            std::string addonPath = packagesDir + addonId + "/";
            std::string packageJsonPath = addonPath + "package.json";

            // Check if this addon is NOT in installed_addons.json (i.e., user-created)
            bool isInstalled = false;
            for (const std::string& id : installedIds)
            {
                if (id == addonId)
                {
                    isInstalled = true;
                    break;
                }
            }

            if (!isInstalled && SYS_DoesFileExist(packageJsonPath.c_str(), false))
            {
                UserAddonInfo info;
                info.mId = addonId;
                info.mPath = addonPath;

                // Read name and type from package.json
                Stream stream;
                if (stream.ReadFile(packageJsonPath.c_str(), false))
                {
                    std::string jsonStr(stream.GetData(), stream.GetSize());
                    rapidjson::Document doc;
                    doc.Parse(jsonStr.c_str());

                    if (!doc.HasParseError())
                    {
                        if (doc.HasMember("name") && doc["name"].IsString())
                            info.mName = doc["name"].GetString();
                        if (doc.HasMember("type") && doc["type"].IsString())
                            info.mType = doc["type"].GetString();
                        if (doc.HasMember("native") && doc["native"].IsObject())
                            info.mType = "native";
                    }
                }

                if (info.mName.empty())
                {
                    info.mName = addonId;
                }

                result.push_back(info);
            }
        }

        SYS_IterateDirectory(dirEntry);
    }
    SYS_CloseDirectory(dirEntry);

    return result;
}

#endif // EDITOR
