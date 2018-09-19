/*--------------------------------------------------------------------------------------+
|
|  $Source: tests/Published/LocaleTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../FormattingTestsPch.h"
#include "../TestFixture/FormattingTestFixture.h"

USING_BENTLEY_FORMATTING

BEGIN_BENTLEY_FORMATTEST_NAMESPACE

struct LocaleTests : public ::testing::Test {};

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
TEST_F(LocaleTests, BasicTest)
    {
    NumericFormatSpec fmtP = NumericFormatSpec(NumericFormatSpec::DefaultFormat());
    fmtP.SetKeepTrailingZeroes(true);
    fmtP.SetUse1000Separator(true);

    double testV = 1000.0 * sqrt(2.0);

    // Testing American
    fmtP.ImbueLocale("en-US");
    fmtP.SetPrecision(DecimalPrecision::Precision8);
    fmtP.SetRoundingFactor(0.05);
    EXPECT_STREQ("1,414.20000000", fmtP.Format(testV).c_str());
    fmtP.SetPrecision(DecimalPrecision::Precision7);
    EXPECT_STREQ("7,071.0500000", fmtP.Format(5.0*testV).c_str());
    fmtP.SetPrecision(DecimalPrecision::Precision6);
    EXPECT_STREQ("4,242.650000", fmtP.Format(3.0*testV).c_str());
    fmtP.SetPrecision(DecimalPrecision::Precision5);
    EXPECT_STREQ("9,899.50000", fmtP.Format(7.0*testV).c_str());
    fmtP.SetPrecision(DecimalPrecision::Precision4);
    EXPECT_STREQ("12,727.9000", fmtP.Format(9.0*testV).c_str());
    fmtP.SetPrecision(DecimalPrecision::Precision3);
    EXPECT_STREQ("2,828.450", fmtP.Format(2.0*testV).c_str());

    // Testing European
    fmtP.SetPrecision(DecimalPrecision::Precision8);
    fmtP.ImbueLocale("de");
    EXPECT_STREQ("1.414,20000000", fmtP.Format(testV).c_str());
    fmtP.SetPrecision(DecimalPrecision::Precision7);
    EXPECT_STREQ("7.071,0500000", fmtP.Format(5.0*testV).c_str());
    fmtP.SetPrecision(DecimalPrecision::Precision6);
    EXPECT_STREQ("4.242,650000", fmtP.Format(3.0*testV).c_str());
    fmtP.SetPrecision(DecimalPrecision::Precision5);
    EXPECT_STREQ("9.899,50000", fmtP.Format(7.0*testV).c_str());
    fmtP.SetPrecision(DecimalPrecision::Precision4);
    EXPECT_STREQ("12.727,9000", fmtP.Format(9.0*testV).c_str());
    fmtP.SetPrecision(DecimalPrecision::Precision3);
    EXPECT_STREQ("2.828,450", fmtP.Format(2.0*testV).c_str());

    // Testing German
    fmtP.ImbueLocale("fi");
    fmtP.SetThousandSeparator(' '); // Setting european locales actually sets the thousands separator to \xA0 which is thin space which breaks non UTF8 comparison TODO
    fmtP.SetPrecision(DecimalPrecision::Precision8);
    EXPECT_STREQ("1 414,20000000", fmtP.Format(testV).c_str());
    fmtP.SetPrecision(DecimalPrecision::Precision7);
    EXPECT_STREQ("7 071,0500000", fmtP.Format(5.0*testV).c_str());
    fmtP.SetPrecision(DecimalPrecision::Precision6);
    EXPECT_STREQ("4 242,650000", fmtP.Format(3.0*testV).c_str());
    fmtP.SetPrecision(DecimalPrecision::Precision5);
    EXPECT_STREQ("9 899,50000", fmtP.Format(7.0*testV).c_str());
    fmtP.SetPrecision(DecimalPrecision::Precision4);
    EXPECT_STREQ("12 727,9000", fmtP.Format(9.0*testV).c_str());
    fmtP.SetPrecision(DecimalPrecision::Precision3);
    EXPECT_STREQ("2 828,450", fmtP.Format(2.0*testV).c_str());
    }

END_BENTLEY_FORMATTEST_NAMESPACE
