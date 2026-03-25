#if EDITOR

#include "DebugLogWindow.h"
#include "Editor/EditorUIHookManager.h"
#include "Engine.h"
#include "Clock.h"
#include "Log.h"
#include "imgui.h"
#include "EditorIcons.h"

#include <cstring>
#include <cstdio>
#include <algorithm>
#include <cctype>

static DebugLogWindow sDebugLogWindow;

DebugLogWindow* GetDebugLogWindow()
{
    return &sDebugLogWindow;
}

void DebugLogWindow::LogCallback(LogSeverity severity, const char* message)
{
    DebugLogEntry entry;
    entry.mSeverity = severity;
    entry.mMessage = message;

    const Clock* clock = GetAppClock();
    entry.mTimestamp = clock ? clock->GetTime() : 0.0f;

    std::lock_guard<std::mutex> lock(sDebugLogWindow.mBufferMutex);
    sDebugLogWindow.mPendingEntries.push_back(std::move(entry));
}

void DebugLogWindow::DrainPendingEntries()
{
    std::lock_guard<std::mutex> lock(mBufferMutex);
    bool added = !mPendingEntries.empty();

    while (!mPendingEntries.empty())
    {
        mEntries.push_back(std::move(mPendingEntries.front()));
        mPendingEntries.pop_front();
    }

    while (mEntries.size() > kMaxEntries)
    {
        mEntries.pop_front();
    }

    if (added && mSearchActive)
    {
        UpdateSearchMatches();
    }
}

void DebugLogWindow::Clear()
{
    mEntries.clear();
    {
        std::lock_guard<std::mutex> lock(mBufferMutex);
        mPendingEntries.clear();
    }
    mSearchMatches.clear();
    mCurrentMatchIndex = -1;
    mNeedScrollToMatch = false;
    mSelectedEntries.clear();
    mLastClickedRow = -1;
}

void DebugLogWindow::CopyAllToClipboard()
{
    std::string text;
    text.reserve(mEntries.size() * 80);

    for (const auto& entry : mEntries)
    {
        bool show = true;
        if (entry.mSeverity == LogSeverity::Debug && !mShowDebug) show = false;
        if (entry.mSeverity == LogSeverity::Warning && !mShowWarnings) show = false;
        if (entry.mSeverity == LogSeverity::Error && !mShowErrors) show = false;
        if (!show) continue;

        int totalSec = (int)entry.mTimestamp;
        int hours = totalSec / 3600;
        int mins = (totalSec % 3600) / 60;
        int secs = totalSec % 60;

        char line[1280];
        snprintf(line, sizeof(line), "[%02d:%02d:%02d] %s\n", hours, mins, secs, entry.mMessage.c_str());
        text += line;
    }

    if (!text.empty())
    {
        ImGui::SetClipboardText(text.c_str());
    }
}

void DebugLogWindow::CopySelectedToClipboard()
{
    std::string text;
    text.reserve(mSelectedEntries.size() * 80);

    for (int idx : mSelectedEntries)
    {
        if (idx < 0 || idx >= (int)mEntries.size())
            continue;

        const auto& entry = mEntries[idx];
        int totalSec = (int)entry.mTimestamp;
        int hours = totalSec / 3600;
        int mins = (totalSec % 3600) / 60;
        int secs = totalSec % 60;

        char line[1280];
        snprintf(line, sizeof(line), "[%02d:%02d:%02d] %s\n", hours, mins, secs, entry.mMessage.c_str());
        text += line;
    }

    if (!text.empty())
    {
        ImGui::SetClipboardText(text.c_str());
    }
}

static bool CaseInsensitiveFind(const std::string& haystack, const char* needle)
{
    if (needle[0] == '\0')
        return false;

    size_t needleLen = strlen(needle);
    if (needleLen > haystack.size())
        return false;

    for (size_t i = 0; i <= haystack.size() - needleLen; ++i)
    {
        bool match = true;
        for (size_t j = 0; j < needleLen; ++j)
        {
            if (tolower((unsigned char)haystack[i + j]) != tolower((unsigned char)needle[j]))
            {
                match = false;
                break;
            }
        }
        if (match) return true;
    }
    return false;
}

