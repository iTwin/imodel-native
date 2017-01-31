/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/drange3d.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------*//**
 @struct DRange3d
 A DRange3d structure holds two points at low and high points of diagonal
   range rectangles.
 @field DPoint3d low lower point of diagonal.
 @field DPoint3d high high pointer of diagonal.
 @bsistruct                                        EarlinLutz      09/00
+----------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


#define FIX_MIN(value, min)          if (value < min) min = value
#define FIX_MAX(value, max)          if (value > max) max = value
#define FIX_MINMAX(value, min, max)  FIX_MIN(value, min); FIX_MAX(value, max);

#define MSVECTOR_BOUNDING_PLANES_PER_CUBE 6
#define MSVECTOR_PLANE_CUBE_INTERSECTION_POINTS 12

#if defined (winNT) && defined (doException)
    static void throwException (const char*);
#endif // defined (winNT) && defined (doException)

#if !defined (DBL_MAX)
#define DBL_MAX 1.0e200
#endif

/*-----------------------------------------------------------------*//**
 @description DRange3d
 A DRange3d structure contains 2 points for use as min (low) and max (high)
 corners of a bounding box.

 For a non-null bounding box, each component (x,y,z) of the min point is
 strictly less than the corresponding component of the max point.


 The base init function DRange3d#Init () initializes the range to large
 values with max less than min.  This null state can be identified as a
 null box later on.

 Methods named DRange3d#InitFrom (*(...)) initialize the box around
 various combinations of points.  (E.g. one point, two points, 3 points,
 array of points.)

 Methods named DRange3d#Extend (..) expand an existing box to include additional
 range. (E.g. one point, array of points, components of point)

 @author   EarlinLutz
 @group DRange3d
 @bsimethod                                               EarlinLutz      12/97
+===============+===============+===============+===============+======*/
/* VBSUB(Range3dInit) */

/*-----------------------------------------------------------------*//**
 @description Initializes a range cube with (inverted) large positive and negative
 values.
 @instance pRange OUT the initialized range.
 @group "DRange3d Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange3d_init

(
DRange3dP pRange
)

    {
    pRange->low.x = pRange->low.y = pRange->low.z =  DBL_MAX;
    pRange->high.x = pRange->high.y = pRange->high.z = -DBL_MAX;
    }


#define PANIC_SIZE 1.0e100
/* VBSUB(Range3dIsNull) */

/*-----------------------------------------------------------------*//**
 @description Check if the range is exactly the same as the null ranges of a just-initialized
 range.
 @instance pRange IN range to test.
 @return true if the range is null.
 @group "DRange3d Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDRange3d_isNull

(
DRange3dCP pRange
)
    {
    if (
           pRange->low.x == DBL_MAX
        && pRange->low.y == DBL_MAX
        && pRange->low.z ==  DBL_MAX
        && pRange->high.x == -DBL_MAX
        && pRange->high.y == -DBL_MAX
        && pRange->high.z == -DBL_MAX
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

/* VBSUB(Range3dExtentSquared) */

/*-----------------------------------------------------------------*//**
 @description returns 0 if the range is null (Range3dIsNull), otherwise
       sum of squared axis extents.
 @param pRange IN range to test
 @return squared magnitude of the diagonal vector.
 @group "DRange3d Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   bsiDRange3d_extentSquared

(
DRange3dCP pRange
)


    {
    double dx, dy, dz;
    if (bsiDRange3d_isNull (pRange))
        return  0.0;

    dx = (pRange->high.x - pRange->low.x);
    dy = (pRange->high.y - pRange->low.y);
    dz = (pRange->high.z - pRange->low.z);

    return  ((dx*dx) + (dy*dy) + (dz*dz));
    }




/*-----------------------------------------------------------------*//**
 @description Test if low component is (strictly) less than high in any direction.
 Note that equal components do not indicate empty.
@instance pRange IN range to test
 @returns true if any low component is less than the corresponding high component
 @group "DRange3d Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDRange3d_isEmpty

(
DRange3dCP pRange
)


    {
    return
            pRange->high.x < pRange->low.x
        ||  pRange->high.y < pRange->low.y
        ||  pRange->high.z < pRange->low.z
        ;
    }



/*-----------------------------------------------------------------*//**
 @instance pRange IN range to test
 @return true if high is less than or equal to low in every direction.
 @group "DRange3d Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDRange3d_isPoint

(
DRange3dCP pRange
)


    {
    return
            pRange->high.x <= pRange->low.x
        &&  pRange->high.y <= pRange->low.y
        &&  pRange->high.z <= pRange->low.z
        ;
    }



/*-----------------------------------------------------------------*//**
 @description Returns product of axis extents.
    Returns 0 if the range matches the initial null range conditions.
 @param pRange IN range to test
 @return product of axis lengths.
 @group "DRange3d Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double     bsiDRange3d_volume

(
DRange3dCP pRange
)


    {
    if (bsiDRange3d_isNull (pRange))
        return 0.0;
    return
            (pRange->high.x - pRange->low.x)
        *   (pRange->high.y - pRange->low.y)
        *   (pRange->high.z - pRange->low.z)
        ;
    }



/* VBSUB(Range3dFromPoint3d) */

/*-----------------------------------------------------------------*//**
 @description Initializes the range to contain the single given point.
 @param pRange OUT the initialized range.
 @param pPoint IN the point
 @group "DRange3d Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange3d_initFromPoint

(
DRange3dP pRange,
DPoint3dCP pPoint
)


    {
    pRange->low = pRange->high = *pPoint;
    }


/* VBSUB(Range3dFromPoint3dPoint3d) */

/*-----------------------------------------------------------------*//**
 @description Initializes the range to contain the two given points.
 @param pRange OUT initialized range.
 @param pPoint0 IN first point
 @param pPoint1 IN second point
 @group "DRange3d Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange3d_initFrom2Points

(
DRange3dP pRange,
DPoint3dCP pPoint0,
DPoint3dCP pPoint1
)


    {
    pRange->low = pRange->high = *pPoint0;

    FIX_MINMAX (pPoint1->x, pRange->low.x, pRange->high.x);
    FIX_MINMAX (pPoint1->y, pRange->low.y, pRange->high.y);
    FIX_MINMAX (pPoint1->z, pRange->low.z, pRange->high.z);
    }
/* VBSUB(Range3dFromXYZXYZ) */

