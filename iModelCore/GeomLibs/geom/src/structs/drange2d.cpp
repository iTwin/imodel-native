/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/drange2d.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------*//**
* @struct DRange2d
* A DRange2d structure holds two points at low and high points of diagonal
*   range rectangles.
* @fields
* @field DPoint2d low lower point of diagonal.
* @field DPoint2d high high pointer of diagonal.
* @endfields
* @bsistruct                                        EarlinLutz      09/00
+----------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

 
#define FIX_MIN(value, min)          if (value < min) min = value
#define FIX_MAX(value, max)          if (value > max) max = value
#define FIX_MINMAX(value, min, max)  FIX_MIN(value, min); FIX_MAX(value, max);

#define MSVECTOR_BOUNDING_PLANES_PER_CUBE 6
#define MSVECTOR_PLANE_CUBE_INTERSECTION_POINTS 12

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


#if !defined (DBL_MAX)
#define DBL_MAX 1.0e200
#endif


/*-----------------------------------------------------------------*//**
* @class DRange2d
* A DRange2d structure contains 2 points for use as min (low) and max (high)
* corners of a rectangle.
*
* For a non-null bounding box, each component (x,y) of the min point is
* strictly less than the corresponding component of the max point.
*
* The base init function ~mbsiDRange2d_init initializes the range to large
* values with max less than min.  This null state can be identified as a
* null box later on.
*
* Methods named init(...) initialize the box around
* various combinations of points.  (E.g. one point, two points, 3 points,
* array of points.)
*
* Methods named extend(..) expand an existing box to include additional
* range. (E.g. one point, array of points, components of point)
*
* @author   EarlinLutz
* @indexVerb
* @bsihdr                                               EarlinLutz      12/97
+===============+===============+===============+===============+======*/







/*-----------------------------------------------------------------*//**
*
* Initializes a range cube with (inverted) large positive and negative
* values.
*
* @param
* @see
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange2d_init

(
DRange2dP pRange        /* <= range to be initialized */
)

    {
    pRange->low.x = pRange->low.y = DBL_MAX;
    pRange->high.x = pRange->high.y = -DBL_MAX;
    }


/*-----------------------------------------------------------------*//**
*
* Initializes a range cube with (inverted) large positive and negative
* values.
*
* @param pFRange <= "float" range.
* @see
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange2d_initFromFRange2d

(
DRange2dP pRange,
FRange2dCP pFRange
)

    {
    if (bsiFRange2d_isNull (pFRange))
        bsiDRange2d_init (pRange);
    else
        {
        bsiDPoint2d_initFromFPoint2d (&pRange->low, &pFRange->low);
        bsiDPoint2d_initFromFPoint2d (&pRange->high, &pFRange->high);
        }
    }

#define PANIC_SIZE 1.0e100

/*-----------------------------------------------------------------*//**
*
* Check if the range is exactly the same as the null ranges returned
* by bsiDRange2d_init.  (Note that ranges with other values with low > high
* are not necessarily null by this condition.)
*
* @see bsiDRange2d_isEmpty
* @indexVerb magnitude
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDRange2d_isNull

(
DRange2dCP pRange
)
    {
    if (
           pRange->low.x == DBL_MAX
        && pRange->low.y == DBL_MAX
        && pRange->high.x == -DBL_MAX
        && pRange->high.y == -DBL_MAX
        )
        {
        return true;
        }
    else if (
           fabs(pRange->low.x) < PANIC_SIZE
        && fabs(pRange->low.y) < PANIC_SIZE
        && fabs(pRange->high.x) < PANIC_SIZE
        && fabs(pRange->high.y) < PANIC_SIZE
        )
        {
        return false;
        }
    else
        {
        /* It doesn't match the definition of a null, but it is definitely not normal.
         This is a good place for a breakpoint!!*/
        return true;
        }
    }



