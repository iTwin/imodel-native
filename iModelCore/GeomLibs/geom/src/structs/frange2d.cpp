/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/frange2d.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*----------------------------------------------------------------------+
|                                                                       |
|   macro definitions                                                   |
|                                                                       |
+----------------------------------------------------------------------*/
#define FIX_MIN(value, min)          if (value < min) min = value
#define FIX_MAX(value, max)          if (value > max) max = value
#define FIX_MINMAX(value, min, max)  FIX_MIN(value, min); FIX_MAX(value, max);

#define FLT_FIX_MIN(value, min)     \
    {float __floatValue__ = (float)(value); \
    FIX_MIN(__floatValue__, min);}

#define FLT_FIX_MAX(value, max)     \
    {float __floatValue__ = (float)(value); \
    FIX_MAX(__floatValue__, max);}

#define FLT_FIX_MINMAX(value, min, max)     \
    {float __floatValue__ = (float)(value); \
    FIX_MIN(__floatValue__, min);           \
    FIX_MAX(__floatValue__, max);}

/*----------------------------------------------------------------------+
|                                                                       |
|   external variables                                                  |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|       local function definitions                                      |
|                                                                       |
+----------------------------------------------------------------------*/

#if !defined (FLT_MAX)
#define FLT_MAX 1.0e200
#endif

/*-----------------------------------------------------------------*//**
* @class FRange2d
* A FRange2d structure contains 2 points for use as min (low) and max (high)
* corners of a bounding box.
*
* For a non-null bounding box, each component (x,y,z) of the min point is
* strictly less than the corresponding component of the max point.

*
* The base init function ~mbsiFRange2d_init initializes the range to large
* values with max less than min.  This null state can be identified as a
* null box later on.
*
* Methods named bsiFRange2d_initFromXXX initialize the box around
* various combinations of points.  (E.g. one point, two points, 3 points,
* array of points.)
*
* Methods named bsiFRange2d_extendXXX expand an existing box to include additional
* range. (E.g. one point, array of points, components of point)
*
* @author   EarlinLutz
* @indexVerb
* @bsihdr                                               EarlinLutz      12/99
+===============+===============+===============+===============+======*/


