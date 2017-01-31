/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/build/bsibasegeom/test/test15/bsp_mxspline.cpp $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*==========================================================================+
|                                                                                                                                                       |
|               Private includes                                                                                |
|                                                                                                                                                       |
+==========================================================================*/
#include <msgeomstructs.hpp>

// toolsubs.h is ONLY used to get proper printf in microstation envrionment.
#include <toolsubs.h>
#include        <mdl.h>
#include        <msbspline/mibsplin.fdf>
#include        <msbsplin.fdf>
#include <mxspline.h>
#include <bsp_mxspline.fdf>
// Incidental spline calculations ....
class MSBsplineSolver
{
public:
/*----------------------------------------------------------------------+
|                                                                       |
| name          computeAlphaBetaGamma                                   |
|                                                                       |
| author        LuHan                                   3/95            |
|                                                                       |
+----------------------------------------------------------------------*/
static void    computeAlphaBetaGamma
(
double          *alpha,
double          *beta,
double          *gamma,
double          deltaIp1,
double          deltaI,
double          deltaIm1,
double          deltaIm2
)
    {
    double      deno;

    deno = deltaIm2 + deltaIm1 + deltaI;
    *alpha = deltaI * deltaI / deno;
    *beta = deltaI * (deltaIm2 + deltaIm1) / deno;
    deno = deltaIm1 + deltaI + deltaIp1;
    *beta += deltaIm1 * (deltaI + deltaIp1) / deno;
    *gamma = deltaIm1 * deltaIm1 / deno;

    deno = deltaIm1 + deltaI;
    *alpha /= deno;
    *beta /= deno;
    *gamma /= deno;
    }

/*========================================================================
Setup the tridiagonal system for known-tangent end condition.
Row i is the matching condition for original data point i,
  except that for row 1 the artifical point goes on the right (instead of
  an interpolated point)
========================================================================*/
static void SetupTridagonalSystem
(
TriDiagonalSolver &solver,
// knot vector, without duplicates at ends.
double *knots,
// Number of interpolation points.
int numPoint
)
    {
    int num1 = numPoint - 2;
    double deltaIm2, deltaIm1, deltaI, deltaIp1;
    double aLeft, aDiag, aRight;
    // First, last equations are direct assignments (and they preserve the tridiagonal form)
    solver.SetRow (0, 0.0, 1.0, 0.0);
    solver.SetRow (num1 + 1, 0.0, 1.0, 0.0);

    // Rows 1 and numPoint - 2 would be just like those in between
    // if the "full" knot vector was provided -- the duplicate knots
    // at the end would generate the 0.0 deltas.
    /* i = 1 */
    deltaI   = knots[2]-knots[1];
    deltaIm1 = knots[1]-knots[0];
    deltaIm2 = 0.0;
    deltaIp1 = knots[3]-knots[2];
    computeAlphaBetaGamma (&aLeft, &aDiag, &aRight,
                            deltaIp1, deltaI, deltaIm1, deltaIm2);
    solver.SetRow (1, aLeft, aDiag, aRight);

    /* i = L-1 = num1 */
    deltaI   = knots[num1+1] - knots[num1];
    deltaIm1 = knots[num1]   - knots[num1-1];
    deltaIm2 = knots[num1-1] - knots[num1-2];
    deltaIp1 = 0.0;
    computeAlphaBetaGamma (&aLeft, &aDiag, &aRight,
                            deltaIp1, deltaI, deltaIm1, deltaIm2);
    solver.SetRow (num1, aLeft, aDiag, aRight);


    /* Now for the main loop: */
    for (int i = 2; i < num1; i++)
        {
        deltaI   = knots[i+1] - knots[i];
        deltaIm2 = knots[i-1] - knots[i-2];
        deltaIm1 = knots[i]   - knots[i-1];
        deltaIp1 = knots[i+2] - knots[i+1];
        computeAlphaBetaGamma (&aLeft, &aDiag, &aRight,
                                deltaIp1, deltaI, deltaIm1, deltaIm2);
        solver.SetRow (i, aLeft, aDiag, aRight);
        }
    }

static void SetupRHS_parametricTangents
(
DPoint3d *pRHS,
DPoint3d *pXYZArray,
double   *pParams,
int     numPoint,
// x derivative (wrt arc length) at first point.
double  dx0,
// y derivative (wrt arc length) at first point.
double  dy0,
// BACKWARDS derivatives at end
double  dx1,
double  dy1
)
    {
    for (int i = 0; i < numPoint; i++)
        {
        pRHS[i] = pXYZArray[i];
        }

    double a0 = (pParams[1] - pParams[0]) / 3.0;
    pRHS[0].x += a0 * dx0;
    pRHS[0].y += a0 * dy0;
    double a1 = (pParams[numPoint - 1] - pParams[numPoint - 2]) / 3.0;
    pRHS[numPoint - 1].x += a1 * dx1;
    pRHS[numPoint - 1].y += a1 * dy1;
    }


};


