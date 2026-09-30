/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE


//!
//! @description Fill in ellipse data.
//!
//! @param pEllipse OUT     initialized ellipse
//! @param cx IN center x coordinate
//! @param cy IN center y coordinate
//! @param cz IN center z coordinate
//! @param ux IN x part of 0 degree vector
//! @param uy IN y part of 0 degree vector
//! @param uz IN z part of 0 degree vector
//! @param vx IN x part of 90 degree vector
//! @param vy IN y part of 90 degree vector
//! @param vz IN z part of 90 degree vector
//! @param theta0 IN start angle in parameter space
//! @param sweep IN sweep angle
//! @group "DEllipse3d Initialization"
//!
Public GEOMDLLIMPEXP void bsiDEllipse3d_init
(
DEllipse3dP pEllipse,
double        cx,
double        cy,
double        cz,
double        ux,
double        uy,
double        uz,
double        vx,
double        vy,
double        vz,
double        theta0,
double        sweep
);

//!
//! @description Fill in ellipse data from 2D major and minor axis lengths and the angle
//!   from the global to the local x-axis.
//!
//! @param pEllipse OUT     initialized ellipse
//! @param cx      IN      center x coordinate
//! @param cy      IN      center y coordinate
//! @param cz      IN      z coordinate of all points on the ellipse
//! @param rx      IN      radius along local x axis
//! @param ry      IN      radius along local y axis
//! @param thetaX  IN      angle from global x to local x
//! @param theta0  IN      start angle in parameter space
//! @param sweep   IN      sweep angle
//! @group "DEllipse3d Initialization"
//!
Public GEOMDLLIMPEXP void bsiDEllipse3d_initFromXYMajorMinor
(
DEllipse3dP pEllipse,
double          cx,
double          cy,
double          cz,
double          rx,
double          ry,
double          thetaX,
double          theta0,
double          sweep
);

//!
//! @nodoc DEllipse4d
//! @description Convert a homogeneous ellipse to cartesian.  Callers should beware of the following
//! significant points:
//! <UL>
//! <LI>A homogeneous "ellipse" may appear as a hyperbola or parabola in xyz space.
//!   Hence the conversion can fail.
//! <LI>When the conversion succeeds, it is still a Very Bad Thing To Do numerically
//!   because a homogeneous ellipse with "nice" numbers can have very large center and axis
//!   coordinates.   It is always preferable to do calculations directly on the homogeneous
//!   ellipse if possible.
//! <LI>When the conversion succeeds, the axis may be non-perpendicular.  A subsequent call
//!   may be made to initWithPerpendicularAxes to correct this.
//! </UL>
//! @param pEllipse OUT     initialized ellipse
//! @param pSource IN      homogeneous ellipse
//! @param sector  IN      angular sector index.  If out of bounds, a full ellipse is created.
//! @return true if homogeneous parts allow reduction to simple ellipse. (false if the homogeneous
//!    parts are a parabola or hyperbola.)
//! @group "DEllipse3d Initialization"
//!
Public GEOMDLLIMPEXP bool     bsiDEllipse3d_initFromDEllipse4d
(
DEllipse3dP pEllipse,
DEllipse4dCP pSource,
int           sector
);

//!
//! @description Convert a homogeneous ellipse to a Cartesian ellipse.
//! @remarks Callers should beware of the following significant points:
//! <UL>
//! <LI>A homogeneous "ellipse" may appear as a hyperbola or parabola in xyz space.
//!   Hence the conversion can fail.
//! <LI>When the conversion succeeds, it is still a Very Bad Thing To Do numerically
//!   because a homogeneous ellipse with "nice" numbers can have very large center and axis
//!   coordinates.   It is always preferable to do calculations directly on the homogeneous
//!   ellipse if possible.
//! <LI>When the conversion succeeds, the axis may be non-perpendicular.  A subsequent call
//!   may be made to initWithPerpendicularAxes to correct this.
//! </UL>
//! @param pEllipse OUT     initialized ellipse
//! @param pSource IN      homogeneous ellipse
//! @return false if the conic weights are zero anywhere, creating hyperbola or parabola which
//!    cannot be reduced to an ellipse.
//! @group "DEllipse3d Initialization"
//!
Public GEOMDLLIMPEXP bool     bsiDEllipse3d_initFromDConic4d
(
DEllipse3dP pEllipse,
DConic4dCP pSource
);

//!
//! @description Project the source ellipse to a plane.
//! @param pSourceEllipse IN known ellipse.
//! @param pEllipse OUT initialized ellipse
//! @param pSweepDirection IN direction to project.  If NULL, the plane normal is used.
//! @param pPlane IN the target plane.
//! @return false if projection direction is parallel to the plane.  The result ellipse
//!    is then a copy of the source.
//! @group "DEllipse3d Initialization"
//!
Public GEOMDLLIMPEXP bool    bsiDEllipse3d_sweepToPlane
(
DEllipse3dCP pSourceEllipse,
DEllipse3dP pEllipse,
DVec3dCP    pSweepDirection,
DPlane3dCP  pPlane
);

//!
//! @description Fill in ellipse data from center, 0 degree, and 90 degree points.
//!
//! @param pEllipse OUT     initialized ellipse
//! @param pCenter IN      ellipse center
//! @param pPoint0 IN      0 degree point
//! @param pPoint90 IN      90 degree point
//! @param theta0 IN      start angle
//! @param sweep IN      sweep angle
//! @group "DEllipse3d Initialization"
//!
Public GEOMDLLIMPEXP void bsiDEllipse3d_initFrom3dPoints
(
DEllipse3dP pEllipse,
DPoint3dCP pCenter,
DPoint3dCP pPoint0,
DPoint3dCP pPoint90,
double          theta0,
double          sweep
);

//!
//! @description Initialize a circlular arc from start point, end point, another vector which
//!  determines the plane, and the arc length.
//!
//! @param pEllipse OUT     initialized ellipse
//! @param pStart IN      start point
//! @param pEnd IN      end point
//! @param arcLength IN      required arc length
//! @param pPlaneVector IN      vector to be used to determine the plane of the
//!                    arc.  The plane is chosen so that it contains both the
//!                    start-to-end vector and the plane vector, and the arc bulge
//!                    is in the direction of the plane vector (rather than opposite).
//! @return true if the arc length exceeds the chord length and the 2 points and plane vector
//!                determine a clear plane.
//! @group "DEllipse3d Initialization"
//!
Public GEOMDLLIMPEXP bool    bsiDEllipse3d_initArcFromChordAndArcLength
(
DEllipse3dP pEllipse,
DPoint3dCP pStart,
DPoint3dCP pEnd,
double                  arcLength,
DPoint3dCP pPlaneVector
);

//!
//! @description Initialize a circular arc from start point, start tangent, and end point.
//!
//! @param pEllipse OUT     initialized ellipse
//! @param pStart IN      start point
//! @param pTangent IN      start tangent
//! @param pEnd IN      end point
//! @return true if circular arc computed.   false if start, end and tangent are colinear.
//! @group "DEllipse3d Initialization"
//!
Public GEOMDLLIMPEXP bool    bsiDEllipse3d_initFromDPoint3dDPoint3dTangent
(
DEllipse3dP pEllipse,
DPoint3dCP pStart,
DPoint3dCP pTangent,
DPoint3dCP pEnd
);

//!
//! @description Initialize a circular arc with given center and start, and with
//! sweep so that the end point is near the given end. (Note that the circle
//! will NOT pass directly through the endpoint itself if it is at a different
//! distance from the center.)  The arc is always the smaller of the two
//! possible parts of the full circle.
//!
//! @param pEllipse OUT     initialized ellipse
//! @param pCenter IN      ellipse center
//! @param pStart IN      start point
//! @param pEnd IN      nominal end point
//! @return false if the the three points are colinear.
//! @group "DEllipse3d Initialization"
//!
Public GEOMDLLIMPEXP bool    bsiDEllipse3d_initFromArcCenterStartEnd
(
DEllipse3dP pEllipse,
DPoint3dCP pCenter,
DPoint3dCP pStart,
DPoint3dCP pEnd
);

//!
//! @description Fill in ellipse data from center and two basis vectors.
//!
//! @param pEllipse OUT     initialized ellipse
//! @param pCenter IN      ellipse center
//! @param pVector0 IN      0 degree vector
//! @param pVector90 IN      90 degree vector
//! @param theta0 IN      start angle
//! @param sweep IN      sweep angle
//! @group "DEllipse3d Initialization"
//!
Public GEOMDLLIMPEXP void bsiDEllipse3d_initFrom3dVectors
(
DEllipse3dP pEllipse,
DPoint3dCP pCenter,
DVec3dCP pVector0,
DVec3dCP pVector90,
double          theta0,
double          sweep
);