/*-----------------------------------------------------------------*//**
* @return 0 if null range (as decided by ~mbsiDRange2d_isNull), otherwise
*       sum of squared axis extents.
* @see bsiDRange2d_isNull
* @indexVerb magnitude
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   bsiDRange2d_extentSquared

(
DRange2dCP pRange
)


    {
    double dx, dy;
    if (bsiDRange2d_isNull (pRange))
        return  0.0;

    dx = (pRange->high.x - pRange->low.x);
    dy = (pRange->high.y - pRange->low.y);

    return  dx*dx + dy*dy;
    }




/*-----------------------------------------------------------------*//**
*
* Test if high component is (strictly) less than low in any direction.
* Note that equal components do not indicate empty.
*
* returns true if any low component is less than the corresponding high component
* @see
* @indexVerb magnitude
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDRange2d_isEmpty

(
DRange2dCP pRange
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
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDRange2d_isPoint

(
DRange2dCP pRange
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
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double     bsiDRange2d_area

(
DRange2dCP pRange
)


    {
    return
            (pRange->high.x - pRange->low.x)
        *   (pRange->high.y - pRange->low.y)
        ;
    }




/*-----------------------------------------------------------------*//**
*
* Initializes the range to contain the single given point.
*
* @see
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange2d_initFromPoint

(
DRange2dP pRange,
DPoint2dCP pPoint
)


    {
    pRange->low = pRange->high = *pPoint;
    }



/*-----------------------------------------------------------------*//**
* Initializes the range to contain the two given points.
* @param pPoint0 => first point
* @param pPoint1 => second point
* @see
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange2d_initFrom2Points

(
DRange2dP pRange,
DPoint2dCP pPoint0,
DPoint2dCP pPoint1
)


    {
    pRange->low = pRange->high = *pPoint0;

    FIX_MINMAX (pPoint1->x, pRange->low.x, pRange->high.x);
    FIX_MINMAX (pPoint1->y, pRange->low.y, pRange->high.y);
    }


/*-----------------------------------------------------------------*//**
* Initializes the range to contain two points given as components.
* Minmax logic is applied to the given points.
* @param x0 => first x
* @param y0 => first y
* @param x1 => second x
* @param y1 => second y
* @see
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange2d_initFrom2Components

(
DRange2dP pRange,
double          x0,
double          y0,
double          x1,
double          y1
)
    {
    pRange->low.x = x0;
    pRange->low.y = y0;
    pRange->high = pRange->low;

    FIX_MINMAX (x1, pRange->low.x, pRange->high.x);
    FIX_MINMAX (y1, pRange->low.y, pRange->high.y);
    }


/*-----------------------------------------------------------------*//**
* Initialize the range from a single point given by components
* @param x0 => x coordinate
* @param y0 => y coordinate
* @see
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange2d_initFromComponents

(
DRange2dP pRange,
double          x,
double          y
)
    {
    pRange->low.x = pRange->high.x = x;
    pRange->low.y = pRange->high.y = y;

    }

/*-----------------------------------------------------------------*//**
* Initialize the range from given min and max in all directions.
* @param v0 => min (or max) for all directions
* @param v1 => max (or min) for all directions
* @see
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange2d_initFromCubeLimits

(
DRange2dP pRange,
double          v0,
double          v1
)
    {
    double a, b;

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

    pRange->low.x  = a;
    pRange->low.y  = a;

    pRange->high.x = b;
    pRange->high.y = b;
    }


/*-----------------------------------------------------------------*//**
* Initialize the range from an arc of the unit circle
* @param theta0 => start angle
* @param sweep  => angular sweep
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange2d_initFromUnitArcSweep

(
DRange2dP pRange,
double          theta0,
double          sweep
)
    {
    double theta1 = theta0 + sweep;
    if (bsiTrig_isAngleFullCircle (sweep))
        {
        bsiDRange2d_initFrom2Components (pRange, -1.0, -1.0, 1.0, 1.0);
        }
    else
        {
        DPoint2d testPoint;

        testPoint.x = cos (theta0);
        testPoint.y = sin (theta0);
        bsiDRange2d_initFromPoint (pRange, &testPoint);
        bsiDRange2d_extendByComponents (pRange, cos (theta1), sin (theta1));

        /* Force the range out to the axis extremes if they are in the sweep */
        if (bsiTrig_angleInSweep (0.0,              theta0, sweep))
            {
            pRange->high.x = 1.0;
            }

        if (bsiTrig_angleInSweep (msGeomConst_pi,  theta0, sweep))
            {
            pRange->low.x = -1.0;
            }

        if (bsiTrig_angleInSweep (msGeomConst_piOver2, theta0, sweep))
            {
            pRange->high.y = 1.0;
            }

        if (bsiTrig_angleInSweep (-msGeomConst_piOver2, theta0, sweep))
            {
            pRange->low.y = -1.0;
            }
        }

    }


