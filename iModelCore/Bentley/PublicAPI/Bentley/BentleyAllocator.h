/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/BentleyAllocator.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/// @cond BENTLEY_SDK_Internal

#include "Bentley.h"
#include <memory>

//! Convenience to declare Bentley overrides of new and delete to allow the operations to be inlined in header files. In this case, it should be included in a public: section.
#define DEFINE_BENTLEY_NEW_DELETE_OPERATORS\
    void* operator new(size_t size) { return bentleyAllocator_new(size); }\
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_delete(rawMemory, size); }

//__PUBLISH_SECTION_END__
#ifndef BENTLEYALLOCATOR_EXPORT
#if defined (__BENTLEYALLOCATOR_BUILD__)
    #define BENTLEYALLOCATOR_EXPORT EXPORT_ATTRIBUTE
#endif
#endif
//__PUBLISH_SECTION_START__

#if !defined (BENTLEYALLOCATOR_EXPORT)
#define BENTLEYALLOCATOR_EXPORT  IMPORT_ATTRIBUTE
#endif

extern "C" BENTLEYALLOCATOR_EXPORT void* bentleyAllocator_new (size_t);
extern "C" BENTLEYALLOCATOR_EXPORT void  bentleyAllocator_delete (void*, size_t);
extern "C" BENTLEYALLOCATOR_EXPORT void* bentleyAllocator_malloc (size_t);
extern "C" BENTLEYALLOCATOR_EXPORT void* bentleyAllocator_realloc (void*, size_t);
extern "C" BENTLEYALLOCATOR_EXPORT void  bentleyAllocator_free (void*, size_t);
extern "C" BENTLEYALLOCATOR_EXPORT void* bentleyAllocator_getNullRefBuffer ();
extern "C" BENTLEYALLOCATOR_EXPORT void  bentleyAllocator_enableLowFragmentationCRTHeap ();

extern "C" BENTLEYALLOCATOR_EXPORT  void  bentleyAllocator_deleteRefCounted (void*object, size_t size);
extern "C" BENTLEYALLOCATOR_EXPORT  void* bentleyAllocator_allocateRefCounted (size_t size);
extern "C" BENTLEYALLOCATOR_EXPORT  void  bentleyAllocator_deleteArrayRefCounted (void*object, size_t size);
extern "C" BENTLEYALLOCATOR_EXPORT  void* bentleyAllocator_allocateArrayRefCounted (size_t size);
extern "C" BENTLEYALLOCATOR_EXPORT  void  bentleyAllocator_deleteIRefCounted (void*object, size_t size);
extern "C" BENTLEYALLOCATOR_EXPORT  void* bentleyAllocator_allocateIRefCounted (size_t size);
extern "C" BENTLEYALLOCATOR_EXPORT  void  bentleyAllocator_deleteArrayIRefCounted (void*object, size_t size);
extern "C" BENTLEYALLOCATOR_EXPORT  void* bentleyAllocator_allocateArrayIRefCounted (size_t size);

BEGIN_BENTLEY_NAMESPACE

/*=================================================================================**//**
* @Description An STL-compliant allocator that calls bentleyAllocator_malloc and bentleyAllocator_free.
* @remarks
* An instance of a class that uses BentleyAllocator avoids cross-DLL memory management problems.
* That makes this allocator useful for defining parameters of public API functions.
* @bsiclass                                                     SamWilson       01/02
+===============+===============+===============+===============+===============+======*/
template <class _Ty> class BentleyAllocator
        {
public:
        typedef _Ty         value_type;
        typedef _Ty*        pointer;
        typedef _Ty&        reference;
        typedef _Ty const*  const_pointer;
        typedef _Ty const&  const_reference;
        typedef std::size_t size_type;
        typedef std::ptrdiff_t difference_type;

        template<class _Other>  struct rebind
                {       // convert an allocator<_Ty> to an allocator <_Other>
                typedef BentleyAllocator<_Other> other;
                };

        pointer address(reference _Val) const
                {       // return address of mutable _Val
                return (&_Val);
                }

        const_pointer address(const_reference _Val) const
                {       // return address of nonmutable _Val
                return (&_Val);
                }

        BentleyAllocator()
                {       // construct default allocator (do nothing)
                }

        BentleyAllocator(const BentleyAllocator<_Ty>&)
                {       // construct by copying (do nothing)
                }

        template<class _Other>
                BentleyAllocator(const BentleyAllocator<_Other>&)
                {       // construct from a related allocator (do nothing)
                }

        template<class _Other>
                BentleyAllocator<_Ty>& operator=(const BentleyAllocator<_Other>&)
                {       // assign from a related allocator (do nothing)
                return (*this);
                }

        void deallocate(pointer _Ptr, size_type _Size)
                {       // deallocate object at _Ptr, ignore size
                bentleyAllocator_free (_Ptr, _Size * sizeof (_Ty));
                }

        pointer allocate(size_type _Count)
                        {       // allocate array of _Count elements
                return (pointer) bentleyAllocator_malloc (_Count * sizeof (_Ty));
                }

        pointer allocate(size_type _Count, typename std::allocator<void>::const_pointer)
                {       // allocate array of _Count elements, ignore hint
                return (allocate(_Count));
                }

        void construct(pointer _Ptr, const _Ty& _Val)
                {       // construct object at _Ptr with value _Val
                new((void*)_Ptr)_Ty(_Val);
                }

        void destroy(pointer _Ptr)
                {       // destroy object at _Ptr
                _Ptr->~_Ty();
                }

        size_type max_size() const
                {       // estimate maximum array size
                size_type _Count = (size_type)(-1) / sizeof (_Ty);
                return (0 < _Count ? _Count : 1);
                }
        };


template<class _Ty, class _Other>
        bool operator== (const BentleyAllocator<_Ty>&, const BentleyAllocator<_Other>&)
        {       // test for allocator equality (always true)
        return true;
        }

template <class _Ty, class _Other>
        bool operator!= (const BentleyAllocator<_Ty>&, const BentleyAllocator<_Other>&)
        {       // test for allocator inequality (always false)
        return false;
        }

END_BENTLEY_NAMESPACE

/// @endcond BENTLEY_SDK_Internal
