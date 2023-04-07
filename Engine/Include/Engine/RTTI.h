#pragma once

#include "Utilities.h"
#include "EngineTypes.h"
#include "ScriptAutoReg.h"
#include <string>
#include <stdint.h>

int CreateClassMetatable(const char* className, const char* classFlag, const char* parentClassName);

class RTTI
{
public:
    virtual ~RTTI() = default;

    virtual const char* RuntimeName() const = 0;
    virtual const char* RuntimeParentName() const = 0;
    virtual RuntimeId InstanceRuntimeId() const = 0;

    static const char* ClassRuntimeName() { return "RTTI"; }

    virtual RTTI* QueryInterface(RuntimeId id) const
    {
        OCT_UNUSED(id);
        return nullptr;
    }

    virtual bool Is(RuntimeId id) const
    {
        OCT_UNUSED(id);
        return false;
    }

    virtual bool Is(const char* name) const
    {
        OCT_UNUSED(name);
        return false;
    }

    template <typename T>
    T* As() const
    {
        if (Is(T::ClassRuntimeId()))
        {
            return (T*)this;
        }

        return nullptr;
    }

    virtual bool Equals(const RTTI* rhs) const
    {
        return this == rhs;
    }
};

#define DECLARE_RTTI(Type, ParentType)                                                                      \
    public:                                                                                                 \
        static const char* ClassRuntimeName() { return #Type; }                                             \
        virtual const char* RuntimeName() const override { return Type::ClassRuntimeName(); }               \
        virtual const char* RuntimeParentName() const override { return ParentType::ClassRuntimeName(); }   \
        static RuntimeId ClassRuntimeId() { return sRuntimeId; }                                            \
        virtual RuntimeId InstanceRuntimeId() const override { return Type::ClassRuntimeId(); }             \
        virtual RTTI* QueryInterface(RuntimeId id) const override                                           \
        {                                                                                                   \
            if (id == sRuntimeId)                                                                           \
                { return (RTTI*)this; }                                                                     \
            else                                                                                            \
                { return ParentType::QueryInterface(id); }                                                  \
        }                                                                                                   \
        virtual bool Is(RuntimeId id) const override                                                        \
        {                                                                                                   \
            if (id == sRuntimeId)                                                                           \
                { return true; }                                                                            \
            else                                                                                            \
                { return ParentType::Is(id); }                                                              \
        }                                                                                                   \
        virtual bool Is(const char* name) const override                                                    \
        {                                                                                                   \
            if (strncmp(name, ClassRuntimeName(), 256) == 0)                                                \
                { return true; }                                                                            \
            else                                                                                            \
                { return ParentType::Is(name); }                                                            \
        }                                                                                                   \
    private:                                                                                                \
        static RuntimeId sRuntimeId;                                                                        \
    public:

#define DEFINE_RTTI(Type) RuntimeId Type::sRuntimeId = reinterpret_cast<RuntimeId>(&Type::sRuntimeId);
