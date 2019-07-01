/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "testHarness.h"

//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    01/17
//---------------------------------------------------------------------------------------
void areaUnionIntersection(DRange2d range0, DRange2d range1) 
    {
    DRange2d unionRange, intersectRange;
    if (range0.IntersectsWith(range1))
        {
        intersectRange.IntersectionOf(range0, range1);
        Check::True(intersectRange.Area() <= range0.Area());
        Check::True(intersectRange.Area() <= range1.Area());
        }
    unionRange.UnionOf(range0, range1);
    Check::True(unionRange.Area() >= range0.Area());
    Check::True(unionRange.Area() >= range1.Area());
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    01/17
//---------------------------------------------------------------------------------------
TEST(DRange2d, BoxArea)
    {
    areaUnionIntersection(DRange2d::From(DPoint2d::From(1, 1), DPoint2d::From(7, 7)),
                          DRange2d::From(DPoint2d::From(1, 1), DPoint2d::From(7, 7)));
    areaUnionIntersection(DRange2d::From(DPoint2d::From(1.2,3.1), DPoint2d::From(5.3,8.9)),
                          DRange2d::From(DPoint2d::From(5.4, 1.2), DPoint2d::From(7.1, 7)));
    areaUnionIntersection(DRange2d::From(DPoint2d::From(0.1, 0.2), DPoint2d::From(0.3, 0.3)),
                          DRange2d::From(DPoint2d::From(0.25, 0.3), DPoint2d::From(0.7, 0.4)));
    areaUnionIntersection(DRange2d::From(DPoint2d::From(3, 3), DPoint2d::From(7, 7)),
                          DRange2d::From(DPoint2d::From(2, 9), DPoint2d::From(4, 7)));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    01/17
//---------------------------------------------------------------------------------------
TEST(DRange2d, IntersectionOfRanges)
    {
    DRange2d range1 = DRange2d::From(DPoint2d::From(1, 1), DPoint2d::From(7, 7));
    DRange2d range2 = DRange2d::From(DPoint2d::From(8,7), DPoint2d::From(12, 7));
    Check::False( range1.IntersectsWith(range2));
    range2.Extend(7, 6);
    Check::True(range1.IntersectsWith(range2));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    01/17
//---------------------------------------------------------------------------------------
TEST(DRange2d, BoxFromIntersection)
    {
    DRange2d range1 = DRange2d::From(DPoint2d::From(1, 1), DPoint2d::From(7, 7));
    DRange2d range2 = DRange2d::From(DPoint2d::From(4.5, 6), DPoint2d::From(7, 7));
    range1.IsContained(range2);
    Check::True(range2.Area() < range1.Area());

    DRange2d boxIntersection;
    boxIntersection.IntersectionOf(range1, range2);
    Check::Near(boxIntersection.Area(), range2.Area());
    Check::Near(boxIntersection, range2);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    01/17
//---------------------------------------------------------------------------------------
TEST(DRange2d, FractionalPointMapping)
    {
    DRange2d rangeFractional = DRange2d::From(DPoint2d::From(2, 3), DPoint2d::From(5, 5));
    DPoint2d fractions = DPoint2d::From(0.2, 0.3);
    DPoint2d xy;
    Check::True(rangeFractional.TryFractionsToRangePoint(fractions, xy));
    DPoint2d fractions2;
    Check::True(rangeFractional.TryRangePointToFractions(xy, fractions2));
    Check::Near(fractions, fractions2);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    01/17
//---------------------------------------------------------------------------------------
TEST(DRange2d, SquaredIntersectionofRanges) 
    {
    DRange2d range1 = DRange2d::From(DPoint2d::From(2, 3), DPoint2d::From(5, 5));
    DRange2d range2 = DRange2d::From(DPoint2d::From(3, 4), DPoint2d::From(6, 5));
    DRange2d rangeExp;
    double received = range1.IntersectionExtentSquared(range2);
    rangeExp.IntersectionOf(range1, range2);
    double expected = rangeExp.ExtentSquared();
    Check::Near(expected, received);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DRange2d, RestrictRange) 
    {
    DRange2d range1 = DRange2d::From(DPoint2d::From(2, 2), DPoint2d::From(5, 5));
    DRange2d rangeLimit = DRange2d::From(DPoint2d::From(3, 3), DPoint2d::From(4, 4));

    DRange2d range;
    range.RestrictToMinMax(range1, rangeLimit);
    Check::Near(range.low, rangeLimit.low);
    Check::Near(range.high, rangeLimit.high);

    range1.InitFrom(DPoint2d::From(2.3, 4), DPoint2d::From(6, 3.9));
    printf("%f   %f    \n", range.low.x, rangeLimit.low.x);
    Check::Near(range.low.x, rangeLimit.low.x);
    printf("%f   %f    \n", range.low.y, rangeLimit.low.y);
    Check::Near(range.low.y, rangeLimit.low.y);
    printf("%f   %f    \n", range.high.x, rangeLimit.high.x);
    Check::Near(range.high.x, rangeLimit.high.x);
    printf("%f   %f    \n", range.high.y, range1.high.y);
    Check::Near(range.high.y, range1.high.y);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DRange2d, RangeArea)
    {
    DRange2d pointRange = DRange2d::From(0.2, 2.1);
    DRange2d pointRange2 = DRange2d::From(DPoint2d::From(5.5, 6.8));
    Check::True(pointRange.Area() == 0);
    Check::True(pointRange2.Area() == 0);
    Check::True(pointRange.Area() == pointRange2.Area());

    DRange2d unionRange = DRange2d::From(DPoint2d::From(0.2, 2.1), DPoint2d::From(3, 3.2), DPoint2d::From(5.5, 6.8));
    DRange2d rangeUnion;
    rangeUnion.UnionOf(pointRange, pointRange2);
    Check::Near(rangeUnion, unionRange);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DRange2d, SettingRange)
    {
    DPoint3d points3d[] = { DPoint3d::From(4, 3, 1),
                           DPoint3d::From(0.1, 0.3, 1.1),
                           DPoint3d::From(-1, 3, 1),
                           DPoint3d::From(4, -3, 1) };
    DRange2d range1 = DRange2d::From(points3d, 4);

    bvector<DPoint3d> pnts3d = { DPoint3d::From(4, 3, 1),
                                DPoint3d::From(0.1, 0.3, 1.1),
                                DPoint3d::From(-1, 3, 1),
                                DPoint3d::From(4, -3, 1) };
    DRange2d range2 = DRange2d::From(pnts3d);

    Check::Near(range1, range2);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DRange2d, RangeProjection3d)
    {
    DRange3d range3d = DRange3d::From(8, 2, 4);
    DRange2d range = DRange2d::From(range3d);
    Check::False(range.IsNull());
    Check::Near(range.XLength(), range3d.XLength());
    Check::Near(range.YLength(), range3d.YLength());
    }

void arcSweepCheck(double degree, double sweep) 
    {
    DRange2d range = DRange2d::FromUnitArcSweep(Angle::FromDegrees(degree).Radians(), Angle::FromDegrees(sweep).Radians());
        if (::fabs(sweep) >= 360)
        {
        Check::Near(range.low.x, -1);
        Check::Near(range.low.y, -1);
        Check::Near(range.high.x, 1);
        Check::Near(range.high.y, 1);
        }
    else
        {
        Check::True((range.low.x > -1) ||
                     (range.low.y > -1) ||
                     (range.high.x < 1) ||
                     (range.high.x < 1));
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DRange2d, ArcSweep)
    {
    arcSweepCheck(135, -360);
    arcSweepCheck(30, 780);
    arcSweepCheck(30, -780);
    arcSweepCheck(45, 135);
    arcSweepCheck(135, 90);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DRange2d, MaximalAxis)
    {
    DRange2d range = DRange2d::From(4, 3, 8, 9);
    Check::ExactDouble(1, range.IndexOfMaximalAxis());
    range.Extend(DPoint4d::From(15, 11, 9, 1));
    Check::ExactDouble(0, range.IndexOfMaximalAxis());
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DRange2d, CornersAndPlanes)
    {
    DRange2d range = DRange2d::From(3, 3, 8, 9);
    //Get4Lines  (DPoint2dP originArray, DPoint2dP normalArray) 
    DPoint2d corners[4];

    range.Get4Corners(corners);
    Check::Near(corners[0], DPoint2d::From(3, 3));
    Check::Near(corners[1], DPoint2d::From(8, 3));
    Check::Near(corners[2], DPoint2d::From(3, 9));
    Check::Near(corners[3], DPoint2d::From(8, 9));
    }
