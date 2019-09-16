/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_NAMESPACE

/*----------------------------------------------------------------------+
|SECTION msdpoint  DPoint3d arrays                                      |
|Functions mdlHPoint_XXXX provide services for manipulating arrays of   |
|points.                        |
|                                                                       |
| User code deals with the arrays via an opaque pointer to a header     |
| structure.  All operations on the array are provided by function      |
| calls.   Using these arrays for recurring graphics operations is      |
| both simpler and more efficient than in-line code.                    |
|                                                                       |
|The header structure for each array has a pointer to the follwoing     |
| array.                                 |
|<UL>                                                                   |
|<LI>points -- an array of DPoint3d's       .                           |
|</UL>                                                                  |
|                                                                       |
|The following functions provide allocation and deallocation of         |
|the array headers:                                                     |
|<UL>                                                                   |
|<LI>jmdlVArrayDPoint3d_new and jmdlVArrayDPoint3d_free -- allocate
| and free headers and associated arrays.                               |
|<LI>jmdlVArrayDPoint3d_grab, jmdlVArrayDPoint3d_drop are
|like jmdlVArrayDPoint3d_new and   jmdlVArrayDPoint3d_free,             |
| but these functions maintain a Cache of previously                    |
| used arrays, so use of system memory management is reduced.           |
| The vast majority of users of DPoint3d arrays will use grab and drop  |
| rather than new and free.                                             |
|<LI>jmdlVArrayDPoint3d_initipointCache -- reinitializes the ipointCache |
|by freeing all by freeing all currently ipointCached arrays.           |
|</UL>                                                                  |
|                                                                       |
|The following functions add points to the arrays.                      |
|<UL>                                                                   |
|<LI>jmdlVArrayDPoint3d_empty --  sets the number of points in the array |
| to zero.                                                              |
|<LI>jmdlVArrayDPoint3d_addDPoint3d, jmdlVArrayDPoint3d_addDPoint3dArray        |
| add a point or array of points, extending the allocation as needed.   |
|<LI>DPoint3dArrayWrapper::DPoint3d_extend -- reallocates (if needed) so a given number     |
|       of points can be added without causing further allocation       |
|</UL>                                                                  |
+----------------------------------------------------------------------*/


/*======================================================================+
|                                                                       |
|   Major Public Code Section                                           |
|                                                                       |
+======================================================================*/

/* MAP jmdlVArrayDPoint3d_new=EmbeddedDPoint3dArray.alloc ENDMAP */

/**
* @see
* @return EmbeddedDPoint3dArray
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP EmbeddedDPoint3dArray *jmdlVArrayDPoint3d_new
(
void
)
    {
    return new EmbeddedDPoint3dArray ();
    }


/**
* Initialize a given EmbeddedDPoint3dArray header.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlVArrayDPoint3d_init
(
EmbeddedDPoint3dArray     *pHeader
)
    {
    if (pHeader)
        DPoint3dArrayWrapper::init(pHeader, (sizeof(DPoint3d)));
    }


/* MAP jmdlVArrayDPoint3d_free=EmbeddedDPoint3dArray.free ENDMAP */

/**
* @param pHeader
* @see
* @return EmbeddedDPoint3dArray
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP EmbeddedDPoint3dArray *jmdlVArrayDPoint3d_free
(
EmbeddedDPoint3dArray *pHeader
)
    {
    if (pHeader)
        {
        delete pHeader;
        }

    return NULL;
    }


/* MAP define jmdlVArrayDPoint3d_empty(pHeader) pHeader->empty() ENDMAP */

/**
* @param pHeader
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlVArrayDPoint3d_empty
(
EmbeddedDPoint3dArray *pHeader
)
    {
    DPoint3dArrayWrapper::empty (pHeader);
    }


/* MAP define jmdlVArrayDPoint3d_releaseMem(pHeader) pHeader->releaseMem() ENDMAP */

/**
* @param pHeader
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlVArrayDPoint3d_releaseMem
(
EmbeddedDPoint3dArray *pHeader
)
    {
    DPoint3dArrayWrapper::releaseMem (pHeader);
    }


/* MAP jmdlVArrayDPoint3d_grab=EmbeddedDPoint3dArray.grabFromCache ENDMAP */

