#pragma once

#include "Datum.h"

#include <string>

class NetDatum : public Datum
{
public:
    NetDatum();
    NetDatum(
        DatumType type,
        Object* owner,
        void* data,
        uint32_t count = 1u,
        DatumChangeHandlerFP changeHandler = nullptr,
        bool alwaysReplicate = false);
    bool ShouldReplicate() const;
    void PostReplicate();
    
protected:
    virtual void Destroy() override;

    DatumData mPrevData = {};
    uint32_t mPrevCount = 0;
    bool mAlwaysReplicate = false;
};

class ScriptNetDatum : public NetDatum
{
public:
    ScriptNetDatum();
    ScriptNetDatum(
        const char* varName,
        DatumType type,
        Object* owner,
        void* data,
        uint32_t count = 1u,
        const char* onRepFuncName = nullptr,
        bool alwaysReplicate = false);

    std::string mVarName;
    std::string mOnRepFuncName;
};
