/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/gpa/graphicspointarray.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/GeomPrivateApi.h>
#include "ArrayWrapper.h"

// Bindings for varray usage in graphicspointarray.cpp ...
#define omdlVArray_init GPAWrapper::init
#define omdlVArray_releaseMem GPAWrapper::releaseMem
#define omdlVArray_empty GPAWrapper::empty
#define omdlVArray_getCount GPAWrapper::getCount
#define omdlVArray_getPtr GPAWrapper::getPtr
#define omdlVArray_getConstPtr GPAWrapper::getConstPtr

#define omdlVArray_get  GPAWrapper::get
#define omdlVArray_insert GPAWrapper::insert
#define omdlVArray_set  GPAWrapper::set
#define omdlVArray_pop  GPAWrapper::pop

#define omdlVArray_setBufferSize GPAWrapper::setBufferSize
#define omdlVArray_append GPAWrapper::append
#define omdlVArray_trim GPAWrapper::trim
#define boolean_omdlVArray_copy GPAWrapper::booleanCopy

typedef VArrayWrapper<GraphicsPoint> GPAWrapper;



#ifdef USE_CACHE
#include "geommem.h"
#include "ptrcache.h"
#include "../bsiinc/Geom/embeddedarraycachemanager.fdf"
#include <Mstn/Tools/ToolsAPI.h>
#endif
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/*----------------------------------------------------------------------+
|                                                                       |
|   Local defines                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
#define HPOINT_CACHE_SIZE 16
#define BLOCKSIZE 1024
#define GET_ELEMENT_PTR(_pInstance_,_index_) \
        ((GraphicsPoint *)omdlVArray_getPtr (&(_pInstance_)->vbArray_hdr,(_index_)))

#define GET_CONST_ELEMENT_PTR(_pInstance_,_index_) \
        ((GraphicsPoint *)omdlVArray_getConstPtr (&(_pInstance_)->vbArray_hdr,(_index_)))

#define GET_ELEMENT(_pInstance_,_pDestPoint_,_index_)           \
            (SUCCESS == omdlVArray_get (                        \
                            &((_pInstance_)->vbArray_hdr),      \
                            (_pDestPoint_),             \
                            (_index_)))

#define GET_COUNT(_pInstance_) (omdlVArray_getCount (&(_pInstance_)->vbArray_hdr))

static const int s_allCurveBits = HPOINT_MASK_CURVETYPE_BITS | HPOINT_MASK_POINTTYPE_BITS;
static const int s_bezierEndMask = HPOINT_MASK_CURVETYPE_BEZIER | HPOINT_MASK_BEZIER_STARTEND;
static const int s_bezierPoleMask = HPOINT_MASK_CURVETYPE_BEZIER | HPOINT_MASK_BEZIER_POLE;
//static const int s_bsplineEndMask = HPOINT_MASK_CURVETYPE_BSPLINE | HPOINT_MASK_BSPLINE_STARTEND;

#define IS_BEZIER_ENDPOINT_MASK(_mask_)    \
            (((_mask_) & s_allCurveBits) == s_bezierEndMask)

#define IS_BEZIER_POLE_MASK(_mask_)    \
            (((_mask_) & s_allCurveBits) == s_bezierPoleMask)

#define IS_BSPLINE_STARTEND_MASK(_mask_)    \
            (((_mask_) & s_allCurveBits) == s_bsplineEndMask)

#define IS_CURVE_POINT_MASK(_mask_)    \
            (((_mask_) & s_allCurveBits) != 0)

#define IS_NON_POLE_MASK(_mask_)    \
            (((_mask_) & HPOINT_MASK_CURVETYPE_BITS) == HPOINT_MASK_CURVETYPE_ELLIPSE)

#define xEXTEND_GPA_BY(pGPA,n) jmdlGraphicsPointArray_extendBy (pGPA, n)
#define EXTEND_GPA_BY(pGPA,n) true
/*----------------------------------------------------------------------+
|                                                                       |
|   Local type definitions                                              |
|                                                                       |
+----------------------------------------------------------------------*/
#ifdef USE_CACHE
/*----------------------------------------------------------------------+
|                                                                       |
|   Private Global variables                                            |
|                                                                       |
+----------------------------------------------------------------------*/
static PtrCache_Functions cacheFunctions =
    {
    (void *(*)(void))jmdlGraphicsPointArray_new,
    (void (*)(void *))jmdlGraphicsPointArray_free,
    (void (*)(void *))jmdlGraphicsPointArray_emptyAll,
    NULL
    };

static PPtrCacheHeader pCache = NULL;
#define INIT_PTR_CACHE  initCache ();
/*----------------------------------------------------------------------+
* Initialize and register the cache.
+----------------------------------------------------------------------*/
static void initCache
(
void
)
    {
    if (!pCache)
        {
        pCache = omdlPtrCache_new (&cacheFunctions, HPOINT_CACHE_SIZE);
        jmdlEmbeddedArrayManager_registerCache (pCache);
        }
    }
#endif
/*----------------------------------------------------------------------+
|                                                                       |
|   Public GEOMDLLIMPEXP Global variables                                             |
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
/*----------------------------------------------------------------------+
|SECTION GraphicsPointArray                                             |
|Functions jmdlHPoint_XXXX provice servics for manipulating arrays of   |
|homogeneous points and accompanying label data used in graphics.       |
|                                                                       |
| User code deals with the arrays via an opaque pointer to a header     |
| structure.  All operations on the array are provided by function      |
| calls.   Using these arrays for recurring graphics operations is      |
| both simpler and more efficient than in-line code.                    |
|                                                                       |
| GraphicsPointArray arrays are implemented as EmbeddedStructArray rubber arrays, i.e.|
| are compatible with mjava StructArray_hdr.                            |
|                                                                       |
| Each array element contains:                                          |
| 1) point -- the 4d point                                              |
| 2) a -- an additional double, for use as temporary during clipping.   |
| 3) mask -- an integer addressed as bits.                              |
| 4) userData -- an integer, for use as a temporary                     |
|                                                                       |
| An ellipse is packed into the array as follows:                       |
|<UL>                                                                   |
|<LI>homogeneous start point.                                           |
|<LI>homogeneous vector0                                                |
|<LI>homogeneous center                                                 |
|<LI>homogeneous vector90                                               |
|<LI>homogeneous end point                                              |
|</UL>                                                                  |
| Ellipses are always stored with a positive sweep.   The ellipse can   |
| always be reversed by reversing all its points.  (Start and end have  |
| the same tag; vector0 and vector90 have the same tag.)
+----------------------------------------------------------------------*/


/*======================================================================+
|                                                                       |
|   Major Public GEOMDLLIMPEXP Code Section                                           |
|                                                                       |
+======================================================================*/


/*---------------------------------------------------------------------------------**//**
* jmdlGraphicsPointArray_new allocates (from the heap) a header structure for a
* GraphicsPointArray.  The array initially contains no points.
* @return pointer to the allocated header.  NULL if allocation failed.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP GraphicsPointArrayP jmdlGraphicsPointArray_new

(
void
)
    {
    GraphicsPointArrayP pInstance = new GraphicsPointArray ();

    if ( pInstance )
            jmdlGraphicsPointArray_init (pInstance);
    return pInstance;
    }




/*---------------------------------------------------------------------------------**//**
*
* Initialize a GraphicsPointArray structure.   To be called immediately after
* the structure is obtained from heap or stack.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_init

(
GraphicsPointArrayP pInstance
)
    {
    if ( pInstance )
        {
        pInstance->vbArray_hdr.clear ();
        pInstance->arrayMask = HPOINT_ARRAYMASK_DEFAULT;
        }
    }


/*---------------------------------------------------------------------------------**//**
* Release all memory associated with a GraphicsPointArray header (but not the header itself)
* Reinitialize the structure.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_releaseMem

(
GraphicsPointArrayP pInstance
)
    {
    if (NULL != pInstance)
        pInstance->vbArray_hdr.clear ();
    }



/*---------------------------------------------------------------------------------**//**
* jmdlGraphicsPointArray_free frees an GraphicsPointArray header and its associated arrays.
* @return always returns NULL.                                           *
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP GraphicsPointArrayP jmdlGraphicsPointArray_free

(
GraphicsPointArrayP pInstance
)
    {
    if (NULL != pInstance)
        delete pInstance;
    return NULL;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| Normal code structure in a process that uses a small number of point  |
| arrays at any one time but many over the entire process:              |
| for each step                                                         |
|   if ( pInstance = jmdlGraphicsPointArray_grab() )                                  |
|       {                                                               |
|       ... process the stuff                                           |
|       jmdlGraphicsPointArray_drop ( pInstance );                                    |
|       }                                                               |
|                                                                       |
+----------------------------------------------------------------------*/