//!
//! @description Fill in ellipse data from center, 0 degree, and 90 degree points presented as an array of 3 points.
//!
//! @param pEllipse OUT     initialized ellipse
//! @param pPointArray IN      ellipse center, 0 degree and 90 degree points
//! @param theta0 IN      start angle
//! @param sweep IN      sweep angle
//! @group "DEllipse3d Initialization"
//!
Public GEOMDLLIMPEXP void bsiDEllipse3d_initFrom3dPointArray
(
DEllipse3dP pEllipse,
DPoint3dCP pPointArray,
double              theta0,
double              sweep
);

//!
//! @description Set angular parameters to have given start and end points.
//! @remarks If the given points are really on the ellipse, this does the expected thing.
//! @remarks If the given points are not on the ellipse, here's exactly what happens.
//!    The start/end points are placed on the original ellipse at the point where the ellipse intersects
//!    the plane formed by the ellipse axis and the given point.  This leaves the problem that the ellipse
//!    defines two paths from the given start to end. This is resolved as follows.  The ellipse's existing
//!    0 and 90 degree vectors define a coordinate system.  In that system, the short sweep from the 0
//!    degree vector to the 90 degree vector is considered "counterclockwise".
//! @remarks Beware that the relation of supposed start/end points to the ellipse is ambiguous.
//!
//! @param pEllipse IN OUT ellipse to update
//! @param pStartPoint IN new start point
//! @param pEndPoint IN new end point
//! @param ccw    IN      true to force counterclockwise direction, false for clockwise.
//! @return true if the ellipse axes are independent.  false if the ellipse is degenerate.
//! @group "DEllipse3d Angles"
//!
Public GEOMDLLIMPEXP bool         bsiDEllipse3d_setStartEnd
(
DEllipse3dP pEllipse,
DPoint3dCP pStartPoint,
DPoint3dCP pEndPoint,
bool        ccw
);

//!
//! @description Fill in ellipse data from center, x and y directions from columns
//! 0 and 1 of a RotMatrix, and scale factors to apply to x and and y directions.
//!
//! @param pEllipse OUT     initialized ellipse
//! @param pCenter IN      ellipse center
//! @param pMatrix IN      columns 0, 1 are ellipse directions (to be scaled by r0, r1)
//! @param r0 IN      scale factor for column 0
//! @param r1 IN      scale factor for column 1
//! @param theta0 IN      start angle
//! @param sweep IN      sweep angle
//! @group "DEllipse3d Initialization"
//!
Public GEOMDLLIMPEXP void bsiDEllipse3d_initFromScaledRotMatrix
(
DEllipse3dP pEllipse,
DPoint3dCP pCenter,
RotMatrixCP pMatrix,
double      r0,
double      r1,
double              theta0,
double              sweep
);

//!
//! @description Fill in ellipse from center, x and y directions from columns
//! 0 and 1 of a DMatrix3d, and scale factors to apply to x and and y directions.
//!
//! @param pEllipse OUT     initialized ellipse
//! @param pCenter IN      ellipse center
//! @param pMatrix IN      columns 0, 1 are ellipse directions (to be scaled by r0, r1)
//! @param r0 IN      scale factor for column 0
//! @param r1 IN      scale factor for column 1
//! @param theta0 IN      start angle
//! @param sweep IN      sweep angle
//! @group "DEllipse3d Initialization"
//!
Public GEOMDLLIMPEXP void bsiDEllipse3d_initFromScaledDMatrix3d
(
DEllipse3dP pEllipse,
DPoint3dCP pCenter,
DMatrix3dCP pMatrix,
double      r0,
double      r1,
double              theta0,
double              sweep
);

//!
//! @description Fill in ellipse data from center and x and y directions as vectors with scale factors.
//!
//! @param pEllipse OUT     initialized ellipse
//! @param pCenter IN      ellipse center
//! @param pVector0 IN      0 degree vector (e.g. major axis)
//! @param pVector90 IN      90 degree vector (e.g. minor axis)
//! @param r0 IN      scale factor for vector 0
//! @param r1 IN      scale factor for vector 90
//! @param theta0 IN      start angle
//! @param sweep IN      sweep angle
//! @group "DEllipse3d Initialization"
//!
Public GEOMDLLIMPEXP void bsiDEllipse3d_initFromScaledVectors
(
DEllipse3dP pEllipse,
DPoint3dCP pCenter,
DPoint3dCP pVector0,
DPoint3dCP pVector90,
double      r0,
double      r1,
double              theta0,
double              sweep
);

//!
//! @description Adjust axis vectors so 0-degree vector is along true major axis.
//! @remarks This is similar to ~mbsiDEllipse3d_initWithPerpendicularAxes, which
//!       chooses the 0 degree vector <EM>closest</EM> to current 0 degrees, even if
//!       that is really the "minor" axis.  This function makes the further adjustment
//!       of bringing the larger axis to 0 degrees in the parameterization.
//! @param pMajorMinorEllipse OUT modified ellipse
//! @param pEllipse IN source ellipse.
//! @group "DEllipse3d Initialization"
//!
Public GEOMDLLIMPEXP void bsiDEllipse3d_initMajorMinor
(
DEllipse3dP pMajorMinorEllipse,
DEllipse3dCP pEllipse
);

//!
//! @description Extract major minor axis form of the ellipse.
//!
//! @param pEllipse IN      ellipse to query
//! @param pCenter OUT     ellipse center
//! @param pMatrix OUT     columns 0, 1 are normalized ellipse basis vectors, column 2 is their cross product
//! @param pR0 OUT     scale factor for column 0
//! @param pR1 OUT     scale factor for column 1
//! @param pTheta0 OUT     start angle
//! @param pSweep OUT     sweep angle
//! @group "DEllipse3d Queries"
//!
Public GEOMDLLIMPEXP void bsiDEllipse3d_getScaledDMatrix3d
(
DEllipse3dCP pEllipse,
DPoint3dP pCenter,
DMatrix3dP pMatrix,
double      *pR0,
double      *pR1,
double      *pTheta0,
double      *pSweep
);

//!
//! @description Extract major minor axis form of the ellipse.
//!
//! @param pEllipse IN      ellipse to query
//! @param pCenter OUT     ellipse center
//! @param pMatrix OUT     columns 0, 1 are normalized ellipse basis vectors, column 2 is their cross product
//! @param pR0 OUT     scale factor for column 0
//! @param pR1 OUT     scale factor for column 1
//! @param pTheta0 OUT     start angle
//! @param pSweep OUT     sweep angle
//! @group "DEllipse3d Queries"
//!
Public GEOMDLLIMPEXP void bsiDEllipse3d_getScaledRotMatrix
(
DEllipse3dCP pEllipse,
DPoint3dP   pCenter,
RotMatrixP   pMatrix,
double      *pR0,
double      *pR1,
double      *pTheta0,
double      *pSweep
);

//!
//! @description Initialize a circle from center, normal and radius.
//! @param pEllipse OUT     initialized ellipse
//! @param pCenter IN      circle center
//! @param pNormal IN      plane normal (NULL for 001)
//! @param radius IN      circle radius
//! @group "DEllipse3d Initialization"
//!
Public GEOMDLLIMPEXP void bsiDEllipse3d_initFromCenterNormalRadius
(
DEllipse3dP pEllipse,
DPoint3dCP pCenter,
DPoint3dCP pNormal,
double          radius
);

//!
//! @description Test whether the ellipse is complete (2pi range).
//! @param pEllipse IN      ellipse to query
//! @return true if the ellipse is complete
//! @group "DEllipse3d Queries"
//!
Public GEOMDLLIMPEXP bool    bsiDEllipse3d_isFullEllipse (DEllipse3dCP pEllipse);

//!
//! @description Set the ellipse sweep to a full 360 degrees (2pi radians), preserving direction of sweep.
//! @remarks Start angle is left unchanged.
//! @param pEllipse IN OUT  ellipse to change
//! @group "DEllipse3d Angles"
//!
Public GEOMDLLIMPEXP void bsiDEllipse3d_makeFullSweep (DEllipse3dP pEllipse);

//!
//! @description Set the ellipse sweep to the complement of its current angular range.
//! @remarks Full ellipse is left unchanged.
//! @param pEllipse IN OUT  ellipse to change
//! @group "DEllipse3d Angles"
//!
Public GEOMDLLIMPEXP void bsiDEllipse3d_complementSweep (DEllipse3dP pEllipse);

//!
//! @description Compute the ellipse xyz point at a given parametric (angular) coordinate.
//! @param pEllipse IN      ellipse to evaluate
//! @param pPoint OUT     evaluated point
//! @param theta IN      angle
//! @group "DEllipse3d Evaluation"
//!
Public GEOMDLLIMPEXP void bsiDEllipse3d_evaluateDPoint3d
(
DEllipse3dCP pEllipse,
DPoint3dP pPoint,
double      theta
);

