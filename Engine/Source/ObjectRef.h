#pragma once

#include "Actor.h"
#include "Components/Component.h"
#include <vector>
#include <stdint.h>

template<typename T>
class ObjectRef
{
public:
    ObjectRef()
    {

    }

    ObjectRef(T* object)
    {
        Set(object);
    }

    ObjectRef(const ObjectRef<T>& src)
    {
        Set(src.mObject);
    }

    ~ObjectRef()
    {
        if (mRegistered)
        {
            RemoveLiveRef(this);
        }
    }

    ObjectRef& operator=(const ObjectRef<T>& src)
    {
        return operator=(src.mObject);
    }

    ObjectRef& operator=(const T* srcObject)
    {
        Set(const_cast<T*>(srcObject));
        return *this;
    }

    bool operator==(const ObjectRef& other) const
    {
        return Get() == other.Get();
    }

    bool operator!=(const ObjectRef& other) const
    {
        return !operator==(other);
    }

    bool operator==(const T* other) const
    {
        return Get() == other;
    }

    bool operator!=(const T* other) const
    {
        return !operator==(other);
    }

    void Set(T* object)
    {
        mObject = object;

        if (object != nullptr &&
            !mRegistered)
        {
            AddLiveRef(this);
        }
    }

    T* Get() const
    {
        return mObject;
    }

    T*& GetRef()
    {
        return mObject;
    }

    // For getting a subclass. T must support RTTI
    template<typename S>
    S* Get() const
    {
        OCT_ASSERT(!mObject || mObject->Is(S::ClassRuntimeId()));
        return static_cast<S*>(Get());
    }

    static void EraseReferencesToObject(T* object)
    {
        for (uint32_t i = 0; i < sLiveRefs.size(); ++i)
        {
            if (sLiveRefs[i]->Get() == object)
            {
                sLiveRefs[i]->Set(nullptr);
            }
        }
    }

private:

    static void AddLiveRef(ObjectRef* ref)
    {
        sLiveRefs.push_back(ref);
        ref->mRegistered = true;
    }

    static void RemoveLiveRef(ObjectRef* ref)
    {
        for (uint32_t i = 0; i < sLiveRefs.size(); ++i)
        {
            if (sLiveRefs[i] == ref)
            {
                sLiveRefs.erase(sLiveRefs.begin() + i);
                ref->mRegistered = false;
                break;
            }
        }
    }

    static std::vector<ObjectRef*> sLiveRefs;

    T* mObject = nullptr;
    bool mRegistered = false;
};

typedef ObjectRef<Actor> ActorRef;
typedef ObjectRef<Component> ComponentRef;

template <typename T>
std::vector<ObjectRef<T>*> ObjectRef<T>::sLiveRefs;