/**
* @see
* @return EmbeddedDPoint3dArray
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP EmbeddedDPoint3dArray *jmdlVArrayDPoint3d_grab
(
void
)
    {
    return jmdlVArrayDPoint3d_new ();
    }



/**
* @param pHeader
* @see
* @return EmbeddedDPoint3dArray
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP EmbeddedDPoint3dArray *jmdlVArrayDPoint3d_drop
(
EmbeddedDPoint3dArray     *pHeader
)
    {
    if (NULL != pHeader)
        jmdlVArrayDPoint3d_free (pHeader);
    return NULL;
    }


/**
* @param pHeader0 <=> first array header
* @param pHeader1 <=> second array header
* @see
* @return EmbeddedDPoint3dArray
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlVArrayDPoint3d_swapContents
(
EmbeddedDPoint3dArray     *pHeader0,
EmbeddedDPoint3dArray     *pHeader1
)
    {
    EmbeddedDPoint3dArray scratchHeader = *pHeader0;
    *pHeader0 = *pHeader1;
    *pHeader1 = scratchHeader;
    }


/**
* @param    pHeader
* @param n
* @see
* @return int
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlVArrayDPoint3d_extend
(
EmbeddedDPoint3dArray     *pHeader,
int                 n
)
    {
    if ( pHeader )
        {
#if defined (INCLUDE_CRTDBG)
        return  DPoint3dArrayWrapper::setBufferSize2(pHeader, n, __FILE__, __LINE__);
#else
        return  DPoint3dArrayWrapper::setBufferSize(pHeader, n);
#endif
        }
    else
        return ERROR;
    }


/**
* Reallocate the buffer to accommodate exactly n DPoint3ds.
* NOTE: this will truncate the contents of this instance if its count is
* greater than n.
*
* @param    pHeader
* @param    n       Number of DPoint3ds to accommodate, no more, no less.
* @return false if unable to reallocate the buffer.
* @bsihdr                                       DavidAssaf      03/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlVArrayDPoint3d_setExactBufferSize
(
EmbeddedDPoint3dArray     *pHeader,
int                 n
)
    {
    if (pHeader)
        {
        return DPoint3dArrayWrapper::setBufferSize (pHeader, n);
        }
    else
        return false;
    }

/* MAP define jmdlVArrayDPoint3d_getCount(pHeader) pHeader->getCount() ENDMAP */

/**
* @param    pHeader
* @see
* @return int
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlVArrayDPoint3d_getCount
(
const EmbeddedDPoint3dArray *pHeader
)
    {
    if (pHeader)
        {
        return DPoint3dArrayWrapper::getCount(pHeader);
        }
    else
        return  ERROR;
    }


/* MAP define jmdlVArrayDPoint3d_addPoint(pHeader,pPoint) pHeader->add(pPoint) ENDMAP */

/**
* @param    pHeader
* @param pPoint
* @see
* @return int
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlVArrayDPoint3d_addPoint
(
EmbeddedDPoint3dArray         *pHeader,
const DPoint3d          *pPoint
)
    {
    if (pHeader)
        {
        return  DPoint3dArrayWrapper::insert(pHeader,  pPoint, -1);
        }
    else
        return  ERROR;
    }


/**
* @param pPoint <= point to be normalized and added to the array.
* @return true if the point had non-zero weight.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlVArrayDPoint3d_addDPoint4d
(
EmbeddedDPoint3dArray         *pHeader,
const DPoint4d          *pPoint
)
    {
    DPoint3d point;
    if (pHeader && bsiDPoint4d_normalize (pPoint, &point))
        {
        return SUCCESS == DPoint3dArrayWrapper::insert(pHeader,  &point, -1);
        }
    else
        return  false;
    }



/* MAP define jmdlVArrayDPoint3d_insert(pHeader,pPoint,index) pHeader->insert(pPoint,index) ENDMAP */

/**
* @param pHeader
* @param pPoint
* @param index
* @see
* @return int
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlVArrayDPoint3d_insert
(
        EmbeddedDPoint3dArray         *pHeader,
const   DPoint3d                *pPoint,
        int                     index
)
    {
    if (pHeader)
        return  DPoint3dArrayWrapper::insert(pHeader,  pPoint, index);
    else
        return  ERROR;
    }


/* MAP define jmdlVArrayDPoint3d_addArray(pHeader,pPoint,n) pHeader->addArray(pPoint,n) ENDMAP */

/**
* @param pHeader <=> header of array receiveing points
* @param pPoint => array of points to add
* @param n => number of points to add
* @see
* @return int
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlVArrayDPoint3d_addArray
(
      EmbeddedDPoint3dArray       *pHeader,
const DPoint3d              *pPoint,
      int                   n
)
    {
    if (pHeader)
#if defined (INCLUDE_CRTDBG)
        return  DPoint3dArrayWrapper::insert2 (pHeader,  pPoint, -1, n, __FILE__, __LINE__);
#else
        return  DPoint3dArrayWrapper::insert (pHeader,  pPoint, -1, n);
#endif
    else
        return  ERROR;
    }


/* MAP define jmdlVArrayDPoint3d_addDPoint3dArrayAtIndex(pHeader,pPoint,index,n) pHeader->addArrayAtIndex(pPoint,index,n) ENDMAP */

/**
* @param pHeader <=> header of array receiveing points
* @param pPoint => array of points to add
* @param index => index location for adding the array
* @param n => number of points to add
* @see
* @return int
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlVArrayDPoint3d_addDPoint3dArrayAtIndex
(
      EmbeddedDPoint3dArray       *pHeader,
const DPoint3d              *pPoint,
      int                   index,
      int                   n
)
    {
    if (pHeader)
#if defined (INCLUDE_CRTDBG)
        return  DPoint3dArrayWrapper::insert2 (pHeader,  pPoint, index, n, __FILE__, __LINE__);
#else
        return  DPoint3dArrayWrapper::insert (pHeader,  pPoint, index, n);
#endif
    else
        return  ERROR;
    }


/* MAP define jmdlVArrayDPoint3d_getArray(pHeader,pPoint,nGot,i0,nreq) pHeader->getArray(pPoint,nGot,i0,nreq) ENDMAP */

