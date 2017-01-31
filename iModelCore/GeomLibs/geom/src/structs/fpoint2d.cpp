/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/fpoint2d.cpp $
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


/*---------------------------------------------------------------------------------**//**
* Initialize with given float values.
* @indexVerb init
* @indexVerb set
* @bsihdr                                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFPoint2d_setXYFloat

(
FPoint2dP pPoint,
float fx,
float fy
)
    {
    pPoint->x = fx;
    pPoint->y = fy;
    }


/*---------------------------------------------------------------------------------**//**
* Initialize with given double values.
* @indexVerb init
* @indexVerb set
* @bsihdr                                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFPoint2d_setXY

(
FPoint2dP pPoint,
double x,
double y
)
    {
    pPoint->x = (float)x;
    pPoint->y = (float)y;
    }


/*---------------------------------------------------------------------------------**//**
* Initialize from a double point.
* @indexVerb init
* @bsihdr                                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFPoint2d_initFromDPoint3d

(
FPoint2dP pPoint,
DPoint3dCP pDPoint
)
    {
    pPoint->x = (float)pDPoint->x;
    pPoint->y = (float)pDPoint->y;
    }


/*---------------------------------------------------------------------------------**//**
* Initialize from a double point.
* @indexVerb init
* @bsihdr                                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFPoint2d_initFromFPoint3d

(
FPoint2dP pPoint,
FPoint3dCP pDPoint
)
    {
    pPoint->x = pDPoint->x;
    pPoint->y = pDPoint->y;
    }


/*---------------------------------------------------------------------------------**//**
* Initialize from a double point.
* @indexVerb init
* @bsihdr                                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFPoint2d_initFromDPoint2d

(
FPoint2dP pPoint,
DPoint2dCP pDPoint
)
    {
    pPoint->x = (float)pDPoint->x;
    pPoint->y = (float)pDPoint->y;
    }


/*-----------------------------------------------------------------*//**
* Computes the (cartesian) distance between this instance and the parameter vector.
*
* @instance pInstance => first point
* @param pPoint => second point
* @see
* @return distance to second point.
* @indexVerb distance
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiFPoint2d_distance

(
FPoint2dCP pInstance,
FPoint2dCP pPoint
)
    {
    double      xdist, ydist;

    xdist = pPoint->x - pInstance->x;
    ydist = pPoint->y - pInstance->y;

    return sqrt (xdist*xdist + ydist*ydist);
    }


/*-----------------------------------------------------------------*//**
* Computes the squared magnitude of this instance vector.
*
* @instance pInstance => vector whose squared length is computed
* @see
* @return squared magnitude of the vector.
* @indexVerb magnitude
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiFPoint2d_magnitudeSquared

(
FPoint2dCP pInstance
)
    {
    return   pInstance->x * pInstance->x
           + pInstance->y * pInstance->y;
    }


/*-----------------------------------------------------------------*//**
* Computes the squared distance from this instance to the point.
*
* @instance pInstance => base point
* @param pPoint => target point
* @see
* @return squared distance between the points.
* @indexVerb distance
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiFPoint2d_distanceSquared

(
FPoint2dCP pInstance,
FPoint2dCP pPoint
)
    {
    double      xdist, ydist;

    xdist = (pPoint->x - pInstance->x);
    ydist = (pPoint->y - pInstance->y);

    return (xdist*xdist + ydist*ydist);
    }


/*-----------------------------------------------------------------*//**
* Computes the magnitude of this instance vector.
*
* @instance pInstance => vector
* @see
* @return length of the vector
* @indexVerb magnitude
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiFPoint2d_magnitude

(
FPoint2dCP pInstance
)
    {
    return  sqrt ( pInstance->x*pInstance->x
                 + pInstance->y*pInstance->y);
    }


/*-----------------------------------------------------------------*//**
* Finds the largest absolute value among the components of this instance.
*
* @instance pInstance => vector
* @see
* @return largest absoluted value among point coordinates.
* @indexVerb magnitude
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiFPoint2d_maxAbs

(
FPoint2dCP pInstance
)
    {
    double maxVal = fabs (pInstance->x);

    if (fabs (pInstance->y) > maxVal)
        maxVal = fabs (pInstance->y);

    return maxVal;
    }



/*-----------------------------------------------------------------*//**
* Tests for exact equality between the instance and the parameter vector.
*
* @instance pInstance => first point
* @param pVec2 => second vector
* @return true if the points are identical.
* @indexVerb equal
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiFPoint2d_pointEqual

(
FPoint2dCP pInstance,
FPoint2dCP pVec2
)
    {
    bool                    result;

    if (pInstance && pVec2)
        {
        result =
                   pInstance->x == pVec2->x
                && pInstance->y == pVec2->y;
        }
    else if (pInstance)
        {
        result =
                   pInstance->x == 0.0
                && pInstance->y == 0.0;
        }
    else if (pVec2)
        {
        result =
                   0.0 == pVec2->x
                && 0.0 == pVec2->y;
        }
    else /* Both are null. Call them equal. */
        result = true;

    return  result;
    }


/*-----------------------------------------------------------------*//**
* Test if the x, y, and z components of this instance and parameter vectors
* are within a tolerance of each other.
* Tests are done independently using the absolute value of each component differences
* (i.e. not the magnitude or sum of squared differences)
*
* @instance pInstance => vector
* @param pVec2 => vector
* @param tolerance => tolerance
* @see
* @return true if all components are within given tolerance of each other.
* @indexVerb equal
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiFPoint2d_pointEqualTolerance

(
FPoint2dCP pInstance,
FPoint2dCP pVec2,
double                  tolerance
)
    {
    bool                    result;

    if (pInstance && pVec2)
        {
        result = fabs(pInstance->x - pVec2->x) <= tolerance &&
                 fabs(pInstance->y - pVec2->y) <= tolerance;
        }
    else if (pInstance)
        {
        result = fabs(pInstance->x) <= tolerance &&
                 fabs(pInstance->y) <= tolerance;
        }
    else if (pVec2)
        {
        result = fabs(pVec2->x) <= tolerance &&
                 fabs(pVec2->y) <= tolerance;
        }
    else /* Both are null. Call them equal. */
        result = true;

    return  result;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