void DebugLogWindow::UpdateSearchMatches()
{
    mSearchMatches.clear();
    mCurrentMatchIndex = -1;

    if (mSearchBuffer[0] == '\0')
        return;

    for (int i = 0; i < (int)mEntries.size(); ++i)
    {
        const auto& entry = mEntries[i];
        bool show = true;
        if (entry.mSeverity == LogSeverity::Debug && !mShowDebug) show = false;
        if (entry.mSeverity == LogSeverity::Warning && !mShowWarnings) show = false;
        if (entry.mSeverity == LogSeverity::Error && !mShowErrors) show = false;
        if (!show) continue;

        if (CaseInsensitiveFind(entry.mMessage, mSearchBuffer))
        {
            mSearchMatches.push_back(i);
        }
    }

    if (!mSearchMatches.empty())
    {
        mCurrentMatchIndex = 0;
        mNeedScrollToMatch = true;
    }
}

void DebugLogWindow::GoToNextMatch()
{
    if (mSearchMatches.empty()) return;
    mCurrentMatchIndex = (mCurrentMatchIndex + 1) % (int)mSearchMatches.size();
    mNeedScrollToMatch = true;
}

void DebugLogWindow::GoToPrevMatch()
{
    if (mSearchMatches.empty()) return;
    mCurrentMatchIndex = (mCurrentMatchIndex - 1 + (int)mSearchMatches.size()) % (int)mSearchMatches.size();
    mNeedScrollToMatch = true;
}

