/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/valueformat_test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include <Bentley/ValueFormat.h>

#define LOCS(str) str

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct  DoubleFormatTestData
    {
    WCharCP          m_expectedString;
    double           m_inputValue;
    PrecisionFormat  m_precision;
    bool             m_leadingZero;
    bool             m_trailingZeros;
    bool             m_insertThousandsSeparator;
    WChar            m_decimalSeparator;
    WChar            m_thousandsSeparator;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void    doFormatDoubleTest (DoubleFormatTestData const& testData)
    {
    DoubleFormatterPtr   formatter = DoubleFormatter::Create();

    formatter->SetPrecision      (testData.m_precision);
    formatter->SetLeadingZero    (testData.m_leadingZero);
    formatter->SetTrailingZeros  (testData.m_trailingZeros);
    formatter->SetInsertThousandsSeparator  (testData.m_insertThousandsSeparator);
    formatter->SetThousandsSeparator  (testData.m_thousandsSeparator);
    formatter->SetDecimalSeparator  (testData.m_decimalSeparator);

    Utf8String outputStr = formatter->ToString (testData.m_inputValue);

    ASSERT_STREQ (testData.m_expectedString, WString(outputStr.c_str(), BentleyCharEncoding::Utf8).c_str());
    }
//=======================================================================================
// @bsiclass                                                    Umar.Hayat     11/2015
//=======================================================================================
struct DoubleFormatterTest : public ::testing::Test
{
};
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleFormatterTest, FormatDecimal_TestPrecision)
    {
    DoubleFormatTestData testDataArray[] =
        {
        // expected         value           precision                        leadingZero, trailingZero, useThousandsSep, decSep, thousandsSep},
        { L"35",            35.123456789,   PrecisionFormat::DecimalWhole,   true,        true,         false,           L'.',   L','},
        { L"35.1",          35.123456789,   PrecisionFormat::Decimal1Place,  true,        true,         false,           L'.',   L','},
        { L"35.12",         35.123456789,   PrecisionFormat::Decimal2Places, true,        true,         false,           L'.',   L','},
        { L"35.123",        35.123456789,   PrecisionFormat::Decimal3Places, true,        true,         false,           L'.',   L','},
        { L"35.1235",       35.123456789,   PrecisionFormat::Decimal4Places, true,        true,         false,           L'.',   L','},
        { L"35.12346",      35.123456789,   PrecisionFormat::Decimal5Places, true,        true,         false,           L'.',   L','},
        { L"35.123457",     35.123456789,   PrecisionFormat::Decimal6Places, true,        true,         false,           L'.',   L','},
        { L"35.1234568",    35.123456789,   PrecisionFormat::Decimal7Places, true,        true,         false,           L'.',   L','},
        { L"35.12345679",   35.123456789,   PrecisionFormat::Decimal8Places, true,        true,         false,           L'.',   L','},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        doFormatDoubleTest (testDataArray[iTest]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleFormatterTest, FormatDecimal_TestRounding)
    {
    DoubleFormatTestData testDataArray[] =
        {
        // expected         value           precision                        leadingZero, trailingZero, useThousandsSep, decSep, thousandsSep},
        { L"35.1234",       35.12340,       PrecisionFormat::Decimal4Places, true,        true,         false,           L'.',   L','},
        { L"35.1234",       35.123445,      PrecisionFormat::Decimal4Places, true,        true,         false,           L'.',   L','},
        { L"35.1234",       35.123449,      PrecisionFormat::Decimal4Places, true,        true,         false,           L'.',   L','},
        { L"35.1234",       35.1234499,     PrecisionFormat::Decimal4Places, true,        true,         false,           L'.',   L','},
        { L"35.1234",       35.12344999,    PrecisionFormat::Decimal4Places, true,        true,         false,           L'.',   L','},
        { L"35.1235",       35.12350000,    PrecisionFormat::Decimal4Places, true,        true,         false,           L'.',   L','},
        { L"35.1235",       35.12350001,    PrecisionFormat::Decimal4Places, true,        true,         false,           L'.',   L','},
        { L"35.1235",       35.12350002,    PrecisionFormat::Decimal4Places, true,        true,         false,           L'.',   L','},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        doFormatDoubleTest (testDataArray[iTest]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleFormatterTest, FormatDecimal_TestZeros)
    {
    DoubleFormatTestData testDataArray[] =
        {
        // expected         value           precision                        leadingZero, trailingZero, useThousandsSep, decSep, thousandsSep},
        { L"0.0000",        0.0,            PrecisionFormat::Decimal4Places, true,        true,         false,           L'.',   L','},
        { L"0",             0.0,            PrecisionFormat::Decimal4Places, true,        false,        false,           L'.',   L','},
        { L".0000",         0.0,            PrecisionFormat::Decimal4Places, false,       true,         false,           L'.',   L','},
        { L"0",             0.0,            PrecisionFormat::Decimal4Places, false,       false,        false,           L'.',   L','},
        { L"100.1000",      100.1,          PrecisionFormat::Decimal4Places, true,        true,         false,           L'.',   L','},
        { L"100.1",         100.1,          PrecisionFormat::Decimal4Places, true,        false,        false,           L'.',   L','},
        { L"100.1000",      100.1,          PrecisionFormat::Decimal4Places, false,       true,         false,           L'.',   L','},
        { L"100.1",         100.1,          PrecisionFormat::Decimal4Places, false,       false,        false,           L'.',   L','},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        doFormatDoubleTest (testDataArray[iTest]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleFormatterTest, FormatDecimal_TestSeparators)
    {
    DoubleFormatTestData testDataArray[] =
        {
        // expected         value           precision                        leadingZero, trailingZero, useThousandsSep, decSep, thousandsSep},
        { L"10.45",         10.45,          PrecisionFormat::Decimal2Places, true,        true,         false,           L'.',   L','},
        { L"10#45",         10.45,          PrecisionFormat::Decimal2Places, true,        true,         false,           L'#',   L','},
        { L"10.45",         10.45,          PrecisionFormat::Decimal2Places, true,        true,         true,            L'.',   L','},
        { L"10#45",         10.45,          PrecisionFormat::Decimal2Places, true,        true,         true,            L'#',   L','},
        { L"1000#45",       1000.45,        PrecisionFormat::Decimal2Places, true,        true,         false,           L'#',   L','},
        { L"1,000#45",      1000.45,        PrecisionFormat::Decimal2Places, true,        true,         true,            L'#',   L','},
        { L"1000#45",       1000.45,        PrecisionFormat::Decimal2Places, true,        true,         false,           L'#',   L'!'},
        { L"1!000#45",      1000.45,        PrecisionFormat::Decimal2Places, true,        true,         true,            L'#',   L'!'},
        { L"100",           100,            PrecisionFormat::Decimal2Places, true,        false,        true,            L'.',   L','},
        { L"1,000",         1000,           PrecisionFormat::Decimal2Places, true,        false,        true,            L'.',   L','},
        { L"10,000",        10000,          PrecisionFormat::Decimal2Places, true,        false,        true,            L'.',   L','},
        { L"100,000",       100000,         PrecisionFormat::Decimal2Places, true,        false,        true,            L'.',   L','},
        { L"1,000,000",     1000000,        PrecisionFormat::Decimal2Places, true,        false,        true,            L'.',   L','},
        { L"1x000x000",     1000000,        PrecisionFormat::Decimal2Places, true,        false,        true,            L'.',   L'x'},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        doFormatDoubleTest (testDataArray[iTest]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleFormatterTest, FormatFractional_TestPrecision)
    {
    DoubleFormatTestData testDataArray[] =
        {
        // expected         value           precision                               leadingZero, trailingZero, useThousandsSep, decSep, thousandsSep},
        { L"35",            35,             PrecisionFormat::FractionalWhole,       true,        true,         false,           L'.',   L','},
        { L"35 1/2",        35.5,           PrecisionFormat::FractionalHalf,        true,        true,         false,           L'.',   L','},
        { L"35 3/4",        35.75,          PrecisionFormat::FractionalQuarter,     true,        true,         false,           L'.',   L','},
        { L"35 7/8",        35.875,         PrecisionFormat::FractionalEighth,      true,        true,         false,           L'.',   L','},
        { L"35 15/16",      35.9375,        PrecisionFormat::FractionalSixteenth,   true,        true,         false,           L'.',   L','},
        { L"35 31/32",      35.96875,       PrecisionFormat::Fractional1_Over_32,   true,        true,         false,           L'.',   L','},
        { L"35 63/64",      35.984375,      PrecisionFormat::Fractional1_Over_64,   true,        true,         false,           L'.',   L','},
        { L"35 127/128",    35.9921875,     PrecisionFormat::Fractional1_Over_128,  true,        true,         false,           L'.',   L','},
        { L"35 255/256",    35.99609375,    PrecisionFormat::Fractional1_Over_256,  true,        true,         false,           L'.',   L','},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        doFormatDoubleTest (testDataArray[iTest]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleFormatterTest, FormatFractional_TestRounding)
    {
    DoubleFormatTestData testDataArray[] =
        {
        // expected         value           precision                               leadingZero, trailingZero, useThousandsSep, decSep, thousandsSep},
        { L"35 3/8",        35.40624,       PrecisionFormat::FractionalSixteenth,   true,        true,         false,           L'.',   L','},
        { L"35 3/8",        35.406245,      PrecisionFormat::FractionalSixteenth,   true,        true,         false,           L'.',   L','},
        { L"35 3/8",        35.40624999,    PrecisionFormat::FractionalSixteenth,   true,        true,         false,           L'.',   L','},
        { L"35 7/16",       35.40625,       PrecisionFormat::FractionalSixteenth,   true,        true,         false,           L'.',   L','},
        { L"35 7/16",       35.41,          PrecisionFormat::FractionalSixteenth,   true,        true,         false,           L'.',   L','},
        { L"35 7/16",       35.4375,        PrecisionFormat::FractionalSixteenth,   true,        true,         false,           L'.',   L','},
        { L"35 7/16",       35.45,          PrecisionFormat::FractionalSixteenth,   true,        true,         false,           L'.',   L','},
        { L"35 1/2",        35.46875,       PrecisionFormat::FractionalSixteenth,   true,        true,         false,           L'.',   L','},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        doFormatDoubleTest (testDataArray[iTest]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleFormatterTest, FormatScientific_TestPrecision)
    {
    DoubleFormatTestData testDataArray[] =
        {
        // expected         value            precision                           leadingZero, trailingZero, useThousandsSep, decSep, thousandsSep},
        { L"1E-03",           0.00123,       PrecisionFormat::ScientificWhole,   true,        true,         false,           L'.',   L','},
        { L"1E-02",           0.0123,        PrecisionFormat::ScientificWhole,   true,        true,         false,           L'.',   L','},
        { L"1E-01",           0.123,         PrecisionFormat::ScientificWhole,   true,        true,         false,           L'.',   L','},
        { L"1E+00",           1.23,          PrecisionFormat::ScientificWhole,   true,        true,         false,           L'.',   L','},
        { L"1E+01",          12.3,           PrecisionFormat::ScientificWhole,   true,        true,         false,           L'.',   L','},
        { L"1E+02",         123.0,           PrecisionFormat::ScientificWhole,   true,        true,         false,           L'.',   L','},
        { L"1.23E-03",        0.00123,       PrecisionFormat::Scientific2Places, true,        true,         false,           L'.',   L','},
        { L"1.23E-02",        0.0123,        PrecisionFormat::Scientific2Places, true,        true,         false,           L'.',   L','},
        { L"1.23E-01",        0.123,         PrecisionFormat::Scientific2Places, true,        true,         false,           L'.',   L','},
        { L"1.23E+00",        1.23,          PrecisionFormat::Scientific2Places, true,        true,         false,           L'.',   L','},
        { L"1.23E+01",       12.3,           PrecisionFormat::Scientific2Places, true,        true,         false,           L'.',   L','},
        { L"1.23E+02",      123.0,           PrecisionFormat::Scientific2Places, true,        true,         false,           L'.',   L','},
        { L"1.2300E-03",      0.00123,       PrecisionFormat::Scientific4Places, true,        true,         false,           L'.',   L','},
        { L"1.2300E-02",      0.0123,        PrecisionFormat::Scientific4Places, true,        true,         false,           L'.',   L','},
        { L"1.2300E-01",      0.123,         PrecisionFormat::Scientific4Places, true,        true,         false,           L'.',   L','},
        { L"1.2300E+00",      1.23,          PrecisionFormat::Scientific4Places, true,        true,         false,           L'.',   L','},
        { L"1.2300E+01",     12.3,           PrecisionFormat::Scientific4Places, true,        true,         false,           L'.',   L','},
        { L"1.2300E+02",    123.0,           PrecisionFormat::Scientific4Places, true,        true,         false,           L'.',   L','},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        doFormatDoubleTest (testDataArray[iTest]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Julija.Suboc    09/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleFormatterTest, MakeClone)
    {
    DoubleFormatterPtr formatter1 = DoubleFormatter::Create();
    DoubleFormatterPtr formatter2 = formatter1->Clone();
    EXPECT_TRUE(formatter1 != formatter2)<<"Expected to get clone of the object.";
    //Save default values for later use
    PrecisionFormat defaultPrecision = formatter1->GetPrecision();
    WChar defaultDecimalSep = formatter1->GetDecimalSeparator();
    WChar defaultThousandSep = formatter1->GetThousandsSeparator();
    bool defaultInsertThousands = formatter1->GetInsertThousandsSeparator();
    bool defaultLeadingZero = formatter1->GetLeadingZero(); 
    bool defaultTrailingZero = formatter1->GetTrailingZeros();
    //Check that formatters are identical 
    EXPECT_EQ(static_cast<int>(defaultPrecision), static_cast<int>(formatter2->GetPrecision()));
    EXPECT_TRUE(defaultDecimalSep == formatter2->GetDecimalSeparator());
    EXPECT_TRUE(defaultThousandSep == formatter2->GetThousandsSeparator());
    EXPECT_EQ(defaultInsertThousands, formatter2->GetInsertThousandsSeparator());
    EXPECT_EQ(defaultLeadingZero, formatter2->GetLeadingZero());
    EXPECT_EQ(defaultTrailingZero, formatter2->GetTrailingZeros());
    //Change clone values
    formatter2->SetPrecision(PrecisionFormat::ScientificWhole);
    formatter2->SetDecimalSeparator('q');
    formatter2->SetThousandsSeparator('|');
    formatter2->SetInsertThousandsSeparator(!defaultInsertThousands);
    formatter2->SetLeadingZero(!defaultLeadingZero);
    formatter2->SetTrailingZeros(!defaultTrailingZero);
    //Check that values were changed correctly for formatter2 but formatter1 has the same values
    EXPECT_TRUE((defaultPrecision == formatter1->GetPrecision()) && (PrecisionFormat::ScientificWhole == formatter2->GetPrecision()));
    
    EXPECT_TRUE (defaultDecimalSep == formatter1->GetDecimalSeparator());
    EXPECT_TRUE('q' == formatter2->GetDecimalSeparator());

    EXPECT_TRUE(defaultThousandSep == formatter1->GetThousandsSeparator());
    EXPECT_TRUE('|' == formatter2->GetThousandsSeparator());
    
    EXPECT_EQ(defaultInsertThousands, formatter1->GetInsertThousandsSeparator());
    EXPECT_EQ(!defaultInsertThousands, formatter2->GetInsertThousandsSeparator());
    
    EXPECT_TRUE((defaultLeadingZero == formatter1->GetLeadingZero()) && (!defaultLeadingZero == formatter2->GetLeadingZero()));
    EXPECT_TRUE((defaultTrailingZero == formatter1->GetTrailingZeros()) && (!defaultTrailingZero == formatter2->GetTrailingZeros()));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Julija.Suboc    09/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DoubleFormatterTest, SetAndGetPrecision)
    {
    DoubleFormatterPtr formatter = DoubleFormatter::Create();
    bvector<PrecisionFormat> precisionTypes;
    precisionTypes.push_back(PrecisionFormat::DecimalWhole);
    precisionTypes.push_back(PrecisionFormat::Decimal1Place);
    precisionTypes.push_back(PrecisionFormat::Decimal2Places);
    precisionTypes.push_back(PrecisionFormat::Decimal3Places);
    precisionTypes.push_back(PrecisionFormat::Decimal4Places);
    precisionTypes.push_back(PrecisionFormat::Decimal5Places);
    precisionTypes.push_back(PrecisionFormat::Decimal6Places);
    precisionTypes.push_back(PrecisionFormat::Decimal7Places);
    precisionTypes.push_back(PrecisionFormat::Decimal8Places);
    precisionTypes.push_back(PrecisionFormat::FractionalWhole);
    precisionTypes.push_back(PrecisionFormat::FractionalHalf);
    precisionTypes.push_back(PrecisionFormat::FractionalQuarter);
    precisionTypes.push_back(PrecisionFormat::FractionalEighth);
    precisionTypes.push_back(PrecisionFormat::FractionalSixteenth);
    precisionTypes.push_back(PrecisionFormat::Fractional1_Over_32);
    precisionTypes.push_back(PrecisionFormat::Fractional1_Over_64);
    precisionTypes.push_back(PrecisionFormat::Fractional1_Over_128);
    precisionTypes.push_back(PrecisionFormat::Fractional1_Over_256);
    precisionTypes.push_back(PrecisionFormat::ScientificWhole);
    precisionTypes.push_back(PrecisionFormat::Scientific1Place);
    precisionTypes.push_back(PrecisionFormat::Scientific2Places);
    precisionTypes.push_back(PrecisionFormat::Scientific3Places);
    precisionTypes.push_back(PrecisionFormat::Scientific4Places);
    precisionTypes.push_back(PrecisionFormat::Scientific5Places);
    precisionTypes.push_back(PrecisionFormat::Scientific6Places);
    precisionTypes.push_back(PrecisionFormat::Scientific7Places);
    precisionTypes.push_back(PrecisionFormat::Scientific8Places);
    while(!precisionTypes.empty())
        {
        PrecisionFormat precision = precisionTypes.back();
        formatter->SetPrecision(precision);
        EXPECT_EQ(static_cast<int>(precision), static_cast<int>(formatter->GetPrecision()));
        precisionTypes.pop_back();
        }
    }