/*-----------------------------------------------------------------*//**
* Initialize the range to contain the three given points.
* @param pPoint0 => first point
* @param pPoint1 => second point
* @param pPoint2 => third point
* @param
* @see
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange2d_initFrom3DPoint2d

(
DRange2dP pRange,
DPoint2dCP pPoint0,
DPoint2dCP pPoint1,
DPoint2dCP pPoint2
)


    {
    pRange->low = pRange->high = *pPoint0;

    FIX_MINMAX (pPoint1->x, pRange->low.x, pRange->high.x);
    FIX_MINMAX (pPoint1->y, pRange->low.y, pRange->high.y);

    FIX_MINMAX (pPoint2->x, pRange->low.x, pRange->high.x);
    FIX_MINMAX (pPoint2->y, pRange->low.y, pRange->high.y);
    }



/*-----------------------------------------------------------------*//**
*
* Initizlizes the range to contain the range of the given array of points.
* If there are no points in the array, the range is initialized by
* DRange2d.init()
*
* @param pPoint => array of points to search
* @param n => number of points in array
* @see
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange2d_initFromArray

(
DRange2dP pRange,
DPoint2dCP pPoint,
int             n
)

    {
    int i;
    DPoint2d *  minP = &pRange->low;
    DPoint2d *  maxP = &pRange->high;
    if (n < 1)
        {
        bsiDRange2d_init (pRange);
        }
    else
        {
        *minP = *maxP = pPoint[0];
        for (i=1; i<n; i++)
            {
            FIX_MINMAX ( pPoint[i].x, minP->x, maxP->x );
            FIX_MINMAX ( pPoint[i].y, minP->y, maxP->y );
            }
        }
    }



/*-----------------------------------------------------------------*//**
*
* Initializes the range to contain the range of the xy parts of
* the array of 3D points.
*
* @param pPoint => array of points to search
* @param n => number of points in array
* @see
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange2d_initFromArray3d

(
DRange2dP pRange,
DPoint3dCP pPoint,
int              n
)

    {
    int i;
    DPoint2d *  minP = &pRange->low;
    DPoint2d *  maxP = &pRange->high;

    if (n < 1)
        {
        bsiDRange2d_init (pRange);
        }
    else
        {
        maxP->x = pPoint[0].x;
        maxP->y = pPoint[0].y;
        *minP = *maxP;
        for (i=1; i<n; i++)
            {
            FIX_MINMAX ( pPoint[i].x, minP->x, maxP->x );
            FIX_MINMAX ( pPoint[i].y, minP->y, maxP->y );
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
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDRange2d_extendByDistance

(
DRange2dP pRange,
double           extend
)

    {
    pRange->low.x -= extend;
    pRange->low.y -= extend;

    pRange->high.x += extend;
    pRange->high.y += extend;
    }



/*-----------------------------------------------------------------*//**
*
* Extends the coordinates of the range cube points in pRange so as
* to include the single additional point pPoint.
*
* @param pPoint => new point to be included in the range.
* @see
* @indexVerb extend
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange2d_extendByDPoint3d

(
DRange2dP pRange,
DPoint2dCP pPoint
)
    {
    FIX_MINMAX (pPoint->x, pRange->low.x, pRange->high.x);
    FIX_MINMAX (pPoint->y, pRange->low.y, pRange->high.y);
    }


/*-----------------------------------------------------------------*//**
*
* extends the coordinates of the range cube points in pRange so as
* to include the single additional point at x,y.
*
* @param pRange <=> range to be extended
* @param x => extended range coordinate
* @param y => extended range coordinate
* @param
* @see
* @indexVerb extend
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange2d_extendByComponents

(
DRange2dP pRange,
double      x,
double      y
)
    {
    FIX_MINMAX (x, pRange->low.x, pRange->high.x);
    FIX_MINMAX (y, pRange->low.y, pRange->high.y);
    }


/*-----------------------------------------------------------------*//**
*
* extends the coordinates of the range cube points in pRange so as
* to include the (normalized image of) the xy projection of the 4D point.
*
* @param pPoint4d => new point to be included in minmax ranges
* @see
* @indexVerb extend
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange2d_extendByDPoint4d

(
DRange2dP pRange,
DPoint4dCP pPoint4d
)

    {
    if (pPoint4d->w != 0.0)
        {
        double coord;
        coord = pPoint4d->x / pPoint4d->w;
        FIX_MINMAX (coord, pRange->low.x, pRange->high.x);
        coord = pPoint4d->y / pPoint4d->w;
        FIX_MINMAX (coord, pRange->low.y, pRange->high.y);
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
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange2d_extendByDPoint2dArray

(
DRange2dP pRange,
DPoint2dCP pArray,
int             n
)

    {
    const DPoint2d *pPoint = pArray;
    for (; n-- > 0; pPoint++)
        {
        FIX_MINMAX (pPoint->x, pRange->low.x, pRange->high.x);
        FIX_MINMAX (pPoint->y, pRange->low.y, pRange->high.y);
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
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange2d_extendByRange

(
DRange2dP pRange0,
DRange2dCP pRange1
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
* Compute the intersection of two ranges and test if it is nonempty.
* If empty (non overlap), result range is not set!!!!
*
* @param pRange1 => first range
* @param pRange2 => second range
* @return same result as checkOverlap(pRange1,pRange2).
* @see
* @indexVerb intersect
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       bsiDRange2d_intersect

(
DRange2dP pOutRange,
DRange2dP pRange1,
DRange2dP pRange2
)

    {
    if (bsiDRange2d_checkOverlap (pRange1, pRange2))
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
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDRange2d_combineRange

(
DRange2dP pCombRange,
DRange2dCP pRange1,
DRange2dCP pRange2
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
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDRange2d_isContained

(
DRange2dCP pInnerRange,
DRange2dCP pOuterRange
)

    {
    return      pInnerRange->low.x >= pOuterRange->low.x
             && pInnerRange->low.y >= pOuterRange->low.y
             && pInnerRange->high.x <= pOuterRange->high.x
             && pInnerRange->high.y <= pOuterRange->high.y;
    }


/*-----------------------------------------------------------------*//**
*
* test if any min or max of the given range touches a limit (min or max)
* of a non-trivial direction of pOuterRange.
*
* @param pOuterRange => outer range
* @indexVerb containment
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDRange2d_touchesEdge

(
DRange2dCP pInnerRange,
DRange2dCP pOuterRange
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
| name           jmdlDRange2d_restrictIntervalToMinMax                     |
|                                                                       |
| author EarlinLutz                              7/96                   |
|                                                                       |
+----------------------------------------------------------------------*/
static void    jmdlDRange2d_restrictIntervalToMinMax

