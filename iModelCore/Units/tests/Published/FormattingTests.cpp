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

struct FormattingTest : public ::testing::Test
    {
    template <typename T, size_t N>
    static constexpr size_t Countof(T const (&arr)[N]) { return std::extent<T[N]>::value; }
    };

//===================================================
// NumericFormatSpec
//===================================================
struct NumericFormatSpecTest : public FormattingTest
    {
    };

struct BinaryFormattingTest : public NumericFormatSpecTest
    {
    struct TestByte
        {
        static Byte const allBitsZero = 0x0;
        static Byte const allBitsOne = 0xFF;
        static Byte const charA = 'A';
        };

    struct TestInt16
        {
        static int16_t const allBitsZero = 0;
        static int16_t const allBitsOne = ~0;
        static short const testVal = 0b01011010'10010110;
        };
    };

struct FormatIntegerTest : public NumericFormatSpecTest
    {
    };

struct FormatDoubleTest : public NumericFormatSpecTest
    {
    };

//===================================================
// FormatUnitSet
//===================================================
struct FormatUnitSetTest : public FormattingTest
    {
    // Method for printing GTest FUS problem description error.
    Utf8String GetTestFusProblemDescription(FormatUnitSetCR fus)
        {
        return Utf8String("Problem: ") + fus.GetProblemDescription();
        }
    };

