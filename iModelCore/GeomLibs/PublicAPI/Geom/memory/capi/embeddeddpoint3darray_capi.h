/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/**
@DocText
@Group "DPoint3d Array"
<P>See the summary for ~s"Embedded Arrays".
*/

/*---------------------------------------------------------------------------------**//**
* @description Allocate a new EmbeddedDPoint3dArray header from the system heap.
*
* @return pointer to the header.
* @DefaultRequiredLibrary mtg.lib
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP EmbeddedDPoint3dArray *jmdlEmbeddedDPoint3dArray_new
(
void
);

/*---------------------------------------------------------------------------------**//**
* @description Initialize an EmbeddedDPoint3dArray header.  Prior contents are
*       destroyed.   Intended for use immediately following uninitialized creation
*       operation such as (a) local variable declaration or (b) allocation from system
*       heap.
* @param pHeader    OUT     array to initialize.
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void  jmdlEmbeddedDPoint3dArray_init
(
EmbeddedDPoint3dArray   *pHeader
);

/*---------------------------------------------------------------------------------**//**
* @description Return both the header and its associated memory to the system heap.
*       This should only be used for a header originally allocated via
*       ~mjmdlEmbeddedDPoint3dArray_new.  Headers allocated as locals should be
*       decommissioned via ~mjmdlEmbeddedDPoint3dArray_releaseMem.
* @param pHeader    IN OUT  array to be freed.
* @return Always returns NULL.
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP EmbeddedDPoint3dArray *jmdlEmbeddedDPoint3dArray_free
(
EmbeddedDPoint3dArray *pHeader
);

/*---------------------------------------------------------------------------------**//**
* @description Reduce the count (number of DPoint3ds) in the array to zero.
*       Existing memory is retained so the array can be refilled to its prior
*       size without requiring reallocation.
* @param pHeader    IN OUT  array to modify
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void  jmdlEmbeddedDPoint3dArray_empty
(
EmbeddedDPoint3dArray *pHeader
);

/*---------------------------------------------------------------------------------**//**
* @description Release all memory attached to the header, and reinitialize the header
*       as an empty array with no buffer.
* @param pHeader    IN OUT  array to empty
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void  jmdlEmbeddedDPoint3dArray_releaseMem
(
EmbeddedDPoint3dArray *pHeader
);

/*---------------------------------------------------------------------------------**//**
* @description Grab (borrow) an array from the cache.  Caller is responsible
*       for using ~mjmdlEmbeddedDPoint3dArray_drop to return the array to the cache when
*       finished.   Controlled "grab and drop" of cache arrays is faster than using
*       either local variables (~mjmdlEmbeddedDPoint3dArray_init and
*       ~mjmdlEmbeddedDPoint3dArray_releaseMem) or heap allocation
*       (~mjmdlEmbeddedDPoint3dArray_new and ~mjmdlEmbeddedDPoint3dArray_free)
*       because the preallocated variable size parts of cached arrays are immediately
*       available without revisiting the system cache.
* @return An array header obtained from the cache.
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP EmbeddedDPoint3dArray *jmdlEmbeddedDPoint3dArray_grab
(
void
);

/*---------------------------------------------------------------------------------**//**
* @description Drop (return) an array to the cache.  Use this to dispose of arrays
*       borrowed with ~mjmdlEmbeddedDPoint3dArray_grab.
* @param pHeader    IN      pointer to array to return to cache.
* @return always returns NULL.
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP EmbeddedDPoint3dArray *jmdlEmbeddedDPoint3dArray_drop
(
EmbeddedDPoint3dArray     *pHeader
);

/*---------------------------------------------------------------------------------**//**
* @description Swap the contents (counts and associated memory) of two headers.
*
* @param pHeader0   IN OUT  first array header
* @param pHeader1   IN OUT  second array header
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void  jmdlEmbeddedDPoint3dArray_swapContents
(
EmbeddedDPoint3dArray   *pHeader0,
EmbeddedDPoint3dArray   *pHeader1
);

/*---------------------------------------------------------------------------------**//**
* @description Ensure the buffer has capacity for n DPoint3ds without
*       reallocation. The count of DPoint3ds in the buffer remains unchanged.
* @param pHeader    IN OUT  array to modify.
* @param n          IN      number of DPoint3ds in buffer.
* @return false if unable to allocate the buffer.
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_ensureCapacity
(
EmbeddedDPoint3dArray   *pHeader,
int                     n
);

/*---------------------------------------------------------------------------------**//**
* @description Reallocate the buffer to accommodate exactly n DPoint3ds
*       NOTE: this will truncate the contents of this instance if its count is
*       greater than n.
* @param pHeader    IN OUT  array to modify.
* @param n          IN      number of values to accommodate, no more, no less.
* @return false if unable to reallocate the buffer.
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_setExactBufferSize
(
EmbeddedDPoint3dArray   *pHeader,
int                     n
);

/*---------------------------------------------------------------------------------**//**
* @description Return the number of DPoint3ds in the array.
*
* @param pHeader    IN      array to query.
* @return array count
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int   jmdlEmbeddedDPoint3dArray_getCount
(
const   EmbeddedDPoint3dArray   *pHeader
);

/*---------------------------------------------------------------------------------**//**
* @description Append a DPoint3d to the end of the array.  The array count is increased
*       by one.
* @param pHeader    IN OUT  array to modify.
* @param pPoint     IN      DPoint3d to append to the array.
* @return true if operation is successful
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_addDPoint3d
(
        EmbeddedDPoint3dArray   *pHeader,
const   DPoint3d                *pPoint
);

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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_insertDPoint3d
(
        EmbeddedDPoint3dArray   *pHeader,
const   DPoint3d                *pPoint,
        int                     index
);

/*---------------------------------------------------------------------------------**//**
* @description Append an array of DPoint3d to the end of the array.
*
* @param pHeader    IN OUT  header of array receiving values
* @param pPoint     IN      array of data to add
* @param n          IN      number to add.
* @return true if operation is successful
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_addDPoint3dArray
(
        EmbeddedDPoint3dArray   *pHeader,
const   DPoint3d                *pPoint,
        int                     n
);

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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_insertDPoint3dArray
(
        EmbeddedDPoint3dArray   *pHeader,
const   DPoint3d                *pPoint,
        int                     index,
        int                     n
);

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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_getDPoint3dArray
(
const   EmbeddedDPoint3dArray   *pHeader,
        DPoint3d                *pBuffer,
        int                     *nGot,
        int                     i0,
        int                     nreq
);

/*---------------------------------------------------------------------------------**//**
* @description Get a DPoint3d from a specified index in the array.
*
* @param pHeader    IN      header of array to access.
* @param pPoint     OUT     DPoint3d accessed from the array.
* @param index      IN      index of DPoint3d to access. Any negative index indicates
*                           highest numbered element in the array.
* @return false if the index is too large, i.e., no DPoint3d was accessed.
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_getDPoint3d
(
const   EmbeddedDPoint3dArray   *pHeader,
        DPoint3d                *pPoint,
        int                     index
);

/*---------------------------------------------------------------------------------**//**
* @description Drop a contiguous block of DPoint3ds.  Copy higher indices back down.
*
* @param pHeader    IN OUT  array to modify.
* @param index      IN      position of first dropped DPoint3d.
* @param nDrop      IN      number of DPoint3ds to drop.
* @return true if operation is successful
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_dropRange
(
EmbeddedDPoint3dArray   *pHeader,
int                     index,
int                     nDrop
);

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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_setDPoint3d
(
EmbeddedDPoint3dArray   *pHeader,
DPoint3dCP              pPoint,
int                     index
);

/*---------------------------------------------------------------------------------**//**
* @description Add n uninitialized DPoint3ds to the array.  The array count is
*       increased by n.
* @param pHeader    IN OUT  array where new block is allocated.
* @param n          IN      number of entries requested.
* @return pointer to the block of memory in the buffer.  This pointer allows fast
*       access to the new buffer area, but becomes invalid if the buffer is reallocated.
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP DPoint3d  *jmdlEmbeddedDPoint3dArray_getBlock
(
EmbeddedDPoint3dArray   *pHeader,
int                     n
);

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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int   jmdlEmbeddedDPoint3dArray_getIndexedDPoint3dArray
(
const   EmbeddedDPoint3dArray   *pHeader,
        DPoint3d                *pOut,
        int                     maxOut,
        int                     *pIndex,
        int                     nIndex
);

/*---------------------------------------------------------------------------------**//**
* @description Get a pointer to the contiguous buffer at specified index.  This pointer
*       may become invalid if array contents are altered.
* @param pHeader    IN      array to access.
* @param index      IN      index of array entry.  Any negative index indicates the final
*                           DPoint3d in the array.
* @return pointer to contiguous buffer (simple C array).
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP DPoint3d* jmdlEmbeddedDPoint3dArray_getPtr
(
EmbeddedDPoint3dArray   *pHeader,
int                     index
);

/*---------------------------------------------------------------------------------**//**
* @description Get a const-qualified pointer to the contiguous buffer at specified index.
*       This pointer may become invalid if array contents are altered.
* @param pHeader    IN      array to access.
* @param index      IN      index of array entry.  Any negative index indicates the final
*                           DPoint3d in the array.
* @return pointer to contiguous buffer (simple C array).
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP const DPoint3d*   jmdlEmbeddedDPoint3dArray_getConstPtr
(
const   EmbeddedDPoint3dArray   *pHeader,
        int                     index
);

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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_swapValues
(
EmbeddedDPoint3dArray   *pHeader,
int                     index1,
int                     index2
);

/*---------------------------------------------------------------------------------**//**
* @description Copy entire contents of source array to dest array.  Reusues existing
*       memory in the destination if possible.
* @param pDestHeader    OUT     destination array.
* @param pSourceHeader  IN      source array.
* @return true if operation is successful
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_copy
(
        EmbeddedDPoint3dArray   *pDestHeader,
const   EmbeddedDPoint3dArray   *pSourceHeader
);

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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void  jmdlEmbeddedDPoint3dArray_sort
(
EmbeddedDPoint3dArray   *pDestHeader,
VBArray_SortFunction    pFunction
);

/*---------------------------------------------------------------------------------**//**
* @description Append a DPoint4d to the array.
*
* @param pHeader    IN OUT  array to modify.
* @param pInPoint   IN      point to append.
* @return true if point normalizes and appends to the array successfully
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_addDPoint4d
(
        EmbeddedDPoint3dArray   *pHeader,
const   DPoint4d                *pInPoint
);

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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_crossProduct3Points
(
const   EmbeddedDPoint3dArray   *pHeader,
        DPoint3d                *pProduct,
        int                     index0,
        int                     index1,
        int                     index2
);

/*---------------------------------------------------------------------------------**//**
* @description Append a DPoint2d to the end of the array.  The array count is increased
*       by one.
* @param pHeader    IN OUT  array to modify.
* @param pInPoint   IN      DPoint2d to append to the array.
* @return true if operation is successful
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_addDPoint2d
(
        EmbeddedDPoint3dArray   *pHeader,
const   DPoint2d                *pInPoint
);

/*---------------------------------------------------------------------------------**//**
* @description Append an array of DPoint2ds to the end of the array.
*
* @param pHeader        IN OUT  header of array receiving values
* @param pPointArray    IN      array of data to add
* @param n              IN      number to add.
* @return true if operation is successful
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_addDPoint2dArray
(
        EmbeddedDPoint3dArray   *pHeader,
const   DPoint2d                *pPointArray,
        int                     n
);

/*---------------------------------------------------------------------------------**//**
* @description Insert at a specified position, shifting others to higher
*       positions as needed.
* @param pHeader    IN OUT  array to modify.
* @param pInPoint   IN      points to insert.
* @param index      IN      index at which the value is to appear in the array. The special
*                           index -1 (negative one) indicates to insert at the end of the array.
* @return true if operation is successful
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_insertDPoint2d
(
        EmbeddedDPoint3dArray   *pHeader,
const   DPoint2d                *pInPoint,
        int                     index
);

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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_insertDPoint2dArray
(
        EmbeddedDPoint3dArray   *pHeader,
const   DPoint2d                *pPointArray,
        int                     index,
        int                     n
);

/*---------------------------------------------------------------------------------**//**
* @description Get a DPoint2d from a specified index in the array.
*
* @param pHeader    IN      header of array to access.
* @param pPoint     OUT     point accessed from the array.
* @param index      IN      index of point to access. Any negative index indicates highest
*                           numbered element in the array.
* @return false if the index is too large, i.e., no point was accessed.
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_getDPoint2d
(
const   EmbeddedDPoint3dArray   *pHeader,
        DPoint2d                *pPoint,
        int                     index
);

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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_getDPoint2dArray
(
const   EmbeddedDPoint3dArray   *pHeader,
        DPoint2d                *pBuffer,
        int                     *pCount,
        int                     i0,
        int                     nreq
);

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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_setDPoint2d
(
EmbeddedDPoint3dArray   *pHeader,
DPoint2dCP              pInPoint,
int                     index
);

/*---------------------------------------------------------------------------------**//**
* @description Return the range of the points in the array.
*
* @param pHeader    IN      array to examine.
* @param pRange     OUT     data range.
* @return false if the array is empty (has no range).
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_getDRange3d
(
const   EmbeddedDPoint3dArray   *pHeader,
        DRange3d                *pRange
);

/*---------------------------------------------------------------------------------**//**
* @description Return the absolute value of the largest coordinate or coordinate difference found
*       in the array.
* @param pHeader    IN      array to examine.
* @return largest coordinate/coordinate difference, 0 if empty array.
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double    jmdlEmbeddedDPoint3dArray_maxAbs
(
const   EmbeddedDPoint3dArray   *pHeader
);

/*---------------------------------------------------------------------------------**//**
* @description Return the absolute value of the largest coordinate difference found
*       in the array.
* @param pHeader    IN      array to examine.
* @return largest coordinate difference, 0 if empty array.
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double    jmdlEmbeddedDPoint3dArray_maxAbsDifference
(
const   EmbeddedDPoint3dArray   *pHeader
);

/*-----------------------------------------------------------------*//**
* @description multiply each point by a Transform.
* @param pHeader    IN OUT  array to transform
* @param pTransform IN      transform to apply
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void  jmdlEmbeddedDPoint3dArray_multiplyByTransformInPlace
(
        EmbeddedDPoint3dArray   *pHeader,
const   Transform               *pTransform
);

/*---------------------------------------------------------------------------------**//**
* @description Reverse the contents of the array in place.
* @param pHeader    IN OUT  array to reverse
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void  jmdlEmbeddedDPoint3dArray_reverse
(
EmbeddedDPoint3dArray*  pHeader
);

/*---------------------------------------------------------------------------------**//**
* @description For each individual line segment in a linestring, add both start and end points
*       to the embedded array.   Disconnects are permitted.
*
* @param pHeader    IN OUT array to receive segments.
* @param pXYZ IN array of points.
* @param numXYZ IN number of points.
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlEmbeddedDPoint3dArray_addDPoint3dLinestringAsSegments
(
EmbeddedDPoint3dArray* pHeader,
DPoint3d const  *      pXYZ,
int                     numXYZ
);

/*---------------------------------------------------------------------------------**//**
* @description Add a disconnect to the array.
* @param pHeader    IN OUT array to receive disconnect.
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlEmbeddedDPoint3dArray_addDisconnect
(
EmbeddedDPoint3dArray* pHeader
);

/*---------------------------------------------------------------------------------**//**
* @description Trim the array to the smaller of given count or current size.
* @param pHeader    IN OUT  array to trim
* @param count      IN      number of points in the output array
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlEmbeddedDPoint3dArray_trim
(
EmbeddedDPoint3dArray *pHeader,
int count
);

/*---------------------------------------------------------------------------------**//**
* @description Count the number of points before disconnect or end of array.
* @param pHeader    IN subject array
* @param index0  IN first point index.
* @return number of non-disconnects.
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlEmbeddedDPoint3dArray_countToEndOrDisconnect
(
EmbeddedDPoint3dArray const * pHeader,
int                     index0
);

/*---------------------------------------------------------------------------------**//**
* @description Compute the centroid and normal of the polygon from index0 through disconnect or end of array.
* @param pHeader    IN subject array
* @param index0  IN first point index.
* @param pCentroid OUT computed centroid
* @param pArea OUT computed area
* @param pNormal OUT computed normal
* @return number of points used.  0 if polygon calculation failed.
* @group        "DPoint3d Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int    jmdlEmbeddedDPoint3dArray_areaPropertiesToDisconnect
(
EmbeddedDPoint3dArray const * pHeader,
int                   index0,
DPoint3dP pCentroid,
DVec3dP   pNormal,
double    *pArea
);

END_BENTLEY_GEOMETRY_NAMESPACE
