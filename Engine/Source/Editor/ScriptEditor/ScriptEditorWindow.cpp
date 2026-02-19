#if EDITOR

// Zep single-header build — pulls all Zep source into this TU.
// Suppress warnings in third-party code (compiled with TreatWarningAsError).
#ifdef _MSC_VER
#pragma warning(push, 0)
#endif

#define ZEP_SINGLE_HEADER_BUILD
#define ZEP_FEATURE_CPP_FILE_SYSTEM
#include "zep.h"

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "ScriptEditorWindow.h"
#include "Preferences/JsonSettings.h"
#include "Log.h"
#include "System/System.h"

#include "imgui.h"
#include "document.h"
#include "writer.h"
#include "stringbuffer.h"

#include <fstream>
#include <sstream>
#include <algorithm>

using namespace Zep;

// ---------------------------------------------------------------------------
// Lua syntax keywords/identifiers for .lua registration
// (Zep ships these in syntax_providers.cpp but doesn't map them to .lua)
// ---------------------------------------------------------------------------
static std::unordered_set<std::string> sLuaKeywords = {
    "and", "break", "do", "else", "elseif", "end", "false", "for",
    "function", "goto", "if", "in", "local", "nil", "not", "or",
    "repeat", "return", "then", "true", "until", "while"
};

static std::unordered_set<std::string> sLuaIdentifiers = {
    "assert", "collectgarbage", "dofile", "error", "getmetatable",
    "ipairs", "loadfile", "load", "loadstring", "next", "pairs",
    "pcall", "print", "rawequal", "rawlen", "rawget", "rawset",
    "require", "select", "setmetatable", "tonumber", "tostring",
    "type", "xpcall", "self", "string", "table", "math", "io",
    "os", "coroutine", "debug", "package", "bit32"
};

// ---------------------------------------------------------------------------
// Singleton
// ---------------------------------------------------------------------------
static ScriptEditorWindow sScriptEditorWindow;

