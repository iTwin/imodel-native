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
// FormatUnitSet
//===================================================
struct FormatUnitSetTest : FormattingTestFixture
    {
    // Method for printing GTest FUS problem description error.
    Utf8String GetTestFusProblemDescription(FormatUnitSetCR fus)
        {
        return Utf8String("Problem: ") + fus.GetProblemDescription();
        }
    };

//===================================================
// NamedFormatSpec
//===================================================
struct NamedFormatSpecTest : FormattingTestFixture
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
        FormatUnitSet fus = FormatUnitSet(expectedUnit->GetName().c_str(), s_unitsContext);
        BEU::Quantity qty = fps.GetQuantity(&probCode, &fus);

        ASSERT_FALSE(qty.IsNullQuantity()) << GetFmtStringErrMsg(fmtStr);
        EXPECT_EQ(expectedUnit, qty.GetUnit()) << GetFmtStringErrMsg(fmtStr);
        EXPECT_DOUBLE_EQ(expectedMagnitude, qty.GetMagnitude()) << GetFmtStringErrMsg(fmtStr);
        }

    void TestValidParseToQuantityUsingStdFmt(Utf8CP fmtStr, BEU::UnitCP expectedUnit, double const expectedMagnitude, Utf8CP stdFmtDescription)
        {
        FormatParsingSet fps(fmtStr, expectedUnit);
        ASSERT_FALSE(fps.HasProblem()) << fps.GetProblemDescription() << '\n' << GetFmtStringErrMsg(fmtStr);

        FormatProblemCode probCode;
        FormatUnitSet fus = FormatUnitSet(stdFmtDescription, s_unitsContext);
        BEU::Quantity qty = fps.GetQuantity(&probCode, &fus);

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
    EXPECT_EQ(FormatConstant::DefaultFractionBarType(), nfs.GetFractionalBarType());
    EXPECT_EQ(FormatConstant::DefaultDecimalSeparator(), nfs.GetDecimalSeparator());
    EXPECT_EQ(FormatConstant::DefaultThousandSeparator(), nfs.GetThousandSeparator());
    EXPECT_EQ(FormatConstant::DefaultUomSeparator(), nfs.GetUomSeparator());
    EXPECT_EQ(FormatConstant::DefaultStopSeparator(), nfs.GetStopSeparator());
    EXPECT_EQ(FormatConstant::DefaultMinWidth(), nfs.GetMinWidth());
    EXPECT_EQ(FormatConstant::DefaultMinWidth(), nfs.GetMinWidth());
    }

    // Constructed with provided precision.
    {
    NumericFormatSpec nfs(DecimalPrecision::Precision9);
    EXPECT_DOUBLE_EQ(FormatConstant::DefaultRoundingFactor(), nfs.GetRoundingFactor());
    EXPECT_EQ(FormatConstant::DefaultPresentaitonType(), nfs.GetPresentationType());
    EXPECT_EQ(FormatConstant::DefaultSignOption(), nfs.GetSignOption());
    EXPECT_EQ(FormatConstant::DefaultFormatTraits(), nfs.GetFormatTraits());
    EXPECT_EQ(DecimalPrecision::Precision9, nfs.GetDecimalPrecision());
    EXPECT_EQ(FormatConstant::DefaultFractionalPrecision(), nfs.GetFractionalPrecision());
    EXPECT_EQ(FormatConstant::DefaultFractionBarType(), nfs.GetFractionalBarType());
    EXPECT_EQ(FormatConstant::DefaultDecimalSeparator(), nfs.GetDecimalSeparator());
    EXPECT_EQ(FormatConstant::DefaultThousandSeparator(), nfs.GetThousandSeparator());
    EXPECT_EQ(FormatConstant::DefaultUomSeparator(), nfs.GetUomSeparator());
    EXPECT_EQ(FormatConstant::DefaultStopSeparator(), nfs.GetStopSeparator());
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
    NumericFormatSpec nfsB(DecimalPrecision::Max);

    EXPECT_TRUE(nfsA.IsIdentical(nfsA)) << "NumericFormatSpec is not identical to itself.";
    EXPECT_FALSE(nfsA.IsIdentical(nfsB));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatIntegerTest, FormatIntegerSignOptionTests)
    {
    NumericFormatSpec nfs;
    Utf8String string;

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
    nfs.SetPresentationType(PresentationType::ScientificNorm);
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
    nfs.SetFormatTraits(FormatTraits::UseFractSymbol);
    EXPECT_STREQ("0", nfs.FormatInteger(0).c_str());
    EXPECT_STREQ("100501", nfs.FormatInteger(testValPos).c_str());
    EXPECT_STREQ("-100501", nfs.FormatInteger(testValNeg).c_str());

    // AppendUnitName should have no effect on the unitless NamedFormatSpec.
    nfs.SetFormatTraits(FormatTraits::AppendUnitName);
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
    nfs.SetPresentationType(PresentationType::ScientificNorm);
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
    nfs.SetPresentationType(PresentationType::ScientificNorm);
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
    EXPECT_STREQ("0.0", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("100501.125", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ("-100501.125", nfs.FormatDouble(testValNeg).c_str());
    EXPECT_STREQ("0.0", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("100501.0", nfs.FormatDouble(testValPosZeroFracComp).c_str());
    EXPECT_STREQ("-100501.0", nfs.FormatDouble(testValNegZeroFracComp).c_str());
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
    // AppendUnitName should have no effect on the unitless NamedFormatSpec.
    nfs.SetFormatTraits(FormatTraits::AppendUnitName);
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

    {
    NumericFormatSpec nfs;
    nfs.SetFormatTraits(FormatTraits::UseFractSymbol);
    EXPECT_STREQ("0", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ(u8"100501 \x21\x5B", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ(u8"-100501 \x21\x5B", nfs.FormatDouble(testValNeg).c_str());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(NumericFormatSpecTest, StdFormatQuantityUsesThousandSeparatorForAllUnits)
    {
    NumericFormatSpec numericFormatSpec;
    numericFormatSpec.SetThousandSeparator('\'');
    numericFormatSpec.SetUse1000Separator(true);
    numericFormatSpec.SetKeepSingleZero(true);

    BEU::UnitCP mile = s_unitsContext->LookupUnit("MILE");
    BEU::UnitCP inch = s_unitsContext->LookupUnit("IN");
    CompositeValueSpec compositeValueSpec(*mile, *inch);
    ASSERT_EQ(2, compositeValueSpec.GetUnitCount());
    NamedFormatSpec namedFormatSpec("TestNamedFormatSpec", numericFormatSpec, compositeValueSpec);

    // 1500.5 miles == 1,500 miles and 31,680 inches
    BEU::Quantity quantity(1500.5, *compositeValueSpec.GetMajorUnit());
    EXPECT_STREQ("1'500.0 31'680.0", NumericFormatSpec::StdFormatQuantity(namedFormatSpec, quantity).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatUnitSetTest, ConstructFusFromDescription)
    {
    {
    FormatUnitSet MMFusNoFormatName("MM", s_unitsContext);
    EXPECT_FALSE(MMFusNoFormatName.HasProblem()) << FormatUnitSetTest::GetTestFusProblemDescription(MMFusNoFormatName);
    EXPECT_STREQ("MM(DefaultReal)", MMFusNoFormatName.ToText().c_str());
    EXPECT_STREQ("MM(Real)", MMFusNoFormatName.ToText().c_str());
    }
    {
    FormatUnitSet MMFusParens("MM(Real2)", s_unitsContext);
    EXPECT_FALSE(MMFusParens.HasProblem()) << FormatUnitSetTest::GetTestFusProblemDescription(MMFusParens);
    EXPECT_STREQ("MM(Real2)", MMFusParens.ToText().c_str());
    }
    {
    FormatUnitSet MMFusBarFormatName("MM|Real2", s_unitsContext);
    EXPECT_FALSE(MMFusBarFormatName.HasProblem()) << FormatUnitSetTest::GetTestFusProblemDescription(MMFusBarFormatName);
    EXPECT_STREQ("MM(Real2)", MMFusBarFormatName.ToText().c_str());
    }
    {
    FormatUnitSet MMFusBarFormatNameBar("MM|Real2|", s_unitsContext);
    EXPECT_FALSE(MMFusBarFormatNameBar.HasProblem()) << FormatUnitSetTest::GetTestFusProblemDescription(MMFusBarFormatNameBar);
    EXPECT_STREQ("MM(Real2)", MMFusBarFormatNameBar.ToText().c_str());
    }
    {
    FormatUnitSet MMFusJsonOnlyUnitName(R"json({
        unitName: "MM"
    })json", s_unitsContext);
    EXPECT_FALSE(MMFusJsonOnlyUnitName.HasProblem()) << FormatUnitSetTest::GetTestFusProblemDescription(MMFusJsonOnlyUnitName);
    EXPECT_STREQ("MM(Real2)", MMFusJsonOnlyUnitName.ToText().c_str());
    EXPECT_STREQ("MM(real2)", MMFusJsonOnlyUnitName.ToText().c_str());
    EXPECT_TRUE(MMFusJsonOnlyUnitName.IsIdentical(FormatUnitSet("MM(Real2)", s_unitsContext)));
    }
    // TODO: The below JSON methods aren't parsing the formatName attribute correctly. I tried
    // "formatName": "Real2"
    // as well as
    // "formatName": "real2"
    // Neither works and I'm not sure whether I'm doing something wrong or the library has a bug.
    // In any case, the documentation doesn't explain how the json description constructor works,
    // even if though its format is described.
    {
    FormatUnitSet MMFusJsonUnitNameFormatName(R"json({
        "unitName": "MM",
        "formatName": "real2"
    })json", s_unitsContext);
    EXPECT_FALSE(MMFusJsonUnitNameFormatName.HasProblem()) << FormatUnitSetTest::GetTestFusProblemDescription(MMFusJsonUnitNameFormatName);
    EXPECT_STREQ("MM(Real2)", MMFusJsonUnitNameFormatName.ToText().c_str());
    EXPECT_STREQ("MM(real2)", MMFusJsonUnitNameFormatName.ToText().c_str());
    EXPECT_TRUE(MMFusJsonUnitNameFormatName.IsIdentical(FormatUnitSet("MM(Real2)", s_unitsContext)));
    }
    {
    FormatUnitSet MMFusJsonAllMembers(R"json({
        "unitName": "MM",
        "formatName": "real2",
        "cloneData": false,
        "formatSpec": { }
    })json", s_unitsContext);
    EXPECT_FALSE(MMFusJsonAllMembers.HasProblem()) << FormatUnitSetTest::GetTestFusProblemDescription(MMFusJsonAllMembers);
    EXPECT_STREQ("MM(Real2)", MMFusJsonAllMembers.ToText().c_str());
    EXPECT_STREQ("MM(real2)", MMFusJsonAllMembers.ToText().c_str());
    EXPECT_TRUE(MMFusJsonAllMembers.IsIdentical(FormatUnitSet("MM(Real2)", s_unitsContext)));
    }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            David.Fox-Rabinovitz                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FormattingTest, LocaleTest)
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
}

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(NamedFormatSpecTest, Constructors)
    {
    // Default constructed.
    {
    NamedFormatSpec namedFmtSpec;

    EXPECT_STREQ("", namedFmtSpec.GetName());
    EXPECT_STREQ("", namedFmtSpec.GetDescription());
    EXPECT_STREQ("", namedFmtSpec.GetDisplayLabel());
    EXPECT_EQ(FormatSpecType::None, namedFmtSpec.GetSpecType());
    EXPECT_TRUE(namedFmtSpec.IsProblem());
    EXPECT_EQ(nullptr, namedFmtSpec.GetNumericSpec());
    EXPECT_EQ(nullptr, namedFmtSpec.GetCompositeSpec());
    }

    // Constructed with name and NumericFormatSpec
    {
    NumericFormatSpec numFmtSpec(DecimalPrecision::Precision9);
    NamedFormatSpec namedFmtSpec("FooBar", numFmtSpec);

    EXPECT_STREQ("FooBar", namedFmtSpec.GetName());
    EXPECT_STREQ("", namedFmtSpec.GetDescription());
    EXPECT_STREQ("", namedFmtSpec.GetDisplayLabel());
    EXPECT_EQ(FormatSpecType::None, namedFmtSpec.GetSpecType());
    EXPECT_FALSE(namedFmtSpec.IsProblem());
    ASSERT_NE(nullptr, namedFmtSpec.GetNumericSpec());
    EXPECT_TRUE(namedFmtSpec.GetNumericSpec()->IsIdentical(numFmtSpec));
    EXPECT_EQ(nullptr, namedFmtSpec.GetCompositeSpec());
    }

    // Constructed with name, NumericFormatSpec, and CompositeValueSpec
    {
    NumericFormatSpec numFmtSpec(DecimalPrecision::Precision9);
    CompositeValueSpec compValSpec(*s_unitsContext->LookupUnit("MILE"));
    NamedFormatSpec namedFmtSpec("FooBar", numFmtSpec, compValSpec);

    EXPECT_STREQ("FooBar", namedFmtSpec.GetName());
    EXPECT_STREQ("", namedFmtSpec.GetDescription());
    EXPECT_STREQ("", namedFmtSpec.GetDisplayLabel());
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
TEST_F(NamedFormatSpecTest, IsIdentical)
    {
    // Test format specs of varying value types.
    // In these cases, only NumericFormatSpecs that are compared against themselves
    // should be concidered identical by the identity principal.
    {
    NumericFormatSpec numFmtSpec(DecimalPrecision::Precision9);
    CompositeValueSpec compValSpec(*s_unitsContext->LookupUnit("MILE"));

    NamedFormatSpec namedFmtSpecUndefined;
    NamedFormatSpec namedFmtSpecNumeric("FooBar", numFmtSpec);
    NamedFormatSpec namedFmtSpecComposite("FooBar", numFmtSpec, compValSpec);

    // Identity checks.
    EXPECT_TRUE(namedFmtSpecUndefined.IsIdentical(namedFmtSpecUndefined))  << "NamedFormatSpec is not identical to itself.";
    EXPECT_TRUE(namedFmtSpecNumeric.IsIdentical(namedFmtSpecNumeric)) << "NamedFormatSpec is not identical to itself.";
    // TODO: CompositeValueSpec::IsIdentical has a hacky unit proxy work around for now.
    //       Until that's cleaned up there's no point in testing the method or testing methods that
    //       call the method.
    //EXPECT_TRUE(namedFmtSpecComposite.IsIdentical(namedFmtSpecComposite)) << "NamedFormatSpec is not identical to itself.";;

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

    // NamedFormatSpecs with differing names/displayLabels/descriptions.
    {
    NumericFormatSpec numFmtSpec;

    NamedFormatSpec namedFmtSpecFoo("Foo", numFmtSpec);
    NamedFormatSpec namedFmtSpecBaz("Baz", numFmtSpec);
    EXPECT_FALSE(namedFmtSpecFoo.IsIdentical(namedFmtSpecBaz));
    EXPECT_FALSE(namedFmtSpecBaz.IsIdentical(namedFmtSpecFoo));

    NamedFormatSpec namedFmtSpecFooDiffDispLabel("Foo", numFmtSpec);
    namedFmtSpecFooDiffDispLabel.SetDisplayLabel("FOO");
    EXPECT_TRUE(namedFmtSpecFoo.IsIdentical(namedFmtSpecFooDiffDispLabel));
    EXPECT_TRUE(namedFmtSpecFooDiffDispLabel.IsIdentical(namedFmtSpecFoo));

    NamedFormatSpec namedFmtSpecFooDiffDesc("Foo", numFmtSpec);
    namedFmtSpecFooDiffDesc.SetDescription("The most common metasyntactic variable.");
    EXPECT_TRUE(namedFmtSpecFoo.IsIdentical(namedFmtSpecFooDiffDesc));
    EXPECT_TRUE(namedFmtSpecFooDiffDesc.IsIdentical(namedFmtSpecFoo));
    }

    // NamedFormatSpecs with differing NumericFormatSpecs.
    {
    NumericFormatSpec numFmtSpecA(DecimalPrecision::Precision8);
    NumericFormatSpec numFmtSpecB(DecimalPrecision::Precision9);

    NamedFormatSpec namedFmtSpecA("FooBar", numFmtSpecA);
    NamedFormatSpec namedFmtSpecB("FooBar", numFmtSpecB);
    EXPECT_FALSE(namedFmtSpecA.IsIdentical(namedFmtSpecB));
    EXPECT_FALSE(namedFmtSpecB.IsIdentical(namedFmtSpecA));
    }

    // NamedFormatSpecs with differing CompositeValueSpecs.
    {
    NumericFormatSpec numFmtSpec;
    BEU::UnitCP mile = s_unitsContext->LookupUnit("MILE");
    CompositeValueSpec compValSpecA(*mile);
    CompositeValueSpec compValSpecB;
    compValSpecB.SetSpacer(" # ");

    NamedFormatSpec namedFmtSpecA("FooBar", numFmtSpec, compValSpecA);
    NamedFormatSpec namedFmtSpecB("FooBar", numFmtSpec, compValSpecB);
    EXPECT_FALSE(namedFmtSpecA.IsIdentical(namedFmtSpecB));
    EXPECT_FALSE(namedFmtSpecB.IsIdentical(namedFmtSpecA));
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
    TestValidParseToQuantityUsingStdFmt("5:6", inch, 5.5, "fi8");
    TestValidParseToQuantityUsingStdFmt("5:", inch, 5.0, "fi8");
    TestValidParseToQuantityUsingStdFmt(":6", inch, 0.5, "fi8");

    TestValidParseToQuantityUsingStdFmt("135:23:11", arcDeg, 1, "dms8");
    TestValidParseToQuantityUsingStdFmt("3  1/5 FT", inch, 3.2, "real");
    }

END_BENTLEY_FORMATTEST_NAMESPACE
