/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/build/bsibasegeom/test/dpoint3darray/jmdl_iarray.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_NAMESPACE

/*----------------------------------------------------------------------+
|SECTION msdpoint  DPoint3d arrays                                      |
|Functions jmdlVArrayInt_XXXX provide services for manipulating arrays of|
|integers.                                                              |
|                                                                       |
| User code deals with the arrays via an opaque pointer to a header     |
| structure.  All operations on the array are provided by function      |
| calls.   Using these arrays for recurring graphics operations is      |
| both simpler and more efficient than in-line code.                    |
|                                                                       |
|The header structure for each array has a pointer to the follwoing     |
| array.                                                                |
|<UL>                                                                   |
|<LI>points -- an array of integers       .                             |
|</UL>                                                                  |
|                                                                       |
|The following functions provide allocation and deallocation of         |
|the array headers:                                                     |
|<UL>                                                                   |
|<LI>jmdlVArrayInt_new and jmdlVArrayInt_free -- allocate               |
| and free headers and associated array.                                |
|<LI>jmdlVArrayInt_grab, jmdlVArrayInt_drop are                                 |
|like jmdlVArrayInt_new and   jmdlVArrayInt_free,                               |
| but these functions maintain a Cache of previously                    |
| used arrays, so use of system memory management is reduced.           |
| The vast majority of users of DPoint3d arrays will use grab and drop  |
| rather than new and free.                                             |
|<LI>jmdlVArrayInt_initipointCache -- reinitializes the Cache           |
|by freeing all currently Cached array.                                 |
|</UL>                                                                  |
|                                                                       |
|The following functions add items to the array.                        |
|<UL>                                                                   |
|<LI>jmdlVArrayInt_clear --  sets the number of items in the array       |
| to zero.                                                              |
|<LI>jmdlVArrayInt_addInteger, jmdlVArrayInt_addIntegerArray            |
| add a point or array of points, extending the allocation as needed.   |
|<LI>jmdlVArrayInt_extend -- reallocates (if needed) so a given         |
|     number  of items can be added without causing further allocation  |
|<LI>jmdlVArrayInt_getInt -- get from designated index.                 |
|</UL>                                                                  |
+----------------------------------------------------------------------*/



