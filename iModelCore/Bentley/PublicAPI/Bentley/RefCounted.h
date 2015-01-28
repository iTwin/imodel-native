/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/RefCounted.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "BentleyAllocator.h"
#include "BeAtomic.h"

// For std::move on Android.
#include <utility>

#if !defined (DOCUMENTATION_GENERATOR)
extern "C" BENTLEYALLOCATOR_EXPORT  void  bentleyAllocator_deleteRefCounted (void*object, size_t size);
extern "C" BENTLEYALLOCATOR_EXPORT  void* bentleyAllocator_allocateRefCounted (size_t size);
extern "C" BENTLEYALLOCATOR_EXPORT  void  bentleyAllocator_deleteArrayRefCounted (void*object, size_t size);
extern "C" BENTLEYALLOCATOR_EXPORT  void* bentleyAllocator_allocateArrayRefCounted (size_t size);

extern "C" BENTLEYALLOCATOR_EXPORT  void  bentleyAllocator_deleteIRefCounted (void*object, size_t size);
extern "C" BENTLEYALLOCATOR_EXPORT  void* bentleyAllocator_allocateIRefCounted (size_t size);
extern "C" BENTLEYALLOCATOR_EXPORT  void  bentleyAllocator_deleteArrayIRefCounted (void*object, size_t size);
extern "C" BENTLEYALLOCATOR_EXPORT  void* bentleyAllocator_allocateArrayIRefCounted (size_t size);
#endif

BEGIN_BENTLEY_NAMESPACE

/*=================================================================================**//**
* Interface to be adopted by a class the implements the reference-counting pattern.
* @bsiclass                                                     Keith.Bentley   09/07
+===============+===============+===============+===============+===============+======*/
struct  IRefCounted
    {
protected:
    virtual ~IRefCounted() {}         // force virtual destructor for all subclasses

    DEFINE_BENTLEY_NEW_DELETE_OPERATORS

public:
    virtual uint32_t AddRef () const = 0;
    virtual uint32_t Release () const = 0;
    };

// You can use this macro to add an implementation of IRefCounted, i.e., the reference-counted pattern, directly into your class.
// You must also put the DEFINE_BENTLEY_REF_COUNTED_MEMBER_INIT below into your constructor.
// You should normally make your class non-copyable. If not, you must define a copy constructor and assignment operator, as shown in the RefCounted template below.
#define DEFINE_BENTLEY_REF_COUNTED_MEMBERS private:\
    mutable BeAtomic<uint32_t> m_refCount;        \
protected:\
    void* operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }                               \
    void  operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }         \
    void* operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }                       \
    void  operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }\
public:\
    uint32_t AddRef() const {return ++m_refCount;}\
    uint32_t Release() const                      \
        {                                         \
        if (1 < m_refCount--)                     \
            return  m_refCount.load();            \
        delete this;                              \
        return  0;                                \
        }

// If you put DEFINE_BENTLEY_REF_COUNTED_MEMBERS in your class definition,
// you must also put the following macro into your constructor:
#define DEFINE_BENTLEY_REF_COUNTED_MEMBER_INIT\
    m_refCount.store(0);


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
* This is an application of the http://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
+===============+===============+===============+===============+===============+======*/
template <class Base> class RefCounted : public Base
    {
    DEFINE_BENTLEY_REF_COUNTED_MEMBERS

protected:
    virtual ~RefCounted() {}         // force virtual destructor for all subclasses

public:
    RefCounted() {DEFINE_BENTLEY_REF_COUNTED_MEMBER_INIT}
    RefCounted(RefCounted const& rhs) {DEFINE_BENTLEY_REF_COUNTED_MEMBER_INIT} // Initialize my ref count to zero. Adopting rhs' data does not add a *reference* to me.
    RefCounted& operator=(RefCounted const& rhs) {if (this != &rhs) {Base::operator=(rhs);} return *this;} // NB: Preserve my ref count! Assigning rhs' data to me does not add a *reference* to me.
    uint32_t GetRefCount() const {return m_refCount.load();}
    };

/*=================================================================================**//**
* Concrete class that can be used to implement the reference-counting pattern.
* Inheriting from this class is equivalent to inheriting from RefCounted<IRefCounted>.
* @bsiclass                                                     Keith.Bentley   09/07
+===============+===============+===============+===============+===============+======*/
class RefCountedBase : public RefCounted <IRefCounted>
    {
public:
    DEFINE_BENTLEY_NEW_DELETE_OPERATORS
    };

