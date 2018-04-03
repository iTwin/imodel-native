/*--------------------------------------------------------------------------------------+
|
|  $Source: tests/Published/FormattingTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../FormattingTestsPch.h"
#include "../TestFixture/FormattingTestFixture.h"

USING_BENTLEY_FORMATTING
BEGIN_BENTLEY_FORMATTEST_NAMESPACE

//===================================================
// NumericFormatSpec
//===================================================
struct NumericFormatSpecTest : FormattingTestFixture
    {
    };

struct FormatIntegerTest : NumericFormatSpecTest
    {
    int32_t const testValPos = 100501;
    int32_t const testValNeg = -100501;
    };

struct FormatDoubleTest : NumericFormatSpecTest
    {
    double const testValPos = 100501.125;
    double const testValNeg = -100501.125;
    double const testValPosZeroFracComp = 100501.0;
    double const testValNegZeroFracComp = -100501.0;
    double const testValMagnitudeLt1Pos = 0.000125;
    double const testValMagnitudeLt1Neg = -0.000125;
    };

//===================================================
// Format
//===================================================
struct FormatTest : FormattingTestFixture
    {
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
    void SetUp()
        {
        ASSERT_NE(nullptr, mile = s_unitsContext->LookupUnit("MILE"));
        ASSERT_NE(nullptr, yard = s_unitsContext->LookupUnit("YRD"));
        ASSERT_NE(nullptr, foot = s_unitsContext->LookupUnit("FT"));
        ASSERT_NE(nullptr, inch = s_unitsContext->LookupUnit("IN"));
        ASSERT_NE(nullptr, arcDeg = s_unitsContext->LookupUnit("ARC_DEG"));
        }

    Utf8String GetFmtStringErrMsg(Utf8CP fmtStr){ return Utf8String("format string: \"") + fmtStr + "\""; }

    void TestValidParseToQuantity(Utf8CP fmtStr, BEU::UnitCP expectedUnit, double const expectedMagnitude)
        {
        FormatParsingSet fps(fmtStr, expectedUnit);
        ASSERT_FALSE(fps.HasProblem()) << fps.GetProblemDescription() << '\n' << GetFmtStringErrMsg(fmtStr);

        FormatProblemCode probCode;
        Format format = Format();
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
        FormatParsingSet fps(fmtStr, expectedUnit);
        ASSERT_TRUE(fps.HasProblem()) << GetFmtStringErrMsg(fmtStr);
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(NumericFormatSpecTest, Constructors)
    {
    // Default constructed.
    {
    NumericFormatSpec nfs;
    EXPECT_DOUBLE_EQ(FormatConstant::DefaultRoundingFactor(), nfs.GetRoundingFactor());
    EXPECT_EQ(FormatConstant::DefaultPresentaitonType(), nfs.GetPresentationType());
    EXPECT_EQ(FormatConstant::DefaultSignOption(), nfs.GetSignOption());
    EXPECT_EQ(FormatConstant::DefaultFormatTraits(), nfs.GetFormatTraits());
    EXPECT_EQ(FormatConstant::DefaultDecimalPrecision(), nfs.GetDecimalPrecision());
    EXPECT_EQ(FormatConstant::DefaultFractionalPrecision(), nfs.GetFractionalPrecision());
    EXPECT_EQ(FormatConstant::DefaultDecimalSeparator(), nfs.GetDecimalSeparator());
    EXPECT_EQ(FormatConstant::DefaultThousandSeparator(), nfs.GetThousandSeparator());
    EXPECT_EQ(FormatConstant::DefaultUomSeparator(), nfs.GetUomSeparator());
    EXPECT_EQ(FormatConstant::DefaultStationSeparator(), nfs.GetStationSeparator());
    EXPECT_EQ(FormatConstant::DefaultMinWidth(), nfs.GetMinWidth());
    EXPECT_EQ(FormatConstant::DefaultMinWidth(), nfs.GetMinWidth());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(NumericFormatSpecTest, IsIdentical)
    {
    NumericFormatSpec nfsA;
    NumericFormatSpec nfsB;
    nfsB.SetDecimalPrecision(DecimalPrecision::Max);

    EXPECT_TRUE(nfsA.IsIdentical(nfsA)) << "NumericFormatSpec is not identical to itself.";
    EXPECT_FALSE(nfsA.IsIdentical(nfsB));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatIntegerTest, FormatIntegerSignOptionTests)
    {
    NumericFormatSpec nfs;

    // NoSign
    {
    nfs.SetSignOption(ShowSignOption::NoSign);
    EXPECT_STREQ("42", nfs.FormatInteger(42).c_str());
    EXPECT_STREQ("42", nfs.FormatInteger(-42).c_str());
    }

    // OnlyNegative
    {
    nfs.SetSignOption(ShowSignOption::OnlyNegative);
    EXPECT_STREQ("42", nfs.FormatInteger(42).c_str());
    EXPECT_STREQ("-42", nfs.FormatInteger(-42).c_str());
    }

    // SignAlways
    {
    nfs.SetSignOption(ShowSignOption::SignAlways);
    EXPECT_STREQ("+42", nfs.FormatInteger(42).c_str());
    EXPECT_STREQ("-42", nfs.FormatInteger(-42).c_str());
    }

    // NegativeParentheses
    {
    nfs.SetSignOption(ShowSignOption::NegativeParentheses);
    EXPECT_STREQ("42", nfs.FormatInteger(42).c_str());
    EXPECT_STREQ("(42)", nfs.FormatInteger(-42).c_str());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatIntegerTest, FormatIntegerThousandsSeparatorTests)
    {
    NumericFormatSpec nfs;
    nfs.SetUse1000Separator(true);
    Utf8String string;

    nfs.SetThousandSeparator(',');
    EXPECT_STREQ("1,000", nfs.FormatInteger(1000).c_str());
    EXPECT_STREQ("-1,000", nfs.FormatInteger(-1000).c_str());

    nfs.SetThousandSeparator('.');
    EXPECT_STREQ("1.000", nfs.FormatInteger(1000).c_str());
    EXPECT_STREQ("-1.000", nfs.FormatInteger(-1000).c_str());

    nfs.SetThousandSeparator('\t'); // I mean technically the API doesn't prevent it right?
    EXPECT_STREQ("1\t000", nfs.FormatInteger(1000).c_str());
    EXPECT_STREQ("-1\t000", nfs.FormatInteger(-1000).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatIntegerTest, FormatIntegerPresentationTypeTests)
    {
    Utf8String string;

    // Decimal
    {
    NumericFormatSpec nfs;
    nfs.SetPresentationType(PresentationType::Decimal);
    EXPECT_STREQ("100501", nfs.FormatInteger(testValPos).c_str());
    EXPECT_STREQ("-100501", nfs.FormatInteger(testValNeg).c_str());
    }

    // Fractional
    {
    NumericFormatSpec nfs;
    nfs.SetPresentationType(PresentationType::Fractional);
    EXPECT_STREQ("100501", nfs.FormatInteger(testValPos).c_str());
    EXPECT_STREQ("-100501", nfs.FormatInteger(testValNeg).c_str());
    }

    // TODO: The below Scientific and Scientific Norm tests fail since the testVal integer does not
    // ever get formatted as a value with a decimal point. Since it is not obvious to the user
    // how the scientific notation affects formatting of integers.
    // Scientific
    {
    NumericFormatSpec nfs;
    nfs.SetPresentationType(PresentationType::Scientific);
    nfs.SetDecimalPrecision(DecimalPrecision::Max);
    EXPECT_STREQ("1.00501e+5", nfs.FormatInteger(testValPos).c_str());
    EXPECT_STREQ("-1.00501e+5", nfs.FormatInteger(testValNeg).c_str());
    }

    // ScientificNorm
    {
    NumericFormatSpec nfs;
    nfs.SetPresentationType(PresentationType::Scientific);
    nfs.SetScientificType(ScientificType::Normal);
    nfs.SetDecimalPrecision(DecimalPrecision::Max);
    EXPECT_STREQ("0.100501e+6", nfs.FormatInteger(testValPos).c_str());
    EXPECT_STREQ("-0.100501e+6", nfs.FormatInteger(testValNeg).c_str());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatIntegerTest, FormatIntegerIndividualFormatTraitsTests)
    {
    NumericFormatSpec nfs;
    Utf8String string;
    int const testValPos = 100501;
    int const testValNeg = -100501;

    nfs.SetFormatTraits(FormatConstant::DefaultFormatTraits());
    EXPECT_STREQ("0", nfs.FormatInteger(0).c_str());
    EXPECT_STREQ("100501", nfs.FormatInteger(testValPos).c_str());
    EXPECT_STREQ("-100501", nfs.FormatInteger(testValNeg).c_str());

    // TODO: Documentation refers to leading zeros for "digital expression" but never
    // defines what a digital expression means. So I don't actually know if this testing
    // is correct.
    nfs.SetMinWidth(10);
    nfs.SetFormatTraits(FormatTraits::LeadingZeroes);
    EXPECT_STREQ("0000000000", nfs.FormatInteger(0).c_str());
    EXPECT_STREQ("0000100501", nfs.FormatInteger(testValPos).c_str());
    EXPECT_STREQ("-0000100501", nfs.FormatInteger(testValNeg).c_str());
    nfs.SetMinWidth(0);

    // TODO: It isn't defined in the spec what the behavior of trailing zeros should
    // be for integer formatting. This testing is likely also incorrect.
    nfs.SetMinWidth(10);
    nfs.SetFormatTraits(FormatTraits::TrailingZeroes);
    EXPECT_STREQ("0", nfs.FormatInteger(0).c_str());
    EXPECT_STREQ("100501", nfs.FormatInteger(testValPos).c_str());
    EXPECT_STREQ("-100501", nfs.FormatInteger(testValNeg).c_str());
    nfs.SetMinWidth(0);

    // Should only affect doubles.
    nfs.SetFormatTraits(FormatTraits::KeepDecimalPoint);
    EXPECT_STREQ("0", nfs.FormatInteger(0).c_str());
    EXPECT_STREQ("100501", nfs.FormatInteger(testValPos).c_str());
    EXPECT_STREQ("-100501", nfs.FormatInteger(testValNeg).c_str());

    // Should only affect doubles.
    nfs.SetFormatTraits(FormatTraits::KeepSingleZero);
    EXPECT_STREQ("0", nfs.FormatInteger(0).c_str());
    EXPECT_STREQ("100501", nfs.FormatInteger(testValPos).c_str());
    EXPECT_STREQ("-100501", nfs.FormatInteger(testValNeg).c_str());

    nfs.SetPresentationType(PresentationType::Scientific);
    nfs.SetFormatTraits(FormatTraits::ExponentZero);
    EXPECT_STREQ("0e+00", nfs.FormatInteger(0).c_str());
    EXPECT_STREQ("1.00501e+05", nfs.FormatInteger(testValPos).c_str());
    EXPECT_STREQ("-1.00501e+05", nfs.FormatInteger(testValNeg).c_str());
    nfs.SetPresentationType(PresentationType::Decimal);

    nfs.SetFormatTraits(FormatTraits::ZeroEmpty);
    EXPECT_STREQ("", nfs.FormatInteger(0).c_str());
    EXPECT_STREQ("100501", nfs.FormatInteger(testValPos).c_str());
    EXPECT_STREQ("-100501", nfs.FormatInteger(testValNeg).c_str());

    nfs.SetFormatTraits(FormatTraits::Use1000Separator);
    EXPECT_STREQ("0", nfs.FormatInteger(0).c_str());
    EXPECT_STREQ("100,501", nfs.FormatInteger(testValPos).c_str());
    EXPECT_STREQ("-100,501", nfs.FormatInteger(testValNeg).c_str());

    // Should only affect doubles.
    nfs.SetFormatTraits(FormatTraits::ApplyRounding);
    EXPECT_STREQ("0", nfs.FormatInteger(0).c_str());
    EXPECT_STREQ("100501", nfs.FormatInteger(testValPos).c_str());
    EXPECT_STREQ("-100501", nfs.FormatInteger(testValNeg).c_str());

    // Should only affect doubles.
    nfs.SetFormatTraits(FormatTraits::FractionDash);
    EXPECT_STREQ("0", nfs.FormatInteger(0).c_str());
    EXPECT_STREQ("100501", nfs.FormatInteger(testValPos).c_str());
    EXPECT_STREQ("-100501", nfs.FormatInteger(testValNeg).c_str());

    // Should only affect doubles.
    //nfs.SetFormatTraits(FormatTraits::UseFractSymbol);
    //EXPECT_STREQ("0", nfs.FormatInteger(0).c_str());
    //EXPECT_STREQ("100501", nfs.FormatInteger(testValPos).c_str());
    //EXPECT_STREQ("-100501", nfs.FormatInteger(testValNeg).c_str());

    // AppendUnitName should have no effect on the unitless Format.
    nfs.SetFormatTraits(FormatTraits::ShowUnitLabel);
    EXPECT_STREQ("0", nfs.FormatInteger(0).c_str());
    EXPECT_STREQ("100501", nfs.FormatInteger(testValPos).c_str());
    EXPECT_STREQ("-100501", nfs.FormatInteger(testValNeg).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatDoubleTest, FormatDoubleSignOptionTests)
    {
    NumericFormatSpec nfs;
    Utf8String string;

    // NoSign
    {
    nfs.SetSignOption(ShowSignOption::NoSign);

    EXPECT_STREQ("101.25", nfs.FormatDouble(101.25).c_str());
    EXPECT_STREQ("101.25", nfs.FormatDouble(-101.25).c_str());
    }

    // OnlyNegative
    {
    nfs.SetSignOption(ShowSignOption::OnlyNegative);

    EXPECT_STREQ("101.25", nfs.FormatDouble(101.25).c_str());
    EXPECT_STREQ("-101.25", nfs.FormatDouble(-101.25).c_str());
    }

    // SignAlways
    {
    nfs.SetSignOption(ShowSignOption::SignAlways);

    EXPECT_STREQ("+101.25", nfs.FormatDouble(101.25).c_str());
    EXPECT_STREQ("-101.25", nfs.FormatDouble(-101.25).c_str());
    }

    // NegativeParentheses
    {
    nfs.SetSignOption(ShowSignOption::NegativeParentheses);

    EXPECT_STREQ("101.25", nfs.FormatDouble(101.25).c_str());
    EXPECT_STREQ("(101.25)", nfs.FormatDouble(-101.25).c_str());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatDoubleTest, FormatDoubleThousandsSeparatorTests)
    {
    NumericFormatSpec nfs;
    nfs.SetUse1000Separator(true);
    Utf8String string;

    nfs.SetThousandSeparator(',');
    EXPECT_STREQ("1,000.55", nfs.FormatDouble(1000.55).c_str());
    EXPECT_STREQ("-1,000.55", nfs.FormatDouble(-1000.55).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatDoubleTest, FormatDoubleDecimalSeparatorTests)
    {
    NumericFormatSpec nfs;
    Utf8String string;

    nfs.SetDecimalSeparator('-');
    EXPECT_STREQ("1000-55", nfs.FormatDouble(1000.55).c_str());
    EXPECT_STREQ("-1000-55", nfs.FormatDouble(-1000.55).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
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
    nfs.SetDecimalPrecision(DecimalPrecision::Max);

    EXPECT_STREQ("100501.125", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ("-100501.125", nfs.FormatDouble(testValNeg).c_str());
    }

    // Fractional
    {
    NumericFormatSpec nfs;
    nfs.SetPresentationType(PresentationType::Fractional);
    nfs.SetDecimalPrecision(DecimalPrecision::Max);

    EXPECT_STREQ("100501 1/8", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ("-100501 1/8", nfs.FormatDouble(testValNeg).c_str());
    }

    // Scientific
    {
    NumericFormatSpec nfs;
    nfs.SetPresentationType(PresentationType::Scientific);
    nfs.SetDecimalPrecision(DecimalPrecision::Max);

    EXPECT_STREQ("1.00501125e+5", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ("-1.00501125e+5", nfs.FormatDouble(testValNeg).c_str());
    }

    // ScientificNorm
    {
    NumericFormatSpec nfs;
    nfs.SetPresentationType(PresentationType::Scientific);
    nfs.SetScientificType(ScientificType::Normal);
    nfs.SetDecimalPrecision(DecimalPrecision::Max);

    EXPECT_STREQ("0.100501125e+6", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ("-0.100501125e+6", nfs.FormatDouble(testValNeg).c_str());
    }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatDoubleTest, FormatDoublePresentationTypeTests_IGNORED)
    {
    Utf8String string;

    // Scientific & ScientificNorm incorrectly format values with magnitude less than 1.
    // At the time of writing this comment, values with magnitude less than 1 are formatted
    // like ScientificNorm when the presentation type is Scientific, and are formatted like
    // Scientific when the value is ScientificNorm.

    // Scientific
    {
    NumericFormatSpec nfs;
    nfs.SetPresentationType(PresentationType::Scientific);
    nfs.SetDecimalPrecision(DecimalPrecision::Max);

    EXPECT_STREQ("1.25e-4", nfs.FormatDouble(testValMagnitudeLt1Pos).c_str());
    EXPECT_STREQ("-1.25e-4", nfs.FormatDouble(testValMagnitudeLt1Neg).c_str());
    }

    // ScientificNorm
    {
    NumericFormatSpec nfs;
    nfs.SetPresentationType(PresentationType::Scientific);
    nfs.SetScientificType(ScientificType::Normal);
    nfs.SetDecimalPrecision(DecimalPrecision::Max);

    EXPECT_STREQ("0.125e-3", nfs.FormatDouble(testValMagnitudeLt1Pos).c_str());
    EXPECT_STREQ("-0.125e-3", nfs.FormatDouble(testValMagnitudeLt1Neg).c_str());
    }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatDoubleTest, FormatDoubleWithDifferentMinWidths)
    {
    NumericFormatSpec nfs;
    Utf8String string;

    nfs.SetMinWidth(0);
    EXPECT_STREQ("1000.0", nfs.FormatDouble(1000).c_str());

    nfs.SetMinWidth(6);
    EXPECT_STREQ("1000.0", nfs.FormatDouble(1000).c_str());

    nfs.SetMinWidth(8);
    EXPECT_STREQ("001000.0", nfs.FormatDouble(1000).c_str());
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatDoubleTest, FormatDoubleIndividualFormatTraitsTests)
    {
    Utf8String string;

    {
    NumericFormatSpec nfs;
    nfs.SetFormatTraits(FormatConstant::DefaultFormatTraits());
    EXPECT_STREQ("0", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("100501.125", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ("-100501.125", nfs.FormatDouble(testValNeg).c_str());
    EXPECT_STREQ("0", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("100501", nfs.FormatDouble(testValPosZeroFracComp).c_str());
    EXPECT_STREQ("-100501", nfs.FormatDouble(testValNegZeroFracComp).c_str());
    }

    {
    NumericFormatSpec nfs;
    nfs.SetFormatTraits(FormatTraits::KeepDecimalPoint);
    EXPECT_STREQ("0.", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("100501.", nfs.FormatDouble(testValPosZeroFracComp).c_str());
    EXPECT_STREQ("-100501.", nfs.FormatDouble(testValNegZeroFracComp).c_str());
    }

    {
    NumericFormatSpec nfs;
    nfs.SetFormatTraits(FormatTraits::KeepSingleZero);
    EXPECT_STREQ("0.0", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("100501.0", nfs.FormatDouble(testValPosZeroFracComp).c_str());
    EXPECT_STREQ("-100501.0", nfs.FormatDouble(testValNegZeroFracComp).c_str());
    }

    {
    NumericFormatSpec nfs;
    nfs.SetFormatTraits(FormatTraits::Use1000Separator);
    EXPECT_STREQ("0", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("100,501.125", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ("-100,501.125", nfs.FormatDouble(testValNeg).c_str());
    }

    {
    NumericFormatSpec nfs;
    // FormatDouble with a rounding factor X should produce formatted string
    // output O such that that X "fits" into the double representation of
    // O evenly.
    nfs.SetFormatTraits(FormatTraits::ApplyRounding);
    nfs.SetRoundingFactor(2);
    EXPECT_STREQ("0", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("100502", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ("-100502", nfs.FormatDouble(testValNeg).c_str());
    nfs.SetRoundingFactor(500);
    EXPECT_STREQ("0", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("100500", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ("-100500", nfs.FormatDouble(testValNeg).c_str());
    nfs.SetRoundingFactor(0.5);
    EXPECT_STREQ("0", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("100501", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ("-100501", nfs.FormatDouble(testValNeg).c_str());
    nfs.SetRoundingFactor(0.25);
    EXPECT_STREQ("0", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("100501.25", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ("-100501.25", nfs.FormatDouble(testValNeg).c_str());
    nfs.SetDecimalPrecision(DecimalPrecision::Precision6);
    nfs.SetRoundingFactor(0.1);
    EXPECT_STREQ("0", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("100501.1", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ("-100501.1", nfs.FormatDouble(testValNeg).c_str());
    nfs.SetRoundingFactor(0.1);
    EXPECT_STREQ("0", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("0.5", nfs.FormatDouble(0.54).c_str());
    EXPECT_STREQ("-0.5", nfs.FormatDouble(-0.54).c_str());
    }

    {
    NumericFormatSpec nfs;
    // AppendUnitName should have no effect on the unitless Format.
    nfs.SetFormatTraits(FormatTraits::ShowUnitLabel);
    EXPECT_STREQ("0", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("100501.125", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ("-100501.125", nfs.FormatDouble(testValNeg).c_str());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatDoubleTest, FormatDoubleIndividualFormatTraitsTests_IGNORED)
    {
    Utf8String string;

    {
    NumericFormatSpec nfs;
    nfs.SetFormatTraits(FormatTraits::LeadingZeroes);
    // TODO: Documentation refers to leading zeros for "digital expression" but never
    // defines what a digital expression means. So I don't actually know if this testing
    // is correct.
    nfs.SetMinWidth(15);
    EXPECT_STREQ("000000000000000", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("00000100501.125", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ("-0000100501.125", nfs.FormatDouble(testValNeg).c_str());

    nfs.SetMinWidth(0);
    EXPECT_STREQ("0", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("100501.125", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ("-100501.125", nfs.FormatDouble(testValNeg).c_str());
    }

    {
    NumericFormatSpec nfs;
    nfs.SetFormatTraits(FormatTraits::TrailingZeroes);

    nfs.SetMinWidth(15);
    EXPECT_STREQ("0000000000000000", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("100501.125000000", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ("-100501.12500000", nfs.FormatDouble(testValNeg).c_str());

    nfs.SetMinWidth(0);
    EXPECT_STREQ("0000000000000000", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("100501.125", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ("-100501.125", nfs.FormatDouble(testValNeg).c_str());
    }

    {
    NumericFormatSpec nfs;
    nfs.SetPresentationType(PresentationType::Scientific);
    nfs.SetDecimalPrecision(DecimalPrecision::Max);
    nfs.SetFormatTraits(FormatTraits::ExponentZero);
    EXPECT_STREQ("0e+00", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("1.00501125e+05", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ("-1.00501125e+05", nfs.FormatDouble(testValNeg).c_str());
    }

    {
    NumericFormatSpec nfs;
    nfs.SetFormatTraits(FormatTraits::ZeroEmpty);
    EXPECT_STREQ("", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("100501.125", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ("-100501.125", nfs.FormatDouble(testValNeg).c_str());
    }

    {
    NumericFormatSpec nfs;
    nfs.SetPresentationType(PresentationType::Fractional);
    nfs.SetFormatTraits(FormatTraits::FractionDash);
    EXPECT_STREQ("0", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("100501-1/8", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ("-100501-1/8", nfs.FormatDouble(testValNeg).c_str());
    nfs.SetPresentationType(PresentationType::Decimal);
    }

    //{
    //NumericFormatSpec nfs;
    //nfs.SetFormatTraits(FormatTraits::UseFractSymbol);
    //EXPECT_STREQ("0", nfs.FormatDouble(0).c_str());
    //EXPECT_STREQ(u8"100501 \x21\x5B", nfs.FormatDouble(testValPos).c_str());
    //EXPECT_STREQ(u8"-100501 \x21\x5B", nfs.FormatDouble(testValNeg).c_str());
    //}
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatTest, Constructors)
    {
    // Default constructed.
    {
    Format namedFmtSpec;

    EXPECT_EQ(FormatSpecType::None, namedFmtSpec.GetSpecType());
    EXPECT_TRUE(namedFmtSpec.IsProblem());
    EXPECT_EQ(nullptr, namedFmtSpec.GetNumericSpec());
    EXPECT_EQ(nullptr, namedFmtSpec.GetCompositeSpec());
    }

    // Constructed with NumericFormatSpec
    {
    NumericFormatSpec numFmtSpec;
    Format namedFmtSpec(numFmtSpec);

    EXPECT_EQ(FormatSpecType::None, namedFmtSpec.GetSpecType());
    EXPECT_FALSE(namedFmtSpec.IsProblem());
    ASSERT_NE(nullptr, namedFmtSpec.GetNumericSpec());
    EXPECT_TRUE(namedFmtSpec.GetNumericSpec()->IsIdentical(numFmtSpec));
    EXPECT_EQ(nullptr, namedFmtSpec.GetCompositeSpec());
    }

    // Constructed with name, NumericFormatSpec, and CompositeValueSpec
    {
    NumericFormatSpec numFmtSpec;
    CompositeValueSpec compValSpec(*s_unitsContext->LookupUnit("MILE"));
    Format namedFmtSpec(numFmtSpec, compValSpec);

    EXPECT_EQ(FormatSpecType::Single, namedFmtSpec.GetSpecType());
    EXPECT_FALSE(namedFmtSpec.IsProblem()) << namedFmtSpec.GetProblemDescription();
    ASSERT_NE(nullptr, namedFmtSpec.GetNumericSpec());
    EXPECT_TRUE(namedFmtSpec.GetNumericSpec()->IsIdentical(numFmtSpec));
    ASSERT_NE(nullptr, namedFmtSpec.GetCompositeSpec());
    // TODO: CompositeValueSpec::IsIdentical has a hacky unit proxy work around for now.
    //       Until that's cleaned up there's no point in testing the method or testing methods that
    //       call the method.
    //EXPECT_TRUE(namedFmtSpec.GetCompositeSpec()->IsIdentical(compValSpec));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatTest, IsIdentical)
    {
    // Test format specs of varying value types.
    // In these cases, only NumericFormatSpecs that are compared against themselves
    // should be concidered identical by the identity principal.
    {
    NumericFormatSpec numFmtSpec;
    CompositeValueSpec compValSpec(*s_unitsContext->LookupUnit("MILE"));

    Format namedFmtSpecUndefined;
    Format namedFmtSpecNumeric(numFmtSpec);
    Format namedFmtSpecComposite(numFmtSpec, compValSpec);

    // Identity checks.
    EXPECT_TRUE(namedFmtSpecUndefined.IsIdentical(namedFmtSpecUndefined))  << "Format is not identical to itself.";
    EXPECT_TRUE(namedFmtSpecNumeric.IsIdentical(namedFmtSpecNumeric)) << "Format is not identical to itself.";
    // TODO: CompositeValueSpec::IsIdentical has a hacky unit proxy work around for now.
    //       Until that's cleaned up there's no point in testing the method or testing methods that
    //       call the method.
    //EXPECT_TRUE(namedFmtSpecComposite.IsIdentical(namedFmtSpecComposite)) << "Format is not identical to itself.";;

    EXPECT_FALSE(namedFmtSpecUndefined.IsIdentical(namedFmtSpecNumeric));
    EXPECT_FALSE(namedFmtSpecUndefined.IsIdentical(namedFmtSpecComposite));

    EXPECT_FALSE(namedFmtSpecNumeric.IsIdentical(namedFmtSpecUndefined));
    EXPECT_FALSE(namedFmtSpecNumeric.IsIdentical(namedFmtSpecComposite));

    // TODO: CompositeValueSpec::IsIdentical has a hacky unit proxy work around for now.
    //       Until that's cleaned up there's no point in testing the method or testing methods that
    //       call the method.
    //EXPECT_FALSE(namedFmtSpecComposite.IsIdentical(namedFmtSpecUndefined));
    //EXPECT_FALSE(namedFmtSpecComposite.IsIdentical(namedFmtSpecNumeric));
    }

    // Formats with differing NumericFormatSpecs.
    {
    NumericFormatSpec numFmtSpecA;
    numFmtSpecA.SetDecimalPrecision(DecimalPrecision::Precision8);
    NumericFormatSpec numFmtSpecB;
    numFmtSpecB.SetDecimalPrecision(DecimalPrecision::Precision9);

    Format namedFmtSpecA(numFmtSpecA);
    Format namedFmtSpecB(numFmtSpecB);
    EXPECT_FALSE(namedFmtSpecA.IsIdentical(namedFmtSpecB));
    EXPECT_FALSE(namedFmtSpecB.IsIdentical(namedFmtSpecA));
    }

    // Formats with differing CompositeValueSpecs.
    {
    NumericFormatSpec numFmtSpec;
    BEU::UnitCP mile = s_unitsContext->LookupUnit("MILE");
    CompositeValueSpec compValSpecA(*mile);
    CompositeValueSpec compValSpecB;
    compValSpecB.SetSpacer(" # ");

    Format namedFmtSpecA(numFmtSpec, compValSpecA);
    Format namedFmtSpecB(numFmtSpec, compValSpecB);
    EXPECT_FALSE(namedFmtSpecA.IsIdentical(namedFmtSpecB));
    EXPECT_FALSE(namedFmtSpecB.IsIdentical(namedFmtSpecA));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatTest, StdFormatQuantityUsesThousandSeparatorForAllUnits)
    {
    NumericFormatSpec numericFormatSpec;
    numericFormatSpec.SetThousandSeparator('\'');
    numericFormatSpec.SetUse1000Separator(true);
    numericFormatSpec.SetKeepSingleZero(true);

    BEU::UnitCP mile = s_unitsContext->LookupUnit("MILE");
    BEU::UnitCP inch = s_unitsContext->LookupUnit("IN");
    CompositeValueSpec compositeValueSpec(*mile, *inch);
    ASSERT_EQ(2, compositeValueSpec.GetUnitCount());
    Format Format(numericFormatSpec, compositeValueSpec);

    // 1500.5 miles == 1,500 miles and 31,680 inches
    BEU::Quantity quantity(1500.5, *compositeValueSpec.GetMajorUnit());
    EXPECT_STREQ("1'500.0 31'680.0", Format::StdFormatQuantity(Format, quantity).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatTest, ParseFormatString)
    {
    // Valid format strings.
    Utf8String const fmtStrBasic("ExampleFmt<9>");
    Utf8String const fmtStrBasicTrailingComma("ExampleFmt<9,>");
    Utf8String const fmtStrBasicNoOverrides("ExampleFmt");
    Utf8String const fmtStrFutureAddition("ExampleFmt<9,banana>");
    Utf8String const fmtStrFutureAdditionWhiteSpace("ExampleFmt \n < 9 \t , banana > ");
    Utf8String const fmtStrFutureAdditionTrailingComma("ExampleFmt<9,banana,>");
    Utf8String const fmtStrFutureAdditionNoFirstOverride("ExampleFmt<,banana>");

    // Invalid format strings.
    Utf8String const fmtStrBasicNoOverridesButStillHasBrackets("ExampleFmt<>");
    Utf8String const fmtStrBasicNoOverridesButStillHasBracketsWithCommas("ExampleFmt<,,,,>");

    // Mapping function that returns nullptr should always fail.
    {
    static auto const nullMapper = [](Utf8StringCR name) -> FormatCP {return nullptr;};
    Format parsedNfs;

    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrBasic, nullMapper));
    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrBasicTrailingComma, nullMapper));
    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrBasicNoOverrides, nullMapper));
    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrFutureAddition, nullMapper));
    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrFutureAdditionWhiteSpace, nullMapper));
    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrFutureAdditionTrailingComma, nullMapper));
    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrFutureAdditionNoFirstOverride, nullMapper));

    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrBasicNoOverridesButStillHasBrackets, nullMapper));
    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrBasicNoOverridesButStillHasBracketsWithCommas, nullMapper));
    }

    // Parsing with a defined mapping function.
    {
    NumericFormatSpec exampleNumericFmtSpec;
    exampleNumericFmtSpec.SetPresentationType(PresentationType::Decimal);
    exampleNumericFmtSpec.SetDecimalPrecision(DecimalPrecision::Precision9);
    Format const exampleNamedFmtSpec(exampleNumericFmtSpec);
    auto const mapper = [exampleNamedFmtSpec](Utf8StringCR name) -> FormatCP
        {
        return name == "ExampleFmt" ? &exampleNamedFmtSpec : nullptr;
        };
    Format parsedNfs;

    ASSERT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrBasic, mapper));
    EXPECT_TRUE(parsedNfs.IsIdentical(exampleNamedFmtSpec));

    ASSERT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrBasicTrailingComma, mapper));
    EXPECT_TRUE(parsedNfs.IsIdentical(exampleNamedFmtSpec));

    ASSERT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrBasicNoOverrides, mapper));
    EXPECT_TRUE(parsedNfs.IsIdentical(exampleNamedFmtSpec));

    ASSERT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrFutureAddition, mapper));
    EXPECT_TRUE(parsedNfs.IsIdentical(exampleNamedFmtSpec));

    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrFutureAdditionWhiteSpace, mapper));
    EXPECT_TRUE(parsedNfs.IsIdentical(exampleNamedFmtSpec));

    ASSERT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrFutureAdditionTrailingComma, mapper));
    EXPECT_TRUE(parsedNfs.IsIdentical(exampleNamedFmtSpec));

    ASSERT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrFutureAdditionNoFirstOverride, mapper));
    EXPECT_TRUE(parsedNfs.IsIdentical(exampleNamedFmtSpec));

    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrBasicNoOverridesButStillHasBrackets, mapper));
    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrBasicNoOverridesButStillHasBracketsWithCommas, mapper));
    }

    // Test using different expected format types.
    {
    NumericFormatSpec exampleNumericFmtSpecDec;
    exampleNumericFmtSpecDec.SetPresentationType(PresentationType::Decimal);
    exampleNumericFmtSpecDec.SetDecimalPrecision(DecimalPrecision::Precision5);
    Format const exampleNamedFmtSpecDec(exampleNumericFmtSpecDec);

    NumericFormatSpec exampleNumericFmtSpecFrac;
    exampleNumericFmtSpecFrac.SetPresentationType(PresentationType::Fractional);
    exampleNumericFmtSpecFrac.SetFractionalPrecision(FractionalPrecision::Over_128);
    Format const exampleNamedFmtSpecFrac(exampleNumericFmtSpecFrac);

    NumericFormatSpec exampleNumericFmtSpecSci;
    exampleNumericFmtSpecSci.SetDecimalPrecision(DecimalPrecision::Precision4);
    exampleNumericFmtSpecSci.SetPresentationType(PresentationType::Scientific);
    Format const exampleNamedFmtSpecSci(exampleNumericFmtSpecSci);

    NumericFormatSpec exampleNumericFmtSpecSciNorm;
    exampleNumericFmtSpecSciNorm.SetDecimalPrecision(DecimalPrecision::Precision7);
    exampleNumericFmtSpecSciNorm.SetPresentationType(PresentationType::Scientific);
    exampleNumericFmtSpecSciNorm.SetScientificType(ScientificType::Normal);
    Format const exampleNamedFmtSpecSciNorm(exampleNumericFmtSpecSciNorm);

    NumericFormatSpec exampleNumericFmtSpecStation;
    exampleNumericFmtSpecStation.SetPresentationType(PresentationType::Station);
    exampleNumericFmtSpecStation.SetStationOffsetSize(2);
    exampleNumericFmtSpecStation.SetDecimalPrecision(DecimalPrecision::Precision9);
    Format const exampleNamedFmtSpecStation(exampleNumericFmtSpecStation);
    auto const mapper = [&](Utf8StringCR name) -> FormatCP
        {
        if (name == "ExDec")
            return &exampleNamedFmtSpecDec;
        if (name == "ExFrac")
            return &exampleNamedFmtSpecFrac;
        if (name == "ExSci")
            return &exampleNamedFmtSpecSci;
        if (name == "ExSciNorm")
            return &exampleNamedFmtSpecSciNorm;
        if (name == "ExStation")
            return &exampleNamedFmtSpecStation;
        return nullptr;
        };
    Format parsedNfs;

    ASSERT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, "ExDec<5>", mapper));
    EXPECT_TRUE(parsedNfs.IsIdentical(exampleNamedFmtSpecDec));
    ASSERT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, "ExFrac<128>", mapper));
    EXPECT_TRUE(parsedNfs.IsIdentical(exampleNamedFmtSpecFrac));
    ASSERT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, "ExSci<4>", mapper));
    EXPECT_TRUE(parsedNfs.IsIdentical(exampleNamedFmtSpecSci));
    ASSERT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, "ExSciNorm<7>", mapper));
    EXPECT_TRUE(parsedNfs.IsIdentical(exampleNamedFmtSpecSciNorm));
    ASSERT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, "ExStation<9>", mapper));
    EXPECT_TRUE(parsedNfs.IsIdentical(exampleNamedFmtSpecStation));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
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
// @bsimethod                                    Victor.Cushman                  03/18
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
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatParsingSetTest, IdentityAlwaysLeadingPlusAndMinus)
    {
    TestValidParseToQuantity("+3 FT", foot, 3.0);
    TestValidParseToQuantity("-3 FT", foot, -3.0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
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
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatParsingSetTest, SingleUnitConvertedToAnotherUnit)
    {
    TestValidParseToQuantity("1 MILE", yard, 1760);
    TestValidParseToQuantity("1 MILE", foot, 5280);
    TestValidParseToQuantity("1 MILE", inch, 63360);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
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
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatParsingSetTest, FractionCanUseUnderscoreBetweenIntegerAndFractionalComponent)
    {
    TestValidParseToQuantity("1_1/2 FT", foot, 1.5);
    TestValidParseToQuantity("-1_1/2 FT", foot, -1.5);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
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
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatParsingSetTest, CompositeFormats)
    {
    TestValidParseToQuantity("2 FT 6 IN", foot, 2*1  + 6.f/12.f);
    TestValidParseToQuantity("2 FT 6 IN", inch, 2*12 + 6*1);

    TestValidParseToQuantity("3 YRD 2 FT 6 IN", yard, 3*1  + 2.f/3.f + 6.f/36.f);
    TestValidParseToQuantity("3 YRD 2 FT 6 IN", foot, 3*3  + 2*1     + 6.f/12.f);
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

TEST_F(FormatParsingSetTest, CompositeFormats_IGNORED)
    {
    TestValidParseToQuantity(u8"135°11'30 1/4\" ", arcDeg, 135.191736);

    TestValidParseToQuantity("1 MILE 3 YRD 2 FT 6 IN", mile, 1*1     + 3.f/1760.f + 2.f/5280.f + 6.f/63360.f);
    TestValidParseToQuantity("1 MILE 3 YRD 2 FT 6 IN", yard, 1*1760  + 3*1        + 2.f/3.f    + 6.f/36.f);
    TestValidParseToQuantity("1 MILE 3 YRD 2 FT 6 IN", foot, 1*5280  + 3*3        + 2*1        + 6.f/12.f);
    TestValidParseToQuantity("1 MILE 3 YRD 2 FT 6 IN", inch, 1*63360 + 3*36       + 2*12       + 6*1);

    TestValidParseToQuantity("2 1/2 FT 6 IN", inch, 2.5*12 + 6*1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatParsingSetTest, TestParseToStd)
    {
    // Move to common spot to reuse within tests.
    CompositeValueSpecP fi = new CompositeValueSpec(*foot, *inch);
    fi->SetMajorLabel("'");
    fi->SetMiddleLabel("\"");

    NumericFormatSpecP numeric8 = new NumericFormatSpec();
    numeric8->SetFormatTraits(FormatTraits::KeepSingleZero);
    numeric8->SetFormatTraits(FormatTraits::KeepDecimalPoint);
    numeric8->SetPresentationType(PresentationType::Fractional);
    numeric8->SetFractionalPrecision(FractionalPrecision::Eighth);
        
    Format fi8 = Format(*numeric8, *fi);

    // Format dms8 = Format();

    // Format real = Format();

    TestValidParseToQuantityUsingStdFmt("5:6", inch, 5.5, &fi8);
    TestValidParseToQuantityUsingStdFmt("5:", inch, 5.0, &fi8);
    TestValidParseToQuantityUsingStdFmt(":6", inch, 0.5, &fi8);

    /*TestValidParseToQuantityUsingStdFmt("135:23:11", arcDeg, 1, &dms8);
    TestValidParseToQuantityUsingStdFmt("3  1/5 FT", inch, 3.2, &real);*/
    }

END_BENTLEY_FORMATTEST_NAMESPACE