ScriptEditorWindow* GetScriptEditorWindow()
{
    return &sScriptEditorWindow;
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
static ZepEditor_ImGui* AsZep(void* ptr)
{
    return static_cast<ZepEditor_ImGui*>(ptr);
}

// ---------------------------------------------------------------------------
// Init / Shutdown
// ---------------------------------------------------------------------------
void ScriptEditorWindow::Init()
{
    LoadRecentFiles();
    mInitialized = true;
}

void ScriptEditorWindow::Shutdown()
{
    if (!mInitialized)
        return;

    SaveRecentFiles();

    if (mEditor)
    {
        delete AsZep(mEditor);
        mEditor = nullptr;
    }

    mInitialized = false;
}

void ScriptEditorWindow::EnsureEditor()
{
    if (mEditor)
        return;

    std::string prefsDir = JsonSettings::GetPreferencesDirectory();
    ZepEditor_ImGui* zep = new ZepEditor_ImGui(
        fs::path(prefsDir),
        NVec2f(1.0f, 1.0f));

    // Note: ZepEditor constructor already calls RegisterSyntaxProviders() internally.
    // Just register Lua syntax for .lua files (not included by default).
    zep->RegisterSyntaxFactory(
        { ".lua" },
        SyntaxProvider{ "lua", tSyntaxFactory([](ZepBuffer* pBuffer) {
            return std::make_shared<ZepSyntax>(*pBuffer, sLuaKeywords, sLuaIdentifiers);
        }) });

    // Don't call SetGlobalMode here — the constructor defaults to Vim mode,
    // and calling it before any windows exist triggers an assertion.
    // We switch to Standard mode after the first file/buffer creates a window.

    mEditor = zep;

    LogDebug("Script Editor created.");
}

// ---------------------------------------------------------------------------
// DrawContent — toolbar + Zep editor
// ---------------------------------------------------------------------------
void ScriptEditorWindow::DrawContent()
{
    if (!mInitialized)
        return;

    // Show placeholder until a file is opened
    if (!mEditor)
    {
        ImGui::TextDisabled("Open a Lua script to start editing.");
        ImGui::TextDisabled("Enable 'Use Internal Editor' in Preferences > External > Editors,");
        ImGui::TextDisabled("then double-click a .lua file in the Scripts panel.");
        return;
    }

    ZepEditor_ImGui* zep = AsZep(mEditor);

    // --- Keyboard shortcuts (only when this panel is focused) ---
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
    {
        ImGuiIO& io = ImGui::GetIO();
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false))
        {
            DoSave();
        }
        else if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O, false))
        {
            DoOpen();
        }
        else if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_N, false))
        {
            DoNew();
        }
    }

    // --- Toolbar ---
    if (ImGui::Button("New"))
    {
        DoNew();
    }
    ImGui::SameLine();

    if (ImGui::Button("Open"))
    {
        DoOpen();
    }
    ImGui::SameLine();

    if (ImGui::Button("Save"))
    {
        DoSave();
    }
    ImGui::SameLine();

    if (ImGui::Button("Save As"))
    {
        DoSaveAs();
    }
    ImGui::SameLine();

    if (ImGui::Button("Close"))
    {
        DoCloseCurrentBuffer();
    }

    // Recent files dropdown
    ImGui::SameLine();
    if (ImGui::BeginCombo("##RecentFiles", "Recent Files", ImGuiComboFlags_NoPreview))
    {
        if (mRecentFiles.empty())
        {
            ImGui::TextDisabled("No recent files");
        }
        else
        {
            for (size_t i = 0; i < mRecentFiles.size(); ++i)
            {
                // Show just the filename for display
                std::string displayName = mRecentFiles[i];
                size_t lastSlash = displayName.find_last_of("/\\");
                if (lastSlash != std::string::npos)
                {
                    displayName = displayName.substr(lastSlash + 1);
                }

                ImGui::PushID((int)i);
                if (ImGui::Selectable(displayName.c_str()))
                {
                    OpenFile(mRecentFiles[i]);
                }
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("%s", mRecentFiles[i].c_str());
                }
                ImGui::PopID();
            }
        }
        ImGui::EndCombo();
    }

    // Show current buffer name
    ZepBuffer* activeBuf = zep->GetActiveBuffer();
    if (activeBuf)
    {
        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();

        bool dirty = activeBuf->HasFileFlags(FileFlags::Dirty);
        std::string name = activeBuf->GetName();
        if (dirty)
        {
            name += " *";
        }
        ImGui::TextDisabled("%s", name.c_str());
    }

    // --- Zep editor region ---
    ImVec2 regionAvail = ImGui::GetContentRegionAvail();
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();

    if (regionAvail.x > 0 && regionAvail.y > 0 && zep->GetActiveTabWindow())
    {
        zep->SetDisplayRegion(
            NVec2f(cursorPos.x, cursorPos.y),
            NVec2f(cursorPos.x + regionAvail.x, cursorPos.y + regionAvail.y));
        zep->Display();

        // Invisible button to capture input for Zep
        ImGui::InvisibleButton("##ZepInput", regionAvail);
        if (ImGui::IsItemActive() || ImGui::IsItemHovered())
        {
            zep->HandleInput();
        }
    }
}

// ---------------------------------------------------------------------------
// File operations
// ---------------------------------------------------------------------------
void ScriptEditorWindow::OpenFile(const std::string& filePath)
{
    if (!mInitialized)
        return;

    EnsureEditor();

    ZepEditor_ImGui* zep = AsZep(mEditor);

    // If this is the first file, use InitWithFileOrDir to set up tab window properly
    bool firstFile = (zep->GetActiveTabWindow() == nullptr);
    if (firstFile)
    {
        zep->InitWithFileOrDir(filePath);
        // Now that windows exist, switch from default Vim to Standard mode
        zep->SetGlobalMode(ZepMode_Standard::StaticName());
    }
    else
    {
        zep->GetFileBuffer(fs::path(filePath));
    }

    AddRecentFile(filePath);

    LogDebug("Script Editor opened: %s", filePath.c_str());
}

bool ScriptEditorWindow::HasUnsavedChanges() const
{
    if (!mInitialized)
        return false;

    ZepEditor_ImGui* zep = AsZep(mEditor);
    for (auto& buf : zep->GetBuffers())
    {
        if (buf->HasFileFlags(FileFlags::Dirty))
        {
            return true;
        }
    }
    return false;
}

