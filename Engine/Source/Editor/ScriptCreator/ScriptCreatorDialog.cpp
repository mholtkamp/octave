#if EDITOR

#include "ScriptCreatorDialog.h"
#include "Engine.h"
#include "Stream.h"
#include "Log.h"
#include "System/System.h"
#include "Addons/NativeAddonManager.h"

#include "imgui.h"

#include <cstring>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>

// ===== Datum type names for the combo (excluding Table/Function) =====

static const char* sDatumTypeNames[] =
{
    "Integer",
    "Float",
    "Bool",
    "String",
    "Vector2D",
    "Vector",
    "Color",
    "Asset",
    "Byte",
    "Node",
    "Short"
};
static const int sDatumTypeCount = sizeof(sDatumTypeNames) / sizeof(sDatumTypeNames[0]);

// Lua DatumType names matching the engine enum
static const char* sDatumTypeLuaNames[] =
{
    "DatumType.Integer",
    "DatumType.Float",
    "DatumType.Bool",
    "DatumType.String",
    "DatumType.Vector2D",
    "DatumType.Vector",
    "DatumType.Color",
    "DatumType.Asset",
    "DatumType.Byte",
    "DatumType.Node",
    "DatumType.Short"
};

// ===== Script Property Entry =====

struct ScriptPropertyEntry
{
    char mName[128] = {};
    int mTypeIndex = 0; // index into sDatumTypeNames

