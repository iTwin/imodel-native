#include "testharness.h"

//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    01/17
//---------------------------------------------------------------------------------------
TEST(DRange1d, DefaultRange)
    {
    DRange1d infRange = DRange1d::InfiniteRange();
    Check::True(infRange.IsDoublyInfinite());
    double_t infLow, infHigh;

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