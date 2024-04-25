/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! Given two control points (xyzw) of an xyw curve, compute and scale the cross product
//! of the xyw parts as used in Sederberg's implicitization.
//! @param pProduct IN      plane coefficients, with z==0, scaled by iCn jCn
//! @param pPointi IN      i'th pole
//! @param pPointj IN      j'th pole
//! @param i       IN      pole index
//! @param j       IN      pole index
//! @param order   IN      curve order.
//!
Public GEOMDLLIMPEXP void     bsiBezierDPoint4d_lineProductXYW
(
DPoint4d    *pProduct,
const DPoint4d    *pPointi,
const DPoint4d    *pPointj,
int         i,
int         j,
int         order
);

//!
//! Given two section curves on a surface, compute and scale the cross product
//! of stringers of designated poles as used in Sederberg's implicitization.
//! @param pProduct IN      poles for plane coefficients, with z==0, scaled by iCn jCn
//! @param pPointi IN      i'th pole stringer
//! @param pPointj IN      j'th pole stringer
//! @param order IN      order of both i, j.
//! @param stride IN      stride between adjacent DPoint4d's within each stringer.
//! @param i       IN      pole index
//! @param j       IN      pole index
//! @param order   IN      curve order.
//!
Public GEOMDLLIMPEXP bool     bsiBezierDPoint4d_stringerLineProductXYW
(
DPoint4d    *pProductPoles,
int         *pProductOrder,
int         maxProductOrder,
const DPoint4d    *pPointPolesi,
const DPoint4d    *pPointPolesj,
int   stringerOrder,
int   stride,
int         stringerI,
int         stringerJ,
int         lineProductOrder
);

//!
//! Evaluate poles the psuedo tangent function for 1, 2, or 3 dimensions of a possibly homogeneous curve.
//! The pseudo tangent of a nonrational curve is "just" the simple derivative of the curve
//!   components. (e.g. the equation above with w(t) identically 1, w'(t) identically 0)
//! For a rational curve, the true (properly scaled) tangent vector x component is
//!               (x'(t) w(t) - x(t) w'(t)) / w(t)^2
//! If only the direction, but not the magnitude, is of interest, many problems (e.g. range,
//!   nearest point) can be formulated ignoring the division by w(t)^2.  The numerator
//!   term is the pseudo tangent.
//! @param pPoleOut       OUT     tangent poles, pure cartesian vector components.
//! @param pOrderOut      OUT     order of the tangent.
//! @param maxOrderOut    IN      maximum allowed order of the tangent.  Be prepared for 2*order - 1 !!!
//! @param pPoleIn        IN      curve poles, full DPoint4d with possibly constant weights
//! @param order          IN      curve order
//! @param numDim         IN      1, 2, or 3 -- number of derivative components desired. Unused
//!                           components of the DPoint3d outputs are zero.
//! @return true if the pseudo tangent was computed in the allowed order.
//!
Public GEOMDLLIMPEXP bool        bsiBezierDPoint4d_pseudoTangent
(
DPoint3d        *pPoleOut,
int             *pOrderOut,
int             maxOrderOut,
const DPoint4d  *pPoleIn,
int             order,
int             numDim
);

//!
//! Evaluate poles of a function which evaluates to a vector from a fixed point towards
//! points on the curve.
//! For a rational curve, the true (properly scaled) difference vector x component is
//!               (x(t) a.w - a.x w(t)) / (w(t) a.w)
//! If the curve weight w(t) is identically 1, this simplifies to
//!               (x(t) a.w - a.x) / a.w
//! If only the direction, but not the magnitude, is of interest, many problems (e.g.
//!   nearest point) can be formulated ignoring the division .  The numerator
//!   term (x(t) a.w - a.x w(t)) is the pseudo tangent; the function uses the simplified
//!   form if w(t) is one.
//!
//! If the a.w is zero, the "vector" part of A is returned as a constant (order 1!!!) bezier.
//!
//! @param pPoleOut       OUT     tangent poles, pure cartesian vector components.
//! @param pOrderOut      OUT     order of the tangent.
//! @param maxOrderOut    IN      maximum allowed order of the tangent.  Be prepared for 2*order - 1 !!!
//! @param pPoleIn        IN      curve poles, full DPoint4d with possibly constant weights
//! @param order          IN      curve order
//! @param pPointA        IN      fixed point.
//! @param numDim         IN      1, 2, or 3 -- number of tangent components desired. Unused
//!                           components of the DPoint3d outputs are zero.
//! @return true if the pseudo tangent was computed in the allowed order.
//!
Public GEOMDLLIMPEXP bool        bsiBezierDPoint4d_pseudoVectorFromDPoint4d
(
DPoint3d        *pPoleOut,
int             *pOrderOut,
int             maxOrderOut,
const DPoint4d  *pPoleIn,
int             order,
const DPoint4d  *pPointA,
int             numDim
);

//!
//! Compute the (exact) 3d range of a homogeneous bezier
//!
//!
Public GEOMDLLIMPEXP void     bsiBezierDPoint4d_getDRange3d
(
        DRange3d    *pRange,
const   DPoint4d    *pPoles,
        int         order
);

//!
//! Compute the (exact) 3d range of a homogeneous bezier
//!
//!
Public GEOMDLLIMPEXP void     bsiBezierDPoint3d_getDRange3d
(
 DRange3dP  pRange,
DPoint3dCP  pPoles,
int         order
);

//!
//! Return a single number which represents the numeric data range in the bezier,
//! using only the normalized xy parts.  This is useful as a maximum coordiante for tolerance
//! determination.
//!
//! @param pPoles IN      poles whose range is computed.
//! @param order  IN      number of poles.
//! @return a single double representing the normalized xy range of the poles.
//!
//!
Public GEOMDLLIMPEXP double   bsiBezierDPoint4d_getLargestCoordinateXY
(
const   DPoint4d    *pPoles,
        int         order
);

//!
//! Compute the arc length of a homogeneous bezier curve.
//!
//!
Public GEOMDLLIMPEXP double   bsiBezierDPoint4d_arcLength
(
const   DPoint4d    *pPoles,
        int         order,
        double      s0,
        double      s1
);

//!
//! Compute the arc length of a transformed homogeneous bezier curve.
//!
//!
Public GEOMDLLIMPEXP double   bsiBezierDPoint4d_arcLength
(
RotMatrixCP worldToLocal,
const   DPoint4d    *pPoles,
        int         order,
        double      s0,
        double      s1
);

