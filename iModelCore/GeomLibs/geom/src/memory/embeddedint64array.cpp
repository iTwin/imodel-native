/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/memory/embeddedint64array.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#define MALLOC BSIBaseGeom::Malloc
#define FREE BSIBaseGeom::Free

/*----------------------------------------------------------------------+
|                                                                       |
|   Local defines                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
#ifdef USE_CACHE
#define DPOINT_CACHE_SIZE 8
/*----------------------------------------------------------------------+
|                                                                       |
|   Local type definitions                                              |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   Private Global variables                                            |
|                                                                       |
+----------------------------------------------------------------------*/
static PtrCache_Functions cacheFunctions =
    {
    (void *(*)(void))jmdlEmbeddedInt64Array_new,
    (void (*)(void *))jmdlEmbeddedInt64Array_free,
    (void (*)(void *))jmdlEmbeddedInt64Array_empty,
    NULL
    };

static PPtrCacheHeader pCache = NULL;
#endif
/*----------------------------------------------------------------------+
|                                                                       |
|   Public Global variables                                             |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   External variables                                                  |
|                                                                       |
+----------------------------------------------------------------------*/

/*======================================================================+
|                                                                       |
|   Private Utility Routines                                            |
|                                                                       |
+======================================================================*/
#ifdef USE_CACHE
/**
* Initialize the cache of arrays to grab.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static void initCache
(
)
    {
    pCache = omdlPtrCache_new (&cacheFunctions, DPOINT_CACHE_SIZE);
    jmdlEmbeddedArrayManager_registerCache (pCache);
    }
#endif
/*======================================================================+
|                                                                       |
|   Major Public Code Section                                           |
|                                                                       |
+======================================================================*/


