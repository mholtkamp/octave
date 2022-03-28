#pragma once

#include <chrono>

class Clock
{
public:

    Clock();

    ~Clock();

    void Start();

    void Stop();

    void Update();

    float DeltaTime() const;

    float GetTime() const;

private:

    uint64_t mPreviousTimeUs = 0;
    uint64_t mCurrentTimeUs = 0;
    uint64_t mStartTimeUs = 0;

    float mTimeSeconds = 0.0f;
    float mDeltaTimeSeconds = 0.0f;
    bool mActive = false;
};