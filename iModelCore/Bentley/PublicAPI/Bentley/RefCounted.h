/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/RefCounted.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "BentleyAllocator.h"
#include "BeAtomic.h"
#include "BeAssert.h"

// For std::move on Android.
#include <utility>

BEGIN_BENTLEY_NAMESPACE

/*=================================================================================**//**
* Interface to be adopted by a class the implements the reference-counting pattern.
* @bsiclass                                                     Keith.Bentley   09/07
+===============+===============+===============+===============+===============+======*/
struct  IRefCounted
    {
protected:
    virtual ~IRefCounted() {}         // force virtual destructor for subclasses
    DEFINE_BENTLEY_NEW_DELETE_OPERATORS

public:
    virtual uint32_t AddRef() const = 0;
    virtual uint32_t Release() const = 0;
    };

// define this macro to catch calls to Release without matching calls to AddRef
#define DEBUG_REF_COUNTING
#if defined DEBUG_REF_COUNTING
    # define REFCOUNT_RELASE_CHECK(val) BeAssert(0!=val)
    #else
    # define REFCOUNT_RELASE_CHECK(val)
#endif

// You can use this macro to implement IRefCounted directly on your class.
// You should normally make your class non-copyable. If not, you must define a copy constructor and assignment operator, as shown in the RefCounted template below.
// Warning: the Release method implementation decreases the value of m_refCount atomically and must not use any member variables after that - other
// threads see the decreased value and can potentially delete the object.
// Memory ordering notes:
//  AddRef() uses std::memory_order_relaxed because the only requirement is atomicity of the ref-count - ordering/visibility of side effects not relevant
//  Release() conceptually uses acquire-release in order to ensure that any modifications made to this object by one releasing thread are visible to another
//  releasing thread which also ends up deleting the object.
//  We avoid the potentially-expensive acquire semantics in the common case in which the object is not deleted by separating it out into an atomic_thread_fence
#define DEFINE_BENTLEY_REF_COUNTED_MEMBERS              \
private:                                                \
    mutable BeAtomic<uint32_t> m_refCount;              \
public:                                                 \
    DEFINE_BENTLEY_NEW_DELETE_OPERATORS                 \
    uint32_t AddRef() const {return m_refCount.IncrementAtomicPreRelaxed();} \
    uint32_t Release() const                            \
        {                                               \
        uint32_t countWas = m_refCount.DecrementAtomicPostRelease(); REFCOUNT_RELASE_CHECK(countWas); \
        if (1 == countWas)                              \
            {                                           \
            BE_ATOMIC_THREAD_FENCE_ACQUIRE;             \
            delete this;                                \
            }                                           \
        return countWas-1;                              \
        ;}

#define DEFINE_REF_COUNTED_PTR(_sname_) typedef RefCountedPtr<_sname_> _sname_##Ptr; typedef RefCountedCPtr<_sname_> _sname_##CPtr;

/*=================================================================================**//**
* Template to simplify the task of writing a class that implements the reference-counting pattern.
* This template contains a complete implementation of the reference-counting pattern.
* If a class inherits from an instantiation of this template, the class inherits that implementation and becomes reference-counted.
* Example:
* \code
struct MyClass : RefCounted<IRefCounted> {...};
\endcode
* Or, RefCounted can be used to define a class that implements some interface, where that interface inherits from IRefCounted. For example:
* \code
struct ISomeInterface : IRefCounted {...};
struct MyClass : RefCounted<ISomeInterface> {...};
\endcode
* @bsiclass                                                     Keith.Bentley   09/07
+===============+===============+===============+===============+===============+======*/
template <class Base> class RefCounted : public Base
    {
    DEFINE_BENTLEY_REF_COUNTED_MEMBERS

protected:
    virtual ~RefCounted() {}    // force virtual destructor for subclasses

public:
    RefCounted() : m_refCount(0) {}
    RefCounted(RefCounted const& rhs) {m_refCount.store(0);} // Initialize my ref count to zero. Adopting rhs' data does not add a *reference* to me.
    RefCounted& operator=(RefCounted const& rhs) {if (this != &rhs) {Base::operator=(rhs);} return *this;} // NB: Preserve my ref count! Copying rhs' data to me does not add a *reference* to me.
    uint32_t GetRefCount() const {return m_refCount.load();}
    };

/*=================================================================================**//**
* Concrete class that can be used to implement the reference-counting pattern.
* Inheriting from this class is equivalent to inheriting from RefCounted<IRefCounted>.
* @bsiclass                                                     Keith.Bentley   09/07
+===============+===============+===============+===============+===============+======*/
class RefCountedBase : public RefCounted<IRefCounted>
    {
    };

