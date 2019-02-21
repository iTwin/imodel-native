/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/build/bsibasegeom/test/dpoint3darray/embeddeddoublearray.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bsibasegeomPCH.h"

BEGIN_BENTLEY_NAMESPACE

typedef VArrayWrapper<double>   DoubleArrayWrapper;


/*---------------------------------------------------------------------------------**//**
* @description Allocate a new MyDoubleArray header from the system heap.
*
* @return pointer to the header.
* @DefaultRequiredLibrary mtg.lib
* @group        "Double Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public MyDoubleArray *MyDoubleArray_new
(
void
)
    {
    return new MyDoubleArray ();
    }


/*---------------------------------------------------------------------------------**//**
* @description Initialize an MyDoubleArray header.  Prior contents are
*       destroyed.   Intended for use immediately following uninitialized creation
*       operation such as (a) local variable declaration or (b) allocation from system
*       heap.
* @param pHeader    OUT     array to initialize.
* @group        "Double Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public void  MyDoubleArray_init
(
MyDoubleArray *pHeader
)
    {
    if (pHeader)
        DoubleArrayWrapper::init(pHeader, (sizeof(double)));
    }


/*---------------------------------------------------------------------------------**//**
* @description Return both the header and its associated memory to the system heap.
*       This should only be used for a header originally allocated via
*       ~mMyDoubleArray_new.  Headers allocated as locals should be
*       decommissioned via ~mMyDoubleArray_releaseMem.
* @param pHeader    IN OUT  array to be freed.
* @return Always returns NULL.
* @group        "Double Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public MyDoubleArray   *MyDoubleArray_free
(
MyDoubleArray *pHeader
)
    {
    if (pHeader)
        delete pHeader;
    return NULL;
    }


/*---------------------------------------------------------------------------------**//**
* @description Reduce the count (number of doubles) in the array to zero.
*       Existing memory is retained so the array can be refilled to its prior
*       size without requiring reallocation.
* @param pHeader    IN OUT  array to modify
* @group        "Double Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public void  MyDoubleArray_empty
(
MyDoubleArray *pHeader
)
    {
    if (pHeader)
        DoubleArrayWrapper::empty (pHeader);
    }


/*---------------------------------------------------------------------------------**//**
* @description Release all memory attached to the header, and reinitialize the header
*       as an empty array with no buffer.
* @param pHeader    IN OUT  array to empty
* @group        "Double Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public void  MyDoubleArray_releaseMem
(
MyDoubleArray *pHeader
)
    {
    if (pHeader)
        DoubleArrayWrapper::releaseMem (pHeader);
    }


/*---------------------------------------------------------------------------------**//**
* @description Grab (borrow) an array from the cache.  Caller is responsible
*       for using ~mMyDoubleArray_drop to return the array to the cache when
*       finished.   Controlled "grab and drop" of cache arrays is faster than using
*       either local variables (~mMyDoubleArray_init and
*       ~mMyDoubleArray_releaseMem) or heap allocation
*       (~mMyDoubleArray_new and ~mMyDoubleArray_free)
*       because the preallocated variable size parts of cached arrays are immediately
*       available without revisiting the system cache.
* @return An array header obtained from the cache.
* @group        "Double Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public MyDoubleArray   *MyDoubleArray_grab
(
void
)
    {
#ifdef USE_PTR_CACHE
    if (!pCache)
        initCache ();
    return (MyDoubleArray *)omdlPtrCache_grabFromCache (pCache);
#else
    return MyDoubleArray_new ();
#endif
    }


/*---------------------------------------------------------------------------------**//**
* @description Drop (return) an array to the cache.  Use this to dispose of arrays
*       borrowed with ~mMyDoubleArray_grab.
* @param pHeader    IN      pointer to array to return to cache.
* @return always returns NULL.
* @group        "Double Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public MyDoubleArray   *MyDoubleArray_drop
(
MyDoubleArray     *pHeader
)
    {
#ifdef USE_PTR_CACHE
    if (!pCache)
        initCache ();
    omdlPtrCache_dropToCache (pCache, pHeader);
    return NULL;
#else
    return MyDoubleArray_free (pHeader);
#endif
    }


/*---------------------------------------------------------------------------------**//**
* @description Swap the contents (counts and associated memory) of two headers.
*
* @param pHeader0   IN OUT  first array header
* @param pHeader1   IN OUT  second array header
* @group        "Double Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public void  MyDoubleArray_swapContents
(
MyDoubleArray *pHeader0,
MyDoubleArray *pHeader1
)
    {
    MyDoubleArray scratchHeader = *pHeader0;
    *pHeader0 = *pHeader1;
    *pHeader1 = scratchHeader;
    }


/*---------------------------------------------------------------------------------**//**
* @description Ensure the buffer has capacity for n doubles without
*       reallocation. The count of doubles in the buffer remains unchanged.
* @param pHeader    IN OUT  array to modify.
* @param n          IN      number of doubles in buffer.
* @return false if unable to allocate the buffer.
* @group        "Double Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      MyDoubleArray_ensureCapacity
(
MyDoubleArray *pHeader,
int                 n
)
    {
    if (NULL != pHeader)
        {
        DoubleArrayWrapper::setBufferSize(pHeader, n);
        return true;
        }
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @description Reallocate the buffer to accommodate exactly n doubles
*       NOTE: this will truncate the contents of this instance if its count is
*       greater than n.
* @param pHeader    IN OUT  array to modify.
* @param n          IN      number of values to accommodate, no more, no less.
* @return false if unable to reallocate the buffer.
* @group        "Double Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      MyDoubleArray_setExactBufferSize
(
MyDoubleArray    *pHeader,
int                 n
)
    {
    if (NULL != pHeader)
        {
        DoubleArrayWrapper::setBufferSize (pHeader, n);
        return true;
        }
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @description Return the number of doubles in the array.
*
* @param pHeader    IN      array to query.
* @return array count
* @group        "Double Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public int   MyDoubleArray_getCount
(
const   MyDoubleArray *pHeader
)
    {
    if (!pHeader)
        return 0;
    return DoubleArrayWrapper::getCount(pHeader);
    }


/*---------------------------------------------------------------------------------**//**
* @description Append a double to the end of the array.  The array count is increased
*       by one.
* @param pHeader    IN OUT  array to modify.
* @param value      IN      double to append to the array.
* @return true if operation is successful
* @group        "Double Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      MyDoubleArray_addDouble
(
        MyDoubleArray *pHeader,
const   double              value
)
    {
    return pHeader != NULL
        && SUCCESS == DoubleArrayWrapper::insert(pHeader,  &value, -1);
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
* @group        "Double Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      MyDoubleArray_insertDouble
(
        MyDoubleArray *pHeader,
const   double              value,
        int                 index
)
    {
    return pHeader != NULL
        && SUCCESS == DoubleArrayWrapper::insert(pHeader, &value, index);
    }


/*---------------------------------------------------------------------------------**//**
* @description Append an array of double to the end of the array.
*
* @param pHeader    IN OUT  header of array receiveing values
* @param pIn        IN      array of data to add
* @param n          IN      number to add.
* @return true if operation is successful
* @group        "Double Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      MyDoubleArray_addDoubleArray
(
        MyDoubleArray *pHeader,
const   double              *pIn,
        int                 n
)
    {
    return pHeader != NULL
            && SUCCESS == DoubleArrayWrapper::insert (pHeader,  pIn, -1, n);
    }


/*---------------------------------------------------------------------------------**//**
* @description Insert an array of doubles in the array, with index given for
*       first new double.  All previous contents from that index up are moved to
*       make room for the new data.
* @param pHeader    IN OUT  header of array receiveing data
* @param pIn        IN      array of values to add
* @param index      IN      index location for adding the array
* @param n          IN      number of values to add
* @return true if operation is successful
* @group        "Double Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      MyDoubleArray_insertDoubleArray
(
        MyDoubleArray *pHeader,
const   double              *pIn,
        int                 index,
        int                 n
)
    {
    return pHeader != NULL
        && SUCCESS == DoubleArrayWrapper::insert (pHeader,  pIn, index, n);
    }


/*---------------------------------------------------------------------------------**//**
* @description Copy up to nreq doubles out of the array into a buffer.
*
* @param pHeader    IN      header of array to access.
* @param pBuffer    OUT     buffer of doubles.
* @param nGot       OUT     number of doubles placed in buffer.
* @param i0         IN      index of first double to access.
* @param nreq       IN      number of doubles requested.
* @return true if at least one double was copied.
* @group        "Double Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      MyDoubleArray_getDoubleArray
(
const   MyDoubleArray *pHeader,
        double              *pBuffer,
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
    *nGot = DoubleArrayWrapper::getArray(pHeader,  pBuffer, i0, nreq);
    return *nGot > 0;
    }


/*---------------------------------------------------------------------------------**//**
* @description Get a double from a specified index in the array.
*
* @param pHeader    IN      header of array to access.
* @param pValue     OUT     double accessed from the array.
* @param index      IN      index of double to access. Any negative index indicates
*                           highest numbered element in the array.
* @return false if the index is too large, i.e., no double was accessed.
* @group        "Double Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      MyDoubleArray_getDouble
(
const   MyDoubleArray *pHeader,
        double              *pValue,
        int                 index
)
    {
    return pHeader != NULL
        && SUCCESS == DoubleArrayWrapper::get (pHeader,  pValue, index);
    }


/*---------------------------------------------------------------------------------**//**
* @description Drop a contiguous block of doubles.  Copy higher indices back down.
*
* @param pHeader    IN OUT  array to modify.
* @param index      IN      position of first dropped double.
* @param nDrop      IN      number of double to drop.
* @return true if operation is successful
* @group        "Double Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      MyDoubleArray_dropRange
(
MyDoubleArray   *pHeader,
int                     index,
int                     nDrop
)
    {
    return pHeader != NULL
        && SUCCESS == DoubleArrayWrapper::dropRange (pHeader, index, nDrop);
    }


/*---------------------------------------------------------------------------------**//**
* @description Store a double in the array at the specified index.
*
* @param pHeader    IN OUT  array to modify.
* @param value      IN      double to store.
* @param index      IN      position where the double is stored.  A negative
*                           indicates replacement of the current final double.  If the
*                           index is beyond the final current double, zeros are
*                           inserted to fill to the new index.
* @return false if the index required array expansion and the reallocation failed.
* @group        "Double Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      MyDoubleArray_setDouble
(
MyDoubleArray *pHeader,
double              value,
int                 index
)
    {
    return pHeader != NULL
        && SUCCESS == DoubleArrayWrapper::set (pHeader,  &value, index);
    }


/*---------------------------------------------------------------------------------**//**
* @description Add n uninitialized doubles to the array.  The array count is
*       increased by n.
* @param pHeader    IN OUT  array where new block is allocated.
* @param n          IN      number of entries requested.
* @return pointer to the block of memory in the buffer.  This pointer allows fast
*       access to the new buffer area, but becomes invalid if the buffer is reallocated.
* @group        "Double Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public double    *MyDoubleArray_getBlock
(
MyDoubleArray *pHeader,
int                 n
)
    {
    if (pHeader)
        return  (double*) DoubleArrayWrapper::getNewBlock (pHeader, n);
    else
        return  NULL;
    }


/*---------------------------------------------------------------------------------**//**
* @description Copy multiple doubles out of the array, using an array of indices
*       to select the doubles.  Any negative index terminates copying.
* @param pHeader    IN      source array
* @param pOut       OUT     packed output data
* @param maxOut     IN      output array limit
* @param pIndex     IN      index array
* @param nIndex     IN      number of indices
* @return number of succesful dereferences.
* @group        "Double Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public int   MyDoubleArray_getIndexedDoubleArray
(
const   MyDoubleArray *pHeader,
        double              *pOut,
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
        int maxIndex = DoubleArrayWrapper::getCount (pHeader);
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
                if (SUCCESS == (status = DoubleArrayWrapper::get
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
*                           double in the array.
* @return pointer to contiguous buffer (simple C array).
* @group        "Double Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public double*   MyDoubleArray_getPtr
(
MyDoubleArray *pHeader,
int                 index
)
    {
    if (pHeader)
        return  (double*) DoubleArrayWrapper::getPtr (pHeader, index);
    else
        return  NULL;
    }


/*---------------------------------------------------------------------------------**//**
* @description Get a const-qualified pointer to the contiguous buffer at specified index.
*       This pointer may become invalid if array contents are altered.
* @param pHeader    IN      array to access.
* @param index      IN      index of array entry.  Any negative index indicates the final
*                           double in the array.
* @return pointer to contiguous buffer (simple C array).
* @group        "Double Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public const double* MyDoubleArray_getConstPtr
(
const   MyDoubleArray *pHeader,
        int                 index
)
    {
    if (pHeader)
        return  (const double*) DoubleArrayWrapper::getConstPtr (pHeader, index);
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
* @group        "Double Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      MyDoubleArray_swapValues
(
MyDoubleArray *pHeader,
int                 index1,
int                 index2
)
    {
    return pHeader != NULL
        && SUCCESS == DoubleArrayWrapper::swapValues (pHeader, index1, index2);
    }


/*---------------------------------------------------------------------------------**//**
* @description Copy entire contents of source array to dest array.  Reusues existing
*       memory in the destination if possible.
* @param pDestHeader    OUT     destination array.
* @param pSourceHeader  IN      source array.
* @return true if operation is successful
* @group        "Double Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool      MyDoubleArray_copy
(
        MyDoubleArray *pDestHeader,
const   MyDoubleArray *pSourceHeader
)
    {
    if (pDestHeader != NULL
        && pSourceHeader != NULL)
        *pDestHeader = *pSourceHeader;
    return true;
    }

Private int compareDoubles
(
const double *pA,
const double *pB
)
    {
    if (*pA < *pB)
        return -1;
    if (*pA > *pB)
        return 1;
    return 0;
    }


/*---------------------------------------------------------------------------------**//**
* @description Sort the doubles within the array.
*
* @param pHeader    IN OUT  array to sort
* @group        "Double Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public void  MyDoubleArray_sort
(
MyDoubleArray *pHeader
)
    {
    if (pHeader)
        {
        std::sort (pHeader->begin (), pHeader->end ());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description Trim the array to the smaller of given count or current size.
* @param pHeader    IN OUT  array to trim
* @param count      IN      number of doubles in the output array
* @group        "Double Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public void MyDoubleArray_trim
(
MyDoubleArray *pHeader,
int count
)
    {
    DoubleArrayWrapper::trim (pHeader, count);
    }

/*---------------------------------------------------------------------------------**//**
* @description Reverse the contents of the array in place.
* @param pHeader    IN OUT  array to reverse
* @group        "Double Array"
* @bsimethod                                                    DavidAssaf      02/05
+---------------+---------------+---------------+---------------+---------------+------*/
Public void  MyDoubleArray_reverse
(
MyDoubleArray*  pHeader
)
    {
    int i, numPoint;

    if (pHeader && 1 < (numPoint = DoubleArrayWrapper::getCount (pHeader)))
        {
        for (i = 0; i < numPoint / 2; i++)
            DoubleArrayWrapper::swapValues (pHeader, i, numPoint - 1 - i);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @description Empty the array and fill it with a constant value.
*
* @param pHeader    IN OUT  array to fill.
* @param value      IN      double to store.
* @param num        IN      number of copies of stored value.
* @group        "Double Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public void  MyDoubleArray_setConstant
(
MyDoubleArray    *pHeader,
double              value,
int                 num
)
    {
    int i;
    double *pBuffer;
    MyDoubleArray_empty (pHeader);
    pBuffer = MyDoubleArray_getBlock (pHeader, num);
    for (i = 0; i < num; i++)
        pBuffer[i] = value;
    }
END_BENTLEY_NAMESPACE
