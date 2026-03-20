/**
 * @file TinyLLMManager.cpp
 * @brief Implementation of TinyLLMManager singleton.
 */

#include "TinyLLMManager.h"
#include "TinyLLMInference.h"
#include "Log.h"

#include <cstdlib>
#include <cstring>
#include <cmath>
#include <chrono>
#include <algorithm>

TinyLLMManager* TinyLLMManager::sInstance = nullptr;

TinyLLMManager* TinyLLMManager::Get()
{
    return sInstance;
}

void TinyLLMManager::Create()
{
    if (!sInstance) {
        sInstance = new TinyLLMManager();
    }
}

void TinyLLMManager::Destroy()
{
    delete sInstance;
    sInstance = nullptr;
}

TinyLLMManager::TinyLLMManager()
{
    auto now = std::chrono::high_resolution_clock::now();
    mRngState = (uint64_t)now.time_since_epoch().count();
}

TinyLLMManager::~TinyLLMManager()
{
    UnloadModel();
}

int32_t TinyLLMManager::GetDefaultMaxSeqLen()
{
#if defined(PLATFORM_DOLPHIN)
    return 128;
#else
    return 512;
#endif
}

bool TinyLLMManager::LoadModel(TinyLLMAsset* asset, int32_t maxSeqLen)
{
    if (!asset) {
        LogError("TinyLLMManager: null asset");
        return false;
    }

    UnloadModel();

    mModel = asset;

    if (maxSeqLen <= 0) {
        maxSeqLen = GetDefaultMaxSeqLen();
    }

    if (maxSeqLen > mModel->GetConfig().seq_len) {
        maxSeqLen = mModel->GetConfig().seq_len;
    }

    mMaxSeqLen = maxSeqLen;

    AllocateRunState(maxSeqLen);

    if (!mRunStateBuffer) {
        LogError("TinyLLMManager: Failed to allocate run state");
        mModel = nullptr;
        return false;
    }

    mProbIndex.resize(mModel->GetConfig().vocab_size);

    LogDebug("TinyLLMManager: Loaded model, maxSeqLen=%d", mMaxSeqLen);
    return true;
}

void TinyLLMManager::UnloadModel()
{
    Abort();
    FreeRunState();
    mProbIndex.clear();
    mModel = nullptr;
    mPos = 0;
    mMaxSeqLen = 0;
    mLastTokPerSec = 0.0f;
}

bool TinyLLMManager::IsModelLoaded() const
{
    return mModel != nullptr && mRunStateBuffer != nullptr;
}

TinyLLMAsset* TinyLLMManager::GetModel() const
{
    return mModel;
}

void TinyLLMManager::AllocateRunState(int32_t maxSeqLen)
{
    FreeRunState();

    const TinyLLMConfig& cfg = mModel->GetConfig();
    mRunStateSize = tinyllm_calc_runstate_size(&cfg, maxSeqLen);
    mRunStateBuffer = (float*)calloc(1, mRunStateSize);

    if (!mRunStateBuffer) {
        mRunStateSize = 0;
        return;
    }

    SetupRunStatePointers();
}

void TinyLLMManager::FreeRunState()
{
    if (mRunStateBuffer) {
        free(mRunStateBuffer);
        mRunStateBuffer = nullptr;
    }

    mRunStateSize = 0;
    mX = nullptr;
    mXb = nullptr;
    mXb2 = nullptr;
    mHb = nullptr;
    mHb2 = nullptr;
    mQ = nullptr;
    mKeyCache = nullptr;
    mValueCache = nullptr;
    mAtt = nullptr;
    mLogits = nullptr;
}

void TinyLLMManager::SetupRunStatePointers()
{
    if (!mRunStateBuffer || !mModel) return;

    const TinyLLMConfig& cfg = mModel->GetConfig();
    int kv_dim = (cfg.dim * cfg.n_kv_heads) / cfg.n_heads;

    float* ptr = mRunStateBuffer;

    mX = ptr;
    ptr += cfg.dim;

    mXb = ptr;
    ptr += cfg.dim;

    mXb2 = ptr;
    ptr += cfg.dim;

    mHb = ptr;
    ptr += cfg.hidden_dim;

    mHb2 = ptr;
    ptr += cfg.hidden_dim;

    mQ = ptr;
    ptr += cfg.dim;

    mKeyCache = ptr;
    ptr += cfg.n_layers * mMaxSeqLen * kv_dim;

    mValueCache = ptr;
    ptr += cfg.n_layers * mMaxSeqLen * kv_dim;

    mAtt = ptr;
    ptr += cfg.n_heads * mMaxSeqLen;

    mLogits = ptr;
}

