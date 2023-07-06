#include "Profiler.h"
#include "Engine.h"
#include "System/System.h"
#include "Maths.h"
#include "Clock.h"
#include "Log.h"
#include "Assertion.h"

#include "Graphics/Graphics.h"

static Profiler* sProfiler = nullptr;

void Profiler::BeginFrame()
{
#if PROFILING_ENABLED
    // Clear out the start/end/elapse time on all stats
    for (uint32_t i = 0; i < mCpuFrameStats.size(); ++i)
    {
        mCpuFrameStats[i].mTime = 0.0f;
        mCpuFrameStats[i].mStartTime = 0;
        mCpuFrameStats[i].mEndTime = 0;
    }
#endif
}

void Profiler::EndFrame()
{
#if PROFILING_ENABLED
    float deltaTime = GetAppClock()->DeltaTime();

    // Calculate the elapsed time in milliseconds from the start/end microsecond times
    for (uint32_t i = 0; i < mCpuFrameStats.size(); ++i)
    {
        mCpuFrameStats[i].mSmoothedTime = Maths::Damp(mCpuFrameStats[i].mSmoothedTime, mCpuFrameStats[i].mTime, 0.05f, deltaTime);
    }

    for (uint32_t i = 0; i < mGpuStats.size(); ++i)
    {
        mGpuStats[i].mSmoothedTime = Maths::Damp(mGpuStats[i].mSmoothedTime, mGpuStats[i].mTime, 0.05f, deltaTime);
    }
#endif
}

void Profiler::BeginCpuStat(const char* name, bool persistent)
{
#if PROFILING_ENABLED
    CpuStat* stat = FindCpuStat(name, persistent);

    if (stat == nullptr)
    {
        CpuStat newStat;
        strncpy(newStat.mName, name, STAT_NAME_LENGTH);

        if (persistent)
        {
            mCpuPersistentStats.push_back(newStat);
            stat = &mCpuPersistentStats.back();
        }
        else
        {
            mCpuFrameStats.push_back(newStat);
            stat = &mCpuFrameStats.back();
        }
    }

    stat->mStartTime = SYS_GetTimeMicroseconds();
#endif
}

void Profiler::EndCpuStat(const char* name, bool persistent)
{
#if PROFILING_ENABLED
    CpuStat* stat = FindCpuStat(name, persistent);
    OCT_ASSERT(stat);

    if (stat)
    {
        stat->mEndTime = SYS_GetTimeMicroseconds();
        stat->mTime += (stat->mEndTime - stat->mStartTime) / 1000.0f;
    }
#endif
}

void Profiler::BeginGpuStat(const char* name)
{
#if PROFILING_ENABLED
    GFX_BeginGpuTimestamp(name);
#endif
}

void Profiler::EndGpuStat(const char* name)
{
#if PROFILING_ENABLED
    GFX_EndGpuTimestamp(name);
#endif
}

void Profiler::SetGpuStatTime(const char* name, float time)
{
    // The GFX implementation is responsible for calling this on all GPU stats at Frame's end.
#if PROFILING_ENABLED
    GpuStat* gpuStat = nullptr;
    for (uint32_t i = 0; i < mGpuStats.size(); ++i)
    {
        if (strncmp(mGpuStats[i].mName, name, STAT_NAME_LENGTH) == 0)
        {
            gpuStat = &mGpuStats[i];
            break;
        }
    }

    if (gpuStat == nullptr)
    {
        mGpuStats.push_back(GpuStat());
        gpuStat = &(mGpuStats.back());
        strncpy(gpuStat->mName, name, STAT_NAME_LENGTH);
    }

    gpuStat->mTime = time;
#endif
}

CpuStat* Profiler::FindCpuStat(const char* name, bool persistent)
{
    std::vector<CpuStat>& stats = persistent ? mCpuPersistentStats : mCpuFrameStats;
    CpuStat* retStat = nullptr;

#if PROFILING_ENABLED
    for (uint32_t i = 0; i < stats.size(); ++i)
    {
        if (strncmp(stats[i].mName, name, STAT_NAME_LENGTH) == 0)
        {
            retStat = &stats[i];
            break;
        }
    }
#endif

    return retStat;
}

const std::vector<CpuStat>& Profiler::GetCpuFrameStats() const
{
    return mCpuFrameStats;
}

const std::vector<CpuStat>& Profiler::GetCpuPersistentStats() const
{
    return mCpuPersistentStats;
}

const std::vector<GpuStat>& Profiler::GetGpuStats() const
{
    return mGpuStats;
}

void Profiler::LogPersistentStats()
{
    LogDebug("----- Persistent Stats -----");

    for (uint32_t i = 0; i < mCpuPersistentStats.size(); ++i)
    {
        LogDebug("%s: %f", mCpuPersistentStats[i].mName, mCpuPersistentStats[i].mTime);
    }

    LogDebug("----------------------------");
}

void Profiler::DumpPersistentStats()
{
    FILE* statFile = fopen("CpuStats.csv", "w");

    if (statFile != nullptr)
    {
        for (uint32_t i = 0; i < mCpuPersistentStats.size(); ++i)
        {
            fprintf(statFile, "%s, %f\n", mCpuPersistentStats[i].mName, mCpuPersistentStats[i].mTime);
        }

        fclose(statFile);
        statFile = nullptr;
    }
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