//!
//! Compute point and tangent at an array of parameters.  Zero weight points are set to zero.
//! @param pX         OUT     array of points on curve.
//! @param pdX        OUT     array of tangent vectors.
//! @param pddX       OUT     array of second derivative vectors
//! @param pPoleArray IN      poles of spline curve
//! @param order      IN      spline order
//! @param pParam     IN      array of parameter values
//! @param numParam   IN      number of points to evaluate.
//!
Public GEOMDLLIMPEXP void    bsiBezierDPoint4d_evaluateDPoint3dArrayExt
(
        DPoint3d    *pX,
        DPoint3d    *pdX,
        DPoint3d    *pddX,
const   DPoint4d    *pPoleArray,
        int         order,
        double      *pParam,
        int         numParam
);

//!
//! Compute point and tangent at an array of parameters.  Zero weight points are set to zero.
//! @param pX         OUT     array of points on curve.
//! @param pdX        OUT     array of tangent vectors.
//! @param pddX       OUT     array of second derivative vectors
//! @param pPoleArray IN      poles of spline curve
//! @param order      IN      spline order
//! @param pParam     IN      array of parameter values
//! @param numParam   IN      number of points to evaluate.
//!
Public GEOMDLLIMPEXP void    bsiBezierDPoint4d_evaluateDPoint4dArrayExt
(
        DPoint4d    *pX,
        DPoint4d    *pdX,
        DPoint4d    *pddX,
const   DPoint4d    *pPoleArray,
        int         order,
        double      *pParam,
        int         numParam
);

//!
//! Compute point and tangent at equally spaced parameters parameters.  Zero weight points are set to zero.
//! @param pX         <= array of points on curve.
//! @param pdX        <= array of tangent vectors.
//! @param pddX       <= array of second derivative vectors
//! @param pPoleArray => poles of spline curve
//! @param order      => spline order
//! @param numParam   => number of (equally spaced) parameters to evaluate. AT LEAST 2 are evaluated.
//!
Public GEOMDLLIMPEXP void    bsiBezierDPoint4d_appendEvaluations
(
bvector<DPoint3d>*pX,
bvector<DVec3d>*pdX,
bvector<DVec3d>*pddX,
const   DPoint4d    *pPoleArray,
        int         order,
        size_t      numParam
);

//!
//! Compute point and tangent at an array of parameters.  Zero weight points are set to zero.
//! @param pX         OUT     array of points on curve.
//! @param pdX        OUT     array of tangent vectors.
//! @param pPoleArray IN      poles of spline curve
//! @param order      IN      spline order
//! @param pParam     IN      array of parameter values
//! @param numParam   IN      number of points to evaluate.
//!
Public GEOMDLLIMPEXP void    bsiBezierDPoint4d_evaluateDPoint3dArray
(
        DPoint3d    *pX,
        DPoint3d    *pdX,
const   DPoint4d    *pPoleArray,
        int         order,
        double      *pParam,
        int         numParam
);

//!
//! Find the Bezier curve point closest to a space point, measuring in
//! cartesian xy space only.  Only check points at or between the given parameters.
//! @param pClosePoint OUT     xyzw of closest point
//! @param pCloseParam OUT     parameter of closest point
//! @param pCloseDist2 OUT     squared xy distance to closest point
//! @param pPoleArray IN      array of curve poles
//! @param order      IN      curve order (number of poles, one more than degree)
//! @param xx         IN      x coordinate of space point
//! @param yy         IN      y coordinate of space point
//! @param s0         IN      minimum parameter to consider
//! @param s1         IN      maximum parameter to consider
//! @return true if a near point (possibly an endpoint) was computed.
//!
Public GEOMDLLIMPEXP bool        bsiBezierDPoint4d_closestXYPoint
(
        DPoint4d    *pClosePoint,
        double      *pCloseParam,
        double      *pCloseDist2,
const   DPoint4d    *pPoleArray,
        int         order,
        double      xx,
        double      yy,
        double      s0,
        double      s1
);

//!
//! Find the Bezier curve point closest to a space point, measuring in
//! cartesian xy space only.  Only check points at or between the given parameters.
//! @param pClosePoint OUT     xyzw of closest point
//! @param pCloseParam OUT     parameter of closest point
//! @param pCloseDist2 OUT     squared xy distance to closest point
//! @param pPoleArray IN      array of curve poles
//! @param order      IN      curve order (number of poles, one more than degree)
//! @param xx         IN      x coordinate of space point
//! @param yy         IN      y coordinate of space point
//! @param zz         IN      z coordinate of space point
//! @param s0         IN      minimum parameter to consider
//! @param s1         IN      maximum parameter to consider
//! @return true if a near point (possibly an endpoint) was computed.
//!
Public GEOMDLLIMPEXP bool        bsiBezierDPoint4d_closestPoint
(
        DPoint4d    *pClosePoint,
        double      *pCloseParam,
        double      *pCloseDist2,
const   DPoint4d    *pPoleArray,
        int         order,
        double      xx,
        double      yy,
        double      zz,
        double      s0,
        double      s1
);

//!
//! Find the points where the bezier its perpendicular to the (moving) line from
//! given fixed point.
//! @param pPoleArray IN      array of curve poles
//! @param order      IN      curve order (number of poles, one more than degree)
//! @param pFixedPoint IN      dropping perpendicular from here.
//! @param dimension  IN      2 for xyw, 3 for xyzw projection. Any other dimension returns false!!!
//!
Public GEOMDLLIMPEXP bool        bsiBezierDPoint4d_allPerpendicularsFromDPoint4d
(
        double      *pParamArray,
        DPoint4d    *pPointArray,
        int         *pNumOut,
        int         maxOut,
const   DPoint4d    *pPoleArray,
        int         order,
const   DPoint4d    *pFixedPoint,
        int         workDimension
);

