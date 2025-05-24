#pragma once

#include "Utilities.h"
#include "EngineTypes.h"
#include "ScriptAutoReg.h"
#include <string>
#include <stdint.h>

int CreateClassMetatable(const char* className, const char* classFlag, const char* parentClassName);

class Object
{
public:
    virtual ~Object() = default;

    virtual const char* RuntimeName() const = 0;
    virtual const char* RuntimeParentName() const = 0;
    virtual RuntimeId InstanceRuntimeId() const = 0;

    static const char* ClassRuntimeName() { return "Object"; }

    virtual Object* QueryInterface(RuntimeId id) const
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

    virtual void GatherProperties(std::vector<Property>& props)
    {

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

    virtual bool Equals(const Object* rhs) const
    {
        return this == rhs;
    }
};

template <typename T>
T* Cast(Object* object)
{
    if (object && object->Is(T::ClassRuntimeId()))
    {
        return (T*)object;
    }

    return nullptr;
}

template <typename T, typename U>
SharedPtr<T> Cast(const SharedPtr<U>& object)
{
    if (object && object->Is(T::ClassRuntimeId()))
    {
        return PtrStaticCast<T>(object);
    }

    return nullptr;
}

template <typename T, typename U>
WeakPtr<T> Cast(const WeakPtr<U>& object)
{
    if (object && object->Is(T::ClassRuntimeId()))
    {
        return PtrStaticCast<T>(object);
    }

    return nullptr;
}

#define DECLARE_OBJECT(Type, ParentType)                                                                    \
    public:                                                                                                 \
        static const char* ClassRuntimeName() { return #Type; }                                             \
        virtual const char* RuntimeName() const override { return Type::ClassRuntimeName(); }               \
        virtual const char* RuntimeParentName() const override { return ParentType::ClassRuntimeName(); }   \
        static RuntimeId ClassRuntimeId() { return sRuntimeId; }                                            \
        virtual RuntimeId InstanceRuntimeId() const override { return Type::ClassRuntimeId(); }             \
        virtual Object* QueryInterface(RuntimeId id) const override                                         \
        {                                                                                                   \
            if (id == sRuntimeId)                                                                           \
                { return (Object*)this; }                                                                   \
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

#define DEFINE_OBJECT(Type) RuntimeId Type::sRuntimeId = reinterpret_cast<RuntimeId>(&Type::sRuntimeId);