//!
//! @description Compute the ellipse xyz point at a given parametric (xy) coordinate.
//! @param pEllipse IN      ellipse to evaluate
//! @param pPoint OUT     evaluated point
//! @param xx IN      local x coordinate: cos(theta)
//! @param yy IN      local y coordinate: sin(theta)
//! @group "DEllipse3d Evaluation"
//!
Public GEOMDLLIMPEXP void bsiDEllipse3d_evaluateDPoint3dFromLocal
(
DEllipse3dCP pEllipse,
DPoint3dP pPoint,
double      xx,
double      yy
);

//!
//! @description Compute the ellipse xyz point at a given parametric (angular) coordinate.
//! @param pEllipse IN      ellipse to evaluate
//! @param pPoint OUT     evaluated point (unit weight)
//! @param theta IN      angle
//! @group "DEllipse3d Evaluation"
//!
Public GEOMDLLIMPEXP void bsiDEllipse3d_evaluateDPoint4d
(
DEllipse3dCP pEllipse,
DPoint4dP pPoint,
double      theta
);

//!
//! @description Compute the ellipse start and end points.
//! @param pEllipse IN      ellipse to evaluate
//! @param pStartPoint OUT     start point of ellipse
//! @param pEndPoint  OUT     end point of ellipse
//! @group "DEllipse3d Evaluation"
//!
Public GEOMDLLIMPEXP void bsiDEllipse3d_evaluateEndPoints
(
DEllipse3dCP pEllipse,
DPoint3dP pStartPoint,
DPoint3dP pEndPoint
);

//!
//! @description  Compute the ellipse xyz point and derivatives at a given parametric (angular) coordinate.
//! @param pEllipse IN      ellipse to evaluate
//! @param pX OUT     (optional) point on ellipse
//! @param pdX OUT     (optional) first derivative vector
//! @param pddX OUT     (optional) second derivative vector
//! @param theta IN      angle for evaluation
//! @group "DEllipse3d Evaluation"
//!
Public GEOMDLLIMPEXP void bsiDEllipse3d_evaluateDerivatives
(
DEllipse3dCP pEllipse,
DPoint3dP pX,
DPoint3dP pdX,
DPoint3dP pddX,
double      theta
);

//!
//! @description Compute the ellipse xyz point at a given fraction of the angular parametric range.
//! @param pEllipse IN      ellipse to evaluate
//! @param pX OUT     point on ellipse
//! @param fraction IN      fractional parameter for evaluation
//! @group "DEllipse3d Evaluation"
//!
Public GEOMDLLIMPEXP void bsiDEllipse3d_fractionParameterToDPoint3d
(
DEllipse3dCP pEllipse,
DPoint3dP pX,
double      fraction
);

//!
//! @description Compute the ellipse xyz point and derivatives at a given fraction of the angular parametric range.
//! @param pEllipse IN      ellipse to evaluate
//! @param pX OUT     (optional) point on ellipse
//! @param pdX OUT     (optional) second derivative vector
//! @param pddX OUT     (optional) second derivative vector
//! @param fraction IN      fractional parameter for evaluation
//! @group "DEllipse3d Evaluation"
//!
Public GEOMDLLIMPEXP void bsiDEllipse3d_fractionParameterToDerivatives
(
DEllipse3dCP pEllipse,
DPoint3dP pX,
DPoint3dP pdX,
DPoint3dP pddX,
double      fraction
);

//!
//! @description Compute ellipse xyz point and derivatives, returned as an array.
//! @param pEllipse IN      ellipse to evaluate
//! @param pX OUT     Array of ellipse point, first derivative, etc.  Must contain room for numDerivatives+1 points.  pX[i] = i_th derivative.
//! @param numDerivative IN      number of derivatives (0 to compute just the xyz point)
//! @param theta IN      angle for evaluation
//! @group "DEllipse3d Evaluation"
//!
Public GEOMDLLIMPEXP void bsiDEllipse3d_evaluateDerivativeArray
(
DEllipse3dCP pEllipse,
DPoint3dP pX,
int         numDerivative,
double      theta
);

//!
//! @description Convert a fractional parameter to ellipse parameterization angle.
//! @param pEllipse IN      ellipse to evaluate
//! @param fraction IN      fraction of angular range
//! @return angular parameter
//! @group "DEllipse3d Evaluation"
//!
Public GEOMDLLIMPEXP double bsiDEllipse3d_fractionToAngle
(
DEllipse3dCP pEllipse,
double      fraction
);

//!
//! @description Compute the determinant of the Jacobian matrix for the transformation from local coordinates (cosine, sine) to global xy-coordinates.
//! @param pEllipse IN      ellipse to query
//! @return determinant of Jacobian.
//! @group "DEllipse3d Queries"
//!
Public GEOMDLLIMPEXP double bsiDEllipse3d_determinantJXY (DEllipse3dCP pEllipse);

//!
//! @description Get the coordinate frame for an ellipse.  X,Y axes are at 0 and 90 degrees.
//! Z axis is perpendicular with magnitude equal to the geometric mean of the other two.
//! @param pEllipse    IN      ellipse whose frame is computed.
//! @param pFrame      OUT     transformation from (cosine, sine, z) coordinates to global xyz.  May be NULL.
//! @param pInverse    OUT     inverse of frame.  May be NULL.
//! @return true if the requested frames were returned.
//! @group "DEllipse3d Queries"
//!
Public GEOMDLLIMPEXP bool    bsiDEllipse3d_getLocalFrame
(
DEllipse3dCP pEllipse,
DTransform3dP pFrame,
DTransform3dP pInverse
);

//!
//! @description Get the coordinate frame for an ellipse.  X,Y axes are at 0 and 90 degrees.
//! Z axis is perpendicular with magnitude equal to the geometric mean of the other two.
//! @param pEllipse    IN      ellipse whose frame is computed.
//! @param pFrame      OUT     transformation from (cosine, sine, z) coordinates to global xyz.  May be NULL.
//! @param pInverse    OUT     inverse of frame.  May be NULL.
//! @return true if the requested frames were returned.
//! @group "DEllipse3d Queries"
//!
Public GEOMDLLIMPEXP bool    bsiDEllipse3d_getLocalFrameTransform
(
DEllipse3dCP pEllipse,
TransformP pFrame,
TransformP pInverse
);




//!
//! @description Get the coordinate frame for an ellipse.  X,Y axes are at 0 and 90 degrees.
//! Z axis is perpendicular with magnitude equal to the geometric mean of the other two.
//! @param pEllipse    IN      ellipse whose frame is computed.
//! @param pFrame      OUT     transformation from (cosine, sine, z) coordinates to global xyz.  May be NULL.
//! @param pInverse    OUT     inverse of frame.  May be NULL.
//! @return true if the requested frames were returned.
//! @group "DEllipse3d Queries"
//!
Public GEOMDLLIMPEXP bool    bsiDEllipse3d_getLocalFrameAsTransform
(
DEllipse3dCP pEllipse,
TransformP   pFrame,
TransformP   pInverse
);

//!
//! @description Get the coordinate frame and inverse of an ellipse as viewed along the global z axis.
//! @param pEllipse    IN      ellipse whose frame is computed.
//! @param pFrame      OUT     transformation from (cosine, sine, z) coordinates to global xyz.  May be NULL.
//! @param pInverse    OUT     inverse of frame.  May be NULL.
//! @return true if the requested frames were returned.
//! @group "DEllipse3d Queries"
//!
Public GEOMDLLIMPEXP bool    bsiDEllipse3d_getXYLocalFrame
(
DEllipse3dCP pEllipse,
DTransform3dP pFrame,
DTransform3dP pInverse
);

//!
//! @description Get the coordinate frame and inverse of an ellipse as viewed along the global z axis.
//! @param pEllipse    IN      ellipse whose frame is computed.
//! @param pFrame      OUT     transformation from (cosine, sine, z) coordinates to global xyz.  May be NULL.
//! @param pInverse    OUT     inverse of frame.  May be NULL.
//! @return true if the requested frames were returned.
//! @group "DEllipse3d Queries"
//!
Public GEOMDLLIMPEXP bool    bsiDEllipse3d_getXYLocalFrameAsTransform
(
DEllipse3dCP pEllipse,
TransformP pFrame,
TransformP pInverse
);