/*-----------------------------------------------------------------*//**
 @description Initializes the range to contain two points given as components.
 Minmax logic is applied to the given points.
 @instance pRange OUT initialized range.
 @param x0 IN first x
 @param y0 IN first y
 @param z0 IN first z
 @param x1 IN second x
 @param y1 IN second y
 @param z1 IN second z
 @group "DRange3d Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange3d_initFrom2Components

(
DRange3dP pRange,
double          x0,
double          y0,
double          z0,
double          x1,
double          y1,
double          z1
)
    {
    pRange->low.x = x0;
    pRange->low.y = y0;
    pRange->low.z = z0;
    pRange->high = pRange->low;

    FIX_MINMAX (x1, pRange->low.x, pRange->high.x);
    FIX_MINMAX (y1, pRange->low.y, pRange->high.y);
    FIX_MINMAX (z1, pRange->low.z, pRange->high.z);
    }

/* VBSUB(Range3dFromXYZ) */

/*-----------------------------------------------------------------*//**
 @description Initialize the range from a single point given by components
 @instance pRange OUT initialized range
 @param x IN x coordinate
 @param y IN y coordinate
 @param z IN z coordinate
 @group "DRange3d Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange3d_initFromComponents

(
DRange3dP pRange,
double          x,
double          y,
double          z
)
    {
    pRange->low.x = pRange->high.x = x;
    pRange->low.y = pRange->high.y = y;
    pRange->low.z = pRange->high.z = z;
    }


/*-----------------------------------------------------------------*//**
 @description Initialize the range from given min and max in all directions.
 Given values will be swapped if needed.
 @param pRange OUT the initialized range.
 @param v0 => min (or max)
 @param v1 => max (or min)
 @group "DRange3d Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange3d_initFromCubeLimits

(
DRange3dP pRange,
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
    pRange->low.z  = a;

    pRange->high.x = b;
    pRange->high.y = b;
    pRange->high.z = b;
    }


/* VBSUB(Range3dFromPoint3dPoint3dPoint3d) */

