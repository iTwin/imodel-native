#include "testharness.h"


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