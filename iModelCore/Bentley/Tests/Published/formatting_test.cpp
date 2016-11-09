/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/formatting_test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include <Bentley/Formatting.h>


TEST(FormattingTest, Simple)
{
    //ShowSignOption sigOpt = ShowSignOption::OnlyNegative;
    NumericFormat numFmt = NumericFormat();
    numFmt.SetSignOption(ShowSignOption::OnlyNegative);
    
    EXPECT_STREQ ("135", numFmt.FormatInteger(135).c_str());
    EXPECT_STREQ ("135689", numFmt.FormatInteger(135689).c_str());
    EXPECT_STREQ ("-846356", numFmt.FormatInteger(-846356).c_str());
    numFmt.SetSignOption(ShowSignOption::SignAlways);
    EXPECT_STREQ ("+135", numFmt.FormatInteger(135).c_str());
    EXPECT_STREQ ("+135689", numFmt.FormatInteger(135689).c_str());
    EXPECT_STREQ ("-846356", numFmt.FormatInteger(-846356).c_str());
    numFmt.SetSignOption(ShowSignOption::NoSign);
    EXPECT_STREQ ("135", numFmt.FormatInteger(135).c_str());
    EXPECT_STREQ ("135689", numFmt.FormatInteger(135689).c_str());
    EXPECT_STREQ ("846356", numFmt.FormatInteger(-846356).c_str());

    numFmt.SetDecimalPrecision(DecimalPrecision::Precision10);
    numFmt.SetPresentationType(PresentationType::Scientific);
    numFmt.SetSignOption(ShowSignOption::OnlyNegative);
    EXPECT_STREQ ("-0.2718281828e-2", numFmt.FormatDouble(-0.0027182818284590).c_str());
    EXPECT_STREQ ("-0.2718281828", numFmt.FormatDouble(-0.2718281828459045).c_str());
}

