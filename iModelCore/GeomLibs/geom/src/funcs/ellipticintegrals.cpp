/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Namespace for cryptic Elliptic Integrals functions.
See Numerical Recipes for cryptic explanation.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
namespace EllipticIntegrals {

static const double Rf_TINY = 1.5e-38;
static const double Rf_BIG = 3.0e37;
static const double Rf_div3 = 1.0 / 3.0;
static const double Rf_div24 = 1.0 / 24.0;
static const double Rf_div10 = 0.1;
static const double Rf_3div44 = 3.0 / 44.0;
static const double Rf_div14  = 1.0 / 14.0;

static double Rf_ERRTOL = 0.0005;


static const int BadInput = -2;
static const int Success  = 0;

static int Rf_count;
static int Rd_count;
static int Rf (double x, double y, double z, double &result)
    {
    double lambda, ave;
    double delx, dely, delz;
    double e2, e3;
    double sqrtx, sqrty, sqrtz;

    result = 0.0;

    if (x < 0.0 || y < 0.0 || z < 0.0)
        return BadInput;
    if (x + y < Rf_TINY || x + z < Rf_TINY ||  y + z < Rf_TINY)
        return BadInput;
    if (x > Rf_BIG || y > Rf_BIG || z > Rf_BIG)
        return BadInput;

    double xt = x;
    double yt = y;
    double zt = z;
    Rf_count = 0;
    do {
        Rf_count++;
        sqrtx = sqrt (xt);
        sqrty = sqrt (yt);
        sqrtz = sqrt (zt);
        lambda = sqrtx * (sqrty + sqrtz) + sqrty * sqrtz;

        xt = 0.25 * (xt + lambda);
        yt = 0.25 * (yt + lambda);
        zt = 0.25 * (zt + lambda);

        ave = Rf_div3 * (xt + yt + zt);

        delx = (ave - xt) / ave;
        dely = (ave - yt) / ave;
        delz = (ave - zt) / ave;

        } while (  fabs (delx) > Rf_ERRTOL
                || fabs (dely) > Rf_ERRTOL
                || fabs (delz) > Rf_ERRTOL);

    e2 = delx * dely - delz * delz;
    e3 = delx * dely * delz;
    result = (1.0 + (Rf_div24 * e2 - Rf_div10 - Rf_3div44 * e3) * e2 + Rf_div14 * e3) / sqrt (ave);
    return Success;
    }

//static const double Rd_TINY = 1.0e-25;
//static const double Rd_BIG  = 4.5e21;
static const double Rd_3div14 = 3.0 / 14.0;
static const double Rd_div6   = 1.0 / 6.0;
static const double Rd_9div22 = 9.0 / 22.0;
static const double Rd_3div26 = 3.0 / 26.0;
static const double Rd_9div88 = 9.0 / 88.0;
static const double Rd_9div52 = 9.0 / 52.0;
static double Rd_ERRTOL = 0.0010;

static int Rd (double x, double y, double z, double &result)
    {
    double lambda, ave;
    double delx, dely, delz;
    double sqrtx, sqrty, sqrtz;

    result = 0.0;

    if (x < 0.0 || y < 0.0 || z < 0.0)
        return BadInput;
    if (x + y < Rf_TINY || x + z < Rf_TINY ||  y + z < Rf_TINY)
        return BadInput;
    if (x > Rf_BIG || y > Rf_BIG || z > Rf_BIG)
        return BadInput;

    double xt = x;
    double yt = y;
    double zt = z;
    double factor = 1.0;
    double sum    = 0.0;
    Rd_count = 0;
    do {
        Rd_count++;
        sqrtx = sqrt (xt);
        sqrty = sqrt (yt);
        sqrtz = sqrt (zt);
        lambda = sqrtx * (sqrty + sqrtz) + sqrty * sqrtz;

        sum += factor / (sqrtz * (zt + lambda));
        factor *= 0.25;

        xt = 0.25 * (xt + lambda);
        yt = 0.25 * (yt + lambda);
        zt = 0.25 * (zt + lambda);

        ave = 0.2 * (xt + yt + 3.0 * zt);

        delx = (ave - xt) / ave;
        dely = (ave - yt) / ave;
        delz = (ave - zt) / ave;

        } while (  fabs (delx) > Rd_ERRTOL
                || fabs (dely) > Rd_ERRTOL
                || fabs (delz) > Rd_ERRTOL);

    double ea = delx * dely;
    double eb = delz * delz;
    double ec = ea - eb;
    double ed = ea - 6.0 * eb;
    double ee = ed + ec + ec;

    double q1 = ed * (-Rd_3div14 + Rd_9div88 * ed - Rd_9div52 * delz * ee);
    double q2 = delz * (Rd_div6 * ee + delz * (-Rd_9div22 * ec + delz * Rd_3div26 * ea));

    result = 3.0 * sum + factor * (1.0 + q1 + q2) / (ave * sqrt (ave));
    return Success;
    }

static int Ephik (double phi, double k, double &result)
    {
    double c = cos (phi);
    double cc = c * c;
    double s = sin (phi);
    double sk = s * k;
    double q = (1.0 + sk) * (1.0 - sk);
    double rf, rd;

    result = 0.0;
    if (BadInput == Rf (cc, q, 1.0, rf))
        return BadInput;
    if (BadInput == Rd (cc, q, 1.0, rd))
        return BadInput;
    result = s * (rf - sk * sk * rd / 3.0);
    return Success;
    }
// Axis and quadrant logic for ellipse arc length integrations.
class ArcLengthContext
{
private:
    double a, b, mu, k;
    double alpha0, alpha1;
    double piOver2;


