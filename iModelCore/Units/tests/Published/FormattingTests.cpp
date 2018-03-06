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
static UnitProxySetCP upx = nullptr;
static int repc = 0;
BE_JSON_NAME(degrees)
BE_JSON_NAME(low)
BE_JSON_NAME(high)
BE_JSON_NAME(yaw)
BE_JSON_NAME(pitch)
BE_JSON_NAME(roll)

struct FormattingTest : public ::testing::Test
    {
    template <typename T, size_t N>
    static constexpr size_t Countof(T const (&arr)[N]) { return std::extent<T[N]>::value; }
    };

struct NumericFormatSpecTest : public FormattingTest
    {
    };

struct FormatBinaryTest : public NumericFormatSpecTest
    {
    struct TestByte
        {
        static Byte const allBitsZero = 0x0;
        static Byte const allBitsOne = 0xFF;
        static Byte const charA = 'A';
        };

    struct TestShort
        {
        static_assert(sizeof(short) == 2*sizeof(char), "C datatype has unexpected size for modern platform.");
        static size_t const shortBits = sizeof(short)*CHAR_BIT;
        static short const allBitsZero = 0;
        static short const allBitsOne = -1; // This is technically UB since processors aren't required
                                            // to use two's compliment. Our strict warnings don't let
                                            // us assign 0xFFFF to this value on MSVC even with a cast
                                            // so this is a hacky workaround. :(
        static short const testVal = 0b01011010'10010110;
        };
    };

struct FormatIntegerTest : public NumericFormatSpecTest
    {
    };

struct FormatDoubleTest : public NumericFormatSpecTest
    {
    };

struct FormatUnitSetTest : public FormattingTest
    {
    // Method for printing GTest FUS problem description error.
    Utf8String GetTestFusProblemDescription(FormatUnitSetCR fus)
        {
        return Utf8String("Problem: ") + fus.GetProblemDescription();
        }
    };

struct CompositeValueSpecTest: public FormattingTest
    {
    };

