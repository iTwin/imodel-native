/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


static double s_lineUnitCircleIntersectionTolerance = 1.0e-12;
static double msGeomConst_smallAngle       = 1.0e-12;

#define FIX_MIN(value, min)          if (value < min) min = value
#define FIX_MAX(value, max)          if (value > max) max = value



/*-----------------------------------------------------------------*//**
* @param degrees => angle in degrees
* @return angle in radians
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiTrig_degreesToRadians

(
double      degrees
)

    {
    return degrees * msGeomConst_radiansPerDegree;
    }


/*-----------------------------------------------------------------*//**
* @param radians => angle in radians
* @return angle in degrees
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiTrig_radiansToDegrees

(
double      radians
)

    {
    return radians * msGeomConst_degreesPerRadian;
    }


/*-----------------------------------------------------------------*//**
* Try to divide numerator by denominator.  If denominator is near zero,
*   return false and set the result to a default value.
* @param pRatio <= numerator / denominator if safe, defaultRatio otherwise
* @param numerator => numerator of ratio
* @param denominator => denominator of ratio
* @param defaultRatio => default ratio if b is small
* @return true if division is numerically safe.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTrig_safeDivide

(
double  *pRatio,
double  numerator,
double  denominator,
double  defaultRatio
)
    {
    static double s_relTol = 1.0e-12;
    if (fabs (denominator) > s_relTol * fabs (numerator))
        {
        *pRatio = numerator / denominator;
        return true;
        }
    else
        {
        *pRatio = defaultRatio;
        return false;
        }
    }


/*-----------------------------------------------------------------*//**
One dimensional interpolation.
* @param [in] a0 start of interval
* @parma [in] fraction 
* @param [in] a1 end of interval
* @return interpolated value.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiTrig_interpolate

(
double  a0,
double  fraction,
double  a1
)
    {
    // be fussy about order -- 
    if (fraction < 0.5)
        return a0 + fraction * (a1 - a0);
    else
        return a1 + (1.0 - fraction) * (a0 - a1);
    }

/*-----------------------------------------------------------------*//**
* Find the fractional position of a between a0 and a1, testing if a1 and a0
*   are too close to do the division safely.
* @param pRatio <= (a - a0) / (a1 - a0) if safe, defaultRatio otherwise
* @param a => position within interval
* @param a0 => start of interval
* @param a1 => end of interval
* @param defaultRatio => default result if division is not safe.
* @return true if division is numerically safe.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTrig_safeInverseLinearInterpolate

(
double  *pRatio,
double  a,
double  a0,
double  a1,
double  defaultRatio
)
    {
    double numerator   =  a - a0;
    double denominator = a1 - a0;
    return DoubleOps::SafeDivide (*pRatio, numerator, denominator, defaultRatio);
    }



/*-----------------------------------------------------------------*//**
* @param angle => angle in radians
* @return angle shifted by 2pi towards (and usually past) zero.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiTrig_complementaryAngle

(
double  angle
)
    {
    if (angle > 0.0)
        {
        return angle - msGeomConst_2pi;
        }
    else
        {
        return angle + msGeomConst_2pi;
        }
    }


/*-----------------------------------------------------------------*//**
* @param theta => nominal angle
* @param theta0 => start of interval
* @param sweep => signed sweep
* @return angle theta adjusted into the 2pi interval beginning at theta0
*       and going in the direction of sweep.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiTrig_adjustAngleToSweep

(
double  theta,
double  theta0,
double  sweep
)

    {
    double fraction = bsiTrig_normalizeAngleToSweep (theta, theta0, sweep);
    return theta0 + fraction * sweep;
    }


/*---------------------------------------------------------------------------------**//**
* Restrict a sweep to that (a) its absolute value is no more than 2pi (full circle) and
* (b) its sign is preserved. (Hence both +2pi and -2pi are possible).   Any value with larger
* absolute value is pulled back to the nearer limit.
* @param sweep => angle to adjust.
*
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   bsiTrig_restrictSweep

(
double      sweep
)
    {
    if (sweep > msGeomConst_2pi)
        sweep = msGeomConst_2pi;
    else if (sweep < -msGeomConst_2pi)
        sweep = -msGeomConst_2pi;
    return sweep;
    }


/*----------------------------------------------------------------------+
|SECTION Spherical coordinates                                          |
|                                                                       |
| Spherical coordinates are related to xyz by:                          |
|<UL>                                                                   |
|<LI>theta = longitude, i.e. angle from (1,0,0) to (x,y,0)              |
|<LI>phi   = latitude, i.e. angle from (x,y,0) and (x,y,z)              |
|<LI>r     = distance from origin                                       |
|</UL>                                                                  |
+----------------------------------------------------------------------*/


