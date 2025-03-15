#pragma once

#include <vector>
#include <stdint.h>
#include "Assertion.h"

class Node;

struct RefCount
{
    int32_t mSharedCount = 0;
    int32_t mWeakCount = 0;
};

template<typename T>
class SharedPtr
{
public:

    friend class WeakPtr<T>;

    SharedPtr()
    {

    }

    SharedPtr(const SharedPtr<T>& src)
    {
        Set(src.mPointer, src.mRefCount);
    }

    SharedPtr(SharedPtr<T>&& src)
    {
        mPointer = src.mPointer;
        mRefCount = src.mRefCount;

        src.mPointer = nullptr;
        src.mRefCount = nullptr;
    }

    ~SharedPtr()
    {
        Clear();
    }

    SharedPtr<T>& operator=(const SharedPtr<T>& src)
    {
        Set(src.mPointer, src.mRefCount);
        return *this;
    }

    SharedPtr<T>& operator=(SharedPtr<T>&& src)
    {
        Clear();

        mPointer = src.mPointer;
        mRefCount = src.mRefCount;

        src.mPointer = nullptr;
        src.mRefCount = nullptr;
    }

    bool operator==(const SharedPtr& other) const
    {
        return mPointer == other.mPointer;
    }

    bool operator!=(const SharedPtr& other) const
    {
        return !operator==(other);
    }

    bool operator==(const T* other) const
    {
        return mPointer == other;
    }

    bool operator!=(const T* other) const
    {
        return !operator==(other);
    }

    T* operator->() const
    {
        return mPointer;
    }

    T& operator*() const
    {
        return *mPointer;
    }

    void Set(T* pointer, RefCount* refCount)
    {
        if (mPointer != pointer)
        {
            // If we have a pointer, we must have a refcount
            // If we have no pointer, then we shouldn't have a refcount
            OCT_ASSERT((mPointer != nullptr) == (mRefCount != nullptr));

            Clear();

            mPointer = pointer;
            mRefCount = refCount;

            if (mPointer != nullptr && mRefCount == nullptr)
            {
                // Initialize ref count
                mRefCount = new RefCount();
            }

            if (mRefCount != nullptr)
            {
                mRefCount->mSharedCount++;
            }
        }
        else
        {
            // Two shared pointers initialized independently to same pointer?
            OCT_ASSERT(mRefCount == refCount);
        }
    }

    void Clear()
    {
        if (mPointer != nullptr)
        {
            OCT_ASSERT(mRefCount);
            OCT_ASSERT(mRefCount->mSharedCount > 0);
            mRefCount->mSharedCount--;

            if (mRefCount->mSharedCount <= 0)
            {
                delete mPointer;
            }

            if (mRefCount->mSharedCount <= 0 &&
                mRefCount->mWeakCount <= 0)
            {
                delete mRefCount;
            }

            mPointer = nullptr;
            mRefCount = nullptr;
        }
        else
        {
            // If we have no assigned pointer, we should have no ref count.
            OCT_ASSERT(mRefCount == nullptr);
        }
    }

    void Reset()
    {
        Clear();
    }

    T* Get() const
    {
        return mObject;
    }

    T*& GetRef()
    {
        return mObject;
    }

    int32_t GetUseCount()
    {
        return mRefCount ? mRefCount->mSharedCount : 0;
    }

    int32_t GetSharedCount()
    {
        return GetUseCount();
    }

    int32_t GetWeakCount()
    {
        return mRefCount ? mRefCount->mWeakCount : 0;
    }

    // For getting a subclass. T must inherit from Object.
    template<typename S>
    S* Get() const
    {
        OCT_ASSERT(!mPointer || mPointer->Is(S::ClassRuntimeId()));
        return static_cast<S*>(Get());
    }

    bool IsValid() const
    {
        return (mPointer != nullptr && mRefCount != nullptr);
    }

private:

    T* mPointer = nullptr;
    RefCount* mRefCount = nullptr;
};

template<typename T>
class WeakPtr
{
public:
    WeakPtr()
    {

    }

    WeakPtr(const WeakPtr<T>& src)
    {
        Set(src.mPointer, src.mRefCount);
    }

