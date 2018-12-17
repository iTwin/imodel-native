/*--------------------------------------------------------------------------------------+
|
|     $Source: vu/src/vumemfuncs.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
typedef struct
    {
    VuMallocFunc malloc;
    VuCallocFunc calloc;
    VuReallocFunc realloc;
    VuFreeFunc free;
    } MemFuncs;

static MemFuncs *s_pMemFuncs = NULL;
static MemFuncs s_substituteMemFuncs = {NULL, NULL, NULL, NULL};

/**----------------------------------------------------------------------+
@description Install pointers to replacements for C runtime memory management functions.
@remarks Call this function once at the start of your session to use application-specific VU memory management.
@param newMalloc IN replacement for malloc
@param newCalloc IN replacement for calloc
@param newRealloc IN replacement for realloc
@param newFree IN replacement for free
@group "VU Memory Management"
@bsimethod                          EarlinLutz          07/04
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void vumemfuncs_setFuncs
(
VuMallocFunc newMalloc,
VuCallocFunc newCalloc,
VuReallocFunc newRealloc,
VuFreeFunc newFree
)
    {
    s_substituteMemFuncs.malloc = newMalloc;
    s_substituteMemFuncs.calloc = newCalloc;
    s_substituteMemFuncs.realloc = newRealloc;
    s_substituteMemFuncs.free = newFree;
    s_pMemFuncs = &s_substituteMemFuncs;
    }

/**----------------------------------------------------------------------+
@description Allocate uninitialized bytes, by C runtime malloc or substitute function.
@param dataSize IN bytes to allocate
@return pointer to newly allocated memory or NULL if insufficient memory available
@group "VU Memory Management"
@see vumemfuncs_setFuncs
@bsimethod                          EarlinLutz          07/04
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     *vumemfuncs_malloc
(
size_t          dataSize
)
    {
    if (s_pMemFuncs)
        return s_pMemFuncs->malloc (dataSize);

    return  malloc (dataSize);
    }

/**----------------------------------------------------------------------+
@description Allocate zero-initialized bytes, by C runtime calloc or substitute function.
@param records IN number of records in buffer
@param recordSize IN bytes per record
@return pointer to newly allocated memory or NULL if insufficient memory available
@group "VU Memory Management"
@see vumemfuncs_setFuncs
@bsimethod                          EarlinLutz          07/04
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     *vumemfuncs_calloc
(
size_t          records,
size_t          recordSize
)
    {
    if (s_pMemFuncs)
        return s_pMemFuncs->calloc (records, recordSize);
    return  calloc (records, recordSize);
    }

/**----------------------------------------------------------------------+
@description Free memory previously allocated by ~mvumemfuncs_malloc, ~mvumemfuncs_calloc or ~mvumemfuncs_realloc.
@param dataP IN pointer to memory to return to heap.
@group "VU Memory Management"
@see vumemfuncs_setFuncs
@bsimethod                          EarlinLutz          07/04
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vumemfuncs_free
(
void            *dataP
)
    {
    if (s_pMemFuncs)
        s_pMemFuncs->free (dataP);
    else
        free (dataP);
    }

/**----------------------------------------------------------------------+
@description Resize memory previously allocated by ~mvumemfuncs_malloc or ~mvumemfuncs_calloc.
@param oldDataP IN prior allocation
@param newSize IN bytes required in reallocated block
@return pointer to newly allocated memory or NULL if insufficient memory available
@group "VU Memory Management"
@see vumemfuncs_setFuncs
@bsimethod                          EarlinLutz          07/04
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     *vumemfuncs_realloc
(
void            *oldDataP,
size_t          newSize
)
    {
    if (s_pMemFuncs)
        return s_pMemFuncs->realloc (oldDataP, newSize);
    return realloc (oldDataP, newSize);
    }
END_BENTLEY_GEOMETRY_NAMESPACE
