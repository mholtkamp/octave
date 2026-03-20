/**
 * @file TinyLLMTokenizerAsset.h
 * @brief Asset type for TinyLLM tokenizer vocabulary.
 */

#pragma once

#include "Asset.h"
#include <vector>
#include <string>

/**
 * @class TinyLLMTokenizerAsset
 * @brief Asset containing tokenizer vocabulary for TinyLLM models.
 */
class OCTAVE_API TinyLLMTokenizerAsset : public Asset
{
public:

    DECLARE_ASSET(TinyLLMTokenizerAsset, Asset);

    TinyLLMTokenizerAsset();
    virtual ~TinyLLMTokenizerAsset();

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

    // Tokenizer access
    const std::vector<std::string>& GetVocab() const { return mVocab; }
    const std::vector<float>& GetVocabScores() const { return mVocabScores; }
    uint32_t GetMaxTokenLength() const { return mMaxTokenLength; }
    int32_t GetVocabSize() const { return (int32_t)mVocab.size(); }

    // Tokenization
    struct TokenIndex {
        const char* str;
        int id;
    };
    int StrLookup(const char* str);
    void Encode(const char* text, bool addBos, bool addEos, std::vector<int32_t>& outTokens);
    std::string Decode(int prevToken, int token);

protected:

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

    void BuildSortedVocab();
    void InitBytePieces();

    // Tokenizer data
    std::vector<std::string> mVocab;
    std::vector<float> mVocabScores;
    uint32_t mMaxTokenLength = 0;

    // Sorted vocab for fast lookup (built lazily)
    std::vector<TokenIndex> mSortedVocab;
    bool mSortedVocabBuilt = false;

    // Byte-level fallback tokens
    char mBytePieces[512];
};
