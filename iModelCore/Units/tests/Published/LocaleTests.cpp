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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            David.Fox-Rabinovitz                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LocaleTests, LocaleTest)
    {    
    LocaleProperties lop = LocaleProperties::DefaultAmerican();
    Json::Value jval = lop.ToJson();
    LocaleProperties lop1 = LocaleProperties(jval);
    EXPECT_EQ(lop.GetDecimalSeparator(), lop1.GetDecimalSeparator());
    EXPECT_EQ(lop.GetThousandSeparator(), lop1.GetThousandSeparator());

    lop = LocaleProperties::DefaultEuropean();
    jval = lop.ToJson();
    lop1 = LocaleProperties(jval);
    EXPECT_EQ(lop.GetDecimalSeparator(), lop1.GetDecimalSeparator());
    EXPECT_EQ(lop.GetThousandSeparator(), lop1.GetThousandSeparator());

    lop = LocaleProperties::DefaultEuropean(true);
    jval = lop.ToJson();
    lop1 = LocaleProperties(jval);
    EXPECT_EQ(lop.GetDecimalSeparator(), lop1.GetDecimalSeparator());
    EXPECT_EQ(lop.GetThousandSeparator(), lop1.GetThousandSeparator());
    }

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
    fmtP.SetDecimalPrecision(DecimalPrecision::Precision8);
    EXPECT_STREQ("1,414.20000000", fmtP.Format(testV, 0.05).c_str());
    fmtP.SetDecimalPrecision(DecimalPrecision::Precision7);
    EXPECT_STREQ("7,071.0500000", fmtP.Format(5.0*testV, 0.05).c_str());
    fmtP.SetDecimalPrecision(DecimalPrecision::Precision6);
    EXPECT_STREQ("4,242.650000", fmtP.Format(3.0*testV, 0.05).c_str());
    fmtP.SetDecimalPrecision(DecimalPrecision::Precision5);
    EXPECT_STREQ("9,899.50000", fmtP.Format(7.0*testV, 0.05).c_str());
    fmtP.SetDecimalPrecision(DecimalPrecision::Precision4);
    EXPECT_STREQ("12,727.9000", fmtP.Format(9.0*testV, 0.05).c_str());
    fmtP.SetDecimalPrecision(DecimalPrecision::Precision3);
    EXPECT_STREQ("2,828.450", fmtP.Format(2.0*testV, 0.05).c_str());

    // Testing European
    fmtP.SetDecimalPrecision(DecimalPrecision::Precision8);
    fmtP.ImbueLocaleProperties(LocaleProperties::DefaultEuropean());
    EXPECT_STREQ("1.414,20000000", fmtP.Format(testV, 0.05).c_str());
    fmtP.SetDecimalPrecision(DecimalPrecision::Precision7);
    EXPECT_STREQ("7.071,0500000", fmtP.Format(5.0*testV, 0.05).c_str());
    fmtP.SetDecimalPrecision(DecimalPrecision::Precision6);
    EXPECT_STREQ("4.242,650000", fmtP.Format(3.0*testV, 0.05).c_str());
    fmtP.SetDecimalPrecision(DecimalPrecision::Precision5);
    EXPECT_STREQ("9.899,50000", fmtP.Format(7.0*testV, 0.05).c_str());
    fmtP.SetDecimalPrecision(DecimalPrecision::Precision4);
    EXPECT_STREQ("12.727,9000", fmtP.Format(9.0*testV, 0.05).c_str());
    fmtP.SetDecimalPrecision(DecimalPrecision::Precision3);
    EXPECT_STREQ("2.828,450", fmtP.Format(2.0*testV, 0.05).c_str());

    // Testing German
    fmtP.ImbueLocaleProperties(LocaleProperties::DefaultEuropean(true));
    fmtP.SetDecimalPrecision(DecimalPrecision::Precision8);
    EXPECT_STREQ("1 414,20000000", fmtP.Format(testV, 0.05).c_str());
    fmtP.SetDecimalPrecision(DecimalPrecision::Precision7);
    EXPECT_STREQ("7 071,0500000", fmtP.Format(5.0*testV, 0.05).c_str());
    fmtP.SetDecimalPrecision(DecimalPrecision::Precision6);
    EXPECT_STREQ("4 242,650000", fmtP.Format(3.0*testV, 0.05).c_str());
    fmtP.SetDecimalPrecision(DecimalPrecision::Precision5);
    EXPECT_STREQ("9 899,50000", fmtP.Format(7.0*testV, 0.05).c_str());
    fmtP.SetDecimalPrecision(DecimalPrecision::Precision4);
    EXPECT_STREQ("12 727,9000", fmtP.Format(9.0*testV, 0.05).c_str());
    fmtP.SetDecimalPrecision(DecimalPrecision::Precision3);
    EXPECT_STREQ("2 828,450", fmtP.Format(2.0*testV, 0.05).c_str());
    }

END_BENTLEY_FORMATTEST_NAMESPACE