/*-----------------------------------------------------------------*//**
* Convert xyz coordinates to spherical (theta, phi, r)                  |
*<UL>
*<LI>theta = longitude, i.e. angle of (x,y,0) from (1,0,0)
*<LI>phi   = latitude, i.e. angle between (x,y,z) and (x,y,0)
*<LI>r     = distance from origin
*</UL>
*
* @param pSphericalPoint => theta, phi, r
* @param pXYZ <= x,y,z
* @return true unless r is zero.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiVector_cartesianToSpherical

(
DPoint3dP pSphericalPoint,
DPoint3dP pXYZ
)

    {
    double r = pXYZ->Magnitude ();
    double rho;
    double theta, phi;
    if (r == 0.0)
        {
        pSphericalPoint->Zero ();
        }
    else
        {
        rho = sqrt (pXYZ->x * pXYZ->x + pXYZ->y * pXYZ->y);
        if (rho <= 0.0)
            {
            theta = 0.0;
            }
        else
            {
            theta = atan2 (pXYZ->y, pXYZ->x);
            }
        phi = atan2 (pXYZ->z, rho);
        pSphericalPoint->Init ( theta, phi, r);
        }
    return true;
    }



/*-----------------------------------------------------------------*//**
* Convert (theta, phi, r) coordinates to spherical (x,y,z)
*
* @param pXYZ <= x,y,z
* @param pSphericalPoint => theta, phi, r
* @return true (always)
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiVector_sphericalToCartesian

(
DPoint3dP pXYZ,
DPoint3dP pSphericalPoint
)

    {
    double cosTheta = cos (pSphericalPoint->x);
    double sinTheta = sin (pSphericalPoint->x);
    double cosPhi   = cos (pSphericalPoint->y);
    double sinPhi   = sin (pSphericalPoint->y);
    double r      = pSphericalPoint->z;

    pXYZ->x = r * cosTheta * cosPhi;
    pXYZ->y = r * sinTheta * cosPhi;
    pXYZ->z = r * sinPhi;

    return true;
    }



/*-----------------------------------------------------------------*//**
* @return system wide small angle tolerance
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiTrig_smallAngle

(
void
)
    {
    return  msGeomConst_smallAngle;
    }


/*-----------------------------------------------------------------*//**
* Compare floating point values, using a tolerance of system small angle times the
*   larger of 1 or either given values.
* @return -1, 0, or 1 according as a < b, a == b, or a > c with the tolerance
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiTrig_tolerancedComparison

(
double a,
double b
)
    {
    double bigValue = 1.0;
    double aa = fabs (a);
    double bb = fabs (b);
    double diff = fabs (a - b);

    if (aa > bigValue)
        bigValue = aa;

    if (bb > bigValue)
        bigValue = bb;

    if (diff < msGeomConst_smallAngle * bigValue)
        return 0;
    if (a < b)
        return -1;
    return 1;
    }


/*-----------------------------------------------------------------*//**
* Compare floating point values, using a tolerance of system small angle times the
*   larger of 1 or either given values or a further reference value
* @param a => first value for comparison
* @param b => second value for comparison
* @param c => additional value which may expand the tolerance.
* @return -1, 0, or 1 according as a < b, a == b, or a > c with the tolerance
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiTrig_tolerancedComparisonWithReferenceDouble

(
double a,
double b,
double c
)
    {
    double bigValue = 1.0;
    double aa = fabs (a);
    double bb = fabs (b);
    double cc = fabs (c);
    double diff = fabs (a - b);

    if (aa > bigValue)
        bigValue = aa;

    if (bb > bigValue)
        bigValue = bb;

    if (cc > bigValue)
        bigValue = cc;

    if (diff < msGeomConst_smallAngle * bigValue)
        return 0;
    if (a < b)
        return -1;
    return 1;
    }


/*-----------------------------------------------------------------*//**
* Compare floating point values, using a tolerance of system small angle times the
*   larger of 1 or either given values or the largest component of a further reference point
* @param a => first value for comparison
* @param b => second value for comparison
* @param pPoint => point whose largest coordinate (x,y, or z) may expand tolerance.
* @return -1, 0, or 1 according as a < b, a == b, or a > c with the tolerance
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiTrig_tolerancedComparisonWithReferenceDPoint3d

(
double a,
double b,
DPoint3dCP pPoint
)
    {
    double c = pPoint->MaxAbs ();
    return bsiTrig_tolerancedComparisonWithReferenceDouble (a, b, c);
    }


/*-----------------------------------------------------------------*//**
* Compare floating point values, using a tolerance of system small angle times the
*   larger of 1 or either given values or the largest component of a further reference point
* @param a => first value for comparison
* @param b => second value for comparison
* @param pPoint => range whose largest coordinate (x,y, or z) may expand tolerance.
* @return -1, 0, or 1 according as a < b, a == b, or a > c with the tolerance
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiTrig_tolerancedComparisonWithReferenceDRange3d

(
double a,
double b,
DRange3dCP pRange
)
    {
    double c = bsiDRange3d_getLargestCoordinate (pRange);
    return bsiTrig_tolerancedComparisonWithReferenceDouble (a, b, c);
    }

Public GEOMDLLIMPEXP int bsiTrig_tolerancedComparisonWithReferenceDPoint3dDPoint3d

(
double a,
double b,
DPoint3dCP pPoint1,
DPoint3dCP pPoint2
)
    {
    return bsiTrig_tolerancedComparisonWithReferenceDouble (a, b,
            bsiTrig_maxAbsDPoint3dDPoint3d (pPoint1, pPoint2));
    }

static double s_defaultScaleDistance = 10000.0;

Public GEOMDLLIMPEXP bool bsiTrig_nearlyEqualDPoint3d
(
DPoint3dCP pointA,
DPoint3dCP pointB
)
    {
    double d = pointA->Distance (*pointB);

    return 0 == bsiTrig_tolerancedComparisonWithReferenceDPoint3dDPoint3d (s_defaultScaleDistance + d, s_defaultScaleDistance, pointA, pointB);
    }

Public GEOMDLLIMPEXP bool bsiTrig_pointNearlyOnRay
(
DPoint3dCP pointA,
DPoint3dCP originB,
DVec3dCP   directionB
)
    {
    DPoint3d pointC;
    double fractionC;
    bsiGeom_projectPointToRay (&pointC, &fractionC, pointA, originB, directionB);
    return bsiTrig_nearlyEqualDPoint3d (pointA, &pointC);
    }



/*-----------------------------------------------------------------*//**
* @return the largest absolute value of two given doubles
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiTrig_maxAbsDoubleDouble

(
double a,
double b
)
    {
    double abs0 = fabs (a);
    double abs1 = fabs (b);
    return (abs0 > abs1) ? abs0 : abs1;
    }


/*-----------------------------------------------------------------*//**
* @return the largest absolute value appearing in a point and a double
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiTrig_maxAbsDPoint3dDouble

(
DPoint3dCP pPoint,
double b
)
    {
    double abs0 = pPoint->MaxAbs ();
    double abs1 = fabs (b);
    return (abs0 > abs1) ? abs0 : abs1;
    }


/*-----------------------------------------------------------------*//**
* @return the largest absolute value appearing in two points.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiTrig_maxAbsDPoint3dDPoint3d

(
DPoint3dCP pPoint0,
DPoint3dCP pPoint1
)
    {
    double abs0 = pPoint0->MaxAbs ();
    double abs1 = pPoint1->MaxAbs ();
    return (abs0 > abs1) ? abs0 : abs1;
    }


/*-----------------------------------------------------------------*//**
* @return the largest absolute value appearing in two points and a double
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiTrig_maxAbsDPoint3dDPoint3dDouble

(
DPoint3dCP pPoint0,
DPoint3dCP pPoint1,
double b
)
    {
    double abs0 = pPoint0->MaxAbs ();
    double abs1 = pPoint1->MaxAbs ();
    double abs2 = fabs (b);

    if (abs0 >= abs1)
        return (abs2 > abs0) ? abs2 : abs0;
    else
        return (abs2 > abs1) ? abs2 : abs1;
    }



/*-----------------------------------------------------------------*//**
* Wrap the system atan2 with checks for (0,0) arguments.
*
* @param y => numerator
* @param x => denominator
* @return arctan of y/x.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiTrig_atan2 (double  y, double  x)
    {
    return Angle::Atan2 (y,x);
    }



/*-----------------------------------------------------------------*//**
* @param angle => angle to test
* @return true if (absolute value of) angle is more than 2pi minus a tolerance
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTrig_isAngleFullCircle

(
double angle
)
    {
    return fabs (angle) > (msGeomConst_2pi - msGeomConst_smallAngle);
    }


/*-----------------------------------------------------------------*//**
* @param angle => angle to test
* @return true if (absolute value of) angle is less than the small angle tolerance.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTrig_isAngleNearZero

(
double angle
)
    {
    return fabs (angle) < msGeomConst_smallAngle;
    }



/*-----------------------------------------------------------------*//**
* @param theta0
* @param theta1
* @return true if theta0 and theta1, or some shift by 2pi, are equal
*       within tolerance.
*
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTrig_equalAngles

(
double theta0,
double theta1
)

    {
    double delta = fabs (theta0 - theta1);
    int periodCount;
    if (delta < msGeomConst_smallAngle)
        return true;
    if (delta < msGeomConst_2pi - msGeomConst_smallAngle)
        return false;
    if (delta <= msGeomConst_2pi + msGeomConst_smallAngle)
        return true;
    periodCount = (int)(delta / msGeomConst_2pi);
    delta -= periodCount * msGeomConst_2pi;
    if (delta < msGeomConst_smallAngle)
        return true;
    if (delta > msGeomConst_2pi - msGeomConst_smallAngle)
        return true;
    return false;
    }




/*-----------------------------------------------------------------*//**
* @param angle => unnormalized angle
* @return angle normalized into (-pi..pi)
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiTrig_getNormalizedAngle

(
double angle
)

    {
    int periodCount;
    if (angle <= msGeomConst_pi)
        {
        if (angle <= -msGeomConst_pi)
            {
            periodCount = (int)((msGeomConst_pi - angle) / msGeomConst_2pi);
            angle += periodCount * msGeomConst_2pi;
            }
        }
    else
        {
        periodCount = (int)((angle + msGeomConst_pi) / msGeomConst_2pi);
        angle -= periodCount *msGeomConst_2pi;
        }
    return  angle;
    }


/*-----------------------------------------------------------------*//**
* @description Normalize the angle so that it falls in the range [0,2pi].
* @param angle => unnormalized angle
* @return normalized angle
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiTrig_getPositiveNormalizedAngle

(
double angle
)
    {
    int periodCount;
    if (angle < 0.0)
        {
        /* Add enough multiples of pi to make it positive */
        periodCount = 1 + (int) (-angle /msGeomConst_2pi);
        angle += periodCount * msGeomConst_2pi;
        }
    else
        {
        periodCount = (int) (angle /msGeomConst_2pi);
        angle -= periodCount * msGeomConst_2pi;
        }

    return  angle;
    }