/*-----------------------------------------------------------------*//**
*
* Initializes a range cube with (inverted) large positive and negative
* values.
*
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFRange2d_init

(
FRange2dP pRange
)

    {
    pRange->low.x = pRange->low.y =  FLT_MAX;
    pRange->high.x = pRange->high.y = -FLT_MAX;
    }


/*-----------------------------------------------------------------*//**
*
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFRange2d_initFromDRange2d

(
FRange2dP pRange,
DRange2dP pSource
)

    {
    if (bsiDRange2d_isNull (pSource))
        bsiFRange2d_init (pRange);
    else
        {
        bsiFPoint2d_initFromDPoint2d (&pRange->low, &pSource->low);
        bsiFPoint2d_initFromDPoint2d (&pRange->high, &pSource->high);
        }
    }

#define PANIC_SIZE 1.0e100


/*-----------------------------------------------------------------*//**
*
* Check if the range is exactly the same as the null ranges returned
* by ~mbsiFRange2d_init.  (Note that ranges with other values with low > high
* are not necessarily null by this condition.)
*
* @see bsiFRange2d_isEmpty
* @indexVerb magnitude
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiFRange2d_isNull

(
FRange2dCP pRange
)
    {
    if (
           pRange->low.x >= FLT_MAX
        && pRange->low.y >= FLT_MAX
        && pRange->high.x >= -FLT_MAX
        && pRange->high.y >= -FLT_MAX
        )
        {
        return true;
        }
    else
        return false;
    }

/*-----------------------------------------------------------------*//**
* @return 0 if null range (as decided by isNull), otherwise
*       sum of squared axis extents.
* @indexVerb magnitude
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double    bsiFRange2d_extentSquared

(
FRange2dCP pRange
)


    {
    double  dx, dy;
    if (bsiFRange2d_isNull (pRange))
        return  0.0;

    dx = (pRange->high.x - pRange->low.x);
    dy = (pRange->high.y - pRange->low.y);

    return dx*dx + dy*dy;
    }




/*-----------------------------------------------------------------*//**
*
* Test if low component is (strictly) less than high in any direction.
* Note that equal components do not indicate empty.
*
* returns true if any low component is less than the corresponding high component
* @see
* @indexVerb magnitude
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiFRange2d_isEmpty

(
FRange2dCP pRange
)


    {
    return
            pRange->high.x < pRange->low.x
        ||  pRange->high.y < pRange->low.y
        ;
    }



/*-----------------------------------------------------------------*//**
*
* @return true if high is less than or equal to low in every direction.
* @see
* @indexVerb magnitude
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiFRange2d_isPoint

(
FRange2dCP pRange
)


    {
    return
            pRange->high.x <= pRange->low.x
        &&  pRange->high.y <= pRange->low.y
        ;
    }



/*-----------------------------------------------------------------*//**
* returns product of axis extents.  No test for zero or negative axes.
* @see
* @indexVerb magnitude
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double      bsiFRange2d_volume

(
FRange2dCP pRange
)


    {
    return
            (pRange->high.x - pRange->low.x)
        *   (pRange->high.y - pRange->low.y)
        ;
    }




/*-----------------------------------------------------------------*//**
* Initializes the range to contain the single given point.
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFRange2d_initFromFPoint2d

(
FRange2dP pRange,
FPoint2dCP pPoint
)
    {
    pRange->low = pRange->high = *pPoint;
    }


/*-----------------------------------------------------------------*//**
* Initializes the range to contain the single given point.
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFRange2d_initFromDPoint2d

(
FRange2dP pRange,
DPoint2dCP pPoint
)
    {
    bsiFPoint2d_initFromDPoint2d (&pRange->low, pPoint);
    pRange->high = pRange->low;
    }




/*-----------------------------------------------------------------*//**
* Initializes the range to contain the two given points.
* @param pPoint0 => first point
* @param pPoint1 => second point
* @see
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFRange2d_initFrom2FPoint2d

(
FRange2dP pRange,
FPoint2dCP pPoint0,
FPoint2dCP pPoint1
)
    {
    pRange->low = pRange->high = *pPoint0;

    FIX_MINMAX (pPoint1->x, pRange->low.x, pRange->high.x);
    FIX_MINMAX (pPoint1->y, pRange->low.y, pRange->high.y);
    }


/*-----------------------------------------------------------------*//**
* Initializes the range to contain the two given points.
* @param pPoint0 => first point
* @param pPoint1 => second point
* @see
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFRange2d_initFrom2DPoint2d

(
FRange2dP pRange,
DPoint2dCP pPoint0,
DPoint2dCP pPoint1
)
    {
    bsiFPoint2d_initFromDPoint2d (&pRange->low, pPoint0);
    pRange->high = pRange->low;
    FLT_FIX_MINMAX (pPoint1->x, pRange->low.x, pRange->high.x);
    FLT_FIX_MINMAX (pPoint1->y, pRange->low.y, pRange->high.y);
    }



/*-----------------------------------------------------------------*//**
* Initializes the range to contain two points given as components.
* Minmax logic is applied to the given points.
* @param x0 => first x
* @param y0 => first y
* @param z0 => first z
* @param x1 => second x
* @param y1 => second y
* @param z1 => second z
* @see
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFRange2d_initFrom2Components

(
FRange2dP pRange,
double          x0,
double          y0,
double          z0,
double          x1,
double          y1,
double          z1
)
    {
    pRange->low.x = (float)x0;
    pRange->low.y = (float)y0;
    pRange->high = pRange->low;

    FLT_FIX_MINMAX (x1, pRange->low.x, pRange->high.x);
    FLT_FIX_MINMAX (y1, pRange->low.y, pRange->high.y);
    }


/*-----------------------------------------------------------------*//**
* Initialize the range from a single point given by components
* @param x0 => x coordinate
* @param y0 => y coordinate
* @param z0 => z coordinate
* @see
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFRange2d_initFromComponents

(
FRange2dP pRange,
double          x,
double          y,
double          z
)
    {
    pRange->low.x = pRange->high.x = (float)x;
    pRange->low.y = pRange->high.y = (float)y;
    }


/*-----------------------------------------------------------------*//**
* Initialize the range from given min and max in all directions.
* @param v0 => min (or max)
* @param v1 => max (or min)
* @see
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFRange2d_initFromCubeLimits

(
FRange2dP pRange,
double          v0,
double          v1
)
    {
    double  a, b;

    if  (v0 <= v1)
        {
        a = v0;
        b = v1;
        }
    else
        {
        a = v1;
        b = v0;
        }

    pRange->low.x  = pRange->low.y  = (float)a;
    pRange->high.x = pRange->high.y = (float)b;
    }



/*-----------------------------------------------------------------*//**
* Initialize the range to contain the three given points.
* @param pPoint0 => first point
* @param pPoint1 => second point
* @param pPoint2 => third point
* @param
* @see
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFRange2d_initFrom3DPoint2d

(
FRange2dP pRange,
DPoint2dCP pPoint0,
DPoint2dCP pPoint1,
DPoint2dCP pPoint2
)


    {
    bsiFPoint2d_initFromDPoint2d (&pRange->low, pPoint0);
    pRange->high = pRange->low;

    FLT_FIX_MINMAX (pPoint1->x, pRange->low.x, pRange->high.x);
    FLT_FIX_MINMAX (pPoint1->y, pRange->low.y, pRange->high.y);

    FLT_FIX_MINMAX (pPoint2->x, pRange->low.x, pRange->high.x);
    FLT_FIX_MINMAX (pPoint2->y, pRange->low.y, pRange->high.y);
    }



/*-----------------------------------------------------------------*//**
*
* Initizlizes the range to contain the range of the given array of points.
* If there are no points in the array, the range is initialized by
* ~mbsiFRange2d_init.
*
* @param pPoint => array of points to search
* @param n => number of points in array
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFRange2d_initFromDPoint3dArray

(
FRange2dP pRange,
DPoint3dCP pPoint,
int             n
)
    {
    int i;
    if (n < 1)
        {
        bsiFRange2d_init (pRange);
        }
    else
        {
        bsiFRange2d_initFromComponents (pRange, pPoint[0].x, pPoint[0].y, 0.0);
        for (i=1; i<n; i++)
            {
            FLT_FIX_MINMAX ( pPoint[i].x, pRange->low.x, pRange->high.x);
            FLT_FIX_MINMAX ( pPoint[i].y, pRange->low.y, pRange->high.y);
            }
        }
    }



/*-----------------------------------------------------------------*//**
*
* Initializes the range to contain the range of the given array of 2D points,
* with a single given z value for both the min and max points.
* If there are no points in the array, the range is initialized by
* ~mbsiFRange2d_init.
*
* @param pPoint => array of points to search
* @param n => number of points in array
* @param zVal => default z value
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFRange2d_initFromDPoint2dArray

(
FRange2dP pRange,
DPoint2dCP pPoint,
int              n,
double           zVal
)
    {
    int i;
    if (n < 1)
        {
        bsiFRange2d_init (pRange);
        }
    else
        {
        bsiFRange2d_initFromComponents (pRange, pPoint[0].x, pPoint[0].y, 0.0);
        for (i=1; i<n; i++)
            {
            FLT_FIX_MINMAX ( pPoint[i].x, pRange->low.x, pRange->high.x);
            FLT_FIX_MINMAX ( pPoint[i].y, pRange->low.y, pRange->high.y);
            }
        }
    }


/*-----------------------------------------------------------------*//**
*
* Extend each axis by the given distance on both ends of the range.
*
* @param extend => distance to extend
* @see
* @indexVerb extend
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiFRange2d_extendByDistance

(
FRange2dP pRange,
double           extend
)

    {
    float delta = (float)extend;
    pRange->low.x -= delta;
    pRange->low.y -= delta;

    pRange->high.x += delta;
    pRange->high.y += delta;
    }



/*-----------------------------------------------------------------*//**
*
* Extends the coordinates of the range cube points in pRange so as
* to include the single additional point pPoint.
*
* @param pPoint => new point to be included in the range.
* @see
* @indexVerb extend
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFRange2d_extendByDPoint2d

(
FRange2dP pRange,
DPoint2dCP pPoint
)
    {
    FLT_FIX_MINMAX (pPoint->x, pRange->low.x, pRange->high.x);
    FLT_FIX_MINMAX (pPoint->y, pRange->low.y, pRange->high.y);
    }


/*-----------------------------------------------------------------*//**
*
* extends the coordinates of the range cube points in pRange so as
* to include the single additional point at x,y,z.
*
* @param pRange <=> range to be extended
* @param x => extended range coordinate
* @param y => extended range coordinate
* @param z => extended range coordinate
* @param
* @see
* @indexVerb extend
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFRange2d_extendByComponents

(
FRange2dP pRange,
double      x,
double      y,
double      z
)
    {
    FLT_FIX_MINMAX (x, pRange->low.x, pRange->high.x);
    FLT_FIX_MINMAX (y, pRange->low.y, pRange->high.y);
    }



/*-----------------------------------------------------------------*//**
*
* extends the coordinates of the range cube points in pRange so as
* to include the (normalized image of) the given 4D point.
*
* @param pPoint4d => new point to be included in minmax ranges
* @indexVerb extend
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFRange2d_extendByDPoint4d

(
FRange2dP pRange,
DPoint4dCP pPoint4d
)

    {
    double  divw;
    if (bsiTrig_safeDivide (&divw, 1.0, pPoint4d->w, 0.0))
        {
        double  coord;
        coord = pPoint4d->x * divw;
        FLT_FIX_MINMAX (coord, pRange->low.x, pRange->high.x);
        coord = pPoint4d->y * divw;
        FLT_FIX_MINMAX (coord, pRange->low.y, pRange->high.y);
        }
    }


/*-----------------------------------------------------------------*//**
*
* extends the coordinates of the range cube points in pRange so as
* to include the (normalized image of the) array of DPoint4d
*
* @param pPoint4d => array of  to be included in minmax ranges
* @see
* @indexVerb extend
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFRange2d_extendByDPoint4dArray

(
FRange2dP pRange,
DPoint4dCP pPoint4d,
int             numPoint
)

    {
    int i;
    for (i = 0; i < numPoint; i++)
        {
        bsiFRange2d_extendByDPoint4d (pRange, &pPoint4d[i]);
        }
    }




/*-----------------------------------------------------------------*//**
*
* extends the coordinates of the range cube points in pRange so as
* to include range of an array of points.
*
* @param pArray => new points to be included in minmax ranges
* @param n => number of points
* @see
* @indexVerb extend
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFRange2d_extendByDPoint2dArray

(
FRange2dP pRange,
FPoint2dCP pArray,
int             n
)

    {
    const FPoint2d *pPoint = pArray;
    for (; n-- > 0; pPoint++)
        {
        FLT_FIX_MINMAX (pPoint->x, pRange->low.x, pRange->high.x);
        FLT_FIX_MINMAX (pPoint->y, pRange->low.y, pRange->high.y);
        }
    }



/*-----------------------------------------------------------------*//**
*
* extends the coordinates of the range cube points to
* include the range cube range1P.
*
* @param pRange1 =>  second range
* @see
* @indexVerb extend
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFRange2d_extendByFRange2d

(
FRange2dP pRange0,
FRange2dCP pRange1
)

    {
    if (pRange0 && pRange1)
        {
        FIX_MIN (pRange1->low.x,  pRange0->low.x);
        FIX_MIN (pRange1->low.y,  pRange0->low.y);

        FIX_MAX (pRange1->high.x, pRange0->high.x );
        FIX_MAX (pRange1->high.y, pRange0->high.y );
        }
    }


/*-----------------------------------------------------------------*//**
*
* extends the coordinates of the range cube points to
* include the range cube range1P.
*
* @param pRange1 =>  second range
* @see
* @indexVerb extend
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFRange2d_extendByDRange2d

(
FRange2dP pRange0,
DRange2dCP pRange1
)

    {
    if (pRange0 && pRange1)
        {
        FLT_FIX_MIN (pRange1->low.x,  pRange0->low.x);
        FLT_FIX_MIN (pRange1->low.y,  pRange0->low.y);

        FLT_FIX_MAX (pRange1->high.x, pRange0->high.x );
        FLT_FIX_MAX (pRange1->high.y, pRange0->high.y );
        }
    }





/*-----------------------------------------------------------------*//**
* Compute the intersection of two ranges and test if it is nonempty.
* If empty (non overlap), result range is not set!!!!
*
* @param pRange1 => first range
* @param pRange2 => second range
* @return same result as checkOverlap(pRange1,pRange2).
* @see
* @indexVerb intersection
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       bsiFRange2d_intersect

(
FRange2dP pOutRange,
FRange2dP pRange1,
FRange2dP pRange2
)

    {
    if (bsiFRange2d_checkOverlap (pRange1, pRange2))
        {
        pOutRange->low.x = pRange1->low.x > pRange2->low.x ? pRange1->low.x : pRange2->low.x;
        pOutRange->low.y = pRange1->low.y > pRange2->low.y ? pRange1->low.y : pRange2->low.y;
        pOutRange->high.x = pRange1->high.x < pRange2->high.x ? pRange1->high.x : pRange2->high.x;
        pOutRange->high.y = pRange1->high.y < pRange2->high.y ? pRange1->high.y : pRange2->high.y;

        return true;
        }
    return false;
    }




/*-----------------------------------------------------------------*//**
*
* Form the union of two ranges.
*
* @param pRange1 => first range.
* @param pRange2 => second range.
* @indexVerb union
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiFRange2d_combineRange

(
FRange2dP pCombRange,
FRange2dCP pRange1,
FRange2dCP pRange2
)

    {
    if (pCombRange)
        {
        pCombRange->low.x = pRange1->low.x < pRange2->low.x ? pRange1->low.x : pRange2->low.x;
        pCombRange->low.y = pRange1->low.y < pRange2->low.y ? pRange1->low.y : pRange2->low.y;
        pCombRange->high.x = pRange1->high.x > pRange2->high.x ? pRange1->high.x : pRange2->high.x;
        pCombRange->high.y = pRange1->high.y > pRange2->high.y ? pRange1->high.y : pRange2->high.y;
        }
    }


/*-----------------------------------------------------------------*//**
*
* Test if the given range is a (possible improper) subset of pOuterRange.
*
* @param pOuterRange => outer range
* @return true if the given range is a (possibly improper) subset of
*   pOuterRange.
* @indexVerb containment
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiFRange2d_isContained

(
FRange2dCP pInnerRange,
FRange2dCP pOuterRange
)

    {
    return  pInnerRange->low.x >= pOuterRange->low.x
             && pInnerRange->low.y >= pOuterRange->low.y
             && pInnerRange->high.x <= pOuterRange->high.x
             && pInnerRange->high.y <= pOuterRange->high.y;
    }


/*-----------------------------------------------------------------*//**
*
* Test if the given range is a proper subset of pOuterRange, using only xy parts
*
* @param pOuterRange => outer range
* @return true if the given range is a proper subset of
*   pOuterRange.
* @indexVerb containment
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiFRange2d_isStrictlyContainedXY

(
FRange2dCP pInnerRange,
FRange2dCP pOuterRange
)

    {
    return      pInnerRange->low.x  > pOuterRange->low.x
             && pInnerRange->low.y  > pOuterRange->low.y
             && pInnerRange->high.x < pOuterRange->high.x
             && pInnerRange->high.y < pOuterRange->high.y;
    }


/*-----------------------------------------------------------------*//**
*
* test if any min or max of the given range touches a limit (min or max)
* of a non-trivial direction of pOuterRange.
*
* @param pOuterRange => outer range
* @indexVerb containment
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiFRange2d_touchesEdge

(
FRange2dCP pInnerRange,
FRange2dCP pOuterRange
)

    {
    if ((pOuterRange->low.x < pOuterRange->high.x) &&
        ((pInnerRange->low.x == pOuterRange->low.x) || (pInnerRange->high.x == pOuterRange->high.x)))
        {
        return  true;
        }

    if ((pOuterRange->low.y < pOuterRange->high.y) &&
        ((pInnerRange->low.y == pOuterRange->low.y) || (pInnerRange->high.y == pOuterRange->high.y)))
        {
        return  true;
        }

    return  false;
    }

/*----------------------------------------------------------------------+
| name           jmdlFRange2d_restrictIntervalToMinMax                     |
|                                                                       |
| author EarlinLutz                              7/96                   |
|                                                                       |
+----------------------------------------------------------------------*/
static void    jmdlFRange2d_restrictIntervalToMinMax