/*---------------------------------------------------------------------------------**//**
* jmdlGraphicsPointArray_grab borrows an GraphicsPointArray header from the cache.
*   The caller is responsible for returning the header via
*   jmdlGraphicsPointArray_drop or jmdlGraphicsPointArray_free.
* @return a pointer to the borrowed header.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP GraphicsPointArrayP jmdlGraphicsPointArray_grab

(
void
)
    {
#ifdef USE_CACHE
    GraphicsPointArrayP pInstance;

    INIT_PTR_CACHE;

    pInstance = (GraphicsPointArrayP) omdlPtrCache_grabFromCache (pCache);
    return pInstance;
#else
    return jmdlGraphicsPointArray_new ();
#endif
    }



/*---------------------------------------------------------------------------------**//**
* jmdlGraphicsPointArray_drop gives the GraphicsPointArray header and its arrays back to the
* cache of GraphicsPointArray headers available for borrowing.
* @return jmdlGraphicsPointArray_drop always returns NULL
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP GraphicsPointArrayP jmdlGraphicsPointArray_drop

(
GraphicsPointArrayP  pInstance
)
    {
#ifdef USE_CACHE
    INIT_PTR_CACHE;

    omdlPtrCache_dropToCache (pCache, pInstance);
    return NULL;
#else
    return jmdlGraphicsPointArray_free (pInstance);
#endif
    }

/*---------------------------------------------------------------------------------**//**

* jmdlGraphicsPointArray clear sets the number of points in a GraphicsPointArray
* to zero.  The array buffers and additional header labeling are retained.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlGraphicsPointArray_empty
(
GraphicsPointArrayP pInstance
)
    {
    if (NULL != pInstance)
        {
        pInstance->vbArray_hdr.clear ();
        pInstance->arrayMask = 0;
        }
    }


/*---------------------------------------------------------------------------------**//**

* jmdlGraphicsPointArray clear sets the number of points in a GraphicsPointArray
* to zero, and resets header data (e.g. array mask).
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlGraphicsPointArray_emptyAll
(
GraphicsPointArrayP pInstance
)
    {
    jmdlGraphicsPointArray_empty (pInstance);
    pInstance->arrayMask = HPOINT_ARRAYMASK_DEFAULT;
    }

/*---------------------------------------------------------------------------------**//**
* @return number of graphics points currently in the array.  Note that this
* may be less than the allocated capacity of the array.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlGraphicsPointArray_getCount
(
GraphicsPointArrayCP pInstance
)
    {
    return NULL == pInstance ? 0 : (int)pInstance->vbArray_hdr.size ();
    }

/*---------------------------------------------------------------------------------**//**
* Trim the array.
* @param newCount => requested number of points.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlGraphicsPointArray_trim
(
GraphicsPointArrayP pInstance,
int newCount
)
    {
    if (NULL != pInstance)
        pInstance->vbArray_hdr.resize ((size_t)newCount);
    }

/*---------------------------------------------------------------------------------**//**
* @return const pointer to an element of the array.  Beware that this pointer may
* become invalid if the array contents change.
* @param index => index of array element to return. -1 for last element.
* @return pointer to array element.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP const GraphicsPoint *jmdlGraphicsPointArray_getConstPtr
(
GraphicsPointArrayCP pInstance,
int     index
)
    {
    return GET_CONST_ELEMENT_PTR (pInstance, index);
    }



/*---------------------------------------------------------------------------------**//**
* Swap the content of two arrays.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_swap
(
GraphicsPointArrayP pInstance,
GraphicsPointArrayP pHeader
)
    {
    GraphicsPointArray tempHeader = *pInstance;
    *pInstance = *pHeader;
    *pHeader = tempHeader;
    }


/*---------------------------------------------------------------------------------**//**
* Copy the array and header parts of pHeader into this.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        jmdlGraphicsPointArray_copyContentsOf
(
GraphicsPointArrayP pInstance,
GraphicsPointArrayCP pHeader
)
    {
    pInstance->arrayMask = pHeader->arrayMask;
    return boolean_omdlVArray_copy (&pInstance->vbArray_hdr, &pHeader->vbArray_hdr);
    }


/*---------------------------------------------------------------------------------**//**
* Append the array from pHeader.  Major break and curve masks from pHeader are OR'ed
* into the receiving header.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        jmdlGraphicsPointArray_appendArray
(
GraphicsPointArrayP pInstance,
GraphicsPointArrayCP pHeader
)
    {
    pInstance->arrayMask |= (pHeader->arrayMask & (HPOINT_ARRAYMASK_HAS_MAJOR_BREAKS | HPOINT_ARRAYMASK_CURVES));
    return SUCCESS == omdlVArray_append (&pInstance->vbArray_hdr, &pHeader->vbArray_hdr);
    }


/*---------------------------------------------------------------------------------**//**
* Ensure that the array capacity is at least n points.
*
* @param n => number of points expected to be added later.
* @return true if the requested capacity is available.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_extend
(
GraphicsPointArrayP pInstance,
int n
)
    {
    return SUCCESS == omdlVArray_setBufferSize (&pInstance->vbArray_hdr, n);
    }


/*---------------------------------------------------------------------------------**//**
* Ensure that the array can hold at least n more points than at present.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       jmdlGraphicsPointArray_extendBy
(
GraphicsPointArrayP pInstance,
int n
)
    {
    return jmdlGraphicsPointArray_extend
                (
                pInstance,
                GET_COUNT (pInstance) + n
                );
    }



/*---------------------------------------------------------------------------------**//**
* Mark the break between disconnected line segments.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlGraphicsPointArray_markBreak
(
GraphicsPointArrayP         pInstance
)
    {
    GraphicsPoint *pElement = GET_ELEMENT_PTR(pInstance, -1);
    if (pElement)
        pElement->mask |= HPOINT_MASK_BREAK;
    }


/*---------------------------------------------------------------------------------**//**
* Mark the break between multiple-loop polygons (e.g. between characters of text)
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlGraphicsPointArray_markMajorBreak
(
GraphicsPointArrayP         pInstance
)
    {
    GraphicsPoint *pElement = GET_ELEMENT_PTR(pInstance, -1);
    if (pElement)
        {
        pElement->mask |= HPOINT_MASK_MAJOR_BREAK | HPOINT_MASK_BREAK;
        pInstance->arrayMask |= HPOINT_ARRAYMASK_HAS_MAJOR_BREAKS;
        }

    }


/*---------------------------------------------------------------------------------**//**
* Mark the break between multiple-loop polygons (e.g. between characters of text) at a particular index.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlGraphicsPointArray_markMajorBreakAt
(
GraphicsPointArrayP         pInstance,
int i
)
    {
    GraphicsPoint *pElement = GET_ELEMENT_PTR(pInstance, i);
    if (pElement)
        {
        pElement->mask |= HPOINT_MASK_MAJOR_BREAK | HPOINT_MASK_BREAK;
        pInstance->arrayMask |= HPOINT_ARRAYMASK_HAS_MAJOR_BREAKS;
        }

    }




/*---------------------------------------------------------------------------------**//**
* Test if the specified point of the array is a major break.
* @param index => index of point to test.  -1 indicates last point of array.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlGraphicsPointArray_isMajorBreak
(
GraphicsPointArrayCP         pInstance,
int                             index
)
    {
    const GraphicsPoint *pElement = GET_CONST_ELEMENT_PTR(pInstance, index);
    return pElement ? (0 != (pElement->mask & HPOINT_MASK_MAJOR_BREAK)) : false;
    }



/*---------------------------------------------------------------------------------**//**
* query if the array has major breaks (i.e. if the array describes closed loops
* to be filled.)
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_hasMajorBreaks
(
GraphicsPointArrayCP         pInstance
)
    {
    return 0 != (pInstance->arrayMask & HPOINT_ARRAYMASK_HAS_MAJOR_BREAKS);
    }


/*---------------------------------------------------------------------------------**//**
* Set the userData field of the ending fragment in the array.
* A fragment is an individual line segment, ellipse segment, or bezier fragment.
* (When multiple line segments are concatentated as a linestring, the
* final two points are marked.   On repetetive addition of single fragments,
* markup of the first point of the final fragment replaces prior markup from
* when that point was the second point of the prior fragment.)
* For ellipse and bezier data, the markup is applied to the entire fragment.
*
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlGraphicsPointArray_setFinalPrimitveFragmentUserData
(
GraphicsPointArrayP         pInstance,
int                            userData
)
    {
    int i0;
    int i1 = GET_COUNT (pInstance);
    int i;
    GraphicsPoint *pElement = GET_ELEMENT_PTR(pInstance, 0);
    if (jmdlGraphicsPointArray_parseFinalPrimitiveFragment (pInstance, &i0, &i1,
                        NULL, NULL, NULL))
        {
        for (i = i0; i <= i1; i++)
            {
            pElement[i].userData = userData;
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* Set the userData field of all array entries beginning with a given index and
* ending at the end of the array.
* @param i0         => first index to mark.
* @param userData   => data to apply.
* @return start index for subsequent calls.
*
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      jmdlGraphicsPointArray_setTailUserData
(
GraphicsPointArrayP         pInstance,
int                            i0,
int                            userData
)
    {
    GraphicsPoint *pElement = GET_ELEMENT_PTR(pInstance, 0);
    int i1 = GET_COUNT (pInstance);
    int i;

    if (i0 < 0)
        i0 = 0;
    for (i = i0; i < i1; i++)
        pElement[i].userData = userData;
    return i1;
    }


/*---------------------------------------------------------------------------------**//**
* Set the "userData" (integer) field at a given index.
* @param index  => index of graphics point
* @param value  => value to set
* @return true if valid index.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_setUserData
(
GraphicsPointArrayP         pInstance,
int                            index,
int                            value
)
    {
    GraphicsPoint *pGP = GET_ELEMENT_PTR(pInstance, index);
    if (pGP)
        pGP->userData = value;
    return pGP ? true : false;
    }


/*---------------------------------------------------------------------------------**//**
* Set the "a" (parameter) field at a given index.
* @param index  => index of graphics point
* @param value  => value to set
* @return true if valid index.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_setParameter
(
GraphicsPointArrayP         pInstance,
int                            index,
double                         value
)
    {
    GraphicsPoint *pGP = GET_ELEMENT_PTR(pInstance, index);
    if (pGP)
        pGP->a = value;
    return pGP ? true : false;
    }


/*---------------------------------------------------------------------------------**//**

* Apply (by bitwise OR) a mask to the final point in the array.
* @param    pInstance
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlGraphicsPointArray_setFinalPointMask
(
GraphicsPointArrayP         pInstance,
int             mask
)
    {
    GraphicsPoint *pElement = GET_ELEMENT_PTR(pInstance, -1);
    if (pElement)
        pElement->mask |= mask;
    }



/*---------------------------------------------------------------------------------**//**

* Apply BREAK and POINT masks to the final point in the array, and BREAK mask to
*   second last if present.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlGraphicsPointArray_markPoint
(
GraphicsPointArrayP         pInstance
)
    {
    int n = omdlVArray_getCount (&pInstance->vbArray_hdr);
        GraphicsPoint *pElement = GET_ELEMENT_PTR(pInstance, -1);

    if  ( pElement)
        {
        pElement->mask |= HPOINT_MASK_POINT | HPOINT_MASK_BREAK;
        if (n > 1)
            pElement[-1].mask |= HPOINT_MASK_BREAK;
        }
    }


/*---------------------------------------------------------------------------------**//**
* Set bits in the summary mask for the array.
* @param    mask    => bits to OR with the array mask.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_setArrayMask
(
GraphicsPointArrayP pInstance,
int             mask
)
    {
    pInstance->arrayMask |= mask;
    }


/*---------------------------------------------------------------------------------**//**
* Get bits in the summary mask for the array.
* @param    mask    => bits to AND with the array mask.
* @return   array mask ANDed with the indicated bits.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      jmdlGraphicsPointArray_getArrayMask
(
GraphicsPointArrayCP pInstance,
int             mask
)
    {
    return pInstance->arrayMask & mask;
    }


/*---------------------------------------------------------------------------------**//**
* Get mask bits for indicated point.
* @return   point mask ORed with the given mask bits.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      jmdlGraphicsPointArray_getPointMask
(
GraphicsPointArrayCP pInstance,
int             index,
int             mask
)
    {
    const GraphicsPoint *pElement = GET_CONST_ELEMENT_PTR (pInstance, index);
    return pElement ? pElement->mask & mask : 0;
    }


/*---------------------------------------------------------------------------------**//**
* Clear bits in the summary mask for the array.
* @param    mask    => bits to clear in the array mask.
* @return   array mask ANDed with the indicated bits.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_clearArrayMask
(
GraphicsPointArrayP pInstance,
int             mask
)
    {
    pInstance->arrayMask &= ~mask;
    }


/*---------------------------------------------------------------------------------**//**
* Clear bits in the in/out bits in the mask for each point.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_clearAllInOutBits
(
GraphicsPointArrayP pInstance
)
    {
    GraphicsPoint *pBuffer = jmdlGraphicsPointArray_getPtr (pInstance, 0);
    int numPoint = jmdlGraphicsPointArray_getCount (pInstance);

    bsiGraphicsPoint_setInOutArray (pBuffer, numPoint, false, false);
    }


/*---------------------------------------------------------------------------------**//**
* Set or clear a mask on all points in the array.
* @param mask => mask to write, e.g. HPOINT_MASK_USER1
* @param value => zero to clear, nonzero to set.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_writeMaskAllPoints
(
GraphicsPointArrayP pInstance,
int mask,
int value
)
    {
    GraphicsPoint *pBuffer = jmdlGraphicsPointArray_getPtr (pInstance, 0);
    int numPoint = jmdlGraphicsPointArray_getCount (pInstance);
    int i;

    if (value)
        {
        for (i = 0; i < numPoint; i++)
            {
            pBuffer[i].mask |= mask;
            }
        }
    else
        {
        int otherMasks = ~mask;

        for (i = 0; i < numPoint; i++)
            {
            pBuffer[i].mask &= otherMasks;
            }
        }
    }



/*---------------------------------------------------------------------------------**//**
* Copy the array mask from one header to another.
* @param    mask    => bits to clear in the array mask.
* @return   array mask ANDed with the indicated bits.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_copyArrayMask
(
GraphicsPointArrayP pInstance,
GraphicsPointArrayP pSource
)
    {
    pInstance->arrayMask = pSource->arrayMask;
    }



/*---------------------------------------------------------------------------------**//**
*
* Add a point to the array.
*
* @param    pInstance <=> header to receive new point
* @param pPoint => point being added.
* @return true unless rubber array could not be extended.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_addDPoint3d
(
GraphicsPointArrayP pInstance,
const DPoint3d              *pPoint
)
    {
    GraphicsPoint  element;
    bsiGraphicsPoint_init (&element,
                    pPoint->x, pPoint->y, pPoint->z, 1.0,
                    0.0, HPOINT_NORMAL, 0);
    pInstance->vbArray_hdr.push_back (element);
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* Construct a graphics point whose DPoint4d is the coefficients of a plane
* with given origin and normal.  No point is inserted if the normal is zero.
* @param pOrigin => any point on plane.
* @param pNormal => outward normal vector.
* @return false if the normal is zero length.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_addPlaneByOriginAndNormal
(
GraphicsPointArrayP pInstance,
const DPoint3d              *pOrigin,
const DPoint3d              *pNormal
)
    {
    GraphicsPoint gp;
    memset (&gp, 0, sizeof (gp));
    if (bsiDPoint4d_planeFromOriginAndNormal (&gp.point, pOrigin, pNormal))
        {
        pInstance->vbArray_hdr.push_back (gp);
        return true;
        }
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* Construct a graphics point whose DPoint4d is the coefficients of a plane
*  through 3 given points, with outward normal determined by the
*   cross product of vectors from orgin to point1 and point2.
* with given
* @param pOrigin => any point on plane.
* @param pPoint1 => another in-plane point.
* @param pPoint2 => another in-plane point.
* @return false if the points are colinear.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_addPlaneBy3DPoint3d
(
GraphicsPointArrayP pInstance,
const DPoint3d              *pOrigin,
const DPoint3d              *pPoint1,
const DPoint3d              *pPoint2
)
    {
    DPoint3d normal;
    bsiDPoint3d_crossProduct3DPoint3d (&normal, pOrigin, pPoint1, pPoint2);
    return jmdlGraphicsPointArray_addPlaneByOriginAndNormal (pInstance, pOrigin, &normal);
    }

/*---------------------------------------------------------------------------------**//**
*
* Add a point to the array.
*
* @param    pInstance <=> header to receive new point
* @param pPoint => point being added
* @return true unless rubber array could not be extended.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_addGraphicsPoint
(
GraphicsPointArrayP pInstance,
const GraphicsPoint                 *pPoint
)
    {
    pInstance->vbArray_hdr.push_back (*pPoint);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
*
* Insert a graphics point at specified index.
*
* @param    pInstance <=> header to receive new point
* @param pPoint => point being added
* @param index => position for point. -1 adds at end.
* @return true unless rubber array could not be extended.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_insertGraphicsPoint
(
GraphicsPointArrayP pInstance,
const GraphicsPoint          *pPoint,
      int                    index
)
    {
    return SUCCESS == omdlVArray_insert (&pInstance->vbArray_hdr, pPoint, index);
    }

/*---------------------------------------------------------------------------------**//**
* Add 2 points to the array, marking the second as a break.
* @param    pInstance <=> header to receive new point
* @param pPoint => point being added
* @return true unless rubber array could not be extended.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_addGraphicsPointSegment
(
GraphicsPointArrayP pInstance,
const GraphicsPoint                *pPoint0,
const GraphicsPoint                *pPoint1
)
    {
    pInstance->vbArray_hdr.push_back (*pPoint0);
    pInstance->vbArray_hdr.push_back (*pPoint1);
    jmdlGraphicsPointArray_markBreak (pInstance);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
*
* Add a point to the array.
*
* @param    pInstance <=> header to receive new point
* @param pPoint => point being set
* @param index => index to set
* @return true unless rubber array could not be extended
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_setGraphicsPoint
(
GraphicsPointArrayP pInstance,
const GraphicsPoint                 *pPoint,
      int                           index
)
    {
    return SUCCESS == omdlVArray_set (&pInstance->vbArray_hdr, pPoint, index);
    }


/*---------------------------------------------------------------------------------**//**
*
* Add a point to the array.
*
* @param    pInstance <=> header to receive new point
* @param pPoint => point being added.
* @return true unless rubber array could not be extended.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_addDPoint2d
(
GraphicsPointArrayP pInstance,
const DPoint2d              *pPoint
)
    {
    GraphicsPoint  element;
    bsiGraphicsPoint_init (&element,
                    pPoint->x, pPoint->y, 0.0, 1.0,
                    0.0, HPOINT_NORMAL, 0);
    pInstance->vbArray_hdr.push_back (element);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
*
* Add a point to the array.
*
* @param    pInstance <=> header to receive new point
* @param x => x coordinate
* @param y => y coordinate
* @param z => z coordinate
* @param w => w coordinate
* @return true unless no memory
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_addComponents
(
GraphicsPointArrayP pInstance,
double                  x,
double                  y,
double                  z,
double                  w
)
    {
    GraphicsPoint  element;
    bsiGraphicsPoint_init (&element,
                    x, y, z, w,
                    0.0, HPOINT_NORMAL, 0);
    pInstance->vbArray_hdr.push_back (element);
    return true;
    }


/*---------------------------------------------------------------------------------**//**
*
* Add a point to the array.
*
* @param    pInstance <=> header to receive new point
* @param x => x coordinate
* @param y => y coordinate
* @param z => z coordinate
* @param w => w coordinate
* @param a => extra data value
* @param mask => mask value
* @param userData => user data value
* @return true unless no memory
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_addComplete
(
GraphicsPointArrayP pInstance,
double                  x,
double                  y,
double                  z,
double                  w,
double                  a,
int                     mask,
int                     userData
)
    {
    GraphicsPoint  element;
    bsiGraphicsPoint_init (&element,
                    x, y, z, w, a, mask, userData);
    pInstance->vbArray_hdr.push_back (element);
    return true;
    }


/*---------------------------------------------------------------------------------**//**
*
* Add a point to the array.
*
* @param    pInstance <=> header to receive new point
* @param pPoint => point being added.
* @param mask   => mask to apply to the point.
* @return true unless no memory
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_addDPoint4dWithMask
(
GraphicsPointArrayP pInstance,
const   DPoint4d      *pPoint,
            int                   mask
)
    {
    GraphicsPoint  element;
    bsiGraphicsPoint_initFromDPoint4d (&element, pPoint, 0.0, mask, 0);
    pInstance->vbArray_hdr.push_back (element);
    return true;
    }


/*---------------------------------------------------------------------------------**//**
*
* Add a point to the array.
*
* @param    pInstance <=> header to receive new point
* @param pPoint => point being added.
* @param mask   => mask to apply to the point.
* @return true unless no memory
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_addDPoint4dWithMaskExt
(
GraphicsPointArrayP pInstance,
const   DPoint4d      *pPoint,
int                   mask,
double                b
)
    {
    GraphicsPoint  element (*pPoint, 0.0, 0, mask, b);
    pInstance->vbArray_hdr.push_back (element);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
*
* Add a point to the array.
*
* @param    pInstance <=> header to receive new point
* @param pPoint => point being added.
* @param mask   => mask to apply to the point.
* @return true unless no memory
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
bool     jmdlGraphicsPointArray_addDPoint4dWithMaskAndIndex
(
GraphicsPointArrayP pInstance,
const   DPoint4d      *pPoint,
int                   mask,
double                b,
size_t                index
)
    {
    GraphicsPoint  element (*pPoint, 0.0, 0, mask, b, index);
    pInstance->vbArray_hdr.push_back (element);
    return true;
    }



/*---------------------------------------------------------------------------------**//**
*
* Add a point with floating point data value.
*
* @param pPoint => point to be added.
* @param dataValue => floating point data
* @return true unless no memory
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_addDPoint4dWithData
(
GraphicsPointArrayP pInstance,
const   DPoint4d            *pPoint,
        double              dataValue

)
    {
    GraphicsPoint  element;
    element.point = *pPoint;
    element.a = dataValue;
    element.mask = HPOINT_NORMAL;
    element.userData = 0;
    pInstance->vbArray_hdr.push_back (element);
    return true;
    }


/*---------------------------------------------------------------------------------**//**
*
* Add a point to the array.
*
* @param    pInstance <=> header to receive new point
* @param pPoint => point being added.
* @param mask   => mask to apply to the point.
* @return true unless rubber array could not be extended.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_addDPoint3dWithMask
(
GraphicsPointArrayP pInstance,
const   DPoint3d      *pPoint,
            int                   mask
)
    {
    GraphicsPoint  element;
    bsiGraphicsPoint_initFromDPoint3d (&element, pPoint, 1.0, 0.0, mask, 0);
    pInstance->vbArray_hdr.push_back (element);
    return true;
    }


/*---------------------------------------------------------------------------------**//**
*
* Add a point to the array.
*
* @param    pInstance <=> header to receive new point
* @param pPoint => point being added.
* @param mask   => mask to apply to the point.
* @return true unless rubber array could not be extended.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_addDPoint4d
(
GraphicsPointArrayP pInstance,
const   DPoint4d      *pPoint
)
    {
    GraphicsPoint  element;
    bsiGraphicsPoint_initFromDPoint4d (&element, pPoint, 0.0, HPOINT_NORMAL, 0);
    pInstance->vbArray_hdr.push_back (element);
    return true;
    }



/**
*
* Add a point array to the array.
*
* @param    pInstance <=> header of array receiveing points
* @param pPoint => array of points to add
* @param n => number of points to add
* @return SUCCESS unless array allocation failed.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlGraphicsPointArray_addDPoint3dArray
(
GraphicsPointArrayP pInstance,
const DPoint3d          *pPoint,
      int               n
)
    {
    int i;
    for (i = 0; i < n; i++)
        {
        /* don't add disconnect point, but break at predecessor (if present) */
        if (pPoint[i].x == DISCONNECT || pPoint[i].y == DISCONNECT)
            jmdlGraphicsPointArray_markBreak (pInstance);
        else 
            jmdlGraphicsPointArray_addDPoint3d (pInstance, &pPoint[i]);
        }
    return true;
    }


/**
* @param    pInstance <=> header of array receiveing points
* @param pPoint => array of points to add
* @param n => number of points to add
* @return SUCCESS unless array allocation failed.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlGraphicsPointArray_addDPoint2dArray
(
GraphicsPointArrayP pInstance,
const DPoint2d      *pPoint,
      int           n
)
    {
    int i;

    if (!EXTEND_GPA_BY (pInstance, n))
        return false;

    for (i = 0; i < n; i++)
        {
        /* don't add disconnect point, but break at predecessor (if present) */
        if (pPoint[i].x == DISCONNECT)
            jmdlGraphicsPointArray_markBreak (pInstance);
        else if (!jmdlGraphicsPointArray_addDPoint2d (pInstance, &pPoint[i]))
                    return false;
        }
    return true;
    }



/**
* @param    pInstance <=> header of array receiveing points
* @param pPoint => array of points to add
* @param n => number of points to add
* @return true unless array allocation failed.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlGraphicsPointArray_addDPoint4dArray
(
GraphicsPointArrayP pInstance,
const   DPoint4d                *pPoint,
            int                         n
)
    {
    int i;

    if (!EXTEND_GPA_BY (pInstance, n))
        return false;

    for (i = 0; i < n; i++)
        {
        if (!jmdlGraphicsPointArray_addDPoint4d (pInstance, &pPoint[i]))
                    return false;
        }

    return true;
    }


/*---------------------------------------------------------------------------------**//**
* Add a single sector of a DEllipse4d to the GraphicsPointArray structure.
*
* @param        pEllipse => ellipse to insert.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_addDEllipse4dSector
(
GraphicsPointArrayP pInstance,
const   DEllipse4d  *pEllipse,
            int             index
)
    {
    double   theta0, theta1;
    bool    sectorOk = bsiDEllipse4d_getSector (pEllipse, &theta0, &theta1, index);

    if (sectorOk)
        {
        jmdlGraphicsPointArray_addDEllipse4dLimits (pInstance, pEllipse, theta0, theta1);
        }
    return sectorOk;
    }


/*---------------------------------------------------------------------------------**//**
* Add a DConic4d to the array.
* @param        pConic => ellipse to insert.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_addDConic4d
(
GraphicsPointArrayP pInstance,
const   DConic4d                *pConic0
)
    {
    DConic4d conic = *pConic0;

    // Force to positive sweep ...
    if (conic.sweep < 0.0)
        {
        conic.sweep *= -1.0;
        conic.start *= -1.0;
        conic.vector90.Negate ();
        }
    DPoint4d startPoint, endPoint;
    double theta0 = conic.start;
    double theta1 = conic.start + conic.sweep;

    //double theta0Eval = theta0;
    //double theta1Eval = theta1;

    bsiDConic4d_angleParameterToDPoint4d (&conic, &startPoint, theta0);
    bsiDConic4d_angleParameterToDPoint4d (&conic, &endPoint, theta1);
    jmdlGraphicsPointArray_addDPoint4dWithMaskExt (pInstance, &startPoint,
                    HPOINT_MASK_CURVETYPE_ELLIPSE | HPOINT_MASK_ELLIPSE_STARTEND, theta0);

    jmdlGraphicsPointArray_addDPoint4dWithMaskExt (pInstance, &conic.vector0,
                        HPOINT_MASK_CURVETYPE_ELLIPSE | HPOINT_MASK_ELLIPSE_VECTOR, 0.0);
    jmdlGraphicsPointArray_addDPoint4dWithMaskExt (pInstance, &conic.center,
                        HPOINT_MASK_CURVETYPE_ELLIPSE | HPOINT_MASK_ELLIPSE_CENTER, conic.sweep);
    jmdlGraphicsPointArray_addDPoint4dWithMaskExt (pInstance, &conic.vector90,
                        HPOINT_MASK_CURVETYPE_ELLIPSE | HPOINT_MASK_ELLIPSE_VECTOR, msGeomConst_piOver2);
    jmdlGraphicsPointArray_addDPoint4dWithMaskExt ( pInstance, &endPoint,
                    HPOINT_MASK_CURVETYPE_ELLIPSE | HPOINT_MASK_ELLIPSE_STARTEND, theta1);

    jmdlGraphicsPointArray_setArrayMask (pInstance, HPOINT_ARRAYMASK_CURVES);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Add a DConic4d to the array.
* @param        pConic => ellipse to insert.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_addDConic4dWithIndices
(
GraphicsPointArrayP pInstance,
const   DConic4d                *pConic,
int *pIndex0,
int *pIndex1
)
    {
    if (pIndex0)
        *pIndex0 = jmdlGraphicsPointArray_getCount (pInstance);
    bool    stat = jmdlGraphicsPointArray_addDConic4d (pInstance, pConic);
    if (pIndex1)
        *pIndex0 = jmdlGraphicsPointArray_getCount (pInstance) - 1;
    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* Test if two points (homogeneous) are close, possibly refering to the array
* for tolerance.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static  bool     jmdlGraphicsPointArray_closePoints
(
GraphicsPointArrayCP pInstance,
const   DPoint4d                *pPoint0,
const   DPoint4d                *pPoint1
)
    {
    double d2;
    double e2;
    /* When comparing sqared distance to squared point magnitude, this
        corresponds to a 10-e9 relative unsquared tolerance .. */
    static double s_localSquareRelTol = 1.0e-18;

    if (!bsiDPoint4d_realDistanceSquared (pPoint0, &d2, pPoint1))
        return false;

    if (d2 == 0.0)
        return true;

    e2 = bsiDPoint3d_magnitudeSquared ((DPoint3d *)pPoint0);
    if (d2 > s_localSquareRelTol * e2)
        return false;
    e2 = bsiDPoint3d_magnitudeSquared ((DPoint3d *)pPoint1);
    if (d2 > s_localSquareRelTol * e2)
        return false;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Add a DSegment4d to the array.
* @param        pSegment => segment to add.
* @param        connectIfPossible => true to enable test for end/start match.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_addDSegment4d
(
GraphicsPointArrayP pInstance,
const   DSegment4d              *pSegment,
        bool                    connectIfPossible
)
    {
    GraphicsPoint *pGP = GET_ELEMENT_PTR (pInstance, -1);
    int k = 0;

    if (pGP && !IS_CURVE_POINT_MASK(pGP->mask))
        {
        if (jmdlGraphicsPointArray_closePoints (pInstance, &pGP->point, &pSegment->point[0]))
            {
            k = 1;
            pGP->mask &= !HPOINT_MASK_BREAK;
            }
        else
            {
            pGP->mask |= HPOINT_MASK_BREAK;
            }
        }

    for (;k < 2; k++)
        jmdlGraphicsPointArray_addDPoint4dWithMask (pInstance, &pSegment->point[k],
                        k == 1 ? HPOINT_MASK_BREAK : 0);
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* Add a single sector of a DEllipse4d to the GraphicsPointArray structure.
*
* @param        pEllipse => ellipse to insert.
* @param    theta0  => start angle
* @param    theta1  => end angle
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_addDEllipse4dLimits
(
GraphicsPointArrayP pInstance,
const   DEllipse4d      *pEllipse,
            double                  theta0,
            double                  theta1
)
    {
    DConic4d conic;
    conic.center = pEllipse->center;
    conic.vector0 = pEllipse->vector0;
    conic.vector90 = pEllipse->vector90;
    conic.start = theta0;
    conic.sweep = theta1 - theta0;
    return jmdlGraphicsPointArray_addDConic4d (pInstance, &conic);
    }


/*---------------------------------------------------------------------------------**//**
* Add a DEllipse4d to the GraphicsPointArray structure.
*
* @param        pEllipse => ellipse to insert.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addDEllipse4d
(
GraphicsPointArrayP pInstance,
const   DEllipse4d  *pEllipse
)
    {
    int index;
    for (index = 0 ; jmdlGraphicsPointArray_addDEllipse4dSector (pInstance, pEllipse, index); index++)
        {
        }
    }


/*---------------------------------------------------------------------------------**//**
* Add a DEllipse3d to the GraphicsPointArray structure.
*
* @param        pEllipse => ellipse to insert.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addDEllipse3d
(
GraphicsPointArrayP pInstance,
const   DEllipse3d  *pEllipse
)
    {
    DConic4d conic;
    bsiDConic4d_initFromDEllipse3d (&conic, pEllipse);
    bsiDConic4d_initFromDEllipse3d (&conic, pEllipse);
    jmdlGraphicsPointArray_addDConic4d (pInstance, &conic);
    }


/*---------------------------------------------------------------------------------**//**
* Add an array of Bezier segments.  If "share" is set, the total number of poles is
*       (order * numSegment - numSegment + 1.  If "share" is not set, the total number of poles
*       is order * numSegment.
*
* @param        pPoleArray => array of poles.
* @param        order      => curve order (number of poles per Bezier segment)
* @param        numSegment => number of bezier segments.
* @param        share      => true if common points are shared
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addDPoint4dBezier
(
GraphicsPointArrayP pInstance,
const   DPoint4d            *pPoleArray,
            int                     order,
            int                     numSegment,
            bool                    share
)
    {
    int i, ii, j;
    int orderM1 = order - 1;
    int endStep = share ? 0 : 1;
    /* We ALWAYS expand into non-shared mode */
    //int numExpandedPoints = order * numSegment;
    int endMask = HPOINT_MASK_BEZIER_STARTEND | HPOINT_MASK_CURVETYPE_BEZIER;
    int poleMask = HPOINT_MASK_BEZIER_POLE | HPOINT_MASK_CURVETYPE_BEZIER;

    if (order >= 2 && EXTEND_GPA_BY (pInstance, numExpandedPoints))
        {
        j = 0;

        for (i = 0; i < numSegment; i++)
            {
            jmdlGraphicsPointArray_addDPoint4dWithMask (pInstance, &pPoleArray[j++], endMask);

            for (ii = 1; ii < orderM1 ; ii++)
                {
                jmdlGraphicsPointArray_addDPoint4dWithMask (pInstance, &pPoleArray[j++], poleMask);
                }

            jmdlGraphicsPointArray_addDPoint4dWithMask (pInstance, &pPoleArray[j], endMask);
            /* Last step through the input array might be zero or one depending on sharing logic */
            j += endStep;
            }

        jmdlGraphicsPointArray_setArrayMask (pInstance, HPOINT_ARRAYMASK_CURVES);
        }
    }