/**
* @param    pHeader
* @param pPoint
* @param nGot
* @param i0
* @param nreq
* @see
* @return SUCCESS if index nGot > 0
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlVArrayDPoint3d_getArray
(
const EmbeddedDPoint3dArray   *pHeader,
DPoint3d                *pPoint,
int                     *nGot,
int                     i0,
int                     nreq
)
    {
    int status = SUCCESS;
    *nGot = DPoint3dArrayWrapper::getArray(pHeader,  pPoint, i0, nreq);

    if (*nGot <= 0)
        status = ERROR;

    return  status;
    }

/* MAP define jmdlVArrayDPoint3d_getDPoint3d(pHeader,pointP,index) pHeader->get(pointP,index) ENDMAP */

/**
* @param    pHeader
* @param pointP
* @param index
* @see
* @return SUCCESS if index is valid
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt jmdlVArrayDPoint3d_getDPoint3d
(
const EmbeddedDPoint3dArray   *pHeader,
DPoint3d                *pointP,
int                     index
)
    {

    if (pHeader)
        return  DPoint3dArrayWrapper::get (pHeader,  pointP, index);
    else
        return  ERROR;
    }


/* MAP define jmdlVArrayDPoint3d_setDPoint3d(pHeader,pointP,index) pHeader->setDPoint3d(pointP,index) ENDMAP */

/**
* @param pHeader
* @param pointP
* @param index
* @see
* @return SUCCESS if
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt jmdlVArrayDPoint3d_setDPoint3d
(
EmbeddedDPoint3dArray   *pHeader,
DPoint3d          *pointP,
int               index
)
    {

    if (pHeader)
        return  DPoint3dArrayWrapper::set (pHeader,  pointP, index);
    else
        return  ERROR;
    }


/* MAP define jmdlVArrayDPoint3d_crossProduct3Points(pHeader,pProduct,index0,index1,2) pHeader->crossProduct3Points(pProduct,index0,index1,2) ENDMAP */

/**
* Compute the cross product of the vectors from point 0 to point 1 and
* point 0 to point 2.
* @param pProduct <= cross product vector
* @param pHeader => array of points
* @param index0 => reference point index
* @param index1 => target point of vector 1
* @param index2 => target point of vector 2
* @see
* @return SUCCESS if
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt jmdlVArrayDPoint3d_crossProduct3Points
(
const EmbeddedDPoint3dArray   *pHeader,   // => array of points
DPoint3d                *pProduct,  // <= cross product vector
int                     index0,     // => reference point index
int                     index1,     // => target point of vector 1
int                     index2      // => target point of vector 2
)
    {
    DPoint3d pointArray[3];

    if (   pHeader
        && SUCCESS == DPoint3dArrayWrapper::get (pHeader, &pointArray[0], index0)
        && SUCCESS == DPoint3dArrayWrapper::get (pHeader, &pointArray[1], index1)
        && SUCCESS == DPoint3dArrayWrapper::get (pHeader, &pointArray[2], index2)
        )
        {
        bsiDPoint3d_crossProduct3DPoint3d (pProduct,
                    &pointArray[0], &pointArray[1], &pointArray[2]);
        return SUCCESS;
        }
    else
        return  ERROR;
    }


/* MAP define jmdlVArrayDPoint3d_getBlock(pHeader,n) pHeader->getNewBlock(n) ENDMAP */

/**
* @param pHeader <=> array from which to get block
* @param n => number of entries requested
* @see
* @return Temporary pointer to block.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP DPoint3d *jmdlVArrayDPoint3d_getBlock
(
EmbeddedDPoint3dArray *pHeader,
int             n
)
    {
    if (pHeader)
        return  (DPoint3d*) DPoint3dArrayWrapper::getNewBlock (pHeader, n);
    else
        return  NULL;
    }


/* MAP define jmdlVArrayDPoint3d_getIndexedArray(pHeader,pVertex,maxVertex,pIndex,nIndex) pHeader->getIndexedArray(pVertex,maxVertex,pIndex,nIndex) ENDMAP */

/**
* @param pVertex Packed vertex array
* @param int            maxVertex vertex array limit
* @param pHeader master vertex array
* @param pIndex index array
* @param int            nIndex               number of vertices
* @see
* @return int
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlVArrayDPoint3d_getIndexedArray
(
const   EmbeddedDPoint3dArray *pHeader,
DPoint3d        *pVertex,
        int             maxVertex,
        int             *pIndex,
        int             nIndex
)
    {
    int n = 0;
    int status, index;
    int i;
    if (pHeader)
        {
        int maxIndex = DPoint3dArrayWrapper::getCount (pHeader);
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
                if (SUCCESS == (status = DPoint3dArrayWrapper::get
                        (pHeader, &pVertex[i], index)))
                    n++;
                }
            }

        }
    return n;
    }


/* MAP define jmdlVArrayDPoint3d_getPtr(pHeader,index) pHeader->getPtr(index) ENDMAP */

