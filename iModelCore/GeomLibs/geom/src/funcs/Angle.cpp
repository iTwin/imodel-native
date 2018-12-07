/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/funcs/Angle.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

#ifndef M_PI_2
// Pi/2 as stated in math.h . . .
#define M_PI_2     1.57079632679489661923
#endif
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#define AngleDigits11_not
#ifdef AngleDigits11
// Accept parasolid's word that 1e-11 is a sensible angle tolerance.
// (Geomlibs was at 1e-12 for a decade.)
static double msGeomConst_smallAngle       = 1.0e-11;
static dobule msGeomConst_smallAngleDegrees = 180.0e-11 / PI;
#else
// This is the angle tolernace in use in geomlibs since around Y2K . . .
static double msGeomConst_smallAngle       = 1.0e-12;
static double msGeomConst_smallAngleDegrees = 180.0e-12 / PI;
#endif
static double msGeomConst_tinyAngle       = 1.0e-15;
static double msGeomConst_smallFloatRadians   = 2.0e-6;

#define FIX_MIN(value, min)          if (value < min) min = value
#define FIX_MAX(value, max)          if (value > max) max = value
static double msGeomConst_mediumAngle       = 1.0e-10;


Angle operator *(Angle const &theta, double factor) { return Angle (theta.m_radians * factor); }
Angle operator *(double factor, Angle const &theta) { return Angle (theta.m_radians * factor); }
Angle operator -(Angle const &theta) { return Angle (-theta.m_radians); }
Angle operator +(Angle const &alpha, Angle const &beta){return Angle (alpha.m_radians + beta.m_radians);}
Angle operator -(Angle const &alpha, Angle const &beta) { return Angle (alpha.m_radians - beta.m_radians); }



// Classic radian-centric methods
void Angle::OverlapWrapableIntervals
(
double startRadiansA,
double sweepRadiansA,
double startRadiansB,
double sweepRadiansB,
bvector<DSegment1d> &startEndFractionA,
bvector<DSegment1d> &startEndFractionB
)
    {
    startRadiansB = Angle::AdjustToSweep (startRadiansB, startRadiansA, sweepRadiansA);    
    DSegment1d intervalB (startRadiansB, startRadiansB + sweepRadiansB);
    double a = Angle::TwoPi ();
    DSegment1d intervalA[3];
    intervalA[0] = DSegment1d (startRadiansA, startRadiansA + sweepRadiansA);
    intervalA[1] = intervalA[0].CopyTranslated (a);
    intervalA[2] = intervalA[0].CopyTranslated (-a);

    startEndFractionA.clear ();
    startEndFractionB.clear ();
    DSegment1d fractionA, fractionB;
    for (size_t i = 0; i < 3; i++)
        {
        if (DSegment1d::NonZeroFractionalDirectedOverlap (intervalA[i], intervalB, fractionA, fractionB))
            {
            startEndFractionA.push_back (fractionA);
            startEndFractionB.push_back (fractionB);
            }
        }
    }            


bool Angle::InExactSweep (double theta, double thetaStart, double sweep)
    {
    if (sweep >= 0.0)
        return theta >= thetaStart && theta <= thetaStart + sweep;
    return theta <= thetaStart && theta >= thetaStart + sweep;
    }

bool Angle::InSweepAllowPeriodShift (double theta, double thetaStart, double sweep)
    {
    if (Angle::NearlyEqualAllowPeriodShift (theta, thetaStart)
        || Angle::NearlyEqualAllowPeriodShift (theta, thetaStart + sweep))
        return true;
    double alpha = Angle::AdjustToSweep (theta, thetaStart, sweep);
    return Angle::InExactSweep (alpha, thetaStart, sweep);
    }

double Angle::NormalizeToSweep (double theta, double thetaStart, double sweep)
    {

    double theta1 = AdjustToSweep (theta, thetaStart, sweep);
    double fraction;
    DoubleOps::SafeDivideParameter (fraction, theta1 - thetaStart, sweep, 0.0);
    // prevent numerical fuzz from flipping "just outside the start" to end.
    if (fraction > 1.0 && NearlyEqualAllowPeriodShift (theta, thetaStart))
        return 0.0;
    return fraction;
    }

