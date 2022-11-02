/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/**
@DocText
@Group "Int Array"
<P>See the summary for ~s"Embedded Arrays".
*/

/*---------------------------------------------------------------------------------**//**
* @description Allocate a new EmbeddedIntArray header from the system heap.
*
* @return pointer to the header.
* @DefaultRequiredLibrary mtg.lib
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP EmbeddedIntArray  *jmdlEmbeddedIntArray_new
(
void
);

/*---------------------------------------------------------------------------------**//**
* @description Initialize an EmbeddedIntArray header.  Prior contents are
*       destroyed.   Intended for use immediately following uninitialized creation
*       operation such as (a) local variable declaration or (b) allocation from system
*       heap.
* @param pHeader    OUT     array to initialize.
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void  jmdlEmbeddedIntArray_init
(
EmbeddedIntArray    *pHeader
);

/*---------------------------------------------------------------------------------**//**
* @description Return both the header and its associated memory to the system heap.
*       This should only be used for a header originally allocated via
*       ~mjmdlEmbeddedIntArray_new.  Headers allocated as locals should be
*       decommissioned via ~mjmdlEmbeddedIntArray_releaseMem.
* @param pHeader    IN OUT  array to be freed.
* @return Always returns NULL.
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP EmbeddedIntArray  *jmdlEmbeddedIntArray_free
(
EmbeddedIntArray *pHeader
);

/*---------------------------------------------------------------------------------**//**
* @description Reduce the count (number of ints) in the array to zero.
*       Existing memory is retained so the array can be refilled to its prior
*       size without requiring reallocation.
* @param pHeader    IN OUT  array to modify
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void  jmdlEmbeddedIntArray_empty
(
EmbeddedIntArray *pHeader
);

/*---------------------------------------------------------------------------------**//**
* @description Release all memory attached to the header, and reinitialize the header
*       as an empty array with no buffer.
* @param pHeader    IN OUT  array to empty
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void  jmdlEmbeddedIntArray_releaseMem
(
EmbeddedIntArray *pHeader
);

/*---------------------------------------------------------------------------------**//**
* @description Grab (borrow) an array from the cache.  Caller is responsible
*       for using ~mjmdlEmbeddedIntArray_drop to return the array to the cache when
*       finished.   Controlled "grab and drop" of cache arrays is faster than using
*       either local variables (~mjmdlEmbeddedIntArray_init and
*       ~mjmdlEmbeddedIntArray_releaseMem) or heap allocation
*       (~mjmdlEmbeddedIntArray_new and ~mjmdlEmbeddedIntArray_free)
*       because the preallocated variable size parts of cached arrays are immediately
*       available without revisiting the system cache.
* @return An array header obtained from the cache.
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP EmbeddedIntArray  *jmdlEmbeddedIntArray_grab
(
void
);

/*---------------------------------------------------------------------------------**//**
* @description Drop (return) an array to the cache.  Use this to dispose of arrays
*       borrowed with ~mjmdlEmbeddedIntArray_grab.
* @param pHeader    IN      pointer to array to return to cache.
* @return always returns NULL.
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP EmbeddedIntArray  *jmdlEmbeddedIntArray_drop
(
EmbeddedIntArray     *pHeader
);

/*---------------------------------------------------------------------------------**//**
* @description Swap the contents (counts and associated memory) of two headers.
*
* @param pHeader0   IN OUT  first array header
* @param pHeader1   IN OUT  second array header
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void  jmdlEmbeddedIntArray_swapContents
(
EmbeddedIntArray    *pHeader0,
EmbeddedIntArray    *pHeader1
);

/*---------------------------------------------------------------------------------**//**
* @description Ensure the buffer has capacity for n ints without
*       reallocation. The count of ints in the buffer remains unchanged.
* @param pHeader    IN OUT  array to modify.
* @param n          IN      number of ints in buffer.
* @return false if unable to allocate the buffer.
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedIntArray_ensureCapacity
(
EmbeddedIntArray    *pHeader,
int                 n
);

/*---------------------------------------------------------------------------------**//**
* @description Reallocate the buffer to accommodate exactly n ints
*       NOTE: this will truncate the contents of this instance if its count is
*       greater than n.
* @param pHeader    IN OUT  array to modify.
* @param n          IN      number of values to accommodate, no more, no less.
* @return false if unable to reallocate the buffer.
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedIntArray_setExactBufferSize
(
EmbeddedIntArray    *pHeader,
int                 n
);

/*---------------------------------------------------------------------------------**//**
* @description Return the number of ints in the array.
*
* @param pHeader    IN      array to query.
* @return array count
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int   jmdlEmbeddedIntArray_getCount
(
const   EmbeddedIntArray    *pHeader
);

/*---------------------------------------------------------------------------------**//**
* @description Append an int to the end of the array.  The array count is increased
*       by one.
* @param pHeader    IN OUT  array to modify.
* @param value      IN      int to append to the array.
* @return true if operation is successful
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedIntArray_addInt
(
        EmbeddedIntArray    *pHeader,
const   int                 value
);

/*---------------------------------------------------------------------------------**//**
* @description Insert at a specified position, shifting others to higher
*       positions as needed.
* @param pHeader    IN OUT  array to modify.
* @param value      IN      data to insert.
* @param index      IN      index at which the value is to appear in the array.
*                           The special index -1 (negative one) indicates to
*                           insert at the end of the array.
* @return true if operation is successful
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedIntArray_insertInt
(
        EmbeddedIntArray    *pHeader,
const   int                 value,
        int                 index
);

/*---------------------------------------------------------------------------------**//**
* @description Append an array of ints to the end of the array.
*
* @param pHeader    IN OUT  header of array receiveing values
* @param pIn        IN      array of data to add
* @param n          IN      number to add.
* @return true if operation is successful
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedIntArray_addIntArray
(
        EmbeddedIntArray    *pHeader,
const   int                 *pIn,
        int                 n
);

/*---------------------------------------------------------------------------------**//**
* @description Insert an array of ints in the array, with index given for
*       first new int.  All previous contents from that index up are moved to
*       make room for the new data.
* @param pHeader    IN OUT  header of array receiveing data
* @param pIn        IN      array of values to add
* @param index      IN      index location for adding the array
* @param n          IN      number of values to add
* @return true if operation is successful
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedIntArray_insertIntArray
(
        EmbeddedIntArray    *pHeader,
const   int                 *pIn,
        int                 index,
        int                 n
);

/*---------------------------------------------------------------------------------**//**
* @description Copy up to nreq ints out of the array into a buffer.
*
* @param pHeader    IN      header of array to access.
* @param pBuffer    OUT     buffer of ints.
* @param nGot       OUT     number of ints placed in buffer.
* @param i0         IN      index of first int to access.
* @param nreq       IN      number of ints requested.
* @return true if at least one int was copied.
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedIntArray_getIntArray
(
const   EmbeddedIntArray    *pHeader,
        int                 *pBuffer,
        int                 *nGot,
        int                 i0,
        int                 nreq
);

/*---------------------------------------------------------------------------------**//**
* @description Get an int from a specified index in the array.
*
* @param pHeader    IN      header of array to access.
* @param pValue     OUT     int accessed from the array.
* @param index      IN      index of int to access. Any negative index indicates
*                           highest numbered element in the array.
* @return false if the index is too large, i.e., no int was accessed.
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedIntArray_getInt
(
const   EmbeddedIntArray    *pHeader,
        int                 *pValue,
        int                 index
);

/*---------------------------------------------------------------------------------**//**
* @description Drop a contiguous block of Ints.  Copy higher indices back down.
*
* @param pHeader    IN OUT  array to modify.
* @param index      IN      position of first dropped int.
* @param nDrop      IN      number of ints to drop.
* @return true if operation is successful
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedIntArray_dropRange
(
EmbeddedIntArray   *pHeader,
int                     index,
int                     nDrop
);

/*---------------------------------------------------------------------------------**//**
* @description Store an int in the array at the specified index.
*
* @param pHeader    IN OUT  array to modify.
* @param value      IN      int to store.
* @param index      IN      position where the int is stored.  A negative
*                           indicates replacement of the current final int.  If the
*                           index is beyond the final current int, zeros are
*                           inserted to fill to the new index.
* @return false if the index required array expansion and the reallocation failed.
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedIntArray_setInt
(
EmbeddedIntArray    *pHeader,
int                 value,
int                 index
);

/*---------------------------------------------------------------------------------**//**
* @description Add n uninitialized ints to the array.  The array count is
*       increased by n.
* @param pHeader    IN OUT  array where new block is allocated.
* @param n          IN      number of entries requested.
* @return pointer to the block of memory in the buffer.  This pointer allows fast
*       access to the new buffer area, but becomes invalid if the buffer is reallocated.
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int   *jmdlEmbeddedIntArray_getBlock
(
EmbeddedIntArray    *pHeader,
int                 n
);

/*---------------------------------------------------------------------------------**//**
* @description Copy multiple ints out of the array, using an array of indices
*       to select the ints.  Any negative index terminates copying.
* @param pHeader    IN      source array
* @param pOut       OUT     packed output data
* @param maxOut     IN      output array limit
* @param pIndex     IN      index array
* @param nIndex     IN      number of indices
* @return number of succesful dereferences.
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int   jmdlEmbeddedIntArray_getIndexedIntArray
(
const   EmbeddedIntArray    *pHeader,
        int                 *pOut,
        int                 maxOut,
        int                 *pIndex,
        int                 nIndex
);

/*---------------------------------------------------------------------------------**//**
* @description Get a pointer to the contiguous buffer at specified index.  This pointer
*       may become invalid if array contents are altered.
* @param pHeader    IN      array to access.
* @param index      IN      index of array entry.  Any negative index indicates the final
*                           int in the array.
* @return pointer to contiguous buffer (simple C array).
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int*  jmdlEmbeddedIntArray_getPtr
(
EmbeddedIntArray    *pHeader,
int                 index
);

/*---------------------------------------------------------------------------------**//**
* @description Get a const-qualified pointer to the contiguous buffer at specified index.
*       This pointer may become invalid if array contents are altered.
* @param pHeader    IN      array to access.
* @param index      IN      index of array entry.  Any negative index indicates the final
*                           int in the array.
* @return pointer to contiguous buffer (simple C array).
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP const int*    jmdlEmbeddedIntArray_getConstPtr
(
const   EmbeddedIntArray    *pHeader,
        int                 index
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
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedIntArray_swapValues
(
EmbeddedIntArray    *pHeader,
int                 index1,
int                 index2
);

/*---------------------------------------------------------------------------------**//**
* @description Copy entire contents of source array to dest array.  Reusues existing
*       memory in the destination if possible.
* @param pDestHeader    OUT     destination array.
* @param pSourceHeader  IN      source array.
* @return true if operation is successful
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedIntArray_copy
(
        EmbeddedIntArray    *pDestHeader,
const   EmbeddedIntArray    *pSourceHeader
);

/*---------------------------------------------------------------------------------**//**
* @description Sort the ints within the array.
*
* @param pHeader    IN OUT  array to modify.
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void  jmdlEmbeddedIntArray_sort
(
EmbeddedIntArray    *pHeader
);

/*---------------------------------------------------------------------------------**//**
* @description Replace contiguous blocks of identical ints by a single int.
*
* @param pHeader    IN OUT  array to modify.
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void  jmdlEmbeddedIntArray_compressMatchingNeighbors
(
EmbeddedIntArray    *pHeader
);

/*---------------------------------------------------------------------------------**//**
* @description Search the array for a specific target int, using binary
*   search.   (Array is presumed to be sorted.)
* @param pHeader    IN      array to search.
* @param target     IN      int to search for.
* @return index where the value was found.  Negative one (-1) if not found.
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int   jmdlEmbeddedIntArray_binarySearch
(
const EmbeddedIntArray    *pHeader,
      int                 target
);

/*---------------------------------------------------------------------------------**//**
* @description Empty the array and fill it with a constant value.
*
* @param pHeader    IN OUT  array to fill.
* @param value      IN      int to store.
* @param num        IN      number of copies of stored value.
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void  jmdlEmbeddedIntArray_setConstant
(
EmbeddedIntArray    *pHeader,
int                 value,
int                 num
);

/*---------------------------------------------------------------------------------**//**
* @description Access the final int in the array, and reduce the count.
*
* @param pHeader    IN OUT  array to read and modify.
* @param pValue     OUT     returned int.
* @return true if array had a value to read, false if array was already empty.
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedIntArray_popInt
(
EmbeddedIntArray    *pHeader,
int                 *pValue
);

/*---------------------------------------------------------------------------------**//**
* @description Add two ints to the end of the array.
*
* @param pHeader    IN OUT  array to modify.
* @param value0     IN      first int to add
* @param value1     IN      second int to add
* @return true if operation is successful
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedIntArray_add2Int
(
EmbeddedIntArray    *pHeader,
int                 value0,
int                 value1
);

/*---------------------------------------------------------------------------------**//**
* @description Shift the last int of the array into the indicated position and
*       decrement the array size.  This is a fast way to delete from the array
*       when order changes are acceptable.
* @param pHeader    IN OUT  int array being manipulated
* @param index      IN      index of item being replaced by the final int in the array.
*                           If index is -1 or final entry, it is just popped.
*                           If index is otherwise out of bounds, no change.
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void  jmdlEmbeddedIntArray_replaceByLast
(
EmbeddedIntArray    *pHeader,
int                 index
);

/*---------------------------------------------------------------------------------**//**
* @description Replace all values that match oldValue.
*
* @param pHeader    IN OUT  array to modify.
* @param oldValue   IN      old value to be replaced.
* @param newValue   IN      replacement value.
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void  jmdlEmbeddedIntArray_replaceMatched
(
EmbeddedIntArray    *pHeader,
int                 oldValue,
int                 newValue
);

/*---------------------------------------------------------------------------------**//**
* @description Return a pointer to an empty int array.  If a user array is provided,
*       just empty and return it.  If not, grab one.
* <P>
* Usage pattern is:
* <PRE>
    EmbeddedIntArray  *pArray = jmdlEmbeddedIntArray_grabOrEmpty (pUserArray);
    ....
    jmdlEmbeddedIntArray_dropIfGrabbed (pArray, pUserArray);
* </PRE>
* <P>
* That is, this function and ~mjmdlEmbeddedIntArray_dropIfGrabbed are replacements for
*       grab and drop, for the case when an array may have been provided from elsewhere.
* @param pUserArray     OUT     An array pointer which is available to the caller but
*                               might not have been allocated.  If pUserArray is NULL,
*                               an array is grabbed and becomes the return value.  If
*                               pUserArray is not NULL, it is emptied and becomes the
*                               returned value.
* @return pointer to the input array or a grabbed array.
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP EmbeddedIntArray  *jmdlEmbeddedIntArray_grabOrEmpty
(
EmbeddedIntArray    *pUserArray
);

/*---------------------------------------------------------------------------------**//**
* @description Partner to ~mjmdlEmbeddedIntArray_grabOrEmpty.
*
* @param pArray     IN      value returned by ~mjmdlEmbeddedIntArray_grabOrEmpty.
* @param pUserArray IN      Same value as passed to ~mjmdlEmbeddedIntArray_grabOrEmpty
*                           as pUserArray.
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void  jmdlEmbeddedIntArray_dropIfGrabbed
(
        EmbeddedIntArray    *pArray,
const   EmbeddedIntArray    *pUserArray
);

/*---------------------------------------------------------------------------------**//**
* @description Linear search for a target int, starting at the end of the array.
* @param pHeader    IN      array to search.
* @param target     IN      int to search for.
* @return index where the value was found.  Negative one (-1) if not found.
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int   jmdlEmbeddedIntArray_findIntFromEnd
(
EmbeddedIntArray    *pHeader,
int                 target
);

/*---------------------------------------------------------------------------------**//**
* @description Append 3 ints to the end of the array.  The array count is increased
*       by three.
* @param pHeader    IN OUT  array to modify.
* @param value0      IN      int to append to the array.
* @param value1      IN      int to append to the array.
* @param value2      IN      int to append to the array.
* @return true if operation is successful
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedIntArray_add3Int
(
EmbeddedIntArray    *pHeader,
int                 value0,
int                 value1,
int                 value2
);

/*---------------------------------------------------------------------------------**//**
* @description Reverse the contents of the array in place.
* @param pHeader    IN OUT  array to reverse
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void  jmdlEmbeddedIntArray_reverse
(
EmbeddedIntArray*  pHeader
);

/*---------------------------------------------------------------------------------**//**
* @description Reverse the contents of a contiguous subset of the array in place.
* @param pHeader    IN OUT  array containing block to reverse
* @param index      IN      index of start of block (or negative to reverse tail of array)
* @param num        IN      number of values in block
* @return true if block successfully accessed and reversed
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedIntArray_reverseBlock
(
EmbeddedIntArray*   pHeader,
int                 index,
int                 num
);

/*---------------------------------------------------------------------------------**//**
* @description Trim the array to the smaller of given count or current size.
* @param pHeader    IN OUT  array to trim
* @param count      IN      number of ints in the output array
* @group        "Int Array"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlEmbeddedIntArray_trim
(
EmbeddedIntArray *pHeader,
int count
);

END_BENTLEY_GEOMETRY_NAMESPACE