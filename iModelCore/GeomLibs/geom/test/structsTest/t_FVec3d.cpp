/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FVec3d,HelloWorld)
    {
    FVec3d point0 = FVec3d::From (1.0, 2.0, 3.0);
    Check::Near (1.0, point0.x);
    Check::Near (2.0, point0.y);
    Check::Near (3.0, point0.z);
    }



#define CompileFVec3dDotProductPrecisionTables_not
#ifdef CompileFVec3dDotProductPrecisionTables
// EDL May 15, 2017
// This is
// Form vectors that should be unit vectors (in double precision)
// Truncate them to single precision (vector0F, vector1F)
// Compare dot products with
//    (a) FVec3d::FDotProduct -- internally work with floats
//    (b) FVec3d::DotProduct -- internally promote to double (in inlined template)
//    (c) DVec3d::DotProduct (DVec3d) -- promote to double in caller, all double internally.
// Visually observable:
//  ** (b) and (c) match.  The template expander really does seem to up-cast to double and accumulate double products.
//  ** (a) differs.
//  ** (a) has largest relative error for when the two vectors being dotted are close to 90 degrees apart.
// BUT ... the (a) differences are (subjectively) less than I expected.
//    I think the FVec3d::DotProduct values are "better" than FVec3d::FDotProduct.   But I can't say definitively it's catastrophic.
// 
void breakPoint (){}  // a place to put a break.
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FVec3d,DotProductPrecison)
    {
    for (double z : bvector<double> {0,1,4})
        {
        GEOMAPI_PRINTF ("z = %g\n  (theta0    sweep    theta1)      dotDD    (eFD)    (eFF)  (eFD/eFF) (eFF/dotDD)\n", z);
        for (double theta0 : bvector<double> {0.0, 0.1, 0.213892901389, 0.3, 0.5, 0.9, Angle::DegreesToRadians (89.0) })
            {
            for (double delta: bvector<double> {
                        Angle::DegreesToRadians (100.0),
                        Angle::DegreesToRadians (91.0),
                        Angle::DegreesToRadians (90.1),
                        Angle::DegreesToRadians (90.01),
                        Angle::DegreesToRadians (90.001),
                        Angle::DegreesToRadians (90.0001),
                        Angle::DegreesToRadians (10.0),
                        Angle::DegreesToRadians (1.0),
                        Angle::DegreesToRadians (0.1),
                        Angle::DegreesToRadians (0.01),
                        Angle::DegreesToRadians (0.001),
                        Angle::DegreesToRadians (0.0001)
                        }
                    )
                {
                double theta1 = theta0 + delta;
                DVec3d vector0D = DVec3d::From (cos(theta0), sin(theta0));
                DVec3d vector1D = DVec3d::From (cos(theta1), sin(theta1));
                FVec3d vector0F = FVec3d::From (vector0D);
                FVec3d vector1F = FVec3d::From (vector1D);
                //double dotDD = vector0D.DotProduct (vector1D);
                float dotFF = vector0F.FDotProduct (vector1F);
                double dotDD = DVec3d::From (vector0F).DotProduct (DVec3d::From (vector1F));
                double dotFD = vector0F.DotProduct (vector1F);
                double eFF = fabs (dotFF - dotDD);
                double eFD = fabs (dotFD - dotDD);
                GEOMAPI_PRINTF (" (%9.6lg %9.6lg %9.6lg)    %12.7le (%8.2le) (%8.2le) (%8.2le) (%8.2le)\n",
                            Angle::RadiansToDegrees (theta0),
                            Angle::RadiansToDegrees (delta),
                            Angle::RadiansToDegrees (theta1),
                            dotDD,
                            eFD, eFF,
                            eFF == 0.0 ? 9999.0 : eFD / eFF,
                            eFF / dotDD
                            );
                breakPoint ();

                }
            }
        }
    }
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FVec3d,OperatorOverload)
    {
    auto point0F = FPoint3d::From (2,3,5);
    auto point1F = FPoint3d::From (11,13,17);
    auto point0D = DPoint3d::From (point0F);
    auto point1D = DPoint3d::From (point1F);
    auto vector01F = point1F - point0F;
    auto vector01D = point1D - point0D;
    double a = -23.0;
    Check::Exact
        (
        vector01D,
        DVec3d::From (vector01F),
        "operator point-point"
        );
    Check::Exact
        (
        vector01D * a,
        DVec3d::From (vector01F * a),
        "operator vector * double"
        );
    Check::Exact
        (
        a * vector01D,
        DVec3d::From (a * vector01F),
        "operator double * vector"
        );
    }