double Angle::NormalizeToSweep (double theta, double thetaStart, double sweep, bool extend0, bool extend1)
    {
    double theta1 = AdjustToSweep (theta, thetaStart, sweep);
    double fraction;
    DoubleOps::SafeDivideParameter (fraction, theta1 - thetaStart, sweep, 0.0);
    // fraction is greater than or equal to zero, may exceed 1 ...
    if (fraction <= 1.0)
        return fraction;

    if (extend1 && !extend0)
        return fraction;

    double maxFraction;
    // Positive fractional steps from each end ....
    DoubleOps::SafeDivideParameter (maxFraction, Angle::TwoPi (), fabs (sweep), 1.0);
    double e1 = fraction - 1.0;
    double e0 = maxFraction - fraction;
    if (extend0)
        {
        if (extend1)
            {
            // both extensions possible.  Compute (positive) fractional steps from each end ...
            return e0 < e1 ? -e0 : fraction;
            }
        return fraction - maxFraction;  // Should be negative !!!
        }
    else
        {
        return e1 < e0 ? 1.0 : 0.0;
        }
    }



double Angle::AdjustToSweep (double theta, double thetaStart, double sweep)
    {
    double alpha = theta - thetaStart;
    if (sweep > 0.0)
        {
        if (alpha > msGeomConst_2pi)
            return thetaStart + fmod (alpha, msGeomConst_2pi);
        else if (alpha >= 0.0)
            return theta;
        else if (alpha >= -msGeomConst_2pi)
            return theta + msGeomConst_2pi;
        else    // alpha < -msGeomConst_2pi
            return thetaStart + msGeomConst_2pi - fmod (-alpha, msGeomConst_2pi);
        }
    else if (sweep < 0.0)
        {
        if (alpha < -msGeomConst_2pi)
            return thetaStart - fmod (-alpha, msGeomConst_2pi);
        else if (alpha <= 0.0)
            return theta;
        else if (alpha <=  msGeomConst_2pi)
            return theta - msGeomConst_2pi;
        else    // alpha > msGeomConst_2pi
            return thetaStart - msGeomConst_2pi + fmod (alpha, msGeomConst_2pi);
        }
    else
        return AdjustToSweep (theta, thetaStart, msGeomConst_2pi);
    }

double Angle::PeriodShift (double theta, double periods)
    {
    return theta + periods * msGeomConst_2pi;
    }

bool Angle::IsFullCircle (double radians)
    {
    return fabs (radians) > (msGeomConst_2pi - msGeomConst_smallAngle);
    }

bool Angle::NearlyEqualAllowPeriodShift (double radiansA, double radiansB)
    {
    return IsNearZeroAllowPeriodShift (radiansA - radiansB);
    }

bool Angle::NearlyEqual (double radiansA, double radiansB)
    {
    return fabs (radiansA - radiansB) < msGeomConst_smallAngle;
    }

bool Angle::IsNearZero (double radians)
    {
    return fabs (radians) <= msGeomConst_smallAngle;
    }

bool Angle::IsNearZero () const {return IsNearZero (m_radians);}


bool Angle::IsNearZeroAllowPeriodShift (double radians)
    {
    radians = fabs (radians);
    if (radians <= msGeomConst_smallAngle)
        return true;
    if (radians < msGeomConst_2pi - msGeomConst_smallAngle)
        return false;
    if (radians <= msGeomConst_2pi + msGeomConst_smallAngle)
        return true;
    // radians is larger than 2pi.  Knock it down to [0..2pi).  Recursion will not reach this line again.
    return IsNearZeroAllowPeriodShift (fmod(radians, msGeomConst_2pi));
    }

double Angle::TwoPi ()      {return msGeomConst_2pi;}
double Angle::Pi    ()      {return msGeomConst_pi;}
double Angle::PiOver2   ()  {return msGeomConst_piOver2;}
double Angle::SmallAngle () {return msGeomConst_smallAngle;}
double Angle::SmallFloatRadians () {return msGeomConst_smallFloatRadians;}
double Angle::TinyAngle () {return msGeomConst_tinyAngle;}
double Angle::MediumAngle () {return msGeomConst_mediumAngle;}


double Angle::CircleFractionToRadians (double fraction) { return fraction * msGeomConst_pi;}

double Angle::DegreesToRadians (double degrees) { return degrees * (msGeomConst_pi / 180.0);}
double Angle::RadiansToDegrees (double radians) { return radians * (180.0 / msGeomConst_pi);}
double Angle::ReverseComplement (double radians)
    {
    radians = Angle::AdjustToSweep (radians, 0.0, radians);
    if (radians > 0.0)
        {
        return radians - msGeomConst_2pi;
        }
    else
        {
        return radians + msGeomConst_2pi;
        }
    }