/*=================================================================================**//**
* A shared pointer template for reference-counted objects.
* Type \b T must have functions named AddRef and Release with signatures that conform to the reference-counting pattern.
* @bsiclass                                                     03/04
+===============+===============+===============+===============+===============+======*/
// This template was adapted from boost instrusive_ptr.
template<class T> class RefCountedPtr
{
private:

    typedef RefCountedPtr this_type;

public:

    typedef T element_type;

    RefCountedPtr(): p_(0)     {}

    //  << BENTLEY COMMENT:
    //      This constructor is used by the compiler for automatic type conversions.
    //      That is what allows code like this to compile:
    //      RefCountedPtr<T> Function ()
    //          {
    //          T* p = ..
    //          return p;
    //          }
    RefCountedPtr(T * p, bool add_ref = true): p_(p)
    {
        if(p_ != 0 && add_ref) p_->AddRef ();
    }

    template<class U> RefCountedPtr(RefCountedPtr<U> const & rhs): p_(rhs.get())
    {
        if(p_ != 0) p_->AddRef ();
    }

    RefCountedPtr(RefCountedPtr const& rhs): p_(rhs.p_)
    {
        if(p_ != 0) p_->AddRef ();
    }

    RefCountedPtr(RefCountedPtr&& rhs) : p_(std::move(rhs.p_)){ rhs.p_ = 0;}

    RefCountedPtr& operator=(RefCountedPtr&& rhs) 
    {
        this_type(std::move(rhs)).swap(*this);
        return *this;
    }

    ~RefCountedPtr()
    {
        if(p_ != 0) p_->Release ();
    }

    template<class U> RefCountedPtr & operator=(RefCountedPtr<U> const & rhs)
    {
        this_type(rhs).swap(*this);
        return *this;
    }

    // << BENTLEY CHANGE:
    //      We moved the comparison operators inside the class.
    bool operator== (RefCountedPtr<T> const& rhs) const
    {
        return p_ == rhs.p_;
    }

    template<class U> bool operator== (RefCountedPtr<U> const& rhs) const
    {
        return p_ == rhs.get();
    }

    bool operator!= (RefCountedPtr<T> const& rhs) const
    {
        return p_ != rhs.p_;
    }

    template<class U> bool operator!= (RefCountedPtr<U> const& rhs) const
    {
        return p_ != rhs.get();
    }

    RefCountedPtr & operator=(RefCountedPtr const & rhs)
    {
        this_type(rhs).swap(*this);
        return *this;
    }

    RefCountedPtr & operator=(T * rhs)
    {
        this_type(rhs).swap(*this);
        return *this;
    }

    //! Get a pointer the intenal object held by the reference-counted object
    T * get() const
    {
        return p_;
    }

    //! Get a constant reference to the intenal object held by the reference-counted object
    T* const& GetCR() const
    {
        return p_;
    }

    //! Get a reference to the intenal object held by the reference-counted object
    T*& GetR()
    {
        return p_;
    }

    //! Get a reference to the intenal object held by the reference-counted object
    T & operator*() const
    {
        return *p_;
    }

    //! Get a pointer to the intenal object held by the reference-counted object
    T * operator->() const
    {
        return p_;
    }

    //! Return true if the ref-counted object has a valid (non-NULL) internal object.
    bool IsValid () const {return 0 != p_;}
    //! Return true if the ref-counted object has a invalid (NULL) internal object.
    bool IsNull () const  {return !IsValid();}
    bool Equals (T const* other) const {return other == get();}
    bool Equals (RefCountedPtr const& other) const {return other.get() == get();}

    // << BENTLEY CHANGE:
    //      We eliminated this type-conversion operator. If the class overloads operator==
    //      and provides a conversion-to-pointer operator, then the compiler thinks that
    //      (ptr == NULL) is ambiguous, since it could either use the overloaded operator==
    //      or convert ptr to a pointer and compare that to NULL instead.
    //      The expression (ptr == NULL) now works by first creating a temporary RefCountedPtr, with p_ = NULL,
    //      and then calling RefCountedPtr::operator==

    //  << BENTLEY COMMENT: The purpose of this strange-looking type and the function
    //  <<  that follows is to allow expressions like
    //  <<      (rcptr == NULL)
    //  <<  without permitting bad things like
    //  <<      delete rcptr
    //  <<  or
    //  <<      (rcptr + 1)
    //  <<  or
    //  <<      (rcptr == 2)
/*
    typedef T * this_type::*unspecified_bool_type;

    operator unspecified_bool_type () const
    {
        return p_ == 0? 0: &this_type::p_;
    }
*/

    //! Swap the internal objects pointed to by two smart pointers.
    void swap(RefCountedPtr & rhs)
    {
        T * tmp = p_;
        p_ = rhs.p_;
        rhs.p_ = tmp;
    }

#if defined (DO_NOT_DO_THIS)
    /* The Boost documentation says that automatic conversion to T* is too error-prone! */
    operator T*() const
    {
        return p_;
    }
#endif

private:

    T * p_;
};

#if !defined (DOCUMENTATION_GENERATOR)

    // << BENTLEY CHANGE:
    //      We removed operator== and similar overloads from global scope. This was causing
    //      a compiler error somewhere else (nobody remembers where). Probably related to
    //      the fact that RefCountedPtr has a constructor that can be used for automatic type conversions.

template<class T> void swap(RefCountedPtr<T> & lhs, RefCountedPtr<T> & rhs)
{
    lhs.swap(rhs);
}

// mem_fn support

template<class T> T * get_pointer(RefCountedPtr<T> const & p)
{
    return p.get();
}

template<class T, class U> RefCountedPtr<T> static_pointer_cast(RefCountedPtr<U> const & p)
{
    return static_cast<T *>(p.get());
}

template<class T, class U> RefCountedPtr<T> const_pointer_cast(RefCountedPtr<U> const & p)
{
    return const_cast<T *>(p.get());
}

template<class T, class U> RefCountedPtr<T> dynamic_pointer_cast(RefCountedPtr<U> const & p)
{
    return dynamic_cast<T *>(p.get());
}

#endif // DOCUMENTATION_GENERATOR

END_BENTLEY_NAMESPACE
