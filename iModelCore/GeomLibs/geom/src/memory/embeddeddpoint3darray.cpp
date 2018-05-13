/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/memory/embeddeddpoint3darray.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
//#include "msvarray.fdf"
#include "../gpa/ArrayWrapper.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

typedef VArrayWrapper<DPoint3d> DPoint3dArrayWrapper;


/*---------------------------------------------------------------------------------**//**
* @description Allocate a new EmbeddedDPoint3dArray header from the system heap.
*
* @return pointer to the header.
* @DefaultRequiredLibrary mtg.lib
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP EmbeddedDPoint3dArray *jmdlEmbeddedDPoint3dArray_new
(
void
)
    {
    return new EmbeddedDPoint3dArray ();
    }


/*---------------------------------------------------------------------------------**//**
* @description Initialize an EmbeddedDPoint3dArray header.  Prior contents are
*       destroyed.   Intended for use immediately following uninitialized creation
*       operation such as (a) local variable declaration or (b) allocation from system
*       heap.
* @param pHeader    OUT     array to initialize.
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void  jmdlEmbeddedDPoint3dArray_init
(
EmbeddedDPoint3dArray   *pHeader
)
    {
    if (pHeader)
        pHeader->clear ();
    }


/*---------------------------------------------------------------------------------**//**
* @description Return both the header and its associated memory to the system heap.
*       This should only be used for a header originally allocated via
*       ~mEmbeddedDPoint3dArray_new.  Headers allocated as locals should be
*       decommissioned via ~mEmbeddedDPoint3dArray_releaseMem.
* @param pHeader    IN OUT  array to be freed.
* @return Always returns NULL.
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP EmbeddedDPoint3dArray *jmdlEmbeddedDPoint3dArray_free
(
EmbeddedDPoint3dArray *pHeader
)
    {
    if (pHeader)
        delete pHeader;
    return NULL;
    }


/*---------------------------------------------------------------------------------**//**
* @description Reduce the count (number of DPoint3ds) in the array to zero.
*       Existing memory is retained so the array can be refilled to its prior
*       size without requiring reallocation.
* @param pHeader    IN OUT  array to modify
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void  jmdlEmbeddedDPoint3dArray_empty
(
EmbeddedDPoint3dArray *pHeader
)
    {
    if (pHeader)
        pHeader->clear ();
    }


/*---------------------------------------------------------------------------------**//**
* @description Release all memory attached to the header, and reinitialize the header
*       as an empty array with no buffer.
* @param pHeader    IN OUT  array to empty
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void  jmdlEmbeddedDPoint3dArray_releaseMem
(
EmbeddedDPoint3dArray *pHeader
)
    {
    if (pHeader)
        pHeader->clear ();
    }


/*---------------------------------------------------------------------------------**//**
* @description Grab (borrow) an array from the cache.  Caller is responsible
*       for using ~mEmbeddedDPoint3dArray_drop to return the array to the cache when
*       finished.   Controlled "grab and drop" of cache arrays is faster than using
*       either local variables (~mEmbeddedDPoint3dArray_init and
*       ~mEmbeddedDPoint3dArray_releaseMem) or heap allocation
*       (~mEmbeddedDPoint3dArray_new and ~mEmbeddedDPoint3dArray_free)
*       because the preallocated variable size parts of cached arrays are immediately
*       available without revisiting the system cache.
* @return An array header obtained from the cache.
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP EmbeddedDPoint3dArray *jmdlEmbeddedDPoint3dArray_grab
(
void
)
    {
    return new EmbeddedDPoint3dArray ();
    }


/*---------------------------------------------------------------------------------**//**
* @description Drop (return) an array to the cache.  Use this to dispose of arrays
*       borrowed with ~mEmbeddedDPoint3dArray_grab.
* @param pHeader    IN      pointer to array to return to cache.
* @return always returns NULL.
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP EmbeddedDPoint3dArray *jmdlEmbeddedDPoint3dArray_drop
(
EmbeddedDPoint3dArray     *pHeader
)
    {
    if (pHeader)
        delete pHeader;
    return NULL;
    }


/*---------------------------------------------------------------------------------**//**
* @description Swap the contents (counts and associated memory) of two headers.
*
* @param pHeader0   IN OUT  first array header
* @param pHeader1   IN OUT  second array header
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void  jmdlEmbeddedDPoint3dArray_swapContents
(
EmbeddedDPoint3dArray   *pHeader0,
EmbeddedDPoint3dArray   *pHeader1
)
    {
    EmbeddedDPoint3dArray scratchHeader = *pHeader0;
    *pHeader0 = *pHeader1;
    *pHeader1 = scratchHeader;
    }


/*---------------------------------------------------------------------------------**//**
* @description Ensure the buffer has capacity for n DPoint3ds without
*       reallocation. The count of DPoint3ds in the buffer remains unchanged.
* @param pHeader    IN OUT  array to modify.
* @param n          IN      number of DPoint3ds in buffer.
* @return false if unable to allocate the buffer.
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_ensureCapacity
(
EmbeddedDPoint3dArray   *pHeader,
int                     n
)
    {
    if (pHeader)
        pHeader->reserve(n);
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @description Reallocate the buffer to accommodate exactly n DPoint3ds
*       NOTE: this will truncate the contents of this instance if its count is
*       greater than n.
* @param pHeader    IN OUT  array to modify.
* @param n          IN      number of values to accommodate, no more, no less.
* @return false if unable to reallocate the buffer.
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_setExactBufferSize
(
EmbeddedDPoint3dArray   *pHeader,
int                     n
)
    {
    if (pHeader)
        pHeader->resize (n);
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @description Return the number of DPoint3ds in the array.
*
* @param pHeader    IN      array to query.
* @return array count
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int   jmdlEmbeddedDPoint3dArray_getCount
(
const   EmbeddedDPoint3dArray   *pHeader
)
    {
    if (!pHeader)
        return 0;
    return (int) pHeader->size ();
    }


/*---------------------------------------------------------------------------------**//**
* @description Append a DPoint3d to the end of the array.  The array count is increased
*       by one.
* @param pHeader    IN OUT  array to modify.
* @param pPoint     IN      DPoint3d to append to the array.
* @return true if operation is successful
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_addDPoint3d
(
        EmbeddedDPoint3dArray   *pHeader,
const   DPoint3d                *pPoint
)
    {
    if (pHeader)
        {
        pHeader->push_back (*pPoint);
        return true;
        }
    return false;
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
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_insertDPoint3d
(
        EmbeddedDPoint3dArray   *pHeader,
const   DPoint3d                *pPoint,
        int                     index
)
    {
    return SUCCESS == DPoint3dArrayWrapper::insert (pHeader, pPoint, index);
    }


/*---------------------------------------------------------------------------------**//**
* @description Append an array of DPoint3d to the end of the array.
*
* @param pHeader    IN OUT  header of array receiving values
* @param pPoint     IN      array of data to add
* @param n          IN      number to add.
* @return true if operation is successful
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_addDPoint3dArray
(
        EmbeddedDPoint3dArray   *pHeader,
const   DPoint3d                *pPoint,
        int                     n
)
    {
    return pHeader != NULL
            && SUCCESS == DPoint3dArrayWrapper::insert (pHeader, pPoint, -1, n);
    }


/*---------------------------------------------------------------------------------**//**
* @description Insert an array of DPoint3ds in the array, with index given for
*       first new DPoint3d.  All previous contents from that index up are moved to
*       make room for the new data.
* @param pHeader    IN OUT  header of array receiving data
* @param pPoint     IN      array of values to add
* @param index      IN      index location for adding the array
* @param n          IN      number of values to add
* @return true if operation is successful
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_insertDPoint3dArray
(
        EmbeddedDPoint3dArray   *pHeader,
const   DPoint3d                *pPoint,
        int                     index,
        int                     n
)
    {
    return SUCCESS == DPoint3dArrayWrapper::insert (pHeader, pPoint, index, n);
    }


/*---------------------------------------------------------------------------------**//**
* @description Copy up to nreq DPoint3ds out of the array into a buffer.
*
* @param pHeader    IN      header of array to access.
* @param pBuffer    OUT     buffer of DPoint3ds.
* @param nGot       OUT     number of DPoint3ds placed in buffer.
* @param i0         IN      index of first DPoint3d to access.
* @param nreq       IN      number of DPoint3ds requested.
* @return true if at least one DPoint3d was copied.
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_getDPoint3dArray
(
const   EmbeddedDPoint3dArray   *pHeader,
        DPoint3d                *pBuffer,
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
    *nGot = DPoint3dArrayWrapper::getArray(pHeader, pBuffer, i0, nreq);
    return *nGot > 0;
    }


/*---------------------------------------------------------------------------------**//**
* @description Get a DPoint3d from a specified index in the array.
*
* @param pHeader    IN      header of array to access.
* @param pPoint     OUT     DPoint3d accessed from the array.
* @param index      IN      index of DPoint3d to access. Any negative index indicates
*                           highest numbered element in the array.
* @return false if the index is too large, i.e., no DPoint3d was accessed.
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_getDPoint3d
(
const   EmbeddedDPoint3dArray   *pHeader,
        DPoint3d                *pPoint,
        int                     index
)
    {
    return pHeader != NULL
        && SUCCESS == DPoint3dArrayWrapper::get (pHeader, pPoint, index);
    }


/*---------------------------------------------------------------------------------**//**
* @description Drop a contiguous block of DPoint3ds.  Copy higher indices back down.
*
* @param pHeader    IN OUT  array to modify.
* @param index      IN      position of first dropped DPoint3d.
* @param nDrop      IN      number of DPoint3ds to drop.
* @return true if operation is successful
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_dropRange
(
EmbeddedDPoint3dArray   *pHeader,
int                     index,
int                     nDrop
)
    {
    return pHeader != NULL
        && SUCCESS == DPoint3dArrayWrapper::dropRange (pHeader, index, nDrop);
    }



/*---------------------------------------------------------------------------------**//**
* @description Store a DPoint3d in the array at the specified index.
*
* @param pHeader    IN OUT  array to modify.
* @param pPoint     IN      DPoint3d to store.
* @param index      IN      position where the DPoint3d is stored.  A negative
*                           indicates replacement of the current final DPoint3d.  If the
*                           index is beyond the final current DPoint3d, zeros are
*                           inserted to fill to the new index.
* @return false if the index required array expansion and the reallocation failed.
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_setDPoint3d
(
EmbeddedDPoint3dArray   *pHeader,
DPoint3dCP              pPoint,
int                     index
)
    {
    return pHeader != NULL
        && SUCCESS == DPoint3dArrayWrapper::set (pHeader, pPoint, index);
    }


/*---------------------------------------------------------------------------------**//**
* @description Add n uninitialized DPoint3ds to the array.  The array count is
*       increased by n.
* @param pHeader    IN OUT  array where new block is allocated.
* @param n          IN      number of entries requested.
* @return pointer to the block of memory in the buffer.  This pointer allows fast
*       access to the new buffer area, but becomes invalid if the buffer is reallocated.
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP DPoint3d  *jmdlEmbeddedDPoint3dArray_getBlock
(
EmbeddedDPoint3dArray   *pHeader,
int                     n
)
    {
    return DPoint3dArrayWrapper::getNewBlock (pHeader, n);
    }


/*---------------------------------------------------------------------------------**//**
* @description Copy multiple DPoint3ds out of the array, using an array of indices
*       to select the DPoint3ds.  Any negative index terminates copying.
* @param pHeader    IN      source array
* @param pOut       OUT     packed output data
* @param maxOut     IN      output array limit
* @param nIndex     IN      number of indices
* @param pIndex     IN      index array
* @return number of succesful dereferences.
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int   jmdlEmbeddedDPoint3dArray_getIndexedDPoint3dArray
(
const   EmbeddedDPoint3dArray   *pHeader,
        DPoint3d                *pOut,
        int                     maxOut,
        int                     *pIndex,
        int                     nIndex
)
    {
    int n = 0;
    int index;
    int i;
    if (pHeader)
        {
        size_t maxIndex = DPoint3dArrayWrapper::getCount (pHeader);
        if (nIndex > maxOut)
            nIndex = maxOut;
        for (i = 0; i < nIndex; i++)
            {
            index = pIndex[i];
            if (index < 0 || (size_t)index >= maxIndex)
                {
                i = nIndex;     /* force exit from loop */
                }
            else
                {
                pOut[i] = pHeader->at((size_t)index);
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
*                           DPoint3d in the array.
* @return pointer to contiguous buffer (simple C array).
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP DPoint3d* jmdlEmbeddedDPoint3dArray_getPtr
(
EmbeddedDPoint3dArray   *pHeader,
int                     index
)
    {
    return DPoint3dArrayWrapper::getPtr (pHeader, index);
    }


/*---------------------------------------------------------------------------------**//**
* @description Get a const-qualified pointer to the contiguous buffer at specified index.
*       This pointer may become invalid if array contents are altered.
* @param pHeader    IN      array to access.
* @param index      IN      index of array entry.  Any negative index indicates the final
*                           DPoint3d in the array.
* @return pointer to contiguous buffer (simple C array).
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP const DPoint3d*   jmdlEmbeddedDPoint3dArray_getConstPtr
(
const   EmbeddedDPoint3dArray   *pHeader,
        int                     index
)
    {
    return DPoint3dArrayWrapper::getConstPtr (pHeader, index);
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
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_swapValues
(
EmbeddedDPoint3dArray   *pHeader,
int                     index1,
int                     index2
)
    {
    return pHeader != NULL
        && SUCCESS == DPoint3dArrayWrapper::swapValues (pHeader, index1, index2);
    }


/*---------------------------------------------------------------------------------**//**
* @description Copy entire contents of source array to dest array.  Reusues existing
*       memory in the destination if possible.
* @param pDestHeader    OUT     destination array.
* @param pSourceHeader  IN      source array.
* @return true if operation is successful
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_copy
(
        EmbeddedDPoint3dArray   *pDestHeader,
const   EmbeddedDPoint3dArray   *pSourceHeader
)
    {
    return DPoint3dArrayWrapper::booleanCopy (pDestHeader, pSourceHeader);
    }

static VBArray_SortFunction s_sortFunc;
static bool sortFuncWrapper (const DPoint3d & point0, const DPoint3d &point1)
    {
    return s_sortFunc (&point0, &point1) < 0;
    }

/*---------------------------------------------------------------------------------**//**
* @description Sort the DPoint3ds within the array.
*
* @param pDestHeader    IN OUT  array to sort
* @param pFunction      IN      comparison function, usual qsort convention
* @remarks This function cannot be called from MDL; instead use
    <PRE>
    ~mmdlUtil_quickSort (jmdlEmbeddedDPoint3dArray_getPtr (pDestHeader, 0),
                       jmdlEmbeddedDPoint3dArray_getCount (pDestHeader),
                       sizeof (DPoint3d), pFunction);
    </PRE>
* @group        "DPoint3d Array"
* @NoVBAWrapper
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void  jmdlEmbeddedDPoint3dArray_sort
(
EmbeddedDPoint3dArray   *pDestHeader,
VBArray_SortFunction    pFunction
)
    {
    if (pDestHeader)
        {
        s_sortFunc = pFunction;
        std::sort (pDestHeader->begin (), pDestHeader->end (), sortFuncWrapper);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @description Append a DPoint4d to the array.
*
* @param pHeader    IN OUT  array to modify.
* @param pInPoint   IN      point to append.
* @return true if point normalizes and appends to the array successfully
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_addDPoint4d
(
        EmbeddedDPoint3dArray   *pHeader,
const   DPoint4d                *pInPoint
)
    {
    DPoint3d point;
    return pHeader != NULL
        && bsiDPoint4d_normalize (pInPoint, &point)
        && jmdlEmbeddedDPoint3dArray_insertDPoint3d (pHeader, &point, -1);
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
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_crossProduct3Points
(
const   EmbeddedDPoint3dArray   *pHeader,
        DPoint3d                *pProduct,
        int                     index0,
        int                     index1,
        int                     index2
)
    {
    DPoint3d point0, point1, point2;

    if (   jmdlEmbeddedDPoint3dArray_getDPoint3d (pHeader, &point0, index0)
            && jmdlEmbeddedDPoint3dArray_getDPoint3d (pHeader, &point1, index1)
            && jmdlEmbeddedDPoint3dArray_getDPoint3d (pHeader, &point2, index2)
       )
        {
        bsiDPoint3d_crossProduct3DPoint3d (pProduct, &point0, &point1, &point2);
        return true;
        }
    else
        return  false;
    }

/*********************************************************************
* Functions to add, set, insert, and get data in the DPoint2d form
* to EmbeddedDPoint3dArray.
*
* If XX is the client type, the methods here are summarized by:
*       add XX                  add array of XX
*       insert XX               insert array of XX
*       get XX                  get array of XX
*       set XX
*
*********************************************************************/

#ifdef Compile_addDPoint2d
/*---------------------------------------------------------------------------------**//**
* @description Append a DPoint2d to the end of the array.  The array count is increased
*       by one.
* @param pHeader    IN OUT  array to modify.
* @param pInPoint   IN      DPoint2d to append to the array.
* @return true if operation is successful
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_addDPoint2d
(
        EmbeddedDPoint3dArray   *pHeader,
const   DPoint2d                *pInPoint
)
    {
    DPoint3d fPoint;
    bsiDPoint3d_initFromDPoint2d (&fPoint, pInPoint);
    return pHeader != NULL
        && SUCCESS == DPoint3dArrayWrapper::insert (pHeader, &fPoint, -1);
    }


/*---------------------------------------------------------------------------------**//**
* @description Append an array of DPoint2ds to the end of the array.
*
* @param pHeader        IN OUT  header of array receiving values
* @param pPointArray    IN      array of data to add
* @param n              IN      number to add.
* @return true if operation is successful
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_addDPoint2dArray
(
        EmbeddedDPoint3dArray   *pHeader,
const   DPoint2d                *pPointArray,
        int                     n
)
    {
    int i;
    for (i = 0; i < n; i++)
        {
        if (!jmdlEmbeddedDPoint3dArray_addDPoint2d (pHeader, pPointArray + i))
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
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_insertDPoint2d
(
        EmbeddedDPoint3dArray   *pHeader,
const   DPoint2d                *pInPoint,
        int                     index
)
    {
    DPoint3d fPoint;
    bsiDPoint3d_initFromDPoint2d (&fPoint, pInPoint);
    return pHeader != NULL
        && SUCCESS == DPoint3dArrayWrapper::insert(pHeader, &fPoint, index);
    }


/*---------------------------------------------------------------------------------**//**
* @description Insert an array of DPoint2ds in the array, with index given for first new
*       DPoint3d.  All previous contents from that index up are moved to make room for
*       the new data.
* @param pHeader        IN OUT  header of array receiving data
* @param pPointArray    IN      array of values to add
* @param index          IN      index location for adding the array
* @param n              IN      number of values to add
* @return true if operation is successful
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_insertDPoint2dArray
(
        EmbeddedDPoint3dArray   *pHeader,
const   DPoint2d                *pPointArray,
        int                     index,
        int                     n
)
    {
    int i;
    for (i = 0; i < n; i++)
        {
        if (!jmdlEmbeddedDPoint3dArray_insertDPoint2d (pHeader, pPointArray + i, index + i))
            return false;
        }
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @description Get a DPoint2d from a specified index in the array.
*
* @param pHeader    IN      header of array to access.
* @param pPoint     OUT     point accessed from the array.
* @param index      IN      index of point to access. Any negative index indicates highest
*                           numbered element in the array.
* @return false if the index is too large, i.e., no point was accessed.
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_getDPoint2d
(
const   EmbeddedDPoint3dArray   *pHeader,
        DPoint2d                *pPoint,
        int                     index
)
    {
    DPoint3d fPoint;
    if (   pHeader != NULL
        && SUCCESS == DPoint3dArrayWrapper::get (pHeader, &fPoint, index))
        {
        bsiDPoint2d_initFromDPoint3d (pPoint, &fPoint);
        return true;
        }
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @description Copy up to nreq DPoint2ds out of the array into a buffer.
*
* @param pHeader    IN      header of array to access.
* @param pBuffer    OUT     buffer of points
* @param pCount     OUT     number of points placed in buffer.
* @param i0         IN      index of first point to access.
* @param nreq       IN      number of points requested.
* @return true if at least one point was copied.
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_getDPoint2dArray
(
const   EmbeddedDPoint3dArray   *pHeader,
        DPoint2d                *pBuffer,
        int                     *pCount,
        int                     i0,
        int                     nreq
)
    {
    int i;
    for (i = 0; i < nreq; i++)
        {
        if (!jmdlEmbeddedDPoint3dArray_getDPoint2d (pHeader, pBuffer + i, i0 + i))
            break;
        *pCount += 1;
        }
    return *pCount > 0;
    }


/*---------------------------------------------------------------------------------**//**
* @description Store one DPoint2d in the array at specified index.
*
* @param pHeader    IN OUT  array to modify.
* @param pInPoint   IN      point to store.
* @param index      IN      position where the point is stored.  A negative indicates
*                           replacement of the highest indexed point.  If the index is
*                           beyond the highest indexed point, zeros are inserted to fill
*                           to the new index.
* @return false if the index required array expansion and the reallocation failed.
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_setDPoint2d
(
EmbeddedDPoint3dArray   *pHeader,
DPoint2dCP              pInPoint,
int                     index
)
    {
    DPoint3d fPoint;
    bsiDPoint3d_initFromDPoint2d (&fPoint, pInPoint);
    return jmdlEmbeddedDPoint3dArray_setDPoint3d (pHeader, &fPoint, index);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @description Return the range of the points in the array.
*
* @param pHeader    IN      array to examine.
* @param pRange     OUT     data range.
* @return false if the array is empty (has no range).
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_getDRange3d
(
const   EmbeddedDPoint3dArray   *pHeader,
        DRange3d                *pRange
)
    {
    bool    boolstat = false;
    size_t numPoint;
    if (pHeader &&
        (numPoint = pHeader->size ()) > 0)
        {
        bsiDRange3d_initFromArray (
                pRange,
                DPoint3dArrayWrapper::getConstPtr (pHeader, 0),
                (int)numPoint
                );
        boolstat = true;
        }
    return boolstat;
    }


/*---------------------------------------------------------------------------------**//**
* @description Return the absolute value of the largest coordinate or coordinate difference found
*       in the array.
* @param pHeader    IN      array to examine.
* @return largest coordinate/coordinate difference, 0 if empty array.
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double    jmdlEmbeddedDPoint3dArray_maxAbs
(
const   EmbeddedDPoint3dArray   *pHeader
)
    {
    return (pHeader)
            ? bsiDPoint3d_getLargestCoordinate
                (
                DPoint3dArrayWrapper::getConstPtr (pHeader, 0),
                (int)pHeader->size ()
                )
            : 0.0;
    }


/*---------------------------------------------------------------------------------**//**
* @description Return the absolute value of the largest coordinate difference found
*       in the array.
* @param pHeader    IN      array to examine.
* @return largest coordinate difference, 0 if empty array.
* @group        "DPoint3d Array"
* @bsimethod                                    DavidAssaf      06/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double    jmdlEmbeddedDPoint3dArray_maxAbsDifference
(
const   EmbeddedDPoint3dArray   *pHeader
)
    {
    return (pHeader)
            ? bsiDPoint3d_getLargestCoordinateDifference
                (
                DPoint3dArrayWrapper::getConstPtr (pHeader, 0),
                (int)pHeader->size ()
                )
            : 0.0;
    }

/*-----------------------------------------------------------------*//**
* @description multiply each point by a Transform.
* @param pHeader    IN OUT  array to transform
* @param pTransform IN      transform to apply
* @group        "DPoint3d Array"
* @bsihdr                                       EarlinLutz      02/02
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void  jmdlEmbeddedDPoint3dArray_multiplyByTransformInPlace
(
        EmbeddedDPoint3dArray   *pHeader,
const   Transform               *pTransform
)
    {
    DPoint3d *pBuffer;
    int count;
    if (pHeader)
        {
        pBuffer = jmdlEmbeddedDPoint3dArray_getPtr (pHeader, 0);
        count = jmdlEmbeddedDPoint3dArray_getCount (pHeader);
        bsiTransform_multiplyDPoint3dArrayInPlace (pTransform, pBuffer, count);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description Reverse the contents of the array in place.
* @param pHeader    IN OUT  array to reverse
* @group        "DPoint3d Array"
* @bsimethod                                                    DavidAssaf      02/05
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void  jmdlEmbeddedDPoint3dArray_reverse
(
EmbeddedDPoint3dArray*  pHeader
)
    {
    DPoint3dArrayWrapper::reverse (pHeader);
    }


/*---------------------------------------------------------------------------------**//**
* @description For each individual line segment in a linestring, add both start and end points
*       to the embedded array.   Disconnects are permitted.
*
* @param pHeader    IN OUT array to receive segments.
* @param pXYZ IN array of points.
* @param numXYZ IN number of points.
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlEmbeddedDPoint3dArray_addDPoint3dLinestringAsSegments
(
EmbeddedDPoint3dArray* pHeader,
DPoint3d const  *      pXYZ,
int                     numXYZ
)
    {
    int i;
    bool    bPreviousPointIsDisconnect = true;
    for (i = 0; i < numXYZ; i++)
        {
        if (bsiDPoint3d_isDisconnect (&pXYZ[i]))
            {
            bPreviousPointIsDisconnect = true;
            }
        else
            {
            if (!bPreviousPointIsDisconnect)
                jmdlEmbeddedDPoint3dArray_addDPoint3d (pHeader, &pXYZ[i - 1]);
                jmdlEmbeddedDPoint3dArray_addDPoint3d (pHeader, &pXYZ[i]);
            bPreviousPointIsDisconnect = false;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description Add a disconnect to the array.
* @param pHeader    IN OUT array to receive disconnect.
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlEmbeddedDPoint3dArray_addDisconnect
(
EmbeddedDPoint3dArray* pHeader
)
    {
        DPoint3d xyz;
        xyz.x = xyz.y = xyz.z = DISCONNECT;
        jmdlEmbeddedDPoint3dArray_addDPoint3d (pHeader, &xyz);
    }

/*---------------------------------------------------------------------------------**//**
* @description Trim the array to the smaller of given count or current size.
* @param pHeader    IN OUT  array to trim
* @param count      IN      number of points in the output array
* @group        "DPoint3d Array"
* @bsimethod                                                    DavidAssaf      03/06
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlEmbeddedDPoint3dArray_trim
(
EmbeddedDPoint3dArray *pHeader,
int count
)
    {
    DPoint3dArrayWrapper::trim (pHeader, count);
    }



/*---------------------------------------------------------------------------------**//**
* @description Count the number of points before disconnect or end of array.
* @param pHeader    IN subject array
* @param index0  IN first point index.
* @return number of non-disconnects.
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlEmbeddedDPoint3dArray_countToEndOrDisconnect
(
EmbeddedDPoint3dArray const * pHeader,
int                     index0
)
    {
    int nout = 0;
    int n = jmdlEmbeddedDPoint3dArray_getCount (pHeader);
    DPoint3dCP pBuffer = jmdlEmbeddedDPoint3dArray_getConstPtr (pHeader, 0);
    int i;
    for (i = index0; i < n && !bsiDPoint3d_isDisconnect (&pBuffer[i]); i++)
        {
        nout++;
        }
    return nout;
    }



/*---------------------------------------------------------------------------------**//**
* @description Compute the centroid and normal of the polygon from index0 through disconnect or end of array.
* @param pHeader    IN subject array
* @param index0  IN first point index.
* @param pCentroid OUT computed centroid
* @param pArea OUT computed area
* @param pNormal OUT computed normal
* @return number of points used.  0 if polygon calculation failed.
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int        jmdlEmbeddedDPoint3dArray_areaPropertiesToDisconnect
(
EmbeddedDPoint3dArray const * pHeader,
int                   index0,
DPoint3dP pCentroid,
DVec3dP   pNormal,
double    *pArea
)
    {
    int numToDisconnect = jmdlEmbeddedDPoint3dArray_countToEndOrDisconnect (pHeader, index0);
    DPoint3dCP pBuffer = jmdlEmbeddedDPoint3dArray_getConstPtr (pHeader, 0);
    if (bsiPolygon_centroidAreaPerimeter ((DPoint3d*)(pBuffer + index0), numToDisconnect, pCentroid, pNormal, pArea, NULL, NULL))
        return numToDisconnect;
    return 0;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