    WeakPtr(const SharedPtr<T>& src)
    {
        Set(src.mPointer, src.mRefCount);
    }

    WeakPtr(WeakPtr<T>&& src)
    {
        mPointer = src.mPointer;
        mRefCount = src.mRefCount;

        src.mPointer = nullptr;
        src.mRefCount = nullptr;
    }

    ~WeakPtr()
    {
        Clear();
    }

    WeakPtr<T>& operator=(const WeakPtr<T>& src)
    {
        Set(src.mPointer, src.mRefCount);
        return *this;
    }

    WeakPtr<T>& operator=(const SharedPtr<T>& src)
    {
        Set(src.mPointer, src.mRefCount);
    }

    WeakPtr<T>& operator=(WeakPtr<T>&& src)
    {
        Clear();

        mPointer = src.mPointer;
        mRefCount = src.mRefCount;

        src.mPointer = nullptr;
        src.mRefCount = nullptr;
    }

    bool operator==(const SharedPtr& other) const
    {
        return mPointer == other.mPointer;
    }

    bool operator!=(const SharedPtr& other) const
    {
        return !operator==(other);
    }

    bool operator==(const WeakPtr& other) const
    {
        return mPointer == other.mPointer;
    }

    bool operator!=(const WeakPtr& other) const
    {
        return !operator==(other);
    }

    bool operator==(const T* other) const
    {
        return mPointer == other;
    }

    bool operator!=(const T* other) const
    {
        return !operator==(other);
    }

    T* operator->() const
    {
        return mPointer;
    }

    T& operator*() const
    {
        return *mPointer;
    }

    void Set(T* pointer, RefCount* refCount)
    {
        // We must always receive a valid refcount if given a valid pointer.
        OCT_ASSERT((pointer != nullptr) == (refCount != nullptr));

        if (mPointer != pointer)
        {
            // If we have a pointer, we must have a refcount
            // If we have no pointer, then we shouldn't have a refcount
            OCT_ASSERT((mPointer != nullptr) == (mRefCount != nullptr));

            Clear();

            mPointer = pointer;
            mRefCount = refCount;

            if (mRefCount != nullptr)
            {
                mRefCount->mWeakCount++;
            }
        }
        else
        {
            // Two shared pointers initialized independently to same pointer?
            OCT_ASSERT(mRefCount == refCount);
        }
    }

    void Clear()
    {
        if (mPointer != nullptr)
        {
            OCT_ASSERT(mRefCount);
            OCT_ASSERT(mRefCount->mWeakCount > 0);
            mRefCount->mWeakCount--;

            if (mRefCount->mSharedCount <= 0 &&
                mRefCount->mWeakCount <= 0)
            {
                delete mRefCount;
            }

            mPointer = nullptr;
            mRefCount = nullptr;
        }
        else
        {
            // If we have no assigned pointer, we should have no ref count.
            OCT_ASSERT(mRefCount == nullptr);
        }
    }

    TSharedPtr<T> Lock() const
    {
        TSharedPtr<T> ret;

        if (IsValid())
        {
            ret.Set(mPointer, mRefCount);
        }

        return ret;
    }

    bool IsValid() const
    {
        return (mPointer != nullptr && mRefCount != nullptr);
    }

private:

    T* mPointer = nullptr;
    RefCount* mRefCount = nullptr;
};

// TODO: Perfect forwarding
template<typename T>
SharedPtr<T> MakeShared()
{
    SharedPtr<T> ret;
    T* newPtr = new T();
    ret.Set(newPtr, nullptr);
    return ret;
}

typedef SharedPtr<Node> NodeRef;
typedef SharedPtr<Node> NodePtr;
typedef WeakPtr<Node> NodePtrWeak;

namespace std
{
    template<typename T>
    struct hash<SharedPtr<T> >
    {
        size_t operator()(const SharedPtr<T>& k) const
        {
            return std::hash<T*>{}(k.Get());
        }
    };

    template<typename T>
    struct hash<WeakPtr<T> >
    {
        size_t operator()(const WeakPtr<T>& k) const
        {
            return std::hash<T*>{}(k.Get());
        }
    };
}