/*-----------------------------------------------------------------*//**
 @description Initializes a range to contain three given points.
 @param pRange OUT initialized range.
 @param pPoint0 IN first point
 @param pPoint1 IN second point
 @param pPoint2 IN third point
 @group "DRange3d Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange3d_initFrom3DPoint3d

(
DRange3dP pRange,
DPoint3dCP pPoint0,
DPoint3dCP pPoint1,
DPoint3dCP pPoint2
)


    {
    pRange->low = pRange->high = *pPoint0;

    FIX_MINMAX (pPoint1->x, pRange->low.x, pRange->high.x);
    FIX_MINMAX (pPoint1->y, pRange->low.y, pRange->high.y);
    FIX_MINMAX (pPoint1->z, pRange->low.z, pRange->high.z);

    FIX_MINMAX (pPoint2->x, pRange->low.x, pRange->high.x);
    FIX_MINMAX (pPoint2->y, pRange->low.y, pRange->high.y);
    FIX_MINMAX (pPoint2->z, pRange->low.z, pRange->high.z);
    }



/*-----------------------------------------------------------------*//**
 @description Initializes the range to contain the range of the given array of points.
 If there are no points in the array, an empty initialized range is returned.

 @param pRange OUT initialized range
 @param pPoint => array of points to search
 @param n => number of points in array
 @group "DRange3d Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange3d_initFromArray

(
DRange3dP pRange,
DPoint3dCP pPoint,
int             n
)

    {
    bsiDRange3d_init (pRange);
    for (int i = 0; i < n; i++)
        bsiDRange3d_extendByDPoint3d (pRange, &pPoint[i]);
    }


/*-----------------------------------------------------------------*//**
 @description Initializes the range to contain the range of the given array of points.
 If there are no points in the array, an initialized range is returned.

 @param pRange OUT initialized range
 @param pPoint => array of points to search
 @param pWeight => optional array of weights.
 @param n => number of points in array
 @group "DRange3d Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange3d_initFromWeightedArray
(
DRange3dP pRange,
DPoint3dCP pPoint,
const double     *pWeight,
int             n
)

    {
    if (pWeight == NULL)
        {
        bsiDRange3d_initFromArray (pRange, pPoint, n);
        }
    else
        {
        for (int i = 0; i < n; i++)
            bsiDRange3d_extendByWeightedDPoint3d (pRange, &pPoint[i], pWeight[i]);
        }
    }



/*-----------------------------------------------------------------*//**
 @description Initializes the range to contain the range of the given array of 2D points,
 with a single given z value for both the min and max points.
 If there are no points in the array, the range is initialized as a null range.
 @instance pRange OUT initialized range.
 @param pPoint => array of points to search
 @param n => number of points in array
 @param zVal => default z value
 @group "DRange3d Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange3d_initFromArray2d

(
DRange3dP pRange,
DPoint2dCP pPoint,
int              n,
double           zVal
)

    {
    int i;
    DPoint3d *  minP = &pRange->low;
    DPoint3d *  maxP = &pRange->high;
    if (n < 1)
        {
        bsiDRange3d_init (pRange);
        }
    else
        {
        maxP->x = pPoint[0].x;
        maxP->y = pPoint[0].y;
        maxP->z = zVal;
        *minP = *maxP;
        for (i=1; i<n; i++)
            {
            FIX_MINMAX ( pPoint[i].x, minP->x, maxP->x );
            FIX_MINMAX ( pPoint[i].y, minP->y, maxP->y );
            }
        }
    }



/*-----------------------------------------------------------------*//**
 @description Extend each axis by the given distance on both ends of the range.
 @instance pRange IN OUT updated range
 @param extend IN distance to extend
 @group "DRange3d Extend"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDRange3d_extendByDistance

(
DRange3dP pRange,
double           extend
)

    {
    if (!bsiDRange3d_isNull (pRange))
        {
        pRange->low.x -= extend;
        pRange->low.y -= extend;
        pRange->low.z -= extend;

        pRange->high.x += extend;
        pRange->high.y += extend;
        pRange->high.z += extend;
        }
    }

/* VBSUB(Range3dFromRange3dMargin) */
/*-----------------------------------------------------------------*//**
 @description Adds a specified margin in all directions.
 @instance pResultRange OUT initialized range
 @param pRange IN initial range.
 @param margin IN margin to add in all directions (positive and negative x, y, z)
 @group "DRange3d Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange3d_initFromDRange3dMargin

(
DRange3dP pResultRange,
DRange3dCP pRange,
double          margin
)
    {
    if (pRange)
        *pResultRange = *pRange;
    else
        bsiDRange3d_init (pResultRange);
    bsiDRange3d_extendByDistance (pResultRange, margin);
    }

/* VBSUB(Range3dUnionPoint3d) */
/*-----------------------------------------------------------------*//**
 @description Extends the coordinates of the range cube points in pRange so as
 to include the single additional point.
 @instance pResultRange OUT initialized range
 @param pRange IN initial range.
 @param pPoint IN new point to be included in the range.
 @group "DRange3d Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange3d_unionDRange3dDPoint3d

(
DRange3dP pResultRange,
DRange3dCP pRange,
DPoint3dCP pPoint
)
    {
    if (pRange)
        *pResultRange = *pRange;
    else
        bsiDRange3d_init (pResultRange);
    FIX_MINMAX (pPoint->x, pResultRange->low.x, pResultRange->high.x);
    FIX_MINMAX (pPoint->y, pResultRange->low.y, pResultRange->high.y);
    FIX_MINMAX (pPoint->z, pResultRange->low.z, pResultRange->high.z);
    }


/*-----------------------------------------------------------------*//**
 @description Extends the coordinates of the range cube points in pRange so as
 to include the single additional point pPoint.
 @instance pRange OUT initialized range
 @param pPoint IN new point to be included in the range.
 @group "DRange3d Extend"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange3d_extendByDPoint3d

(
DRange3dP pRange,
DPoint3dCP pPoint
)
    {
    if (!bsiDPoint3d_isDisconnect (pPoint))
        {
        FIX_MINMAX (pPoint->x, pRange->low.x, pRange->high.x);
        FIX_MINMAX (pPoint->y, pRange->low.y, pRange->high.y);
        FIX_MINMAX (pPoint->z, pRange->low.z, pRange->high.z);
        }
    }


/*-----------------------------------------------------------------*//**
 @description Extends the coordinates of the range cube points in pRange so as
 to include the single additional point pPoint.
 @instance pRange OUT initialized range
 @param pPoint IN new point to be included in the range.
 @param weight IN weight for new point.
 @group "DRange3d Extend"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange3d_extendByWeightedDPoint3d
(
DRange3dP pRange,
DPoint3dCP pPoint,
double     weight
)
    {
    double a;
    if (!bsiDPoint3d_isDisconnect (pPoint) && bsiTrig_safeDivide (&a, 1.0, weight, 0.0))
        {
        DPoint3d realPoint;
        realPoint.Scale (*pPoint, a);
        FIX_MINMAX (realPoint.x, pRange->low.x, pRange->high.x);
        FIX_MINMAX (realPoint.y, pRange->low.y, pRange->high.y);
        FIX_MINMAX (realPoint.z, pRange->low.z, pRange->high.z);
        }
    }


/* VBSUB(Range3dUnionXYZ) */
/*-----------------------------------------------------------------*//**
 @description Extends the coordinates of the range cube points in pRange so as
 to include the single additional point given as xyz coordinates.
 @instance pResultRange OUT initialized range
 @param pRange IN initialized range.
 @param x IN x coordinate of additional point.
 @param y IN y coordinate of additional point.
 @param z IN z coordinate of additional point.
 @group "DRange3d Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange3d_unionDRange3dXYZ

(
DRange3dP pResultRange,
DRange3dCP pRange,
double          x,
double          y,
double          z
)
    {
    if (pRange)
        *pResultRange = *pRange;
    else
        bsiDRange3d_init (pResultRange);
    FIX_MINMAX (x, pResultRange->low.x, pResultRange->high.x);
    FIX_MINMAX (y, pResultRange->low.y, pResultRange->high.y);
    FIX_MINMAX (z, pResultRange->low.z, pResultRange->high.z);
    }


/*-----------------------------------------------------------------*//**

 @description Extends the coordinates of the range cube points in pRange so as
 to include the single additional point at x,y,z.
 @param pRange <=> range to be extended
 @param x => extended range coordinate
 @param y => extended range coordinate
 @param z => extended range coordinate
 @group "DRange3d Extend"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange3d_extendByComponents

(
DRange3dP pRange,
double      x,
double      y,
double      z
)
    {
    FIX_MINMAX (x, pRange->low.x, pRange->high.x);
    FIX_MINMAX (y, pRange->low.y, pRange->high.y);
    FIX_MINMAX (z, pRange->low.z, pRange->high.z);
    }


/*-----------------------------------------------------------------*//**
 @description Extends the coordinates of the range cube points in pRange so as
 to include the (normalized image of) the given 4D point.
 @instance pRange IN OUT updated range
 @param pPoint4d => new point to be included in minmax ranges
 @group "DRange3d Extend"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange3d_extendByDPoint4d

(
DRange3dP pRange,
DPoint4dCP pPoint4d
)

    {
    double divw;
    if (bsiTrig_safeDivide (&divw, 1.0, pPoint4d->w, 0.0))
        {
        double coord;
        coord = pPoint4d->x * divw;
        FIX_MINMAX (coord, pRange->low.x, pRange->high.x);
        coord = pPoint4d->y * divw;
        FIX_MINMAX (coord, pRange->low.y, pRange->high.y);
        coord = pPoint4d->z * divw;
        FIX_MINMAX (coord, pRange->low.z, pRange->high.z);
        }
    }


/*-----------------------------------------------------------------*//**
 @description Extends the coordinates of the range cube points in pRange so as
 to include the (normalized image of the) array of DPoint4d
 @instance pRange IN OUT updated range
 @param pPoint4d => array of  to be included in minmax ranges
 @param numPoint IN number of points.
 @group "DRange3d Extend"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange3d_extendByDPoint4dArray

(
DRange3dP pRange,
DPoint4dCP pPoint4d,
int             numPoint
)

    {
    int i;
    for (i = 0; i < numPoint; i++)
        {
        bsiDRange3d_extendByDPoint4d (pRange, &pPoint4d[i]);
        }
    }




/*-----------------------------------------------------------------*//**
 @description Extends the coordinates of the range cube points in pRange so as
 to include range of an array of points.
 @instance pRange IN OUT updated range
 @param pArray => new points to be included in minmax ranges
 @param n => number of points
 @group "DRange3d Extend"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange3d_extendByDPoint3dArray

(
DRange3dP pRange,
DPoint3dCP pArray,
int             n
)

    {
    const DPoint3d *pPoint = pArray;
    for (; n-- > 0; pPoint++)
        {
        if (!bsiDPoint3d_isDisconnect (pPoint))
            {
            FIX_MINMAX (pPoint->x, pRange->low.x, pRange->high.x);
            FIX_MINMAX (pPoint->y, pRange->low.y, pRange->high.y);
            FIX_MINMAX (pPoint->z, pRange->low.z, pRange->high.z);
            }
        }
    }



/*-----------------------------------------------------------------*//**
 @description Extends the coordinates of the range cube by transformed points
 @instance pRange IN OUT updated range
 @param pTransform => transform to apply to points.
 @param pArray => new points to be included in minmax ranges
 @param n => number of points
 @group "DRange3d Extend"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange3d_extendByTransformDPoint3dArray

(
DRange3dP pRange,
TransformCP pTransform,
DPoint3dCP pArray,
int             n
)

    {
    int i;
    DPoint3d point;
    for (i = 0; i < n; i++)
        {
        point = pArray[i];
        if (!bsiDPoint3d_isDisconnect (&point))
            {
            bsiTransform_multiplyDPoint3dInPlace (pTransform, &point);
            FIX_MINMAX (point.x, pRange->low.x, pRange->high.x);
            FIX_MINMAX (point.y, pRange->low.y, pRange->high.y);
            FIX_MINMAX (point.z, pRange->low.z, pRange->high.z);
            }
        }
    }




/*-----------------------------------------------------------------*//**
 @description Extends the coordinates of the range cube points to
 include the range cube range1P.
 @instance pRange0 IN OUT updated range

 @param pRange1 =>  second range
 @group "DRange3d Extend"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange3d_extendByRange

(
DRange3dP pRange0,
DRange3dCP pRange1
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


/* VBSUB(Range3dUnion) */

