/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <Geom/BSIQuadrature.h>


/*-----------------------------------------------------------------*//**
@description Coordinate data for a bounded interval of a clothoid curve.
The curve is defined as
<list>
<item> x(t1) = x(t0) + integral [t0..t1] dt cos(alpha + t*t/(2C) </item>
<item> y(t1) = y(t0) + integral [t0..t1] dt sin(alpha + t*t/(2C) </item>
<list>
The meaning of C, alpha, t0, t1 are:
<item>C -- the DENOMINATOR constant inside the trig functions.
    The square root of mC is a characteristic length of the clothoid
</item>
<item>alpha -- the angle from the global axis to the clothoid tangent at its inflection point.</item>
<item>t0 -- distance along curve from inflection point to start point of the
        bounded interval.</item>
<item>t1 -- distance along curve from inflection point to the end point of the
        bounded interval.
</list>
This structure is expected to be used in a context in which either (a) the inflection point itself
or (b) the point at t0 are specified.  All calculations in the DCLothoid2d
produce vector displacments that the caller will add to its own reference point.

+---------------+---------------+---------------+---------------+------*/

struct DClothoid2d : BSIVectorIntegrand
{
private:
    double   mC;
    double   mAlpha;
    double   mt0, mt1;

public:
DClothoid2d (double alpha, double A)
    : mC(A), mAlpha(alpha)
    {
    mt0 = 0.0;
    mt1 = 1.0;
    }

// methods for the VectorIntegrand interface ...

int  GetVectorIntegrandCount () override { return 2;}
void EvaluateVectorIntegrand (double t, double *pF) override
    {
    double beta = mAlpha + t * t  / (2.0 * mC);
    pF[0] = cos (beta);
    pF[1] = sin (beta);
    }

/*-----------------------------------------------------------------*//**
@description Evaluate tangent direction, measured from global x axis.
@param t IN position on curve, specified as distance from inflection point.
@returns heading angle, in radians, measured from the global x axis.
+---------------+---------------+---------------+---------------+------*/
double GlobalHeadingRadiansAtDistance (double t)
    {
    return mAlpha + t * t / (2.0 * mC);
    }

/*-----------------------------------------------------------------*//**
@description Evaluate tangent direction, measured from the clothoid's reference axis.
@param t IN position on curve, specified as distance from inflection point.
@returns heading angle, in radians, measured from the local axis.
+---------------+---------------+---------------+---------------+------*/
double LocalHeadingRadiansAtDistance (double t)
    {
    return t * t / (2.0 * mC);
    }

/*-----------------------------------------------------------------*//**
@description Determine the distance along the curve from the inflection point
        to the point with accumulated turn angle beta.
@param beta IN accumuated turn nagle.
@returns distance along curve.
+---------------+---------------+---------------+---------------+------*/
double DistanceAtLocalHeadingRadians (double beta)
    {
    return sqrt (beta * 2.0 * mC);
    }

/// Compute C, alpha, t0, t1 from start and end curvature and heading.
bool InvertParams
(
double curvature0,
double curvature1,
double theta0,
double theta1
)
    {
    double k02 = curvature0 * curvature0;
    double k12 = curvature1 * curvature1;
    double dk2 = k12 - k02;
    if (dk2 == 0.0)
        {
        mt0 = mt1 = 0.0;
        mAlpha = 0.0;
        mC     = 0.0;
        return false;
        }
    else
        {
        mC = 2.0 * (theta1 - theta0) / dk2;
        mAlpha = (k12 * theta0 - k02 * theta1) / dk2;
        mt0 = mC * curvature0;
        mt1 = mC * curvature1;
        }
    }
};

/*-----------------------------------------------------------------*//**
@description Integrate the vector displacements of a clothoid from a starting arc length
 to multiple points on the clothoid up to an ending arc length.
 This uses the angle and scale factor of the clothoid.  The t0,t1 distances
of the parameter list override those stored in the clothoid.
@param t0 IN starting distance.
@param t1 IN end distance.
@param maxRadians IN maximum bearing change between computed points.
        Recommended value is 0.08.
@param numGauss IN number of points in gauss rule. Recommended value is 4.
     In each interval,
    the rule is applied to the entire interval and then to left and right
    halves.  The two results are extrapolated with Romberg extrapolation,
    which also provides the error estimate.
@param pDXY INOUT buffer to receive points.
@param numDXY OUT number of points computed.
@param maxDXY IN maximum number of points to compute.
@param errorBound OUT estimated bound on error.
@returns false if point buffer exceeded.
+---------------+---------------+---------------+---------------+------*/
bool bsiDClothoid2d_integrateBetweenArcLengthPositions
(
DClothoid2d &clothoid,
double t0,
double t1,
double maxRadians,
int    numGauss,
DVec2d *pDXY,
int    &numDXY,
int    maxDXY,
double &errorBound
)
    {
    static double sMaxRadians = atan (1.0);
    static double sDefaultRadians = sMaxRadians * 0.25;
    BSIQuadraturePoints gauss;
    gauss.InitGauss (numGauss);
    double beta0 = clothoid.LocalHeadingRadiansAtDistance (t0);
    double beta1 = clothoid.LocalHeadingRadiansAtDistance (t1);
    if (maxRadians <= 0.0)
        maxRadians = sDefaultRadians;
    else if (maxRadians > sMaxRadians)
        maxRadians = maxRadians;

    double rombergFactor = 1.0 / (pow (2.0, gauss.GetConvergencePower ()) - 1.0);
    int numInterval = (int) (0.9999999999 + fabs (beta1 - beta0) / maxRadians);
    DVec2d dxy;
    dxy.zero ();
    errorBound = 0.0;
    double dGamma = (beta1 - beta0) / numInterval;
    numDXY = 0;
    for (int interval = 0; interval < numInterval; interval++)
        {
        double gammaA = beta0 + interval * dGamma;
        double gammaB = beta0 + (interval + 1) * dGamma;
        double tA     = clothoid.DistanceAtLocalHeadingRadians (gammaA);
        double tB     = clothoid.DistanceAtLocalHeadingRadians (gammaB);
        DVec2d result1, result2;
        result1.zero ();
        result2.zero ();
        gauss.AccumulateWeightedSums (clothoid, tA, tB, (double*)&result1, 1);
        gauss.AccumulateWeightedSums (clothoid, tA, tB, (double*)&result2, 2);
        DVec2d dr;
        dr.differenceOf (&result2, &result1);
        // Richardson says we can extrapolate by a small fraction of the step ...
        dxy.sumOf (&dxy, &result2, 1.0, &dr, rombergFactor);
        if (numDXY < maxDXY)
            {
            pDXY[numDXY] = dxy;
            numDXY += 1;
            }
        else
            return false;
        // and the update is an overestimate of the error ...
        errorBound += rombergFactor * dr.maxAbs ();
        }
    return true;
    }