/**
* Initialize a the rubber array.  Prior contents destroyed.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlVArrayInt_init
(
EmbeddedIntArray  *pInstance
)
    {
    IntArrayWrapper::init(pInstance, (sizeof(int)));
#ifdef ROWDIMENSION
    pInstance->rowDimension = 0;
#endif
    }




/**
* @see
* @return
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP EmbeddedIntArray *jmdlVArrayInt_new
(
)
    {
    return new EmbeddedIntArray ();
    }


/**
* @param pHeader
* @see
* @return EmbeddedIntArray
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP EmbeddedIntArray *jmdlVArrayInt_free
(
EmbeddedIntArray *pHeader
)
    {
    if (pHeader)
        {
        delete pHeader;
        }
    return NULL;
    }


/**
* Clear the array and release all associated memory.
* @bsihdr                                       EarlinLutz      12/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlVArrayInt_releaseMem
(
EmbeddedIntArray *pInstance
)
    {
    if (pInstance)
        IntArrayWrapper::releaseMem (pInstance);
    }


/**
* @param pHeader
* @see
* @return int
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlVArrayInt_getCount
(
const EmbeddedIntArray *pHeader
)
    {
     if (pHeader)
        return IntArrayWrapper::getCount(pHeader);
     else
        return  ERROR;
    }


/**
* @param pHeader
* @see
* @return int
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlVArrayInt_getFullRowCount
(
const EmbeddedIntArray *pHeader,
int   rowDimension
)
    {
    int rowCount;

    if ( !pHeader )
        {
        rowCount = 0;
        }
    else if (rowDimension <= 1)
        {
        rowCount = IntArrayWrapper::getCount(pHeader);
        }
    else
        {
        rowCount = IntArrayWrapper::getCount(pHeader) / rowDimension;
        }
    return rowCount;
    }


/**
* @param pHeader
* @param iRow
* @see
* @return int
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int *jmdlVArrayInt_getRowAddress
(
EmbeddedIntArray *pHeader,
int         iRow,
int         rowDimension
)
    {
    if (rowDimension <= 0)
        rowDimension = 1;
    if (pHeader)
        {
        int offset = iRow * rowDimension;
        return  (int*) IntArrayWrapper::getPtr (pHeader, offset);
        }
    else
        return  NULL;
    }



/**
* @param pHeader
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlVArrayInt_clear
(
EmbeddedIntArray *pHeader
)
    {
    if (pHeader)
        IntArrayWrapper::empty(pHeader);
    }



/**
* @param void
* @see
* @return EmbeddedIntArray
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP EmbeddedIntArray *jmdlVArrayInt_grab
(
void
)
    {
    return jmdlVArrayInt_new ();
    }


/**
* Return a pointer to an empty int array.  If a user array is provided,
*    just empty and return if.  If not, grab one.
* Usage pattern is
*<pre>
*   EmbeddedIntArray  *pArray = grabOrClear (pUserArray);
*   ....
*   dropIfGrabbed (pArray, pUserArray);
*</pre>
* That is, grabOrClear and dropIfGrabbed are replacements for grab and
* drop, for the case when an array may have been provided from elsewhere.
* @return pointer to the input array or a grabbed array.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP EmbeddedIntArray *jmdlVArrayInt_grabOrClear
(
EmbeddedIntArray  *pUserArray
)
    {
    if (pUserArray)
        {
        jmdlVArrayInt_clear (pUserArray);
        return pUserArray;
        }
    else
        return jmdlVArrayInt_new ();
    }


/**
* @param pHeader
* @see
* @return EmbeddedIntArray
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP EmbeddedIntArray *jmdlVArrayInt_drop
(
EmbeddedIntArray * pHeader
)
    {
    jmdlVArrayInt_free (pHeader);
    return NULL;
    }


/**
* Partner to grabOrClear.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlVArrayInt_dropIfGrabbed
(
EmbeddedIntArray  *pArray,
EmbeddedIntArray  *pUserArray
)
    {
    if (!pUserArray)
        {
        jmdlVArrayInt_drop (pArray);
        }
    }



/**
* @param pHeader
* @param int        n
* @see
* @return int
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlVArrayInt_extend
(
EmbeddedIntArray *pHeader,
int         n
)
    {
    if ( pHeader )
        {
        return  IntArrayWrapper::setBufferSize(pHeader, n);
        }
    else
        return ERROR;
    }


/**
* @param pHeader <=> header of array receiveing points
* @param pvbArray => array of items to add
* @param n => number of items to add
* @see
* @return int
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlVArrayInt_addArray
(
EmbeddedIntArray            *pHeader,
const int                   *pvbArray,
      int                   n
)
    {
    if (pHeader)
#if defined (INCLUDE_CRTDBG)
        return  IntArrayWrapper::insert2 (pHeader,  pvbArray, -1, n, __FILE__, __LINE__);
#else
        return  IntArrayWrapper::insert (pHeader,  pvbArray, -1, n);
#endif
    else
        return  ERROR;
    }


/**
* @param pHeader <=> header of array receiveing points
* @param value => value to apply
* @param index => index where the value is to be replaced
* @see
* @return SUCCESS if
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt jmdlVArrayInt_set
(
EmbeddedIntArray            *pHeader,
int                 value,
int                 index
)
    {
    return  IntArrayWrapper::set(pHeader,  &value, index);
    }


/**
* @param pHeader <=> header of array receiveing points
* @param value => value to apply
* @param count => -1 to set all current entries, n to set to this size
* @see
* @return SUCCESS if
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt jmdlVArrayInt_setConstant
(
EmbeddedIntArray            *pHeader,
int                 value,
int                 count
)
    {
    if (pHeader)
        {
        int i, currCount, status = ERROR;

        if (count > 0)
            {
            IntArrayWrapper::releaseMem(pHeader);
            if (SUCCESS == (status = IntArrayWrapper::setBufferSize (pHeader, count)))

                {
                for (i = 0; i < count; i++ )
                    {
                    status = IntArrayWrapper::insert(pHeader, &value, -1);
                    }

                }

            }

        else if (count < 0)
            {
            currCount = IntArrayWrapper::getCount(pHeader);
            for (i = 0; i < currCount; i++ )
                {
                status = IntArrayWrapper::set(pHeader, &value, i);
                }

            }

        return  status;

        }

    else

        return  ERROR;

    }


/**
* @param pvbArray
* @param nGot
* @param pHeader
* @param int                i0
* @param int                nreq
* @see
* @return int
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlVArrayInt_getArray
(
int                 *pvbArray,
int                 *nGot,
const EmbeddedIntArray    *pHeader,
int                 i0,
int                 nreq
)
    {
    int status = ERROR;
    if (pHeader)
        {
        *nGot = IntArrayWrapper::getArray(pHeader,  pvbArray, i0, nreq);

        if (*nGot > 0)
            status = SUCCESS;
        }
    return  status;
    }


/**
* @param pHeader <=> array from which to get block
* @param n => number of entries requested
* @see
* @return int
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int *jmdlVArrayInt_getBlock   /* <= temporary pointer to block, NULL on error */
(
EmbeddedIntArray     *pHeader,
int             n
)
    {
    if (pHeader)
        return (int*) IntArrayWrapper::getNewBlock (pHeader, n);
    else
        return  NULL;

    }


