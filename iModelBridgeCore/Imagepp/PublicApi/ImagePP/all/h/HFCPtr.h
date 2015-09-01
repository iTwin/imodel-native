//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCPtr.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:> Class: HFCPtr<T>
// ----------------------------------------------------------------------------
#pragma once

#include <Bentley/BeAtomic.h>

BEGIN_IMAGEPP_NAMESPACE
/**

 This is the companion class for the HFCPtr class.

 To use the template class HFCPtr<T>, T must inherit HFCShareableObject<T>
 or implement methods @c{_internal_NotifyAdditionOfASmartPointer} and
 @c{_internal_NotifyRemovalOfASmartPointer} with the same behavior as
 described here.

 Nothing special is to be known about the interface of this class.  Simply add
 it in the inheritance list of the class to allow it to have instance managed
 by smart pointers.  No method overload is required, and the methods defined
 here are not to be called (excepted in very special circumstances).

 This is a template class.  It is necessary because it support multithreaded
 environments, where smart pointers may point to common object through many
 threads.  In order to not have one key per managed object (requiring too much
 resources), nor one single global key (causing bottleneck congestion), we have
 choosen to have one global key per inheritance inclusion point.  This is why this
 is a template class, where the template argument is the class that inherits
 this one (yes, C++ allows that!).  Here is an example:

 @code
 class HPMPersistentObject : public HFCShareableObject<HPMPersistentObject>
 {
     ....
 @end

 The template argument is only used for multiplying instances of internal keys.
 No interface feature is required for the class to use as template argument.
 See the technical notes in the documentation of the HFCPtr class for more
 informations.

 @see &{link:HFCPtr}

*/

template<class T> class HNOVTABLEINIT HFCShareableObject
    {
public:

    typedef HFCShareableObject<T>  ShareableAncestor;
    typedef HFCShareableObject<T>* ShareableAncestorPtr;

    HFCShareableObject();
    HFCShareableObject(const HFCShareableObject<T>& pi_rObj);
    ~HFCShareableObject();
    const HFCShareableObject<T>& operator=(const HFCShareableObject<T>& pi_rObj);


    //:> These methods should be protected, since they are used only by
    //:> smart pointers.  But we cannot give friendship to template
    //:> classes.  So we use names that are longer than usual :)

    void                         _internal_NotifyAdditionOfASmartPointer();
    void                         _internal_NotifyRemovalOfASmartPointer();

    //:> These methods are sometimes required to protect the object

    void                         IncrementRef();
    void                         DecrementRef();
    uint32_t                     GetRefCount() const;

private:

    BeAtomic<uint32_t>    m_RefCount;
    };

//-----------------------------------------------------------------------------