/*-----------------------------------------------------------------*//**
 @description returns the union of two ranges.
 @instance pResultRange OUT result range
 @param pRange0 IN first range
 @param pRange1 IN second range
 @group "DRange3d Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange3d_unionDRange3dDRange3d

(
DRange3dP pResultRange,
DRange3dCP pRange0,
DRange3dCP pRange1
)

    {
    *pResultRange = *pRange0;
    bsiDRange3d_extendByRange (pResultRange, pRange1);
    }

/*-----------------------------------------------------------------*//**
 @description Compute the intersection of two ranges if they overlap (even with
       zero thickness); otherwise, return false without setting the output range.
 @instance pResultRange OUT intersection range.
 @param pRange1 => first range
 @param pRange2 => second range
 @return same result as checkOverlap(pRange1,pRange2).
 @group "DRange3d Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       bsiDRange3d_intersect

(
DRange3dP pResultRange,
DRange3dP pRange1,
DRange3dP pRange2
)

    {
    if (bsiDRange3d_checkOverlap (pRange1, pRange2))
        {
        pResultRange->low.x = pRange1->low.x > pRange2->low.x ? pRange1->low.x : pRange2->low.x;
        pResultRange->low.y = pRange1->low.y > pRange2->low.y ? pRange1->low.y : pRange2->low.y;
        pResultRange->low.z = pRange1->low.z > pRange2->low.z ? pRange1->low.z : pRange2->low.z;
        pResultRange->high.x = pRange1->high.x < pRange2->high.x ? pRange1->high.x : pRange2->high.x;
        pResultRange->high.y = pRange1->high.y < pRange2->high.y ? pRange1->high.y : pRange2->high.y;
        pResultRange->high.z = pRange1->high.z < pRange2->high.z ? pRange1->high.z : pRange2->high.z;

        return true;
        }
    return false;
    }

/* VBSUB(Range3dIntersect) */

/*-----------------------------------------------------------------*//**
 @description Compute the intersection of two ranges.  If any direction has no intersection,
       or if the intersection has zero thickness, the result range is initialized to a null range.
 @instance pResultRange OUT intersection range.
 @param pRange1 => first range
 @param pRange2 => second range
 @group "DRange3d Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiDRange3d_intersectDRange3dDRange3d

(
DRange3dP pResultRange,
DRange3dCP pRange1,
DRange3dCP pRange2
)
    {
    pResultRange->low.x = pRange1->low.x > pRange2->low.x ? pRange1->low.x : pRange2->low.x;
    pResultRange->low.y = pRange1->low.y > pRange2->low.y ? pRange1->low.y : pRange2->low.y;
    pResultRange->low.z = pRange1->low.z > pRange2->low.z ? pRange1->low.z : pRange2->low.z;
    pResultRange->high.x = pRange1->high.x < pRange2->high.x ? pRange1->high.x : pRange2->high.x;
    pResultRange->high.y = pRange1->high.y < pRange2->high.y ? pRange1->high.y : pRange2->high.y;
    pResultRange->high.z = pRange1->high.z < pRange2->high.z ? pRange1->high.z : pRange2->high.z;

    if (   pResultRange->low.x >= pResultRange->high.x
        || pResultRange->low.y >= pResultRange->high.y
        || pResultRange->low.z >= pResultRange->high.z
        )
        bsiDRange3d_init (pResultRange);
    }

/*-----------------------------------------------------------------*//**
 @description Form the union of two ranges.
 @instance pCombRange OUT combined range
 @param pRange1 => first range.
 @param pRange2 => second range.
 @group "DRange3d Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDRange3d_combineRange

(
DRange3dP pCombRange,
DRange3dCP pRange1,
DRange3dCP pRange2
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

/* VBSUB(Range3dIsContainedInRange3d) */

/*-----------------------------------------------------------------*//**
 @description Test if the first range is contained in the second range.
 @param pInnerRange IN candidate inner range.
 @param pOuterRange IN candidate outer range.
 @return true if the inner range is a (possibly improper) subset of the outer range.
 @group "DRange3d Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDRange3d_isContained

(
DRange3dCP pInnerRange,
DRange3dCP pOuterRange
)
    {
    return  (pInnerRange->low.x >= pOuterRange->low.x
             && pInnerRange->low.y >= pOuterRange->low.y
             && pInnerRange->low.z >= pOuterRange->low.z
             && pInnerRange->high.x <= pOuterRange->high.x
             && pInnerRange->high.y <= pOuterRange->high.y
             && pInnerRange->high.z <= pOuterRange->high.z);
    }

/* VBSUB(Range3dContainsPoint3d) */

/*-----------------------------------------------------------------*//**
 @description Test if a point is contained in a range.
 @param pRange IN candidate containing range.
 @param pPoint IN point to test.
 @return true if the point is in (or on boundary of)
 @group "DRange3d Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDRange3d_isDPoint3dContained

(
DRange3dCP pRange,
DPoint3dCP pPoint
)

    {
    return  (   pPoint->x >= pRange->low.x
             && pPoint->y >= pRange->low.y
             && pPoint->z >= pRange->low.z
             && pPoint->x <= pRange->high.x
             && pPoint->y <= pRange->high.y
             && pPoint->z <= pRange->high.z
             );
    }

/* VBSUB(Range3dContainsXYZ) */

