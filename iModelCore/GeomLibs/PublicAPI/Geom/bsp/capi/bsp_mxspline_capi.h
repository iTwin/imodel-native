/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*-----------------------------------------------------------------
@description Initialize an MSBspline using MX-style conditions, i.e.
(a) both bearing and curvature at ends
(b) knots spaced to match arc length.
@param pCurve OUT curve
@param pXYZ IN points to interpolate.  All spline z values are z from the first point.
@param numXYZ IN number of points.
@param radiansA IN start point bearing (into curve) in radians.
@param radiusA IN start point radius of curvature.  Positive is to left of curve.
             zero radius means straight line.
@param radiansA IN end point bearing (outbounds from curve!!!) in radians.
@param radiusA IN end point radius of curvature.  Positive is to left of curve.
             zero radius means straight line.
@returns ERROR if unable to fit.
-------------------------------------------------------------------*/
Public StatusInt GEOMDLLIMPEXP bspcurv_interpolateXYWithBearingAndRadius
(
MSBsplineCurve *pCurve,
DPoint3dP pXYZ,
int numXYZ,
double radiansA,
double radiusA,
double radiansB,
double radiusB
);

/*-----------------------------------------------------------------
@description Initialize an MSBspline using MX-style conditions, i.e.
(a) both bearing and curvature at ends
(b) knots spaced to match arc length.
@param pCurve OUT curve
@param pXYZ IN points to interpolate.  All spline z values are z from the first point.
@param numXYZ IN number of points.
@param radiansA IN start point bearing (into curve) in radians.
@param bApplyBearingA IN true if start bearing is active.
@param radiusA IN start point radius of curvature.  Positive is to left of curve.
             zero radius means straight line.
@param bApplyBearingA IN true if start radius is active.
@param radiansA IN end point bearing (outbounds from curve!!!) in radians.
@param bApplyBearingA IN true if end bearing is active.
@param radiusA IN end point radius of curvature.  Positive is to left of curve.
             zero radius means straight line.
@param bApplyBearingA IN true if end radius is active.
@returns ERROR if unable to fit.
-------------------------------------------------------------------*/
Public StatusInt GEOMDLLIMPEXP bspcurv_interpolateXYWithBearingAndRadiusExt
(
MSBsplineCurve *pCurve,
DPoint3dP pXYZ,
int numXYZ,
bool    bApplyBearingA,
double radiansA,
bool    bApplyRadiusA,
double radiusA,
bool    bApplyBearingB,
double radiansB,
bool    bApplyRadiusB,
double radiusB
);


// 2008 Spiral types for Civil transitions
#define SPIRALTYPE_TransitionClothoid    10
#define SPIRALTYPE_TransitionBloss       11
#define SPIRALTYPE_TransitionBiQuadratic 12
#define SPIRALTYPE_TransitionCosine      13
#define SPIRALTYPE_TransitionSine        14


/*-----------------------------------------------------------------
@description Initialize an MSBspline using spiral start/end data and a placement.
@param pCurve OUT returned curve
@param spiralType IN type code for spiral curve.
@param pOrigin IN curve start point.
@param pAxes IN coordinate directions for placement in space.
@param startBearingRadians IN angle of departure from start point.
@param startRadius IN radius of curvature at start point (0 for straight line)
@param endBearingRadians IN angle of departure from end point.
@param endRadius IN radius of curvature at end point (0 for straight line)
@param pExtraParams IN array of additional parameters specific to spiral type.
------------------------------------------------------------------------*/
Public StatusInt GEOMDLLIMPEXP bspcurv_curveFromTransitionSpiralBearingAndCurvature
(
MSBsplineCurveP pCurve,
int spiralType,
DPoint3dP pOrigin,
RotMatrixP pFrame,
double startBearingRadians,
double startRadius,
double endBearingRadians,
double endRadius,
double *pExtraParams
);

/*-----------------------------------------------------------------
@description Initialize an MSBspline using spiral start/end data and a placement.
@param pCurve OUT returned curve
@param spiralType IN type code for spiral curve.
@param pOrigin IN curve start point.
@param pAxes IN coordinate directions for placement in space.
@param startBearingRadians IN angle of departure from start point.
@param startRadius IN radius of curvature at start point (0 for straight line)
@param endBearingRadians IN angle of departure from end point.
@param endRadius IN radius of curvature at end point (0 for straight line)
@param pExtraParams IN array of additional parameters specific to spiral type.
------------------------------------------------------------------------*/
Public StatusInt GEOMDLLIMPEXP bspcurv_curveFromTransitionSpiralBearingCurvatureBearingLength
(
MSBsplineCurveP pCurve,
int spiralType,
DPoint3dP pOrigin,
RotMatrixP pFrame,
double startBearingRadians,
double startRadius,
double endBearingRadians,
double length,
double *pExtraParams
);

