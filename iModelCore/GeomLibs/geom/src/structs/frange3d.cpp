/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/frange3d.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


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

#if !defined (FLT_MAX)
#define FLT_MAX 1.0e200
#endif

#ifdef CompileDRange3dPointerMethods
// BEGIN_NONEDITABLE_CODE
//<cppIMethodImpl>
/*-----------------------------------------------------------------*//**
*
* Initializes a range cube with (inverted) large positive and negative
* values.
*
* @param [out] pFRange "float" range.
* @bsimethod                                                    EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
void DRange3d::initFrom
(
        /* <= range to be initialized */
FRange3dCP pFRange

)
    {
    bsiDRange3d_initFromFRange3d (this, pFRange);
    }
//</cppIMethodImpl>
// END_NONEDITABLE_CODE
#endif




/*-----------------------------------------------------------------*//**
 @description Copy from float to double range.
 @instance pRange OUT initialized range.
 @param pFRange <= "float" range.
 @group "DRange3d Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange3d_initFromFRange3d

(
DRange3dP pRange,        /* <= range to be initialized */
FRange3dCP pFRange
)

    {
    if (bsiFRange3d_isNull (pFRange))
        bsiDRange3d_init (pRange);
    else
        {
        bsiDPoint3d_initFromFPoint3d (&pRange->low, &pFRange->low);
        bsiDPoint3d_initFromFPoint3d (&pRange->high, &pFRange->high);
        }
    }

/*-----------------------------------------------------------------*//**
* @class FRange3d
* A FRange3d structure contains 2 points for use as min (low) and max (high)
* corners of a bounding box.
*
* For a non-null bounding box, each component (x,y,z) of the min point is
* strictly less than the corresponding component of the max point.

*
* The base init function ~mbsiFRange3d_init initializes the range to large
* values with max less than min.  This null state can be identified as a
* null box later on.
*
* Methods named bsiFRange3d_initFromXXX initialize the box around
* various combinations of points.  (E.g. one point, two points, 3 points,
* array of points.)
*
* Methods named bsiFRange3d_extendXXX expand an existing box to include additional
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
* @param
* @see
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFRange3d_init

(
FRange3dP pRange
)

    {
    pRange->low.x = pRange->low.y = pRange->low.z =  FLT_MAX;
    pRange->high.x = pRange->high.y = pRange->high.z = -FLT_MAX;
    }


/*-----------------------------------------------------------------*//**
*
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFRange3d_initFromDRange3d

(
FRange3dP pRange,
DRange3dP pSource
)

    {
    if (bsiDRange3d_isNull (pSource))
        bsiFRange3d_init (pRange);
    else
        {
        bsiFPoint3d_initFromDPoint3d (&pRange->low, &pSource->low);
        bsiFPoint3d_initFromDPoint3d (&pRange->high, &pSource->high);
        }
    }

#define PANIC_SIZE 1.0e100


/*-----------------------------------------------------------------*//**
*
* Check if the range is exactly the same as the null ranges returned
* by ~mbsiFRange3d_init.  (Note that ranges with other values with low > high
* are not necessarily null by this condition.)
*
* @see bsiFRange3d_isEmpty
* @indexVerb magnitude
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiFRange3d_isNull

(
FRange3dCP pRange
)
    {
    if (
           pRange->low.x >= FLT_MAX
        && pRange->low.y >= FLT_MAX
        && pRange->low.z >=  FLT_MAX
        && pRange->high.x >= -FLT_MAX
        && pRange->high.y >= -FLT_MAX
        && pRange->high.z >= -FLT_MAX
        )
        {
        return true;
        }
    else if (
           fabs(pRange->low.x) < PANIC_SIZE
        && fabs(pRange->low.y) < PANIC_SIZE
        && fabs(pRange->low.z) < PANIC_SIZE
        && fabs(pRange->high.x) < PANIC_SIZE
        && fabs(pRange->high.y) < PANIC_SIZE
        && fabs(pRange->high.z) < PANIC_SIZE
        )
        {
        return false;
        }
    else
        {
        /* It doesn't match the definition of a null, but it is definitely not normal.
         This is a good place for a breakpoint!!*/
