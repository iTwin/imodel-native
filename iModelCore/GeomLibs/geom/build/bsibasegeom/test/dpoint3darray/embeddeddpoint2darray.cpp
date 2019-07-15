/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_NAMESPACE
typedef VArrayWrapper<DPoint2d> DPoint2dArrayWrapper;

Public bool      EmbeddedDPoint2dArray_insertDPoint3d
(
        MyDPoint2dArray   *pHeader,
const   DPoint3d                *pInPoint,
        int                     index
);
Public bool      EmbeddedDPoint2dArray_getDPoint3d
(
const   MyDPoint2dArray   *pHeader,
        DPoint3d                *pPoint,
        int                     index
);


/*---------------------------------------------------------------------------------**//**
* @description Allocate a new MyDPoint2dArray header from the system heap.
*
* @return pointer to the header.
* @DefaultRequiredLibrary mtg.lib
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public MyDPoint2dArray *EmbeddedDPoint2dArray_new
(
void
)
    {
    return new MyDPoint2dArray ();
    }


/*---------------------------------------------------------------------------------**//**
* @description Initialize an MyDPoint2dArray header.  Prior contents are
*       destroyed.  Intended for use immediately following uninitialized creation
*       operation such as (a) local variable declaration or (b) allocation from system
*       heap.
* @param pHeader    OUT     array to initialize.
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public void  EmbeddedDPoint2dArray_init
(
MyDPoint2dArray   *pHeader
)
    {
    if (pHeader)
        DPoint2dArrayWrapper::init(pHeader, (sizeof(DPoint2d)));
    }


/*---------------------------------------------------------------------------------**//**
* @description Return both the header and its associated memory to the system heap.
*       This should only be used for a header originally allocated via
*       ~mEmbeddedDPoint2dArray_new.  Headers allocated as locals should be
*       decommissioned via ~mEmbeddedDPoint2dArray_releaseMem.
* @param pHeader    IN OUT  array to be freed.
* @return Always returns NULL.
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public MyDPoint2dArray *EmbeddedDPoint2dArray_free
(
MyDPoint2dArray   *pHeader
)
    {
    if (pHeader)
        {
        delete pHeader;
        }
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @description Reduce the count (number of DPoint2ds) in the array to zero.
*       Existing memory is retained so the array can be refilled to its prior
*       size without requiring reallocation.
* @param pHeader    IN OUT  array to modify
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public void  EmbeddedDPoint2dArray_empty
(
MyDPoint2dArray   *pHeader
)
    {
    if (pHeader)
        DPoint2dArrayWrapper::empty (pHeader);
    }

/*---------------------------------------------------------------------------------**//**
* @description Release all memory attached to the header, and reinitialize the header
*       as an empty array with no buffer.
* @param pHeader    IN OUT  array to empty
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public void  EmbeddedDPoint2dArray_releaseMem
(
MyDPoint2dArray   *pHeader
)
    {
    if (pHeader)
        DPoint2dArrayWrapper::releaseMem (pHeader);
    }

/*---------------------------------------------------------------------------------**//**
* @description Grab (borrow) an array from the cache.  Caller is responsible
*       for using ~mEmbeddedDPoint2dArray_drop to return the array to the cache when
*       finished.   Controlled "grab and drop" of cache arrays is faster than using
*       either local variables (~mEmbeddedDPoint2dArray_init and
*       ~mEmbeddedDPoint2dArray_releaseMem) or heap allocation
*       (~mEmbeddedDPoint2dArray_new and ~mEmbeddedDPoint2dArray_free)
*       because the preallocated variable size parts of cached arrays are immediately
*       available without revisiting the system cache.
* @return An array header obtained from the cache.
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public MyDPoint2dArray *EmbeddedDPoint2dArray_grab
(
void
)
    {
#ifdef USE_PTR_CACHE
    if (!pCache)
        initCache ();
    return (MyDPoint2dArray *)omdlPtrCache_grabFromCache (pCache);
#else
    return EmbeddedDPoint2dArray_new ();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @description Drop (return) an array to the cache.  Use this to dispose of arrays
*       borrowed with ~mEmbeddedDPoint2dArray_grab.
* @param pHeader    IN      pointer to array to return to cache.
* @return always returns NULL.
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public MyDPoint2dArray *EmbeddedDPoint2dArray_drop
(
MyDPoint2dArray   *pHeader
)
    {
#ifdef USE_PTR_CACHE
    if (!pCache)
        initCache ();
    omdlPtrCache_dropToCache (pCache, pHeader);
    return NULL;
#else
    return EmbeddedDPoint2dArray_free (pHeader);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @description Swap the contents (counts and associated memory) of two headers.
*
* @param pHeader0   IN OUT  first array header
* @param pHeader1   IN OUT  second array header
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public void  EmbeddedDPoint2dArray_swapContents
(
MyDPoint2dArray   *pHeader0,
MyDPoint2dArray   *pHeader1
)
    {
    MyDPoint2dArray scratchHeader = *pHeader0;
    *pHeader0 = *pHeader1;
    *pHeader1 = scratchHeader;
    }


/*---------------------------------------------------------------------------------**//**
* @description Ensure the buffer has capacity for n DPoint2ds without
*       reallocation. The count of DPoint2ds in the buffer remains unchanged.
* @param pHeader    IN OUT  array to modify.
* @param n          IN      number of DPoint2ds in buffer.
* @return false if unable to allocate the buffer.
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_ensureCapacity
(
MyDPoint2dArray   *pHeader,
int                     n
)
    {
    if (NULL != pHeader)
        {
        DPoint2dArrayWrapper::setBufferSize(pHeader, n);
        return true;
        }
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @description Reallocate the buffer to accommodate exactly n DPoint2ds
*       NOTE: this will truncate the contents of this instance if its count is
*       greater than n.
* @param pHeader    IN OUT  array to modify.
* @param n          IN      number of values to accommodate, no more, no less.
* @return false if unable to reallocate the buffer.
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_setExactBufferSize
(
MyDPoint2dArray   *pHeader,
int                     n
)
    {
    if (NULL != pHeader)
        {
        DPoint2dArrayWrapper::setBufferSize(pHeader, n);
        return true;
        }
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @description Return the number of DPoint2ds in the array.
*
* @param pHeader    IN      array to query.
* @return array count
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public int   EmbeddedDPoint2dArray_getCount
(
const   MyDPoint2dArray   *pHeader
)
    {
    if (!pHeader)
        return 0;
    return DPoint2dArrayWrapper::getCount(pHeader);
    }


/*---------------------------------------------------------------------------------**//**
* @description Append a DPoint2d to the end of the array.  The array count is increased
*       by one.
* @param pHeader    IN OUT  array to modify.
* @param pPoint     IN      DPoint2d to append to the array.
* @return true if operation is successful
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_addDPoint2d
(
        MyDPoint2dArray   *pHeader,
const   DPoint2d                *pPoint
)
    {
    return pHeader != NULL
        && SUCCESS == DPoint2dArrayWrapper::insert(pHeader,  pPoint, -1);
    }


/*---------------------------------------------------------------------------------**//**
* @description Insert at a specified position, shifting others to higher
*       positions as needed.
* @param pHeader    IN OUT  array to modify.
* @param pPoint     IN      data to insert.
* @param index      IN      index at which the value is to appear in the array.
*                           The special index -1 (negative one) indicates to
*                           insert at the end of the array.
* @return true if operation is successful
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_insertDPoint2d
(
        MyDPoint2dArray   *pHeader,
const   DPoint2d                *pPoint,
        int                     index
)
    {
    return pHeader != NULL
        && SUCCESS == DPoint2dArrayWrapper::insert(pHeader,  pPoint, index);
    }


/*---------------------------------------------------------------------------------**//**
* @description Append an array of DPoint2d to the end of the array.
*
* @param pHeader    IN OUT  header of array receiving values
* @param pPoint     IN      array of data to add
* @param n          IN      number to add.
* @return true if operation is successful
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_addDPoint2dArray
(
        MyDPoint2dArray   *pHeader,
const   DPoint2d                *pPoint,
        int                     n
)
    {
    return pHeader != NULL
            && SUCCESS == DPoint2dArrayWrapper::insert (pHeader,  pPoint, -1, n);
    }


/*---------------------------------------------------------------------------------**//**
* @description Insert an array of DPoint2ds in the array, with index given for
*       first new DPoint2d.  All previous contents from that index up are moved to
*       make room for the new data.
* @param pHeader    IN OUT  header of array receiving data
* @param pPoint     IN      array of values to add
* @param index      IN      index location for adding the array
* @param n          IN      number of values to add
* @return true if operation is successful
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_insertDPoint2dArray
(
        MyDPoint2dArray   *pHeader,
const   DPoint2d                *pPoint,
        int                     index,
        int                     n
)
    {
    return pHeader != NULL
        && SUCCESS == DPoint2dArrayWrapper::insert (pHeader,  pPoint, index, n);
    }


/*---------------------------------------------------------------------------------**//**
* @description Copy up to nreq DPoint2ds out of the array into a buffer.
*
* @param pHeader    IN      header of array to access.
* @param pBuffer    OUT     buffer of DPoint2ds.
* @param nGot       OUT     number of DPoint2ds placed in buffer.
* @param i0         IN      index of first DPoint2d to access.
* @param nreq       IN      number of DPoint2ds requested.
* @return true if at least one DPoint2d was copied.
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_getDPoint2dArray
(
const   MyDPoint2dArray   *pHeader,
        DPoint2d                *pBuffer,
        int                     *nGot,
        int                     i0,
        int                     nreq
)
    {
    if (!pHeader)
        {
        *nGot = 0;
        return false;
        }
    *nGot = DPoint2dArrayWrapper::getArray(pHeader,  pBuffer, i0, nreq);
    return *nGot > 0;
    }


/*---------------------------------------------------------------------------------**//**
* @description Get a DPoint2d from a specified index in the array.
*
* @param pHeader    IN      header of array to access.
* @param pPoint     OUT     DPoint2d accessed from the array.
* @param index      IN      index of DPoint2d to access. Any negative index indicates
*                           highest numbered element in the array.
* @return false if the index is too large, i.e., no DPoint2d was accessed.
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_getDPoint2d
(
const   MyDPoint2dArray   *pHeader,
        DPoint2d                *pPoint,
        int                     index
)
    {
    return pHeader != NULL
        && SUCCESS == DPoint2dArrayWrapper::get (pHeader,  pPoint, index);
    }


/*---------------------------------------------------------------------------------**//**
* @description Drop a contiguous block of DPoint2ds.  Copy higher indices back down.
*
* @param pHeader    IN OUT  array to modify.
* @param index      IN      position of first dropped DPoint2d.
* @param nDrop      IN      number of DPoint2ds to drop.
* @return true if operation is successful
* @group        "DPoint2d Array"
* @bsihdr                                       EarlinLutz  01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_dropRange
(
MyDPoint2dArray   *pHeader,
int                     index,
int                     nDrop
)
    {
    return pHeader != NULL
        && SUCCESS == DPoint2dArrayWrapper::dropRange (pHeader, index, nDrop);
    }


/*---------------------------------------------------------------------------------**//**
* @description Store a DPoint2d in the array at the specified index.
*
* @param pHeader    IN OUT  array to modify.
* @param pPoint     IN      DPoint2d to store.
* @param index      IN      position where the DPoint2d is stored.  A negative value
*                           indicates replacement of the current final DPoint2d.  If the
*                           index is beyond the final current DPoint2d, zeros are
*                           inserted to fill to the new index.
* @return false if the index required array expansion and the reallocation failed.
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_setDPoint2d
(
MyDPoint2dArray   *pHeader,
DPoint2d                *pPoint,
int                     index
)
    {
    return pHeader != NULL
        && SUCCESS == DPoint2dArrayWrapper::set (pHeader,  pPoint, index);
    }


/*---------------------------------------------------------------------------------**//**
* @description Add n uninitialized DPoint2ds to the array.  The array count is
*       increased by n.
* @param pHeader    IN OUT  array where new block is allocated.
* @param n          IN      number of entries requested.
* @return pointer to the block of memory in the buffer.  This pointer allows fast
*       access to the new buffer area, but becomes invalid if the buffer is reallocated.
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public DPoint2d  *EmbeddedDPoint2dArray_getBlock
(
MyDPoint2dArray   *pHeader,
int                     n
)
    {
    if (pHeader)
        return  (DPoint2d*) DPoint2dArrayWrapper::getNewBlock (pHeader, n);
    else
        return  NULL;
    }


/*---------------------------------------------------------------------------------**//**
* @description Copy multiple DPoint2ds out of the array, using an array of indices
*       to select the DPoint2ds.  Any negative index terminates copying.
* @param pHeader    IN      source array
* @param pVertex    OUT     packed output data
* @param maxVertex  IN      output array limit
* @param pIndex     IN      index array
* @param nIndex     IN      number of indices
* @return number of succesful dereferences.
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public int   EmbeddedDPoint2dArray_getIndexedDPoint2dArray
(
const   MyDPoint2dArray   *pHeader,
        DPoint2d                *pVertex,
        int                     maxVertex,
        int                     *pIndex,
        int                     nIndex
)
    {
    int n = 0;
    int status, index;
    int i;
    if (pHeader)
        {
        int maxIndex = DPoint2dArrayWrapper::getCount (pHeader);
        if (nIndex > maxVertex)
            nIndex = maxVertex;
        for (i = 0; i < nIndex; i++)
            {
            index = pIndex[i];
            if (index < 0 || index >= maxIndex)
                {
                i = nIndex;     /* force exit from loop */
                }
            else
                {
                if (SUCCESS == (status = DPoint2dArrayWrapper::get
                        (pHeader, &pVertex[i], index)))
                    n++;
                }
            }
        }
    return n;
    }


