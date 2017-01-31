/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/dpoint3d.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_NAMESPACE


/*-----------------------------------------------------------------*//**
 @doctext
 @group DPoint3d
 @defaultGroup DPoint3d
 In classic microstation the single type DPoint3d was interchangeably used for
        both points and vectors.
 It is strongly recommended that new code distinguish carefully between
        points and vectors, using DPoint3d for points and DVec3d for vectors.
 @bsistruct                                         EarlinLutz      09/00
+----------------------------------------------------------------------*/

#define FIX_MIN(value, min)          if (value < min) min = value
#define FIX_MAX(value, max)          if (value > max) max = value
#define FIX_MINMAX(value, min, max)  FIX_MIN(value, min); else FIX_MAX(value, max)

/* VBSUB(Point3dCrossProduct3PointsXY) */
/* CSVFUNC(crossProductToPointsXY) */

/*-----------------------------------------------------------------*//**
 @description Returns the (scalar) cross product of the xy parts of two vectors.
   The vectors are computed from the Origin to Target1 and Target2.
 @param pOrigin => The base point for computing vectors
 @param pTarget1 => The target point for the first vector
 @param pTarget2 => The target point for the second vector
 @group "DPoint3d Dot and Cross"
 @return scalar cross product
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_crossProduct3DPoint3dXY


(
DPoint3dCP pOrigin,
DPoint3dCP pTarget1,
DPoint3dCP pTarget2
)
    {
    double x1 = pTarget1->x - pOrigin->x;
    double y1 = pTarget1->y - pOrigin->y;

    double x2 = pTarget2->x - pOrigin->x;
    double y2 = pTarget2->y - pOrigin->y;

    return x1 * y2 - y1 * x2;
    }

/* VBSUB(Point3dDotProduct3Points) */
/* CSVFUNC(dotProductToPoints) */

/*-----------------------------------------------------------------*//**
 @description Returns the (scalar) dot product of two vectors.
   The vectors are computed from the Origin to Target1 and Target2.
 @param pOrigin => The base point for computing vectors
 @param pTarget1 => The target point for the first vector
 @param pTarget2 => The target point for the second vector
 @group "DPoint3d Dot and Cross"
 @return dot product
  @bsimethod                                                                    EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_dotProduct3DPoint3d


(
DPoint3dCP pOrigin,
DPoint3dCP pTarget1,
DPoint3dCP pTarget2
)
    {
    double x1 = pTarget1->x - pOrigin->x;
    double y1 = pTarget1->y - pOrigin->y;
    double z1 = pTarget1->z - pOrigin->z;

    double x2 = pTarget2->x - pOrigin->x;
    double y2 = pTarget2->y - pOrigin->y;
    double z2 = pTarget2->z - pOrigin->z;

    return  x1 * x2 + y1 * y2 + z1 * z2;
    }

/* VBSUB(Point3dDotProduct3PointsXY) */
/* CSVFUNC(dotProductToPointsXY) */

/*-----------------------------------------------------------------*//**
 @description Returns the (scalar) dot product of xy parts of two vectors.
   The vectors are computed from the Origin to Target1 and Target2.
 @param pOrigin => The base point for computing vectors
 @param pTarget1 => The target point for the first vector
 @param pTarget2 => The target point for the second vector
@group "DPoint3d Dot and Cross"
 @return dot product
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_dotProduct3DPoint3dXY


(
DPoint3dCP pOrigin,
DPoint3dCP pTarget1,
DPoint3dCP pTarget2
)
    {
    double x1 = pTarget1->x - pOrigin->x;
    double y1 = pTarget1->y - pOrigin->y;

    double x2 = pTarget2->x - pOrigin->x;
    double y2 = pTarget2->y - pOrigin->y;

    return  x1 * x2 + y1 * y2;
    }

/* VBSUB(Point3dDotDifferenceVec3d) */
/* CSVFUNC(dotDifference) */

/*-----------------------------------------------------------------*//**
 @description Returns the (scalar) dot product of two vectors.
  One vector is computed internally as the difference of the Target
   and Origin (Target - Origin).
  The other is given directly as a single argument.

 @param pTarget IN The end point of first vector of the cross product.
 @param pOrigin IN The start point of the first vector of the cross product.
 @param pVector IN The second vector of the cross product.
@group "DPoint3d Dot and Cross"
 @return dot product
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_dotDifference


(
DPoint3dCP pTarget,
DPoint3dCP pOrigin,
DVec3dCP pVector
)
    {
    return
            (pTarget->x - pOrigin->x) * pVector->x
        +   (pTarget->y - pOrigin->y) * pVector->y
        +   (pTarget->z - pOrigin->z) * pVector->z;
    }

/* VBSUB(Point3dTripleProduct4Points) */
/* CSVFUNC(tripleProductToPoints) */