//      throwException ("BadRange");
        return true;
        }
    }


/*-----------------------------------------------------------------*//**
* @return 0 if null range (as decided by ~mbsiFRange3d_isNull), otherwise
*       sum of squared axis extents.
* @indexVerb magnitude
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double    bsiFRange3d_extentSquared

(
FRange3dCP pRange
)


    {
    double  dx, dy, dz;
    if (bsiFRange3d_isNull (pRange))
        return  0.0;

    dx = (pRange->high.x - pRange->low.x);
    dy = (pRange->high.y - pRange->low.y);
    dz = (pRange->high.z - pRange->low.z);

    return  ((dx*dx) + (dy*dy) + (dz*dz));
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
Public GEOMDLLIMPEXP bool        bsiFRange3d_isEmpty

(
FRange3dCP pRange
)


    {
    return
            pRange->high.x < pRange->low.x
        ||  pRange->high.y < pRange->low.y
        ||  pRange->high.z < pRange->low.z
        ;
    }



/*-----------------------------------------------------------------*//**
*
* @return true if high is less than or equal to low in every direction.
* @see
* @indexVerb magnitude
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiFRange3d_isPoint

(
FRange3dCP pRange
)


    {
    return
            pRange->high.x <= pRange->low.x
        &&  pRange->high.y <= pRange->low.y
        &&  pRange->high.z <= pRange->low.z
        ;
    }



/*-----------------------------------------------------------------*//**
* returns product of axis extents.  No test for zero or negative axes.
* @see
* @indexVerb magnitude
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double      bsiFRange3d_volume

(
FRange3dCP pRange
)


    {
    return
            (pRange->high.x - pRange->low.x)
        *   (pRange->high.y - pRange->low.y)
        *   (pRange->high.z - pRange->low.z)
        ;
    }




/*-----------------------------------------------------------------*//**
* Initializes the range to contain the single given point.
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFRange3d_initFromFPoint3d

(
FRange3dP pRange,
FPoint3dCP pPoint
)
    {
    pRange->low = pRange->high = *pPoint;
    }


/*-----------------------------------------------------------------*//**
* Initializes the range to contain the single given point.
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFRange3d_initFromDPoint3d

(
FRange3dP pRange,
DPoint3dCP pPoint
)
    {
    bsiFPoint3d_initFromDPoint3d (&pRange->low, pPoint);
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
Public GEOMDLLIMPEXP void     bsiFRange3d_initFrom2FPoint3d

(
FRange3dP pRange,
FPoint3dCP pPoint0,
FPoint3dCP pPoint1
)
    {
    pRange->low = pRange->high = *pPoint0;

    FIX_MINMAX (pPoint1->x, pRange->low.x, pRange->high.x);
    FIX_MINMAX (pPoint1->y, pRange->low.y, pRange->high.y);
    FIX_MINMAX (pPoint1->z, pRange->low.z, pRange->high.z);
    }


/*-----------------------------------------------------------------*//**
* Initializes the range to contain the two given points.
* @param pPoint0 => first point
* @param pPoint1 => second point
* @see
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFRange3d_initFrom2DPoint3d

(
FRange3dP pRange,
DPoint3dCP pPoint0,
DPoint3dCP pPoint1
)
    {
    bsiFPoint3d_initFromDPoint3d (&pRange->low, pPoint0);
    pRange->high = pRange->low;
    FLT_FIX_MINMAX (pPoint1->x, pRange->low.x, pRange->high.x);
    FLT_FIX_MINMAX (pPoint1->y, pRange->low.y, pRange->high.y);
    FLT_FIX_MINMAX (pPoint1->z, pRange->low.z, pRange->high.z);
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
Public GEOMDLLIMPEXP void     bsiFRange3d_initFrom2Components

(
FRange3dP pRange,
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
    pRange->low.z = (float)z0;
    pRange->high = pRange->low;

    FLT_FIX_MINMAX (x1, pRange->low.x, pRange->high.x);
    FLT_FIX_MINMAX (y1, pRange->low.y, pRange->high.y);
    FLT_FIX_MINMAX (z1, pRange->low.z, pRange->high.z);
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
Public GEOMDLLIMPEXP void     bsiFRange3d_initFromComponents

(
FRange3dP pRange,
double          x,
double          y,
double          z
)
    {
    pRange->low.x = pRange->high.x = (float)x;
    pRange->low.y = pRange->high.y = (float)y;
    pRange->low.z = pRange->high.z = (float)z;
    }


/*-----------------------------------------------------------------*//**
* Initialize the range from given min and max in all directions.
* @param v0 => min (or max)
* @param v1 => max (or min)
* @see
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFRange3d_initFromCubeLimits

(
FRange3dP pRange,
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

    pRange->low.x  = pRange->low.y  = pRange->low.z  = (float)a;
    pRange->high.x = pRange->high.y = pRange->high.z = (float)b;
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
Public GEOMDLLIMPEXP void     bsiFRange3d_initFrom3DPoint3d

(
FRange3dP pRange,
DPoint3dCP pPoint0,
DPoint3dCP pPoint1,
DPoint3dCP pPoint2
)


    {
    bsiFPoint3d_initFromDPoint3d (&pRange->low, pPoint0);
    pRange->high = pRange->low;

    FLT_FIX_MINMAX (pPoint1->x, pRange->low.x, pRange->high.x);
    FLT_FIX_MINMAX (pPoint1->y, pRange->low.y, pRange->high.y);
    FLT_FIX_MINMAX (pPoint1->z, pRange->low.z, pRange->high.z);

    FLT_FIX_MINMAX (pPoint2->x, pRange->low.x, pRange->high.x);
    FLT_FIX_MINMAX (pPoint2->y, pRange->low.y, pRange->high.y);
    FLT_FIX_MINMAX (pPoint2->z, pRange->low.z, pRange->high.z);
    }



/*-----------------------------------------------------------------*//**
*
* Initizlizes the range to contain the range of the given array of points.
* If there are no points in the array, the range is initialized by
* ~mbsiFRange3d_init.
*
* @param pPoint => array of points to search
* @param n => number of points in array
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFRange3d_initFromDPoint3dArray

(
FRange3dP pRange,
DPoint3dCP pPoint,
int             n
)
    {
    int i;
    if (n < 1)
        {
        bsiFRange3d_init (pRange);
        }
    else
        {
        bsiFRange3d_initFromDPoint3d (pRange, &pPoint[0]);
        for (i=1; i<n; i++)
            {
            FLT_FIX_MINMAX ( pPoint[i].x, pRange->low.x, pRange->high.x);
            FLT_FIX_MINMAX ( pPoint[i].y, pRange->low.y, pRange->high.y);
            FLT_FIX_MINMAX ( pPoint[i].z, pRange->low.z, pRange->high.z);
            }
        }
    }


/*-----------------------------------------------------------------*//**
*
* Initizlizes the range to contain the range of the given array of points.
* If there are no points in the array, the range is initialized by
* ~mbsiFRange3d_init.
*
* @param pPoint => array of points to search
* @param n => number of points in array
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFRange3d_initFromFPoint3dArray

(
FRange3dP pRange,
FPoint3dCP pPoint,
int             n
)
    {
    int i;
    if (n < 1)
        {
        bsiFRange3d_init (pRange);
        }
    else
        {
        bsiFRange3d_initFromFPoint3d (pRange, &pPoint[0]);
        for (i=1; i<n; i++)
            {
            FLT_FIX_MINMAX ( pPoint[i].x, pRange->low.x, pRange->high.x);
            FLT_FIX_MINMAX ( pPoint[i].y, pRange->low.y, pRange->high.y);
            FLT_FIX_MINMAX ( pPoint[i].z, pRange->low.z, pRange->high.z);
            }
        }
    }


/*-----------------------------------------------------------------*//**
*
* Initializes the range to contain the range of the given array of 2D points,
* with a single given z value for both the min and max points.
* If there are no points in the array, the range is initialized by
* ~mbsiFRange3d_init.
*
* @param pPoint => array of points to search
* @param n => number of points in array
* @param zVal => default z value
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFRange3d_initFromDPoint2dArray

(
FRange3dP pRange,
DPoint2dCP pPoint,
int              n,
double           zVal
)
    {
    int i;
    if (n < 1)
        {
        bsiFRange3d_init (pRange);
        }
    else
        {
        bsiFRange3d_initFromComponents (pRange, pPoint[0].x, pPoint[0].y, 0.0);
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
Public GEOMDLLIMPEXP void bsiFRange3d_extendByDistance

(
FRange3dP pRange,
double           extend
)

    {
    float delta = (float)extend;
    pRange->low.x -= delta;
    pRange->low.y -= delta;
    pRange->low.z -= delta;

    pRange->high.x += delta;
    pRange->high.y += delta;
    pRange->high.z += delta;
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
Public GEOMDLLIMPEXP void     bsiFRange3d_extendByDPoint3d

(
FRange3dP pRange,
DPoint3dCP pPoint
)
    {
    FLT_FIX_MINMAX (pPoint->x, pRange->low.x, pRange->high.x);
    FLT_FIX_MINMAX (pPoint->y, pRange->low.y, pRange->high.y);
    FLT_FIX_MINMAX (pPoint->z, pRange->low.z, pRange->high.z);
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
Public GEOMDLLIMPEXP void     bsiFRange3d_extendByComponents

(
FRange3dP pRange,
double      x,
double      y,
double      z
)
    {
    FLT_FIX_MINMAX (x, pRange->low.x, pRange->high.x);
    FLT_FIX_MINMAX (y, pRange->low.y, pRange->high.y);
    FLT_FIX_MINMAX (z, pRange->low.z, pRange->high.z);
    }



/*-----------------------------------------------------------------*//**
*
* extends the coordinates of the range cube points in pRange so as
* to include the (normalized image of) the given 4D point.
*
* @param pPoint4d => new point to be included in minmax ranges
* @see
* @indexVerb extend
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFRange3d_extendByDPoint4d

(
FRange3dP pRange,
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
        coord = pPoint4d->z * divw;
        FLT_FIX_MINMAX (coord, pRange->low.z, pRange->high.z);
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
Public GEOMDLLIMPEXP void     bsiFRange3d_extendByDPoint4dArray

(
FRange3dP pRange,
DPoint4dCP pPoint4d,
int             numPoint
)

    {
    int i;
    for (i = 0; i < numPoint; i++)
        {
        bsiFRange3d_extendByDPoint4d (pRange, &pPoint4d[i]);
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
Public GEOMDLLIMPEXP void     bsiFRange3d_extendByDPoint3dArray

(
FRange3dP pRange,
FPoint3dCP pArray,
int             n
)

    {
    const FPoint3d *pPoint = pArray;
    for (; n-- > 0; pPoint++)
        {
        FLT_FIX_MINMAX (pPoint->x, pRange->low.x, pRange->high.x);
        FLT_FIX_MINMAX (pPoint->y, pRange->low.y, pRange->high.y);
        FLT_FIX_MINMAX (pPoint->z, pRange->low.z, pRange->high.z);
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
Public GEOMDLLIMPEXP void     bsiFRange3d_extendByFRange3d

(
FRange3dP pRange0,
FRange3dCP pRange1
)

    {
    if (pRange0 && pRange1)
        {
        FIX_MIN (pRange1->low.x,  pRange0->low.x);
        FIX_MIN (pRange1->low.y,  pRange0->low.y);
        FIX_MIN (pRange1->low.z,  pRange0->low.z);

        FIX_MAX (pRange1->high.x, pRange0->high.x );
        FIX_MAX (pRange1->high.y, pRange0->high.y );
        FIX_MAX (pRange1->high.z, pRange0->high.z );
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
Public GEOMDLLIMPEXP void     bsiFRange3d_extendByDRange3d

(
FRange3dP pRange0,
DRange3dCP pRange1
)

    {
    if (pRange0 && pRange1)
        {
        FLT_FIX_MIN (pRange1->low.x,  pRange0->low.x);
        FLT_FIX_MIN (pRange1->low.y,  pRange0->low.y);
        FLT_FIX_MIN (pRange1->low.z,  pRange0->low.z);

        FLT_FIX_MAX (pRange1->high.x, pRange0->high.x );
        FLT_FIX_MAX (pRange1->high.y, pRange0->high.y );
        FLT_FIX_MAX (pRange1->high.z, pRange0->high.z );
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
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       bsiFRange3d_intersect

(
FRange3dP pOutRange,
FRange3dP pRange1,
FRange3dP pRange2
)

    {
    if (bsiFRange3d_checkOverlap (pRange1, pRange2))
        {
        pOutRange->low.x = pRange1->low.x > pRange2->low.x ? pRange1->low.x : pRange2->low.x;
        pOutRange->low.y = pRange1->low.y > pRange2->low.y ? pRange1->low.y : pRange2->low.y;
        pOutRange->low.z = pRange1->low.z > pRange2->low.z ? pRange1->low.z : pRange2->low.z;
        pOutRange->high.x = pRange1->high.x < pRange2->high.x ? pRange1->high.x : pRange2->high.x;
        pOutRange->high.y = pRange1->high.y < pRange2->high.y ? pRange1->high.y : pRange2->high.y;
        pOutRange->high.z = pRange1->high.z < pRange2->high.z ? pRange1->high.z : pRange2->high.z;

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
Public GEOMDLLIMPEXP void bsiFRange3d_combineRange

(
FRange3dP pCombRange,
FRange3dCP pRange1,
FRange3dCP pRange2
)

    {
    if (pCombRange)
        {
        pCombRange->low.x = pRange1->low.x < pRange2->low.x ? pRange1->low.x : pRange2->low.x;
        pCombRange->low.y = pRange1->low.y < pRange2->low.y ? pRange1->low.y : pRange2->low.y;
        pCombRange->low.z = pRange1->low.z < pRange2->low.z ? pRange1->low.z : pRange2->low.z;
        pCombRange->high.x = pRange1->high.x > pRange2->high.x ? pRange1->high.x : pRange2->high.x;
        pCombRange->high.y = pRange1->high.y > pRange2->high.y ? pRange1->high.y : pRange2->high.y;
        pCombRange->high.z = pRange1->high.z > pRange2->high.z ? pRange1->high.z : pRange2->high.z;
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
Public GEOMDLLIMPEXP bool    bsiFRange3d_isContained

(
FRange3dCP pInnerRange,
FRange3dCP pOuterRange
)

    {
    return  (pInnerRange->low.x >= pOuterRange->low.x
             && pInnerRange->low.y >= pOuterRange->low.y
             && pInnerRange->low.z >= pOuterRange->low.z
             && pInnerRange->high.x <= pOuterRange->high.x
             && pInnerRange->high.y <= pOuterRange->high.y
             && pInnerRange->high.z <= pOuterRange->high.z);
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
Public GEOMDLLIMPEXP bool    bsiFRange3d_isStrictlyContainedXY

(
FRange3dCP pInnerRange,
FRange3dCP pOuterRange
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
Public GEOMDLLIMPEXP bool        bsiFRange3d_touchesEdge

(
FRange3dCP pInnerRange,
FRange3dCP pOuterRange
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

    if ((pOuterRange->low.z < pOuterRange->high.z) &&
        ((pInnerRange->low.z == pOuterRange->low.z) || (pInnerRange->high.z == pOuterRange->high.z)))
        {
        return  true;
        }

    return  false;
    }

/*----------------------------------------------------------------------+
| name           jmdlFRange3d_restrictIntervalToMinMax                     |
|                                                                       |
| author EarlinLutz                              7/96                   |
|                                                                       |
+----------------------------------------------------------------------*/
static void    jmdlFRange3d_restrictIntervalToMinMax

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
Public GEOMDLLIMPEXP void     bsiFRange3d_restrictToMinMax