//!
//! @description Get the coordinate directions and inverse of an ellipse as viewed along the global z axis.
//! @remarks This is the same as ~mbsiDEllipse3d_getXYLocalFrame, but ignores ellipse center.
//! @param pEllipse      IN      ellipse whose orientation is computed.
//! @param pMatrix        OUT     matrix from (cosine, sine, z) coordinates to global.  May be NULL.
//! @param pInverse       OUT     inverse of orientation.  May be NULL.
//! @return true if the requested orientations were returned.
//! @group "DEllipse3d Queries"
//!
Public GEOMDLLIMPEXP bool     bsiDEllipse3d_getXYLocalOrientation
(
DEllipse3dCP pEllipse,
DMatrix3dP pMatrix,
DMatrix3dP pInverse
);

//!
//! @description Compute the local coordinates of a point in the skewed coordinates of the ellipse, using
//! only xy parts of both the ellipse and starting point.
//! @remarks This is equivalent to computing the intersection of the ellipse plane with a line through the point and
//! parallel to the z axis, and returning the coordinates of the intersection relative to the
//! skewed axes of the ellipse.
//! @param pEllipse IN      ellipse to evaluate
//! @param pLocalPoint OUT     evaluated point.  Coordinates x,y are multipliers for the ellipse axes.
//!                        Coordinate z is height of the initial point from the plane of the ellipse.
//! @param pPoint IN      point to convert to local coordinates
//! @return true if ellipse axes are independent.
//! @group "DEllipse3d Closest Point"
//!
Public GEOMDLLIMPEXP bool    bsiDEllipse3d_pointToXYLocal
(
DEllipse3dCP pEllipse,
DPoint3dP pLocalPoint,
DPoint3dCP pPoint
);

//!
//! @description Compute the angular position of the point relative to the ellipse's local coordinates.
//! @remarks If the point is on the ellipse, this is the inverse of evaluating the ellipse at the angle.
//! @param pEllipse IN      ellipse definining angular space
//! @param pPoint IN      point to evaluate
//! @return angle in ellipse parameterization
//! @group "DEllipse3d Closest Point"
//!
Public GEOMDLLIMPEXP double   bsiDEllipse3d_pointToAngle
(
DEllipse3dCP pEllipse,
DPoint3dCP pPoint
);

//!
//! @description Project a point onto the plane of the ellipse.
//!
//! @param pEllipse IN      ellipse whose axes become 3d plane directions.
//! @param pXYZNear OUT     projection of point onto ellipse plane
//! @param pCoff0 OUT     coefficient on vector towards 0 degree point
//! @param pCoff90 OUT     coefficient on vector towards 90 degree point
//! @param pXYZ IN      point to project onto plane
//! @return true if the plane is well defined.
//! @group "DEllipse3d Closest Point"
//!
Public GEOMDLLIMPEXP bool    bsiDEllipse3d_projectPointToPlane
(
DEllipse3dCP pEllipse,
DPoint3dP pXYZNear,
double      *pCoff0,
double      *pCoff90,
DPoint3dCP pXYZ
);

//!
//! @description Compute an estimated number of points needed to stroke a full ellipse to within the given chord height tolerance.
//! @param pEllipse IN      ellipse to be stroked
//! @param n IN      default number of points on full ellipse (used if tol OUT     0.0)
//! @param nMax IN      max number of points on full ellipse (minimum is 4)
//! @param tol IN      tolerance for stroking
//! @return number of strokes required on the full ellipse
//! @group "DEllipse3d Queries"
//!
Public GEOMDLLIMPEXP int bsiDEllipse3d_getStrokeCount
(
DEllipse3dCP pEllipse,
int             n,
int             nMax,
double          tol
);

//!
//! @description Evaluate an ellipse using given coefficients for the axes.
//! @remarks If the x,y components of the coefficients define a unit vector, the point is "on" the ellipse.
//! @param pEllipse IN      ellipse to evaluate
//! @param pPoint OUT     array of cartesian points
//! @param pTrig IN      array of local coords (e.g., (cos, sin)).
//! @param numPoint IN      number of pairs
//! @group "DEllipse3d Evaluation"
//!
Public GEOMDLLIMPEXP void bsiDEllipse3d_evaluateTrigPairs
(
DEllipse3dCP pEllipse,
DPoint3dP pPoint,
DPoint2dCP pTrig,
int       numPoint
);

//!
//! @description Evaluate an ellipse at a number of (cosine, sine) pairs, removing
//! pairs whose corresponding angle is not in range.
//!
//! @param pEllipse IN      ellipse to evaluate
//! @param pPoint OUT     array of cartesian points
//! @param pTrig IN      array of local coords
//! @param numPoint IN      number of pairs
//! @return number of points found to be in the angular range of the ellipse.
//! @group "DEllipse3d Evaluation"
//!
Public GEOMDLLIMPEXP int bsiDEllipse3d_testAndEvaluateTrigPairs
(
DEllipse3dCP pEllipse,
DPoint3dP pPoint,
DPoint2dCP pTrig,
int       numPoint
);

//!
//! @description Test if a specified angle is within the sweep of the ellipse.
//! @param pEllipse IN      ellipse whose angular range is queried
//! @param angle IN      angle (radians) to test
//! @return true if angle is within the sweep angle of the elliptical arc.
//! @group "DEllipse3d Angles"
//!
Public GEOMDLLIMPEXP bool        bsiDEllipse3d_angleInSweep
(
DEllipse3dCP pEllipse,
double      angle
);

//!
//! @description Convert an angular parameter to a fraction of bounded arc length.
//! @param pEllipse IN      ellipse whose angular range is queried
//! @param angle      IN      angle (radians) to convert
//! @return fractional parameter
//! @group "DEllipse3d Evaluation"
//!
Public GEOMDLLIMPEXP double     bsiDEllipse3d_angleToFraction
(
DEllipse3dCP pEllipse,
double      angle
);

//!
//! @description Get the start and end angles of the ellipse.
//! @param pEllipse IN      ellipse whose angular range is queried
//! @param pStartAngle OUT     start angle
//! @param pEndAngle OUT     end angle
//! @group "DEllipse3d Angles"
//!
Public GEOMDLLIMPEXP void     bsiDEllipse3d_getLimits
(
DEllipse3dCP pEllipse,
double    *pStartAngle,
double    *pEndAngle
);

//!
//! @description Get the start and sweep angles of the ellipse.
//! @param pEllipse IN      ellipse whose angular range is queried.
//! @param pStartAngle OUT     start angle
//! @param pSweep OUT     sweep angle
//! @group "DEllipse3d Angles"
//!
Public GEOMDLLIMPEXP void     bsiDEllipse3d_getSweep
(
DEllipse3dCP pEllipse,
double    *pStartAngle,
double    *pSweep
);

//!
//! @description Set the start and end angles of the ellipse.
//! @param pEllipse OUT     ellipse whose angular range is changed
//! @param startAngle IN      start angle
//! @param endAngle   IN      end angle
//! @group "DEllipse3d Angles"
//!
Public GEOMDLLIMPEXP void     bsiDEllipse3d_setLimits
(
DEllipse3dP pEllipse,
double    startAngle,
double    endAngle
);

//!
//! @description Set the start and sweep angles of the ellipse.
//! @param pEllipse OUT     ellipse whose angular range is changed
//! @param startAngle IN      start angle
//! @param sweep      IN      sweep angle
//! @group "DEllipse3d Angles"
//!
Public GEOMDLLIMPEXP void     bsiDEllipse3d_setSweep
(
DEllipse3dP pEllipse,
double    startAngle,
double    sweep
);

//!
//! @description Make a copy of the source ellipse, altering the axis vectors and angular limits so that
//! the revised ellipse has perpendicular axes in the conventional major/minor axis form.
//! @remarks Inputs may be the same.
//! @param pEllipse OUT     ellipse with perpendicular axes
//! @param pSource IN      ellipse with unconstrained axes
//! @group "DEllipse3d Initialization"
//!
Public GEOMDLLIMPEXP void bsiDEllipse3d_initWithPerpendicularAxes
(
DEllipse3dP pEllipse,
DEllipse3dCP pSource
);

//!
//! @description Compute the range box of the ellipse in its major-minor axis coordinate system.
//! Compute line segments that are the horizontal and vertical midlines in that system.
//! Return those line segments ordered with the longest first, and return the shorter length.
//!
//! @remarks The typical use of this is that if the shorter length is less than some tolerance the
//! points swept out by the ellipse are the longer segment.  (But beware that the start and
//! end points of the segment can be other than the start and end points of the ellipse.)
//!
//! @param pEllipse   IN      ellipse to analyze
//! @param pLongSegment  OUT     longer axis of local conic range box
//! @param pShortSegment OUT     shorter axis of local conic range box
//! @return size of the shorter dimension
//! @group "DEllipse3d Queries"
//!
Public GEOMDLLIMPEXP double           bsiDEllipse3d_getMajorMinorRangeMidlines
(
DEllipse3dCP pEllipse,
DSegment3dP pLongSegment,
DSegment3dP pShortSegment
);