/*---------------------------------------------------------------------------------**//**
* Add an array of Bezier segments.  If "share" is set, the total number of poles is
*       (order * numSegment - numSegment + 1.  If "share" is not set, the total number of poles
*       is order * numSegment.
*
* @param        pPoleArray => array of poles.
* @param        order      => curve order (number of poles per Bezier segment)
* @param        numSegment => number of bezier segments.
* @param        share      => true if common points are shared
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlGraphicsPointArray_addDPoint4dBezierWithIndices
(
GraphicsPointArrayP pInstance,
        int                 *pIndex0,
        int                 *pIndex1,
const   DPoint4d            *pPoleArray,
        int                 order,
        int                 numSegment,
        bool                share
)
    {
    if (pIndex0)
        *pIndex0 = GET_COUNT (pInstance);
    jmdlGraphicsPointArray_addDPoint4dBezier (pInstance, pPoleArray, order, numSegment, share);

    if (pIndex1)
            *pIndex1 = GET_COUNT (pInstance) - 1;

    jmdlGraphicsPointArray_setArrayMask (pInstance, HPOINT_ARRAYMASK_CURVES);
    }



/*---------------------------------------------------------------------------------**//**
* Add an array of Bezier segments.  If "share" is set, the total number of poles is
*       (order * numSegment - numSegment + 1.  If "share" is not set, the total number of poles
*       is order * numSegment.
*
* @param        pPoleArray => array of poles.
* @param        order      => curve order (number of poles per Bezier segment)
* @param        numSegment => number of bezier segments.
* @param        share      => true if common points are shared
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addDPoint3dBezier
(
GraphicsPointArrayP pInstance,
const   DPoint3d            *pPoleArray,
            int                     order,
            int                     numSegment,
            bool                    share
)
    {
    int i, ii, j;
    int orderM1 = order - 1;
    int endStep = share ? 0 : 1;
    /* We ALWAYS expand into non-shared mode */
    //int numExpandedPoints = order * numSegment;
    int endMask = HPOINT_MASK_BEZIER_STARTEND | HPOINT_MASK_CURVETYPE_BEZIER;
    int poleMask = HPOINT_MASK_BEZIER_POLE | HPOINT_MASK_CURVETYPE_BEZIER;

    if (order >= 2 && EXTEND_GPA_BY (pInstance, numExpandedPoints))
        {
        j = 0;

        for (i = 0; i < numSegment; i++)
            {
            jmdlGraphicsPointArray_addDPoint3dWithMask (pInstance, &pPoleArray[j++], endMask);

            for (ii = 1; ii < orderM1 ; ii++)
                {
                jmdlGraphicsPointArray_addDPoint3dWithMask (pInstance, &pPoleArray[j++], poleMask);
                }

            jmdlGraphicsPointArray_addDPoint3dWithMask (pInstance, &pPoleArray[j], endMask);
            /* Last step through the input array might be zero or one depending on sharing logic */
            j += endStep;
            }

        jmdlGraphicsPointArray_setArrayMask (pInstance, HPOINT_ARRAYMASK_CURVES);
        }
    }


/*---------------------------------------------------------------------------------**//**
* Add an array of Bezier segments.  If "share" is set, the total number of poles is
*       (order * numSegment - numSegment + 1.  If "share" is not set, the total number of poles
*       is order * numSegment.
*
* @param        pPoleArray => array of poles.
* @param        pWeightArray => array of weights.  May be NULL (unit weight).
* @param        order      => curve order (number of poles per Bezier segment)
* @param        numSegment => number of bezier segments.
* @param        share      => true if common points are shared
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addDPoint3dWeightBezier
(
GraphicsPointArrayP pInstance,
const   DPoint3d                    *pPoleArray,
const   double                      *pWeightArray,
            int                     order,
            int                     numSegment,
            bool                    share
)
    {
    int i, ii, j;
    int orderM1 = order - 1;
    int endStep = share ? 0 : 1;
    /* We ALWAYS expand into non-shared mode */
    //int numExpandedPoints = order * numSegment;
    int endMask = HPOINT_MASK_BEZIER_STARTEND | HPOINT_MASK_CURVETYPE_BEZIER;
    int poleMask = HPOINT_MASK_BEZIER_POLE | HPOINT_MASK_CURVETYPE_BEZIER;
    DPoint4d currPoint;

    if (order >= 2 && EXTEND_GPA_BY (pInstance, numExpandedPoints))
        {
        j = 0;

        for (i = 0; i < numSegment; i++)
            {
            bsiDPoint4d_initFromDPoint3dAndWeight (
                    &currPoint,
                    &pPoleArray[j],
                    pWeightArray ? pWeightArray[j] : 1.0
                    );
            j++;
            jmdlGraphicsPointArray_addDPoint4dWithMask (pInstance, &currPoint, endMask);

            for (ii = 1; ii < orderM1 ; ii++)
                {
                bsiDPoint4d_initFromDPoint3dAndWeight (
                        &currPoint,
                        &pPoleArray[j],
                        pWeightArray ? pWeightArray[j] : 1.0
                        );
                j++;
                jmdlGraphicsPointArray_addDPoint4dWithMask (pInstance, &currPoint, poleMask);
                }

            bsiDPoint4d_initFromDPoint3dAndWeight (
                    &currPoint,
                    &pPoleArray[j],
                    pWeightArray ? pWeightArray[j] : 1.0
                    );
            jmdlGraphicsPointArray_addDPoint4dWithMask (pInstance, &currPoint, endMask);
            /* Last step through the input array might be zero or one depending on sharing logic */
            j += endStep;
            }

        jmdlGraphicsPointArray_setArrayMask (pInstance, HPOINT_ARRAYMASK_CURVES);
        }
    }

#define MAX_POLE_BUFFER 1000


/*---------------------------------------------------------------------------------**//**
* @description Add a 2D B-spline.  If pWeights is null, a non-rational B-spline is assumed.
*   If pKnots is null, a uniform knot sequence is assumed (and numKnots is ignored).
*
* @remarks The B-spline knots and multiplicities spanned by each Bezier are not recorded
*   in the GPA.  See mdlGPA_addBsplineCurve for this.
*
* @param        pPoles      => array of poles (weighted if rational).
* @param        pWeights    => array of numPoles weights or null.
* @param        numPoles    => number of poles.
* @param        pKnots      => full knot sequence or null.
* @param        numKnots    => number of knots.
* @param        bClosed     => true if B-spline is periodic; false if clamped open.
* @param        order       => order of B-spline (degree + 1)
* @bsimethod                                                    DavidAssaf      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addDPoint2dBspline
(
GraphicsPointArrayP pInstance,
const   DPoint2d        *pPoles,
const   double          *pWeights,
        int             numPoles,
const   double          *pKnots,
        int             numKnots,
        bool            bClosed,
        int             order
)
    {
    ExtractContext          Context;
    DPoint4d                pBezSegmentPoles[MAX_BEZIER_CURVE_ORDER], pBspHPoleBuffer[MAX_POLE_BUFFER];
    DPoint4d*               pBspHPoles = pBspHPoleBuffer;
    bvector<DPoint4d>   bspHPoles;
    int                     i;

    // favor static homogeneous pole array in most cases
    if (numPoles > MAX_POLE_BUFFER)
        {
        bspHPoles.reserve (numPoles);
        pBspHPoles = &bspHPoles[0];
        }

    for (i = 0; i < numPoles; i++)
        {
        pBspHPoles[i].x = pPoles[i].x;
        pBspHPoles[i].y = pPoles[i].y;
        pBspHPoles[i].z = 0.0;
        pBspHPoles[i].w = pWeights ? pWeights[i] : 1.0;
        }

    /* Add Bezier segments one at a time to the GPA */
    if (bsiBezierDPoint4d_extractNextBezierFromBsplineInit (&Context, pBspHPoles, numPoles, pKnots, numKnots,
                                                             RELATIVE_BSPLINE_KNOT_TOLERANCE, order, bClosed))
        {
        while (bsiBezierDPoint4d_extractNextBezierFromBspline (pBezSegmentPoles, &Context))
            jmdlGraphicsPointArray_addDPoint4dBezier (pInstance, pBezSegmentPoles, order, 1, false);

        bsiBezierDPoint4d_extractNextBezierFromBsplineEnd (&Context);
        }

    }


/*---------------------------------------------------------------------------------**//**
* @description Add a 3D B-spline.  If pWeights is null, a non-rational B-spline is assumed.
*   If pKnots is null, a uniform knot sequence is assumed (and numKnots is ignored).
*
* @remarks The B-spline knots and multiplicities spanned by each Bezier are not recorded
*   in the GPA.  See mdlGPA_addBsplineCurve for this.
*
* @param        pPoles      => array of poles (weighted if rational).
* @param        pWeights    => array of numPoles weights or null.
* @param        numPoles    => number of poles.
* @param        pKnots      => full knot sequence or null.
* @param        numKnots    => number of knots.
* @param        bClosed     => true if B-spline is periodic; false if clamped open.
* @param        order       => order of B-spline (degree + 1)
* @bsimethod                                                    DavidAssaf      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addDPoint3dBspline
(
GraphicsPointArrayP pInstance,
const   DPoint3d        *pPoles,
const   double          *pWeights,
        int             numPoles,
const   double          *pKnots,
        int             numKnots,
        bool            bClosed,
        int             order
)
    {
    ExtractContext          Context;
    DPoint4d                pBezSegmentPoles[MAX_BEZIER_CURVE_ORDER], pBspHPoleBuffer[MAX_POLE_BUFFER];
    DPoint4d*               pBspHPoles = pBspHPoleBuffer;
    bvector<DPoint4d>   bspHPoles;
    int                     i;

    // favor static homogeneous poles array in most cases
    if (numPoles > MAX_POLE_BUFFER)
        {
        bspHPoles.reserve (numPoles);
        pBspHPoles = &bspHPoles[0];
        }

    for (i = 0; i < numPoles; i++)
        {
        pBspHPoles[i].x = pPoles[i].x;
        pBspHPoles[i].y = pPoles[i].y;
        pBspHPoles[i].z = pPoles[i].z;
        pBspHPoles[i].w = pWeights ? pWeights[i] : 1.0;
        }

    /* Add Bezier segments one at a time to the GPA */
    if  (bsiBezierDPoint4d_extractNextBezierFromBsplineInit (&Context, pBspHPoles, numPoles, pKnots, bspknot_numberKnots (numPoles, order, bClosed),
                                                              RELATIVE_BSPLINE_KNOT_TOLERANCE, order, bClosed))
        {
        while (bsiBezierDPoint4d_extractNextBezierFromBspline (pBezSegmentPoles, &Context))
            jmdlGraphicsPointArray_addDPoint4dBezier (pInstance, pBezSegmentPoles, order, 1, false);

        bsiBezierDPoint4d_extractNextBezierFromBsplineEnd (&Context);
        }

    }


/*---------------------------------------------------------------------------------**//**
* Extract an ellipse from its packed GraphicsPointArray form.
*
* @param        pReadIndex <=> On input, index of first point of ellipse data.
*                               On output, the first point after the ellipse data.
* @param        pEllipse    <= extracted ellipse
* @param        pTheta0     <= start angle
* @param        pSweep      <= end angle
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       jmdlGraphicsPointArray_getDEllipse4d
(
GraphicsPointArrayCP pInstance,
                int                 *pReadIndex,
                DEllipse4d  *pEllipse,
            double      *pTheta0,
            double      *pSweep,
                DPoint4d    *pStartPoint,
                DPoint4d    *pEndPoint
)
    {
    bool    funcStat = false;
    DConic4d conic;

    if (jmdlGraphicsPointArray_getDConic4d (pInstance, pReadIndex, &conic, pTheta0, pSweep, pStartPoint, pEndPoint))
        {
        pEllipse->center = conic.center;
        pEllipse->vector0 = conic.vector0;
        pEllipse->vector90 = conic.vector90;
        bsiRange1d_setUncheckedArcSweep (&pEllipse->sectors, conic.start, conic.sweep);
        funcStat = true;
        }

    return funcStat;
    }


/*---------------------------------------------------------------------------------**//**
* Extract a conic from its packed form.
*
* @param        pReadIndex <=> On input, index of first point of ellipse data.
*                               On output, the first point after the ellipse data.
* @param        pConic    <= extracted ellipse, parabola, or hyperbola, parameterized
*                                   as a unit circle with homogeneous center and axis vectors.
* @param        pTheta0     <= start angle
* @param        pSweep      <= end angle
* @param        pStartPoint <= start point
* @param        pEndPoint   <= end point
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       jmdlGraphicsPointArray_getDConic4d
(
GraphicsPointArrayCP pInstance,
                int         *pReadIndex,
                DConic4d    *pConic,
                double      *pTheta0,
                double      *pSweep,
                DPoint4d    *pStartPoint,
                DPoint4d    *pEndPoint
)
    {
    int index = *pReadIndex;
    DPoint4d center, vector0, vector90;
    int centerMask, vector0Mask, vector90Mask, mask0, mask1;
    bool    funcStat = false;
    DPoint4d point0, point1;
    double theta0, theta1;

    /* Parse 5 points. Move the index ahead with each read EXCEPT the last */
    if (
           jmdlGraphicsPointArray_getDPoint4dWithMaskExt (pInstance, &point0, &mask0, &theta0, index++)
        && (mask0 & HPOINT_MASK_CURVE_BITS) == (HPOINT_MASK_CURVETYPE_ELLIPSE | HPOINT_MASK_ELLIPSE_STARTEND)
        && jmdlGraphicsPointArray_getDPoint4dWithMask (pInstance, &vector0, &vector0Mask, index++)
        && vector0Mask      == (HPOINT_MASK_CURVETYPE_ELLIPSE | HPOINT_MASK_ELLIPSE_VECTOR)
        && jmdlGraphicsPointArray_getDPoint4dWithMask (pInstance, &center, &centerMask, index++)
        && centerMask   == (HPOINT_MASK_CURVETYPE_ELLIPSE | HPOINT_MASK_ELLIPSE_CENTER)
        && jmdlGraphicsPointArray_getDPoint4dWithMask (pInstance, &vector90, &vector90Mask, index++)
        && vector90Mask == (HPOINT_MASK_CURVETYPE_ELLIPSE | HPOINT_MASK_ELLIPSE_VECTOR)
        && jmdlGraphicsPointArray_getDPoint4dWithMaskExt (pInstance, &point1, &mask1, &theta1, index++)
        && (mask1 & HPOINT_MASK_CURVE_BITS) == (HPOINT_MASK_CURVETYPE_ELLIPSE | HPOINT_MASK_ELLIPSE_STARTEND)
       )
        {
        pConic->center = center;
        pConic->vector0 = vector0;
        pConic->vector90 = vector90;
        pConic->start = theta0;
        pConic->sweep = theta1 - theta0;
        if (pTheta0)
                *pTheta0 = theta0;

        if (pSweep)
                *pSweep = pConic->sweep;

        if (pStartPoint)
                *pStartPoint = point0;

        if (pEndPoint)
                *pEndPoint = point1;

        funcStat = true;
        }

    if (funcStat)
        {
        *pReadIndex = index;
        }

    return funcStat;
    }


