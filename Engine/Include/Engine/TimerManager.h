#pragma once

#include <stdint.h>
#include <string>
#include "ObjectRef.h"

class ScriptComponent;
class Actor;

typedef void(*TimerHandlerFP)();
typedef void(*PointerTimerHandlerFP)(void*);
typedef void(*ActorTimerHandlerFP)(Actor* actor);

enum class TimerType
{
    Void,
    Pointer,
    Actor,
    Script,

    Count
};

struct TimerData
{
    // Can this be made a union?
    // Not sure about ActorRef construction/destruction...
    void* mPointer = nullptr;
    ActorRef mActor;
    std::string mScriptTableName;
    std::string mScriptFuncName;

    int32_t mId = -1;
    void* mHandler = nullptr;
    float mDuration = 0.0f;
    float mTimeRemaining = 0.0f;
    bool mLoop = false;
    bool mPaused = false;
    TimerType mType = TimerType::Count;
};

class TimerManager
{
public:

    void Update(float deltaTime);

    // Returns the timer ID.
    int32_t SetTimer(TimerHandlerFP handler, float time, bool loop = false);
    int32_t SetTimer(void* vp, PointerTimerHandlerFP handler, float time, bool loop = false);
    int32_t SetTimer(Actor* actor, ActorTimerHandlerFP handler, float time, bool loop = false);
    int32_t SetTimer(ScriptComponent* scriptComp, const char* funcName, float time, bool loop = false);

    void ClearAllTimers();
    void ClearTimer(int32_t id);
    void PauseTimer(int32_t id);
    void ResumeTimer(int32_t id);
    void ResetTimer(int32_t id);
    float GetTimeRemaining(int32_t id);

    TimerData* FindTimerData(int32_t id, int32_t* outIndex = nullptr);

protected:

    int32_t mNextTimerId = 0;
    std::vector<TimerData> mTimerData;
};

TimerManager* GetTimerManager();
