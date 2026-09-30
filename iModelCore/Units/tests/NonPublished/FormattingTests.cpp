/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../FormattingTestsPch.h"
#include "../TestFixture/FormattingTestFixture.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>

USING_BENTLEY_FORMATTING
BEGIN_BENTLEY_FORMATTEST_NAMESPACE

#define CONCAT(value1, value2, value3) value1 value2 value3

struct FormatUtilsTest : FormattingTestFixture {};
struct FormatIntegerTest : FormattingTestFixture
    {
    int32_t const testValPos = 100501;
    int32_t const testValNeg = -100501;
    };

struct FormatDoubleTest : FormattingTestFixture
    {
    double const testValPos = 100501.125;
    double const testValNeg = -100501.125;
    double const testValPosZeroFracComp = 100501.0;
    double const testValNegZeroFracComp = -100501.0;
    double const testValMagnitudeLt1Pos = 0.000125;
    double const testValMagnitudeLt1Neg = -0.000125;
    };

//===================================================
// FormatParsingSet
//===================================================
struct FormatParsingSetTest : FormattingTestFixture
    {
public:

    // Test units availiable pre-looked-up for use in tests.
    BEU::UnitCP mile;
    BEU::UnitCP yard;
    BEU::UnitCP foot;
    BEU::UnitCP inch;
    BEU::UnitCP arcDeg;
    BEU::UnitCP meter;
    BEU::UnitCP millimeter;
    void SetUp() override
        {
        FormattingTestFixture::SetUp();
        ASSERT_NE(nullptr, mile = s_unitsContext->LookupUnit("MILE"));
        ASSERT_NE(nullptr, yard = s_unitsContext->LookupUnit("YRD"));
        ASSERT_NE(nullptr, foot = s_unitsContext->LookupUnit("FT"));
        ASSERT_NE(nullptr, inch = s_unitsContext->LookupUnit("IN"));
        ASSERT_NE(nullptr, arcDeg = s_unitsContext->LookupUnit("ARC_DEG"));
        ASSERT_NE(nullptr, meter = s_unitsContext->LookupUnit("M"));
        ASSERT_NE(nullptr, millimeter = s_unitsContext->LookupUnit("MM"));
        }

    Utf8String GetFmtStringErrMsg(Utf8CP fmtStr){ return Utf8String("format string: \"") + fmtStr + "\""; }

    void TestValidParseToQuantity(Utf8CP fmtStr, BEU::UnitCP expectedUnit, double const expectedMagnitude)
        {
        FormatParsingSet fps(fmtStr, expectedUnit);
        ASSERT_FALSE(fps.HasProblem()) << fps.GetProblemDescription() << '\n' << GetFmtStringErrMsg(fmtStr);

        FormatProblemCode probCode;
        Format format;
        BEU::Quantity qty = fps.GetQuantity(&probCode, &format);

        ASSERT_FALSE(qty.IsNullQuantity()) << GetFmtStringErrMsg(fmtStr);
        EXPECT_EQ(expectedUnit, qty.GetUnit()) << GetFmtStringErrMsg(fmtStr);
        EXPECT_DOUBLE_EQ(expectedMagnitude, qty.GetMagnitude()) << GetFmtStringErrMsg(fmtStr);
        }

    void TestValidParseToQuantityUsingStdFmt(Utf8CP fmtStr, BEU::UnitCP expectedUnit, double const expectedMagnitude, FormatCP format)
        {
        FormatParsingSet fps(fmtStr, expectedUnit);
        ASSERT_FALSE(fps.HasProblem()) << fps.GetProblemDescription() << '\n' << GetFmtStringErrMsg(fmtStr);

        FormatProblemCode probCode;
        BEU::Quantity qty = fps.GetQuantity(&probCode, format);

        ASSERT_FALSE(qty.IsNullQuantity()) << GetFmtStringErrMsg(fmtStr);
        EXPECT_EQ(expectedUnit, qty.GetUnit()) << GetFmtStringErrMsg(fmtStr);
        EXPECT_DOUBLE_EQ(expectedMagnitude, qty.GetMagnitude()) << GetFmtStringErrMsg(fmtStr);
        }


    void TestInvalidFmtStr(Utf8CP fmtStr, BEU::UnitCP expectedUnit)
        {
        Format format;
        NumericFormatSpec numericSpec;
        numericSpec.SetPresentationType(PresentationType::Decimal);
        format.SetNumericSpec(numericSpec);
        FormatProblemCode probCode = FormatProblemCode::NoProblems;

        FormatParsingSet fps(fmtStr, expectedUnit);
        fps.GetQuantity(&probCode, &format);
        ASSERT_EQ(probCode, FormatProblemCode::QT_InvalidSyntax) << GetFmtStringErrMsg(fmtStr);
        }
    };

