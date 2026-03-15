#if EDITOR

#include "LaunchersModule.h"
#include "../JsonSettings.h"

#include "System/System.h"
#include "Log.h"

#include "document.h"
#include "imgui.h"

#include <cstdlib>

DEFINE_PREFERENCES_MODULE(LaunchersModule, "Launchers", "External")

LaunchersModule::LaunchersModule()
{
}

LaunchersModule::~LaunchersModule()
{
}

void LaunchersModule::Render()
{
    bool changed = false;

    // Dolphin settings (GameCube/Wii)
    ImGui::Text("Dolphin (GameCube/Wii)");
    ImGui::Separator();

    if (DrawPathInput("Path##Dolphin", mDolphinPath, "Select Dolphin Executable"))
    {
        changed = true;
    }

    ImGui::SetNextItemWidth(-1);
    char dolphinArgsBuffer[512];
    strncpy(dolphinArgsBuffer, mDolphinArgs.c_str(), sizeof(dolphinArgsBuffer) - 1);
    dolphinArgsBuffer[sizeof(dolphinArgsBuffer) - 1] = '\0';
    if (ImGui::InputText("Args##Dolphin", dolphinArgsBuffer, sizeof(dolphinArgsBuffer)))
    {
        mDolphinArgs = dolphinArgsBuffer;
        changed = true;
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Command-line arguments. Default: {emulator} --batch -e {output}");
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // Azahar/Citra settings (3DS)
    ImGui::Text("Azahar/Citra (3DS Emulator)");
    ImGui::Separator();

    if (DrawPathInput("Path##Azahar", mAzaharPath, "Select Azahar/Citra Executable"))
    {
        changed = true;
    }

    ImGui::SetNextItemWidth(-1);
    char azaharArgsBuffer[512];
    strncpy(azaharArgsBuffer, mAzaharArgs.c_str(), sizeof(azaharArgsBuffer) - 1);
    azaharArgsBuffer[sizeof(azaharArgsBuffer) - 1] = '\0';
    if (ImGui::InputText("Args##Azahar", azaharArgsBuffer, sizeof(azaharArgsBuffer)))
    {
        mAzaharArgs = azaharArgsBuffer;
        changed = true;
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Command-line arguments. Default: {emulator} {output}");
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // 3dslink info
    ImGui::Text("3dslink (3DS Hardware)");
    ImGui::Separator();
    ImGui::TextDisabled("3dslink is used via 'Build & Run On Device'.");
    ImGui::TextDisabled("Requires devkitPro to be installed.");

    ImGui::Spacing();
    ImGui::Spacing();

    // Placeholder help
    ImGui::TextDisabled("Placeholders: {emulator}, {output}, {outputdir}");

    if (changed)
    {
        SetDirty(true);
    }
}

void LaunchersModule::LoadSettings(const rapidjson::Document& doc)
{
    mDolphinPath = JsonSettings::GetString(doc, "dolphinPath", "");
    mDolphinArgs = JsonSettings::GetString(doc, "dolphinArgs", "{emulator} --batch -e {output}");
    mAzaharPath = JsonSettings::GetString(doc, "azaharPath", "");
    mAzaharArgs = JsonSettings::GetString(doc, "azaharArgs", "{emulator} {output}");
}

void LaunchersModule::SaveSettings(rapidjson::Document& doc)
{
    JsonSettings::SetString(doc, "dolphinPath", mDolphinPath);
    JsonSettings::SetString(doc, "dolphinArgs", mDolphinArgs);
    JsonSettings::SetString(doc, "azaharPath", mAzaharPath);
    JsonSettings::SetString(doc, "azaharArgs", mAzaharArgs);
}

bool LaunchersModule::IsEmulatorConfigured(Platform platform) const
{
    switch (platform)
    {
        case Platform::GameCube:
        case Platform::Wii:
            return !mDolphinPath.empty();

        case Platform::N3DS:
            return !mAzaharPath.empty();

        default:
            return false;
    }
}

bool LaunchersModule::Is3dsLinkConfigured() const
{
    // 3dslink is available if devkitPro is installed
#if PLATFORM_WINDOWS
    // Check if 3dslink.exe exists at the standard devkitPro location
    return SYS_DoesFileExist("C:/devkitPro/tools/bin/3dslink.exe", false);
#else
    // On Linux, just check if 3dslink command exists
    std::string output;
    SYS_Exec("which 3dslink", &output);
    return !output.empty();
#endif
}

std::string LaunchersModule::BuildLaunchCommand(Platform platform, const std::string& outputPath) const
{
    std::string args;
    std::string emulatorPath;

    switch (platform)
    {
        case Platform::GameCube:
        case Platform::Wii:
            emulatorPath = mDolphinPath;
            args = mDolphinArgs.empty() ? "{emulator --batch -e {output}" : mDolphinArgs;
            break;

        case Platform::N3DS:
            emulatorPath = mAzaharPath;
            args = mAzaharArgs.empty() ? "{emulator} {output}" : mAzaharArgs;
            break;

        default:
            return "";
    }

    // Get output directory
    std::string outputDir = outputPath;
    size_t lastSlash = outputDir.find_last_of("/\\");
    if (lastSlash != std::string::npos)
    {
        outputDir = outputDir.substr(0, lastSlash);
    }

    // Replace placeholders
    std::string cmd = args;
    ReplaceAll(cmd, "{emulator}", "\"" + emulatorPath + "\"");
    ReplaceAll(cmd, "{output}", "\"" + outputPath + "\"");
    ReplaceAll(cmd, "{outputdir}", "\"" + outputDir + "\"");

#if PLATFORM_WINDOWS
    // start "" launches the command asynchronously so the editor doesn't freeze.
    // The "" is the window title (required when the first real arg is quoted).
    // No cmd /c wrapper needed — the "start" prefix prevents cmd.exe's quote-stripping.
    cmd = "start \"\" " + cmd;
#elif PLATFORM_LINUX
    // Background the process so the editor doesn't freeze.
    cmd += " &";
#endif

    return cmd;
}

std::string LaunchersModule::Build3dsLinkCommand(const std::string& outputPath) const
{
#if PLATFORM_WINDOWS
    // On Windows, 3dslink.exe is at C:\devkitPro\tools\bin\3dslink.exe
    std::string threeDsLink = "C:\\devkitPro\\tools\\bin\\3dslink.exe";

    if (!SYS_DoesFileExist(threeDsLink.c_str(), false))
    {
        LogError("3dslink.exe not found at %s", threeDsLink.c_str());
        return "";
    }

    // Normalize path to use backslashes on Windows
    std::string normalizedPath = outputPath;
    ReplaceAll(normalizedPath, "/", "\\");

    // Use cmd /c with special quoting for paths with spaces
    std::string cmd = "cmd /c \"\"" + threeDsLink + "\" \"" + normalizedPath + "\"\"";
    return cmd;
#else
    // On Linux, just run 3dslink directly
    return "3dslink \"" + outputPath + "\"";
#endif
}

bool LaunchersModule::DrawPathInput(const char* label, std::string& path, const char* dialogTitle)
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

void LaunchersModule::ReplaceAll(std::string& str, const std::string& from, const std::string& to)
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