/**
* @param pHeader
* @param index
* @see
* @return const
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP DPoint3d* jmdlVArrayDPoint3d_getPtr
(
EmbeddedDPoint3dArray *pHeader,
      int        index
)
    {
    if (pHeader)
        return  (DPoint3d*) DPoint3dArrayWrapper::getPtr (pHeader, index);
    else
        return  NULL;
    }


/* MAP define jmdlVArrayDPoint3d_getConstPtr(pHeader,index) pHeader->getConstPtr(index) ENDMAP */

/**
* @param pHeader
* @param index
* @see
* @return const
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP const DPoint3d* jmdlVArrayDPoint3d_getConstPtr
(
const EmbeddedDPoint3dArray *pHeader,
      int        index
)
    {
    if (pHeader)
        return  (DPoint3d*) DPoint3dArrayWrapper::getConstPtr (pHeader, index);
    else
        return  NULL;
    }


/* MAP define jmdlVArrayDPoint3d_getRange(pHeader,pRange) pHeader->getRange(pRange) ENDMAP */

/**
* @param pHeader => point array header
* @param pRange <= computed range. Not set if no points.
* @see
* @return ERROR if no points in array.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt jmdlVArrayDPoint3d_getRange
(
const EmbeddedDPoint3dArray   *pHeader,
DRange3d                *pRange
)
    {
    StatusInt status = ERROR;
    int numPoint ;
    if (pHeader &&
        (numPoint = DPoint3dArrayWrapper::getCount (pHeader)) > 0)
        {
        bsiDRange3d_initFromArray (
                pRange,
                (DPoint3d *)DPoint3dArrayWrapper::getConstPtr (pHeader, 0),
                numPoint
                );
        status = SUCCESS;
        }
    return status;
    }


/* MAP define jmdlVArrayDPoint3d_getPolygonPlane(pHeader,pNormal,pOrigin,pError) pHeader->getPolygonPlane(pNormal,pOrigin,pError) ENDMAP */

/**
* @param    pHeader => point array header
* @param pNormal <= plane normal
* @param pOrigin <= plane origin
* @param pError <= nonplanarity estimate.
* @see
* @return ERROR if no points in array.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt jmdlVArrayDPoint3d_getPolygonPlane
(
const EmbeddedDPoint3dArray   *pHeader,
DPoint3d                *pNormal,
DPoint3d                *pOrigin,
double                  *pError
)
    {
    StatusInt status = ERROR;
    int numPoint;
    const DPoint3d *pPoint;
    DPoint3d origin, normal;
    if (pError)
        *pError = 0.0;

    if (pHeader &&
        (numPoint = DPoint3dArrayWrapper::getCount (pHeader)) > 0)
        {
        pPoint = (const DPoint3d *)DPoint3dArrayWrapper::getConstPtr (pHeader, 0);

        status = bsiGeom_polygonNormal (
                &normal,
                &origin,
                pPoint,
                numPoint
                ) ? SUCCESS : ERROR;

        if (pOrigin)
            *pOrigin = origin;
        if (pNormal)
            *pNormal = normal;
        if (SUCCESS == status && pError)
            {
            double maxError = 0.0;
            double thisError;
            int i;
            for (i = 0; i < numPoint; i++)
                {
                thisError = fabs (bsiDPoint3d_dotDifference
                                        (
                                        pPoint + i,
                                        &origin,
                                        (DVec3d *) &normal
                                        ));
                if (thisError > maxError)
                    maxError = thisError;
                }
            *pError = maxError;
            }
        }
    return status;
    }


/* MAP define jmdlVArrayDPoint3d_applyTransform(pDest,pTransform) pDest->transform(pTransform) ENDMAP */

/**
* @param pHeader <=> array to transform
* @param pTransform => transform to apply
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlVArrayDPoint3d_applyTransform
(
EmbeddedDPoint3dArray     *pHeader,
const DTransform3d  *pTransform
)
    {
    int numPoint ;
    DPoint3d *pPointArray;
    if (pHeader &&
        (numPoint = DPoint3dArrayWrapper::getCount (pHeader)) > 0)
        {
        pPointArray = (DPoint3d *)DPoint3dArrayWrapper::getPtr (pHeader, 0);
        bsiDTransform3d_multiplyDPoint3dArray (pTransform, pPointArray, pPointArray, numPoint);
        }
    }



/* MAP define jmdlVArrayDPoint3d_applyTransform2(pDest,pSource,pTransform) pDest->transform(pSource,pTransform) ENDMAP */