double Angle::ForwardComplement (double radians)
    {
    radians = Angle::AdjustToSweep (radians, 0.0, radians);
    if (radians > 0.0)
        {
        return msGeomConst_2pi - radians;
        }
    else
        {
        return -(msGeomConst_2pi + radians);
        }
    }

void Angle::HalfAngleFuctions (double &cosA, double &sinA, double rCos2A, double rSin2A)
    {
    double r = sqrt (rCos2A * rCos2A + rSin2A * rSin2A);
    /* If the caller really gave you sine and cosine values, r should be 1.  However,*/
    /* to allow scaled values -- e.g. the x and y components of any vector -- we normalize*/
    /* right here.  This adds an extra sqrt and 2 divides to the whole process, but improves*/
    /* both the usefulness and robustness of the computation.*/
    double cos2A, sin2A;
    if (r == 0.0)
        {
        cosA = 1.0;
        sinA = 0.0;
        }
    else
        {
        cos2A = rCos2A / r;
        sin2A = rSin2A / r;
        if (cos2A >= 0.0)
            {
            /* Original angle in NE and SE quadrants.  Half angle in same quadrant */
            cosA = sqrt (0.5 * ( 1.0 + cos2A ));
            sinA = sin2A / (2.0 * (cosA));
            }
        else
            {
            if (sin2A > 0.0)
                {
                /* Original angle in NW quadrant. Half angle in NE quadrant */
                sinA = sqrt (0.5 * ( 1.0 - cos2A));
                }
            else
                {
                /* Original angle in SW quadrant. Half angle in SE quadrant*/
                /* cosA comes out positive because both sines are negative. */
                sinA = - sqrt (0.5 * ( 1.0 - cos2A));
                }
            cosA = sin2A / (2.0 * (sinA));
            }
        }
    }

void Angle::Rotate90UntilSmallAngle (double &x1, double &y1, double x0, double y0)
    {
    if (fabs (x0) <= fabs (y0))
        {
        if (y0 > 0.0)
            {
            /* Pointing up -- back up 90 degrees */
            x1 = y0;
            y1 = -x0;
            }
        else
            {
            x1 = -y0;
            y1 = x0;
            }
        }
    else
        {
        if (x0 < 0.0)
            {
            x1 = -x0;
            y1 = -y0;
            }
        else
            {
            x1 = x0;
            y1 = y0;
            }
        }
    }

double Angle::Atan2 (double y, double x)
    {
    if (y == -0.0)
        y = 0.0;        // force consistent behavior at branch.
    if (x == 0.0 && y == 0.0)
        return 0.0;
    return atan2 (y,x);
    }

double Angle::Acos(double arg)
    {
    if (arg >= 1.0)
        return 0.0;
    if (arg <= -1.0)
        return Angle::Pi ();
    return acos (arg);
    }

double Angle::Asin(double arg)
    {
    if (arg > 1.0)
       return Angle::PiOver2 ();
    else if (arg < -1.0)
       return -Angle::PiOver2 ();
    return asin (arg);
    }


void Angle::TrigCombinationRange
        (
        double constCoff, double cosCoff, double sinCoff,
        double &thetaMin, double &fMin,
        double &thetaMax, double &fMax
        )
    {
    if (sinCoff == 0.0 && cosCoff == 0.0)
        {
        // degenerate constant function. 
        fMin = fMax = constCoff;
        thetaMin = thetaMax = 0.0;
        }
    else
        {
        double thetaStar = atan2 (sinCoff, cosCoff);
        double c = cos (thetaStar);
        double s = sin (thetaStar);
        double e = cosCoff * c + sinCoff * s;
        if (e > 0.0)
            {
            fMin = constCoff - e;
            fMax = constCoff + e;
            thetaMax = thetaStar;
            thetaMin = thetaStar > 0.0 ? thetaStar - msGeomConst_pi : thetaStar + msGeomConst_pi;
            }
        else
            {
            fMax = constCoff - e;
            fMin = constCoff + e;
            thetaMin = thetaStar;
            thetaMax = thetaStar > 0.0 ? thetaStar - msGeomConst_pi : thetaStar + msGeomConst_pi;
            }
        }
    }