//!
//! @description Make a copy of the source ellipse, reversing the start and end angles.
//! @remarks Inputs may be the same.
//! @param pEllipse   OUT     copied and reversed ellipse
//! @param pSource IN      source ellipse
//! @group "DEllipse3d Initialization"
//!
Public GEOMDLLIMPEXP void bsiDEllipse3d_initReversed
(
DEllipse3dP pEllipse,
DEllipse3dCP pSource
);

//!
//! @description Compute the magnitude of the tangent vector to the ellipse at the specified angle.
//! @param pEllipse IN      ellipse to evaluate
//! @param theta IN      angular parameter
//! @return tangent magnitude
//! @group "DEllipse3d Evaluation"
//!
Public GEOMDLLIMPEXP double bsiDEllipse3d_tangentMagnitude
(
DEllipse3dCP pEllipse,
double      theta
);

//!
//! @description Return arc length of ellipse.
//! @param pEllipse IN ellipse to integrate
//! @return arc length of ellipse.
//! @group "DEllipse3d Queries"
//!
Public GEOMDLLIMPEXP double bsiDEllipse3d_arcLength (DEllipse3dCP pEllipse);

//!
//! @description Compute the signed arc length of the ellipse.
//! Negative sweep produces negative arc length, so the return from this
//! can be negative.
//! @param pEllipse IN ellipse.
//! @return arc length of ellipse
//! @group "DEllipse3d Queries"
//!
Public GEOMDLLIMPEXP double bsiDEllipse3d_arcLengthAdaptive (DEllipse3dCP pEllipse);

//!
//! @description Return the sweep angle corresponding to an arc length.
//! @remarks Negative returned sweep angle corresponds to arclength traversed in the opposite direction of the ellipse sweep.
//! @param pEllipse IN ellipse to integrate
//! @param arcLength IN arc length to invert
//! @return sweep angle
//! @group "DEllipse3d Queries"
//!
Public GEOMDLLIMPEXP double bsiDEllipse3d_inverseArcLength
(
DEllipse3dCP pEllipse,
double arcLength
);

//!
//! @description Compute the (signed) arc length between specified fractional parameters.
//! @remarks Fractions outside [0,1] return error.
//! @param pEllipse IN      ellipse to measure.
//! @param pArcLength OUT     computed arc length.  Negative if fraction1 < fraction0.
//! @param fraction0 IN      start fraction for interval to measure
//! @param fraction1 IN      end fraction for interval to measure
//! @return true if the arc length was computed.
//! @group "DEllipse3d Queries"
//!
Public GEOMDLLIMPEXP bool     bsiDEllipse3d_fractionToLength
(
DEllipse3dCP pEllipse,
double      *pArcLength,
double      fraction0,
double      fraction1
);

//!
//! @description Compute the xyz range limits of a 3D ellipse.
//! @param pEllipse IN      ellipse whose range is determined
//! @param pRange OUT     computed range
//! @group "DEllipse3d Queries"
//!
Public GEOMDLLIMPEXP void bsiDEllipse3d_getRange
(
DEllipse3dCP pEllipse,
DRange3dP pRange
);

//!
//! @description Compute the range of the ellipse in its own coordinate system..
//! @remarks This depends on the start and sweep angles but not the center or axis coordinates.
//! @param pEllipse IN      ellipse whose range is determined
//! @param pRange OUT     computed range
//! @group "DEllipse3d Queries"
//!
Public GEOMDLLIMPEXP void bsiDEllipse3d_getLocalRange
(
DEllipse3dCP pEllipse,
DRange2dP pRange
);

//!
//! @description Find intersections of a (full) ellipse with a plane.
//! @remarks Return value n=1 is a single tangency point returned in pTrigPoints[0];
//!        n=2 is two simple intersections returned in pTrigPoints[0..1]
//! @remarks The three component values in pTrigPoints are:
//! <UL>
//! <LI>x == cosine of angle
//! <LI>y == sine of angle
//! <LI>z == angle in radians
//! </UL>
//! @param pEllipse      IN      ellipse to intersect with plane
//! @param pTrigPoints    OUT     2 points: cosine, sine, theta values of plane intersection
//! @param pPlane          IN      homogeneous plane equation
//! @return The number of intersections, i.e. 0, 1, or 2
//! @group "DEllipse3d Intersection"
//!
Public GEOMDLLIMPEXP int bsiDEllipse3d_intersectPlane
(
DEllipse3dCP pEllipse,
DPoint3dP pTrigPoints,
DPoint4dCP pPlane
);

//!
//! @description Find the intersections of xy projections of an ellipse and line.
//! @remarks May return 0, 1, or 2 points.  Both ellipse and line are unbounded.
//! @param pEllipse      IN      ellipse to intersect with line
//! @param pCartesianPoints   OUT     cartesian intersection points
//! @param pLineParams        OUT     array of line parameters (0=start, 1=end)
//! @param pEllipseCoffs      OUT     array of coordinates relative to the ellipse.
//!                              For each point, (xy) are the cosine and sine of the
//!                              ellipse parameter, (z) is z distance from the plane of
//!                              of the ellipse.
//! @param pEllipseAngle      OUT     array of angles on the ellipse
//! @param pStartPoint    IN      line start
//! @param pEndPoint      IN      line end
//! @return the number of intersections.
//! @group "DEllipse3d Intersection"
//!
Public GEOMDLLIMPEXP int bsiDEllipse3d_intersectXYLine
(
DEllipse3dCP pEllipse,
DPoint3dP pCartesianPoints,
double        *pLineParams,
DPoint3dP pEllipseCoffs,
double        *pEllipseAngle,
DPoint3dCP pStartPoint,
DPoint3dCP pEndPoint
);

//!
//! @description Test if the ellipse is circular.
//! @param pEllipse IN ellipse to test
//! @return true if circular
//! @group "DEllipse3d Queries"
//!
Public GEOMDLLIMPEXP bool    bsiDEllipse3d_isCircular (DEllipse3dCP pEllipse);

//!
//! @description Test if the XY projection of the ellipse is circular.
//! @param pEllipse IN ellipse to test
//! @return true if circular
//! @group "DEllipse3d Queries"
//!
Public GEOMDLLIMPEXP bool     bsiDEllipse3d_isCircularXY (DEllipse3dCP pEllipse);

//!
//! @description Find the intersections of xy projections of two ellipses.
//! @remarks May return 0, 1, 2, 3 or 4 points.  Both ellipses are unbounded.
//! @param pEllipse0      IN      ellipse to intersect with line.
//! @param pCartesianPoints   OUT     cartesian intersection points.
//! @param pEllipse0Params    OUT     array of coordinates relative to the first ellipse.
//!                            For each point, (xy) are the cosine and sine of the
//!                            ellipse parameter, (z) is z distance from the plane of
//!                            of the ellipse.
//! @param pEllipse1Params    OUT     array of coordinates relative to the second ellipse.
//!                            For each point, (xy) are the cosine and sine of the
//!                            ellipse parameter, (z) is z distance from the plane of
//!                            of the ellipse.
//! @param pEllipse1       IN      the other ellipse.
//! @return the number of intersections.
//! @group "DEllipse3d Intersection"
//!
Public GEOMDLLIMPEXP int bsiDEllipse3d_intersectXYDEllipse3d
(
DEllipse3dCP pEllipse0,
DPoint3dP pCartesianPoints,
DPoint3dP pEllipse0Params,
DPoint3dP pEllipse1Params,
DEllipse3dCP pEllipse1
);

//!
//! @description Find the intersections of xy projections of two ellipses, with bounds applied.
//! @remarks May return 0, 1, 2, 3 or 4 points.
//! @param pEllipse0      IN      ellipse to intersect with line.
//! @param pCartesianPoints   OUT     cartesian intersection points.
//! @param pEllipse0Coffs    OUT     array of coordinates relative to the first ellipse
//!                            For each point, (xy) are the cosine and sine of the
//!                            ellipse parameter, (z) is z distance from the plane of
//!                            of the ellipse.
//! @param pEllipse0Angle OUT     array of angles on the first ellipse
//! @param pEllipse1Coffs     OUT     array of coordinates relative to the second ellipse.
//!                            For each point, (xy) are the cosine and sine of the
//!                            ellipse parameter, (z) is z distance from the plane of
//!                            of the ellipse.
//! @param pEllipse1Angle OUT     array of angles on the other ellipse
//! @param pEllipse1       IN      the other ellipse.
//! @return the number of intersections.
//! @group "DEllipse3d Intersection"
//!
Public GEOMDLLIMPEXP int bsiDEllipse3d_intersectXYDEllipse3dBounded
(
DEllipse3dCP pEllipse0,
DPoint3dP pCartesianPoints,
DPoint3dP pEllipse0Coffs,
double        *pEllipse0Angle,
DPoint3dP pEllipse1Coffs,
double        *pEllipse1Angle,
DEllipse3dCP pEllipse1
);