/*-----------------------------------------------------------------*//**
 @description Computes the triple product of vectors from a base point to three target points.
 @param pOrigin => The base point for all three vectors
 @param pTarget1 => The target point for the first vector
 @param pTarget2 => The target point for the second vector
 @param pTarget3 => The target point for the third vector
 @return triple product
@group "DPoint3d Dot and Cross"
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_tripleProduct4Points


(
DPoint3dCP pOrigin,
DPoint3dCP pTarget1,
DPoint3dCP pTarget2,
DPoint3dCP pTarget3
)
    {
    DVec3d   vector1, vector2, vector3;
    bsiDPoint3d_subtractDPoint3dDPoint3d ( &vector1, pTarget1, pOrigin);
    bsiDPoint3d_subtractDPoint3dDPoint3d ( &vector2, pTarget2, pOrigin);
    bsiDPoint3d_subtractDPoint3dDPoint3d ( &vector3, pTarget3, pOrigin);
    return bsiDPoint3d_tripleProduct (&vector1, &vector2, &vector3);
    }

/* VBSUB(Point3dZero) */


/* VBSUB(Point3dOne) */

/*-----------------------------------------------------------------*//**
 @description Sets all components of a point or vector to 1.0.
 @param pPoint <= initialized point or vector
@group "DPoint3d Initialization"
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_one


(
DPoint3dP pPoint
)
    {
    pPoint->x = 1.0;
    pPoint->y = 1.0;
    pPoint->z = 1.0;
    }



/*-----------------------------------------------------------------*//**
 @description Copies doubles from a 3 component array to the x,y, and z components of a DPoint3d
 @param pPoint <= initialized point or vector
 @param pXyz => x, y, z components
@group "DPoint3d Initialization"
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_fromArray


(
DPoint3dP pPoint,
const   double      *pXyz
)
    {
    pPoint->x = pXyz[0];
    pPoint->y = pXyz[1];
    pPoint->z = pXyz[2];
    }


/*-----------------------------------------------------------------*//**
 @description Copy (and promote) from a float point.
 @param pPoint <= initialized point or vector
 @param pFPoint => source point
@group "DPoint3d Initialization"
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_initFromFPoint3d


(
DPoint3dP pPoint,
FPoint3dCP pFPoint
)
    {
    pPoint->x = pFPoint->x;
    pPoint->y = pFPoint->y;
    pPoint->z = pFPoint->z;
    }


/*-----------------------------------------------------------------*//**
 @description Copy from a 2d point, setting z to zero.
 @param pPoint <= initialized point or vector
 @param pFPoint => source point
@group "DPoint3d Initialization"
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_initFromDPoint2d


(
DPoint3dP pPoint,
DPoint2dCP pFPoint
)
    {
    pPoint->x = pFPoint->x;
    pPoint->y = pFPoint->y;
    pPoint->z = 0.0;
    }


/*-----------------------------------------------------------------*//**
 @description Copy from a 2d float point, setting z to zero.
 @param pPoint <= initialized point or vector
 @param pFPoint => source point
@group "DPoint3d Initialization"
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_initFromFPoint2d


(
DPoint3dP pPoint,
FPoint2dCP pFPoint
)
    {
    pPoint->x = pFPoint->x;
    pPoint->y = pFPoint->y;
    pPoint->z = 0.0;
    }

/* VBSUB(Point3dFromXY) */
/* CSVFUNC(FromXY) */