    void accumulateSweepFromZero
        (
        double phi,
        int direction, // plus or minus 1.
        int &numQuadrant,
        double &residual
        )
        {
        // Count full quadrants ....
        int currCallQuadrants = 0;
        for (;phi >= piOver2;)
            {
            numQuadrant += direction;
            currCallQuadrants += 1;
            phi -= piOver2;
            }

        if (phi > 0.0)
            {
            double aa = 0.0;
            double s = (double)direction;
            if (currCallQuadrants & 0x01) //
                {
                Ephik (piOver2 - phi, k, aa);
                residual -= s * aa;
                numQuadrant += direction;
                }
            else
                {
                Ephik (phi, k, aa);
                residual += s * aa;
                }
            }
        }
public:
    ArcLengthContext ()
        {
        piOver2 = 2.0 * atan (1.0);
        }
    // Setup for integral of sqrt (b^2 cos ^2 + a^2 sin^2)
    // Be careful about order of a and b!!!
    bool Setup (double bb, double aa, double startRadians, double sweepRadians)
        {

        if (aa == 0.0 && bb == 0.0)
            return false;

        aa = fabs (aa);
        bb = fabs (bb);
        if (sweepRadians < 0.0)
            {
            startRadians = - startRadians;
            sweepRadians = - sweepRadians;
            }

        if (aa >= bb)
            {
            a = aa; b = bb; mu = bb / aa;
            alpha0 = startRadians;
            }
        else
            {
            a = bb; b = aa; mu = aa / bb;
            alpha0 = startRadians - piOver2;
            }
        alpha1 = alpha0 + sweepRadians;
        k = sqrt (1 - mu * mu);
        return true;
        }

    // 0 <= mu <= 1
    // k is computed
    // alpha0 <= alpha1
    double go ()
        {
        double residual = 0.0;
        int numQuadrant = 0;
        if (alpha1 < 0.0)
            {
            accumulateSweepFromZero (-alpha0,  1, numQuadrant, residual);
            accumulateSweepFromZero (-alpha1, -1, numQuadrant, residual);
            }
        else if (alpha0 < 0.0)
            {
            accumulateSweepFromZero (-alpha0,  1, numQuadrant, residual);
            accumulateSweepFromZero ( alpha1,  1, numQuadrant, residual);
            }
        else
            {
            accumulateSweepFromZero (alpha0, -1, numQuadrant, residual);
            accumulateSweepFromZero (alpha1,  1, numQuadrant, residual);
            }
        double result = residual;
        if (numQuadrant != 0)
            {
            double q;
            Ephik (piOver2, k, q);
            result += q * numQuadrant;
            }
        return a * result;
        }

};

}; // namespace EllipticIntegrals

