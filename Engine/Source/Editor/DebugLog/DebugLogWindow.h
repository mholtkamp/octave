#pragma once

#if EDITOR

#include "System/SystemTypes.h"
#include <string>
#include <deque>
#include <vector>
#include <set>
#include <mutex>

struct DebugLogEntry
{
    LogSeverity mSeverity;
    std::string mMessage;
    float mTimestamp; // seconds since engine start
};

class DebugLogWindow
{
public:
    void Draw();
    void DrawContent();
    void Clear();
    static void LogCallback(LogSeverity severity, const char* message);

    bool mShowDebug = true;
    bool mShowWarnings = true;
    bool mShowErrors = true;
    bool mAutoScroll = true;

private:
    std::deque<DebugLogEntry> mEntries;
    std::deque<DebugLogEntry> mPendingEntries;
    std::mutex mBufferMutex;
    static const size_t kMaxEntries = 2048;

    char mSearchBuffer[256] = {};
    bool mSearchActive = false;
    std::vector<int> mSearchMatches;
    int mCurrentMatchIndex = -1;
    bool mNeedScrollToMatch = false;

    std::set<int> mSelectedEntries;
    int mLastClickedRow = -1;

    void DrainPendingEntries();
    void CopyAllToClipboard();
    void CopySelectedToClipboard();
    void UpdateSearchMatches();
    void GoToNextMatch();
    void GoToPrevMatch();
};

DebugLogWindow* GetDebugLogWindow();

#endif
