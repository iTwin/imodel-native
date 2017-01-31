/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/fpoint3d.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE



/*---------------------------------------------------------------------------------**//**
* Initialize with given float values.
* @indexVerb init
* @indexVerb set
* @bsihdr                                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFPoint3d_setXYZFloat

(
FPoint3dP pPoint,
float fx,
float fy,
float fz
)
    {
    pPoint->x = fx;
    pPoint->y = fy;
    pPoint->z = fz;
    }


/*---------------------------------------------------------------------------------**//**
* Initialize with given double values.
* @indexVerb init
* @bsihdr                                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFPoint3d_initFromDoubleArrayXYZ

(
FPoint3dP pPoint,
double      *pXYZ
)
    {
    pPoint->x = (float)pXYZ[0];
    pPoint->y = (float)pXYZ[1];
    pPoint->z = (float)pXYZ[2];
    }


/*---------------------------------------------------------------------------------**//**
* Initialize with given float values.
* @indexVerb init
* @bsihdr                                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFPoint3d_initFromFloatArrayXYZ

(
FPoint3dP pPoint,
float       *pXYZ
)
    {
    memcpy (pPoint, pXYZ, 3 * sizeof (float));
    }



/*---------------------------------------------------------------------------------**//**
* Initialize with given double values.
* @indexVerb init
* @indexVerb set
* @bsihdr                                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFPoint3d_setXYZ

(
FPoint3dP pPoint,
double x,
double y,
double z
)
    {
    pPoint->x = (float)x;
    pPoint->y = (float)y;
    pPoint->z = (float)z;
    }


/*---------------------------------------------------------------------------------**//**
* Initialize from a double point.
* @indexVerb init
* @bsihdr                                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFPoint3d_initFromDPoint3d

(
FPoint3dP pPoint,
DPoint3dCP pSource
)
    {
    pPoint->x = (float)pSource->x;
    pPoint->y = (float)pSource->y;
    pPoint->z = (float)pSource->z;
    }


/*---------------------------------------------------------------------------------**//**
* Initialize from an xy point.
* @indexVerb init
* @bsihdr                                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFPoint3d_initFromFPoint2d

(
FPoint3dP pPoint,
FPoint2dCP pSource
)
    {
    pPoint->x = pSource->x;
    pPoint->y = pSource->y;
    pPoint->z = 0.0;
    }


/*---------------------------------------------------------------------------------**//**
* Initialize from a double point.
* @indexVerb init
* @bsihdr                                                                       EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiFPoint3d_initFromDPoint2d

(
FPoint3dP pPoint,
DPoint2dCP pSource
)
    {
    pPoint->x = (float)pSource->x;
    pPoint->y = (float)pSource->y;
    pPoint->z = 0.0;
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
Public GEOMDLLIMPEXP double bsiFPoint3d_distance

(
FPoint3dCP pInstance,
FPoint3dCP pPoint
)
    {
    double      xdist, ydist, zdist;

    xdist = pPoint->x - pInstance->x;
    ydist = pPoint->y - pInstance->y;
    zdist = pPoint->z - pInstance->z;

    return (sqrt (xdist*xdist + ydist*ydist + zdist*zdist));
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
Public GEOMDLLIMPEXP double bsiFPoint3d_magnitudeSquared

(
FPoint3dCP pInstance
)
    {
    return   pInstance->x * pInstance->x
           + pInstance->y * pInstance->y
           + pInstance->z * pInstance->z;
    }


/*-----------------------------------------------------------------*//**
* Computes the squared distance from this instance to the point.
*
* @instance pInstance => base point
* @param pPoint => target point
* @see
* @return squared distance between the points.
* @indexVerb magnitude
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiFPoint3d_distanceSquared

(
FPoint3dCP pInstance,
FPoint3dCP pPoint
)
    {
    double      xdist, ydist, zdist;

    xdist = (pPoint->x - pInstance->x);
    ydist = (pPoint->y - pInstance->y);
    zdist = (pPoint->z - pInstance->z);

    return (xdist*xdist + ydist*ydist + zdist*zdist);
    }


/*-----------------------------------------------------------------*//**
* Computes the squred distance between this instance and the point, using only
* the x and y components.
*
* @instance pInstance => base point
* @param pPoint => target point.
* @see
* @return squared distance between the XY projections ofhte two points.
*               (i.e. any z difference is ignored)
* @indexVerb distance
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiFPoint3d_distanceSquaredXY

(
FPoint3dCP pInstance,
FPoint3dCP pPoint
)
    {
    double      xdist, ydist;

    xdist = pPoint->x - pInstance->x;
    ydist = pPoint->y - pInstance->y;

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
Public GEOMDLLIMPEXP double bsiFPoint3d_magnitude

(
FPoint3dCP pInstance
)
    {
    return  sqrt ( pInstance->x*pInstance->x
                 + pInstance->y*pInstance->y
                 + pInstance->z*pInstance->z);
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
Public GEOMDLLIMPEXP double bsiFPoint3d_maxAbs

(
FPoint3dCP pInstance
)
    {
    double maxVal = fabs (pInstance->x);

    if (fabs (pInstance->y) > maxVal)
        maxVal = fabs (pInstance->y);

    if (fabs (pInstance->z) > maxVal)
        maxVal = fabs (pInstance->z);

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
Public GEOMDLLIMPEXP bool    bsiFPoint3d_pointEqual

(
FPoint3dCP pInstance,
FPoint3dCP pVec2
)
    {
    bool                    result;

    if (pInstance && pVec2)
        {
        result =
                   pInstance->x == pVec2->x
                && pInstance->y == pVec2->y
                && pInstance->z == pVec2->z;
        }
    else if (pInstance)
        {
        result =
                   pInstance->x == 0.0
                && pInstance->y == 0.0
                && pInstance->z == 0.0;
        }
    else if (pVec2)
        {
        result =
                   0.0 == pVec2->x
                && 0.0 == pVec2->y
                && 0.0 == pVec2->z;
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
Public GEOMDLLIMPEXP bool    bsiFPoint3d_pointEqualTolerance

(
FPoint3dCP pInstance,
FPoint3dCP pVec2,
double                  tolerance
)
    {
    bool                    result;

    if (pInstance && pVec2)
        {
        result = fabs(pInstance->x - pVec2->x) <= tolerance &&
                 fabs(pInstance->y - pVec2->y) <= tolerance &&
                 fabs(pInstance->z - pVec2->z) <= tolerance;
        }
    else if (pInstance)
        {
        result = fabs(pInstance->x) <= tolerance &&
                 fabs(pInstance->y) <= tolerance &&
                 fabs(pInstance->z) <= tolerance;
        }
    else if (pVec2)
        {
        result = fabs(pVec2->x) <= tolerance &&
                 fabs(pVec2->y) <= tolerance &&
                 fabs(pVec2->z) <= tolerance;
        }
    else /* Both are null. Call them equal. */
        result = true;

    return  result;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