//!
//! Find the points where the bezier its perpendicular to the (moving) line from
//! given fixed point.
//! @param pPoleArray IN      array of curve poles
//! @param order      IN      curve order (number of poles, one more than degree)
//! @param pFixedPoint IN      dropping perpendicular from here.
//! @param dimension  IN      2 for xyw, 3 for xyzw projection. Any other dimension returns false!!!
//! @param extend IN      true to use unbounded (extended) geometry when possible.
//!
Public GEOMDLLIMPEXP bool        bsiBezierDPoint4d_allPerpendicularsFromDPoint4dExt
(
        double      *pParamArray,
        DPoint4d    *pPointArray,
        int         *pNumOut,
        int         maxOut,
const   DPoint4d    *pPoleArray,
        int         order,
const   DPoint4d    *pFixedPoint,
        int         workDimension,
        bool        extend
);

//!
//! Find the points where the bezier intersects a plane.
//! @param pPoleArray IN      array of curve poles
//! @param order      IN      curve order (number of poles, one more than degree)
//! @param pPlaneCoffs IN      plane equation
//! @param workDimension  IN      unused.  (set z=0 in plane coefficients to get xy effects)
//! @param extend IN      true to use unbounded (extended) geometry when possible.
//!
Public GEOMDLLIMPEXP bool        bsiBezierDPoint4d_allDPlane4dIntersections
(
        double      *pParamArray,
        DPoint4d    *pPointArray,
        int         *pNumOut,
        int         maxOut,
const   DPoint4d    *pPoleArray,
        int         order,
const   DPoint4d    *pPlaneCoffs,
        int         workDimension,
        bool        extend
);

//!
//! Find the points where the bezier tangent is small.  Points considered are
//! (1) both endpoints and (2) all extrema of the (squared) tangent vector.
//! @param pPoleArray IN      array of curve poles
//! @param order      IN      curve order (number of poles, one more than degree)
//! @param pFixedPoint IN      dropping perpendicular from here.
//! @param dimension  IN      2 for xyw, 3 for xyzw projection. Any other dimension returns false!!!
//! @param relTol IN      relative tolerance to consider a tangent magnitude small, as compared
//!               to large tangent magnitudes elsewhere on this bezier.
//!
Public GEOMDLLIMPEXP bool        bsiBezierDPoint4d_allNearCusps
(
        double      *pParamArray,
        DPoint4d    *pPointArray,
        int         *pNumOut,
        int         maxOut,
const   DPoint4d    *pPoleArray,
        int         order,
        int         workDimension,
        double      relTol
);

//!
//! Find the points where the bezier its tangent to the (moving) line from
//! given fixed point.
//! @param pPoleArray IN      array of curve poles
//! @param order      IN      curve order (number of poles, one more than degree)
//! @param pFixedPoint IN      dropping perpendicular from here.
//! @param dimension  IN      2 for xyw, 3 for xyzw projection. Any other dimension returns false!!!
//!
Public GEOMDLLIMPEXP bool        bsiBezierDPoint4d_allTangentsFromDPoint4d
(
        double      *pParamArray,
        DPoint4d    *pPointArray,
        int         *pNumOut,
        int         maxOut,
const   DPoint4d    *pPoleArray,
        int         order,
const   DPoint4d    *pFixedPoint,
        int         workDimension
);

//!
//! Find the points where the bezier its tangent to the (moving) line from
//! given fixed point.
//! @param pPoleArray IN      array of curve poles
//! @param order      IN      curve order (number of poles, one more than degree)
//! @param pFixedPoint IN      dropping perpendicular from here.
//! @param dimension  IN      2 for xyw, 3 for xyzw projection. Any other dimension returns false!!!
//! @param extend IN      true to use unbounded (extended) geometry when possible.
//!
Public GEOMDLLIMPEXP bool        bsiBezierDPoint4d_allTangentsFromDPoint4dExt
(
        double      *pParamArray,
        DPoint4d    *pPointArray,
        int         *pNumOut,
        int         maxOut,
const   DPoint4d    *pPoleArray,
        int         order,
const   DPoint4d    *pFixedPoint,
        int         workDimension,
        bool        extend
);

//!
//! Find the Bezier curve point closest to a space point.  Only check points at or
//!                   between the given parameters.
//! @param pClosePoint OUT     xyzw of closest point
//! @param pCloseParam OUT     parameter of closest point
//! @param pCloseDist2 OUT     squared xy distance to closest point
//! @param pPoleArray IN      array of curve poles
//! @param pWeightArray IN      array of curve weights.  May be null.
//! @param order      IN      curve order (number of poles, one more than degree)
//! @param xx         IN      x coordinate of space point
//! @param yy         IN      y coordinate of space point
//! @param zz         IN      z coordinate of space point
//! @param s0         IN      minimum parameter to consider
//! @param s1         IN      maximum parameter to consider
//! @return true if a near point (possibly an endpoint) was computed.
//!
Public GEOMDLLIMPEXP bool        bsiBezierDPoint3d_closestPoint
(
        DPoint3d    *pClosePoint,
        double      *pCloseParam,
        double      *pCloseDist2,
const   DPoint3d    *pPoleArray,
const   double      *pWeightArray,
        int         order,
        double      xx,
        double      yy,
        double      zz,
        double      s0,
        double      s1
);

//!
//! Find the Bezier curve point closest to a space point.  Only check points at or
//!                   between the given parameters.
//! @param pClosePoint OUT     xyzw of closest point
//! @param pCloseParam OUT     parameter of closest point
//! @param pCloseDist2 OUT     squared xy distance to closest point
//! @param pPoleArray IN      array of curve poles
//! @param pWeightArray IN      array of curve weights.  May be null.
//! @param order      IN      curve order (number of poles, one more than degree)
//! @param xx         IN      x coordinate of space point
//! @param yy         IN      y coordinate of space point
//! @param s0         IN      minimum parameter to consider
//! @param s1         IN      maximum parameter to consider
//! @return true if a near point (possibly an endpoint) was computed.
//!
Public GEOMDLLIMPEXP bool        bsiBezierDPoint3d_closestXYPoint
(
        DPoint3d    *pClosePoint,
        double      *pCloseParam,
        double      *pCloseDist2,
const   DPoint3d    *pPoleArray,
const   double      *pWeightArray,
        int         order,
        double      xx,
        double      yy,
        double      s0,
        double      s1
);