/*---------------------------------------------------------------------------------**//**
* Extract a conic from its packed form.
*
* @param        pReadIndex <=> On input, index of first point of ellipse data.
*                               On output, the first point after the segment data.
* @param        pConic    <= extracted ellipse, parabola, or hyperbola, parameterized
*                                   as a unit circle with homogeneous center and axis vectors.
* @param        pTheta0     <= start angle
* @param        pSweep      <= end angle
* @param        pStartPoint <= start point
* @param        pEndPoint   <= end point
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       jmdlGraphicsPointArray_getDSegment4d
(
GraphicsPointArrayCP pInstance,
                int         *pReadIndex,
                DSegment4d  *pSegment
)
    {
    int index = *pReadIndex;
    DPoint4d point0, point1;
    bool    funcstat = false;
    int mask0, mask1;

    if (jmdlGraphicsPointArray_getDPoint4dWithMask (pInstance, &point0, &mask0, index)
        && (mask0 & HPOINT_MASK_CURVETYPE_BITS) == 0
        && (mask0 & HPOINT_MASK_BREAK) == 0
        && jmdlGraphicsPointArray_getDPoint4dWithMask (pInstance, &point1, &mask1, index + 1)
        && (mask1 & HPOINT_MASK_CURVETYPE_BITS) == 0
       )
       {
       *pReadIndex = mask1 & HPOINT_MASK_BREAK ? index + 2 : index + 1;
       pSegment->point[0] = point0;
       pSegment->point[1] = point1;
       funcstat = true;
       }

    return funcstat;
    }


/*---------------------------------------------------------------------------------**//**
* Extract as DSegment3d.   Return ERROR for any failure -- no points, other type, or DSegment4d with infinite points.
*
* @param        pReadIndex <=> On input, index of first point of segment.
*                               On output, the first point after the segment data.
* @param        pSegment    <= extracted segment
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt  jmdlGraphicsPointArray_getDSegment3d
(
GraphicsPointArrayCP pInstance,
                int         *pReadIndex,
                DSegment3d  *pSegment
)
    {
    int index = *pReadIndex;
    DSegment4d hSegment;
    if (   jmdlGraphicsPointArray_getDSegment4d (pInstance, &index, &hSegment)
        && bsiDPoint4d_normalize (&hSegment.point[0], &pSegment->point[0])
        && bsiDPoint4d_normalize (&hSegment.point[1], &pSegment->point[1]))
        {
        *pReadIndex = index;
        return SUCCESS;
        }
    return ERROR;
    }


/*---------------------------------------------------------------------------------**//**
* Extract a DEllipse3d from the array.  If possible reduce to ellipse.
* (Note: Use ellipse.GetScaledRotmatrix () to convert to conventional form with
*    an orthogonal matrix and axis lengths.  Try to keep in as is though -- the DEllipse3d
*    and DConic4d are much better for further computing.)
*
* @param        pReadIndex <=> On input, index of first point of ellipse data.
*                               On output, the first point after the ellipse data.
* @param        pConic      <= conic form of data.  May be null.  This is the original
*                                   parameterization -- angles may be different from pEllipse.
* @param        pEllipse    <= ellipse form of data.    May be null.
* @param        pTheta0     <= start angle
* @param        pSweep      <= end angle
* @param        pStartPoint <= start point
* @param        pEndPoint   <= end point
* @return       true if a conic was found.  Note that this can be true but pIsEllipse is false,
*                   i.e. the conic could not be reduced to an ellipse.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       jmdlGraphicsPointArray_getDEllipse3d
(
GraphicsPointArrayCP pInstance,
                int         *pReadIndex,
                DConic4d    *pConic,
                DEllipse3d  *pEllipse,
                bool        *pIsEllipse,
                DPoint4d    *pStartPoint,
                DPoint4d    *pEndPoint
)
    {
    DConic4d conic;
    bool    funcStat = jmdlGraphicsPointArray_getDConic4d (pInstance, pReadIndex, &conic,
                                NULL, NULL, pStartPoint, pEndPoint);
    if (funcStat)
        {
        if (pConic)
            *pConic = conic;

        if (pEllipse || pIsEllipse)
            {
            DEllipse3d ellipse;
            bool    isEllipse = bsiDEllipse3d_initFromDConic4d (&ellipse, &conic);

            if (pIsEllipse)
                *pIsEllipse = isEllipse;

            if (pEllipse)
                *pEllipse = ellipse;
            }
        }
    return funcStat;
    }


/*---------------------------------------------------------------------------------**//**
* Extract a single bezier segemtn from its packed GraphicsPointArray form.
*
* @param        pReadIndex <=> On input, index of first point of bezier
*                               On output, index of next readable thing.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       jmdlGraphicsPointArray_getBezier
(
GraphicsPointArrayCP pInstance,
        int         *pReadIndex,
        DPoint4d    *pPoleArray,
        int         *pNumPole,
        int         maxPole
)
    {
    int     index = *pReadIndex;
    int     numPole = 0;
        int     currMask;
    int     typeMask = HPOINT_MASK_CURVETYPE_BITS | HPOINT_MASK_POINTTYPE_BITS;
    int     endPointType = HPOINT_MASK_CURVETYPE_BEZIER | HPOINT_MASK_BEZIER_STARTEND;
    int     interiorType = HPOINT_MASK_CURVETYPE_BEZIER | HPOINT_MASK_BEZIER_POLE;

    *pNumPole = 0;
    if (maxPole < 2)
            return false;

    for (;
            numPole < maxPole
         && jmdlGraphicsPointArray_getDPoint4dWithMask
                            (
                            pInstance,
                            &pPoleArray[numPole++],
                            &currMask,
                            index++
                            )
        ; )
        {
        if (numPole == 1)
            {
            if ((currMask & typeMask) != endPointType)
                    return false;
            }
        else
            {
            if ((currMask & typeMask) == endPointType)
                {
                *pReadIndex = index;
                *pNumPole = numPole;
                return true;
                }

            if ((currMask & typeMask) != interiorType)
                {
                return false;
                }
            }
        }
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* Extract a single bezier segemtn from its packed GraphicsPointArray form.
* Also compute its range.
*
* @param        pReadIndex <=> On input, index of first point of bezier
*                               On output, index of next readable thing.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       jmdlGraphicsPointArray_getBezierAndDRange3d
(
GraphicsPointArrayCP pInstance,
        int         *pReadIndex,
        DPoint4d    *pPoleArray,
        int         *pNumPole,
        DRange3dP   pRange,
        int         maxPole
)
    {
    bool    bStat = jmdlGraphicsPointArray_getBezier (pInstance, pReadIndex, pPoleArray, pNumPole, maxPole);
    if (NULL != pRange)
        {
        if (!bStat)
            bsiDRange3d_init (pRange);
        else
            bsiBezierDPoint4d_getDRange3d (pRange, pPoleArray, *pNumPole);
        }
    return bStat;
    }


/*---------------------------------------------------------------------------------**//**
* Extract a single bezier segemnt.  Returns data in both DPoint4d and
* split array forms.
*
* @param        pReadIndex <=> On input, index of first point of bezier
*                               On output, index of next readable thing.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       jmdlGraphicsPointArray_getBezierDPoint3dWeight
(
GraphicsPointArrayCP pInstance,
        int         *pReadIndex,
        DPoint4d    *pPoleArray,
        DPoint3d    *pXYZArray,
        double      *pWeightArray,
        int         *pNumPole,
        int         maxPole
)
    {
    int     index = *pReadIndex;
    DPoint4d pole;
    int     numPole = 0;
        int     currMask;
    //int     typeMask = HPOINT_MASK_CURVETYPE_BITS | HPOINT_MASK_POINTTYPE_BITS;
    int     endPointType = HPOINT_MASK_CURVETYPE_BEZIER | HPOINT_MASK_BEZIER_STARTEND;
    int     interiorType = HPOINT_MASK_CURVETYPE_BEZIER | HPOINT_MASK_BEZIER_POLE;

    *pNumPole = 0;
    if (maxPole < 2)
            return false;

    for (;
            numPole < maxPole
         && jmdlGraphicsPointArray_getDPoint4dWithMask
                            (
                            pInstance,
                            &pole,
                            &currMask,
                            index++
                            )
        ; )
        {
        if (pPoleArray)
            pPoleArray[numPole] = pole;
        if (pXYZArray)
            memcpy (pXYZArray + numPole, &pole, sizeof (DPoint3d));
        if (pWeightArray)
            pWeightArray[numPole] = pole.w;
        numPole++;

        if (numPole == 1)
            {
            if ((currMask & endPointType) != endPointType)
                    return false;
            }
        else
            {
            if ((currMask & endPointType) == endPointType)
                {
                *pReadIndex = index;
                *pNumPole = numPole;
                return true;
                }

            if ((currMask & interiorType) == endPointType)
                {
                return false;
                }
            }
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* Pop the final point from the array.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_pop
(
GraphicsPointArrayP pInstance,
GraphicsPoint           *pOut
)
    {
    return omdlVArray_pop (&pInstance->vbArray_hdr, pOut);
    }


/*---------------------------------------------------------------------------------**//**
* Search forward from i0 for the corresponding end-of-fragment point.
* @param pI1 <= last index of fragment.
* @param pPoint0 <= fragment start point. May be null.
* @param pPoint1 <= fragment end point. May be null.
* @param pCurveType <= fragment curve type.  May be null.
* @param i0 => first index of fragment
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_parseFragment
(
GraphicsPointArrayCP pInstance,
      int       *pI1,
      DPoint4d  *pPoint0,
      DPoint4d  *pPoint1,
      int       *pCurveType,
      int       i0
)
    {

    int n = GET_COUNT (pInstance);
        GraphicsPoint *pElement = GET_CONST_ELEMENT_PTR (pInstance, 0);
    int i1;
    int curveType;

    if (pI1)
        *pI1 = i0 - 1;

    if (!pElement || i0 >= n)
            return false;

    curveType = HPOINT_GET_CURVETYPE_BITS (pElement[i0].mask);
    if (pCurveType)
        *pCurveType = curveType;
    if (pPoint0)
        *pPoint0 = pElement[i0].point;

    if (curveType == 0)
        {
        for (i1 = i0 + 1;   i1 < n
              && 0 == (curveType = HPOINT_GET_CURVETYPE_BITS (pElement[i1].mask))
              ;i1++)
            {
            if (pElement[i1].mask & HPOINT_MASK_BREAK)
                {
                if (pPoint1)
                    *pPoint1 = pElement[i1].point;
                if (pI1)
                    *pI1 = i1;
                return true;
                }
            }
        if (pPoint1)
            *pPoint1 = pElement[i1-1].point;
        if (pI1)
            *pI1 = i1 - 1;
        return true;
        }
    else if (curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
        {
        i1 = i0 + 4;
        if (pI1)
            *pI1 = i1;
        if (pPoint1)
            *pPoint1 = pElement[i1].point;
        return true;
        }
    else if (curveType == HPOINT_MASK_CURVETYPE_BEZIER) // BSPLINE_CODED
        {
            /* March to the end of the bezier ... */
        for (i1 = i0 + 1;
                i1 < n
             && IS_BEZIER_POLE_MASK (pElement[i1].mask);
             i1++)
            {
            }

        if (i1 < n && IS_BEZIER_ENDPOINT_MASK (pElement[i1].mask))
            {
            if (pPoint1)
                *pPoint1 = pElement[i1].point;
            if (pI1)
                *pI1 = i1;
            return true;
            }
        return false;
        }

    else if (curveType == HPOINT_MASK_CURVETYPE_BSPLINE)
        {
        i1 = i0 + (int)pElement[i0].index;
        size_t j0 = (size_t)i0;
        size_t j1;
        if (!pInstance->IsBsplineCurve (j0, j1, true))
            return false;
        if (pPoint1)
            *pPoint1 = pElement[i1].point;
        if (pI1)
            *pI1 = i1;
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* Search backward from the end of the array for the limits of a primitive fragment.
* A primitive fragment is a single line segment, a complete ellipse sector, or a bezier.
* @param pI0 <= first index of fragment.
* @param pI1 <= last index of fragment.
* @param pPoint0 <= fragment start point. May be null.
* @param pPoint1 <= fragment end point. May be null.
* @param pCurveType <= fragment curve type.  May be null.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_parseFinalPrimitiveFragment
(
GraphicsPointArrayCP pInstance,
      int       *pI0,
      int       *pI1,
      DPoint4d  *pPoint0,
      DPoint4d  *pPoint1,
      int       *pCurveType
)
    {
    return jmdlGraphicsPointArray_parsePrimitiveBefore
                (
                pInstance,
                pI0,
                pI1,
                pPoint0,
                pPoint1,
                pCurveType,
                jmdlGraphicsPointArray_getCount (pInstance));
    }

/* METHOD(GraphicsPointArray,none,primitiveFractionToApplicationParameter)
/*---------------------------------------------------------------------------------**//**
* Find the fragment at specified index.  Interplate between the parameter values
*   stored the start and end of the fragment.  NOTE THAT SEMANTICS OF THE STORED PARAMETERS
*   ARE APPLICATION SPECIFIC.   CALLER IS REPSONSIBLE FOR KNOWING (ENSURING) THAT
*   MEANINGFUL VALUES ARE PRESENT IN THE PARAMETER FIELD.
* @param iStart => fragment start index.
* @param fraction => fraction parameter within the fragment.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_primitiveFractionToApplicationParameter
(
GraphicsPointArrayCP pInstance,
      double *pParameter,
      int iStart,
      double fraction
)
    {
    int i0, i1;
    GraphicsPoint gp0, gp1;

    if (pParameter)
        *pParameter = 0.0;

    if (   jmdlGraphicsPointArray_parsePrimitiveAt (pInstance, &i0, &i1,
                    NULL, NULL, NULL, iStart)
        && jmdlGraphicsPointArray_getGraphicsPoint (pInstance, &gp0, i0)
        && jmdlGraphicsPointArray_getGraphicsPoint (pInstance, &gp1, i1)
        )
        {
        if (pParameter)
            *pParameter = gp0.a + fraction * (gp1.a - gp0.a);

        return true;
        }
    return false;
    }

/* METHOD(GraphicsPointArray,none,primitiveFractionToApplicationParameter)
/*---------------------------------------------------------------------------------**//**
Find the fragment at specified index.  Return the parameter values stored at the
    start and end of the interval.
    NOTE THAT SEMANTICS OF THE STORED PARAMETERS
   ARE APPLICATION SPECIFIC.   CALLER IS REPSONSIBLE FOR KNOWING (ENSURING) THAT
   MEANINGFUL VALUES ARE PRESENT IN THE PARAMETER FIELD.
@param pParameter0 OUT start parameter
@param pParameter1 OUT end parameter
@param iStart IN index at start of primitive.
@bsimethod                                                    EarlinLutz      06/07
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_primitiveIndexToApplicationParameterInterval
(
GraphicsPointArrayCP pInstance,
      double *pParameter0,
      double *pParameter1,
      int iStart
)
    {
    int i0, i1;
    GraphicsPoint gp0, gp1;

    if (pParameter0)
        *pParameter0 = 0.0;
    if (pParameter1)
        *pParameter1 = 0.0;

    if (   jmdlGraphicsPointArray_parsePrimitiveAt (pInstance, &i0, &i1,
                    NULL, NULL, NULL, iStart)
        && jmdlGraphicsPointArray_getGraphicsPoint (pInstance, &gp0, i0)
        && jmdlGraphicsPointArray_getGraphicsPoint (pInstance, &gp1, i1)
        )
        {
        if (pParameter0)
            *pParameter0 = gp0.a;
        if (pParameter1)
            *pParameter1 = gp1.a;
        return true;
        }
    return false;
    }




/*---------------------------------------------------------------------------------**//**
* Search forward for the index range of primitive starting at iStart.
* A primitive is a single line segment, a complete ellipse sector, or a bezier.
* @param pI0 <= first index of primitive.
* @param pI1 <= last index of primitive.
* @param pCurveType <= fragment curve type.  May be null.
* @param iStart => start index of fragment. Any out of bounds index is invalid.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_parsePrimitiveAt
(
GraphicsPointArrayCP pInstance,
      int       *pI0,
      int       *pI1,
      DPoint4d  *pPoint0,
      DPoint4d  *pPoint1,
      int       *pCurveType,
      int       iStart
)
    {
    int n = GET_COUNT (pInstance);
    int i1;
    int i0;
    int curveType = 0;
    bool    boolstat = false;
    GraphicsPoint *pElement = GET_CONST_ELEMENT_PTR (pInstance, 0);

    if (pI0)
        *pI0 = -1;
    if (pI1)
        *pI1 = -1;

    if (!pElement || iStart < 0 || iStart >= n)
            return false;

    /* First try simple connect from prior tail */
    i0 = iStart;
    i1 = iStart + 1;
    curveType = HPOINT_GET_CURVETYPE_BITS (pElement[i0].mask);

    if (   i0 >= 0
        && i1 < n
        && HPOINT_GET_CURVETYPE_BITS(pElement[i0].mask) == 0
        && HPOINT_GET_CURVETYPE_BITS(pElement[i1].mask) == 0
        && !(pElement[i0].mask & HPOINT_MASK_BREAK)
       )
        {
        boolstat = true;
        }
    else if (    curveType == HPOINT_MASK_CURVETYPE_ELLIPSE
            && (i1 = i0 + 4) < n
            )
        {
        boolstat = true;
        }
    else if (curveType == HPOINT_MASK_CURVETYPE_BEZIER) // BSPLINE_CODED
        {
        /* March to the beginning of the bezier ... */
        for (i1 = i0 + 1;
                i1 < n
             && IS_BEZIER_POLE_MASK (pElement[i1].mask);
             i1++)
            {
            }

        if (i1 >= n || !IS_BEZIER_ENDPOINT_MASK (pElement[i1].mask))
            i1--;
        boolstat = true;
        }
    else if (curveType == HPOINT_MASK_CURVETYPE_BSPLINE)
        {
        i1 = i0 + (int)pElement[i0].index;
        size_t j0 = (size_t)i0;
        size_t j1;
        if (pInstance->IsBsplineCurve (j0, j1, true))
            boolstat = true;
        }

    if (!boolstat || i1 >= n)
        {
        i0 = i1 = -1;
        boolstat = false;
        curveType = 0;
        }

    if (pI0)
        *pI0 = i0;

    if (pI1)
        *pI1 = i1;

    if (pCurveType)
        *pCurveType = curveType;

    if (pPoint0 && boolstat)
        *pPoint0 = pElement[i0].point;

    if (pPoint1 && boolstat)
        *pPoint1 = pElement[i1].point;

    return boolstat;
    }


/*---------------------------------------------------------------------------------**//**
* Search backward from a given index for the start of a primitive.
* A primitive is a single line segment, a complete ellipse sector, or a bezier.
* @param pI0 <= first index of primitive.
* @param pI1 <= last index of primitive.
* @param pCurveType <= fragment curve type.  May be null.
* @param iNext => first index of successor primitive.
*           Negative is NOT interpretted as reference to end of array.
*           Enter the array count to start at end.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_parsePrimitiveBefore
(
GraphicsPointArrayCP pInstance,
      int       *pI0,
      int       *pI1,
      DPoint4d  *pPoint0,
      DPoint4d  *pPoint1,
      int       *pCurveType,
      int       iNext
)
    {
    int n = GET_COUNT (pInstance);
    GraphicsPoint *pElement = GET_CONST_ELEMENT_PTR (pInstance, 0);
    int i1;
    int i0;
    int curveType = 0;
    bool    boolstat = false;

    if (iNext > n)
        iNext = n;
    if (pI0)
        *pI0 = -1;
    if (pI1)
        *pI1 = -1;

    if (!pElement || iNext <= 0)
            return false;

    /* First try simple connect from prior tail */
    i0 = iNext - 1;
    i1 = iNext;
    if (   i0 >= 0
        && i1 < n
        && !(pElement[i0].mask & HPOINT_MASK_BREAK)
        && HPOINT_GET_CURVETYPE_BITS(pElement[i0].mask) == 0
        && HPOINT_GET_CURVETYPE_BITS(pElement[i1].mask) == 0
        )
        {
        }
    else
        {
        i1 = iNext - 1;
        curveType = HPOINT_GET_CURVETYPE_BITS (pElement[i1].mask);
        if (pCurveType)
            *pCurveType = curveType;

        if (curveType == 0)
            {
            i0 = i1 - 1;
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
            {
            i0 = i1 - 4;
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_BEZIER) // BSPLINE_CODED
            {
                /* March to the beginning of the bezier ... */
            for (i0 = i1 - 1;
                    i0 >= 0
                 && IS_BEZIER_POLE_MASK (pElement[i0].mask);
                 i0--)
                {
                }

            if (i0 < 0 || !IS_BEZIER_ENDPOINT_MASK (pElement[i0].mask))
                i0++;
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_BSPLINE)
            {
            size_t j0;
            size_t j1 = (size_t) i1;
            if (pInstance->IsBsplineCurveTail (j1, j0, true))
                {
                i0 = (int)j0;
                }
            else
                {
                i0 = -1;
                }
            }
        }

    if (i0 < 0)
        {
        i0 = i1 = -1;
        boolstat = false;
        }
    else
        boolstat = true;

    if (pI0)
        *pI0 = i0;

    if (pI1)
        *pI1 = i1;

    if (pCurveType)
        *pCurveType = curveType;

    if (pPoint0 && boolstat)
        *pPoint0 = pElement[i0].point;

    if (pPoint1 && boolstat)
        *pPoint1 = pElement[i1].point;

    return boolstat;
    }


/*---------------------------------------------------------------------------------**//**
* Search backward from a given index for the start of a primitive.
* A primitive is a single line segment, a complete ellipse sector, or a bezier.
* @param pI0 <= first index of primitive.
* @param pI1 <= last index of primitive.
* @param pCurveType <= fragment curve type.  May be null.
* @param iPrev => Final index of prior fragment.  -1 indicates parse first primitive.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_parsePrimitiveAfter
(
GraphicsPointArrayCP pInstance,
      int       *pI0,
      int       *pI1,
      DPoint4d  *pPoint0,
      DPoint4d  *pPoint1,
      int       *pCurveType,
      int       iPrev
)
    {
    int n = GET_COUNT (pInstance);
    GraphicsPoint *pElement = GET_CONST_ELEMENT_PTR (pInstance, 0);
    int i1;
    int i0;
    int curveType = 0;
    bool    boolstat = false;

    if (iPrev < 0)
        iPrev = -1;
    if (pI0)
        *pI0 = -1;
    if (pI1)
        *pI1 = -1;

    if (!pElement || iPrev >= n - 1)
            return false;

    /* First try simple connect from prior tail */
    i0 = iPrev;
    i1 = iPrev + 1;
    if (   i0 >= 0
        && i1 < n
        && !(pElement[i0].mask & HPOINT_MASK_BREAK)
        && HPOINT_GET_CURVETYPE_BITS(pElement[i0].mask) == 0
        && HPOINT_GET_CURVETYPE_BITS(pElement[i1].mask) == 0
       )
        {
        curveType = 0;
        }
    else
        {
        /* Interpret iPrev+1 as the start of a fragment. */
        i0 = iPrev + 1;

        curveType = HPOINT_GET_CURVETYPE_BITS (pElement[i0].mask);

        if (curveType == 0)
            {
            i1 = i0 + 1;
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
            {
            i1 = i0 + 4;
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_BEZIER) // BSPLINE_CODED
            {
            /* March to the beginning of the bezier ... */
            for (i1 = i0 + 1;
                    i1 < n
                 && IS_BEZIER_POLE_MASK (pElement[i1].mask);
                 i1++)
                {
                }

            if (i1 >= n || !IS_BEZIER_ENDPOINT_MASK (pElement[i1].mask))
                i1--;
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_BSPLINE)
            {
            i1 = i0 + (int)pElement[i0].index;
            size_t j0 = (size_t)i0;
            size_t j1;
            if (pInstance->IsBsplineCurve (j0, j1, true))
                boolstat = true;
            }
        }

    if (i1 >= n)
        {
        i0 = i1 = n;
        boolstat = false;
        }
    else
        boolstat = true;

    if (pI0)
        *pI0 = i0;

    if (pI1)
        *pI1 = i1;

    if (pCurveType)
        *pCurveType = curveType;

    if (pPoint0 && boolstat)
        *pPoint0 = pElement[i0].point;

    if (pPoint1 && boolstat)
        *pPoint1 = pElement[i1].point;

    return boolstat;
    }


