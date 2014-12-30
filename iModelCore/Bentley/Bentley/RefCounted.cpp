/*--------------------------------------------------------------------------------------+
|
|     $Source: Bentley/RefCounted.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    #include <windows.h>
#endif
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <Bentley/RefCounted.h>
#include <Bentley/BentleyAllocator.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            bentleyAllocator_delete (void*rawMemory, size_t size)
    {
    ::operator delete (rawMemory);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void*           bentleyAllocator_new (size_t size)
    {
    return ::operator new (size);
    }

/*---------------------------------------------------------------------------------**//**
*** deprecated 
* @bsimethod                                    Keith.Bentley                   01/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            bentleyAllocator_deleteRefCounted (void*rawMemory, size_t size)
    {
    ::operator delete (rawMemory);
    }

/*---------------------------------------------------------------------------------**//**
*** deprecated 
* @bsimethod                                    John.Gooding                    08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void*           bentleyAllocator_allocateRefCounted(size_t size)
    {
    return ::operator new (size);
    }

/*---------------------------------------------------------------------------------**//**
*** deprecated 
* @bsimethod                                    Keith.Bentley                   01/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            bentleyAllocator_deleteArrayRefCounted (void*rawMemory, size_t size)
    {
    ::operator delete (rawMemory);
    }

/*---------------------------------------------------------------------------------**//**
*** deprecated 
* @bsimethod                                    John.Gooding                    08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void*           bentleyAllocator_allocateArrayRefCounted(size_t size)
    {
    return ::operator new (size);
    }

//  I tried to make the new and delete operators private on IRefCounted to prevent 
//  uses of the default new and delete.  When I did so, for some files the compiler
//  generated an error saying that it cannot generate __vecDelDtor because it
//  could not access operator delete.
//
//  The compiler generates __vecDelDtor for a class if delete [] is ever used for the class.
//  It appears that the compiler is generating a __vecDelDtor that uses IRefCounted's 
//  operator delete because something is deleting an array of instances of a class derived from IRefCounted 
//  and either the class is not derived from RefCounted or the compiler does not know 
//  the class is derived from RefCounted.  Unfortunately, the compiler error message
//  does not help in finding the delete.  I've implemented these functions hoping they
//  will get called and show me the source of the problem. 
/*---------------------------------------------------------------------------------**//**
*** deprecated 
* @bsimethod                                    Keith.Bentley                   01/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            bentleyAllocator_deleteIRefCounted (void*rawMemory, size_t size)
    {
    assert (false && "DeleteIRefCounted");
    ::operator delete (rawMemory);
    }

/*---------------------------------------------------------------------------------**//**
*** deprecated 
* @bsimethod                                    John.Gooding                    08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void*           bentleyAllocator_allocateIRefCounted(size_t size)
    {
    assert (false && "AllocateIRefCounted");
    return ::operator new (size);
    }

/*---------------------------------------------------------------------------------**//**
*** deprecated 
* @bsimethod                                    Keith.Bentley                   01/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            bentleyAllocator_deleteArrayIRefCounted (void*rawMemory, size_t size)
    {
    assert (false && "DeleteArrayIRefCounted");
    ::operator delete (rawMemory);
    }

/*---------------------------------------------------------------------------------**//**
*** deprecated 
* @bsimethod                                    John.Gooding                    08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void*           bentleyAllocator_allocateArrayIRefCounted(size_t size)
    {
    assert (false && "AllocateArrayIRefCounted");
    return ::operator new (size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void*           bentleyAllocator_realloc (void* p, size_t n) {return realloc(p,n);}
void*           bentleyAllocator_malloc (size_t n) {return malloc(n);}
void            bentleyAllocator_free (void* p, size_t) {return free(p);}

void*           bentleyAllocator_getNullRefBuffer ()
    {
    static char s_buf[32];
    return s_buf;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void bentleyAllocator_enableLowFragmentationCRTHeap()
    {
#if defined (BENTLEY_WIN32)

    intptr_t hCrtHeap = _get_heap_handle();
    ULONG ulEnableLFH = 2;
    HeapSetInformation((PVOID)hCrtHeap, HeapCompatibilityInformation, &ulEnableLFH, sizeof(ulEnableLFH));

#elif defined (BENTLEY_WINRT)

    //

#elif defined (__unix__)
    
    // is there a Linux equivalent?

#else
#error unknown runtime
#endif
    }

