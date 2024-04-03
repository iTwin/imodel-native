/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! @description Fill in conic data.
//!
//! @param pConic OUT     initialized conic
//! @param cx IN      center x coordinate
//! @param cy IN      center y coordinate
//! @param cz IN      center z coordinate
//! @param ux IN      vector0 x coordinate
//! @param uy IN      vector0 y coordinate
//! @param uz IN      vector0 z coordinate
//! @param vx IN      vector90 x coordinate
//! @param vy IN      vector90 y coordinate
//! @param vz IN      vector90 z coordinate
//! @param theta0 IN      start angle in parameter space
//! @param sweep IN      sweep angle in parameter space
//! @group "DConic4d Initialization"
//!
Public GEOMDLLIMPEXP void bsiDConic4d_init
(
DConic4dP pConic,
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
//! @description Fill in conic data from 2D major and minor axis lengths and the angle
//!   from the global to the local x-axis.
//!
//! @param pConic OUT     initialized conic
//! @param cx IN      center x coordinate
//! @param cy IN      center y coordinate
//! @param cz IN      z coordinate of all points on the conic
//! @param rx IN      radius along local x axis
//! @param ry IN      radius along local y axis
//! @param thetaX IN      angle from global x-axis to local x-axis
//! @param theta0 IN      start angle in parameter space
//! @param sweep IN      sweep angle in parameter space
//! @group "DConic4d Initialization"
//!
Public GEOMDLLIMPEXP void bsiDConic4d_initFromXYMajorMinor
(
DConic4dP pConic,
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
//! @description Convert a cartesian ellipse to a homogeneous conic.
//! @param pConic OUT     initialized conic
//! @param pSource IN      cartesian ellipse
//! @group "DConic4d Initialization"
//!
Public GEOMDLLIMPEXP void bsiDConic4d_initFromDEllipse3d
(
DConic4dP pConic,
DEllipse3dCP pSource
);

//!
//! @description Initialize a conic from its center, 0 degree, and 90 degree points.
//!
//! @param pConic OUT     initialized conic
//! @param pCenter IN      conic center
//! @param pPoint0 IN      0 degree point
//! @param pPoint90 IN      90 degree point
//! @param theta0 IN      start angle
//! @param sweep IN      sweep angle
//! @group "DConic4d Initialization"
//!
Public GEOMDLLIMPEXP void bsiDConic4d_initFrom3dPoints
(
DConic4dP pConic,
DPoint3dCP pCenter,
DPoint3dCP pPoint0,
DPoint3dCP pPoint90,
double          theta0,
double          sweep
);

//!
//! @description Initalize a conic from its center, vector to 0 degree point, and vector to 90 degree point.
//!
//! @param pConic OUT     initialized conic
//! @param pCenter IN      conic center
//! @param pVector0 IN      0 degree vector
//! @param pVector90 IN      90 degree vector
//! @param theta0 IN      start angle
//! @param sweep IN      sweep angle
//! @group "DConic4d Initialization"
//!
Public GEOMDLLIMPEXP void bsiDConic4d_initFrom3dVectors
(
DConic4dP pConic,
DPoint3dCP pCenter,
DPoint3dCP pVector0,
DPoint3dCP pVector90,
double          theta0,
double          sweep
);

//!
//! @description Initalize a conic from its homogeneous center, homogeneous vector to 0 degree point, and homogeneous vector to 90 degree point.
//!
//! @param pConic OUT     initialized conic
//! @param pCenter IN      conic center
//! @param pVector0 IN      0 degree vector
//! @param pVector90 IN      90 degree vector
//! @param theta0 IN      start angle
//! @param sweep IN      sweep angle
//! @group "DConic4d Initialization"
//!
Public GEOMDLLIMPEXP void bsiDConic4d_initFrom4dVectors
(
DConic4dP pConic,
DPoint4dCP pCenter,
DPoint4dCP pVector0,
DPoint4dCP pVector90,
double          theta0,
double          sweep
);

//!
//! @description Initialize a conic from its center, 0 degree, and 90 degree points presented
//! as an array of 3 points.
//!
//! @param pConic OUT     initialized conic.
//! @param pPointArray IN      conic center, 0 degree and 90 degree points.
//! @param theta0 IN      start angle
//! @param sweep IN      sweep angle
//! @group "DConic4d Initialization"
//!
Public GEOMDLLIMPEXP void bsiDConic4d_initFrom3dPointArray
(
DConic4dP pConic,
DPoint3dCP pPointArray,
double              theta0,
double              sweep
);

//!
//! @description Set angular parameters to have given start and end points.
//!
//! @remarks If
//! the given points are really on the conic, this does the expected thing.
//! If they are not, here's exactly what happens: the start/end points are
//! placed on the original conic at the point where the conic intersects
//! the plane formed by the conic axis and the given point.
//!
//! @remarks This leaves the problem that the conic defines two paths from the
//! given start to end. This is resolved as follows.  The conic's existing
//! 0 and 90 degree vectors define a coordinate system.  In that system, the short
//! sweep from the 0 degree vector to the 90 degree vector is considered "counterclockwise".
//!
//! @remarks Beware that the relation of supposed start/end points to the conic
//!  is ambiguous
//!
//! @param pConic IN OUT  conic to modify
//! @param pStartPoint IN      start point to set
//! @param pEndPoint IN      end point to set
//! @param ccw    IN      true to force counterclockwise direction, false for clockwise.
//! @return true if the conic's local to global transformation is invertible
//! @group "DConic4d Modification"
//!
Public GEOMDLLIMPEXP bool         bsiDConic4d_setStartEnd
(
DConic4dP pConic,
DPoint3dCP pStartPoint,
DPoint3dCP pEndPoint,
bool        ccw
);

//!
//! @description Initialize a conic from its center, x and y directions from columns
//! 0 and 1 of a RotMatrix, scaled factors to apply to x and and y directions.
//!
//! @param pConic OUT     initialized conic.
//! @param pCenter IN      conic center
//! @param pMatrix IN      columns 0, 1 are conic directions (to be scaled by r0, r1)
//! @param r0 IN      scale factor for column 0.
//! @param r1 IN      scale factor for column 1.
//! @param theta0 IN      start angle
//! @param sweep IN      sweep angle
//! @group "DConic4d Initialization"
//!
Public GEOMDLLIMPEXP void bsiDConic4d_initFromScaledRotMatrix
(
DConic4dP pConic,
DPoint3dCP pCenter,
RotMatrixCP pMatrix,
double      r0,
double      r1,
double              theta0,
double              sweep
);

//!
//! @description Initialize a conic from its center, x and y directions from vectors
//!   with scale factors.
//!
//! @param pConic OUT     initialized conic.
//! @param pCenter IN      conic center
//! @param pVector0 IN      0 degree vector (e.g. major axis)
//! @param pVector90 IN      90 degree vector (e.g. minor axis)
//! @param r0 IN      scale factor for vector 0
//! @param r1 IN      scale factor for vector 90.
//! @param theta0 IN      start angle
//! @param sweep IN      sweep angle
//! @group "DConic4d Initialization"
//!
Public GEOMDLLIMPEXP void bsiDConic4d_initFromScaledVectors
(
DConic4dP pConic,
DPoint3dCP pCenter,
DPoint3dCP pVector0,
DPoint3dCP pVector90,
double      r0,
double      r1,
double              theta0,
double              sweep
);

//!
//! @description Initialize a conic from a circle.
//! @param pConic OUT     initialized conic
//! @param pCenter IN      circle center
//! @param pNormal IN      plane normal
//! @param radius IN      circle radius
//! @group "DConic4d Initialization"
//!
Public GEOMDLLIMPEXP void bsiDConic4d_initFromCenterNormalRadius
(
DConic4dP pConic,
DPoint3dCP pCenter,
DVec3dCP pNormal,
double          radius
);

//!
//! @description Tests whether the conic is complete (2pi range).
//! @param pConic OUT     initialized conic.
//! @return true if the conic is complete (2pi range)
//! @group "DConic4d Parameter Range"
//!
Public GEOMDLLIMPEXP bool    bsiDConic4d_isFullSweep (DConic4dCP pConic);

//!
//! @description Set the ellipse sweep to a full 360 degrees (2pi radians).  Start angle is left unchanged.
//! @param pConic IN OUT  ellipse to change
//! @group "DConic4d Parameter Range"
//!
Public GEOMDLLIMPEXP void bsiDConic4d_makeFullSweep (DConic4dP pConic);

//!
//! @description Set the ellipse sweep the the complement of its current angular range.
//! @remarks A full ellipse is left unchanged.
//! @param pConic IN OUT  ellipse to change
//! @group "DConic4d Parameter Range"
//!
Public GEOMDLLIMPEXP void bsiDConic4d_complementSweep (DConic4dP pConic);

//!
//! @description Compute the conic xyz point at a given parametric (angular) coordinate.
//! @param pConic IN      conic to evaluate
//! @param pPoint OUT     evaluated point, projected back to 3D
//! @param theta IN      angle
//! @return true if normalization of the point succeeded
//! @group "DConic4d Parameterization"
//!
Public GEOMDLLIMPEXP bool     bsiDConic4d_angleParameterToDPoint3d
(
DConic4dCP pConic,
DPoint3dP pPoint,
double      theta
);

//!
//! @description Compute the conic point at a given parametric (angular) coordinate.
//! @param pConic IN      conic to evaluate
//! @param pPoint OUT     evaluated point
//! @param theta IN      angle
//! @group "DConic4d Parameterization"
//!
Public GEOMDLLIMPEXP void  bsiDConic4d_angleParameterToDPoint4d
(
DConic4dCP pConic,
DPoint4dP pPoint,
double      theta
);

//!
//! @description Compute the conic xyz point at a given parametric (angular) coordinate.
//! @param pConic IN      conic to evaluate
//! @param pPoint OUT     evaluated point
//! @param xx IN      local x coordinate
//! @param yy IN      local y coordinate
//! @group "DConic4d Parameterization"
//!
Public GEOMDLLIMPEXP void bsiDConic4d_trigParameterToDPoint4d
(
DConic4dCP pConic,
DPoint4dP pPoint,
double      xx,
double      yy
);

//!
//! @description Compute the conic start and end points.
//! @param pConic IN      conic to evaluate
//! @param pStartPoint OUT     start point of conic
//! @param pEndPoint  OUT     end point of conic
//! @group "DConic4d Parameterization"
//!
Public GEOMDLLIMPEXP void bsiDConic4d_getDPoint4dEndPoints
(
DConic4dCP pConic,
DPoint4dP pStartPoint,
DPoint4dP pEndPoint
);

//!
//! @description Compute the conic start and end points (normalized).
//! @param pConic IN      conic to evaluate
//! @param pStartPoint OUT     start point of conic
//! @param pEndPoint  OUT     end point of conic
//! @group "DConic4d Parameterization"
//!
Public GEOMDLLIMPEXP void bsiDConic4d_getDPoint3dEndPoints
(
DConic4dCP pConic,
DPoint3dP pStartPoint,
DPoint3dP pEndPoint
);

//!
//! @description Compute the conic point and derivatives at a given parametric (angular) coordinate.
//! @param pConic IN      conic to evaluate
//! @param pX OUT     (optional) point on conic
//! @param pdX OUT     (optional) first derivative vector
//! @param pddX OUT     (optional) second derivative vector
//! @param theta IN      angle for evaluation
//! @group "DConic4d Parameterization"
//!
Public GEOMDLLIMPEXP void bsiDConic4d_angleParameterToDPoint4dDerivatives
(
DConic4dCP pConic,
DPoint4dP pX,
DPoint4dP pdX,
DPoint4dP pddX,
double      theta
);

//!
//! @description Compute the conic homogeneous point and derivatives at a given parametric (fractional) coordinate.
//! @param pConic IN      conic to evaluate
//! @param pX OUT     (optional) point on conic
//! @param pdX OUT     (optional) first derivative vector
//! @param pddX OUT     (optional) second derivative vector
//! @param fraction IN      fractional parameter for evaluation
//! @group "DConic4d Parameterization"
//!
Public GEOMDLLIMPEXP void bsiDConic4d_fractionParameterToDPoint4dDerivatives
(
DConic4dCP pConic,
DPoint4dP pX,
DPoint4dP pdX,
DPoint4dP pddX,
double      fraction
);

//!
//! @description Compute the conic xyz point at a given parametric (angular) coordinate.
//! @param pConic IN      conic to evaluate
//! @param pX OUT     (optional) point on conic
//! @param pdX OUT     (optional) first derivative vector
//! @param pddX OUT     (optional) second derivative vector
//! @param theta IN      angle for evaluation
//! @group "DConic4d Parameterization"
//! @return true if normalization of the point succeeded
//!
Public GEOMDLLIMPEXP bool    bsiDConic4d_angleParameterToDPoint3dDerivatives
(
DConic4dCP pConic,
DPoint3dP pX,
DPoint3dP pdX,
DPoint3dP pddX,
double      theta
);

//!
//! @description Compute numDerivatives+1 points pX[i]= i'th derivative
//! @param pConic IN      conic to evaluate
//! @param pX OUT     Array of conic point, first derivative, etc.
//! @param numDerivative IN      number of derivatives
//! @param theta IN      angle for evaluation
//! @group "DConic4d Parameterization"
//!
Public GEOMDLLIMPEXP void bsiDConic4d_angleParameterToDerivativeArray
(
DConic4dCP pConic,
DPoint4dP pX,
int         numDerivative,
double      theta
);

//!
//! @description Convert a fractional parameter to conic parameterization angle.
//! @param pConic IN      conic to evaluate
//! @param fraction IN      fractional parameter
//! @return angular parameter
//! @group "DConic4d Parameterization"
//!
Public GEOMDLLIMPEXP double bsiDConic4d_fractionToAngle
(
DConic4dCP pConic,
double      fraction
);

//!
//! @description Get the center, 0 degree, and 90 homogeneous degree basis points of the conic.
//! @param pConic     IN      conic
//! @param pCenter    OUT     (optional) center point
//! @param pVector0   OUT     (optional) 0 degree basis vector.
//! @param pVector90  OUT     (optional) 90 degree basis vector.
//! @group "DConic4d Local Coordinates"
//!
Public GEOMDLLIMPEXP void bsiDConic4d_getDPoint4dBasis
(
DConic4dCP pConic,
DPoint4dP pCenter,
DPoint4dP pVector0,
DPoint4dP pVector90
);

//!
//! @description Get the coordinate frame for a conic.  X,Y, and W colums are the vector0, vector90, and
//! center of the conic. Z column is out of plane.
//! @param pConic      IN      conic whose frame is computed.
//! @param pFrame         OUT     transformation from (cosine, sine, z, 1) coordinates to global.  May be NULL.
//! @param pInverse       OUT     inverse of frame.  May be NULL.
//! @return true if the requested frames were returned.
//! @group "DConic4d Local Coordinates"
//!
Public GEOMDLLIMPEXP bool    bsiDConic4d_getLocalFrame
(
DConic4dCP pConic,
DMatrix4dP pFrame,
DMatrix4dP pInverse
);

//!
//! @description Extract the xyw parts of each coordinate into a DEllipse3d.
//! @param pConic      IN      conic to be reduced.
//! @param pEllipse       OUT     reduced-dimension conic.
//! @group "DConic4d Queries"
//!
Public GEOMDLLIMPEXP void     bsiDConic4d_getDConic3dXYW
(
DConic4dCP pConic,
DEllipse3dP pEllipse
);

//!
//! @description Extract the xyw parts of each coordinate into a RotMatrix.
//! Column 0 is the 0 degree (cosine) vector.
//! Column 1 is the 90 degree (sine) vector.
//! Column 2 is the center.
//! @param pConic      IN      conic to be reduced.
//! @param pMatrix        OUT     matrix form
//! @group "DConic4d Local Coordinates"
//!
Public GEOMDLLIMPEXP void     bsiDConic4d_getRotMatrixXYW
(
DConic4dCP pConic,
RotMatrixP pMatrix
);

//!
//! @description Extract the xyw parts of each coordinate into a RotMatrix, with separate translation.
//! The ellipse is Translation * Basis * UnitCircle.
//! @param pConic      IN      conic to be reduced.
//! @param pTranslation   OUT     translation to center of ellipse
//! @param pInverseTranslation OUT     translation back to origin.
//! @param pMatrix        OUT     matrix form
//! @return true if the center has nonzero weight.  If false, the basis matrix is returned without translation.
//! @group "DConic4d Local Coordinates"
//!
Public GEOMDLLIMPEXP bool        bsiDConic4d_getTranslatedRotMatrixXYW
(
DConic4dCP pConic,
RotMatrixP pTranslation,
RotMatrixP pInverseTranslation,
RotMatrixP pMatrix
);

//!
//! @description Compute the angular position of the point relative to the conic's local
//! coordinates.  If the point is on the conic, this is the inverse of
//! evaluating the conic at the angle.
//!
//! @param pConic IN      conic definining angular space
//! @param pPoint IN      point to evaluate
//! @return angular parameter
//! @group "DConic4d Projection"
//!
Public GEOMDLLIMPEXP double   bsiDConic4d_DPoint3dToAngle
(
DConic4dCP pConic,
DPoint3dCP pPoint
);

//!
//! @description Compute the angular position of the homogeneous point relative to the conic's local
//! coordinates.  If the point is on the conic, this is the inverse of
//! evaluating the conic at the angle.
//!
//! @param pConic IN      conic definining angular space
//! @param pPoint IN      point to evaluate
//! @return angular parameter
//! @group "DConic4d Projection"
//!
Public GEOMDLLIMPEXP double   bsiDConic4d_DPoint4dToAngle
(
DConic4dCP pConic,
DPoint4dCP pPoint
);

//!
//! @description Project a point to the plane of the conic.
//!
//! @param pConic IN      conic whose axes become 3d plane directions.
//! @param pXYZNear OUT     nearest point
//! @param pCoff0 OUT     coefficient on vector towards 0 degree point
//! @param pCoff90 OUT     coefficient on vector towards 90 degree point
//! @param pXYZ IN      point to project onto plane
//! @return true if the plane is well defined.
//! @group "DConic4d Projection"
//!
Public GEOMDLLIMPEXP bool    bsiDConic4d_projectPointToPlane
(
DConic4dCP pConic,
DPoint3dP pXYZNear,
double      *pCoff0,
double      *pCoff90,
DPoint3dCP pXYZ
);

//!
//! @description Evaluate a conic using given coefficients for the axes.  If the x,y components
//! of the coefficients define a unit vector, the point is "on" the ellispe.
//! @param pConic IN      conic to evaluate
//! @param pPoint OUT     cartesian points
//! @param pTrig IN      x component of each point multiplies the
//!       0 degree vector, y component multiplies the 90 degree vector.
//! @param numPoint IN      number of pairs
//! @group "DConic4d Parameterization"
//!
Public GEOMDLLIMPEXP void bsiDConic4d_angleParameterToTrigPairs
(
DConic4dCP pConic,
DPoint4dP pPoint,
DPoint2dCP pTrig,
int       numPoint
);

//!
//! @description Evaluate a conic at a number of (cosine, sine) pairs, removing
//! pairs whose corresponding angle is not in range.
//!
//! @param pConic IN      conic to evaluate
//! @param pPoint OUT     cartesian points
//! @param pTrig IN      c,s pairs for evaluation
//! @param numPoint IN      number of pairs
//! @return number of points found to be in the angular range of the conic.
//! @group "DConic4d Parameterization"
//!
Public GEOMDLLIMPEXP int bsiDConic4d_testAndEvaluateTrigPairs
(
DConic4dCP pConic,
DPoint4dP pPoint,
DPoint2dCP pTrig,
int       numPoint
);

//!
//! @description Tests whether the angle is in the conic's angular range.
//! @return true if the angle is in the conic's angular range.
//! @param pConic IN      conic to evaluate
//! @param angle IN      angle to test
//! @group "DConic4d Parameter Range"
//!
Public GEOMDLLIMPEXP bool        bsiDConic4d_angleInSweep
(
DConic4dCP pConic,
double      angle
);

//!
//! @description Convert an angular parameter to fraction of bounded arc length.
//! @param pConic IN      conic whose angular range is queried.
//! @param angle      IN      angle to convert
//! @return fractional parameter.
//! @group "DConic4d Parameterization"
//!
Public GEOMDLLIMPEXP double     bsiDConic4d_angleParameterToFraction
(
DConic4dCP pConic,
double      angle
);

//!
//! @description Get the start and end angles of the conic.
//! @param pConic IN      conic whose angular range is queried.
//! @param pStartAngle OUT     start angle
//! @param pEndAngle OUT     end angle
//! @group "DConic4d Parameter Range"
//!
Public GEOMDLLIMPEXP void     bsiDConic4d_getLimits
(
DConic4dCP pConic,
double    *pStartAngle,
double    *pEndAngle
);

//!
//! @description Get the start and end angles of the conic.
//! @param pConic IN      conic whose angular range is queried.
//! @param pStartAngle OUT     start angle
//! @param pSweep OUT     sweep angle
//! @group "DConic4d Parameter Range"
//!
Public GEOMDLLIMPEXP void     bsiDConic4d_getSweep
(
DConic4dCP pConic,
double    *pStartAngle,
double    *pSweep
);

//!
//! @description Set the start and end angles of the conic.
//! @param pConic OUT     conic whose angular range is changed
//! @param startAngle IN      start angle
//! @param endAngle   IN      end angle
//! @group "DConic4d Parameter Range"
//!
Public GEOMDLLIMPEXP void     bsiDConic4d_setLimits
(
DConic4dP pConic,
double    startAngle,
double    endAngle
);

//!
//! @description Set the start and sweep angles of the conic.
//! @param pConic OUT     conic whose angular range is changed
//! @param startAngle IN      start angle
//! @param sweep      IN      sweep angle
//! @group "DConic4d Parameter Range"
//!
Public GEOMDLLIMPEXP void     bsiDConic4d_setSweep
(
DConic4dP pConic,
double    startAngle,
double    sweep
);

//!
//! @description Test if the conic is in the pleasant form with center weight 1 and vector weights 0.
//! @remarks In this case the conic is simply a conic.   If this test fails, the
//! curve could still be a conic but requires reparameterization and transformation
//! of weights.
//! @param pConic IN      conic to be inspected
//! @return true if the conic has center weight 1 and vector weights 0.
//! @group "DConic4d Weights"
//!
Public GEOMDLLIMPEXP bool    bsiDConic4d_isUnitWeighted (DConic4dCP pConic);

//!
//! @description Test if the conic is elliptical, albeit possibly requiring tricky reweighting and reparameterization.
//! @param pConic IN      conic to be inspected
//! @group "DConic4d Circular"
//! @return true if the conic is elliptical.
//!
Public GEOMDLLIMPEXP bool     bsiDConic4d_isEllipse (DConic4dCP pConic);

//!
//! @description Negate vectors as needed so the center weight is positive (but retains absolute value)
//! @param pNormalized OUT     copied and scaled conic
//! @param pWeighted IN      original conic
//! @group "DConic4d Weights"
//! @return true unless center weight is zero
//!
Public GEOMDLLIMPEXP bool    bsiDConic4d_absCenterWeight
(
DConic4dP pNormalized,
DConic4dCP pWeighted
);

//!
//! @description Make a copy of the source conic, reversing the start and end angles.
//! @remarks Inputs may be the same.
//! @param pConic   OUT     copied and reversed conic
//! @param pSource        IN      source conic
//! @group "DConic4d Initialization"
//!
Public GEOMDLLIMPEXP void bsiDConic4d_initReversed
(
DConic4dP pConic,
DConic4dCP pSource
);

//!
//! @description Compute the magnitude of the tangent vector to the conic at the specified angle.
//! @param pConic IN      conic to evaluate
//! @param theta IN      angular parameter
//! @return tangent magnitude
//! @group "DConic4d Parameterization"
//!
Public GEOMDLLIMPEXP double bsiDConic4d_tangentMagnitude
(
DConic4dCP pConic,
double      theta
);

//!
//! @description Compute a cartesian ellipse whose xyz coordinates (as evaluated at some
//!       parameter angle theta) are a vector in the direction of the tangent to the
//!       cartesian image of the given homogeneous conic.
//! @remarks Informally, the direction of
//!       this vector is tangent to the cartesian image, and its length is arbitrary.
//!       Formally, the vector is the numerator part of the cartesian tangent
//!       expression, X'w-Xw'.
//! @param pConic     IN      conic to evaluate
//! @param pTangentEllipse   OUT     computed ellipse
//! @group "DConic4d Parameterization"
//!
Public GEOMDLLIMPEXP void bsiDConic4d_pseudoTangent
(
DConic4dCP pConic,
DEllipse3dP pTangentEllipse
);

//!
//! @description Compute a cartesian ellipse whose xyz coordinates (as evaluated at some
//!       parameter angle theta) are a vector in the direction from a (cartesian projection of a)
//!       fixed (eye) point to the (cartesian projection of the) conic.
//! @remarks Informally, the
//!       direction of this vector is from the eye to the ellipse, and its length is arbitrary.
//!       Formally, the vector is the numerator part of the cartesian difference:
//!       X(theta)/X.w(theta) - Eye/Eye.w
//! @param pConic             IN      conic to evaluate
//! @param pVectorEllipse     OUT     computed ellipse
//! @param pEyePoint          IN      eye point
//! @group "DConic4d Parameterization"
//!
Public GEOMDLLIMPEXP void bsiDConic4d_pseudoVector
(
DConic4dCP pConic,
DEllipse3dP pVectorEllipse,
DPoint4dCP pEyePoint
);

//!
//! @description Compute the signed arc length of the conic.
//! @remarks Negative sweep produces negative arc length, so the return from this
//! can be negative.
//! @param pConic   IN      conic to evaluate
//! @return arc length of conic
//! @group "DConic4d Arc Length"
//!
Public GEOMDLLIMPEXP double bsiDConic4d_arcLength (DConic4dCP pConic);

//!
//! @description Compute the xyz range limits of a homogeneous conic.
//! @remarks Warning: If there is an asymptote in range, results are unpredictable.
//! @param pConic IN      conic whose range is determined
//! @param pRange OUT     computed range
//! @group "DConic4d Range"
//!
Public GEOMDLLIMPEXP void bsiDConic4d_getRange
(
DConic4dCP pConic,
DRange3dP pRange
);

//!
//! @description Compute the range of the conic in its own coordinate system.
//! @remarks This depends on the start and sweep angles but not the center or axis coordinates.
//! @param pConic IN      conic whose range is determined
//! @param pRange OUT     computed range
//! @group "DConic4d Range"
//!
Public GEOMDLLIMPEXP void bsiDConic4d_getLocalRange
(
DConic4dCP pConic,
DRange2dP pRange
);

//!
//! @description Find intersections of a (full) conic with a plane.
//! @remarks Return value n=1 is a single tangency point returned in pTrigPoints[0];
//!       n=2 is two simple intersections returned in pTrigPoints[0..1].
//! @remarks The three component values in pTrigPoints are:
//! <UL>
//! <LI>x == cosine of angle
//! <LI>y == sine of angle
//! <LI>z == angle in radians
//! </UL>
//! @param pConic      IN      conic to intersect with plane
//! @param pTrigPoints    OUT     2 points: cosine, sine, theta values of plane intersection
//! @param pPlane         IN      homogeneous plane equation
//! @return The number of intersections, i.e. 0, 1, or 2.
//! @group "DConic4d Intersection"
//!
Public GEOMDLLIMPEXP int bsiDConic4d_intersectPlane
(
DConic4dCP pConic,
DPoint3dP pTrigPoints,
DPoint4dCP pPlane
);

//!
//! @description Find the angles at which the conic becomes infinite.
//! @param pConic IN      conic to evaluate
//! @param pTrigPoints OUT     array of 0, 1 or 2  (cosine, sine, theta) triples.
//! @group "DConic4d Asymptotes"
//! @return Number of singular angles (0, 1 or 2).
//!
Public GEOMDLLIMPEXP int bsiDConic4d_singularAngles
(
DConic4dCP pConic,
DPoint3dP pTrigPoints
);

//!
//! @description Build an array containing all angles at which the conic starts, ends, or
//! has a simple intersection with a range box.
//! @param pConic         IN      conic to evaluate
//! @param pAngleArray    OUT     array of fractional parameters where crossings occur.
//! @param pInOutArray    OUT     flag pInOutArray[i] indicates whether the interval from pAngleArray[i] to
//!                               pAngleArray[i+1] is "inside" (nonzero) or "outside" (zero) the range limits.
//! @param pNumOut        OUT     number of angles returned
//! @param maxOut         IN      max allowed number of angles.  (Size of output arrays. Suggested size is 14)
//! @param pRange         IN      clip range
//! @param clipX          IN      true to enable x direction tests
//! @param clipY          IN      true to enable y direction tests
//! @param clipZ          IN      true to enable z direction tests
//! @group "DConic4d Intersection"
//!
Public GEOMDLLIMPEXP void         bsiDConic4d_intersectDRange3d
(
DConic4dCP pConic,
double        *pAngleArray,
int           *pInOutArray,
int           *pNumOut,
int           maxOut,
DRange3dCP pRange,
bool          clipX,
bool          clipY,
bool          clipZ
);

//!
//! @description Find the apparent intersections of (perspective) xy projections of a conic and line.
//! @remarks May return 0, 1, or 2 points.  Both conic and line are unbounded
//!
//! @param pConic             IN      conic to intersect with line
//! @param pConicPoints       OUT     points on conic
//! @param pConicAngles       OUT     angles on conic
//! @param pLinePoints        OUT     points on line
//! @param pLineParams        OUT     parameters on line
//! @param pSegment           IN      line to intersect with conic
//! @return the number of intersections, i.e. 0, 1, or 2
//! @group "DConic4d Intersection"
//!
Public GEOMDLLIMPEXP int bsiDConic4d_intersectDSegment4dXYW
(
DConic4dCP pConic,
DPoint4dP pConicPoints,
double        *pConicAngles,
DPoint4dP pLinePoints,
double        *pLineParams,
DSegment4dCP pSegment
);

//!
//! @description Find the apparent intersections of (perspective) xy projections of two conics.
//! @remarks May return 0 to 4 points.  Both conics are unbounded.
//! @param pConic         IN      first conic
//! @param pConic0Points  OUT     points on conic0
//! @param pConic0Angles  OUT     angles on conic0
//! @param pConic1Points  OUT     points on conic1
//! @param pConic1Angles  OUT     angles on conic1
//! @param pConic1        IN      second conic
//! @return the number of intersections, i.e. 0 to 4
//! @group "DConic4d Intersection"
//!
Public GEOMDLLIMPEXP int bsiDConic4d_intersectDConic4dXYW
(
DConic4dCP pConic,
DPoint4dP pConic0Points,
double        *pConic0Angles,
DPoint4dP pConic1Points,
double        *pConic1Angles,
DConic4dCP pConic1
);

//!
//! @description Test if the conic is circular.
//! @param pConic IN      conic to test
//! @return true if circular
//! @group "DConic4d Circular"
//!
Public GEOMDLLIMPEXP bool    bsiDConic4d_isCircular (DConic4dCP pConic);

//!
//! @description Test if the XY projection of the conic is circular.
//! @param pConic IN      conic to test
//! @return true if circular
//! @group "DConic4d Circular"
//!
Public GEOMDLLIMPEXP bool     bsiDConic4d_isCircularXY (DConic4dCP pConic);

//!
//! @description Project a point to the unbounded xy projection of the conic.
//! @remarks May return up to 4 points and angles.
//! @param pConic IN      conic to evaluate
//! @param pXYZOut OUT     array of xyz coordinates of projections.
//! @param pThetaOut OUT     array of parameter angles of projections.
//! @param pPoint OUT     space point to project to curve.
//! @return number of projected points.
//! @group "DConic4d Projection"
//!
Public GEOMDLLIMPEXP int  bsiDConic4d_projectDPoint3dXY
(
DConic4dCP pConic,
DPoint3dP pXYZOut,
double        *pThetaOut,
DPoint3dCP pPoint
);

//!
//! @description Project a point to the xy proction of the conic, and apply sector bounds.
//! @param pConic IN      conic to evaluate
//! @param pXYZOut OUT     array of xyz coordinates of projections.
//! @param pThetaOut OUT     array of parameter angles of projections.
//! @param pPoint OUT     space point to project to curve.
//! @return the number of bounded projection points
//! @group "DConic4d Projection"
//!
Public GEOMDLLIMPEXP int bsiDConic4d_projectDPoint3dXYBounded
(
DConic4dCP pConic,
DPoint3dP pXYZOut,
double        *pThetaOut,
DPoint3dCP pPoint
);

//!
//! @description Find the closest point on a bounded conic, considering both endpoints and perpendicular projections.
//! @param pConic IN      conic to evaluate
//! @param pMinAngle OUT     parameter at closest approach point
//! @param pMinDistSquared OUT     squared distance to closest approach point
//! @param pMinPoint OUT     closest approach point
//! @param pPoint IN      point to project
//! @param extend IN true if extension is allowed
//! @return true if a closest point is returned
//! @group "DConic4d Projection"
//!
Public GEOMDLLIMPEXP bool         bsiDConic4d_closestPointXYBounded
(
DConic4dCP pConic,
double        *pMinAngle,
double        *pMinDistSquared,
DPoint3dP pMinPoint,
DPoint3dCP pPoint,
bool extend
);

//!
//! @description Find points where lines from a (possibly infinite) eyepoint have a specified
//!   angle relation with the conic's tangent.   The relation is expressed as cosine
//!   and sine of the angle.
//! @param pConic IN      conic to evaluate
//! @param pAngleArray OUT     angle parameters of computed points.
//! @param pNumAngle OUT     number of angles returned
//! @param maxAngle  IN      max angles expected.
//! @param pEye       IN      eyepoint.
//! @param cosine IN      cosine of target angle.  0.0 for perpendicular, 1.0 for tangent
//! @param sine   IN      sine of target angle.    1.0 for perpendicular, 0.0 for tangent
//! @group "DConic4d Eye Point"
//! @return always true
//!
Public GEOMDLLIMPEXP bool         bsiDConic4d_angularRelationFromDPoint4dEyeXYW
(
DConic4dCP pConic,
double        *pAngleArray,
int           *pNumAngle,
int           maxAngle,
DPoint4dCP pEye,
double        cosine,
double        sine
);

//!
//! @description Apply a transformation to the source conic.
//! @param pDest OUT     transformed conic
//! @param pTransform IN      transformation to apply.
//! @param pSource IN      source conic.
//! @group "DConic4d Transform"
//!
Public GEOMDLLIMPEXP void     bsiDConic4d_applyTransform
(
DConic4dP pDest,
TransformCP pTransform,
DConic4dCP pSource
);

//!
//! @description Apply a transformation to the source conic.
//! @param pDest OUT     transformed conic
//! @param pTransform IN      transformation to apply.
//! @param pSource IN      source conic.
//! @group "DConic4d Transform"
//!
Public GEOMDLLIMPEXP void     bsiDConic4d_applyDMatrix4d
(
DConic4dP pDest,
DMatrix4dCP pTransform,
DConic4dCP pSource
);

//!
//! @description Initialize conic with axes subjected to a post transform.
//! @param pConic OUT     transformed conic
//! @param pSource IN      source conic
//! @param pAxisTransform IN      transform for axes
//! @group "DConic4d Transform"
//!
Public GEOMDLLIMPEXP void bsiDConic4d_initTransformedAxes
(
DConic4dP pConic,
DConic4dCP pSource,
RotMatrixCP pAxisTransform
);

//!
//! @description Initialize with angle space subjected to a homogeneous transformation.
//! @param pConic OUT     transformed conic
//! @param pSource IN      source conic
//! @param pAngleTransform IN      transform for angles
//! @group "DConic4d Transform"
//!
Public GEOMDLLIMPEXP void bsiDConic4d_initTransformedAngles
(
DConic4dP pConic,
DConic4dCP pSource,
RotMatrixCP pAngleTransform
);

//!
//! @description Construct a center, 0 degree, and 90 degree vector of the "conventional"
//! classification of the homogeneous conic.
//! @remarks Curve type is returned with one of the below values:
//! <UL>
//! </LI>*pCurveType = 1: The curve is the ellipse
//!       C + U cos(theta) + V sin (theta). That is, the curve is a unit circle in the (C,U,V) coordinate system.
//! </LI>*pCurveType = -1: The curve is the hyperbola
//!       C + U * s + V /s, where s is a free parameter.  U and V give the asymptotic directions,
//!   and C is the intersection of the asymptotes.  That is, the curve is the hyperbola y = 1/x in the C,U,V coordinate system.
//! </LI>*pCurveType = 0: The curve is the parabola
//!       C + U * s + V * s^2.  That is, the curve is the parabola y = x^2 in the C,U,V coordinate system.
//! </UL>
//!
//! @param pConic IN      conic to evaluate
//! @param pAxes OUT     A DConic4d structure which gives the axes in the restricted form indicated above.
//! @param pBasis OUT     Matrix D so that the vectors [U,V,C]*D, when drawn as a homogeneous
//!       "ellipse" driven by its unit circle forcing function, are the curve.
//! @param pCurveType OUT     curve type as indicated above.
//! @group "DConic4d Major Minor Axes"
//! @return true if the sum of the absolute values of the weights of the conic vectors and center is positive.
//!
Public GEOMDLLIMPEXP bool    bsiDConic4d_getCommonAxes
(
DConic4dCP pConic,
DConic4dP pAxes,
RotMatrixP pBasis,
int               *pCurveType
);

//!
//! @description Make a copy of the input curve, revising axes and angles so that
//! the start angle is zero and the sweep is positive.
//! @param pConicOut  OUT     output conic
//! @param pConicIn   IN      input conic
//! @group "DConic4d Parameterization"
//!
Public GEOMDLLIMPEXP void     bsiDConic4d_initWithPositiveSweepFromZero
(
DConic4dP pConicOut,
DConic4dCP pConicIn
);

//!
//! @description Initialize a (complete) conic from coefficients of implicit form.
//! @param pConic OUT     initialized conic
//! @param axx IN      xx coefficient
//! @param axy IN      xy coefficient
//! @param axw IN      xw coefficient
//! @param ayy IN      yy coefficient
//! @param ayw IN      yw coefficient
//! @param aww IN      ww coefficient
//! @return integer indicating the classification of degenerate cases:
//! <UL>
//! <LI>0: (normal) conic
//! <LI>1: (degenerate) pair of lines.  First line is any linear combinations of
//!               center and vector0.  Second is any linear combinations of
//!               center and vector1.  Center is common point of the two lines.
//!               (note that zero-weight center is possible, i.e. parallel lines)
//! <LI>2: (degenerate) single line.  The line is formed by linear combinations of
//!               center and vector0.
//! <LI>3: (degenerate) single point, returned as (center) in the conic.
//! <LI>4: no solutions.
//! </UL>
//! @group "DConic4d Implicit"
//!
Public GEOMDLLIMPEXP int  bsiDConic4d_classifyImplicitXYW
(
DConic4dP pConic,
double axx,
double axy,
double axw,
double ayy,
double ayw,
double aww
);

//!
//! @description Initialize a (complete) conic from coefficients of implicit form.
//! @param pConic OUT     initialized conic
//! @param axx IN      xx coefficient
//! @param axy IN      xy coefficient
//! @param axw IN      xw coefficient
//! @param ayy IN      yy coefficient
//! @param ayw IN      yw coefficient
//! @param aww IN      ww coefficient
//! @return true if the coefficients are clearly non-singular.
//! @group "DConic4d Implicit"
//!
Public GEOMDLLIMPEXP bool     bsiDConic4d_initFromImplicitXYW
(
DConic4dP pConic,
double axx,
double axy,
double axw,
double ayy,
double ayw,
double aww
);

//!
//! @description Return the (weighted) control points of a quartic bezier that represents the ellipse.
//! @remarks A quadratic bezier can represent any arc of a circle, but
//!       it cannot wrap and cover the complete range.  The quartic
//!       can cover the complete circle with a single bezier span.
//! @param pConic IN      conic to evaluate
//! @param pPoleArray OUT     array of (exactly) 5 poles for the bezier
//! @param pCirclePoleArray OUT     array of (exactly) 5 poles for a quadric
//!               bezier giving (cosine,sine,weight) as a function of the bezier parameter.
//! @group "DConic4d Bezier"
//!
Public GEOMDLLIMPEXP void     bsiDConic4d_getQuarticBezierPoles
(
DConic4dCP pConic,
DPoint4dP pPoleArray,
DPoint3dP pCirclePoleArray
);

//!
//! @description Return the (weighted) control points of quadratic beziers which
//!   combine to represent the full conic section.
//!
//! @param pConic IN      conic to evaluate
//! @param pPoleArray OUT     array of poles of composite quadratic Bezier curve.  Array count depends on the angular extent.
//! @param pCirclePoleArray OUT     array of corresponding poles which
//!           give the bezier polynomials back to the unit circle points (x,y,w)
//!           where x^2 + y^2 = w^2, but beware that w is not identically 1!  (Use this to map from bezier parameter back to angles.)
//! @param pAngleRadians OUT array of 1+numSpan angle breaks
//! @param pNumPole OUT     number of poles returned
//! @param pNumSpan OUT     number of spans returned.  The pole indices for the first span are (always) 0,1,2; for the second span (if present),
//!                    2,3,4, and so on.
//! @param maxPole IN      maximum number of poles desired.  maxPole must be at least
//!               5.  The circle is split into at most (maxPole - 1) / 2 spans.
//!               Beware that for 5 poles the circle is split into at most
//!               two spans, and there may be zero weights.   For 7 or more poles
//!               all weights can be positive.   The function may return fewer
//!               poles.
//! @group "DConic4d Bezier"
//!
Public GEOMDLLIMPEXP void     bsiDConic4d_getQuadricBezierPoles
(
DConic4dCP pConic,
DPoint4dP pPoleArray,
DPoint3dP pCirclePoleArray,
double      *pAngleRadians,
int       *pNumPole,
int       *pNumSpan,
int       maxPole
);

Public GEOMDLLIMPEXP void     bsiDConic4d_getQuadricBezierPoles
(
DConic4dCP pConic,
DPoint4dP pPoleArray,
DPoint3dP pCirclePoleArray,
int       *pNumPole,
int       *pNumSpan,
int       maxPole
);
//!
//! @description Return 0, 1, or 2 parameter angles at which the (unbounded) conic goes to inifinity.
//! @remarks Unlike ~mbsiDConic4d_singularAngles, this function uses a quick initial test for a pure ellipse (zero asymptotes).
//! @param pConic IN      conic to evaluate
//! @param pTrigPoint OUT     (x,y) = cosine and sine of angles; z = angle.  This array is assumed dimensioned to hold 2 outputs.
//! @return number of asymptotes placed in pTrigPoint.
//! @group "DConic4d Asymptotes"
//!
Public GEOMDLLIMPEXP int bsiDConic4d_evaluateAsymptotes
(
DConic4dCP pConic,
DPoint3dP pTrigPoint
);

//!
//! @description Clips a conic to multiple planes.
//! @param pConic IN      conic to evaluate
//! @param pClipped OUT     unused
//! @param pOutputArray OUT     caller-allocated array to hold up to nPlane+3 conics.
//!       (three extras allow for both start and end point occuring in an active section, and for asymptotes)
//! @param pNumOut OUT     number of conics returned.  May be zero.
//! @param maxOut IN      caller's array dimension
//! @param pPlaneArray IN      homogeneous plane equations
//! @param nPlane IN      number of planes.  "Negative" plane functions are "in".
//! @param clipType IN      0 for inside, 1 for outside
//! @return true unless the output array is too small.
//! @group "DConic4d Intersection"
//!
Public GEOMDLLIMPEXP bool    bsiDConic4d_clipToPlanes
(
DConic4dCP pConic,
bool      *pClipped,
DConic4dP pOutputArray,
int       *pNumOut,
int       maxOut,
DPoint4dCP pPlaneArray,
int        nPlane,
int        clipType
);

//! @description Find angles at which the conic tangent vector is perpendicular to given vector.
//! @param [in] pConic conic to search.
//! @param [out] angles 0,1, or 2 angles.   This is an array that must be allocated by the caller.
//! @param [in] vector perpendicular vector.
//! @return The number of solutions, i.e. 0, 1, or 2
//! @group "DConic4d Intersection"
Public GEOMDLLIMPEXP int bsiDConic4d_solveTangentsPerpendicularToVector
(
DConic4dP pConic,
double  *angles,
DVec3dR vector
);

//! @return max xyz part of center, vector0, vector90 -- i.e. ignore weights.
//! @remarks note that this inspects the data in the structure. The actual curve can have larger coordinates (to infinity if there is an asymptote)
Public GEOMDLLIMPEXP double bsiDConic4d_maxAbsUnnormalizedXYZ (DConic4dCR conic);

//! @return max weight part of center, vector0, vector90
Public GEOMDLLIMPEXP double bsiDConic4d_maxAbsWeight (DConic4dCR conic);

//! @return max absolute difference between weights of center, vector0, and vector 90.
Public GEOMDLLIMPEXP double bsiDConic4d_maxWeightDiff (DConic4dCR conicA, DConic4dCR conicB);

//! @return max absolute difference between corresponding xyz of center, vector0, and vector 90.
Public GEOMDLLIMPEXP double bsiDConic4d_maxUnnormalizedXYZDiff (DConic4dCR conicA, DConic4dCR conicB);

END_BENTLEY_GEOMETRY_NAMESPACE