//!
//! @description Find "intersections" of two DEllipse3d.
//! @remarks Ellipses in space can pass very close to
//! each other without intersecting, so some logic must be applied to define intersection
//! more cleanly. The logic applied is to choose the more circular ellipse and apply the
//! transformation which makes that one a unit circle, then intersect the xy projections of the
//! transformations.
//!
//! @param pEllipse0           IN      ellipse to intersect with line.
//! @param pCartesianPoints    OUT     cartesian intersection points.
//! @param pEllipse0Params     OUT     array of coordinates relative to the first ellipse
//!                                For each point, (xy) are the cosine and sine of the
//!                                ellipse parameter, (z) is z distance from the plane of
//!                                of the ellipse.
//! @param pEllipse1Params      OUT     array of coordinates relative to the second ellipse.
//!                                For each point, (xy) are the cosine and sine of the
//!                                ellipse parameter, (z) is z distance from the plane of
//!                                of the ellipse.
//! @param pEllipse1            IN      the other ellipse.
//! @return the number of intersections.
//! @group "DEllipse3d Intersection"
//!
Public GEOMDLLIMPEXP int bsiDEllipse3d_intersectSweptDEllipse3d
(
DEllipse3dCP pEllipse0,
DPoint3dP pCartesianPoints,
DPoint3dP pEllipse0Params,
DPoint3dP pEllipse1Params,
DEllipse3dCP pEllipse1
);

//!
//! @description Intersect two ellipses as described in ~mbsiDEllipse3d_intersectSweptDEllipse3d, and
//! filter out results not part of both ranges.
//!
//! @param pEllipse0      IN      ellipse to intersect with cylinder of second ellipse.
//! @param pCartesianPoints   OUT     cartesian intersection points.
//! @param pEllipse0Coffs    OUT     array of coordinates relative to the first ellipse
//!                            For each point, (xy) are the cosine and sine of the
//!                            ellipse parameter, (z) is z distance from the plane of
//!                            of the ellipse.
//! @param pEllipse0Angle    OUT     array of angles on the first ellipse.
//! @param pEllipse1Coffs     OUT     array of coordinates relative to the second ellipse.
//!                            For each point, (xy) are the cosine and sine of the
//!                            ellipse parameter, (z) is z distance from the plane of
//!                            of the ellipse.
//! @param pEllipse1Angle   OUT     array of angles on the other ellipse.
//! @param pEllipse1       IN      the other ellipse.
//! @return the number of intersections.
//! @group "DEllipse3d Intersection"
//!
Public GEOMDLLIMPEXP int bsiDEllipse3d_intersectSweptDEllipse3dBounded
(
DEllipse3dCP pEllipse0,
DPoint3dP pCartesianPoints,
DPoint3dP pEllipse0Coffs,
double        *pEllipse0Angle,
DPoint3dP pEllipse1Coffs,
double        *pEllipse1Angle,
DEllipse3dCP pEllipse1
);

//!
//! @description Find "intersections" of a DSegment3d and a DEllipse3d.
//! @remarks Curves in space can pass very close to
//! each other without intersecting, so some logic must be applied to define intersection
//! more cleanly. The logic applied is to compute the intersection of the line with
//! the cylinder swept by the ellipse along its plane normal.
//!
//! @param pEllipse      IN      base ellipse for the cylinder.
//! @param pPointArray   OUT     cartesian intersection points.
//! @param pEllipseParams     OUT     array of coordinates relative to the instance
//!                            For each point, (xy) are the cosine and sine of the
//!                            ellipse parameter, (z) is z distance from the plane of
//!                            of the ellipse.
//! @param pLineParams     OUT     array of parametric coordinates on the line.
//! @param pSegment       IN      the line segment
//! @return the number of intersections.
//!  @group "DEllipse3d Intersection"
//!
Public GEOMDLLIMPEXP int bsiDEllipse3d_intersectSweptDSegment3d
(
DEllipse3dCP pEllipse,
DPoint3dP pPointArray,
DPoint3dP pEllipseParams,
double        *pLineParams,
DSegment3dCP pSegment
);

//!
//! @description Intersect an ellipse and a segment as described in ~mbsiDEllipse3d_intersectSweptDSegment3d, and
//! filter out results not part of both ranges.
//!
//! @param pEllipse      IN      base ellipse for the cylinder.
//! @param pPointArray   OUT     cartesian intersection points.
//! @param pEllipseParams     OUT     array of coordinates relative to the instance
//!                            For each point, (xy) are the cosine and sine of the
//!                            ellipse parameter, (z) is z distance from the plane of
//!                            of the ellipse.
//! @param pLineParams     OUT     array of parametric coordinates on the line.
//! @param pSegment       IN      the line segment
//! @return the number of intersections.
//! @group "DEllipse3d Intersection"
//!
Public GEOMDLLIMPEXP int bsiDEllipse3d_intersectSweptDSegment3dBounded
(
DEllipse3dCP pEllipse,
DPoint3dP pPointArray,
DPoint3dP pEllipseParams,
double        *pLineParams,
DSegment3dCP pSegment
);

//!
//! @description Project a point onto the (unbounded) ellipse.
//! @remarks May return up to 4 points.
//! @param pEllipse IN ellipse to search
//! @param pCartesianPoints OUT array (allocated by caller) of points on the ellipse.
//! @param pEllipseAngle OUT array (allocated by caller) of ellipse angles.
//! @param pPoint IN space point
//! @return the number of projection points
//! @group "DEllipse3d Closest Point"
//!
Public GEOMDLLIMPEXP int bsiDEllipse3d_projectPoint
(
DEllipse3dCP pEllipse,
DPoint3dP pCartesianPoints,
double        *pEllipseAngle,
DPoint3dCP pPoint
);

//!
//! @description Project a point onto the xy projection of the (unbounded) ellipse.
//! @remarks May return up to 4 points.
//! @param pEllipse IN ellipse
//! @param pCartesianPoints OUT array (allocated by caller) of points on the ellipse.
//! @param pEllipseAngle OUT array (allocated by caller) of ellipse angles.
//! @param pPoint IN space point
//! @return the number of projection points
//! @group "DEllipse3d Closest Point"
//!
Public GEOMDLLIMPEXP int bsiDEllipse3d_projectPointXY
(
DEllipse3dCP pEllipse,
DPoint3dP pCartesianPoints,
double        *pEllipseAngle,
DPoint3dCP pPoint
);

//!
//! @description Project a point to the xy projection of the ellipse, and apply sector bounds.
//! @remarks May return up to 4 points.
//! @param pEllipse IN ellipse
//! @param pCartesianPoints OUT array (allocated by caller) of points on the ellipse.
//! @param pEllipseAngle OUT array (allocated by caller) of ellipse angles.
//! @param pPoint IN space point
//! @return the number of projection points
//! @group "DEllipse3d Closest Point"
//!
Public GEOMDLLIMPEXP int bsiDEllipse3d_projectPointXYBounded
(
DEllipse3dCP pEllipse,
DPoint3dP pCartesianPoints,
double        *pEllipseAngle,
DPoint3dCP pPoint
);

//!
//! @description Project a point to the xy projection of the ellipse, and apply sector bounds.
//! @remarks May return up to 4 points.
//! @param pEllipse IN ellipse
//! @param pCartesianPoints OUT array (allocated by caller) of points on the ellipse.
//! @param pEllipseAngle OUT array (allocated by caller) of ellipse angles.
//! @param pPoint IN space point
//! @return the number of projection points
//! @group "DEllipse3d Closest Point"
//!
Public GEOMDLLIMPEXP int bsiDEllipse3d_projectPointBounded
(
DEllipse3dCP pEllipse,
DPoint3dP pCartesianPoints,
double        *pEllipseAngle,
DPoint3dCP pPoint
);

//!
//! @description Find the closest point on a bounded ellipse, considering both endpoints and perpendicular
//! projections, and ignoring z of both the ellipse and space point.
//! @param pEllipse IN ellipse to search
//! @param pMinAngle OUT angular parameter at closest point
//! @param pMinDistSquared OUT squared distance to closest point
//! @param pMinPoint OUT closest point
//! @param pPoint IN space point
//! @return always true
//! @group "DEllipse3d Closest Point"
//!
Public GEOMDLLIMPEXP bool        bsiDEllipse3d_closestPointXYBounded
(
DEllipse3dCP pEllipse,
double        *pMinAngle,
double        *pMinDistSquared,
DPoint3dP pMinPoint,
DPoint3dCP pPoint
);

