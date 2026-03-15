#include "Timeline/TimelineInstance.h"
#include "Timeline/TimelineTrack.h"
#include "Nodes/Node.h"
#include "Nodes/3D/Node3d.h"
#include "World.h"

TimelineInstance::TimelineInstance()
{
}

TimelineInstance::~TimelineInstance()
{
}

void TimelineInstance::SetTrackCount(uint32_t count)
{
    mTrackData.resize(count);
}

TrackInstanceData& TimelineInstance::GetTrackData(uint32_t index)
{
    return mTrackData[index];
}

void TimelineInstance::ResolveBindings(World* world, const std::vector<TimelineTrack*>& tracks)
{
    if (world == nullptr)
        return;

    SetTrackCount((uint32_t)tracks.size());

    for (uint32_t i = 0; i < tracks.size(); ++i)
    {
        TrackInstanceData& data = mTrackData[i];

        if (!data.mBindingResolved)
        {
            uint64_t uuid = tracks[i]->GetTargetNodeUuid();
            data.mResolvedNode = world->FindNodeByUuid(uuid);
            data.mBindingResolved = (data.mResolvedNode != nullptr);
        }
    }
}

void TimelineInstance::CapturePrePlayState(const std::vector<TimelineTrack*>& tracks)
{
    for (uint32_t i = 0; i < mTrackData.size() && i < tracks.size(); ++i)
    {
        TrackInstanceData& data = mTrackData[i];
        Node* node = data.mResolvedNode;

        if (node != nullptr && !data.mStateSnapshotted)
        {
            data.mPrePlayActive = node->IsActive();
            data.mPrePlayVisible = node->IsVisible();

            Node3D* node3d = node->As<Node3D>();
            if (node3d != nullptr)
            {
                data.mPrePlayPosition = node3d->GetPosition();
                data.mPrePlayRotation = node3d->GetRotationQuat();
                data.mPrePlayScale = node3d->GetScale();
            }

            data.mStateSnapshotted = true;
        }
    }
}

void TimelineInstance::RestorePrePlayState(const std::vector<TimelineTrack*>& tracks)
{
    for (uint32_t i = 0; i < mTrackData.size() && i < tracks.size(); ++i)
    {
        TrackInstanceData& data = mTrackData[i];
        Node* node = data.mResolvedNode;

        if (node != nullptr && data.mStateSnapshotted)
        {
            node->SetActive(data.mPrePlayActive);
            node->SetVisible(data.mPrePlayVisible);

            Node3D* node3d = node->As<Node3D>();
            if (node3d != nullptr)
            {
                node3d->SetPosition(data.mPrePlayPosition);
                node3d->SetRotation(data.mPrePlayRotation);
                node3d->SetScale(data.mPrePlayScale);
            }

            data.mStateSnapshotted = false;
        }
    }
}

void TimelineInstance::ResetAll()
{
    for (uint32_t i = 0; i < mTrackData.size(); ++i)
    {
        mTrackData[i].mClipPlaying = false;
        mTrackData[i].mLastEvaluatedClipIndex = -1;
    }
}
