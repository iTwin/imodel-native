/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*-----------------------------------------------------------------*//**
* Fill a homogenous ellipse from a cartesian ellipse.
* @param ellipseP IN OUT  header to receive points
* @param centerP IN      ellipse center
* @param point0P IN      0 degree vector
* @param point90P IN      90 degree vector
* @param theta0 IN      start angle
* @param delta IN      sweep angle
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_initFromDEllipse3d


(
DEllipse4dP pInstance,
DEllipse3dCP pEllipse
);

/*-----------------------------------------------------------------*//**
* Fill a homogenous ellipse from a 3D ellipse, using the input Z as weight and setting
*       output z to a specified constant.
* @param pEllipse IN      ellipse with x,y,w as x,y,z
* @param z IN      z to use in homogeneous form.
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_initFromDEllipse3dXYW


(
DEllipse4dP pInstance,
DEllipse3dCP pEllipse,
double      z
);

/*-----------------------------------------------------------------*//**
* Fill in ellipse data from 3D coordinates
* @param ellipseP IN OUT  header to receive points
* @param centerP IN      ellipse center
* @param point0P IN      0 degree vector
* @param point90P IN      90 degree vector
* @param theta0 IN      start angle
* @param delta IN      sweep angle
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_initFromDPoint4d


(
DEllipse4dP ellipseP,
DPoint4dCP centerP,
DPoint4dCP point0P,
DPoint4dCP point90P,
double          theta0,
double          delta
);

/*-----------------------------------------------------------------*//**
* Fill in ellipse data from 3D coordinates
*
* @param ellipseP IN OUT  header to receive points
* @param centerP IN      ellipse center
* @param vectorUP IN      0 degree vector
* @param vectorVP IN      90 degree vector
* @param theta0 IN      start angle
* @param delta IN      sweep angle
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_initFrom3dVectors


(
DEllipse4dP ellipseP,
DPoint3dCP centerP,
DPoint3dCP vectorUP,
DPoint3dCP vectorVP,
double          theta0,
double          delta
);

/*-----------------------------------------------------------------*//**
* Fill in ellipse data from 4d coordinates of center, vector0, vector90
*
* @param pEllipse IN OUT  header to receive points
* @param pCenter IN      ellipse center
* @param pVector0 IN      0 degree vector
* @param pVector90 IN      90 degree vector
* @param theta0 IN      start angle
* @param delta IN      sweep angle
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_initFrom4dVectors


(
DEllipse4dP pEllipse,
DPoint4dCP pCenter,
DPoint4dCP pVector0,
DPoint4dCP pVector90,
double          theta0,
double          delta
);

/*-----------------------------------------------------------------*//**
* Fill in ellipse data so the form of the ellipse is
*      P(theta) = P0 + cos(theta)*U + sin(theta)*V
* where P, P0, U, V are homogeneous points, with
*       P0 = (point[0], weight[0])  directly from given data.
*       U   = (point[1]point[0], weight[1])
*       V   = (point[2]point[1], weight[2])
*
* @param ellipseP IN OUT  header to receive points
* @param pPointArray * @param pWeightArray * @param theta0 IN      start angle
* @param delta IN      sweep angle
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_initFromWeighted3dPoints


(
DEllipse4dP ellipseP,
DPoint3dCP pPointArray,
const double        *pWeightArray,
double        theta0,
double        delta
);

/**
           bsiDEllipse4d_initFrom3dPoints
* Fill in ellipse data from 3D coordinates


* @param ellipseP IN OUT  header to receive points
* @param arrayP IN      ellipse center
* @param theta0 IN      start angle
* @param delta IN      sweep angle
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_initFrom3dPoints


(
DEllipse4dP ellipseP,
DPoint3dCP arrayP,
double          theta0,
double          delta
);

/*-----------------------------------------------------------------*//**
* @param ellipseP OUT     header to receive points
* @param centerP IN      circle center
* @param normalP IN      plane normal
* @param radius IN      circle radius
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_initFromCenterNormalRadius


(
DEllipse4dP ellipseP,
DPoint3dCP centerP,
DPoint3dCP normalP,
double          radius
);

/*-----------------------------------------------------------------*//**
* @param pEllipse IN      ellipse to test
* @see
* @return true if the ellipse angular range is a full circle
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDEllipse4d_isFullEllipse


(
DEllipse4dCP pEllipse
);

/*-----------------------------------------------------------------*//**
*  Evaluate the homogenous point on a homogeneous ellipse at given
* angle.
* @param pHEllipse IN      ellipse to evaluate
* @param pPoint OUT     evaluted point
* @param theta IN      angle
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_evaluateDPoint4d


(
DEllipse4dCP pHEllipse,
DPoint4dP pPoint,
double      theta
);

/*-----------------------------------------------------------------*//**
*  Evaluate the homogeneous endpoints of the ellipse.
* @param pInstance IN      ellipse to evaluate
* @param pStart OUT     start point
* @param pEnd   OUT     end point
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_evaluateDPoint4dEndPoints


(
DEllipse4dCP pInstance,
DPoint4dP pStartPoint,
DPoint4dP pEndPoint
);

/*-----------------------------------------------------------------*//**
* Compute an angle associated with a 4d point.   If the point is really on the ellipse, this
*   angle is the parameter of the point.  If the point is not on the ellipse, the angle minimizes
*   the 4D vector from ellipse points to the given point.
* @param pEllipse IN      ellipse to evaluate
* @param    pPoint IN      point where angle is computed.
* @return computed angle.  0 is returned if the ellipse is degenerate or the point is at the
*           ellipse center.
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDEllipse4d_angleFromDPoint4d


(
DEllipse4dCP pEllipse,
DPoint4dCP pPoint
);

/*-----------------------------------------------------------------*//**
*  Evaluate the homogenous point on a homogeneous ellipse at given
* angle and reduce the point back to 3d.
* @param pHEllipse IN      ellipse to evaluate
* @param    pPoint OUT     point on ellipse
* @param    theta IN      angle of evaluation.
* @see
* @return true if weight is nonzero and point was normalized.
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDEllipse4d_evaluateDPoint3d


(
DEllipse4dCP pHEllipse,
DPoint3dP pPoint,
double      theta
);

/*-----------------------------------------------------------------*//**
* Project pXYZ to the plane with center, 0, and 90 degree cartesian
* points.

* @param pHEllipse IN      ellipse whose axes become 3d plane directions.
* @param pXYZNear OUT     nearest point
* @param pCoff0 OUT     coefficient on vector towards 0 degree point
* @param pCoff90 OUT     coefficient on vector towards 90 degree point
* @param pXYZ
* @see
* @return true if plane is well defined.
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDEllipse4d_projectPointTo3dPlane


(
DEllipse4dCP pHEllipse,
DPoint3dP pXYZNear,
double      *pCoff0,
double      *pCoff90,
DPoint3dCP pXYZ
);

/**

* @param ellipseP IN      ellipse to be stroked
* @param n IN      default number of points on full ellipse
* @param nMax IN      max number of points on full ellipse
* @param tol IN      tolerance for stroking
* @see
* @return number of strokes required to reach the stroke tolerance
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDEllipse4d_getStrokeCount


(
DEllipse4dCP ellipseP,
int             n,
int             nMax,
double          tol
);

/*-----------------------------------------------------------------*//**
* Find the sector index containing angle theta.
* @param pHEllipse IN      Header to search
* @param theta = angle whose sector is required
* @see
* @return sector index containing angle
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDEllipse4d_findSectorIndex


(
DEllipse4dCP pHEllipse,
double  theta
);

/*-----------------------------------------------------------------*//**
* Evaluate an ellipse at a number of (cosine, sine) pairs, removing
* pairs whose corresponding angle is not in range.

* @param pEllipse IN      ellipse to evaluate
* @param pPoint OUT     cartesian points
* @param pTrig IN      c,s pairs for evaluation
* @param numPoint IN      number of pairs
* @see
* @return int number of cartesian points returned.
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDEllipse4d_evaluateTrigPairs


(
DEllipse4dCP pEllipse,
DPoint3dP pPoint,
DPoint2dCP pTrig,
int       numPoint
);

/*-----------------------------------------------------------------*//**
* Evaluate an ellipse at a number of (cosine, sine) pairs, removing
* pairs whose corresponding angle is not in range.

* @param pEllipse IN      ellipse to evaluate
* @param pPoint OUT     cartesian points
* @param pTrig IN      c,s pairs for evaluation
* @param numPoint IN      number of pairs
* @see
* @return int
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDEllipse4d_testAndEvaluateTrigPairs


(
DEllipse4dCP pEllipse,
DPoint3dP pPoint,
DPoint2dCP pTrig,
int       numPoint
);

/*-----------------------------------------------------------------*//**
* Evaluate an ellipse at a number of (cosine, sine) pairs, removing
* pairs whose corresponding angle is not in range.

* @param pEllipse IN      ellipse to evaluate
* @param pPoint OUT     cartesian points
* @param pTrig IN      c,s pairs for evaluation
* @param numPoint IN      number of pairs
* @see
* @return int
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDEllipse4d_testAndEvaluateAngles


(
DEllipse4dCP pEllipse,
DPoint4dP pPointOut,
double      *pAngleOut,
double      *pAngleIn,
int         numPoint
);

/*-----------------------------------------------------------------*//**
* Clears the sector count for an ellipse.
* @param pEllipse OUT     Ellipse to be cleared
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_clearSectors


(
DEllipse4dP pEllipse
);

/*-----------------------------------------------------------------*//**
* Set the sweep as a complete circle.
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_setFullCircleSweep


(
DEllipse4dP pEllipse
);

/*-----------------------------------------------------------------*//**
*
* Add an interval with no test for min/max relationship
*
* @param minValue IN      new interval min.
* @param maxValue IN      new interval max
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void      bsiDEllipse4d_addUnorderedSector


(
DEllipse4dP pInstance,
double          minValue,
double          maxValue
);

/*-----------------------------------------------------------------*//**
* Get the i'th sector angular range
* @param pEllipse IN      ellipse whose angular range is queried.
* @param pStartAngle OUT     start angle
* @param pEndAngle OUT     end angle
* @param i IN      sector to read
* @see
* @return true if sector index is valid.
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDEllipse4d_getSector


(
DEllipse4dCP pEllipse,
double    *pStartAngle,
double    *pEndAngle,
int             i
);

/*-----------------------------------------------------------------*//**
* Clips the sector data of the hyperbola to a plane.


* @param pEllipse IN      Ellipse to evaluate
* @param pPlaneTrigPoint IN      x,y=cosine and sine where plane is intersected.
* @param numPlaneIntersection IN      number of plane intersections
* @param pPlaneVector IN      Homogeneous plane point
* @param pTrigPoint IN      x,y=cosine and sine of angles. z = angle
* @param numAsymptote IN      number of singular points on the ellipse
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_clipHyperbolicSectors


(
DEllipse4dP pEllipse,
DPoint3dCP pPlaneTrigPoint,
int     numPlaneIntersection,
DPoint4dCP pPlaneVector,
DPoint3dCP pTrigPoint,
int     numAsymptote
);

/**
           bsiDEllipse4d_evaluateAsymptotes
* Clips the sector data of the ellipse to a plane.


* @param pTrigPoint OUT     x,y=cosine and sine of angles. z= angle
* @param pEllipse IN      Ellipse to evaluate
* @see
* @return int
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDEllipse4d_evaluateAsymptotes


(
DPoint3dP pTrigPoint,
DEllipse4dCP pEllipse
);

/*-----------------------------------------------------------------*//**
* Clips the sector data of the ellipse to a plane.


* @param ellipseP OUT     Ellipse to be tested
* @param trigPointP IN      trig values of clip sector
* @param planeP IN      plane vector
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_clipSectors


(
DEllipse4dP ellipseP,
DPoint3dP trigPointP,
DPoint4dCP planeP
);

/**
           bsiDEllipse4d_clipToPlanes
* Clips an ellipse to multiple planes.


* @param ellipseP OUT     Ellipse to be tested
* @param clippedP OUT     Set true if clipped
* @param planeP IN      homogeneous plane equations
* @param nPlane IN      number of planes
* @param clipType IN      0 for inside, 1 for outside
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_clipToPlanes


(
DEllipse4dP ellipseP,
bool      *clippedP,
DPoint4dCP planeP,
int        nPlane,
int        clipType
);

/*-----------------------------------------------------------------*//**
* Force the (homogeneous!!!!!!) 0 and 90 degree vectors to be
* perpendicular (as 4D vectors, not in projection).
*
* @param pNormalizedEllipse
* @param pEllipse
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_normalizeAxes


(
DEllipse4dP pNormalizedEllipse,
DEllipse4dCP pEllipse
);

/*-----------------------------------------------------------------*//**
*
* Set the ellipse so that it passes through the same points (same start, same intermediate path,
* same end) of the input ellipse, but has a positive sweep angle.
* This may require altering both the basis vectors and the angle limits.
* @param pEllipse IN      prior ellipse
* @param theta0 IN      start angle
* @param theta1 IN      end angle
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_initWithPositiveSweep


(
DEllipse4dP pInstance,
DEllipse4dCP pEllipse,
double        theta0,
double        theta1
);

/**



* @param pHEllipse IN      ellipse to evaluate
* @param theta IN      angular parameter
* @see
* @return double
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDEllipse4d_tangentMagnitude


(
DEllipse4dCP pHEllipse,
double      theta
);

/*-----------------------------------------------------------------*//**
*
* Compute the signed arc length of a part of a conic section.
* (Negative sweep produces negative arc length, so the return from this
* can be negative.)
*
* @param pEllipse IN      ellipse to measure
* @param    theta0  IN      start angle (parametric angle)
* @param    sweep   IN      sweep angel (parametric angle)
* @see
* @return double
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDEllipse4d_arcLength


(
DEllipse4dCP pEllipse,
double      theta0,
double      sweep
);

/*-----------------------------------------------------------------*//**
* Compute the 0, 1, or 2 angles at which a homogeneous ellipse is goes
* to infinity.
*
* @param pHEllipse IN      Ellipse whose singular (asymptote) angles are to be computed.
* @param pAngleArray IN      0, 1, or 2 singular angles
* @see
* @return number of singular angles
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDEllipse4d_singularAngles


(
DEllipse4dCP pHEllipse,
double      *pAngleArray
);

/*-----------------------------------------------------------------*//**
* Find the singular angles of the homogeneous ellipse, and clip out
* a small angular sector around each singular angle.
*
* @param pHEllipse IN OUT  Ellipse whose singular points are clipped
* @param cutAngle IN      size of cutout
* @see
* @return int
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDEllipse4d_removeSingularSectors


(
DEllipse4dP pHEllipse,
double      cutAngle
);

/*-----------------------------------------------------------------*//**
* Compute the transfer matrix to normalize a weighted, uncentered
* ellipse into a centered cartesian ellipse.

* @param pMatrix OUT     transfer matrix
* @param pInverse OUT     its inverse.   Pass NULL if not needed.
* @param w0 IN      cosine weight
* @param w90 IN      sine weight
* @param wCenter IN      center weight
* @see
* @return true if weights define an angle change.
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiEllipse_angularTransferMatrix


(
RotMatrixP pMatrix,
RotMatrixP pInverse,
double          w0,
double          w90,
double          wCenter
);

/*-----------------------------------------------------------------*//**
* Let F = [cos(theta), sin(theta), 1+alpha*cos(theta)+beta*sin(theta)]
*     G = [cos(phi), sin(phi), 1]
* and G = M*F   (possibly scaled?)
* Return the phi corresponding to theta.

* @param theta IN      known angle prior to transform
* @param pMatrix IN      transfer matrix.
* @param  IN      matrix M
* @param alpha IN      cosine coefficient
* @param beta IN      sine coefficient
* @see
* @return modified angle
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDEllipse4d_transferAngle


(
double          theta,
RotMatrixP pMatrix,

double          alpha,
double          beta
);

/*-----------------------------------------------------------------*//**
* Let F = [cos(theta), sin(theta), 1+alpha*cos(theta)+beta*sin(theta)]
*     G = [cos(phi), sin(phi), 1]
* and G = M*F   (possibly scaled?)
* Replace all angles (theta) in an ellispe's stroke intervals by
* corresponding phi angles.

* @param pDest IN OUT  Ellipse whose angles are corrected.
* @param pSource IN      source of angle data
* @param pMatrix IN      matrix M
* @param alpha IN      cosine coefficient
* @param beta IN      sine coefficient
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDEllipse4d_transferAngles


(
DEllipse4dP pDest,
DEllipse4dCP pSource,
RotMatrixP pMatrix,
double          alpha,
double          beta
);

/*-----------------------------------------------------------------*//**
*
* Find new basis vectors with 0 weights on the U and V vectors, and unit
* on the C vector.  This computation is not possible if the curve is
* a hyperbola or parabola when projected to 3D.
*
* @param pNormalized OUT     normalized form
* @param pWeighted IN      unnormalized form
* @see
* @return true if the curve really is an ellipse (i.e. not hyperbola or parabola)
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDEllipse4d_normalizeWeights


(
DEllipse4dP pNormalized,
DEllipse4dCP pWeighted
);

/*-----------------------------------------------------------------*//**
*
* Negate vectors as needed so the center weight is positive (but retains absolute
* value)                                                                *
*
* @param pNormalized OUT     copied and scaled ellipse
* @param pWeighted IN      original ellipse
* @see
* @return true unless weight is zero
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDEllipse4d_absCenterWeight


(
DEllipse4dP pNormalized,
DEllipse4dCP pWeighted
);

/*-----------------------------------------------------------------*//**
*
* Initialize a (complete) homogeneous ellipse from its xy implicit form.
* The implicit form is
*       axx * x^2 + axy * x * y + ayy * y * y + ax * x + ay * y + a = 0
* The ellipse is placed with z=0.
*
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDEllipse4d_initFromImplicitCoefficients


(
DEllipse4dP pInstance,
double          axx,
double          axy,
double          ayy,
double          ax,
double          ay,
double          a
);

/*-----------------------------------------------------------------*//**
* Extend a range to include the range of an ellipse.
* If the homogeneous weights have zeros (i.e. the cartesian image is a
* hyperbola or or parabola) the range is computed with a small angular
* sector clipped away from the singularity.

* @param pRange IN OUT  range extended by ellipse range.
* @param pHEllipse IN      Ellipse whose range is computed
* @see
* @indexVerb extend
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse4d_extendRange


(
DRange3dP pRange,
DEllipse4dCP pHEllipse
);

/*-----------------------------------------------------------------*//**
* Extend a range to include the range of the xy parts of an ellipse.
* If the homogeneous weights have zeros (i.e. the cartesian image is a
* hyperbola or or parabola) the range is computed with a small angular
* sector clipped away from the singularity.

* @param pRange IN OUT  range extended by ellipse range.
* @param pHEllipse IN      Ellipse whose range is computed
* @see
* @indexVerb extend
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDRange2d_extendByDellipse4d


(
DRange2dP pRange,
DEllipse4dCP pHEllipse
);

/*-----------------------------------------------------------------*//**
*
* Fill in conic data from the center and vectors of a DEllipse4d,
* with given angles.
*
* @param pInstance OUT     initialized conic
* @param pSource    IN      source conic
* @param theta0     IN      start angle
* @param sweep      IN      sweep angle
*
* @indexVerb init
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDConic4d_initFromDEllipse4dSweep


(
DConic4dP pInstance,
DEllipse4dCP pSource,
double      theta0,
double      sweep
);

/*-----------------------------------------------------------------*//**
*
* Fill in conic data from the center and vectors of a DEllipse4d
* with angular range from a specified sector.  If sector index is invalid,
* make the DConic4d complete.
*
* @param pInstance OUT     initialized conic
* @param pSource    IN      source conic
* @param sector     IN      sector index
*
* @indexVerb init
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDConic4d_initFromDEllipse4dSector


(
DConic4dP pInstance,
DEllipse4dCP pSource,
int         index
);

/*---------------------------------------------------------------------------------**//**
* Project xy coordinates to the xy projection of a homogeneous ellipse.
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiGeom_projectDPoint3dToDEllipse4dXY


(
DPoint4dP pProjection,
double      *pParam,
DPoint3dCP pPoint,
DEllipse4dCP pEllipse

);

/*----------------------------------------------------------------------+
|                                                                       |
|   name        bsiDEllipse4d_componentRange                            |
|                                                                       |
|                                                                       |
| Conditional update range of one component of a homogeneous trig       |
| function.                                                             |
| That is, find local extrema of                                        |
|<p>                                                                    |
|   (x0 + x1 * cos(theta) + x2 * sin(theta)) / (w0 + w1 * cos(theta) + w2 * sin(theta))     |
|<p>                                                                    |
| and augment minP, maxP if the angle of the extrema is 'in' the        |
| the sector set.                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void    bsiDEllipse4d_componentRange


(
double*         pMin,           /* IN OUT  min coordiante of range box */
double*         pMax,           /* IN OUT  max coordinate of range box */
double          x0,             /* IN      constant term of numerator */
double          x1,             /* IN      cosine term of numerator */
double          x2,             /* IN      sine term of numerator */
double          w0,             /* IN      constant term of numerator */
double          w1,             /* IN      cosine term of numerator */
double          w2,             /* IN      sine term of numerator */
SmallSetRange1dP pRangeSet    /* IN      range set with 'in' intervals. */
);

