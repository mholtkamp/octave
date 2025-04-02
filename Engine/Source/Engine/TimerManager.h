#pragma once

#include <stdint.h>
#include <string>
#include "SmartPointer.h"
#include "ScriptFunc.h"

class ScriptComponent;
class Node;

typedef void(*TimerHandlerFP)();
typedef void(*PointerTimerHandlerFP)(void*);
typedef void(*NodeTimerHandlerFP)(Node* node);

enum class TimerType
{
    Void,
    Object,
    Node,
    ScriptFunc,

    Count
};

struct TimerData
{
    // Can this be made a union?
    // Not sure about ActorRef construction/destruction...
    void* mPointer = nullptr;
    NodePtr mNode;
    ScriptFunc mScriptFunc;

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
    int32_t SetTimer(Node* node, NodeTimerHandlerFP handler, float time, bool loop = false);
    int32_t SetTimer(ScriptFunc scriptFunc, float time, bool loop = false);

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