/**
* @param pDest => destination array
* @param pSource <=> array to transform
* @param pTransform => transform to apply
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlVArrayDPoint3d_applyTransform2
(
EmbeddedDPoint3dArray   *pDest,
const EmbeddedDPoint3dArray   *pSource,
const DTransform3d  *pTransform
)
    {
    int numPoint ;
    const DPoint3d *pSourceBuffer;
    DPoint3d *pDestBuffer;

    if (pSource && pDest &&
        (numPoint = DPoint3dArrayWrapper::getCount (pSource)) > 0)
        {
        pSourceBuffer = (DPoint3d *)DPoint3dArrayWrapper::getConstPtr
                    (pSource, 0);
        DPoint3dArrayWrapper::empty (pDest);
        pDestBuffer   = (DPoint3d *)DPoint3dArrayWrapper::getNewBlock
                    (pDest, numPoint);
        bsiDTransform3d_multiplyDPoint3dArray (pTransform, pDestBuffer, pSourceBuffer, numPoint);
        }
    }


/* MAP define jmdlVArrayDPoint3d_applyMatrix(pHeader,pMatrix) pHeader->transform(pMatrix) ENDMAP */

/**
* @param pHeader <=> array to transform
* @param pMatrix => transform to apply
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlVArrayDPoint3d_applyMatrix
(
EmbeddedDPoint3dArray     *pHeader,
const DMatrix3d     *pMatrix
)
    {
    int numPoint ;
    DPoint3d *pPointArray;
    if (pHeader &&
        (numPoint = DPoint3dArrayWrapper::getCount (pHeader)) > 0)
        {
        pPointArray = (DPoint3d *)DPoint3dArrayWrapper::getPtr (pHeader, 0);
        bsiDMatrix3d_multiplyDPoint3dArray (pMatrix, pPointArray, pPointArray, numPoint);
        }
    }



/* MAP define jmdlVArrayDPoint3d_swapValues(pHeader,index1,index2) pHeader->swapValues(index1,index2) ENDMAP */

/**
* @param pHeader
* @param index1
* @param index2
* @see
* @return SUCCESS if
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt jmdlVArrayDPoint3d_swapValues
(
EmbeddedDPoint3dArray *pHeader,
int             index1,
int             index2
)
    {
    if (pHeader)
        return  DPoint3dArrayWrapper::swapValues (pHeader, index1, index2);
    else
        return  ERROR;
    }


/* MAP define jmdlVArrayDPoint3d_copy(pDestHeader,pSourceHeader) pDestHeader->copy(pSourceHeader) ENDMAP */