void TinyLLMManager::Reset()
{
    mPos = 0;
    mIsGenerating = false;

    if (mKeyCache && mModel) {
        const TinyLLMConfig& cfg = mModel->GetConfig();
        int kv_dim = (cfg.dim * cfg.n_kv_heads) / cfg.n_heads;
        size_t cacheSize = cfg.n_layers * mMaxSeqLen * kv_dim * sizeof(float);
        memset(mKeyCache, 0, cacheSize);
        memset(mValueCache, 0, cacheSize);
    }
}

float* TinyLLMManager::Forward(int32_t token, int32_t pos)
{
    if (!IsModelLoaded()) return nullptr;

    TinyLLMConfig cfg = mModel->GetConfig();
    cfg.seq_len = mMaxSeqLen;

    return tinyllm_forward(
        mX, mXb, mXb2, mHb, mHb2,
        mQ, mKeyCache, mValueCache, mAtt, mLogits,
        mModel->GetTokenEmbedding(), mModel->GetRmsAttWeight(),
        mModel->GetWq(), mModel->GetWk(), mModel->GetWv(), mModel->GetWo(),
        mModel->GetRmsFfnWeight(),
        mModel->GetW1(), mModel->GetW2(), mModel->GetW3(),
        mModel->GetRmsFinalWeight(), mModel->GetWcls(),
        &cfg,
        token, pos
    );
}

// XorShift RNG
static uint32_t RandomU32(uint64_t* state)
{
    *state ^= *state >> 12;
    *state ^= *state << 25;
    *state ^= *state >> 27;
    return (uint32_t)((*state * 0x2545F4914F6CDD1DULL) >> 32);
}

static float RandomF32(uint64_t* state)
{
    return (float)(RandomU32(state) >> 8) / 16777216.0f;
}

static int CompareProbIndex(const void* a, const void* b)
{
    const TinyLLMManager::ProbIndex* pa = (const TinyLLMManager::ProbIndex*)a;
    const TinyLLMManager::ProbIndex* pb = (const TinyLLMManager::ProbIndex*)b;
    if (pa->prob > pb->prob) return -1;
    if (pa->prob < pb->prob) return 1;
    return 0;
}

int32_t TinyLLMManager::SampleInternal(float* logits, float temperature, float topP)
{
    if (!mModel) return 0;

    int vocabSize = mModel->GetConfig().vocab_size;

    if (temperature == 0.0f) {
        int maxI = 0;
        float maxP = logits[0];
        for (int i = 1; i < vocabSize; i++) {
            if (logits[i] > maxP) {
                maxI = i;
                maxP = logits[i];
            }
        }
        return maxI;
    }

    for (int i = 0; i < vocabSize; i++) {
        logits[i] /= temperature;
    }

    tinyllm_softmax(logits, vocabSize);

    float coin = RandomF32(&mRngState);

    if (topP <= 0.0f || topP >= 1.0f) {
        float cdf = 0.0f;
        for (int i = 0; i < vocabSize; i++) {
            cdf += logits[i];
            if (coin < cdf) {
                return i;
            }
        }
        return vocabSize - 1;
    }

    int n0 = 0;
    float cutoff = (1.0f - topP) / (float)(vocabSize - 1);

    for (int i = 0; i < vocabSize; i++) {
        if (logits[i] >= cutoff) {
            mProbIndex[n0].index = i;
            mProbIndex[n0].prob = logits[i];
            n0++;
        }
    }

    qsort(mProbIndex.data(), n0, sizeof(ProbIndex), CompareProbIndex);

    float cumProb = 0.0f;
    int lastIdx = n0 - 1;
    for (int i = 0; i < n0; i++) {
        cumProb += mProbIndex[i].prob;
        if (cumProb > topP) {
            lastIdx = i;
            break;
        }
    }

    float r = coin * cumProb;
    float cdf = 0.0f;
    for (int i = 0; i <= lastIdx; i++) {
        cdf += mProbIndex[i].prob;
        if (r < cdf) {
            return mProbIndex[i].index;
        }
    }

    return mProbIndex[lastIdx].index;
}

int32_t TinyLLMManager::Sample(float temperature, float topP)
{
    if (!mLogits) return 0;
    return SampleInternal(mLogits, temperature, topP);
}

