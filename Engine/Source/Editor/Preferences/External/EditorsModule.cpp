#if EDITOR

#include "EditorsModule.h"
#include "../JsonSettings.h"
#include "ScriptEditor/ScriptEditorWindow.h"

#include "System/System.h"
#include "Log.h"

#include "document.h"
#include "imgui.h"

DEFINE_PREFERENCES_MODULE(EditorsModule, "Editors", "External")

EditorsModule::EditorsModule()
{
}

EditorsModule::~EditorsModule()
{
}

void EditorsModule::Render()
{
    bool changed = false;

    // Lua Editor settings
    ImGui::Text("Lua Editor");
    ImGui::Separator();

    if (DrawPathInput("Path##LuaEditor", mLuaEditorPath, "Select Lua Editor Executable"))
    {
        changed = true;
    }

    ImGui::SetNextItemWidth(-1);
    char luaArgsBuffer[512];
    strncpy(luaArgsBuffer, mLuaEditorArgs.c_str(), sizeof(luaArgsBuffer) - 1);
    luaArgsBuffer[sizeof(luaArgsBuffer) - 1] = '\0';
    if (ImGui::InputText("Args##LuaEditor", luaArgsBuffer, sizeof(luaArgsBuffer)))
    {
        mLuaEditorArgs = luaArgsBuffer;
        changed = true;
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Command-line arguments. Default: {editor} {file}");
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // C++ Editor settings
    ImGui::Text("C++ Editor / IDE");
    ImGui::Separator();

    if (DrawPathInput("Path##CppEditor", mCppEditorPath, "Select C++ Editor Executable"))
    {
        changed = true;
    }

    ImGui::SetNextItemWidth(-1);
    char cppArgsBuffer[512];
    strncpy(cppArgsBuffer, mCppEditorArgs.c_str(), sizeof(cppArgsBuffer) - 1);
    cppArgsBuffer[sizeof(cppArgsBuffer) - 1] = '\0';
    if (ImGui::InputText("Args##CppEditor", cppArgsBuffer, sizeof(cppArgsBuffer)))
    {
        mCppEditorArgs = cppArgsBuffer;
        changed = true;
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Command-line arguments. Default: {editor} {project}");
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // Internal Editor
    ImGui::Text("Internal Editor");
    ImGui::Separator();
    if (ImGui::Checkbox("Use Internal Editor", &mUseInternalEditor))
    {
        changed = true;
    }
    ImGui::TextDisabled("Open Lua scripts in the built-in Script Editor tab instead of an external program.");

    ImGui::Spacing();
    ImGui::Spacing();

    // Placeholder help
    ImGui::TextDisabled("Placeholders: {editor}, {file}, {filedir}, {project}");

    if (changed)
    {
        SetDirty(true);
    }
}

void EditorsModule::LoadSettings(const rapidjson::Document& doc)
{
    mLuaEditorPath = JsonSettings::GetString(doc, "luaEditorPath", "");
    mLuaEditorArgs = JsonSettings::GetString(doc, "luaEditorArgs", "{editor} {file}");
    mCppEditorPath = JsonSettings::GetString(doc, "cppEditorPath", "");
    mCppEditorArgs = JsonSettings::GetString(doc, "cppEditorArgs", "{editor} {project}");
    mUseInternalEditor = JsonSettings::GetBool(doc, "useInternalEditor", false);
}

void EditorsModule::SaveSettings(rapidjson::Document& doc)
{
    JsonSettings::SetString(doc, "luaEditorPath", mLuaEditorPath);
    JsonSettings::SetString(doc, "luaEditorArgs", mLuaEditorArgs);
    JsonSettings::SetString(doc, "cppEditorPath", mCppEditorPath);
    JsonSettings::SetString(doc, "cppEditorArgs", mCppEditorArgs);
    JsonSettings::SetBool(doc, "useInternalEditor", mUseInternalEditor);
}

bool EditorsModule::IsLuaEditorConfigured() const
{
    return !mLuaEditorPath.empty();
}

bool EditorsModule::IsCppEditorConfigured() const
{
    return !mCppEditorPath.empty();
}

std::string EditorsModule::BuildLuaOpenCommand(const std::string& filePath) const
{
    std::string args = mLuaEditorArgs.empty() ? "{editor} {file}" : mLuaEditorArgs;

    // Get file directory
    std::string fileDir = filePath;
    size_t lastSlash = fileDir.find_last_of("/\\");
    if (lastSlash != std::string::npos)
    {
        fileDir = fileDir.substr(0, lastSlash);
    }

    std::string cmd = args;
    ReplaceAll(cmd, "{editor}", "\"" + mLuaEditorPath + "\"");
    ReplaceAll(cmd, "{file}", "\"" + filePath + "\"");
    ReplaceAll(cmd, "{filedir}", "\"" + fileDir + "\"");

#if PLATFORM_WINDOWS
    cmd = "start \"\" " + cmd;
#elif PLATFORM_LINUX
    cmd += " &";
#endif

    return cmd;
}

std::string EditorsModule::BuildCppOpenCommand(const std::string& filePath, const std::string& projectPath) const
{
    std::string args = mCppEditorArgs.empty() ? "{editor} {project}" : mCppEditorArgs;

    // Get file directory
    std::string fileDir = filePath;
    size_t lastSlash = fileDir.find_last_of("/\\");
    if (lastSlash != std::string::npos)
    {
        fileDir = fileDir.substr(0, lastSlash);
    }

    std::string cmd = args;
    ReplaceAll(cmd, "{editor}", "\"" + mCppEditorPath + "\"");
    ReplaceAll(cmd, "{file}", "\"" + filePath + "\"");
    ReplaceAll(cmd, "{filedir}", "\"" + fileDir + "\"");
    ReplaceAll(cmd, "{project}", "\"" + projectPath + "\"");

#if PLATFORM_WINDOWS
    cmd = "start \"\" " + cmd;
#elif PLATFORM_LINUX
    cmd += " &";
#endif

    return cmd;
}

void EditorsModule::OpenLuaScript(const std::string& filePath)
{
    if (SYS_DoesFileExist(filePath.c_str(), false)) {
        if (mUseInternalEditor)
        {
            GetScriptEditorWindow()->OpenFile(filePath);
            return;
        }
    }

    if (!IsLuaEditorConfigured())
    {
        LogWarning("No Lua editor configured. Set one in Preferences > External > Editors.");
        return;
    }

    std::string cmd = BuildLuaOpenCommand(filePath);
    LogDebug("Opening Lua script: %s", cmd.c_str());
    SYS_Exec(cmd.c_str());
}

void EditorsModule::OpenCppFile(const std::string& filePath, const std::string& vcxprojPath)
{
    if (!IsCppEditorConfigured())
    {
        LogWarning("No C++ editor configured. Set one in Preferences > External > Editors.");
        return;
    }

    std::string cmd = BuildCppOpenCommand(filePath, vcxprojPath);
    LogDebug("Opening C++ file: %s", cmd.c_str());
    SYS_Exec(cmd.c_str());
}

bool EditorsModule::DrawPathInput(const char* label, std::string& path, const char* dialogTitle)
{
    bool changed = false;

    ImGui::PushID(label);

    float buttonWidth = 70.0f;
    float spacing = ImGui::GetStyle().ItemSpacing.x;
    float inputWidth = ImGui::GetContentRegionAvail().x - buttonWidth - spacing;

    ImGui::SetNextItemWidth(inputWidth);
    char pathBuffer[512];
    strncpy(pathBuffer, path.c_str(), sizeof(pathBuffer) - 1);
    pathBuffer[sizeof(pathBuffer) - 1] = '\0';
    if (ImGui::InputText("##path", pathBuffer, sizeof(pathBuffer)))
    {
        path = pathBuffer;
        changed = true;
    }

    ImGui::SameLine();

    if (ImGui::Button("Browse...", ImVec2(buttonWidth, 0)))
    {
        std::vector<std::string> files = SYS_OpenFileDialog();
        if (!files.empty() && !files[0].empty())
        {
            path = files[0];
            changed = true;
        }
    }

    ImGui::PopID();

    return changed;
}

void EditorsModule::ReplaceAll(std::string& str, const std::string& from, const std::string& to)
{
    if (from.empty())
    {
        return;
    }

    size_t startPos = 0;
    while ((startPos = str.find(from, startPos)) != std::string::npos)
    {
        str.replace(startPos, from.length(), to);
        startPos += to.length();
    }
}

#endif