/**
* @param pHeader <=> array from which to get block
* @param n => number of entries requested
* @see
* @return int
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int* jmdlVArrayInt_getPtr
(
EmbeddedIntArray *pHeader,
      int        index
)
    {
    if (pHeader)
        return  (int*) IntArrayWrapper::getPtr (pHeader, index);
    else
        return  NULL;
    }


/**
* @param pHeader
* @param       int        index
* @see
* @return const
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP const int* jmdlVArrayInt_getConstPtr
(
const EmbeddedIntArray *pHeader,
      int        index
)
    {
    if (pHeader)
        return  (int*) IntArrayWrapper::getConstPtr (pHeader, index);
    else
        return  NULL;
    }


/**
* @param pBuffer <= buffer containing one item
* @param pHeader => array header
* @see
* @return SUCCESS if
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt jmdlVArrayInt_pop
(
int    *pBuffer,
EmbeddedIntArray    *pHeader
)
    {
    return  IntArrayWrapper::pop (pHeader, pBuffer) != 0 ? SUCCESS : ERROR;
    }


/**
* @param pBuffer <= buffer containing one item
* @param pHeader => array header
* @see
* @return true if an item was popped
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     boolean_jmdlVArrayInt_pop
(
EmbeddedIntArray  *pHeader,
        int *pInt
)
    {
    return  IntArrayWrapper::pop (pHeader, pInt);
    }


/**
* @param pHeader => array to access
* @param pInt <= integer from array
* @param index => item id within array
* @see
* @return ERROR if index out of bounds
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt jmdlVArrayInt_getInt
(

const EmbeddedIntArray    *pHeader,
      int           *pInt,
      int           index
)
    {
    if (pHeader)
        return IntArrayWrapper::get (pHeader, pInt, index);
    else
        return  ERROR;
    }


/**
* Add one int to the end of the array.
* @param pHeader
* @param int            value
* @see
* @return SUCCESS if
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt jmdlVArrayInt_addInt
(
EmbeddedIntArray        *pHeader,
int             value
)
    {
    if (pHeader)
        return  IntArrayWrapper::insert(pHeader,  &value, -1);
    else
        return  ERROR;
    }


/**
* Add one int to the end of the array.
* @param value0 => first int to add
* @param value1 => second int to add
* @see
* @return SUCCESS if
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt jmdlVArrayInt_add2Int
(
EmbeddedIntArray        *pHeader,
int             value0,
int             value1
)
    {
    if (pHeader)
        return SUCCESS == IntArrayWrapper::insert (pHeader,  &value0, -1)
            && SUCCESS == IntArrayWrapper::insert (pHeader,  &value1, -1)
            ? SUCCESS : ERROR;
    else
        return  ERROR;
    }


/**
* Add one int to the end of the array.
* @param value0 => first int to add
* @param value1 => second int to add
* @param value2 => third int to add
* @see
* @return SUCCESS if
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt jmdlVArrayInt_add3Int
(
EmbeddedIntArray        *pHeader,
int             value0,
int             value1,
int             value2
)
    {
    if (pHeader)
        return SUCCESS == IntArrayWrapper::insert (pHeader,  &value0, -1)
            && SUCCESS == IntArrayWrapper::insert (pHeader,  &value1, -1)
            && SUCCESS == IntArrayWrapper::insert (pHeader,  &value2, -1)
            ? SUCCESS : ERROR;
    else
        return  ERROR;
    }


/**
* @param pDestArray array where shuffled data is placed
* @param pSourceArray original array
* @param pIndexArray index information
* @see
* @return SUCCESS if
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt jmdlVArrayInt_shuffleArray
(
        EmbeddedDPoint3dArray     *pDestArray,    // array where shuffled data is placed
const   EmbeddedDPoint3dArray     *pSourceArray,  // original array
const   EmbeddedIntArray            *pIndexArray    // index information
)
    {
    int i, j, status = ERROR;

    for (i = 0; SUCCESS == (status = jmdlVArrayInt_getInt (pIndexArray, &j, i)); i++)
        {
        if (j >= 0)
            DPoint3dArrayWrapper::moveItem (pDestArray, j, pSourceArray, i);
        }
    return  status;
    }


/*---------------------------------------------------------------------------------
* Functions to implement "Union-Find" in a EmbeddedIntArray.
*
* Start with an empty array.
*
* Call jmdlVArrayInt_newClusterIndex () as needed to get cluster id's.
*
* Call jmdlVArrayInt_mergeClusters (cluster0, cluster1) to do "union" operation
* on the clusters. Returns the id of the merged cluster.  Thereafter cluster0
* and cluster1 are still valid ids -- you can get the merged cluster from either
* by calling  ...
*
* mergedCluseter = jmdlVArrayInt_getMergedClusterIndex (cluster)
*
* Implementation:
* A cluster indices are indices into the array.
* A new cluster is a new entry at the end of the array, referencing itself as its parent.
+---------------+---------------+---------------+---------------+---------------+------*/