/*---------------------------------------------------------------------------------**//**
* @description Get a pointer to the contiguous buffer at specified index.  This pointer
*       may become invalid if array contents are altered.
* @param pHeader    IN      array to access.
* @param index      IN      index of array entry.  Any negative index indicates the final
*                           DPoint2d in the array.
* @return pointer to contiguous buffer (simple C array).
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public DPoint2d* EmbeddedDPoint2dArray_getPtr
(
MyDPoint2dArray   *pHeader,
int                     index
)
    {
    if (pHeader)
        return  (DPoint2d*) DPoint2dArrayWrapper::getPtr (pHeader, index);
    else
        return  NULL;
    }


/*---------------------------------------------------------------------------------**//**
* @description Get a const-qualified pointer to the contiguous buffer at specified index.
*       This pointer may become invalid if array contents are altered.
* @param pHeader    IN      array to access.
* @param index      IN      index of array entry.  Any negative index indicates the final
*                           DPoint2d in the array.
* @return pointer to contiguous buffer (simple C array).
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public const DPoint2d* EmbeddedDPoint2dArray_getConstPtr
(
const   MyDPoint2dArray   *pHeader,
        int                     index
)
    {
    if (pHeader)
        return  (const DPoint2d*) DPoint2dArrayWrapper::getConstPtr (pHeader, index);
    else
        return  NULL;
    }


/*---------------------------------------------------------------------------------**//**
* @description Swap values at designated indices in the array.
*
* @param pHeader    IN OUT  array to modify.
* @param index1     IN      index of first swap value.  Negative value indicates final
*                           current value.
* @param index2     IN      index of second swap value.  Negative value indicates final
*                           current value.
* @return true if operation is successful
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_swapValues
(
MyDPoint2dArray   *pHeader,
int                     index1,
int                     index2
)
    {
    return pHeader != NULL
        && SUCCESS == DPoint2dArrayWrapper::swapValues (pHeader, index1, index2);
    }


/*---------------------------------------------------------------------------------**//**
* @description Copy entire contents of source array to dest array.  Reuses existing
*       memory in the destination if possible.
* @param pDestHeader    OUT     destination array.
* @param pSourceHeader  IN      source array.
* @return true if operation is successful
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_copy
(
        MyDPoint2dArray   *pDestHeader,
const   MyDPoint2dArray   *pSourceHeader
)
    {
    return pDestHeader != NULL
        && pSourceHeader != NULL
        && SUCCESS == DPoint2dArrayWrapper::copy (pDestHeader, pSourceHeader);
    }

#ifdef CompileSort

/*---------------------------------------------------------------------------------**//**
* @description Sort the DPoint2ds within the array.
*
* @param pDestHeader    IN OUT  array to modify.
* @param pFunction      IN      comparison function, usual qsort convention
* @remarks This function cannot be called from MDL; instead use
    <PRE>
    ~mmdlUtil_quickSort (EmbeddedDPoint2dArray_getPtr (pDestHeader, 0),
                       EmbeddedDPoint2dArray_getCount (pDestHeader),
                       sizeof (DPoint2d), pFunction);
    </PRE>
* @group        "DPoint2d Array"
* @NoVBAWrapper
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public void  EmbeddedDPoint2dArray_sort
(
MyDPoint2dArray   *pDestHeader,
VBArray_SortFunction    pFunction
)
    {
    if (pDestHeader)
        {
        DPoint2dArrayWrapper::sort (pDestHeader, pFunction);
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @description Append a DPoint4d to the array.
*
* @param pHeader    IN OUT  array to modify.
* @param pInPoint   IN      point to append.
* @return true if point normalizes and appends to the array successfully
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_addDPoint4d
(
        MyDPoint2dArray   *pHeader,
const   DPoint4d                *pInPoint
)
    {
    DPoint3d point;
    return pHeader != NULL
        && bsiDPoint4d_normalize (pInPoint, &point)
        && EmbeddedDPoint2dArray_insertDPoint3d (pHeader, &point, -1);
    }


/*---------------------------------------------------------------------------------**//**
* @description Compute the cross product of the vectors from point 0 to point 1 and
*       point 0 to point 2.
* @param pHeader    IN      array of points
* @param pProduct   OUT     cross product vector
* @param index0     IN      reference point index
* @param index1     IN      target point of vector 1
* @param index2     IN      target point of vector 2
* @return true if all indices are valid.
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_crossProduct3Points
(
const   MyDPoint2dArray   *pHeader,
        DPoint3d                *pProduct,
        int                     index0,
        int                     index1,
        int                     index2
)
    {
    DPoint3d point0, point1, point2;

    if (   EmbeddedDPoint2dArray_getDPoint3d (pHeader, &point0, index0)
            && EmbeddedDPoint2dArray_getDPoint3d (pHeader, &point1, index1)
            && EmbeddedDPoint2dArray_getDPoint3d (pHeader, &point2, index2)
       )
        {
        bsiDPoint3d_crossProduct3DPoint3d (pProduct, &point0, &point1, &point2);
        return true;
        }
    else
        return  false;
    }

/*********************************************************************
* Functions to add, set, insert, and get data in the DPoint3d form
* to MyDPoint2dArray.
*
* If XX is the client type, the methods here are summarized by:
*       add XX                  add array of XX
*       insert XX               insert array of XX
*       get XX                  get array of XX
*       set XX
*
*********************************************************************/


