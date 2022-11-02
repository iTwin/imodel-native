/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! Try to divide numerator by denominator.  If denominator is near zero,
//!   return false and set the result to a default value.
//! @param pRatio OUT     numerator / denominator if safe, defaultRatio otherwise
//! @param numerator IN      numerator of ratio
//! @param denominator IN      denominator of ratio
//! @param defaultRatio IN      default ratio if b is small
//! @return true if division is numerically safe.
//!
Public GEOMDLLIMPEXP bool    bsiTrig_safeDivide
(
double  *pRatio,
double  numerator,
double  denominator,
double  defaultRatio
);
//!
//! @param angle IN      angle in radians
//! @return angle shifted by 2pi towards (and usually past) zero.
//!
Public GEOMDLLIMPEXP double bsiTrig_complementaryAngle (double angle);

//!
//! @param theta IN      nominal angle
//! @param theta0 IN      start of interval
//! @param sweep IN      signed sweep
//! @return angle theta adjusted into the 2pi interval beginning at theta0
//!       and going in the direction of sweep.
//!
Public GEOMDLLIMPEXP double bsiTrig_adjustAngleToSweep
(
double  theta,
double  theta0,
double  sweep
);

//!
//! Restrict a sweep to that (a) its absolute value is no more than 2pi (full circle) and
//! (b) its sign is preserved. (Hence both +2pi and -2pi are possible).   Any value with larger
//! absolute value is pulled back to the nearer limit.
//! @param sweep IN      angle to adjust.
//!
//!
Public GEOMDLLIMPEXP double   bsiTrig_restrictSweep (double sweep);

//!
//! Convert xyz coordinates to spherical (theta, phi, r)                  |
//! <UL>
//! <LI>theta = longitude, i.e. angle of (x,y,0) from (1,0,0)
//! <LI>phi   = latitude, i.e. angle between (x,y,z) and (x,y,0)
//! <LI>r     = distance from origin
//! </UL>
//!
//! @param pSphericalPoint OUT      theta, phi, r
//! @param pXYZ IN     x,y,z
//! @return true unless r is zero.
//!
Public GEOMDLLIMPEXP bool    bsiVector_cartesianToSpherical
(
DPoint3dP pSphericalPoint,
DPoint3dP pXYZ
);

//!
//! Convert (theta, phi, r) coordinates to spherical (x,y,z)
//!
//! @param pXYZ OUT     x,y,z
//! @param pSphericalPoint IN      theta, phi, r
//! @return true (always)
//!
Public GEOMDLLIMPEXP bool    bsiVector_sphericalToCartesian
(
DPoint3dP pXYZ,
DPoint3dP pSphericalPoint
);

//!
//! @return system wide small angle tolerance
//!
Public GEOMDLLIMPEXP double bsiTrig_smallAngle
(
void
);

//!  Compare point coordinates, using a tolerance of system small angle times the
//!   larger of 1 or any given coordinate.
//!  @param [in] pointA first value for comparison
//!  @param [in] pointB second value for comparison
//! @return true of points are nearly equal.
Public GEOMDLLIMPEXP bool bsiTrig_nearlyEqualDPoint3d
(
DPoint3dCP pointA,
DPoint3dCP pointB
);

//!
//! Wrap the system atan2 with checks for (0,0) arguments.
//!
//! @param y IN      numerator
//! @param x IN      denominator
//! @return arctan of y/x.
//!
Public GEOMDLLIMPEXP double bsiTrig_atan2
(
double  y,
double  x
);

//!
//! @param angle IN      angle to test
//! @return true if (absolute value of) angle is more than 2pi minus a tolerance
//!
Public GEOMDLLIMPEXP bool    bsiTrig_isAngleFullCircle (double angle);

//!
//! @param angle IN      angle to test
//! @return true if (absolute value of) angle is less than the small angle tolerance.
//!
Public GEOMDLLIMPEXP bool    bsiTrig_isAngleNearZero (double angle);

//!
//! @param theta0
//! @param theta1
//! @return true if theta0 and theta1, or some shift by 2pi, are equal
//!       within tolerance.
//!
//!
Public GEOMDLLIMPEXP bool    bsiTrig_equalAngles
(
double theta0,
double theta1
);

//!
//! @param angle IN      unnormalized angle
//! @return angle normalized into (-pi..pi)
//!
Public GEOMDLLIMPEXP double bsiTrig_getNormalizedAngle (double angle);

//!
//! @description Normalize the angle so that it falls in the range [0,2pi].
//! @param angle IN      unnormalized angle
//! @return normalized angle
//!
Public GEOMDLLIMPEXP double bsiTrig_getPositiveNormalizedAngle (double angle);