/**
* @param pDestHeader
* @param pSourceHeader
* @see
* @return SUCCESS if
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt jmdlVArrayDPoint3d_copy
(
EmbeddedDPoint3dArray *pDestHeader,
const EmbeddedDPoint3dArray *pSourceHeader
)
    {
    if (pDestHeader)
#if defined (INCLUDE_CRTDBG)
        return  DPoint3dArrayWrapper::copy2 (pDestHeader, pSourceHeader, __FILE__, __LINE__);
#else
        return  DPoint3dArrayWrapper::copy (pDestHeader, pSourceHeader);
#endif
    else
        return  ERROR;
    }
#ifdef CompileSort

/**
* @param pDestHeader
* @param VBArray_SortFunction pFunction
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlVArrayDPoint3d_sort
(
EmbeddedDPoint3dArray *pDestHeader,
VBArray_SortFunction pFunction
)
    {
    if (pDestHeader)
        {
        DPoint3dArrayWrapper::sort (pDestHeader, pFunction);
        }
    }
#endif

/**
* Writes a DPoint3d array to the stream.
*
* @param pStream    <=> data stream ptr
* @param pArrayHdr  => header for DPoint3d array to write to data stream
* @param pFuncs     => callback bundle for reading/writing to the stream
* @return false if error; true if success
* @see #jmdlVArrayDPoint3d_loadFromDataStream
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlVArrayDPoint3d_storeToDataStream
(
void                    *pStream,
const EmbeddedDPoint3dArray   *pArrayHdr,
const MTG_IOFuncs       *pFuncs
)
    {
    if (!pArrayHdr)
        return false;

    // write count
    int numDoubles = 3 * DPoint3dArrayWrapper::getCount (pArrayHdr);
    if (1 != pFuncs->pWriteInts (pStream, &numDoubles, 1))
        return false;

    // write DPoint3d array as array of doubles
    if (numDoubles > 0)
        {
        const double *pArray;
        pArray = (const double *)DPoint3dArrayWrapper::getConstPtr (pArrayHdr, 0);
        if (numDoubles != pFuncs->pWriteDoubles (pStream, pArray, numDoubles))
            return false;
        }

    return true;
    }


/**
* Writes or appends to a DPoint3d array the given stream of doubles, as written
* by jmdlVArrayDPoint3d_storeToDataStream.
*
* @param pArrayHdr  <=> header for DPoint3d array to read from data stream
* @param pStream    <=> data stream ptr
* @param pFuncs     => callback bundle for reading/writing to the stream
* @param bAppend    => true to append new points; false to overwrite old points
* @return false if error; true if success
* @see #jmdlVArrayDPoint3d_storeToDataStream
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlVArrayDPoint3d_loadFromDataStream
(
EmbeddedDPoint3dArray           *pArrayHdr,
void                    *pStream,
const MTG_IOFuncs       *pFuncs,
bool                    bAppend
)
    {
    if (!pArrayHdr)
        return false;

    // read count
    int numDoubles, numPoints;
    if (1 != pFuncs->pReadInts (&numDoubles, pStream, 1))
        return false;
    numPoints = numDoubles / 3;

    // fill pList as array of doubles
    if (numDoubles > 0)
        {
        double *pArray;
        if (!bAppend)
            DPoint3dArrayWrapper::empty (pArrayHdr);
        pArray = (double *) DPoint3dArrayWrapper::getNewBlock (pArrayHdr, numPoints);
        if (numDoubles != pFuncs->pReadDoubles (pArray, pStream, numDoubles))
            return false;
        }

    return true;
    }


/**
* Add points of a transfinite mapping grid.
* The 4 input arrays contain the boundary points in order around the boundary.
* Nominally, the last point in each array must match the zeroth of the next array.
* However, the code never references any point indexed num0-1 or num1-1 -- it
* always uses point 0 of the "next" array.   That is, corner points are
* taken from beginning of the array for the outgoing side, not
* from the end of array for the incoming side.
*
* Points are entered from left to right along each row, starting at the bottom.
* @param pXYZ0 => array of num0 points along lower edge, left to right.
* @param pXYZ1 => array of num1 points along right edge, bottom to top.
* @param pXYZ2 => array of num0 points along upper edge, right to left
* @param pXYZ3 => array of num1 points along left edge, top to bottom.
* @param num0 => number of points along bottom and top edges of grid.  num0 - 1 points
*               of pXYZ0 and pXYZ2 are referenced.
* @param num1 => number of points along left and right edges of grid.  num1 - 1 points
*               of pXYZ1 and pXYZ3 are referenced.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlVArrayDPoint3d_addTransfiniteGrid
(
EmbeddedDPoint3dArray *pArray,
DPoint3d *pXYZ0,
DPoint3d *pXYZ1,
DPoint3d *pXYZ2,
DPoint3d *pXYZ3,
int       num0,
int       num1
)
    {
    int i, j;
    int k0, k1, k2, k3;
    double fi, fj;
    DPoint3d Xij, X0, X1;
    DPoint3d X00, X01, X10, X11;
    DPoint3d X;
    DPoint3d U0, U1;
    DPoint3d Uij;
    int m0 = num0 - 1;
    int m1 = num1 - 1;
    X00 = pXYZ0[0];
    X10 = pXYZ1[0];
    X01 = pXYZ3[0];
    X11 = pXYZ2[0];
    /* Explicit bottom row ... */
    for (i = 0; i < m0; i++)
        jmdlVArrayDPoint3d_addPoint (pArray, pXYZ0 + i);
    jmdlVArrayDPoint3d_addPoint (pArray, pXYZ1);

    for (j = 1; j < m1; j++)
        {
        fj = (double) j / (double)(m1);
        jmdlVArrayDPoint3d_addPoint (pArray, pXYZ3 + m1 - j);
        for (i = 1; i < m0; i++)
            {
            fi = (double) i / (double)(m0);
            k0 = i;
            k1 = j;
            k2 = m0 - i;
            k3 = m1 - j;
            bsiDPoint3d_interpolate (&X0, &X00, fi, &X10);
            bsiDPoint3d_interpolate (&X1, &X01, fi, &X11);
            bsiDPoint3d_interpolate (&Xij, &pXYZ3[k3], fi, &pXYZ1[k1]);
            bsiDPoint3d_subtractDPoint3dDPoint3d (&U0, &pXYZ0[k0], &X0);
            bsiDPoint3d_subtractDPoint3dDPoint3d (&U1, &pXYZ2[k2], &X1);
            bsiDPoint3d_interpolate (&Uij, &U0, fj, &U1);
            bsiDPoint3d_addDPoint3dDPoint3d (&X, &Xij, &Uij);
            jmdlVArrayDPoint3d_addPoint (pArray, &X);
            }
        jmdlVArrayDPoint3d_addPoint (pArray, pXYZ1 + j);
        }

    /* Explicit top row ... */
    jmdlVArrayDPoint3d_addPoint (pArray, pXYZ3);
    for (i = 1; i <= m0; i++)
        jmdlVArrayDPoint3d_addPoint (pArray, pXYZ2 +  m0 - i);
    }

/*----------------------------------------------------------------------+
|FUNC           compareDPoint2dX                                        |
|AUTHOR         EarlinLutz                              06/00           |
+----------------------------------------------------------------------*/
Private int compareDPoint2dX
(
const void    *pElem1,
const void    *pElem2
)
    {
    DPoint2d *pA = (DPoint2d*) pElem1;
    DPoint2d *pB= (DPoint2d*) pElem2;
    if (pA->x < pB->x)
        return -1;
    if (pA->x > pB->x)
        return 1;
    return 0;
    }

