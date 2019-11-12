/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "mxspline.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

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
    return bspcurv_interpolateXYWithBearingAndRadiusExt
                (
                pCurve,
                pXYZ, numXYZ,
                true, radiansA,
                true, radiusA,
                true, radiansB,
                true, radiusB
                );
    }
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
Public StatusInt bspcurv_interpolateXYWithBearingAndRadiusExt
(
MSBsplineCurve *pCurve,
DPoint3d *pXYZ,
int numXYZ,
bool    bApplyBearingA,
double radiansA,
bool    bApplyRadiusA,
double radiusA,
bool    bApplyBearingB,
double radiansB,
bool    bApplyRadiusB,
double radiusB
)
    {
#define MAX_MX_POINTS 1000

    bvector<double> xArray (numXYZ);
    bvector<double> yArray (numXYZ);

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
    if (bApplyBearingA)
        ec0.SetDirectionRadians (radiansA);
    if (bApplyRadiusA)
        ec0.SetRadiusOfCurvature (radiusA);
    if (bApplyBearingB)
        ec1.SetDirectionRadians (radiansB);
    if (bApplyRadiusB)
        ec1.SetRadiusOfCurvature (radiusB);

    // Create an MX spline.   This will ADD two points to the end intervals
    //  in order to provide flexibility needed for the dual end condition.
    MXSpline2d spline (ec0, ec1);
    if (spline.FitWithArcLengthParameterization (&xArray[0], &yArray[0], numXYZ, 1.0e-6))
        {
        double z;
        z = pXYZ[0].z;

        bvector<DPoint3d> xyzMX;
        bvector<double> arcLengthMX;
        int numMXPoint = 0;
        DVec3d ecDeriv[2];
        double mx, my;
        
        DPoint3d tmpXYZ;
        double   tmpArcLength;
        // Obtain the points and end derivatives from the mx spline ...

        for (int i = 0;
            spline.GetSolutionPoint (i, tmpArcLength, tmpXYZ.x, tmpXYZ.y, mx, my);
            i++)
            {
            tmpXYZ.z = z;
            xyzMX.push_back (tmpXYZ);
            arcLengthMX.push_back (tmpArcLength);
            numMXPoint++;
            }

        spline.EvalDerivativeInInterval (0,          0.0, ecDeriv[0].x, ecDeriv[0].y);
        spline.EvalDerivativeInInterval (numMXPoint - 2, 1.0, ecDeriv[1].x, ecDeriv[1].y);
        ecDeriv[1].x *= -1.0;
        ecDeriv[1].y *= -1.0;
        ecDeriv[0].z = 0.0;
        ecDeriv[1].z = 0.0;
        bvector<DPoint3d> rhs (numMXPoint);

        // Setup yet another tridiagonal system with specified end derivatives ...
        bvector<double> knot01 (numMXPoint);
        double dMax = arcLengthMX[numMXPoint-1];
        TriDiagonalSolver solver (numMXPoint);
        MSBsplineSolver::SetupTridagonalSystem (solver, &arcLengthMX[0], numMXPoint);
        for (int i = 0; i < numMXPoint; i++)
            knot01[i] = arcLengthMX[i] / dMax;

        MSBsplineSolver::SetupRHS_parametricTangents
                (
                &rhs[0], &xyzMX[0], &knot01[0], numMXPoint,
                ecDeriv[0].x * dMax, ecDeriv[0].y * dMax,
                ecDeriv[1].x * dMax, ecDeriv[1].y * dMax
                );
        if (    solver.FactorInPlace ()
            &&  solver.SolveRowMajorInPlace ((double*)(&rhs[0]), numMXPoint, 3))
            {
            // Allocate and populate the MSBspline ...
            pCurve->params.order = 4;
            pCurve->params.numPoles = numMXPoint + 2;
            pCurve->params.numKnots = numMXPoint - 2;
            if (SUCCESS == bspcurv_allocateCurve (pCurve))
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    EarlinLutz                        11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static double CurvatureFromRadius
(
double radius
)
    {
    return radius == 0.0 ? 0.0 : (1.0 / radius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    EarlinLutz                        11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static double RadiusFromCurvature
(
double curvature
)
    {
    return curvature == 0.0 ? 0.0 : (1.0 / curvature);
    }

/*---------------------------------------------------------------------------------**//**
Add PI to bearing if it appears to contradict point direction
* @bsimethod                                    EarlinLutz                        11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static double CorrectBearing
(
DPoint3dCR xyz0,
DPoint3dCR xyz1,
double bearingRadians
)
    {
    DVec3d vector01, bearingVector;
    vector01.DifferenceOf (xyz1, xyz0);
    bearingVector.Init (cos (bearingRadians), sin(bearingRadians), 0.0);
    if (vector01.DotProduct (bearingVector) < 0.0)
        return bearingRadians + msGeomConst_pi;
    return bearingRadians;
    }


#define MAX_SPIRAL_BUFFER_POINTS 2000
/// <summary>
/// Create a bspline curve for specified transition spiral type and
/// start/end bearings, clipped as a fractional range of an (extensible) parent spiral.
/// </summary>
Public StatusInt bspcurv_curveFromDSpiral2dBaseInterval
(
MSBsplineCurveP pCurve,
DSpiral2dBase *pSpiral,
double fractionA,
double fractionB,
DPoint3dP pOrigin,
RotMatrixP pFrame,
double maxStrokeLength
)
    {
    StatusInt status = ERROR;
    static double sMaxRadians = 0.02;
    static bool    bApplyRadius = true;
    bvector<DVec2d> uvPoints;
    bvector<double>fractions;
    bvector<DPoint3d> xyzPoints;

    double errorEstimate;
    if (NULL != pSpiral)
        {
        DVec2d uvA;
        // Stroke from 000 ...
        if (DSpiral2dBase::Stroke (*pSpiral, fractionA, fractionB, sMaxRadians, uvPoints, fractions, errorEstimate, maxStrokeLength)
          && DSpiral2dBase::Stroke (*pSpiral, 0.0, fractionA, sMaxRadians, uvA, errorEstimate, maxStrokeLength)
          )
            {
            for (auto &uv : uvPoints)
                xyzPoints.push_back (DPoint3d::From (uvA.x + uv.x, uvA.y + uv.y, 0.0));
            size_t numXYZ = xyzPoints.size ();
            double distanceA = pSpiral->FractionToDistance (fractionA);
            double distanceB = pSpiral->FractionToDistance (fractionB);
            double localBearingA = pSpiral->DistanceToGlobalAngle (distanceA);
            double localBearingB = pSpiral->DistanceToGlobalAngle (distanceB);
            double curvatureA    = pSpiral->DistanceToCurvature (distanceA);
            double curvatureB    = pSpiral->DistanceToCurvature (distanceB);
            double bearingA = CorrectBearing (xyzPoints[0], xyzPoints[1], localBearingA);
            double bearingB = CorrectBearing (xyzPoints[numXYZ - 2], xyzPoints[numXYZ - 1], localBearingB);
            status = bspcurv_interpolateXYWithBearingAndRadiusExt
                        (
                        pCurve,
                        &xyzPoints[0], (int)numXYZ,
                        true,
                        bearingA,
                        bApplyRadius,
                        RadiusFromCurvature (curvatureA),
                        true,
                        bearingB,
                        bApplyRadius,
                        RadiusFromCurvature (curvatureB)
                        );
            Transform placement;
            placement.InitFrom (*pFrame, *pOrigin);
            bspcurv_transformCurve (pCurve, pCurve, &placement);
            }
        }
    
    return status;
    }

/// <summary>
/// Create a bspline curve for specified transition spiral type and
/// start/end bearings.
/// </summary>
Public StatusInt bspcurv_curveFromDSpiral2dBase
(
MSBsplineCurveP pCurve,
DSpiral2dBase *pSpiral,
DPoint3dP pOrigin,
RotMatrixP pFrame,
double maxStrokeLength
)
    {
    return bspcurv_curveFromDSpiral2dBaseInterval (pCurve, pSpiral, 0.0, 1.0, pOrigin, pFrame, maxStrokeLength);
    }

/// <summary>
/// Create a bspline curve for specified transition spiral type and
/// start/end bearings.
/// </summary>
Public StatusInt bspcurv_curveFromTransitionSpiralBearingAndCurvature
(
MSBsplineCurveP pCurve,
int spiralType,
DPoint3dP pOrigin,
RotMatrixP pFrame,
double startBearingRadians,
double startRadius,
double endBearingRadians,
double endRadius,
double *pExtraParams
)
    {
    StatusInt status = ERROR;
    DSpiral2dBase * pSpiral = NULL;
    memset (pCurve, 0, sizeof (MSBsplineCurve));

    if (spiralType == SPIRALTYPE_TransitionClothoid)
        pSpiral = new DSpiral2dClothoid ();
    else if (spiralType == SPIRALTYPE_TransitionBloss)
        pSpiral = new DSpiral2dBloss ();
    else if (spiralType == SPIRALTYPE_TransitionBiQuadratic)
        pSpiral = new DSpiral2dBiQuadratic ();
    else if (spiralType == SPIRALTYPE_TransitionCosine)
        pSpiral = new DSpiral2dCosine ();
    else if (spiralType == SPIRALTYPE_TransitionSine)
        pSpiral = new DSpiral2dSine ();

    if (NULL != pSpiral)
        {
        pSpiral->SetBearingAndCurvatureLimits (
                    startBearingRadians,
                    CurvatureFromRadius (startRadius),
                    endBearingRadians,
                    CurvatureFromRadius (endRadius));
        status = bspcurv_curveFromDSpiral2dBase (pCurve, pSpiral, pOrigin, pFrame);
        }
    if (pSpiral != NULL)
        delete pSpiral;
    return status;
    }


/// <summary>
/// Create a bspline curve for specified transition spiral type, start bearing, start radius,
///
/// start/end bearings.
/// </summary>
Public StatusInt bspcurv_curveFromTransitionSpiralBearingCurvatureBearingLength
(
MSBsplineCurveP pCurve,
int spiralType,
DPoint3dP pOrigin,
RotMatrixP pFrame,
double startBearingRadians,
double startRadius,
double endBearingRadians,
double length,
double *pExtraParams
)
    {
    memset (pCurve, 0, sizeof (MSBsplineCurve));
    double Q;
    // L = 2 theta / (K1 + K2)
    // K2 unknown ====> K2 = (2 theta /L) - K1
    if (!DoubleOps::SafeDivide (Q, 2 * (endBearingRadians - startBearingRadians), length, 0.0))
        return ERROR;
    double endRadius, endCurvature, startCurvature;
    startCurvature = startRadius == 0.0 ? 0.0 : 1.0 / startRadius;
    endCurvature = Q - startCurvature;
    DoubleOps::SafeDivide (endRadius, 1.0, endCurvature, 0.0);
    return bspcurv_curveFromTransitionSpiralBearingAndCurvature (pCurve, spiralType, pOrigin, pFrame,
                startBearingRadians, startRadius, endBearingRadians, endRadius, pExtraParams);
    }


END_BENTLEY_GEOMETRY_NAMESPACE
