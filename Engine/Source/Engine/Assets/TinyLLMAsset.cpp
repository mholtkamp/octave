/**
 * @file TinyLLMAsset.cpp
 * @brief Implementation of TinyLLMAsset.
 */

#include "Assets/TinyLLMAsset.h"
#include "Assets/TinyLLMTokenizerAsset.h"
#include "Stream.h"
#include "Property.h"
#include "Log.h"

#include <cstring>
#include <cstdlib>
#include <algorithm>

FORCE_LINK_DEF(TinyLLMAsset);
DEFINE_ASSET(TinyLLMAsset);

bool TinyLLMAsset::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    return HandleAssetPropChange(datum, index, newValue);
}

TinyLLMAsset::TinyLLMAsset()
{
    mType = TinyLLMAsset::GetStaticType();
}

TinyLLMAsset::~TinyLLMAsset()
{
}

void TinyLLMAsset::Create()
{
    Asset::Create();
}

void TinyLLMAsset::Destroy()
{
    Asset::Destroy();
    mWeightData.clear();
    mTokenizer = nullptr;

    mTokenEmbedding = nullptr;
    mRmsAttWeight = nullptr;
    mWq = nullptr;
    mWk = nullptr;
    mWv = nullptr;
    mWo = nullptr;
    mRmsFfnWeight = nullptr;
    mW1 = nullptr;
    mW2 = nullptr;
    mW3 = nullptr;
    mRmsFinalWeight = nullptr;
    mWcls = nullptr;
}

void TinyLLMAsset::LoadStream(Stream& stream, Platform platform)
{
    Asset::LoadStream(stream, platform);

    // Read config
    mConfig.dim = stream.ReadInt32();
    mConfig.hidden_dim = stream.ReadInt32();
    mConfig.n_layers = stream.ReadInt32();
    mConfig.n_heads = stream.ReadInt32();
    mConfig.n_kv_heads = stream.ReadInt32();
    mConfig.vocab_size = stream.ReadInt32();
    mConfig.seq_len = stream.ReadInt32();

    LogDebug("TinyLLMAsset::LoadStream config - dim=%d, hidden=%d, layers=%d, heads=%d, kv_heads=%d, vocab=%d, seq=%d",
        mConfig.dim, mConfig.hidden_dim, mConfig.n_layers, mConfig.n_heads,
        mConfig.n_kv_heads, mConfig.vocab_size, mConfig.seq_len);

    // Read tokenizer reference
    stream.ReadAsset(mTokenizer);

    // Read weights
    uint32_t numFloats = stream.ReadUint32();
    mWeightData.resize(numFloats);
    for (uint32_t i = 0; i < numFloats; i++) {
        mWeightData[i] = stream.ReadFloat();
    }

    // Read shared weights flag
    bool sharedWeights = stream.ReadBool();

    // Setup pointers
    SetupWeightPointers();

    if (sharedWeights) {
        mWcls = mTokenEmbedding;
    }
}

void TinyLLMAsset::SaveStream(Stream& stream, Platform platform)
{
    Asset::SaveStream(stream, platform);

    LogDebug("TinyLLMAsset::SaveStream config - dim=%d, hidden=%d, layers=%d, heads=%d, kv_heads=%d, vocab=%d, seq=%d",
        mConfig.dim, mConfig.hidden_dim, mConfig.n_layers, mConfig.n_heads,
        mConfig.n_kv_heads, mConfig.vocab_size, mConfig.seq_len);

    // Write config
    stream.WriteInt32(mConfig.dim);
    stream.WriteInt32(mConfig.hidden_dim);
    stream.WriteInt32(mConfig.n_layers);
    stream.WriteInt32(mConfig.n_heads);
    stream.WriteInt32(mConfig.n_kv_heads);
    stream.WriteInt32(mConfig.vocab_size);
    stream.WriteInt32(mConfig.seq_len);

    // Write tokenizer reference
    stream.WriteAsset(mTokenizer);

    // Write weights
    stream.WriteUint32((uint32_t)mWeightData.size());
    for (size_t i = 0; i < mWeightData.size(); i++) {
        stream.WriteFloat(mWeightData[i]);
    }

    // Write shared weights flag
    bool sharedWeights = (mWcls == mTokenEmbedding);
    stream.WriteBool(sharedWeights);
}