/*---------------------------------------------------------------------------------**//**
* @description Append a DPoint3d to the end of the array.  The array count is increased
*       by one.
* @param pHeader    IN OUT  array to modify.
* @param pInPoint   IN      DPoint3d to append to the array.
* @return true if operation is successful
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_addDPoint3d
(
        MyDPoint2dArray   *pHeader,
const   DPoint3d                *pInPoint
)
    {
    DPoint2d fPoint;
    bsiDPoint2d_initFromDPoint3d (&fPoint, pInPoint);
    return pHeader != NULL
        && SUCCESS == DPoint2dArrayWrapper::insert(pHeader,  &fPoint, -1);
    }


/*---------------------------------------------------------------------------------**//**
* @description Append an array of DPoint3ds to the end of the array.
*
* @param pHeader        IN OUT  header of array receiving values
* @param pPointArray    IN      array of data to add
* @param n              IN      number to add.
* @return true if operation is successful
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_addDPoint3dArray
(
        MyDPoint2dArray   *pHeader,
const   DPoint3d                *pPointArray,
        int                     n
)
    {
    int i;
    for (i = 0; i < n; i++)
        {
        if (!EmbeddedDPoint2dArray_addDPoint3d (pHeader, pPointArray + i))
            return false;
        }
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @description Insert at a specified position, shifting others to higher
*       positions as needed.
* @param pHeader    IN OUT  array to modify.
* @param pInPoint   IN      points to insert.
* @param index      IN      index at which the value is to appear in the array. The special
*                           index -1 (negative one) indicates to insert at the end of the array.
* @return true if operation is successful
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_insertDPoint3d
(
        MyDPoint2dArray   *pHeader,
const   DPoint3d                *pInPoint,
        int                     index
)
    {
    DPoint2d fPoint;
    bsiDPoint2d_initFromDPoint3d (&fPoint, pInPoint);
    return pHeader != NULL
        && SUCCESS == DPoint2dArrayWrapper::insert(pHeader,  &fPoint, index);
    }


/*---------------------------------------------------------------------------------**//**
* @description Insert an array of DPoint3ds in the array, with index given for first new
*       DPoint2d.  All previous contents from that index up are moved to make room for
*       the new data.
* @param pHeader        IN OUT  header of array receiving data
* @param pPointArray    IN      array of values to add
* @param index          IN      index location for adding the array
* @param n              IN      number of values to add
* @return true if operation is successful
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_insertDPoint3dArray
(
        MyDPoint2dArray   *pHeader,
const   DPoint3d                *pPointArray,
        int                     index,
        int                     n
)
    {
    int i;
    for (i = 0; i < n; i++)
        {
        if (!EmbeddedDPoint2dArray_insertDPoint3d (pHeader, pPointArray + i, index + i))
            return false;
        }
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @description Get a DPoint3d from a specified index in the array.
*
* @param pHeader    IN      header of array to access.
* @param pPoint     OUT     point accessed from the array.
* @param index      IN      index of point to access. Any negative index indicates highest
*                           numbered element in the array.
* @return false if the index is too large, i.e., no point was accessed.
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_getDPoint3d
(
const   MyDPoint2dArray   *pHeader,
        DPoint3d                *pPoint,
        int                     index
)
    {
    DPoint2d fPoint;
    if (   pHeader != NULL
        && SUCCESS == DPoint2dArrayWrapper::get (pHeader,  &fPoint, index))
        {
        bsiDPoint3d_initFromDPoint2d (pPoint, &fPoint);
        return true;
        }
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @description Copy up to nreq DPoint3ds out of the array into a buffer.
*
* @param pHeader    IN      header of array to access.
* @param pBuffer    OUT     buffer of points
* @param pCount     OUT     number of points placed in buffer.
* @param i0         IN      index of first point to access.
* @param nreq       IN      number of points requested.
* @return true if at least one point was copied.
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_getDPoint3dArray
(
const   MyDPoint2dArray   *pHeader,
        DPoint3d                *pBuffer,
        int                     *pCount,
        int                     i0,
        int                     nreq
)
    {
    int i;
    for (i = 0; i < nreq; i++)
        {
        if (!EmbeddedDPoint2dArray_getDPoint3d (pHeader, pBuffer + i, i0 + i))
            break;
        *pCount += 1;
        }
    return *pCount > 0;
    }


/*---------------------------------------------------------------------------------**//**
* @description Store one DPoint3d in the array at specified index.
*
* @param pHeader    IN OUT  array to modify.
* @param pInPoint   IN      point to store.
* @param index      IN      position where the point is stored.  A negative indicates
*                           replacement of the highest indexed point.  If the index is
*                           beyond the highest indexed point, zeros are inserted to fill
*                           to the new index.
* @return false if the index required array expansion and the reallocation failed.
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_setDPoint3d
(
MyDPoint2dArray   *pHeader,
DPoint3d                *pInPoint,
int                     index
)
    {
    DPoint2d fPoint;
    bsiDPoint2d_initFromDPoint3d (&fPoint, pInPoint);
    return EmbeddedDPoint2dArray_setDPoint2d (pHeader, &fPoint, index);
    }

/*********************************************************************
* Functions to add, set, insert, and get data in the FPoint3d form
* to MyDPoint2dArray.
*
* If XX is the client type, the methods here are summarized by:
*       add XX                  add array of XX
*       insert XX               insert array of XX
*       get XX                  get array of XX
*       set XX
*
*********************************************************************/