/*-----------------------------------------------------------------*//**
 @description Sets the x and y components of a point.  Sets z to zero.
 @param pPoint <= initialized point or vector
 @param ax => x component
 @param ay => y component
@group "DPoint3d Initialization"
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_setXY


(
DPoint3dP pPoint,
double       ax,
double       ay
)
    {
    pPoint->x = ax;
    pPoint->y = ay;
    pPoint->z = 0.0;
    }



/*-----------------------------------------------------------------*//**
 @description Sets the x,y, and z components of a DPoint3d structure from the
 corresponding parts of a DPoint4d.  Weight part of DPoint4d is not used.
 @param pPoint <= initialized point
 @param pHPoint => homogeneous point
 @group "DPoint3d Initialization"
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_getXYZ


(
DPoint3dP pPoint,
DPoint4dCP pHPoint
)
    {
    pPoint->x = pHPoint->x;
    pPoint->y = pHPoint->y;
    pPoint->z = pHPoint->z;
    }

/*-----------------------------------------------------------------*//**
 @description Sets the x,y, and z components of a DPoint3d structure from the
    corresponding parts of a DPoint4d, multiplied by the given scalar.  Weight part of DPoint4d is not used.
 @param pPoint <= initialized point
 @param pHPoint => homogeneous point
 @param a => scale
 @group "DPoint3d Initialization"
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_getScaledXYZ


(
DPoint3dP pPoint,
DPoint4dCP pHPoint,
double          a
)
    {
    pPoint->x = pHPoint->x * a;
    pPoint->y = pHPoint->y * a;
    pPoint->z = pHPoint->z * a;
    }



/*-----------------------------------------------------------------*//**
 @description Set one of the x,y,z components of the point.
 @param pPoint <= point or vector whose component is set
 @param a => component value
 @param index => selects the the component: 0=x, 1=y, 2=z, others cyclic
@group "DPoint3d Modification"
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_setComponent


(
DPoint3dP pPoint,
double       a,
int         index
)
    {
    if (index > 2)
        index = index % 3;
    if (index == 0)
        {
        pPoint->x = a;
        }
    else if (index == 1)
        {
        pPoint->y = a;
        }
    else if (index == 2)
        {
        pPoint->z = a;
        }
    else /* index < 0.*/
        {
        bsiDPoint3d_setComponent (pPoint, a, 3 - ( (-index) % 3));
        }
    }

/* VBSUB(Point3dGetComponent) */


/*-----------------------------------------------------------------*//**
 @description Gets a single component of a point.
 @param pPoint => point or vector whose component is accessed
 @param index => selects the component: 0=x, 1=y, 2=z, others cyclic
 @return specified component of the point or vector
@group "DPoint3d Queries"
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_getComponent


(
DPoint3dCP pPoint,
int         index
)
    {
    if (index > 2)
        index = index % 3;
    if (index == 0)
        {
        return pPoint->x;
        }
    else if (index == 1)
        {
        return pPoint->y;
        }
    else if (index == 2)
        {
        return pPoint->z;
        }
    else /* index < 0.*/
        {
        return bsiDPoint3d_getComponent (pPoint, 3 - ( (-index) % 3));
        }
    }




/*-----------------------------------------------------------------*//**
 @description Copies x,y,z components from a point to individual variables.
 @param pPoint => source point
 @param pXCoord <= x component
 @param pYCoord <= y component
 @param pZCoord <= z component
@group "DPoint3d Queries"
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_getComponents


(
DPoint3dCP pPoint,
double      *pXCoord,
double      *pYCoord,
double      *pZCoord
)
    {
    if (pXCoord)
        *pXCoord = pPoint->x;
    if (pYCoord)
        *pYCoord = pPoint->y;
    if (pZCoord)
        *pZCoord = pPoint->z;
    }

/* VBSUB(Point3dInterpolate) */
/* CSVFUNC(interpolate) */

/*-----------------------------------------------------------------*//**
 @description Computes a point whose position is given by a fractional
    argument and two endpoints.
 @param pSum <= interpolated point
 @param pPoint0 => point corresponding to fractionParameter 0.0
 @param fractionParameter => fractional parametric coordinate (0.0 is the start of the segment, 1.0 is the end, 0.5 is midpoint)
 @param pPoint1 => point corresponding to fractionParameter 1.0
@group "DPoint3d Addition"
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_interpolate


(
DPoint3dP pSum,
DPoint3dCP pPoint0,
double           fractionParameter,
DPoint3dCP pPoint1
)
    {
    pSum->x = pPoint0->x + fractionParameter * (pPoint1->x - pPoint0->x);
    pSum->y = pPoint0->y + fractionParameter * (pPoint1->y - pPoint0->y);
    pSum->z = pPoint0->z + fractionParameter * (pPoint1->z - pPoint0->z);
    }

/* VBSUB(Point3dIsPointInSmallerSector) */
/* CSVFUNC(isPointInSmallerSector) */