(
double          *pStart,
double          *pEnd,
double          start0,
double          end0,
double          min1,
double          max1
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
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange2d_restrictToMinMax

(
DRange2dP pRange,
DRange2dCP pRange0,
DRange2dCP pMinMax
)

    {
    jmdlDRange2d_restrictIntervalToMinMax
            (
            &pRange->low.x, &pRange->high.x,
            pRange0->low.x, pRange0->high.x,
            pMinMax->low.x, pMinMax->high.x
            );

    jmdlDRange2d_restrictIntervalToMinMax
            (
            &pRange->low.y, &pRange->high.y,
            pRange0->low.y, pRange0->high.y,
            pMinMax->low.y, pMinMax->high.y
            );

    }

/* METHOD(DRange2d,none,scaleAboutCenter)
/*-----------------------------------------------------------------*//**
*
* Scale pRangeIn about its center point
*
* @param pRangeIn => original range
* @param scale => scale factor
* @see
* @indexVerb scale
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            bsiDRange2d_scaleAboutCenter

(
DRange2dP pRange,
DRange2dCP pRangeIn,
double          scale
)

    {
    DPoint2d low;
    DPoint2d diagonal;
    double   eps = scale - 1.0;

    low = pRangeIn->low;
    diagonal.DifferenceOf (pRangeIn->high, pRangeIn->low);
    pRange->high.SumOf (low, diagonal, 1.0 + eps);
    pRange->low.SumOf (low, diagonal, -eps);
    }


/*-----------------------------------------------------------------*//**
*
* Extract the 4 bounding lines for a range rectangle, in origin normal form
*
* @param pOriginArray <= array of line origins
* @param pNormalArray <= array of plane normals. Directions down, left, right, up.
* @see
* @indexVerb get
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            bsiDRange2d_extractPlanes

(
DRange2dCP pRange,
DPoint2dP pOriginArray,
DPoint2dP pNormalArray
)
    {
    DRange2d minMaxRange;

    bsiDRange2d_initFrom2Points (&minMaxRange, &pRange->low, &pRange->high);
    pOriginArray[0] = pOriginArray[1] =  minMaxRange.low;
    pOriginArray[3] = pOriginArray[4] =  minMaxRange.high;

    pNormalArray[0].Init(-1.0, 0.0);
    pNormalArray[3].Init(1.0, 0.0);

    pNormalArray[1].Init(0.0, -1.0);
    pNormalArray[4].Init(0.0, 1.0);

    }


/*-----------------------------------------------------------------*//**
*
* Return the index of the axis with largest absolute range.
*
* @indexVerb extrema
* @bsihdr                                                       EarlinLutz      09/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsiDRange2d_indexOfMaximalAxis

(
DRange2dCP pRange
)
    {
    double a, aMax;
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
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDRange2d_intersectRay

(
DRange2dCP pRange,
double      *pParam0,
double      *pParam1,
DPoint2dP pPoint0,
DPoint2dP pPoint1,
DPoint2dCP pStart,
DPoint2dCP pDirection
)
    {
    double s0, s1;      /* parameters of 'in' segment */
    int     contents = -1;
    bool    boolStat;
    /* save points in case of duplicate pointers by caller */
    DPoint2d start;
    DPoint2d direction;

    start = *pStart;
    direction = *pDirection;

    bsiRange1d_intersectLine
                (
                &s0, &s1, &contents,
                pStart->x, pDirection->x,
                pRange->low.x, pRange->high.x
                );
    bsiRange1d_intersectLine
                (
                &s0, &s1, &contents,
                pStart->y, pDirection->y,
                pRange->low.y, pRange->high.y
                );

    if (contents > 0)
        {
        boolStat = true;
        }
    else
        {
        s0 = 0.0;
        s1 = 0.0;
        boolStat = false;
        }

    /* Copy to outputs (all optional) */
    if (pParam0)
        *pParam0 = s0;

    if (pParam1)
        *pParam1 = s1;

    if (pPoint0)
        pPoint0->SumOf (start, direction, s0);

    if (pPoint1)
        pPoint1->SumOf (start, direction, s1);

    return  boolStat;
    }