struct CompositeValueTest: public FormattingTest
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
    NumericFormatSpec nfsMin(FormatConstant::MinDecimalPrecisionIndex());
    EXPECT_DOUBLE_EQ(FormatConstant::DefaultRoundingFactor(), nfsMin.GetRoundingFactor());
    EXPECT_EQ(FormatConstant::DefaultPresentaitonType(), nfsMin.GetPresentationType());
    EXPECT_EQ(FormatConstant::DefaultSignOption(), nfsMin.GetSignOption());
    EXPECT_EQ(FormatConstant::DefaultFormatTraits(), nfsMin.GetFormatTraits());
    EXPECT_EQ(Utils::DecimalPrecisionByIndex(FormatConstant::MinDecimalPrecisionIndex()), nfsMin.GetDecimalPrecision());
    EXPECT_EQ(FormatConstant::DefaultFractionalPrecision(), nfsMin.GetFractionalPrecision());
    EXPECT_EQ(FormatConstant::DefaultFractionBarType(), nfsMin.GetFractionalBarType());
    EXPECT_EQ(FormatConstant::DefaultDecimalSeparator(), nfsMin.GetDecimalSeparator());
    EXPECT_EQ(FormatConstant::DefaultThousandSeparator(), nfsMin.GetThousandSeparator());
    EXPECT_EQ(FormatConstant::DefaultUomSeparator(), nfsMin.GetUomSeparator());
    EXPECT_EQ(FormatConstant::DefaultStopSeparator(), nfsMin.GetStopSeparator());
    EXPECT_EQ(FormatConstant::DefaultMinWidth(), nfsMin.GetMinWidth());
    EXPECT_EQ(FormatConstant::DefaultMinWidth(), nfsMin.GetMinWidth());

    NumericFormatSpec nfsMax(FormatConstant::MaxDecimalPrecisionIndex());
    EXPECT_DOUBLE_EQ(FormatConstant::DefaultRoundingFactor(), nfsMax.GetRoundingFactor());
    EXPECT_EQ(FormatConstant::DefaultPresentaitonType(), nfsMax.GetPresentationType());
    EXPECT_EQ(FormatConstant::DefaultSignOption(), nfsMax.GetSignOption());
    EXPECT_EQ(FormatConstant::DefaultFormatTraits(), nfsMax.GetFormatTraits());
    EXPECT_EQ(Utils::DecimalPrecisionByIndex(FormatConstant::MaxDecimalPrecisionIndex()), nfsMax.GetDecimalPrecision());
    EXPECT_EQ(FormatConstant::DefaultFractionalPrecision(), nfsMax.GetFractionalPrecision());
    EXPECT_EQ(FormatConstant::DefaultFractionBarType(), nfsMax.GetFractionalBarType());
    EXPECT_EQ(FormatConstant::DefaultDecimalSeparator(), nfsMax.GetDecimalSeparator());
    EXPECT_EQ(FormatConstant::DefaultThousandSeparator(), nfsMax.GetThousandSeparator());
    EXPECT_EQ(FormatConstant::DefaultUomSeparator(), nfsMax.GetUomSeparator());
    EXPECT_EQ(FormatConstant::DefaultStopSeparator(), nfsMax.GetStopSeparator());
    EXPECT_EQ(FormatConstant::DefaultMinWidth(), nfsMax.GetMinWidth());
    EXPECT_EQ(FormatConstant::DefaultMinWidth(), nfsMax.GetMinWidth());

    NumericFormatSpec nfsOutOfBounds(FormatConstant::MaxDecimalPrecisionIndex() + 1);
    EXPECT_DOUBLE_EQ(FormatConstant::DefaultRoundingFactor(), nfsOutOfBounds.GetRoundingFactor());
    EXPECT_EQ(FormatConstant::DefaultPresentaitonType(), nfsOutOfBounds.GetPresentationType());
    EXPECT_EQ(FormatConstant::DefaultSignOption(), nfsOutOfBounds.GetSignOption());
    EXPECT_EQ(FormatConstant::DefaultFormatTraits(), nfsOutOfBounds.GetFormatTraits());
    EXPECT_EQ(FormatConstant::DefaultDecimalPrecision(), nfsOutOfBounds.GetDecimalPrecision());
    EXPECT_EQ(FormatConstant::DefaultFractionalPrecision(), nfsOutOfBounds.GetFractionalPrecision());
    EXPECT_EQ(FormatConstant::DefaultFractionBarType(), nfsOutOfBounds.GetFractionalBarType());
    EXPECT_EQ(FormatConstant::DefaultDecimalSeparator(), nfsOutOfBounds.GetDecimalSeparator());
    EXPECT_EQ(FormatConstant::DefaultThousandSeparator(), nfsOutOfBounds.GetThousandSeparator());
    EXPECT_EQ(FormatConstant::DefaultUomSeparator(), nfsOutOfBounds.GetUomSeparator());
    EXPECT_EQ(FormatConstant::DefaultStopSeparator(), nfsOutOfBounds.GetStopSeparator());
    EXPECT_EQ(FormatConstant::DefaultMinWidth(), nfsOutOfBounds.GetMinWidth());
    EXPECT_EQ(FormatConstant::DefaultMinWidth(), nfsOutOfBounds.GetMinWidth());
    }

    // TODO: Test the rest of the constructors and the Init method
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatBinaryTest, FormatBinaryByteFailsWhenBufferIsTooSmall)
    {
    NumericFormatSpec nfs;

    Utf8Char buffLen1[1];
    EXPECT_EQ((CHAR_BIT+sizeof('\0'))-FormattingTest::Countof(buffLen1), nfs.FormatBinaryByte(FormatBinaryTest::TestByte::charA, buffLen1, (int)FormattingTest::Countof(buffLen1)));

    Utf8Char buffLenCharBits[CHAR_BIT];
    EXPECT_EQ((CHAR_BIT+sizeof('\0'))-FormattingTest::Countof(buffLenCharBits), nfs.FormatBinaryByte(FormatBinaryTest::TestByte::charA, buffLenCharBits, (int)FormattingTest::Countof(buffLenCharBits)));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatBinaryTest, FormatBinaryByteFormatsCorrectly)
    {
    NumericFormatSpec nfs;
    Utf8Char buff[9];

    ASSERT_EQ(0, nfs.FormatBinaryByte(FormatBinaryTest::TestByte::allBitsZero, buff, (int)FormattingTest::Countof(buff)));
    EXPECT_STREQ("00000000", buff);

    ASSERT_EQ(0, nfs.FormatBinaryByte(FormatBinaryTest::TestByte::allBitsOne, buff, (int)FormattingTest::Countof(buff)));
    EXPECT_STREQ("11111111", buff);

    ASSERT_EQ(0, nfs.FormatBinaryByte(FormatBinaryTest::TestByte::charA, buff, (int)FormattingTest::Countof(buff)));
    EXPECT_STREQ("01000001", buff);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatBinaryTest, FormatBinaryShortFailsWhenBufferIsTooSmall)
    {
    NumericFormatSpec nfs;

    Utf8Char buffLen1[1];
    EXPECT_EQ((FormatBinaryTest::TestShort::shortBits+sizeof('\0'))-FormattingTest::Countof(buffLen1),
        nfs.FormatBinaryShort(FormatBinaryTest::TestShort::testVal, buffLen1, (int)FormattingTest::Countof(buffLen1), false));
    EXPECT_EQ((FormatBinaryTest::TestShort::shortBits+sizeof('\0'))-FormattingTest::Countof(buffLen1),
        nfs.FormatBinaryShort(FormatBinaryTest::TestShort::testVal, buffLen1, (int)FormattingTest::Countof(buffLen1), true));

    Utf8Char buffLenShortBits[FormatBinaryTest::TestShort::shortBits];
    EXPECT_EQ((FormatBinaryTest::TestShort::shortBits+sizeof('\0'))-FormattingTest::Countof(buffLenShortBits),
        nfs.FormatBinaryShort(FormatBinaryTest::TestShort::testVal, buffLenShortBits, (int)FormattingTest::Countof(buffLenShortBits), false));

    bvector<Utf8Char> buffLenShortBitsPlusThousandsSeparator(FormatBinaryTest::TestShort::shortBits + nfs.GetThousandSeparatorSize());
    EXPECT_EQ((FormatBinaryTest::TestShort::shortBits+sizeof('\0'))-buffLenShortBitsPlusThousandsSeparator.size(),
        nfs.FormatBinaryShort(FormatBinaryTest::TestShort::testVal, buffLenShortBitsPlusThousandsSeparator.data(), (int)buffLenShortBitsPlusThousandsSeparator.size(), true));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatBinaryTest, FormatBinaryShortFormatsCorrectly)
    {
    NumericFormatSpec nfsComma;
    nfsComma.SetThousandSeparator(',');
    nfsComma.SetUse1000Separator(true);
    NumericFormatSpec nfsPlus;
    nfsPlus.SetThousandSeparator('+');
    nfsPlus.SetUse1000Separator(true);
    Utf8Char buff[256]; // Size chosen to be more than large enough to hold the formatted string.

    ASSERT_EQ(0, nfsComma.FormatBinaryShort(FormatBinaryTest::TestShort::allBitsZero, buff, (int)FormattingTest::Countof(buff), false));
    EXPECT_STREQ("0000000000000000", buff);
    ASSERT_EQ(0, nfsComma.FormatBinaryShort(FormatBinaryTest::TestShort::allBitsZero, buff, (int)FormattingTest::Countof(buff), true));
    EXPECT_STREQ("00000000,00000000", buff);
    ASSERT_EQ(0, nfsPlus.FormatBinaryShort(FormatBinaryTest::TestShort::allBitsZero, buff, (int)FormattingTest::Countof(buff), true));
    EXPECT_STREQ("00000000+00000000", buff);

    ASSERT_EQ(0, nfsComma.FormatBinaryShort(FormatBinaryTest::TestShort::allBitsOne, buff, (int)FormattingTest::Countof(buff), false));
    EXPECT_STREQ("1111111111111111", buff);
    ASSERT_EQ(0, nfsComma.FormatBinaryShort(FormatBinaryTest::TestShort::allBitsOne, buff, (int)FormattingTest::Countof(buff), true));
    EXPECT_STREQ("11111111,11111111", buff);
    ASSERT_EQ(0, nfsPlus.FormatBinaryShort(FormatBinaryTest::TestShort::allBitsOne, buff, (int)FormattingTest::Countof(buff), true));
    EXPECT_STREQ("11111111+11111111", buff);

    ASSERT_EQ(0, nfsComma.FormatBinaryShort(FormatBinaryTest::TestShort::testVal, buff, (int)FormattingTest::Countof(buff), false));
    EXPECT_STREQ("0101101010010110", buff);
    ASSERT_EQ(0, nfsComma.FormatBinaryShort(FormatBinaryTest::TestShort::testVal, buff, (int)FormattingTest::Countof(buff), true));
    EXPECT_STREQ("01011010,10010110", buff);
    ASSERT_EQ(0, nfsPlus.FormatBinaryShort(FormatBinaryTest::TestShort::testVal, buff, (int)FormattingTest::Countof(buff), true));
    EXPECT_STREQ("01011010+10010110", buff);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatBinaryTest, FormatBinaryInt)
    {
    FAIL() << "TODO: Untested methods are failing methods.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatBinaryTest, FormatBinaryDouble)
    {
    FAIL() << "TODO: Untested methods are failing methods.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatIntegerTest, FormatIntegerFailsWhenBufferIsTooSmall)
    {
    NumericFormatSpec nfs;

    Utf8Char buffLen1[1];
    EXPECT_EQ(0, nfs.FormatInteger(42, buffLen1, (int)FormattingTest::Countof(buffLen1)));
    EXPECT_EQ(0, NumericFormatSpec::FormatIntegerSimple(42, buffLen1, (int)FormattingTest::Countof(buffLen1), false, false));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatIntegerTest, FormatIntegerSignOptionTests)
    {
    NumericFormatSpec nfs;
    Utf8Char buff[256];
    Utf8String string;

    // NoSign
    {
    nfs.SetSignOption(ShowSignOption::NoSign);

    EXPECT_EQ((int)strlen("42"), nfs.FormatInteger(42, buff, (int)FormattingTest::Countof(buff)));
    EXPECT_STREQ("42", buff);
    EXPECT_EQ(strlen("42"), nfs.FormatInteger(-42, buff, (int)FormattingTest::Countof(buff)));
    EXPECT_STREQ("42", buff);

    EXPECT_STREQ("42", nfs.FormatInteger(42).c_str());
    EXPECT_STREQ("42", nfs.FormatInteger(-42).c_str());

    EXPECT_STREQ("42", nfs.FormatIntegerToString(42, 0).c_str());
    EXPECT_STREQ("00042", nfs.FormatIntegerToString(42, (int)strlen("42")+3).c_str());
    EXPECT_STREQ("42", nfs.FormatIntegerToString(-42, 0).c_str());
    EXPECT_STREQ("00042", nfs.FormatIntegerToString(-42, (int)strlen("42")+3).c_str());
    }

    // OnlyNegative
    {
    nfs.SetSignOption(ShowSignOption::OnlyNegative);

    EXPECT_EQ((int)strlen("42"), nfs.FormatInteger(42, buff, (int)FormattingTest::Countof(buff)));
    EXPECT_STREQ("42", buff);
    EXPECT_EQ((int)strlen("-42"), nfs.FormatInteger(-42, buff, (int)FormattingTest::Countof(buff)));
    EXPECT_STREQ("-42", buff);

    EXPECT_STREQ("42", nfs.FormatInteger(42).c_str());
    EXPECT_STREQ("-42", nfs.FormatInteger(-42).c_str());

    EXPECT_STREQ("42", nfs.FormatIntegerToString(42, 0).c_str());
    EXPECT_STREQ("00042", nfs.FormatIntegerToString(42, (int)strlen("42")+3).c_str());
    EXPECT_STREQ("-42", nfs.FormatIntegerToString(-42, 0).c_str());
    EXPECT_STREQ("-00042", nfs.FormatIntegerToString(-42, (int)strlen("42")+3).c_str());
    }

    // SignAlways
    {
    nfs.SetSignOption(ShowSignOption::SignAlways);

    EXPECT_EQ((int)strlen("+42"), nfs.FormatInteger(42, buff, (int)FormattingTest::Countof(buff)));
    EXPECT_STREQ("+42", buff);
    EXPECT_EQ((int)strlen("-42"), nfs.FormatInteger(-42, buff, (int)FormattingTest::Countof(buff)));
    EXPECT_STREQ("-42", buff);

    EXPECT_STREQ("+42", nfs.FormatInteger(42).c_str());
    EXPECT_STREQ("-42", nfs.FormatInteger(-42).c_str());

    EXPECT_STREQ("+42", nfs.FormatIntegerToString(42, 0).c_str());
    EXPECT_STREQ("+00042", nfs.FormatIntegerToString(42, (int)strlen("42")+3).c_str());
    EXPECT_STREQ("-42", nfs.FormatIntegerToString(-42, 0).c_str());
    EXPECT_STREQ("-00042", nfs.FormatIntegerToString(-42, (int)strlen("42")+3).c_str());
    }

    // NegativeParentheses
    {
    nfs.SetSignOption(ShowSignOption::NegativeParentheses);

    EXPECT_EQ((int)strlen("42"), nfs.FormatInteger(42, buff, (int)FormattingTest::Countof(buff)));
    EXPECT_STREQ("42", buff);
    EXPECT_EQ((int)strlen("(42)"), nfs.FormatInteger(-42, buff, (int)FormattingTest::Countof(buff)));
    EXPECT_STREQ("(42)", buff);

    EXPECT_STREQ("42", nfs.FormatInteger(42).c_str());
    EXPECT_STREQ("(42)", nfs.FormatInteger(-42).c_str());

    EXPECT_STREQ("42", nfs.FormatIntegerToString(42, 0).c_str());
    EXPECT_STREQ("00042", nfs.FormatIntegerToString(42, (int)strlen("42")+3).c_str());
    EXPECT_STREQ("(42)", nfs.FormatIntegerToString(-42, 0).c_str());
    EXPECT_STREQ("(00042)", nfs.FormatIntegerToString(-42, (int)strlen("42")+3).c_str());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatIntegerTest, FormatIntegerThousandsSeparatorTests)
    {
    NumericFormatSpec nfs;
    nfs.SetUse1000Separator(true);
    Utf8Char buff[256];
    Utf8String string;

    nfs.SetThousandSeparator(',');
    EXPECT_EQ((int)strlen("1,000"), nfs.FormatInteger(1000, buff, (int)FormattingTest::Countof(buff)));
    EXPECT_STREQ("1,000", buff);
    EXPECT_EQ((int)strlen("-1,000"), nfs.FormatInteger(-1000, buff, (int)FormattingTest::Countof(buff)));
    EXPECT_STREQ("-1,000", buff);

    EXPECT_STREQ("1,000", nfs.FormatInteger(1000).c_str());
    EXPECT_STREQ("-1,000", nfs.FormatInteger(-1000).c_str());

    EXPECT_STREQ("1,000", nfs.FormatIntegerToString(1000, 0).c_str());
    EXPECT_STREQ("001,000", nfs.FormatIntegerToString(1000, (int)strlen("1000")+2).c_str());
    EXPECT_STREQ("0,001,000", nfs.FormatIntegerToString(1000, (int)strlen("1000")+3).c_str());
    EXPECT_STREQ("-1,000", nfs.FormatIntegerToString(-1000, 0).c_str());
    EXPECT_STREQ("-001,000", nfs.FormatIntegerToString(-1000, (int)strlen("1000")+2).c_str());
    EXPECT_STREQ("-0,0001,000", nfs.FormatIntegerToString(-1000, (int)strlen("1000")+3).c_str());

    nfs.SetThousandSeparator('.');
    EXPECT_EQ((int)strlen("1.000"), nfs.FormatInteger(1000, buff, (int)FormattingTest::Countof(buff)));
    EXPECT_STREQ("1.000", buff);
    EXPECT_EQ((int)strlen("-1.000"), nfs.FormatInteger(-1000, buff, (int)FormattingTest::Countof(buff)));
    EXPECT_STREQ("-1.000", buff);

    EXPECT_STREQ("1.000", nfs.FormatInteger(1000).c_str());
    EXPECT_STREQ("-1.000", nfs.FormatInteger(-1000).c_str());

    EXPECT_STREQ("1.000", nfs.FormatIntegerToString(1000, 0).c_str());
    EXPECT_STREQ("001.000", nfs.FormatIntegerToString(1000, (int)strlen("1000")+2).c_str());
    EXPECT_STREQ("0.001.000", nfs.FormatIntegerToString(1000, (int)strlen("1000")+3).c_str());
    EXPECT_STREQ("-1.000", nfs.FormatIntegerToString(-1000, 0).c_str());
    EXPECT_STREQ("-001.000", nfs.FormatIntegerToString(-1000, (int)strlen("1000")+2).c_str());
    EXPECT_STREQ("-0.0001.000", nfs.FormatIntegerToString(-1000, (int)strlen("1000")+3).c_str());

    nfs.SetThousandSeparator('\t'); // I mean technically the API doesn't prevent it right?
    EXPECT_EQ((int)strlen("1\t000"), nfs.FormatInteger(1000, buff, (int)FormattingTest::Countof(buff)));
    EXPECT_STREQ("1\t000", buff);
    EXPECT_EQ((int)strlen("-1\t000"), nfs.FormatInteger(-1000, buff, (int)FormattingTest::Countof(buff)));
    EXPECT_STREQ("-1\t000", buff);

    EXPECT_STREQ("1\t000", nfs.FormatInteger(1000).c_str());
    EXPECT_STREQ("-1\t000", nfs.FormatInteger(-1000).c_str());

    EXPECT_STREQ("1\t000", nfs.FormatIntegerToString(1000, 0).c_str());
    EXPECT_STREQ("001\t000", nfs.FormatIntegerToString(1000, (int)strlen("1000")+2).c_str());
    EXPECT_STREQ("0\t001\t000", nfs.FormatIntegerToString(1000, (int)strlen("1000")+3).c_str());
    EXPECT_STREQ("-1\t000", nfs.FormatIntegerToString(-1000, 0).c_str());
    EXPECT_STREQ("-001\t000", nfs.FormatIntegerToString(-1000, (int)strlen("1000")+2).c_str());
    EXPECT_STREQ("-0\t0001\t000", nfs.FormatIntegerToString(-1000, (int)strlen("1000")+3).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatIntegerTest, FormatIntegerPresentationTypeTests)
    {
    NumericFormatSpec nfs;
    Utf8Char buff[256];
    Utf8String string;
    int const testValPos = 100501;
    int const testValNeg = -100501;

    // Decimal
    {
    nfs.SetPresentationType(PresentationType::Decimal);

    EXPECT_EQ((int)strlen("100501"), nfs.FormatInteger(testValPos, buff, (int)FormattingTest::Countof(buff)));
    EXPECT_STREQ("100501", buff);
    EXPECT_STREQ("100501", nfs.FormatInteger(testValPos).c_str());

    EXPECT_EQ((int)strlen("-100501"), nfs.FormatInteger(testValNeg, buff, (int)FormattingTest::Countof(buff)));
    EXPECT_STREQ("-100501", buff);
    EXPECT_STREQ("-100501", nfs.FormatInteger(testValNeg).c_str());
    }

    // Fractional
    {
    nfs.SetPresentationType(PresentationType::Fractional);

    EXPECT_EQ((int)strlen("100501"), nfs.FormatInteger(testValPos, buff, (int)FormattingTest::Countof(buff)));
    EXPECT_STREQ("100501", buff);
    EXPECT_STREQ("100501", nfs.FormatInteger(testValPos).c_str());

    EXPECT_EQ((int)strlen("-100501"), nfs.FormatInteger(testValNeg, buff, (int)FormattingTest::Countof(buff)));
    EXPECT_STREQ("-100501", buff);
    EXPECT_STREQ("-100501", nfs.FormatInteger(testValNeg).c_str());
    }

    // TODO: The below Scientific and Scientific Norm tests fail since the testVal integer does not
    // ever get formatted as a value with a decimal point. Since it is not obvious to the user
    // how the scientific notation affects formatting of integers.
    // Scientific
    {
    nfs.SetPresentationType(PresentationType::Scientific);
    nfs.SetDecimalPrecision(DecimalPrecision::Max);

    EXPECT_EQ((int)strlen("1.00501e+5"), nfs.FormatInteger(testValPos, buff, (int)FormattingTest::Countof(buff)));
    EXPECT_STREQ("1.00501e+5", buff);
    EXPECT_STREQ("1.00501e+5", nfs.FormatInteger(testValPos).c_str());

    EXPECT_EQ((int)strlen("-1.00501e+5"), nfs.FormatInteger(testValNeg, buff, (int)FormattingTest::Countof(buff)));
    EXPECT_STREQ("-1.00501e+5", buff);
    EXPECT_STREQ("-11.00501e+5", nfs.FormatInteger(testValNeg).c_str());
    }

    // ScientificNorm
    {
    nfs.SetPresentationType(PresentationType::ScientificNorm);
    nfs.SetDecimalPrecision(DecimalPrecision::Max);

    EXPECT_EQ((int)strlen("0.100501e+6"), nfs.FormatInteger(testValPos, buff, (int)FormattingTest::Countof(buff)));
    EXPECT_STREQ("0.100501e+6", buff);
    EXPECT_STREQ("0.100501e+6", nfs.FormatInteger(testValPos).c_str());

    EXPECT_EQ((int)strlen("-0.100501e+6"), nfs.FormatInteger(testValNeg, buff, (int)FormattingTest::Countof(buff)));
    EXPECT_STREQ("-0.100501e+6", buff);
    EXPECT_STREQ("-0.100501e+6", nfs.FormatInteger(testValNeg).c_str());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatDoubleTest, FormatDoubleFailsWhenBufferIsTooSmall)
    {
    NumericFormatSpec nfs;

    Utf8Char buffLen1[1];
    EXPECT_EQ(FormattingTest::Countof(buffLen1), nfs.FormatDoubleBuf(99.25, buffLen1, FormattingTest::Countof(buffLen1)));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatDoubleTest, FormatDoubleSignOptionTests)
    {
    NumericFormatSpec nfs;
    Utf8Char buff[256];
    Utf8String string;

    // NoSign
    {
    nfs.SetSignOption(ShowSignOption::NoSign);

    EXPECT_EQ(strlen("101.25"), nfs.FormatDoubleBuf(101.25, buff, FormattingTest::Countof(buff)));
    EXPECT_STREQ("101.25", buff);
    EXPECT_EQ(strlen("101.25"), nfs.FormatDoubleBuf(-101.25, buff, FormattingTest::Countof(buff)));
    EXPECT_STREQ("101.25", buff);

    EXPECT_STREQ("101.25", nfs.FormatDouble(101.25).c_str());
    EXPECT_STREQ("101.25", nfs.FormatDouble(-101.25).c_str());

    EXPECT_STREQ("101.0", nfs.FormatRoundedDouble(101.25, 101).c_str());
    EXPECT_STREQ("102.0", nfs.FormatRoundedDouble(101.25, 102).c_str());
    EXPECT_STREQ("101.0", nfs.FormatRoundedDouble(-101.25, -101).c_str());
    EXPECT_STREQ("102.0", nfs.FormatRoundedDouble(-101.25, -102).c_str());
    }

    // OnlyNegative
    {
    nfs.SetSignOption(ShowSignOption::OnlyNegative);

    EXPECT_EQ(strlen("101.25"), nfs.FormatDoubleBuf(101.25, buff, FormattingTest::Countof(buff)));
    EXPECT_STREQ("101.25", buff);
    EXPECT_EQ(strlen("-101.25"), nfs.FormatDoubleBuf(-101.25, buff, FormattingTest::Countof(buff)));
    EXPECT_STREQ("-101.25", buff);

    EXPECT_STREQ("101.25", nfs.FormatDouble(101.25).c_str());
    EXPECT_STREQ("-101.25", nfs.FormatDouble(-101.25).c_str());

    EXPECT_STREQ("101.0", nfs.FormatRoundedDouble(101.25, 101).c_str());
    EXPECT_STREQ("102.0", nfs.FormatRoundedDouble(101.25, 102).c_str());
    EXPECT_STREQ("-101.0", nfs.FormatRoundedDouble(-101.25, -101).c_str());
    EXPECT_STREQ("-102.0", nfs.FormatRoundedDouble(-101.25, -102).c_str());
    }

    // SignAlways
    {
    nfs.SetSignOption(ShowSignOption::SignAlways);

    EXPECT_EQ(strlen("+101.25"), nfs.FormatDoubleBuf(101.25, buff, FormattingTest::Countof(buff)));
    EXPECT_STREQ("+101.25", buff);
    EXPECT_EQ(strlen("-101.25"), nfs.FormatDoubleBuf(-101.25, buff, FormattingTest::Countof(buff)));
    EXPECT_STREQ("-101.25", buff);

    EXPECT_STREQ("+101.25", nfs.FormatDouble(101.25).c_str());
    EXPECT_STREQ("-101.25", nfs.FormatDouble(-101.25).c_str());

    EXPECT_STREQ("+101.0", nfs.FormatRoundedDouble(101.25, 101).c_str());
    EXPECT_STREQ("+102.0", nfs.FormatRoundedDouble(101.25, 102).c_str());
    EXPECT_STREQ("-101.0", nfs.FormatRoundedDouble(-101.25, -101).c_str());
    EXPECT_STREQ("-102.0", nfs.FormatRoundedDouble(-101.25, -102).c_str());
    }

    // NegativeParentheses
    {
    nfs.SetSignOption(ShowSignOption::NegativeParentheses);

    EXPECT_EQ(strlen("101.25"), nfs.FormatDoubleBuf(101.25, buff, FormattingTest::Countof(buff)));
    EXPECT_STREQ("101.25", buff);
    EXPECT_EQ(strlen("(101.25)"), nfs.FormatDoubleBuf(-101.25, buff, FormattingTest::Countof(buff)));
    EXPECT_STREQ("(101.25)", buff);

    EXPECT_STREQ("101.25", nfs.FormatDouble(101.25).c_str());
    EXPECT_STREQ("(101.25)", nfs.FormatDouble(-101.25).c_str());

    EXPECT_STREQ("101.0", nfs.FormatRoundedDouble(101.25, 101).c_str());
    EXPECT_STREQ("102.0", nfs.FormatRoundedDouble(101.25, 102).c_str());
    EXPECT_STREQ("(101.0)", nfs.FormatRoundedDouble(-101.25, -101).c_str());
    EXPECT_STREQ("(102.0)", nfs.FormatRoundedDouble(-101.25, -102).c_str());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatDoubleTest, FormatDoubleThousandsSeparatorTests)
    {
    NumericFormatSpec nfs;
    nfs.SetUse1000Separator(true);
    Utf8Char buff[256];
    Utf8String string;

    nfs.SetThousandSeparator(',');
    EXPECT_EQ(strlen("1,000.55"), nfs.FormatDoubleBuf(1000.55, buff, FormattingTest::Countof(buff)));
    EXPECT_STREQ("1,000.55", buff);
    EXPECT_EQ(strlen("-1,000.55"), nfs.FormatDoubleBuf(-1000.55, buff, FormattingTest::Countof(buff)));
    EXPECT_STREQ("-1,000.55", buff);

    EXPECT_STREQ("1,000.55", nfs.FormatDouble(1000.55).c_str());
    EXPECT_STREQ("-1,000.55", nfs.FormatDouble(-1000.55).c_str());

    EXPECT_STREQ("1,000.0", nfs.FormatRoundedDouble(1000.55, 1000).c_str());
    EXPECT_STREQ("1,001.0", nfs.FormatRoundedDouble(1000.55, 1001).c_str());
    EXPECT_STREQ("-1,000.0", nfs.FormatRoundedDouble(-1000.55, -1000).c_str());
    EXPECT_STREQ("-1,001.0", nfs.FormatRoundedDouble(-1000.55, -1001).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatDoubleTest, FormatDoubleDecimalSeparatorTests)
    {
    NumericFormatSpec nfs;
    Utf8Char buff[256];
    Utf8String string;

    nfs.SetDecimalSeparator('-');
    EXPECT_EQ(strlen("1000-55"), nfs.FormatDoubleBuf(1000.55, buff, FormattingTest::Countof(buff)));
    EXPECT_STREQ("1000-55", buff);
    EXPECT_EQ(strlen("-1000-55"), nfs.FormatDoubleBuf(-1000.55, buff, FormattingTest::Countof(buff)));
    EXPECT_STREQ("-1000-55", buff);

    EXPECT_STREQ("1000-55", nfs.FormatDouble(1000.55).c_str());
    EXPECT_STREQ("-1000-55", nfs.FormatDouble(-1000.55).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatDoubleTest, FormatDoublePresentationTypeTests)
    {
    NumericFormatSpec nfs;
    Utf8Char buff[256];
    Utf8String string;
    double const testValPos = 100501.125;
    double const testValNeg = -100501.125;

    // Decimal
    {
    nfs.SetPresentationType(PresentationType::Decimal);
    nfs.SetDecimalPrecision(DecimalPrecision::Max);

    EXPECT_EQ(strlen("100501.125"), nfs.FormatDoubleBuf(testValPos, buff, FormattingTest::Countof(buff)));
    EXPECT_STREQ("100501.125", buff);
    EXPECT_STREQ("100501.125", nfs.FormatDouble(testValPos).c_str());

    EXPECT_EQ(strlen("-100501.125"), nfs.FormatDoubleBuf(testValNeg, buff, FormattingTest::Countof(buff)));
    EXPECT_STREQ("-100501.125", buff);
    EXPECT_STREQ("-100501.125", nfs.FormatDouble(testValNeg).c_str());
    }

    // Fractional
    {
    nfs.SetPresentationType(PresentationType::Fractional);
    nfs.SetDecimalPrecision(DecimalPrecision::Max);

    EXPECT_EQ(strlen("100501 1/8"), nfs.FormatDoubleBuf(testValPos, buff, FormattingTest::Countof(buff)));
    EXPECT_STREQ("100501 1/8", buff);
    EXPECT_STREQ("100501 1/8", nfs.FormatDouble(testValPos).c_str());

    EXPECT_EQ(strlen("-100501 1/8"), nfs.FormatDoubleBuf(testValNeg, buff, FormattingTest::Countof(buff)));
    EXPECT_STREQ("-100501 1/8", buff);
    EXPECT_STREQ("-100501 1/8", nfs.FormatDouble(testValNeg).c_str());
    }

    // Scientific
    {
    nfs.SetPresentationType(PresentationType::Scientific);
    nfs.SetDecimalPrecision(DecimalPrecision::Max);

    EXPECT_EQ(strlen("1.00501125e+5"), nfs.FormatDoubleBuf(testValPos, buff, FormattingTest::Countof(buff)));
    EXPECT_STREQ("1.00501125e+5", buff);
    EXPECT_STREQ("1.00501125e+5", nfs.FormatDouble(testValPos).c_str());

    EXPECT_EQ(strlen("-1.00501125e+5"), nfs.FormatDoubleBuf(testValNeg, buff, FormattingTest::Countof(buff)));
    EXPECT_STREQ("-1.00501125e+5", buff);
    EXPECT_STREQ("-1.00501125e+5", nfs.FormatDouble(testValNeg).c_str());
    }

    // ScientificNorm
    {
    nfs.SetPresentationType(PresentationType::ScientificNorm);
    nfs.SetDecimalPrecision(DecimalPrecision::Max);

    EXPECT_EQ(strlen("0.100501125e+6"), nfs.FormatDoubleBuf(testValPos, buff, FormattingTest::Countof(buff)));
    EXPECT_STREQ("0.100501125e+6", buff);
    EXPECT_STREQ("0.100501125e+6", nfs.FormatDouble(testValPos).c_str());

    EXPECT_EQ(strlen("-0.100501125e+6"), nfs.FormatDoubleBuf(testValNeg, buff, FormattingTest::Countof(buff)));
    EXPECT_STREQ("-0.100501125e+6", buff);
    EXPECT_STREQ("-0.100501125e+6", nfs.FormatDouble(testValNeg).c_str());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatDoubleTest, FormatDoubleWithDifferentMinWidths)
    {
    NumericFormatSpec nfs;
    Utf8Char buff[256];
    Utf8String string;

    nfs.SetMinWidth(0);
    EXPECT_EQ(strlen("1000.0"), nfs.FormatDoubleBuf(1000, buff, FormattingTest::Countof(buff)));
    EXPECT_STREQ("1000.0", buff);
    EXPECT_STREQ("1000.0", nfs.FormatDouble(1000).c_str());

    nfs.SetMinWidth(6);
    EXPECT_EQ(strlen("1000.0"), nfs.FormatDoubleBuf(1000, buff, FormattingTest::Countof(buff)));
    EXPECT_STREQ("1000.0", buff);
    EXPECT_STREQ("1000.0", nfs.FormatDouble(1000).c_str());
    
    nfs.SetMinWidth(8);
    EXPECT_EQ(strlen("001000.0"), nfs.FormatDoubleBuf(1000, buff, FormattingTest::Countof(buff)));
    EXPECT_STREQ("001000.0", buff);
    EXPECT_STREQ("001000.0", nfs.FormatDouble(1000).c_str());
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatUnitSetTest, ConstructFusFromDescription)
    {
    {
    FormatUnitSet MMFusNoFormatName("MM");
    EXPECT_FALSE(MMFusNoFormatName.HasProblem()) << FormatUnitSetTest::GetTestFusProblemDescription(MMFusNoFormatName);
    EXPECT_STREQ("MM(DefaultReal)", MMFusNoFormatName.ToText(false).c_str());
    EXPECT_STREQ("MM(real)", MMFusNoFormatName.ToText(true).c_str());
    }
    {
    FormatUnitSet MMFusParens("MM(Real2)");
    EXPECT_FALSE(MMFusParens.HasProblem()) << FormatUnitSetTest::GetTestFusProblemDescription(MMFusParens);
    EXPECT_STREQ("MM(Real2)", MMFusParens.ToText(false).c_str());
    EXPECT_STREQ("MM(real2)", MMFusParens.ToText(true).c_str());
    }
    {
    FormatUnitSet MMFusBarFormatName("MM|Real2");
    EXPECT_FALSE(MMFusBarFormatName.HasProblem()) << FormatUnitSetTest::GetTestFusProblemDescription(MMFusBarFormatName);
    EXPECT_STREQ("MM(Real2)", MMFusBarFormatName.ToText(false).c_str());
    EXPECT_STREQ("MM(real2)", MMFusBarFormatName.ToText(true).c_str());
    }
    {
    FormatUnitSet MMFusBarFormatNameBar("MM|Real2|");
    EXPECT_FALSE(MMFusBarFormatNameBar.HasProblem()) << FormatUnitSetTest::GetTestFusProblemDescription(MMFusBarFormatNameBar);
    EXPECT_STREQ("MM(Real2)", MMFusBarFormatNameBar.ToText(false).c_str());
    EXPECT_STREQ("MM(real2)", MMFusBarFormatNameBar.ToText(true).c_str());
    }
    {
    FormatUnitSet MMFusJsonOnlyUnitName(R"json({
        unitName: "MM"
    })json");
    EXPECT_FALSE(MMFusJsonOnlyUnitName.HasProblem()) << FormatUnitSetTest::GetTestFusProblemDescription(MMFusJsonOnlyUnitName);
    EXPECT_STREQ("MM(Real2)", MMFusJsonOnlyUnitName.ToText(false).c_str());
    EXPECT_STREQ("MM(real2)", MMFusJsonOnlyUnitName.ToText(true).c_str());
    EXPECT_TRUE(MMFusJsonOnlyUnitName.IsIdentical(FormatUnitSet("MM(Real2)")));
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
    })json");
    EXPECT_FALSE(MMFusJsonUnitNameFormatName.HasProblem()) << FormatUnitSetTest::GetTestFusProblemDescription(MMFusJsonUnitNameFormatName);
    EXPECT_STREQ("MM(Real2)", MMFusJsonUnitNameFormatName.ToText(false).c_str());
    EXPECT_STREQ("MM(real2)", MMFusJsonUnitNameFormatName.ToText(true).c_str());
    EXPECT_TRUE(MMFusJsonUnitNameFormatName.IsIdentical(FormatUnitSet("MM(Real2)")));
    }
    {
    FormatUnitSet MMFusJsonAllMembers(R"json({
        "unitName": "MM",
        "formatName": "real2",
        "cloneData": false,
        "formatSpec": { }
    })json");
    EXPECT_FALSE(MMFusJsonAllMembers.HasProblem()) << FormatUnitSetTest::GetTestFusProblemDescription(MMFusJsonAllMembers);
    EXPECT_STREQ("MM(Real2)", MMFusJsonAllMembers.ToText(false).c_str());
    EXPECT_STREQ("MM(real2)", MMFusJsonAllMembers.ToText(true).c_str());
    EXPECT_TRUE(MMFusJsonAllMembers.IsIdentical(FormatUnitSet("MM(Real2)")));
    }
    {
    FAIL() << "TODO: Test FUS from JSON with formatSpec fully defined.";
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CompositeValueSpecTest, Constructors)
    {
    // Single Unit
    CompositeValueSpec cvs1unit("MILE");
    ASSERT_EQ(1, cvs1unit.GetUnitCount());
    ASSERT_EQ(CompositeSpecType::Single, cvs1unit.GetType());
    ASSERT_FALSE(cvs1unit.IsProblem());
    EXPECT_STREQ("mi", cvs1unit.GetMajorUnit()->GetLabel().c_str());
    // TODO: This causes a segfault >> EXPECT_STREQ("", cvs1unit.GetMiddleUnit()->GetLabel().c_str());
    // TODO: This causes a segfault >> EXPECT_STREQ("", cvs1unit.GetMinorUnit()->GetLabel().c_str());
    // TODO: This causes a segfault >> EXPECT_STREQ("", cvs1unit.GetSubUnit()->GetLabel().c_str());

    // Two Units
    CompositeValueSpec cvs2unit("MILE", "YRD");
    ASSERT_EQ(2, cvs2unit.GetUnitCount());
    ASSERT_EQ(CompositeSpecType::Double, cvs2unit.GetType());
    ASSERT_FALSE(cvs2unit.IsProblem());
    EXPECT_STREQ("mi", cvs2unit.GetMajorUnit()->GetLabel().c_str());
    EXPECT_STREQ("yd", cvs2unit.GetMiddleUnit()->GetLabel().c_str());
    // TODO: This causes a segfault >> EXPECT_STREQ("", cvs2unit.GetMinorUnit()->GetLabel().c_str());
    // TODO: This causes a segfault >> EXPECT_STREQ("", cvs2unit.GetSubUnit()->GetLabel().c_str());
    EXPECT_EQ(1760, cvs2unit.GetMajorToMiddleRatio());

    // Three Units
    CompositeValueSpec cvs3unit("MILE", "YRD", "FOOT");
    ASSERT_EQ(3, cvs3unit.GetUnitCount());
    ASSERT_EQ(CompositeSpecType::Triple, cvs3unit.GetType());
    ASSERT_FALSE(cvs3unit.IsProblem());
    EXPECT_STREQ("mi", cvs3unit.GetMajorUnit()->GetLabel().c_str());
    EXPECT_STREQ("yd", cvs3unit.GetMiddleUnit()->GetLabel().c_str());
    EXPECT_STREQ("ft", cvs3unit.GetMinorUnit()->GetLabel().c_str());
    // TODO: This causes a segfault >> EXPECT_STREQ("", cvs3unit.GetSubUnit()->GetLabel().c_str());
    EXPECT_EQ(1760, cvs3unit.GetMajorToMiddleRatio());
    EXPECT_EQ(3, cvs3unit.GetMiddleToMinorRatio());

    // Four Units
    CompositeValueSpec cvs4unit("MILE", "YRD", "FOOT", "INCH");
    ASSERT_EQ(4, cvs4unit.GetUnitCount());
    ASSERT_EQ(CompositeSpecType::Quatro, cvs4unit.GetType());
    ASSERT_FALSE(cvs4unit.IsProblem());
    EXPECT_STREQ("mi", cvs4unit.GetMajorUnit()->GetLabel().c_str());
    EXPECT_STREQ("yd", cvs4unit.GetMiddleUnit()->GetLabel().c_str());
    EXPECT_STREQ("ft", cvs4unit.GetMinorUnit()->GetLabel().c_str());
    EXPECT_STREQ("in", cvs4unit.GetSubUnit()->GetLabel().c_str());
    EXPECT_EQ(1760, cvs4unit.GetMajorToMiddleRatio());
    EXPECT_EQ(3, cvs4unit.GetMiddleToMinorRatio());
    EXPECT_EQ(12, cvs4unit.GetMinorToSubRatio());

    FAIL() << "Until the above commented out segfaulting code is investigated, this test should not be concidered passing. Maybe I'm accessing the public API wrong???";
    // TODO: Test the rest of the constructors.
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CompositeValueSpecTest, FormatValue)
    {
    CompositeValueSpec cvs("MILE", "YRD", "FOOT", "INCH");
    ASSERT_EQ(CompositeSpecType::Quatro, cvs.GetType());

    NumericFormatSpec nfs;
    nfs.SetPresentationType(PresentationType::Decimal);
    nfs.SetSignOption(ShowSignOption::OnlyNegative);

    // Yes putting the TODO in the expect stmt is bad style, but it's an easy way to show that this test is jank.
    EXPECT_STREQ("TODO: FormatValue always returns empty string. Figure out what's up", cvs.FormatValue(100.0, &nfs, "MILE").c_str());
    EXPECT_STREQ("TODO: FormatValue always returns empty string. Figure out what's up", cvs.FormatValue(100.0, &nfs, "YRD").c_str());
    EXPECT_STREQ("TODO: FormatValue always returns empty string. Figure out what's up", cvs.FormatValue(100.0, &nfs, "FOOT").c_str());
    EXPECT_STREQ("TODO: FormatValue always returns empty string. Figure out what's up", cvs.FormatValue(100.0, &nfs, "INCH").c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CompositeValueTest, Constructors)
    {
    CompositeValue cv;
    ASSERT_FALSE(cv.IsProblem());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CompositeValueTest, DataMemberSettersAndGetters)
    {
    CompositeValue cv;

    cv.SetMajor(1.0);
    cv.SetMiddle(2.0);
    cv.SetMinor(3.0);
    cv.SetSub(4.0);
    cv.SetInput(5.0);
    EXPECT_EQ(1.0, cv.GetMajor());
    EXPECT_EQ(2.0, cv.GetMiddle());
    EXPECT_EQ(3.0, cv.GetMinor());
    EXPECT_EQ(4.0, cv.GetSub());
    EXPECT_EQ(5.0, cv.GetInput());
    
    cv.SetPositive();
    EXPECT_STREQ("", cv.GetSignPrefix().c_str());
    EXPECT_STREQ("", cv.GetSignPrefix(true).c_str());
    EXPECT_STREQ("", cv.GetSignSuffix().c_str());
    EXPECT_STREQ("", cv.GetSignSuffix(true).c_str());

    cv.SetNegative();
    EXPECT_STREQ("-", cv.GetSignPrefix().c_str());
    EXPECT_STREQ("(", cv.GetSignPrefix(true).c_str());
    EXPECT_STREQ("", cv.GetSignSuffix().c_str());
    EXPECT_STREQ(")", cv.GetSignSuffix(true).c_str());
    }

END_BENTLEY_FORMATTEST_NAMESPACE