/*---------------------------------------------------------------------------------**//**
* Create a new cluster index for a union-find algorithm.
*
* @param    pInstance => int array being used for union find.
* @return
* @bsimethod                                                    EarlinLutz      06/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      jmdlVArrayInt_newClusterIndex
(
EmbeddedIntArray  *pInstance
)
    {
    int index = IntArrayWrapper::getCount (pInstance);

    IntArrayWrapper::set (pInstance, &index, index);
    return  index;
    }


/*---------------------------------------------------------------------------------**//**
* Search upwards in the union-find structure for a parent cluster.
* Fixup indices along the way!!!   The parent index always is a root (i.e. is its own parent)
* @param    pInstance <=> int array being used for union find.
* @param cluster0 => first cluster id
* @return the merged cluster index.
* @bsimethod                                                    EarlinLutz      06/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int  jmdlVArrayInt_getMergedClusterIndexExt
(
EmbeddedIntArray  *pInstance,
int         cluster,
int         depth
)
    {
    int parent;
    static int errors = 0;
    if (SUCCESS == IntArrayWrapper::get (pInstance, &parent, cluster))
        {
        if (parent != cluster)
            {
            if (depth > 10)
                {
                errors++;
                IntArrayWrapper::set (pInstance, &cluster, cluster);
                }
            parent = jmdlVArrayInt_getMergedClusterIndexExt (pInstance, parent, depth + 1);
            IntArrayWrapper::set (pInstance, &parent, cluster);
            }
        }
    return  parent;
    }


/*---------------------------------------------------------------------------------**//**
* Search upwards in the union-find structure for a parent cluster.
* Fixup indices along the way!!!   The parent index always is a root (i.e. is its own parent)
* @param    pInstance <=> int array being used for union find.
* @param cluster0 => first cluster id
* @return the merged cluster index.
* @bsimethod                                                    EarlinLutz      06/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int  jmdlVArrayInt_getMergedClusterIndex
(
EmbeddedIntArray  *pInstance,
int         cluster
)
    {
    return jmdlVArrayInt_getMergedClusterIndexExt (pInstance, cluster, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @param    pInstance <=> int array being used for union find.
* @param cluster0 => first cluster id
* @param cluster1 => second cluster id
* @return the merged cluster index (may be different from both!!)
* @bsimethod                                                    EarlinLutz      06/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      jmdlVArrayInt_mergeClusters
(
EmbeddedIntArray  *pInstance,
int         cluster0,
int         cluster1
)
    {
    int parent0 = jmdlVArrayInt_getMergedClusterIndex (pInstance, cluster0);
    int parent1 = jmdlVArrayInt_getMergedClusterIndex (pInstance, cluster1);
    if (parent1 != parent0)
        {
        IntArrayWrapper::set (pInstance,  &parent1, parent0);
        jmdlVArrayInt_getMergedClusterIndex (pInstance, cluster0);
        }
    return  parent1;
    }

/*---------------------------------------------------------------------------------**//**
* Shift the last member of the array into the indicated position and decrement the array size.
* This is a fast way to delete from the array when order changes are acceptable.
* @param    pInstance <=> int array being manipulated
* @param index => index of item being replaced by the final item in the array.
*                   If index is -1 or final entry, it is just popped.
*                   If index is otherwise out of bounds, no change.
* @bsimethod                                                    EarlinLutz      06/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlVArrayInt_replaceByLast
(
EmbeddedIntArray  *pInstance,
int         index
)
    {
    int last = IntArrayWrapper::getCount (pInstance) - 1;
    if (last < 0)
        return;

    int lastValue;
    if (index >= 0 && index < last)
        {
        IntArrayWrapper::pop (pInstance, &lastValue);
        IntArrayWrapper::set (pInstance, &lastValue, index);
        }
    else if (index == -1 || index >= last)
        {
        IntArrayWrapper::pop (pInstance, &lastValue);
        }
    }

/*---------------------------------------------------------------------------------**//**
* Replace all values that match oldValue.
* @param oldValue => old value to be replaced.
* @param newvalue => replacement value.
* @bsimethod                                                    EarlinLutz      06/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlVArrayInt_replaceMatched
(
EmbeddedIntArray  *pInstance,
int         oldValue,
int         newValue
)
    {
    int n = IntArrayWrapper::getCount (pInstance);
    int i;

    int *pBase = (int *)IntArrayWrapper::getPtr (pInstance, 0);

    if (oldValue != newValue)
        {
        for (i = 0; i < n; i++)
            {
            if (pBase[i] == oldValue)
                pBase[i] = newValue;
            }
        }
    }
END_BENTLEY_NAMESPACE
