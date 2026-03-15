#if EDITOR

#include "Timeline/TimelinePanel.h"
#include "EditorState.h"
#include "EditorConstants.h"
#include "ActionManager.h"
#include "Assets/Timeline.h"
#include "Timeline/TimelineTrack.h"
#include "Timeline/TimelineClip.h"
#include "Timeline/TimelineInstance.h"
#include "Timeline/Tracks/TransformTrack.h"
#include "Timeline/Tracks/AudioTrack.h"
#include "Timeline/Tracks/AnimationTrack.h"
#include "Timeline/Tracks/AnimationClip.h"
#include "Timeline/Tracks/ScriptValueTrack.h"
#include "Timeline/Tracks/ActivateTrack.h"
#include "Timeline/Tracks/FunctionCallTrack.h"
#include "Timeline/Tracks/FunctionCallClip.h"
#include "Script.h"
#include "Nodes/TimelinePlayer.h"
#include "Nodes/Node.h"
#include "Nodes/3D/SkeletalMesh3d.h"
#include "Assets/SkeletalMesh.h"
#include "World.h"
#include "Engine.h"
#include "Log.h"
#include "InputDevices.h"

#include "imgui.h"

static const float kTrackListWidth = 220.0f;
static const float kTrackHeight = 24.0f;
static const float kRulerHeight = 24.0f;
static const float kTransportHeight = 30.0f;
static const float kMinClipWidth = 4.0f;

static float TimeToPixel(float time, float zoom, float scrollX)
{
    return (time * zoom) - scrollX;
}

static float PixelToTime(float pixel, float zoom, float scrollX)
{
    return (pixel + scrollX) / zoom;
}

static float SnapTime(float time, float snapInterval)
{
    if (snapInterval > 0.0f)
    {
        return glm::round(time / snapInterval) * snapInterval;
    }
    return time;
}

static void DrawTimeRuler(ImDrawList* drawList, ImVec2 pos, ImVec2 size, float zoom, float scrollX, float duration)
{
    ImU32 bgColor = IM_COL32(40, 40, 40, 255);
    ImU32 lineColor = IM_COL32(100, 100, 100, 255);
    ImU32 textColor = IM_COL32(180, 180, 180, 255);
    ImU32 majorLineColor = IM_COL32(140, 140, 140, 255);

    drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), bgColor);

    // Determine tick interval based on zoom
    float tickInterval = 1.0f;
    if (zoom < 30.0f) tickInterval = 5.0f;
    else if (zoom < 60.0f) tickInterval = 2.0f;
    else if (zoom > 200.0f) tickInterval = 0.5f;
    else if (zoom > 500.0f) tickInterval = 0.1f;

    float startTime = PixelToTime(0.0f, zoom, scrollX);
    float endTime = PixelToTime(size.x, zoom, scrollX);

    startTime = glm::max(0.0f, glm::floor(startTime / tickInterval) * tickInterval);

    for (float t = startTime; t <= endTime && t <= duration; t += tickInterval)
    {
        float x = pos.x + TimeToPixel(t, zoom, scrollX);
        bool isMajor = (fmodf(t, tickInterval * 5.0f) < 0.001f) || (t < 0.001f);

        float lineLen = isMajor ? size.y : size.y * 0.5f;
        drawList->AddLine(ImVec2(x, pos.y + size.y - lineLen), ImVec2(x, pos.y + size.y), isMajor ? majorLineColor : lineColor);

        if (isMajor || tickInterval >= 1.0f)
        {
            char buf[32];
            int mins = (int)(t / 60.0f);
            float secs = t - (mins * 60.0f);
            if (mins > 0)
                snprintf(buf, sizeof(buf), "%d:%05.2f", mins, secs);
            else
                snprintf(buf, sizeof(buf), "%.2fs", secs);
            drawList->AddText(ImVec2(x + 2, pos.y + 2), textColor, buf);
        }
    }
}