/*-----------------------------------------------------------------*//**
* Computes the silhouette ellipse of an ellipsoid under arbitrary
* DMap4d and viewpoint.
*
* @param pHEllipse OUT     silhouette ellipse/parabola/hyperbola
* @param pEllipsoidPoint IN      4 defining points of the ellipsoid
* @param pHMap IN      further mapping
* @param pEyePoint IN      eyepoint
* @return false iff the eyeponit is inside the ellipsoid.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiGeom_ellipsoidSilhouette


(
DEllipse4dP pHEllipse,
DPoint3dCP pEllipsoidPoint,
DMap4dCP pHMap,
DPoint4dCP pEyePoint
);

/*-----------------------------------------------------------------*//**
*
* @param pHEllipse IN      the silhouette curve.
* @param pHMap IN      mapping from local to model.  If NULL, all computations
*                 are local identity map
* @param pEyePoint IN      eyepoint, in model.  For perspective, from xyz, set w=1
*                     For flat view in direction xyz, set w=0
* @return int
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiHyperboloid_silhouetteCurve


(
DEllipse4dP pHEllipse,
DMap4dCP pHMap,
DPoint4dCP pEyePoint
);

/*-----------------------------------------------------------------*//**
*
* @param pTrig0ut OUT     points that are IN the ellipse bounds.  May be same as input.
* @param pNumOut OUT     number of returned points
* @param pTrigIn OUT     points to test
* @param numIn IN      number of points to test
* @param pEllipse IN      bounding ellipse
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDEllipse4d_selectTrigPointsInBounds


(
DPoint3dP pTrig0ut,
int         *pNumOut,
DPoint3dCP pTrigIn,
int         numIn,
DEllipse4dCP pEllipse
);

/*-----------------------------------------------------------------*//**
 @param pTransform IN      The transform
 @param pOutEllipse OUT     transformed ellipse
 @param pInEllipse IN      The untransformed ellipse
 @see
 @indexVerb
 @group DTransform3d
 @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDTransform3d_multiplyDEllipse4d


(
DTransform3dCP pTransform,
DEllipse4dP pOutEllipse,
DEllipse4dCP pInEllipse
);

/*-----------------------------------------------------------------*//**
 @param pTransform IN      The transform
 @param pOutEllipse OUT     transformed ellipse
 @param pInEllipse IN      The untransformed ellipse
 @see
 @indexVerb
 @group Transform
 @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTransform_multiplyDEllipse4d


(
TransformCP pTransform,
DEllipse4dP pOutEllipse,
DEllipse4dCP pInEllipse
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