/*-----------------------------------------------------------------*//**
* @description Convert an angle (in radians) so that:
* <UL>
* <LI>0 is start of sweep range
* <LI>1 is end of sweep
* <LI>If outside of sweep, angle is positive.
* </UL>
* @param angle => unnormalized angle
* @param startAngle => start of sweep range
* @param sweep => sweep extent
* @return fraction of angular sweep
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiTrig_normalizeAngleToSweep

(
double angle,
double startAngle,
double sweep
)

    {
    double fraction;
    angle = angle - startAngle;

    if (sweep < 0.0)
        {
        angle = - angle;
        sweep = - sweep;
        }

    if (angle >= 0.0 && angle <= sweep)
        {
        /* The common case ... just divide it out */
        fraction = angle / sweep;
        }
    else
        {
        angle = bsiTrig_getPositiveNormalizedAngle (angle);
        if (fabs (angle - msGeomConst_2pi) <= msGeomConst_smallAngle)
            {
            fraction = 0.0;
            }
        else if (fabs (sweep) < msGeomConst_smallAngle)
            {
            fraction = 0.0;
            }
        else
            {
            fraction = angle / sweep;
            }
        }

    return fraction;
    }



/*-----------------------------------------------------------------*//**
* @param testAngle => angle to test
* @param startAngle => start of sweep range
* @param sweepAngle => signed sweep
* @return true if angle is within the signed, toleranced sweep.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTrig_angleInSweep

(
double testAngle,
double startAngle,
double sweepAngle
)
    {
     double offset;
     bool    result = false;
     double minusZero = - msGeomConst_smallAngle;
     double excess;

     if (sweepAngle > 0)
         {
         offset = testAngle - startAngle;
         }
     else
         {
         offset = startAngle - testAngle;
         sweepAngle = -sweepAngle;
         }

     sweepAngle += msGeomConst_smallAngle;

     if (offset >= minusZero && offset <= sweepAngle)
         {
         result = true;
         }
     else if (offset > 0.0)
         {
         /* Handle first wraparound directly */
         excess = offset - msGeomConst_2pi;
         if (excess >= minusZero && excess <= sweepAngle)
             {
             result = true;
             }
         else
             {
             /* It's a big angle.  Have to normalize */
             int numPeriod = (int)(offset / msGeomConst_2pi);
             excess = offset - numPeriod * msGeomConst_2pi;
             if (excess < sweepAngle || excess > msGeomConst_2pi - msGeomConst_smallAngle)
                 result = true;

             }
         }
     else /* negative offset */
         {
         /* Handle first wraparound directly */
         excess = offset + msGeomConst_2pi;
         if (excess >= minusZero && excess <= sweepAngle)
             {
             result = true;
             }
         else
             {
             /* It's a big angle.  Have to normalize */
             int numPeriod = (int)(-offset / msGeomConst_2pi);
             excess = offset + (numPeriod + 1) * msGeomConst_2pi;
             if (excess < sweepAngle || excess > msGeomConst_2pi - msGeomConst_smallAngle)
                 result = true;

             }
         }
    return  result;

    }