//!
//! Find the inflection points of a (rational) Bezier curve, ignoring the
//! z-coordinate.  Caller ensures that both output arrays have length at
//! least 3*order - 9.
//!
//! @param pInflectPoints OUT     array of inflection points (or null)
//! @param pInflectParams OUT     array of parameters of inflection points (or null)
//! @param pPoleArray     IN      array of curve poles
//! @param order          IN      curve order (number of poles, one more than degree)
//! @return number of isolated inflection points found or -1 if invalid order
//!
Public GEOMDLLIMPEXP int         bsiBezierDPoint4d_inflectionPointsXY
(
        DPoint4d    *pInflectPoints,
        double      *pInflectParams,
const   DPoint4d    *pPoleArray,
        int         order
);

//!
//! Find the inflection points of a (rational) 3D Bezier curve.
//! Caller ensures that both output arrays have length at least 3*order - 9.
//! If the poles are only 2D, it is more efficient to use
//! <A HREF="#inflectionPointsXY">inflectionPointsXY</A>.
//!
//! @param inflectPoints OUT     array of inflection points, with curve parameter in the "a" field.
//! @param poles     IN      array of curve poles
//! @see #inflectionPointsXY
//!
Public GEOMDLLIMPEXP void         bsiBezierDPoint4d_inflectionPoints
(
bvector<GraphicsPoint> inflectionPoints,
bvector<DPoint4d> poles
);


//!
//! Initializes the data structure shared between successive calls to
//! <A HREF="#extractNextBezierFromBspline">extractNextBezierFromBspline</A>,
//! which can be used in a loop to extract Bezier segments corresponding to the
//! given B-spline curve.
//! <P>
//! The knot sequence, if given, is assumed to be valid, i.e., it is
//! <UL><LI> nondecreasing,
//!     <LI> has no knot of multiplicity greater than order,and
//!     <LI> satisfies periodicity (if closed) or clamped (if open) conditions on
//!          the first and last order knots.
//! </UL>
//! The periodicity condition requires that the i_th knot interval equal the
//! (numPoles + i)_th knot interval, for i = 0, ..., 2*order - 3 (counting null
//! knot intervals due to multiplicities).
//! The clamped condition requires the first and last knots to have multiplicity
//! order.
//! <P>
//! Two knots in a given sequence are considered to be equal if the distance
//! between them does not exceed the given relative tolerance.  If pKnots is null,
//! then a uniform knot sequence is assumed and both numKnots and knotTolerance
//! are ignored.
//!
//! @param pContext           OUT     local and global curve data initialized
//! @param pPoles             IN      B-spline curve homogeneous poles
//! @param numPoles           IN      # B-spline curve poles
//! @param pKnots             IN      full B-spline curve knots; null for uniform
//! @param numKnots           IN      # B-spline knots
//! @param knotTolerance      IN      relative minimal distance separating unique knots
//! @param order              IN      B-spline curve order (= degree + 1)
//! @param bClosed            IN      true for closed curve, false for open
//! @return false iff order out of range or invalid input
//! @see #extractNextBezierFromBspline
//! @see #extractNextBezierFromBsplineEnd
//!
Public GEOMDLLIMPEXP bool        bsiBezierDPoint4d_extractNextBezierFromBsplineInit
(
        ExtractContext  *pContext,
const   DPoint4d        *pPoles,
        int             numPoles,
const   double          *pKnots,
        int             numKnots,
        double          knotTolerance,
        int             order,
        bool            bClosed
);

//!
//! Dummy function for symmetry.
//!
//! @see #extractNextBezierFromBsplineInit
//! @see #extractNextBezierFromBspline
//!
Public GEOMDLLIMPEXP void    bsiBezierDPoint4d_extractNextBezierFromBsplineEnd
(
ExtractContext  *pContext
);

//!
//! @description Outputs the homogeneous poles of the next Bezier segment of the given
//!   B-spline curve, as determined by the local curve data in pContext.
//!   The number of poles output equals the order of the curve.
//! @remarks To process all Bezier segments of a B-spline, call this method in a loop until
//!   false is returned.  Sandwich the loop by a call to
//!   <A HREF="#extractNextBezierFromBsplineInit">extractNextBezierFromBsplineInit</A>,
//!   beforehand (to initialize the context), and a call to
//!   <A HREF="#extractNextBezierFromBsplineInit">extractNextBezierFromBsplineEnd</A>,
//!   afterwards.
//!
//! @param pBezierPoles       OUT     order homogeneous poles of next Bez segment
//! @param pContext           IN OUT  local and global curve data
//! @return true iff a Bezier segment was extracted
//! @see #extractNextBezierFromBsplineInit
//! @see #extractNextBezierFromBsplineEnd
//! @see #convertBsplineToBeziers
//!
Public GEOMDLLIMPEXP bool        bsiBezierDPoint4d_extractNextBezierFromBspline
(
        DPoint4d        *pBezierPoles,
        ExtractContext  *pContext
);

//!
//! @description Outputs the homogeneous poles of the next Bezier segment of the given
//!   B-spline curve, as determined by the local curve data in pContext.
//!   The number of poles output equals the order of the curve.
//! @remarks Optionally outputs the Bezier segment's knot span: the original B-spline
//!   knots (parameters) between which the Bezier segment lies.
//! @remarks To process all Bezier segments of a B-spline, call this method in a loop until
//!   false is returned.  Sandwich the loop by a call to
//!   <A HREF="#extractNextBezierFromBsplineInit">extractNextBezierFromBsplineInit</A>,
//!   beforehand (to initialize the context), and a call to
//!   <A HREF="#extractNextBezierFromBsplineInit">extractNextBezierFromBsplineEnd</A>,
//!   afterwards.
//!
//! @param pBezierPoles       OUT     order homogeneous poles of next Bez segment
//! @param pStartKnot         OUT     B-spline param at which next Bez segment starts (or NULL)
//! @param pEndKnot           OUT     B-spline param at which next Bez segment ends (or NULL)
//! @param pContext           IN OUT  local and global curve data
//! @return true iff a Bezier segment was extracted
//! @see #extractNextBezierFromBsplineInit
//! @see #extractNextBezierFromBsplineEnd
//! @see #convertBsplineToBeziers
//!
Public GEOMDLLIMPEXP bool        bsiBezierDPoint4d_extractNextBezierFromBsplineExt
(
        DPoint4d        *pBezierPoles,
        double          *pStartKnot,
        double          *pEndKnot,
        ExtractContext  *pContext
);

