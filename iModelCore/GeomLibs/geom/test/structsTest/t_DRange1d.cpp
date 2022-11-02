/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(DRange1d, DefaultRange)
    {
    DRange1d infRange = DRange1d::InfiniteRange();
    Check::True(infRange.IsDoublyInfinite());
    double infLow, infHigh;

    Check::True(infRange.GetLowHigh(infLow, infHigh));
    DRange1d expLow, expHigh;
    expLow = DRange1d::ZeroAndNegativeRange();
    expHigh = DRange1d::ZeroAndPositiveRange();
    Check::ExactDouble(expLow.Low(), infLow);
    Check::True(expLow.IsInfiniteNegative());
    Check::ExactDouble(expHigh.High(), infHigh);
    Check::True(expHigh.IsInfinitePositive());
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(DRange1d, FindLength)
    {
    double arr[] = { 23.1,11.2,55.2,14.9,-88.8,12.5,76.2 };
    DRange1d dRange = DRange1d::From(arr, 7);
    Check::ExactDouble(-88.8, dRange.Low());
    Check::ExactDouble(76.2, dRange.High());
    Check::True(dRange.IsPositiveLength());
    Check::ExactDouble(88.8, dRange.MaxAbs());
    Check::Near(dRange.High() - dRange.Low(), dRange.Length());
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(DRange1d, RangeChecks)
    {
    DRange1d range1 = DRange1d::FromLowHigh(3, 11);
    Check::True(range1.IsPositiveLength());

    DRange1d range2 = DRange1d::FromLowHigh(3, 3);
    Check::True(range2.IsSinglePoint());

    DRange1d range3 = DRange1d::FromLowHigh(-3, -11);
    Check::True(range3.IsEmpty());

    range2.Extend(4);
    Check::True(range2.IsPositiveLength());

    range3.Extend(1, 11);
    Check::True(range3.IsPositiveLength());
    }

////---------------------------------------------------------------------------------------
//// @bsimethod
////---------------------------------------------------------------------------------------
TEST(DRange1d, RangeExtension) 
    {
    DRange1d rangeNull = DRange1d::NullRange();
    DRange1d rangeNull2;
    rangeNull2.InitNull();
    Check::True(rangeNull.IsNull());
    Check::True(rangeNull2.IsNull());

    double vals[] = { 23, 34,22, 11.2, 3 };
    rangeNull.Extend(vals, 5);
    Check::Near(rangeNull.high - rangeNull.low, rangeNull.Length());
    Check::False(rangeNull.IsEqualLowHigh(rangeNull2));
    }
bvector<DRange1d> MakeRanges
(
double a,       // interval size
double x0,      // lowest start
double x1,      // highest start
double b        // gap
)
    {
    bvector<DRange1d> data;
    for (double x = x0; x < x1; x += a + b)
        data.push_back (DRange1d::From (x, x + a));
    return data;
    }
void SaveRanges(bvector<DRange1d> &data, double x0, double y0, double dyTick = 0.0)
    {
    for (auto &range: data)
        {
        double xa = x0 + range.low;
        double xb = x0 + range.high;
        if (dyTick == 0.0)
        Check::SaveTransformed (DSegment3d::From (xa, y0, 0, xb, y0, 0));
        else
            Check::SaveTransformed({
                DPoint3d::From (xa, y0 + dyTick),
                DPoint3d::From(xa, y0),
                DPoint3d::From(xb, y0),
                DPoint3d::From(xb, y0 + dyTick)
                });
        }
    }
void appendCoordinates(bvector<DRange1d> const &ranges, bvector<double> &coordinates)
    {
    for (auto &range : ranges)
        {
        coordinates.push_back (range.low);
        coordinates.push_back(range.high);
        }
    }
void testDRange1dBoolean
(
void (*opFunc) (bvector<DRange1d> &dataA, bvector<DRange1d> &dataB, bvector<DRange1d> &result),
bool (*binaryOperation)(bool a, bool b)
)
    {
    double y0 = 0;
    double x0 = 0;
    double dy = 0.5;
    double dyTickC = -2.1 * dy;
    double dyTickA = 0.9 * dy;
    double delta = 0.0001;
    for (double scaleA = 1.0; scaleA < 10.0; scaleA *= 2.0)
        {
        auto dataA = MakeRanges(scaleA, 1.0, 20.0, scaleA * 0.6);
        for (double scaleB = 1.0; scaleB < 10.0; scaleB *= 1.8)
            {
            auto dataB = MakeRanges(scaleB, 1.0, 20.0, scaleB * 0.5);
            bvector<DRange1d> dataC;
            opFunc(dataA, dataB, dataC);
            SaveRanges(dataA, x0, y0, dyTickA);
            SaveRanges(dataB, x0, y0 + dy);
            SaveRanges(dataC, x0, y0 + 2 * dy, dyTickC);

            bvector<double> allCoordinates;
            appendCoordinates (dataA, allCoordinates);
            appendCoordinates (dataB, allCoordinates);
            for (double q : allCoordinates)
                {
                double q1 = q - delta;
                double q2 = q + delta;
                Check::Bool (DRange1d::AnyRangeContains(dataC, q1),
                    binaryOperation(DRange1d::AnyRangeContains (dataA, q1), DRange1d::AnyRangeContains(dataB, q1)));
                Check::Bool(DRange1d::AnyRangeContains(dataC, q2),
                    binaryOperation(DRange1d::AnyRangeContains(dataA, q2), DRange1d::AnyRangeContains(dataB, q2)));
                }
            y0 += 4 * dy;
            }
        }
    }
////---------------------------------------------------------------------------------------
//// @bsimethod
////---------------------------------------------------------------------------------------
TEST(DRange1d, ParitySorted)
    {
    testDRange1dBoolean (DRange1d::ParitySorted, DRange1d::BooleanXOR);
    Check::ClearGeometry ("DRange1d.ParitySorted");
    }

////---------------------------------------------------------------------------------------
//// @bsimethod
////---------------------------------------------------------------------------------------
TEST(DRange1d, UnionSorted)
    {
    testDRange1dBoolean(DRange1d::UnionSorted, DRange1d::BooleanOR);
    Check::ClearGeometry("DRange1d.UnionSorted");
    }

////---------------------------------------------------------------------------------------
//// @bsimethod
////---------------------------------------------------------------------------------------
TEST(DRange1d, DifferenceSorted)
    {
    testDRange1dBoolean(DRange1d::DifferenceSorted, DRange1d::BooleanANotB);
    Check::ClearGeometry("DRange1d.DifferenceSorted");
    }
////---------------------------------------------------------------------------------------
//// @bsimethod
////---------------------------------------------------------------------------------------
TEST(DRange1d, IntersectSorted)
    {
    testDRange1dBoolean(DRange1d::IntersectSorted, DRange1d::BooleanAND);
    Check::ClearGeometry("DRange1d.IntersectSorted");
    }