/*---------------------------------------------------------------------------------**//**
* Search forward for the index range of a linestring starting at iStart.
* Return the number of points (including the start) in the linestring.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int  jmdlGraphicsPointArray_countToEndOfLinestring
(
GraphicsPointArrayCP pInstance,
      int       iStart
)
    {
    int n = GET_COUNT (pInstance);
    int i = iStart;
    int numPoint = 0;
    GraphicsPoint *pElement = GET_CONST_ELEMENT_PTR (pInstance, 0);

    while (  i < n
          && HPOINT_GET_CURVETYPE_BITS(pElement[i].mask) == 0
          )
        {
        numPoint++;
        if (pElement[i].mask & HPOINT_MASK_BREAK)
            break;
        i++;

        }
    return numPoint;
    }



/*---------------------------------------------------------------------------------**//**
* Search backward from the end of the array for the limits of a fragment.
* A fragment is a polyline, a complete ellipse sector, or a bezier.
* @param pI0 <= first index of fragment.
* @param pI1 <= last index of fragment.
* @param pPoint0 <= fragment start point. May be null.
* @param pPoint1 <= fragment end point. May be null.
* @param pCurveType <= fragment curve type.  May be null.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_parseFinalFragment
(
GraphicsPointArrayCP pInstance,
      int       *pI0,
      int       *pI1,
      DPoint4d  *pPoint0,
      DPoint4d  *pPoint1,
      int       *pCurveType
)
    {
    int n = GET_COUNT (pInstance);
    GraphicsPoint *pElement = GET_CONST_ELEMENT_PTR (pInstance, 0);
    int i1 = -1;
    int i0 = -1;
    int curveType;

    if (pI0)
        *pI0 = -1;
    if (pI1)
        *pI1 = -1;

    if (!pElement || n <= 0)
            return false;

    i1 = n - 1;
    curveType = HPOINT_GET_CURVETYPE_BITS (pElement[i1].mask);
    if (pCurveType)
        *pCurveType = curveType;
    if (pPoint1)
        *pPoint1 = pElement[i1].point;

    if (curveType == 0)
        {
        int k;
        i0 = i1;
        for (k = i1 - 1;k >=0;k--)
            {
            curveType = HPOINT_GET_CURVETYPE_BITS (pElement[i1].mask);
            if  (  curveType != 0
                || pElement[k].mask & HPOINT_MASK_BREAK
                || pElement[k].mask & HPOINT_MASK_POINT
                )
                {
                break;
                }
            i0 = k;
            }
        }
    else if (curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
        {
        i0 = i1 - 4;
        }
    else if (curveType == HPOINT_MASK_CURVETYPE_BEZIER) // BSPLINE_CODED
        {
            /* March to the beginning of the bezier ... */
        for (i0 = i1 - 1;
                i0 >= 0
             && IS_BEZIER_POLE_MASK (pElement[i1].mask);
             i0--)
            {
            }

        if (i0 < 0 || !IS_BEZIER_ENDPOINT_MASK (pElement[i0].mask))
            i0++;
        }
    else if (curveType == HPOINT_MASK_CURVETYPE_BSPLINE)
        {
        size_t j0;
        size_t j1 = (size_t) i1;
        if (pInstance->IsBsplineCurveTail (j1, j0, false))
            {
            i0 = (int)j0;
            }
        else
            {
            i0 = -1;
            }
        }
    
    if (i0 < 0)
        {
        return false;
        }
    if (pI0)
        *pI0 = i0;

    if (pI1)
        *pI1 = i1;

    if (pPoint0)
        *pPoint0 = pElement[i0].point;

    if (pPoint1)
        *pPoint1 = pElement[i1].point;

    return true;
    }


/*---------------------------------------------------------------------------------**//**
* Interpolate along a primitive.
* The primitive is identified by an index in the array.  The parameter is
* taken as a fraction of the local parameterization of a line segment, ellipse, or
* bezier.   The index should be to the start of the primitive data.
* @param pPoint <= evaluated point.
* @param i0 <= start point for the primitive.
* @param s <= fractional parameter along the primitive.
* @return true if the index is a valid primitive
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_primitiveFractionToDPoint4d
(
GraphicsPointArrayCP pInstance,
      DPoint4d                  *pPoint,
      int                       i0,
      double                    s
)
    {
    return jmdlGraphicsPointArray_primitiveFractionToDPoint4dTangent
                (pInstance, pPoint, NULL, i0, s);
    }


/*---------------------------------------------------------------------------------**//**
* Interpolate along a primitive.
* The primitive is identified by an index in the array.  The parameter is
* taken as a fraction of the local parameterization of a line segment, ellipse, or
* bezier.   The index should be to the start of the primitive data.
* @param pXArray <= evaluated points.
* @param pdXArray <= evaluated tangents.
* @param pddXArray <= evaluated second derivatives.
* @param i0 => start point for the primitive.
* @param *pFractionArray => array of fractional positions.
& @param numFraction => number of fractions.
* @return true if the index is a valid primitive
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_primitiveFractionArrayToDPoint4dDerivatives
(
GraphicsPointArrayCP pInstance,
      DPoint4d                  *pXArray,
      DPoint4d                  *pdXArray,
      DPoint4d                  *pddXArray,
      int                       i0,
      double                    *pFractionArray,
      int                       numFraction
)
    {
    return pInstance->PrimitiveFractionArrayToDPoint4dDerivatives ((size_t)i0, pFractionArray, (size_t)numFraction,
                            pXArray, pdXArray, pddXArray)
            ? true : false;
    }


/*---------------------------------------------------------------------------------**//**
* Interpolate along a primitive.
* The primitive is identified by an index in the array.  The parameter is
* taken as a fraction of the local parameterization of a line segment, ellipse, or
* bezier.   The index should be to the start of the primitive data.
* @param pXArray <= evaluated points.
* @param pdXArray <= evaluated tangents.
* @param pddXArray <= evaluated second derivatives.
* @param i0 => start point for the primitive.
* @param *pFractionArray => array of fractional positions.
& @param numFraction => number of fractions.
* @return true if the index is a valid primitive
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_primitiveFractionArrayToDPoint3dDerivatives
(
GraphicsPointArrayCP pInstance,
      DPoint3d                  *pXArray,
      DPoint3d                  *pdXArray,
      DPoint3d                  *pddXArray,
      int                       i0,
      double                    *pFractionArray,
      int                       numFraction
)
    {
    return pInstance->PrimitiveFractionArrayToDPoint3dDerivatives ((size_t)i0, pFractionArray, (size_t)numFraction,
                            pXArray, (DVec3dP)pdXArray, (DVec3dP)pddXArray)
            ? true : false;
    }


/*---------------------------------------------------------------------------------**//**
* Interpolate along a primitive.
* The primitive is identified by an index in the array.  The parameter is
* taken as a fraction of the local parameterization of a line segment, ellipse, or
* bezier.   The index should be to the start of the primitive data.
* @param pPoint <= evaluated point.
* @param pTangent <= evaluated tangent.
* @param i0 => start point for the primitive.
* @param s => fractional parameter along the primitive.
* @return true if the index is a valid primitive
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_primitiveFractionToDPoint4dTangent
(
GraphicsPointArrayCP pInstance,
      DPoint4d                  *pPoint,
      DPoint4d                  *pTangent,
      int                       i0,
      double                    s
)
    {
    return jmdlGraphicsPointArray_primitiveFractionArrayToDPoint4dDerivatives
                    (
                    pInstance,
                    pPoint,
                    pTangent,
                    NULL,
                    i0,
                    &s,
                    1
                    );
    }


/*---------------------------------------------------------------------------------**//**
* Interpolate along a primitive.
* The primitive is identified by an index in the array.  The parameter is
* taken as a fraction of the local parameterization of a line segment, ellipse, or
* bezier.   The index should be to the start of the primitive data.
* @param pPoint <= evaluated point.
* @param pTangent <= evaluated tangent.
* @param i0 => start point for the primitive.
* @param s => fractional parameter along the primitive.
* @return true if the index is a valid primitive
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_primitiveFractionToDPoint3dTangent
(
GraphicsPointArrayCP pInstance,
      DPoint3d                  *pPoint,
      DPoint3d                  *pTangent,
      int                       i0,
      double                    s
)
    {
    return jmdlGraphicsPointArray_primitiveFractionArrayToDPoint3dDerivatives
            (
            pInstance,
            pPoint,
            pTangent,
            NULL,
            i0,
            &s,
            1
            );
    }


/*---------------------------------------------------------------------------------**//**
* Interpolate along a primitive.
* The primitive is identified by an index in the array.  The parameter is
* taken as a fraction of the local parameterization of a line segment, ellipse, or
* bezier.   The index should be to the start of the primitive data.
* @param pPoint <= evaluated, normalized point.
* @param i0 <= start point for the primitive.
* @param s <= fractional parameter along the primitive.
* @return true if the index is a valid primitive and the point could be normalized.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_primitiveFractionToDPoint3d
(
GraphicsPointArrayCP pInstance,
      DPoint3d                  *pPoint,
      int                       i0,
      double                    s
)
    {
    DPoint4d hPoint;
    return jmdlGraphicsPointArray_primitiveFractionToDPoint4dTangent
                        (pInstance, &hPoint, NULL, i0, s)
        && bsiDPoint4d_normalize (&hPoint, pPoint);
    }


/*---------------------------------------------------------------------------------**//**
* @return the curve type bits from a point mask.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int    jmdlGraphicsPointArray_getCurveType
(
GraphicsPointArrayCP pInstance,
        int                         index
)
    {
    int curveType = 0;
    GraphicsPoint *pGP = GET_CONST_ELEMENT_PTR (pInstance, index);
    if (pGP)
        curveType =HPOINT_GET_CURVETYPE_BITS (pGP->mask);
    return curveType;
    }

#ifdef Compile_jmdlGraphicsPointArray_copyCurve
// Remark: I don't think this is used -- doesn't copy linestrings.
/*---------------------------------------------------------------------------------**//**
* Copy a single curve from one array to another.
* @param        pDest <=> desination header.
* @param        pSourceIndex <=> read index in source array.  Updated only if curve read successfully.
* @param        pSource => source header.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlGraphicsPointArray_copyCurve
(
GraphicsPointArrayP pDest,
        int                         *pSourceIndex,
GraphicsPointArrayCP pSource
)
    {
    /* This should be rewritten as PARSE + COPY */
    int i = *pSourceIndex;
    bool    funcStat = false;
    int curveType;
    DConic4d conic;
    DPoint4d bezierPole[MAX_BEZIER_ORDER];
    int numPole;

    GraphicsPoint *pSourcePoint = GET_CONST_ELEMENT_PTR (pSource, i);

    if (pSourcePoint)
        {
        curveType = HPOINT_GET_CURVETYPE_BITS (pSourcePoint[i].mask);
        if (curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
            {
            if (jmdlGraphicsPointArray_getDConic4d (pSource, &i, &conic, NULL, NULL, NULL, NULL))
                {
                jmdlGraphicsPointArray_addDConic4d (pDest, &conic);
                *pSourceIndex = i;
                funcStat = true;
                }
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_BEZIER) // Not compiled
            {
            if (jmdlGraphicsPointArray_getBezier (pSource, &i, bezierPole, &numPole, MAX_BEZIER_ORDER))
                {
                jmdlGraphicsPointArray_addDPoint4dBezier (pDest, bezierPole, numPole, 1, false);
                *pSourceIndex = i;
                funcStat = true;
                }

            }
        else if (curveType == HPOINT_MASK_CURVETYPE_BSPLINE)
            {
            // NEEDS_WORK evaluate bspline
            }
        }

    return funcStat;
    }
#endif
/*---------------------------------------------------------------------------------**//**
* Copy a single curve from one array to another.
* @param        pDest <=> desination header.
* @param        pSourceIndex <=> read index in source array.  Updated only if curve read successfully.
* @param        pSource => source header.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlGraphicsPointArray_copyPartialPrimitive
(
GraphicsPointArrayP pDest,

GraphicsPointArrayCP pSource,
        int                         startIndex,
        double                      param0,
        double                      param1
)
    {
    DSegment4d segment;
    DConic4d   conic;
    DPoint4d   poleArray[MAX_BEZIER_CURVE_ORDER];
    int         order;
    int readIndex;
    bool    copied = false;
    /* Only optimize exact case */
    bool    complete = param0 == 0.0 && param1 == 1.0;

    if (readIndex = startIndex,
        jmdlGraphicsPointArray_getDSegment4d (pSource, &readIndex, &segment))
        {
        if (complete)
            {
            jmdlGraphicsPointArray_addDSegment4d (pDest, &segment, true);
            }
        else
            {
            DSegment4d trimmedSegment;
            bsiDPoint4d_add2ScaledDPoint4d (&trimmedSegment.point[0],
                                        NULL,
                                        &segment.point[0], 1.0 - param0,
                                        &segment.point[1], param0
                                        );
            bsiDPoint4d_add2ScaledDPoint4d (&trimmedSegment.point[1],
                                        NULL,
                                        &segment.point[0], 1.0 - param1,
                                        &segment.point[1], param1
                                        );
            jmdlGraphicsPointArray_addDSegment4d (pDest, &trimmedSegment, true);
            }
        copied = true;
        }
    else if (readIndex = startIndex,
             jmdlGraphicsPointArray_getDConic4d (pSource, &readIndex, &conic,
                                                        NULL, NULL, NULL, NULL))
        {
        double dTheta = (param1 - param0) * conic.sweep;
        if (fabs (dTheta) < bsiTrig_smallAngle ())
            {
            }
        else
            {
            if (!complete)
                {
                conic.start = conic.start + param0 * conic.sweep;
                conic.sweep = (param1 - param0) * conic.sweep;
                }
            jmdlGraphicsPointArray_addDConic4d (pDest, &conic);
            copied = true;
            }
        }
    else if (readIndex = startIndex,
             jmdlGraphicsPointArray_getBezier (pSource, &readIndex, poleArray, &order, MAX_BEZIER_CURVE_ORDER))
        {
        GraphicsPoint gp0, gp1;
        double a0, a1;
        int oldCount = jmdlGraphicsPointArray_getCount (pDest);
        // Get spline parameters out of first, last points of the bezier.
        jmdlGraphicsPointArray_getGraphicsPoint (pSource, &gp0, startIndex);
        jmdlGraphicsPointArray_getGraphicsPoint (pSource, &gp1, startIndex + order - 1);
        a0 = gp0.a;
        a1 = gp1.a;
        if (!complete)
            {
            bsiBezierDPoint4d_subdivideToIntervalInPlace (poleArray, order, param0, param1);
            a0 = gp0.a + param0 * (gp1.a - gp0.a);
            a1 = gp0.a + param1 * (gp1.a - gp0.a);
            }
        jmdlGraphicsPointArray_addDPoint4dBezier (pDest, poleArray, order, 1, FALSE);
        
        jmdlGraphicsPointArray_getGraphicsPoint (pDest, &gp0, oldCount);
        jmdlGraphicsPointArray_getGraphicsPoint (pDest, &gp1, oldCount + order - 1);
        gp0.a = a0;
        gp1.a = a1;
        jmdlGraphicsPointArray_setGraphicsPoint (pDest, &gp0, oldCount);
        jmdlGraphicsPointArray_setGraphicsPoint (pDest, &gp1, oldCount + order - 1);
        copied = true;
        }
    else if (pSource->IsBsplineCurve (startIndex))
        {
        MSBsplineCurve fullCurve, partialCurve;
        size_t i0, i1;
        double knotA, knotB;
        if (pSource->ParseBsplineCurveKnotDomain (startIndex, i0, i1, knotA, knotB)
            && pSource->GetBsplineCurve (startIndex, fullCurve))
            {
            double knot0 = knotA + param0 * (knotB - knotA);
            double knot1 = knotA + param1 * (knotB - knotA);
            if (SUCCESS == partialCurve.CopySegment (fullCurve, knot0, knot1))
                {
                pDest->AddAsCompleteBsplineCurve (partialCurve);
                copied = true;
                partialCurve.ReleaseMem ();
                }
            fullCurve.ReleaseMem ();
            }
        }
    return copied;
    }



/*---------------------------------------------------------------------------------**//**
* Add a point array to the array, placing row breaks
*
* @param pPoint     => arry of points in grid.
* @param pWeight    => (optioinal)array of weights.
* @param n                  => total number of points. Not required to be a multiple of pointsPerRow.
* @param pointsPerRow   => number of points per row.
* @return true unless no memory.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlGraphicsPointArray_addDPoint3dGridByRow
(
GraphicsPointArrayP pInstance,
const DPoint3d  *pPoint,
const double    *pWeight,
int              n,
int              pointsPerRow
)
    {
    int i,pointsInRow=0;

    if (EXTEND_GPA_BY (pInstance, n))
        {
        pointsInRow = 0;
        if (!pWeight)
            {
            for (i = 0; i < n; i++)
                {
                if (!jmdlGraphicsPointArray_addDPoint3d (pInstance, &pPoint[i]))
                        return false;
                pointsInRow++;
                if (pointsInRow >= pointsPerRow)
                    {
                    jmdlGraphicsPointArray_markBreak (pInstance);
                    pointsInRow = 0;
                    }
                }
            }
        else
            {
            DPoint4d point;
            for (i = 0; i < n; i++)
                {
                    bsiDPoint4d_initFromDPoint3dAndWeight (&point, &pPoint[i], pWeight[i]);
                if (!jmdlGraphicsPointArray_addDPoint4d (pInstance, &point))
                        return false;
                pointsInRow++;
                if (pointsInRow > pointsPerRow)
                    {
                    jmdlGraphicsPointArray_markBreak (pInstance);
                    pointsInRow = 0;
                    }
                }
            }
        }

    jmdlGraphicsPointArray_markBreak (pInstance);
        return true;
    }


/*---------------------------------------------------------------------------------**//**
* Add linestrings from a grid, moving along columns.
*
* @param pPoint     => arry of points in grid.
* @param pWeight    => (optioinal)array of weights.
* @param n                  => total number of points. Not required to be a multiple of pointsPerRow.
* @param pointsPerRow   => number of points per row.
* @return true unless no memory.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlGraphicsPointArray_addDPoint3dGridByColumn
(
GraphicsPointArrayP pInstance,
const DPoint3d  *pPoint,
const double    *pWeight,
int              n,
int              pointsPerRow
)
    {
    int i, j, column;

    column = 0;
    if (EXTEND_GPA_BY (pInstance, n))
        {
        if (!pWeight)
            {
            for (i = j = 0; i < n; i++, j+= pointsPerRow)
                {
                if (j >= n)
                    {
                    j = ++column;
                    jmdlGraphicsPointArray_markBreak (pInstance);
                    }
                if (!jmdlGraphicsPointArray_addDPoint3d (pInstance, &pPoint[j]))
                    return false;
                }
            }
        else
            {
            DPoint4d point;
            for (i = j = 0; i < n; i++, j+= pointsPerRow)
                {
                if (j >= n)
                    {
                    j = ++column;
                    jmdlGraphicsPointArray_markBreak (pInstance);
                    }

                bsiDPoint4d_initFromDPoint3dAndWeight (&point, &pPoint[j], 1.0);
                if (!jmdlGraphicsPointArray_addDPoint4d (pInstance, &point))
                        return false;
                }
            }
        }

    jmdlGraphicsPointArray_markBreak (pInstance);
    return true;
    }


/*---------------------------------------------------------------------------------**//**
*
* Get a normalized point out of the array.  If the stored point has non-zero weight,
*   divide through and return the weight.  If the stored point has zero weight,
*   leave the (vector) coordinates alone and (optionially) return the weight.
*
* @param pPoint  <= normalized point.
* @param pWeight <= pre-normalization weight.
* @param index   <= index of accessed point.
* @return true if the index is valid.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_getNormalizedDPoint3d
(
GraphicsPointArrayCP pInstance,
DPoint3d        *pPoint,
double          *pWeight,
int             index
)
    {
    GraphicsPoint *pGP = GET_CONST_ELEMENT_PTR (pInstance, index);
    DPoint3d realPoint;

    if (pGP)
        {
        if (bsiDPoint4d_normalize (&pGP->point, &realPoint))
            {
            if (pWeight)
                *pWeight = pGP->point.w;
            if (pPoint)
                *pPoint = realPoint;
            }
        else
            {
            if (pWeight)
                *pWeight = 0.0;
            if (pPoint)
                bsiDPoint4d_cartesianFromHomogeneous (&pGP->point, pPoint);
            }

        return true;
        }
    else
        {
        return false;
        }
    }


/*---------------------------------------------------------------------------------**//**
*
* Get normalized points out of the array.
* w=0 points are set to zero
*
* @param    pInstance => source of points
* @param pPoint <= buffer of returned points
* @param nGot <= number of points returned
* @param i0 => first index to retrieve from GraphicsPointArray.
* @param nreq => number of points requested
* @return true if all copied points had nonzero weights.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_getDPoint3dArray
(
GraphicsPointArrayCP pInstance,
DPoint3d        *pPoint,
int             *pNGot,
int              i0,
int              nreq
)
    {
    bool        allWeightsNonZero = true;
    DPoint3d    *pDest;
    const GraphicsPoint *pSource;
        int nPoint = GET_COUNT (pInstance);
    double      w, dw;
    *pNGot = 0;

    if (i0 < 0)
        {
        i0 = 0;
        }

    pSource = GET_CONST_ELEMENT_PTR (pInstance, i0);
    if  (pSource)
        {
        pDest = pPoint;
        if ( i0 + nreq > nPoint )
                nreq = nPoint - i0;
        *pNGot = nreq;

        for ( ; nreq > 0 ;  nreq--, pDest++, pSource++)
            {
            if ((w = pSource->point.w) != 0.0)
                {
                                dw = 1.0 / w;
                pDest->x = pSource->point.x * dw;
                pDest->y = pSource->point.y * dw;
                pDest->z = pSource->point.z * dw;
                }
            else
                {
                pDest->x = pDest->y = pDest->z = 0.0;
                allWeightsNonZero = false;
                }
            }
        }

    return  allWeightsNonZero;
    }

/* Woops!! don't export this -- already supported via StructArray_hdr, but
    as a StatusInt
*/

