#include "Profiler.h"
#include "Engine.h"
#include "System/System.h"
#include "Maths.h"
#include "Clock.h"

#include <assert.h>

static Profiler* sProfiler = nullptr;

void Profiler::BeginFrame()
{
#if PROFILING_ENABLED
    // Clear out the start/end/elapse time on all stats
    for (uint32_t i = 0; i < mCpuStats.size(); ++i)
    {
        mCpuStats[i].mTime = 0.0f;
        mCpuStats[i].mStartTime = 0;
        mCpuStats[i].mEndTime = 0;
    }
#endif
}

void Profiler::EndFrame()
{
#if PROFILING_ENABLED
    float deltaTime = GetAppClock()->DeltaTime();

    // Calculate the elapsed time in milliseconds from the start/end microsecond times
    for (uint32_t i = 0; i < mCpuStats.size(); ++i)
    {
        mCpuStats[i].mSmoothedTime = Maths::Damp(mCpuStats[i].mSmoothedTime, mCpuStats[i].mTime, 0.05f, deltaTime);
    }
#endif
}

void Profiler::BeginCpuStat(const char* name)
{
#if PROFILING_ENABLED
    CpuStat* stat = FindCpuStat(name);

    if (stat == nullptr)
    {
        CpuStat newStat;
        strncpy(newStat.mName, name, STAT_NAME_LENGTH);
        mCpuStats.push_back(newStat);
        stat = &mCpuStats.back();
    }

    stat->mStartTime = SYS_GetTimeMicroseconds();
#endif
}

void Profiler::EndCpuStat(const char* name)
{
#if PROFILING_ENABLED
    CpuStat* stat = FindCpuStat(name);
    assert(stat);

    if (stat)
    {
        stat->mEndTime = SYS_GetTimeMicroseconds();
        stat->mTime += (stat->mEndTime - stat->mStartTime) / 1000.0f;
    }
#endif
}

CpuStat* Profiler::FindCpuStat(const char* name)
{
    CpuStat* retStat = nullptr;

#if PROFILING_ENABLED
    for (uint32_t i = 0; i < mCpuStats.size(); ++i)
    {
        if (strncmp(mCpuStats[i].mName, name, STAT_NAME_LENGTH) == 0)
        {
            retStat = &mCpuStats[i];
        }
    }
#endif

    return retStat;
}

const std::vector<CpuStat>& Profiler::GetCpuStats() const
{
    return mCpuStats;
}

void CreateProfiler()
{
#if PROFILING_ENABLED
    if (sProfiler == nullptr)
    {
        sProfiler = new Profiler();
    }
#endif
}

void DestroyProfiler()
{
#if PROFILING_ENABLED
    if (sProfiler != nullptr)
    {
        delete sProfiler;
        sProfiler = nullptr;
    }
#endif
}

Profiler* GetProfiler()
{
#if PROFILING_ENABLED
    return sProfiler;
#else
    return nullptr;
#endif
}
