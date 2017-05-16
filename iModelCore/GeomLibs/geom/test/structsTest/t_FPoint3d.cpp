#include "testHarness.h"


TEST(FPoint3d,HelloWorld)
    {
    FPoint3d point0 = FPoint3d::From (1.0, 2.0, 3.0);
    Check::Near (1.0, point0.x);
    }

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

    auto rotation = RotMatrix::From(transform);
    Check::Exact(DPoint3d::FromSumOf(originD0, (rotation * DPoint3d::From(originF1))),
                 DPoint3d::From(FPoint3d::FromMultiply(originF0, rotation, originD1.x, originD1.y, originD1.z)), "Rotation * point");

    Check::Exact(DPoint3d::FromSumOf(originD0, (rotation * DPoint3d::From(vector0.x, vector0.y, vector0.z))),
                 DPoint3d::From(FPoint3d::FromMultiply(originF0, rotation, vector0)), "Rotation * point");

    // Farhad -- you can test lots and lots of FPoint3d methods (SumOf, FromSumOf, FromMultiply) .. with just originD, vector0, vector1, vector2, and this transform
    // You'll need a few more points for Distance and others that have multiple FPoint3d inputs.

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

    //Equal AlmostEqual
    bvector<FPoint3d> left = { originF0, originF1, originF2 };
    bvector<FPoint3d> right = { FPoint3d::From(originD0), FPoint3d::From(originD1), FPoint3d::From(originD2) };
    Check::True(FPoint3d::AlmostEqual(left, right));
    Check::True(FPoint3d::AlmostEqualXY(left, right));

    }