static void DrawPlayhead(ImDrawList* drawList, ImVec2 rulerPos, float totalHeight, float time, float zoom, float scrollX)
{
    float x = rulerPos.x + TimeToPixel(time, zoom, scrollX);
    ImU32 playheadColor = IM_COL32(255, 80, 80, 255);
    drawList->AddLine(ImVec2(x, rulerPos.y), ImVec2(x, rulerPos.y + totalHeight), playheadColor, 2.0f);

    // Triangle marker at top
    drawList->AddTriangleFilled(
        ImVec2(x - 5, rulerPos.y),
        ImVec2(x + 5, rulerPos.y),
        ImVec2(x, rulerPos.y + 8),
        playheadColor);
}

void OpenTimelineForEditing(Timeline* timeline)
{
    EditorState* state = GetEditorState();
    state->mEditedTimelineRef = timeline;
    state->mShowTimelinePanel = true;
    state->mTimelinePlayheadTime = 0.0f;
    state->mTimelineSelectedTrack = -1;
    state->mTimelineSelectedClip = -1;
    state->mTimelineSelectedKeyframe = -1;

    if (state->mTimelinePreviewInstance != nullptr)
    {
        delete state->mTimelinePreviewInstance;
        state->mTimelinePreviewInstance = nullptr;
    }
}

void CloseTimelinePanel()
{
    EditorState* state = GetEditorState();

    if (state->mTimelinePreviewing && state->mTimelinePreviewInstance != nullptr)
    {
        Timeline* timeline = state->mEditedTimelineRef.Get<Timeline>();
        if (timeline != nullptr)
        {
            state->mTimelinePreviewInstance->RestorePrePlayState(timeline->GetTracks());
        }
    }

    if (state->mTimelinePreviewInstance != nullptr)
    {
        delete state->mTimelinePreviewInstance;
        state->mTimelinePreviewInstance = nullptr;
    }

    state->mEditedTimelineRef = nullptr;
    state->mTimelinePreviewing = false;
    state->mShowTimelinePanel = false;
}