/*-----------------------------------------------------------------*//**
 @description Test if the vector from the Origin to the TestPoint is within the smaller
    angle between the vectors from the Origin to the Target points.
 @param pTestPoint => point to test
 @param pOrigin => origin for vectors
 @param pTarget1 => first target point
 @param pTarget2 => second target point
 @return true if the test point is within the angle
@group "DPoint3d Queries"
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3d_isPointInSmallerSector


(
DPoint3dCP pTestPoint,
DPoint3dCP pOrigin,
DPoint3dCP pTarget1,
DPoint3dCP pTarget2
)
    {
    DVec3d   vector0, vector1, vector;
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vector0, pTarget1, pOrigin);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vector1, pTarget2, pOrigin);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vector , pTestPoint, pOrigin);
    return bsiDPoint3d_isVectorInSmallerSector (&vector, &vector0, &vector1);
    }

/* VBSUB(Point3dIsPointInCCWSector) */
/* CSVFUNC(isPointInCCWector) */

/*-----------------------------------------------------------------*//**
 @description Test if a point is within the counter-clockwise sector defined by
    an origin and two boundary points, with an up vector to determine which
    direction is counter clockwise.
 @param pTestPoint => point to test
 @param pOrigin => origin for vectors
 @param pTarget0 => first target point
 @param pTarget1 => second target point
 @param pUpVector => vector pointing upward from the plane in which CCW direction is determined
 @return true if the test point is within the angle
@group "DPoint3d Queries"
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3d_isPointInCCWSector


(
DPoint3dCP pTestPoint,
DPoint3dCP pOrigin,
DPoint3dCP pTarget0,
DPoint3dCP pTarget1,
DVec3dCP pUpVector
)
    {
    DVec3d   vector0, vector1, vector;
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vector0, pTarget0, pOrigin);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vector1, pTarget1, pOrigin);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vector , pTestPoint, pOrigin);
    return bsiDPoint3d_isVectorInCCWSector (&vector, &vector0, &vector1, pUpVector);
    }

/* VBSUB(Point3dDistance) */
/* CSVFUNC(distance) */

/*-----------------------------------------------------------------*//**
 @description Computes the (cartesian) distance between two points.
 @param pPoint1 => first point
 @param pPoint2 => second point
 @return distance between points
@group "DPoint3d Distance"
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_distance


(
DPoint3dCP pPoint1,
DPoint3dCP pPoint2
)
    {
    double      xdist, ydist, zdist;

    xdist = pPoint2->x - pPoint1->x;
    ydist = pPoint2->y - pPoint1->y;
    zdist = pPoint2->z - pPoint1->z;

    return (sqrt (xdist*xdist + ydist*ydist + zdist*zdist));
    }

/* VBSUB(Point3dDistanceXY) */
/* CSVFUNC(distanceXY) */

/*-----------------------------------------------------------------*//**
 @description Computes the distance between two points, using only x and y components.
 @param pPoint1 => first point
 @param pPoint2 => second point
 @return distance between the XY projections of the two points (i.e. any z difference is ignored)
@group "DPoint3d Distance"
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_distanceXY


(
DPoint3dCP pPoint1,
DPoint3dCP pPoint2
)
    {
    double      xdist, ydist;

    xdist = pPoint2->x - pPoint1->x;
    ydist = pPoint2->y - pPoint1->y;

    return sqrt (xdist*xdist + ydist*ydist);
    }

/* VBSUB(Point3dMaxAbs) */


/*-----------------------------------------------------------------*//**
 @description Finds the largest absolute value among the components of a difference of points or vectors.
 @param pVector1 => first point or vector
 @param pVector2 => second point or vector
 @return largest absolute value among point/vector coordinates
@group "DPoint3d Queries"
 @bsimethod                                       DavidAssaf    04/02
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_maxAbsDifference


(
DPoint3dCP pVector1,
DPoint3dCP pVector2
)
    {
    DPoint3d diff;
    diff.x = pVector1->x - pVector2->x;
    diff.y = pVector1->y - pVector2->y;
    diff.z = pVector1->z - pVector2->z;

    return bsiDPoint3d_maxAbs (&diff);
    }


/* VBSUB(Point3dEqual) */
/* CSVFUNC(isEqual) */