double Angle::EvaluateTrigCombination (double constCoff, double cosCoff, double sinCoff, double theta)
    {
    return constCoff + cosCoff * cos(theta) + sinCoff * sin(theta);
    }

void Angle::TrigCombinationRangeInSweep
        (
        double constCoff, double cosCoff, double sinCoff,
        double thetaA, double sweep,
        double &thetaMin, double &fMin,
        double &thetaMax, double &fMax
        )
    {
    double phiMin, phiMax, gMin, gMax;
    TrigCombinationRange (constCoff, cosCoff, sinCoff, phiMin, gMin, phiMax, gMax);
    thetaMin = thetaMax = thetaA;
    fMin = fMax = EvaluateTrigCombination (constCoff, cosCoff, sinCoff, thetaA);
    double thetaB = thetaA + sweep;
    double fB = EvaluateTrigCombination (constCoff, cosCoff, sinCoff, thetaB);
    DoubleOps::UpdateMinMax (thetaB, fB, thetaMin, fMin, thetaMax, fMax);

    if (Angle::InSweepAllowPeriodShift (phiMin, thetaA, sweep))
        DoubleOps::UpdateMinMax (phiMin, gMin, thetaMin, fMin, thetaMax, fMax);
    if (Angle::InSweepAllowPeriodShift (phiMax, thetaA, sweep))
        DoubleOps::UpdateMinMax (phiMax, gMax, thetaMin, fMin, thetaMax, fMax);
    }

int Angle::Cyclic2dAxis (int i)
    {
    return i & 0x01;
    }

void Angle::Cyclic3dAxes (int &i0, int &i1, int &i2, int i)
    {
    i0 = Cyclic3dAxis (i);
    i1 = (i0 + 1) % 3;
    i2 = (i1 + 1) % 3;
    }


int Angle::Cyclic3dAxis (int i)
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

void Angle::ApplyGivensWeights
(
double      &aOut,
double      &bOut,
double      a,
double      b,
double      cosine,
double      sine
)
    {
    aOut = a * cosine + b * sine;
    bOut = b * cosine - a * sine;
    }

void Angle::ConstructGivensWeights
(
double      &cosine,
double      &sine,
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
        cosine = 1.0;
        sine   = 0.0;
        }
    else if (fabs (a) > fabs (b))
        {
        t = b / a;
        cosine = 1.0 / sqrt (1.0 + t * t);
        sine   = (cosine) * t;
        }
    else
        {
        t = a / b;
        sine = 1.0 / sqrt (1.0 + t * t);
        cosine   = (sine) * t;
    }
#else
    double scale = fabs (a) + fabs(b);
    if (scale == 0.0)
        {
        cosine = 1.0;
        sine   = 0.0;
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
        sine = b / denom;
        cosine = a / denom;
        }
#endif
    }


/*-----------------------------------------------------------------*//**
* @description Given trig functions (cosine and sine) of some (double) angle 2A, find trig functions for the angle A.
* @remarks Inputs are <EM>not</EM> assumed to be a unit (2D) vector -- first step is to compute r = c^2 + s^2.
* @param pCosA <= cosine of angle
* @param pSinA <= sine of angle
* @param rCos2A => cosine of double angle, possibly scaled by some r
* @param rSin2A => sine of double angle, possibly scaled by same r.
* @group "Trigonometric Rotations"
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTrig_halfAngleFunctions

(
double      *pCosA,
double      *pSinA,
double      rCos2A,
double      rSin2A
)
    {
    double r = sqrt (rCos2A * rCos2A + rSin2A * rSin2A);
    /* If the caller really gave you sine and cosine values, r should be 1.  However,*/
    /* to allow scaled values -- e.g. the x and y components of any vector -- we normalize*/
    /* right here.  This adds an extra sqrt and 2 divides to the whole process, but improves*/
    /* both the usefulness and robustness of the computation.*/
    double cos2A, sin2A;
    if (r == 0.0)
        {
        *pCosA = 1.0;
        *pSinA = 0.0;
        }
    else
        {
        cos2A = rCos2A / r;
        sin2A = rSin2A / r;
        if (cos2A >= 0.0)
            {
            /* Original angle in NE and SE quadrants.  Half angle in same quadrant */
            *pCosA = sqrt (0.5 * ( 1.0 + cos2A ));
            *pSinA = sin2A / (2.0 * (*pCosA));
            }
        else
            {
            if (sin2A > 0.0)
                {
                /* Original angle in NW quadrant. Half angle in NE quadrant */
                *pSinA = sqrt (0.5 * ( 1.0 - cos2A));
                }
            else
                {
                /* Original angle in SW quadrant. Half angle in SE quadrant*/
                /* cosA comes out positive because both sines are negative. */
                *pSinA = - sqrt (0.5 * ( 1.0 - cos2A));
                }
            *pCosA = sin2A / (2.0 * (*pSinA));
            }
        }
    }


