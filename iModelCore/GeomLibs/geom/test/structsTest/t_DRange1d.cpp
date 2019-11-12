/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"

//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    01/17
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
// @bsimethod                                     Farhad.Kabir                    01/17
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
// @bsimethod                                     Farhad.Kabir                    01/17
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
//// @bsimethod                                     Farhad.Kabir                    01/17
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