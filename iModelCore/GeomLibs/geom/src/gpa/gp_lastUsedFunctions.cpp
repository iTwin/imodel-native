/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/gpa/gp_lastUsedFunctions.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*-----------------------------------------------------------------*//**
* @description Store origin and unnormalized vector.
* @param pPlane <= initialized plane.
* @param pOrigin => origin point
* @param pNormal => normal vector
* @group "DPlane3d Initialization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPlane3d_initFromOriginAndNormal

(
DPlane3dP pPlane,
DPoint3dCP pOrigin,
DVec3dCP pNormal
)
    {
    pPlane->origin = *pOrigin;
    pPlane->normal = *pNormal;
    }

/*-----------------------------------------------------------------*//**
* @description Return the plane as a DPoint4d.
* @param pPlane => plane structure with origin, normal
* @param pHPlane <= 4D plane coefficients
* @see bsiDPlane3d_initFromDPoint4d
* @group "DPlane3d Queries"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void      bsiDPlane3d_getDPoint4d

(
DPlane3dCP pPlane,
DPoint4dP pHPlane
)
    {
    bsiDPlane3d_getImplicitPlaneCoefficients
                            (
                            pPlane,
                            &pHPlane->x,
                            &pHPlane->y,
                            &pHPlane->z,
                            &pHPlane->w
                            );
    pHPlane->w = -pHPlane->w;
    }


/*-----------------------------------------------------------------*//**
* @description Convert the plane to implicit coeffcients ax+by+cz=d.
* @remarks WARNING: Check your usage.  It is about equally common to write the plane equation with
*       negated d, i.e. ax+by+cz+d=0.  If so, pass in (a,b,c,-d).
* @param pPlane => plane structure with origin, normal
* @param pA <= 4D plane x-coefficient
* @param pB <= 4D plane y-coefficient
* @param pC <= 4D plane z-coefficient
* @param pD <= 4D plane constant coefficient
* @see bsiDPlane3d_initFromImplicitPlaneCoefficients
* @group "DPlane3d Initialization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPlane3d_getImplicitPlaneCoefficients

(
DPlane3dCP pPlane,
double      *pA,
double      *pB,
double      *pC,
double      *pD
)
    {
    *pA = pPlane->normal.x;
    *pB = pPlane->normal.y;
    *pC = pPlane->normal.z;
    *pD = bsiDPoint3d_dotProduct (&pPlane->origin, &pPlane->normal);
    }

/*---------------------------------------------------------------------------------**//**
* Get the parameter range as start/sweep pairs.
* @indexVerb parameterRange
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiDDisk3d_getScalarNaturalParameterSweep

(
DDisk3dCP pInstance,
double          *pRadius0,
double          *pRadiusDelta,
double          *pAngle0,
double          *pAngleSweep
)
    {
    *pRadius0       = pInstance->parameterRange.low.x;
    *pRadiusDelta   = pInstance->parameterRange.high.x - *pRadius0;

    *pAngle0        = pInstance->parameterRange.low.y;
    *pAngleSweep    = pInstance->parameterRange.high.y - *pAngle0;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
