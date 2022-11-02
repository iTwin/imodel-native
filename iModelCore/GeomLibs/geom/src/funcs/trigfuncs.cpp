/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


static double s_lineUnitCircleIntersectionTolerance = 1.0e-12;
static double msGeomConst_smallAngle       = 1.0e-12;

#define FIX_MIN(value, min)          if (value < min) min = value
#define FIX_MAX(value, max)          if (value > max) max = value

/*-----------------------------------------------------------------*//**
* Try to divide numerator by denominator.  If denominator is near zero,
*   return false and set the result to a default value.
* @param pRatio <= numerator / denominator if safe, defaultRatio otherwise
* @param numerator => numerator of ratio
* @param denominator => denominator of ratio
* @param defaultRatio => default ratio if b is small
* @return true if division is numerically safe.
* @bsimethod
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
* @param theta => nominal angle
* @param theta0 => start of interval
* @param sweep => signed sweep
* @return angle theta adjusted into the 2pi interval beginning at theta0
*       and going in the direction of sweep.
* @bsimethod
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
* @bsimethod
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
* @return system wide small angle tolerance
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiTrig_smallAngle

(
void
)
    {
    return  msGeomConst_smallAngle;
    }



/*-----------------------------------------------------------------*//**
* Wrap the system atan2 with checks for (0,0) arguments.
*
* @param y => numerator
* @param x => denominator
* @return arctan of y/x.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiTrig_atan2 (double  y, double  x)
    {
    return Angle::Atan2 (y,x);
    }



/*-----------------------------------------------------------------*//**
* @param angle => angle to test
* @return true if (absolute value of) angle is more than 2pi minus a tolerance
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTrig_isAngleFullCircle

(
double angle
)
    {
    return fabs (angle) > (msGeomConst_2pi - msGeomConst_smallAngle);
    }


/*-----------------------------------------------------------------*//**
* @param theta0
* @param theta1
* @return true if theta0 and theta1, or some shift by 2pi, are equal
*       within tolerance.
*
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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

/*---------------------------------------------------------------------------------**//**
* Conditional update range of one component of a homogeneous trig function. That is, find local extrema of <p> (x0 + x1 * cos(theta) + x2 *
* sin(theta)) / (w0 + w1 * cos(theta) + w2 * sin(theta)) <p> and augment minP, maxP if the angle of the extrema is 'in' the the sector set.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
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

#ifdef CompileAllTrigFuncs
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
#endif



/*-----------------------------------------------------------------*//**
* Construct the givens rotation trig values (cosine, sine) so as to
* transfer the weight of entries to the "a" position, and make "b" zero.
*
* @param pCosine <= cosine of givens angle
* @param pSine <= sine of givens angle
* @param a => x part of vector
* @param b => y part of vector
* @see
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiTrig_trigPointToAngle (DPoint3dCR csw)
    {
    if (csw.z < 0.0)
        return atan2 (-csw.y, -csw.x);
    else
        return atan2 ( csw.y,  csw.x);
    }

/*-----------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiTrig_quadricTrigPointBezierFractionToAngle
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