//===================================================
// CompositeValueSpec
//===================================================
//===================================================
// CompositeValue
//===================================================
//===================================================
// NamedFormatSpec
//===================================================
struct NamedFormatSpecTest : public FormattingTest
    {
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

    FAIL() << "TODO: Test NumericFormatSpec constructed from JSON";
    }

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
TEST_F(BinaryFormattingTest, FormatByte)
    {
    NumericFormatSpec nfs;
    EXPECT_STREQ("00000000", nfs.ByteToBinaryText(BinaryFormattingTest::TestByte::allBitsZero).c_str());
    EXPECT_STREQ("11111111", nfs.ByteToBinaryText(BinaryFormattingTest::TestByte::allBitsOne).c_str());
    EXPECT_STREQ("01000001", nfs.ByteToBinaryText(BinaryFormattingTest::TestByte::charA).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(BinaryFormattingTest, FormatInt16)
    {
    NumericFormatSpec nfsComma;
    nfsComma.SetThousandSeparator(',');
    nfsComma.SetUse1000Separator(true);
    NumericFormatSpec nfsPlus;
    nfsPlus.SetThousandSeparator('+');
    nfsPlus.SetUse1000Separator(true);

    EXPECT_STREQ("0000000000000000", nfsComma.Int16ToBinaryText(BinaryFormattingTest::TestInt16::allBitsZero, false).c_str());
    EXPECT_STREQ("00000000,00000000", nfsComma.Int16ToBinaryText(BinaryFormattingTest::TestInt16::allBitsZero, true).c_str());
    EXPECT_STREQ("00000000+00000000", nfsPlus.Int16ToBinaryText(BinaryFormattingTest::TestInt16::allBitsZero, true).c_str());

    EXPECT_STREQ("1111111111111111", nfsComma.Int16ToBinaryText(BinaryFormattingTest::TestInt16::allBitsOne, false).c_str());
    EXPECT_STREQ("11111111,11111111", nfsComma.Int16ToBinaryText(BinaryFormattingTest::TestInt16::allBitsOne, true).c_str());
    EXPECT_STREQ("11111111+11111111", nfsPlus.Int16ToBinaryText(BinaryFormattingTest::TestInt16::allBitsOne, true).c_str());

    EXPECT_STREQ("0101101010010110", nfsComma.Int16ToBinaryText(BinaryFormattingTest::TestInt16::testVal, false).c_str());
    EXPECT_STREQ("01011010,10010110", nfsComma.Int16ToBinaryText(BinaryFormattingTest::TestInt16::testVal, true).c_str());
    EXPECT_STREQ("01011010+10010110", nfsPlus.Int16ToBinaryText(BinaryFormattingTest::TestInt16::testVal, true).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(BinaryFormattingTest, FormatInt32)
    {
    FAIL() << "TODO: Untested methods are failing methods.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(BinaryFormattingTest, FormatDouble)
    {
    FAIL() << "TODO: Untested methods are failing methods.";
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
    NumericFormatSpec nfs;
    Utf8String string;
    int const testValPos = 100501;
    int const testValNeg = -100501;

    // Decimal
    {
    nfs.SetPresentationType(PresentationType::Decimal);
    EXPECT_STREQ("100501", nfs.FormatInteger(testValPos).c_str());
    EXPECT_STREQ("-100501", nfs.FormatInteger(testValNeg).c_str());
    }

    // Fractional
    {
    nfs.SetPresentationType(PresentationType::Fractional);
    EXPECT_STREQ("100501", nfs.FormatInteger(testValPos).c_str());
    EXPECT_STREQ("-100501", nfs.FormatInteger(testValNeg).c_str());
    }

    // TODO: The below Scientific and Scientific Norm tests fail since the testVal integer does not
    // ever get formatted as a value with a decimal point. Since it is not obvious to the user
    // how the scientific notation affects formatting of integers.
    // Scientific
    {
    nfs.SetPresentationType(PresentationType::Scientific);
    nfs.SetDecimalPrecision(DecimalPrecision::Max);
    EXPECT_STREQ("1.00501e+5", nfs.FormatInteger(testValPos).c_str());
    EXPECT_STREQ("-11.00501e+5", nfs.FormatInteger(testValNeg).c_str());
    }

    // ScientificNorm
    {
    nfs.SetPresentationType(PresentationType::ScientificNorm);
    nfs.SetDecimalPrecision(DecimalPrecision::Max);
    EXPECT_STREQ("0.100501e+6", nfs.FormatInteger(testValPos).c_str());
    EXPECT_STREQ("-0.100501e+6", nfs.FormatInteger(testValNeg).c_str());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatIntegerTest, FormatIntegerFormatTraitsTests)
    {
    NumericFormatSpec nfs;
    Utf8String string;
    int const testValPos = 100501;
    int const testValNeg = -100501;

    nfs.SetFormatTraits(FormatTraits::DefaultTraits);
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

    FAIL() << "TODO: Test with the buffer version.";
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
    NumericFormatSpec nfs;
    Utf8String string;
    double const testValPos = 100501.125;
    double const testValNeg = -100501.125;
    double const testValMagnitudeLt1Pos = 0.000125;
    double const testValMagnitudeLt1Neg = -0.000125;

    // Decimal
    {
    nfs.SetPresentationType(PresentationType::Decimal);
    nfs.SetDecimalPrecision(DecimalPrecision::Max);

    EXPECT_STREQ("100501.125", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ("-100501.125", nfs.FormatDouble(testValNeg).c_str());
    }

    // Fractional
    {
    nfs.SetPresentationType(PresentationType::Fractional);
    nfs.SetDecimalPrecision(DecimalPrecision::Max);

    EXPECT_STREQ("100501 1/8", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ("-100501 1/8", nfs.FormatDouble(testValNeg).c_str());
    }

    // Scientific
    {
    nfs.SetPresentationType(PresentationType::Scientific);
    nfs.SetDecimalPrecision(DecimalPrecision::Max);

    EXPECT_STREQ("1.00501125e+5", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ("1.25e-4", nfs.FormatDouble(testValMagnitudeLt1Pos).c_str());
    EXPECT_STREQ("-1.00501125e+5", nfs.FormatDouble(testValNeg).c_str());
    EXPECT_STREQ("-1.25e-4", nfs.FormatDouble(testValMagnitudeLt1Neg).c_str());
    }

    // ScientificNorm
    {
    nfs.SetPresentationType(PresentationType::ScientificNorm);
    nfs.SetDecimalPrecision(DecimalPrecision::Max);

    EXPECT_STREQ("0.100501125e+6", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ("0.125e-3", nfs.FormatDouble(testValMagnitudeLt1Pos).c_str());
    EXPECT_STREQ("-0.100501125e+6", nfs.FormatDouble(testValNeg).c_str());
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
TEST_F(FormatDoubleTest, FormatDoubleFormatTraitsTests)
    {
    NumericFormatSpec nfs;
    Utf8String string;
    double const testValPos = 100501.125;
    double const testValNeg = -100501.125;
    double const testValPosZeroFracComp = 100501.0;
    double const testValNegZeroFracComp = -100501.0;

    nfs.SetFormatTraits(FormatTraits::DefaultTraits);
    EXPECT_STREQ("0", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("100501.125", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ("-100501.125", nfs.FormatDouble(testValNeg).c_str());
    EXPECT_STREQ("0", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("100501", nfs.FormatDouble(testValPosZeroFracComp).c_str());
    EXPECT_STREQ("-100501", nfs.FormatDouble(testValNegZeroFracComp).c_str());

    // TODO: Documentation refers to leading zeros for "digital expression" but never
    // defines what a digital expression means. So I don't actually know if this testing
    // is correct.
    nfs.SetMinWidth(15);
    nfs.SetFormatTraits(FormatTraits::LeadingZeroes);
    EXPECT_STREQ("000000000000000", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("00000100501.125", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ("-0000100501.125", nfs.FormatDouble(testValNeg).c_str());
    nfs.SetMinWidth(0);

    nfs.SetMinWidth(15);
    nfs.SetFormatTraits(FormatTraits::TrailingZeroes);
    EXPECT_STREQ("0000000000000000", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("100501.125000000", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ("-100501.12500000", nfs.FormatDouble(testValNeg).c_str());
    nfs.SetMinWidth(0);

    nfs.SetFormatTraits(FormatTraits::KeepDecimalPoint);
    EXPECT_STREQ("0.", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("100501.", nfs.FormatDouble(testValPosZeroFracComp).c_str());
    EXPECT_STREQ("-100501.", nfs.FormatDouble(testValNegZeroFracComp).c_str());

    nfs.SetFormatTraits(FormatTraits::KeepSingleZero);
    EXPECT_STREQ("0.0", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("100501.0", nfs.FormatDouble(testValPosZeroFracComp).c_str());
    EXPECT_STREQ("-100501.0", nfs.FormatDouble(testValNegZeroFracComp).c_str());

    nfs.SetPresentationType(PresentationType::Scientific);
    nfs.SetDecimalPrecision(DecimalPrecision::Max);
    nfs.SetFormatTraits(FormatTraits::ExponentZero);
    EXPECT_STREQ("0e+00", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("1.00501125e+05", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ("-1.00501125e+05", nfs.FormatDouble(testValNeg).c_str());
    nfs.SetPresentationType(PresentationType::Decimal);

    nfs.SetFormatTraits(FormatTraits::ZeroEmpty);
    EXPECT_STREQ("", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("100501.125", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ("-100501.125", nfs.FormatDouble(testValNeg).c_str());

    nfs.SetFormatTraits(FormatTraits::Use1000Separator);
    EXPECT_STREQ("0", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("100,501.125", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ("-100,501.125", nfs.FormatDouble(testValNeg).c_str());

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
    DecimalPrecision savePrecision = nfs.GetDecimalPrecision();
    nfs.SetDecimalPrecision(DecimalPrecision::Precision6);
    nfs.SetRoundingFactor(0.1);
    EXPECT_STREQ("0", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("100501.1", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ("-100501.1", nfs.FormatDouble(testValNeg).c_str());
    nfs.SetRoundingFactor(0.1);
    EXPECT_STREQ("0", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("0.5", nfs.FormatDouble(0.54).c_str());
    EXPECT_STREQ("-0.5", nfs.FormatDouble(-0.54).c_str());
    nfs.SetDecimalPrecision(savePrecision); // Restore the precision for the rest of the test.

    nfs.SetPresentationType(PresentationType::Fractional);
    nfs.SetFormatTraits(FormatTraits::FractionDash);
    EXPECT_STREQ("0", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("100501-1/8", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ("-100501-1/8", nfs.FormatDouble(testValNeg).c_str());
    nfs.SetPresentationType(PresentationType::Decimal);

    nfs.SetFormatTraits(FormatTraits::UseFractSymbol);
    EXPECT_STREQ("0", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ(u8"100501 \x21\x5B", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ(u8"-100501 \x21\x5B", nfs.FormatDouble(testValNeg).c_str());

    // AppendUnitName should have no effect on the unitless NamedFormatSpec.
    nfs.SetFormatTraits(FormatTraits::AppendUnitName);
    EXPECT_STREQ("0", nfs.FormatDouble(0).c_str());
    EXPECT_STREQ("100501.125", nfs.FormatDouble(testValPos).c_str());
    EXPECT_STREQ("-100501.125", nfs.FormatDouble(testValNeg).c_str());
    }

TEST_F(NumericFormatSpecTest, StdFormatDouble)
    {
    FAIL() << "TODO: Untested methods are failing methods.";
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

    BEU::UnitCP mile = BEU::UnitRegistry::Get().LookupUnit("MILE");
    BEU::UnitCP inch = BEU::UnitRegistry::Get().LookupUnit("IN");
    CompositeValueSpec compositeValueSpec(mile, inch);
    ASSERT_EQ(2, compositeValueSpec.GetUnitCount());
    ASSERT_EQ(CompositeSpecType::Double, compositeValueSpec.GetType());
    NamedFormatSpec namedFormatSpec("TestNamedFormatSpec", numericFormatSpec, compositeValueSpec);

    // 1500.5 miles == 1,500 miles and 31,680 inches
    BEU::Quantity quantity(1500.5, *compositeValueSpec.GetMajorUnit());
    EXPECT_STREQ("1'500.0 31'680.0", NumericFormatSpec::StdFormatQuantity(namedFormatSpec, quantity).c_str());
    }

TEST_F(NumericFormatSpecTest, FormatQuantity)
    {
    FAIL() << "TODO: Untested methods are failing methods.";
    }

TEST_F(NumericFormatSpecTest, ToJson)
    {
    FAIL() << "TODO: Untested methods are failing methods.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatUnitSetTest, ConstructFusFromDescription)
    {
    {
    FormatUnitSet MMFusNoFormatName("MM", &BEU::UnitRegistry::Get());
    EXPECT_FALSE(MMFusNoFormatName.HasProblem()) << FormatUnitSetTest::GetTestFusProblemDescription(MMFusNoFormatName);
    EXPECT_STREQ("MM(DefaultReal)", MMFusNoFormatName.ToText().c_str());
    EXPECT_STREQ("MM(real)", MMFusNoFormatName.ToText().c_str());
    }
    {
    FormatUnitSet MMFusParens("MM(Real2)", &BEU::UnitRegistry::Get());
    EXPECT_FALSE(MMFusParens.HasProblem()) << FormatUnitSetTest::GetTestFusProblemDescription(MMFusParens);
    EXPECT_STREQ("MM(Real2)", MMFusParens.ToText().c_str());
    EXPECT_STREQ("MM(real2)", MMFusParens.ToText().c_str());
    }
    {
    FormatUnitSet MMFusBarFormatName("MM|Real2", &BEU::UnitRegistry::Get());
    EXPECT_FALSE(MMFusBarFormatName.HasProblem()) << FormatUnitSetTest::GetTestFusProblemDescription(MMFusBarFormatName);
    EXPECT_STREQ("MM(Real2)", MMFusBarFormatName.ToText().c_str());
    EXPECT_STREQ("MM(real2)", MMFusBarFormatName.ToText().c_str());
    }
    {
    FormatUnitSet MMFusBarFormatNameBar("MM|Real2|", &BEU::UnitRegistry::Get());
    EXPECT_FALSE(MMFusBarFormatNameBar.HasProblem()) << FormatUnitSetTest::GetTestFusProblemDescription(MMFusBarFormatNameBar);
    EXPECT_STREQ("MM(Real2)", MMFusBarFormatNameBar.ToText().c_str());
    EXPECT_STREQ("MM(real2)", MMFusBarFormatNameBar.ToText().c_str());
    }
    {
    FormatUnitSet MMFusJsonOnlyUnitName(R"json({
        unitName: "MM"
    })json", &BEU::UnitRegistry::Get());
    EXPECT_FALSE(MMFusJsonOnlyUnitName.HasProblem()) << FormatUnitSetTest::GetTestFusProblemDescription(MMFusJsonOnlyUnitName);
    EXPECT_STREQ("MM(Real2)", MMFusJsonOnlyUnitName.ToText().c_str());
    EXPECT_STREQ("MM(real2)", MMFusJsonOnlyUnitName.ToText().c_str());
    EXPECT_TRUE(MMFusJsonOnlyUnitName.IsIdentical(FormatUnitSet("MM(Real2)", &BEU::UnitRegistry::Get())));
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
    })json", &BEU::UnitRegistry::Get());
    EXPECT_FALSE(MMFusJsonUnitNameFormatName.HasProblem()) << FormatUnitSetTest::GetTestFusProblemDescription(MMFusJsonUnitNameFormatName);
    EXPECT_STREQ("MM(Real2)", MMFusJsonUnitNameFormatName.ToText().c_str());
    EXPECT_STREQ("MM(real2)", MMFusJsonUnitNameFormatName.ToText().c_str());
    EXPECT_TRUE(MMFusJsonUnitNameFormatName.IsIdentical(FormatUnitSet("MM(Real2)", &BEU::UnitRegistry::Get())));
    }
    {
    FormatUnitSet MMFusJsonAllMembers(R"json({
        "unitName": "MM",
        "formatName": "real2",
        "cloneData": false,
        "formatSpec": { }
    })json", &BEU::UnitRegistry::Get());
    EXPECT_FALSE(MMFusJsonAllMembers.HasProblem()) << FormatUnitSetTest::GetTestFusProblemDescription(MMFusJsonAllMembers);
    EXPECT_STREQ("MM(Real2)", MMFusJsonAllMembers.ToText().c_str());
    EXPECT_STREQ("MM(real2)", MMFusJsonAllMembers.ToText().c_str());
    EXPECT_TRUE(MMFusJsonAllMembers.IsIdentical(FormatUnitSet("MM(Real2)", &BEU::UnitRegistry::Get())));
    }
    {
    FAIL() << "TODO: Test FUS from JSON with formatSpec fully defined.";
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
    EXPECT_EQ(FormatSpecType::Undefined, namedFmtSpec.GetSpecType());
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
    EXPECT_EQ(FormatSpecType::Numeric, namedFmtSpec.GetSpecType());
    EXPECT_FALSE(namedFmtSpec.IsProblem());
    ASSERT_NE(nullptr, namedFmtSpec.GetNumericSpec());
    EXPECT_TRUE(namedFmtSpec.GetNumericSpec()->IsIdentical(numFmtSpec));
    EXPECT_EQ(nullptr, namedFmtSpec.GetCompositeSpec());
    }

    // Constructed with name, NumericFormatSpec, and CompositeValueSpec
    {
    NumericFormatSpec numFmtSpec(DecimalPrecision::Precision9);
    CompositeValueSpec compValSpec;
    NamedFormatSpec namedFmtSpec("FooBar", numFmtSpec, compValSpec);

    EXPECT_STREQ("FooBar", namedFmtSpec.GetName());
    EXPECT_STREQ("", namedFmtSpec.GetDescription());
    EXPECT_STREQ("", namedFmtSpec.GetDisplayLabel());
    EXPECT_EQ(FormatSpecType::Composite, namedFmtSpec.GetSpecType());
    EXPECT_FALSE(namedFmtSpec.IsProblem());
    ASSERT_NE(nullptr, namedFmtSpec.GetNumericSpec());
    EXPECT_TRUE(namedFmtSpec.GetNumericSpec()->IsIdentical(numFmtSpec));
    ASSERT_NE(nullptr, namedFmtSpec.GetCompositeSpec());
    EXPECT_TRUE(namedFmtSpec.GetCompositeSpec()->IsIdentical(compValSpec));
    }

    FAIL() << "TODO: Test const ref copy constructor and JSON constructors.";
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
    CompositeValueSpec compValSpec;

    NamedFormatSpec namedFmtSpecUndefined;
    NamedFormatSpec namedFmtSpecNumeric("FooBar", numFmtSpec);
    NamedFormatSpec namedFmtSpecComposite("FooBar", numFmtSpec, compValSpec);

    // Identity checks.
    EXPECT_TRUE(namedFmtSpecUndefined.IsIdentical(namedFmtSpecUndefined))  << "NamedFormatSpec is not identical to itself.";
    EXPECT_TRUE(namedFmtSpecNumeric.IsIdentical(namedFmtSpecNumeric)) << "NamedFormatSpec is not identical to itself.";
    EXPECT_TRUE(namedFmtSpecComposite.IsIdentical(namedFmtSpecComposite)) << "NamedFormatSpec is not identical to itself.";;

    EXPECT_FALSE(namedFmtSpecUndefined.IsIdentical(namedFmtSpecNumeric));
    EXPECT_FALSE(namedFmtSpecUndefined.IsIdentical(namedFmtSpecComposite));

    EXPECT_FALSE(namedFmtSpecNumeric.IsIdentical(namedFmtSpecUndefined));
    EXPECT_FALSE(namedFmtSpecNumeric.IsIdentical(namedFmtSpecComposite));

    EXPECT_FALSE(namedFmtSpecComposite.IsIdentical(namedFmtSpecUndefined));
    EXPECT_FALSE(namedFmtSpecComposite.IsIdentical(namedFmtSpecNumeric));
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
    BEU::UnitCP mile = BEU::UnitRegistry::Get().LookupUnit("MILE");
    CompositeValueSpec compValSpecA(mile);
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
//TEST_F(NamedFormatSpecTest, )
//    {
//

END_BENTLEY_FORMATTEST_NAMESPACE