/**
* Allocate a new header from the system heap.
* @return pointer to the header.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP EmbeddedInt64Array *jmdlEmbeddedInt64Array_new
(
void
)
    {
    EmbeddedInt64Array *pHeader =
        (EmbeddedInt64Array *)MALLOC (sizeof (EmbeddedInt64Array));
    omdlVArray_init(&pHeader->vbArray, (sizeof(int64_t)));
    return pHeader;
    }


/**
* Initialize a given EmbeddedInt64Array header.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlEmbeddedInt64Array_init
(
EmbeddedInt64Array     *pHeader
)
    {
    if (pHeader)
        omdlVArray_init(&pHeader->vbArray, (sizeof(int64_t)));
    }



/**
* Free both the header and is associated memory.
* @return always null.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP EmbeddedInt64Array *jmdlEmbeddedInt64Array_free
(
EmbeddedInt64Array *pHeader
)
    {
    if (pHeader)
        {
        omdlVArray_releaseMem (&pHeader->vbArray);
        FREE ( pHeader );
        }
    return NULL;
    }



/**
* @param pHeader
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlEmbeddedInt64Array_empty
(
EmbeddedInt64Array *pHeader
)
    {
    omdlVArray_empty (&pHeader->vbArray);
    }


/**
* Release all memory attached to the header, and reinitialize the header
* as an empty array with no buffer.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlEmbeddedInt64Array_releaseMem
(
EmbeddedInt64Array *pHeader
)
    {
    omdlVArray_releaseMem (&pHeader->vbArray);
    }



/**
* Grab (borrow) an array from the cache.
* @return pointer to the borrowed header.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP EmbeddedInt64Array *jmdlEmbeddedInt64Array_grab
(
void
)
    {
#ifdef USE_CACHE
    if (!pCache)
        initCache ();
    return (EmbeddedInt64Array *)omdlPtrCache_grabFromCache (pCache);
#else
    return jmdlEmbeddedInt64Array_new ();
#endif
    }



/**
* Drop (return) an array to the cache.
* @return always returns null.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP EmbeddedInt64Array *jmdlEmbeddedInt64Array_drop
(
EmbeddedInt64Array     *pHeader
)
    {
#ifdef USE_CACHE
    if (!pCache)
        initCache ();
    omdlPtrCache_dropToCache (pCache, pHeader);
    return NULL;
#else
    return jmdlEmbeddedInt64Array_free (pHeader);
#endif
    }


/**
* Swap the contents (counts and associated memory) of two headers.
* @param pHeader0 <=> first array header
* @param pHeader1 <=> second array header
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlEmbeddedInt64Array_swapContents
(
EmbeddedInt64Array     *pHeader0,
EmbeddedInt64Array     *pHeader1
)
    {
    EmbeddedInt64Array scratchHeader = *pHeader0;
    *pHeader0 = *pHeader1;
    *pHeader1 = scratchHeader;
    }


/**
* Ensure the buffer is at least a specified minimum size.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlEmbeddedInt64Array_ensureCapacity
(
EmbeddedInt64Array     *pHeader,
int                 n
)
    {
    return   pHeader != NULL
          && boolean_omdlVArray_setBufferSize(&pHeader->vbArray, n);
    }


/**
* Reallocate the buffer to accommodate exactly n values.
* NOTE: this will truncate the contents of this instance if its count is
* greater than n.
*
* @param    n       Number of values to accommodate, no more, no less.
* @return false if unable to reallocate the buffer.
* @bsihdr                                       DavidAssaf      03/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlEmbeddedInt64Array_setExactBufferSize
(
EmbeddedInt64Array     *pHeader,
int                 n
)
    {
    return      pHeader != NULL
           &&   omdlVArray_setExactBufferSize (&pHeader->vbArray, n);
    }


/**
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlEmbeddedInt64Array_getCount
(
const EmbeddedInt64Array *pHeader
)
    {
    if (!pHeader)
        return 0;
    return omdlVArray_getCount(&pHeader->vbArray);
    }


/**
* Add an Int64 to the array.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlEmbeddedInt64Array_addInt64
(
EmbeddedInt64Array     *pHeader,
const int64_t           value
)
    {
    return pHeader != NULL
        && SUCCESS == omdlVArray_insert(&pHeader->vbArray, (char*) &value, -1);
    }


/**
* Insert at specified position, shifting others to higher positions as needed.
* @param pIn => data to insert.
* @param index => index at which the value is to appear in the array.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlEmbeddedInt64Array_insertInt64
(
        EmbeddedInt64Array         *pHeader,
const   int64_t                 value,
        int                     index
)
    {
    return pHeader != NULL
        && SUCCESS == omdlVArray_insert(&pHeader->vbArray, (char*)&value, index);
    }


/**
* @param pHeader <=> header of array receiveing values
* @param pIn => array of data to add
* @param n => number of values to add
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlEmbeddedInt64Array_addInt64Array
(
      EmbeddedInt64Array   *pHeader,
const int64_t              *pIn,
      int                   n
)
    {
    return pHeader != NULL
            && SUCCESS == omdlVArray_insertArray (&pHeader->vbArray, (char*) pIn, -1, n);
    }


/**
* @param pHeader <=> header of array receiveing data
* @param pIn => array of values to add
* @param index => index location for adding the array
* @param n => number of valuess to add
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlEmbeddedInt64Array_insertInt64Array
(
      EmbeddedInt64Array       *pHeader,
const int64_t            *pIn,
      int                   index,
      int                   n
)
    {
    return pHeader != NULL
        && SUCCESS == omdlVArray_insertArray (&pHeader->vbArray, (char*) pIn, index, n);
    }


/**
* Copy up to nreq values out of the array into a buffer.
* @param pBuffer <= buffer of values.
* @nGot   nGot <= number of values placed in buffer.
* @i0 => index of first value to access.
* @nreq => number of values requested.
* @return true if at least one value was returned.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlEmbeddedInt64Array_getInt64Array
(
const EmbeddedInt64Array   *pHeader,
int64_t              *pBuffer,
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
    *nGot = omdlVArray_getArray(&pHeader->vbArray, (char*) pBuffer, i0, nreq);
    return *nGot > 0;
    }


/**
* Get a value from a specified index.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlEmbeddedInt64Array_getInt64
(
const EmbeddedInt64Array   *pHeader,
int64_t                 *pValue,
int                     index
)
    {
    return pHeader != NULL
        && SUCCESS == omdlVArray_get (&pHeader->vbArray, (char*) pValue, index);
    }


/*---------------------------------------------------------------------------------**//**
* @description Drop a contiguous block of int64_t.  Copy higher indices back down.
*
* @param pHeader    IN OUT  array to modify.
* @param index      IN      position of first dropped int64_t.
* @param nDrop      IN      number of int64_t to drop.
* @return true if operation is successful
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedInt64Array_dropRange
(
EmbeddedInt64Array   *pHeader,
int                     index,
int                     nDrop
)
    {
    return pHeader != NULL
        && SUCCESS == omdlVArray_dropRange (&pHeader->vbArray, index, nDrop);
    }


/**
* Store a value at specified index.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlEmbeddedInt64Array_setInt64
(
EmbeddedInt64Array     *pHeader,
int64_t                 value,
int                     index
)
    {
    return pHeader != NULL
        && SUCCESS == omdlVArray_set (&pHeader->vbArray, (char*) &value, index);
    }



/**
* Add n uninitialized int64s to the array.
* @param pHeader <=> array from which to get block
* @param n => number of entries requested
* @return Temporary pointer to block.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int64_t *jmdlEmbeddedInt64Array_getBlock
(
EmbeddedInt64Array *pHeader,
int             n
)
    {
    if (pHeader)
        return  (int64_t*) omdlVArray_getNewBlock (&pHeader->vbArray, n);
    else
        return  NULL;
    }



/**
* @param pHeader source array
* @param pOut Packed output data
* @param maxOut output array limit
* @param pIndex index array
* @param int            nIndex               number of vertices
* @return number of succesful dereferences.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlEmbeddedInt64Array_getIndexedInt64Array
(
const   EmbeddedInt64Array *pHeader,
        int64_t             *pOut,
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
        int maxIndex = omdlVArray_getCount (&pHeader->vbArray);
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
                if (SUCCESS == (status = omdlVArray_get
                        (&pHeader->vbArray, (char*)&pOut[i], index)))
                    n++;
                }
            }

        }
    return n;
    }



/**
* Get a pointer to a position in the array.  This pointer may become invalid
* due to modifications of the array.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int64_t* jmdlEmbeddedInt64Array_getPtr
(
EmbeddedInt64Array *pHeader,
      int        index
)
    {
    if (pHeader)
        return  (int64_t*) omdlVArray_getPtr (&pHeader->vbArray, index);
    else
        return  NULL;
    }



/**
* Get a pointer to a position in the array.  This pointer may become invalid
* due to modifications of the array.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP const int64_t* jmdlEmbeddedInt64Array_getConstPtr
(
const EmbeddedInt64Array *pHeader,
      int        index
)
    {
    if (pHeader)
        return  (const int64_t*) omdlVArray_getConstPtr (&pHeader->vbArray, index);
    else
        return  NULL;
    }




/**
* @param pHeader
* @param index1
* @param index2
* @see
* @return SUCCESS if
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlEmbeddedInt64Array_swapValues
(
EmbeddedInt64Array *pHeader,
int             index1,
int             index2
)
    {
    return pHeader != NULL
        && SUCCESS == omdlVArray_swapValues (&pHeader->vbArray, index1, index2);
    }


/**
* @param pDestHeader
* @param pSourceHeader
* @see
* @return SUCCESS if
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlEmbeddedInt64Array_copy
(
EmbeddedInt64Array *pDestHeader,
const EmbeddedInt64Array *pSourceHeader
)
    {
    return pDestHeader != NULL
        && pSourceHeader != NULL
        && SUCCESS == omdlVArray_copy (&pDestHeader->vbArray, &pSourceHeader->vbArray);
    }

static int compareInt64s
(
const int64_t *pA,
const int64_t *pB
)
    {
    if (*pA < *pB)
        return -1;
    if (*pA > *pB)
        return 1;
    return 0;
    }


/**
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlEmbeddedInt64Array_sort
(
EmbeddedInt64Array *pDestHeader
)
    {
    if (pDestHeader)
        {
        omdlVArray_sort (&pDestHeader->vbArray, (VBArray_SortFunction)compareInt64s);
        }
    }


END_BENTLEY_GEOMETRY_NAMESPACE