/*-----------------------------------------------------------------*//**
* @param testAngle => angle to test
* @param startAngle => start of sweep range
* @param endAngle => end of sweep range.
* @return true if angle is within the angular range
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTrig_angleInRange

(
double testAngle,
double startAngle,
double endAngle
)
    {
    return Angle::InSweepAllowPeriodShift (testAngle, startAngle, endAngle - startAngle);
    }

static double s_relTol = 1.0e-12;
static double s_absTol = 1.0e-12;


/*-----------------------------------------------------------------*//**
* Test if a (not angular) scalar is within a given interval, using a relative tolerance.
* @param a => fraction to test
* @param a0 => start of interval
* @param a1 => end of interval
* @return true if a is within the signed, toleranced interval
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTrig_scalarInInterval

(
double a,
double a0,
double a1
)
    {
    double absTol;
    /* Quick exit for most common in case */
    if ((a - a0) * (a - a1) <= 0.0)
        return true;

    /* tolerance test at endpoints. */
    absTol = s_absTol + s_relTol * (fabs (a) + fabs (a0) + fabs (a1));
    return fabs (a - a0) < absTol || fabs (a - a1) < absTol;
    }


/*-----------------------------------------------------------------*//**
* Test if a (scalar, not angular) scalar is within a given sweep, using a relative tolerance.
* @param a => fraction to test
* @param a0 => start of interval
* @param da => (signed) sweep of interval (end minus start)
* @return true if a is within the signed, toleranced sweep.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTrig_scalarInSweep

(
double a,
double a0,
double da
)
    {
    double absTol;
    /* convert to offset from start point */
    a -= a0;
    /* Force sweep to be positive */
    if (da < 0.0)
        {
        da = - da;
        a  = - a;
        }

    if (a >= 0.0 && a <= da)
        return true;

    /* tolerance test at endpoints. */
    absTol = s_absTol + s_relTol * (fabs (a) + fabs (a0) + fabs (da));
    return fabs (a) < absTol || fabs (a - da) < absTol;
    }

