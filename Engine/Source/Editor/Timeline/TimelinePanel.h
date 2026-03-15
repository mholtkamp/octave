#pragma once

#if EDITOR

class Timeline;
class TimelineTrack;

void DrawTimelinePanel();
void OpenTimelineForEditing(Timeline* timeline);
void CloseTimelinePanel();

#endif