    // Default values
    int32_t mIntValue = 0;
    float mFloatValue = 0.0f;
    bool mBoolValue = false;
    char mStringValue[256] = {};
    float mVec2[2] = {};
    float mVec3[3] = {};
    float mVec4[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    int32_t mByteValue = 0;
    int32_t mShortValue = 0;
};

// ===== Create Script Dialog State =====

static bool sShowCreateScriptDialog = false;
static char sScriptClassName[256] = {};
static bool sExtendEnabled = false;
static int sExtendMode = 0; // 0 = Lua Script, 1 = C++ Node Type
static char sParentScriptName[256] = {};
static char sParentNodeType[256] = {};
static std::vector<ScriptPropertyEntry> sScriptProperties;
static char sScriptSaveDir[512] = {};
static bool sCustomSaveLocation = false;
static bool sIncludeTick = true;
static bool sIncludeDestroy = false;
static bool sIncludeBeginOverlap = false;
static bool sIncludeEndOverlap = false;
static std::string sScriptError;
static std::string sScriptSuccess;

// ===== Create C++ File Dialog State =====

static bool sShowCreateCppFileDialog = false;
static int sCppSelectedAddon = 0;
static char sCppFileName[256] = {};
static std::string sCppFileError;
static std::string sCppFileSuccess;

// ===== Helpers =====

static std::string GetProjectDir()
{
    return GetEngineState()->mProjectDirectory;
}

static bool IsValidLuaIdentifier(const char* name)
{
    if (!name || name[0] == '\0')
        return false;
    // Must start with letter or underscore
    if (!std::isalpha((unsigned char)name[0]) && name[0] != '_')
        return false;
    for (int i = 1; name[i] != '\0'; ++i)
    {
        if (!std::isalnum((unsigned char)name[i]) && name[i] != '_')
            return false;
    }
    return true;
}

static void OpenInExplorer(const std::string& path)
{
#if PLATFORM_WINDOWS
    SYS_Exec(("start \"\" \"" + path + "\"").c_str());
#elif PLATFORM_LINUX
    SYS_Exec(("xdg-open \"" + path + "\" &").c_str());
#endif
}

static std::string FormatDefaultValue(const ScriptPropertyEntry& prop)
{
    switch (prop.mTypeIndex)
    {
    case 0: // Integer
        return std::to_string(prop.mIntValue);
    case 1: // Float
    {
        std::ostringstream oss;
        oss << prop.mFloatValue;
        std::string s = oss.str();
        if (s.find('.') == std::string::npos)
            s += ".0";
        return s;
    }
    case 2: // Bool
        return prop.mBoolValue ? "true" : "false";
    case 3: // String
        return std::string("\"") + prop.mStringValue + "\"";
    case 4: // Vector2D
    {
        std::ostringstream oss;
        oss << "Vec2.New(" << prop.mVec2[0] << ", " << prop.mVec2[1] << ")";
        return oss.str();
    }
    case 5: // Vector
    {
        std::ostringstream oss;
        oss << "Vec.New(" << prop.mVec3[0] << ", " << prop.mVec3[1] << ", " << prop.mVec3[2] << ")";
        return oss.str();
    }
    case 6: // Color
    {
        std::ostringstream oss;
        oss << "Vec4.New(" << prop.mVec4[0] << ", " << prop.mVec4[1] << ", " << prop.mVec4[2] << ", " << prop.mVec4[3] << ")";
        return oss.str();
    }
    case 7: // Asset
    case 9: // Node
        return "nil";
    case 8: // Byte
        return std::to_string(prop.mByteValue);
    case 10: // Short
        return std::to_string(prop.mShortValue);
    default:
        return "nil";
    }
}

// ===== Script Generation =====

static std::string GenerateLuaScript()
{
    std::string cls = sScriptClassName;
    std::ostringstream ss;

    // Header: Require if extending Lua parent
    if (sExtendEnabled && sExtendMode == 0 && sParentScriptName[0] != '\0')
    {
        ss << "Script.Require(\"" << sParentScriptName << "\")\n\n";
    }

    // Class table
    ss << cls << " = {}\n";

    // Extend/Inherit
    if (sExtendEnabled)
    {
        if (sExtendMode == 0 && sParentScriptName[0] != '\0')
        {
            ss << "Script.Extend(" << cls << ", " << sParentScriptName << ")\n";
        }
        else if (sExtendMode == 1 && sParentNodeType[0] != '\0')
        {
            ss << "Script.Inherit(" << cls << ", \"" << sParentNodeType << "\")\n";
        }
    }

    ss << "\n";

    bool hasLuaParent = sExtendEnabled && sExtendMode == 0 && sParentScriptName[0] != '\0';
    std::string parent = sParentScriptName;

    // Create()
    ss << "function " << cls << ":Create()\n";
    if (hasLuaParent)
    {
        ss << "    " << parent << ".Create(self)\n";
    }
    for (const ScriptPropertyEntry& prop : sScriptProperties)
    {
        if (prop.mName[0] != '\0')
        {
            ss << "    self." << prop.mName << " = " << FormatDefaultValue(prop) << "\n";
        }
    }
    ss << "end\n\n";

    // GatherProperties() - only if there are properties
    if (!sScriptProperties.empty())
    {
        bool hasProps = false;
        for (const ScriptPropertyEntry& prop : sScriptProperties)
        {
            if (prop.mName[0] != '\0')
            {
                hasProps = true;
                break;
            }
        }

        if (hasProps)
        {
            ss << "function " << cls << ":GatherProperties()\n";
            ss << "    return {\n";
            for (const ScriptPropertyEntry& prop : sScriptProperties)
            {
                if (prop.mName[0] != '\0')
                {
                    ss << "        { name = \"" << prop.mName << "\", type = " << sDatumTypeLuaNames[prop.mTypeIndex] << " },\n";
                }
            }
            ss << "    }\n";
            ss << "end\n\n";
        }
    }

    // Optional stubs
    if (sIncludeTick)
    {
        ss << "function " << cls << ":Tick(deltaTime)\n";
        if (hasLuaParent)
            ss << "    " << parent << ".Tick(self, deltaTime)\n";
        ss << "end\n\n";
    }

    if (sIncludeDestroy)
    {
        ss << "function " << cls << ":Destroy()\n";
        if (hasLuaParent)
            ss << "    " << parent << ".Destroy(self)\n";
        ss << "end\n\n";
    }

    if (sIncludeBeginOverlap)
    {
        ss << "function " << cls << ":BeginOverlap(other)\n";
        if (hasLuaParent)
            ss << "    " << parent << ".BeginOverlap(self, other)\n";
        ss << "end\n\n";
    }

    if (sIncludeEndOverlap)
    {
        ss << "function " << cls << ":EndOverlap(other)\n";
        if (hasLuaParent)
            ss << "    " << parent << ".EndOverlap(self, other)\n";
        ss << "end\n\n";
    }

    return ss.str();
}

// ===== Validate and Create =====

static bool ValidateAndCreateScript()
{
    sScriptError.clear();
    sScriptSuccess.clear();

    // Validate class name
    if (sScriptClassName[0] == '\0')
    {
        sScriptError = "Class name is required.";
        return false;
    }

    if (!IsValidLuaIdentifier(sScriptClassName))
    {
        sScriptError = "Class name must be a valid Lua identifier (letters, digits, underscores; cannot start with digit).";
        return false;
    }

    // Validate extends
    if (sExtendEnabled)
    {
        if (sExtendMode == 0)
        {
            if (sParentScriptName[0] == '\0')
            {
                sScriptError = "Parent Lua script name is required when extending.";
                return false;
            }
            if (!IsValidLuaIdentifier(sParentScriptName))
            {
                sScriptError = "Parent script name must be a valid Lua identifier.";
                return false;
            }
        }
        else if (sExtendMode == 1)
        {
            if (sParentNodeType[0] == '\0')
            {
                sScriptError = "Parent C++ node type is required when extending.";
                return false;
            }
        }
    }

    // Check duplicate property names
    for (size_t i = 0; i < sScriptProperties.size(); ++i)
    {
        if (sScriptProperties[i].mName[0] == '\0')
            continue;
        for (size_t j = i + 1; j < sScriptProperties.size(); ++j)
        {
            if (strcmp(sScriptProperties[i].mName, sScriptProperties[j].mName) == 0)
            {
                sScriptError = "Duplicate property name: " + std::string(sScriptProperties[i].mName);
                return false;
            }
        }
    }

    // Determine save path
    std::string saveDir;
    if (sCustomSaveLocation && sScriptSaveDir[0] != '\0')
    {
        saveDir = sScriptSaveDir;
    }
    else
    {
        saveDir = GetProjectDir() + "Scripts/";
    }

    // Ensure trailing slash
    if (!saveDir.empty() && saveDir.back() != '/' && saveDir.back() != '\\')
        saveDir += '/';

    std::string filePath = saveDir + sScriptClassName + ".lua";

    // Check if file already exists
    if (SYS_DoesFileExist(filePath.c_str(), false))
    {
        sScriptError = "File already exists: " + filePath;
        return false;
    }

    // Ensure directory exists
    SYS_CreateDirectory(saveDir.c_str());

    // Generate script content
    std::string content = GenerateLuaScript();
    Stream stream(content.c_str(), (uint32_t)content.size());
    if (!stream.WriteFile(filePath.c_str()))
    {
        sScriptError = "Failed to write file: " + filePath;
        return false;
    }

    LogDebug("Created Lua script: %s", filePath.c_str());
    sScriptSuccess = "Created: " + filePath;
    return true;
}

// ===== Draw Property Default Widget =====

static void DrawPropertyDefault(ScriptPropertyEntry& prop, int index)
{
    ImGui::PushID(index);

    switch (prop.mTypeIndex)
    {
    case 0: // Integer
        ImGui::SetNextItemWidth(-1);
        ImGui::InputInt("##defInt", &prop.mIntValue);
        break;
    case 1: // Float
        ImGui::SetNextItemWidth(-1);
        ImGui::InputFloat("##defFloat", &prop.mFloatValue, 0.0f, 0.0f, "%.3f");
        break;
    case 2: // Bool
        ImGui::Checkbox("##defBool", &prop.mBoolValue);
        break;
    case 3: // String
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##defStr", prop.mStringValue, sizeof(prop.mStringValue));
        break;
    case 4: // Vector2D
        ImGui::SetNextItemWidth(-1);
        ImGui::InputFloat2("##defVec2", prop.mVec2, "%.2f");
        break;
    case 5: // Vector
        ImGui::SetNextItemWidth(-1);
        ImGui::InputFloat3("##defVec3", prop.mVec3, "%.2f");
        break;
    case 6: // Color
        ImGui::SetNextItemWidth(-1);
        ImGui::ColorEdit4("##defColor", prop.mVec4);
        break;
    case 7: // Asset
    case 9: // Node
        ImGui::TextDisabled("nil");
        break;
    case 8: // Byte
        ImGui::SetNextItemWidth(-1);
        ImGui::InputInt("##defByte", &prop.mByteValue);
        if (prop.mByteValue < 0) prop.mByteValue = 0;
        if (prop.mByteValue > 255) prop.mByteValue = 255;
        break;
    case 10: // Short
        ImGui::SetNextItemWidth(-1);
        ImGui::InputInt("##defShort", &prop.mShortValue);
        if (prop.mShortValue < -32768) prop.mShortValue = -32768;
        if (prop.mShortValue > 32767) prop.mShortValue = 32767;
        break;
    }

    ImGui::PopID();
}

// ===== Draw Create Script Dialog =====

static void DrawCreateScriptDialog()
{
    if (!sShowCreateScriptDialog)
        return;

    ImGui::SetNextWindowSize(ImVec2(550, 550), ImGuiCond_Appearing);
    ImVec2 center = ImGui::GetIO().DisplaySize;
    center.x *= 0.5f;
    center.y *= 0.5f;
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (!ImGui::Begin("Create Lua Script", &sShowCreateScriptDialog))
    {
        ImGui::End();
        return;
    }

    // Class Name
    ImGui::Text("Class Name *");
    ImGui::SameLine(120);
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##ClassName", sScriptClassName, sizeof(sScriptClassName));

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Extends
    ImGui::Text("Extends:");
    ImGui::Checkbox("Extend a parent class", &sExtendEnabled);
    if (sExtendEnabled)
    {
        ImGui::Indent(20.0f);
        ImGui::RadioButton("Lua Script", &sExtendMode, 0);
        ImGui::SameLine(130);
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##ParentScript", sParentScriptName, sizeof(sParentScriptName));

        ImGui::RadioButton("C++ Node Type", &sExtendMode, 1);
        ImGui::SameLine(130);
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##ParentNode", sParentNodeType, sizeof(sParentNodeType));
        ImGui::Unindent(20.0f);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Optional Stubs
    ImGui::Text("Optional Stubs:");
    ImGui::Checkbox("Tick", &sIncludeTick);
    ImGui::SameLine();
    ImGui::Checkbox("Destroy", &sIncludeDestroy);
    ImGui::SameLine();
    ImGui::Checkbox("BeginOverlap", &sIncludeBeginOverlap);
    ImGui::SameLine();
    ImGui::Checkbox("EndOverlap", &sIncludeEndOverlap);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Properties header
    ImGui::Text("Properties");
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 60);
    if (ImGui::Button("+ Add"))
    {
        sScriptProperties.push_back(ScriptPropertyEntry());
    }

    // Properties table
    if (!sScriptProperties.empty())
    {
        ImGui::BeginChild("##PropertiesList", ImVec2(0, 200), true);

        // Header
        ImGui::Columns(4, "##PropColumns", true);
        ImGui::SetColumnWidth(0, 140);
        ImGui::SetColumnWidth(1, 100);
        ImGui::SetColumnWidth(2, 200);
        ImGui::SetColumnWidth(3, 30);
        ImGui::Text("Name");
        ImGui::NextColumn();
        ImGui::Text("Type");
        ImGui::NextColumn();
        ImGui::Text("Default");
        ImGui::NextColumn();
        ImGui::Text("");
        ImGui::NextColumn();
        ImGui::Separator();

        int removeIdx = -1;
        for (int i = 0; i < (int)sScriptProperties.size(); ++i)
        {
            ImGui::PushID(i);
            ScriptPropertyEntry& prop = sScriptProperties[i];

            ImGui::SetNextItemWidth(-1);
            ImGui::InputText("##propName", prop.mName, sizeof(prop.mName));
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            ImGui::Combo("##propType", &prop.mTypeIndex, sDatumTypeNames, sDatumTypeCount);
            ImGui::NextColumn();

            DrawPropertyDefault(prop, i);
            ImGui::NextColumn();

            if (ImGui::Button("X"))
            {
                removeIdx = i;
            }
            ImGui::NextColumn();

            ImGui::PopID();
        }

        ImGui::Columns(1);
        ImGui::EndChild();

        if (removeIdx >= 0)
        {
            sScriptProperties.erase(sScriptProperties.begin() + removeIdx);
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Save Location
    ImGui::Text("Save Location:");
    {
        std::string displayPath;
        if (sCustomSaveLocation && sScriptSaveDir[0] != '\0')
        {
            displayPath = std::string(sScriptSaveDir) + "/" + sScriptClassName + ".lua";
        }
        else
        {
            displayPath = GetProjectDir() + "Scripts/" + sScriptClassName + ".lua";
        }
        ImGui::TextDisabled("%s", displayPath.c_str());
    }

    ImGui::Checkbox("Custom location", &sCustomSaveLocation);
    if (sCustomSaveLocation)
    {
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 80);
        ImGui::InputText("##SaveDir", sScriptSaveDir, sizeof(sScriptSaveDir));
        ImGui::SameLine();
        if (ImGui::Button("Browse..."))
        {
            std::string folder = SYS_SelectFolderDialog();
            if (!folder.empty())
            {
                strncpy(sScriptSaveDir, folder.c_str(), sizeof(sScriptSaveDir) - 1);
                sScriptSaveDir[sizeof(sScriptSaveDir) - 1] = '\0';
            }
        }
    }

    ImGui::Spacing();

    // Error / Success
    if (!sScriptError.empty())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        ImGui::TextWrapped("%s", sScriptError.c_str());
        ImGui::PopStyleColor();
    }
    if (!sScriptSuccess.empty())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.3f, 1.0f));
        ImGui::TextWrapped("%s", sScriptSuccess.c_str());
        ImGui::PopStyleColor();
    }

    // Buttons
    ImGui::Spacing();
    if (ImGui::Button("Create", ImVec2(80, 0)))
    {
        if (ValidateAndCreateScript())
        {
            // Keep dialog open to show success
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(80, 0)))
    {
        sShowCreateScriptDialog = false;
    }

    ImGui::End();
}