/*---------------------------------------------------------------------------------**//**
*
* Copy a graphics point structure out of the array.
*
* @param    pInstance => source of points
* @param pPoint <= graphics point copied from array
* @param index  => index of point to copy.  -1 for last element.
* @return true if point was returned
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_getGraphicsPoint
(
GraphicsPointArrayCP pInstance,
      GraphicsPoint         *pPoint,
      int                   index
)
    {
    return GET_ELEMENT (pInstance, pPoint, index);
    }


/*---------------------------------------------------------------------------------**//**
*
* Get a pointer to an array element.   Pointers can become invalid if points are added to the array.
*
* @param index => index of array element to return. -1 for last element.
* @return pointer to array element.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP GraphicsPoint    *jmdlGraphicsPointArray_getPtr
(
GraphicsPointArrayP pInstance,
int                         index
)
    {
    return GET_ELEMENT_PTR (pInstance, index);
    }


/*---------------------------------------------------------------------------------**//**
* Get the fields of the desired array element.  Any output parameter may be null.
*
* @param    pPoint      <= the homogeneous point
* @param    pA          <= the floating point label value
* @param    pMask       <= the mask value
* @param    pUserData   <= the user data label
* @param    index       => index of array element to return. -1 for last element.
* @return true if index is valid
* @bsihdr                                       DavidAssaf      02/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_getComplete
(
GraphicsPointArrayP pInstance,
DPoint4d                    *pPoint,
double                      *pA,
int                         *pMask,
int                         *pUserData,
int                         index
)
    {
    if (pInstance)
        {
        GraphicsPoint *pGP = GET_ELEMENT_PTR (pInstance, index);

        if (pGP)
            {
            if (pPoint)
                *pPoint = pGP->point;

            if (pA)
                *pA = pGP->a;

            if (pMask)
                *pMask = pGP->mask;

            if (pUserData)
                *pUserData = pGP->userData;

            return true;
            }
        }
    return false;
    }
#ifdef INT_POINT2D_DEFINED

/*---------------------------------------------------------------------------------**//**
*
* Get normalized points out of the array, starting at index i0 and
* continuing until a point with a marker (break mask) is reached.
* w=0 points are set to zero
*
* @param pPoint <= filled point buffer
* @param nGotP <= number of points copied to buffer
* @param pStartIndex <=> start index.   Initialize to 0 before
*                                   first call; this function increments
*                                   it as points are removed.
* @param maxGet => max points to get
* @return true if all points had nonzero weight
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_getPoint2dLoop
(
GraphicsPointArrayCP pInstance,
      Point2d               *pPoint,
      int                   *pNGot,
      int                   *pStartIndex,
      int                   maxGet
)
    {
    bool        allWeightsNonZero = true;
    Point2d     *pDest;
    const GraphicsPoint *pSource;
        int nPoint = GET_COUNT (pInstance);
    double      w, dw;
        int nGot = 0;
        int i0;

    i0 = *pStartIndex;
    if (i0 < 0)
        {
        i0 = 0;
        }

    pSource = GET_CONST_ELEMENT_PTR (pInstance, i0);
    if  (pSource)
        {
        pDest = pPoint;
        if (i0 + maxGet > nPoint)
            maxGet = nPoint - i0;

        for ( ; nGot < maxGet; pSource++, pDest++)
            {
            if ((w = pSource->point.w) != 0.0)
                {
                dw = 1.0 / w;
                pDest->x = (int) (pSource->point.x * w);
                pDest->y = (int) (pSource->point.y * w);
                }
            else
                {
                pDest->x = pDest->y = 0;
                allWeightsNonZero = false;
                }
            nGot++;
            i0++;
            if (pSource->mask & HPOINT_MASK_BREAK)
                break;
            }
        }

    *pStartIndex = i0;
    *pNGot = nGot;
    return  allWeightsNonZero;
    }
#endif


/*---------------------------------------------------------------------------------**//**
* Get normalized points out of the array.
* w=0 points are set to zero
* @param    pInstance => source array
* @param pPoint <=> rubber array to receive points
* @param i0 => index of first point to copy
* @param nreq => number of points requested
* @return true if all copied points had nonzero weights.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlGraphicsPointArray_getDPoint3dVarArray
(
GraphicsPointArrayCP pInstance,
      EmbeddedDPoint3dArray     *pPointArray,
      int                   i0,
      int                   nreq
)
    {
    bool        allWeightsNonZero = true;
    DPoint3d    point;
    const GraphicsPoint *pSource;
        int nPoint = GET_COUNT (pInstance);
    double      w, dw;

    if (i0 < 0)
        {
        i0 = 0;
        }

    pSource = GET_CONST_ELEMENT_PTR (pInstance, i0);
    if  (pSource)
        {
        if ( i0 + nreq > nPoint )
                nreq = nPoint - i0;

        for ( ; nreq > 0 ;  nreq--, pSource++)
            {
            if ((w = pSource->point.w) != 0.0)
                {
                dw = 1.0 / w;
                point.x = pSource->point.x * dw;
                point.y = pSource->point.y * dw;
                point.z = pSource->point.z * dw;
                }
            else
                {
                point.x = point.y = point.z = 0.0;
                allWeightsNonZero = false;
                }
            jmdlEmbeddedDPoint3dArray_addDPoint3d (pPointArray, &point);
            }
        }

    return  allWeightsNonZero;
    }


/*---------------------------------------------------------------------------------**//**
*
* Get normalized points out of the array, starting at index i0 and
* continuing until a point with a marker (break mask) is reached.
* w=0 points are set to zero
*
* @param pPoint <= filled point buffer
* @param nGotP <= number of points copied to buffer
* @param pStartIndex <=> start index.   Initialize to 0 before
*                                   first call; this function increments
*                                   it as points are removed.
* @param maxGet => max points to get
* @return true if all points had nonzero weight
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlGraphicsPointArray_getDPoint3dLoop
(
GraphicsPointArrayCP pInstance,
      DPoint3d      *pPoint,
      int           *pNGot,
      int           *pStartIndex,
      int           maxGet
)
    {
    bool        allWeightsNonZero = true;
    DPoint3d    *pDest;
    const GraphicsPoint *pSource;
    int nPoint = GET_COUNT (pInstance);
    double      w, dw;
    int nGot = 0;
    int i0;

    i0 = *pStartIndex;
    if (i0 < 0)
        {
        i0 = 0;
        }

    pSource = GET_CONST_ELEMENT_PTR (pInstance, i0);
    if  (pSource)
        {
        pDest = pPoint;
        if (i0 + maxGet > nPoint)
            maxGet = nPoint - i0;

        for ( ; nGot < maxGet;  pSource++, pDest++)
            {
            if ((w = pSource->point.w) != 0.0)
                {
                dw = 1.0 / w;
                pDest->x = pSource->point.x * w;
                pDest->y = pSource->point.y * w;
                pDest->z = pSource->point.z * w;
                }
            else
                {
                pDest->x = pDest->y = pDest->z = 0.0;
                allWeightsNonZero = false;
                }
            nGot++;
            i0++;
            if (pSource->mask & HPOINT_MASK_BREAK)
                break;
            }
        }

    *pStartIndex = i0;
    *pNGot = nGot;
    return  allWeightsNonZero;
    }


/*---------------------------------------------------------------------------------**//**
*
* Get normalized points out of the array, starting at index i0 and
* continuing until a point with a marker (break mask) is reached.
* Points are copied out as x,y,z with no normalization, i.e. this assumes
* callers knows the points are normalized a priori.
*
* @param pPoint <= filled point buffer
* @param nGotP <= number of points copied to buffer
* @param pStartIndex <=> start index.   Initialize to 0 before
*                                   first call; this function increments
*                                   it as points are removed.
* @param maxGet => max points to get
* @param mask => mask to terminate array.  Usually HPOINT_MASK_BREAK or HPOINT_MASK_MAJOR_BREAK.
* @return true if all points had nonzero weight
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void         jmdlGraphicsPointArray_getUnnormalizedDPoint3dArrayToBreak
(
GraphicsPointArrayCP pInstance,
      DPoint3d      *pPoint,
      int           *pNGot,
      int           *pStartIndex,
      int           maxGet,
      int           mask
)
    {
    //bool        allWeightsNonZero = true;
    DPoint3d    *pDest;
    const GraphicsPoint *pSource;
    int nPoint = GET_COUNT (pInstance);
    int nGot = 0;
    int i0;

    i0 = *pStartIndex;
    if (i0 < 0)
        {
        i0 = 0;
        }

    pSource = GET_CONST_ELEMENT_PTR (pInstance, i0);
    if  (pSource)
        {
        pDest = pPoint;
        if (i0 + maxGet > nPoint)
            maxGet = nPoint - i0;

        for ( ; nGot < maxGet;  pSource++, pDest++)
            {
            pDest->x = pSource->point.x;
            pDest->y = pSource->point.y;
            pDest->z = pSource->point.z;
            nGot++;
            i0++;
            if (pSource->mask & mask)
                break;
            }
        }

    *pStartIndex = i0;
    *pNGot = nGot;
    }


/*---------------------------------------------------------------------------------**//**
* Get unnormalized points out of the array.
*
* @param    pPoint  <= filled buffer
* @param    nGot    <= number of points received
* @param    i0      => first point to get
* @param    nReq    => number of points requested
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_getDPoint4dArray
(
GraphicsPointArrayCP pInstance,
            DPoint4d        *pPoint,
            int                     *nGot,
            int                     i0,
            int                     nreq
)
    {
    //bool        allWeightsNonZero = true;
    DPoint4d    *pDest;
    const GraphicsPoint *pSource;
    int nPoint = GET_COUNT (pInstance);

    if (i0 < 0)
        i0 = 0;

    pSource = GET_CONST_ELEMENT_PTR (pInstance, i0);
    if  (pSource)
        {
        if ( i0 + nreq > nPoint )
            nreq = nPoint - i0;
        pDest = pPoint;
        for ( ; nreq > 0 ;  nreq--, pSource++, pDest++)
            {
            *pDest = pSource->point;
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
*
* Get an (unnormalized) point and its mask.
*
* @param    pInstance
* @param    pPoint <= point copied from the array.
* @param    pMask  <= mask copied from the array.
* @return true if index is valid.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_getDPoint4dWithMask
(
GraphicsPointArrayCP pInstance,
            DPoint4d    *pPoint,
            int             *pMask,
            int             index
)
    {
    GraphicsPoint *pElement = GET_CONST_ELEMENT_PTR (pInstance, index);
    bool    valid = false;
    if (pElement)
        {
        if (pPoint)
            *pPoint = pElement->point;
        if (pMask)
            *pMask      = pElement->mask;
        valid = true;
        }
    return valid;
    }


/*---------------------------------------------------------------------------------**//**
* Get an (unnormalized) point and its mask.
*
* @param    pInstance
* @param    pPoint <= point copied from the array.
* @param    pMask  <= mask copied from the array.
* @return true if index is valid.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_getDPoint4dWithMaskExt
(
GraphicsPointArrayCP pInstance,
            DPoint4d    *pPoint,
            int             *pMask,
            double          *pB,
            int             index
)
    {
    GraphicsPoint *pElement = GET_CONST_ELEMENT_PTR (pInstance, index);
    bool    valid = false;
    if (pElement)
        {
        if (pPoint)
            *pPoint = pElement->point;
        if (pMask)
            *pMask      = pElement->mask;
        if (pB)
            *pB = pElement->b;
        valid = true;
        }
    return valid;
    }



/*---------------------------------------------------------------------------------**//**
*
* Extend a range to include the (normalized) points in a GraphicsPointArray.
* Points with zero weight are ignored for range purposes, and a count
* of such is returned.
*
* @param pRange <=> range to update
* @param    pInstance => point array
* @return Number of points with weight = 0
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlGraphicsPointArray_extendDRange3d
(
GraphicsPointArrayCP pInstance,
        DRange3d *pRange
)
    {
    return jmdlDRange3d_extendByGraphicsPointArray (pRange, pInstance);
    }


/*---------------------------------------------------------------------------------**//**
@description return a tolerance based on the ranges of the arrays.
The returned number is (absTol + globalRelTol * A + rangeFraction * B)
where A is largest coordinate in combined range, B is diagonal of combined range.
@param [in] pInstanceA source data.
@param [in] pInstanceB source data.
@param [in] absTol absolute tolerance
@param [in] globalRelTol fraction of global size (data range extended to origin)
@Param [in] rangeFraction fraction of the data range itself.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double jmdlGraphicsPointArray_getTolerance
(
GraphicsPointArrayCP pInstanceA,
GraphicsPointArrayCP pInstanceB,
double absTol,
double globalRelTol,
double rangeFraction
)
    {
    DRange3d range;
    range.Init ();
    if (pInstanceA)
        jmdlDRange3d_extendByGraphicsPointArray (&range, pInstanceA);
    if (pInstanceB)
        jmdlDRange3d_extendByGraphicsPointArray (&range, pInstanceA);
    double result = absTol;
    if (!range.IsNull ())
        {
        result += globalRelTol * range.LargestCoordinate ();
        result += rangeFraction * bsiDPoint3d_maxAbsDifference (&range.low, &range.high);
        }
    return result;
    }




/*---------------------------------------------------------------------------------**//**
*
* Extend a range to include the (normalized) points in a GraphicsPointArray.
* Points with zero weight are ignored for range purposes, and a count
* of such is returned.
*
* @param pRange <=> range to update
* @param    pInstance => point array
* @return Number of points with weight = 0
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlDRange3d_extendByGraphicsPointArray
(
DRange3d *pRange,
GraphicsPointArrayCP pInstance
)
    {
    int numZero = 0;
    size_t tailIndex;
    int n, i;
    double w;
    int curveType;
    DPoint3d normalizedPoint;
    DPoint4d poleArray[MAX_BEZIER_ORDER];
    int      numPole;
    DConic4d conic;
    double      dw ;
    GraphicsPoint *pElement = GET_CONST_ELEMENT_PTR (pInstance, 0);

    if (pInstance && pElement)
        {
        for (n = GET_COUNT (pInstance), i = 0;
             i < n;
             )
            {
            curveType = HPOINT_GET_CURVETYPE_BITS (pElement[i].mask);

            if (   curveType == HPOINT_MASK_CURVETYPE_ELLIPSE
                 && jmdlGraphicsPointArray_getDConic4d (pInstance, &i, &conic, NULL, NULL, NULL, NULL)
                )
                {
                DRange3d curveRange;
                bsiDConic4d_getRange (&conic, &curveRange);
                bsiDRange3d_extendByRange (pRange, &curveRange);
                }
            else if (   curveType == HPOINT_MASK_CURVETYPE_BEZIER   // BSPLINE_CODED
                 && jmdlGraphicsPointArray_getBezier (pInstance, &i, poleArray, &numPole, MAX_BEZIER_ORDER))
                {
                DRange3d curveRange;
                bsiBezierDPoint4d_getDRange3d (&curveRange, poleArray, numPole);
                bsiDRange3d_extendByRange (pRange, &curveRange);
                }
            else if (pInstance->IsBsplineCurve (i, tailIndex))
                {
                bool isNullInterval;
                double knot0, knot1;
                for (size_t spanIndex = 0;
                    pInstance->GetBezierSpanFromBsplineCurve (i, spanIndex, poleArray, numPole,
                            MAX_BEZIER_ORDER, isNullInterval, knot0, knot1); spanIndex++)
                    {
                    if (!isNullInterval)
                        {
                        DRange3d curveRange;
                        bsiBezierDPoint4d_getDRange3d (&curveRange, poleArray, numPole);
                        bsiDRange3d_extendByRange (pRange, &curveRange);
                        }
                    }
                i = (int)tailIndex + 1;
                }
            else
                {
                w = pElement[i].point.w;
                if (w == 0.0)
                        {
                        numZero++;
                        }
                else
                        {
                            dw = 1.0 / w;
                        normalizedPoint.x = pElement[i].point.x * dw;
                        normalizedPoint.y = pElement[i].point.y * dw;
                        normalizedPoint.z = pElement[i].point.z * dw;
                        bsiDRange3d_extendByDPoint3d (pRange, &normalizedPoint);
                        }
                i++;
                }
            }
        }

    return numZero;
    }

/*---------------------------------------------------------------------------------**//**
*
* Return the range of a single primitive in the GPA.
*
* @param    pInstance => point array
* @param    pRange <= range of primitive.
* @return false if not a primitive index or zero weight encountered.
* @bsihdr                                       EarlinLutz      03/11
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool jmdlGraphicsPointArray_getPrimitiveRange
(
GraphicsPointArrayCP pInstance,
DRange3d *pRange,
int i0
)
    {
    int i = i0;
    int curveType;
    DPoint4d poleArray[MAX_BEZIER_ORDER];
    int      numPole;
    DConic4d conic;
    GraphicsPoint *pElement = GET_CONST_ELEMENT_PTR (pInstance, 0);
    bsiDRange3d_init (pRange);
    if (pInstance && pElement)
        {
        int n = GET_COUNT (pInstance);
        if (i < n)
            {
            curveType = HPOINT_GET_CURVETYPE_BITS (pElement[i].mask);

            if (   curveType == HPOINT_MASK_CURVETYPE_ELLIPSE
                 && jmdlGraphicsPointArray_getDConic4d (pInstance, &i, &conic, NULL, NULL, NULL, NULL)
                )
                {
                DRange3d curveRange;
                bsiDConic4d_getRange (&conic, &curveRange);
                bsiDRange3d_extendByRange (pRange, &curveRange);
                return true;
                }
            else if (   curveType == HPOINT_MASK_CURVETYPE_BEZIER
                 && jmdlGraphicsPointArray_getBezier (pInstance, &i, poleArray, &numPole, MAX_BEZIER_ORDER))
                {
                DRange3d curveRange;
                bsiBezierDPoint4d_getDRange3d (&curveRange, poleArray, numPole);
                bsiDRange3d_extendByRange (pRange, &curveRange);
                return true;
                }
            else
                {
                bsiDRange3d_extendByDPoint4d (pRange, &pElement[i0].point);
                if (i0 + 1 < n)
                    bsiDRange3d_extendByDPoint4d (pRange, &pElement[i0 + 1].point);
                return true;
                }
            }
        }

    return false;
    }


/*---------------------------------------------------------------------------------**//**
* Find the closest approach to a given point, using only xy data parts after normalization.
* @param pInteriorProximity <= closest point, only interior normal points.
* @param pEndpointProximity <= closest endpoint.
* @param pPickPoint => the pick point.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlGraphicsPointArray_closestXYPoint
(
GraphicsPointArrayCP pInstance,
      ProximityData *pInteriorProximity,
      ProximityData *pEndpointProximity,
const DPoint3d      *pPickPoint
)
    {
    //int numZero = 0;
    int n, i;
    int curveType;
    double param;
    DPoint4d poleArray[MAX_BEZIER_ORDER];
    DPoint4d nearPoint;
    DPoint4d startPoint, endPoint;
    int      numPole;
    DConic4d conic;
    size_t tailIndex;
    int i0;
        GraphicsPoint *pElement = GET_CONST_ELEMENT_PTR (pInstance, 0);

    bsiProximityData_init (pInteriorProximity, pPickPoint, -1, 0.0);
    bsiProximityData_init (pEndpointProximity, pPickPoint, -1, 0.0);

    if (pInstance)
        {
        for (n = GET_COUNT (pInstance), i = 0;
             i < n;
             )
            {
            curveType = HPOINT_GET_CURVETYPE_BITS (pElement[i].mask);
            i0 = i;

            if (   curveType == HPOINT_MASK_CURVETYPE_ELLIPSE
                     && jmdlGraphicsPointArray_getDConic4d
                                (pInstance, &i, &conic, NULL, NULL, &startPoint, &endPoint)
                    )
                {
                int j;
                double projectionAngle[4];
                int numProjection = bsiDConic4d_projectDPoint3dXYBounded
                        (
                        &conic,
                        NULL,
                        projectionAngle,
                        pPickPoint
                        );
                for (j = 0; j < numProjection; j++)
                    {
                    double fraction = bsiDConic4d_angleParameterToFraction (&conic, projectionAngle[j]);
                    bsiDConic4d_angleParameterToDPoint4d (&conic, &nearPoint, projectionAngle[j]);
                    bsiProximityData_testXY (pInteriorProximity, &nearPoint, fraction, i0);
                    }

                bsiProximityData_testXY (pEndpointProximity, &startPoint, 0.0,  i0);
                bsiProximityData_testXY (pEndpointProximity, &endPoint, 1.0, i0);
                }
            else if (   curveType == HPOINT_MASK_CURVETYPE_BEZIER   // BSPLINE_CODED
                     && jmdlGraphicsPointArray_getBezier (pInstance, &i, poleArray, &numPole, MAX_BEZIER_ORDER))
                {
                double dist2;
                if (bsiBezierDPoint4d_closestXYPoint (&nearPoint, &param, &dist2,
                                    poleArray, numPole, pPickPoint->x, pPickPoint->y,
                                    0.0, 1.0))
                        {
                        bsiProximityData_testXY (pInteriorProximity, &nearPoint, param, i0);
                        }

                bsiProximityData_testXY (pEndpointProximity, &poleArray[0], 0.0, i0);
                bsiProximityData_testXY (pEndpointProximity, &poleArray[numPole - 1], 1.0, i0);

                }
            else if (pInstance->IsBsplineCurve (i, tailIndex))
                {
                bool isNullInterval;
                double knot0, knot1;
                double dist2;
                for (size_t spanIndex = 0;
                    pInstance->GetBezierSpanFromBsplineCurve (i, spanIndex, poleArray, numPole,
                            MAX_BEZIER_ORDER, isNullInterval, knot0, knot1); spanIndex++)
                    {
                    if (!isNullInterval)
                        {
                    if (bsiBezierDPoint4d_closestXYPoint (&nearPoint, &param, &dist2,
                                    poleArray, numPole, pPickPoint->x, pPickPoint->y,
                                    0.0, 1.0))
                            {
                            bsiProximityData_testXY (pInteriorProximity, &nearPoint, param, i0);
                            }
                        }
                    }
                DPoint4d startPoint, endPoint;
                pInstance->GetDPoint4d (i, startPoint);
                pInstance->GetDPoint4d (i, endPoint);
                bsiProximityData_testXY (pEndpointProximity, &startPoint, 0.0, i0);
                bsiProximityData_testXY (pEndpointProximity, &endPoint  , 1.0, i0);
                
                i = (int)tailIndex + 1;
                }
            else
                {
                int j = i + 1;
                if (j < n && !(pElement[i].mask & HPOINT_MASK_BREAK))
                    {
                    DPoint4d point0 = pElement[i].point;
                    DPoint4d point1 = pElement[j].point;
                    /* line segment pick. */
                    bsiGeom_projectDPoint3dToDPoint4dLineXY
                            (
                            &nearPoint,
                            &param,
                            pPickPoint,
                            &point0,
                            &point1
                            );

                    if (0.0 <= param && param <= 1.0)
                        {
                        bsiProximityData_testXY (pInteriorProximity, &nearPoint, param, i);
                        }

                    bsiProximityData_testXY (pEndpointProximity, &point0, 0.0,i);
                    if (pElement[j].mask & HPOINT_MASK_BREAK)
                        {
                        bsiProximityData_testXY (pEndpointProximity, &point1, 1.0, i);
                        }
                    }
                i++;
                }
            }
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
*
* Copy the contents of one array into another
*
* @param pDestHeader <= array to be expanded
* @param pSourceHeader => source array
* @return true if both arrays are well defined an memory is available.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_append
(
GraphicsPointArrayP pDestHeader,
GraphicsPointArrayCP pSourceHeader
)
    {
    return SUCCESS == omdlVArray_append (&pDestHeader->vbArray_hdr, &pSourceHeader->vbArray_hdr);
    }