/*-----------------------------------------------------------------*//**
 @description Test for exact equality between all components of two points or vectors.
 @param pVector1 => first point or vector
 @param pVector2 => second point or vector
 @return true if the points are identical
@group "DPoint3d Queries"
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3d_pointEqual


(
DPoint3dCP pVector1,
DPoint3dCP pVector2
)
    {
    bool                    result;

    if (pVector1 && pVector2)
        {
        result =
                   pVector1->x == pVector2->x
                && pVector1->y == pVector2->y
                && pVector1->z == pVector2->z;
        }
    else if (pVector1)
        {
        result =
                   pVector1->x == 0.0
                && pVector1->y == 0.0
                && pVector1->z == 0.0;
        }
    else if (pVector2)
        {
        result =
                   0.0 == pVector2->x
                && 0.0 == pVector2->y
                && 0.0 == pVector2->z;
        }
    else /* Both are null. Call them equal. */
        result = true;

    return  result;
    }

/* VBSUB(Point3dEqualTolerance) */
/* CSVFUNC(isEqual) */

/*-----------------------------------------------------------------*//**
 @description Test if the x, y, and z components of two points or vectors are equal within tolerance.
 @remarks Tests are done against the absolute value of <EM>each</EM> component difference
    (i.e., not against the sum of these absolute differences or the square root of the sum of the squares of these differences).
 @param pVector1 => first point or vector
 @param pVector2 => second point or vector
 @param tolerance => tolerance
 @return true if all components are within given tolerance of each other.
@group "DPoint3d Queries"
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3d_pointEqualTolerance


(
DPoint3dCP pVector1,
DPoint3dCP pVector2,
double                  tolerance
)
    {
    bool                    result;

    if (pVector1 && pVector2)
        {
        result = fabs(pVector1->x - pVector2->x) <= tolerance &&
                 fabs(pVector1->y - pVector2->y) <= tolerance &&
                 fabs(pVector1->z - pVector2->z) <= tolerance;
        }
    else if (pVector1)
        {
        result = fabs(pVector1->x) <= tolerance &&
                 fabs(pVector1->y) <= tolerance &&
                 fabs(pVector1->z) <= tolerance;
        }
    else if (pVector2)
        {
        result = fabs(pVector2->x) <= tolerance &&
                 fabs(pVector2->y) <= tolerance &&
                 fabs(pVector2->z) <= tolerance;
        }
    else /* Both are null. Call them equal. */
        result = true;

    return  result;
    }


