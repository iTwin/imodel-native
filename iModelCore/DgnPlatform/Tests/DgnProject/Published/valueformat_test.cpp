/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/valueformat_test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnPlatformApi.h>
#include <Bentley/BeTest.h>
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/BackDoor.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_DGNDB_UNIT_TESTS_NAMESPACE

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DoubleFormatter, FormatDecimal_TestPrecision)
    {
    ScopedDgnHost autoDgnHost;
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
TEST (DoubleFormatter, FormatDecimal_TestRounding)
    {
    ScopedDgnHost autoDgnHost;
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
TEST (DoubleFormatter, FormatDecimal_TestZeros)
    {
    ScopedDgnHost autoDgnHost;
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
TEST (DoubleFormatter, FormatDecimal_TestSeparators)
    {
    ScopedDgnHost autoDgnHost;
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
TEST (DoubleFormatter, FormatFractional_TestPrecision)
    {
    ScopedDgnHost autoDgnHost;
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
TEST (DoubleFormatter, FormatFractional_TestRounding)
    {
    ScopedDgnHost autoDgnHost;
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
TEST (DoubleFormatter, FormatScientific_TestPrecision)
    {
    ScopedDgnHost autoDgnHost;
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
TEST(DoubleFormatter, MakeClone)
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
TEST(DoubleFormatter, SetAndGetPrecision)
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
	
WChar const DEGREE_PLACEHOLDER = L'^';

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
WString replaceDegreeChar (WCharCP str)
    {
    WString outStr(str);

    size_t placeHolderPos = outStr.find (DEGREE_PLACEHOLDER);

    if (placeHolderPos != std::string::npos )
        {
        outStr.replace (placeHolderPos, 1, 1, 0x00b0 /* Unicode code point for degree*/);
        }

    return outStr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
struct  AngleFormatTestData
    {
    WCharCP         m_expectedString;
    double          m_inputValue;
    AngleMode       m_angleMode;
    AnglePrecision  m_anglePrec;
    bool            m_leadingZero;
    bool            m_trailingZeros;
    bool            m_allowNegative;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    doFormatAngleTest (AngleFormatTestData const& testData)
    {
#if defined (NOT_NOW)
    static int s_count = 0;
    printf ("%d\n", s_count++);
#endif

    AngleFormatterPtr   formatter = AngleFormatter::Create();

    formatter->SetAngleMode      (testData.m_angleMode);
    formatter->SetAnglePrecision (testData.m_anglePrec);
    formatter->SetLeadingZero    (testData.m_leadingZero);
    formatter->SetTrailingZeros  (testData.m_trailingZeros);
    formatter->SetAllowNegative  (testData.m_allowNegative);

    WString outputStr = formatter->ToString (testData.m_inputValue);
    WString expectStr = replaceDegreeChar (testData.m_expectedString);

    ASSERT_STREQ (expectStr.c_str(), outputStr.c_str());
    }

#define MIN_TO_DEG  (1.0 / 60)
#define SEC_TO_DEG  (1.0 / 3600)
#define GRAD_TO_DEG (90.0 / 100)
#define RAD_TO_DEG  (180.0 / 3.141592653589793238462643)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (AngleFormatter, FormatDegree_TestPrecision)
    {
    ScopedDgnHost autoDgnHost;
    AngleFormatTestData testDataArray[] =
        {
        { L"35^",          35.123456789, AngleMode::Degrees, AnglePrecision::Whole,   true, true, true},
        { L"35.1^",        35.123456789, AngleMode::Degrees, AnglePrecision::Use1Place, true, true, true},
        { L"35.12^",       35.123456789, AngleMode::Degrees, AnglePrecision::Use2Places, true, true, true},
        { L"35.123^",      35.123456789, AngleMode::Degrees, AnglePrecision::Use3Places, true, true, true},
        { L"35.1235^",     35.123456789, AngleMode::Degrees, AnglePrecision::Use4Places, true, true, true},
        { L"35.12346^",    35.123456789, AngleMode::Degrees, AnglePrecision::Use5Places, true, true, true},
        { L"35.123457^",   35.123456789, AngleMode::Degrees, AnglePrecision::Use6Places, true, true, true},
        { L"35.1234568^",  35.123456789, AngleMode::Degrees, AnglePrecision::Use7Places, true, true, true},
        { L"35.12345679^", 35.123456789, AngleMode::Degrees, AnglePrecision::Use8Places, true, true, true},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        doFormatAngleTest (testDataArray[iTest]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (AngleFormatter, FormatDegree_TestFullCircle)
    {
    ScopedDgnHost autoDgnHost;
    AngleFormatTestData testDataArray[] =
        {
        {   L"0.00^",     0.0, AngleMode::Degrees, AnglePrecision::Use2Places, true, true, true},
        {  L"30.00^",    30.0, AngleMode::Degrees, AnglePrecision::Use2Places, true, true, true},
        {  L"60.00^",    60.0, AngleMode::Degrees, AnglePrecision::Use2Places, true, true, true},
        {  L"90.00^",    90.0, AngleMode::Degrees, AnglePrecision::Use2Places, true, true, true},
        { L"120.00^",   120.0, AngleMode::Degrees, AnglePrecision::Use2Places, true, true, true},
        { L"150.00^",   150.0, AngleMode::Degrees, AnglePrecision::Use2Places, true, true, true},
        { L"180.00^",   180.0, AngleMode::Degrees, AnglePrecision::Use2Places, true, true, true},
        { L"210.00^",   210.0, AngleMode::Degrees, AnglePrecision::Use2Places, true, true, true},
        { L"240.00^",   240.0, AngleMode::Degrees, AnglePrecision::Use2Places, true, true, true},
        { L"270.00^",   270.0, AngleMode::Degrees, AnglePrecision::Use2Places, true, true, true},
        { L"300.00^",   300.0, AngleMode::Degrees, AnglePrecision::Use2Places, true, true, true},
        { L"330.00^",   330.0, AngleMode::Degrees, AnglePrecision::Use2Places, true, true, true},
        { L"360.00^",   360.0, AngleMode::Degrees, AnglePrecision::Use2Places, true, true, true},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        doFormatAngleTest (testDataArray[iTest]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (AngleFormatter, FormatDegMin_TestPrecision)
    {
    ScopedDgnHost autoDgnHost;
    AngleFormatTestData testDataArray[] =
        {
        { L"35^35'",           35 + 35.123456789 * MIN_TO_DEG, AngleMode::DegMin, AnglePrecision::Whole,   true, true, true},
        { L"35^35.1'",         35 + 35.123456789 * MIN_TO_DEG, AngleMode::DegMin, AnglePrecision::Use1Place, true, true, true},
        { L"35^35.12'",        35 + 35.123456789 * MIN_TO_DEG, AngleMode::DegMin, AnglePrecision::Use2Places, true, true, true},
        { L"35^35.123'",       35 + 35.123456789 * MIN_TO_DEG, AngleMode::DegMin, AnglePrecision::Use3Places, true, true, true},
        { L"35^35.1235'",      35 + 35.123456789 * MIN_TO_DEG, AngleMode::DegMin, AnglePrecision::Use4Places, true, true, true},
        { L"35^35.12346'",     35 + 35.123456789 * MIN_TO_DEG, AngleMode::DegMin, AnglePrecision::Use5Places, true, true, true},
        { L"35^35.123457'",    35 + 35.123456789 * MIN_TO_DEG, AngleMode::DegMin, AnglePrecision::Use6Places, true, true, true},
        { L"35^35.1234568'",   35 + 35.123456789 * MIN_TO_DEG, AngleMode::DegMin, AnglePrecision::Use7Places, true, true, true},
        { L"35^35.12345679'",  35 + 35.123456789 * MIN_TO_DEG, AngleMode::DegMin, AnglePrecision::Use8Places, true, true, true},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        doFormatAngleTest (testDataArray[iTest]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (AngleFormatter, FormatDegMinSec_TestPrecision)
    {
    ScopedDgnHost autoDgnHost;
    AngleFormatTestData testDataArray[] =
        {
        { L"35^00'35\"",           35 + 35.123456789 * SEC_TO_DEG, AngleMode::DegMinSec, AnglePrecision::Whole,   true, true, true},
        { L"35^00'35.1\"",         35 + 35.123456789 * SEC_TO_DEG, AngleMode::DegMinSec, AnglePrecision::Use1Place, true, true, true},
        { L"35^00'35.12\"",        35 + 35.123456789 * SEC_TO_DEG, AngleMode::DegMinSec, AnglePrecision::Use2Places, true, true, true},
        { L"35^00'35.123\"",       35 + 35.123456789 * SEC_TO_DEG, AngleMode::DegMinSec, AnglePrecision::Use3Places, true, true, true},
        { L"35^00'35.1235\"",      35 + 35.123456789 * SEC_TO_DEG, AngleMode::DegMinSec, AnglePrecision::Use4Places, true, true, true},
        { L"35^00'35.12346\"",     35 + 35.123456789 * SEC_TO_DEG, AngleMode::DegMinSec, AnglePrecision::Use5Places, true, true, true},
        { L"35^00'35.123457\"",    35 + 35.123456789 * SEC_TO_DEG, AngleMode::DegMinSec, AnglePrecision::Use6Places, true, true, true},
        { L"35^00'35.1234568\"",   35 + 35.123456789 * SEC_TO_DEG, AngleMode::DegMinSec, AnglePrecision::Use7Places, true, true, true},
        { L"35^00'35.12345679\"",  35 + 35.123456789 * SEC_TO_DEG, AngleMode::DegMinSec, AnglePrecision::Use8Places, true, true, true},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        doFormatAngleTest (testDataArray[iTest]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (AngleFormatter, FormatCentesimal_TestPrecision)
    {
    ScopedDgnHost autoDgnHost;
    AngleFormatTestData testDataArray[] =
        {
        { L"50g",          50.123456789 * GRAD_TO_DEG, AngleMode::Centesimal, AnglePrecision::Whole,   true, true, true},
        { L"50.1g",        50.123456789 * GRAD_TO_DEG, AngleMode::Centesimal, AnglePrecision::Use1Place, true, true, true},
        { L"50.12g",       50.123456789 * GRAD_TO_DEG, AngleMode::Centesimal, AnglePrecision::Use2Places, true, true, true},
        { L"50.123g",      50.123456789 * GRAD_TO_DEG, AngleMode::Centesimal, AnglePrecision::Use3Places, true, true, true},
        { L"50.1235g",     50.123456789 * GRAD_TO_DEG, AngleMode::Centesimal, AnglePrecision::Use4Places, true, true, true},
        { L"50.12346g",    50.123456789 * GRAD_TO_DEG, AngleMode::Centesimal, AnglePrecision::Use5Places, true, true, true},
        { L"50.123457g",   50.123456789 * GRAD_TO_DEG, AngleMode::Centesimal, AnglePrecision::Use6Places, true, true, true},
        { L"50.1234568g",  50.123456789 * GRAD_TO_DEG, AngleMode::Centesimal, AnglePrecision::Use7Places, true, true, true},
        { L"50.12345679g", 50.123456789 * GRAD_TO_DEG, AngleMode::Centesimal, AnglePrecision::Use8Places, true, true, true},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        doFormatAngleTest (testDataArray[iTest]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (AngleFormatter, FormatCentesimal_TestFullCircle)
    {
    ScopedDgnHost autoDgnHost;
    AngleFormatTestData testDataArray[] =
        {
        {   L"0.00g",     0.0 * GRAD_TO_DEG, AngleMode::Centesimal, AnglePrecision::Use2Places, true, true, true},
        {  L"25.00g",    25.0 * GRAD_TO_DEG, AngleMode::Centesimal, AnglePrecision::Use2Places, true, true, true},
        {  L"50.00g",    50.0 * GRAD_TO_DEG, AngleMode::Centesimal, AnglePrecision::Use2Places, true, true, true},
        {  L"75.00g",    75.0 * GRAD_TO_DEG, AngleMode::Centesimal, AnglePrecision::Use2Places, true, true, true},
        { L"100.00g",   100.0 * GRAD_TO_DEG, AngleMode::Centesimal, AnglePrecision::Use2Places, true, true, true},
        { L"125.00g",   125.0 * GRAD_TO_DEG, AngleMode::Centesimal, AnglePrecision::Use2Places, true, true, true},
        { L"150.00g",   150.0 * GRAD_TO_DEG, AngleMode::Centesimal, AnglePrecision::Use2Places, true, true, true},
        { L"175.00g",   175.0 * GRAD_TO_DEG, AngleMode::Centesimal, AnglePrecision::Use2Places, true, true, true},
        { L"200.00g",   200.0 * GRAD_TO_DEG, AngleMode::Centesimal, AnglePrecision::Use2Places, true, true, true},
        { L"225.00g",   225.0 * GRAD_TO_DEG, AngleMode::Centesimal, AnglePrecision::Use2Places, true, true, true},
        { L"250.00g",   250.0 * GRAD_TO_DEG, AngleMode::Centesimal, AnglePrecision::Use2Places, true, true, true},
        { L"275.00g",   275.0 * GRAD_TO_DEG, AngleMode::Centesimal, AnglePrecision::Use2Places, true, true, true},
        { L"300.00g",   300.0 * GRAD_TO_DEG, AngleMode::Centesimal, AnglePrecision::Use2Places, true, true, true},
        { L"325.00g",   325.0 * GRAD_TO_DEG, AngleMode::Centesimal, AnglePrecision::Use2Places, true, true, true},
        { L"350.00g",   350.0 * GRAD_TO_DEG, AngleMode::Centesimal, AnglePrecision::Use2Places, true, true, true},
        { L"375.00g",   375.0 * GRAD_TO_DEG, AngleMode::Centesimal, AnglePrecision::Use2Places, true, true, true},
        { L"400.00g",   400.0 * GRAD_TO_DEG, AngleMode::Centesimal, AnglePrecision::Use2Places, true, true, true},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        doFormatAngleTest (testDataArray[iTest]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (AngleFormatter, FormatRadians_TestPrecision)
    {
    ScopedDgnHost autoDgnHost;
    AngleFormatTestData testDataArray[] =
        {
        { L"2r",          2.123456789 * RAD_TO_DEG, AngleMode::Radians, AnglePrecision::Whole,   true, true, true},
        { L"2.1r",        2.123456789 * RAD_TO_DEG, AngleMode::Radians, AnglePrecision::Use1Place, true, true, true},
        { L"2.12r",       2.123456789 * RAD_TO_DEG, AngleMode::Radians, AnglePrecision::Use2Places, true, true, true},
        { L"2.123r",      2.123456789 * RAD_TO_DEG, AngleMode::Radians, AnglePrecision::Use3Places, true, true, true},
        { L"2.1235r",     2.123456789 * RAD_TO_DEG, AngleMode::Radians, AnglePrecision::Use4Places, true, true, true},
        { L"2.12346r",    2.123456789 * RAD_TO_DEG, AngleMode::Radians, AnglePrecision::Use5Places, true, true, true},
        { L"2.123457r",   2.123456789 * RAD_TO_DEG, AngleMode::Radians, AnglePrecision::Use6Places, true, true, true},
        { L"2.1234568r",  2.123456789 * RAD_TO_DEG, AngleMode::Radians, AnglePrecision::Use7Places, true, true, true},
        { L"2.12345679r", 2.123456789 * RAD_TO_DEG, AngleMode::Radians, AnglePrecision::Use8Places, true, true, true},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        doFormatAngleTest (testDataArray[iTest]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (AngleFormatter, FormatDegree_TestZeros)
    {
    ScopedDgnHost autoDgnHost;
    AngleFormatTestData testDataArray[] =
        {
        { L"0^",         0.0, AngleMode::Degrees, AnglePrecision::Whole,   true,  true,  true},
        { L"0^",         0.0, AngleMode::Degrees, AnglePrecision::Whole,   true,  false, true},
        { L"0^",         0.0, AngleMode::Degrees, AnglePrecision::Whole,   false, true,  true},
        { L"0^",         0.0, AngleMode::Degrees, AnglePrecision::Whole,   false, false, true},

        { L"0.0^",       0.0, AngleMode::Degrees, AnglePrecision::Use1Place, true,  true,  true},
        { L"0^",         0.0, AngleMode::Degrees, AnglePrecision::Use1Place, true,  false, true},
        { L".0^",        0.0, AngleMode::Degrees, AnglePrecision::Use1Place, false, true,  true},
        { L"0^",         0.0, AngleMode::Degrees, AnglePrecision::Use1Place, false, false, true},

        { L"0.5^",       0.5, AngleMode::Degrees, AnglePrecision::Use1Place, true,  true,  true},
        { L"0.5^",       0.5, AngleMode::Degrees, AnglePrecision::Use1Place, true,  false, true},
        { L".5^",        0.5, AngleMode::Degrees, AnglePrecision::Use1Place, false, true,  true},
        { L".5^",        0.5, AngleMode::Degrees, AnglePrecision::Use1Place, false, false, true},

        { L"0.5000^",    0.5, AngleMode::Degrees, AnglePrecision::Use4Places, true,  true,  true},
        { L"0.5^",       0.5, AngleMode::Degrees, AnglePrecision::Use4Places, true,  false, true},
        { L".5000^",     0.5, AngleMode::Degrees, AnglePrecision::Use4Places, false, true,  true},
        { L".5^",        0.5, AngleMode::Degrees, AnglePrecision::Use4Places, false, false, true},

        { L"35.5000^",  35.5, AngleMode::Degrees, AnglePrecision::Use4Places, true,  true,  true},
        { L"35.5^",     35.5, AngleMode::Degrees, AnglePrecision::Use4Places, true,  false, true},
        { L"35.5000^",  35.5, AngleMode::Degrees, AnglePrecision::Use4Places, false, true,  true},
        { L"35.5^",     35.5, AngleMode::Degrees, AnglePrecision::Use4Places, false, false, true},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        doFormatAngleTest (testDataArray[iTest]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (AngleFormatter, FormatDegMin_TestZeros)
    {
    ScopedDgnHost autoDgnHost;
    AngleFormatTestData testDataArray[] =
        {
        { L"00^00'",       0.0, AngleMode::DegMin, AnglePrecision::Whole,   true,  true,  true},
        { L"00^00'",       0.0, AngleMode::DegMin, AnglePrecision::Whole,   true,  false, true},
        { L"0^0'",         0.0, AngleMode::DegMin, AnglePrecision::Whole,   false, true,  true},
        { L"0^0'",         0.0, AngleMode::DegMin, AnglePrecision::Whole,   false, false, true},

        { L"00^00.0'",     0.0, AngleMode::DegMin, AnglePrecision::Use1Place, true,  true,  true},
        { L"00^00'",       0.0, AngleMode::DegMin, AnglePrecision::Use1Place, true,  false, true},
        { L"0^0.0'",       0.0, AngleMode::DegMin, AnglePrecision::Use1Place, false, true,  true},
        { L"0^0'",         0.0, AngleMode::DegMin, AnglePrecision::Use1Place, false, false, true},

        { L"00^30.0'",     0.5, AngleMode::DegMin, AnglePrecision::Use1Place, true,  true,  true},
        { L"00^30'",       0.5, AngleMode::DegMin, AnglePrecision::Use1Place, true,  false, true},
        { L"0^30.0'",      0.5, AngleMode::DegMin, AnglePrecision::Use1Place, false, true,  true},
        { L"0^30'",        0.5, AngleMode::DegMin, AnglePrecision::Use1Place, false, false, true},

        { L"00^30.0000'",  0.5, AngleMode::DegMin, AnglePrecision::Use4Places, true,  true,  true},
        { L"00^30'",       0.5, AngleMode::DegMin, AnglePrecision::Use4Places, true,  false, true},
        { L"0^30.0000'",   0.5, AngleMode::DegMin, AnglePrecision::Use4Places, false, true,  true},
        { L"0^30'",        0.5, AngleMode::DegMin, AnglePrecision::Use4Places, false, false, true},

        { L"35^30.0000'", 35.5, AngleMode::DegMin, AnglePrecision::Use4Places, true,  true,  true},
        { L"35^30'",      35.5, AngleMode::DegMin, AnglePrecision::Use4Places, true,  false, true},
        { L"35^30.0000'", 35.5, AngleMode::DegMin, AnglePrecision::Use4Places, false, true,  true},
        { L"35^30'",      35.5, AngleMode::DegMin, AnglePrecision::Use4Places, false, false, true},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        doFormatAngleTest (testDataArray[iTest]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (AngleFormatter, FormatDegMinSec_TestZeros)
    {
    ScopedDgnHost autoDgnHost;
    AngleFormatTestData testDataArray[] =
        {
        { L"00^00'00\"",      0.0,                  AngleMode::DegMinSec, AnglePrecision::Whole,   true,  true,  true},
        { L"00^00'00\"",      0.0,                  AngleMode::DegMinSec, AnglePrecision::Whole,   true,  false, true},
        { L"0^0'0\"",         0.0,                  AngleMode::DegMinSec, AnglePrecision::Whole,   false, true,  true},
        { L"0^0'0\"",         0.0,                  AngleMode::DegMinSec, AnglePrecision::Whole,   false, false, true},

        { L"00^00'00.0\"",    0.0,                  AngleMode::DegMinSec, AnglePrecision::Use1Place, true,  true,  true},
        { L"00^00'00\"",      0.0,                  AngleMode::DegMinSec, AnglePrecision::Use1Place, true,  false, true},
        { L"0^0'0.0\"",       0.0,                  AngleMode::DegMinSec, AnglePrecision::Use1Place, false, true,  true},
        { L"0^0'0\"",         0.0,                  AngleMode::DegMinSec, AnglePrecision::Use1Place, false, false, true},

        { L"00^00'30.0\"",    30 * SEC_TO_DEG,      AngleMode::DegMinSec, AnglePrecision::Use1Place, true,  true,  true},
        { L"00^00'30\"",      30 * SEC_TO_DEG,      AngleMode::DegMinSec, AnglePrecision::Use1Place, true,  false, true},
        { L"0^0'30.0\"",      30 * SEC_TO_DEG,      AngleMode::DegMinSec, AnglePrecision::Use1Place, false, true,  true},
        { L"0^0'30\"",        30 * SEC_TO_DEG,      AngleMode::DegMinSec, AnglePrecision::Use1Place, false, false, true},

        { L"00^00'30.0000\"", 30 * SEC_TO_DEG,      AngleMode::DegMinSec, AnglePrecision::Use4Places, true,  true,  true},
        { L"00^00'30\"",      30 * SEC_TO_DEG,      AngleMode::DegMinSec, AnglePrecision::Use4Places, true,  false, true},
        { L"0^0'30.0000\"",   30 * SEC_TO_DEG,      AngleMode::DegMinSec, AnglePrecision::Use4Places, false, true,  true},
        { L"0^0'30\"",        30 * SEC_TO_DEG,      AngleMode::DegMinSec, AnglePrecision::Use4Places, false, false, true},

        { L"35^00'30.0000\"", 35 + 30 * SEC_TO_DEG, AngleMode::DegMinSec, AnglePrecision::Use4Places, true,  true,  true},
        { L"35^00'30\"",      35 + 30 * SEC_TO_DEG, AngleMode::DegMinSec, AnglePrecision::Use4Places, true,  false, true},
        { L"35^0'30.0000\"",  35 + 30 * SEC_TO_DEG, AngleMode::DegMinSec, AnglePrecision::Use4Places, false, true,  true},
        { L"35^0'30\"",       35 + 30 * SEC_TO_DEG, AngleMode::DegMinSec, AnglePrecision::Use4Places, false, false, true},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        doFormatAngleTest (testDataArray[iTest]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
struct  DirectionFormatTestData
    {
    WCharCP       m_expectedString;
    double          m_inputValue;

    DirectionMode   m_dirMode;
    double          m_baseDir;
    bool            m_clockwise;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    doFormatDirectionTest (DirectionFormatTestData const& testData)
    {
    DirectionFormatterPtr  formatter = DirectionFormatter::Create();

    formatter->SetDirectionMode  (testData.m_dirMode);
    formatter->SetBaseDirection  (testData.m_baseDir);
    formatter->SetClockwise      (testData.m_clockwise);

    formatter->GetAngleFormatter().SetAngleMode      (AngleMode::DegMinSec);
    formatter->GetAngleFormatter().SetAnglePrecision (AnglePrecision::Whole);
    formatter->GetAngleFormatter().SetLeadingZero    (true);
    formatter->GetAngleFormatter().SetTrailingZeros  (true);

    WString outputStr = formatter->ToString (testData.m_inputValue);
    WString expectStr = replaceDegreeChar (testData.m_expectedString);

    ASSERT_STREQ (expectStr.c_str(), outputStr.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DirectionFormatter, FormatAzimuth_TestFullCircle1)
    {
    ScopedDgnHost autoDgnHost;
    DirectionFormatTestData testDataArray[] =
        {
        {  L"00^00'00\"",       0.0,   DirectionMode::Azimuth, 0.0, false},
        {  L"30^00'00\"",      30.0,   DirectionMode::Azimuth, 0.0, false},
        {  L"60^00'00\"",      60.0,   DirectionMode::Azimuth, 0.0, false},
        {  L"90^00'00\"",      90.0,   DirectionMode::Azimuth, 0.0, false},
        { L"120^00'00\"",     120.0,   DirectionMode::Azimuth, 0.0, false},
        { L"150^00'00\"",     150.0,   DirectionMode::Azimuth, 0.0, false},
        { L"180^00'00\"",     180.0,   DirectionMode::Azimuth, 0.0, false},
        { L"210^00'00\"",     210.0,   DirectionMode::Azimuth, 0.0, false},
        { L"240^00'00\"",     240.0,   DirectionMode::Azimuth, 0.0, false},
        { L"270^00'00\"",     270.0,   DirectionMode::Azimuth, 0.0, false},
        { L"300^00'00\"",     300.0,   DirectionMode::Azimuth, 0.0, false},
        { L"330^00'00\"",     330.0,   DirectionMode::Azimuth, 0.0, false},
        { L"360^00'00\"",     360.0,   DirectionMode::Azimuth, 0.0, false},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        doFormatDirectionTest (testDataArray[iTest]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DirectionFormatter, FormatAzimuth_TestFullCircle2)
    {
    ScopedDgnHost autoDgnHost;
    DirectionFormatTestData testDataArray[] =
        {
        {  L"90^00'00\"",       0.0,   DirectionMode::Azimuth, 90.0, true},
        {  L"60^00'00\"",      30.0,   DirectionMode::Azimuth, 90.0, true},
        {  L"30^00'00\"",      60.0,   DirectionMode::Azimuth, 90.0, true},
        {  L"00^00'00\"",      90.0,   DirectionMode::Azimuth, 90.0, true},
        { L"330^00'00\"",     120.0,   DirectionMode::Azimuth, 90.0, true},
        { L"300^00'00\"",     150.0,   DirectionMode::Azimuth, 90.0, true},
        { L"270^00'00\"",     180.0,   DirectionMode::Azimuth, 90.0, true},
        { L"240^00'00\"",     210.0,   DirectionMode::Azimuth, 90.0, true},
        { L"210^00'00\"",     240.0,   DirectionMode::Azimuth, 90.0, true},
        { L"180^00'00\"",     270.0,   DirectionMode::Azimuth, 90.0, true},
        { L"150^00'00\"",     300.0,   DirectionMode::Azimuth, 90.0, true},
        { L"120^00'00\"",     330.0,   DirectionMode::Azimuth, 90.0, true},
        {  L"90^00'00\"",     360.0,   DirectionMode::Azimuth, 90.0, true},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        doFormatDirectionTest (testDataArray[iTest]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DirectionFormatter, FormatBearing_TestFullCircle1)
    {
    ScopedDgnHost autoDgnHost;
    DirectionFormatTestData testDataArray[] =
        {
        { L"N90^00'00\"E",       0.0,   DirectionMode::Bearing, 0.0, false},
        { L"N60^00'00\"E",      30.0,   DirectionMode::Bearing, 0.0, false},
        { L"N30^00'00\"E",      60.0,   DirectionMode::Bearing, 0.0, false},
        { L"N00^00'00\"E",      90.0,   DirectionMode::Bearing, 0.0, false},
        { L"N30^00'00\"W",     120.0,   DirectionMode::Bearing, 0.0, false},
        { L"N60^00'00\"W",     150.0,   DirectionMode::Bearing, 0.0, false},
        { L"N90^00'00\"W",     180.0,   DirectionMode::Bearing, 0.0, false},
        { L"S60^00'00\"W",     210.0,   DirectionMode::Bearing, 0.0, false},
        { L"S30^00'00\"W",     240.0,   DirectionMode::Bearing, 0.0, false},
        { L"S00^00'00\"E",     270.0,   DirectionMode::Bearing, 0.0, false},
        { L"S30^00'00\"E",     300.0,   DirectionMode::Bearing, 0.0, false},
        { L"S60^00'00\"E",     330.0,   DirectionMode::Bearing, 0.0, false},
        { L"N90^00'00\"E",     360.0,   DirectionMode::Bearing, 0.0, false},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        doFormatDirectionTest (testDataArray[iTest]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DirectionFormatter, FormatBearing_TestFullCircle2)
    {
    ScopedDgnHost autoDgnHost;
    DirectionFormatTestData testDataArray[] =
        {
        { L"N90^00'00\"E",       0.0,   DirectionMode::Bearing, 90.0, true},
        { L"N60^00'00\"E",      30.0,   DirectionMode::Bearing, 90.0, true},
        { L"N30^00'00\"E",      60.0,   DirectionMode::Bearing, 90.0, true},
        { L"N00^00'00\"E",      90.0,   DirectionMode::Bearing, 90.0, true},
        { L"N30^00'00\"W",     120.0,   DirectionMode::Bearing, 90.0, true},
        { L"N60^00'00\"W",     150.0,   DirectionMode::Bearing, 90.0, true},
        { L"N90^00'00\"W",     180.0,   DirectionMode::Bearing, 90.0, true},
        { L"S60^00'00\"W",     210.0,   DirectionMode::Bearing, 90.0, true},
        { L"S30^00'00\"W",     240.0,   DirectionMode::Bearing, 90.0, true},
        { L"S00^00'00\"E",     270.0,   DirectionMode::Bearing, 90.0, true},
        { L"S30^00'00\"E",     300.0,   DirectionMode::Bearing, 90.0, true},
        { L"S60^00'00\"E",     330.0,   DirectionMode::Bearing, 90.0, true},
        { L"N90^00'00\"E",     360.0,   DirectionMode::Bearing, 90.0, true},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        doFormatDirectionTest (testDataArray[iTest]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   JoeZbuchalski    03/10
+---------------+---------------+---------------+---------------+---------------+------*/
struct  DistanceFormatTestDataBase
    {
    WCharCP         m_expectedString;
    PrecisionFormat m_precision;
    bool            m_unitFlag;
    bool            m_leadingZero;
    bool            m_trailingZero;
    bool            m_insertThousandsSeparator;
    WChar           m_thousandsSeparator;
    WChar           m_decimalSeparator;
    bool            m_suppressZeroMasterUnits;
    bool            m_suppressZeroSubUnits;
    double          m_scaleFactor;
    DgnUnitFormat   m_unitFormat;
    UnitDefinition  m_masterUnits;
    UnitDefinition  m_subUnits;
    UnitDefinition  m_storageUnits;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   JoeZbuchalski    03/10
+---------------+---------------+---------------+---------------+---------------+------*/
struct  DistanceFormatTestData
    {
    double  m_inputValue;
    DistanceFormatTestDataBase m_base;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   JoeZbuchalski    03/10
+---------------+---------------+---------------+---------------+---------------+------*/
void    doFormatDistanceTest (DistanceFormatTestData const& testData)
    {
#if defined (NOT_NOW)
    static int s_count = 0;
    printf ("%d\n", s_count++);
#endif

    DistanceFormatterPtr  formatter = DistanceFormatter::Create();

    formatter->SetUnits(testData.m_base.m_masterUnits, &testData.m_base.m_subUnits);
    formatter->SetUnitFormat (testData.m_base.m_unitFormat);
    formatter->SetPrecision (testData.m_base.m_precision);
    formatter->SetUnitLabelFlag (testData.m_base.m_unitFlag);
    formatter->SetLeadingZero (testData.m_base.m_leadingZero);
    formatter->SetTrailingZeros (testData.m_base.m_trailingZero);
    formatter->SetInsertThousandsSeparator (testData.m_base.m_insertThousandsSeparator);
    formatter->SetThousandsSeparator (testData.m_base.m_thousandsSeparator);
    formatter->SetDecimalSeparator (testData.m_base.m_decimalSeparator);
    formatter->SetSuppressZeroMasterUnits (testData.m_base.m_suppressZeroMasterUnits);
    formatter->SetSuppressZeroSubUnits (testData.m_base.m_suppressZeroSubUnits);
    formatter->SetScaleFactor (testData.m_base.m_scaleFactor);
    WString outputStr = formatter->ToString(testData.m_inputValue / 1000.); // input is in millimters, storage units are meters

    ASSERT_STREQ (testData.m_base.m_expectedString, outputStr.c_str());
    }
#define MM_TO_METERS(a) (a/1000.)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   JoeZbuchalski    03/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DistanceFormatter, Format_TestGeneral)
    {
    ScopedDgnHost autoDgnHost;
    UnitDefinition masterUnits = UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters);
    UnitDefinition subUnits = UnitDefinition::GetStandardUnit (StandardUnit::MetricMillimeters);
    UnitDefinition storageUnits = UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters);

    DistanceFormatTestData testDataArray[] =
        {
        //millimeters           value               precision                       unitflag    leadingzero     trailingzero    insertthousands     thousandssep    decimalsep    suppresszeromaster  suppresszerosub   scalefactor format                master        sub         storage     
        { .1,          L"0.000",           PrecisionFormat::Decimal3Places,      false,      true,           true,           false,              ',',            '.',          false,              false,            0.0,        DgnUnitFormat::MU,       masterUnits,  subUnits,   storageUnits },
        { 100,         L"0.100",           PrecisionFormat::Decimal3Places,      false,      true,           true,           false,              ',',            '.',          false,              false,            0.0,        DgnUnitFormat::MU,       masterUnits,  subUnits,   storageUnits },
        { 1000,        L"1.000",           PrecisionFormat::Decimal3Places,      false,      true,           true,           false,              ',',            '.',          false,              false,            0.0,        DgnUnitFormat::MU,       masterUnits,  subUnits,   storageUnits },
        { 10000,       L"10.000",          PrecisionFormat::Decimal3Places,      false,      true,           true,           false,              ',',            '.',          false,              false,            0.0,        DgnUnitFormat::MU,       masterUnits,  subUnits,   storageUnits },
        { .1,          L"0.100",           PrecisionFormat::Decimal3Places,      false,      true,           true,           false,              ',',            '.',          false,              false,            0.0,        DgnUnitFormat::SU,       masterUnits,  subUnits,   storageUnits },
        { 100,         L"100.000",         PrecisionFormat::Decimal3Places,      false,      true,           true,           false,              ',',            '.',          false,              false,            0.0,        DgnUnitFormat::SU,       masterUnits,  subUnits,   storageUnits },
        { 1000,        L"1000.000",        PrecisionFormat::Decimal3Places,      false,      true,           true,           false,              ',',            '.',          false,              false,            0.0,        DgnUnitFormat::SU,       masterUnits,  subUnits,   storageUnits },
        { 10000,       L"10000.000",       PrecisionFormat::Decimal3Places,      false,      true,           true,           false,              ',',            '.',          false,              false,            0.0,        DgnUnitFormat::SU,       masterUnits,  subUnits,   storageUnits },
        { .1,          L"0:0.100",         PrecisionFormat::Decimal3Places,      false,      true,           true,           false,              ',',            '.',          false,              false,            0.0,        DgnUnitFormat::MUSU,     masterUnits,  subUnits,   storageUnits },
        { 100,         L"0:100.000",       PrecisionFormat::Decimal3Places,      false,      true,           true,           false,              ',',            '.',          false,              false,            0.0,        DgnUnitFormat::MUSU,     masterUnits,  subUnits,   storageUnits },
        { 1000,        L"1:0.000",         PrecisionFormat::Decimal3Places,      false,      true,           true,           false,              ',',            '.',          false,              false,            0.0,        DgnUnitFormat::MUSU,     masterUnits,  subUnits,   storageUnits },
        { 10000,       L"10:0.000",        PrecisionFormat::Decimal3Places,      false,      true,           true,           false,              ',',            '.',          false,              false,            0.0,        DgnUnitFormat::MUSU,     masterUnits,  subUnits,   storageUnits },
        };

    for (int iTest = 0; iTest < _countof(testDataArray); iTest++)
        doFormatDistanceTest (testDataArray[iTest]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   JoeZbuchalski    03/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DistanceFormatter, Format_TestUnitLabelFlag)
    {
    ScopedDgnHost autoDgnHost;
    UnitDefinition masterUnits = UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters);
    UnitDefinition subUnits = UnitDefinition::GetStandardUnit (StandardUnit::MetricMillimeters);
    UnitDefinition storageUnits = UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters);

    DistanceFormatTestData testDataArray[] =
        {
        //millimeters           value                                    precision                         unitflag  leadingzero  trailingzero  insertthousands  thousandssep  decimalsep  suppresszeromaster  suppresszerosub  scalefactor  format                master        sub         storage     
        { 1000,        L"1.000" LOCS(L"m"),                     PrecisionFormat::Decimal3Places,  true,     true,        true,         false,           ',',          '.',        false,              false,           0.0,         DgnUnitFormat::MU,       masterUnits,  subUnits,   storageUnits },
        { 1000,        L"1000.000" LOCS(L"mm"),                 PrecisionFormat::Decimal3Places,  true,     true,        true,         false,           ',',          '.',        false,              false,           0.0,         DgnUnitFormat::SU,       masterUnits,  subUnits,   storageUnits },
        { 1000,        L"1" LOCS(L"m") L" 0.000" LOCS(L"mm"),   PrecisionFormat::Decimal3Places,  true,     true,        true,         false,           ',',          '.',        false,              false,           0.0,         DgnUnitFormat::MUSU,     masterUnits,  subUnits,   storageUnits },
        };

    for (int iTest = 0; iTest < _countof(testDataArray); iTest++)
        doFormatDistanceTest (testDataArray[iTest]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   JoeZbuchalski    03/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DistanceFormatter, Format_TestPrecision)
    {
    ScopedDgnHost autoDgnHost;
    UnitDefinition masterUnits = UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters);
    UnitDefinition subUnits = UnitDefinition::GetStandardUnit (StandardUnit::MetricMillimeters);
    UnitDefinition storageUnits = UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters);

    DistanceFormatTestData testDataArray[] =
        {
        //meters           value                      precision                         unitflag    leadingzero trailingzero  insertthousands  thousandssep  decimalseparator  suppresszeromaster  suppresszerosub  scalefactor format                master        sub         storage         
        { .1,          L"0.00",                   PrecisionFormat::Decimal2Places,  false,      true,       true,         false,           ',',          '.',              false,              false,           0.0,        DgnUnitFormat::MU,       masterUnits,  subUnits,   storageUnits },
        { 100,         L"0.10",                   PrecisionFormat::Decimal2Places,  false,      true,       true,         false,           ',',          '.',              false,              false,           0.0,        DgnUnitFormat::MU,       masterUnits,  subUnits,   storageUnits },
        { 1000,        L"1.0",                    PrecisionFormat::Decimal1Place,   false,      true,       true,         false,           ',',          '.',              false,              false,           0.0,        DgnUnitFormat::MU,       masterUnits,  subUnits,   storageUnits },
        { 10000,       L"10.0",                   PrecisionFormat::Decimal1Place,   false,      true,       true,         false,           ',',          '.',              false,              false,           0.0,        DgnUnitFormat::MU,       masterUnits,  subUnits,   storageUnits },
        { 10000,       L"10",                     PrecisionFormat::DecimalWhole,    false,      true,       true,         false,           ',',          '.',              false,              false,           0.0,        DgnUnitFormat::MU,       masterUnits,  subUnits,   storageUnits },
        { 10000,       L"10.0000",                PrecisionFormat::Decimal4Places,  false,      true,       true,         false,           ',',          '.',              false,              false,           0.0,        DgnUnitFormat::MU,       masterUnits,  subUnits,   storageUnits },
        { 1000,        L"1000.00000" LOCS(L"mm"), PrecisionFormat::Decimal5Places,  true,       true,       true,         false,           ',',          '.',              false,              false,           0.0,        DgnUnitFormat::SU,       masterUnits,  subUnits,   storageUnits },
        { 1000,        L"1.0",                    PrecisionFormat::Decimal1Place,   false,      true,       true,         false,           ',',          '.',              false,              false,           0.0,        DgnUnitFormat::MU,       masterUnits,  subUnits,   storageUnits }, 
        { 1020,        L"1.0",                    PrecisionFormat::Decimal1Place,   false,      true,       true,         false,           ',',          '.',              false,              false,           0.0,        DgnUnitFormat::MU,       masterUnits,  subUnits,   storageUnits }, 
        { 1040,        L"1.0",                    PrecisionFormat::Decimal1Place,   false,      true,       true,         false,           ',',          '.',              false,              false,           0.0,        DgnUnitFormat::MU,       masterUnits,  subUnits,   storageUnits }, 
        { 1049.9,      L"1.0",                    PrecisionFormat::Decimal1Place,   false,      true,       true,         false,           ',',          '.',              false,              false,           0.0,        DgnUnitFormat::MU,       masterUnits,  subUnits,   storageUnits }, 
        { 1050,        L"1.1",                    PrecisionFormat::Decimal1Place,   false,      true,       true,         false,           ',',          '.',              false,              false,           0.0,        DgnUnitFormat::MU,       masterUnits,  subUnits,   storageUnits }, 
        { 1050.1,      L"1.1",                    PrecisionFormat::Decimal1Place,   false,      true,       true,         false,           ',',          '.',              false,              false,           0.0,        DgnUnitFormat::MU,       masterUnits,  subUnits,   storageUnits }, 
        { 1060,        L"1.1",                    PrecisionFormat::Decimal1Place,   false,      true,       true,         false,           ',',          '.',              false,              false,           0.0,        DgnUnitFormat::MU,       masterUnits,  subUnits,   storageUnits }, 
        { 1070,        L"1.1",                    PrecisionFormat::Decimal1Place,   false,      true,       true,         false,           ',',          '.',              false,              false,           0.0,        DgnUnitFormat::MU,       masterUnits,  subUnits,   storageUnits }, 
        { 1080,        L"1.1",                    PrecisionFormat::Decimal1Place,   false,      true,       true,         false,           ',',          '.',              false,              false,           0.0,        DgnUnitFormat::MU,       masterUnits,  subUnits,   storageUnits }, 
        };

    for (int iTest = 0; iTest < _countof(testDataArray); iTest++)
        doFormatDistanceTest (testDataArray[iTest]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   JoeZbuchalski    03/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DistanceFormatter, Format_TestLeadingTrailingZeros)
    {
    ScopedDgnHost autoDgnHost;
    UnitDefinition masterUnits = UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters);
    UnitDefinition subUnits = UnitDefinition::GetStandardUnit (StandardUnit::MetricMillimeters);
    UnitDefinition storageUnits = UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters);

    DistanceFormatTestData testDataArray[] =
        {
        //uor           value            precision                         unitflag    leadingzero trailingzero    insertthousands  thousandssep  decimalsep  suppresszeromaster  suppresszerosub  scalefactor format                master        sub         storage
        { 10000,       L"10.0",         PrecisionFormat::Decimal1Place,   false,      true,       true,           false,           ',',          '.',        false,              false,           0.0,        DgnUnitFormat::MU,       masterUnits,  subUnits,   storageUnits },
        { 10000,       L"10",           PrecisionFormat::Decimal1Place,   false,      true,       false,          false,           ',',          '.',        false,              false,           0.0,        DgnUnitFormat::MU,       masterUnits,  subUnits,   storageUnits },
        };

    for (int iTest = 0; iTest < _countof(testDataArray); iTest++)
        doFormatDistanceTest (testDataArray[iTest]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   JoeZbuchalski    03/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DistanceFormatter, Format_TestThousandsDecimalSeparator)
    {
    ScopedDgnHost autoDgnHost;
    UnitDefinition masterUnits = UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters);
    UnitDefinition subUnits = UnitDefinition::GetStandardUnit (StandardUnit::MetricMillimeters);
    UnitDefinition storageUnits = UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters);

    DistanceFormatTestData testDataArray[] =
        {
        //uor           value                                       precision                         unitflag    leadingzero trailingzero  insertthousands  thousandssep  decimalsep  suppresszeromaster  suppresszerosub  scalefactor format             master        sub         storage   
        { 1000000,     L"1000.000" LOCS(L"m"),                     PrecisionFormat::Decimal3Places,  true,       true,       true,         false,           ',',          '.',        false,              false,           0.0,        DgnUnitFormat::MU,    masterUnits,  subUnits,   storageUnits },
        { 1000000,     L"1,000.000" LOCS(L"m"),                    PrecisionFormat::Decimal3Places,  true,       true,       true,         true,            ',',          '.',        false,              false,           0.0,        DgnUnitFormat::MU,    masterUnits,  subUnits,   storageUnits },
        { 1000000,     L"1`000^000" LOCS(L"m"),                    PrecisionFormat::Decimal3Places,  true,       true,       true,         true,            '`',          '^',        false,              false,           0.0,        DgnUnitFormat::MU,    masterUnits,  subUnits,   storageUnits },
        { 1000,        L"1,000.000" LOCS(L"mm"),                   PrecisionFormat::Decimal3Places,  true,       true,       true,         true,            ',',          '.',        false,              false,           0.0,        DgnUnitFormat::SU,    masterUnits,  subUnits,   storageUnits },
        { 1000,        L"1`000^000" LOCS(L"mm"),                   PrecisionFormat::Decimal3Places,  true,       true,       true,         true,            '`',          '^',        false,              false,           0.0,        DgnUnitFormat::SU,    masterUnits,  subUnits,   storageUnits },
        { 1000000,     L"1000" LOCS(L"m") L" 0.000"  LOCS(L"mm"),  PrecisionFormat::Decimal3Places,  true,       true,       true,         false,           ',',          '.',        false,              false,           0.0,        DgnUnitFormat::MUSU,  masterUnits,  subUnits,   storageUnits },
        { 1000000,     L"1,000" LOCS(L"m") L" 0.000" LOCS(L"mm"),  PrecisionFormat::Decimal3Places,  true,       true,       true,         true,            ',',          '.',        false,              false,           0.0,        DgnUnitFormat::MUSU,  masterUnits,  subUnits,   storageUnits },
        };
        
    for (int iTest = 0; iTest < _countof(testDataArray); iTest++)
        doFormatDistanceTest (testDataArray[iTest]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   JoeZbuchalski    03/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DistanceFormatter, Format_TestSuppressZeroMasterSubUnits)
    {
    ScopedDgnHost autoDgnHost;
    UnitDefinition masterUnits = UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters);
    UnitDefinition subUnits = UnitDefinition::GetStandardUnit (StandardUnit::MetricMillimeters);
    UnitDefinition storageUnits = UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters);

    DistanceFormatTestData testDataArray[] =
        {
        // uor          value                    precision                         unitflag  leadingzero trailingzero    insertthousands  thousandssep  decimalsep  suppresszeromaster  suppresszerosub  scalefactor format                master        sub         storage   
        { 100,         L":100.000",             PrecisionFormat::Decimal3Places,  false,    true,       true,           false,           ',',          '.',        true,               false,           0.0,        DgnUnitFormat::MUSU,     masterUnits,  subUnits,   storageUnits },
        { 100,         L" 100.000" LOCS(L"mm"), PrecisionFormat::Decimal3Places,  true,     true,       true,           false,           ',',          '.',        true,               false,           0.0,        DgnUnitFormat::MUSU,     masterUnits,  subUnits,   storageUnits },
        { 1000,        L"1:",                   PrecisionFormat::Decimal3Places,  false,    true,       true,           false,           ',',          '.',        false,              true,            0.0,        DgnUnitFormat::MUSU,     masterUnits,  subUnits,   storageUnits },
        { 1000,        L"1" LOCS(L"m") L" ",    PrecisionFormat::Decimal3Places,  true,     true,       true,           false,           ',',          '.',        false,              true,            0.0,        DgnUnitFormat::MUSU,     masterUnits,  subUnits,   storageUnits },
        };

    for (int iTest = 0; iTest < _countof(testDataArray); iTest++)
        doFormatDistanceTest (testDataArray[iTest]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   JoeZbuchalski    03/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DistanceFormatter, Format_TestScientific)
    {
    ScopedDgnHost autoDgnHost;
    UnitDefinition masterUnits = UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters);
    UnitDefinition subUnits = UnitDefinition::GetStandardUnit (StandardUnit::MetricMillimeters);
    UnitDefinition storageUnits = UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters);

    DistanceFormatTestData testDataArray[] =
        {
        //uor           value                                    precision                            unitflag  leadingzero trailingzero  insertthousands  thousandssep  decimalsep  suppresszeromaster  suppresszerosub  scalefactor format             master        sub         storage   
        { 100000,      L"1.000E+02" LOCS(L"m") ,                PrecisionFormat::Scientific3Places,  true,     true,       true,         true,            ',',          '.',        false,              false,           0.0,        DgnUnitFormat::MU,    masterUnits,  subUnits,   storageUnits },
        { 100000,      L"1.000E+05" LOCS(L"mm"),                PrecisionFormat::Scientific3Places,  true,     true,       true,         true,            ',',          '.',        false,              false,           0.0,        DgnUnitFormat::SU,    masterUnits,  subUnits,   storageUnits },
        { 100000,      L"100" LOCS(L"m") L" 0.000" LOCS(L"mm"), PrecisionFormat::Scientific3Places,  true,     true,       true,         false,           ',',          '.',        false,              false,           0.0,        DgnUnitFormat::MUSU,  masterUnits,  subUnits,   storageUnits },
        { 100000,      L"100:0.000",                            PrecisionFormat::Decimal3Places,     false,    true,       true,         false,           ',',          '.',        false,              false,           0.0,        DgnUnitFormat::MUSU,  masterUnits,  subUnits,   storageUnits },
        };

    for (int iTest = 0; iTest < _countof(testDataArray); iTest++)
        doFormatDistanceTest (testDataArray[iTest]);
    }

 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   JoeZbuchalski    03/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DistanceFormatter, Format_TestFractions)
    {
    ScopedDgnHost autoDgnHost;
    UnitDefinition masterUnits = UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters);
    UnitDefinition subUnits = UnitDefinition::GetStandardUnit (StandardUnit::MetricMillimeters);
    UnitDefinition storageUnits = UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters);

    DistanceFormatTestData testDataArray[] =
        {
        //uor           value                                       precision                            unitflag  leadingzero trailingzero  insertthousands  thousandssep  decimalsep  suppresszeromaster  suppresszerosub  scalefactor format            master        sub         storage   
        { 1000,        L"1",                                       PrecisionFormat::FractionalWhole,    false,    true,       true,         false,           ',',          '.',        false,              false,           0.0,        DgnUnitFormat::MU,   masterUnits,  subUnits,   storageUnits },
        { 1500,        L"1 1/2",                                   PrecisionFormat::FractionalHalf,     false,    true,       true,         false,           ',',          '.',        false,              false,           0.0,        DgnUnitFormat::MU,   masterUnits,  subUnits,   storageUnits },
        { 1500,        L"1 1/2",                                   PrecisionFormat::FractionalQuarter,  false,    true,       true,         false,           ',',          '.',        false,              false,           0.0,        DgnUnitFormat::MU,   masterUnits,  subUnits,   storageUnits },
        { 1750,        L"2",                                       PrecisionFormat::FractionalHalf,     false,    true,       true,         false,           ',',          '.',        false,              false,           0.0,        DgnUnitFormat::MU,   masterUnits,  subUnits,   storageUnits },
        { 10555.5,     L"10" LOCS(L"m") L" 555 1/2" LOCS(L"mm"),   PrecisionFormat::FractionalQuarter,  true,     true,       true,         false,           ',',          '.',        false,              false,           0.0,        DgnUnitFormat::MUSU, masterUnits,  subUnits,   storageUnits },
        };

    for (int iTest = 0; iTest < _countof(testDataArray); iTest++)
        doFormatDistanceTest (testDataArray[iTest]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   JoeZbuchalski    03/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DistanceFormatter, Format_TestScaleFactor)
    {
    ScopedDgnHost autoDgnHost;
    UnitDefinition masterUnits = UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters);
    UnitDefinition subUnits = UnitDefinition::GetStandardUnit (StandardUnit::MetricMillimeters);
    UnitDefinition storageUnits = UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters);

    DistanceFormatTestData testDataArray[] =
        {
        //uor           value                   precision                         unitflag  leadingzero trailingzero  insertthousands  thousandssep  decimalsep  suppresszeromaster  suppresszerosub  scalefactor format                master        sub         storage   
        { 100,         L"100.000",             PrecisionFormat::Decimal3Places,  false,    true,       true,         false,           ',',          '.',        false,              false,           1.0,        DgnUnitFormat::SU,       masterUnits,  subUnits,   storageUnits },
        { 100,         L"200.000",             PrecisionFormat::Decimal3Places,  false,    true,       true,         false,           ',',          '.',        false,              false,           2.0,        DgnUnitFormat::SU,       masterUnits,  subUnits,   storageUnits },
        { 100,         L"50.000",              PrecisionFormat::Decimal3Places,  false,    true,       true,         false,           ',',          '.',        false,              false,           0.5,        DgnUnitFormat::SU,       masterUnits,  subUnits,   storageUnits },
        { 1564555.4,   L"3129.111" LOCS(L"m"), PrecisionFormat::Decimal3Places,  true,     true,       true,         false,           ',',          '.',        false,              false,           2,          DgnUnitFormat::MU,       masterUnits,  subUnits,   storageUnits },
        };

    for (int iTest = 0; iTest < _countof(testDataArray); iTest++)
        doFormatDistanceTest (testDataArray[iTest]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   JoeZbuchalski    03/10
+---------------+---------------+---------------+---------------+---------------+------*/
struct PointFormatTestData
   {
   DPoint3d                     m_point;
   DistanceFormatTestDataBase   m_base;
   };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   JoeZbuchalski    03/10
+---------------+---------------+---------------+---------------+---------------+------*/
void    doFormatPointTest (PointFormatTestData const& testData)
    {
#if defined (NOT_NOW)
    static int s_count = 0;
    printf ("%d\n", s_count++);
#endif

    PointFormatterPtr  formatter = PointFormatter::Create();

    formatter->GetDistanceFormatter().SetUnits(testData.m_base.m_masterUnits, &testData.m_base.m_subUnits);
    formatter->GetDistanceFormatter().SetUnitFormat (testData.m_base.m_unitFormat);
    formatter->GetDistanceFormatter().SetPrecision (testData.m_base.m_precision);
    formatter->GetDistanceFormatter().SetUnitLabelFlag (testData.m_base.m_unitFlag);
    formatter->GetDistanceFormatter().SetLeadingZero (testData.m_base.m_leadingZero);
    formatter->GetDistanceFormatter().SetTrailingZeros (testData.m_base.m_trailingZero);
    formatter->GetDistanceFormatter().SetInsertThousandsSeparator (testData.m_base.m_insertThousandsSeparator);
    formatter->GetDistanceFormatter().SetThousandsSeparator (testData.m_base.m_thousandsSeparator);
    formatter->GetDistanceFormatter().SetDecimalSeparator (testData.m_base.m_decimalSeparator);
    formatter->GetDistanceFormatter().SetSuppressZeroMasterUnits (testData.m_base.m_suppressZeroMasterUnits);
    formatter->GetDistanceFormatter().SetSuppressZeroSubUnits (testData.m_base.m_suppressZeroSubUnits);
    formatter->GetDistanceFormatter().SetScaleFactor (testData.m_base.m_scaleFactor);

    formatter->SetIs3d (false);

    DPoint3d meters = DPoint3d::FromScale(testData.m_point, 1./1000.);
    WString outputStr = formatter->ToString(meters);

    ASSERT_STREQ (testData.m_base.m_expectedString, outputStr.c_str());
    }

 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   JoeZbuchalski    03/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (PointFormatter, Format_TestGeneral)
    {
    ScopedDgnHost autoDgnHost;
    UnitDefinition masterUnits = UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters);
    UnitDefinition subUnits = UnitDefinition::GetStandardUnit (StandardUnit::MetricMillimeters);
    UnitDefinition storageUnits = UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters);

    PointFormatTestData testDataArray[] =
        {
        //point             value             precision                         unitflag  leadingzero  trailingzero  insertthousands  thousandssep  decimalsep  suppresszeromaster  suppresszerosub  scalefactor format            masterunits   subunits    storageunis   
        { {0,0,0},          L"0, 0",        PrecisionFormat::DecimalWhole,    false,    true,        true,         false,           ',',          '.',        false,              false,           0.0,        DgnUnitFormat::MU,   masterUnits,  subUnits,   storageUnits },	
        { {100,500,0},    L"0.100, 0.500",  PrecisionFormat::Decimal3Places,  false,    true,        true,         false,           ',',          '.',        false,              false,           0.0,        DgnUnitFormat::MU,   masterUnits,  subUnits,   storageUnits },	
        };

    for (int iTest = 0; iTest < _countof(testDataArray); iTest++)
        doFormatPointTest (testDataArray[iTest]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct          AreaOrVolumeFormatTestData
    {
    WCharCP         m_expectedString;
    double          m_inputValue;

    PrecisionFormat m_precision;
    bool            m_showUnitLabel;
    bool            m_leadingZero;
    bool            m_trailingZero;
    bool            m_insertThousandsSeparator;
    WChar           m_thousandsSeparator;
    WChar           m_decimalSeparator;
    double          m_scaleFactor;
    UnitDefinition  m_masterUnits;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void    doFormatAreaTest (AreaOrVolumeFormatTestData const& testData)
    {
    AreaFormatterPtr  formatter = AreaFormatter::Create();

    formatter->SetPrecision                     (testData.m_precision);
    formatter->SetShowUnitLabel                 (testData.m_showUnitLabel);
    formatter->SetLeadingZero                   (testData.m_leadingZero);
    formatter->SetTrailingZeros                 (testData.m_trailingZero);
    formatter->SetInsertThousandsSeparator      (testData.m_insertThousandsSeparator);
    formatter->SetThousandsSeparator            (testData.m_thousandsSeparator);
    formatter->SetDecimalSeparator              (testData.m_decimalSeparator);
    formatter->SetScaleFactor                   (testData.m_scaleFactor);
    formatter->SetMasterUnit                    (testData.m_masterUnits);
    WString outputStr = formatter->ToString (testData.m_inputValue / (1000. * 1000.));

    ASSERT_STREQ (testData.m_expectedString, outputStr.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (AreaFormatter, Format_TestGeneral)
    {
    ScopedDgnHost autoDgnHost;
    UnitDefinition masterUnits  = UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters);
    UnitDefinition storageUnits = UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters);

    AreaOrVolumeFormatTestData testDataArray[] =
        {
        //expected              value             precision                         showLabel leadingzero  trailingzero  insertthousands  thousandssep  decimalsep  scalefactor masterunits    
        { L"1",                 1E6,              PrecisionFormat::DecimalWhole,    false,    true,        true,         false,           ',',          '.',        1.0,        masterUnits },
        { L"1" LOCS(L"m2"),     1E6,              PrecisionFormat::DecimalWhole,    true,     true,        true,         false,           ',',          '.',        1.0,        masterUnits },
        { L"1.000",             1E6,              PrecisionFormat::Decimal3Places,  false,    true,        true,         false,           ',',          '.',        1.0,        masterUnits },
        { L"1.000" LOCS(L"m2"), 1E6,              PrecisionFormat::Decimal3Places,  true,     true,        true,         false,           ',',          '.',        1.0,        masterUnits },
        { L"25",                25E6,             PrecisionFormat::DecimalWhole,    false,    true,        true,         false,           ',',          '.',        1.0,        masterUnits },
        { L"25.000",            25E6,             PrecisionFormat::Decimal3Places,  false,    true,        true,         false,           ',',          '.',        1.0,        masterUnits },
        };

    for (int iTest = 0; iTest < _countof(testDataArray); iTest++)
        doFormatAreaTest (testDataArray[iTest]);
    }

 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (AreaFormatter, Format_TestUnitsAndScales)
    {
    ScopedDgnHost autoDgnHost;
    UnitDefinition meters       = UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters);
    UnitDefinition millimeters  = UnitDefinition::GetStandardUnit (StandardUnit::MetricMillimeters);
    UnitDefinition feet         = UnitDefinition::GetStandardUnit (StandardUnit::EnglishFeet);
    UnitDefinition inches       = UnitDefinition::GetStandardUnit (StandardUnit::EnglishInches);

    double oneFoot = 12.*25.4;
    double squareFoot = oneFoot*oneFoot;
    
    AreaOrVolumeFormatTestData testDataArray[] =
        {
        //expected          value             precision                        showLabel leadingzero  trailingzero  insertthousands  thousandssep  decimalsep  scalefactor masterunits   storageunits    uorperstorage
        /* A Cube 2m x 2m = 4m2 = 2000mm * 2000mm = 4E6mm2 */
        { L"4000000.0",     4E6,            PrecisionFormat::Decimal1Place,  false,    true,        true,         false,           ',',          '.',        1.0,        millimeters},
        { L"4.0",           4E6,            PrecisionFormat::Decimal1Place,  false,    true,        true,         false,           ',',          '.',        1.0,        meters    },
        { L"144.0",         squareFoot,     PrecisionFormat::Decimal1Place,  false,    true,        true,         false,           ',',          '.',        1.0,        inches    },
        { L"1.0",           squareFoot,     PrecisionFormat::Decimal1Place,  false,    true,        true,         false,           ',',          '.',        1.0,        feet      },
        };

    for (int iTest = 0; iTest < _countof(testDataArray); iTest++)
        doFormatAreaTest (testDataArray[iTest]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void    doFormatVolumeTest (AreaOrVolumeFormatTestData const& testData)
    {
    VolumeFormatterPtr  formatter = VolumeFormatter::Create();

    formatter->SetPrecision                     (testData.m_precision);
    formatter->SetShowUnitLabel                 (testData.m_showUnitLabel);
    formatter->SetLeadingZero                   (testData.m_leadingZero);
    formatter->SetTrailingZeros                 (testData.m_trailingZero);
    formatter->SetInsertThousandsSeparator      (testData.m_insertThousandsSeparator);
    formatter->SetThousandsSeparator            (testData.m_thousandsSeparator);
    formatter->SetDecimalSeparator              (testData.m_decimalSeparator);
    formatter->SetScaleFactor                   (testData.m_scaleFactor);
    formatter->SetMasterUnit                    (testData.m_masterUnits);
    WString outputStr = formatter->ToString (testData.m_inputValue / (1000.*1000.*1000.));

    printf ("%ls\n", outputStr.c_str());

    ASSERT_STREQ (testData.m_expectedString, outputStr.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (VolumeFormatter, Format_TestGeneral)
    {
    ScopedDgnHost autoDgnHost;
    UnitDefinition masterUnits  = UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters);

    AreaOrVolumeFormatTestData testDataArray[] =
        {
        //expected               value              precision                         showLabel leadingzero  trailingzero  insertthousands  thousandssep  decimalsep  scalefactor masterunits   storageunits
        { L"1",                  1E9,              PrecisionFormat::DecimalWhole,    false,    true,        true,         false,           ',',          '.',        1.0,        masterUnits},
        { L"1" LOCS(L"m3"),      1E9,              PrecisionFormat::DecimalWhole,    true,     true,        true,         false,           ',',          '.',        1.0,        masterUnits},
        { L"1.000",              1E9,              PrecisionFormat::Decimal3Places,  false,    true,        true,         false,           ',',          '.',        1.0,        masterUnits},
        { L"1.000" LOCS(L"m3"),  1E9,              PrecisionFormat::Decimal3Places,  true,     true,        true,         false,           ',',          '.',        1.0,        masterUnits},
        { L"25",                 25E9,             PrecisionFormat::DecimalWhole,    false,    true,        true,         false,           ',',          '.',        1.0,        masterUnits},
        { L"25.000",             25E9,             PrecisionFormat::Decimal3Places,  false,    true,        true,         false,           ',',          '.',        1.0,        masterUnits},
        };

    for (int iTest = 0; iTest < _countof(testDataArray); iTest++)
        doFormatVolumeTest (testDataArray[iTest]);
    }

 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (VolumeFormatter, Format_TestUnitsAndScales)
    {
    ScopedDgnHost autoDgnHost;
    UnitDefinition meters       = UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters);
    UnitDefinition millimeters  = UnitDefinition::GetStandardUnit (StandardUnit::MetricMillimeters);
    UnitDefinition feet         = UnitDefinition::GetStandardUnit (StandardUnit::EnglishFeet);
    UnitDefinition inches       = UnitDefinition::GetStandardUnit (StandardUnit::EnglishInches);

    double oneFoot = 12.*25.4;
    double cubeFoot = oneFoot*oneFoot*oneFoot;

    AreaOrVolumeFormatTestData testDataArray[] =
        {
        //expected          value             precision                         showLabel leadingzero  trailingzero  insertthousands  thousandssep  decimalsep  scalefactor masterunits   storageunits  
        /* A Cube 20000 uor x 20000 uor x 20000 uor = 8E12 uor3 = 200mm x 200mm x 200mm = 8E6mm3 =  0.2m x 0.2m x 0.2m = 8E-3 m3*/
        { L"8,000,000.0",   8E6,             PrecisionFormat::Decimal1Place,   false,    true,        true,         true,            ',',          '.',        1.0,        millimeters},
        { L"0.008",         8E6,             PrecisionFormat::Decimal3Places,  false,    true,        true,         true,            ',',          '.',        1.0,        meters    },
        /* Under an ACS with scale 2, the same cube is 100mm x 100mm x 100mm = 1E6mm3 */
        { L"1,000,000.0",   8E6,             PrecisionFormat::Decimal1Place,   false,    true,        true,         true,            ',',          '.',        2.0,        millimeters},
        { L"0.001",         8E6,             PrecisionFormat::Decimal3Places,  false,    true,        true,         true,            ',',          '.',        2.0,        meters    },
        /* A Cube 1 ft x 1 ft x 1 ft = 12 in x 12 in x 12 in = 1728 in3 */
        { L"1,728.0",       cubeFoot,        PrecisionFormat::Decimal1Place,   false,    true,        true,         true,            ',',          '.',        1.0,        inches       },
        { L"1.0",           cubeFoot,        PrecisionFormat::Decimal1Place,   false,    true,        true,         true,            ',',          '.',        1.0,        feet         },
        /* Under an ACS with scale 2, the same cube is 0.5 ft x 0.5 ft x 0.5 ft = 0.125 ft3 = 6 in x 6 in x 6 in = 216 in3 */
        { L"216.0",         cubeFoot,        PrecisionFormat::Decimal1Place,   false,    true,        true,         true,            ',',          '.',        2.0,        inches       },
        { L"0.125",         cubeFoot,        PrecisionFormat::Decimal3Places,  false,    true,        true,         true,            ',',          '.',        2.0,        feet         },
        };

    for (int iTest = 0; iTest < _countof(testDataArray); iTest++)
        doFormatVolumeTest (testDataArray[iTest]);
    }