/*-----------------------------------------------------------------*//**
* Update a bounding interval of a line based on the variation of that
* line within one dimension.
*
* @param pParam0 <=> lower limit of 'in' segment in parametric space
* @param pParam1 <=> upper limit of 'in' segment in parametric space
* @param pContents <=> description of current intersection interval.  Updated here.
                            -1 ===> unbounded.
                             0 ===> empty
                             1 ===> bounded
* @param x0 => Start coordinate in this dimension
* @param dxds => Rate of change of this coordiante wrt line's s coordinate
* @param low => start of bounding interval in this dimension
* @param high => end of bounding interval in this dimension
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRange1d_intersectLine

(
double  *pParam0,
double  *pParam1,
int     *pContents,
double  x0,
double  dxds,
double  low,
double  high
)

    {
    double s0, s1;
    if (*pContents == 0)
        return;

    if (dxds != 0.0)
        {
        s0 = (low  - x0) / dxds;
        s1 = (high - x0) / dxds;
        if (s1 < s0)
            {
            double temp = s0;
            s0 = s1;
            s1 = temp;
            }

        if (*pContents < 0)
            {
            /* No prior bounds to consider */
            *pParam0 = s0;
            *pParam1 = s1;
            *pContents = 1;   /* It will get tested for empty interval later */
            }
        else if (*pContents == 0)
            {
            /* interval is already empty */
            }
        else
            {
            if (s0 > *pParam0)
                *pParam0 = s0;
            if (s1 < *pParam1)
                *pParam1 = s1;
            }
        }
    else
        {
        if ( (x0 - low) * (high - x0) >= 0.0)
            {
            /* remains contained for all s -- no need to change prior contents analysis */
            }
        else
            {
            *pContents = 0;
            }
        }

    if (*pContents > 0 && *pParam0 > *pParam1)
        *pContents = 0;
    }