void DebugLogWindow::Draw()
{
    DrainPendingEntries();

    const float dispWidth = ImGui::GetIO().DisplaySize.x;
    const float dispHeight = ImGui::GetIO().DisplaySize.y;

    ImGui::SetNextWindowPos(ImVec2(0.0f, dispHeight - 200.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(dispWidth, 200.0f), ImGuiCond_FirstUseEver);

    ImGui::Begin("Debug Log", nullptr, ImGuiWindowFlags_NoCollapse);
    DrawContent();
    ImGui::End();
}

void DebugLogWindow::DrawContent()
{
    DrainPendingEntries();

    // Toolbar row
    if (ImGui::Button(ICON_ZONDICONS_TRASH "##LogClear"))
        Clear();
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Clear");
    ImGui::SameLine();

    if (ImGui::Button(ICON_MATERIAL_SYMBOLS_FILE_COPY_SHARP "##LogCopy"))
        CopyAllToClipboard();
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Copy All");
    ImGui::SameLine();

    if (ImGui::Button(ICON_IC_BASELINE_SEARCH "##LogFind"))
    {
        mSearchActive = !mSearchActive;
        if (!mSearchActive)
        {
            mSearchBuffer[0] = '\0';
            mSearchMatches.clear();
            mCurrentMatchIndex = -1;
        }
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Find");
    ImGui::SameLine();

    ImGui::Spacing();
    ImGui::SameLine();

    // Severity filter toggles
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 1.0f, 0.5f, 1.0f));
    ImGui::Checkbox(ICON_MDI_BUG "##FilterDebug", &mShowDebug);
    ImGui::PopStyleColor();
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Debug");
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.5f, 1.0f));
    ImGui::Checkbox(ICON_MATERIAL_SYMBOLS_WARNING "##FilterWarning", &mShowWarnings);
    ImGui::PopStyleColor();
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Warning");
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.5f, 0.5f, 1.0f));
    ImGui::Checkbox(ICON_DASHICONS_NO_ALT "##FilterError", &mShowErrors);
    ImGui::PopStyleColor();
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Error");
    ImGui::SameLine();

    ImGui::Spacing();
    ImGui::SameLine();
    ImGui::Checkbox(ICON_BXS_ARROW_TO_BOTTOM "##AutoScroll", &mAutoScroll);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Auto-scroll");

    // Search bar
    if (mSearchActive)
    {
        bool searchChanged = false;

        ImGui::PushItemWidth(200.0f);
        if (ImGui::InputText("##Search", mSearchBuffer, sizeof(mSearchBuffer)))
        {
            searchChanged = true;
        }
        ImGui::PopItemWidth();

        // Focus search box when it first opens
        if (ImGui::IsWindowAppearing())
        {
            ImGui::SetKeyboardFocusHere(-1);
        }

        ImGui::SameLine();

        if (ImGui::Button("<"))
        {
            GoToPrevMatch();
        }
        ImGui::SameLine();
        if (ImGui::Button(">"))
        {
            GoToNextMatch();
        }
        ImGui::SameLine();

        if (!mSearchMatches.empty())
        {
            ImGui::Text("%d/%d", mCurrentMatchIndex + 1, (int)mSearchMatches.size());
        }
        else if (mSearchBuffer[0] != '\0')
        {
            ImGui::Text("0/0");
        }

        ImGui::SameLine();
        if (ImGui::Button("X##CloseSearch"))
        {
            mSearchActive = false;
            mSearchBuffer[0] = '\0';
            mSearchMatches.clear();
            mCurrentMatchIndex = -1;
        }

        // Handle Escape to close search
        if (ImGui::IsKeyPressed(ImGuiKey_Escape))
        {
            mSearchActive = false;
            mSearchBuffer[0] = '\0';
            mSearchMatches.clear();
            mCurrentMatchIndex = -1;
        }

        if (searchChanged)
        {
            UpdateSearchMatches();
        }
    }

    // Build filtered list for clipper
    std::vector<int> filteredIndices;
    filteredIndices.reserve(mEntries.size());
    for (int i = 0; i < (int)mEntries.size(); ++i)
    {
        const auto& entry = mEntries[i];
        if (entry.mSeverity == LogSeverity::Debug && !mShowDebug) continue;
        if (entry.mSeverity == LogSeverity::Warning && !mShowWarnings) continue;
        if (entry.mSeverity == LogSeverity::Error && !mShowErrors) continue;
        filteredIndices.push_back(i);
    }

    // Determine which entry index is the current match target for scrolling
    int currentMatchEntryIdx = -1;
    if (mSearchActive && mCurrentMatchIndex >= 0 && mCurrentMatchIndex < (int)mSearchMatches.size())
    {
        currentMatchEntryIdx = mSearchMatches[mCurrentMatchIndex];
    }

    ImGui::BeginChild("LogScroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    // Alternating row color (theme-aware)
    ImU32 altRowColor;
    {
        const auto& style = ImGui::GetStyle();
        ImVec4 windowBg = style.Colors[ImGuiCol_WindowBg];
        float luminance = windowBg.x * 0.299f + windowBg.y * 0.587f + windowBg.z * 0.114f;
        if (luminance < 0.5f)
            altRowColor = ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 0.02f));
        else
            altRowColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.0f, 0.0f, 0.03f));
    }

    ImGuiListClipper clipper;
    clipper.Begin((int)filteredIndices.size());

    while (clipper.Step())
    {
        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
        {
            int entryIdx = filteredIndices[row];
            const auto& entry = mEntries[entryIdx];

            int totalSec = (int)entry.mTimestamp;
            int hours = totalSec / 3600;
            int mins = (totalSec % 3600) / 60;
            int secs = totalSec % 60;

            const char* severityIcon = ICON_MDI_BUG;
            switch (entry.mSeverity)
            {
                case LogSeverity::Warning: severityIcon = ICON_MATERIAL_SYMBOLS_WARNING; break;
                case LogSeverity::Error:   severityIcon = ICON_DASHICONS_NO_ALT; break;
                default: break;
            }

            // Replace newlines with visual separator for single-line display
            // (full message preserved in entry.mMessage for copy operations)
            std::string displayMsg = entry.mMessage;
            for (size_t i = 0; i < displayMsg.size(); ++i)
            {
                if (displayMsg[i] == '\n' || displayMsg[i] == '\r')
                    displayMsg[i] = ' ';
            }

            char label[1280];
            snprintf(label, sizeof(label), "%s [%02d:%02d:%02d] %s", severityIcon, hours, mins, secs, displayMsg.c_str());

            ImVec4 color;
            switch (entry.mSeverity)
            {
                case LogSeverity::Warning: color = ImVec4(1.0f, 1.0f, 0.5f, 1.0f); break;
                case LogSeverity::Error:   color = ImVec4(1.0f, 0.5f, 0.5f, 1.0f); break;
                default:                   color = ImVec4(0.8f, 0.8f, 0.8f, 1.0f); break;
            }

            // Highlight current search match
            bool isCurrentMatch = (entryIdx == currentMatchEntryIdx);
            bool isSearchMatch = false;

            if (mSearchActive && mSearchBuffer[0] != '\0')
            {
                for (int mi : mSearchMatches)
                {
                    if (mi == entryIdx)
                    {
                        isSearchMatch = true;
                        break;
                    }
                }
            }

            bool isSelected = mSelectedEntries.count(entryIdx) > 0;

            if (isCurrentMatch)
            {
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.4f, 0.4f, 0.1f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.5f, 0.5f, 0.15f, 1.0f));
            }
            else if (isSearchMatch)
            {
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.2f, 0.05f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.3f, 0.1f, 1.0f));
            }
            else if (isSelected)
            {
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.15f, 0.35f, 0.6f, 0.6f));
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.2f, 0.4f, 0.65f, 0.7f));
            }

            ImGui::PushStyleColor(ImGuiCol_Text, color);

            ImGui::PushID(entryIdx);
            bool highlight = isSelected || isCurrentMatch || isSearchMatch;
            if (ImGui::Selectable(label, highlight, ImGuiSelectableFlags_AllowDoubleClick))
            {
                const ImGuiIO& io = ImGui::GetIO();
                if (io.KeyCtrl)
                {
                    // Ctrl+Click: toggle this entry
                    if (isSelected)
                        mSelectedEntries.erase(entryIdx);
                    else
                        mSelectedEntries.insert(entryIdx);
                }
                else if (io.KeyShift && mLastClickedRow >= 0)
                {
                    // Shift+Click: select range
                    int rangeStart = std::min(mLastClickedRow, row);
                    int rangeEnd = std::max(mLastClickedRow, row);
                    for (int r = rangeStart; r <= rangeEnd; ++r)
                    {
                        if (r >= 0 && r < (int)filteredIndices.size())
                            mSelectedEntries.insert(filteredIndices[r]);
                    }
                }
                else
                {
                    // Plain click: select only this entry
                    mSelectedEntries.clear();
                    mSelectedEntries.insert(entryIdx);
                }
                mLastClickedRow = row;
            }

            // Alternating row background
            if (row % 2 == 1)
            {
                ImVec2 rMin = ImGui::GetItemRectMin();
                ImVec2 rMax = ImGui::GetItemRectMax();
                rMin.x = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMin().x;
                rMax.x = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
                ImGui::GetWindowDrawList()->AddRectFilled(rMin, rMax, altRowColor);
            }

            // Right-click context menu
            if (ImGui::BeginPopupContextItem("LogEntryCtx"))
            {
                if (ImGui::Selectable("Copy"))
                {
                    if (mSelectedEntries.size() > 1)
                    {
                        CopySelectedToClipboard();
                    }
                    else
                    {
                        // Copy with original message (preserving newlines)
                        char copyText[1280];
                        snprintf(copyText, sizeof(copyText), "[%02d:%02d:%02d] %s", hours, mins, secs, entry.mMessage.c_str());
                        ImGui::SetClipboardText(copyText);
                    }
                }
                if (ImGui::Selectable("Copy All"))
                {
                    CopyAllToClipboard();
                }

                // Addon debug log context items (Batch 8)
                EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
                if (hookMgr != nullptr)
                {
                    hookMgr->DrawDebugLogContextItems();
                }

                ImGui::EndPopup();
            }

            ImGui::PopID();
            ImGui::PopStyleColor(); // Text color

            if (isCurrentMatch || isSearchMatch || isSelected)
            {
                ImGui::PopStyleColor(2); // Header colors
            }
        }
    }

    clipper.End();

    // Scroll to current match
    if (mNeedScrollToMatch && currentMatchEntryIdx >= 0)
    {
        // Find the row in filteredIndices
        for (int row = 0; row < (int)filteredIndices.size(); ++row)
        {
            if (filteredIndices[row] == currentMatchEntryIdx)
            {
                float itemHeight = ImGui::GetTextLineHeightWithSpacing();
                ImGui::SetScrollY(row * itemHeight);
                break;
            }
        }
        mNeedScrollToMatch = false;
    }
    else if (mAutoScroll && !mSearchActive && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 10.0f)
    {
        ImGui::SetScrollHereY(1.0f);
    }

    // Keyboard shortcuts (when log child is hovered and no popup open)
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && !ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId))
    {
        const ImGuiIO& io = ImGui::GetIO();
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_C))
        {
            if (!mSelectedEntries.empty())
                CopySelectedToClipboard();
        }
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_A))
        {
            mSelectedEntries.clear();
            for (int idx : filteredIndices)
                mSelectedEntries.insert(idx);
        }
    }

    ImGui::EndChild();
}

#endif