template <typename T>
void TestVectorParallel ()
    {
    // verify behavior of IsParallelTo () with opposite vectors
    auto plusZ = T::UnitZ ();
    auto negZ = -1.0 * plusZ;
    auto plusX = T::UnitX ();
    Check::True (plusZ.IsParallelTo (negZ));
    Check::True (plusZ.IsParallelTo (plusZ));
    Check::False (plusZ.IsPositiveParallelTo (negZ));
    Check::False(plusZ.IsParallelTo (plusX));
    Check::True(plusZ.IsPerpendicularTo (plusX));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FVec3d,ParallelAndPositiveParallel)
    {
    TestVectorParallel <DVec3d> ();
    TestVectorParallel <FVec3d> ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FVec3d, DotCrossProduct) 
    {
    double refValueShift = 1.0e6;   // to force loose tolerances.
    auto point0F = FPoint3d::From (2.0,3.0,5.0);
    auto point1F = FPoint3d::From (11.0,13.0,17.0);
    auto point2F = FPoint3d::From (5.0,6.0,-1.0);
    auto point3F = FPoint3d::From (10.0,8.0,11.0);
    auto point0D = DPoint3d::From (point0F);
    auto point1D = DPoint3d::From (point1F);
    auto point2D = DPoint3d::From (point2F);
    auto point3D = DPoint3d::From (point3F);
    auto vector01F = point1F - point0F;
    auto vector01D = point1D - point0D;
    auto vector23F = point3F - point2F;
    auto vector23D = point3D - point2D;
    Check::Exact(DVec3d::From(FVec3d::FromStartEnd(point0D, point1D)) , DVec3d::FromStartEnd(point0D, point1D));
    Check::Exact(DVec3d::From(FVec3d::FromStartEnd(point0F, point1F)) , DVec3d::FromStartEnd(point0D, point1D));
    Check::Near (DVec3d::From(FVec3d::FromStartEndNormalized(point0D, point1D)) , DVec3d::FromStartEndNormalize(point0D, point1D), "Normalize", refValueShift);
    Check::Near (DVec3d::From(FVec3d::FromStartEndNormalized(point0F, point1F)) , DVec3d::FromStartEndNormalize(point0D, point1D), "Normalize", refValueShift);
    Check::ExactDouble(vector01F.DotProduct(vector23F) , vector01D.DotProduct(vector23D));
    Check::ExactDouble(vector01F.DotProductXY(vector23F) , vector01D.DotProductXY(vector23D));
    Check::ExactDouble(vector01F.CrossProductXY(vector23F) , vector01D.CrossProductXY(vector23D));
    Check::Exact(DVec3d::From(FVec3d::FromCrossProduct(vector01F, vector23F)) , DVec3d::FromCrossProduct(vector01D, vector23D));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FVec3d, Angles)
    {
    auto point0F = FVec3d::From(2.0, 0, 0);
    auto point1F = FVec3d::From(-2.0, 2.0, 0);
    auto point0D = DVec3d::From(point0F);
    auto point1D = DVec3d::From(point1F);
    auto planeNormF = FVec3d::From(0, 0, 1);
    auto planeNormD = DVec3d::From(planeNormF);
    Check::ExactDouble(point0D.AngleTo(point1D), point0F.AngleTo(point1F).Radians());
    Check::ExactDouble(point0D.AngleToXY(point1D), point0F.AngleToXY(point1F).Radians());
    Check::ExactDouble(point0D.AngleFromPerpendicular(point1D),  point0F.AngleFromPerpendicular(point1F).Radians());
    Check::ExactDouble(point0D.PlanarAngleTo(point1D, planeNormD), point0F.PlanarAngleTo(point1F, planeNormF).Radians());
    Check::ExactDouble(point0D.SignedAngleTo(point1D, planeNormD), point0F.SignedAngleTo(point1F, planeNormF).Radians());
    //change to orientation vector
    Check::ExactDouble(point0D.SmallerUnorientedAngleTo(point1D), point0F.SmallerUnorientedAngleTo(point1F).Radians());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FVec3d, Magnitude)
    {
    auto point0F = FVec3d::From(2.0, 3.0, -7.0);
    auto point1F = FVec3d::From(-2.0, 2.0, 1.0);
    auto point0D = DVec3d::From(point0F);
    auto point1D = DVec3d::From(point1F);
    Check::Exact(DVec3d::From(FVec3d::From(point0D)), point0D);
    Check::Exact(DVec3d::From(FVec3d::FromOne()), DVec3d::FromOne());
    Check::Exact(DVec3d::From(FVec3d::FromZero()), DVec3d::FromZero());
    Check::Exact(DVec3d::From(FVec3d::FromXY(point0D)), DVec3d::From(point0D.x, point0D.y));
    Check::Exact(DVec3d::From(FVec3d::FromXY(point0F)), DVec3d::From(point0D.x, point0D.y));

    Check::ExactDouble(point0F.MaxAbs(), point0D.MaxAbs());
    Check::ExactDouble(point0F.Magnitude(), point0D.Magnitude());
    Check::ExactDouble(point0F.MagnitudeSquared(), point0D.MagnitudeSquared());
    double tol = 0.001;
    Check::True(Check::False(point0F.IsEqual(point1F)) == Check::False(point0D.IsEqual(point1D)));
    auto tpointF = FVec3d::From(2.0, 3.1, -7.0);
    auto tpointD = DVec3d::From(tpointF);
    Check::True(Check::False(point0F.IsEqual(tpointF, tol)) == Check::False(point0D.IsEqual(tpointD,tol)));
    }