/*---------------------------------------------------------------------------------**//**
* @description Append an FPoint3d to the end of the array.  The array count is increased
*       by one.
* @param pHeader    IN OUT  array to modify
* @param pInPoint   IN      FPoint3d to append to the array
* @return true if operation is successful
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_addFPoint3d
(
        MyDPoint2dArray   *pHeader,
const   FPoint3d                *pInPoint
)
    {
    DPoint2d fPoint;
    bsiDPoint2d_initFromFPoint3d (&fPoint, pInPoint);
    return pHeader != NULL
        && SUCCESS == DPoint2dArrayWrapper::insert(pHeader,  &fPoint, -1);
    }


/*---------------------------------------------------------------------------------**//**
* @description Append an array of FPoint3ds to the end of the array.
*
* @param pHeader        IN OUT  header of array receiving values
* @param pPointArray    IN      array of data to add
* @param n              IN      number to add
* @return true if operation is successful
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_addFPoint3dArray
(
        MyDPoint2dArray   *pHeader,
const   FPoint3d                *pPointArray,
        int                     n
)
    {
    int i;
    for (i = 0; i < n; i++)
        {
        if (!EmbeddedDPoint2dArray_addFPoint3d (pHeader, pPointArray + i))
            return false;
        }
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @description Insert at a specified position, shifting others to higher
*       positions as needed.
* @param pHeader    IN OUT  array to modify.
* @param pInPoint   IN      data to insert.
* @param index      IN      index at which the value is to appear in the array. The
*                           special index -1 (negative one) indicates to insert at the
*                           end of the array.
* @return true if operation is successful
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_insertFPoint3d
(
        MyDPoint2dArray   *pHeader,
const   FPoint3d                *pInPoint,
        int                     index
)
    {
    DPoint2d fPoint;
    bsiDPoint2d_initFromFPoint3d (&fPoint, pInPoint);
    return pHeader != NULL
        && SUCCESS == DPoint2dArrayWrapper::insert(pHeader,  &fPoint, index);
    }


/*---------------------------------------------------------------------------------**//**
* @description Insert an array of FPoint3ds in the array, with index given for
*       first new point.  All previous contents from that index up are moved to
*       make room for the new data.
* @param pHeader        IN OUT  header of array receiving data
* @param pPointArray    IN      array of values to add
* @param index          IN      index location for adding the array
* @param n              IN      number of values to add
* @return true if operation is successful
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_insertFPoint3dArray
(
        MyDPoint2dArray   *pHeader,
const   FPoint3d                *pPointArray,
        int                     index,
        int                     n
)
    {
    int i;
    for (i = 0; i < n; i++)
        {
        if (!EmbeddedDPoint2dArray_insertFPoint3d (pHeader, pPointArray + i, index + i))
            return false;
        }
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @description Get an FPoint3d from a specified index in the array.
*
* @param pHeader    IN      header of array to access.
* @param pPoint     OUT     FPoint3d accessed from the array.
* @param index      IN      index of FPoint3d to access. Any negative index indicates
*                           highest numbered element in the array.
* @return false if the index is too large, i.e., no FPoint3d was accessed.
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_getFPoint3d
(
const   MyDPoint2dArray   *pHeader,
        FPoint3d                *pPoint,
        int                     index
)
    {
    DPoint2d fPoint;
    if (   pHeader != NULL
        && SUCCESS == DPoint2dArrayWrapper::get (pHeader,  &fPoint, index))
        {
        bsiFPoint3d_initFromDPoint2d (pPoint, &fPoint);
        return true;
        }
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @description Copy up to nreq FPoint3ds out of the array into a buffer.
*
* @param pHeader    IN      header of array to access.
* @param pBuffer    OUT     buffer of FPoint3ds.
* @param pCount     OUT     number of FPoint3ds placed in buffer.
* @param i0         IN      index of first FPoint3d to access.
* @param nreq       IN      number of FPoint3ds requested.
* @return true if at least one FPoint3d was copied.
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_getFPoint3dArray
(
const   MyDPoint2dArray   *pHeader,
        FPoint3d                *pBuffer,
        int                     *pCount,
        int                     i0,
        int                     nreq
)
    {
    int i;
    for (i = 0; i < nreq; i++)
        {
        if (!EmbeddedDPoint2dArray_getFPoint3d (pHeader, pBuffer + i, i0 + i))
            break;
        *pCount += 1;
        }
    return *pCount > 0;
    }


/*---------------------------------------------------------------------------------**//**
* @description Store one FPoint3d in the array at specified index.
*
* @param pHeader    IN OUT  array to modify.
* @param pInPoint   IN      FPoint3d to store.
* @param index      IN      position where the FPoint3d is stored.  A negative indicates
*                           replacement of the current final FPoint3d.  If the index is
*                           beyond the final current FPoint3d, zeros are inserted to fill
*                           to the new index.
* @return false if the index required array expansion and the reallocation failed.
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_setFPoint3d
(
MyDPoint2dArray   *pHeader,
FPoint3d                *pInPoint,
int                     index
)
    {
    DPoint2d fPoint;
    bsiDPoint2d_initFromFPoint3d (&fPoint, pInPoint);
    return EmbeddedDPoint2dArray_setDPoint2d (pHeader, &fPoint, index);
    }

/*********************************************************************
* Functions to add, set, insert, and get data in the FPoint2d form
* to MyDPoint2dArray.
*
* If XX is the client type, the methods here are summarized by:
*       add XX                  add array of XX
*       insert XX               insert array of XX
*       get XX                  get array of XX
*       set XX
*
*********************************************************************/