(
float           *pStart,
float           *pEnd,
float           start0,
float           end0,
float           min1,
float           max1
)

    {
    if (start0 < min1)
                {
                *pStart = min1;
                }
    else if (start0 > max1)
                {
                *pStart = max1;
                }
    else
                {
                *pStart = start0;
                }

    if (end0 < min1)
                {
                *pEnd = min1;
                }
    else if (end0 > max1)
                {
                *pEnd = max1;
                }
    else
                {
                *pEnd = end0;
                }

    }


/*-----------------------------------------------------------------*//**
*
* Returns a range which is the intersection of two ranges.  The first
* range is treated as a signed range, i.e. decreasing values from low
* to high are a nonempty range, and the output will maintain the
* direction.
* In a direction where there is no overlap, pRange high and low values
* are identical and are at the limit of pRange1 that is nearer to the
* values in pRange0.
* (Intended use: pRange0 is the 'actual' stroking range of a surface
*   i.e. may go 'backwards'.  pRange1 is the nominal full surface range,
*   i.e. is known a priori to be 'forwards'.  The clipping restricts
*   unreliable pRange0 to the nominal surface range pRange1.
* pRange0 and pRange may be the same address.  pMinMax must be different.
*
* @param pRange0 => range to be restricted
* @param pMinMax => allowable minmax range.  Assumed to have low < high
* @see
* @indexVerb intersection
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFRange2d_restrictToMinMax

(
FRange2dP pRange,
FRange2dCP pRange0,
FRange2dCP pMinMax
)

    {
    jmdlFRange2d_restrictIntervalToMinMax
            (
            &pRange->low.x, &pRange->high.x,
            pRange0->low.x, pRange0->high.x,
            pMinMax->low.x, pMinMax->high.x
            );

    jmdlFRange2d_restrictIntervalToMinMax
            (
            &pRange->low.y, &pRange->high.y,
            pRange0->low.y, pRange0->high.y,
            pMinMax->low.y, pMinMax->high.y
            );

    }

/* METHOD(FRange2d,none,scaleAboutCenter)
/*-----------------------------------------------------------------*//**
*
* Scale pRangeIn about its center point
*
* @param pRangeIn => original range
* @param scale => scale factor
* @see
* @indexVerb scale
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            bsiFRange2d_scaleAboutCenter

(
FRange2dP pRange,
FRange2dCP pRangeIn,
double           scale
)

    {
    DRange2d dRange;
    bsiDRange2d_initFromFRange2d (&dRange, pRangeIn);
    bsiDRange2d_scaleAboutCenter (&dRange, &dRange, scale);
    bsiFRange2d_initFromDRange2d (pRange, &dRange);
    }


/*-----------------------------------------------------------------*//**
*
* Extract the 6 bounding planes for a range cube.
*
* @param pOriginArray <= array of plane origins
* @param pNormalArray <= array of plane normals
* @see
* @indexVerb get
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            bsiFRange2d_extractPlanes

(
FRange2dCP pRange,
DPoint2dP pOriginArray,
DPoint2dP pNormalArray
)
    {
    DRange2d dRange;
    bsiDRange2d_initFromFRange2d (&dRange, pRange);
    bsiDRange2d_extractPlanes (&dRange, pOriginArray, pNormalArray);
    }


/*-----------------------------------------------------------------*//**
*
* Return the index of the axis with largest absolute range.
*
* @indexVerb magnitude
* @bsihdr                                                       EarlinLutz      09/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsiFRange2d_indexOfMaximalAxis

(
FRange2dCP pRange
)
    {
    double  a, aMax;
    int    index = 0;
    aMax = fabs (pRange->high.x - pRange->low.x);

    a = fabs (pRange->high.y - pRange->low.y);
    if (a > aMax)
        {
        aMax = a;
        index = 1;
        }

    return index;
    }


/*-----------------------------------------------------------------*//**
*
* Compute the intersection of a range cube and a ray.
*
* If there is not a finite intersection, both params are set to 0 and
* and both points to pPoint0.
*
* @param pParam0 <= ray parameter where cube is entered
* @param pParam1 <= ray parameter where cube is left
* @param pPoint0 <= entry point
* @param pPoint1 <= exit point
* @param pStart => start point of ray
* @param pDirection => direction of ray
* @return true if non-empty intersection.
* @see
* @indexVerb intersect
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiFRange2d_intersectRay

(
FRange2dCP pRange,
double      *pParam0,
double      *pParam1,
DPoint2dP pPoint0,
DPoint2dP pPoint1,
DPoint2dCP pStart,
DPoint2dCP pDirection
)
    {
    DRange2d dRange;
    bsiDRange2d_initFromFRange2d (&dRange, pRange);
    return bsiDRange2d_intersectRay (&dRange,
                                pParam0, pParam1,
                                pPoint0, pPoint1,
                                pStart, pDirection);
    }


/*-----------------------------------------------------------------*//**
*
* @return the largest individual coordinate value among (a) range min point,
* (b) range max point, and (c) range diagonal vector.
* @see
* @indexVerb magnitude
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double  bsiFRange2d_getLargestCoordinate

(
FRange2dCP pRange
)
    {
    double      max;
    DPoint2d diagonal;

    max = fabs(pRange->low.x);
    FIX_MAX(fabs(pRange->high.x), max);
    FIX_MAX(fabs(pRange->low.y), max);
    FIX_MAX(fabs(pRange->high.y), max);

    diagonal.x = pRange->high.x - pRange->low.x;
    diagonal.y = pRange->high.y - pRange->low.y;

    FIX_MAX(fabs(diagonal.x), max);
    FIX_MAX(fabs(diagonal.y), max);

    return max;
    }




/*-----------------------------------------------------------------*//**
*
* Generates a 4-point box around around a range cube.*
* @param pBox <= array of 4 points of the box
* @see
* @indexVerb boxCorners
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFRange2d_box2Points

(
FRange2dCP pRange,
DPoint2dP pBox
)
    {
    DRange2d range;
    bsiDRange2d_initFromFRange2d (&range, pRange);
    bsiDRange2d_box2Points (&range, pBox);
    }



/*-----------------------------------------------------------------*//**
*
* Starting at the beginning of the array, test if points from pPointArray are "in" the range.
*
* @param pPointArray => points
* @param numPoint => number of points
* @param pRange => range cube
* @param mask   => selects faces to consider. Valid values are the constants
*       RangePlane_XMin
* @return number of points that were "in" before the first "out" or end of array.
* @see
* @indexVerb containment
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsiFRange2d_numLeadingPointsInRange

(
FRange2dCP pRange,
DPoint2dCP pPointArray,
int         numPoint,
RangePlaneMask mask
)
    {
    DRange2d range;
    bsiDRange2d_initFromFRange2d (&range, pRange);
    return bsiDRange2d_numLeadingPointsInRange (&range, pPointArray, numPoint, mask);
    }


/*-----------------------------------------------------------------*//**
*
* Compute the intersection of given range with another range and return the
* extentSquared of the intersection range.
*
* @param pRange2 => second range
* @return extentSquared() for the intersection range.
* @indexVerb intersection
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double  bsiFRange2d_getOverlap

(
FRange2dCP pRange1,
FRange2dCP pRange2
)

    {
    FRange2d overlapRange;
    overlapRange.low.x = pRange1->low.x > pRange2->low.x ? pRange1->low.x : pRange2->low.x;
    overlapRange.low.y = pRange1->low.y > pRange2->low.y ? pRange1->low.y : pRange2->low.y;
    overlapRange.high.x = pRange1->high.x < pRange2->high.x ? pRange1->high.x : pRange2->high.x;
    overlapRange.high.y = pRange1->high.y < pRange2->high.y ? pRange1->high.y : pRange2->high.y;

    return  bsiFRange2d_extentSquared (&overlapRange);
    }


/*-----------------------------------------------------------------*//**
*
* Test if two ranges have strictly non-null overlap (intersection)
*
* @param pRange1 => first range
* @param pRange2 => second range
* @return true if ranges overlap, false if not.
* @indexVerb intersection
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiFRange2d_checkOverlap

(
FRange2dCP pRange1,
FRange2dCP pRange2
)
    {
    if (pRange1->low.x > pRange2->high.x
        || pRange1->low.y > pRange2->high.y
        || pRange2->low.x > pRange1->high.x
        || pRange2->low.y > pRange1->high.y
        )
        return  false;
    else
        return  true;
    }


/*-----------------------------------------------------------------*//**
*
* Test if a modification of the given (instance) range would have a different
* touching relationship with pOuterRange.
*
* @remark This may only be meaningful in context of range tree tests where
*   some prior relationship among ranges is known to apply.
*
* @param pNewRange => candidate for modified range relationship.
* @param pOuterRnage => containing range
* @return true if touching condition occurs.
* @indexVerb extend
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiFRange2d_moveChangesRange

(
FRange2dCP pOldRange,
FRange2dCP pNewRange,
FRange2dCP pOuterRange
)

    {
    if (!pNewRange)
        return  bsiFRange2d_touchesEdge (pOldRange, pOuterRange);

    if ((pOldRange->low.x != pNewRange->low.x) && (pOldRange->low.x == pOuterRange->low.x))
        return  true;

    if ((pOldRange->low.y != pNewRange->low.y) && (pOldRange->low.y == pOuterRange->low.y))
        return  true;

    if ((pOldRange->high.x != pNewRange->high.x) && (pOldRange->high.x == pOuterRange->high.x))
        return  true;

    if ((pOldRange->high.y != pNewRange->high.y) && (pOldRange->high.y == pOuterRange->high.y))
        return  true;

    return  false;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
