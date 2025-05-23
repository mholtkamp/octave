#pragma once

#include "Utilities.h"
#include "EngineTypes.h"
#include "ScriptAutoReg.h"
#include "Factory.h"

#include <string>
#include <stdint.h>
#include <type_traits>

int CreateClassMetatable(const char* className, const char* classFlag, const char* parentClassName);

class Object
{

public:

    DECLARE_FACTORY_MANAGER(Object);
    DECLARE_FACTORY(Object, Object);

    virtual ~Object() = default;

    virtual const char* RuntimeName() const { return Object::ClassRuntimeName(); }
    virtual const char* RuntimeParentName() const { return ""; };
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

    template<typename T>
    static SharedPtr<T> New()
    {
        SharedPtr<T> ret = MakeShared<T>();
        ret->mSelf = ret;
        return ret;
    }

    template<typename T>
    static SharedPtr<T> New()
    {
        SharedPtr<T> ret = MakeShared<T>();
        ret->mSelf = ret;
        return ret;
    }

    inline const WeakPtr<Object>& GetSelfPtr()
    {
        return mSelf;
    }

protected:

    WeakPtr<Object> mSelf;
};

template <typename T>
T* Cast(Object* object)
{
    if (std::is_same_v<T, Object>())
    {
        return object;
    }
    else if (object && object->Is(T::ClassRuntimeId()))
    {
        return (T*)object;
    }

    return nullptr;
}

template <typename T, typename U>
SharedPtr<T> Cast(const SharedPtr<U>& object)
{
    if (std::is_same_v<T, Object>())
    {
        return object;
    }
    else if (object && object->Is(T::ClassRuntimeId()))
    {
        return PtrStaticCast<T>(object);
    }

    return nullptr;
}

template <typename T, typename U>
WeakPtr<T> Cast(const WeakPtr<U>& object)
{
    if (std::is_same_v<T, Object>())
    {
        return object;
    }
    else if (object && object->Is(T::ClassRuntimeId()))
    {
        return PtrStaticCast<T>(object);
    }

    return nullptr;
}

template<typename T>
inline SharedPtr<T> ResolvePtr(Object* obj)
{
    SharePtr<T> ptr = obj ? obj->GetSelfPtr().Lock() : nullptr;
    return Cast<T>(ptr);
}

template<typename T>
inline WeakPtr<T> ResolveWeakPtr(Object* obj)
{
    WeakPtr<T> ptr = obj ? obj->GetSelfPtr() : nullptr;
    return Cast<T>(ptr);
}

template<typename T>
SharedPtr<T> NewObject()
{
    return Object::New();
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