bool TinyLLMAsset::Import(const std::string& path, ImportOptions* options)
{
    bool success = Asset::Import(path, options);
    if (!success) {
        return false;
    }

#if EDITOR
    // Read the checkpoint file using Stream
    Stream checkpointStream;
    checkpointStream.ReadFile(path.c_str(), false);

    if (checkpointStream.GetSize() < 28) {
        LogError("TinyLLMAsset: Checkpoint file too small");
        return false;
    }

    // Read config header (28 bytes)
    struct Llama2cConfig {
        int dim;
        int hidden_dim;
        int n_layers;
        int n_heads;
        int n_kv_heads;
        int vocab_size;
        int seq_len;
    } l2config;

    l2config.dim = checkpointStream.ReadInt32();
    l2config.hidden_dim = checkpointStream.ReadInt32();
    l2config.n_layers = checkpointStream.ReadInt32();
    l2config.n_heads = checkpointStream.ReadInt32();
    l2config.n_kv_heads = checkpointStream.ReadInt32();
    l2config.vocab_size = checkpointStream.ReadInt32();
    l2config.seq_len = checkpointStream.ReadInt32();

    LogDebug("TinyLLMAsset::Import raw config - dim=%d, hidden=%d, layers=%d, heads=%d, kv_heads=%d, vocab=%d, seq=%d",
        l2config.dim, l2config.hidden_dim, l2config.n_layers, l2config.n_heads,
        l2config.n_kv_heads, l2config.vocab_size, l2config.seq_len);

    // Handle shared weights flag (negative vocab_size)
    bool sharedWeights = l2config.vocab_size > 0;
    l2config.vocab_size = abs(l2config.vocab_size);

    // Copy config
    mConfig.dim = l2config.dim;
    mConfig.hidden_dim = l2config.hidden_dim;
    mConfig.n_layers = l2config.n_layers;
    mConfig.n_heads = l2config.n_heads;
    mConfig.n_kv_heads = l2config.n_kv_heads;
    mConfig.vocab_size = l2config.vocab_size;
    mConfig.seq_len = l2config.seq_len;

    // Read all weights
    uint32_t weightsSize = checkpointStream.GetSize() - 28;
    uint32_t numFloats = weightsSize / sizeof(float);
    mWeightData.resize(numFloats);

    for (uint32_t i = 0; i < numFloats; i++) {
        mWeightData[i] = checkpointStream.ReadFloat();
    }

    // Setup weight pointers
    SetupWeightPointers();

    // Handle shared weights
    if (sharedWeights) {
        mWcls = mTokenEmbedding;
    }

    LogDebug("TinyLLMAsset: Imported model - dim=%d, layers=%d, vocab=%d",
             mConfig.dim, mConfig.n_layers, mConfig.vocab_size);
    LogDebug("TinyLLMAsset: Remember to assign a TinyLLMTokenizer asset!");

    success = true;
#endif

    return success;
}

void TinyLLMAsset::GatherProperties(std::vector<Property>& outProps)
{
    Asset::GatherProperties(outProps);

    // Tokenizer reference
    {
        SCOPED_CATEGORY("References");
        outProps.push_back(Property(DatumType::Asset, "Tokenizer", this, &mTokenizer, 1, HandlePropChange, int32_t(TinyLLMTokenizerAsset::GetStaticType())));
    }

#if EDITOR
    // Read-only config display
    static int32_t sDim, sHiddenDim, sLayers, sHeads, sKVHeads, sVocabSize, sSeqLen;
    sDim = mConfig.dim;
    sHiddenDim = mConfig.hidden_dim;
    sLayers = mConfig.n_layers;
    sHeads = mConfig.n_heads;
    sKVHeads = mConfig.n_kv_heads;
    sVocabSize = mConfig.vocab_size;
    sSeqLen = mConfig.seq_len;

    {
        SCOPED_CATEGORY("Model Config");
        outProps.push_back(Property(DatumType::Integer, "Dimension", this, &sDim));
        outProps.push_back(Property(DatumType::Integer, "Hidden Dimension", this, &sHiddenDim));
        outProps.push_back(Property(DatumType::Integer, "Layers", this, &sLayers));
        outProps.push_back(Property(DatumType::Integer, "Heads", this, &sHeads));
        outProps.push_back(Property(DatumType::Integer, "KV Heads", this, &sKVHeads));
        outProps.push_back(Property(DatumType::Integer, "Vocab Size", this, &sVocabSize));
        outProps.push_back(Property(DatumType::Integer, "Seq Length", this, &sSeqLen));
    }
#endif
}