/*-----------------------------------------------------------------
@description Initialize an MSBspline using MX-style conditions, i.e.
(a) both bearing and curvature at ends
(b) knots spaced to match arc length.
@param pCurve OUT curve
@param pXYZ IN points to interpolate.  All spline z values are z from the first point.
@param numXYZ IN number of points.
@param radiansA IN start point bearing (into curve) in radians.
@param radiusA IN start point radius of curvature.  Positive is to left of curve.
             zero radius means straight line.
@param radiansA IN end point bearing (outbounds from curve!!!) in radians.
@param radiusA IN end point radius of curvature.  Positive is to left of curve.
             zero radius means straight line.
@returns ERROR if unable to fit.
-------------------------------------------------------------------*/
Public StatusInt bspcurv_interpolateXYWithBearingAndRadius
(
MSBsplineCurve *pCurve,
DPoint3d *pXYZ,
int numXYZ,
double radiansA,
double radiusA,
double radiansB,
double radiusB
)
    {
#define MAX_MX_POINTS 1000

    double xArray[MAX_MX_POINTS];
    double yArray[MAX_MX_POINTS];
    MSElementDescr *pResultDescr = NULL;

    memset (pCurve, 0, sizeof (MSBsplineCurve));
    if (numXYZ + 2 > MAX_MX_POINTS)
        return ERROR;

    // Copy to flat arrays ..
    for (int k = 0; k < numXYZ; k++)
        {
        xArray[k] = pXYZ[k].x;
        yArray[k] = pXYZ[k].y;
        }
    // Create end condition objects ...
    EndCondition2d ec0, ec1;
    ec0.SetDirectionRadians (radiansA);
    ec0.SetRadiusOfCurvature (radiusA);
    ec1.SetDirectionRadians (radiansB);
    ec1.SetRadiusOfCurvature (radiusB);

    // Create an MX spline.   This will ADD two points to the end intervals
    //  in order to provide flexibility needed for the dual end condition.
    MXSpline2d spline (ec0, ec1);
    if (spline.FitWithArcLengthParameterization (xArray, yArray, numXYZ, 1.0e-6))
        {
        double z;
        z = pXYZ[0].z;

        DPoint3d xyzMX[MAX_MX_POINTS];
        double   arcLengthMX[MAX_MX_POINTS];
        int numMXPoint = 0;
        DVec3d ecDeriv[2];
        double mx, my;
        // Obtain the points and end derivatives from the mx spline ...

        for (int i = 0;
            spline.GetSolutionPoint (i, arcLengthMX[numMXPoint], xyzMX[numMXPoint].x, xyzMX[numMXPoint].y, mx, my);
            i++)
            {
            xyzMX[numMXPoint].z = z;
            numMXPoint++;
            }

        spline.EvalDerivativeInInterval (0,          0.0, ecDeriv[0].x, ecDeriv[0].y);
        spline.EvalDerivativeInInterval (numMXPoint - 2, 1.0, ecDeriv[1].x, ecDeriv[1].y);
        ecDeriv[1].x *= -1.0;
        ecDeriv[1].y *= -1.0;
        ecDeriv[0].z = 0.0;
        ecDeriv[1].z = 0.0;
        DPoint3d rhs[MAX_MX_POINTS];


        // Setup yet another tridiagonal system with specified end derivatives ...
        double   knot01[MAX_MX_POINTS];
        double dMax = arcLengthMX[numMXPoint-1];
        TriDiagonalSolver solver (numMXPoint);
        MSBsplineSolver::SetupTridagonalSystem (solver, arcLengthMX, numMXPoint);
        for (int i = 0; i < numMXPoint; i++)
            knot01[i] = arcLengthMX[i] / dMax;

        MSBsplineSolver::SetupRHS_parametricTangents
                (
                rhs, xyzMX, knot01, numMXPoint,
                ecDeriv[0].x * dMax, ecDeriv[0].y * dMax,
                ecDeriv[1].x * dMax, ecDeriv[1].y * dMax
                );
        if (    solver.FactorInPlace ()
            &&  solver.SolveRowMajorInPlace ((double*)rhs, numMXPoint, 3))
            {
            // Allocate and populate the MSBspline ...
            pCurve->params.order = 4;
            pCurve->params.numPoles = numMXPoint + 2;
            pCurve->params.numKnots = numMXPoint - 2;
            if (SUCCESS == mdlBspline_allocateCurve (pCurve))
                {
                int m = 0;
                pCurve->poles[m++] = xyzMX[0];
                for (int k = 0; k < numMXPoint; k++)
                    pCurve->poles[m++] = rhs[k];
                pCurve->poles[m++] = xyzMX[numMXPoint - 1];

                // As is well known, a cubic MSBsplineCurve
                //   has 4 leading 0 knots, then the interior values,
                //   4 trailing ones.

                m = 0;
                pCurve->knots[m++] = 0.0;
                pCurve->knots[m++] = 0.0;
                pCurve->knots[m++] = 0.0;
                pCurve->knots[m++] = 0.0;
                for (int k = 1; k < numMXPoint - 1; k++)
                    {
                    pCurve->knots[m++] = knot01[k];
                    }
                pCurve->knots[m++] = 1.0; //
                pCurve->knots[m++] = 1.0;
                pCurve->knots[m++] = 1.0;
                pCurve->knots[m++] = 1.0;
                pCurve->display.curveDisplay = 1;
                return SUCCESS;
                }
            }
        }
    return ERROR;
    }

