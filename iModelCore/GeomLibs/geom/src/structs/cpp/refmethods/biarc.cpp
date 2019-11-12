/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          02/95
+---------------+---------------+---------------+---------------+---------------+------*/
static bool calcBiarcCenters
(
DPoint3dR       center0,
DPoint3dR       center1,
DPoint3dCR      point0,
DPoint3dCR      point1,
DPoint3dCR      radialDirection0,
DPoint3dCR      radialDirection1,
double          rad0,
double          rad1,
bool            isUnimodel
)
    {
    int         i0, i1, i0Min = 0, i1Min = 0;
    double      diff, dist, delta, deltaMin = DBL_MAX;
    DPoint3d    arcCenter0[2], arcCenter1[2];
    double factor0, factor1;
    
    if (!DoubleOps::SafeDivide (factor0, rad0, radialDirection0.Magnitude (), 0.0))
        return false;
    if (!DoubleOps::SafeDivide (factor1, rad1, radialDirection1.Magnitude (), 0.0))
        return false;

    arcCenter0[0] = DPoint3d::FromSumOf (point0, radialDirection0, factor0);
    arcCenter0[1] = DPoint3d::FromSumOf (point0, radialDirection0, -factor0);

    arcCenter1[0] = DPoint3d::FromSumOf (point1, radialDirection1,  factor1);
    arcCenter1[1] = DPoint3d::FromSumOf (point1, radialDirection1, -factor1);

    diff = isUnimodel ? fabs (rad0 - rad1) : fabs (rad0 + rad1);

    // TR #116288: take centers whose distance is closest to radius difference/sum;
    // previous logic tested three differences against fc_epsilon and failing that, assumed last was smallest,
    // but this failed to find smallest differences larger than fc_epsilon.
    for (i0 = 0; i0 < 2; i0++)
        {
        for (i1 = 0; i1 < 2; i1++)
            {
            dist = arcCenter0[i0].Distance(arcCenter1[i1]);
            delta = fabs (dist - diff);
            if (delta < deltaMin)
                {
                i0Min = i0;
                i1Min = i1;
                deltaMin = delta;
                }
            }
        }
    center0 = arcCenter0[i0Min];
    center1 = arcCenter1[i1Min];
    return true;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          02/95
+---------------+---------------+---------------+---------------+---------------+------*/
bool calculateBiarc
(
DEllipse3dR ellipse0,
DEllipse3dR ellipse1,
DPoint3dCP point0P,
DPoint3dCP point1P,
DVec3dCP tangent0P,
DVec3dCP tangent1P
)
    {
    bool        isUnimodel;
    double      radius0, radius1, magnitude, cosine0, cosine1, sine0, sine1,
                tmp, tmp0, tmp1;
    DPoint3d    chord, dir0, dir1, center0, center1, normal;

    /* Compute the radii for arcs */
    chord.DifferenceOf (*point1P, *point0P);
    magnitude = chord.Magnitude ();
    chord.Normalize ();
    cosine0 = chord.DotProduct (*tangent0P);
    cosine1 = chord.DotProduct (*tangent1P);
    sine0 = sqrt(1.0 - cosine0 * cosine0);
    sine1 = sqrt(1.0 - cosine1 * cosine1);
    dir0.CrossProduct (*tangent0P, chord);
    dir1.CrossProduct (*tangent1P, chord);

    static double s_normalFlipTol = 1.0e-8;
    // TR #116288:
    // If tangent0 is (nearly) parallel to chord, then |dir0| ~ 0.
    // If this makes the below dot product just slightly negative, then since cosine0 ~ 1 and sine0 ~ 0,
    //  we have the possibility that tmp can be +-INF.
    // To rule this out, allow for some floating point slop in the negativity test below.
    // Using isUnimodal = false for the (near) tangency case avoids the bad intermediate value in computing the arc radii.
    // The same problem occurs (and is fixed by this change) if tangent1 is (nearly) parallel to chord.
    isUnimodel = dir0.DotProduct (dir1) < -s_normalFlipTol;
    if (isUnimodel)
        {
        tmp0 = 1.0 - cosine0;
        tmp1 = 1.0 - cosine1;
        tmp = magnitude / (sine0 * tmp1 + sine1 * tmp0);
        radius0 = tmp1 * tmp;
        radius1 = tmp0 * tmp;
        }
    else
        {
        radius0 = radius1 =  magnitude / (sine0 + sine1 + sqrt(4.0 - (cosine0 + cosine1) * (cosine0 + cosine1)));
        }

    // TR #116288: avoid colinear tangent & chord---caller ensures one tangent is not colinear
    if (fabs (cosine0) < fabs (cosine1))
        normal.CrossProduct (chord, *tangent0P);
    else
        normal.CrossProduct (chord, *tangent1P);

    /* Compute the centers for the arcs */
    dir0.CrossProduct (*tangent0P, normal);
    dir1.CrossProduct (*tangent1P, normal);
    calcBiarcCenters (center0, center1, *point0P, *point1P, dir0, dir1, radius0, radius1, isUnimodel);

    DPoint3d pointC;
    /* Create points on arc */
    if (isUnimodel)
        {
        if (DoubleOps::AlmostEqual (radius0, radius1))
            {
            dir0.Interpolate (*point0P, 0.5, *point1P);
            dir0.DifferenceOf (dir0, center0);
            }
        else
            {
            if (radius1 >= radius0)
                dir0.DifferenceOf (center0, center1);
            else
                dir0.DifferenceOf (center1, center0);
            }
        dir0.Normalize ();
        pointC.SumOf (center0, dir0, radius0);
        }
    else
        {
        pointC.Interpolate (center0, 0.5, center1);
        }
    DVec3d vector0A = DVec3d::FromStartEnd (center0, *point0P);
    DVec3d vector0C = DVec3d::FromStartEnd (center0, pointC);
    DVec3d vector1C = DVec3d::FromStartEnd (center1, pointC);
    DVec3d vector1B = DVec3d::FromStartEnd (center1, *point1P);
    double theta0 = vector0A.AngleTo (vector0C);
    double theta1 = vector1C.AngleTo (vector1B);
    DVec3d normal0 = DVec3d::FromNormalizedCrossProduct (vector0A, vector0C);
    DVec3d normal1 = DVec3d::FromNormalizedCrossProduct (vector1C, vector1B);
    DVec3d perp0 = DVec3d::FromCrossProduct (normal0, vector0A);
    DVec3d perp1 = DVec3d::FromCrossProduct (normal1, vector1C);
    ellipse0 = DEllipse3d::FromVectors (center0, vector0A, perp0, 0.0, theta0);
    ellipse1 = DEllipse3d::FromVectors (center1, vector1C, perp1, 0.0, theta1);
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          02/95
+---------------+---------------+---------------+---------------+---------------+------*/
bool DEllipse3d::Construct_Biarcs
(
DEllipse3dR ellipse0,
DEllipse3dR ellipse1,
DPoint3dCR pointA,
DVec3dCR   tangentA,
DPoint3dCR pointB,
DVec3dCR   tangentB
)
    {
    return calculateBiarc (ellipse0, ellipse1, &pointA, &pointB, &tangentA, &tangentB);
    }
END_BENTLEY_GEOMETRY_NAMESPACE