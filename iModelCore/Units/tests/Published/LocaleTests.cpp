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
    LOG.infov("\n================  Locale Test ===========================");

    LocaleProperties lop = LocaleProperties::DefaultAmerican();
    Json::Value jval = lop.ToJson();
    LOG.infov("American Default %s", jval.ToString().c_str());
    LocaleProperties lop1 = LocaleProperties(jval);
    LOG.infov("American Default origin %s", lop.ToText().c_str());
    LOG.infov("American Default restor %s", lop1.ToText().c_str());

    lop = LocaleProperties::DefaultEuropean();
    jval = lop.ToJson();
    LOG.infov("European Default %s", jval.ToString().c_str());
    lop1 = LocaleProperties(jval);
    LOG.infov("European Default origin %s", lop.ToText().c_str());
    LOG.infov("European Default restor %s", lop1.ToText().c_str());

    lop = LocaleProperties::DefaultEuropean(true);
    jval = lop.ToJson();
    LOG.infov("European1 Default %s", jval.ToString().c_str());
    lop1 = LocaleProperties(jval);
    LOG.infov("European1 Default origin %s", lop.ToText().c_str());
    LOG.infov("European1 Default restor %s", lop1.ToText().c_str());

    LOG.infov("================  Locale Test (end) ===========================\n");

    LOG.infov("================  Formatting Log (completed) ===========================\n\n\n");
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
    EXPECT_STREQ("1,414.20000000", fmtP.FormatDouble(testV, 8, 0.05).c_str());
    EXPECT_STREQ("7,071.0500000", fmtP.FormatDouble(5.0*testV, 7, 0.05).c_str());
    EXPECT_STREQ("4,242.650000", fmtP.FormatDouble(3.0*testV, 6, 0.05).c_str());
    EXPECT_STREQ("9,899.50000", fmtP.FormatDouble(7.0*testV, 5, 0.05).c_str());
    EXPECT_STREQ("12,727.9000", fmtP.FormatDouble(9.0*testV, 4, 0.05).c_str());
    EXPECT_STREQ("2,828.450", fmtP.FormatDouble(2.0*testV, 3, 0.05).c_str());

    LOG.info("\n=============== Testing European ==================");
    fmtP.ImbueLocaleProperties(LocaleProperties::DefaultEuropean());

    EXPECT_STREQ("1.414,20000000", fmtP.FormatDouble(testV, 8, 0.05).c_str());
    EXPECT_STREQ("7.071,0500000", fmtP.FormatDouble(5.0*testV, 7, 0.05).c_str());
    EXPECT_STREQ("4.242,650000", fmtP.FormatDouble(3.0*testV, 6, 0.05).c_str());
    EXPECT_STREQ("9.899,50000", fmtP.FormatDouble(7.0*testV, 5, 0.05).c_str());
    EXPECT_STREQ("12.727,9000", fmtP.FormatDouble(9.0*testV, 4, 0.05).c_str());
    EXPECT_STREQ("2.828,450", fmtP.FormatDouble(2.0*testV, 3, 0.05).c_str());

    LOG.info("\n=============== Testing German ==================");
    fmtP.ImbueLocaleProperties(LocaleProperties::DefaultEuropean(true));

    EXPECT_STREQ("1 414,20000000", fmtP.FormatDouble(testV, 8, 0.05).c_str());
    EXPECT_STREQ("7 071,0500000", fmtP.FormatDouble(5.0*testV, 7, 0.05).c_str());
    EXPECT_STREQ("4 242,650000", fmtP.FormatDouble(3.0*testV, 6, 0.05).c_str());
    EXPECT_STREQ("9 899,50000", fmtP.FormatDouble(7.0*testV, 5, 0.05).c_str());
    EXPECT_STREQ("12 727,9000", fmtP.FormatDouble(9.0*testV, 4, 0.05).c_str());
    EXPECT_STREQ("2 828,450", fmtP.FormatDouble(2.0*testV, 3, 0.05).c_str());

    LOG.info("\n=============== Switching back to American ==================\n");

    fmtP.ImbueLocaleProperties(LocaleProperties::DefaultAmerican());
    }

END_BENTLEY_FORMATTEST_NAMESPACE
