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
Public GEOMDLLIMPEXP int      bspcurv_constructInterpolationWithKnots
(
MSInterpolationCurve    *pCurve,        /* OUT     interpolation curve */
DPoint3d                *pPoints,       /* IN      interpolation points */
DPoint3d                *pStartTan,     /* IN      not used if periodic */
DPoint3d                *pEndTan,       /* IN      not used if periodic */
double                  *pKnots,        /* IN      full knot vector */
InterpolationParam      *pParams
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_constructInterpolationCurve
(
MSInterpolationCurve    *pCurve,
DPoint3d        *inPts,             /* IN      points to be interpolated */
int             numPts,             /* IN      number of points */
bool            remvData,           /* IN      true = remove coincident points */
double          remvTol,            /* IN      for finding coincident pts or closed curve */
DPoint3d        *endTangents,       /* IN      normalized end tangents or NULL */
bool            closedCurve,        /* IN      if true, closed Bspline is created */
bool            chordLenKnots,      /* IN      T/F: chordlen/uniform knots (!inParams, closed spline) */
bool            colinearTangents,   /* IN      T: ensure colinear computed end tangents (zero/NULL endTangent(s), geometrically closed spline) */
bool            chordLenTangents,   /* IN      T/F: scale endTangent by chordlen/bessel (nonzero endTangent, open spline) */
bool            naturalTangents     /* IN      T/F: compute natural/bessel endTangent (zero/NULL endTangent, open spline) */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     mdlBspline_removeCoincidePoint
(
DPoint3d        *outPts,
int             *numOut,
DPoint3d        *inPts,
int             numIn,
double          tol
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     mdlBspline_removeCoincidePointExt
(
DPoint3d        *outPts,        /* compressed points */
double          *outParams,     /* compressed params (or NULL) */
int             *numOut,        /* size of both output arrays */
const DPoint3d  *inPts,         /* point array to compress */
const double    *inParams,      /* param associated with each pt (or NULL) */
int             numIn,          /* size of both input arrays */
double          tol             /* min squared distance between different points */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int  mdlBspline_extractInterpolationTangentPoints
(
DPoint3d                *pStartTangentPoint,
DPoint3d                *pEndTangentPoint,
MSInterpolationCurve    *pCurve
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      mdlBspline_convertInterpolationToBspline
(
MSBsplineCurve          *pCurve,
MSInterpolationCurve    *pFitCurve
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      mdlBspline_openInterpolationCurveWithHeapDescr
(
MSInterpolationCurve    *outCurve,
MSInterpolationCurve    *inCurve,
double                  u,          /* IN      currently only allows u=0.0 */
void                 *pHeapDescr
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      mdlBspline_openInterpolationCurve
(
MSInterpolationCurve    *outCurve,
MSInterpolationCurve    *inCurve,
double                  u           /* IN      currently only allows u=0.0 */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      mdlBspline_reverseInterpolationCurve
(
MSInterpolationCurve  *inCurve,
MSInterpolationCurve  *outCurve
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      mdlBspline_closeInterpolationCurveWithHeapDescr
(
MSInterpolationCurve    *outCurve,
MSInterpolationCurve    *inCurve,
void                 *pHeapDescr
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      mdlBspline_closeInterpolationCurve
(
MSInterpolationCurve    *outCurve,
MSInterpolationCurve    *inCurve
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt    mdlBspline_computeChordLengthKnotVector
(
double*         pKnots,         /* full knot vector */
const DPoint3d* pPoles,         /* weighted if rational */
const double*   pWeights,       /* for rational (or NULL) */
int             numPoles,
int             order,
bool            bPeriodic       /* if true, it is assumed that first/last pole are NOT equal */
);

/*---------------------------------------------------------------------------------**//**
* Handles data with sharp corners.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt bspconv_cubicFitAndReducePoints
(
MSBsplineCurve      *curveP,
DPoint3d            *inPointP,
int                 nPoints,
double              tolerance
);

/*---------------------------------------------------------------------------------**//**
* Handles data with sharp corners.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt bspconv_cubicFitAndReducePoints
(
CurveVectorPtr      &curves,
DPoint3d            *inPointP,
int                 nPoints,
double              tolerance
);
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_transformInterpolationCurve
(
MSInterpolationCurve    *outCurveP,         /* OUT     transformed curve */
MSInterpolationCurve    *inCurveP,          /* IN      input curve */
Transform const         *transformP         /* IN      transform */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  int     bspcurv_ellipticalArc
(
MSBsplineCurve  *curveP,
DPoint3d        *centerP,
double          x1,
double          x2,
double          start,
double          sweep,
RotMatrix       *rotMatrixP,
bool            close
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_spiral
(
MSBsplineCurve  *curve,     /* OUT     spiral bspline curve */
double          iRad,       /* IN      initial Radius */
double          fRad,       /* IN      final Radius */
double          sweep,              /* IN      angle of spiral */
DPoint3d        *startPt,
DPoint3d        *tangentPt,
DPoint3d        *directionPt
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_helix
(
MSBsplineCurve  *curve,                /* OUT     spiral bspline curve */
double          iRad,                  /* IN      initial Radius */
double          fRad,                  /* IN      final Radius */
double          pitchValue,            /* IN      pitch height or number */
DPoint3d        *startPt,
DPoint3d        *axis1,
DPoint3d        *axis2,
int             valueIsHeight
);

/*---------------------------------------------------------------------------------**//**
* Return a curve that blends from inCurve1 at param1 to inCurve2 at param2 with specified degree of continuity
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_blendCurve
(
MSBsplineCurve  *blend,        /* OUT     resulting curve */
MSBsplineCurve  *inCurve1,     /* IN      curve to blend */
MSBsplineCurve  *inCurve2,     /* IN      curve to blend */
double          param1,        /* IN      blend curve1 from here */
double          param2,        /* IN      blend curve2 to here */
int             degree,        /* IN      degree of continuity desired */
double          mag1,          /* IN      relative magnitude of blend  */
double          mag2           /*    tangent between 0.0 and 1.0 */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_leastSquaresToCurve
(
MSBsplineCurve  *curve,
double          *avgDistance,
double          *maxDistance,
DPoint3d        *pnts,
double          *uValues,
int             numPnts
);

END_BENTLEY_GEOMETRY_NAMESPACE

