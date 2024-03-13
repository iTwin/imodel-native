/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   bspcurv_fixRelTol
(
double          relTol
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_sweptArea
(
double          *fP,                /* OUT     integral */
double          *errorAchievedP,     /* OUT     the absolute error achieved, i.e.,
                                          abs(arcLengthP - trueArcLength) */
double          startFraction,      /* IN      strating parameter */
double          endFraction,        /* IN      ending parameter */
MSBsplineCurve  *curveP,            /* IN      input curve */
const DPoint3d  *originP,           /* IN      origin for sweeping ray */
double          relativeTol         /* IN      relative tolerance for integration
                                          using Simpson's Rule, i.e.,
                                          abs(errorAchivedP) / arcLengthP OUT     relativeTol */
);

/*-----------------------------------------------------------------*//**
* @param arclengthP OUT     arc length
* @param centroidP OUT     centroid
* @param startParam IN      start parameter of effective interval.
* @param endParam IN      end parameter of effective interval.
* @param curveP IN      curve
* @param relativeTol relative tolerance for integrations.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_pathCentroid
(
double          *arcLengthP,
DPoint3d        *centroidP,
double          startFraction,
double          endFraction,
MSBsplineCurve  *curveP,
double          relativeTol
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_sweptAngle
(
double          *fP,                /* OUT     integral */
double          *errorAchievedP,     /* OUT     the absolute error achieved, i.e.,
                                          abs(arcLengthP - trueArcLength) */
double          startFraction,      /* IN      strating parameter */
double          endFraction,        /* IN      ending parameter */
MSBsplineCurve  *curveP,            /* IN      input curve */
const DPoint3d  *originP,           /* IN      origin for sweeping ray */
double          relativeTol         /* IN      relative tolerance for integration
                                          using Simpson's Rule, i.e.,
                                          abs(errorAchivedP) / arcLengthP OUT     relativeTol */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_arcLength
(
double          *fP,                /* OUT     integral */
double          *errorAchievedP,     /* OUT     the absolute error achieved, i.e.,
                                          abs(arcLengthP - trueArcLength) */
double          startFraction,      /* IN      strating parameter */
double          endFraction,        /* IN      ending parameter */
MSBsplineCurve  *curveP,            /* IN      input curve */
const DPoint3d  *originP,           /* IN      origin for sweeping ray */
double          relativeTol         /* IN      relative tolerance for integration
                                          using Simpson's Rule, i.e.,
                                          abs(errorAchivedP) / arcLengthP OUT     relativeTol */
);

/*--------------------------------------------------------------------*//**
@description Compute the intersection of a bspline curve with a triangle.
@param curveP IN curve
@param startFraction IN start fraction for curve portion to be considered
@param endFraction IN end fraction for curve portion to be considered
@param pXYZ0 IN triangle vertex
@param pXYZ1 IN triangle vertex
@param pXYZ2 IN triangle vertex
@param F IN function to call to accumulate results
        Call is F (curveP, startFraction, endFraction, rootFraction, pUserData0, pUserData1);
@param pUserData0 IN arg for F
@param pUserData1 IN arg for F
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void  bspcurv_intersectTriangle
(
MSBsplineCurve  *curveP,            /* IN      input curve */
double          startFraction,      /* IN      starting fractional parameter */
double          endFraction,        /* IN      ending fractional parameter */
DPoint3d        *pXYZ0,
DPoint3d        *pXYZ1,
DPoint3d        *pXYZ2,
MSBsplineCurve_AnnounceParameter F,
void            *pUserData0,
void            *pUserData1
);
#ifdef __cplusplus
/*--------------------------------------------------------------------*//**
@description Compute the intersection of a bspline curve with a triangle.
@param curveP IN curve
@param startFraction IN start fraction for curve portion to be considered
@param endFraction IN end fraction for curve portion to be considered
@param pXYZ0 IN triangle vertex
@param pXYZ1 IN triangle vertex
@param pXYZ2 IN triangle vertex
@param pFractionArray IN OUT intersect parameters wrt curve are appended to this array.
@param pXYZArray IN OUT intersection xyz appended to this array.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void  bspcurv_collectTriangleIntersections
(
MSBsplineCurve  *curveP,            /* IN      input curve */
double          startFraction,      /* IN      starting fractional parameter */
double          endFraction,        /* IN      ending fractional parameter */
DPoint3d        *pXYZ0,
DPoint3d        *pXYZ1,
DPoint3d        *pXYZ2,
bvector<double> *pFractionArray,
EmbeddedDPoint3dArray *pXYZArray
);
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_convertDPoint4dBezierToCurve
(
MSBsplineCurve      *pCurve,
const DPoint4d      *pPoles,
int                 order
);

#ifdef __cplusplus
/*---------------------------------------------------------------------------------**//**
@description Sweep a base curve along a vector until it is on a target plane.
    Optionally determine the parameter intervals at which projected curve is within the
    bounded sweep.
@param pBaseCurve IN base curve.
@parma pTargetCurve OUT curve on target plane.
@param pStartEndParams OUT array of start-end pairs for pTargetCurve segments that are within the sweep.
@param pRuleLineParams OUT array of base curve parameters at which the plane exactly passes through the rule
        line.  (i.e. the sweep direction is in the plane)
@param pSweepVector IN (non-unit) vector with complete extent of sweep.
@param pPlane IN target plane.
@returns  SUCCESS if results computed.
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bspcurv_sweepToPlane
(
MSBsplineCurve const *pBaseCurve,
MSBsplineCurve  *pTargetCurve,
bvector<double> *pStartEndParams,
bvector<double> *pRuleLineParams,
DVec3dCP        pSweepVector,
DPlane3dCP      pPlane
);
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bspcurv_computeLength
(
MSBsplineCurve  *curveP,
double          tolerance
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  void                bspcurv_intersectPlane
(
MSBsplineCurve  *curveP,            /* IN      input curve */
double          startFraction,      /* IN      starting fractional parameter */
double          endFraction,        /* IN      ending fractional parameter */
DPoint4d        *pPlaneCoff,        /* IN      plane coefficients */
double          absTol,             /* IN      absolute tolerance for double roots. */
MSBsplineCurve_AnnounceParameter F,
void            *pUserData0,
void            *pUserData1
);
#ifdef __cplusplus
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public void   cb_collectParameter
(
MSBsplineCurveP         pCurve,
double                  param0,
double                  param1,
double                  currParam,
/*bvector<double>&    pParamArray, */
bvector<double> *pParamArray,
void*                   pVoid
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  StatusInt     bspcurv_curvePlaneIntersects
(
/*bvector<double>&    pParamArray, */
bvector<double>     *pParamArray,
DPoint3dCP              pOrigin,
DPoint3dCP              pNormal,
MSBsplineCurveP         pCurve,
double                  tolerance
);
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  void                bspcurv_intersectPlaneExt
(
MSBsplineCurve  *curveP,            /* IN      input curve */
double          startFraction,      /* IN      starting fractional parameter */
double          endFraction,        /* IN      ending fractional parameter */
DPoint4d        *pPlaneCoff,        /* IN      plane coefficients */
double          absTol,             /* IN      absolute tolerance for double roots. */
MSBsplineCurve_AnnounceParameter F,
void            *pUserData0,
void            *pUserData1
);


END_BENTLEY_GEOMETRY_NAMESPACE

