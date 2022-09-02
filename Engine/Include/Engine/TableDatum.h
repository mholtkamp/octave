#pragma once

#include "Datum.h"

class TableDatum : public Datum
{
public:

    TableDatum();

    TableDatum(const TableDatum& src);
    TableDatum& operator=(const TableDatum& src);

    virtual void ReadStream(Stream& stream, bool external) override;
    virtual void WriteStream(Stream& stream) const override;
    virtual uint32_t GetSerializationSize() const override;
    virtual bool IsTableDatum() const override;
    virtual void DeepCopy(const Datum& src, bool forceInternalStorage) override;

    const char* GetStringKey() const;
    int32_t GetIntegerKey() const;
    bool IsStringKey() const;
    bool IsIntegerKey() const;

    void SetStringKey(const char* stringKey);
    void SetIntegerKey(int32_t integerKey);

public:

    // Only string and integer keys are supported for now.
    // Theortically... you could use a Datum as key so you could support any type.
    // But let's not go crazy and allocate unnecessary data.
    // I think realistically, most of the time strings or integers are used as keys,
    // but I'm not really a Lua programmer.
    std::string mStringKey;
    int32_t mIntegerKey = 0;
};

