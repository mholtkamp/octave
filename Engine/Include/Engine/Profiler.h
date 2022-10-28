#pragma once

#include <stdint.h>
#include <vector>

#include <string.h>

#define PROFILING_ENABLED 1

#define STAT_NAME_LENGTH 31
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

    void BeginCpuStat(const char* name, bool persistent);
    void EndCpuStat(const char* name, bool persistent);

    //void BeginGpuStat(const char* name);
    //void EndGpuStat();

    CpuStat* FindCpuStat(const char* name, bool persistent);
    const std::vector<CpuStat>& GetCpuFrameStats() const;

    const std::vector<CpuStat>& GetCpuPersistentStats() const;

    void LogPersistentStats();
    void DumpPersistentStats();

protected:

    std::vector<CpuStat> mCpuFrameStats;
    std::vector<CpuStat> mCpuPersistentStats;
    //std::vector<GpuStat> mGpuStats;
};

void CreateProfiler();
void DestroyProfiler();
Profiler* GetProfiler();

struct ScopedCpuStat
{
    ScopedCpuStat(const char* name, bool persistent)
    {
        strncpy(mName, name, STAT_NAME_LENGTH);
        mPersistent = persistent;
        GetProfiler()->BeginCpuStat(mName, mPersistent);
    }

    ~ScopedCpuStat()
    {
        GetProfiler()->EndCpuStat(mName, mPersistent);
    }

    char mName[STAT_NAME_BUFFER_LENGTH] = {};
    bool mPersistent = false;
};

#if PROFILING_ENABLED
#define SCOPED_FRAME_STAT(name) ScopedCpuStat scopedStat##__LINE__(name, false);
#define BEGIN_FRAME_STAT(name) GetProfiler()->BeginCpuStat(name, false);
#define END_FRAME_STAT(name) GetProfiler()->EndCpuStat(name, false);

#define SCOPED_STAT(name) ScopedCpuStat scopedStat##__LINE__(name, true);
#define BEGIN_STAT(name) GetProfiler()->BeginCpuStat(name, true);
#define END_STAT(name) GetProfiler()->EndCpuStat(name, true);
#else
#define SCOPED_FRAME_STAT(name) 
#define BEGIN_FRAME_STAT(name) 
#define END_FRAME_STAT(name) 

#define SCOPED_STAT(name)
#define BEGIN_STAT(name)
#define END_STAT(name)
#endif