/*-----------------------------------------------------------------*//**
*
* @return the largest individual coordinate value among (a) range min point,
* (b) range max point, and (c) range diagonal vector.
* @see
* @indexVerb extrema
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDRange2d_getLargestCoordinate

(
DRange2dCP pRange
)

    {
     double     max;
     DPoint2d diagonal;

     max = fabs(pRange->low.x);
     FIX_MAX(fabs(pRange->high.x), max);
     FIX_MAX(fabs(pRange->low.y), max);
     FIX_MAX(fabs(pRange->high.y), max);

     diagonal.DifferenceOf (pRange->high, pRange->low);

     FIX_MAX(fabs(diagonal.x), max);
     FIX_MAX(fabs(diagonal.y), max);

     return max;

    }




/*-----------------------------------------------------------------*//**
*
* Generates an 8point box around around a range cube.  Point ordering is by
* "x varies fastest" --- 00, 10, 01, 11 for the unit range.
* @param pBox <= array of 4 points of the box
* @see
* @indexVerb boxCorners
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange2d_box2Points

(
DRange2dCP pRange,
DPoint2dP pBox
)
    {
    pBox[0] = pBox[1] = pRange->low;
    pBox[2] = pBox[3] = pRange->high;
    pBox[1].x = pBox[3].x;
    pBox[2].x = pBox[0].x;
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
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsiDRange2d_numLeadingPointsInRange

(
DRange2dCP pRange,
DPoint2dCP pPointArray,
int         numPoint,
RangePlaneMask mask
)
    {
    int nMax = numPoint;
    int i;
    double a;

    if (mask & RangePlane_XMin)
        {
        a = pRange->low.x;
        for (i = 0; i < nMax && pPointArray[i].x >= a; i++)
            {}
        nMax = i;
        }
    if (nMax == 0)
        return 0;


    if (mask & RangePlane_XMax)
        {
        a = pRange->high.x;
        for (i = 0; i < nMax && pPointArray[i].x <= a; i++)
            {}
        nMax = i;
        }

    if (nMax == 0)
        return 0;




    if (mask & RangePlane_YMin)
        {
        a = pRange->low.y;
        for (i = 0; i < nMax && pPointArray[i].y >= a; i++)
            {}
        nMax = i;
        }
    if (nMax == 0)
        return 0;


    if (mask & RangePlane_YMax)
        {
        a = pRange->high.y;
        for (i = 0; i < nMax && pPointArray[i].y <= a; i++)
            {}
        nMax = i;
        }

    if (nMax == 0)
        return 0;

    return nMax;
    }



/*-----------------------------------------------------------------*//**
*
* Compute the intersection of given range with another range and return the
* extentSquared of the intersection range.
*
* @param pRange2 => second range
* @return extentSquared() for the intersection range.
* @indexVerb intersection
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDRange2d_getOverlap

(
DRange2dCP pRange1,
DRange2dCP pRange2
)

    {
    DRange2d overlapRange;
    overlapRange.low.x = pRange1->low.x > pRange2->low.x ? pRange1->low.x : pRange2->low.x;
    overlapRange.low.y = pRange1->low.y > pRange2->low.y ? pRange1->low.y : pRange2->low.y;
    overlapRange.high.x = pRange1->high.x < pRange2->high.x ? pRange1->high.x : pRange2->high.x;
    overlapRange.high.y = pRange1->high.y < pRange2->high.y ? pRange1->high.y : pRange2->high.y;

    return  bsiDRange2d_extentSquared (&overlapRange);
    }




/*-----------------------------------------------------------------*//**
*
* Test if two ranges have strictly non-null overlap (intersection)
*
* @param pRange1 => first range
* @param pRange2 => second range
* @return true if ranges overlap, false if not.
* @indexVerb intersectioin
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDRange2d_checkOverlap

(
DRange2dCP pRange1,
DRange2dCP pRange2
)

    {
    if (pRange1->low.x > pRange2->high.x
        || pRange1->low.y > pRange2->high.y
        || pRange2->low.x > pRange1->high.x
        || pRange2->low.y > pRange1->high.y)
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
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDRange2d_moveChangesRange

(
DRange2dCP pOldRange,
DRange2dCP pNewRange,
DRange2dCP pOuterRange
)

    {
    if (!pNewRange)
        return  bsiDRange2d_touchesEdge (pOldRange, pOuterRange);

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


/*-----------------------------------------------------------------*//**
 @description Test if a point is contained in a range.
 @param pRange IN candidate containing range.
 @param pPoint IN point to test. (z is ignored)
 @return true if the point is in (or on boundary of)
 @group "DRange2d Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDRange2d_isDPoint3dContained

(
DRange2dCP pRange,
DPoint3dCP pPoint
)

    {
    return  (   pPoint->x >= pRange->low.x
             && pPoint->y >= pRange->low.y
             && pPoint->x <= pRange->high.x
             && pPoint->y <= pRange->high.y
             );
    }


/*-----------------------------------------------------------------*//**
 @description Test if a point is contained in a range.
 @param pRange IN candidate containing range.
 @param pPoint IN point to test. (z is ignored)
 @return true if the point is in (or on boundary of)
 @group "DRange2d Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDRange2d_isDPoint2dContained

(
DRange2dCP pRange,
DPoint2dCP pPoint
)

    {
    return  (   pPoint->x >= pRange->low.x
             && pPoint->y >= pRange->low.y
             && pPoint->x <= pRange->high.x
             && pPoint->y <= pRange->high.y
             );
    }


/*-----------------------------------------------------------------*//**
 @description Test if a point given as x,y,z is contained in a range.
 @param pRange IN candidate containing range.
 @param x IN x coordinate
 @param y IN y coordinate
 @param z IN z coordinate
 @return true if the point is in (or on boundary of)
 @group "DRange2d Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDRange2d_isXYContained

(
DRange2dCP pRange,
double    x,
double    y
)

    {
    return  (   x >= pRange->low.x
             && y >= pRange->low.y
             && x <= pRange->high.x
             && y <= pRange->high.y
            );
    }


/*-----------------------------------------------------------------*//**
 @description Test if two ranges are exactly equal.
 @param pRange0 IN first range
 @param pRange1 IN second range
 @param tolerance IN toleranc to be applied to each component
 @return true if ranges are identical in all components.
 @group "DRange2d Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDRange2d_isEqual

(
DRange2dCP pRange0,
DRange2dCP pRange1
)

    {
    return
           pRange0->low.x  == pRange1->low.x
        && pRange0->low.y  == pRange1->low.y
        && pRange0->high.x == pRange1->high.x
        && pRange0->high.y == pRange1->high.y
        ;
    }


/*-----------------------------------------------------------------*//**
 @description Test if two ranges are equal within a tolerance applied componentwise.
 @param pRange0 IN first range
 @param pRange1 IN second range
 @param tolerance IN toleranc to be applied to each component
 @return true if ranges are within tolerance in all components.
 @group "DRange2d Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDRange2d_isEqualTolerance

(
DRange2dCP pRange0,
DRange2dCP pRange1,
double tolerance
)
    {
    return
           fabs (pRange0->low.x  - pRange1->low.x ) <= tolerance
        && fabs (pRange0->low.y  - pRange1->low.y ) <= tolerance
        && fabs (pRange0->high.x - pRange1->high.x) <= tolerance
        && fabs (pRange0->high.y - pRange1->high.y) <= tolerance
        ;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
