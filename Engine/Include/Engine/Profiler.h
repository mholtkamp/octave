#pragma once

#include <stdint.h>
#include <vector>

#include <string.h>

#define PROFILING_ENABLED 1

#define STAT_NAME_LENGTH 15
#define STAT_NAME_BUFFER_LENGTH (STAT_NAME_LENGTH + 1)

struct CpuStat
{
    char mName[STAT_NAME_BUFFER_LENGTH] = {};
    uint64_t mStartTime = 0;
    uint64_t mEndTime = 0;
    float mTime = 0.0f;
    float mSmoothedTime = 0.0f;
};

//struct GpuStat
//{
//    const char mName[STAT_NAME_BUFFER_LENGTH] = {};
//    float mTime = 0.0f;
//    float mSmoothedTime = 0.0f;
//};

class Profiler
{
public:

    void BeginFrame();
    void EndFrame();

    void BeginCpuStat(const char* name);
    void EndCpuStat(const char* name);

    //void BeginGpuStat(const char* name);
    //void EndGpuStat();

    CpuStat* FindCpuStat(const char* name);
    const std::vector<CpuStat>& GetCpuStats() const;

protected:

    std::vector<CpuStat> mCpuStats;
    //std::vector<GpuStat> mGpuStats;
};

void CreateProfiler();
void DestroyProfiler();
Profiler* GetProfiler();

struct ScopedCpuStat
{
    ScopedCpuStat(const char* name)
    {
        strncpy(mName, name, STAT_NAME_LENGTH);
        GetProfiler()->BeginCpuStat(mName);
    }

    ~ScopedCpuStat()
    {
        GetProfiler()->EndCpuStat(mName);
    }

    char mName[STAT_NAME_BUFFER_LENGTH] = {};
};

#if PROFILING_ENABLED
#define SCOPED_CPU_STAT(name) ScopedCpuStat scopedStat##__LINE__(name);
#define BEGIN_CPU_STAT(name) GetProfiler()->BeginCpuStat(name);
#define END_CPU_STAT(name) GetProfiler()->EndCpuStat(name);
#else
#define SCOPED_CPU_STAT(name) 
#define BEGIN_CPU_STAT(name) 
#define END_CPU_STAT(name) 
#endif