//!
//! @description Find the closest point on a bounded ellipse, considering both endpoints and perpendicular
//! projections.
//! @param pEllipse IN ellipse to search
//! @param pMinAngle OUT angular parameter at closest point
//! @param pMinDistSquared OUT squared distance to closest point
//! @param pMinPoint OUT closest point
//! @param pPoint IN space point
//! @return always true
//! @group "DEllipse3d Closest Point"
//!
Public GEOMDLLIMPEXP bool        bsiDEllipse3d_closestPointBounded
(
DEllipse3dCP pEllipse,
double        *pMinAngle,
double        *pMinDistSquared,
DPoint3dP pMinPoint,
DPoint3dCP pPoint
);

//!
//! @description Find the closest point on a complete ellipse or its contained disk.
//! @param pEllipse IN ellipse to search
//! @param pLocalX OUT local x coordinate.
//! @param pLocalY OUT local y coordinate.
//! @param pMinDistSquared OUT squared distance to closest point
//! @param pMinPoint OUT closest point
//! @param pSpacePoint IN space point
//! @return always true
//! @group "DEllipse3d Closest Point"
//!
Public GEOMDLLIMPEXP bool        bsiDEllipse3d_closestPointOnDisk
(
DEllipse3dCP pEllipse,
double        *pLocalX,
double        *pLocalY,
double        *pMinDistSquared,
DPoint3dP pMinPoint,
DPoint3dCP pSpacePoint
);

//!
//! @description Find the intersections of xy projections of an ellipse and line, applying both ellipse and line parameter bounds.
//! @param pEllipse      IN      ellipse to intersect with line.
//! @param pCartesianPoints   OUT     cartesian intersection points.
//! @param pLineParams         OUT     array of line parameters (0=start, 1=end)
//! @param pEllipseCoffs      OUT     array of intersection coordinates in ellipse
//!                                frame.   xy are cosine and sine of angles.
//!                                z is z distance from plane of ellipse.
//! @param pEllipseAngle      OUT     array of angles in ellipse parameter space.
//! @param pIsTangency    OUT     true if the returned intersection is a tangency.
//! @param pStartPoint    IN      line start
//! @param pEndPoint      IN      line end
//! @return the number of intersections after applying ellipse and line parameter limits.
//! @group "DEllipse3d Intersection"
//!
Public GEOMDLLIMPEXP int bsiDEllipse3d_intersectXYLineBounded
(
DEllipse3dCP pEllipse,
DPoint3dP pCartesianPoints,
double        *pLineParams,
DPoint3dP pEllipseCoffs,
double        *pEllipseAngle,
bool          *pIsTangency,
DPoint3dCP pStartPoint,
DPoint3dCP pEndPoint
);

//!
//! @description Compute area and swept angle as seen from given point.
//! @param pEllipse IN ellipse
//! @param pArea OUT swept area
//! @param pSweep OUT swept angle (in radians)
//! @param pPoint IN base point for sweep line.
//! @group "DEllipse3d Queries"
//!
Public GEOMDLLIMPEXP void bsiDEllipse3d_xySweepProperties
(
DEllipse3dCP pEllipse,
double        *pArea,
double        *pSweep,
DPoint3dCP pPoint
);

//!
//! @description Apply a transformation to the source ellipse.
//! @param pDest OUT     transformed ellipse
//! @param pTransform IN      transformation to apply
//! @param pSource IN      source ellipse
//! @group "DEllipse3d Transform"
//!
Public GEOMDLLIMPEXP void     bsiDEllipse3d_transform
(
DEllipse3dP pDest,
DTransform3dCP pTransform,
DEllipse3dCP pSource
);

//!
//! @description Apply a transformation to the source ellipse.
//! @param pDest OUT     transformed ellipse
//! @param pTransform IN      transformation to apply
//! @param pSource IN      source ellipse
//! @group "DEllipse3d Transform"
//!
Public GEOMDLLIMPEXP void     bsiDEllipse3d_multiplyTransformDEllipse3d
(
DEllipse3dP pDest,
TransformCP pTransform,
DEllipse3dCP pSource
);

//!
//! @description Construct a circular fillet arc from 3 points of the unfilleted corner.
//! @param pEllipse      OUT     fillet arc
//! @param pFilletStart  OUT     tangency point on first line.
//! @param pFilletEnd    OUT     tangency point on second line.
//! @param pPoint0       IN      first point of unfilleted chain
//! @param pPoint1       IN      second (middle) point of unfilleted chain
//! @param pPoint2       IN      third point of unfilleted chain
//! @param radius      IN      radius of fillet
//! @return true if fillet construted.
//! @group "DEllipse3d Initialization"
//!
Public GEOMDLLIMPEXP bool          bsiDEllipse3d_constructFillet
(
DEllipse3dP pEllipse,
DPoint3dP pFilletStart,
DPoint3dP pFilletEnd,
DPoint3dCP pPoint0,
DPoint3dCP pPoint1,
DPoint3dCP pPoint2,
double          radius
);

//!
//! @description Return the (weighted) control points of quadratic beziers which
//!   combine to represent the full conic section.
//!
//! @param pEllipse IN      ellipse to query
//! @param pPoleArray OUT     array of poles of composite quadratic Bezier curve.  Array count depends on the angular extent.
//! @param pCirclePoleArray OUT     array of corresponding poles which
//!            map the bezier polynomials back to the unit circle points (x,y,w)
//!            where x^2 + y^2 = w^2, but beware that w is not identically 1!  (Use this to map from bezier parameter back to angles.)
//! @param pNumPole OUT     number of poles returned
//! @param pNumSpan OUT     number of spans returned.  The pole indices for the first span are (always) 0,1,2; for the second span (if present),
//!                    2,3,4, and so on.
//! @param maxPole IN      maximum number of poles desired.  maxPole must be at least
//!                5.  The circle is split into (maxPole - 1) / 2 spans.
//!                Beware that for 5 poles the circle is split into at most
//!                two spans, and there may be zero weights.   For 7 or more poles
//!                all weights can be positive.  The function may return fewer
//!                poles.
//! @group "DEllipse3d Queries"
//!
Public GEOMDLLIMPEXP void     bsiDEllipse3d_getQuadricBezierPoles
(
DEllipse3dCP pEllipse,
DPoint4dP pPoleArray,
DPoint3dP pCirclePoleArray,
int       *pNumPole,
int       *pNumSpan,
int       maxPole
);

//!
//! @description Return the (weighted) control points of a quadratic bezier
//!   for one sector of the arc.  Sector size is arc sweep divided by numSector.
//!   There are no checks for conventional sector size limits or whether the index
//!   is within 0..numSector-1.
//!
//! @param pEllipse IN      ellipse to query
//! @param pPoleArray OUT     array of 3 poles.
//! @param pCirclePoleArray OUT     array of 3 poles in unit circle space.
//! @param sectorIndex IN      integer index of sector.
//! @param numSector IN      number of sectors in overall arc.  The computed sector sweep is the total sweep divided
//!    by numSector.
//! @group "DEllipse3d Queries"
//!
Public GEOMDLLIMPEXP void     bsiDEllipse3d_getQuadricBezierSectorPoles
(
DEllipse3dCP pEllipse,
DPoint4dP pPoleArray,
DPoint3dP pCirclePoleArray,
int sector,
int numSector
);

//!
//! @description Compute an array of 5 points such that points 0,1,2 and 2,3,4 define arcs which have a common tangent at point 2, and have
//!    specified start and end points and tangents.
//! @remarks At least one of the tangents should not be parallel to the vector between the points.
//! @remarks All inputs should be coplanar.
//! @param pArcPoints OUT     array of 5 points
//! @param pPoint0 IN      start point
//! @param pPoint1 IN      end point
//! @param pTangent0 IN      tangent at start
//! @param pTangent1 IN      tangent at end
//! @return true if the arcs are computed
//! @group "DEllipse3d Initialization"
//!
Public GEOMDLLIMPEXP bool     bsiDEllipse3d_compute3PointBiarcs
(
DPoint3dP pArcPoints,
DPoint3dP pPoint0,
DPoint3dP pPoint1,
DPoint3dP pTangent0,
DPoint3dP pTangent1
);

//!
//! @description Initialize an ellipse from center, primary axis point, and additional pass-though point.
//! @param pEllipse OUT initialized ellipse
//! @param pCenter IN      center point of ellipse.
//! @param pPoint0 IN      point to appear at the zero degree point.   The ellipse must pass through
//!                this point as a major or minor axis point, i.e. its tangent must be perpendicular
//!                to the vector from the center to this point.
//! @param pPoint1 IN      additional pass-through point.
//! @return false if center, point0 and point1 are not independent, or if
//!    point1 is too far away from center to allow ellipse constrution.
//! @group "DEllipse3d Initialization"
//!
Public GEOMDLLIMPEXP bool    bsiDEllipse3d_initFromCenterMajorAxisPointAndThirdPoint
(
DEllipse3dP pEllipse,
DPoint3dCP pCenter,
DPoint3dCP pPoint0,
DPoint3dCP pPoint1
);

