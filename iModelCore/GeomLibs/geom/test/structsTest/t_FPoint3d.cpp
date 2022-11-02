/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FPoint3d,HelloWorld)
    {
    FPoint3d point0 = FPoint3d::From (1.0f, 2.0f, 3.0f);
    Check::Near (1.0, point0.x);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FPoint3d, SumOf)
    {
    // THIS IS ALL THE INPUT VALUES NEEDED TO TEST ALL SUMOF METHODS ....
    auto originF0 = FPoint3d::From(1, 2, 3);
    auto originD0 = DPoint3d::From(originF0);
    auto originF1 = FPoint3d::From(5, 6, 7);
    auto originD1 = DPoint3d::From(originF1);
    auto originF2 = FPoint3d::From(9, 10, 11);
    auto originD2 = DPoint3d::From(originF2);

    auto vector0 = DVec3d::From(4, 5, 6);
    auto vector1 = DVec3d::From(7, 8, 9);
    auto vector2 = DVec3d::From(10, 11, 12);

    double scale0 = -3.0;
    double scale1 = 17.0;
    double scale2 = -13.0;

    //FromSumOf()
    Check::Exact(
        DPoint3d::FromSumOf(originD0, vector0),
        DPoint3d::From(FPoint3d::FromSumOf(originF0, vector0)), "SumOf DPoint3d matches FPoint3d");

    Check::Exact(
        DPoint3d::FromSumOf(originD0, vector0, scale0),
        DPoint3d::From(FPoint3d::FromSumOf(originF0, vector0, scale0)), "SumOf DPoint3d matches FPoint3d");

    Check::Exact(
        DPoint3d::FromSumOf(originD0, vector0, scale0, vector1, scale1),
        DPoint3d::From(FPoint3d::FromSumOf(originF0, vector0, scale0, vector1, scale1)), "SumOf DPoint3d matches FPoint3d");

    Check::Exact(
        DPoint3d::FromSumOf(originD0, vector0, scale0, vector1, scale1, vector2, scale2),
        DPoint3d::From(FPoint3d::FromSumOf(originF0, vector0, scale0, vector1, scale1, vector2, scale2)), "SumOf DPoint3d matches FPoint3d");

    Check::Exact(
        DPoint3d::FromSumOf(originD0, scale0, originD1, scale1),
        DPoint3d::From(FPoint3d::FromSumOf(originF0, scale0, originF1, scale1)), "SumOf DPoint3d matches FPoint3d");

    Check::Exact(
        DPoint3d::FromSumOf(originD0, scale0, originD1, scale1, originD2, scale2),
        DPoint3d::From(FPoint3d::FromSumOf(originF0, scale0, originF1, scale1, originF2, scale2)), "SumOf DPoint3d matches FPoint3d");

    //SumOf
    FPoint3d fpoint;
    DPoint3d dpoint;
    fpoint.SumOf(originF0, vector0, scale0);
    dpoint.SumOf(originD0, vector0, scale0);
    Check::Exact(DPoint3d::From(dpoint.x, dpoint.y, dpoint.z),
                 DPoint3d::From(fpoint), "SumOf DPoint3d matches FPoint3d");

    fpoint.SumOf(originF0, vector0, scale0, vector1, scale1);
    dpoint.SumOf(originD0, vector0, scale0, vector1, scale1);
    Check::Exact(DPoint3d::From(dpoint.x, dpoint.y, dpoint.z),
                 DPoint3d::From(fpoint), "SumOf DPoint3d matches FPoint3d");

    fpoint.SumOf(originF0, vector0, scale0, vector1, scale1, vector2, scale2);
    dpoint.SumOf(originD0, vector0, scale0, vector1, scale1, vector2, scale2);
    Check::Exact(DPoint3d::From(dpoint.x, dpoint.y, dpoint.z),
                 DPoint3d::From(fpoint), "SumOf DPoint3d matches FPoint3d");

    fpoint.SumOf(originF0, scale0, originF1, scale1);
    dpoint.SumOf(originD0, scale0, originD1, scale1);
    Check::Exact(DPoint3d::From(dpoint.x, dpoint.y, dpoint.z),
                 DPoint3d::From(fpoint), "SumOf DPoint3d matches FPoint3d");

    fpoint.SumOf(originF0, scale0, originF1, scale1, originF2, scale2);
    dpoint.SumOf(originD0, scale0, originD1, scale1, originD2, scale2);
    Check::Exact(DPoint3d::From(dpoint.x, dpoint.y, dpoint.z),
                 DPoint3d::From(fpoint), "SumOf DPoint3d matches FPoint3d");


    //transform
    auto transform = Transform::FromOriginAndVectors(originD0, vector0, vector1, vector2);
    Check::Exact(transform * DPoint3d::From(originF0),
                 DPoint3d::From(FPoint3d::FromMultiply(transform, originD0)), "Transform * point");

    Check::Exact(transform * DPoint3d::From(originF0),
                 DPoint3d::From(FPoint3d::FromMultiply(transform, originF0)), "Transform * point");

    Check::Exact(transform * DPoint3d::From(originF0),
                 DPoint3d::From(FPoint3d::FromMultiply(transform, originD0.x, originD0.y, originD0.z)), "Transform * point");

    DVec3d vectorW = DVec3d::From (-2,-4,-5);
    auto rotation = RotMatrix::From(transform);
    Check::Exact(DPoint3d::FromProduct (originD0, rotation, vectorW.x, vectorW.y, vectorW.z),
                 DPoint3d::From(FPoint3d::FromMultiply(originF0, rotation, vectorW.x, vectorW.y, vectorW.z)), "origin + rotation * vector");

    Check::Exact(DPoint3d::FromSumOf(originD0, (rotation * DVec3d::From(vector0.x, vector0.y, vector0.z))),
                 DPoint3d::From(FPoint3d::FromMultiply(originF0, rotation, vector0)), "Rotation * point");

    Check::Exact(DPoint3d::From(FPoint3d::FromShift(originF0, scale0, scale1, scale2)),
                 DPoint3d::FromShift(originD0, scale0, scale1, scale2));

    fpoint.Subtract(originF0, vector0);
    dpoint.Subtract(originD0, vector0);
    Check::Exact(DPoint3d::From(fpoint), dpoint);
    fpoint.Add(vector1);
    dpoint.Add(vector1);
    Check::Exact(DPoint3d::From(fpoint), dpoint);

    Check::Exact(DPoint3d::From(FPoint3d::From(DPoint3d::From(2, 3, 4))),
                 DPoint3d::From(2, 3, 4));
    Check::Exact(DPoint3d::From(FPoint3d::From(DPoint2d::From(2, 3), 4)),
                 DPoint3d::From(2, 3, 4));
   
    Check::Exact(DPoint3d::From(FPoint3d::FromOne()),
                 DPoint3d::FromOne());
    Check::Exact(DPoint3d::From(FPoint3d::FromZero()),
                 DPoint3d::FromZero());

    //SetGetComponent
    fpoint = FPoint3d::From(originD0);
    dpoint = DPoint3d::From(originF0);
    double fpts[3], dpts[3];
    fpoint.GetComponents(fpts[0], fpts[1], fpts[2]);
    dpoint.GetComponents(dpts[0], dpts[1], dpts[2]);
    Check::ExactDouble(fpts[0], dpts[0]);
    Check::ExactDouble(fpts[1], dpts[1]);
    Check::ExactDouble(fpts[2], dpts[2]);

    fpoint.SetComponent(3, 2);
    dpoint.SetComponent(3, 2);
    Check::ExactDouble(fpoint.GetComponent(2), dpoint.GetComponent(2));
    fpoint.Zero(); dpoint.Zero();
    Check::Exact(DPoint3d::From(fpoint), dpoint);
    fpoint.One(); dpoint.One();
    Check::Exact(DPoint3d::From(fpoint), dpoint);

    //Scalar queries
    Check::ExactDouble(originF0.MaxAbs(), originD0.MaxAbs());
    Check::ExactDouble(originF0.MaxAbsIndex(), originD0.MaxAbsIndex());
    Check::ExactDouble(originF0.MaxDiff(originF1), originD0.MaxDiff(originD1));
    Check::ExactDouble(originF0.MinAbs(), originD0.MinAbs());
    Check::ExactDouble(originF0.MinAbsIndex(), originD0.MinAbsIndex());
    Check::True(originF0.ComponentRange().low == originD0.ComponentRange().low);
    Check::True(originF0.ComponentRange().high == originD0.ComponentRange().high);

    //Interpolation 
    Check::Exact(DPoint3d::From(FPoint3d::FromInterpolate(originF0, scale0, originF1)),
                 DPoint3d::FromInterpolate(originD0, scale0, originD1));
    Check::Exact(DPoint3d::From(FPoint3d::FromInterpolateBilinear(originF0, originF1, originF2, fpoint, scale0, scale1)),
                 DPoint3d::FromInterpolateBilinear(originD0, originD1, originD2, dpoint, scale0, scale1));
    fpoint.Interpolate(originF0, 2, originF1);
    dpoint.Interpolate(originD0, 2, originD1);
    Check::Exact(DPoint3d::From(fpoint), dpoint);

    //Is Instance in sector
    
    FPoint3d origin = FPoint3d::From (0.0, 0.0, 0.0);
    FPoint3d target0 = FPoint3d::From (1.0, 0.0, 0.0);
    FPoint3d target1 = FPoint3d::From (0.0, 1.0, 0.0);
    FPoint3d testpoint0 = FPoint3d::From (0.0, 2.0, 0.0);
    DPoint3d dtestpoint0 = DPoint3d::From (testpoint0);
    FPoint3d testpoint1 = FPoint3d::From (-1.0, -1.0, 0.0);
    DPoint3d dtestpoint1 = DPoint3d::From(testpoint1);
    DVec3d upVector = DVec3d::From(0.0, 1.0, 0.0);
    Check::True(testpoint0.IsPointInCCWector(origin, target0, target1, upVector) ==
                dtestpoint0.IsPointInCCWector(DPoint3d::From(origin), DPoint3d::From(target0), DPoint3d::From(target1), upVector));
    Check::True(testpoint1.IsPointInCCWector(origin, target0, target1, upVector) ==
                dtestpoint1.IsPointInCCWector(DPoint3d::From(origin), DPoint3d::From(target0), DPoint3d::From(target1), upVector));
    
    FPoint3d fptest0 = FPoint3d::From(2, 2, 0);
    DPoint3d dptest0 = DPoint3d::From(fptest0);
    Check::True(
        Check::True(fpoint.IsPointInSmallerSector(FPoint3d::From(0, 0, 0), FPoint3d::From(0, 3, 0), FPoint3d::From(1, -3, 0))) ==
        Check::True(dpoint.IsPointInSmallerSector(DPoint3d::From(FPoint3d::From(0, 0, 0)), DPoint3d::From(FPoint3d::From(0, 3, 0)), DPoint3d::From(FPoint3d::From(1, -3, 0))))
    );
    fptest0.Init(2, 2, 0);
    dptest0 = DPoint3d::From(fptest0);
    Check::True(
        Check::False(fpoint.IsPointInSmallerSector(FPoint3d::From(0, 0, 0), FPoint3d::From(0, 3, 0), FPoint3d::From(-1, -3, 0))) ==
        Check::False(dpoint.IsPointInSmallerSector(DPoint3d::From(FPoint3d::From(0, 0, 0)), DPoint3d::From(FPoint3d::From(0, 3, 0)), DPoint3d::From(FPoint3d::From(-1, -3, 0))))
    );


    //Equal AlmostEqual
    bvector<FPoint3d> left = { originF0, originF1, originF2 };
    bvector<FPoint3d> right = { FPoint3d::From(originD0), FPoint3d::From(originD1), FPoint3d::From(originD2) };
    Check::True(FPoint3d::AlmostEqual(left, right));
    Check::True(FPoint3d::AlmostEqualXY(left, right));

    Check::True(originF0.AlmostEqual(FPoint3d::From(1, 2, 3)) == originD0.AlmostEqual(DPoint3d::From(1, 2, 3)));
    Check::True(originF0.AlmostEqual(FPoint3d::From(1, 2, 3), 0.001) == originD0.AlmostEqual(DPoint3d::From(1, 2, 3), 0.001));
    Check::True(originF0.AlmostEqualXY(FPoint3d::From(1, 2, 3)) == originD0.AlmostEqualXY(DPoint3d::From(1, 2, 3)));
    Check::True(originF0.AlmostEqualXY(FPoint3d::From(1, 2, 3), 0.001) == originD0.AlmostEqualXY(DPoint3d::From(1, 2, 3), 0.001));


    Check::True(originF0.IsEqual(originF1)== originD0.IsEqual(originD1));
    Check::True(originF0.IsEqual(FPoint3d::From(originD0)) == originD0.IsEqual(DPoint3d::From(originF0)));
    Check::True(originF0.IsEqual(FPoint3d::From(1.001, 2.001, 3.001), 0.01) ==
    originD0.IsEqual(DPoint3d::From(1.001, 2.001, 3.001), 0.01));

    // Dot/cross/triple products with instance as base point of vectors to other FPoint3d
    Check::ExactDouble(originF0.TripleProductToPoints(originF1, originF2, FPoint3d::From(13,14,15)),
                       originD0.TripleProductToPoints(originD1, originD2, DPoint3d::From(13,14,15)));
    Check::ExactDouble(originF0.DotDifference(originF1, vector0),
                       originD0.DotDifference(originD1, vector0));
    Check::ExactDouble(originF0.DotProductToPoints(originF1, originF2),
                       originD0.DotProductToPoints(originD1, originD2));
    Check::ExactDouble(originF0.DotProductToPointsXY(originF1, originF2),
                       originD0.DotProductToPointsXY(originD1, originD2));
    Check::ExactDouble(originF0.CrossProductToPointsXY(originF1, originF2),
                       originD0.CrossProductToPointsXY(originD1, originD2));

    DPoint3d fullXYZD = DPoint3d::From (2,3,4);
    FPoint3d fullXYZF = FPoint3d::From (2,3,4);
    Check::Exact(DPoint3d::From(FPoint3d::FromXY(fullXYZF)),
                 DPoint3d::FromXY(fullXYZD));
    Check::Exact(DPoint3d::From(FPoint3d::FromXY(fullXYZF, 1.0)),
                 DPoint3d::FromXY(fullXYZD, 1.0));
    Check::Exact(DPoint3d::From(FPoint3d::FromXY(fullXYZD, 1.0)),
                 DPoint3d::FromXY(fullXYZD, 1.0));

    //other
    originF0.Swap(originF1);
    originD0.Swap(originD1);
    Check::Exact(DPoint3d::From(originF0), originD0);
    Check::Exact(DPoint3d::From(originF1), originD1);

    DPoint4d pnt4d = DPoint4d::From(3, 4, 5, 2);
    originF0.XyzOf(pnt4d);
    originD0.XyzOf(pnt4d);

    Check::Exact(DPoint3d::From(originF0), originD0);
    

    //init
    FPoint3d fpnt;
    DPoint3d dpnt;
    fpnt.Init(2.0, 3.0, 4.0);
    dpnt.Init(2.0, 3.0, 4.0);
    Check::Exact(DPoint3d::From(fpnt), dpnt);
    fpnt.Init(DPoint2d::From( 2.0, 3.0), 4.0);
    Check::Exact(DPoint3d::From(fpnt), dpnt);
    fpnt.Init(DVec3d::From( 2.0, 3.0, 4.0));
    dpnt.Init(DVec3d::From( 2.0, 3.0, 4.0));
    Check::Exact(DPoint3d::From(fpnt), dpnt);
    }
	
	
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FPoint3d,ROUND_AWAY)
    {
    int one = 1;
    int a = one << 24;
    double ed = 1.0 / (double)a;
    float  ef = (float)1.0 / (float)a;
    float onePlus = 1.0F + 2.0F * ef;
    float oneMinus = 1.0F - ef;
    float roundAway = 1.000000119f;
    float roundToward = 0.9999999404f;
    GEOMAPI_PRINTF ("Single Precision Unit Roundoff = %.10lg %.10lg\n", ed, (double)ef); // arg passing is always double.
    GEOMAPI_PRINTF ("Single Precision Rounders = %.10lg %.10lg\n", (double)onePlus, (double)oneMinus); // arg passing is always double.
    GEOMAPI_PRINTF ("Single Precision nextAfter neighbors= %.10lg %.10lg\n",
                (double)BeNumerical::BeNextafterf (1.0, 2.0), (double)BeNumerical::BeNextafterf (1.0, 0.0)); // arg passing is always double.

    GEOMAPI_PRINTF ("Single Precision roundTowards roundAway= %.10lg %.10lg\n",
                (double)roundToward, (double)roundAway); // arg passing is always double.
    }