static double applyBounds
(
double a,
double aMin,
double aMax
)
    {
    if (a < aMin)
        a = aMin;
    if (a > aMax)
        a = aMax;
    return a;
    }

// Construct a cubic approximation to an arc.
// For positive radius, the arc turns to the left (and the arc is to the right of the simple segment)
// For negative radius, the arc turns to the right (and the arc is to the left of the simple segment)
// For "larger arc" the arc takes the longer path.
Public StatusInt bspcurv_approximateXYArc
(
MSBsplineCurve *pCurve,
DPoint3d *pXYZStart,
DPoint3d *pXYZEnd,
double   radius,
bool     bLargerArc,
double   maxRadiansBetweenPassthroughPoints
)
    {
    static double sdThetaMin = 0.15;
    double dThetaMax = atan (1.0);

    double angleStep = applyBounds (maxRadiansBetweenPassthroughPoints, sdThetaMin, dThetaMax);

    DPoint3d midpoint, center;
    midpoint.interpolate (pXYZStart, 0.5, pXYZEnd);
    DVec3d uDir, vDir;
    uDir.normalizedDifference (pXYZEnd, &midpoint);
    vDir.unitPerpendicularXY (&uDir);
    if (radius < 0.0)
        vDir.negate ();
    double a = midpoint.distance (pXYZEnd);
    double r = fabs (radius);

    memset (pCurve, 0, sizeof (MSBsplineCurve));
    if (a > r)
        return ERROR;
    double b = sqrt (r * r - a * a);
    if (bLargerArc)
        b = - b;
    double alpha = atan2 (a, b);
    double sweep = 2.0 * alpha;
    center.sumOf (&midpoint, &vDir, b);

    double aNumStep = floor(0.999999 + fabs (sweep) / angleStep);
    if (aNumStep < 1.0)
        aNumStep = 1.0;
    int numPoint = 1 + (int)aNumStep;

#define MAX_ARC_POINT 500
    if (numPoint > MAX_ARC_POINT)
        numPoint = MAX_ARC_POINT;
    double dTheta = sweep / (double)(numPoint - 1);

    DPoint3d xyz[MAX_ARC_POINT];
    xyz[0] = *pXYZStart;
    xyz[numPoint - 1] = *pXYZEnd;
    for (int i = 1; i < numPoint - 1; i++)
        {
        double theta = - alpha + i * dTheta;
        xyz[i].sumOf (&center, &uDir, radius * cos(theta), &vDir, radius * sin (theta));
        }
    return ERROR;
    }