//!
//! @description Return an array of up to 4 points where a ray has closest approach to an ellipse.
//! @remarks Both ellipse and ray are unbounded.
//! @param pEllipse IN ellipse to search
//! @param pEllipseAngleBuffer OUT array (allocated by caller) to hold 4 angles on ellipse
//! @param pRayFractionBuffer OUT array (allocated by caller) to hold 4 fractions on ray
//! @param pEllipsePointBuffer OUT array (allocated by caller) to hold 4 ellipse points
//! @param pRayPointBuffer OUT array (allocated by caller) to hold 4 ray points
//! @param pRay IN ray to search
//! @return number of approach points computed.
//! @group "DEllipse3d Closest Point"
//!
Public GEOMDLLIMPEXP int             bsiDEllipse3d_closestApproachDRay3d
(
DEllipse3dCP pEllipse,
double          *pEllipseAngleBuffer,
double          *pRayFractionBuffer,
DPoint3dP pEllipsePointBuffer,
DPoint3dP pRayPointBuffer,
DRay3dCP pRay
);

//!
//! @description Fill in ellipse data from data fields in DGN 3d ellipse element.
//! @param pEllipse OUT initialized DEllipse3d
//! @param pCenter IN center of ellipse.
//! @param pQuatWXYZ IN array of 4 doubles (ordered w,x,y,z) with quaternion for orthogonal frame.  If this is NULL,
//!           major and minor directions must be supplied as pDirection0 and pDirection90;
//! @param pDirectionX IN vector in the x axis direction.  This is scaled by rX. (It is NOT normalized before
//!                scaling.  In common use, it will be a unit vector.)
//! @param pDirectionY IN vector in the y axis direction.  This is scaled by rY. (It is NOT normalized before
//!                scaling.  In common use, it will be a unit vector.)
//! @param rX IN scale factor (usually a true distance) for x direction.
//! @param rY IN scale factor (usually a true distance) for y direction.
//! @param pStart IN optional start angle.  Defaults to zero
//! @param pSweep IN optional sweep angle.  Defaults to 2pi.
//! @group "DEllipse3d DGN Elements"
//!
Public GEOMDLLIMPEXP void bsiDEllipse3d_initFromDGNFields3d
(
DEllipse3dP     pEllipse,
DPoint3dCP      pCenter,
double const*   pQuatWXYZ,
DVec3dCP        pDirectionX,
DVec3dCP        pDirectionY,
double          rX,
double          rY,
double const*   pStart,
double const*   pSweep
);

//!
//! @description Fill in ellipse data from data fields in DGN 2d ellipse element.
//! @param pEllipse OUT initialized DEllipse3d
//! @param pCenter IN center of ellipse.
//! @param pXAngle IN optional angle from global x axis to local x axis.
//! @param pDirection0 IN optional vector form of ellipse x axis direction.
//! @param rX IN scale factor for ellipse x direction.
//! @param rY IN scale factor for ellipse y direction.
//! @param pStart IN optional start angle.  Defaults to zero
//! @param pSweep IN optional sweep angle.  Defaults to 2pi.
//! @param zDepth IN z value for ellipse.
//! @group "DEllipse3d DGN Elements"
//!
Public GEOMDLLIMPEXP void bsiDEllipse3d_initFromDGNFields2d
(
DEllipse3dP     pEllipse,
DPoint2dCP      pCenter,
double const*   pXAngle,
DVec2dCP        pDirection0,
double          rX,
double          rY,
double const*   pStart,
double const*   pSweep,
double          zDepth
);

//!
//! @description Fill in ellipse data from data fields in DGN 3d ellipse element.
//! @param pEllipse IN initialized DEllipse3d
//! @param pCenter OUT center of ellipse.
//! @param pQuatWXYZ OUT quaternion for orthogonal frame.
//!            As per DGN convention, ordered WXYZ.
//! @param pDirectionX OUT unit vector in ellipse x direction.
//! @param pDirectionY OUT unit vector in ellipse y direction.
//! @param pRX OUT scale factor (usually a true distance) for x direction.
//! @param pRY OUT scale factor (usually a true distance) for y direction.
//! @param pStartAngle OUT optional start angle.  Defaults to zero
//! @param pSweepAngle OUT optional sweep angle.  Defaults to 2pi.
//! @group "DEllipse3d DGN Elements"
//!
Public GEOMDLLIMPEXP void bsiDEllipse3d_getDGNFields3d
(
DEllipse3dCP pEllipse,
DPoint3dP    pCenter,
double *    pQuatWXYZ,
DVec3dP     pDirectionX,
DVec3dP     pDirectionY,
double *    pRX,
double *    pRY,
double *    pStartAngle,
double *    pSweepAngle
);

//!
//! @param pEllipse IN initialized DEllipse3d
//! @param pCenter OUT center of ellipse.
//! @param pXAngle OUT angle from global x axis to local x axis.
//! @param pDirection0 OUT direction for x radius.
//! @param pRX OUT scale factor for ellipse x direction.
//! @param pRY OUT scale factor for ellipse y direction.
//! @param pStartAngle OUT optional start angle.
//! @param pSweepAngle OUT optional sweep angle.
//! @group "DEllipse3d DGN Elements"
//!
Public GEOMDLLIMPEXP void bsiDEllipse3d_getDGNFields2d
(
DEllipse3dCP    pEllipse,
DPoint2dP       pCenter,
double *        pXAngle,
DVec2dP         pDirection0,
double *        pRX,
double *        pRY,
double *        pStartAngle,
double *        pSweepAngle
);

//!
//! @param ellipse IN ellispe
//! @param localToGlobal OUT coordinate frame with origin at lower right of local range.
//! @param globalToLocal OUT transformation from world to local
//! @param range OUT ellipse range in the local coordinates.
//! @group "DEllipse3d Queries"
//!
bool    GEOMDLLIMPEXP bsiDEllipse3d_alignedRange
(
DEllipse3dCP ellipse,
TransformP   localToGlobal,
TransformP    globalToLocal,
DRange3dP     range
);

//!
//! @param ellipse IN ellipse
//! @return largest (absolute) coordinate or vector component.
//!
double GEOMDLLIMPEXP bsiDEllipse3d_maxAbs (DEllipse3dCP pInstance);


#ifdef __cplusplus
//!
//! @description Test if two ellipses are circular and parallel.  If so, compute intersections of (unbounded) circles.
//! If not, return with no computed points.
//! Note that true/false return does not correspond to intersecting and non-intersecting.   Two ellipses that
//! intersect will return false with no intersections.   Two coplanar circles with no intersections return
//! true with no intersections.
//! @remark The order of the returned points is as per legacy microstation l_intsec.c.
//! @param [out] pIntersectionPoints1 xyz coordinates on ellipse1.  May not be NULL.
//! @param [out] pIntersectionPoints2 xyz coordinates on ellipse2   May be NULL.
//! @param [out] pAngles1 angular position on ellipse1. May be NULL.
//! @param [out] pAngles2 angular position on ellipse2. May be NULL.
//! @param [in] pEllipse1 first ellipse
//! @param [in] pEllipse2 second ellipse
//! @param [in] tangency tolerance.  If 0, a default will be supplied based on ellipse coordinates.
//! @return true if (a) both ellipses are cicular (by isCircular),
//!   (b) the normals are parallel (by isParallelTo).
//!
bool    GEOMDLLIMPEXP bsiDEllipse3d_isParallelPlaneCircleCircleIntersect
(
bvector<DPoint3d>*pIntersectionPoints1,
bvector<DPoint3d>*pIntersectionPoints2,
bvector<double>*pAngles1,
bvector<double>*pAngles2,
DEllipse3dCP    pEllipse1,
DEllipse3dCP    pEllipse2,
double          tolerance
);

//! Modify an ellipse so that
//!  1) one endpoint and its tangent are preserved.
//!  2) sweep angle is preserved
//!  3) the other endpoint moves a specified vector distance.
//! @param [out] result transformed ellipse
//! @param [in] source original ellipse
//! @param [in] translation translation vector to apply.
//! @param [in] movingEndIndex 0 to move startpoint, 1 to move endpoint
bool GEOMDLLIMPEXP bsiDEllipse3d_translateEndPoint (DEllipse3dP result, DEllipse3dCP source, DVec3dCP translation, int movingEndIndex);

#endif
END_BENTLEY_GEOMETRY_NAMESPACE

