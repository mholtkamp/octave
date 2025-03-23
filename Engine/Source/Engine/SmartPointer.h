#pragma once

#include <vector>
#include <stdint.h>
#include "Assertion.h"

class Node;

template<typename T>
struct RefCount
{
    typedef void(*DeleterFP)(T*);

    int32_t mSharedCount = 0;
    int32_t mWeakCount = 0;
    DeleterFP mDeleter = nullptr;
};

template<typename T>
class SharedPtr
{
public:

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

    SharedPtr(std::nullptr_t)
    {
        Clear();
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

        return *this;
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

    operator bool() const
    {
        return IsValid();
    }

    void Set(T* pointer, RefCount<T>* refCount)
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
                mRefCount = new RefCount<T>();
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
                // In the case where Node maintains a weak pointer to itself,
                // it's possible that deleting the pointer will trigger a weak
                // pointer cleanup which will destroy the mRefCount data.
                // So temporarily up the weak count so that doesn't happen.
                mRefCount->mWeakCount++;

                if (mRefCount->mDeleter)
                {
                    mRefCount->mDeleter(mPointer);
                }

                delete mPointer;

                mRefCount->mWeakCount--;
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

    void SetDeleter(typename RefCount<T>::DeleterFP deleteFunc)
    {
        if (IsValid())
        {
            mRefCount->mDeleter = deleteFunc;
        }
    }

    void Reset()
    {
        Clear();
    }

    T* Get() const
    {
        return mPointer;
    }

    T*& GetRef()
    {
        return mPointer;
    }

    RefCount<T>* GetRefCount() const
    {
        return mRefCount;
    }

    int32_t GetUseCount() const
    {
        return mRefCount ? mRefCount->mSharedCount : 0;
    }

    int32_t GetSharedCount() const
    {
        return GetUseCount();
    }

    int32_t GetWeakCount() const
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
    RefCount<T>* mRefCount = nullptr;
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
        Set(src.Get(), src.GetRefCount());
    }

    WeakPtr(std::nullptr_t)
    {
        Clear();
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
        Set(src.Get(), src.GetRefCount());
        return *this;
    }

    WeakPtr<T>& operator=(WeakPtr<T>&& src)
    {
        Clear();

        mPointer = src.mPointer;
        mRefCount = src.mRefCount;

        src.mPointer = nullptr;
        src.mRefCount = nullptr;

        return *this;
    }

    bool operator==(const SharedPtr<T>& other) const
    {
        return mPointer == other.Get();
    }

    bool operator!=(const SharedPtr<T>& other) const
    {
        return !operator==(other);
    }

    bool operator==(const WeakPtr<T>& other) const
    {
        return mPointer == other.mPointer;
    }

    bool operator!=(const WeakPtr<T>& other) const
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

    operator bool() const
    {
        return IsValid();
    }

    void Set(T* pointer, RefCount<T>* refCount)
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

    SharedPtr<T> Lock() const
    {
        SharedPtr<T> ret;

        if (IsValid())
        {
            ret.Set(mPointer, mRefCount);
        }

        return ret;
    }

    bool IsValid() const
    {
        return (mPointer != nullptr && mRefCount != nullptr && mRefCount->mSharedCount > 0);
    }

    void Reset()
    {
        Clear();
    }

    T* Get() const
    {
        return IsValid() ? mPointer : nullptr;
    }

    T*& GetRef()
    {
        return IsValid() ? mPointer : nullptr;
    }

    RefCount<T>* GetRefCount() const
    {
        return mRefCount;
    }

    int32_t GetUseCount() const
    {
        return mRefCount ? mRefCount->mSharedCount : 0;
    }

    int32_t GetSharedCount() const
    {
        return GetUseCount();
    }

    int32_t GetWeakCount() const
    {
        return mRefCount ? mRefCount->mWeakCount : 0;
    }

    // For getting a subclass. T must inherit from Object.
    template<typename S>
    S* Get() const
    {
        if (!IsValid())
        {
            return nullptr;
        }

        OCT_ASSERT(!mPointer || mPointer->Is(S::ClassRuntimeId()));
        return static_cast<S*>(Get());
    }

private:

    T* mPointer = nullptr;
    RefCount<T>* mRefCount = nullptr;
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

template<typename T, typename U>
SharedPtr<T> PtrStaticCast(const SharedPtr<U>& src)
{
    T* pointer = static_cast<T*>(src.Get());
    SharedPtr<T> dst;
    dst.Set(pointer, (RefCount<T>*)src.GetRefCount());
    return dst;
}

template<typename T, typename U>
SharedPtr<T> PtrDynamicCast(const SharedPtr<U>& src)
{
    U* srcPointer = src.Get();
    T* dstPointer = nullptr;

    if (srcPointer)
    {
        dstPointer = srcPointer->template As<T>();
    }

    if (dstPointer)
    {
        SharedPtr<T> dst;
        dst.Set(dstPointer, (RefCount<T>*)src.GetRefCount());
        return dst;
    }
    else
    {
        return SharedPtr<T>();
    }

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