//=======================================================================================
//! A shared pointer template for reference-counted objects.
//! Type @b T must have functions named AddRef and Release with signatures that conform to the reference-counting pattern.
//=======================================================================================
template<class T> class RefCountedPtr
{
private:
    T* m_p;

public:
    typedef T element_type;

    RefCountedPtr() : m_p(nullptr) {}
    ~RefCountedPtr() {if (m_p != nullptr) m_p->Release();}
    RefCountedPtr(T* p, bool add_ref = true) : m_p(p) {if (m_p != nullptr && add_ref) m_p->AddRef();}
    template<class U> RefCountedPtr(RefCountedPtr<U> const & rhs) : m_p(rhs.get()) {if (m_p != nullptr) m_p->AddRef();}
    RefCountedPtr(RefCountedPtr const& rhs): m_p(rhs.m_p) {if (m_p != nullptr) m_p->AddRef();}
    RefCountedPtr(RefCountedPtr&& rhs) : m_p(std::move(rhs.m_p)){rhs.m_p = nullptr;}
    RefCountedPtr& operator=(RefCountedPtr&& rhs) {RefCountedPtr(std::move(rhs)).swap(*this); return *this;}

    template<class U> RefCountedPtr & operator=(RefCountedPtr<U> const & rhs) {RefCountedPtr(rhs).swap(*this); return *this;}
    bool operator== (RefCountedPtr<T> const& rhs) const {return m_p == rhs.m_p;}
    template<class U> bool operator== (RefCountedPtr<U> const& rhs) const {return m_p == rhs.get();}
    bool operator!= (RefCountedPtr<T> const& rhs) const {return m_p != rhs.m_p;}
    template<class U> bool operator!= (RefCountedPtr<U> const& rhs) const {return m_p != rhs.get();}

    RefCountedPtr& operator=(RefCountedPtr const & rhs) {RefCountedPtr(rhs).swap(*this); return *this;}
    RefCountedPtr& operator=(T* rhs) {RefCountedPtr(rhs).swap(*this); return *this;}
    T* get() const {return m_p;}
    T& operator*() const {return *m_p;}
    T* operator->() const {return m_p;}

    bool IsValid() const {return nullptr != m_p;} //!< Return true if the ref-counted object has a valid (non-NULL) internal object.
    bool IsNull() const  {return !IsValid();}     //!< Return true if the ref-counted object has a invalid (NULL) internal object.
    bool Equals(T const* other) const {return other == get();}
    bool Equals(RefCountedPtr const& other) const {return other.get() == get();}
    void swap(RefCountedPtr& rhs) {T* tmp = m_p; m_p = rhs.m_p; rhs.m_p = tmp;} //!< Swap the internal objects pointed to by two smart pointers.
};

//=======================================================================================
//! A shared pointer template for a reference-counted pointer to a const object.
//! Type @b T must have functions named AddRef and Release with signatures that conform to the reference-counting pattern.
//=======================================================================================
template<class T> class RefCountedCPtr
{
private:
    T const* m_p;

public:
    typedef T element_type;

    RefCountedCPtr() : m_p(nullptr) {}
    ~RefCountedCPtr() {if (m_p != nullptr) m_p->Release();}
    RefCountedCPtr(T const* p, bool add_ref = true) : m_p(p){if (m_p != nullptr && add_ref) m_p->AddRef();}
    template<class U> RefCountedCPtr(RefCountedCPtr<U> const & rhs) : m_p(rhs.get()) {if (m_p != nullptr) m_p->AddRef();}
    RefCountedCPtr(RefCountedCPtr const& rhs) : m_p(rhs.m_p) {if (m_p != nullptr) m_p->AddRef();}
    RefCountedCPtr(RefCountedPtr<T> const& rhs) : m_p(rhs.get()) {if (m_p != nullptr) m_p->AddRef();}
    RefCountedCPtr(RefCountedCPtr&& rhs) : m_p(std::move(rhs.m_p)) {rhs.m_p = nullptr;}
    RefCountedCPtr& operator=(RefCountedCPtr&& rhs) {RefCountedCPtr(std::move(rhs)).swap(*this); return *this;}

    template<class U> RefCountedCPtr& operator=(RefCountedCPtr<U> const& rhs){RefCountedCPtr(rhs).swap(*this); return *this;}
    bool operator== (RefCountedCPtr<T> const& rhs) const {return m_p == rhs.m_p;}
    template<class U> bool operator== (RefCountedCPtr<U> const& rhs) const {return m_p == rhs.get();}
    bool operator!= (RefCountedCPtr<T> const& rhs) const {return m_p != rhs.m_p;}
    template<class U> bool operator!= (RefCountedCPtr<U> const& rhs) const {return m_p != rhs.get();}

    RefCountedCPtr& operator=(RefCountedCPtr const& rhs) {RefCountedCPtr(rhs).swap(*this); return *this;}
    RefCountedCPtr& operator=(T const* rhs) {RefCountedCPtr(rhs).swap(*this); return *this;}
    T const* get() const {return m_p;}
    T const& operator*() const {return *m_p;}
    T const* operator->() const {return m_p;}

    bool IsValid() const {return nullptr != m_p;} //!< Return true if the ref-counted object has a valid (non-NULL) internal object.
    bool IsNull() const  {return !IsValid();}     //!< Return true if the ref-counted object has a invalid (NULL) internal object.
    bool Equals(T const* other) const {return other == get();}
    bool Equals(RefCountedCPtr const& other) const {return other.get() == get();}
    void swap(RefCountedCPtr & rhs) {T const* tmp = m_p; m_p = rhs.m_p; rhs.m_p = tmp;} //!< Swap the internal objects pointed to by two smart pointers.
};

#if !defined (DOCUMENTATION_GENERATOR)
template<class T> void swap(RefCountedPtr<T> & lhs, RefCountedPtr<T> & rhs) {lhs.swap(rhs);}
template<class T> T * get_pointer(RefCountedPtr<T> const & p) {return p.get();}
template<class T, class U> RefCountedPtr<T> static_pointer_cast(RefCountedPtr<U> const & p){return static_cast<T *>(p.get());}
template<class T, class U> RefCountedPtr<T> const_pointer_cast(RefCountedPtr<U> const & p) {return const_cast<T *>(p.get());}
template<class T, class U> RefCountedPtr<T> dynamic_pointer_cast(RefCountedPtr<U> const & p){return dynamic_cast<T *>(p.get());}
#endif // DOCUMENTATION_GENERATOR

END_BENTLEY_NAMESPACE
