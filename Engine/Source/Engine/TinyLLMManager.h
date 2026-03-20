/**
 * @file TinyLLMManager.h
 * @brief Singleton manager for TinyLLM inference.
 */

#pragma once

#include "OctaveAPI.h"
#include "Assets/TinyLLMAsset.h"
#include <string>
#include <vector>
#include <stdint.h>

/**
 * @class TinyLLMManager
 * @brief Singleton that manages LLM inference state.
 */
class OCTAVE_API TinyLLMManager
{
public:
    static TinyLLMManager* Get();
    static void Create();
    static void Destroy();

    // Model lifecycle
    bool LoadModel(TinyLLMAsset* asset, int32_t maxSeqLen = 0);
    void UnloadModel();
    bool IsModelLoaded() const;
    TinyLLMAsset* GetModel() const;

    // Low-level inference
    void Reset();
    float* Forward(int32_t token, int32_t pos);
    int32_t Sample(float temperature = 1.0f, float topP = 0.9f);

    // High-level generation
    std::string Generate(const std::string& prompt, int32_t maxTokens,
                         float temperature = 1.0f, float topP = 0.9f);

    // Tokenization (uses engine's TinyLLMAsset directly - no DLL issues)
    std::vector<int32_t> Encode(const std::string& text, bool addBos = true);
    std::string Decode(int32_t prevToken, int32_t token);

    // Streaming support
    bool BeginGenerate(const std::string& prompt, int32_t maxTokens,
                       float temperature = 1.0f, float topP = 0.9f);
    std::string ContinueGenerate();
    bool IsGenerating() const;
    void Abort();

    // Stats
    float GetLastTokPerSec() const;
    int32_t GetPosition() const;
    int32_t GetMaxSeqLen() const;

    // Platform-specific defaults
    static int32_t GetDefaultMaxSeqLen();

    // Sampler state (public for comparison function)
    struct ProbIndex {
        float prob;
        int index;
    };

private:
    TinyLLMManager();
    ~TinyLLMManager();

    void AllocateRunState(int32_t maxSeqLen);
    void FreeRunState();
    void SetupRunStatePointers();
    int32_t SampleInternal(float* logits, float temperature, float topP);

    static TinyLLMManager* sInstance;

    TinyLLMAsset* mModel = nullptr;

    // RunState buffers
    float* mRunStateBuffer = nullptr;
    size_t mRunStateSize = 0;

    // Pointers into RunState
    float* mX = nullptr;
    float* mXb = nullptr;
    float* mXb2 = nullptr;
    float* mHb = nullptr;
    float* mHb2 = nullptr;
    float* mQ = nullptr;
    float* mKeyCache = nullptr;
    float* mValueCache = nullptr;
    float* mAtt = nullptr;
    float* mLogits = nullptr;

    std::vector<ProbIndex> mProbIndex;
    uint64_t mRngState = 0;

    // Sequence state
    int32_t mPos = 0;
    int32_t mMaxSeqLen = 0;

    // Streaming generation state
    bool mIsGenerating = false;
    std::vector<int32_t> mPromptTokens;
    int32_t mPromptIdx = 0;
    int32_t mLastToken = 0;
    int32_t mGeneratedCount = 0;
    int32_t mMaxGenTokens = 0;
    float mTemperature = 1.0f;
    float mTopP = 0.9f;

    // Stats
    float mLastTokPerSec = 0.0f;
    int64_t mGenStartTime = 0;
};
