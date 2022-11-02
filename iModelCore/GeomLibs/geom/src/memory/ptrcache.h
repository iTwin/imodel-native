/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   ptrcache.h -- cache for heavyweight data structures                 |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
#if !defined (__ptrcacheH__)
#define __ptrcacheH__
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*----------------------------------------------------------------------+
|SECTION pcache Pointer Cache Manager                                   |
| This function package implements a cache of pointers to data          |
| structures.                                                           |
| Callers can then 'grab' and 'drop' individual instances of the        |
| structure; since the grab and drop are implemented by just moving     |
| pointers from and to the cache, this can be significant efficiency    |
| improvement over full allocation of the large structures via the      |
| heap manager.                                                         |
|                                                                       |
| Each cache is created with at least two, and possibly 3 or 4,         |
| callback functions:                                                   |
|   pNewFunc() -- function to call to allocate a new instance of the    |
|       data structure.                                                 |
|   pFreeFunc(pInstance) -- function to call to return an instance      |
|       of the data structure to the heap manager.                      |
|   pGrabFunc (pInstance), pDropFunc (pInstance) -- (optional)          |
|       to be called as instances are grabbed and dropped from the      |
|       cache.  Possible actions at this time are:                      |
|       1) reinitialize the instance to size 0 (although possibly       |
|           retaining allocated memory.)                                |
|       2) free some or all allocated memory.                           |
+----------------------------------------------------------------------*/
typedef struct _PtrCacheHeader *PPtrCacheHeader;

typedef struct
    {
    void * (*pNewFunc) (void);      // Return a new instance of the data structure
    void (*pFreeFunc) (void *);     // Free the instance (deep free)
    void (*pDropFunc) (void *);     // cleanup when inserted to cache.
    void (*pGrabFunc) (void *);     // cleanup when taken out of cache.
    } PtrCache_Functions;
#if defined (__cplusplus)
extern "C" {
#endif

/*---------------------------------------------------------------------------------**//**
* Allocate and return a pointer to a new cache of pointers to some data structure.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public PPtrCacheHeader omdlPtrCache_new     // newly allocated cache header
(
const PtrCache_Functions *pFunctions,   // => functions to call to create, free, and empty instances
int   maxCacheCount                     // => max number of instances to hold in the cache.
);

/*---------------------------------------------------------------------------------**//**
* Set a debug level for a pointer cache. 0 is no debug, nonzero enables console output of grab, drop, free.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public void omdlPtrCache_setDebug
(
PPtrCacheHeader pHeader,
int             level
);

/*---------------------------------------------------------------------------------**//**
* Print summary of the pointer cache activity and contents.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public void omdlPtrCache_debugEvent
(
PPtrCacheHeader pHeader,
void *pCachee,
char *pEventName
);

/*---------------------------------------------------------------------------------**//**
* Free a cache. All instances in the cache are passed to the free function indicated in the cache definition. (Note that all cached pointers
* were passed to the drop function when dropped into the cache.)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public PPtrCacheHeader omdlPtrCache_free    // always NULL
(
PPtrCacheHeader pHeader                     // => header to be freed.
);

/*---------------------------------------------------------------------------------**//**
* Grab an instance of the data structure. If a cached instance is available, use it; otherwise use pNewFunc to get a fresh one. In either
* case, invoke the grab function.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public void * omdlPtrCache_grabFromCache
(
PPtrCacheHeader pHeader         // <=> header holding cache.
);

/*---------------------------------------------------------------------------------**//**
* Pass pInstance to the cache's drop function, then either insert the pointer to the cache or free it.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public void omdlPtrCache_dropToCache
(
PPtrCacheHeader pHeader,                // <=> header holding cache.
void            *pInstance      // => data structure instance to insert to cache.
);

/*---------------------------------------------------------------------------------**//**
* Free all ptrs in cache
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public void omdlPtrCache_flushCache
(
PPtrCacheHeader pHeader
);
END_BENTLEY_GEOMETRY_NAMESPACE

#if defined (__cplusplus)
}
#endif

#endif