/**

    This parameterized class is used to create some kind of "intelligent
    pointers" for dynamically allocated objects.  These pointers have the
    ability to destroy the referenced object as soon as nothing references
    it anymore, preventing this way the existence of ghost instances in
    memory.  It also prevents deletion of an object that is still referenced
    somewhere else.  To do this, objects of type HFCPtr count instances of
    themselves that refers a particuliar object in memory.

    To make it work, pointed object must inherit from a class provided with
    the smart pointer class : it is HFCShareableObject.  It is a small class
    that contains no virtual method, so it does not affect the performance
    of class to modify, even if it causes multiple inheritance.  It is used
    to qualify the "shareability" of a kind of object, by giving a common
    interface to them (used internally by smart pointers) and by adding a
    reference count to objects to be shared.  All classes of objects to be
    pointed to by smart pointers must inherits, directly of not, from
    HFCShareableObject.

    HFCPtr is designed to let its instances be used nearly the same way as
    any other standard C++ pointer, with one major exception: you @b{cannot}
    operate @b{delete} on it nor on the real pointer it encapsulates.   Many
    operators are overloaded to mimic the behavior of C++ pointers.

    Results are unpredicable when using HFCPtr with objects that are not
    allocated from the heap and when using HFCPtr on arrays.

    Throughout this document we identify instances of this class as "smart
    pointers".

    @h3{Some methods available in HFCShareableObject class}

    There may be some specific situation where actual functionnality cannot
    be used to get correct results.  So there are a few methods that are
    provided as runarounds for these cases.

      @list{@b{RestrictSharing} : makes the object to become "protected", prevents
            deletion of the object by smart pointers.}
      @list{@b{IncrementRef} and @b{DecrementRef} : can be used to artificially alter
            the reference count maintained for that object.}

    Use them with great care, as they do not respect the normal behavior of
    smart pointers.

    @h3{Technical notes about multithreading}

    This class is multithread safe.  The reference count hidden in shared
    objects is protected by an exclusive key that allow the sharing of
    objects by more than one thread. However, to avoid abusive usage of
    system resource, there is not a single key for each reference count to
    protect, but a common key used for many counts. The side-effect of this
    economy of resources should be some loss of performance caused by the
    fact that only one smart pointer may be copied at a time in a given
    application, no matter of how many threads are running.

    To avoid such undesirable effect, a compromise have been made: there is
    a single key to manage counts for a given inheritance tree.  This is
    done by making the HFCShareableObject class as a paramterized class by
    the use of templates.  The template argument of the HFCShareableClass
    has only that  role of multiplying the instances of that class to create
    one for each inheritance tree of shareable ojbects in a given system.
    Since the key is a hidden static  member of that class, we get one key
    for each tree and that key is used to manage the access to the reference
    count of all instances of all classes found in the associated tree.

    Such sharing management is also entirely delegated to the
    HFCShareableClass, letting the programmer to create another solution
    that fits better its need.

    @h3{Technical notes about the HFCShareableObject class}

    There is no thight relationship between the HFCPtr class and the
    HFCShareableObject class.  The use of generic programming techniques (in
    the STL manner) allowed the use of the HFCShareableObject to define an
    interface for all managed objects while  having no single explicit
    reference to that class in the HFCPtr code.  All operations about
    reference count management and object deletion have been delegated to
    the HFCShareableObject class. This permit the programmer to create a
    completely different class for reference count management wherever it
    needs to do one.  The most common situation is to provide a
    multithreading support better than the default one.

    If you provide your own class, there is a simple rule to follow :
    provide the same minimal interface defined in the actual class, which
    minimum is limited to two of its methods:

    list{@k{_internal_NotifyAdditionOfASmartPointer} : called when a pointer
            begin reference to the object. Usually reference count is
            incremented.}

    list{@k{_internal_NotifyRemovalOfASmartPointer} : called when a pointer
             stops reference to the object. Usually reference count is
             decremented, and object is deleted when count reaches zero.}

    Both methods do not take any parameter and do not return a value.

*/

template<class T> class HNOVTABLEINIT HFCPtr
    {
public:
    typedef T POINTED_TYPE;
    //:> Primary methods

    HFCPtr();
    HFCPtr(T* pi_Ptr);
    HFCPtr(const HFCPtr<T>& pi_rObj);
    HFCPtr<T>&      operator=(const HFCPtr<T>& pi_rObj);
    HFCPtr<T>&      operator=(T* pi_Ptr);
    ~HFCPtr();

    //:> Comparison operators

    bool             operator<(const HFCPtr<T>& pi_rObj) const;
    bool             operator>(const HFCPtr<T>& pi_rObj) const;
    bool             operator==(const HFCPtr<T>& pi_rObj) const;
    bool             operator!=(const HFCPtr<T>& pi_rObj) const;
    bool             operator==(const T* pi_pObj) const;
    bool             operator!=(const T* pi_pObj) const;

    //:> Dereferencing

    T&              operator*() const;
    T*              operator->() const;
    T*              operator->*(int) const;
    operator T* () const;

    //:> Other methods

    T*              GetPtr() const;

private:

    // The real pointer behind the smart one

    T*              m_pObject;
    };

// Boost like C++ cast familly 
// e.g. : 
// HFCPtr<U> pUType(new U);
// HFCPtr<T> pTType = static_pointer_cast<T>(pUType)
template<class T, class U> HFCPtr<T> static_pcast(const HFCPtr<U>& pi_rPtr);
template<class T, class U> HFCPtr<T> const_pcast(const HFCPtr<U>& pi_rPtr);
template<class T, class U> HFCPtr<T> dynamic_pcast(const HFCPtr<U>& pi_rPtr);

END_IMAGEPP_NAMESPACE
#include "HFCPtr.hpp"