/*---------------------------------------------------------------------------------**//**
@description Return arc length of ellipse given in major/minor axis form.
@param a IN major axis of ellipse.
@param b IN minor axis of ellipse.
@param startRadians IN start of arc.
@param sweepRadians IN sweep angle of arc.
@return arc length
@group "Elliptic Integrals"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiGeom_ellipseArcLength

(
double a,
double b,
double startRadians,
double sweepRadians
)
    {
    EllipticIntegrals::ArcLengthContext ac = EllipticIntegrals::ArcLengthContext ();
    if (ac.Setup (a, b, startRadians, sweepRadians))
        {
        return ac.go();
        }
    return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
@description Return sweep angle which travels specified arc length of ellipse given in major/minor axis form.
@param a IN major axis of ellipse
@param b IN minor axis of ellipse
@param startRadians IN start of arc
@param distance IN arc length to travel
@return sweep angle
@group "Elliptic Integrals"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiGeom_ellipseInverseArcLength

(
double a,
double b,
double startRadians,
double distance
)
    {
    double period = 4.0 * atan (1.0);

    // We will search for 12 digits out of 16 possible.
    // For usual Newton convergence, that last step will be full precision.
    //    However we very definitely do not want to test for full precision, because the
    //    underlying arc length function is itself an approximation and might not behave as
    //    smoothly as we would like in the limit.
    // So we will say that reaching tolerance twice in a row is the goal...
    double angleTol = 1.0e-12;
    int    targetNumConverged = 2;

    if (distance == 0.0)
        return 0.0;
    if (distance < 0.0)
        return -bsiGeom_ellipseInverseArcLength (a, b, -startRadians, -distance);
    // Subtract off quarter-ellipse steps until close ....
    double periodLength = bsiGeom_ellipseArcLength (a, b, 0.0, period);
    double dNumperiod;

    if (!DoubleOps::SafeDivide (dNumperiod, distance, periodLength, 0.0))
        return 0.0;
    int numperiod = (int)floor (dNumperiod);

    double periodStep = numperiod * periodLength;
    double residualDistance = distance - periodStep;
    double residualStart = startRadians + numperiod * period;
    // Estimate remaining sweep as fraction of period...
    double residualSweep = period * residualDistance / periodLength;

    int numNewton = 0;
    static int sMaxNewton = 10;
    double aa = a * a;
    double bb = b * b;
    int numConverged = 0;
    while (numNewton++ < sMaxNewton)
        {
        double f   = bsiGeom_ellipseArcLength (a, b, residualStart, residualSweep) - residualDistance;
        // Ellipse tangent magnitiude at endpoint is
        double residualEnd = residualStart + residualSweep;
        double co = cos (residualEnd);
        double si = sin (residualEnd);
        double df  = sqrt (aa * si * si + bb * co * co);
        double dTheta;
        if (!DoubleOps::SafeDivide (dTheta, f, df, 0.0))
            return 0.0;
        residualSweep -= dTheta;
        if (fabs (dTheta) < angleTol)
            {
            numConverged++;
            if (numConverged >= targetNumConverged)
                return numperiod * period + residualSweep;
            }
        else
            {
            numConverged = 0;
            }
        }
    // hmm .. Newton got lost.
    return periodStep + residualSweep;
    }


/*---------------------------------------------------------------------------------**//**
@description Return radius of curvature on simple major/minor ellipse.
@param a IN major axis of ellipse
@param b IN minor axis of ellipse
@param radians IN angular parameter for evaluation.
@return radius of curvature
@group "Elliptic Integrals"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiGeom_ellipseRadiusOfCurvature

(
double a,
double b,
double radians
)
    {
    double tx  = - a * sin (radians);
    double ty  =   b * cos (radians);
    double magT = sqrt (tx * tx + ty * ty);
    return magT * magT * magT / (a * b);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static long double lfabs(long double d)
    {
    return ((d < 0.0L) ? -d : d);
    }

/*---------------------------------------------------------------------------------**//**
@description Return parameter angle at which a major/minor ellipse has specified radius of curvature.
@param pRadians OUT first quadrant parameter angle.
@param a IN major axis of ellipse
@param b IN minor axis of ellipse
@param rho IN target radius.  The valid range is between a^2/b and b^2/a.
@return true if the radius of curvature is valid.
@group "Elliptic Integrals"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiGeom_ellipseInverseRadiusOfCurvature (double *pRadians, double a, double b, double rho)
    {
    a = fabs (a);
    b = fabs (b);
    if (a < b)
        {
        double beta;
        bool    bStat = bsiGeom_ellipseInverseRadiusOfCurvature (&beta, b, a, rho);
        *pRadians = 2.0 * atan (1.0) - beta;
        return bStat;
        }
    //long double aa = a * a;
    //long double bb = b * b;
    rho = fabs (rho);
    // Use long doubles for cube root and subtractions with touchy roundoffs
    // But alas, modest ratios of a/b still get only 8 digits back
    //     if angle is near 0 or 90.
    long double rhoab = rho * a * b;
    long double qrt = pow (rhoab, 1.0L / 3.0L);
    long double q = qrt * qrt;
    long double ea = q - a * a;
    long double eb = q - b * b;
    long double f = b * b - a * a;
    long double af = lfabs (f);

    *pRadians = (double) atan2 (sqrt(lfabs (eb)), sqrt (lfabs (ea)));
    return ea * f >= 0.0 && eb * f <= 0.0
        && lfabs (ea) <= af && lfabs (eb) <= af;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