(
FRange3dP pRange,
FRange3dCP pRange0,
FRange3dCP pMinMax
)

    {
    jmdlFRange3d_restrictIntervalToMinMax
            (
            &pRange->low.x, &pRange->high.x,
            pRange0->low.x, pRange0->high.x,
            pMinMax->low.x, pMinMax->high.x
            );

    jmdlFRange3d_restrictIntervalToMinMax
            (
            &pRange->low.y, &pRange->high.y,
            pRange0->low.y, pRange0->high.y,
            pMinMax->low.y, pMinMax->high.y
            );

    jmdlFRange3d_restrictIntervalToMinMax
            (
            &pRange->low.z, &pRange->high.z,
            pRange0->low.z, pRange0->high.z,
            pMinMax->low.z, pMinMax->high.z
            );
    }

/* METHOD(FRange3d,none,scaleAboutCenter)
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
Public GEOMDLLIMPEXP void            bsiFRange3d_scaleAboutCenter

(
FRange3dP pRange,
FRange3dCP pRangeIn,
double           scale
)

    {
    DRange3d dRange;
    bsiDRange3d_initFromFRange3d (&dRange, pRangeIn);
    bsiDRange3d_scaleAboutCenter (&dRange, &dRange, scale);
    bsiFRange3d_initFromDRange3d (pRange, &dRange);
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
Public GEOMDLLIMPEXP void            bsiFRange3d_extractPlanes

(
FRange3dCP pRange,
DPoint3dP pOriginArray,
DPoint3dP pNormalArray
)
    {
    DRange3d dRange;
    bsiDRange3d_initFromFRange3d (&dRange, pRange);
    bsiDRange3d_extractPlanes (&dRange, pOriginArray, pNormalArray);
    }


/*-----------------------------------------------------------------*//**
*
* Return the index of the axis with largest absolute range.
*
* @indexVerb magnitude
* @bsihdr                                                       EarlinLutz      09/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsiFRange3d_indexOfMaximalAxis

(
FRange3dCP pRange
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

    a = fabs (pRange->high.z - pRange->low.z);
    if (a > aMax)
        {
        aMax = a;
        index = 2;
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
Public GEOMDLLIMPEXP bool        bsiFRange3d_intersectRay

(
FRange3dCP pRange,
double      *pParam0,
double      *pParam1,
DPoint3dP pPoint0,
DPoint3dP pPoint1,
DPoint3dCP pStart,
DPoint3dCP pDirection
)
    {
    DRange3d dRange;
    bsiDRange3d_initFromFRange3d (&dRange, pRange);
    return bsiDRange3d_intersectRay (&dRange,
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
Public GEOMDLLIMPEXP double  bsiFRange3d_getLargestCoordinate

(
FRange3dCP pRange
)
    {
    double      max;
    DPoint3d diagonal;

    max = fabs(pRange->low.x);
    FIX_MAX(fabs(pRange->high.x), max);
    FIX_MAX(fabs(pRange->low.y), max);
    FIX_MAX(fabs(pRange->high.y), max);
    FIX_MAX(fabs(pRange->low.z), max);
    FIX_MAX(fabs(pRange->high.z), max);

    diagonal.x = pRange->high.x - pRange->low.x;
    diagonal.y = pRange->high.y - pRange->low.y;
    diagonal.z = pRange->high.z - pRange->low.z;

    FIX_MAX(fabs(diagonal.x), max);
    FIX_MAX(fabs(diagonal.y), max);
    FIX_MAX(fabs(diagonal.z), max);

    return max;
    }



/*-----------------------------------------------------------------*//**
*
* Compute the intersection of a range cube with a plane.
* If sorting is requested, the n point polygon is returned as n+1 points
*   with first and last duplicated.
* If no sorting is requested, the polygon is returned as up to 12 points
*   arranged as start-end pairs, in arbitrary order, i.e. probably not chained.
*
* @param pPointArray <= Array to receive points.  MUST be dimensioned to at
*                       least 12.
* @param pNumPoints <= number of points returned.
* @param maxPoints => dimensioned size of receiver buffer.
* @param pOrigin => any point on the plane
* @param pNormal => plane normal vector (not necessarily unit)
* @see
* @indexVerb intersect
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiFRange3d_intersectPlane

(
DPoint3dP pPointArray,
int         *pNumPoints,
int         maxPoints,
FRange3dCP pRange,
DPoint3dCP pOrigin,
DPoint3dCP pNormal,
int         sort
)
    {
    DRange3d range;
    bsiDRange3d_initFromFRange3d (&range, pRange);
    bsiDRange3d_intersectPlane (pPointArray, pNumPoints, maxPoints,
                                    &range, pOrigin, pNormal, sort);
    }


/*-----------------------------------------------------------------*//**
*
* Generates an 8point box around around a range cube.  Point ordering is
* maintained from the cube.
*
* @param pBox <= array of 8 points of the box
* @see
* @indexVerb boxCorners
* @bsihdr                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFRange3d_box2Points

(
FRange3dCP pRange,
DPoint3dP pBox
)
    {
    DRange3d range;
    bsiDRange3d_initFromFRange3d (&range, pRange);
    bsiDRange3d_box2Points (&range, pBox);
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
Public GEOMDLLIMPEXP int      bsiFRange3d_numLeadingPointsInRange

(
FRange3dCP pRange,
DPoint3dCP pPointArray,
int         numPoint,
RangePlaneMask mask
)
    {
    DRange3d range;
    bsiDRange3d_initFromFRange3d (&range, pRange);
    return bsiDRange3d_numLeadingPointsInRange (&range, pPointArray, numPoint, mask);
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
Public GEOMDLLIMPEXP double  bsiFRange3d_getOverlap

(
FRange3dCP pRange1,
FRange3dCP pRange2
)

    {
    FRange3d overlapRange;
    overlapRange.low.x = pRange1->low.x > pRange2->low.x ? pRange1->low.x : pRange2->low.x;
    overlapRange.low.y = pRange1->low.y > pRange2->low.y ? pRange1->low.y : pRange2->low.y;
    overlapRange.low.z = pRange1->low.z > pRange2->low.z ? pRange1->low.z : pRange2->low.z;
    overlapRange.high.x = pRange1->high.x < pRange2->high.x ? pRange1->high.x : pRange2->high.x;
    overlapRange.high.y = pRange1->high.y < pRange2->high.y ? pRange1->high.y : pRange2->high.y;
    overlapRange.high.z = pRange1->high.z < pRange2->high.z ? pRange1->high.z : pRange2->high.z;

    return  bsiFRange3d_extentSquared (&overlapRange);
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
Public GEOMDLLIMPEXP bool    bsiFRange3d_checkOverlap

(
FRange3dCP pRange1,
FRange3dCP pRange2
)
    {
    if (pRange1->low.x > pRange2->high.x
        || pRange1->low.y > pRange2->high.y
        || pRange1->low.z > pRange2->high.z
        || pRange2->low.x > pRange1->high.x
        || pRange2->low.y > pRange1->high.y
        || pRange2->low.z > pRange1->high.z)
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
Public GEOMDLLIMPEXP bool     bsiFRange3d_moveChangesRange

(
FRange3dCP pOldRange,
FRange3dCP pNewRange,
FRange3dCP pOuterRange
)

    {
    if (!pNewRange)
        return  bsiFRange3d_touchesEdge (pOldRange, pOuterRange);

    if ((pOldRange->low.x != pNewRange->low.x) && (pOldRange->low.x == pOuterRange->low.x))
        return  true;

    if ((pOldRange->low.y != pNewRange->low.y) && (pOldRange->low.y == pOuterRange->low.y))
        return  true;

    if ((pOldRange->low.z != pNewRange->low.z) && (pOldRange->low.z == pOuterRange->low.z))
        return  true;

    if ((pOldRange->high.x != pNewRange->high.x) && (pOldRange->high.x == pOuterRange->high.x))
        return  true;

    if ((pOldRange->high.y != pNewRange->high.y) && (pOldRange->high.y == pOuterRange->high.y))
        return  true;

    if ((pOldRange->high.z != pNewRange->high.z) && (pOldRange->high.z == pOuterRange->high.z))
        return  true;

    return  false;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
