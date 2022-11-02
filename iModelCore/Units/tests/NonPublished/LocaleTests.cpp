/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../FormattingTestsPch.h"
#include "../TestFixture/FormattingTestFixture.h"

USING_BENTLEY_FORMATTING

BEGIN_BENTLEY_FORMATTEST_NAMESPACE

#define CONCAT(value1, value2, value3) value1 value2 value3

struct LocaleTests : public ::testing::Test {};

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
#if !defined(BENTLEYCONFIG_OS_ANDROID) && !defined(BENTLEYCONFIG_OS_APPLE) && !defined(BENTLEYCONFIG_OS_LINUX)

    TEST_F(LocaleTests, BasicTest)
        {
        NumericFormatSpec fmtP = NumericFormatSpec(NumericFormatSpec::DefaultFormat());
        fmtP.SetKeepTrailingZeroes(true);
        fmtP.SetUse1000Separator(true);

        double testV = 1000.0 * sqrt(2.0);

        // Testing American
        #ifdef BENTLEYCONFIG_OS_UNIX
            fmtP.ImbueLocale("en_US");
        #else
            fmtP.ImbueLocale("en-US");
        #endif

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

        // Testing German
        #ifdef BENTLEYCONFIG_OS_UNIX
            fmtP.ImbueLocale("de_DE");
        #else
            fmtP.ImbueLocale("de");
        #endif

        fmtP.SetPrecision(DecimalPrecision::Precision8);
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

        // Testing Finnish
        #ifdef BENTLEYCONFIG_OS_UNIX
            fmtP.ImbueLocale("fi_FI");
        #else
            fmtP.ImbueLocale("fi");
        #endif

        fmtP.SetPrecision(DecimalPrecision::Precision8);
        EXPECT_STREQ(CONCAT("1", "\xA0", "414,20000000"), fmtP.Format(testV).c_str());
        fmtP.SetPrecision(DecimalPrecision::Precision7);
        EXPECT_STREQ(CONCAT("7", "\xA0", "071,0500000"), fmtP.Format(5.0*testV).c_str());
        fmtP.SetPrecision(DecimalPrecision::Precision6);
        EXPECT_STREQ(CONCAT("4", "\xA0", "242,650000"), fmtP.Format(3.0*testV).c_str());
        fmtP.SetPrecision(DecimalPrecision::Precision5);
        EXPECT_STREQ(CONCAT("9", "\xA0", "899,50000"), fmtP.Format(7.0*testV).c_str());
        fmtP.SetPrecision(DecimalPrecision::Precision4);
        EXPECT_STREQ(CONCAT("12", "\xA0", "727,9000"), fmtP.Format(9.0*testV).c_str());
        fmtP.SetPrecision(DecimalPrecision::Precision3);
        EXPECT_STREQ(CONCAT("2", "\xA0", "828,450"), fmtP.Format(2.0*testV).c_str());
        }

#endif

END_BENTLEY_FORMATTEST_NAMESPACE
