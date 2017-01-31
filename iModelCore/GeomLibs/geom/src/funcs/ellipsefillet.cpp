/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/funcs/ellipsefillet.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include    <Geom/bezeval.fdf>
#include    <Geom/bezroot.fdf>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static bool proportionalBeziers
(
double f[],
double g[],
int order,
double relTol,
double &fOverG
)
    {
    double gMax = fabs (g[0]);
    int    iMax = 0;
    fOverG = 1.0;
    for (int i = 1; i < order; i++)
        {
        double a = fabs (g[i]);
        if (a > gMax)
            {
            gMax = a;
            iMax = i;
            }
        }

    if (gMax <= 0.0)
        return false;
    fOverG = f[iMax] / g[iMax];
    double tol = f[iMax] * relTol;
    for (int i = 0; i < order; i++)
        {
        if (fabs (g[i] * fOverG - f[i]) > tol)
            return false;
        }
    return true;
    }

// Test if all f coefficients are small relative to the largest coefficient in g;
static bool zeroBezier
(
double f[],     // zero candidate
int    orderF,  // order of f
double g[],     // reference coefficients.
int orderG,      // order of g
double absTol,
double relTol
)
    {

    double fMax = bsiDoubleArray_maxAbs (f, orderF);
    double gMax = bsiDoubleArray_maxAbs (g, orderG);
    return fMax < absTol + relTol * gMax;
    }

