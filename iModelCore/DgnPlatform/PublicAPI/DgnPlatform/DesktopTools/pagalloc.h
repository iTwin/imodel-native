/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DesktopTools/pagalloc.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------
Here are the environment variables that affect PAGALLOC memory debugging:

    MS_PAGALLOC                 when set (to any value), enables PAGALLOC memory
                    debugging if EXE uses papatch.c in the way
                    that w32start.c does for USTATION.EXE,
                    and if the MSVCRT.DLL has recognizable
                    signatures for malloc, free, etc.
    PAGE_DEBUG_HEADERS          when non-zero, makes header pages visible
                    instead of PAGE_NOACCESS-protected at the
                    expense of one page per allocation.
    PAGE_DEBUG_LOW              when non-zero, top-justifies allocation for
                    more effectively finding accesses
                    at addresses lower than malloced ptrs.

    PAGE_DEBUG_NO_RELEASE_SBH
    PAGE_DEBUG_NO_RELEASE       when non-zero, address space is never re-used.
                    This will cause address space exhaustion
                    for all but the smallest programs.
    PAGE_DEBUG_FREE_CHECK       when non-zero, checks fill pattern during frees.
    PAGE_DEBUG_ALLOC_BREAK      when non-zero, breakpoints when can't malloc.
    PAGE_DEBUG_CALL_STACK       when non-zero, captures call stack on mallocs.
    PAGE_DEBUG_ALLOC_DUMP       when non-zero, dumps "heap" on malloc failures.
                    Value can be an integral number of mallocs
                    to skip for "heap" dump.
    PAGE_DEBUG_REALLOC_BREAK    when non-zero, breakpoints when realloc frees fail.
    PAGE_DEBUG_FREE_BREAK       when non-zero, breakpoints when free fails.
    PAGE_DEBUG_MSIZE_BREAK      when non-zero, breakpoints when msize fails.
    PAGE_DEBUG_EXPAND_BREAK     when non-zero, breakpoints when expand fails.
    PAGE_DEBUG_ADDRESS_BREAK    when non-zero, breakpoints when address freed
                    doesn't exactly match address returned by malloc.
    PAGE_DEBUG_FILL_DATA        if set to hex value, this is used to fill
                    malloced memory.  Will accept 1, 2, or 4
                    byte fill patterns.
    PAGE_DEBUG_ALIGN_BITS       if set to integral value, will align all
                    malloced pointers by masking off this many
                    bits.  For example, 1 bit will align on word
                    bounds, and 2 bits will align on 32-bit bounds.
    PAGE_DEBUG_SBH_MB           if set to integral value, uses this to size the
                    small block heap (units are megabytes).
    PAGE_DEBUG_SBH_DECR_MB      if set to integral value, uses this to decrement
                    the size of the small block heap when the
                    initial reservation fails (units are megabytes).
    PAGE_DEBUG_SBH_BASE         if set to integral value, uses this to set the
                    base address of the small block heap.
    PAGE_DEBUG_SBH_TOP_DOWN     when non-zero, allocates the small block heap using
                    highest possible virtual address range.
    PAGE_DEBUG_LRU_DISABLE      when non-zero, disables least recently used
                    allocation algorithm for small block heap.
    PAGE_DEBUG_TRACELOG         if set to integral non-zero value, calls toolsubs
                    traceLog functions to log significant events,
                    potentially even logging each malloc and free
                    if the pagalloc_malloc and pagalloc_free
                    traceLog layers are toggled on with blogtog.
    PAGE_DEBUG_DUMP_MODULES     if set to integral value, dumps all module names
                    and base addresses after this many mallocs.
                    Dump goes to debugger output and, if enabled,
                    traceLog severity debug.
    PAGE_DEBUG_OUTFILE          if set to filename (full path), output normally
                    sent to debugger and/or traceLog will get
                    appended to this file.
    PAGE_DEBUG_CHECK_DOUBLE_FREE
                if non-zero, print out the stack for a double freeded block.
                (asserts PAGE_DEBUG_CALL_STACK)
-----------------------------------------------------------------------*/
#ifndef __pagallocH__
#define __pagallocH__
/*__BENTLEY_INTERNAL_ONLY__*/
#ifndef PAGALLOC_API
#define PAGALLOC_API
#endif
#include <Bentley/Bentley.h>
#if defined (__cplusplus)
extern "C" {
#endif
/*-----------------------------------------------------------------------
C runtime memory allocation replacements
-----------------------------------------------------------------------*/
PAGALLOC_API void * __cdecl pagalloc_malloc (size_t const size);
PAGALLOC_API void * __cdecl pagalloc_nh_malloc (size_t const size, int32_t const nhFlag);
PAGALLOC_API void * __cdecl pagalloc_calloc (size_t const num, size_t const size);
PAGALLOC_API void   __cdecl pagalloc_free(void * const voidP);
PAGALLOC_API void * __cdecl pagalloc_realloc(void  *oldDataP, size_t newSize);
PAGALLOC_API void * __cdecl pagalloc_expand (void *oldDataP, size_t newsize);
PAGALLOC_API size_t __cdecl pagalloc_msize (void * pblck);
PAGALLOC_API char * __cdecl pagalloc_strdup(char const * const);      // WIP_CHAR_OK - pagalloc has to catch these
PAGALLOC_API wchar_t * __cdecl pagalloc_wcsdup(wchar_t const * const);

PAGALLOC_API void * __cdecl pagalloc_operatorNew(size_t size);
PAGALLOC_API void __cdecl pagalloc_operatorDelete(void * const voidP);
#ifndef _HEAPINFO_DEFINED
typedef struct _heapinfo _HEAPINFO;
#endif
PAGALLOC_API int32_t __cdecl pagalloc_heapWalk(_HEAPINFO * const pHeapInfo);

/*-----------------------------------------------------------------------
Exported internal functions with context parameter
-----------------------------------------------------------------------*/
PAGALLOC_API void * __cdecl pagallocI_malloc (size_t  size, void const * const context, unsigned char const lastOperation);
PAGALLOC_API void * __cdecl pagallocI_realloc(void  * const oldDataP, size_t const newSize, void const * const context);
PAGALLOC_API void * __cdecl pagallocI_expand (void * const oldDataP, size_t const newsize, void const * const context);

/*-----------------------------------------------------------------------
These functions set handler functions for free and msize failures
-----------------------------------------------------------------------*/
typedef void (*CRuntimeFreeFunc) (void*);
typedef int32_t (*CRuntimeMsizeFunc)(void*);
PAGALLOC_API void __cdecl pagalloc_set_free_handler(CRuntimeFreeFunc freeHandler);
PAGALLOC_API void __cdecl pagalloc_set_msize_handler(CRuntimeMsizeFunc msizeHandler);
PAGALLOC_API void __cdecl pagalloc_showCallStackSince(void const * const  rawAddress, uint32_t const  thisSerialNumber);
void __declspec(dllexport) __cdecl pagalloc_showCallStack(void const * const rawAddress);
#if defined (__cplusplus)
}
#endif

#endif