/*---------------------------------------------------------------------------------**//**
* @description Append an FPoint2d to the end of the array.  The array count is increased
*       by one.
* @param pHeader    IN OUT  array to modify
* @param pInPoint   IN      FPoint2d to append to the array
* @return true if operation is successful
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_addFPoint2d
(
        MyDPoint2dArray   *pHeader,
const   FPoint2d                *pInPoint
)
    {
    DPoint2d fPoint;
    bsiDPoint2d_initFromFPoint2d (&fPoint, pInPoint);
    return pHeader != NULL
        && SUCCESS == DPoint2dArrayWrapper::insert(pHeader,  &fPoint, -1);
    }


/*---------------------------------------------------------------------------------**//**
* @description Append an array of FPoint2ds to the end of the array.
*
* @param pHeader        IN OUT  header of array receiving values
* @param pPointArray    IN      array of data to add
* @param n              IN      number to add
* @return true if operation is successful
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_addFPoint2dArray
(
        MyDPoint2dArray   *pHeader,
const   FPoint2d                *pPointArray,
        int                     n
)
    {
    int i;
    for (i = 0; i < n; i++)
        {
        if (!EmbeddedDPoint2dArray_addFPoint2d (pHeader, pPointArray + i))
            return false;
        }
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @description Insert at a specified position, shifting others to higher
*       positions as needed.
* @param pHeader    IN OUT  array to modify.
* @param pInPoint   IN      data to insert.
* @param index      IN      index at which the value is to appear in the array. The
*                           special index -1 (negative one) indicates to insert at the
*                           end of the array.
* @return true if operation is successful
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_insertFPoint2d
(
        MyDPoint2dArray   *pHeader,
const   FPoint2d                *pInPoint,
        int                     index
)
    {
    DPoint2d fPoint;
    bsiDPoint2d_initFromFPoint2d (&fPoint, pInPoint);
    return pHeader != NULL
        && SUCCESS == DPoint2dArrayWrapper::insert(pHeader,  &fPoint, index);
    }


/*---------------------------------------------------------------------------------**//**
* @description Insert an array of FPoint2ds in the array, with index given for
*       first new point.  All previous contents from that index up are moved to
*       make room for the new data.
* @param pHeader        IN OUT  header of array receiving data
* @param pPointArray    IN      array of values to add
* @param index          IN      index location for adding the array
* @param n              IN      number of values to add
* @return true if operation is successful
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_insertFPoint2dArray
(
        MyDPoint2dArray   *pHeader,
const   FPoint2d                *pPointArray,
        int                     index,
        int                     n
)
    {
    int i;
    for (i = 0; i < n; i++)
        {
        if (!EmbeddedDPoint2dArray_insertFPoint2d (pHeader, pPointArray + i, index + i))
            return false;
        }
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @description Get an FPoint2d from a specified index in the array.
*
* @param pHeader    IN      header of array to access.
* @param pPoint     OUT     FPoint2d accessed from the array.
* @param index      IN      index of FPoint2d to access. Any negative index indicates
*                           highest numbered element in the array.
* @return false if the index is too large, i.e., no FPoint2d was accessed.
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_getFPoint2d
(
const   MyDPoint2dArray   *pHeader,
        FPoint2d                *pPoint,
        int                     index
)
    {
    DPoint2d fPoint;
    if (   pHeader != NULL
        && SUCCESS == DPoint2dArrayWrapper::get (pHeader,  &fPoint, index))
        {
        bsiFPoint2d_initFromDPoint2d (pPoint, &fPoint);
        return true;
        }
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @description Copy up to nreq FPoint2ds out of the array into a buffer.
*
* @param pHeader    IN      header of array to access.
* @param pBuffer    OUT     buffer of FPoint2ds.
* @param pCount     OUT     number of FPoint2ds placed in buffer.
* @param i0         IN      index of first FPoint2d to access.
* @param nreq       IN      number of FPoint2ds requested.
* @return true if at least one FPoint2d was copied.
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_getFPoint2dArray
(
const   MyDPoint2dArray   *pHeader,
        FPoint2d                *pBuffer,
        int                     *pCount,
        int                     i0,
        int                     nreq
)
    {
    int i;
    for (i = 0; i < nreq; i++)
        {
        if (!EmbeddedDPoint2dArray_getFPoint2d (pHeader, pBuffer + i, i0 + i))
            break;
        *pCount += 1;
        }
    return *pCount > 0;
    }


/*---------------------------------------------------------------------------------**//**
* @description Store one FPoint2d in the array at specified index.
*
* @param pHeader    IN OUT  array to modify.
* @param pInPoint   IN      FPoint2d to store.
* @param index      IN      position where the FPoint2d is stored.  A negative indicates
*                           replacement of the current final FPoint2d.  If the index is
*                           beyond the final current FPoint2d, zeros are inserted to fill
*                           to the new index.
* @return false if the index required array expansion and the reallocation failed.
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_setFPoint2d
(
MyDPoint2dArray   *pHeader,
FPoint2d                *pInPoint,
int                     index
)
    {
    DPoint2d fPoint;
    bsiDPoint2d_initFromFPoint2d (&fPoint, pInPoint);
    return EmbeddedDPoint2dArray_setDPoint2d (pHeader, &fPoint, index);
    }


/*---------------------------------------------------------------------------------**//**
* @description Return the range of the points in the array.
*
* @param pHeader    IN      array to examine.
* @param pRange     OUT     data range.
* @return false if the array is empty (has no range).
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_getDRange3d
(
const   MyDPoint2dArray   *pHeader,
        DRange3d                *pRange
)
    {
    int numPoint, i;
    const DPoint2d *pSourceBuffer;
    bool    boolstat = false;
    bsiDRange3d_init (pRange);
    if (    pHeader
        &&  (numPoint = DPoint2dArrayWrapper::getCount (pHeader)) > 0
        &&  NULL != (pSourceBuffer = (const DPoint2d *)DPoint2dArrayWrapper::getConstPtr (pHeader, 0))
        )
        {
        for (i = 0; i < numPoint; i++)
            {
            bsiDRange3d_extendByComponents
                        (
                        pRange,
                        pSourceBuffer[i].x,
                        pSourceBuffer[i].y,
                        0.0
                        );
            }
        boolstat = true;
        }
    return boolstat;
    }


/*---------------------------------------------------------------------------------**//**
* @description Return the range of the points in the array.
*
* @param pHeader    IN      array to examine.
* @param pRange     OUT     data range.
* @return false if the array is empty (has no range).
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedDPoint2dArray_getFRange3d
(
const   MyDPoint2dArray   *pHeader,
        FRange3d                *pRange
)
    {
    DRange3d range;
    if (EmbeddedDPoint2dArray_getDRange3d (pHeader, &range))
        {
        bsiFRange3d_initFromDRange3d (pRange, &range);
        return true;
        }
    else
        {
        bsiFRange3d_init (pRange);
        return false;
        }
    }


/*---------------------------------------------------------------------------------**//**
* @description Multiply by a transformation, return results in place.
*
* @param pHeader    IN OUT  array to transform.
* @param pTransform IN      transformation.
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public void  EmbeddedDPoint2dArray_multiplyByDTransform3dInPlace
(
        MyDPoint2dArray   *pHeader,
const   DTransform3d            *pTransform
)
    {
    int numPoint;
    DPoint2d *pSourceBuffer;
    if (    pHeader
        &&  (numPoint = DPoint2dArrayWrapper::getCount (pHeader)) > 0
        &&  NULL != (pSourceBuffer = (DPoint2d *)DPoint2dArrayWrapper::getPtr (pHeader, 0))
        )
        {
        bsiDTransform3d_multiplyDPoint2dArray (pTransform,
                    pSourceBuffer, pSourceBuffer, numPoint);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @description Multiply by a transformation, return results in output array.
*
* @param pDestHeader    OUT     destination array.
* @param pSourceHeader  IN      source array.
* @param pTransform     IN      transformation.
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public void  EmbeddedDPoint2dArray_multiplyByDTransform3d
(
        MyDPoint2dArray   *pDestHeader,
const   MyDPoint2dArray   *pSourceHeader,
const   DTransform3d            *pTransform
)
    {
    int numPoint;
    const DPoint2d *pSourceBuffer;
    DPoint2d *pDestBuffer;

    if (    pSourceHeader
        &&  pDestHeader
        &&  (numPoint = DPoint2dArrayWrapper::getCount (pSourceHeader)) > 0
        &&  NULL != (pSourceBuffer = (DPoint2d *)DPoint2dArrayWrapper::getConstPtr
                                                    (pSourceHeader, 0))
        )
        {
        DPoint2dArrayWrapper::empty (pDestHeader);
        if (NULL != (pDestBuffer = (DPoint2d *)DPoint2dArrayWrapper::getNewBlock
                                                (pDestHeader, numPoint)))
            {
            bsiDTransform3d_multiplyDPoint2dArray (pTransform,
                                pDestBuffer, pSourceBuffer, numPoint);
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* @description Multiply by a matrix, return results in place.
*
* @param pHeader    IN OUT  array to transform.
* @param pMatrix    IN      matrix.
* @group        "DPoint2d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public void  EmbeddedDPoint2dArray_multiplyByDMatrix3dInPlace
(
        MyDPoint2dArray   *pHeader,
const   DMatrix3d               *pMatrix
)
    {
    int numPoint;
    DPoint2d *pSourceBuffer;
    if (    pHeader
        &&  (numPoint = DPoint2dArrayWrapper::getCount (pHeader)) > 0
        &&  NULL != (pSourceBuffer = (DPoint2d *)DPoint2dArrayWrapper::getPtr (pHeader, 0))
        )
        {
        bsiDMatrix3d_multiplyDPoint2dArray (pMatrix,
                    pSourceBuffer, pSourceBuffer, numPoint);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description Reverse the contents of the array in place.
* @param pHeader    IN OUT  array to reverse
* @group        "DPoint2d Array"
* @bsimethod                                                    DavidAssaf      02/05
+---------------+---------------+---------------+---------------+---------------+------*/
Public void  EmbeddedDPoint2dArray_reverse
(
MyDPoint2dArray*  pHeader
)
    {
    int i, numPoint;

    if (pHeader && 1 < (numPoint = DPoint2dArrayWrapper::getCount (pHeader)))
        {
        for (i = 0; i < numPoint / 2; i++)
            DPoint2dArrayWrapper::swapValues (pHeader, i, numPoint - 1 - i);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description Trim the array to the smaller of given count or current size.
* @param pHeader    IN OUT  array to trim
* @param count      IN      number of points in the output array
* @group        "DPoint2d Array"
* @bsimethod                                                    DavidAssaf      03/06
+---------------+---------------+---------------+---------------+---------------+------*/
Public void EmbeddedDPoint2dArray_trim
(
MyDPoint2dArray *pHeader,
int count
)
    {
    DPoint2dArrayWrapper::trim (pHeader, count);
    }

END_BENTLEY_NAMESPACE