/*-----------------------------------------------------------------*//**
* @description Find a vector that differs from (x0,y0) by a multiple of 90 degrees,
*   x1 is positive, and y1 is as small as possible in absolute value, i.e. points to the right.
* @param pX1 <= rotated x component
* @param pY1 <= rotated y component
* @param x0 =>  initial x component
* @param x1 =>  initial y component
* @group "Trigonometric Rotations"
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTrig_rotate90UntilSmallAngle

(
double      *pX1,
double      *pY1,
double      x0,
double      y0
)

    {
    if (fabs (x0) <= fabs (y0))
        {
        if (y0 > 0.0)
            {
            /* Pointing up -- back up 90 degrees */
            *pX1 = y0;
            *pY1 = -x0;
            }
        else
            {
            *pX1 = -y0;
            *pY1 = x0;
            }
        }
    else
        {
        if (x0 < 0.0)
            {
            *pX1 = -x0;
            *pY1 = -y0;
            }
        else
            {
            *pX1 = x0;
            *pY1 = y0;
            }
        }
    }



/*-----------------------------------------------------------------*//**
* Compute the cosine and sine of a rotation matrix which can be used to post-multiply
* a 2-column matrix so that the resulting colums become perpendicular.
* The (2x2) matrix has first row (c,-s), second row (s,c)
*
* @param      pCosine <= cosine of rotation angle
* @param      pSine   <= sine of rotation angle
* @param      pU      => first vector (column of matrix being rotated)
* @param      pV      => second vector (column of matrix being rotated)
* @bsimethod                                                  EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void        bsiTrig_constructOneSided3DJacobiRotation

(
double    *pCosine,
double    *pSine,
DVec3dP     pU,
DVec3dP     pV
)
    {
    double   dotUV, dotUU, dotVV;
    double   ax, ay, a, tol;

    dotUV = pU->DotProduct (*pV);
    dotUU = pU->DotProduct (*pU);
    dotVV = pV->DotProduct (*pV);

    ay = dotUU - dotVV;
    ax = 2.0 * dotUV;
    a = dotUU + dotVV;
    tol = Angle::SmallAngle () * a;
    if (fabs (ax) < tol && fabs (ay) < tol)
        {
        *pCosine = 1.0;
        *pSine   = 0.0;
        }
    else
        {
        bsiTrig_halfAngleFunctions (pCosine, pSine, ay, ax);
        }
    }


/*-----------------------------------------------------------------*//**
* Compute the cosine and sine of a rotation matrix which can be used to post-multiply
* a 2-column matrix so that the resulting colums become perpendicular.
* The (2x2) matrix has first row (c,-s), second row (s,c)
*
* @param      pCosine <= cosine of rotation angle
* @param      pSine   <= sine of rotation angle
* @param      pU      => first vector (column of matrix being rotated)
* @param      pV      => second vector (column of matrix being rotated)
* @bsimethod                                                  EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void        bsiTrig_constructOneSided4DJacobiRotation

(
double    *pCosine,
double    *pSine,
DPoint4dCP pU,
DPoint4dCP pV
)
    {
    double   dotUV, dotUU, dotVV;
    double   ax, ay, a, tol;

    dotUV = pU->DotProduct (*pV);
    dotUU = pU->DotProduct (*pU);
    dotVV = pV->DotProduct (*pV);

    ay = dotUU - dotVV;
    ax = 2.0 * dotUV;
    a = dotUU + dotVV;
    tol = Angle::SmallAngle () * a;
    if (fabs (ax) < tol && fabs (ay) < tol)
        {
        *pCosine = 1.0;
        *pSine   = 0.0;
        }
    else
        {
        bsiTrig_halfAngleFunctions (pCosine, pSine, ay, ax);
#ifdef VERIFY_bsiTrig_constructOneSided4DJacobiRotation
        {
        double r = sqrt (ax * ax + ay * ay);
        double c2 = ay / r;
        double s2 = ax / r;
        double theta = atan2 (*pSine, *pCosine);
        double alpha = atan2 (s2 , c2);
        double mu = 2.0 * theta - alpha;
        DPoint4d U, V;
        double dot;
        U = *pU;
        V = *pV;
        bsiDPoint4d_add2ScaledDPoint4d (&U, NULL, pU, *pCosine, pV, *pSine);
        bsiDPoint4d_add2ScaledDPoint4d (&V, NULL, pU, -*pSine, pV, *pCosine);
        dot = bsiDPoint4d_dotProduct (&U, &V);
        bsiDPoint3d_angleBetweenVectors ((DPoint3d*)pU, (DPoint3d*)pV);
        }
#endif
        }
    }

/*---------------------------------------------------------------------------------**//**
* Normalize an value to a period with specified base value.
* (Typically used with angles, but period is given as an arg so it might be other than 2pi)
*
* @bsimethod                                                    EarlinLutz      08/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiTrig_normalizeToPeriod

(
double a,
double a0,
double da
)
    {
    double a1 = a0 + da;
    double b;
    int ib;
    /* Short circuit the common case where a is already in period.
        (If da is negative, this should always fail.) */
    if (a >= a0 && a < a1)
        {
        /* Already in period.  Leave it alone. */
        return a;
        }
    else if (da < 0.0)
        {
        /* Recurse on negated coordinates */
        return - bsiTrig_normalizeToPeriod (-a, -a0, -da);
        }
    else if (da == 0.0)
        {
        /* Invalid period. Leave a alone. */
        return a;
        }
    else if (a < a0)
        {
        b = a + da;
        /* Quick out for adjacent period ... */
        if (b > a0)
            return b;
        /* Shift and recurse for far regions.  We expect the shift to be right, but
            let the recursion correct it if there is bit-level error. */
        b = (a0 - a) / da;
        ib = 1 + (int)b;
        return bsiTrig_normalizeToPeriod (a + ib * da, a0, da);
        }
    else    /* a >= a1 */
        {
        b = a - da;
        /* Quick out for adjacent period ... */
        if (b < a1)
            return b;
        /* Shift and recurse for far regions.  We expect the shift to be right, but
            let the recursion correct it if there is bit-level error. */
        b = (a - a0) / da;
        ib =(int)b;
        return bsiTrig_normalizeToPeriod (a - ib * da, a0, da);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description normalize to a period centered around zero.  Shift by a multiple
*   of fullPeriod to bring to between negative and positive half period.
* @param a IN periodic value
* @param fullPeriod IN the full period.
* @bsimethod                                                    EarlinLutz      08/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiTrig_normalizeToPeriodAroundZero

(
double a,
double fullPeriod
)
    {
    double halfPeriod;
    if (fullPeriod == 0.0)
        return a;
    halfPeriod = 0.5 * fullPeriod;
    if (fabs (a) < halfPeriod)
        return a;
    return bsiTrig_normalizeToPeriod (a, -halfPeriod, fullPeriod);
    }

static RotMatrix IndefiniteTrigIntegrals (double theta)
    {
    double c = cos (theta);
    double s = sin (theta);
    double s2 = sin (2.0 * theta);
    RotMatrix integrals;
    integrals.form3d[0][0] = 0.5 * (theta + s2);
    integrals.form3d[0][1] = integrals.form3d[1][0] = 0.5 * c * c;
    integrals.form3d[1][1] = 0.5 * (theta - s2);
    integrals.form3d[0][2] = integrals.form3d[2][0] =  s;
    integrals.form3d[1][2] = integrals.form3d[2][1] = -c;
    integrals.form3d[2][2] = theta;
    return integrals;
    }
void Angle::TrigIntegrals (double theta0, double theta1, RotMatrixR integrals)
    {
    integrals = IndefiniteTrigIntegrals (theta1);
    integrals.Subtract (IndefiniteTrigIntegrals (theta0));
    }

static double s_epsilon = 1.0e-12;
double Angle::Cos () const
    {

    if (fabs (m_radians) < 1.0)
        return cos(m_radians);

    double delta = m_radians - M_PI_2;
    if (fabs (delta) < s_epsilon)
        return -sin(delta);

    delta = m_radians + M_PI_2;
    if (fabs (delta) < s_epsilon)
        return sin(delta);

    return cos(m_radians);
    }
double Angle::Sin () const
    {

    if (fabs (m_radians) < 1.0)
        return sin(m_radians);
    double delta = m_radians - PI;
    if (fabs (delta) < s_epsilon)
        return -sin(delta);
    delta = m_radians + PI;
    if (fabs (delta) < s_epsilon)
        return -sin(delta);
    return sin(m_radians);
    }

double Angle::Tan () const {return tan (m_radians);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2015
+--------------------------------------------------------------------------------------*/
bool AngleInDegrees::AlmostEqual (AngleInDegrees const &other)
    {
    return Angle::DegreesToRadians (fabs (m_degrees - other.m_degrees)) < Angle::SmallAngle ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2015
+--------------------------------------------------------------------------------------*/
AngleInDegrees AngleInDegrees::SmallAngleInDegrees () {return msGeomConst_smallAngleDegrees;}

/*--------------------------------------------------------------------------------**//**
* ASSUME angle is positive ....
* (or only modestly negative -- no loss of special case behavior symmetry down to -45 degrees)
* @bsimethod                                                    EarlinLutz      03/2015
+--------------------------------------------------------------------------------------*/
static double CosPositive (double positiveDegrees)
    {
    if (positiveDegrees < 45.0)
        return cos (Angle::DegreesToRadians (positiveDegrees));
    if (positiveDegrees < 135.0)
        return sin (Angle::DegreesToRadians (90.0 - positiveDegrees));
    if (positiveDegrees < 225.0)
        return -cos (Angle::DegreesToRadians (180.0 - positiveDegrees));
    if (positiveDegrees < 315.0)
        return sin (Angle::DegreesToRadians (positiveDegrees - 270.0));
    // do one wraparound explictly (without divide)
    if (positiveDegrees < 720.0)
        return CosPositive (positiveDegrees - 360.0);
    double numWrap = floor (positiveDegrees / 360.0);
    return CosPositive (positiveDegrees - numWrap * 360.0);
    }

/*--------------------------------------------------------------------------------**//**
* ASSUME angle is positive ....
* (or only modestly negative -- no loss of special case behavior symmetry down to -45 degrees)
* @bsimethod                                                    EarlinLutz      03/2015
+--------------------------------------------------------------------------------------*/
static double SinPositive (double positiveDegrees)
    {
    if (positiveDegrees < 45.0)
        return sin (Angle::DegreesToRadians (positiveDegrees));
    if (positiveDegrees < 135.0)
        return cos (Angle::DegreesToRadians (90.0 - positiveDegrees));
    if (positiveDegrees < 225.0)
        return sin (Angle::DegreesToRadians (180.0 - positiveDegrees));
    if (positiveDegrees < 315.0)
        return -cos (Angle::DegreesToRadians (270.0 - positiveDegrees));
    // do one wraparound explictly (without divide)
    if (positiveDegrees < 720.0)
        return SinPositive (positiveDegrees - 360.0);
    double numWrap = floor (positiveDegrees / 360.0);
    return SinPositive (positiveDegrees - numWrap * 360.0);
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2015
+--------------------------------------------------------------------------------------*/
double AngleInDegrees::Cos () const
    {
    // TRIG FACT:    cos(-theta) == cos(theta)
    return CosPositive (fabs (m_degrees));
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2015
+--------------------------------------------------------------------------------------*/
double AngleInDegrees::Sin () const
    {
    // TRIG FACT:    sin(-theta) == -sin(theta)
    if (m_degrees > 0)
        return SinPositive (m_degrees);
    else
        return -SinPositive (-m_degrees);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2015
+--------------------------------------------------------------------------------------*/
AngleInDegrees AngleInDegrees::FromAtan2 (double y, double x)
    {
    double ax = fabs (x);
    double ay = fabs (y);
    // Compute the smallest possible atan2, use as shift from multiple of 90 ...
    if (ax > ay)
        {
        double d = Angle::RadiansToDegrees (atan2 (y,ax));
        if (x > 0.0)
          return d;
        else if (y >= 0.0)
          return AngleInDegrees::FromDegrees (180.0 - d);
        else
          return AngleInDegrees::FromDegrees (-180.0 - d);
        }
    else
        {
        double d = Angle::RadiansToDegrees (atan2 (x,ay));
        if (y >= 0.0)
            return AngleInDegrees::FromDegrees (90.0 - d);
        else
            return AngleInDegrees::FromDegrees (-90.0 + d);
        }
    }

END_BENTLEY_GEOMETRY_NAMESPACE
