/*--------------------------------------------------------------------------*/ 
/*  ReferenceCountedObject.h												*/ 
/*	Reference counted object base class										*/ 
/*  (C) 2003 Copyright Pointools Ltd, UK | All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 26 Nov 2004 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 

#ifndef POINTOOLS_REF_COUNT
#define POINTOOLS_REF_COUNT

#include <pt/debug.h>

namespace pt
{
class ReferenceCountedObject {
public:

    int ReferenceCountedObject_refCount;

protected:

    ReferenceCountedObject() : ReferenceCountedObject_refCount(0) {
        debugAssertM(isValidHeapPointer(this), 
            "Reference counted objects must be allocated on the heap.");
    }

public:

    virtual ~ReferenceCountedObject() {}

    ReferenceCountedObject(const ReferenceCountedObject& notUsed) : 
        ReferenceCountedObject_refCount(0) {
        debugAssertM(isValidHeapPointer(this), 
            "Reference counted objects must be allocated on the heap.");
    }
};
template <class T>
class ReferenceCountedPointer {
private:

    T*           pointer;

public:

    inline T* getPointer() const {
        return pointer;
    }

private:

    void registerReference() { 
        pointer->ReferenceCountedObject_refCount += 1;
        //debugPrintf("  ++0x%x\n", pointer);
        //debugPrintf("  [0x%x] = %d\n", pointer, pointer->ReferenceCountedObject_refCount);
    }


    int deregisterReference() {
        if (pointer->ReferenceCountedObject_refCount > 0) {
            pointer->ReferenceCountedObject_refCount -= 1;
            //debugPrintf("  --0x%x\n", pointer);
            //debugPrintf("  [0x%x] = %d\n", pointer, pointer->ReferenceCountedObject_refCount);
        }

        return pointer->ReferenceCountedObject_refCount;
    }


    void zeroPointer() {
        if (pointer != NULL) {

            debugAssert(isValidHeapPointer(pointer));

            if (deregisterReference() <= 0) {
                // We held the last reference, so delete the object
                //debugPrintf("  delete 0x%x\n", pointer);
                delete pointer;
            }

            pointer = NULL;
        }
    }


    void setPointer(T* x) {
        if (x != pointer) {
            zeroPointer();

            if (x != NULL) {
                debugAssert(isValidHeapPointer(x));

		        pointer = x;
		        registerReference();
            }
        }
    }

public:      

    inline ReferenceCountedPointer() : pointer(NULL) {}

    /**
      Allow compile time subtyping rule 
      RCP&lt;<I>T</I>&gt; &lt;: RCP&lt;<I>S</I>&gt; if <I>T</I> &lt;: <I>S</I>
     */
    template <class S>
    inline ReferenceCountedPointer(const ReferenceCountedPointer<S>& p) : pointer(NULL) {
        setPointer(p.getPointer());
    }

    // We need an explicit version of the copy constructor as well or 
    // the default copy constructor will be used.
    inline ReferenceCountedPointer(const ReferenceCountedPointer<T>& p) : pointer(NULL) {
        setPointer(p.pointer);
    }

    /** Allows construction from a raw pointer.  That object will thereafter be
        reference counted -- do not call delete on it. */
    inline ReferenceCountedPointer(T* p) : pointer(NULL) { 
        setPointer(p); 
    }
    
    inline ~ReferenceCountedPointer() {
        zeroPointer();
    }
  

    inline const ReferenceCountedPointer<T>& operator=(const ReferenceCountedPointer<T>& p) {
        setPointer(p.pointer);
        return *this;
    }   


    inline ReferenceCountedPointer<T>& operator=(T* p) {
        setPointer(p);
        return *this;
    }


    inline bool operator==(const ReferenceCountedPointer<T>& y) const { 
        return (pointer == y.pointer); 
    }

    inline bool operator==(const T *y) const { 
        return (pointer == y); 
    }

    inline bool operator!=(const ReferenceCountedPointer<T>& y) const { 
        return (pointer != y.pointer); 
    }

	inline bool operator!=(const T *y) const { 
        return (pointer != y); 
    }

    inline T& operator*() const {
        return (*pointer);
    }


    inline T* operator->() const {
        return pointer;
    }


    inline bool isNull() const {
        return (pointer == NULL);
    }

    inline bool notNull() const {
        return (pointer != NULL);
    }

    /**
     Returns true if this is the last reference to an object.
     Useful for flushing memoization caches-- a cache that holds the last
     reference is unnecessarily keeping an object alive.
     */
    inline int isLastReference() const {
        return (pointer->ReferenceCountedObject_refCount == 1);
    }
};
};
#endif