// ===== C++ File Creation =====

static bool ValidateAndCreateCppFiles()
{
    sCppFileError.clear();
    sCppFileSuccess.clear();

    if (sCppFileName[0] == '\0')
    {
        sCppFileError = "File name is required.";
        return false;
    }

    // Validate name (alphanumeric + underscore)
    for (int i = 0; sCppFileName[i] != '\0'; ++i)
    {
        char c = sCppFileName[i];
        if (!std::isalnum((unsigned char)c) && c != '_')
        {
            sCppFileError = "File name must contain only letters, digits, and underscores.";
            return false;
        }
    }

    NativeAddonManager* nam = NativeAddonManager::Get();
    if (!nam)
    {
        sCppFileError = "Native addon manager not available.";
        return false;
    }

    std::vector<std::string> addonIds = nam->GetDiscoveredAddonIds();
    if (sCppSelectedAddon < 0 || sCppSelectedAddon >= (int)addonIds.size())
    {
        sCppFileError = "No addon selected.";
        return false;
    }

    const NativeAddonState* state = nam->GetState(addonIds[sCppSelectedAddon]);
    if (!state)
    {
        sCppFileError = "Addon state not found.";
        return false;
    }

    std::string sourceDir = state->mSourcePath + state->mNativeMetadata.mSourceDir + "/";
    std::string headerPath = sourceDir + sCppFileName + ".h";
    std::string sourcePath = sourceDir + sCppFileName + ".cpp";

    if (SYS_DoesFileExist(headerPath.c_str(), false))
    {
        sCppFileError = "File already exists: " + headerPath;
        return false;
    }

    if (SYS_DoesFileExist(sourcePath.c_str(), false))
    {
        sCppFileError = "File already exists: " + sourcePath;
        return false;
    }

    // Generate header
    {
        std::ostringstream ss;
        ss << "#pragma once\n\n";
        ss << "class " << sCppFileName << "\n";
        ss << "{\n";
        ss << "public:\n";
        ss << "    " << sCppFileName << "();\n";
        ss << "    ~" << sCppFileName << "();\n";
        ss << "};\n";

        std::string content = ss.str();
        Stream stream(content.c_str(), (uint32_t)content.size());
        if (!stream.WriteFile(headerPath.c_str()))
        {
            sCppFileError = "Failed to write: " + headerPath;
            return false;
        }
    }

    // Generate source
    {
        std::ostringstream ss;
        ss << "#include \"" << sCppFileName << ".h\"\n\n";
        ss << sCppFileName << "::" << sCppFileName << "()\n";
        ss << "{\n";
        ss << "}\n\n";
        ss << sCppFileName << "::~" << sCppFileName << "()\n";
        ss << "{\n";
        ss << "}\n";

        std::string content = ss.str();
        Stream stream(content.c_str(), (uint32_t)content.size());
        if (!stream.WriteFile(sourcePath.c_str()))
        {
            sCppFileError = "Failed to write: " + sourcePath;
            return false;
        }
    }

    LogDebug("Created C++ files: %s.h/.cpp in %s", sCppFileName, sourceDir.c_str());
    sCppFileSuccess = "Created: " + headerPath + " and " + sourcePath;
    return true;
}