/*----------------------------------------------------------------------+
|FUNC           arePointsClose_absXYZ                                   |
|AUTHOR         EarlinLutz                              07/01           |
+----------------------------------------------------------------------*/
Private bool    arePointsClose_absXYZ
(
const DPoint3d *pPoint0,
const DPoint3d *pPoint1,
double epsilon
)
    {
    return  fabs (pPoint0->x - pPoint1->x) <= epsilon
        &&  fabs (pPoint0->y - pPoint1->y) <= epsilon
        &&  fabs (pPoint0->z - pPoint1->z) <= epsilon
        ;
    }

/*----------------------------------------------------------------------+
|FUNC           arePointsClose_absXY                                    |
|AUTHOR         EarlinLutz                              07/01           |
+----------------------------------------------------------------------*/
Private bool    arePointsClose_absXY
(
const DPoint3d *pPoint0,
const DPoint3d *pPoint1,
double epsilon
)
    {
    return  fabs (pPoint0->x - pPoint1->x) <= epsilon
        &&  fabs (pPoint0->y - pPoint1->y) <= epsilon
        ;
    }

typedef bool    (*PointComparisonFunction)
    (
    const DPoint3d *pPoint0,
    const DPoint3d *pPoint1,
    double epsilon
    );

/*--------------------------------------------------------------------*//*
* @param pXYZArray => array of n points, containing possibly matched points.
* @param pCycleArray => array of n indices, arranged as cyclic linked lists
*               joining points with identical points.  May be null pointer.
* @param pBlockedIndexArray => array containing packed blocks of point indices,
*               each terminated by index -1.  This will contain at least n+1
*               and at most 2n indices.  May be null pointer.
* @param absTol = absolute tolerance for common points.
* @param relTol = relative tolerance for common points.
* @return number of distinct points, hence number of cycles and blocks
*               in the index arrays.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Private int jmdlVArrayDPoint3d_identifyMatchedVertices_generic
(
const EmbeddedDPoint3dArray *pXYZArray,
EmbeddedIntArray      *pCycleArray,
EmbeddedIntArray      *pBlockedIndexArray,
double          absTol,
double          relTol,
DPoint3d        *pSortVector,
PointComparisonFunction cb_pointsClose
)
    {
    const DPoint3d *pXYZBuffer = jmdlVArrayDPoint3d_getConstPtr (pXYZArray, 0);
    int numXYZ = jmdlVArrayDPoint3d_getCount (pXYZArray);

    EmbeddedDPoint2dArray *pSortArray = jmdlEmbeddedDPoint2dArray_grab();
    DPoint3d normalizedSortVector;
    DPoint2d *pSortBuffer;
    int *pCycleBuffer;
    DPoint3d point0, point1;
    double lowerBlockCoordinate, upperBlockCoordinate;
    int i0, i1;
    int k0, k1, k2;
    int numBlock = 0;

    double largestCoordinate = bsiDPoint3d_getLargestCoordinateDifference (pXYZBuffer, numXYZ);
    double epsilon;

    if (numXYZ == 0)
        {
        if (pBlockedIndexArray)
            jmdlVArrayInt_clear (pBlockedIndexArray);
        if (pCycleArray)
            jmdlVArrayInt_clear (pCycleArray);
        return 0;
        }

    if (absTol <= 0.0)
        absTol = 0.0;
    if (relTol <= 0.0)
        relTol = 0.0;

    epsilon = absTol + relTol * largestCoordinate;
    bsiDPoint3d_normalize (&normalizedSortVector, pSortVector);

    jmdlEmbeddedDPoint2dArray_ensureCapacity (pSortArray, numXYZ);
    /* Force data into the sort area.. */
    jmdlEmbeddedDPoint2dArray_setDPoint2d (pSortArray, NULL, numXYZ - 1);
    pSortBuffer = jmdlEmbeddedDPoint2dArray_getPtr (pSortArray, 0);

    if (pBlockedIndexArray)
        {
        jmdlVArrayInt_clear (pBlockedIndexArray);
        jmdlVArrayInt_extend (pBlockedIndexArray, (numXYZ * 3) / 2);
        }

    /* Search for non-disconnect reference point */
    for (k0 = 0; k0 < numXYZ; k0++)
        {
        if (!bsiDPoint3d_isDisconnect (pXYZBuffer + k0))
            break;
        }

    point0 = pXYZBuffer[k0];
    /* Initialize sort indices as identity permutation with dot product along
        skewed dimension as sort quantity. */
    for (;k0 < numXYZ; k0++)
        {
        if (!bsiDPoint3d_isDisconnect (pXYZBuffer + k0))
            {
            pSortBuffer[k0].x = bsiDPoint3d_dotDifference (pXYZBuffer + k0, &point0, (DVec3d *) &normalizedSortVector);
            pSortBuffer[k0].y = (double)k0;
            }
        }

    if (pCycleArray)
        {
        jmdlVArrayInt_clear (pCycleArray);
        jmdlVArrayInt_extend (pCycleArray, numXYZ);
        for (k0 = 0; k0 < numXYZ; k0++)
            {
            jmdlVArrayInt_addInt (pCycleArray, k0);
            }
        pCycleBuffer = jmdlVArrayInt_getPtr (pCycleArray, 0);
        }
    else
        {
        pCycleBuffer = NULL;
        }

    qsort ((void*) pSortBuffer, numXYZ, sizeof (DPoint2d), compareDPoint2dX);

    for (i0 = 0; i0 < numXYZ; i0++)
        {
        k0 = (int)pSortBuffer[i0].y;
        if (k0 >= 0)
            {
            lowerBlockCoordinate = pSortBuffer[i0].x;
            upperBlockCoordinate = lowerBlockCoordinate + epsilon;
            /* This coordinate starts a new block.
               Record it and all succeeding near points into a block of the index array.
               The points that are near in ALL directions are clustered in a block
               with almost the same sort coordinate;  walk through the block
               of points with similar sort coordinate, begin aware that there may
               be (a) points previously picked out from prior blocks, (index -1)
                    (b) points far away but not yet recorded.
               In the cycle index array, each index is initially a singleton cycle.
               When a point is identified as part of a block, it is still a singleton
               cycle.  The singleton cycle is spliced together with the growing block
               cycle by swapping successor indices athe the new point and the base
               point of the block.
            */
            if (pBlockedIndexArray)
                jmdlVArrayInt_addInt (pBlockedIndexArray, k0);
            point0 = pXYZBuffer[k0];
            numBlock++;
            for (i1 = i0 + 1;
                 i1 < numXYZ && pSortBuffer[i1].x < upperBlockCoordinate;
                 i1++)
                {
                k1 = (int)pSortBuffer[i1].y;
                if (k1 >= 0)
                    {
                    point1 = pXYZBuffer[k1];
                    if (cb_pointsClose (&point0, &point1, epsilon))
                        {
                        pSortBuffer[i1].y = -1;
                        if (pBlockedIndexArray)
                            jmdlVArrayInt_addInt (pBlockedIndexArray, k1);
                        if (pCycleBuffer)
                            {
                            k2 = pCycleBuffer[k0];
                            pCycleBuffer[k0] = k1;
                            pCycleBuffer[k1] = k2;
                            }
                        }
                    }
                }
            if (pBlockedIndexArray)
                jmdlVArrayInt_addInt (pBlockedIndexArray, -1);
            }
        }
    jmdlEmbeddedDPoint2dArray_drop (pSortArray);
    return numBlock;
    }


