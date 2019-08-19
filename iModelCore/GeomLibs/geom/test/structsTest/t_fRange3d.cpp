/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "testHarness.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FRange3d, RangeDistance)
    {
    auto point0F = FPoint3d::From(3.0, 3.0, 8.0);
    auto point1F = FPoint3d::From(18.0, 13.0, 10.0);
    auto point0D = DPoint3d::From(point0F);
    //auto point1D = DPoint3d::From(point1F);

   // Check::ExactRange(DRange3d::From(point0F), DRange3d::From(FRange3d::From(point0D)));    --error here
    Check::ExactRange(DRange3d::From(point0D), DRange3d::From(FRange3d::From(point0F)));
    Check::True(FRange3d::From(point0F).IsSinglePoint());
    auto range01F = FRange3d::From(point0F, point1F);
    auto range01D = DRange3d::From(range01F);
    auto pnttoExtF = FPoint3d::From(22.0, 23.0, 30.0);
    auto pnttoExtD = DPoint3d::From(pnttoExtF);
    range01F.Extend(pnttoExtF);
    range01D.Extend(pnttoExtD);
    Check::ExactRange(DRange3d::From(range01F), range01D);
    Check::True(range01D.MaxAbs() == range01F.MaxAbs());
    DPoint3d cornersD[8];
    range01D.Get8Corners(cornersD);
    bvector<FPoint3d> cornersF;
    range01F.Get8Corners(cornersF);
    for (int i = 0; i < 8; i++)
        Check::Exact(cornersD[i], DPoint3d::From(cornersF[i]));

    Check::True(FRange3d::NullRange().IsNull());
    Check::True(range01D.MaxAbs() == range01F.MaxAbs());
    //diagonal tests
    Check::ExactDouble(range01F.DiagonalDistance(), range01D.DiagonalDistance());
    Check::ExactDouble(range01F.DiagonalDistanceXY(), range01D.DiagonalDistanceXY());
    Check::Exact(DVec3d::From(range01F.DiagonalVector()), range01D.DiagonalVector());
    Check::Exact(DVec3d::From(range01F.DiagonalVectorXY()), range01D.DiagonalVectorXY());
    Check::True(range01F.AreAllSidesLongerThan(11.0));

    auto tpointF = FPoint3d::From(30.0, 30.0, 30.0);
    auto tpointD = DPoint3d::From(tpointF);
    Check::ExactDouble(range01D.DistanceSquaredOutside(tpointD), range01F.DistanceSquaredOutside(tpointD));
    Check::ExactDouble(range01D.DistanceOutside(tpointD), range01F.DistanceOutside(tpointD));

    tpointF = FPoint3d::From(11.0, 11.0, 11.0);
    tpointD = DPoint3d::From(tpointF);
    Check::True(Check::True(range01D.IsContained(tpointD)) == Check::True(range01F.IsContained(tpointF)));
    Check::True(Check::True(range01D.IsContained(tpointD.x, tpointD.y, tpointD.z)) == Check::True(range01F.IsContained(tpointF.x, tpointF.y, tpointF.z)));
    Check::True(Check::True(range01D.IsContainedXY(tpointD)) == Check::True(range01F.IsContainedXY(tpointF)));
    Check::ExactDouble(range01D.LargestCoordinate(), range01F.LargestCoordinate());
    Check::ExactDouble(range01D.LargestCoordinateXY(), range01F.LargestCoordinateXY());
    Check::Exact(range01D.low, DPoint3d::From(range01F.Low()));
    Check::Exact(range01D.high, DPoint3d::From(range01F.High()));
    Check::ExactDouble(range01D.XLength(), range01F.XLength());
    Check::ExactDouble(range01D.YLength(), range01F.YLength());
    Check::ExactDouble(range01D.ZLength(), range01F.ZLength());


    auto point2F = FPoint3d::From(-3.0, -3.0, -8.0);
    auto point3F = FPoint3d::From(-18.0, -13.0, -10.0);
    //auto point2D = DPoint3d::From(point2F);
    //auto point3D = DPoint3d::From(point3F);
    auto range23F = FRange3d::From(point2F, point3F);
    auto range23D = DRange3d::From(range23F);
    Check::ExactDouble(range01D.DistanceSquaredTo(range23D), range01F.DistanceSquaredTo(range23F));
    
    Check::False(range01F.IntersectsWith(range23F, false));
    Check::ExactRange(DRange3d::FromIntersection(range01D, range23D), DRange3d::From(FRange3d::FromIntersection(range01F, range23F)));
    FPoint3d pntExtF0, pntExtF1;
    pntExtF0.Init(3.0, 3.0, 8.0);
    pntExtF1.Init(0.0, 0.0, 0.0);

    range23F.Extend(pntExtF0, pntExtF1);
    range23D.Extend(pntExtF0, pntExtF1);
    Check::True(Check::True(range01F.IntersectsWith(range23F, true)) == Check::True(range01D.IntersectsWith(range23D)));

    auto unionRange = DRange3d::From(range01F);
    unionRange.Extend(range23D.low);
    unionRange.Extend(range23D.high);
    Check::ExactRange(unionRange, DRange3d::From(FRange3d::FromUnion(range01F, range23F)));
    } 

//static FRange3d From (DRange3dCR dRange);
//From (DPoint3dCR pointA, DPoint3dCR pointB);
//From (bvector<DPoint3d> const &points);
//From (FPoint3dCR point);
//From (FPoint3dCR pointA, FPoint3dCR pointB);
//From (bvector<FPoint3d> const &points);


//DRange3d From (DPoint3dCR point)
//From (FPoint3dCR pointA, FPoint3dCR pointB);
//From (bvector<FPoint3d> const &points);
//static DRange3d From(double x, double y, double z);
//DRange3d From (DPoint3dCP point, int n);
//From (bvector<DPoint3d> const &points);
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FRange3d, Volume)
    {
    auto point0F = FPoint3d::From(-3.0, -3.0, -8.0);
    auto point1F = FPoint3d::From(-18.0, -13.0, -10.0);
    auto range01F = FRange3d::From(point0F, point1F);
    auto range01D = DRange3d::From(range01F);
    Check::ExactDouble(range01D.Volume(), range01F.Volume());
    }