glm::vec4 TinyLLMAsset::GetTypeColor()
{
    return glm::vec4(0.9f, 0.3f, 0.7f, 1.0f); // Pink/magenta for AI assets
}

const char* TinyLLMAsset::GetTypeName()
{
    return "TinyLLM";
}

const char* TinyLLMAsset::GetTypeImportExt()
{
    return ".bin";
}

void TinyLLMAsset::SetupWeightPointers()
{
    if (mWeightData.empty()) return;

    float* ptr = mWeightData.data();
    int head_size = mConfig.dim / mConfig.n_heads;
    int n_layers = mConfig.n_layers;
    int dim = mConfig.dim;
    int hidden_dim = mConfig.hidden_dim;
    int vocab_size = mConfig.vocab_size;
    int n_heads = mConfig.n_heads;
    int n_kv_heads = mConfig.n_kv_heads;

    mTokenEmbedding = ptr;
    ptr += vocab_size * dim;

    mRmsAttWeight = ptr;
    ptr += n_layers * dim;

    mWq = ptr;
    ptr += n_layers * dim * (n_heads * head_size);

    mWk = ptr;
    ptr += n_layers * dim * (n_kv_heads * head_size);

    mWv = ptr;
    ptr += n_layers * dim * (n_kv_heads * head_size);

    mWo = ptr;
    ptr += n_layers * (n_heads * head_size) * dim;

    mRmsFfnWeight = ptr;
    ptr += n_layers * dim;

    mW1 = ptr;
    ptr += n_layers * dim * hidden_dim;

    mW2 = ptr;
    ptr += n_layers * hidden_dim * dim;

    mW3 = ptr;
    ptr += n_layers * dim * hidden_dim;

    mRmsFinalWeight = ptr;
    ptr += dim;

    // Skip freq_cis (RoPE) - computed on the fly
    ptr += mConfig.seq_len * head_size / 2; // freq_cis_real
    ptr += mConfig.seq_len * head_size / 2; // freq_cis_imag

    mWcls = ptr; // May be overwritten if shared weights
}

void TinyLLMAsset::SetTokenizer(TinyLLMTokenizerAsset* tokenizer)
{
    mTokenizer = tokenizer;
}

TinyLLMTokenizerAsset* TinyLLMAsset::GetTokenizer() const
{
    return mTokenizer.Get<TinyLLMTokenizerAsset>();
}

void TinyLLMAsset::Encode(const char* text, bool addBos, bool addEos,
                          std::vector<int32_t>& outTokens)
{
    TinyLLMTokenizerAsset* tok = GetTokenizer();
    if (tok) {
        tok->Encode(text, addBos, addEos, outTokens);
    } else {
        outTokens.clear();
        LogError("TinyLLMAsset: No tokenizer assigned!");
    }
}

std::string TinyLLMAsset::Decode(int prevToken, int token)
{
    TinyLLMTokenizerAsset* tok = GetTokenizer();
    if (tok) {
        return tok->Decode(prevToken, token);
    } else {
        LogError("TinyLLMAsset: No tokenizer assigned!");
        return "";
    }
}

size_t TinyLLMAsset::GetWeightMemorySize() const
{
    return mWeightData.size() * sizeof(float);
}

size_t TinyLLMAsset::GetKVCacheMemorySize(int32_t maxSeqLen) const
{
    if (mConfig.n_heads == 0) return 0;
    int kv_dim = (mConfig.dim * mConfig.n_kv_heads) / mConfig.n_heads;
    return 2 * mConfig.n_layers * maxSeqLen * kv_dim * sizeof(float);
}