/*--------------------------------------------------------------------*//*
* @param pXYZArray => array of n points, containing possibly matched points.
* @param pCycleArray => array of n indices, arranged as cyclic linked lists
*               joining points with identical points.  May be null pointer.
* @param pBlockedIndexArray => array containing packed blocks of point indices,
*               each terminated by index -1.  This will contain at least n+1
*               and at most 2n indices.  May be null pointer.
* @param absTol = absolute tolerance for common points.
* @param relTol = relative tolerance for common points.
* @return number of distinct points, hence number of cycles and blocks
*               in the index arrays.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlVArrayDPoint3d_identifyMatchedVertices
(
const EmbeddedDPoint3dArray *pXYZArray,
EmbeddedIntArray      *pCycleArray,
EmbeddedIntArray      *pBlockedIndexArray,
double          absTol,
double          relTol
)
    {
    DPoint3d sortVector;
    sortVector.x = 0.5677470545;
    sortVector.y = 1.8340234005;
    sortVector.z = 1.3472498290;
    return jmdlVArrayDPoint3d_identifyMatchedVertices_generic (pXYZArray, pCycleArray, pBlockedIndexArray,
                    absTol, relTol, &sortVector, arePointsClose_absXYZ);
    }


/*--------------------------------------------------------------------*//*
* @param pXYZArray => array of n points, containing possibly matched points.
* @param pCycleArray => array of n indices, arranged as cyclic linked lists
*               joining points with identical points.  May be null pointer.
* @param pBlockedIndexArray => array containing packed blocks of point indices,
*               each terminated by index -1.  This will contain at least n+1
*               and at most 2n indices.  May be null pointer.
* @param absTol = absolute tolerance for common points.
* @param relTol = relative tolerance for common points.
* @return number of distinct points, hence number of cycles and blocks
*               in the index arrays.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlVArrayDPoint3d_identifyMatchedVerticesXY
(
const EmbeddedDPoint3dArray *pXYZArray,
EmbeddedIntArray      *pCycleArray,
EmbeddedIntArray      *pBlockedIndexArray,
double          absTol,
double          relTol
)
    {
    DPoint3d sortVector;
    sortVector.x = 0.5677470545;
    sortVector.y = 1.8340234005;
    sortVector.z = 0.0;
    return jmdlVArrayDPoint3d_identifyMatchedVertices_generic (pXYZArray, pCycleArray, pBlockedIndexArray,
                    absTol, relTol, &sortVector, arePointsClose_absXY);
    }
END_BENTLEY_NAMESPACE