//!
//! @description Outputs the homogeneous poles of the next Bezier segment of the given
//!   B-spline curve, as determined by the local curve data in pContext.
//!   The number of poles output equals the order of the curve.
//! @remarks Optionally outputs the Bezier segment's knot span: the original B-spline
//!   knots (parameters) between which the Bezier segment lies, and their multiplicities.
//! @remarks To process all Bezier segments of a B-spline, call this method in a loop until
//!   false is returned.  Sandwich the loop by a call to
//!   <A HREF="#extractNextBezierFromBsplineInit">extractNextBezierFromBsplineInit</A>,
//!   beforehand (to initialize the context), and a call to
//!   <A HREF="#extractNextBezierFromBsplineInit">extractNextBezierFromBsplineEnd</A>,
//!   afterwards.
//!
//! @param pBezierPoles       <= order homogeneous poles of next Bez segment
//! @param pStartKnot         <= B-spline param at which next Bez segment starts (or NULL)
//! @param pEndKnot           <= B-spline param at which next Bez segment ends (or NULL)
//! @param pStartKnotMult     <= multiplicity of pStartKnot (or NULL)
//! @param pEndKnotMult       <= multiplicity of pEndKnot (or NULL)
//! @param pContext           <=> local and global curve data
//! @return true iff a Bezier segment was extracted
//! @see #extractNextBezierFromBsplineInit
//! @see #extractNextBezierFromBsplineEnd
//! @see #convertBsplineToBeziers
//!
Public GEOMDLLIMPEXP bool        bsiBezierDPoint4d_extractNextBezierFromBsplineExt2
(
        DPoint4d        *pBezierPoles,
        double          *pStartKnot,
        double          *pEndKnot,
        int             *pStartKnotMult,
        int             *pEndKnotMult,
        ExtractContext  *pContext
);

//!
//! Output the homogeneous poles of the Bezier spline corresponding to the given
//! B-spline curve.  The input/output curves have the same order.
//! <P>
//! The knot sequence, if given, is assumed to be valid, i.e., it is
//! <UL><LI> nondecreasing,
//!     <LI> has no knot of multiplicity greater than order,and
//!     <LI> satisfies periodicity (if closed) or clamped (if open) conditions on
//!          the first and last order knots.
//! </UL>
//! The periodicity condition requires that the i_th knot interval equal the
//! (numPoles + i)_th knot interval, for i = 0, ..., 2*order - 3 (counting null
//! knot intervals due to multiplicities).
//! The clamped condition requires the first and last knots to have multiplicity
//! order.
//! <P>
//! Two knots in a given sequence are considered to be equal if the distance
//! between them does not exceed the given relative tolerance.  If pKnots is null,
//! then a uniform knot sequence is assumed and both numKnots and knotTolerance
//! are ignored.
//! <P>
//! The number N of Bezier poles output has the sharp upper bound
//! <UL>
//! <LI> <CODE>N OUT     1 + (order - 1) * (numKnots - 2 * order + 1)</CODE>,
//!      if bShare is true
//! <LI> <CODE>N OUT     order * (numKnots - 2 * order + 1)</CODE>, if bShare is false
//! </UL>
//! In particular, if the curve has uniform knot sequence,
//! <UL>
//! <LI> <CODE>N = 1 + (order - 1) * numPoles</CODE>,
//!      if bShare and bClosed are true
//! <LI> <CODE>N = 1 + (order - 1) * (numPoles - order + 1)</CODE>,
//!      if bShare is true and bClosed is false
//! <LI> <CODE>N = order * (numPoles - order + 1)</CODE>,
//!      if bShare and bClosed are false
//! <LI> <CODE>N = order * numPoles</CODE>, if bShare is false and bClosed is true
//! </UL>
//! <P>
//! Optionally outputs each Bezier segment's knot (parameter) span in the B-spline
//! curve.  The number M of parameters returned, and has the sharp upper bound
//! <UL>
//! <LI> <CODE>M OUT     numKnots - 2 * order + 2</CODE>, if bShare is true
//! <LI> <CODE>M OUT     2 * (numKnots - 2 * order + 1)</CODE>, if bShare is false
//! </UL>
//! <P>
//! Note: for open uniform B-spline curves, it is faster to call
//! <A HREF="#convertOpenUniformBsplineToBeziers">convertOpenUniformBsplineToBeziers</A>.
//!
//! @param pBezierPoles   OUT     array of homogeneous poles of Bezier spline (must hold N entries)
//! @param pParameters    OUT     array of B-spline parameter spans of the Bezier segments (must hold M entries) (or NULL)
//! @param pPoles         IN      array of homogeneous poles of B-spline curve
//! @param numPoles       IN      number of poles of B-spline curve
//! @param pKnots         IN      full knot sequence of B-spline curve (null for uniform)
//! @param numKnots       IN      size of full knot sequence
//! @param knotTolerance  IN      relative minimal distance separating unique knots
//! @param order          IN      B-spline/Bezier spline curve order (= degree + 1)
//! @param bClosed        IN      true for closed curve, false for open
//! @param bShare         IN      true if each Bezier segment end point/parameter is to appear only once in output
//! @return number of Bezier segments output or 0 if order out of range.
//! @see #convertOpenUniformBsplineToBeziers
//!
Public GEOMDLLIMPEXP int         bsiBezierDPoint4d_convertBsplineToBeziersExt
(
        DPoint4d*   pBezierPoles,
        double*     pParameters,
const   DPoint4d*   pPoles,
        int         numPoles,
const   double*     pKnots,
        int         numKnots,
        double      knotTolerance,
        int         order,
        bool        bClosed,
        bool        bShare
);