/*-----------------------------------------------------------------*//**
 @description Computes the coordinates of pPoint under the translation and scaling that puts 000 at pCube->low and 111 at pCube->high.
 @param pParameters <= NPC coordinates of pPoint within pCube
 @param pPoint => point whose NPC coordinates are to be computed
 @param pCube => range box whose corners map to 000 and 111
@group "DPoint3d Initialization"
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_npcCoordinates


(
DPoint3dP pParameters,
DPoint3dCP pPoint,
DRange3dCP pCube
)
    {
    //int     result = 0;
    double  a;

    a = pCube->high.x - pCube->low.x;
    pParameters->x = (a != 0.0) ? (pPoint->x - pCube->low.x) / a : 0.0;

    a = pCube->high.y - pCube->low.y;
    pParameters->y = (a != 0.0) ? (pPoint->y - pCube->low.y) / a : 0.0;

    a = pCube->high.z - pCube->low.z;
    pParameters->z = (a != 0.0) ? (pPoint->z - pCube->low.z) / a : 0.0;
    }


/*-----------------------------------------------------------------*//**
@description Tests if a point is a disconnect (separator) point.
 @param pPoint => point to test
 @return true if the any component of the point is the disconnect value
@see bsiDPoint3d_initDisconnect
@group "DPoint3d Queries"
 @bsimethod                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3d_isDisconnect


(
DPoint3dCP pPoint
)
    {
    return pPoint->x == DISCONNECT
        || pPoint->y == DISCONNECT
        || pPoint->z == DISCONNECT;
    }


/*-----------------------------------------------------------------*//**
@description Initialize a point with all coordinates as the disconnect value.
 @param pPoint <= point to initialize
@see bsiDPoint3d_isDisconnect
@group "DPoint3d Initialization"
 @bsimethod                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_initDisconnect


(
DPoint3dP pPoint
)
    {
    pPoint->x = pPoint->y = pPoint->z = DISCONNECT;
    }

/* VBSUB(Point3dAddAngleDistance) */
/* CSVFUNC(AddAngleDistanceZ) */
/*-----------------------------------------------------------------*//**
 @description Add an offset vector to a given starting point or vector,
       with the offset specified in XY polar form, i.e. as angle and distance.
 @param pSum <= computed sum
 @param pOrigin => point or vector to offset
 @param angleRadians => angle from x-axis to projection of offset vector in xy-plane
 @param distanceXY => length of projection of offset vector in xy-plane
 @param dz => z part of offset vector
@group "DPoint3d Addition"
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_addAngleDistance


(
DPoint3dP pSum,
DPoint3dCP pOrigin,
double      angleRadians,
double      distanceXY,
double      dz
)
    {
    pSum->x = pOrigin->x + distanceXY * cos (angleRadians);
    pSum->y = pOrigin->y + distanceXY * sin (angleRadians);
    pSum->z = pOrigin->z + dz;
    }

/* VBSUB(Point3dFromAngleDistance) */
/* CSVFUNC(fromAngleDistanceZ) */
/*-----------------------------------------------------------------*//**
 @description Initialize a point from polar angle and distance from origin.
 @param pPoint <= computed point
 @param angleRadians => angle in xy-plane from x-axis
 @param distanceXY => xy plane distance from origin to result point
 @param z => z part of point
@group "DPoint3d Initialization"
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_initFromAngleDistance


(
DPoint3dP pPoint,
double      angleRadians,
double      distanceXY,
double      z
)
    {
    pPoint->x = distanceXY * cos (angleRadians);
    pPoint->y = distanceXY * sin (angleRadians);
    pPoint->z = z;
    }

/*-----------------------------------------------------------------*//**
@description Sweep a point along a specified direction until it is on a plane.
@instance pPoint IN start point.
@param pResult OUT initialized point
@param pSweepMultiplier OUT muliplier of sweep vector.
@param pSweepDirection IN direction to project.  If NULL, the plane normal is used.
@param pPlane IN the target plane.
@return false if projection direction is parallel to the plane.  The result
    is then a copy of the source.
@group "DPoint3d Queries"
@bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3d_sweepToPlane
(
DPoint3dCP  pPoint,
DPoint3dP   pResult,
double *    pSweepMultiplier,
DVec3dCP    pSweepDirection,
DPlane3dCP  pPlane
)
    {
    DVec3d sweepDirection = NULL != pSweepDirection ? *pSweepDirection : pPlane->normal;
    double dot0 = bsiDPoint3d_dotDifference (pPoint, &pPlane->origin, &pPlane->normal);
    double dot1 = bsiDVec3d_dotProduct (&sweepDirection, &pPlane->normal);
    double f;
    bool    boolstat = bsiTrig_safeDivide (&f, -dot0, dot1, 0.0);
    if (pSweepMultiplier)
        *pSweepMultiplier = f;
    if (pResult)
        bsiDPoint3d_addScaledDVec3d (pResult, pPoint, &sweepDirection, f);
    return boolstat;
    }
/*-----------------------------------------------------------------*//**
@description Sweep a weighted point along a specified direction until it is on a plane.
        Return the weighted plane point.
@instance pPoint IN weighted start point.
@param    pResult OUT initialized point
@param    pSweepMultiplier OUT muliplier of sweep vector.
@param    weight IN weight of both start point and result.
@param    pSweepDirection IN direction to project.  If NULL, the plane normal is used.
@param    pPlane IN the target plane.
@return false if projection direction is parallel to the plane.  The result
    is then a copy of the source.
@group "DPoint3d Queries"
@bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3d_sweepWeightedToPlane
(
DPoint3dCP  pPoint,
DPoint3dP   pResult,
double *    pSweepMultiplier,
double      weight,
DVec3dCP    pSweepDirection,
DPlane3dCP  pPlane
)
    {
    DVec3d sweepDirection = NULL != pSweepDirection ? *pSweepDirection : pPlane->normal;
    DVec3d vectorAB;
    bsiDVec3d_addScaled (&vectorAB, (DVec3d*)pPoint, (DVec3d*)&pPlane->origin, -weight);
    double dot0 = bsiDVec3d_dotProduct(&vectorAB, &pPlane->normal);
    double dot1 = bsiDVec3d_dotProduct (&sweepDirection, &pPlane->normal);
    double f;
    bool    boolstat = bsiTrig_safeDivide (&f, -dot0, dot1, 0.0);
    if (pSweepMultiplier)
        *pSweepMultiplier = f;
    if (pResult)
        bsiDPoint3d_addScaledDVec3d (pResult, pPoint, &sweepDirection, f);
    return boolstat;
    }


END_BENTLEY_NAMESPACE
