/**
 * @file TinyLLMTokenizerAsset.cpp
 * @brief Implementation of TinyLLMTokenizerAsset.
 */

#include "Assets/TinyLLMTokenizerAsset.h"
#include "Stream.h"
#include "Property.h"
#include "Log.h"

#include <cstring>
#include <cstdlib>
#include <cstdio>

FORCE_LINK_DEF(TinyLLMTokenizerAsset);
DEFINE_ASSET(TinyLLMTokenizerAsset);

// Comparison function for qsort/bsearch
static int CompareTokens(const void* a, const void* b)
{
    return strcmp(((TinyLLMTokenizerAsset::TokenIndex*)a)->str,
                  ((TinyLLMTokenizerAsset::TokenIndex*)b)->str);
}

bool TinyLLMTokenizerAsset::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    return HandleAssetPropChange(datum, index, newValue);
}

TinyLLMTokenizerAsset::TinyLLMTokenizerAsset()
{
    mType = TinyLLMTokenizerAsset::GetStaticType();
    InitBytePieces();
}

TinyLLMTokenizerAsset::~TinyLLMTokenizerAsset()
{
}

void TinyLLMTokenizerAsset::Create()
{
    Asset::Create();
}

void TinyLLMTokenizerAsset::Destroy()
{
    Asset::Destroy();
    mVocab.clear();
    mVocabScores.clear();
    mSortedVocab.clear();
    mSortedVocabBuilt = false;
}

void TinyLLMTokenizerAsset::InitBytePieces()
{
    for (int i = 0; i < 256; i++) {
        mBytePieces[i * 2] = (char)i;
        mBytePieces[i * 2 + 1] = '\0';
    }
}

void TinyLLMTokenizerAsset::LoadStream(Stream& stream, Platform platform)
{
    Asset::LoadStream(stream, platform);

    // Read tokenizer data
    mMaxTokenLength = stream.ReadUint32();
    uint32_t vocabSize = stream.ReadUint32();

    LogDebug("TinyLLMTokenizerAsset::LoadStream - maxTokenLen=%u, vocabSize=%u",
        mMaxTokenLength, vocabSize);

    if (vocabSize > 100000) {
        LogError("TinyLLMTokenizerAsset: Invalid vocab size %u, aborting load", vocabSize);
        return;
    }

    mVocab.resize(vocabSize);
    mVocabScores.resize(vocabSize);

    for (uint32_t i = 0; i < vocabSize; i++) {
        stream.ReadString(mVocab[i]);
        mVocabScores[i] = stream.ReadFloat();
    }

    LogDebug("TinyLLMTokenizerAsset: Loaded %u tokens", (uint32_t)mVocab.size());
    mSortedVocabBuilt = false;
}

void TinyLLMTokenizerAsset::SaveStream(Stream& stream, Platform platform)
{
    Asset::SaveStream(stream, platform);

    // Write tokenizer data
    stream.WriteUint32(mMaxTokenLength);
    stream.WriteUint32((uint32_t)mVocab.size());

    for (size_t i = 0; i < mVocab.size(); i++) {
        stream.WriteString(mVocab[i]);
        stream.WriteFloat(mVocabScores[i]);
    }
}

bool TinyLLMTokenizerAsset::Import(const std::string& path, ImportOptions* options)
{
    bool success = Asset::Import(path, options);
    if (!success) {
        return false;
    }

#if EDITOR
    // Read the tokenizer file
    Stream tokStream;
    if (!tokStream.ReadFile(path.c_str(), false)) {
        LogError("TinyLLMTokenizerAsset: Failed to read file %s", path.c_str());
        return false;
    }

    if (tokStream.GetSize() < 4) {
        LogError("TinyLLMTokenizerAsset: File too small");
        return false;
    }

    // Read max token length
    mMaxTokenLength = tokStream.ReadUint32();

    // We need to know vocab size - read until EOF
    // The format is: [score:float][len:int][string:chars]*
    mVocab.clear();
    mVocabScores.clear();

    while (tokStream.GetPos() < tokStream.GetSize()) {
        float score = tokStream.ReadFloat();
        int32_t len = tokStream.ReadInt32();

        if (len < 0 || len > 1024) {
            // Sanity check failed
            break;
        }

        std::string token;
        token.resize(len);
        for (int32_t j = 0; j < len; j++) {
            token[j] = (char)tokStream.ReadUint8();
        }

        mVocab.push_back(token);
        mVocabScores.push_back(score);
    }

    LogDebug("TinyLLMTokenizerAsset: Imported %d tokens, max length %d",
             (int)mVocab.size(), mMaxTokenLength);

    success = true;
#endif

    return success;
}