/*-----------------------------------------------------------------*//**
*
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiGeom_cyclic2dAxis

(
int i
)
    {
    return i & 0x01;
    }

/*-----------------------------------------------------------------*//**
*
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiGeom_cyclic3dAxis

(
int i
)
    {
    if  (0 <= i)
        {
        if  (i < 3)
            {
            return  i;
            }
        else
            return  i % 3;
        }
    else
        {
        if  (i > -3)
            {
            return  3 + i;
            }
        else
            {
            return  2 - ((-1 - i) % 3);
            }
        }
    }


/*-----------------------------------------------------------------*//**
*
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiGeom_cyclic3dAxes

(
int *pI0,
int *pI1,
int *pI2,
int i
)
    {
    *pI0 = bsiGeom_cyclic3dAxis (i);
    *pI1 = (*pI0 + 1) % 3;
    *pI2 = (*pI1 + 1) % 3;
    }


/*-----------------------------------------------------------------*//**
*
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiGeom_cyclic2dAxes

(
int *pI0,
int *pI1,
int i
)
    {
    *pI0 = i & 0x01;
    *pI1 = 1 - *pI0;
    }

/* STUBS FOR BACKWARD COMPATIBILITY -- do not export as methods */
/*---------------------------------------------------------------------------------**//**
* Deprecated function -- use bsiTrig_getNormalizedAngle
* @param    theta => angle to normalize
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiRange1d_getNormalizedAngle

(
double theta
)
        {
        return bsiTrig_getNormalizedAngle (theta);
        }

/*---------------------------------------------------------------------------------**//**
* Deprecated function -- use bsiTrig_getPositiveNormalizedAngle
* @param    theta => angle to normalize
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiRange1d_getPositiveNormalizedAngle

(
double theta
)
        {
        return bsiTrig_getPositiveNormalizedAngle (theta);
        }

/*---------------------------------------------------------------------------------**//**
* Deprecated function -- use bsiTrig_normalizeAngleToSweep
* @param    theta => angle to normalize
* @param    start => start of sweep range
* @param    sweep => signed sweep
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiRange1d_normalizeAngleToSweep

(
double theta,
double start,
double sweep
)
        {
        return bsiTrig_normalizeAngleToSweep (theta, start, sweep);
        }

/*---------------------------------------------------------------------------------**//**
* Deprecated function -- use Angle::InSweepAllowPeriodShift.
* WARNING-- this function is incorrectly typed as a double.  The original bsiRange1d_angleInSweep,
*  and the current Angle::InSweepAllowPeriodShift, are bool.  It's here so your old code can be
*  compiled, but please change over.
* @param    theta => angle to test
* @param    start => start of sweep range
* @param    sweep => signed sweep
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiRange1d_angleInSweep

(
double theta,
double start,
double sweep
)
        {
        return Angle::InSweepAllowPeriodShift (theta, start, sweep);
        }

/*----------------------------------------------------------------------+
|                                                                       |
|   name        jmdlTrig_componentRange                                 |
|                                                                       |
|   author      EarlinLutz                                              |
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
double*         pMin,           /* <=> min coordiante of range box. */
double*         pMax,           /* <=> max coordinate of range box */
double          x0,             /* => constant term of numerator */
double          x1,             /* => cosine term of numerator */
double          x2,             /* => sine term of numerator */
double          w0,             /* => constant term of numerator */
double          w1,             /* => cosine term of numerator */
double          w2,             /* => sine term of numerator */
double          theta0,         /* => start angle */
double          sweep,          /* => sweep of live part of angle range */
bool            applySweep      /* => false to suppress use of angle range. */
)
    {
    double alpha, beta, gamma;
    int numPole, i;
    double numerator, denominator, x;
    double cosine[2], sine[2];
    double angle;

    alpha   = x2 * w1 - x1 * w2;
    beta    = x2 * w0 - x0 * w2;
    gamma   = x0 * w1 - x1 * w0;

    numPole = bsiMath_solveApproximateUnitQuadratic (
                &cosine[0], &sine[0],
                &cosine[1], &sine[1],
                alpha, beta, gamma,
                s_lineUnitCircleIntersectionTolerance
                );

    if (numPole > 0)
        {
        for (i = 0; i < numPole; i++)
            {
            angle = Angle::Atan2 (sine[i], cosine[i]);
            if (!applySweep || Angle::InSweepAllowPeriodShift (angle, theta0, sweep))
                {
                numerator   = x0 + x1 * cosine[i] + x2 * sine[i];
                denominator = w0 + w1 * cosine[i] + w2 * sine[i];
                if (denominator != 0.0)
                    {
                    x = numerator / denominator;
                    FIX_MIN(x, *pMin);
                    FIX_MAX(x, *pMax);
                    }
                }
            }
        }
    else
        {
        /* degenerate equation.*/
        /* Plug in the point at 0 degrees as a dummy*/
        numerator = x0 + x1;
        denominator = w0 + w1;
        if (denominator != 0.0)
            {
            x = numerator / denominator;
            FIX_MIN(x, *pMin);
            FIX_MAX(x, *pMax);
            }
        }
    }