/*---------------------------------------------------------------------------------**//**
Compute intersections of the (signed, possibly different) offsets of an ellipse and a plane.
@param pXYZ OUT array (ALLOCATED BY CALLER, OPTIONAL) of intersection points.
@param pTrigPoint OUT array (ALLOCATED BY CALLER, OPTIONAL) with x,y,z carrying
            cosine, sine, and angle of the point on the ellipse.
@param pNumXYZ OUT number of intersection points.
@param maxXYZ IN max number of points to place in pXYZ.  Note that returned count may exceed this number --
    all intersections are computed and counted, but extras are not returned.
@param pEllipse IN ellipse to offset.
@param ellipseOffset IN signed offset of the ellipse.  The positive offset direction is defined in the
       in the right handed system of the ellipse axes.
@param pPlane IN base plane
@param planeOffset IN signed offset from plane.
* @bsimethod                                                    EarlinLutz      04/07
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiGeom_intersectOffsetEllipseOffsetPlane
(
DPoint3dP pXYZ,
DPoint3dP pTrigPoint,
int      *pNumXYZ,
int      maxXYZ,
DEllipse3dCP pEllipse,
double     ellipseOffset,
DPlane3dCP  pPlane,
double     planeOffset
)
    {
#ifdef ANALYSIS
Point offset from ellipse by distance a is
    X = A + Uc + Vs + a (-UxM s + VxM c) / mag(-UxMs + VxMc)
    (c,s are cosine and sine of usual ellipse parameterization.
    M is unit normal.
Signed distance from plane with origin B, unit normal n is
    b = (X - B).n = (A + Uc + Vs + a(-UxM s + VxM c)/mag(-UxM s+VxM c) -B).n
    b = (A-B).n + U.n c + V.n s + a (-UxM.n s + VxM.n c)/mag(..)
    (b - (A-B).n - U.n c - V.n s)^2 mag^2(..) = a^2(-UxM.n s + VxM.n c)^2
    (alpha - U.n c - V.n s)^2  (g0 s^2 + h0 c^2 + f0 cs) = a^2 (g1 s^2 + h1 c^2 + f1 cs)

    Trig polynomials c = C/W, s = S/W:

    (alpha  W - U.n C - V.n S)^2 (g0 S^2 + h0 C^2 + f0 C S) = a^2 (g1 S^2 + h1 C^2 + f1 C S)
    (alpha  W - U.n C - V.n S)^2 (g0 S^2 + h0 C^2 + f0 C S) = W^2 (g2 S^2 + h2 C^2 + f2 C S)

    alpha = b - (A-B).n
    mag^2 = (-UxM s + VxMc ).(-UxM s + VxM c) = UxM.UxM s^2 + VxM.VxM c^2 - 2 UxM.VxM cs
#endif
    DVec3d unitPlaneNormal, unitEllipseNormal;
    DVec3d UxM, VxM;
    unitPlaneNormal.Normalize (pPlane->normal);
    unitEllipseNormal.NormalizedCrossProduct (pEllipse->vector0, pEllipse->vector90);

    UxM.CrossProduct (pEllipse->vector0, unitEllipseNormal);
    VxM.CrossProduct (pEllipse->vector90, unitEllipseNormal);

    double alpha = planeOffset - pEllipse->center.DotDifference (pPlane->origin, unitPlaneNormal);
    double UdotN = pEllipse->vector0.DotProduct (unitPlaneNormal);
    double VdotN = pEllipse->vector90.DotProduct (unitPlaneNormal);

    double UxMdotN = UxM.DotProduct (unitPlaneNormal);
    double VxMdotN = VxM.DotProduct (unitPlaneNormal);

    double a2 = ellipseOffset * ellipseOffset;
    double g2 = a2 * UxMdotN * UxMdotN;
    double h2 = a2 * VxMdotN * VxMdotN;
    double f2 = -2.0 * a2 * UxMdotN * VxMdotN;

    double UxMdotUxM = UxM.DotProduct (UxM);
    double UxMdotVxM = UxM.DotProduct (VxM);
    double VxMdotVxM = VxM.DotProduct (VxM);

    double g0 = UxMdotUxM;
    double h0 = VxMdotVxM;
    double f0 = -2.0 * UxMdotVxM;

    double yy[2] = {-1.0, 1.0};
    int numXYZ = 0;
    for (int direction = 0; direction < 2; direction++)
        {
        // Bezier coffs for half of a unit circle ...
        double bezC[3] = {1.0,   0.0, -1.0};    // cosine == C(t) / W(t)
        double bezS[3] = {0.0, yy[direction],  0.0};    // sine   == S(t) / W(t)
        double bezW[3] = {1.0,   0.0,  1.0};

        double bezCC[5], bezSS[5], bezCS[5];

        bsiBezier_univariateProduct (bezCC, 0, 1,
                            bezC, 3, 0, 1,
                            bezC, 3, 0, 1);
        bsiBezier_univariateProduct (bezSS, 0, 1,
                            bezS, 3, 0, 1,
                            bezS, 3, 0, 1);
        bsiBezier_univariateProduct (bezCS, 0, 1,
                            bezC, 3, 0, 1,
                            bezS, 3, 0, 1);

        // (alpha  W - U.n C - V.n S)^2 (g0 S^2 + h0 C^2 + f0 C S) = W^2(g2 S^2 + h2 C^2 + f2 C S)
        //    P ^2 Q = R

        double bezP[3];
        for (int i = 0; i < 3; i++)
            bezP[i] = alpha * bezW[i] - UdotN * bezC[i] - VdotN * bezS[i];

        double bezQQ[5];
        for (int i = 0; i < 5; i++)
            bezQQ[i] = g0 * bezSS[i] + h0 * bezCC[i] + f0 * bezCS[i];
        double bezW2[5];
        bsiBezier_univariateProduct (bezW2, 0, 1,
                                    bezW, 3, 0, 1,
                                    bezW, 3, 0, 1);

        double roots[9];
        int numRoots = 0;

        double q2OverW2;
        static double sQQRelTol = 1.0e-12;
        static double sGRelTol  = 1.0e-12;
        static double sGAbsTol  = 1.0e-15;
        if (proportionalBeziers (bezQQ, bezW2, 5, sQQRelTol, q2OverW2))
            {
            // It's a circle hiding underneath the ellipse ...
            //  (b - (A-B).n - U.n c - V.n s) qOverW = a(-UxM.n s + VxM.n c)
            //  (alpha - U.n c - V.n s)
            double qOverW = sqrt (q2OverW2);
            double bezG[3], bezG0[3], bezG1[3];
            for (int i = 0; i < 3; i++)
                {
                bezG0[i] = qOverW * (alpha * bezW[i] - UdotN * bezC[i] - VdotN * bezS[i]);
                bezG1[i] = ellipseOffset * ( - UxMdotN * bezS[i] + VxMdotN * bezC[i]);
                bezG[i] = bezG0[i] - bezG1[i];
                }

            if (!zeroBezier (bezG, 3, bezG0, 3, sGAbsTol, sGRelTol))
                bsiBezier_univariateRoots (roots, &numRoots, bezG, 3);
            }
        else
            {
            double bezP2[5];
            bsiBezier_univariateProduct (bezP2, 0, 1,
                                        bezP, 3, 0, 1,
                                        bezP, 3, 0, 1);
            double bezP2QQ[9];
            bsiBezier_univariateProduct (bezP2QQ, 0, 1,
                                        bezP2, 5, 0, 1,
                                        bezQQ,  5, 0, 1);





            double bezR[5];
            for (int i = 0; i < 5; i++)
                bezR[i] = g2 * bezSS[i] + h2 * bezCC[i] + f2 * bezCS[i];

            double bezW2R[9];
            bsiBezier_univariateProduct (bezW2R, 0, 1,
                                        bezR,  5, 0, 1,
                                        bezW2, 5, 0, 1);

            double bezF[9];
            for (int i = 0; i < 9; i++)
                bezF[i] = bezP2QQ[i] - bezW2R[i];

            bsiBezier_univariateRoots (roots, &numRoots, bezF, 9);
            }

        for (int j = 0; j < numRoots; j++)
            {
            double fC, fS, fW;
            bsiBezier_evaluate (&fC, bezC, 3, 1, roots[j]);
            bsiBezier_evaluate (&fS, bezS, 3, 1, roots[j]);
            bsiBezier_evaluate (&fW, bezW, 3, 1, roots[j]);
            double c = fC / fW;
            double s = fS / fW;
            DVec3d Q, R;
            Q.SumOf (UxM, -s, VxM, c);
            R.SumOf (pEllipse->vector0, c, pEllipse->vector90, s);
            double aQdotNdivMagQ = ellipseOffset * Q.DotProduct (unitPlaneNormal) / Q.Magnitude ();
            double alphaMinusRdotN = alpha - R.DotProduct (unitPlaneNormal);
            bool bIsReversed = aQdotNdivMagQ * alphaMinusRdotN < 0.0;

            DPoint3d xyzE, xyz;
            bsiDEllipse3d_evaluateDPoint3dFromLocal (pEllipse, &xyzE, c, s);
            DVec3d TxM;
            double direction = bIsReversed ? -1.0 : 1.0;
            TxM.SumOf (UxM, -direction * s, VxM, direction * c);
            DVec3d unitNormal;
            unitNormal.Normalize (TxM);
            bsiDPoint3d_addScaledDVec3d (&xyz, &xyzE, &unitNormal, ellipseOffset);
            if (numXYZ < maxXYZ)
                {
                if (pXYZ != NULL)
                    pXYZ[numXYZ] = xyz;
                if (pTrigPoint != NULL)
                    pTrigPoint[numXYZ].Init (c, s, bsiTrig_atan2 (s,c));
                }
            numXYZ++;
            }
        }

    if (pNumXYZ != NULL)
        *pNumXYZ = numXYZ;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
