#include "Clock.h"
#include "Log.h"

#include "System/System.h"

Clock::Clock()
{

}

Clock::~Clock()
{

}

void Clock::Start()
{
    mActive = true;
    mDeltaTimeSeconds = 0.0f;

    mStartTimeUs = SYS_GetTimeMicroseconds();
    mPreviousTimeUs = mStartTimeUs;
    mCurrentTimeUs = mStartTimeUs;
}

void Clock::Stop()
{
    Update(); // Record the latest time.
    mActive = false;
}

void Clock::Update()
{
    if (mActive)
    {
        mCurrentTimeUs = SYS_GetTimeMicroseconds();
        if (mCurrentTimeUs > mPreviousTimeUs)
        {
            mDeltaTimeSeconds = (mCurrentTimeUs - mPreviousTimeUs) / 1000000.0f;
        }
        else
        {
            // Somehow on the Dolphin emulator, the current time was less than the previous time??
            // Didn't get to test this on console, but since we want the engine to work in Dolphin 5.0,
            // we'll keep this safety check in for now.
            mDeltaTimeSeconds = 0.0f;
        }

        mPreviousTimeUs = mCurrentTimeUs;

        mTimeSeconds = (mCurrentTimeUs - mStartTimeUs) / 1000000.0f;
    }
}

float Clock::DeltaTime() const
{
    return mDeltaTimeSeconds;
}

float Clock::GetTime() const
{
    return mTimeSeconds;
}