/**
@description split a number into "digits" with varying digit size.
<p>
Examples:
<ul>
  <li>DEGREES (double) to DDD MM SS.xxx is split with 60,60,1000</li>
  <li>ARCES (double) to ACRES SQFEET SQINCHES 43560 144</li>
</p>
@param value IN value to split
@param valueIsNegative OUT true if original value was negative.
@param pDigit OUT array of numSplitter+1 integer digits.
@param residual OUT the difference between the original number and digit series, expressed as
                a signed fraction of the smallest unit.  Under normal rounding (i.e. with biasFactor=0)
               the residual is between -0.5 and 0.5.
@param pSplitter IN array of unit factors
@param numSplitter IN number of splits.
@param biasFactor IN factor indicating caller's urge to have rounding candidates near 0.5 rounded up or down.
        A bias factor of zero means rounding occurs by adding 0.5 and truncating.
        A nonzero bias factor means that in addition to adding 0.5 the candidate is increased by
            biasFactor times the smallest unit of precision for the candidate.  The smallest unit
            of precision is determined by the C intrinsic function "nextafter".
*/
Public GEOMDLLIMPEXP void bsiTrig_splitDoubleToUnitizedDigits
(
double value,
bool    *pValueIsNegative,
int    *pDigit,
double *pResidual,
int   *pSplitter,
int  numSplitter,
double biasFactor
)
    {
    if (pValueIsNegative)
        *pValueIsNegative = value < 0.0;
    value = fabs (value);
    double a = value;
    for (int i = 0; i < numSplitter; i++)
        a *= pSplitter[i];
    // Now a is the number of smallest-unit increments, with floating remainder.  Round it to intger su:
    double b = a + 0.5;
    if (biasFactor != 0.0)
        {
        double c = BeNumerical::BeNextafter (b, DBL_MAX);
        b += biasFactor * (c - b);
        }
    int n = (int)(b);
    if (pResidual)
        *pResidual = a - (double)n;
    for (int k = numSplitter - 1; k >= 0; k--)
        {
        pDigit[k+1] = n % pSplitter[k];
        n /= pSplitter[k];
        }
    pDigit[0] = n;
    }

