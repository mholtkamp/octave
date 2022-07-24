#pragma once

#include "Datum.h"

enum class ByteExtra
{
    None,
    FlagWidget,
    ExclusiveFlagWidget
};

class Property : public Datum
{
public:
    Property();
    Property(DatumType type,
        const std::string& name,
        void* owner,
        void* data,
        uint32_t count = 1,
        DatumChangeHandlerFP changeHandler = nullptr,
        int32_t extra = 0,
        int32_t enumCount = 0,
        const char** enumStrings = nullptr);

    Property(const Property& src);

    Property& operator=(const Property& src);


    virtual void ReadStream(Stream& stream, bool external) override;
    virtual void WriteStream(Stream& stream) const override;
    virtual uint32_t GetSerializationSize() const override;

    virtual bool IsProperty() const override;
    virtual void DeepCopy(const Datum& src, bool forceInternalStorage) override;

    void PushBackVector(void* value = nullptr);
    void EraseVector(uint32_t index);
    Property& MakeVector(uint8_t minCount = 0, uint8_t maxCount = 255);
    bool IsVector() const;
    bool IsArray() const;

protected:

    virtual void Reset() override;

public:
    std::string mName;
    int32_t mExtra = 0;
    int32_t mEnumCount = 0;
    const char** mEnumStrings = nullptr;
    void* mVector = nullptr;
    uint8_t mMinCount = 0;
    uint8_t mMaxCount = 255;
};

struct PropertyOverride
{
    int32_t mIndex = -1;
    Property mProperty;
};