//!
//! Output the homogeneous poles of the Bezier spline corresponding to the given
//! B-spline curve.  The input/output curves have the same order.
//! <P>
//! The knot sequence, if given, is assumed to be valid, i.e., it is
//! <UL><LI> nondecreasing,
//!     <LI> has no knot of multiplicity greater than order,and
//!     <LI> satisfies periodicity (if closed) or clamped (if open) conditions on
//!          the first and last order knots.
//! </UL>
//! The periodicity condition requires that the i_th knot interval equal the
//! (numPoles + i)_th knot interval, for i = 0, ..., 2*order - 3 (counting null
//! knot intervals due to multiplicities).
//! The clamped condition requires the first and last knots to have multiplicity
//! order.
//! <P>
//! Two knots in a given sequence are considered to be equal if the distance
//! between them does not exceed the given relative tolerance.  If pKnots is null,
//! then a uniform knot sequence is assumed and both numKnots and knotTolerance
//! are ignored.
//! <P>
//! The number N of Bezier poles output has the sharp upper bound
//! <UL>
//! <LI> <CODE>N OUT     1 + (order - 1) * (numKnots - 2 * order + 1)</CODE>,
//!      if bShare is true
//! <LI> <CODE>N OUT     order * (numKnots - 2 * order + 1)</CODE>, if bShare is false
//! </UL>
//! In particular, if the curve has uniform knot sequence,
//! <UL>
//! <LI> <CODE>N = 1 + (order - 1) * numPoles</CODE>,
//!      if bShare and bClosed are true
//! <LI> <CODE>N = 1 + (order - 1) * (numPoles - order + 1)</CODE>,
//!      if bShare is true and bClosed is false
//! <LI> <CODE>N = order * (numPoles - order + 1)</CODE>,
//!      if bShare and bClosed are false
//! <LI> <CODE>N = order * numPoles</CODE>, if bShare is false and bClosed is true
//! </UL>
//! <P>
//! Note: for open uniform B-spline curves, it is faster to call
//! <A HREF="#convertOpenUniformBsplineToBeziers">convertOpenUniformBsplineToBeziers</A>.
//!
//! @param pBezierPoles   OUT     array of homogeneous poles of Bezier spline
//! @param pPoles         IN      array of homogeneous poles of B-spline curve
//! @param numPoles       IN      number of poles of B-spline curve
//! @param pKnots         IN      full knot sequence of B-spline curve (null for uniform)
//! @param numKnots       IN      size of full knot sequence
//! @param knotTolerance  IN      relative minimal distance separating unique knots
//! @param order          IN      B-spline/Bezier spline curve order (= degree + 1)
//! @param bClosed        IN      true for closed curve, false for open
//! @param bShare         IN      true if each Bezier segment end point is to appear only once in output
//! @return number of Bezier segments output or 0 if order out of range.
//! @see #convertOpenUniformBsplineToBeziers
//! @see #convertBsplineToBeziersExt
//!
Public GEOMDLLIMPEXP int         bsiBezierDPoint4d_convertBsplineToBeziers
(
        DPoint4d    *pBezierPoles,
const   DPoint4d    *pPoles,
        int         numPoles,
const   double      *pKnots,
        int         numKnots,
        double      knotTolerance,
        int         order,
        bool        bClosed,
        bool        bShare
);

//!
//! @param    pPointArray OUT     evaluated points
//! @param    pDerivArray OUT     evaluated derivatives
//! @param    pPoleArray  IN      master poles.
//! @param    order       IN      curve order.
//! @param    pParamArray       IN      parametric coordinates
//! @param    numParam        IN      number of parameters, points, derivatives
//!
Public GEOMDLLIMPEXP void     bsiBezierDPoint4d_evaluateArray
(
        DPoint4d    *pPointArray,
        DPoint4d    *pDerivArray,
const   DPoint4d    *pPoleArray,
        int         order,
const   double      *pParamArray,
        int         numParam
);

//!
//! @param    pPointArray OUT     evaluated points
//! @param    pDerivArray OUT     evaluated derivatives
//! @param    pPoleArray  IN      master poles.
//! @param    order       IN      curve order.
//! @param    pParamArray       IN      parametric coordinates
//! @param    numParam        IN      number of parameters, points, derivatives
//!
Public GEOMDLLIMPEXP void     bsiBezierDPoint3d_evaluateArray
(
        DPoint3d    *pPointArray,
        DPoint3d    *pDerivArray,
const   DPoint3d    *pPoleArray,
        int         order,
const   double      *pParamArray,
        int         numParam
);

//!
//! Single point evaluation of a bezier
//!
//! @param    pPoint OUT     evaluated point
//! @param    pDeriv OUT     evaluated derivative
//! @param    pPoleArray  IN      master poles.
//! @param    order       IN      curve order.
//! @param    param       IN      parametric coordinate
//!
Public GEOMDLLIMPEXP void     bsiBezierDPoint4d_evaluateDPoint4d
(
        DPoint4d    *pPoint,
        DPoint4d    *pDeriv,
const   DPoint4d    *pPoleArray,
        int         order,
const   double      param
);

//!
//! Single point evaluation of a bezier
//!
//! @param    pPoint OUT     evaluated point
//! @param    pDeriv OUT     evaluated derivative
//! @param    pPoleArray  IN      master poles.
//! @param    order       IN      curve order.
//! @param    param       IN      parametric coordinate
//!
Public GEOMDLLIMPEXP void     bsiBezierDPoint3d_evaluateDPoint3d
(
        DPoint3d    *pPoint,
        DPoint3d    *pDeriv,
const   DPoint3d    *pPoleArray,
        int         order,
const   double      param
);


//!
//! Finds the intersections, if any, of the given Bezier curve with the given
//! plane.  The maximum number of intersections is order - 1.
//!
//! @param    pPointArray         OUT     coordinates of intersection points (or null)
//! @param    pDerivArray         OUT     derivatives of intersection points (or null)
//! @param    pParamArray         OUT     parameters of intersection points (or null)
//! @param    pNumIntersection    OUT     number of intersections (or null)
//! @param    pAllOn              OUT     true if curve is entirely within the plane (or null)
//! @param    pPoleArray          IN      array of homogeneous Bezier poles
//! @param    order               IN      curve order = # poles
//! @param    pPlane              IN      homogeneous coordinates of the plane
//!
Public GEOMDLLIMPEXP bool       bsiBezierDPoint4d_intersectPlane
(
        DPoint4d    *pPointArray,
        DPoint4d    *pDerivArray,
        double      *pParamArray,
        int         *pNumIntersection,
        bool        *pAllOn,
const   DPoint4d    *pPoleArray,
        int         order,
const   DPoint4d    *pPlane
);

//!
//! Test if all weights are near one.
//! @param    pPoleArray  IN      poles.
//! @param    order       IN      curve order.
//! @param    tolerance   IN      tolerance for declaring non-unit weight.
//!                           If a negative is passed, a small default is used.
//!
Public GEOMDLLIMPEXP bool     bsiBezierDPoint4d_isUnitWeight
(
const   DPoint4d    *pPoleArray,
        int         order,
        double      tolerance
);