/**
@description recombine the results of bsiTrig_splitDoubleToUnitizedDigits.
@param valueIsNegative IN true to negate value
@param pDigit IN array of (numSplitter+1) integer digits
@param residual IN residual, as a fraction of least significant place.
@param pSplitter IN array of digits spliters.
@param numSplitter IN numbr of splitters.
*/
Public GEOMDLLIMPEXP double bsiTrig_mergeUnitizedDigits
(
bool    valueIsNegative,
int    *pDigit,
double residual,
int   *pSplitter,
int  numSplitter
)
    {
    double microunits = pDigit[0];
    double f = 1.0;
    for (int i = 0; i < numSplitter; i++)
        {
        microunits = microunits * pSplitter[i] + pDigit[i+1];
        f *= pSplitter[i];
        }
    microunits += residual;
    double value = microunits / f;
    if (valueIsNegative)
        value = - value;
    return value;
    }



/*-----------------------------------------------------------------*//**
* Construct the givens rotation trig values (cosine, sine) so as to
* transfer the weight of entries to the "a" position, and make "b" zero.
*
* @param pCosine <= cosine of givens angle
* @param pSine <= sine of givens angle
* @param a => x part of vector
* @param b => y part of vector
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTrig_constructGivensWeights

(
double      *pCosine,
double      *pSine,
double      a,
double      b
)
    {
#ifdef GOLUB_AND_VAN_LOAN
    /* Exact test for zero is ok.  If b is epsilon and a is large, we will divide by a (safe).*/
    /*      If both are near zero, we will still divide by the larger and get something no*/
    /*      more than one.*/
    double t;
    if (b == 0.0)
        {
        *pCosine = 1.0;
        *pSine   = 0.0;
        }
    else if (fabs (a) > fabs (b))
        {
        t = b / a;
        *pCosine = 1.0 / sqrt (1.0 + t * t);
        *pSine   = (*pCosine) * t;
        }
    else
        {
        t = a / b;
        *pSine = 1.0 / sqrt (1.0 + t * t);
        *pCosine   = (*pSine) * t;
    }
#else
    double scale = fabs (a) + fabs(b);
    if (scale == 0.0)
        {
        *pCosine = 1.0;
        *pSine   = 0.0;
        }
    else
        {
        /* divide both by sum of absolute values to prevent overflow while squaring.*/
        /* (Stewart)*/
        double f = 1.0 / scale;
        double denom;
        a *= f;
        b *= f;
        denom = sqrt (a * a + b * b);
        *pSine = b / denom;
        *pCosine = a / denom;
        }
#endif
    }


/*-----------------------------------------------------------------*//**
* @param pA <= modified x part of vector
* @param pB <= modified y part of vector
* @param a => x part of vector
* @param b => y part of vector
* @param cosine <= cosine of givens angle
* @param sine <= sine of givens angle
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTrig_applyGivensWeights

(
double      *pA,
double      *pB,
double      a,
double      b,
double      cosine,
double      sine
)
    {
    *pA = a * cosine + b * sine;
    *pB = b * cosine - a * sine;
    }


/*-----------------------------------------------------------------*//**
* @bsihdr                                       EarlinLutz      11/15
+---------------+---------------+---------------+---------------+------*/
Public double bsiTrig_trigPointToAngle (DPoint3dCR csw)
    {
    if (csw.z < 0.0)
        return atan2 (-csw.y, -csw.x);
    else
        return atan2 ( csw.y,  csw.x);
    }

/*-----------------------------------------------------------------*//**
* @bsihdr                                       EarlinLutz      11/15
+---------------+---------------+---------------+---------------+------*/
Public double bsiTrig_quadricTrigPointBezierFractionToAngle
(
DPoint3d *trigPole,
double f
)
    {
    double b0, b1, b2;
    Polynomial::Bezier::Order3::BasisFunctions (f, b0, b1, b2);
    DPoint3d csw = DPoint3d::FromSumOf (trigPole[0], b0, trigPole[1], b1, trigPole[2], b2);
    return bsiTrig_trigPointToAngle (csw);
    }

END_BENTLEY_GEOMETRY_NAMESPACE
