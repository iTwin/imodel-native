/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_NAMESPACE
typedef VArrayWrapper<int>      IntArrayWrapper;


/*---------------------------------------------------------------------------------**//**
* @description Allocate a new MyIntArray header from the system heap.
*
* @return pointer to the header.
* @DefaultRequiredLibrary mtg.lib
* @group        "Int Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public MyIntArray  *EmbeddedIntArray_new
(
void
)
    {
    return new MyIntArray ();
    }


/*---------------------------------------------------------------------------------**//**
* @description Initialize an MyIntArray header.  Prior contents are
*       destroyed.   Intended for use immediately following uninitialized creation
*       operation such as (a) local variable declaration or (b) allocation from system
*       heap.
* @param pHeader    OUT     array to initialize.
* @group        "Int Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public void  EmbeddedIntArray_init
(
MyIntArray    *pHeader
)
    {
    if (pHeader)
        IntArrayWrapper::init(pHeader, (sizeof(int)));
    }


/*---------------------------------------------------------------------------------**//**
* @description Return both the header and its associated memory to the system heap.
*       This should only be used for a header originally allocated via
*       ~mEmbeddedIntArray_new.  Headers allocated as locals should be
*       decommissioned via ~mEmbeddedIntArray_releaseMem.
* @param pHeader    IN OUT  array to be freed.
* @return Always returns NULL.
* @group        "Int Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public MyIntArray  *EmbeddedIntArray_free
(
MyIntArray *pHeader
)
    {
    if (pHeader)
        {
        delete pHeader;
        }
    return NULL;
    }


/*---------------------------------------------------------------------------------**//**
* @description Reduce the count (number of ints) in the array to zero.
*       Existing memory is retained so the array can be refilled to its prior
*       size without requiring reallocation.
* @param pHeader    IN OUT  array to modify
* @group        "Int Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public void  EmbeddedIntArray_empty
(
MyIntArray *pHeader
)
    {
    if (pHeader)
        IntArrayWrapper::empty (pHeader);
    }


/*---------------------------------------------------------------------------------**//**
* @description Release all memory attached to the header, and reinitialize the header
*       as an empty array with no buffer.
* @param pHeader    IN OUT  array to empty
* @group        "Int Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public void  EmbeddedIntArray_releaseMem
(
MyIntArray *pHeader
)
    {
    if (pHeader)
        IntArrayWrapper::releaseMem (pHeader);
    }


/*---------------------------------------------------------------------------------**//**
* @description Grab (borrow) an array from the cache.  Caller is responsible
*       for using ~mEmbeddedIntArray_drop to return the array to the cache when
*       finished.   Controlled "grab and drop" of cache arrays is faster than using
*       either local variables (~mEmbeddedIntArray_init and
*       ~mEmbeddedIntArray_releaseMem) or heap allocation
*       (~mEmbeddedIntArray_new and ~mEmbeddedIntArray_free)
*       because the preallocated variable size parts of cached arrays are immediately
*       available without revisiting the system cache.
* @return An array header obtained from the cache.
* @group        "Int Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public MyIntArray  *EmbeddedIntArray_grab
(
void
)
    {
#ifdef USE_PTR_CACHE
    if (!pCache)
        initCache ();
    return (MyIntArray *)omdlPtrCache_grabFromCache (pCache);
#else
    return EmbeddedIntArray_new ();
#endif
    }


/*---------------------------------------------------------------------------------**//**
* @description Drop (return) an array to the cache.  Use this to dispose of arrays
*       borrowed with ~mEmbeddedIntArray_grab.
* @param pHeader    IN      pointer to array to return to cache.
* @return always returns NULL.
* @group        "Int Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public MyIntArray  *EmbeddedIntArray_drop
(
MyIntArray     *pHeader
)
    {
#ifdef USE_PTR_CACHE
    if (!pCache)
        initCache ();
    omdlPtrCache_dropToCache (pCache, pHeader);
    return NULL;
#else
    return EmbeddedIntArray_free (pHeader);
#endif
    }


/*---------------------------------------------------------------------------------**//**
* @description Swap the contents (counts and associated memory) of two headers.
*
* @param pHeader0   IN OUT  first array header
* @param pHeader1   IN OUT  second array header
* @group        "Int Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public void  EmbeddedIntArray_swapContents
(
MyIntArray    *pHeader0,
MyIntArray    *pHeader1
)
    {
    MyIntArray scratchHeader = *pHeader0;
    *pHeader0 = *pHeader1;
    *pHeader1 = scratchHeader;
    }


/*---------------------------------------------------------------------------------**//**
* @description Ensure the buffer has capacity for n ints without
*       reallocation. The count of ints in the buffer remains unchanged.
* @param pHeader    IN OUT  array to modify.
* @param n          IN      number of ints in buffer.
* @return false if unable to allocate the buffer.
* @group        "Int Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedIntArray_ensureCapacity
(
MyIntArray    *pHeader,
int                 n
)
    {
    if (NULL != pHeader)
        {
        IntArrayWrapper::setBufferSize(pHeader, n);
        return true;
        }
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @description Reallocate the buffer to accommodate exactly n ints
*       NOTE: this will truncate the contents of this instance if its count is
*       greater than n.
* @param pHeader    IN OUT  array to modify.
* @param n          IN      number of values to accommodate, no more, no less.
* @return false if unable to reallocate the buffer.
* @group        "Int Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedIntArray_setExactBufferSize
(
MyIntArray    *pHeader,
int                 n
)
    {
    if (NULL != pHeader)
        {
        IntArrayWrapper::setBufferSize (pHeader, n);
        return true;
        }
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @description Return the number of ints in the array.
*
* @param pHeader    IN      array to query.
* @return array count
* @group        "Int Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public int   EmbeddedIntArray_getCount
(
const   MyIntArray    *pHeader
)
    {
    if (!pHeader)
        return 0;
    return IntArrayWrapper::getCount(pHeader);
    }


/*---------------------------------------------------------------------------------**//**
* @description Append an int to the end of the array.  The array count is increased
*       by one.
* @param pHeader    IN OUT  array to modify.
* @param value      IN      int to append to the array.
* @return true if operation is successful
* @group        "Int Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedIntArray_addInt
(
        MyIntArray    *pHeader,
const   int                 value
)
    {
    return pHeader != NULL
        && SUCCESS == IntArrayWrapper::insert(pHeader,  &value, -1);
    }


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
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedIntArray_insertInt
(
        MyIntArray    *pHeader,
const   int                 value,
        int                 index
)
    {
    return pHeader != NULL
        && SUCCESS == IntArrayWrapper::insert(pHeader,  &value, index);
    }


/*---------------------------------------------------------------------------------**//**
* @description Append an array of ints to the end of the array.
*
* @param pHeader    IN OUT  header of array receiveing values
* @param pIn        IN      array of data to add
* @param n          IN      number to add.
* @return true if operation is successful
* @group        "Int Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedIntArray_addIntArray
(
        MyIntArray    *pHeader,
const   int                 *pIn,
        int                 n
)
    {
    return pHeader != NULL
            && SUCCESS == IntArrayWrapper::insert (pHeader,  pIn, -1, n);
    }


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
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedIntArray_insertIntArray
(
        MyIntArray    *pHeader,
const   int                 *pIn,
        int                 index,
        int                 n
)
    {
    return pHeader != NULL
        && SUCCESS == IntArrayWrapper::insert (pHeader,  pIn, index, n);
    }


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
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedIntArray_getIntArray
(
const   MyIntArray    *pHeader,
        int                 *pBuffer,
        int                 *nGot,
        int                 i0,
        int                 nreq
)
    {
    if (!pHeader)
        {
        *nGot = 0;
        return false;
        }
    *nGot = IntArrayWrapper::getArray(pHeader,  pBuffer, i0, nreq);
    return *nGot > 0;
    }


/*---------------------------------------------------------------------------------**//**
* @description Get an int from a specified index in the array.
*
* @param pHeader    IN      header of array to access.
* @param pValue     OUT     int accessed from the array.
* @param index      IN      index of int to access. Any negative index indicates
*                           highest numbered element in the array.
* @return false if the index is too large, i.e., no int was accessed.
* @group        "Int Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedIntArray_getInt
(
const   MyIntArray    *pHeader,
        int                 *pValue,
        int                 index
)
    {
    return pHeader != NULL
        && SUCCESS == IntArrayWrapper::get (pHeader,  pValue, index);
    }


/*---------------------------------------------------------------------------------**//**
* @description Drop a contiguous block of Ints.  Copy higher indices back down.
*
* @param pHeader    IN OUT  array to modify.
* @param index      IN      position of first dropped int.
* @param nDrop      IN      number of ints to drop.
* @return true if operation is successful
* @group        "Int Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedIntArray_dropRange
(
MyIntArray   *pHeader,
int                     index,
int                     nDrop
)
    {
    return pHeader != NULL
        && SUCCESS == IntArrayWrapper::dropRange (pHeader, index, nDrop);
    }


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
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedIntArray_setInt
(
MyIntArray    *pHeader,
int                 value,
int                 index
)
    {
    return pHeader != NULL
        && SUCCESS == IntArrayWrapper::set (pHeader,  &value, index);
    }


/*---------------------------------------------------------------------------------**//**
* @description Add n uninitialized ints to the array.  The array count is
*       increased by n.
* @param pHeader    IN OUT  array where new block is allocated.
* @param n          IN      number of entries requested.
* @return pointer to the block of memory in the buffer.  This pointer allows fast
*       access to the new buffer area, but becomes invalid if the buffer is reallocated.
* @group        "Int Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public int   *EmbeddedIntArray_getBlock
(
MyIntArray    *pHeader,
int                 n
)
    {
    if (pHeader)
        return  (int*) IntArrayWrapper::getNewBlock (pHeader, n);
    else
        return  NULL;
    }


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
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public int   EmbeddedIntArray_getIndexedIntArray
(
const   MyIntArray    *pHeader,
        int                 *pOut,
        int                 maxOut,
        int                 *pIndex,
        int                 nIndex
)
    {
    int n = 0;
    int status, index;
    int i;
    if (pHeader)
        {
        int maxIndex = IntArrayWrapper::getCount (pHeader);
        if (nIndex > maxOut)
            nIndex = maxOut;
        for (i = 0; i < nIndex; i++)
            {
            index = pIndex[i];
            if (index < 0 || index >= maxIndex)
                {
                i = nIndex;     /* force exit from loop */
                }
            else
                {
                if (SUCCESS == (status = IntArrayWrapper::get
                        (pHeader, &pOut[i], index)))
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
*                           int in the array.
* @return pointer to contiguous buffer (simple C array).
* @group        "Int Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public int*  EmbeddedIntArray_getPtr
(
MyIntArray    *pHeader,
int                 index
)
    {
    if (pHeader)
        return  (int*) IntArrayWrapper::getPtr (pHeader, index);
    else
        return  NULL;
    }


/*---------------------------------------------------------------------------------**//**
* @description Get a const-qualified pointer to the contiguous buffer at specified index.
*       This pointer may become invalid if array contents are altered.
* @param pHeader    IN      array to access.
* @param index      IN      index of array entry.  Any negative index indicates the final
*                           int in the array.
* @return pointer to contiguous buffer (simple C array).
* @group        "Int Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public const int*    EmbeddedIntArray_getConstPtr
(
const   MyIntArray    *pHeader,
        int                 index
)
    {
    if (pHeader)
        return  (const int*) IntArrayWrapper::getConstPtr (pHeader, index);
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
* @group        "Int Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedIntArray_swapValues
(
MyIntArray    *pHeader,
int                 index1,
int                 index2
)
    {
    return pHeader != NULL
        && SUCCESS == IntArrayWrapper::swapValues (pHeader, index1, index2);
    }


/*---------------------------------------------------------------------------------**//**
* @description Copy entire contents of source array to dest array.  Reusues existing
*       memory in the destination if possible.
* @param pDestHeader    OUT     destination array.
* @param pSourceHeader  IN      source array.
* @return true if operation is successful
* @group        "Int Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedIntArray_copy
(
        MyIntArray    *pDestHeader,
const   MyIntArray    *pSourceHeader
)
    {
    if (pDestHeader != NULL
        && pSourceHeader != NULL)
    *pDestHeader = *pSourceHeader;
    return true;
    }

Private int compareInts
(
const int *pA,
const int *pB
)
    {
    if (*pA < *pB)
        return -1;
    if (*pA > *pB)
        return 1;
    return 0;
    }


/*---------------------------------------------------------------------------------**//**
* @description Sort the ints within the array.
*
* @param pHeader    IN OUT  array to modify.
* @group        "Int Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public void  EmbeddedIntArray_sort
(
MyIntArray    *pHeader
)
    {
    if (pHeader)
        {
        std::sort (pHeader->begin (), pHeader->end ());
        }
    }


/*---------------------------------------------------------------------------------**//**
* @description Replace contiguous blocks of identical ints by a single int.
*
* @param pHeader    IN OUT  array to modify.
* @group        "Int Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public void  EmbeddedIntArray_compressMatchingNeighbors
(
MyIntArray    *pHeader
)
    {
    int iRead, iWrite, n;
    int *pBuffer = (int*)IntArrayWrapper::getPtr (pHeader, 0);
    n = IntArrayWrapper::getCount (pHeader);

    if (pBuffer && n > 1)
        {
        iWrite = 0;
        for (iRead = 1; iRead < n; iRead++)
            {
            if (pBuffer[iRead] != pBuffer[iWrite])
                {
                iWrite++;
                pBuffer[iWrite] = pBuffer[iRead];
                }
            }
        IntArrayWrapper::trim (pHeader, iWrite+1);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @description Search the array for a specific target int, using binary
*   search.   (Array is presumed to be sorted.)
* @param pHeader    IN      array to search.
* @param target     IN      int to search for.
* @return index where the value was found.  Negative one (-1) if not found.
* @group        "Int Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public int   EmbeddedIntArray_binarySearch
(
const MyIntArray    *pHeader,
      int                 target
)
    {
    int i0, i2, i1;
    int candidate;
    const int *pBuffer = (const int*) IntArrayWrapper::getConstPtr (pHeader, 0);
    int n = IntArrayWrapper::getCount (pHeader);

    if (pBuffer && n > 0)
        {
        i0 = 0;
        i2 = n - 1;
        if (target < pBuffer[i0] || target > pBuffer[i2])
            return -1;
        if (target == pBuffer[i0])
            return i0;
        if (target == pBuffer[i2])
            return i2;
        /* The target is STRICTLY between values at i0 and i2, i.e.
                [i0] < target < [i2]
           Reduce interval by testing middle index.
        */
        while (i2 > i0 + 1)
            {
            i1 = (i0 + i2) >> 1;
            candidate = pBuffer[i1];
            if (candidate == target)
                return i1;
            if (candidate < target)
                {
                i0 = i1;
                }
            else
                {
                i2 = i1;
                }
            }


        }
    return -1;
    }