//!
//! Find the maximum absolute deviation between any pole and the midpoint of the chord
//!   between the two adjacent poles.  Deviation is measured as sum of absolute differences.
//!
//! @param    pPoleArray  IN      master poles.
//! @param    order       IN      curve order.
//!
Public GEOMDLLIMPEXP double   bsiBezierDPoint4d_maxAbsMidpointDeviationXYZ
(
const   DPoint4d    *pPoleArray,
        int         order
);

//!
//! Recursively subdivide a bezier curve.  Pass incremental control polygons to separate test and
//! output functions.
//!
//! @param    pPoleArray  IN      master poles.
//! @param    order       IN      curve order.
//! @param    testFunc     IN      Function with args
//!                           testFunc (pTestContext, pPoles, numPoint, s0, s1)
//!                   to test if a fragment of the final control polygon should be accepted.
//!                   Return true to continue subdivision.
//! @param    handlerFunc     IN      Function with args
//!                           handlerFunc (pHandlerInstance, pStrokeArray, numPoint, s0, s1)
//!                   to receive (a fragment of) the final control polygon.  Successive
//!                   fragments duplicate the shared point.
//! @param    pHandlerContext  IN      instance var for handler
//!
Public GEOMDLLIMPEXP void     bsiBezierDPoint4d_testAndSubdivide
(
const   DPoint4d                    *pPoleArray,
        int                         order,
        DPoint4dSubdivisionHandler  testFunc,
        void                        *pTestContext,
        DPoint4dSubdivisionHandler  handlerFunc,
        void                        *pHandlerContext,
        int                         maxRecursion
);

//! Return an estimate of the number of edges needed so that strokes with uniform fractional step
//! satisfy tolerances.
//! @param [in] chordTol largest distance from chord to curve.
//! @param [in] angleTol largest angle between successive chords.
//! @param [in] maxEdgeLength max length of individual chord
//! @param in] true if caller knows that all the bezier points have weight 1.  If not sure, just say false.
Public GEOMDLLIMPEXP int bsiBezierDPoint4d_estimateEdgeCount
(
const   DPoint4d    *pPoleArray,
        int         order,
        double      chordTol,
        double      angleTol,
        double      maxEdgeLength,
        bool        weightsAreAllOne
);

//! Add a fixed number of points (given as edge count) to arrays.
//! @param [in] poles bezier poles
//! @param [in] order bezier order
//! @param [out] points destination for points
//! @param [out] params (optional) pointer to destination for parameters
//! @param [out] derivatives (optional) pointer to destination for (non unit) derivatives
//! @param [in] numEdge number of edges
//! @param [in] includeStartPoint true to force startpoint into arrays.  (e.g. set it true for first bezier of a bspline, false for continuations.)
//! @param [in] param0 parameter (e.g. bspline knot) at start of bezier section.
//! @param [in] param1 parameter (e.g. bspline knot) at end of bezier section.
Public GEOMDLLIMPEXP void     bsiBezierDPoint4d_addStrokes
(
const   DPoint4d    *pPoleArray,
        int         order,
bvector<DPoint3d>&points,
bvector<double> *params,
bvector<DVec3d> *derivatives,
size_t numEdge,
bool includeStartPoint,
double param0,
double param1
);



//!
//! Stroke a homogeneous curve to a cartesian tolerance.
//!
//! @param    pPoleArray  IN      master poles.
//! @param    order       IN      curve order.
//! @param    tolerance   IN      carteian (normalized) tolerance
//! @param    handlerFunc     IN      Function with args
//!                           handlerFunc (pHandlerInstance, pStrokeArray, numPoint)
//!                   to receive array of strokes.
//! @param    pHandlerContext  IN      instance var for handler
//!
Public GEOMDLLIMPEXP void     bsiBezierDPoint4d_stroke
(
const   DPoint4d    *pPoleArray,
        int         order,
        double      tolerance,
        DPoint4dArrayHandler handlerFunc,
        void        *pHandlerContext
);

//!
//! DeCasteljou subdivision.
//! given interpolating fraction.
//! @param pPoleArray IN OUT  On input, full interval pole array. On output, left part of subdivision
//!       (top of triangle)
//! @param pRightPoleArray OUT     poles for right of subdivision interval (bottom of triangle).
//! @param order IN      number of poles.
//! @param u IN      interpolating parameter.
//!
Public GEOMDLLIMPEXP void     bsiBezierDPoint4d_subdivideLeftInPlace
(
DPoint4d    *pPoleArray,
DPoint4d    *pRightPoleArray,
    int     order,
    double  u
);

//!
//! DeCasteljou subdivision.
//! given interpolating fraction.
//! @param pPoleArray IN OUT  On input, full interval pole array. On output, right part of subdivision
//!       (bottom of triangle)
//! @param pLeftPoleArray OUT     poles for left of subdivision interval (top of triangle).
//! @param order IN      number of poles.
//! @param u IN      interpolating parameter.
//!
Public GEOMDLLIMPEXP void     bsiBezierDPoint4d_subdivideRightInPlace
(
DPoint4d    *pPoleArray,
DPoint4d    *pLeftPoleArray,
    int     order,
    double  u
);

//!
//! DeCasteljou subdivision.
//! given interpolating fraction.
//! @param pPoleArray IN OUT  On input, full interval pole array. On output, left part of subdivision
//!       (top of triangle)
//! @param pRightPoleArray OUT     poles for right of subdivision interval (bottom of triangle).
//! @param order IN      number of poles.
//! @param u IN      interpolating parameter.
//!
Public GEOMDLLIMPEXP void     bsiBezierDPoint3d_subdivideLeftInPlace
(
DPoint3d    *pPoleArray,
DPoint3d    *pRightPoleArray,
    int     order,
    double  u
);

//!
//! DeCasteljou subdivision.
//! given interpolating fraction.
//! @param pPoleArray IN OUT  On input, full interval pole array. On output, right part of subdivision
//!       (bottom of triangle)
//! @param pLeftPoleArray OUT     poles for left of subdivision interval (top of triangle).
//! @param order IN      number of poles.
//! @param u IN      interpolating parameter.
//!
Public GEOMDLLIMPEXP void     bsiBezierDPoint3d_subdivideRightInPlace
(
DPoint3d    *pPoleArray,
DPoint3d    *pLeftPoleArray,
    int     order,
    double  u
);