/*---------------------------------------------------------------------------------**//**
*
* @param    pInstance <=> header to receive polygons
* @param pPoint     => vertex array
* @param nPoint     => range limit for vertex array
* @param vertIndexP => array of maxPerFace*nFace vertex indices.
*                                     Unused indices in each row are assumed filled
*                                     with negative values
* @param maxPerFace => row dimension in pIndex
* @param nFace      => number of faces
* @param isSolid    => If true, mesh represents a full solid boundary. (Mesh itself has no boundary)
* @return int
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlGraphicsPointArray_addMeshArrays
(
GraphicsPointArrayP pInstance,
const DPoint3d  *pPoint,
int              nPoint,
const int       *vertIndexP,
int              maxPerFace,
int              nFace,
bool             isSolid
)
    {
    int status = ERROR;
    //int nAdd = nFace * (maxPerFace + 1);

    if  ( pInstance && EXTEND_GPA_BY (pInstance, nAdd))
        {
        int mask;
        int face;
        int i,n, index;
        const int *indexP;
        status = SUCCESS;

        for ( face = 0; face < nFace; face++)
            {
            n = 0;
            for (i = 0, indexP = vertIndexP + face * maxPerFace;
                 i < maxPerFace && (index = indexP[i]) >= 0 && index < nPoint;
                 i++
                 )
                {
                mask = HPOINT_NORMAL;
                if (isSolid)
                    {
                    /* if the mesh is from a solid, only draw ascending lines to avoid
                       double drawing */
                    int nextIndex = indexP[(i + 1) % maxPerFace];
                    mask = nextIndex > index ? HPOINT_NORMAL : HPOINT_MASK_BREAK;
                    }
                jmdlGraphicsPointArray_addDPoint3dWithMask (pInstance, &pPoint[index], mask);
                n++;
                }

            /* Normally expect to add a closure edge back to first vertex of this face.
               If 2 or fewer points, just mark the break.
            */
            if (n > 2)
                {
                /* Close the loop */
                jmdlGraphicsPointArray_addDPoint3dWithMask
                                    (
                                    pInstance,
                                    &pPoint[indexP[0]],
                                    HPOINT_MASK_BREAK
                                    );
                }
            else if (n > 0)
                {
                jmdlGraphicsPointArray_markBreak (pInstance);
                }
            }
        }
    return status;
    }





/*---------------------------------------------------------------------------------**//**
* @param    pInstance <=> header whose points are to be transformed
* @param pTransform => transform to apply
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlGraphicsPointArray_multiplyByTransform
(
GraphicsPointArrayP pInstance,
const Transform      *pTransform
)
    {
    int nPoint = GET_COUNT (pInstance);
    GraphicsPoint *pElement;
    int i;

    pElement = GET_ELEMENT_PTR (pInstance, 0);

    for (i = 0; i < nPoint; i++)
        {
        bsiTransform_multiplyDPoint4dArray
                        (
                            pTransform,
                        &pElement[i].point,
                        &pElement[i].point,
                        1
                        );
        }
    }



/*---------------------------------------------------------------------------------**//**
* @param    pInstance <=> header whose points to be unweighted
* @bsihdr                                       EarlinLutz      12/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlGraphicsPointArray_normalizeWeights
(
GraphicsPointArrayP pInstance
)
    {
    int nPoint = GET_COUNT (pInstance);
    GraphicsPoint *pElement;
    int i;

    pElement = GET_ELEMENT_PTR (pInstance, 0);

    for (i = 0; i < nPoint; i++)
        {
        bsiDPoint4d_normalizeWeightInPlace (&pElement[i].point);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @param    pInstance => header whose max x,y, or z component is returned.  The
*       components are used "as is" without normalizing by the weights.
* @bsihdr                                       EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   jmdlGraphicsPointArray_getMaxXYZComponent
(
GraphicsPointArrayCP pInstance
)
    {
    int nPoint = GET_COUNT (pInstance);
    const GraphicsPoint *pElement;
    int i;
    double maxValue = 0.0;

    pElement = GET_CONST_ELEMENT_PTR (pInstance, 0);

    for (i = 0; i < nPoint; i++)
        {
        if (fabs (pElement[i].point.x) > maxValue)
            maxValue = fabs (pElement[i].point.x);
        if (fabs (pElement[i].point.y) > maxValue)
            maxValue = fabs (pElement[i].point.y);
        if (fabs (pElement[i].point.z) > maxValue)
            maxValue = fabs (pElement[i].point.y);
        }

    return maxValue;
    }


/*---------------------------------------------------------------------------------**//**
* @param    pInstance => test if there are any non-unit weights
*       components are used "as is" without normalizing by the weights.
* @bsihdr                                       EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_hasNonUnitWeights
(
GraphicsPointArrayCP pInstance
)
    {
    int nPoint = GET_COUNT (pInstance);
    const GraphicsPoint *pElement;
    int i;
    double tolerance = 1.0e-12;

    pElement = GET_CONST_ELEMENT_PTR (pInstance, 0);

    for (i = 0; i < nPoint; i++)
        {
        if (fabs (pElement[i].point.w) - 1 > tolerance)
            return true;
        }

    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @param    pInstance   <=> header whose points are to be transformed
* @param pMatrix         => transform to apply
* @bsihdr                                       EarlinLutz      12/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlGraphicsPointArray_multiplyByDMatrix4d
(
GraphicsPointArrayP pInstance,
const DMatrix4d                 *pMatrix
)
    {
    int nPoint = GET_COUNT (pInstance);
    GraphicsPoint *pElement;
    int i;
    Transform transform;
    pElement = GET_ELEMENT_PTR (pInstance, 0);
    

    if (bsiTransform_initFromDMatrix4d (&transform, pMatrix))
        {
        for (i = 0; i < nPoint; i++)
            {
            bsiTransform_multiplyDPoint4dArray (&transform, 
                            &pElement[i].point,
                            &pElement[i].point,
                            1
                            );
            }
        }
    else
        {
        for (i = 0; i < nPoint; i++)
            {
            bsiDMatrix4d_multiplyMatrixPoint
                            (
                            pMatrix,
                            &pElement[i].point,
                            &pElement[i].point
                            );
            }
        }
    }


#ifdef SupportGetBlock

/*---------------------------------------------------------------------------------**//**
* @param    pInstance <=> array from which to get block
* @param n => number of entries requested
* @return pointer to contiguous block of new GraphicsPoint structures
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP GraphicsPoint * jmdlGraphicsPointArray_getBlock
(
GraphicsPointArrayP pInstance,
int             n
)
    {
    return (GraphicsPoint *)omdlVArray_getNewBlock (&pInstance->vbArray_hdr, n);
    }
#endif



/*---------------------------------------------------------------------------------**//**
* @param    pInstance <=> header of array receiveing points
* @param pPoint => array of 2 points to add as a line segment if not already present.
* @param tol => tolerance for comparisons.
* @bsihdr                                       WouterRombouts      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlGraphicsPointArray_addLine3dUnique
(
GraphicsPointArrayP pInstance,
const DPoint3d              *pPoint,
      double                 tol
)
    {
    int cnt = jmdlGraphicsPointArray_getCount (pInstance);
    int i;

    int dummy;
    DPoint3d pair [2];

    for (i = 0; i < cnt; i+=2)
        {
        if (SUCCESS != jmdlGraphicsPointArray_getDPoint3dArray (pInstance, pair, &dummy, i, 2))
            return;
        if (bsiDPoint3d_distance (&pPoint[0], &pair[0]) < tol)
            {
            if (bsiDPoint3d_distance (&pPoint[1], &pair[1]) < tol)
                                return;
            }
        else
            {
            if (   bsiDPoint3d_distance (&pPoint[0], &pair[1]) < tol
                && bsiDPoint3d_distance (&pPoint[1], &pair[0]) < tol)
                return;
            }
        }

    jmdlGraphicsPointArray_addDPoint3dArray (pInstance, pPoint, 2);
    jmdlGraphicsPointArray_markBreak (pInstance);
    }

/*---------------------------------------------------------------------------------**//**
* Copy a contiguous block of points from one array to another, maintaining
* connectivity at the first point.   Terminate break or end of source.
* @param mask   => mask to OR not copied points.
*
* @bsimethod                                                                                                    EarlinLutz              11/98
+---------------+---------------+---------------+---------------+---------------+------*/
static int jmdlGraphicsPointArray_copyBlock
(
GraphicsPointArrayP pDestHeader,
GraphicsPointArrayP pSourceHeader,
int           iSource,
int           mask
)
    {
    GraphicsPoint dest;
    GraphicsPoint source;
    int                     numCopied   = 0;
    int stopMask = HPOINT_MASK_POINT | HPOINT_MASK_BREAK;
    double abstol = bsiTrig_smallAngle ();
    /* Remove the last point of the destination of it matches the new data. */

    if (   GET_ELEMENT (pSourceHeader, &source, iSource)
        && GET_ELEMENT (pDestHeader,   &dest,  -1)
        && !(dest.mask & stopMask)
        && bsiDPoint4d_realDistance (&source.point, &dest.point) <= abstol
        )
        {
        GraphicsPoint gp;
        omdlVArray_pop (&pDestHeader->vbArray_hdr, &gp);
        }

    do
        {
        numCopied++;
        GET_ELEMENT (pSourceHeader, &source, iSource);
        source.mask |= mask;
        pDestHeader->vbArray_hdr.push_back (source);
        } while (!(source.mask & stopMask));

    return numCopied;
    }

/*----------------------------------------------------------------------+
* @return index of first point matching pComparePoint; if none found, first point
*       without excludeMask.
| jmdlGraphicsPointArray_findBlockStart                 WouterRombouts       07/97  |
+----------------------------------------------------------------------*/
static int jmdlGraphicsPointArray_findBlockStart
(
GraphicsPointArrayP pInstance,
const   DPoint4d  *pComparePoint,
int     excludeMask
)
    {
    int i;
    const GraphicsPoint *pElement = GET_CONST_ELEMENT_PTR (pInstance, 0);
    int   n = GET_COUNT (pInstance);
    bool    atBreak = true;
    int stopMask = HPOINT_MASK_POINT | HPOINT_MASK_BREAK;
    int firstIndexWithoutExcludeMask = -1;
    DPoint4d refPoint = *pComparePoint;    /* Because distance calculator will normalize */
    DPoint4d currPoint;
    double abstol = bsiTrig_smallAngle ();

    for (i = 0; i < n; i++)
        {
        if (!(excludeMask & pElement[i].mask))
            {
            if (firstIndexWithoutExcludeMask == -1)
                firstIndexWithoutExcludeMask  =  i;
            currPoint = pElement[i].point;
            if (  atBreak
                && bsiDPoint4d_realDistance (&currPoint, &refPoint) < abstol
               )
                return i;
            }
        atBreak = (0 != (stopMask & pElement[i].mask));
        }
    return firstIndexWithoutExcludeMask;
    }


/*---------------------------------------------------------------------------------**//**
* Search for the first index at which a mask is set.
* @param i0 = start point of search
* @param mask = mask to search for
* @param implyFinalMask = true if final point is implied to have the mask when no mask is found.
*           If false, -1 is returned when no mask found.
* @return
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      jmdlGraphicsPointArray_findMask
(
GraphicsPointArrayCP pInstance,
        int             i0,
        int             mask,
        bool            implyFinalPoint
)
    {
    int i;
    const GraphicsPoint *pElement = GET_CONST_ELEMENT_PTR (pInstance, 0);
    int   n = GET_COUNT (pInstance);

    for (i = i0; i < n; i++)
        {
        if (pElement[i].mask & mask)
            return i;
        }
    return implyFinalPoint ? n - 1 : -1;
    }


/*---------------------------------------------------------------------------------**//**
* Copy a fragment from pSource into the instance array.   Data is copied within the
* given index range, reversing order if indicated by index values.  Do nothing if
* any index is out of range.
*
* @param i0 = first index of fragment to copy.
* @param i1 = last index of fragment to copy.
* @param clearMask = mask to be cleared throughout the fragment.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_appendFragment
(
GraphicsPointArrayP pInstance,
GraphicsPointArrayCP pSource,
        int             i0,
        int             i1,
        int             clearMask
)
    {
    int nSource = GET_COUNT (pSource);
    //int nDest   = GET_COUNT (pInstance);
    int i;
    int andMask = ~clearMask;
    GraphicsPoint  element;

    if (i0 >= 0 && i0 < nSource && i1 >= 0 && i1 < nSource)
        {
        if (i0 >= i1)
            {
            for (i = i0; i >= i1; i--)
                {
                GET_ELEMENT (pSource, &element, i);
                // Needs work - This is going to lose index. - No way to test, but needs to be recording reversed index.
                jmdlGraphicsPointArray_addDPoint4dWithMaskExt (pInstance, &element.point, andMask & element.mask, element.b);
                }
            }
        else
            {
            for (i = i0; i <= i1; i++)
                {
                GET_ELEMENT (pSource, &element, i);
                jmdlGraphicsPointArray_addDPoint4dWithMaskAndIndex (pInstance, &element.point, andMask & element.mask, element.b, element.index);
                }
            }
        }

    /* Call any old fragment a curve??? */
    jmdlGraphicsPointArray_setArrayMask (pInstance, HPOINT_ARRAYMASK_CURVES);
    }

/*---------------------------------------------------------------------------------**//**
* Copy coordinates of single line segment with ends at given indices in pSource into the instance array.
*
* @param i0 = index of start point.
* @param mask0 = mask to apply to start point.
* @param i1 = index of end point.
* @param mask1 = mask to apply to end point
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_appendSegment
(
GraphicsPointArrayP pInstance,
GraphicsPointArrayCP pSource,
        int             i0,
        int             mask0,
        int             i1,
        int             mask1
)
    {
    GraphicsPoint  element;
    int nSource = GET_COUNT (pSource);

    if (i0 >= 0 && i0 < nSource && i1 >= 0 && i1 < nSource)
        {
        GET_ELEMENT (pSource, &element, i0);
        jmdlGraphicsPointArray_addDPoint4dWithMask (pInstance, &element.point, mask0);

        GET_ELEMENT (pSource, &element, i1);
        jmdlGraphicsPointArray_addDPoint4dWithMask (pInstance, &element.point, mask1);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @param    pHeaderToOptimize <=> header of array receiveing points
* @return int
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_optimize
(
GraphicsPointArrayP pHeaderToOptimize
)
    {
    int numHandled;
    int fromIndex;

    if (pHeaderToOptimize)
        {
        int toOptimizeCount = jmdlGraphicsPointArray_getCount (pHeaderToOptimize);
        if (toOptimizeCount)
            {
            //make a copy
GraphicsPointArrayP pHPointListTemp = jmdlGraphicsPointArray_grab ();
            if (pHPointListTemp)
                {
                jmdlGraphicsPointArray_append (pHPointListTemp, pHeaderToOptimize);
                jmdlGraphicsPointArray_empty (pHeaderToOptimize);

                numHandled = jmdlGraphicsPointArray_copyBlock
                        (pHeaderToOptimize, pHPointListTemp, 0, HPOINT_MASK_USER1);
                while (numHandled < toOptimizeCount)
                    {
                    GraphicsPoint element;
                    GET_ELEMENT (pHeaderToOptimize, &element, -1);
                    fromIndex = jmdlGraphicsPointArray_findBlockStart
                                        (
                                        pHPointListTemp,
                                        &element.point,
                                        HPOINT_MASK_USER1
                                        );
                    numHandled += jmdlGraphicsPointArray_copyBlock
                                (pHeaderToOptimize, pHPointListTemp, fromIndex, HPOINT_MASK_USER1);
                    }

                jmdlGraphicsPointArray_drop (pHPointListTemp);
                return true;
                }
            }
        }
    return false;
    }



/*---------------------------------------------------------------------------------**//**
*
* Add a DPoint3d array to array.   For use as a callback.
*
* @param    pInstance <=> header to receive points.
* @param pPoint => PointArray
* @param numPoint => number of points
* @param pVoid => unused
* @return SUCCESS if
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt jmdlGraphicsPointArray_cbAddDPoint3dArray
(
GraphicsPointArrayP pInstance,
const   DPoint3d    *pPoint,
        int         numPoint,
        void        *pVoid
)
    {
    jmdlGraphicsPointArray_addDPoint3dArray (pInstance, pPoint, numPoint);
    jmdlGraphicsPointArray_markBreak (pInstance);
    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
*
* Set the "a" field of each point as its planar halfspace "height" value.
* @param pPlane     => plane coefficients
* @return -1 i fall in (negative height), 1 if all out (positive height), 0 if mixed.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlGraphicsPointArray_setHeight     /* <= -1 if all IN, 1 if all OUT, 0 if mixed */
(
GraphicsPointArrayP pInstance,     /* <=> Polyline or polygon. */
const   DPoint4d                *pPlane         /* => homogeneous plane equations */
)
    {
    int i;
    int outFlag = 0;
    int inFlag  = 0;
    int curveFlag = 0;
    //int retValue = 0;
    int n = jmdlGraphicsPointArray_getCount (pInstance);
    GraphicsPoint   *pPoint = jmdlGraphicsPointArray_getPtr (pInstance, 0);
    double h0;

    /* Precompute the height of each point over the plane */
    for (i = 0;
        i < n;
        i++, pPoint++ )
        {
        pPoint->a = h0 =      pPlane->x * pPoint->point.x
                           +  pPlane->y * pPoint->point.y
                           +  pPlane->z * pPoint->point.z
                           +  pPlane->w * pPoint->point.w;

        if (IS_NON_POLE_MASK (pPoint->mask))
            {
            curveFlag = 1;
            }
        else if (h0 > 0.0)
            {
            outFlag = 1;
            }
        else if (h0 < 0.0)
            {
            inFlag = 1;
            }
        }

    return curveFlag ? 0 : outFlag - inFlag;
    }