/*---------------------------------------------------------------------------------**//**
* @description Empty the array and fill it with a constant value.
*
* @param pHeader    IN OUT  array to fill.
* @param value      IN      int to store.
* @param num        IN      number of copies of stored value.
* @group        "Int Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public void  EmbeddedIntArray_setConstant
(
MyIntArray    *pHeader,
int                 value,
int                 num
)
    {
    int i;
    int *pBuffer;
    EmbeddedIntArray_empty (pHeader);
    pBuffer = EmbeddedIntArray_getBlock (pHeader, num);
    for (i = 0; i < num; i++)
        pBuffer[i] = value;
    }


/*---------------------------------------------------------------------------------**//**
* @description Access the final int in the array, and reduce the count.
*
* @param pHeader    IN OUT  array to read and modify.
* @param pValue     OUT     returned int.
* @return true if array had a value to read, false if array was already empty.
* @group        "Int Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedIntArray_popInt
(
MyIntArray    *pHeader,
int                 *pValue
)
    {
    return IntArrayWrapper::pop (pHeader, pValue);
    }


/*---------------------------------------------------------------------------------**//**
* @description Add two ints to the end of the array.
*
* @param pHeader    IN OUT  array to modify.
* @param value0     IN      first int to add
* @param value1     IN      second int to add
* @return true if operation is successful
* @group        "Int Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedIntArray_add2Int
(
MyIntArray    *pHeader,
int                 value0,
int                 value1
)
    {
    return      NULL != pHeader
           &&   SUCCESS == IntArrayWrapper::insert (pHeader,  &value0, -1)
           &&   SUCCESS == IntArrayWrapper::insert (pHeader,  &value1, -1);
    }

/*---------------------------------------------------------------------------------**//**
* @description Shift the last int of the array into the indicated position and
*       decrement the array size.  This is a fast way to delete from the array
*       when order changes are acceptable.
* @param pHeader    IN OUT  int array being manipulated
* @param index      IN      index of item being replaced by the final int in the array.
*                           If index is -1 or final entry, it is just popped.
*                           If index is otherwise out of bounds, no change.
* @group        "Int Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public void  EmbeddedIntArray_replaceByLast
(
MyIntArray    *pHeader,
int                 index
)
    {
    int last = IntArrayWrapper::getCount (pHeader) - 1;
    if (last < 0)
        return;
    int lastValue;
    IntArrayWrapper::pop (pHeader, &lastValue);
    if (index >= 0 && index < last)
        {
        IntArrayWrapper::set (pHeader, &lastValue, index);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description Replace all values that match oldValue.
*
* @param pHeader    IN OUT  array to modify.
* @param oldValue   IN      old value to be replaced.
* @param newValue   IN      replacement value.
* @group        "Int Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public void  EmbeddedIntArray_replaceMatched
(
MyIntArray    *pHeader,
int                 oldValue,
int                 newValue
)
    {
    int n = IntArrayWrapper::getCount (pHeader);
    int i;

    int *pBase = (int *)IntArrayWrapper::getPtr (pHeader, 0);

    if (oldValue != newValue)
        {
        for (i = 0; i < n; i++)
            {
            if (pBase[i] == oldValue)
                pBase[i] = newValue;
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* @description Return a pointer to an empty int array.  If a user array is provided,
*       just empty and return it.  If not, grab one.
* <P>
* Usage pattern is:
* <PRE>
    MyIntArray  *pArray = EmbeddedIntArray_grabOrEmpty (pUserArray);
    ....
    EmbeddedIntArray_dropIfGrabbed (pArray, pUserArray);
* </PRE>
* <P>
* That is, this function and ~mEmbeddedIntArray_dropIfGrabbed are replacements for
*       grab and drop, for the case when an array may have been provided from elsewhere.
* @param pUserArray     OUT     An array pointer which is available to the caller but
*                               might not have been allocated.  If pUserArray is NULL,
*                               an array is grabbed and becomes the return value.  If
*                               pUserArray is not NULL, it is emptied and becomes the
*                               returned value.
* @return pointer to the input array or a grabbed array.
* @group        "Int Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public MyIntArray  *EmbeddedIntArray_grabOrEmpty
(
MyIntArray    *pUserArray
)
    {
    if (pUserArray)
        {
        EmbeddedIntArray_empty (pUserArray);
        return pUserArray;
        }
    else
        return EmbeddedIntArray_grab ();
    }


/*---------------------------------------------------------------------------------**//**
* @description Partner to ~mEmbeddedIntArray_grabOrEmpty.
*
* @param pArray     IN      value returned by ~mEmbeddedIntArray_grabOrEmpty.
* @param pUserArray IN      Same value as passed to ~mEmbeddedIntArray_grabOrEmpty
*                           as pUserArray.
* @group        "Int Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public void  EmbeddedIntArray_dropIfGrabbed
(
        MyIntArray    *pArray,
const   MyIntArray    *pUserArray
)
    {
    if (!pUserArray)
        {
        EmbeddedIntArray_drop (pArray);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @description Linear search for a target int, starting at the end of the array.
* @param pHeader    IN      array to search.
* @param target     IN      int to search for.
* @return index where the value was found.  Negative one (-1) if not found.
* @group        "Int Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public int   EmbeddedIntArray_findIntFromEnd
(
MyIntArray    *pHeader,
int                 target
)
    {
    int i;
    int n = EmbeddedIntArray_getCount(pHeader);
    int *pBuffer = EmbeddedIntArray_getPtr (pHeader, 0);
    for (i = n; --i >= 0; )
        {
        if (pBuffer[i] == target)
            return i;
        }
    return -1;
    }


/*---------------------------------------------------------------------------------**//**
* @description Append 3 ints to the end of the array.  The array count is increased
*       by three.
* @param pHeader    IN OUT  array to modify.
* @param value0      IN      int to append to the array.
* @param value1      IN      int to append to the array.
* @param value2      IN      int to append to the array.
* @return true if operation is successful
* @group        "Int Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedIntArray_add3Int
(
MyIntArray    *pHeader,
int                 value0,
int                 value1,
int                 value2
)
    {
    return pHeader != NULL
        && SUCCESS == IntArrayWrapper::insert(pHeader,  &value0, -1)
        && SUCCESS == IntArrayWrapper::insert(pHeader,  &value1, -1)
        && SUCCESS == IntArrayWrapper::insert(pHeader,  &value2, -1);
    }

/*---------------------------------------------------------------------------------**//**
* @description Reverse the contents of the array in place.
* @param pHeader    IN OUT  array to reverse
* @group        "Int Array"
* @bsimethod                                                    DavidAssaf      02/05
+---------------+---------------+---------------+---------------+---------------+------*/
Public void  EmbeddedIntArray_reverse
(
MyIntArray*  pHeader
)
    {
    int i, numPoint;

    if (pHeader && 1 < (numPoint = IntArrayWrapper::getCount (pHeader)))
        {
        for (i = 0; i < numPoint / 2; i++)
            IntArrayWrapper::swapValues (pHeader, i, numPoint - 1 - i);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description Reverse the contents of a contiguous subset of the array in place.
* @param pHeader    IN OUT  array containing block to reverse
* @param index      IN      index of start of block (or negative to reverse tail of array)
* @param num        IN      number of values in block
* @return true if block successfully accessed and reversed
* @group        "Int Array"
* @bsimethod                                                    DavidAssaf      02/05
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      EmbeddedIntArray_reverseBlock
(
MyIntArray*   pHeader,
int                 index,
int                 num
)
    {
    int i, numIndex;

    if (pHeader && (1 < num) && (1 < (numIndex = IntArrayWrapper::getCount (pHeader))))
        {
        if (num > numIndex || index >= numIndex)
            return false;

        // reverse tail
        if (index < 0)
            index = numIndex - num;

        if (index + num > numIndex)
            return false;

        for (i = 0; i < num / 2; i++)
            IntArrayWrapper::swapValues (pHeader, index + i, index + num - 1 - i);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @description Trim the array to the smaller of given count or current size.
* @param pHeader    IN OUT  array to trim
* @param count      IN      number of ints in the output array
* @group        "Int Array"
* @bsimethod                                                    DavidAssaf      03/06
+---------------+---------------+---------------+---------------+---------------+------*/
Public void EmbeddedIntArray_trim
(
MyIntArray *pHeader,
int count
)
    {
    IntArrayWrapper::trim (pHeader, count);
    }
END_BENTLEY_NAMESPACE