void ScriptEditorWindow::DoNew()
{
    if (!mInitialized)
        return;

    EnsureEditor();

    ZepEditor_ImGui* zep = AsZep(mEditor);

    // Generate a unique untitled name
    static int sUntitledCounter = 1;
    std::string name = "Untitled_" + std::to_string(sUntitledCounter++) + ".lua";

    if (!zep->GetActiveTabWindow())
    {
        zep->InitWithText(name, "");
        zep->SetGlobalMode(ZepMode_Standard::StaticName());
    }
    else
    {
        zep->GetEmptyBuffer(name);
    }
}

void ScriptEditorWindow::DoOpen()
{
    if (!mInitialized)
        return;

    std::vector<std::string> files = SYS_OpenFileDialog();
    if (!files.empty() && !files[0].empty())
    {
        OpenFile(files[0]);
    }
}

void ScriptEditorWindow::DoSave()
{
    if (!mInitialized)
        return;

    ZepEditor_ImGui* zep = AsZep(mEditor);
    ZepBuffer* buf = zep->GetActiveBuffer();
    if (!buf)
        return;

    fs::path filePath = buf->GetFilePath();

    // If buffer has no real path (untitled), do Save As instead
    if (filePath.empty() || filePath.filename().string().find("Untitled_") == 0)
    {
        DoSaveAs();
        return;
    }

    zep->SaveBuffer(*buf);
    LogDebug("Script Editor saved: %s", filePath.string().c_str());
}

void ScriptEditorWindow::DoSaveAs()
{
    if (!mInitialized)
        return;

    ZepEditor_ImGui* zep = AsZep(mEditor);
    ZepBuffer* buf = zep->GetActiveBuffer();
    if (!buf)
        return;

    std::string savePath = SYS_SaveFileDialog();
    if (savePath.empty())
        return;

    zep->SaveBufferAs(*buf, fs::path(savePath));
    AddRecentFile(savePath);
    LogDebug("Script Editor saved as: %s", savePath.c_str());
}

void ScriptEditorWindow::DoCloseCurrentBuffer()
{
    if (!mInitialized)
        return;

    ZepEditor_ImGui* zep = AsZep(mEditor);
    ZepBuffer* buf = zep->GetActiveBuffer();
    if (!buf)
        return;

    zep->RemoveBuffer(buf);
}

// ---------------------------------------------------------------------------
// Recent files persistence
// ---------------------------------------------------------------------------
std::string ScriptEditorWindow::GetRecentFilesPath() const
{
    return JsonSettings::GetPreferencesDirectory() + "/script_editor_recent.json";
}

void ScriptEditorWindow::AddRecentFile(const std::string& filePath)
{
    // Remove duplicate if present
    auto it = std::find(mRecentFiles.begin(), mRecentFiles.end(), filePath);
    if (it != mRecentFiles.end())
    {
        mRecentFiles.erase(it);
    }

    // Insert at front
    mRecentFiles.insert(mRecentFiles.begin(), filePath);

    // Trim to max
    if (mRecentFiles.size() > kMaxRecentFiles)
    {
        mRecentFiles.resize(kMaxRecentFiles);
    }
}

void ScriptEditorWindow::LoadRecentFiles()
{
    mRecentFiles.clear();

    std::string path = GetRecentFilesPath();
    rapidjson::Document doc;
    if (!JsonSettings::LoadFromFile(path, doc))
        return;

    if (!doc.HasMember("recentFiles") || !doc["recentFiles"].IsArray())
        return;

    const auto& arr = doc["recentFiles"];
    for (rapidjson::SizeType i = 0; i < arr.Size(); ++i)
    {
        if (arr[i].IsString())
        {
            mRecentFiles.push_back(arr[i].GetString());
        }
    }
}

void ScriptEditorWindow::SaveRecentFiles()
{
    rapidjson::Document doc;
    doc.SetObject();

    rapidjson::Value arr(rapidjson::kArrayType);
    for (const auto& file : mRecentFiles)
    {
        arr.PushBack(
            rapidjson::Value(file.c_str(), doc.GetAllocator()).Move(),
            doc.GetAllocator());
    }

    doc.AddMember("recentFiles", arr, doc.GetAllocator());

    JsonSettings::EnsurePreferencesDirectory();
    JsonSettings::SaveToFile(GetRecentFilesPath(), doc);
}

#endif