struct FormatParseStringTest : FormattingTestFixture
    {
    public:
        
        void CreateFormatObject(FormatR format, Utf8CP thousandSep = ",", Utf8CP decimalSep = ".")
            {
            NumericFormatSpec nfs = NumericFormatSpec();
            nfs.SetUse1000Separator(true);
            nfs.SetThousandSeparator(*thousandSep);
            nfs.SetDecimalSeparator(*decimalSep);
            format = Format(nfs);
            ASSERT_FALSE(format.IsProblem()) << "Format Initialization Error: " << format.GetProblemDescription().c_str();
            }

        void ParseStringToDouble(Utf8CP input, FormatCP expectedFormat, double const expectedMagnitude)
            {
            FormatParsingSet fps(input, s_unitsContext->LookupUnit("MM"), expectedFormat);
            ASSERT_FALSE(fps.HasProblem()) << fps.GetProblemDescription();

            FormatProblemCode probCode;
            BEU::Quantity qty = fps.GetQuantity(&probCode, expectedFormat);
            ASSERT_FALSE(qty.IsNullQuantity()) << "Parsed output of input \"" << input << "\" is unexpectedly null";
            EXPECT_DOUBLE_EQ(expectedMagnitude, qty.GetMagnitude()) << "Parsed output of input \"" << input << "\" does not match expected value " << expectedMagnitude;
            }

    };

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatUtilsTest, GetAvailableStrings)
    {
    SignOption signOption;
    auto options = UIUtils::GetAvailableSignOption(signOption);
    EXPECT_EQ(SignOption::OnlyNegative, signOption);
    EXPECT_EQ(4, options.size());

    PresentationType type;
    auto types = UIUtils::GetAvailablePresentationTypes(type);
    EXPECT_EQ(PresentationType::Decimal, type);
    EXPECT_EQ(4, types.size());

    FormatTraits trait;
    auto traits = UIUtils::GetAvailableTraits(trait);
    EXPECT_EQ(FormatTraits::None, trait);
    EXPECT_EQ(10, traits.size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatIntegerTest, SignOptionTests)
    {
    NumericFormatSpec nfs;

    // NoSign
    {
    nfs.SetSignOption(SignOption::NoSign);
    EXPECT_STREQ("42", nfs.Format(42).c_str());
    EXPECT_STREQ("42", nfs.Format(-42).c_str());
    }

    // OnlyNegative
    {
    nfs.SetSignOption(SignOption::OnlyNegative);
    EXPECT_STREQ("42", nfs.Format(42).c_str());
    EXPECT_STREQ("-42", nfs.Format(-42).c_str());
    }

    // SignAlways
    {
    nfs.SetSignOption(SignOption::SignAlways);
    EXPECT_STREQ("+42", nfs.Format(42).c_str());
    EXPECT_STREQ("-42", nfs.Format(-42).c_str());
    }

    // NegativeParentheses
    {
    nfs.SetSignOption(SignOption::NegativeParentheses);
    EXPECT_STREQ("42", nfs.Format(42).c_str());
    EXPECT_STREQ("(42)", nfs.Format(-42).c_str());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatIntegerTest, ThousandsSeparatorTests)
    {
    NumericFormatSpec nfs;
    nfs.SetUse1000Separator(true);
    Utf8String string;

    nfs.SetThousandSeparator(',');
    EXPECT_STREQ("1,000", nfs.Format(1000).c_str());
    EXPECT_STREQ("-1,000", nfs.Format(-1000).c_str());

    nfs.SetThousandSeparator('.');
    EXPECT_STREQ("1.000", nfs.Format(1000).c_str());
    EXPECT_STREQ("-1.000", nfs.Format(-1000).c_str());

    nfs.SetThousandSeparator('\t'); // I mean technically the API doesn't prevent it right?
    EXPECT_STREQ("1\t000", nfs.Format(1000).c_str());
    EXPECT_STREQ("-1\t000", nfs.Format(-1000).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatIntegerTest, PresentationTypeTests)
    {
    // Decimal
    {
    NumericFormatSpec nfs;
    nfs.SetPresentationType(PresentationType::Decimal);
    EXPECT_STREQ("100501", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-100501", nfs.Format(testValNeg).c_str());
    }

    // Fractional
    {
    NumericFormatSpec nfs;
    nfs.SetPresentationType(PresentationType::Fractional);
    EXPECT_STREQ("100501", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-100501", nfs.Format(testValNeg).c_str());
    }

    {
    NumericFormatSpec nfs;
    nfs.SetPresentationType(PresentationType::Scientific);
    nfs.SetPrecision(DecimalPrecision::Max);
    EXPECT_STREQ("1.00501e+5", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-1.00501e+5", nfs.Format(testValNeg).c_str());
    }

    // ScientificNorm
    {
    NumericFormatSpec nfs;
    nfs.SetPresentationType(PresentationType::Scientific);
    nfs.SetScientificType(ScientificType::ZeroNormalized);
    nfs.SetPrecision(DecimalPrecision::Max);
    EXPECT_STREQ("0.100501e+6", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-0.100501e+6", nfs.Format(testValNeg).c_str());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatIntegerTest, IndividualFormatTraitsTests)
    {
    NumericFormatSpec nfs;
    Utf8String string;
    int const testValPos = 100501;
    int const testValNeg = -100501;

    nfs.SetFormatTraits(FormatConstant::DefaultFormatTraits());
    EXPECT_STREQ("0", nfs.Format(0).c_str());
    EXPECT_STREQ("100501", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-100501", nfs.Format(testValNeg).c_str());

    nfs.SetMinWidth(10);
    EXPECT_STREQ("0000000000", nfs.Format(0).c_str());
    EXPECT_STREQ("0000100501", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-0000100501", nfs.Format(testValNeg).c_str());
    nfs.SetMinWidth(0);

    // Trailing zeroes only affect integers
    nfs.SetMinWidth(10);
    nfs.SetFormatTraits(FormatTraits::TrailingZeroes);
    EXPECT_STREQ("0000000000", nfs.Format(0).c_str());
    EXPECT_STREQ("0000100501", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-0000100501", nfs.Format(testValNeg).c_str());
    nfs.SetMinWidth(0);

    // Should only affect doubles.
    nfs.SetFormatTraits(FormatTraits::KeepDecimalPoint);
    EXPECT_STREQ("0", nfs.Format(0).c_str());
    EXPECT_STREQ("100501", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-100501", nfs.Format(testValNeg).c_str());

    // Should only affect doubles.
    nfs.SetFormatTraits(FormatTraits::KeepSingleZero);
    EXPECT_STREQ("0", nfs.Format(0).c_str());
    EXPECT_STREQ("100501", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-100501", nfs.Format(testValNeg).c_str());

    nfs.SetPresentationType(PresentationType::Scientific);
    EXPECT_STREQ("0.0e+0", nfs.Format(0).c_str());
    EXPECT_STREQ("1.00501e+5", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-1.00501e+5", nfs.Format(testValNeg).c_str());
    nfs.SetPresentationType(PresentationType::Decimal);

    nfs.SetFormatTraits(FormatTraits::ZeroEmpty);
    EXPECT_STREQ("", nfs.Format(0).c_str());
    EXPECT_STREQ("100501", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-100501", nfs.Format(testValNeg).c_str());

    nfs.SetFormatTraits(FormatTraits::Use1000Separator);
    EXPECT_STREQ("0", nfs.Format(0).c_str());
    EXPECT_STREQ("100,501", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-100,501", nfs.Format(testValNeg).c_str());

    // Should only affect doubles.
    nfs.SetFormatTraits(FormatTraits::ApplyRounding);
    EXPECT_STREQ("0", nfs.Format(0).c_str());
    EXPECT_STREQ("100501", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-100501", nfs.Format(testValNeg).c_str());

    // Should only affect doubles.
    nfs.SetFormatTraits(FormatTraits::FractionDash);
    EXPECT_STREQ("0", nfs.Format(0).c_str());
    EXPECT_STREQ("100501", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-100501", nfs.Format(testValNeg).c_str());

    // AppendUnitName should have no effect on the unitless Format.
    nfs.SetFormatTraits(FormatTraits::ShowUnitLabel);
    EXPECT_STREQ("0", nfs.Format(0).c_str());
    EXPECT_STREQ("100501", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-100501", nfs.Format(testValNeg).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatIntegerTest, MaxInt)
    {
    NumericFormatSpec nfs;
    nfs.SetUse1000Separator(true);

    int const intMax = INT_MAX;
    int const intMin = -intMax; //INT_MIN;    
    // INT_MIN doesn't work because the FormatInt function negates the value (i.e. -INT_MIN)
    // This can't be represented and just ends up being INT_MIN again

    nfs.SetThousandSeparator(',');
    EXPECT_STREQ("2,147,483,647", nfs.Format(intMax).c_str());
    EXPECT_STREQ("-2,147,483,647", nfs.Format(intMin).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatIntegerTest, MinWidthTests)
    {
    NumericFormatSpec nfs;

    nfs.SetMinWidth(1);
    EXPECT_STREQ("100501", nfs.Format(testValPos).c_str());

    nfs.SetMinWidth(6);
    EXPECT_STREQ("100501", nfs.Format(testValPos).c_str());

    nfs.SetMinWidth(10);
    EXPECT_STREQ("0000100501", nfs.Format(testValPos).c_str());

    nfs.SetMinWidth(63);
    EXPECT_STREQ("000000000000000000000000000000000000000000000000000000000100501", nfs.Format(testValPos).c_str());

    nfs.SetMinWidth(100); // maxed out
    EXPECT_STREQ("000000000000000000000000000000000000000000000000000000000100501", nfs.Format(testValPos).c_str());


    // Min width includes separator characters
    nfs.SetUse1000Separator(true);
    nfs.SetThousandSeparator(',');

    nfs.SetMinWidth(1);
    EXPECT_STREQ("100,501", nfs.Format(testValPos).c_str());

    nfs.SetMinWidth(7);
    EXPECT_STREQ("100,501", nfs.Format(testValPos).c_str());

    nfs.SetMinWidth(10);
    EXPECT_STREQ("000100,501", nfs.Format(testValPos).c_str());

    nfs.SetMinWidth(63);
    EXPECT_STREQ("00000000000000000000000000000000000000000000000000000000100,501", nfs.Format(testValPos).c_str());

    nfs.SetMinWidth(100); // maxed out
    EXPECT_STREQ("00000000000000000000000000000000000000000000000000000000100,501", nfs.Format(testValPos).c_str());


    // Min width does not include sign characters
    nfs.SetSignOption(SignOption::SignAlways);

    nfs.SetMinWidth(1);
    EXPECT_STREQ("+100,501", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-100,501", nfs.Format(testValNeg).c_str());

    nfs.SetMinWidth(7);
    EXPECT_STREQ("+100,501", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-100,501", nfs.Format(testValNeg).c_str());

    nfs.SetMinWidth(10);
    EXPECT_STREQ("+000100,501", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-000100,501", nfs.Format(testValNeg).c_str());

    nfs.SetMinWidth(62);
    EXPECT_STREQ("+0000000000000000000000000000000000000000000000000000000100,501", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-0000000000000000000000000000000000000000000000000000000100,501", nfs.Format(testValNeg).c_str());

    nfs.SetMinWidth(100); // maxed out
    EXPECT_STREQ("+0000000000000000000000000000000000000000000000000000000100,501", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-0000000000000000000000000000000000000000000000000000000100,501", nfs.Format(testValNeg).c_str());


    // Min width does not include parentheses
    nfs.SetSignOption(SignOption::NegativeParentheses);

    nfs.SetMinWidth(1);
    EXPECT_STREQ("(100,501)", nfs.Format(testValNeg).c_str());

    nfs.SetMinWidth(7);
    EXPECT_STREQ("(100,501)", nfs.Format(testValNeg).c_str());

    nfs.SetMinWidth(10);
    EXPECT_STREQ("(000100,501)", nfs.Format(testValNeg).c_str());

    nfs.SetMinWidth(61);
    EXPECT_STREQ("(000000000000000000000000000000000000000000000000000000100,501)", nfs.Format(testValNeg).c_str());

    nfs.SetMinWidth(100); // maxed out
    EXPECT_STREQ("(000000000000000000000000000000000000000000000000000000100,501)", nfs.Format(testValNeg).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatDoubleTest, FormatDoubleSignOptionTests)
    {
    NumericFormatSpec nfs;
    Utf8String string;

    // NoSign
    {
    nfs.SetSignOption(SignOption::NoSign);

    EXPECT_STREQ("101.25", nfs.Format(101.25).c_str());
    EXPECT_STREQ("101.25", nfs.Format(-101.25).c_str());
    }

    // OnlyNegative
    {
    nfs.SetSignOption(SignOption::OnlyNegative);

    EXPECT_STREQ("101.25", nfs.Format(101.25).c_str());
    EXPECT_STREQ("-101.25", nfs.Format(-101.25).c_str());
    }

    // SignAlways
    {
    nfs.SetSignOption(SignOption::SignAlways);

    EXPECT_STREQ("+101.25", nfs.Format(101.25).c_str());
    EXPECT_STREQ("-101.25", nfs.Format(-101.25).c_str());
    }

    // NegativeParentheses
    {
    nfs.SetSignOption(SignOption::NegativeParentheses);

    EXPECT_STREQ("101.25", nfs.Format(101.25).c_str());
    EXPECT_STREQ("(101.25)", nfs.Format(-101.25).c_str());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatDoubleTest, FormatDoubleThousandsSeparatorTests)
    {
    NumericFormatSpec nfs;
    nfs.SetUse1000Separator(true);
    Utf8String string;

    nfs.SetThousandSeparator(',');
    EXPECT_STREQ("1,000.55", nfs.Format(1000.55).c_str());
    EXPECT_STREQ("-1,000.55", nfs.Format(-1000.55).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatDoubleTest, FormatDoubleDecimalSeparatorTests)
    {
    NumericFormatSpec nfs;
    Utf8String string;

    nfs.SetDecimalSeparator('-');
    EXPECT_STREQ("1000-55", nfs.Format(1000.55).c_str());
    EXPECT_STREQ("-1000-55", nfs.Format(-1000.55).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatDoubleTest, FormatDoublePresentationTypeTests)
    {
    Utf8String string;
    double const testValPos = 100501.125;
    double const testValNeg = -100501.125;
    double const testValMagnitudeLt1Pos = 0.000125;
    double const testValMagnitudeLt1Neg = -0.000125;

    // Decimal
    {
    NumericFormatSpec nfs;
    nfs.SetPresentationType(PresentationType::Decimal);
    nfs.SetPrecision(DecimalPrecision::Max);

    EXPECT_STREQ("100501.125", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-100501.125", nfs.Format(testValNeg).c_str());
    }

    // Fractional
    {
    NumericFormatSpec nfs;
    nfs.SetPresentationType(PresentationType::Fractional);
    nfs.SetPrecision(DecimalPrecision::Max);

    EXPECT_STREQ("100501 1/8", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-100501 1/8", nfs.Format(testValNeg).c_str());
    }

    // Scientific
    {
    NumericFormatSpec nfs;
    nfs.SetPresentationType(PresentationType::Scientific);
    nfs.SetPrecision(DecimalPrecision::Max);

    EXPECT_STREQ("1.00501125e+5", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-1.00501125e+5", nfs.Format(testValNeg).c_str());
    }

    // ScientificNorm
    {
    NumericFormatSpec nfs;
    nfs.SetPresentationType(PresentationType::Scientific);
    nfs.SetScientificType(ScientificType::ZeroNormalized);
    nfs.SetPrecision(DecimalPrecision::Max);

    EXPECT_STREQ("0.100501125e+6", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-0.100501125e+6", nfs.Format(testValNeg).c_str());
    }

    // TODO Scientific & ScientificNorm incorrectly format values with magnitude less than 1.
    // At the time of writing this comment, values with magnitude less than 1 are formatted
    // like ScientificNorm when the presentation type is Scientific, and are formatted like
    // Scientific when the value is ScientificNorm.

    // Scientific
    {
    NumericFormatSpec nfs;
    nfs.SetPresentationType(PresentationType::Scientific);
    nfs.SetPrecision(DecimalPrecision::Max);

    EXPECT_STREQ("1.25e-4", nfs.Format(testValMagnitudeLt1Pos).c_str());
    EXPECT_STREQ("-1.25e-4", nfs.Format(testValMagnitudeLt1Neg).c_str());
    EXPECT_STREQ("1.25e-3", nfs.Format(0.00125).c_str());
    EXPECT_STREQ("1.25e-2", nfs.Format(0.0125).c_str());
    EXPECT_STREQ("1.25e-1", nfs.Format(0.125).c_str());
    EXPECT_STREQ("1.25e+0", nfs.Format(1.25).c_str());
    }

    // ScientificNorm
    {
    NumericFormatSpec nfs;
    nfs.SetPresentationType(PresentationType::Scientific);
    nfs.SetScientificType(ScientificType::ZeroNormalized);
    nfs.SetPrecision(DecimalPrecision::Max);

    EXPECT_STREQ("0.125e-3", nfs.Format(testValMagnitudeLt1Pos).c_str());
    EXPECT_STREQ("-0.125e-3", nfs.Format(testValMagnitudeLt1Neg).c_str());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatDoubleTest, FormatDoubleWithDifferentMinWidths)
    {
    NumericFormatSpec nfs(NumericFormatSpec::DefaultFormat());

    nfs.SetMinWidth(0);
    EXPECT_STREQ("1000.0", nfs.Format(double(1000)).c_str());

    nfs.SetMinWidth(6);
    EXPECT_STREQ("1000.0", nfs.Format(double(1000)).c_str());

    nfs.SetMinWidth(8);
    EXPECT_STREQ("001000.0", nfs.Format(double(1000)).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatDoubleTest, FormatDoubleIndividualFormatTraitsTests)
    {
    {
    NumericFormatSpec nfs;
    nfs.SetFormatTraits(FormatConstant::DefaultFormatTraits());
    EXPECT_STREQ("0", nfs.Format((double) 0).c_str());
    EXPECT_STREQ("100501.125", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-100501.125", nfs.Format(testValNeg).c_str());
    EXPECT_STREQ("0", nfs.Format((double) 0).c_str());
    EXPECT_STREQ("100501", nfs.Format(testValPosZeroFracComp).c_str());
    EXPECT_STREQ("-100501", nfs.Format(testValNegZeroFracComp).c_str());
    }

    {
    NumericFormatSpec nfs;
    nfs.SetFormatTraits(FormatTraits::KeepDecimalPoint);
    EXPECT_STREQ("0.", nfs.Format((double) 0).c_str());
    EXPECT_STREQ("100501.", nfs.Format(testValPosZeroFracComp).c_str());
    EXPECT_STREQ("-100501.", nfs.Format(testValNegZeroFracComp).c_str());
    }

    {
    NumericFormatSpec nfs;
    nfs.SetFormatTraits(FormatTraits::KeepSingleZero);
    EXPECT_STREQ("0.0", nfs.Format((double) 0).c_str());
    EXPECT_STREQ("100501.0", nfs.Format(testValPosZeroFracComp).c_str());
    EXPECT_STREQ("-100501.0", nfs.Format(testValNegZeroFracComp).c_str());
    }

    {
    NumericFormatSpec nfs;
    nfs.SetFormatTraits(FormatTraits::Use1000Separator);
    EXPECT_STREQ("0", nfs.Format((double) 0).c_str());
    EXPECT_STREQ("100,501.125", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-100,501.125", nfs.Format(testValNeg).c_str());
    }

    {
    NumericFormatSpec nfs;
    // FormatDouble with a rounding factor X should produce formatted string
    // output O such that that X "fits" into the double representation of
    // O evenly.
    nfs.SetFormatTraits(FormatTraits::ApplyRounding);
    nfs.SetRoundingFactor(2);
    EXPECT_STREQ("0", nfs.Format((double) 0).c_str());
    EXPECT_STREQ("100502", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-100502", nfs.Format(testValNeg).c_str());
    nfs.SetRoundingFactor(500);
    EXPECT_STREQ("0", nfs.Format((double) 0).c_str());
    EXPECT_STREQ("100500", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-100500", nfs.Format(testValNeg).c_str());
    nfs.SetRoundingFactor(0.5);
    EXPECT_STREQ("0", nfs.Format((double) 0).c_str());
    EXPECT_STREQ("100501", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-100501", nfs.Format(testValNeg).c_str());
    nfs.SetRoundingFactor(0.25);
    EXPECT_STREQ("0", nfs.Format((double) 0).c_str());
    EXPECT_STREQ("100501.25", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-100501.25", nfs.Format(testValNeg).c_str());
    nfs.SetPrecision(DecimalPrecision::Precision6);
    nfs.SetRoundingFactor(0.1);
    EXPECT_STREQ("0", nfs.Format((double) 0).c_str());
    EXPECT_STREQ("100501.1", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-100501.1", nfs.Format(testValNeg).c_str());
    nfs.SetRoundingFactor(0.1);
    EXPECT_STREQ("0", nfs.Format((double) 0).c_str());
    EXPECT_STREQ("0.5", nfs.Format(0.54).c_str());
    EXPECT_STREQ("-0.5", nfs.Format(-0.54).c_str());
    }

    {
    NumericFormatSpec nfs;
    // AppendUnitName should have no effect on the unitless Format.
    nfs.SetFormatTraits(FormatTraits::ShowUnitLabel);
    EXPECT_STREQ("0", nfs.Format(0).c_str());
    EXPECT_STREQ("100501.125", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-100501.125", nfs.Format(testValNeg).c_str());
    }

    {
    NumericFormatSpec nfs;
    nfs.SetMinWidth(15);
    EXPECT_STREQ("000000000000000", nfs.Format((double) 0).c_str());
    EXPECT_STREQ("00000100501.125", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-00000100501.125", nfs.Format(testValNeg).c_str());

    nfs.SetMinWidth(0);
    EXPECT_STREQ("0", nfs.Format((double) 0).c_str());
    EXPECT_STREQ("100501.125", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-100501.125", nfs.Format(testValNeg).c_str());
    }

    {
    NumericFormatSpec nfs;
    nfs.SetFormatTraits(FormatTraits::TrailingZeroes);

    nfs.SetMinWidth(15);
    EXPECT_STREQ("00000000.000000", nfs.Format((double) 0).c_str());
    EXPECT_STREQ("00100501.125000", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-00100501.125000", nfs.Format(testValNeg).c_str());

    nfs.SetMinWidth(0);
    EXPECT_STREQ("0.000000", nfs.Format((double)0).c_str());
    EXPECT_STREQ("100501.125000", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-100501.125000", nfs.Format(testValNeg).c_str());
    }

    {
    NumericFormatSpec nfs;
    nfs.SetPresentationType(PresentationType::Scientific);
    nfs.SetPrecision(DecimalPrecision::Max);
    EXPECT_STREQ("0e+0", nfs.Format((double) 0).c_str());
    EXPECT_STREQ("1.00501125e+5", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-1.00501125e+5", nfs.Format(testValNeg).c_str());
    }

    {
    NumericFormatSpec nfs;
    nfs.SetFormatTraits(FormatTraits::ZeroEmpty);
    EXPECT_STREQ("", nfs.Format((double) 0).c_str());
    EXPECT_STREQ("100501.125", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-100501.125", nfs.Format(testValNeg).c_str());
    }

    {
    NumericFormatSpec nfs;
    nfs.SetPresentationType(PresentationType::Fractional);
    nfs.SetFormatTraits(FormatTraits::FractionDash);
    EXPECT_STREQ("0", nfs.Format((double) 0).c_str());
    EXPECT_STREQ("100501-1/8", nfs.Format(testValPos).c_str());
    EXPECT_STREQ("-100501-1/8", nfs.Format(testValNeg).c_str());
    }

    }

TEST_F(FormatDoubleTest, ValuesThatPrintOutAsZeroDoNotShowSign)
    {
    NumericFormatSpec nfs(NumericFormatSpec::DefaultFormat());

    nfs.SetSignOption(SignOption::OnlyNegative);
    EXPECT_STREQ("0.0", nfs.Format(0.00000000000001).c_str());
    EXPECT_STREQ("0.0", nfs.Format(-0.00000000000001).c_str());

    nfs.SetSignOption(SignOption::NegativeParentheses);
    EXPECT_STREQ("0.0", nfs.Format(0.00000000000001).c_str());
    EXPECT_STREQ("0.0", nfs.Format(-0.00000000000001).c_str());

    nfs.SetSignOption(SignOption::SignAlways);
    EXPECT_STREQ("0.0", nfs.Format(0.00000000000001).c_str());
    EXPECT_STREQ("0.0", nfs.Format(-0.00000000000001).c_str());

    nfs.SetSignOption(SignOption::NoSign);
    EXPECT_STREQ("0.0", nfs.Format(0.00000000000001).c_str());
    EXPECT_STREQ("0.0", nfs.Format(-0.00000000000001).c_str());

    nfs.SetPrecision(DecimalPrecision::Precision6);
    nfs.SetSignOption(SignOption::OnlyNegative);
    EXPECT_STREQ("-0.000001", nfs.Format(-0.000001).c_str());
    EXPECT_STREQ("0.0",       nfs.Format(-0.0000001).c_str());
    EXPECT_STREQ("-0.000001", nfs.Format(-0.0000005).c_str());
    EXPECT_STREQ("0.0",       nfs.Format(-0.0000004999999).c_str());

    nfs.SetPrecision(DecimalPrecision::Precision0);
    EXPECT_STREQ("-1.0", nfs.Format(-1.0).c_str());
    EXPECT_STREQ("0.0",  nfs.Format(-0.4999999).c_str());
    EXPECT_STREQ("-1.0", nfs.Format(-0.5).c_str());
    EXPECT_STREQ("0.0",  nfs.Format(-0.1).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatParsingSetTest, IdentityWholeNumber)
    {
    TestValidParseToQuantity("1 MILE", mile, 1.0);
    TestValidParseToQuantity("3 FT", foot, 3.0);
    TestValidParseToQuantity("500 IN", inch, 500.0);

    TestValidParseToQuantity("-1", mile, -1.0);
    TestValidParseToQuantity("-3", foot, -3.0);
    TestValidParseToQuantity("-500", inch, -500.0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatParsingSetTest, IdentityRationalNumber)
    {
    TestValidParseToQuantity("1.5 MILE", mile, 1.5);
    TestValidParseToQuantity("3.14159 FT", foot, 3.14159);
    TestValidParseToQuantity("500.00125 IN", inch, 500.00125);

    TestValidParseToQuantity("-1.5 MILE", mile, -1.5);
    TestValidParseToQuantity("-3.14159 FT", foot, -3.14159);
    TestValidParseToQuantity("-500.00125 IN", inch, -500.00125);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatParsingSetTest, IdentityAlwaysLeadingPlusAndMinus)
    {
    TestValidParseToQuantity("+3 FT", foot, 3.0);
    TestValidParseToQuantity("-3 FT", foot, -3.0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatParsingSetTest, IdentityIncorrectLeadingPlusAndMinus)
    {
    TestInvalidFmtStr("+ 3 FT", foot);
    TestInvalidFmtStr("- 3 FT", foot);
    TestInvalidFmtStr("++3 FT", foot);
    TestInvalidFmtStr("--3 FT", foot);
    TestInvalidFmtStr("+-3 FT", foot);
    TestInvalidFmtStr("-+3 FT", foot);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatParsingSetTest, SingleUnitConvertedToAnotherUnit)
    {
    TestValidParseToQuantity("1 MILE", yard, 1760);
    TestValidParseToQuantity("1 MILE", foot, 5280);
    TestValidParseToQuantity("1 MILE", inch, 63360);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatParsingSetTest, SingleUnitFraction)
    {
    TestValidParseToQuantity("1/2", foot, 1.0/2);
    TestValidParseToQuantity("1/2 FT", foot, 1.0/2);
    TestValidParseToQuantity("-1/2", foot, -1.0/2);
    TestValidParseToQuantity("-1/2 FT", foot, -1.0/2);

    TestValidParseToQuantity("2/4", foot, 1.0/2);
    TestValidParseToQuantity("2/4 FT", foot, 1.0/2);

    TestValidParseToQuantity("1 1/2", foot, 1.5);
    TestValidParseToQuantity("1\t1/2", foot, 1.5);
    TestValidParseToQuantity("1  1/2", foot, 1.5);
    TestValidParseToQuantity("1 \t 1/2", foot, 1.5);
    TestValidParseToQuantity("1 1/2 FT", foot, 1.5);
    TestValidParseToQuantity("1 1/2 FT", foot, 1.5);

    // Some more fractions.
    TestValidParseToQuantity("1/3", foot, 1.0/3);
    TestValidParseToQuantity("1/4", foot, 1.0/4);
    TestValidParseToQuantity("1/5", foot, 1.0/5);
    TestValidParseToQuantity("1/8", foot, 1.0/8);
    TestValidParseToQuantity("1/16", foot, 1.0/16);
    TestValidParseToQuantity("1/32", foot, 1.0/32);
    TestValidParseToQuantity("1/64", foot, 1.0/64);
    TestValidParseToQuantity("1/128", foot, 1.0/128);
    TestValidParseToQuantity("1/256", foot, 1.0/256);
    TestValidParseToQuantity("4/2", foot, 2.0);
    TestValidParseToQuantity("4/2 FT", foot, 2.0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatParsingSetTest, FractionCanUseUnderscoreBetweenIntegerAndFractionalComponent)
    {
    TestValidParseToQuantity("1_1/2 FT", foot, 1.5);
    TestValidParseToQuantity("-1_1/2 FT", foot, -1.5);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatParsingSetTest, SingleUnitWithVaryingUnitSeperators)
    {
    TestValidParseToQuantity("42.125 FT", foot, 42.125);
    TestValidParseToQuantity("42.125_FT", foot, 42.125);
    TestValidParseToQuantity("42.125_FT", foot, 42.125);
    TestValidParseToQuantity("42 1/8_FT", foot, 42.125);
    TestValidParseToQuantity("42.125FT", foot, 42.125);
    TestValidParseToQuantity("42 1/8FT", foot, 42.125);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatParsingSetTest, CompositeFormats)
    {
    TestValidParseToQuantity("2 FT 6 IN", foot, 2*1  + 6.0/12.0);
    TestValidParseToQuantity("2 FT 6 IN", inch, 2*12 + 6*1);

    TestValidParseToQuantity("3 YRD 2 FT 6 IN", yard, 3*1  + 2.0/3.0 + 6.0/36.0);
    TestValidParseToQuantity("3 YRD 2 FT 6 IN", foot, 3*3  + 2*1     + 6.0/12.0);
    TestValidParseToQuantity("3 YRD 2 FT 6 IN", inch, 3*36 + 2*12    + 6*1);

    // Make sure composite formats with fractional/decimal components work.
    TestValidParseToQuantity("2.5 FT 6 IN", inch, 2.5*12 + 6*1);

    // Whitespace shouldn't matter between the numeric and unit portions of a unit.
    TestValidParseToQuantity("2FT 6IN", inch, 2*12 + 6*1);
    TestValidParseToQuantity("2          FT 6     IN", inch, 2*12 + 6*1);

    // Whitespace shouldn't matter between units.
    TestValidParseToQuantity("2FT6IN", inch, 2*12 + 6*1);
    TestValidParseToQuantity("2 FT6 IN", inch, 2*12 + 6*1);
    TestValidParseToQuantity("2 FT        6 IN", inch, 2*12 + 6*1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatParsingSetTest, CompositeFormats_IGNORED)
    {
    TestValidParseToQuantity(Utf8Chars(u8"135°11'30 1/4\" "), arcDeg, 135.191736);

    TestValidParseToQuantity("1 MILE 3 YRD 2 FT 6 IN", mile, 1*1     + 3.0/1760.0 + 2.0/5280.0 + 6.0/63360.0);
    TestValidParseToQuantity("1 MILE 3 YRD 2 FT 6 IN", yard, 1*1760  + 3*1        + 2.0/3.0    + 6.0/36.0);
    TestValidParseToQuantity("1 MILE 3 YRD 2 FT 6 IN", foot, 1*5280  + 3*3        + 2*1        + 6.0/12.0);
    TestValidParseToQuantity("1 MILE 3 YRD 2 FT 6 IN", inch, 1*63360 + 3*36       + 2*12       + 6*1);

    TestValidParseToQuantity("2 1/2 FT 6 IN", inch, 2.5*12 + 6*1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatParsingSetTest, OverflowNumberTest)
    {
    TestValidParseToQuantity("812345678910111 m", meter, 812345678910111);
    Format format;
    FormatParsingSet fps("8123456789101110 m", meter, &format);
    
    fps.GetQuantity(nullptr, &format);
    EXPECT_TRUE(fps.HasProblem());
    EXPECT_TRUE(fps.GetProblemCode() == FormatProblemCode::TooManyDigits);

    TestValidParseToQuantity("0.012345678910111 m", meter, 0.01234567891011101);
    fps = FormatParsingSet("0.0123456789101112 m", meter);
    fps.GetQuantity(nullptr, &format);
    EXPECT_TRUE(fps.HasProblem()) << "Should be overflow because the number of digits in the floating part is more than 15";
    EXPECT_TRUE(fps.GetProblemCode() == FormatProblemCode::TooManyDigits);

    TestValidParseToQuantity("0.000000000000001 m", meter, 0.000000000000001);
    fps = FormatParsingSet("0.0123456789101112 m", meter);
    fps.GetQuantity(nullptr, &format);
    EXPECT_TRUE(fps.HasProblem()) << "Should be overflow because the number of digits in the floating part is more than 15";
    EXPECT_TRUE(fps.GetProblemCode() == FormatProblemCode::TooManyDigits);

    TestValidParseToQuantity("123456789101112.123456789101112 m", meter, 123456789101112.123456789101112);
    fps = FormatParsingSet("123456789101112134.123456789101112 m", meter);
    fps.GetQuantity(nullptr, &format);
    EXPECT_TRUE(fps.HasProblem()) << "Should be overflow because the number of digits in the integer part is more than 15";
    EXPECT_TRUE(fps.GetProblemCode() == FormatProblemCode::TooManyDigits);

    TestValidParseToQuantity("223,372,036,854,775 m", millimeter, 2.2337203685477501e+17);
    fps = FormatParsingSet("9,223,372,036,854,775 m", meter);
    fps.GetQuantity(nullptr, &format);
    EXPECT_TRUE(fps.HasProblem()) << "Should be overflow because the number of digits in the integer part is more than 15";
    EXPECT_TRUE(fps.GetProblemCode() == FormatProblemCode::TooManyDigits);

    TestValidParseToQuantity("1/223372036854775 m", meter, 4.4768361075121889e-15);
    fps = FormatParsingSet("1/9223372036854775 m", meter);
    fps.GetQuantity(nullptr, &format);
    EXPECT_TRUE(fps.HasProblem()) << "Should be overflow because the number of digits in the denominator part is more than 15";
    EXPECT_TRUE(fps.GetProblemCode() == FormatProblemCode::TooManyDigits);

    TestValidParseToQuantity("1/999999999999999 m", meter, 1.0000000000000011e-15);
    fps = FormatParsingSet("1/9999999999999999 m", meter);
    fps.GetQuantity(nullptr, &format);
    EXPECT_TRUE(fps.HasProblem()) << "Should be overflow because the number of digits in the denominator part is more than 15";
    EXPECT_TRUE(fps.GetProblemCode() == FormatProblemCode::TooManyDigits);

    TestValidParseToQuantity("100000000000000/1 m", meter, 100000000000000);
    fps = FormatParsingSet("1000000000000000/1 m", meter);
    fps.GetQuantity(nullptr, &format);
    EXPECT_TRUE(fps.HasProblem()) << "Should be overflow because the number of digits in the nominator part is more than 15";
    EXPECT_TRUE(fps.GetProblemCode() == FormatProblemCode::TooManyDigits);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatParsingSetTest, TestParseToStd)
    {
    // Move to common spot to reuse within tests.
    CompositeValueSpec fi(*foot, *inch);
    fi.SetMajorLabel("'");
    fi.SetMiddleLabel("\"");

    NumericFormatSpec numeric8 = NumericFormatSpec();
    numeric8.SetFormatTraits(FormatTraits::KeepSingleZero);
    numeric8.SetFormatTraits(FormatTraits::KeepDecimalPoint);
    numeric8.SetPresentationType(PresentationType::Fractional);
    numeric8.SetPrecision(FractionalPrecision::Eighth);
        
    Format fi8 = Format(numeric8, fi);

    // Format dms8 = Format();

    // Format real = Format();

    TestValidParseToQuantityUsingStdFmt("5:6", foot, 5.5, &fi8);
    TestValidParseToQuantityUsingStdFmt("5:", foot, 5.0, &fi8);
    TestValidParseToQuantityUsingStdFmt(":6", foot, 0.5, &fi8);

    /*TestValidParseToQuantityUsingStdFmt("135:23:11", arcDeg, 1, &dms8);
    TestValidParseToQuantityUsingStdFmt("3  1/5 FT", inch, 3.2, &real);*/
    auto formats = GetStdFormats();
    auto mapper = [&](Utf8StringCR formatString)
        {
        return &formats[formatString];
        };

    auto getOverride = [&](Utf8CP formatString) -> Format
        {
        Format f;
        Format::ParseFormatString(f, formatString, mapper, s_unitsContext);
        return f;
        };

    auto const VerifyQuantity = [&](Utf8CP input, Utf8CP unitName, Utf8CP formatName, double magnitude, Utf8CP qtyUnitName)
        {
        auto format = getOverride(formatName);
        FormatProblemCode problemCode;
        auto formatParsingSet = FormatParsingSet(input, s_unitsContext->LookupUnit(unitName), &format);
        EXPECT_FALSE(format.IsProblem());

        auto const actualQuantity = formatParsingSet.GetQuantity(&problemCode, &format);
        EXPECT_EQ(FormatProblemCode::NoProblems, problemCode);

        auto const unit = s_unitsContext->LookupUnit(qtyUnitName);
        auto const expectedQuantity = BEU::Quantity(magnitude, *unit);
        EXPECT_TRUE(actualQuantity.IsClose(expectedQuantity, 0.0001));
        };

    // TODO synonyms don't work. Only can use Unit name or label. 
    VerifyQuantity("-23.45E-03_M", "MM", "DefaultReal", -23.45, "MM");
    VerifyQuantity("30 1/2 IN", "FT", "DefaultReal", 2.541667, "FT");
    VerifyQuantity("30 1/2",    "IN", "DefaultReal", 30.5, "IN");
    VerifyQuantity("3 FT 6 IN", "FT", "DefaultReal", 3.5, "FT");
    VerifyQuantity("3 FT 6 IN", "IN", "DefaultReal", 42.0, "IN");
    VerifyQuantity("3 FT 6 IN", "CM", "DefaultReal", 106.68, "CM");
    VerifyQuantity("3 1/2 FT", "CM", "DefaultReal", 106.68, "CM");
    VerifyQuantity("3 1/2FT", "CM", "DefaultReal",  1.0668, "M");
    VerifyQuantity("3 1/2_FT", "M", "DefaultReal",  1.0668, "M");
    VerifyQuantity("2/3_FT", "IN", "DefaultReal", 8.0, "IN");
    VerifyQuantity("3_FT 1/2IN", "IN", "DefaultReal", 36.5, "IN");
    VerifyQuantity("5 ft 0 in", "FT", "DefaultReal", 5.0, "FT");
    VerifyQuantity("0 ft 3 in", "FT", "DefaultReal", 0.25, "FT");
    VerifyQuantity("3 HR 13 MIN 7 S", "MIN", "DefaultReal", 193.116667, "MIN");
    VerifyQuantity("3 HR 13 MIN 7 S", "MIN", "DefaultReal", 11587.0, "S");
    VerifyQuantity("135:23:11", "ARC_DEG", "AngleDMS", 135.386389, "ARC_DEG");
    VerifyQuantity("135::", "ARC_DEG", "AngleDMS", 135.0, "ARC_DEG");
    VerifyQuantity("135:30:", "ARC_DEG", "AngleDMS", 135.5, "ARC_DEG");
    VerifyQuantity("5:6", "IN", "AmerFI", 1.6764, "M");
    VerifyQuantity("5:6", "IN", "AmerFI", 5.5, "FT");
    VerifyQuantity("5:", "IN", "AmerFI", 152.4, "CM");
    VerifyQuantity(":6", "IN", "AmerFI", 15.24, "CM");
    VerifyQuantity("3:13:7", "S", "HMS", 11587.0, "S");
    VerifyQuantity("3:13:7", "S", "HMS", 193.116667, "MIN");
    VerifyQuantity("3 1/5 FT", "IN", "DefaultReal", 975.36, "MM");
    VerifyQuantity("3 1/5 FT", "IN", "DefaultReal", 38.4, "IN");
    VerifyQuantity("+975.36", "MM", "DefaultReal", 38.4, "IN");
    VerifyQuantity("1 3/13 IN", "IN", "DefaultReal", 1.2307692, "IN");
    VerifyQuantity("1 3/13 IN", "IN", "DefaultReal", 31.26154, "MM");
    VerifyQuantity("3/23", "M", "DefaultReal", 130.4348, "MM");
    VerifyQuantity("13/113", "M", "DefaultReal", 115.044, "MM");
    VerifyQuantity("13/113", "M", "DefaultReal", 0.37744, "FT");
    VerifyQuantity("50+00.1", "FT", "StationZ_100_2", 1524.03048, "M");
    VerifyQuantity("-50+00.1", "FT", "StationZ_100_2", -1524.03048, "M");
    VerifyQuantity("  +   50+000000.1000001  ", "FT", "StationZ_100_2", 1524.03048, "M");
    VerifyQuantity("14+2500.1", "M", "StationZ_100_2", 12795.603674, "FT");
    VerifyQuantity("20+1", "FT", "StationZ_1000_3", 6096.3048, "M");
    VerifyQuantity("20+00", "FT", "StationZ_1000_3", 6096, "M");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FormatParsingSetTest, FailedToParseStation)
    {
    auto formats = GetStdFormats();

    auto mapper = [&formats]
        (Utf8StringCR formatString)
            {
            return &formats[formatString];
            };

    auto const VerifyFailure = [&mapper]
        (Utf8CP input, Utf8CP formatName, FormatProblemCode getQuantityProblem = FormatProblemCode::NoProblems)
            {
            Format format;
            Format::ParseFormatString(format, formatName, mapper, s_unitsContext);

            auto formatParsingSet = FormatParsingSet(input, s_unitsContext->LookupUnit("M"), &format);
            formatParsingSet.GetQuantity(nullptr, &format);
            EXPECT_EQ(getQuantityProblem, formatParsingSet.GetProblemCode());
            };
            
    VerifyFailure("50+00 a", "StationZ_1000_3", FormatProblemCode::NA_InvalidSyntax);
    VerifyFailure("50 +00", "StationZ_1000_3", FormatProblemCode::NA_InvalidSyntax);
    VerifyFailure("50+ 00", "StationZ_1000_3", FormatProblemCode::NA_InvalidSyntax);
    VerifyFailure("a 50+00", "StationZ_1000_3", FormatProblemCode::NA_InvalidSyntax);
    VerifyFailure("50+00.1", "DefaultReal", FormatProblemCode::QT_InvalidSyntax);
    VerifyFailure("30 40", "DefaultReal", FormatProblemCode::QT_InvalidSyntax);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatParsingSetTest, TestParseToStdSynonyms)
    {
    auto length = s_unitsContext->LookupPhenomenon("Length")->GetName();
    auto angle = s_unitsContext->LookupPhenomenon("Angle")->GetName();
    auto foot = s_unitsContext->LookupUnit("FT");
    auto in = s_unitsContext->LookupUnit("IN");
    auto meter = s_unitsContext->LookupUnit("M");
    auto arcDeg = s_unitsContext->LookupUnit("ARC_DEG");
    auto arcMin = s_unitsContext->LookupUnit("ARC_MINUTE");
    auto arcSec = s_unitsContext->LookupUnit("ARC_SECOND");
    bmap<Utf8String, bmap<Utf8String, BEU::UnitCP>> resolverMap = bmap<Utf8String, bmap<Utf8String, BEU::UnitCP>>();
     
    auto lengthMap = bmap<Utf8String, BEU::UnitCP>();
    auto angleMap = bmap<Utf8String, BEU::UnitCP>();
    lengthMap["Feet"] = foot;
    lengthMap[&Utf8Chars(u8"фута")] = foot;
    lengthMap[&Utf8Chars(u8"фут")] = foot;
    lengthMap["'"] = foot;
    lengthMap["\""] = in;
    lengthMap[&Utf8Chars(u8"дюйма")] = in;
    lengthMap[&Utf8Chars(u8"дюйм")] = in;
    lengthMap["ft"] = foot;
    lengthMap["m"] = meter;
    lengthMap["meters"] = meter;

    angleMap[Utf8String(Utf8Chars(u8"°"))] = arcDeg;
    angleMap["'"] = arcMin;
    angleMap["''"] = arcSec;

    resolverMap[length] = lengthMap;
    resolverMap[angle] = angleMap;
    auto formats = GetStdFormats();
    auto mapper = [&](Utf8StringCR formatString)
        {

        return &formats[formatString];
        };

    auto getOverride = [&](Utf8CP formatString) -> Format
        {
        Format f;
        Format::ParseFormatString(f, formatString, mapper, s_unitsContext);
        return f;
        };

    auto VerifyQuantity = [&](Utf8CP input, Utf8CP unitName, Utf8CP formatName, double magnitude, Utf8CP qtyUnitName)
        {
        Format fus = getOverride(formatName);
        FormatProblemCode probCode;
        QuantityFormatting::UnitResolver resolver = [&](Utf8CP i, BEU::PhenomenonCP p) 
        {
            auto map = resolverMap[p->GetName()];
            auto unit = map[i];
            return unit;
        };
        FormatParsingSet fps = FormatParsingSet(input, s_unitsContext->LookupUnit(unitName), nullptr, &resolver);
        BEU::Quantity qty = fps.GetQuantity(&probCode, &fus);
        BEU::UnitCP unit = s_unitsContext->LookupUnit(qtyUnitName);
        BEU::Quantity temp = BEU::Quantity(magnitude, *unit);
        bool eq = qty.IsClose(temp, 0.0001);
        EXPECT_TRUE(eq);
        };

    VerifyQuantity("4'", "FT", "DefaultReal", 4.0, "FT");
    VerifyQuantity("4'6\"", "FT", "DefaultReal", 4.5, "FT");
    VerifyQuantity(Utf8Chars(u8"135°11'30 1/4''"), "ARC_DEG", "DefaultReal", 135.191736, "ARC_DEG");
    VerifyQuantity(Utf8Chars(u8"135°11'30 1/4''"), "ARC_DEG", "real", 2.359541, "RAD");
    VerifyQuantity(Utf8Chars(u8"1 3/13 дюйма"), "IN", "real", 1.2307692, "IN");
    VerifyQuantity(Utf8Chars(u8"3 фута 4 дюйма"), "IN", "real", 40.0, "IN");
    VerifyQuantity(Utf8Chars(u8"1 фут 1 дюйм"), "IN", "real", 13.0, "IN");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatParseStringTest, ParseStringsWithPeriodDecimalSeparator)
    {
    Format format;
    CreateFormatObject(format, ",", ".");
    
    ParseStringToDouble("1", &format, 1);
    ParseStringToDouble("1.2", &format, 1.2);
    ParseStringToDouble("1,234", &format, 1234);
    ParseStringToDouble("1,234.12", &format, 1234.12);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatParseStringTest, ParseStringsWithCommaDecimalSeparator)
    {
    Format format;
    CreateFormatObject(format, ".", ",");

    ParseStringToDouble("1", &format, 1);
    ParseStringToDouble("1,2", &format, 1.2);
    ParseStringToDouble("1.234", &format, 1234);
    ParseStringToDouble("1.234,12", &format, 1234.12);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatParseStringTest, ParseStringsWithTwoCommaSeparators)
    {
    Format format;
    CreateFormatObject(format, ",", ",");

    ParseStringToDouble("1", &format, 1);
    ParseStringToDouble("1,2", &format, 1.2);
    ParseStringToDouble("1,234", &format, 1.234);
    ParseStringToDouble("1,234,12", &format, 1234.12);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FormattingTestFixture, StdFormatting)
    {
    auto formats = GetStdFormats();

    EXPECT_STREQ("15+17.23", formats["Station_100"].GetNumericSpec()->Format(1517.23).c_str());
    EXPECT_STREQ("1+517.23", formats["Station_1000"].GetNumericSpec()->Format(1517.23).c_str());
    EXPECT_STREQ("15+0017.23", formats["Station_100_4"].GetNumericSpec()->Format(1517.23).c_str());
    EXPECT_STREQ("1+0517.23", formats["Station_1000_4"].GetNumericSpec()->Format(1517.23).c_str());
    EXPECT_STREQ("0+0012.23", formats["Station_100_4"].GetNumericSpec()->Format(12.23).c_str());
    EXPECT_STREQ("0+0003.17", formats["Station_100_4"].GetNumericSpec()->Format(3.17).c_str());
    NumericFormatSpec numFmt = NumericFormatSpec();
    numFmt.SetMinWidth(0);
    EXPECT_STREQ("152", numFmt.Format(152).c_str());
    numFmt.SetMinWidth(5);
    EXPECT_STREQ("00152", numFmt.Format(152).c_str());
    numFmt.SetMinWidth(4);
    EXPECT_STREQ("-0152",  numFmt.Format(-152).c_str());
    numFmt.SetMinWidth(4);
    EXPECT_STREQ("0000", numFmt.Format(0).c_str());
    numFmt.SetSignOption(Formatting::SignOption::SignAlways);
    numFmt.SetMinWidth(5);
    EXPECT_STREQ("+00152", numFmt.Format(152).c_str());
    numFmt.SetSignOption(Formatting::SignOption::NegativeParentheses);
    numFmt.SetMinWidth(8);
    EXPECT_STREQ("(00000152)", numFmt.Format(-152).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FormattingTestFixture, Simple)
    {
    double testV = 1000.0 * sqrt(2.0);
    double fval = sqrt(2.0);
    bmap<Utf8String, Format> formats = GetStdFormats();
    EXPECT_STREQ ("1 27/64",  formats["Fractional"].GetNumericSpec()->Format(fval).c_str());
    EXPECT_STREQ ("+1 27/64", formats["SignedFractional"].GetNumericSpec()->Format(fval).c_str());
    EXPECT_STREQ ("-1 27/64", formats["Fractional"].GetNumericSpec()->Format(-fval).c_str());
    EXPECT_STREQ ("-1 27/64", formats["SignedFractional"].GetNumericSpec()->Format(-fval).c_str());
    fval  *= 3.5;

    EXPECT_STREQ ("4 61/64",  formats["Fractional"].GetNumericSpec()->Format(fval).c_str());
    EXPECT_STREQ ("+4 61/64", formats["SignedFractional"].GetNumericSpec()->Format(fval).c_str());
    EXPECT_STREQ ("-4 61/64", formats["Fractional"].GetNumericSpec()->Format(-fval).c_str());
    EXPECT_STREQ ("-4 61/64", formats["SignedFractional"].GetNumericSpec()->Format(-fval).c_str());

    EXPECT_STREQ ("1414.213562", formats["DefaultReal"].GetNumericSpec()->Format(testV).c_str());
    auto f = *formats["DefaultReal"].GetNumericSpec();
    f.SetPrecision(DecimalPrecision::Precision8);
    EXPECT_STREQ ("1414.21356237", f.Format(testV).c_str());
    f.SetPrecision(DecimalPrecision::Precision7);
    EXPECT_STREQ ("1414.2135624", f.Format(testV).c_str());
    f.SetPrecision(DecimalPrecision::Precision6);
    EXPECT_STREQ ("1414.213562", f.Format(testV).c_str());
    f.SetPrecision(DecimalPrecision::Precision5);
    EXPECT_STREQ ("1414.21356", f.Format(testV).c_str());
    f.SetPrecision(DecimalPrecision::Precision4);
    EXPECT_STREQ ("1414.2136", f.Format(testV).c_str());
    f.SetPrecision(DecimalPrecision::Precision3);
    EXPECT_STREQ ("1414.214", f.Format(testV).c_str());
    f.SetPrecision(DecimalPrecision::Precision2);
    EXPECT_STREQ ("1414.21", f.Format(testV).c_str());
    f.SetPrecision(DecimalPrecision::Precision1);
    EXPECT_STREQ ("1414.2", f.Format(testV).c_str());
    f.SetPrecision(DecimalPrecision::Precision0);
    EXPECT_STREQ ("1414.0", f.Format(testV).c_str());
    f.SetPrecision(DecimalPrecision::Precision8);
    f.SetRoundingFactor(5.0);
    EXPECT_STREQ ("1415.0", f.Format(testV).c_str());
    f.SetPrecision(DecimalPrecision::Precision7);
    EXPECT_STREQ ("1415.0", f.Format(testV).c_str());
    f.SetPrecision(DecimalPrecision::Precision6);
    EXPECT_STREQ ("1415.0", f.Format(testV).c_str());
    f.SetPrecision(DecimalPrecision::Precision5);
    EXPECT_STREQ ("1415.0", f.Format(testV).c_str());
    f.SetPrecision(DecimalPrecision::Precision4);
    EXPECT_STREQ ("1415.0", f.Format(testV).c_str());
    f.SetPrecision(DecimalPrecision::Precision3);
    EXPECT_STREQ ("1415.0", f.Format(testV).c_str());
    f.SetPrecision(DecimalPrecision::Precision8);
    f.SetRoundingFactor(0.05);
    EXPECT_STREQ ("1414.2", f.Format(testV).c_str());
    f.SetPrecision(DecimalPrecision::Precision7);
    EXPECT_STREQ ("7071.05", f.Format(5.0*testV).c_str());
    f.SetPrecision(DecimalPrecision::Precision6);
    EXPECT_STREQ ("4242.65", f.Format(3.0*testV).c_str());
    f.SetPrecision(DecimalPrecision::Precision5);
    EXPECT_STREQ ("9899.5", f.Format(7.0*testV).c_str());
    f.SetPrecision(DecimalPrecision::Precision4);
    EXPECT_STREQ ("12727.9", f.Format(9.0*testV).c_str());
    f.SetPrecision(DecimalPrecision::Precision3);
    EXPECT_STREQ ("2828.45", f.Format(2.0*testV).c_str());
    f = *formats["Scientific"].GetNumericSpec();
    f.SetPrecision(DecimalPrecision::Precision5);
    EXPECT_STREQ ("2.82843e+3", f.Format(2.0*testV).c_str());
    f = *formats["ScientificNormal"].GetNumericSpec();
    f.SetPrecision(DecimalPrecision::Precision5);
    EXPECT_STREQ ("0.28284e+4", f.Format(2.0*testV).c_str());

    NumericFormatSpec fmtP = NumericFormatSpec(NumericFormatSpec::DefaultFormat());
    fmtP.SetKeepTrailingZeroes(true);
    fmtP.SetUse1000Separator(true);
    fmtP.SetPrecision(DecimalPrecision::Precision8);
    fmtP.SetRoundingFactor(0.05);
    EXPECT_STREQ ("1,414.20000000", fmtP.Format(testV).c_str());
    fmtP.SetPrecision(DecimalPrecision::Precision7);
    EXPECT_STREQ ("7,071.0500000", fmtP.Format(5.0*testV).c_str());
    fmtP.SetPrecision(DecimalPrecision::Precision6);
    EXPECT_STREQ ("4,242.650000", fmtP.Format(3.0*testV).c_str());
    fmtP.SetPrecision(DecimalPrecision::Precision5);
    EXPECT_STREQ ("9,899.50000", fmtP.Format(7.0*testV).c_str());
    fmtP.SetPrecision(DecimalPrecision::Precision4);
    EXPECT_STREQ ("12,727.9000", fmtP.Format(9.0*testV).c_str());
    fmtP.SetPrecision(DecimalPrecision::Precision3);
    EXPECT_STREQ ("2,828.450", fmtP.Format(2.0*testV).c_str());
    fmtP.SetPrecision(DecimalPrecision::Precision8);

    #if !defined(BENTLEYCONFIG_OS_ANDROID) && !defined(BENTLEYCONFIG_OS_APPLE) && !defined(BENTLEYCONFIG_OS_LINUX)
        #ifdef BENTLEYCONFIG_OS_UNIX
            fmtP.ImbueLocale("de_DE");
        #else
            fmtP.ImbueLocale("de");
        #endif

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
        fmtP.SetPrecision(DecimalPrecision::Precision8);

        #ifdef BENTLEYCONFIG_OS_UNIX
            fmtP.ImbueLocale("fi_FI");
        #else
            fmtP.ImbueLocale("fi");
        #endif

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
        fmtP.SetPrecision(DecimalPrecision::Precision8);

        #ifdef BENTLEYCONFIG_OS_UNIX
            fmtP.ImbueLocale("en_US");
        #else
            fmtP.ImbueLocale("en-US");
        #endif

        fmtP.SetKeepTrailingZeroes(false);
        fmtP.SetUse1000Separator(false);

        EXPECT_STREQ ("1414.2", fmtP.Format(testV).c_str());
        fmtP.SetPrecision(DecimalPrecision::Precision7);
        EXPECT_STREQ ("-7071.05", fmtP.Format(-5.0*testV).c_str());
        fmtP.SetPrecision(DecimalPrecision::Precision6);
        EXPECT_STREQ ("-4242.65", fmtP.Format(-3.0*testV).c_str());
        fmtP.SetPrecision(DecimalPrecision::Precision5);
        EXPECT_STREQ ("-9899.5", fmtP.Format(-7.0*testV).c_str());
        fmtP.SetPrecision(DecimalPrecision::Precision4);
        EXPECT_STREQ ("-12727.9", fmtP.Format(-9.0*testV).c_str());
        fmtP.SetPrecision(DecimalPrecision::Precision3);
        EXPECT_STREQ ("-2828.45", fmtP.Format(-2.0*testV).c_str());
        fmtP.SetPrecision(DecimalPrecision::Precision8);

    #endif /* not BENTLEYCONFIG_OS_ANDROID and not BENTLEYCONFIG_OS_APPLE */

    NumericFormatSpec numFmt = NumericFormatSpec();
    numFmt.SetSignOption(SignOption::OnlyNegative);
    EXPECT_STREQ ("135", numFmt.Format(135).c_str());
    EXPECT_STREQ ("135689", numFmt.Format(135689).c_str());
    EXPECT_STREQ ("-846356", numFmt.Format(-846356).c_str());
    numFmt.SetSignOption(SignOption::SignAlways);
    EXPECT_STREQ ("+135", numFmt.Format(135).c_str());
    EXPECT_STREQ ("+135689", numFmt.Format(135689).c_str());
    EXPECT_STREQ ("-846356", numFmt.Format(-846356).c_str());
    numFmt.SetSignOption(SignOption::NoSign);
    EXPECT_STREQ ("135", numFmt.Format(135).c_str());
    EXPECT_STREQ ("135689", numFmt.Format(135689).c_str());
    EXPECT_STREQ ("846356", numFmt.Format(-846356).c_str());

    numFmt.SetPrecision(DecimalPrecision::Precision10);
    numFmt.SetSignOption(SignOption::OnlyNegative);

    double dval1 = 123.0004567;
    EXPECT_STREQ ("123.0004567", numFmt.Format(dval1).c_str());
    EXPECT_STREQ ("-123.0004567", numFmt.Format(-dval1).c_str());
    dval1 = 0.000012345;
    EXPECT_STREQ ("0.000012345", numFmt.Format(dval1).c_str());
    numFmt.SetPrecision(DecimalPrecision::Precision8);
    EXPECT_STREQ ("0.00001235", numFmt.Format(dval1).c_str());
    numFmt.SetPrecision(DecimalPrecision::Precision10);
    EXPECT_STREQ ("-0.000012345", numFmt.Format(-dval1).c_str());
    numFmt.SetKeepTrailingZeroes(true);
    EXPECT_STREQ ("0.0000123450", numFmt.Format(dval1).c_str());
    EXPECT_STREQ ("-0.0000123450", numFmt.Format(-dval1).c_str());

    dval1 = 3456.0;

    numFmt.SetKeepTrailingZeroes(true);
    numFmt.SetKeepSingleZero(true);
    numFmt.SetKeepDecimalPoint(true);
    EXPECT_STREQ ("3456.0000000000", numFmt.Format(dval1).c_str());
    EXPECT_STREQ ("-3456.0000000000", numFmt.Format(-dval1).c_str());
    numFmt.SetPrecision(DecimalPrecision::Precision4);
    EXPECT_STREQ ("3456.0000", numFmt.Format(dval1).c_str());
    EXPECT_STREQ ("-3456.0000", numFmt.Format(-dval1).c_str());
    numFmt.SetKeepTrailingZeroes(false);
    EXPECT_STREQ ("3456.0", numFmt.Format(dval1).c_str());
    EXPECT_STREQ ("-3456.0", numFmt.Format(-dval1).c_str());
    numFmt.SetKeepSingleZero(false);
    EXPECT_STREQ ("3456.", numFmt.Format(dval1).c_str());
    EXPECT_STREQ ("-3456.", numFmt.Format(-dval1).c_str());
    numFmt.SetKeepDecimalPoint(false);
    EXPECT_STREQ ("3456", numFmt.Format(dval1).c_str());
    EXPECT_STREQ ("-3456", numFmt.Format(-dval1).c_str());
    numFmt.SetSignOption(SignOption::NegativeParentheses);
    EXPECT_STREQ ("3456", numFmt.Format(dval1).c_str());
    EXPECT_STREQ ("(3456)", numFmt.Format(-dval1).c_str());

    numFmt.SetSignOption(SignOption::OnlyNegative);
    numFmt.SetPrecision(DecimalPrecision::Precision10);
    numFmt.SetPresentationType(PresentationType::Scientific);
    EXPECT_STREQ ("-2.7182818285e-3", numFmt.Format(-0.0027182818284590).c_str());
    EXPECT_STREQ ("-2.7182818285e-1", numFmt.Format(-0.2718281828459045).c_str());
    numFmt.SetPresentationType(PresentationType::Scientific);
    numFmt.SetScientificType(ScientificType::ZeroNormalized);
    EXPECT_STREQ ("-0.2718281828e-2", numFmt.Format(-0.0027182818284590).c_str());
    EXPECT_STREQ ("-0.2718281828e+0", numFmt.Format(-0.2718281828459045).c_str());
    EXPECT_STREQ ("-0.2718281828e+4", numFmt.Format(-2718.2818284590).c_str());
    EXPECT_STREQ ("0.2718281828e+4", numFmt.Format(2718.2818284590).c_str());    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FormattingTestFixture, LargeNumbers)
    {
    NumericFormatSpec spec = NumericFormatSpec(NumericFormatSpec::DefaultFormat());
    spec.SetPrecision(DecimalPrecision::Precision4);
    EXPECT_STREQ("1.0", spec.Format(1.0).c_str());
    EXPECT_STREQ("1000.0", spec.Format(1000.0).c_str());
    EXPECT_STREQ("1234567.0", spec.Format(1234567.0).c_str());
    EXPECT_STREQ("1234567891.0", spec.Format(1234567891.0).c_str());
    EXPECT_STREQ("1.2346e+12", spec.Format(1234567891.0e+3).c_str());
    EXPECT_STREQ("1.2346e+15", spec.Format(1234567891.0e+6).c_str());
    EXPECT_STREQ("1.2346e+18", spec.Format(1234567891.0e+9).c_str());
    EXPECT_STREQ("-3.048e+15", spec.Format(-3.0479999999999998e+15).c_str());
    EXPECT_STREQ("-3.048e+18", spec.Format(-3.0479999999999998e+18).c_str());
    EXPECT_STREQ("-3.048e+21", spec.Format(-3.0479999999999998e+21).c_str());
    EXPECT_STREQ("-3.048e+22", spec.Format(-3.0479999999999998e+22).c_str());
    EXPECT_STREQ("-3.048e+23", spec.Format(-3.0479999999999998e+23).c_str());
    EXPECT_STREQ("-3.048e+24", spec.Format(-3.0479999999999998e+24).c_str());
    EXPECT_STREQ("-3.048e+25", spec.Format(-3.0479999999999998e+25).c_str());
    EXPECT_STREQ("-3.048e+26", spec.Format(-3.0479999999999998e+26).c_str());
    EXPECT_STREQ("-3.048e+29", spec.Format(-3.0479999999999998e+29).c_str());
    EXPECT_STREQ("-3.048e+31", spec.Format(-3.0479999999999998e+31).c_str());
    EXPECT_STREQ("-3.048e+35", spec.Format(-3.0479999999999998e+35).c_str());
    }
    
//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(FormattingTestFixture, TestParseUnitFormatDescriptor) 
    {
    Utf8String unitName;
    Utf8String formatString;

    Utf8String input = "(N*M)/DEG";
    Utf8String input2 = "(BTU*IN)/(SQ.FT*HR*FAHRENHEIT)";
    Utf8String input3 = "(N*M)/DEG(real)";
    Utf8String input4 = "(BTU*IN)/(SQ.FT*HR*FAHRENHEIT)(real)";
    Utf8String input5 = "W/(SQ.M*K)";
    Utf8String input6 = "W/(SQ.M*K)(real)";

    Format::ParseUnitFormatDescriptor(unitName, formatString, input.c_str());
    EXPECT_STREQ("(N*M)/DEG", unitName.c_str());
    EXPECT_TRUE(formatString.empty());

    Format::ParseUnitFormatDescriptor(unitName, formatString, input2.c_str());
    EXPECT_STREQ("(BTU*IN)/(SQ.FT*HR*FAHRENHEIT)", unitName.c_str());
    EXPECT_TRUE(formatString.empty());

    Format::ParseUnitFormatDescriptor(unitName, formatString, input3.c_str());
    EXPECT_STREQ("(N*M)/DEG", unitName.c_str());
    EXPECT_STREQ("real", formatString.c_str());

    Format::ParseUnitFormatDescriptor(unitName, formatString, input4.c_str());
    EXPECT_STREQ("(BTU*IN)/(SQ.FT*HR*FAHRENHEIT)", unitName.c_str());
    EXPECT_STREQ("real", formatString.c_str());

    Format::ParseUnitFormatDescriptor(unitName, formatString, input5.c_str());
    EXPECT_STREQ("W/(SQ.M*K)", unitName.c_str());
    EXPECT_TRUE(formatString.empty());

    Format::ParseUnitFormatDescriptor(unitName, formatString, input6.c_str());
    EXPECT_STREQ("W/(SQ.M*K)", unitName.c_str());
    EXPECT_STREQ("real", formatString.c_str());
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(FormattingTestFixture, FormatsUsingDefaultNumericFormatSpecWhenItsNotSet)
    {
    CompositeValueSpec compSpec(*s_unitsContext->LookupUnit("M"));
    Format fmtSpec;
    fmtSpec.SetCompositeSpec(compSpec);
    Units::Quantity quantity(1500.5, *compSpec.GetMajorUnit());
    EXPECT_STREQ("1500.5 M", fmtSpec.FormatQuantity(quantity).c_str());
    }

struct BearingTestData 
    {
    double angleDegree;
    Utf8String bearingDMS; //degrees minutes seconds
    Utf8String bearingDMSWithLabel;
    Utf8String bearingDecimal;
    Utf8String northAzimuthDMS; //degrees minutes seconds
    Utf8String northAzimuthDecimal;
    };

double DegreesToRadians(double degrees)
    {
    return degrees * (M_PI / 180.0);
    };

TEST_F(FormattingTestFixture, FormatBearingAndAzimuth) {
    std::vector<BearingTestData> testData = {
        //DEG,    BEAR DMS      WITH LABEL        BEAR         AZI DMS     AZI
        {0.0,     "N00:00:00E", "N00°00'00\"E", "N00.000°E", "00:00:00", "00.000"},
        {5.0,     "N05:00:00E", "N05°00'00\"E", "N05.000°E", "05:00:00", "05.000"},
        {45.0,    "N45:00:00E", "N45°00'00\"E", "N45.000°E", "45:00:00", "45.000"},
        {45.5028, "N45:30:10E", "N45°30'10\"E", "N45.503°E", "45:30:10", "45.503"},
        {90.0,    "N90:00:00E", "N90°00'00\"E", "N90.000°E", "90:00:00", "90.000"},
        {135.0,   "S45:00:00E", "S45°00'00\"E", "S45.000°E", "135:00:00", "135.000"},
        {180.0,   "S00:00:00E", "S00°00'00\"E", "S00.000°E", "180:00:00", "180.000"},
        {225.0,   "S45:00:00W", "S45°00'00\"W", "S45.000°W", "225:00:00", "225.000"},
        {234.4972,"S54:29:50W", "S54°29'50\"W", "S54.497°W", "234:29:50", "234.497"},
        {270.0,   "N90:00:00W", "N90°00'00\"W", "N90.000°W", "270:00:00", "270.000"},
        {315.0,   "N45:00:00W", "N45°00'00\"W", "N45.000°W", "315:00:00", "315.000"},
        {360.0,   "N00:00:00E", "N00°00'00\"E", "N00.000°E", "00:00:00", "00.000"},
        {412.0,   "N52:00:00E", "N52°00'00\"E", "N52.000°E", "52:00:00", "52.000"},
        {470.0,   "S70:00:00E", "S70°00'00\"E", "S70.000°E", "110:00:00", "110.000"},
        {580.0,   "S40:00:00W", "S40°00'00\"W", "S40.000°W", "220:00:00", "220.000"},
        {640.0,   "N80:00:00W", "N80°00'00\"W", "N80.000°W", "280:00:00", "280.000"},
    };


    auto unitDegree = s_unitsContext->LookupUnit("ARC_DEG");
    auto unitRadian = s_unitsContext->LookupUnit("RAD");
    auto revolution = s_unitsContext->LookupUnit("REVOLUTION");

    NumericFormatSpec bearingDMSSpec;
    bearingDMSSpec.SetMinWidth(2);
    bearingDMSSpec.SetPrecision(DecimalPrecision::Precision0);
    bearingDMSSpec.SetKeepDecimalPoint(false);
    bearingDMSSpec.SetPresentationType(PresentationType::Bearing);
    bearingDMSSpec.SetRevolutionUnit(revolution);
    Format bearingDMS(bearingDMSSpec);
    bearingDMS.SetSuppressUnitLabel();
    auto bearingDMScomp = CompositeValueSpec(*s_unitsContext->LookupUnit("ARC_DEG"), *s_unitsContext->LookupUnit("ARC_MINUTE"), *s_unitsContext->LookupUnit("ARC_SECOND"));
    bearingDMScomp.SetSeparator(":");
    bearingDMS.SetCompositeSpec(bearingDMScomp);
    EXPECT_FALSE(bearingDMS.IsProblem());

    NumericFormatSpec bearingDMSWithLabelSpec;
    bearingDMSWithLabelSpec.SetMinWidth(2);
    bearingDMSWithLabelSpec.SetPrecision(DecimalPrecision::Precision0);
    bearingDMSWithLabelSpec.SetKeepDecimalPoint(false);
    bearingDMSWithLabelSpec.SetPresentationType(PresentationType::Bearing);
    bearingDMSWithLabelSpec.SetShowUnitLabel(true);
    bearingDMSWithLabelSpec.SetRevolutionUnit(revolution);
    Format bearingDMSWithLabel(bearingDMSWithLabelSpec);
    auto bearingDMSWithLabelComp = CompositeValueSpec(*s_unitsContext->LookupUnit("ARC_DEG"), *s_unitsContext->LookupUnit("ARC_MINUTE"), *s_unitsContext->LookupUnit("ARC_SECOND"));
    bearingDMSWithLabelComp.SetMajorLabel("°");
    bearingDMSWithLabelComp.SetMiddleLabel("'");
    bearingDMSWithLabelComp.SetMinorLabel("\"");
    bearingDMSWithLabelComp.SetSeparator("");
    bearingDMSWithLabel.SetCompositeSpec(bearingDMSWithLabelComp);
    EXPECT_FALSE(bearingDMSWithLabel.IsProblem());

    NumericFormatSpec bearingSpec;
    bearingSpec.SetPresentationType(PresentationType::Bearing);
    bearingSpec.SetMinWidth(6);
    bearingSpec.SetPrecision(DecimalPrecision::Precision3);
    bearingSpec.SetKeepDecimalPoint(true);
    bearingSpec.SetKeepTrailingZeroes(true);
    bearingSpec.SetKeepSingleZero(true);
    bearingSpec.SetShowUnitLabel(true);
    bearingSpec.SetRevolutionUnit(revolution);
    Format bearing(bearingSpec);
    auto bearingComp = CompositeValueSpec(*s_unitsContext->LookupUnit("ARC_DEG"));
    bearingComp.SetMajorLabel("°");
    bearingComp.SetSpacer("");
    bearing.SetCompositeSpec(bearingComp);
    EXPECT_FALSE(bearing.IsProblem());
    
    NumericFormatSpec azimuthDMSSpec;
    azimuthDMSSpec.SetPresentationType(PresentationType::Azimuth);
    azimuthDMSSpec.SetMinWidth(2);
    azimuthDMSSpec.SetPrecision(DecimalPrecision::Precision0);
    azimuthDMSSpec.SetKeepDecimalPoint(false);
    azimuthDMSSpec.SetRevolutionUnit(revolution);
    Format azimuthDMS(azimuthDMSSpec);
    azimuthDMS.SetSuppressUnitLabel();
    auto azimuthDMScomp = CompositeValueSpec(*s_unitsContext->LookupUnit("ARC_DEG"), *s_unitsContext->LookupUnit("ARC_MINUTE"), *s_unitsContext->LookupUnit("ARC_SECOND"));
    azimuthDMScomp.SetSeparator(":");
    azimuthDMS.SetCompositeSpec(azimuthDMScomp);
    EXPECT_FALSE(azimuthDMS.IsProblem());

    NumericFormatSpec azimuthSpec;
    azimuthSpec.SetPresentationType(PresentationType::Azimuth);
    azimuthSpec.SetMinWidth(6);
    azimuthSpec.SetPrecision(DecimalPrecision::Precision3);
    azimuthSpec.SetKeepDecimalPoint(true);
    azimuthSpec.SetKeepTrailingZeroes(true);
    azimuthSpec.SetKeepSingleZero(true);
    azimuthSpec.SetRevolutionUnit(revolution);
    Format azimuth(azimuthSpec);
    auto azimuthComp = CompositeValueSpec(*s_unitsContext->LookupUnit("ARC_DEG"));
    azimuth.SetCompositeSpec(azimuthComp);
    EXPECT_FALSE(azimuth.IsProblem());

    auto TestFormat = [&unitDegree, &unitRadian](const Units::Quantity& degree, const Units::Quantity& radian, const Utf8StringCR expectedString, const Format& format)
    {
        Utf8String fromDeg = format.FormatQuantity(degree);
        Utf8String fromRad = format.FormatQuantity(radian);

        ASSERT_STREQ(fromDeg.c_str(), fromRad.c_str());
        ASSERT_STREQ(fromDeg.c_str(), expectedString.c_str());

        FormatProblemCode problemCode_degree = FormatProblemCode::NoProblems;
        FormatProblemCode problemCode_radian = FormatProblemCode::NoProblems;

        BEU::Quantity qtyFromDegree = QuantityFormatting::CreateQuantity(expectedString.c_str(), format,unitDegree, &problemCode_degree);
        BEU::Quantity qtyFromRadian = QuantityFormatting::CreateQuantity(expectedString.c_str(), format,unitRadian, &problemCode_radian);

        ASSERT_EQ(FormatProblemCode::NoProblems, problemCode_degree);
        ASSERT_EQ(FormatProblemCode::NoProblems, problemCode_radian);

        double degreeNormalized = std::fmod(degree.GetMagnitude(), 360.0);
        double radianNormalized = DegreesToRadians(degreeNormalized);

        ASSERT_NEAR(degreeNormalized, qtyFromDegree.GetMagnitude(), 0.001); 
        ASSERT_NEAR(radianNormalized, qtyFromRadian.GetMagnitude(), 0.001);
    };

    for(auto& row : testData)
    { 
        Units::Quantity degree(row.angleDegree, *unitDegree);
        Units::Quantity radian(DegreesToRadians(row.angleDegree), *unitRadian);

        auto degConverted = radian.ConvertTo(unitDegree);
        ASSERT_TRUE(degree.IsClose(degConverted, 0.001));

        TestFormat(degree, radian, row.bearingDMS, bearingDMS);
        TestFormat(degree, radian, row.bearingDMSWithLabel, bearingDMSWithLabel);
        TestFormat(degree, radian, row.bearingDecimal, bearing);
        TestFormat(degree, radian, row.northAzimuthDMS, azimuthDMS);
        TestFormat(degree, radian, row.northAzimuthDecimal, azimuth);

    };
}

TEST_F(FormattingTestFixture, AzimuthWithVariousBases) {
    auto unitDegree = s_unitsContext->LookupUnit("ARC_DEG");
    auto unitRadian = s_unitsContext->LookupUnit("RAD");
    auto unitRevolution = s_unitsContext->LookupUnit("REVOLUTION");

    NumericFormatSpec azimuthSpec;
    azimuthSpec.SetPresentationType(PresentationType::Azimuth);
    azimuthSpec.SetMinWidth(4);
    azimuthSpec.SetPrecision(DecimalPrecision::Precision1);
    azimuthSpec.SetKeepDecimalPoint(true);
    azimuthSpec.SetKeepTrailingZeroes(true);
    azimuthSpec.SetKeepSingleZero(true);
    azimuthSpec.SetShowUnitLabel(true);
    azimuthSpec.SetAzimuthBaseUnit(unitRadian);
    azimuthSpec.SetRevolutionUnit(unitRevolution);

    // azimuth with various bases - roundtrip test
    struct AzimuthTestCase {
        double value;
        double baseOffsetInDeg;
        bool counterClockwise;
        Utf8String expectedString;
    };

    std::vector<AzimuthTestCase> testCases = {
        // {value, baseOffsetInDeg, counterClockwise, expectedString}
        {0.0,   0.0,   false, "00.0°"},
        {0.0,   180.0, false, "180.0°"},
        {0.0,   185.0, false, "175.0°"},
        {0.0,   185.0, true,  "185.0°"},
        {0.0,   95.0,  false, "265.0°"},
        {0.0,   85.0,  false, "275.0°"},
        {0.0,   270.0, false, "90.0°"},
        {0.0,   270.0, true,  "270.0°"},
        {90.0,  0.0,   false, "90.0°"},
        {90.0,  180.0, false, "270.0°"},
        {90.0,  185.0, false, "265.0°"},
        {90.0,  185.0, true,  "95.0°"},
        {90.0,  95.0,  false, "355.0°"},
        {90.0,  85.0,  false, "05.0°"},
        {90.0,  270.0, false, "180.0°"},
        {90.0,  270.0, true,  "180.0°"},
    };

    for (const auto& testCase : testCases)
        {
        // Configure the numeric format specification
        azimuthSpec.SetAzimuthBase(DegreesToRadians(testCase.baseOffsetInDeg));
        azimuthSpec.SetAzimuthCounterClockwise(testCase.counterClockwise);

        Format azimuth(azimuthSpec);
        CompositeValueSpec azimuthComp(*unitDegree);
        azimuthComp.SetMajorLabel("°");
        azimuthComp.SetSpacer("");
        azimuth.SetCompositeSpec(azimuthComp);

        Units::Quantity degreeQuantity(testCase.value, *unitDegree);

        // Format the quantity
        Utf8String result = azimuth.FormatQuantity(degreeQuantity);
        ASSERT_STREQ(testCase.expectedString.c_str(), result.c_str());

        // Parse the formatted string back into quantities
        FormatProblemCode problemCode = FormatProblemCode::NoProblems;
        Units::Quantity qty = QuantityFormatting::CreateQuantity(result.c_str(), azimuth, unitDegree, &problemCode);
        ASSERT_EQ(FormatProblemCode::NoProblems, problemCode);
        ASSERT_NEAR(testCase.value, qty.GetMagnitude(), 0.001);
        }
}

TEST_F(FormattingTestFixture, ParseBearingProblemCode){
    auto unitDegree = s_unitsContext->LookupUnit("ARC_DEG");
    auto revolution = s_unitsContext->LookupUnit("REVOLUTION");

    NumericFormatSpec bearingDMSSpec;
    bearingDMSSpec.SetMinWidth(2);
    bearingDMSSpec.SetPrecision(DecimalPrecision::Precision0);
    bearingDMSSpec.SetKeepDecimalPoint(false);
    bearingDMSSpec.SetPresentationType(PresentationType::Bearing);
    bearingDMSSpec.SetRevolutionUnit(revolution);
    Format bearingDMS(bearingDMSSpec);
    bearingDMS.SetSuppressUnitLabel();
    auto bearingDMScomp = CompositeValueSpec(*s_unitsContext->LookupUnit("ARC_DEG"), *s_unitsContext->LookupUnit("ARC_MINUTE"), *s_unitsContext->LookupUnit("ARC_SECOND"));
    bearingDMScomp.SetSeparator(":");
    bearingDMS.SetCompositeSpec(bearingDMScomp);
    EXPECT_FALSE(bearingDMS.IsProblem());

    struct testData {
        std::string inputString;
        FormatProblemCode expectedProblemCode;
    };

    std::string zeroInput = "N00:00:00E";

    std::vector<testData> testDataVec = {
        {zeroInput, FormatProblemCode::NoProblems},
        {"", FormatProblemCode::QT_NoValueOrUnitFound},
        {"00:00:00", FormatProblemCode::QT_BearingPrefixOrSuffixMissing},
        {"A00:00:00B", FormatProblemCode::QT_BearingPrefixOrSuffixMissing},
        {"N00:00:00", FormatProblemCode::QT_BearingPrefixOrSuffixMissing},
        {"00:00:00S", FormatProblemCode::QT_BearingPrefixOrSuffixMissing},
    };

    BEU::Quantity qty;
    for(auto& row : testDataVec)
    {
        FormatProblemCode problemCode = FormatProblemCode::NoProblems;
        qty = QuantityFormatting::CreateQuantity(row.inputString.c_str(), bearingDMS, unitDegree, &problemCode);
        EXPECT_EQ(row.expectedProblemCode, problemCode);
    }

    auto invalidUnit = s_unitsContext->LookupUnit("VERTICAL_PER_HORIZONTAL");

    // invalid unit
    FormatProblemCode problemCode = FormatProblemCode::NoProblems;

    Format bearingDMSInvalidCompUnit(bearingDMSSpec);
    bearingDMSInvalidCompUnit.SetSuppressUnitLabel();
    auto bearingDMScompInvalidUnit = CompositeValueSpec(*invalidUnit, *s_unitsContext->LookupUnit("ARC_MINUTE"), *s_unitsContext->LookupUnit("ARC_SECOND"));
    bearingDMScompInvalidUnit.SetSeparator(":");
    bearingDMSInvalidCompUnit.SetCompositeSpec(bearingDMScompInvalidUnit);
    EXPECT_FALSE(bearingDMSInvalidCompUnit.IsProblem());

    bearingDMSInvalidCompUnit.SetCompositeSpec(bearingDMScompInvalidUnit);
    qty = QuantityFormatting::CreateQuantity(zeroInput.c_str(), bearingDMSInvalidCompUnit, unitDegree, &problemCode);
    EXPECT_EQ(FormatProblemCode::PS_MissingCompositeSpec, problemCode); // composite spec wont get set with invalid composite units

    // cant convert
    problemCode = FormatProblemCode::NoProblems;
    qty = QuantityFormatting::CreateQuantity(zeroInput.c_str(), bearingDMS, invalidUnit, &problemCode);
    EXPECT_EQ(FormatProblemCode::QT_ConversionFailed, problemCode);
}

TEST_F(FormattingTestFixture, ParseAzimuthProblemCode){
    auto unitDegree = s_unitsContext->LookupUnit("ARC_DEG");
    auto revolution = s_unitsContext->LookupUnit("REVOLUTION");
    FormatProblemCode problemCode = FormatProblemCode::NoProblems;

    NumericFormatSpec azimuthSpec;
    azimuthSpec.SetPresentationType(PresentationType::Azimuth);
    azimuthSpec.SetMinWidth(6);
    azimuthSpec.SetPrecision(DecimalPrecision::Precision3);
    azimuthSpec.SetKeepDecimalPoint(true);
    azimuthSpec.SetKeepTrailingZeroes(true);
    azimuthSpec.SetKeepSingleZero(true);
    azimuthSpec.SetAzimuthBase(180.0);
    // azimuthSpec.SetAzimuthBaseUnit(unitDegree); missing for this test
    azimuthSpec.SetRevolutionUnit(revolution);

    Format azimuth(azimuthSpec);
    auto azimuthComp = CompositeValueSpec(*unitDegree);
    azimuth.SetCompositeSpec(azimuthComp);
    EXPECT_FALSE(azimuth.IsProblem());

    std::string zeroInput = "00.0";
    BEU::Quantity qty = QuantityFormatting::CreateQuantity(zeroInput.c_str(), azimuth, unitDegree, &problemCode);
    EXPECT_EQ(FormatProblemCode::QT_NoValueOrUnitFound, problemCode);
}

TEST_F(FormattingTestFixture, FormatRatio){
    auto formatRatioTest = [](const char* ratioString, double value, Units::UnitCP persistenceUnit, RatioType ratioType,  Units::UnitCP presentationUnit, DecimalPrecision precision = DecimalPrecision::Precision3)
    {
        NumericFormatSpec ratioSpec;
        ratioSpec.SetPresentationType(PresentationType::Ratio);
        ratioSpec.SetRatioType(ratioType);
        ratioSpec.SetPrecision(precision);
        
        CompositeValueSpec ratioComp(*presentationUnit);
        ratioComp.SetIncludeZero(true);

        Format ratioFormat(ratioSpec);
        ratioFormat.SetCompositeSpec(ratioComp);

        Units::Quantity quantity(value, *persistenceUnit);
        EXPECT_STREQ(ratioString, ratioFormat.FormatQuantity(quantity).c_str());

        // parsing back to quantity
        FormatProblemCode problemCode = FormatProblemCode::NoProblems;
        BEU::Quantity qty = QuantityFormatting::CreateQuantity(ratioString, ratioFormat, persistenceUnit, &problemCode);

        EXPECT_EQ(FormatProblemCode::NoProblems, problemCode); 
        auto precisionNum = static_cast<int>(precision);
        auto tolerance = pow(10, -precisionNum) * 4.999;
        EXPECT_NEAR(value, qty.GetMagnitude(), tolerance);
    };

    auto v_h = s_unitsContext->LookupUnit("VERTICAL_PER_HORIZONTAL");
    auto h_v = s_unitsContext->LookupUnit("HORIZONTAL_PER_VERTICAL");

    auto oneToN = RatioType::OneToN;
    auto NtoOne = RatioType::NToOne;
    auto valueBased = RatioType::ValueBased;
    auto useGreatestCommonDivisor = RatioType::UseGreatestCommonDivisor;

    // corner cases, input vaue is 0
    {
    auto ratioTypes = {RatioType::OneToN, RatioType::NToOne, RatioType::ValueBased, RatioType::UseGreatestCommonDivisor};

    for (const auto& ratioType : ratioTypes) 
        {
        formatRatioTest("0:1", 0.0, v_h, ratioType, v_h);
        formatRatioTest("0:1", 0.0, h_v, ratioType, h_v);
        formatRatioTest("1:0", 0.0, v_h, ratioType, h_v);
        formatRatioTest("1:0", 0.0, h_v, ratioType, v_h);
        }
    }

    // v:h (persistent) -> v:h (presentation) | one to N
    {
    formatRatioTest("1:1", 1.0, v_h, oneToN, v_h);
    formatRatioTest("1:0.5", 2.0, v_h, oneToN, v_h);
    formatRatioTest("1:2", 0.5, v_h, oneToN, v_h);
    formatRatioTest("1:3.003", 0.333, v_h, oneToN, v_h);
    formatRatioTest("1:3", 0.3333, v_h, oneToN, v_h);
    formatRatioTest("1:3.5", 0.2857, v_h, oneToN, v_h);
    formatRatioTest("1:4", 0.25, v_h, oneToN, v_h);
    formatRatioTest("1:1.5", 0.6667, v_h, oneToN, v_h);
    }

    // v:h -> h:v | one to N
    {
    formatRatioTest("1:1", 1.0, v_h, oneToN, h_v);
    formatRatioTest("1:2", 2.0, v_h, oneToN, h_v);
    formatRatioTest("1:0.5", 0.5, v_h, oneToN, h_v);
    formatRatioTest("1:0.333", 0.333, v_h, oneToN, h_v);
    formatRatioTest("1:0.333", 0.3333, v_h, oneToN, h_v);
    formatRatioTest("1:0.286", 0.2857, v_h, oneToN, h_v);
    formatRatioTest("1:0.25", 0.25, v_h, oneToN, h_v);
    formatRatioTest("1:0.667", 0.6667, v_h, oneToN, h_v);
    }

    // v:h -> v:h | N to one
    {
    formatRatioTest("1:1", 1.0, v_h, NtoOne, v_h);
    formatRatioTest("2:1", 2.0, v_h, NtoOne, v_h);
    formatRatioTest("0.5:1", 0.5, v_h, NtoOne, v_h);
    formatRatioTest("0.333:1", 0.333, v_h, NtoOne, v_h);
    formatRatioTest("0.333:1", 0.3333, v_h, NtoOne, v_h);
    formatRatioTest("0.286:1", 0.2857, v_h, NtoOne, v_h);
    formatRatioTest("0.25:1", 0.25, v_h, NtoOne, v_h);
    formatRatioTest("0.667:1", 0.6667, v_h, NtoOne, v_h);
    }

    // v:h -> h:v | N to one
    {
    formatRatioTest("1:1", 1.0, v_h, NtoOne, h_v);
    formatRatioTest("0.5:1", 2.0, v_h, NtoOne, h_v);
    formatRatioTest("2:1", 0.5, v_h, NtoOne, h_v);
    formatRatioTest("3.003:1", 0.333, v_h, NtoOne, h_v);
    formatRatioTest("3:1", 0.3333, v_h, NtoOne, h_v);
    formatRatioTest("3.5:1", 0.2857, v_h, NtoOne, h_v);
    formatRatioTest("4:1", 0.25, v_h, NtoOne, h_v);
    formatRatioTest("1.5:1", 0.6667, v_h, NtoOne, h_v);
    }

    {
    // decimal precision
    // formatRatioTest("1:0", 3, v_h, oneToN, v_h, DecimalPrecision::Precision0);
    formatRatioTest("1:0.3", 3, v_h, oneToN, v_h, DecimalPrecision::Precision1);
    formatRatioTest("1:0.33", 3, v_h, oneToN, v_h, DecimalPrecision::Precision2);

    formatRatioTest("1:2500", 0.0004, v_h, oneToN, v_h, DecimalPrecision::Precision3);

    formatRatioTest("1:0", 0.0004, v_h, oneToN, h_v, DecimalPrecision::Precision3);
    formatRatioTest("1:0.0004", 0.0004, v_h, oneToN, h_v, DecimalPrecision::Precision4);
    formatRatioTest("1:-0.0004", -0.0004, v_h, oneToN, h_v, DecimalPrecision::Precision4);
    }

    // v:h -> v:h | ValueBased
    // if the value is smaller than 1, its 1 to N. else, N to 1
    {
    formatRatioTest("1:1", 1.0, v_h, valueBased, v_h);
    formatRatioTest("2:1", 2.0, v_h, valueBased, v_h);
    formatRatioTest("1:2", 0.5, v_h, valueBased, v_h);
    formatRatioTest("1:3.003", 0.333, v_h, valueBased, v_h);
    formatRatioTest("3.333:1", 3.3333, v_h, valueBased, v_h);
    formatRatioTest("1:3.5", 0.2857, v_h, valueBased, v_h);
    formatRatioTest("3.5:1", 3.5, v_h, valueBased, v_h);
    formatRatioTest("1:4", 0.25, v_h, valueBased, v_h);
    formatRatioTest("4:1", 4.0, v_h, valueBased, v_h);
    }   

    // v:h -> v:h | UseGreatestCommonDivisor
    {
    formatRatioTest("1:1", 1.0, v_h, useGreatestCommonDivisor, v_h);
    formatRatioTest("2:1", 2.0, v_h, useGreatestCommonDivisor, v_h);
    formatRatioTest("1:2", 0.5, v_h, useGreatestCommonDivisor, v_h);
    formatRatioTest("333:1000", 0.333, v_h, useGreatestCommonDivisor, v_h);
    formatRatioTest("333:1000", 0.3333, v_h, useGreatestCommonDivisor, v_h);
    formatRatioTest("143:500", 0.2857, v_h, useGreatestCommonDivisor, v_h);
    formatRatioTest("1:4", 0.25, v_h, useGreatestCommonDivisor, v_h);
    formatRatioTest("667:1000", 0.6667, v_h, useGreatestCommonDivisor, v_h);
    }

    // negative values
    {
    formatRatioTest("-1:1", -1.0, v_h, NtoOne, v_h);
    formatRatioTest("1:-1", -1.0, v_h, valueBased, v_h);

    formatRatioTest("-0.5:1", -0.5, v_h, NtoOne, v_h);
    formatRatioTest("1:-2", -0.5, v_h, valueBased, v_h);

    formatRatioTest("-0.5:1", -2, v_h, NtoOne, h_v);
    formatRatioTest("1:-2", -2, v_h, valueBased, h_v);
    }

    // really large/small numbers
    {
    formatRatioTest("0:1", 0.00000001, v_h, NtoOne, v_h);
    formatRatioTest("100000000:1", 0.00000001, v_h, NtoOne, h_v);

    formatRatioTest("100000000:1", 100000000, v_h, NtoOne, v_h);
    // formatRatioTest("0:1", 100000000, v_h, NtoOne, h_v); //works one way parsing from quantity to ratio, but not the other way around. from "0:1" cant be parsed to 100000000
    }

    // irrational numbers
    {
    formatRatioTest("0.143:1", 1.0/7, v_h, NtoOne, v_h);
    formatRatioTest("1:7", 1.0/7, v_h, oneToN, v_h);
    formatRatioTest("0.286:1", 2.0/7, v_h, NtoOne, v_h);
    formatRatioTest("143:500", 2.0/7, v_h, useGreatestCommonDivisor, v_h); // loose precision from 0.28571428571 to 0.286
    formatRatioTest("7:2", 2.0/7, v_h, useGreatestCommonDivisor, h_v); // didnt loose much precision from 3.50000000005 to 3.5
    }

    // parseRatioString specific tests
    auto formatRatioErrorTest = [](Utf8CP ratioString, Units::UnitCP persistenceUnit, Units::UnitCP presentationUnit, FormatProblemCode expectedProblemCode = FormatProblemCode::NoProblems)
    {
        NumericFormatSpec ratioSpec;
        ratioSpec.SetPresentationType(PresentationType::Ratio);
        ratioSpec.SetRatioType(RatioType::OneToN);
        ratioSpec.SetPrecision(DecimalPrecision::Precision3);
        CompositeValueSpec ratioComp(*presentationUnit);
        ratioComp.SetIncludeZero(true);
        Format ratioFormat(ratioSpec);
        ratioFormat.SetCompositeSpec(ratioComp);

        FormatProblemCode problemCode;
        BEU::Quantity qty = QuantityFormatting::CreateQuantity(ratioString, ratioFormat, persistenceUnit, &problemCode);

        EXPECT_EQ(expectedProblemCode, problemCode);
    };

    {
        formatRatioErrorTest("", v_h, v_h, FormatProblemCode::QT_NoValueOrUnitFound);
        formatRatioErrorTest("1:1", nullptr, v_h, FormatProblemCode::QT_NoValueOrUnitFound); // test this
        formatRatioErrorTest("3:1:2", v_h, v_h, FormatProblemCode::QT_InvalidRatioArgument);
        formatRatioErrorTest("A:2", v_h, v_h, FormatProblemCode::QT_InvalidRatioArgument);
        formatRatioErrorTest("1:1e1000", v_h, v_h, FormatProblemCode::QT_ValueOutOfRange);

        formatRatioErrorTest("0:1", v_h, h_v, FormatProblemCode::QT_MathematicOperationFoundButIsNotAllowed);
        formatRatioErrorTest("1:0", v_h, v_h, FormatProblemCode::QT_MathematicOperationFoundButIsNotAllowed);
    }
    
}

END_BENTLEY_FORMATTEST_NAMESPACE