std::vector<int32_t> TinyLLMManager::Encode(const std::string& text, bool addBos)
{
    std::vector<int32_t> tokens;
    if (mModel) {
        mModel->Encode(text.c_str(), addBos, false, tokens);
    }
    return tokens;
}

std::string TinyLLMManager::Decode(int32_t prevToken, int32_t token)
{
    if (mModel) {
        return mModel->Decode(prevToken, token);
    }
    return "";
}

std::string TinyLLMManager::Generate(const std::string& prompt, int32_t maxTokens,
                                     float temperature, float topP)
{
    if (!IsModelLoaded()) return "";

    Reset();

    std::vector<int32_t> promptTokens = Encode(prompt, true);
    if (promptTokens.empty()) {
        LogError("TinyLLMManager: Encode returned empty tokens");
        return "";
    }

    std::string result;
    int32_t token = promptTokens[0];
    int32_t prevToken = 0;
    int32_t pos = 0;

    auto startTime = std::chrono::high_resolution_clock::now();
    bool started = false;

    while (pos < maxTokens && pos < mMaxSeqLen) {
        float* logits = Forward(token, pos);
        if (!logits) break;

        int32_t next;
        if (pos < (int32_t)promptTokens.size() - 1) {
            next = promptTokens[pos + 1];
        } else {
            next = SampleInternal(logits, temperature, topP);

            if (!started) {
                startTime = std::chrono::high_resolution_clock::now();
                started = true;
            }
        }

        pos++;

        if (next == 1 || next == 2) break;

        if (pos > (int32_t)promptTokens.size()) {
            std::string piece = Decode(prevToken, next);
            result += piece;
        }

        prevToken = token;
        token = next;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    int generatedTokens = pos - (int)promptTokens.size();
    if (duration.count() > 0 && generatedTokens > 0) {
        mLastTokPerSec = (float)generatedTokens / ((float)duration.count() / 1000.0f);
    }

    mPos = pos;
    return result;
}

bool TinyLLMManager::BeginGenerate(const std::string& prompt, int32_t maxTokens,
                                   float temperature, float topP)
{
    if (!IsModelLoaded()) return false;

    Reset();

    mPromptTokens = Encode(prompt, true);
    if (mPromptTokens.empty()) return false;

    mIsGenerating = true;
    mPromptIdx = 0;
    mLastToken = mPromptTokens[0];
    mGeneratedCount = 0;
    mMaxGenTokens = maxTokens;
    mTemperature = temperature;
    mTopP = topP;

    mGenStartTime = std::chrono::high_resolution_clock::now().time_since_epoch().count();

    return true;
}

std::string TinyLLMManager::ContinueGenerate()
{
    if (!mIsGenerating || !IsModelLoaded()) {
        mIsGenerating = false;
        return "";
    }

    if (mPos >= mMaxSeqLen || mGeneratedCount >= mMaxGenTokens) {
        mIsGenerating = false;
        return "";
    }

    float* logits = Forward(mLastToken, mPos);
    if (!logits) {
        mIsGenerating = false;
        return "";
    }

    int32_t next;
    bool isPrompt = (mPromptIdx < (int32_t)mPromptTokens.size() - 1);

    if (isPrompt) {
        mPromptIdx++;
        next = mPromptTokens[mPromptIdx];
    } else {
        next = SampleInternal(logits, mTemperature, mTopP);
        mGeneratedCount++;
    }

    mPos++;

    if (next == 1 || next == 2) {
        mIsGenerating = false;
        return "";
    }

    std::string result;
    if (!isPrompt) {
        int prevToken = mPromptIdx > 0 ? mPromptTokens[mPromptIdx - 1] : mLastToken;
        result = Decode(prevToken, next);
    }

    mLastToken = next;
    return result;
}

bool TinyLLMManager::IsGenerating() const
{
    return mIsGenerating;
}

void TinyLLMManager::Abort()
{
    if (mIsGenerating) {
        auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        int64_t duration = now - mGenStartTime;
        if (duration > 0 && mGeneratedCount > 0) {
            float seconds = (float)duration / 1e9f;
            mLastTokPerSec = (float)mGeneratedCount / seconds;
        }
    }

    mIsGenerating = false;
    mPromptTokens.clear();
    mPromptIdx = 0;
}

float TinyLLMManager::GetLastTokPerSec() const
{
    return mLastTokPerSec;
}

int32_t TinyLLMManager::GetPosition() const
{
    return mPos;
}

int32_t TinyLLMManager::GetMaxSeqLen() const
{
    return mMaxSeqLen;
}