static void DrawCreateCppFileDialog()
{
    if (!sShowCreateCppFileDialog)
        return;

    ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Appearing);
    ImVec2 center = ImGui::GetIO().DisplaySize;
    center.x *= 0.5f;
    center.y *= 0.5f;
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (!ImGui::Begin("Create New C++ File", &sShowCreateCppFileDialog))
    {
        ImGui::End();
        return;
    }

    NativeAddonManager* nam = NativeAddonManager::Get();
    std::vector<std::string> addonIds;
    if (nam)
        addonIds = nam->GetDiscoveredAddonIds();

    // Addon combo
    ImGui::Text("Addon:");
    ImGui::SameLine(80);
    ImGui::SetNextItemWidth(-1);
    if (ImGui::BeginCombo("##AddonCombo", sCppSelectedAddon < (int)addonIds.size() ? addonIds[sCppSelectedAddon].c_str() : ""))
    {
        for (int i = 0; i < (int)addonIds.size(); ++i)
        {
            bool selected = (i == sCppSelectedAddon);
            if (ImGui::Selectable(addonIds[i].c_str(), selected))
                sCppSelectedAddon = i;
            if (selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    // File name
    ImGui::Text("Name:");
    ImGui::SameLine(80);
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##CppFileName", sCppFileName, sizeof(sCppFileName));

    ImGui::Spacing();

    // Error / Success
    if (!sCppFileError.empty())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        ImGui::TextWrapped("%s", sCppFileError.c_str());
        ImGui::PopStyleColor();
    }
    if (!sCppFileSuccess.empty())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.3f, 1.0f));
        ImGui::TextWrapped("%s", sCppFileSuccess.c_str());
        ImGui::PopStyleColor();
    }

    ImGui::Spacing();
    if (ImGui::Button("Create", ImVec2(80, 0)))
    {
        ValidateAndCreateCppFiles();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(80, 0)))
    {
        sShowCreateCppFileDialog = false;
    }

    ImGui::End();
}

// ===== Public API =====

void OpenCreateScriptDialog()
{
    sShowCreateScriptDialog = true;
    memset(sScriptClassName, 0, sizeof(sScriptClassName));
    sExtendEnabled = false;
    sExtendMode = 0;
    memset(sParentScriptName, 0, sizeof(sParentScriptName));
    memset(sParentNodeType, 0, sizeof(sParentNodeType));
    sScriptProperties.clear();
    memset(sScriptSaveDir, 0, sizeof(sScriptSaveDir));
    sCustomSaveLocation = false;
    sIncludeTick = true;
    sIncludeDestroy = false;
    sIncludeBeginOverlap = false;
    sIncludeEndOverlap = false;
    sScriptError.clear();
    sScriptSuccess.clear();
}

void OpenCreateCppFileDialog()
{
    sShowCreateCppFileDialog = true;
    sCppSelectedAddon = 0;
    memset(sCppFileName, 0, sizeof(sCppFileName));
    sCppFileError.clear();
    sCppFileSuccess.clear();
}

void DrawScriptCreatorDialogs()
{
    DrawCreateScriptDialog();
    DrawCreateCppFileDialog();
}

#endif // EDITOR