/*-----------------------------------------------------------------*//**
 @description Test if a point given as x,y,z is contained in a range.
 @param pRange IN candidate containing range.
 @param x IN x coordinate
 @param y IN y coordinate
 @param z IN z coordinate
 @return true if the point is in (or on boundary of)
 @group "DRange3d Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDRange3d_isXYZContained

(
DRange3dCP pRange,
double    x,
double    y,
double    z
)

    {
    return  (   x >= pRange->low.x
             && y >= pRange->low.y
             && z >= pRange->low.z
             && x <= pRange->high.x
             && y <= pRange->high.y
             && z <= pRange->high.z);
    }


/* VBSUB(Range3dEqual) */

/*-----------------------------------------------------------------*//**
 @description Test if two ranges are exactly equal.
 @param pRange0 IN first range
 @param pRange1 IN second range
 @param tolerance IN toleranc to be applied to each component
 @return true if ranges are identical in all components.
 @group "DRange3d Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDRange3d_isEqual

(
DRange3dCP pRange0,
DRange3dCP pRange1
)

    {
    return
           pRange0->low.x  == pRange1->low.x
        && pRange0->low.y  == pRange1->low.y
        && pRange0->low.z  == pRange1->low.z
        && pRange0->high.x == pRange1->high.x
        && pRange0->high.y == pRange1->high.y
        && pRange0->high.z == pRange1->high.z
        ;
    }
/* VBSUB(Range3dEqualTolerance) */

