/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "msbsplinemaster.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#ifdef CURVATURE_DEBUG
extern "C" int sNoisy;
#endif
/*---------------------------------------------------------------------------------**//**
Compute control points of cubic bezier from point and tangent at each end.
tangents are given as vector and scale factor.
@bsimethod                                    Earlin.Lutz                     11/2005
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    addHermiteCubic
(
DPoint3d *pPoles,
double *pDist0,
double *pDist1,
int *pNumCurve,
bool    bSaveAll,
DPoint3d const * pXYZ0,
DVec3d const *pDir0,
double dist0,
DPoint3d const *pXYZ1,
DVec3d *pDir1,
double dist1
)
    {
    int numCurve = *pNumCurve;
    int i0 = 4 * numCurve;
    if (bSaveAll || (dist0 > 0.0 && dist1 > 0.0))
        {
        pPoles[i0] = *pXYZ0;
        pPoles[i0 + 1].SumOf (*pXYZ0, *pDir0, dist0);
        pPoles[i0 + 2].SumOf (*pXYZ1, *pDir1, dist1);
        pPoles[i0 + 3] = *pXYZ1;

        pDist0[numCurve] = dist0;
        pDist1[numCurve] = dist1;
        *pNumCurve += 1;
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
Compute poles of 0 to 4 cubic bezier curves which interpolate point, tangent direction,
 and curvature at start and end.  The tangent mangitude is adjusted to obtain the
 desired curvature.  Among the multiple curves, the one usually desired is the one
 with two positive tangent magnitudes.
@param pPoles OUT poles of 0 to 4 curves.  (i.e. allocate at least 16 doubles)
@param pDist0 OUT signed start tangent magnitude.
@param pDist1 OUT signed end tangent magnitude.
@param pNumCurve OUT number of curves
@param pXYZ0 IN start point.
@param pDir0 IN start vector in desired direction of curve.
@param r0 IN initial radius of curvature.  Use zero to blend to straight line (infinite radius).
@param pXYZ1 IN end point
@param pDir1 final vector, pointing "back" towards arriving curve.
@param r1 IN final radius of curvature.  Use zero to blend to straight line (infinite radius).
@returns ERROR if conditions cannot be met.  This is not uncommon because the cubic
   has limited flexibility.
@bsimethod                                    Earlin.Lutz                     11/2005
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt mdlBspline_bezierCubicTangentCurvature
(
DPoint3d *pPoles16,
double *pDist0,
double *pDist1,
int *pNumCurve,
DPoint3d const *pXYZ0,
DVec3d   const *pDir0,
double   r0,
DPoint3d const *pXYZ1,
DVec3d const *pDir1,
double r1
)
    {
    DVec3d dir0, dir1, perp0, perp1, zVec;
    StatusInt status = ERROR;
    Transform T0ToWorld, T1ToWorld, TWorldTo0, TWorldTo1;
    DPoint3d xy3, uv0;
    double a = 2.0 / 3.0;
    double q0, q1;
    double idotv, udotj;
    double bb, cc;
    double root[5], qcoff[5];
    int numRoot;
    int numCurve = 0;
    double a0, a1, a2;
    zVec.Init ( 0.0 ,0.0 ,1.0);

    dir0.Normalize (*pDir0);
    dir1.Normalize (*pDir1);

    perp0.CrossProduct (zVec, dir0);
    perp1.CrossProduct (zVec, dir1);

    T0ToWorld.InitFromOriginAndVectors(*pXYZ0, dir0, perp0, zVec);
    T1ToWorld.InitFromOriginAndVectors(*pXYZ1, dir1, perp1, zVec);

    TWorldTo0.InverseOf (T0ToWorld);
    TWorldTo1.InverseOf (T1ToWorld);

    TWorldTo1.Multiply (uv0, *pXYZ0);
    TWorldTo0.Multiply (xy3, *pXYZ1);

    idotv = dir0.DotProduct (perp1);
    udotj = dir1.DotProduct (perp0);

    q0 = a * r0;
    q1 = a * r1;
    if (xy3.y < 0.0)
        q0 = - q0;

    if (uv0.y < 0.0)
        q1 = -q1;

    if (r0 == 0.0 || r1 == 0.0)
        {
        double x1, u2;
        double v0 = uv0.y;
        double y3 = xy3.y;
        if (r0 == 0.0 && r1 == 0.0)
            {
            // This is bad.
            }
        else if (r0 == 0.0)
            {
            u2 = - y3 / udotj;
            x1 = (u2 * u2 - v0 * q1) / (q1 * idotv);
            if (addHermiteCubic (pPoles16, pDist0, pDist1, &numCurve, true, pXYZ0, &dir0, x1, pXYZ1, &dir1, u2))
                status = SUCCESS;
            }
        else if (r1 == 0.0)
            {
            x1 = - v0 / idotv;
            u2 = (x1 * x1 - y3 * q0) / (q0 * udotj);
            if (addHermiteCubic (pPoles16, pDist0, pDist1, &numCurve, true, pXYZ0, &dir0, x1, pXYZ1, &dir1, u2))
                status = SUCCESS;
            }

        }
    else
        {
        bb = q1 * idotv;
        cc = q0 * bb * bb;
        a0 = cc * xy3.y;
        a1 = cc * udotj;
        a2 = q1 * uv0.y;
        // quartic is: a0  + a1 * u = (u^2 - a2)^2
        //   u ^ 4 - 2 a2 u^2 - a1 u + a2^2 - a0 = 0;
        qcoff[0] = a2 * a2 - a0;
        qcoff[1] = -a1;
        qcoff[2] = - 2.0 * a2;
        qcoff[3] = 0.0;
        qcoff[4] = 1.0;
        if (bsiBezier_univariateStandardRoots (root, &numRoot, qcoff, 4))
            {
            int ii;
#ifdef CURVATURE_DEBUG
            printf (" y3=%20.14le    u0=%20.14le\n", xy3.y, uv0.y);
            printf (" r0=%20.14le    r3=%20.14le\n", r0, r1);
            printf (" Coffs: \n");
            for (ii = 0; ii < 5; ii++)
                printf (" (%22.14le)\n", qcoff[ii]);
            printf (" Roots: \n");
            for (ii = 0; ii < numRoot; ii++)
                printf (" (%22.14le)\n", root[ii]);
#endif
            for (ii = 0; ii < numRoot; ii++)
                {
                double u2 = root[ii];
                double u2sq = u2 * u2;
                //double v1 = u2sq / q1;
                //double y2 = xy3.y + u2 * udotj;
                double x1 =  (u2sq - q1 * uv0.y) / bb;
#ifdef CURVATURE_DEBUG
                printf (" (u2,x1) = (%22.14le, %22.14le)\n", u2, x1);
#endif
                if (addHermiteCubic (pPoles16, pDist0, pDist1, &numCurve, true, pXYZ0, &dir0, x1, pXYZ1, &dir1, u2))
                    status = SUCCESS;
                }
            }
        }
    *pNumCurve = numCurve;
    return status;
    }


/*---------------------------------------------------------------------------------**//**
Construct a quartic bezier which interpolates point, tangent direction, tangent magnitude,
   and curvature at both start and end.  These conditions fully specify the middle control
   control point of the bezier.
@param pBezierXYZ OUT 5 poles.
@param pXYZ0 IN start point.
@param pDir0 IN start vector in desired direction of curve.
@param r0 IN initial radius of curvature.  Use zero to blend to straight line (infinite radius).
@param pXYZ1 IN end point
@param pDir1 final vector, pointing "back" towards arriving curve.
@param r1 IN final radius of curvature.  Use zero to blend to straight line (infinite radius).
@returns ERROR if dirctions are parallel.
@bsimethod                                    Earlin.Lutz                     11/2005
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt mdlBspline_bezierQuarticWithTangentAndCurvature
(
DPoint3d *pBezierXYZ,
DPoint3d const *pXYZ0,
DVec3d   const *pTangent0,
double r0,
DPoint3d const *pXYZ1,
DVec3d   const *pTangent1,
double   r1
)
    {
    StatusInt status = ERROR;
    DVec3d dir0, dir1;
    DVec3d perp0, perp1, zVec;
    DVec3d vector01;
    DPoint3d cp0, cp1, cp;
    DRay3d offsetRay0, offsetRay1;
    double b0, b1;  // magnitudes of raw control polygon tangent vectors
    double bsum;
    double a = 3.0 / 4.0;   // constant for y = x / (ar)  relation.
    double q0, q1;  // a * b0, a * b1, with possible sign change.
    double h0, h1;  // Signed altitudes of point0 from final tangent, point1 from initial tangent.
    double e0, e1;
    dir0 = *pTangent0;
    dir1 = *pTangent1;
    b0 = dir0.Normalize ();
    b1 = dir1.Normalize ();
    bsum = b0 + b1;
    zVec.Init ( 0,0,1);
    perp0.CrossProduct (zVec, dir0);
    perp1.CrossProduct (zVec, dir1);
    vector01.DifferenceOf (*pXYZ1, *pXYZ0);
    h1 = perp0.DotProduct (vector01);
    h0 = -perp1.DotProduct (vector01);

    q0 = a * r0;
    if (h0 < 0.0)
        q0 = - q0;

    q1 = a * r1;
    if (h1 < 0.0)
        q1 = -q1;

    DoubleOps::SafeDivide (e0, b0 * b0, q0, 0.0);
    DoubleOps::SafeDivide (e1, b1 * b1, q1, 0.0);

    // Shift each endpoint perpendicular by its offset.
    offsetRay0.origin.SumOf (*pXYZ0, perp0, -e0);
    offsetRay1.origin.SumOf (*pXYZ1, perp1, -e1);
    offsetRay0.direction = dir0;
    offsetRay1.direction = dir1;
    if (bsiDRay3d_closestApproach (NULL, NULL, &cp0, &cp1, &offsetRay0, &offsetRay1))
        {
        cp.Interpolate (cp0, b1 / bsum, cp1);
        pBezierXYZ[0] = *pXYZ0;
        bsiDPoint3d_addDPoint3dDVec3d (&pBezierXYZ[1], pXYZ0, pTangent0);
        pBezierXYZ[2] = cp;
        bsiDPoint3d_addDPoint3dDVec3d (&pBezierXYZ[3], pXYZ1, pTangent1);
        pBezierXYZ[4] = *pXYZ1;
        status = SUCCESS;
        }
    return status;
    }




/*---------------------------------------------------------------------------------**//**
Construct a quartic bezier which has given tangent and curvature at endpoints.  The quartic
uses the explictly given middle control point.  The MAGNTIUDE of the tangents is varied to achieve
curvature control.
@param pBezierXYZ OUT 5 poles.
@param pXYZ0 IN start point.
@param pDir0 IN start vector in desired direction of curve.
@param r0 IN initial radius of curvature.  Use zero to blend to straight line (infinite radius).
@param pXYZ1 IN end point
@param pDir1 final vector, pointing "back" towards arriving curve.
@param r1 IN final radius of curvature.  Use zero to blend to straight line (infinite radius).
@param pControlPoint Explicit middle control point.
@returns ERROR if either radius is zero.
* @bsimethod                                    Earlin.Lutz                     11/2005
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt mdlBspline_bezierQuarticWithTangentCurvatureControlPoint
(
DPoint3d *pBezierXYZ,
DPoint3d const *pXYZ0,
DVec3d   const *pTangent0,  // Start direction, magnitude to be changed to achieve curvature.
double r0,
DPoint3d const *pXYZ1,
DVec3d   const *pTangent1,  // End direction, magnitude to be changed to achieve curvature.
double   r1,
DPoint3d const *pControlPoint   // middle control point.
)
    {
    StatusInt status = ERROR;
    DVec3d dir0, dir1;
    DVec3d perp0, perp1, zVec;
    DVec3d vector0CP, vector1CP;
    double b0, b1;
    double a = 3.0 / 4.0;   // constant for y = x / (ar)  relation.
    double h0, h1;  // Signed altitudes of point0 from final tangent, point1 from initial tangent.
    double e0, e1;  // Computed tangent magnitude.
    dir0 = *pTangent0;
    dir1 = *pTangent1;
    b0 = dir0.Normalize ();
    b1 = dir1.Normalize ();

    zVec.Init ( 0,0,1);
    perp0.CrossProduct (zVec, dir0);
    perp1.CrossProduct (zVec, dir1);
    vector0CP.DifferenceOf (*pControlPoint, *pXYZ0);
    vector1CP.DifferenceOf (*pControlPoint, *pXYZ1);
    h0 = perp0.DotProduct (vector0CP);
    h1 = perp1.DotProduct (vector1CP);

    if (r0 == 0.0 || r1 == 0.0)
        return ERROR;   // We're not dealing with the degnerate cases...
    e0 = sqrt (fabs (a * r0 * h0));
    e1 = sqrt (fabs (a * r1 * h1));

    pBezierXYZ[0] = *pXYZ0;
    pBezierXYZ[1].SumOf (*pXYZ0, dir0, e0);
    pBezierXYZ[2] = *pControlPoint;
    pBezierXYZ[3].SumOf (*pXYZ1, dir1, e1);
    pBezierXYZ[4] = *pXYZ1;

    status = SUCCESS;
    return status;
    }


/*---------------------------------------------------------------------------------**//**
Interpolate a point in a triangle defined by start, tangent vector, and off-tangent direction point.

@param pXYZ0 IN point 0 on triangle.
@param pDir01 IN direction from point 0 to (implicit) point 1.
@param f0 IN First fractional point.  Defines a segment A B.
        B is point interpolated at this fraction from point 0 to point 2.
        A is projection of B on line from point 0 towards point 1 (in the direction pDir01).
@param pXYZ2 IN triangle point 2
@param f1 IN fractional coordinate from A towards B.
* @bsimethod                                    Earlin.Lutz                     11/2005
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    bsiDPoint3d_interpolatePVPRightTriangle
(
DPoint3d *pResult,
DPoint3d const *pXYZ0,
DVec3d   const *pDir01,
double   f0,
DPoint3d const *pXYZ2,
double  fperp
)
    {
    // Fractional coordinate of pXYZ1 projected to ray (pOrigin, pDir0)
    double s;
    DVec3d dir2;
    dir2.DifferenceOf (*pXYZ2, *pXYZ0);
    bool    boolstat = DoubleOps::SafeDivide (s,
                        dir2.DotProduct (*pDir01),
                        pDir01->DotProduct (*pDir01), 0.0);
    bsiDPoint3d_add2ScaledDVec3d (pResult, pXYZ0, pDir01, f0 * (1.0 - fperp) * s, &dir2, f0 * fperp);
    return boolstat;
    }



class CurvatureIterationManager
{
private:
    int mNumPrimaryXYZ;
    DPoint3d const *mpPrimaryXYZ;
    DVec3d mTangentA, mTangentB;
    bool mbCurveDefined;
    MSBsplineCurve mCurve;
    DVec3d mCross[2];

    void PurgeCurve ()
        {
        if (mbCurveDefined)
            bspcurv_freeCurve (&mCurve);
        memset (&mCurve, 0, sizeof (mCurve));
        mbCurveDefined = false;
        }

    void init (double xx[2], double x0, double x1)
        {
        xx[0] = x0;
        xx[1] = x1;
        }

    void addAtTestedIndex (double xx[2], int index, double a)
        {
        if (0 <= index && index < 2)
            xx[index] += a;
        }

    void addScaled (double zz[2], double const xx[2], double yy[2], double a)
        {
        zz[0] = xx[0] + yy[0] * a;
        zz[1] = xx[1] + yy[1] * a;
        }

    void subtract (double zz[2], double const xx[2], double yy[2])
        {
        zz[0] = xx[0] - yy[0];
        zz[1] = xx[1] - yy[1];
        }

public:
    // Constructor to announce pointers to primary data array
    // Caller remains responsible for free.
    CurvatureIterationManager
        (
        DPoint3d const  *pPrimaryXYZ,
        int              numXYZ,
        DVec3d const    *pTangentA,
        DVec3d const    *pTangentB
        )
        {
        mpPrimaryXYZ   = pPrimaryXYZ;
        mNumPrimaryXYZ = numXYZ;
        mbCurveDefined = false;
        mTangentA = *pTangentA;
        mTangentB = *pTangentB;
        memset (&mCurve, 0, sizeof (MSBsplineCurve));
        DVec3d chordA, chordB;
        chordA.DifferenceOf (pPrimaryXYZ[1], pPrimaryXYZ[0]);
        chordB.DifferenceOf (pPrimaryXYZ[numXYZ-2], pPrimaryXYZ[numXYZ-1]);
        mCross[0].CrossProduct (*pTangentA, chordA);
        mCross[1].CrossProduct (*pTangentB, chordB);
        }

    ~CurvatureIterationManager ()
        {
        PurgeCurve ();
        }

    bool GrabCurve (MSBsplineCurve &curve)
        {
        bool bValid = mbCurveDefined;
        if (bValid)
            {
            // hand the curve over and zero our structure ...
            mbCurveDefined = false;
            curve = mCurve;
            memset (&mCurve, 0, sizeof (curve));
            }
        else
            memset (&curve, 0, sizeof (curve));
        return bValid;
        }

    // Given fractional coordinates of "added" point, build expanded array with original start
    // point plus the added points in first and last intervals.
    bool    ComputeExpandedPoints
        (
        DPoint3d *pExpandedXYZ, // Caller must allocated for mNumXYZ+2 points
        double fA0, // tangent fraction (usually 0.5??) at start
        double fA1, // transverse fraction at start.
        double fB0, // tangent fraction (usually 0.5??) at end
        double fB1  // transverse fraction at end
        )
        {
        bool    boolA, boolB = true;
        pExpandedXYZ[0] = mpPrimaryXYZ[0];
        boolA = bsiDPoint3d_interpolatePVPRightTriangle (&pExpandedXYZ[1],
                            &mpPrimaryXYZ[0],
                            &mTangentA, fA0,
                            &mpPrimaryXYZ[1], fA1);

        memcpy (&pExpandedXYZ[2], &mpPrimaryXYZ[1], (mNumPrimaryXYZ - 2) * sizeof (DPoint3d));

        boolB = bsiDPoint3d_interpolatePVPRightTriangle
                            (
                            &pExpandedXYZ[mNumPrimaryXYZ],
                            &mpPrimaryXYZ[mNumPrimaryXYZ - 1],
                            &mTangentB, fB0,
                            &mpPrimaryXYZ[mNumPrimaryXYZ - 2], fB1
                            );
        pExpandedXYZ[mNumPrimaryXYZ+1] = mpPrimaryXYZ[mNumPrimaryXYZ - 1];
        return boolA && boolB;
        }

    bool ComputeCurve (DPoint3d *pExpandedXYZ, int numExpandedPoints)
        {
        PurgeCurve ();
        DPoint3d tangents[2];
        tangents[0] = mTangentA;
        tangents[1] = mTangentB;

        if (SUCCESS == bspcurv_c2CubicInterpolateCurveExt
                    (
                    &mCurve,
                    pExpandedXYZ, NULL, numExpandedPoints,
                    false, 0.0,     // For duplicate point logic?
                    tangents,
                    false,  // closed
                    true,  // chord length knots
                    false,  // colinearTangents
                    false,  // chordLenTangents
                    false   // naturalTangents
                    ))
            {
            mbCurveDefined = true;
            return true;
            }
        return false;
        }

    // Build expanded point array with specificed extra point placement and setup curve.
    // Expanded array is local -- computed poles go into the curve.
    bool ComputeCurve (double u0, double f0, double u1, double f1)
        {
        int numExpandedXYZ = mNumPrimaryXYZ + 2;
        DPoint3d *pExpandedXYZ = (DPoint3d *)_alloca (numExpandedXYZ * sizeof(DPoint3d));
        return ComputeExpandedPoints (pExpandedXYZ, u0, f0, u1, f1)
            && ComputeCurve (pExpandedXYZ, numExpandedXYZ);
        }

    // Evaluate bspline curvature. Orientation of frenet Z gives sign
    bool ComputeStartEndCurvature (double &k0, double &k1)
        {
        k0 = k1 = 0.0;
        if (!mbCurveDefined)
            return false;
        DPoint3d tnbVectors [2][3];
        DPoint3d origin[2];
        double curvature[2];
        double torsion[2];
        double param[2];
        init (param, 0.0, 1.0);

        for (int iEnd = 0; iEnd < 2; iEnd++)
            {
            if (SUCCESS != bspcurv_frenetFrame
                    (
                    &tnbVectors[iEnd][0],
                    &origin[iEnd],
                    &curvature[iEnd],
                    &torsion[iEnd],
                    &mCurve,
                    param[iEnd], NULL)
                    )
                return false;
            double dot = mCross[iEnd].DotProduct (*((DVec3dCP) &tnbVectors[iEnd][2]));
            if (iEnd == 1)
                dot = -dot;
            if (dot < 0.0)
                curvature[iEnd] = - curvature[iEnd];
            }

        k0 = curvature[0];
        k1 = curvature[1];
        return true;
        }

    // Scale each step by its finite difference size.
    // Restrict to cap value (signed)
    void fixStep (double dX[2], double stepSize, double stepCap)
        {
        for (int i = 0; i < 2; i++)
            {
            dX[i] *= stepSize;
            if (fabs (dX[i]) > stepCap)
                dX[i] = dX[i] > 0.0 ? stepCap : -stepCap;
            }
        }

    bool RunIteration
        (
        double r0,
        double r1,
        int maxIterations,
        double stepTol,  // Tolerance, as a fraction (relative tolerance) of the
                        // physical distance from tangent line to first interpolation point.
        double stepCap
        )
        {
        double transverseFraction[2];  // fractional coordinate of transverse points.
        double tangentFraction = 0.5;
        double aMin = 0.0;
        double aMax = 1.0;
        init (transverseFraction, 0.5, 0.5);
        double fractionStep = 1.0e-3;

        double kTarget[2];
        double kCurr[2];
        double rTarget[2];
        double xx[2];

        init (rTarget, r0, r1);

        // r==0 means no curvature -- the divide by zero case is fine.
        DoubleOps::SafeDivide (kTarget[0], 1.0, rTarget[0], 0.0);
        DoubleOps::SafeDivide (kTarget[1], 1.0, rTarget[1], 0.0);

        if (  !ComputeCurve (tangentFraction, transverseFraction[0], tangentFraction, transverseFraction[1]))
            return false;

        for (int iteration = 0; iteration < maxIterations; iteration++)
            {
            double f[3][2];
            double df[2][2];
            double dx[2];

#ifdef CURVATURE_DEBUG
            if (sNoisy)
                printf ("\n ITERATION %d\n", iteration);
#endif
            // Evaluate at primary point, shifted start, and shifed end.
            for (int iEval = 0; iEval < 3; iEval++)
                {
                init (xx, transverseFraction[0], transverseFraction[1]);
                addAtTestedIndex (xx, iEval - 1, fractionStep);

                if (  !ComputeCurve (tangentFraction, xx[0], tangentFraction, xx[1])
                   || !ComputeStartEndCurvature (kCurr[0], kCurr[1]))
                    return false;

                subtract (f[iEval], kCurr, kTarget);
#ifdef CURVATURE_DEBUG
                if (sNoisy)
                    {
                    printf (" iEval = %d\n", iEval);
                    printf ("    kCurr   = (%20.14le %20.14le)\n",  kCurr[0], kCurr[1]);
                    printf ("    kTarget = (%20.14le %20.14le)\n",  kTarget[0], kTarget[1]);
                    }
#endif
                }

            // subtract of primary evaluation to get approximate derivatives
            subtract (df[0], f[1], f[0]);
            subtract (df[1], f[2], f[0]);

#ifdef CURVATURE_DEBUG
            if (sNoisy)
                {
                printf (" J dX = F at X= (%20.14le %20.14le)\n",
                                        transverseFraction[0], transverseFraction[1]);
                printf (" [%20.14le %20.14le] [dx0] = [%20.14le]\n", df[0][0], df[0][1], f[0][0]);
                printf (" [%20.14le %20.14le] [dx1] = [%20.14le]\n", df[1][0], df[1][1], f[1][0]);
                }
#endif
            if (!bsiSVD_solve2x2
                    (
                    &dx[0], &dx[1],
                    df[0][0], df[1][0],
                    df[0][1], df[1][1],     // C indexing is transposed.
                     f[0][0],  f[0][1]
                    ))
                return false;

#ifdef CURVATURE_DEBUG
            if (sNoisy)
                printf (" dX = (%20.14le %20.14le)\n",  dx[0], dx[1]);
#endif
            fixStep (dx, fractionStep, stepCap);
            addScaled (transverseFraction, transverseFraction, dx, -1.0);
            if (transverseFraction[0] < aMin)
                transverseFraction[0] = aMin;
            if (transverseFraction[0] > aMax)
                transverseFraction[0] = aMax;
            if (transverseFraction[1] < aMin)
                transverseFraction[1] = aMin;
            if (transverseFraction[1] > aMax)
                transverseFraction[1] = aMax;

            double stepSize = fabs (dx[0]) + fabs (dx[1]);
            if (stepSize <= stepTol)
                {
                // Converged -- one last evaluation...
                if (  !ComputeCurve (tangentFraction, transverseFraction[0], tangentFraction, transverseFraction[1]))
                    return false;
                return true;
                }
            }   // main loop over iterations


        if (  !ComputeCurve (tangentFraction, transverseFraction[0], tangentFraction, transverseFraction[1]))
            return false;
        return false;
        }
};

/*---------------------------------------------------------------------------------**//**
@description Construct a cubic bspline which interpolates given points with end conditions
@param pCurve OUT computed curve.
@param pXYZ IN points to interpolate
@param numXYZ IN number of interpolation points.
@param pTangent0 IN tangent at start.
@param r0 IN radius of curvature at start
@param pTangent1 IN tangent (towards curve) at end
@param r1 IN radius of curvature at end.
   that have both direction and curvature.  It is not always possible to meet these conditions!!!
@remark Radius values of zero indicate no curvature straight line condition, i.e. infinite radius of curvature.
@returns ERROR if unable to compute curve.   Curvature end conditions are very
   demanding -- be prepared to notice failures!!!
@bsimethod                                    Earlin.Lutz                     11/2005
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt mdlBspline_interpolateCubicWithTangentAndCurvature
(
MSBsplineCurve  *pCurve,
DPoint3d const  *pXYZ,
int             numXYZ,
DVec3d   const  *pTangent0,
double          r0,
DVec3d   const *pTangent1,
double          r1
)
    {
    static double stepCap = 0.1;
    static double convergenceTol = 1.0e-8;
    CurvatureIterationManager manager(pXYZ, numXYZ, pTangent0, pTangent1);
    return manager.RunIteration (r0, r1, 50, convergenceTol, stepCap)
        && manager.GrabCurve (*pCurve) ? SUCCESS : ERROR;
    }
END_BENTLEY_GEOMETRY_NAMESPACE