#pragma once

#include "Factory.h"
#include "Object.h"
#include "Property.h"
#include "Stream.h"
#include "Timeline/TimelineClip.h"

#include <string>
#include <vector>

class Node;
class TimelineInstance;

#define DECLARE_TRACK(Class, Parent) \
    DECLARE_FACTORY(Class, TimelineTrack); \
    DECLARE_OBJECT(Class, Parent); \
    typedef Parent Super;

#define DEFINE_TRACK(Class) \
    DEFINE_FACTORY(Class, TimelineTrack); \
    DEFINE_OBJECT(Class);

class TimelineTrack : public Object
{
public:

    DECLARE_FACTORY_MANAGER(TimelineTrack);
    DECLARE_FACTORY(TimelineTrack, TimelineTrack);
    DECLARE_OBJECT(TimelineTrack, Object);

    TimelineTrack();
    virtual ~TimelineTrack();

    virtual void SaveStream(Stream& stream);
    virtual void LoadStream(Stream& stream, uint32_t version);

    virtual void GatherProperties(std::vector<Property>& outProps) override;

    virtual void Evaluate(float time, Node* target, TimelineInstance* inst);
    virtual void Reset(Node* target, TimelineInstance* inst);

    virtual const char* GetTrackTypeName() const { return "Track"; }
    virtual glm::vec4 GetTrackColor() const;
    virtual TypeId GetDefaultClipType() const;

    uint64_t GetTargetNodeUuid() const { return mTargetNodeUuid; }
    void SetTargetNodeUuid(uint64_t uuid) { mTargetNodeUuid = uuid; }

    const std::string& GetTargetNodeName() const { return mTargetNodeName; }
    void SetTargetNodeName(const std::string& name) { mTargetNodeName = name; }

    uint32_t GetNumClips() const { return (uint32_t)mClips.size(); }
    TimelineClip* GetClip(uint32_t index) const;
    void AddClip(TimelineClip* clip);
    void RemoveClip(uint32_t index);
    void InsertClip(TimelineClip* clip, uint32_t index);
    int32_t FindClipIndex(TimelineClip* clip) const;

    bool IsMuted() const { return mMuted; }
    void SetMuted(bool muted) { mMuted = muted; }

    bool IsLocked() const { return mLocked; }
    void SetLocked(bool locked) { mLocked = locked; }

protected:

    uint64_t mTargetNodeUuid = 0;
    std::string mTargetNodeName;
    std::vector<TimelineClip*> mClips;
    bool mMuted = false;
    bool mLocked = false;
};