/*-----------------------------------------------------------------
@description Initialize an MSBspline using spiral start/end data and a placement.
@param pCurve OUT returned curve
@param spiralType IN type code for spiral curve.
@param pOrigin IN curve start point.
@param pAxes IN coordinate directions for placement in space.
@param startBearingRadians IN angle of departure from start point.
@param startRadius IN radius of curvature at start point (0 for straight line)
@param endBearingRadians IN angle of departure from end point.
@param length IN length of curve
@param pExtraParams IN array of additional parameters specific to spiral type.
------------------------------------------------------------------------*/
Public StatusInt GEOMDLLIMPEXP bspcurv_curveFromTransitionSpiralBearingCurvatureCurvatureLength
(
MSBsplineCurveP pCurve,
int spiralType,
DPoint3dP pOrigin,
RotMatrixP pFrame,
double startBearingRadians,
double startRadius,
double endBearingRadians,
double length,
double *pExtraParams
);


//struct DSpiral2dBase;
/*-----------------------------------------------------------------
@description Initialize an MSBspline using virtual spiral object
@param pCurve OUT returned curve
@param pSpiral IN spiral computational object
@param pOrigin IN curve start point.
@param pAxes IN coordinate directions for placement in space.
@param maxStrokeLength IN max length of stroke interval.   Recommended 10 (in meter world)
------------------------------------------------------------------------*/
Public StatusInt GEOMDLLIMPEXP bspcurv_curveFromDSpiral2dBase
(
MSBsplineCurveP pCurve,
DSpiral2dBase *pSpiral,
DPoint3dP pOrigin,
RotMatrixP pFrame,
double maxStrokeLength = 10.0
);

/// <summary>
/// Create a bspline curve for specified transition spiral type and
/// start/end bearings, clipped as a fractional range of an (extensible) parent spiral.
/// </summary>
/*-----------------------------------------------------------------
@description Initialize an MSBspline using a fractional mapped interval of a virtual spiral object
@param pCurve OUT returned curve
@param fractionA IN start fraction of mapped interval.
@param fractionB IN end fraction of mapped interval.
@param pSpiral IN spiral computational object
@param pOrigin IN curve start point.
@param pAxes IN coordinate directions for placement in space.
@param maxStrokeLength IN max length of stroke interval.   Recommended 10 (in meter world)
------------------------------------------------------------------------*/
Public StatusInt GEOMDLLIMPEXP bspcurv_curveFromDSpiral2dBaseInterval
(
MSBsplineCurveP pCurve,
DSpiral2dBase *pSpiral,
double fractionA,
double fractionB,
DPoint3dP pOrigin,
RotMatrixP pFrame,
double maxStrokeLength = 10.0
);

//! Evaluate exactly numPoints offset points.
//! Construct a bspline that interpolates these points.
//!  End condition is the MX spline condition (see bspcurv_interpolateXYWithBearingAndRadius)
//!    **** bearing (xy angle) at start and end match the base curve
//!    **** radius at start and end are base curve radius of curvature ajusted by the offset radius.
//! @param [in] offset0 offset at start
//! @param [in] offset1 offset at end
//! @param [in] numPoints number of points to offset and pass through.
//! @remark points are spaced evenly in the parameter space
//! @remakr offset distnace varies proportional to parameter.
Public StatusInt GEOMDLLIMPEXP bspcurv_interpolatedOffsetXY
(
MSBsplineCurveR offsetCurve,
MSBsplineCurveCR sourceCurve,
double offset0,
double offset1,
int numPoints
);

//! Evaluate an approximated offset for source curve..
//! @param [in] offset0 offset at start
//! @param [in] offset1 offset at end
//! @param [in] numPoles number of poles of offset curve.
//! @param [in] geomTol sample tolerance.
//! @remakr offset distnace varies proportional to parameter.
Public StatusInt GEOMDLLIMPEXP bspcurv_approximateOffsetXY
(
MSBsplineCurveR offsetCurve,
MSBsplineCurveCR sourceCurve,
double offset0,
double offset1,
int numPoles,
double geomTol
);

//! Evaluate an approximated offset for source curve..
//! @param [in] offset0 offset at start
//! @param [in] offset1 offset at end
//! @param [in] numPoints number of points to offset and pass through.
//! @remark points are spaced evenly in the parameter space
//! @remakr offset distnace varies proportional to parameter.
Public StatusInt GEOMDLLIMPEXP bspcurv_offsetEllipseXY
(
MSBsplineCurveR offsetCurve,
DEllipse3dCR ellipse,
double offset0,
double offset1,
int numPoints
);

//! Evaluate a set of parameters for point offset.
//! @param [in] offset0 offset at start
//! @param [in] offset1 offset at end
//! @param [in] numPerKnotSpan Number of parameter to be inserted in each knot interval.
Public StatusInt GEOMDLLIMPEXP bspcuv_interpolatedOffSetXYSubdivide
(
MSBsplineCurveR offsetCurve,
MSBsplineCurveCR sourceCurve,
double offset0,
double offset1,
int numPerKnotSpan
);

END_BENTLEY_GEOMETRY_NAMESPACE