//!
//! DeCasteljou subdivision.
//! given interpolating fraction.
//! @param pPoleArray IN OUT  On input, full interval pole array. On output, left part of subdivision
//!       (top of triangle)
//! @param pRightPoleArray OUT     poles for right of subdivision interval (bottom of triangle).
//! @param order IN      number of poles.
//! @param u IN      interpolating parameter.
//!
Public GEOMDLLIMPEXP void bsiBezierDPoint4d_subdivideToIntervalInPlace
(
DPoint4d    *pPoleArray,
    int     order,
    double  u0,
    double  u1
);

//!
//! DeCasteljou subdivision.
//! given interpolating fraction.
//! @param pPoleArray IN OUT  On input, full interval pole array. On output, left part of subdivision
//!       (top of triangle)
//! @param pRightPoleArray OUT     poles for right of subdivision interval (bottom of triangle).
//! @param order IN      number of poles.
//! @param u IN      interpolating parameter.
//!
Public GEOMDLLIMPEXP void bsiBezierDPoint3d_subdivideToIntervalInPlace
(
DPoint3d    *pPoleArray,
    int     order,
    double  u0,
    double  u1
);


//!
//! Compute intersection of two bezier curves, starting with chordal approximations.
//! @param pPointA OUT computed points on curve A
//! @parma pParamA OUT parameters on curve A
//! @param pPointB OUT computed points on curve B
//! @parma pParamB OUT parameters on curve B
//! @param pNumIntersection OUT number of returned intersections
//! @param pNumExtra OUT number of extra intersections
//! @param maxIntersection OUT max returned (additional intersections are computed and
//!            counted in pNumExtra)
//! @param pA OUT curve A
//! @param orderA OUT order of curve A
//! @param pB OUT curve B
//! @param orderB OUT order of curve B
//!
//! @param
//! @return true if sufficient storage to solve.
//!
Public GEOMDLLIMPEXP bool    bsiBezierDPoint4d_intersectXY_chordal
(
      DPoint4d  *pPointA,
      double    *pParamA,
      DPoint4d  *pPointB,
      double    *pParamB,
      int       *pNumIntersection,
      int       *pNumExtra,
      int       maxIntersection,
const DPoint4d  *pA,
      int       orderA,
const DPoint4d  *pB,
      int       orderB
);

#ifdef __cplusplus
//!
//! Given {order} poles and {2*(order-1)} knots, apply blossoming to clamp the poles in place.
//! @param [in,out] pPoint on input, the unclamped poles. On output, bezier poles.
//! @param [in] pKnot The unclampled knots.
//! @param [in] order spline order.
//! @param [out] isNullInterval true if the interval is null.
//!     (Computation proceeds, computed point of degenerate bezier may be meaningful).
//!
Public GEOMDLLIMPEXP void bsiBezierDPoint4d_saturateKnotsInInterval
(
DPoint4d  *pPoint,
double    *pKnot,
int order,
bool &isNullInterval,
bool saturateLeft = true,
bool saturateRight = true
);


//!
//! Saturate knots for points, weights in place presented as separate arrays.
//! @param [in,out] pPoint on input, the unclamped poles. On output, bezier poles.
//! @param [in,out] pPoint on input, the unclamped weights. On output, bezier weights.
//! @param [in] pKnot The unclampled knots.
//! @param [in] order spline order.
//! @param [out] isNullInterval true if the interval is null.
//!     (Computation proceeds, computed point of degenerate bezier may be meaningful).
//!
Public GEOMDLLIMPEXP void bsiBezierDPoint3d_saturateKnotsInInterval
(
DPoint3d  *pPoint,
double    *pWeight,
double    *pKnot,
int order,
bool &isNullInterval,
bool saturateLeft,
bool saturateRight
);

//! @return true if knot values appear identical.
Public GEOMDLLIMPEXP bool bsiBezier_isNullKnotInterval (double a, double b);

//!
//! Given {order} poles and {2*(order-1)} knots, apply blossoming to clamp the poles in place.
//! @param [in,out] pPoles on input, the unclamped poles. On output, bezier poles.
//! @param [in] numPerPole number of doubles per pole.
//! @param [in] pKnot The unclampled knots.
//! @param [in] order spline order.
//! @param [out] isNullInterval true if the interval is null.
//!     (Computation proceeds, computed point of degenerate bezier may be meaningful).
//!
Public GEOMDLLIMPEXP void bsiBezier_saturateKnotsInInterval
(
double *pPoleData,
size_t numPerPole,
double    *pKnot,
int order,
bool &isNullInterval,
bool saturateLeft = true,
bool saturateRight = true
);


//!
//! Insert a knot in arrays of poles and knots.  Both arrays are extended.
//! @param [in,out] pPoint pole array.
//! @param [in,out] pKnot knot array.
//! @param [in] numPoint number of poles.
//! @param [in] order spline order (one more than degree)
//! @param [in] numLeadingKnot In customary storage (with extraneous leading knot), equal to {order}.
//! @param [in] leftPoleIndex index of leftmost pole of the {order} poles that apply.
//! @param [in] leftKnotIndex index of the left end of the knot interval.  {order-1}
//!                knots ENDING here are referenced as "left" of the interval. {order-1}
//!                knots BEGINNING at {leftKnot+1} are referenced.
//! @param [in] fraction fractional parameter within interval from knot {leftKnotIndex}
//!                to {leftKnotIndex+1}
//! @return true if the knot interval has nonzero length and all referenced knots are non-decreasing.
//!
Public GEOMDLLIMPEXP bool    bsiBezierDPoint4d_insertKnot
(
bvector<DPoint4d> &poles,
bvector<double> &knots,
int        order,
size_t leftPoleIndex,
size_t leftKnotIndex,
double fraction
);

Public GEOMDLLIMPEXP void bsiBezierDPoint4d_intersectDSegment4dXY
(
DPoint4dP bezierPoints,
double *bezierFractions,
DPoint4dP segmentPoints,
double *segmentFractions,
size_t &numOut,
size_t maxOut,
DPoint4dCP bezierPoles,
size_t order,
DSegment4dCR  segment,
bool extendSegment0,
bool extendSegment1
);
#endif
END_BENTLEY_GEOMETRY_NAMESPACE