//!
//! @description Convert an angle (in radians) so that:
//! <UL>
//! <LI>0 is start of sweep range
//! <LI>1 is end of sweep
//! <LI>If outside of sweep, angle is positive.
//! </UL>
//! @param angle IN      unnormalized angle
//! @param startAngle IN      start of sweep range
//! @param sweep IN      sweep extent
//! @return fraction of angular sweep
//!
Public GEOMDLLIMPEXP double bsiTrig_normalizeAngleToSweep
(
double angle,
double startAngle,
double sweep
);




/*----------------------------------------------------------------------+
|                                                                       |
|   name        jmdlTrig_componentRange                                 |
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
Public GEOMDLLIMPEXP void    bsiTrig_extendComponentRange
(
double*         pMin,           /* IN OUT  min coordiante of range box. */
double*         pMax,           /* IN OUT  max coordinate of range box */
double          x0,             /* IN      constant term of numerator */
double          x1,             /* IN      cosine term of numerator */
double          x2,             /* IN      sine term of numerator */
double          w0,             /* IN      constant term of numerator */
double          w1,             /* IN      cosine term of numerator */
double          w2,             /* IN      sine term of numerator */
double          theta0,         /* IN      start angle */
double          sweep,          /* IN      sweep of live part of angle range */
bool            applySweep      /* IN      false to suppress use of angle range. */
);

//!
//! Normalize an value to a period with specified base value.
//! (Typically used with angles, but period is given as an arg so it might be other than 2pi)
//!
//!
Public GEOMDLLIMPEXP double bsiTrig_normalizeToPeriod
(
double a,
double a0,
double da
);

//!
//! @description normalize to a period centered around zero.  Shift by a multiple
//!   of fullPeriod to bring to between negative and positive half period.
//! @param a IN periodic value
//! @param fullPeriod IN the full period.
//!
Public GEOMDLLIMPEXP double bsiTrig_normalizeToPeriodAroundZero
(
double a,
double fullPeriod
);
//!
//! @description split a number into "digits" with varying digit size.
//! <p>
//! Examples:
//! <ul>
//!  <li>DEGREES (double) to DDD MM SS.xxx is split with 60,60,1000
//!  <li>ARCES (double) to ACRES SQFEET SQINCHES 43560 144
//! </p>
//! @param value IN value to split
//! @param valueIsNegative OUT true if original value was negative.
//! @param pDigit OUT array of numSplitter+1 integer digits.
//! @param residual OUT the difference between the original number and digit series, expressed as
//!                a signed fraction of the smallest unit.  Under normal rounding (i.e. with biasFactor=0)
//!               the residual is between -0.5 and 0.5.
//! @param pSplitter IN array of unit factors
//! @param numSplitter IN number of splits.
//! @param biasFactor IN factor indicating caller's urge to have rounding candidates near 0.5 rounded up or down.
//!        A bias factor of zero means rounding occurs by adding 0.5 and truncating.
//!        A nonzero bias factor means that in addition to adding 0.5 the candidate is increased by
//!            biasFactor times the smallest unit of precision for the candidate.  The smallest unit
//!            of precision is determined by the C intrinsic function "nextafter".
//!
Public GEOMDLLIMPEXP void bsiTrig_splitDoubleToUnitizedDigits
(
double value,
bool    *pValueIsNegative,
int    *pDigit,
double *pResidual,
int   *pSplitter,
int  numSplitter,
double biasFactor
);

//!
//! @description recombine the results of bsiTrig_splitDoubleToUnitizedDigits.
//! @param valueIsNegative IN true to negate value
//! @param pDigit IN array of (numSplitter+1) integer digits
//! @param residual IN residual, as a fraction of least significant place.
//! @param pSplitter IN array of digits spliters.
//! @param numSplitter IN numbr of splitters.
//!
Public GEOMDLLIMPEXP double bsiTrig_mergeUnitizedDigits
(
bool    valueIsNegative,
int    *pDigit,
double residual,
int   *pSplitter,
int  numSplitter
);

//! Convert (c,s,w) point to angle.
Public GEOMDLLIMPEXP double bsiTrig_trigPointToAngle (DPoint3dCR csw);


//! Evalute the (quadratic) bezier on the trig poles.
//! convert the resulting (cosine, sine, weight) to an angle.
//! @param [in] trigPole array of 3 (three !!) poles of [c,s,w] order 3 bezier.
//! @param [in] fraction bezier evaluation parameter
//! @return angle (in radians)
Public GEOMDLLIMPEXP double bsiTrig_quadricTrigPointBezierFractionToAngle
(
DPoint3d *trigPole,
double f
);


END_BENTLEY_GEOMETRY_NAMESPACE