void TinyLLMTokenizerAsset::GatherProperties(std::vector<Property>& outProps)
{
    Asset::GatherProperties(outProps);

#if EDITOR
    static int32_t sVocabSize;
    static uint32_t sMaxTokenLen;
    sVocabSize = (int32_t)mVocab.size();
    sMaxTokenLen = mMaxTokenLength;

    SCOPED_CATEGORY("Tokenizer");
    outProps.push_back(Property(DatumType::Integer, "Vocab Size", this, &sVocabSize));
    outProps.push_back(Property(DatumType::Integer, "Max Token Length", this, &sMaxTokenLen));
#endif
}

glm::vec4 TinyLLMTokenizerAsset::GetTypeColor()
{
    return glm::vec4(0.7f, 0.5f, 0.9f, 1.0f); // Light purple
}

const char* TinyLLMTokenizerAsset::GetTypeName()
{
    return "TinyLLMTokenizer";
}

const char* TinyLLMTokenizerAsset::GetTypeImportExt()
{
    return ".bin";
}

void TinyLLMTokenizerAsset::BuildSortedVocab()
{
    if (mSortedVocabBuilt) return;

    mSortedVocab.resize(mVocab.size());
    for (size_t i = 0; i < mVocab.size(); i++) {
        mSortedVocab[i].str = mVocab[i].c_str();
        mSortedVocab[i].id = (int)i;
    }

    qsort(mSortedVocab.data(), mSortedVocab.size(),
          sizeof(TokenIndex), CompareTokens);

    mSortedVocabBuilt = true;
}

int TinyLLMTokenizerAsset::StrLookup(const char* str)
{
    if (!mSortedVocabBuilt) {
        BuildSortedVocab();
    }

    TokenIndex key;
    key.str = str;
    key.id = 0;

    TokenIndex* res = (TokenIndex*)bsearch(&key, mSortedVocab.data(),
                                            mSortedVocab.size(),
                                            sizeof(TokenIndex), CompareTokens);
    return res ? res->id : -1;
}

void TinyLLMTokenizerAsset::Encode(const char* text, bool addBos, bool addEos,
                                    std::vector<int32_t>& outTokens)
{
    if (!text) return;

    LogDebug("TinyLLMTokenizerAsset::Encode - vocabSize=%u, maxTokenLen=%u, text='%.20s...'",
        (uint32_t)mVocab.size(), mMaxTokenLength, text);

    if (mVocab.empty()) {
        LogError("TinyLLMTokenizerAsset::Encode - vocab is empty!");
        outTokens.clear();
        return;
    }

    BuildSortedVocab();

    size_t bufSize = mMaxTokenLength * 2 + 3;
    std::vector<char> strBuffer(bufSize);
    size_t strLen = 0;

    outTokens.clear();

    if (addBos) {
        outTokens.push_back(1);
    }

    // Add dummy prefix (sentencepiece convention)
    if (text[0] != '\0') {
        int dummyPrefix = StrLookup(" ");
        if (dummyPrefix >= 0) {
            outTokens.push_back(dummyPrefix);
        }
    }

    // Process UTF-8 byte sequence
    for (const char* c = text; *c != '\0'; c++) {
        if ((*c & 0xC0) != 0x80) {
            strLen = 0;
        }

        strBuffer[strLen++] = *c;
        strBuffer[strLen] = '\0';

        if ((*(c + 1) & 0xC0) == 0x80 && strLen < 4) {
            continue;
        }

        int id = StrLookup(strBuffer.data());
        if (id != -1) {
            outTokens.push_back(id);
        } else {
            for (size_t i = 0; i < strLen; i++) {
                outTokens.push_back((unsigned char)strBuffer[i] + 3);
            }
        }
        strLen = 0;
    }

    // BPE merging
    while (true) {
        float bestScore = -1e10f;
        int bestId = -1;
        int bestIdx = -1;

        for (size_t i = 0; i + 1 < outTokens.size(); i++) {
            snprintf(strBuffer.data(), bufSize, "%s%s",
                     mVocab[outTokens[i]].c_str(),
                     mVocab[outTokens[i + 1]].c_str());
            int id = StrLookup(strBuffer.data());
            if (id != -1 && mVocabScores[id] > bestScore) {
                bestScore = mVocabScores[id];
                bestId = id;
                bestIdx = (int)i;
            }
        }

        if (bestIdx == -1) {
            break;
        }

        outTokens[bestIdx] = bestId;
        outTokens.erase(outTokens.begin() + bestIdx + 1);
    }

    if (addEos) {
        outTokens.push_back(2);
    }
}

std::string TinyLLMTokenizerAsset::Decode(int prevToken, int token)
{
    if (token < 0 || token >= (int)mVocab.size()) {
        return "";
    }

    const char* piece = mVocab[token].c_str();

    if (prevToken == 1 && piece[0] == ' ') {
        piece++;
    }

    unsigned char byteVal;
    if (sscanf(piece, "<0x%02hhX>", &byteVal) == 1) {
        return std::string(1, (char)byteVal);
    }

    return piece;
}