/*-----------------------------------------------------------------*//**
 @description Test if two ranges are equal within a tolerance applied componentwise.
 @param pRange0 IN first range
 @param pRange1 IN second range
 @param tolerance IN toleranc to be applied to each component
 @return true if ranges are within tolerance in all components.
 @group "DRange3d Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDRange3d_isEqualTolerance

(
DRange3dCP pRange0,
DRange3dCP pRange1,
double tolerance
)
    {
    return
           fabs (pRange0->low.x  - pRange1->low.x ) <= tolerance
        && fabs (pRange0->low.y  - pRange1->low.y ) <= tolerance
        && fabs (pRange0->low.z  - pRange1->low.z ) <= tolerance
        && fabs (pRange0->high.x - pRange1->high.x) <= tolerance
        && fabs (pRange0->high.y - pRange1->high.y) <= tolerance
        && fabs (pRange0->high.z - pRange1->high.z) <= tolerance
        ;
    }


/*-----------------------------------------------------------------*//**
 @description Test if the given range is a proper subset of pOuterRange, using only xy parts
 @param pInnerRange => inner range
 @param pOuterRange => outer range
 @return true if the given range is a proper subset of
   pOuterRange.
 @group "DRange3d Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDRange3d_isStrictlyContainedXY

(
DRange3dCP pInnerRange,
DRange3dCP pOuterRange
)

    {
    return      pInnerRange->low.x  > pOuterRange->low.x
             && pInnerRange->low.y  > pOuterRange->low.y
             && pInnerRange->high.x < pOuterRange->high.x
             && pInnerRange->high.y < pOuterRange->high.y;
    }


/*-----------------------------------------------------------------*//**
 @description test if any min or max of the given range touches a limit (min or max)
 of a non-trivial direction of pOuterRange.
 @param pInnerRange => inner range
 @param pOuterRange => outer range
 @return true if there is an edge touch.
 @group "DRange3d Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDRange3d_touchesEdge

(
DRange3dCP pInnerRange,
DRange3dCP pOuterRange
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
| name           jmdlDRange3d_restrictIntervalToMinMax                     |
|                                                                       |
| author EarlinLutz                              7/96                   |
|                                                                       |
+----------------------------------------------------------------------*/
static void    jmdlDRange3d_restrictIntervalToMinMax

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
 @description Returns a range which is the intersection of two ranges.  The first
 range is treated as a signed range, i.e. decreasing values from low
 to high are a nonempty range, and the output will maintain the
 direction.
 In a direction where there is no overlap, pRange high and low values
 are identical and are at the limit of pRange1 that is nearer to the
 values in pRange0.
 (Intended use: pRange0 is the 'actual' stroking range of a surface
   i.e. may go 'backwards'.  pRange1 is the nominal full surface range,
   i.e. is known a priori to be 'forwards'.  The clipping restricts
   unreliable pRange0 to the nominal surface range pRange1.
 pRange0 and pRange may be the same address.  pMinMax must be different.
 @param pRange OUT computed range
 @param pRange0 => range to be restricted
 @param pMinMax => allowable minmax range.  Assumed to have low < high
 @group "DRange3d Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange3d_restrictToMinMax

(
DRange3dP pRange,
DRange3dCP pRange0,
DRange3dCP pMinMax
)

    {
    jmdlDRange3d_restrictIntervalToMinMax
            (
            &pRange->low.x, &pRange->high.x,
            pRange0->low.x, pRange0->high.x,
            pMinMax->low.x, pMinMax->high.x
            );

    jmdlDRange3d_restrictIntervalToMinMax
            (
            &pRange->low.y, &pRange->high.y,
            pRange0->low.y, pRange0->high.y,
            pMinMax->low.y, pMinMax->high.y
            );

    jmdlDRange3d_restrictIntervalToMinMax
            (
            &pRange->low.z, &pRange->high.z,
            pRange0->low.z, pRange0->high.z,
            pMinMax->low.z, pMinMax->high.z
            );
    }

/* VBSUB(Range3dScaleAboutCenter) */

/*-----------------------------------------------------------------*//**
 @description scale a range about its center point.
 @param pRange OUT scaled range.
 @param pRangeIn IN original range
 @param scale IN scale factor
 @group "DRange3d Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            bsiDRange3d_scaleAboutCenter

(
DRange3dP pRange,
DRange3dCP pRangeIn,
double          scale
)

    {
    DPoint3d low;
    DPoint3d diagonal;
    double   eps = scale - 1.0;

    low = pRangeIn->low;
    bsiDPoint3d_subtractDPoint3dDPoint3d (&diagonal, &pRangeIn->high, &pRangeIn->low);
    bsiDPoint3d_addScaledDPoint3d (&pRange->high, &low, &diagonal, 1.0 + eps);
    bsiDPoint3d_addScaledDPoint3d (&pRange->low, &low, &diagonal, -eps);
    }


/*-----------------------------------------------------------------*//**
 @description Extract the 6 bounding planes for a range cube.
 @instance pRange IN range to query
 @param pOriginArray <= array of plane origins
 @param pNormalArray <= array of plane normals
 @group "DRange3d Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            bsiDRange3d_extractPlanes

(
DRange3dCP pRange,
DPoint3dP pOriginArray,
DPoint3dP pNormalArray
)
    {
    DRange3d minMaxRange;

    bsiDRange3d_initFrom2Points (&minMaxRange, &pRange->low, &pRange->high);
    pOriginArray[0] = pOriginArray[1] = pOriginArray[2] = minMaxRange.low;
    pOriginArray[3] = pOriginArray[4] = pOriginArray[5] = minMaxRange.high;

    bsiDPoint3d_setXYZ (&pNormalArray[0], -1.0, 0.0, 0.0);
    bsiDPoint3d_setXYZ (&pNormalArray[3],  1.0, 0.0, 0.0);

    bsiDPoint3d_setXYZ (&pNormalArray[1], 0.0, -1.0, 0.0);
    bsiDPoint3d_setXYZ (&pNormalArray[4], 0.0,  1.0, 0.0);

    bsiDPoint3d_setXYZ (&pNormalArray[2], 0.0, 0.0, -1.0);
    bsiDPoint3d_setXYZ (&pNormalArray[5], 0.0, 0.0,  1.0);
    }


/*-----------------------------------------------------------------*//**
 @description Return the index of the axis with largest absolute range.
 @param pRange IN range to query
 @return axis index
 @group "DRange3d Queries"
 @bsimethod                                                       EarlinLutz      09/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsiDRange3d_indexOfMaximalAxis

(
DRange3dCP pRange
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

    a = fabs (pRange->high.z - pRange->low.z);
    if (a > aMax)
        {
        aMax = a;
        index = 2;
        }

    return index;
    }


/*-----------------------------------------------------------------*//**
 @description Compute the intersection of a range cube and a ray.

 If there is not a finite intersection, both params are set to 0 and
 and both points to pPoint0.
 @param pRange IN range to test
 @param pParam0 <= ray parameter where cube is entered
 @param pParam1 <= ray parameter where cube is left
 @param pPoint0 <= entry point
 @param pPoint1 <= exit point
 @param pStart => start point of ray
 @param pDirection => direction of ray
 @return true if non-empty intersection.
 @group "DRange3d Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDRange3d_intersectRay

(
DRange3dCP pRange,
double      *pParam0,
double      *pParam1,
DPoint3dP pPoint0,
DPoint3dP pPoint1,
DPoint3dCP pStart,
DPoint3dCP pDirection
)
    {
    double s0, s1;      /* parameters of 'in' segment */
    int     contents = -1;
    bool    boolStat;
    /* save points in case of duplicate pointers by caller */
    DPoint3d start;
    DPoint3d direction;

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
    bsiRange1d_intersectLine
                (
                &s0, &s1, &contents,
                pStart->z, pDirection->z,
                pRange->low.z, pRange->high.z
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
        bsiDPoint3d_addScaledDPoint3d (pPoint0, &start, &direction, s0);

    if (pPoint1)
        bsiDPoint3d_addScaledDPoint3d (pPoint1, &start, &direction, s1);

    return  boolStat;
    }


/*-----------------------------------------------------------------*//**
 @description Compute the intersection of a range cube and a line segment

 If there is not a finite intersection, both params are set to 0 and
 and the output segment consists of only the start point.
 @instance pRange IN range to test
 @param pParam0 <= ray parameter where cube is entered
 @param pParam1 <= ray parameter where cube is left
 @param pClipped <= clipped segment
 @param pSegment IN segment to clip
 @return true if non-empty intersection.
 @group "DRange3d Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDRange3d_intersectDSegment3dBounded

(
DRange3dCP pRange,
double      *pParam0,
double      *pParam1,
DSegment3dP pClipped,
DSegment3dCP pSegment
)
    {
    DPoint3d point0, point1;
    double param0, param1;
    DPoint3d direction;
    bool    boolstat = false;
    bool    unboundedStat;
    bsiDPoint3d_subtractDPoint3dDPoint3d (&direction, &pSegment->point[1], &pSegment->point[0]);
    unboundedStat = bsiDRange3d_intersectRay (pRange, &param0, &param1, NULL, NULL, &pSegment->point[0], &direction);
    if (unboundedStat)
        {
        if (param1 > 1.0)
            param1 = 1.0;
        if (param0 < 0.0)
            param0 = 0.0;

        if (param1 > param0)
            {
            boolstat = true;
            }
        }

    if (!boolstat)
        {
        param0 = param1 = 0.0;
        }

    if (pClipped)
        {
        bsiDSegment3d_fractionParameterToDPoint3d (pSegment, &point0, param0);
        bsiDSegment3d_fractionParameterToDPoint3d (pSegment, &point1, param1);
        bsiDSegment3d_initFromDPoint3d (pClipped, &point0, &point1);
        }

    if (pParam0)
        *pParam0 = param0;
    if (pParam1)
        *pParam1 = param1;
    return  boolstat;
    }

/* VBSUB(Point3dFromRange3dPoint3dFractions) */
/*-----------------------------------------------------------------*//**
 @description Convert fractional coordinates within a range cube to point coordinates.
 @instance pRange IN range to query
 @param pPoint OUT the computed point
 @param pFractionPoint OUT coordinates as fractions with 000 at range box low,
                   111 at range box high.
 @return true if all conversions are completed.  A just-initialized range
       box fails.  A range box with zero thickness in one or more directions also fails.
 @group "DRange3d Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDRange3d_dPoint3dFractionsToDPoint3d

(
DRange3dCP pRange,
DPoint3dP pPoint,
DPoint3dCP pFractionPoint
)
    {
    bool    boolstat = false;

    if (bsiDRange3d_isNull (pRange))
        {
        bsiDPoint3d_zero (pPoint);
        }
    else
        {
        pPoint->x = pRange->low.x + pFractionPoint->x * (pRange->high.x - pRange->low.x);
        pPoint->y = pRange->low.y + pFractionPoint->y * (pRange->high.y - pRange->low.y);
        pPoint->z = pRange->low.z + pFractionPoint->z * (pRange->high.z - pRange->low.z);
        }

    return  boolstat;
    }

/* VBSUB(Point3dFromRange3Fractions) */
/*-----------------------------------------------------------------*//**
 @description Convert fractional coordinates within a range cube to point coordinates.
 @param pRange IN range to query
 @param pPoint OUT the computed point
 @param x IN fractional coordinate in x direction
 @param y IN fractional coordinate in y direction
 @param z IN fractional coordinate in z direction
 @return true if all conversions are completed.  A just-initialized range
       box fails.  A range box with zero thickness in one or more directions also fails.
 @group "DRange3d Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDRange3d_fractionsToDPoint3d

(
DRange3dCP pRange,
DPoint3dP pPoint,
double      x,
double      y,
double      z
)
    {
    bool    boolstat = false;

    if (bsiDRange3d_isNull (pRange))
        {
        bsiDPoint3d_zero (pPoint);
        }
    else
        {
        pPoint->x = pRange->low.x + x * (pRange->high.x - pRange->low.x);
        pPoint->y = pRange->low.y + y * (pRange->high.y - pRange->low.y);
        pPoint->z = pRange->low.z + z * (pRange->high.z - pRange->low.z);
        }

    return  boolstat;
    }

/* VBSUB(Point3dFractionsFromRange3dDPoint3d) */
/*-----------------------------------------------------------------*//**
 @description Compute point coordinates to fractional coordinates within a range cube.
 @param pRange IN range to query
 @param pFractionPoint OUT coordinates as fractions with 000 at range box low,
                   111 at range box high.
 @param pPoint IN original coordinates
 @return true if all conversions are completed.  A just-initialized range
       box fails.  A range box with zero thickness in one or more directions is ok.
 @group "DRange3d Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDRange3d_dPoint3dToDPoint3dFractions

(
DRange3dCP pRange,
DPoint3dP pFractionPoint,
DPoint3dCP pPoint
)
    {
    bool    boolstat = false;

    if (bsiDRange3d_isNull (pRange))
        {
        bsiDPoint3d_zero (pFractionPoint);
        }
    else
        {
        boolstat = bsiTrig_safeDivide (&pFractionPoint->x,
                                    pPoint->x - pRange->low.x,
                                    pRange->high.x - pRange->low.x,
                                    0.0
                                    )
                && bsiTrig_safeDivide (&pFractionPoint->y,
                                    pPoint->y - pRange->low.y,
                                    pRange->high.y - pRange->low.y,
                                    0.0
                                    )
                && bsiTrig_safeDivide (&pFractionPoint->z,
                                    pPoint->z - pRange->low.z,
                                    pRange->high.z - pRange->low.z,
                                    0.0
                                    )
                    ;
        }

    return  boolstat;
    }


/*-----------------------------------------------------------------*//**
 @description Compute the intersection of a range cube with a plane.
 If sorting is requested, the n point polygon is returned as n+1 points
   with first and last duplicated.
 If no sorting is requested, the polygon is returned as up to 12 points
   arranged as start-end pairs, in arbitrary order, i.e. probably not chained.
 @param pPointArray <= Array to receive points.  MUST be dimensioned to at
                       least 12.
 @param pNumPoints <= number of points returned.
 @param maxPoints => dimensioned size of receiver buffer.
 @instance pRange IN range to test
 @param pOrigin => any point on the plane
 @param pNormal => plane normal vector (not necessarily unit)
 @param sort IN true to chain the intersection segments as a polygon
 @group "DRange3d Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDRange3d_intersectPlane

(
DPoint3dP pPointArray,
int         *pNumPoints,
int         maxPoints,
DRange3dCP pRange,
DPoint3dCP pOrigin,
DPoint3dCP pNormal,
int         sort
)
    {
    int startIndex[12] = {0,0,0, 3,3,3, 5,5,5, 6,6,6};
    int endIndex[12]   = {1,2,4, 1,2,7, 1,4,7, 2,4,7};
    DPoint3d corner[8];
    double   h[8];
    DPoint3d xyz[20];  // There are 12 edges and 8 vertices.  Allocate
                       // as if each of those will generate a point.
    DVec3d centroid;
    DVec3d normal = *(DVec3d*)pNormal;   // pNormal should be DVec3d !!!
    int n = 0;
    bsiDRange3d_box2Points (pRange, corner);

    if (bsiDRange3d_isNull (pRange))
        {
        *pNumPoints = 0;
        return;
        }

    for (int i = 0; i < 8; i++)
        {
        h[i] = corner[i].DotDifference (*pOrigin, normal);
        if (h[i] == 0.0)
            {
            xyz[n++] = corner[i];
            }
        }

    for (int edge = 0; edge < 12; edge++)
        {
        double h0 = h[startIndex[edge]];
        double h1 = h[  endIndex[edge]];
        if (h0 * h1 < 0.0)
            {
            double f = -h0 / (h1 - h0);
            xyz[n++].Interpolate (corner[startIndex[edge]], f, corner[endIndex[edge]]);
            }
        }

    if (sort && n > 2)
        {
        // Sort so points are CCW in plane.
        // (n=3 is may need to be reversed!!!)
        bsiDPoint3d_averageDPoint3dArray (&centroid, xyz, n);
        DVec3d refVector, vector;
        // make h[i] = angle from vector [centroid,xyz[0]] to vector [centroid,xyz[i])
        refVector.DifferenceOf (xyz[0], centroid);
        h[0] = 0.0;
        for (int i = 1; i < n; i++)
            {
            vector.DifferenceOf (xyz[i], centroid);
            h[i] = refVector.SignedAngleTo(vector, normal);
            }

        // Selection sort on angles:
        //int numOut = 0;
        for (int i = 0; i < n; i++)
            {
            int jMin = i;
            double aMin = h[i];
            for (int j = i+1; j < n; j++)
                {
                if (h[j] < aMin)
                    {
                    aMin = h[j];
                    jMin = j;
                    }
                }
            if (jMin > i)
                {
                DPoint3d tempXYZ = xyz[i];
                double   tempH   = h[i];
                h[i] = h[jMin];
                xyz[i] = xyz[jMin];
                h[jMin]   = tempH;
                xyz[jMin] = tempXYZ;
                }
            }
        }
    if (n > maxPoints)
        n = maxPoints;
    *pNumPoints = n;
    for (int i = 0; i < n && i < maxPoints; i++)
        {
        pPointArray[i] = xyz[i];
        }
    }


/*-----------------------------------------------------------------*//**
 @description Generates an 8point box around around a range cube.  Point ordering is
 maintained from the cube.
 @param pRange IN range to query
 @param pBox <= array of 8 points of the box
 @group "DRange3d Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange3d_box2Points

(
DRange3dCP pRange,
DPoint3dP pBox
)
    {
    DPoint3d minmax[2];
    int ix,iy,iz,i;
    minmax[0] = pRange->low;
    minmax[1] = pRange->high;
    i = 0;
    for( iz = 0; iz < 2; iz++ )
        for ( iy = 0; iy < 2; iy++ )
            for ( ix = 0; ix < 2; ix++ )
                {
                bsiDPoint3d_setXYZ( &pBox[i], minmax[ix].x, minmax[iy].y, minmax[iz].z );
                i++;
                }
    }



/*-----------------------------------------------------------------*//**
 @description Starting at the beginning of the array, test if points from pPointArray are "in" the range.

 @param pPointArray => points
 @param numPoint => number of points
 @param pRange => range cube
 @param mask   => selects faces to consider. Valid values are the constants
       RangePlane_XMin
 @return number of points that were "in" before the first "out" or end of array.
 @group "DRange3d Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsiDRange3d_numLeadingPointsInRange

(
DRange3dCP pRange,
DPoint3dCP pPointArray,
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


    if (mask & RangePlane_ZMin)
        {
        a = pRange->low.z;
        for (i = 0; i < nMax && pPointArray[i].z >= a; i++)
            {}
        nMax = i;
        }
    if (nMax == 0)
        return 0;


    if (mask & RangePlane_ZMax)
        {
        a = pRange->high.z;
        for (i = 0; i < nMax && pPointArray[i].z <= a; i++)
            {}
        nMax = i;
        }

    return nMax;
    }



/*-----------------------------------------------------------------*//**
 @description Compute the intersection of given range with another range and return the
 extentSquared of the intersection range.
 @instance pRange1 => first range
 @param pRange2 => second range
 @return extentSquared() for the intersection range.
 @group "DRange3d Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDRange3d_getOverlap

(
DRange3dCP pRange1,
DRange3dCP pRange2
)

    {
    DRange3d overlapRange;
    overlapRange.low.x = pRange1->low.x > pRange2->low.x ? pRange1->low.x : pRange2->low.x;
    overlapRange.low.y = pRange1->low.y > pRange2->low.y ? pRange1->low.y : pRange2->low.y;
    overlapRange.low.z = pRange1->low.z > pRange2->low.z ? pRange1->low.z : pRange2->low.z;
    overlapRange.high.x = pRange1->high.x < pRange2->high.x ? pRange1->high.x : pRange2->high.x;
    overlapRange.high.y = pRange1->high.y < pRange2->high.y ? pRange1->high.y : pRange2->high.y;
    overlapRange.high.z = pRange1->high.z < pRange2->high.z ? pRange1->high.z : pRange2->high.z;

    return  bsiDRange3d_extentSquared (&overlapRange);
    }




/*-----------------------------------------------------------------*//**
 @description Test if two ranges have strictly non-null overlap (intersection)

 @param pRange1 => first range
 @param pRange2 => second range
 @return true if ranges overlap, false if not.
 @group "DRange3d Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDRange3d_checkOverlap

(
DRange3dCP pRange1,
DRange3dCP pRange2
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
 @description Compute vectors from origin to range corners.  Find extrema
  of their dot products with a vector.
 @param pRange => range to test
 @param pMin <= minimum dot value.
 @param pMax <= maximum dot value.
 @param pBasePoint => optional base point.  If null, 000 is assumed.
 @param pVector => vector for dot product.
 @group "DRange3d Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDRange3d_projectedExtrema

(
DRange3dCP pRange,
double    *pMin,
double    *pMax,
DPoint3dP pBasePoint,
DPoint3dP pVector
)
    {
    double dot[8];
    int i;
    double mindot, maxdot;
    DPoint3d corner[8];
    bsiDRange3d_box2Points (pRange, corner);

    if (pBasePoint)
        {
        for (i = 0; i < 8; i++)
            dot[i] = bsiDPoint3d_dotDifference (&corner[i], pBasePoint, (DVec3d*)pVector);
        }
    else
        {
        for (i = 0; i < 8; i++)
            dot[i] = bsiDPoint3d_dotProduct (pVector, &corner[i]);
        }

    mindot = maxdot = dot[0];
    for (i = 1; i < 8; i++)
        {
        if (dot[i] < mindot)
            mindot = dot[i];
        else if (dot[i] > maxdot)
            maxdot = dot[i];
        }

    if (pMin)
        *pMin = mindot;

    if (pMax)
        *pMax = maxdot;

    }



/*-----------------------------------------------------------------*//**
 @description Test if a modification of the given (instance) range would have a different
 touching relationship with pOuterRange.

 @remarks This may only be meaningful in context of range tree tests where
   some prior relationship among ranges is known to apply.
 @instance pOldRange => original range
 @param pNewRange => candidate for modified range relationship.
 @param pOuterRange => containing range
 @return true if touching condition occurs.
 @group "DRange3d Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDRange3d_moveChangesRange

(
DRange3dCP pOldRange,
DRange3dCP pNewRange,
DRange3dCP pOuterRange
)

    {
    if (!pNewRange)
        return  bsiDRange3d_touchesEdge (pOldRange, pOuterRange);

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



/*-----------------------------------------------------------------*//**
* Initialize the range from an arc of the unit circle
* @param theta0 => start angle
* @param sweep  => angular sweep
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDRange3d_initFromUnitArcSweep

(
DRange3dP pRange,
double          theta0,
double          sweep
)
    {
    double theta1 = theta0 + sweep;
    if (bsiTrig_isAngleFullCircle (sweep))
        {
        bsiDRange3d_initFrom2Components (pRange, -1.0, -1.0, 0.0, 1.0, 1.0, 0.0);
        }
    else
        {
        DPoint3d testPoint;

        testPoint.x = cos (theta0);
        testPoint.y = sin (theta0);
        testPoint.z = 0.0;
        bsiDRange3d_initFromPoint (pRange, &testPoint);
        bsiDRange3d_extendByComponents (pRange, cos (theta1), sin (theta1), 0.0);

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


/*----------------------------------------------------------------------+
|                                                                       |
| name          throwException                                          |
|                                                                       |
| author        SamWilson                               06/99           |
|                                                                       |
+----------------------------------------------------------------------*/
#if defined (winNT) && defined (doException)

#undef ERROR
#define Rectangle WindowsRectangle

END_BENTLEY_GEOMETRY_NAMESPACE
#include <windows.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#undef Rectangle

typedef void (*t_fcall) ();

static void throwException

(
const char      *pDesc
)
    {
    HANDLE h;
    if ( (h = GetModuleHandle ("javai_g.dll")) || (h = GetModuleHandle ("javai.dll")) )
        {
        t_fcall fc = (t_fcall) GetProcAddress(h, "SignalError");
        fc (NULL, "java/lang/Exception", "BadRange");
        }

    // _asm int 3;
    }

#endif

END_BENTLEY_GEOMETRY_NAMESPACE