static int jmdlGraphicsPoint_compareA
(
const GraphicsPoint *pPoint0,
const GraphicsPoint *pPoint1
)
    {
    if (pPoint0->a < pPoint1->a)
        return -1;
    if (pPoint0->a > pPoint1->a)
        return 1;
    return 0;
    }

static bool compareLessThan_byA
(
const GraphicsPoint &point0,
const GraphicsPoint &point1
)
    {
    return jmdlGraphicsPoint_compareA (&point0, &point1) <= 0;
    }

static int jmdlGraphicsPoint_compareUserDataAndA
(
const GraphicsPoint *pPoint0,
const GraphicsPoint *pPoint1
)
    {
    if (pPoint0->userData < pPoint1->userData)
        return -1;
    if (pPoint0->userData > pPoint1->userData)
        return 1;

    if (pPoint0->a < pPoint1->a)
        return -1;
    if (pPoint0->a > pPoint1->a)
        return 1;
    return 0;
    }

static bool compareLessThan_byUserDataThenA
(
const GraphicsPoint &point0,
const GraphicsPoint &point1
)
    {
    return jmdlGraphicsPoint_compareUserDataAndA (&point0, &point1) <= 0;
    }



static int jmdlGraphicsPoint_compareYThenX
(
const GraphicsPoint *pPoint0,
const GraphicsPoint *pPoint1
)
    {
    if (pPoint0->point.y < pPoint1->point.y)
        return -1;
    if (pPoint0->point.y > pPoint1->point.y)
        return 1;

    if (pPoint0->point.x < pPoint1->point.x)
        return -1;
    if (pPoint0->point.x > pPoint1->point.x)
        return 1;
    return 0;
    }

static bool compareLessThan_byYThenX
(
const GraphicsPoint &point0,
const GraphicsPoint &point1
)
    {
    return jmdlGraphicsPoint_compareYThenX(&point0, &point1) <= 0;
    }




/*---------------------------------------------------------------------------------**//**
*
* Sort by the "a" field of each graphics point.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlGraphicsPointArray_sortByA
(
GraphicsPointArrayP pInstance
)
    {
    std::sort (pInstance->vbArray_hdr.begin (), pInstance->vbArray_hdr.end (),
                        compareLessThan_byA);
    }


/*---------------------------------------------------------------------------------**//**
*
* Sort by the "a" field of each graphics point.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlGraphicsPointArray_sortByUserDataAndA
(
GraphicsPointArrayP pInstance
)
    {
    std::sort (pInstance->vbArray_hdr.begin (), pInstance->vbArray_hdr.end (),
                        compareLessThan_byUserDataThenA);
    }


/*---------------------------------------------------------------------------------**//**
*
* Sort by the y fields, with x to resolve ties.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlGraphicsPointArray_sortByYThenX
(
GraphicsPointArrayP pInstance
)
    {
    std::sort (pInstance->vbArray_hdr.begin (), pInstance->vbArray_hdr.end (),
                        compareLessThan_byYThenX);
    }


/*---------------------------------------------------------------------------------**//**
*
* Sort by the y fields, with x to resolve ties.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlGraphicsPointArray_installSortIndex
(
GraphicsPointArrayP pInstance
)
    {
    size_t count = pInstance->vbArray_hdr.size ();
    for (size_t i = 0; i < count; i++)
        pInstance->vbArray_hdr[i].index = i;
    }


/*---------------------------------------------------------------------------------**//**
* On input, pSortedArray is sorted, and index field indicates original position.
* pArrayToShuffle is to be rearranged by same sort.
@param [in,out] pDest points to shuffle.
@param [in] pSource sorted and tagged array.
@return true if sort indices are valid.  Array sizes must match.  index fields in source
 must be a permutation.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool jmdlGraphicsPointArray_shuffleBySortIndex
(
GraphicsPointArrayP pDest,
GraphicsPointArrayP pSource
)
    {
    size_t count = pSource->vbArray_hdr.size ();
    if (pDest->vbArray_hdr.size () != count)
        return false;
    // Install invalid indices in second array...
    for (size_t i = 0; i < count; i++)
        pDest->vbArray_hdr[i].index = count;
    // Mark each destination point with where it SHOULD BE (And confirm no duplicates)
    for (size_t i = 0; i < count; i++)
        {
        size_t k = pSource->vbArray_hdr[i].index;
        if (k >= count)
            return false;
        if (pDest->vbArray_hdr[k].index != count)
            return false;
        pDest->vbArray_hdr[k].index = i;
        }

    for (size_t i0 = 0; i0 < count; i0++)
        {
        size_t i1 = pDest->vbArray_hdr[i0].index;
        if (i0 == i1)
            i0++;
        else
            {
            GraphicsPoint q = pDest->vbArray_hdr[i0];
            pDest->vbArray_hdr[i0] = pDest->vbArray_hdr[i1];
            pDest->vbArray_hdr[i1] = q;
            }
        }
    return true;
    }




/*---------------------------------------------------------------------------------**//**
* Given two references to (primitive, fractionParameter), generate a (fractionParameter)
* return a comparison in qsort style: -1, 0, or 1 according as point 0 is before, equal
* to or following point1.
* @param i0 => primitive index for first point.
* @param param0 => parameter for first point.
* @param i1 => primitive index for second point.
* @param param1 => parameter for second point.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlGraphicsPointArray_comparePrimitiveFraction
(
GraphicsPointArrayCP pSource,
int i0,
double  param0,
int i1,
double param1
)
    {
    if (i0 < i1)
        return -1;
    if (i0 > i1)
        return 1;
    if (param0 < param1)
        return -1;
    if (param0 > param1)
        return 1;
    return 0;
    }


/*---------------------------------------------------------------------------------**//**
* Given two references to (primitive, fractionParameter), generate a (fractionParameter)
* for an arbitrary point within the interval.
* @param pI2 <= primitive index for intermediate point.
* @param param2 <= parameter for intermediate point.
* @param i0 => primitive index for start point.
* @param param0 => parameter for start point.
* @param i1 => primitive index for end point.
* @param param1 => parameter for end point.
* @return true if both positions are valid.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlGraphicsPointArray_anyPrimitiveFractionInInterval
(
GraphicsPointArrayCP pSource,
int *pI2,
double *pParam2,
int i0,
double  param0,
int i1,
double param1
)
    {
    if (i0 == i1)
        {
        *pI2 = i0;
        *pParam2 = 0.5 * (param0 + param1);
        return true;
        }

    if (i0 < i1)
        {
        if (1.0 - param0 > param1)
            {
            /* Move to the end of the first primitive */
            *pI2 = i0;
            *pParam2 = 1.0;
            return true;
            }
        else
            {
            /* Move to the beginning of the second primitive */
            *pI2 = i1;
            *pParam2 = 0.0;
            return true;
            }
        }
    else
        {
        if (param0 < 1.0 - param1)
            {
            /* Move to the end of the "second" primitive */
            *pI2 = i1;
            *pParam2 = 1.0;
            return true;
            }
        else
            {
            /* Move to the beginning of the "first" primitive */
            *pI2 = i0;
            *pParam2 = 0.0;
            return true;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Append an interval of the source to the destination.  The geometry starting at
* fraction param0 on primitive i0 and ending at fraction param1 on primitive i1 is
* copied into the destination (instance) array.  If (i0,param1) is "after" (i1,param1),
* the geoemtry is reversed during the copy.
* @param pSource => source data for copy.
* @param i0 => start index of first primitive.
* @param param0 => fractional parameter on first primitive.
* @param i1 => start index of second primitive.
* @param param1 => fractional parameter on second primitive.
*
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlGraphicsPointArray_appendInterval
(
GraphicsPointArrayP pDest,
GraphicsPointArrayCP pSource,
int i0,
double  param0,
int i1,
double param1

)
    {
    int curr0, curr1;
    curr0 = curr1 = 0;
    if (!jmdlGraphicsPointArray_parsePrimitiveAt (pSource, &curr0, &curr1, NULL, NULL, NULL, i0))
        {
        return false;
        }
    else if (i0 == i1)
        {
        /* Subset of single primitive. */
        return jmdlGraphicsPointArray_copyPartialPrimitive (pDest, pSource, i0, param0, param1);
        }
    else if (i1 > i0)
        {
        /* Forward direction copy */
        jmdlGraphicsPointArray_copyPartialPrimitive (pDest, pSource, curr0, param0, 1.0);
        while (jmdlGraphicsPointArray_parsePrimitiveAfter (pSource, &curr0, &curr1,
                            NULL, NULL, NULL, curr1) && curr0 < i1)
            {
            jmdlGraphicsPointArray_copyPartialPrimitive (pDest, pSource, curr0, 0.0, 1.0);
            }

        if (curr0 == i1)
            jmdlGraphicsPointArray_copyPartialPrimitive (pDest, pSource, curr0, 0.0, param1);
        }
    else
        {
        /* Reverse direction copy */
        jmdlGraphicsPointArray_copyPartialPrimitive (pDest, pSource, curr0, param0, 0.0);
        while (jmdlGraphicsPointArray_parsePrimitiveBefore (pSource, &curr0, &curr1,
                            NULL, NULL, NULL, curr0) && curr0 > i1)
            {
            jmdlGraphicsPointArray_copyPartialPrimitive (pDest, pSource, curr0, 1.0, 0.0);
            }

        if (curr0 == i1)
            jmdlGraphicsPointArray_copyPartialPrimitive (pDest, pSource, curr0, 1.0, param1);

        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Sort both arrays, using <userdata,a> in pGPA1 as sort key.
* The mask fields of all points in both arrays is cleared (working memory in sort)
* @return true if arrays had same counts and were sorted.
* @bsimethod                                                    EarlinLutz      12/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlGraphicsPointArray_sortParallelGPAs
(
GraphicsPointArrayP pGPA1,
GraphicsPointArrayP pGPA2
)
    {
    int n1 = jmdlGraphicsPointArray_getCount (pGPA1);
    int n2 = jmdlGraphicsPointArray_getCount (pGPA2);
    int i, j, k;
    int numSwap = 0;
    GraphicsPoint gp1, gp2, gp2j, gp2k;

    if (n1 != n2)
        return false;

    if (n1 <= 1)
        return true;

    for (i = 0; i < n1; i++)
        {
        jmdlGraphicsPointArray_getGraphicsPoint (pGPA1, &gp1, i);
        jmdlGraphicsPointArray_getGraphicsPoint (pGPA2, &gp2, i);
        gp1.mask = i;
        gp2.mask = i;
        jmdlGraphicsPointArray_setGraphicsPoint (pGPA1, &gp1, i);
        jmdlGraphicsPointArray_setGraphicsPoint (pGPA2, &gp2, i);
        }
    /* Sort pGPA1 directly */
    jmdlGraphicsPointArray_sortByUserDataAndA (pGPA1);
    /* Mask data in each point of pGPA1 tells original index for parallel motion in pGPA2 */

    for (i = 0; i < n1; i++)
        {
        /* Invariant: in GPA2, every entry is either in its original position or is final sorted position. */
        for (j = i;;j = k)
            {
            jmdlGraphicsPointArray_getGraphicsPoint (pGPA1, &gp1, j);
            jmdlGraphicsPointArray_getGraphicsPoint (pGPA2, &gp2j, j);
            k = gp1.mask; /* This is what SHOULD BE at position j in pGPA2 */
            if (k == gp2j.mask) /* Compare to what IS there .. */
                break;
            /* Swap j, k in gGPA2.  This makes position "j" right, but "k" may violate the
                invariant, so we need to continue fixup at "k".  This hopscotches through the
                order from pGPA1 until the swapping ends with k matched.
            */
            numSwap++;
            jmdlGraphicsPointArray_getGraphicsPoint (pGPA2, &gp2k, k);
            jmdlGraphicsPointArray_setGraphicsPoint (pGPA2, &gp2j, k);
            jmdlGraphicsPointArray_setGraphicsPoint (pGPA2, &gp2k, j);
            }
        }
#define CHECK_SORT_RESULTnot
#ifdef CHECK_SORT_RESULT
        {
        int errors = 0;
        for (i = 0; i < n1; i++)
            {
            jmdlGraphicsPointArray_getGraphicsPoint (pGPA1, &gp1, i);
            jmdlGraphicsPointArray_getGraphicsPoint (pGPA2, &gp2, i);
            if (gp1.mask != gp2.mask)
                {
                errors++;
                }
            }
        }

#endif
    /* Clear the masks */
    for (i = 0; i < n1; i++)
        {
        jmdlGraphicsPointArray_getGraphicsPoint (pGPA1, &gp1, i);
        jmdlGraphicsPointArray_getGraphicsPoint (pGPA2, &gp2, i);
        gp1.mask = 0;
        gp2.mask = 0;
        jmdlGraphicsPointArray_setGraphicsPoint (pGPA1, &gp1, i);
        jmdlGraphicsPointArray_setGraphicsPoint (pGPA2, &gp2, i);
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Compare two graphics points which originated in same GPA, i.e. have
* consistent userData and parameters.
* @param paramTol IN tolerance for parameter comparison.  Negative to suppress.  0 for bitwise equality.
* @param closureTol2 IN squared tolerance for xyz comparison of points which are
*               at start and end of same primitive.
* @param xyzTol2 IN squared tolerance for normalized point comparison.  Negative to suppress. 0 for bitwise equality.
* @bsimethod                                                    EarlinLutz      03/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_sameGraphicsPoint
(
const GraphicsPoint *pGP1,
const GraphicsPoint *pGP2,
double parameterTol,
double closureTol2,
double xyzTol2
)
    {
    double dist2;
    double nearOne = 1.0 - parameterTol;
    if (parameterTol >= 0.0)
        {
        if (pGP1->userData == pGP2->userData)
            {
            /* Same primitive.  Direct parameter comparison works. */
            if (fabs (pGP1->a - pGP2->a) <= parameterTol)
                return true;
            /* May be duplicate start/end of closed primitive. */
            if (closureTol2 >= 0.0
                &&  (   (pGP1->a < parameterTol && pGP2->a >= nearOne)
                    ||  (pGP1->a > nearOne && pGP2->a < parameterTol)
                    )
                && bsiDPoint4d_realDistanceSquared (&pGP1->point, &dist2, &pGP2->point)
                && dist2 < closureTol2
                )
                {
                return true;
                }
            }
        else if (pGP1->userData + 1 == pGP2->userData
                && pGP1->a > nearOne
                && pGP2->a < parameterTol
                )
            {
            return true;
            }
        else if (pGP1->userData == pGP2->userData + 1
                && pGP1->a < parameterTol
                && pGP2->a > nearOne
                )
            {
            return true;
            }
         }

    if (   xyzTol2 >= 0.0
        && bsiDPoint4d_realDistanceSquared (&pGP1->point, &dist2, &pGP2->point)
        && dist2 <= xyzTol2)
        return true;

    // OK, got past everything.
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* Linear search (quadratic time for all points!!) to eliminate duplicate points.
* @bsimethod                                                    EarlinLutz      03/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlGraphicsPointArray_cullDuplicatePoints
(
GraphicsPointArrayP         pGPA1,
GraphicsPointArrayP         pGPA2,
double                      parameterTol,
double                      xyzTol
)
    {
    GraphicsPoint gp1A, gp2A;
    double closureTol, closureTol2;
    static double s_closureRelTol = 1.0e-12;
    GraphicsPoint gp1B, gp2B;
    int iA, iB, numOut;
    double xyzTol2;
    int n1 = jmdlGraphicsPointArray_getCount (pGPA1);
    int n2 = jmdlGraphicsPointArray_getCount (pGPA2);
    bool    isDup = false;

    xyzTol2 = xyzTol > 0.0 ? xyzTol * xyzTol : xyzTol;

    if (n1 != n2)
        return false;

    if (n1 <= 1)
        return true;

    closureTol = s_closureRelTol * jmdlGraphicsPointArray_getMaxXYZComponent (pGPA1);
    closureTol2 = closureTol * closureTol;
    numOut = 0;
    /* iA and iB are where we read the arrays.
       iB is the lead point, the one being considered to keep.
       numOut is the count of points that have been accepted (as iB)
            and packed to the front of the array.
        for each iB, sweep iA from 0 to numOut-1 and check for dups.
           In the inner loop, we are comparing WITHIN arrays -- index A to index B.
    */
    for (iB = 0; iB < n1; iB++)
        {
        jmdlGraphicsPointArray_getGraphicsPoint (pGPA1, &gp1B, iB);
        jmdlGraphicsPointArray_getGraphicsPoint (pGPA2, &gp2B, iB);
        isDup = false;

        for (iA = 0; iA < numOut; iA++)
            {
            jmdlGraphicsPointArray_getGraphicsPoint (pGPA1, &gp1A, iA);
            jmdlGraphicsPointArray_getGraphicsPoint (pGPA2, &gp2A, iA);
            if (   jmdlGraphicsPointArray_sameGraphicsPoint (&gp1A, &gp1B, parameterTol, closureTol2, xyzTol2)
                && jmdlGraphicsPointArray_sameGraphicsPoint (&gp2A, &gp2B, parameterTol, closureTol2, xyzTol2))
                {
                isDup = true;
                break;
                }
            }

        if (!isDup)
            {
            jmdlGraphicsPointArray_setGraphicsPoint (pGPA1, &gp1B, numOut);
            jmdlGraphicsPointArray_setGraphicsPoint (pGPA2, &gp2B, numOut);
            numOut++;
            }
        }

    if (numOut < n1)
        {
        jmdlGraphicsPointArray_trim (pGPA1, numOut);
        jmdlGraphicsPointArray_trim (pGPA2, numOut);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
@description Test two arrays for exact equality of all data.
@param pGPA1 IN first array
@param pGPA2 IN second array
@return true if (a) array pointers are the same, (b) both arrays are empty, or (c) arrays
        are bitwise identical.
* @bsimethod                                                    EarlinLutz      03/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlGraphicsPointArray_identicalContents
(
GraphicsPointArrayCP  pGPA1,
GraphicsPointArrayCP  pGPA2
)
    {
    int n1 = jmdlGraphicsPointArray_getCount (pGPA1);
    int n2 = jmdlGraphicsPointArray_getCount (pGPA2);
    const GraphicsPoint*  pBuf1 = jmdlGraphicsPointArray_getConstPtr (pGPA1, 0);
    const GraphicsPoint*  pBuf2 = jmdlGraphicsPointArray_getConstPtr (pGPA2, 0);

    if (pGPA1 == pGPA2)
        return true;
    if (n1 != n2)
        return false;
    if (n1 == 0)
        return true;

    return 0 == memcmp (pBuf1, pBuf2, n1 * sizeof (pBuf1[0]));
    }

END_BENTLEY_GEOMETRY_NAMESPACE
