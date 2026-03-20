/**
 * @file TinyLLMAsset.h
 * @brief Asset type for ultra-tiny LLM models (llama2.c format).
 *
 * Contains model weights and configuration. References a separate
 * TinyLLMTokenizerAsset for vocabulary.
 * Supports models up to ~15M parameters, though only ~260K is
 * practical on memory-constrained platforms like Wii.
 */

#pragma once

#include "Asset.h"
#include "AssetRef.h"
#include <vector>
#include <string>

class TinyLLMTokenizerAsset;

// Prevent duplicate definition in plugin's TinyLLMInference.h
#ifndef TINYLLM_CONFIG_DEFINED
#define TINYLLM_CONFIG_DEFINED
/**
 * @brief Configuration for the transformer model.
 */
struct TinyLLMConfig
{
    int32_t dim = 0;        // transformer dimension
    int32_t hidden_dim = 0; // FFN hidden dimension
    int32_t n_layers = 0;   // number of transformer layers
    int32_t n_heads = 0;    // number of query heads
    int32_t n_kv_heads = 0; // number of key/value heads (can differ for GQA)
    int32_t vocab_size = 0; // vocabulary size
    int32_t seq_len = 0;    // max sequence length
};
#endif

/**
 * @class TinyLLMAsset
 * @brief Asset containing a tiny LLM model and tokenizer.
 */
class OCTAVE_API TinyLLMAsset : public Asset
{
public:

    DECLARE_ASSET(TinyLLMAsset, Asset);

    TinyLLMAsset();
    virtual ~TinyLLMAsset();

    // Asset interface
    virtual void Create() override;
    virtual void Destroy() override;
    virtual void LoadStream(Stream& stream, Platform platform) override;
    virtual void SaveStream(Stream& stream, Platform platform) override;
    virtual bool Import(const std::string& path, ImportOptions* options) override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual glm::vec4 GetTypeColor() override;
    virtual const char* GetTypeName() override;
    virtual const char* GetTypeImportExt() override;

    // Configuration
    const TinyLLMConfig& GetConfig() const { return mConfig; }

    // Weight access (pointers into mWeightData)
    float* GetTokenEmbedding() const { return mTokenEmbedding; }
    float* GetRmsAttWeight() const { return mRmsAttWeight; }
    float* GetWq() const { return mWq; }
    float* GetWk() const { return mWk; }
    float* GetWv() const { return mWv; }
    float* GetWo() const { return mWo; }
    float* GetRmsFfnWeight() const { return mRmsFfnWeight; }
    float* GetW1() const { return mW1; }
    float* GetW2() const { return mW2; }
    float* GetW3() const { return mW3; }
    float* GetRmsFinalWeight() const { return mRmsFinalWeight; }
    float* GetWcls() const { return mWcls; }

    // Tokenizer asset reference
    void SetTokenizer(TinyLLMTokenizerAsset* tokenizer);
    TinyLLMTokenizerAsset* GetTokenizer() const;

    // Tokenization (delegates to tokenizer asset)
    void Encode(const char* text, bool addBos, bool addEos, std::vector<int32_t>& outTokens);
    std::string Decode(int prevToken, int token);

    // Memory utilities
    size_t GetWeightMemorySize() const;
    size_t GetKVCacheMemorySize(int32_t maxSeqLen) const;

protected:

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

    void SetupWeightPointers();

    // Configuration
    TinyLLMConfig mConfig;

    // Weight pointers (into mWeightData)
    float* mTokenEmbedding = nullptr;
    float* mRmsAttWeight = nullptr;
    float* mWq = nullptr;
    float* mWk = nullptr;
    float* mWv = nullptr;
    float* mWo = nullptr;
    float* mRmsFfnWeight = nullptr;
    float* mW1 = nullptr;
    float* mW2 = nullptr;
    float* mW3 = nullptr;
    float* mRmsFinalWeight = nullptr;
    float* mWcls = nullptr;

    // Weight storage
    std::vector<float> mWeightData;

    // Reference to tokenizer asset
    AssetRef mTokenizer;
};
