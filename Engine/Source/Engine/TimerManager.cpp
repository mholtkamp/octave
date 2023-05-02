#include "TimerManager.h"

#include "Actor.h"
#include "Components/ScriptComponent.h"

TimerManager gTimerManager;

TimerManager* GetTimerManager()
{
    return &gTimerManager;
}

void TimerManager::Update(float deltaTime)
{
    static std::vector<TimerData> sTimersToExecute;
    sTimersToExecute.clear();

    for (int32_t i = 0; i < (int32_t)mTimerData.size(); ++i)
    {
        TimerData* timer = &(mTimerData[i]);

        if (!timer->mPaused)
        {
            timer->mTimeRemaining -= deltaTime;

            if (timer->mTimeRemaining <= 0.0f)
            {
                // Add this to the list of timers to execute.
                // This has to be done after iterating, otherwise these timer
                // handler functions could add/remove timers from the timer array.
                sTimersToExecute.push_back(*timer);

                if (timer->mLoop)
                {
                    // If looping, reset time remaining
                    timer->mTimeRemaining = timer->mDuration;
                }
                else
                {
                    // If not looping, remove the timer from the vector
                    mTimerData.erase(mTimerData.begin() + i);
                    --i;
                }
            }
        }
    }

    for (uint32_t i = 0; i < sTimersToExecute.size(); ++i)
    {
        TimerData* timer = &(sTimersToExecute[i]);

        // Execute callback handler
        switch (timer->mType)
        {
        case TimerType::Void:
        {
            if (timer->mHandler != nullptr)
            {
                TimerHandlerFP handler = (TimerHandlerFP)timer->mHandler;
                handler();
            }
            break;
        }
        case TimerType::Pointer:
        {
            if (timer->mHandler != nullptr)
            {
                PointerTimerHandlerFP handler = (PointerTimerHandlerFP)timer->mHandler;
                handler(timer->mPointer);
            }
            break;
        }
        case TimerType::Actor:
        {
            if (timer->mHandler != nullptr)
            {
                ActorTimerHandlerFP handler = (ActorTimerHandlerFP)timer->mHandler;
                Actor* actor = timer->mActor.Get();

                if (actor != nullptr)
                {
                    handler(actor);
                }
            }
            break;
        }
        case TimerType::Script:
        {
            if (timer->mScriptTableName != "" &&
                timer->mScriptFuncName != "")
            {
                ScriptUtils::CallMethod(
                    timer->mScriptTableName.c_str(),
                    timer->mScriptFuncName.c_str(),
                    0,
                    nullptr,
                    nullptr);
            }
            break;
        }
        case TimerType::ScriptFunc:
        {
            if (timer->mScriptFunc.IsValid())
            {
                timer->mScriptFunc.Call();
            }
            break;
        }
        default:
            OCT_ASSERT(0);
            break;
        }
    }
}

int32_t TimerManager::SetTimer(TimerHandlerFP handler, float time, bool loop)
{
    int32_t id = mNextTimerId++;

    TimerData timerData;
    timerData.mId = id;
    timerData.mHandler = (void*)handler;
    timerData.mType = TimerType::Void;
    timerData.mDuration = time;
    timerData.mLoop = loop;
    timerData.mTimeRemaining = time;
    mTimerData.push_back(timerData);

    return id;
}

int32_t TimerManager::SetTimer(void* vp, PointerTimerHandlerFP handler, float time, bool loop)
{
    int32_t id = mNextTimerId++;

    TimerData timerData;
    timerData.mId = id;
    timerData.mHandler = (void*)handler;
    timerData.mType = TimerType::Pointer;
    timerData.mPointer = vp;
    timerData.mDuration = time;
    timerData.mLoop = loop;
    timerData.mTimeRemaining = time;
    mTimerData.push_back(timerData);

    return id;
}

int32_t TimerManager::SetTimer(Actor* actor, ActorTimerHandlerFP handler, float time, bool loop)
{
    int32_t id = mNextTimerId++;

    TimerData timerData;
    timerData.mId = id;
    timerData.mHandler = (void*)handler;
    timerData.mType = TimerType::Actor;
    timerData.mActor = actor;
    timerData.mDuration = time;
    timerData.mLoop = loop;
    timerData.mTimeRemaining = time;
    mTimerData.push_back(timerData);

    return id;
}

int32_t TimerManager::SetTimer(const char* tableName, const char* funcName, float time, bool loop)
{
    int32_t id = mNextTimerId++;

    TimerData timerData;
    timerData.mId = id;
    timerData.mType = TimerType::Script;
    timerData.mScriptTableName = tableName;
    timerData.mScriptFuncName = funcName;
    timerData.mDuration = time;
    timerData.mLoop = loop;
    timerData.mTimeRemaining = time;
    mTimerData.push_back(timerData);

    return id;
}

int32_t TimerManager::SetTimer(ScriptFunc scriptFunc, float time, bool loop)
{
    int32_t id = mNextTimerId++;

    TimerData timerData;
    timerData.mId = id;
    timerData.mType = TimerType::ScriptFunc;
    timerData.mScriptFunc = scriptFunc;
    timerData.mDuration = time;
    timerData.mLoop = loop;
    timerData.mTimeRemaining = time;
    mTimerData.push_back(timerData);

    return id;
}

void TimerManager::ClearAllTimers()
{
    mTimerData.clear();
    mTimerData.shrink_to_fit();
}

void TimerManager::ClearTimer(int32_t id)
{
    int32_t index = -1;
    FindTimerData(id, &index);

    if (index >= 0)
    {
        mTimerData.erase(mTimerData.begin() + index);
    }
}

void TimerManager::PauseTimer(int32_t id)
{
    TimerData* timerData = FindTimerData(id);

    if (timerData)
    {
        timerData->mPaused = true;
    }
}

void TimerManager::ResumeTimer(int32_t id)
{
    TimerData* timerData = FindTimerData(id);

    if (timerData)
    {
        timerData->mPaused = false;
    }
}

void TimerManager::ResetTimer(int32_t id)
{
    TimerData* timerData = FindTimerData(id);

    if (timerData)
    {
        timerData->mTimeRemaining = timerData->mDuration;
    }
}

float TimerManager::GetTimeRemaining(int32_t id)
{
    float ret = 0.0f;
    TimerData* timerData = FindTimerData(id);

    if (timerData)
    {
        ret = timerData->mTimeRemaining;
    }

    return ret;
}


TimerData* TimerManager::FindTimerData(int32_t id, int32_t* outIndex)
{
    TimerData* ret = nullptr;
    int32_t index = -1;

    for (uint32_t i = 0; i < mTimerData.size(); ++i)
    {
        if (mTimerData[i].mId == id)
        {
            ret = &(mTimerData[i]);
            index = (int32_t)i;
            break;
        }
    }

    if (outIndex != nullptr)
    {
        *outIndex = index;
    }

    return ret;
}