void DrawTimelinePanel()
{
    EditorState* state = GetEditorState();
    Timeline* timeline = state->mEditedTimelineRef.Get<Timeline>();

    bool open = state->mShowTimelinePanel;
    ImGui::SetNextWindowSize(ImVec2(800, 300), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Timeline", &open))
    {
        ImGui::End();
        if (!open)
        {
            CloseTimelinePanel();
        }
        return;
    }

    if (!open)
    {
        ImGui::End();
        CloseTimelinePanel();
        return;
    }

    if (timeline == nullptr)
    {
        ImGui::TextDisabled("No timeline asset selected. Double-click a Timeline asset to edit it.");
        ImGui::End();
        return;
    }

    float& zoom = state->mTimelineZoom;
    float& scrollX = state->mTimelineScrollX;
    float& playheadTime = state->mTimelinePlayheadTime;
    float snapInterval = state->mTimelineSnapInterval;
    float duration = timeline->GetDuration();

    ImVec2 windowPos = ImGui::GetCursorScreenPos();
    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    float contentHeight = windowSize.y - kTransportHeight;
    float trackAreaHeight = contentHeight - kRulerHeight;

    bool resetTracks = false;

    // ========== Transport Bar ==========
    {
        ImGui::BeginGroup();

        if (ImGui::Button("|<"))
        {
            playheadTime = 0.0f;
            resetTracks = true;
        }
        ImGui::SameLine();
        if (ImGui::Button(">"))
        {
            state->mTimelinePreviewing = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("||"))
        {
            state->mTimelinePreviewing = false;
            resetTracks = true;
        }
        ImGui::SameLine();
        if (ImGui::Button(">|"))
        {
            playheadTime = duration;
        }

        ImGui::SameLine();
        ImGui::SetNextItemWidth(80);
        ImGui::DragFloat("##time", &playheadTime, 0.01f, 0.0f, duration, "%.2fs");
        playheadTime = glm::clamp(playheadTime, 0.0f, duration);

        ImGui::SameLine();
        ImGui::Text("/ %.2fs", duration);

        ImGui::SameLine();
        ImGui::SetNextItemWidth(60);
        float dur = timeline->GetDuration();
        if (ImGui::DragFloat("##duration", &dur, 0.1f, 0.1f, 600.0f, "Dur:%.1f"))
        {
            timeline->SetDuration(dur);
        }

        ImGui::SameLine();
        bool loop = timeline->IsLooping();
        if (ImGui::Checkbox("Loop", &loop))
        {
            timeline->SetLooping(loop);
        }

        ImGui::SameLine();
        ImGui::SetNextItemWidth(80);
        static const char* snapValues[] = { "0.01", "0.05", "0.1", "0.25", "0.5", "1.0" };
        static int snapIdx = 2;
        if (ImGui::Combo("Snap", &snapIdx, snapValues, 6))
        {
            state->mTimelineSnapInterval = (float)atof(snapValues[snapIdx]);
        }

        ImGui::SameLine();
        ImGui::SetNextItemWidth(80);
        ImGui::DragFloat("Zoom", &zoom, 1.0f, 10.0f, 1000.0f, "%.0f");

        ImGui::EndGroup();
    }

    ImVec2 contentPos = ImGui::GetCursorScreenPos();

    // ========== Track List (left side) ==========
    ImVec2 trackListPos = contentPos;
    ImVec2 trackListSize = ImVec2(kTrackListWidth, trackAreaHeight);

    drawList->AddRectFilled(trackListPos, ImVec2(trackListPos.x + trackListSize.x, trackListPos.y + kRulerHeight), IM_COL32(35, 35, 35, 255));

    const std::vector<TimelineTrack*>& tracks = timeline->GetTracks();
    for (uint32_t i = 0; i < tracks.size(); ++i)
    {
        float y = trackListPos.y + kRulerHeight + i * kTrackHeight;
        bool selected = ((int32_t)i == state->mTimelineSelectedTrack);

        ImU32 trackBg = selected ? IM_COL32(60, 60, 80, 255) : ((i % 2 == 0) ? IM_COL32(45, 45, 45, 255) : IM_COL32(50, 50, 50, 255));
        drawList->AddRectFilled(ImVec2(trackListPos.x, y), ImVec2(trackListPos.x + kTrackListWidth, y + kTrackHeight), trackBg);

        // Mute/Lock buttons
        char muteBuf[32];
        snprintf(muteBuf, sizeof(muteBuf), "%s##m%d", tracks[i]->IsMuted() ? "M" : "m", i);
        ImGui::SetCursorScreenPos(ImVec2(trackListPos.x + 2, y + 2));
        if (ImGui::SmallButton(muteBuf))
        {
            tracks[i]->SetMuted(!tracks[i]->IsMuted());
        }

        char lockBuf[32];
        snprintf(lockBuf, sizeof(lockBuf), "%s##l%d", tracks[i]->IsLocked() ? "L" : "l", i);
        ImGui::SameLine();
        if (ImGui::SmallButton(lockBuf))
        {
            tracks[i]->SetLocked(!tracks[i]->IsLocked());
        }

        ImGui::SameLine();

        // Target node button
        char targetBuf[64];
        const std::string& nodeName = tracks[i]->GetTargetNodeName();
        snprintf(targetBuf, sizeof(targetBuf), "%s##t%d", nodeName.empty() ? "[Set Target]" : nodeName.c_str(), i);
        if (ImGui::SmallButton(targetBuf))
        {
            state->mTimelineSelectedTrack = (int32_t)i;
            state->mTimelineSelectedClip = -1;
            state->mTimelineSelectedKeyframe = -1;
            state->InspectObject(tracks[i], true, false);
            ImGui::OpenPopup("NodePickerPopup");
        }

        // Drop target for node drag-and-drop from scene hierarchy
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(DRAGDROP_NODE))
            {
                Node* droppedNode = *(Node**)payload->Data;
                if (droppedNode != nullptr)
                {
                    uint64_t oldUuid = tracks[i]->GetTargetNodeUuid();
                    std::string oldName = tracks[i]->GetTargetNodeName();
                    uint64_t newUuid = droppedNode->GetPersistentUuid();
                    const std::string& newName = droppedNode->GetName();
                    ActionManager::Get()->EXE_TimelineBindTrack(timeline, (int32_t)i, oldUuid, newUuid, oldName, newName);
                }
            }
            ImGui::EndDragDropTarget();
        }

        ImGui::SameLine();
        ImVec2 textPos = ImGui::GetCursorScreenPos();

        // Track type label
        char trackLabel[32];
        snprintf(trackLabel, sizeof(trackLabel), "[%s]", tracks[i]->GetTrackTypeName());
        drawList->AddText(ImVec2(textPos.x, y + 4), IM_COL32(150, 150, 150, 255), trackLabel);

        // Click to select track
        ImGui::SetCursorScreenPos(ImVec2(trackListPos.x, y));
        char selBuf[32];
        snprintf(selBuf, sizeof(selBuf), "##track%d", i);
        if (ImGui::InvisibleButton(selBuf, ImVec2(kTrackListWidth, kTrackHeight)))
        {
            state->mTimelineSelectedTrack = (int32_t)i;
            state->mTimelineSelectedClip = -1;
            state->mTimelineSelectedKeyframe = -1;
            state->InspectObject(tracks[i], true, false);
        }
        if (ImGui::IsItemClicked(1))
        {
            state->mTimelineSelectedTrack = (int32_t)i;
            state->mTimelineSelectedClip = -1;
            state->mTimelineSelectedKeyframe = -1;
            ImGui::OpenPopup("TrackContextMenu");
        }
    }

    // Node Picker Popup
    if (ImGui::BeginPopup("NodePickerPopup"))
    {
        static char nodeFilter[128] = "";
        ImGui::InputText("Filter", nodeFilter, sizeof(nodeFilter));

        World* pickerWorld = GetWorld(0);
        if (pickerWorld != nullptr && state->mTimelineSelectedTrack >= 0)
        {
            std::vector<Node*> allNodes = pickerWorld->GatherNodes();

            if (ImGui::Selectable("(None)"))
            {
                TimelineTrack* trk = timeline->GetTrack(state->mTimelineSelectedTrack);
                if (trk != nullptr)
                {
                    uint64_t oldUuid = trk->GetTargetNodeUuid();
                    std::string oldName = trk->GetTargetNodeName();
                    ActionManager::Get()->EXE_TimelineBindTrack(timeline, state->mTimelineSelectedTrack, oldUuid, 0, oldName, "");
                }
                nodeFilter[0] = '\0';
                ImGui::CloseCurrentPopup();
            }

            for (uint32_t n = 0; n < allNodes.size(); ++n)
            {
                const std::string& name = allNodes[n]->GetName();
                if (nodeFilter[0] != '\0')
                {
                    // Simple case-insensitive substring filter
                    bool match = false;
                    std::string lowerName = name;
                    std::string lowerFilter = nodeFilter;
                    for (auto& ch : lowerName) ch = (char)tolower(ch);
                    for (auto& ch : lowerFilter) ch = (char)tolower(ch);
                    match = lowerName.find(lowerFilter) != std::string::npos;
                    if (!match) continue;
                }

                char nodeBuf[256];
                snprintf(nodeBuf, sizeof(nodeBuf), "%s##np%d", name.c_str(), n);
                if (ImGui::Selectable(nodeBuf))
                {
                    TimelineTrack* trk = timeline->GetTrack(state->mTimelineSelectedTrack);
                    if (trk != nullptr)
                    {
                        uint64_t oldUuid = trk->GetTargetNodeUuid();
                        std::string oldName = trk->GetTargetNodeName();
                        uint64_t newUuid = allNodes[n]->GetPersistentUuid();
                        ActionManager::Get()->EXE_TimelineBindTrack(timeline, state->mTimelineSelectedTrack, oldUuid, newUuid, oldName, name);
                    }
                    nodeFilter[0] = '\0';
                    ImGui::CloseCurrentPopup();
                }
            }
        }
        ImGui::EndPopup();
    }

    // Add Track button
    float addTrackY = trackListPos.y + kRulerHeight + tracks.size() * kTrackHeight + 4;
    ImGui::SetCursorScreenPos(ImVec2(trackListPos.x + 4, addTrackY));
    if (ImGui::Button("+ Add Track"))
    {
        ImGui::OpenPopup("AddTrackPopup");
    }

    if (ImGui::BeginPopup("AddTrackPopup"))
    {
        if (ImGui::Selectable("Transform"))
            timeline->AddTrack(TransformTrack::GetStaticType());
        if (ImGui::Selectable("Audio"))
            timeline->AddTrack(AudioTrack::GetStaticType());
        if (ImGui::Selectable("Animation"))
            timeline->AddTrack(AnimationTrack::GetStaticType());
        if (ImGui::Selectable("Script Value"))
            timeline->AddTrack(ScriptValueTrack::GetStaticType());
        if (ImGui::Selectable("Activate"))
            timeline->AddTrack(ActivateTrack::GetStaticType());
        if (ImGui::Selectable("Function Call"))
            timeline->AddTrack(FunctionCallTrack::GetStaticType());
        ImGui::EndPopup();
    }

    // ========== Time Ruler + Clip Area (right side) ==========
    ImVec2 rulerPos = ImVec2(contentPos.x + kTrackListWidth, contentPos.y);
    float timeAreaWidth = windowSize.x - kTrackListWidth;
    ImVec2 rulerSize = ImVec2(timeAreaWidth, kRulerHeight);

    DrawTimeRuler(drawList, rulerPos, rulerSize, zoom, scrollX, duration);

    // Click on ruler to set playhead
    ImGui::SetCursorScreenPos(rulerPos);
    ImGui::InvisibleButton("##ruler", rulerSize);
    if (ImGui::IsItemActive())
    {
        ImVec2 mousePos = ImGui::GetMousePos();
        float clickTime = PixelToTime(mousePos.x - rulerPos.x, zoom, scrollX);
        clickTime = SnapTime(clickTime, snapInterval);
        playheadTime = glm::clamp(clickTime, 0.0f, duration);
    }

    // ========== Draw Clips ==========
    ImVec2 clipAreaPos = ImVec2(rulerPos.x, rulerPos.y + kRulerHeight);
    drawList->AddRectFilled(clipAreaPos, ImVec2(clipAreaPos.x + timeAreaWidth, clipAreaPos.y + trackAreaHeight - kRulerHeight), IM_COL32(30, 30, 30, 255));

    bool clipContextOpened = false;

    for (uint32_t t = 0; t < tracks.size(); ++t)
    {
        float trackY = clipAreaPos.y + t * kTrackHeight;
        glm::vec4 trackColor = tracks[t]->GetTrackColor();
        ImU32 clipColor = IM_COL32(
            (int)(trackColor.r * 180),
            (int)(trackColor.g * 180),
            (int)(trackColor.b * 180),
            200);
        ImU32 clipSelectedColor = IM_COL32(
            (int)(trackColor.r * 255),
            (int)(trackColor.g * 255),
            (int)(trackColor.b * 255),
            255);

        // Track separator line
        drawList->AddLine(ImVec2(clipAreaPos.x, trackY + kTrackHeight), ImVec2(clipAreaPos.x + timeAreaWidth, trackY + kTrackHeight), IM_COL32(60, 60, 60, 255));

        for (uint32_t c = 0; c < tracks[t]->GetNumClips(); ++c)
        {
            TimelineClip* clip = tracks[t]->GetClip(c);
            float clipStartX = clipAreaPos.x + TimeToPixel(clip->GetStartTime(), zoom, scrollX);
            float clipEndX = clipAreaPos.x + TimeToPixel(clip->GetEndTime(), zoom, scrollX);
            float clipWidth = glm::max(clipEndX - clipStartX, kMinClipWidth);

            bool isSelected = ((int32_t)t == state->mTimelineSelectedTrack && (int32_t)c == state->mTimelineSelectedClip);

            ImVec2 clipMin = ImVec2(clipStartX, trackY + 1);
            ImVec2 clipMax = ImVec2(clipStartX + clipWidth, trackY + kTrackHeight - 1);

            drawList->AddRectFilled(clipMin, clipMax, isSelected ? clipSelectedColor : clipColor, 3.0f);
            drawList->AddRect(clipMin, clipMax, IM_COL32(200, 200, 200, isSelected ? 255 : 100), 3.0f);

            // Click on clip to select
            ImGui::SetCursorScreenPos(clipMin);
            char clipBuf[32];
            snprintf(clipBuf, sizeof(clipBuf), "##clip%d_%d", t, c);
            ImGui::SetNextItemAllowOverlap();
            if (ImGui::InvisibleButton(clipBuf, ImVec2(clipWidth, kTrackHeight - 2)))
            {
                state->mTimelineSelectedTrack = (int32_t)t;
                state->mTimelineSelectedClip = (int32_t)c;
                state->mTimelineSelectedKeyframe = -1;
                state->InspectObject(clip, true, false);
            }

            // Drag to move clip
            if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0) && !tracks[t]->IsLocked())
            {
                float dragDelta = ImGui::GetMouseDragDelta(0).x;
                float timeDelta = dragDelta / zoom;
                float newStart = clip->GetStartTime() + timeDelta;
                newStart = SnapTime(newStart, snapInterval);
                newStart = glm::max(0.0f, newStart);
                clip->SetStartTime(newStart);
                ImGui::ResetMouseDragDelta(0);
            }

            // Right-click context menu
            if (ImGui::IsItemClicked(1))
            {
                state->mTimelineSelectedTrack = (int32_t)t;
                state->mTimelineSelectedClip = (int32_t)c;
                state->mTimelineSelectedKeyframe = -1;
                state->InspectObject(clip, true, false);
                ImGui::OpenPopup("ClipContextMenu");
                clipContextOpened = true;
            }

            // ========== Draw Keyframe Diamonds ==========
            if (clip->SupportsKeyframes())
            {
                uint32_t numKf = clip->GetNumKeyframes();
                for (uint32_t k = 0; k < numKf; ++k)
                {
                    float kfAbsTime = clip->GetStartTime() + clip->GetKeyframeTime(k);
                    float kfX = clipAreaPos.x + TimeToPixel(kfAbsTime, zoom, scrollX);
                    float kfCenterY = trackY + kTrackHeight / 2.0f;
                    float halfSize = 4.0f;

                    bool kfSelected = (isSelected && (int32_t)k == state->mTimelineSelectedKeyframe);

                    ImVec2 diamondPoints[4] = {
                        ImVec2(kfX, kfCenterY - halfSize),  // top
                        ImVec2(kfX + halfSize, kfCenterY),  // right
                        ImVec2(kfX, kfCenterY + halfSize),  // bottom
                        ImVec2(kfX - halfSize, kfCenterY)   // left
                    };

                    ImU32 kfFillColor = kfSelected ? IM_COL32(255, 255, 255, 255) : IM_COL32(200, 200, 200, 255);
                    ImU32 kfOutlineColor = kfSelected ? IM_COL32(255, 255, 100, 255) : IM_COL32(150, 150, 150, 255);

                    drawList->AddConvexPolyFilled(diamondPoints, 4, kfFillColor);
                    drawList->AddPolyline(diamondPoints, 4, kfOutlineColor, ImDrawFlags_Closed, 1.0f);

                    // Hit-test invisible button for keyframe
                    ImGui::SetCursorScreenPos(ImVec2(kfX - halfSize, kfCenterY - halfSize));
                    char kfBuf[48];
                    snprintf(kfBuf, sizeof(kfBuf), "##kf%d_%d_%d", t, c, k);
                    ImGui::SetNextItemAllowOverlap();
                    if (ImGui::InvisibleButton(kfBuf, ImVec2(halfSize * 2.0f, halfSize * 2.0f)))
                    {
                        state->mTimelineSelectedTrack = (int32_t)t;
                        state->mTimelineSelectedClip = (int32_t)c;
                        state->mTimelineSelectedKeyframe = (int32_t)k;
                        state->InspectObject(clip, true, false);
                    }

                    // Drag to move keyframe
                    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0) && !tracks[t]->IsLocked())
                    {
                        float dragDelta = ImGui::GetMouseDragDelta(0).x;
                        float timeDelta = dragDelta / zoom;
                        float newTime = clip->GetKeyframeTime(k) + timeDelta;
                        newTime = SnapTime(newTime, snapInterval);
                        newTime = glm::max(0.0f, newTime);
                        clip->SetKeyframeTime(k, newTime);
                        ImGui::ResetMouseDragDelta(0);
                    }
                }
            }
        }
    }

    // Clip context menu
    if (ImGui::BeginPopup("ClipContextMenu"))
    {
        if (state->mTimelineSelectedTrack >= 0 && state->mTimelineSelectedClip >= 0)
        {
            TimelineTrack* track = timeline->GetTrack(state->mTimelineSelectedTrack);
            if (track != nullptr)
            {
                if (ImGui::Selectable("Delete Clip"))
                {
                    state->InspectObject(nullptr, true, false);
                    track->RemoveClip(state->mTimelineSelectedClip);
                    state->mTimelineSelectedClip = -1;
                }
                else if (ImGui::Selectable("Duplicate Clip"))
                {
                    TimelineClip* srcClip = track->GetClip(state->mTimelineSelectedClip);
                    if (srcClip != nullptr)
                    {
                        TimelineClip* newClip = TimelineClip::CreateInstance(srcClip->GetType());
                        if (newClip != nullptr)
                        {
                            // Copy basic properties
                            newClip->SetStartTime(srcClip->GetEndTime());
                            newClip->SetDuration(srcClip->GetDuration());
                            newClip->SetSpeed(srcClip->GetSpeed());
                            track->AddClip(newClip);
                        }
                    }
                }
                else if (state->mTimelineSelectedClip >= 0)
                {
                    TimelineClip* selClip = track->GetClip(state->mTimelineSelectedClip);
                    if (selClip != nullptr && selClip->SupportsKeyframes())
                    {
                        ImGui::Separator();

                        // Resolve target node for capturing transform
                        Node* resolvedNode = nullptr;
                        World* kfWorld = GetWorld(0);
                        if (kfWorld != nullptr && state->mTimelinePreviewInstance != nullptr)
                        {
                            TrackInstanceData& data = state->mTimelinePreviewInstance->GetTrackData(state->mTimelineSelectedTrack);
                            resolvedNode = data.mResolvedNode;
                        }

                        float clipLocalTime = playheadTime - selClip->GetStartTime();
                        if (ImGui::Selectable("Add Keyframe at Playhead"))
                        {
                            selClip->AddKeyframeAtTime(clipLocalTime, resolvedNode);
                        }
                        if (selClip->GetNumKeyframes() > 0 && ImGui::Selectable("Remove Last Keyframe"))
                        {
                            selClip->RemoveKeyframe(selClip->GetNumKeyframes() - 1);
                        }
                    }

                    // "Set Function" per-keyframe submenu for FunctionCallClip
                    if (selClip != nullptr && selClip->GetType() == FunctionCallClip::GetStaticType()
                        && state->mTimelineSelectedKeyframe >= 0)
                    {
                        FunctionCallClip* fcClip = static_cast<FunctionCallClip*>(selClip);
                        uint32_t kfIdx = (uint32_t)state->mTimelineSelectedKeyframe;

                        ImGui::Separator();

                        Node* fcResolvedNode = nullptr;
                        if (state->mTimelinePreviewInstance != nullptr)
                        {
                            TrackInstanceData& data = state->mTimelinePreviewInstance->GetTrackData(state->mTimelineSelectedTrack);
                            fcResolvedNode = data.mResolvedNode;
                        }

                        if (fcResolvedNode != nullptr && fcResolvedNode->GetScript() != nullptr
                            && kfIdx < fcClip->GetNumKeyframes())
                        {
                            Script* script = fcResolvedNode->GetScript();
                            if (ImGui::BeginMenu("Set Function"))
                            {
                                std::vector<std::string> funcNames;
                                script->GatherFunctionNames(funcNames);

                                if (funcNames.empty())
                                {
                                    ImGui::TextDisabled("(no functions found)");
                                }
                                else
                                {
                                    const std::string& currentFunc = fcClip->GetKeyframeFunctionName(kfIdx);
                                    for (uint32_t fn = 0; fn < funcNames.size(); ++fn)
                                    {
                                        bool isCurrentFunc = (currentFunc == funcNames[fn]);
                                        if (ImGui::Selectable(funcNames[fn].c_str(), isCurrentFunc))
                                        {
                                            fcClip->SetKeyframeFunctionName(kfIdx, funcNames[fn]);
                                        }
                                    }
                                }

                                ImGui::EndMenu();
                            }
                        }
                        else
                        {
                            ImGui::TextDisabled("Set Function (select a keyframe on a node with a script)");
                        }
                    }
                }
            }
        }
        ImGui::EndPopup();
    }

    // Track context menu (opened by right-clicking track list on the left)
    if (ImGui::BeginPopup("TrackContextMenu"))
    {
        if (state->mTimelineSelectedTrack >= 0)
        {
            TimelineTrack* track = timeline->GetTrack(state->mTimelineSelectedTrack);
            if (track != nullptr)
            {
                if (ImGui::Selectable("Add Clip"))
                {
                    TimelineClip* newClip = TimelineClip::CreateInstance(track->GetDefaultClipType());
                    if (newClip != nullptr)
                    {
                        newClip->SetStartTime(playheadTime);
                        newClip->SetDuration(1.0f);
                        track->AddClip(newClip);
                    }
                }
                if (ImGui::Selectable("Remove Track"))
                {
                    state->InspectObject(nullptr, true, false);
                    timeline->RemoveTrack(state->mTimelineSelectedTrack);
                    state->mTimelineSelectedTrack = -1;
                    state->mTimelineSelectedClip = -1;
                }
            }
        }
        ImGui::EndPopup();
    }

    // ========== Draw Playhead ==========
    float totalTracksHeight = kRulerHeight + tracks.size() * kTrackHeight;
    DrawPlayhead(drawList, rulerPos, totalTracksHeight, playheadTime, zoom, scrollX);

    // ========== Scroll with mouse wheel ==========
    // Use a dummy item instead of InvisibleButton so it doesn't steal clicks from clips.
    ImGui::SetCursorScreenPos(contentPos);
    ImGui::Dummy(ImVec2(windowSize.x, contentHeight));
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
    {
        float wheel = ImGui::GetIO().MouseWheel;
        if (IsControlDown())
        {
            // Zoom
            zoom += wheel * 10.0f;
            zoom = glm::clamp(zoom, 10.0f, 1000.0f);
        }
        else
        {
            // Scroll
            scrollX -= wheel * 30.0f;
            scrollX = glm::max(0.0f, scrollX);
        }
    }

    // ========== Preview: Evaluate timeline at playhead ==========
    if (state->mTimelinePreviewing)
    {
        float dt = ImGui::GetIO().DeltaTime;
        playheadTime += dt * timeline->GetPlayRate();

        if (playheadTime >= duration)
        {
            if (timeline->IsLooping())
            {
                playheadTime = fmodf(playheadTime, duration);
            }
            else
            {
                playheadTime = duration;
                state->mTimelinePreviewing = false;
                resetTracks = true;
            }
        }
    }

    // Evaluate timeline at current playhead position for editor preview
    // Find a TimelinePlayer in the world, or just evaluate directly
    World* world = GetWorld(0);
    if (world != nullptr && timeline != nullptr)
    {
        if (state->mTimelinePreviewInstance == nullptr)
        {
            state->mTimelinePreviewInstance = new TimelineInstance();
        }

        TimelineInstance* inst = state->mTimelinePreviewInstance;
        inst->SetTrackCount(timeline->GetNumTracks());
        inst->ResolveBindings(world, timeline->GetTracks());

        if (state->mTimelinePreviewing)
        {
            for (uint32_t i = 0; i < tracks.size(); ++i)
            {
                if (tracks[i]->IsMuted())
                    continue;

                TrackInstanceData& data = inst->GetTrackData(i);
                tracks[i]->Evaluate(playheadTime, data.mResolvedNode, inst);
            }
        }

        if (resetTracks)
        {
            for (uint32_t i = 0; i < tracks.size(); ++i)
            {
                TrackInstanceData& data = inst->GetTrackData(i);
                tracks[i]->Reset(data.mResolvedNode, inst);
            }
        }
    }

    ImGui::End();
}

#endif
