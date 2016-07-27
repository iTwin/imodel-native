/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/ValueParse_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnPlatformApi.h>
#include <Bentley/BeTest.h>
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/BackDoor.h"
#include "../TestFixture/DgnDbTestFixtures.h"

USING_NAMESPACE_BENTLEY_DGN
USING_DGNDB_UNIT_TESTS_NAMESPACE
USING_NAMESPACE_BENTLEY_DPTEST


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     05/11
+---------------+---------------+---------------+---------------+---------------+------*/
static double vp_createAngleValueFromMDS (double deg, double min, double sec)
    {
    double result = deg;
    result += min * 1.0 / 60.0;
    result += sec * 1.0 / 3600.0;
    return result;
    }

#define DEG_TO_RAD  PI/180.
#define RAD(x)      (x * DEG_TO_RAD)

static double       vp_createAngleValueFromRadians (double rad)                         { return rad * 180.0 / PI; }
static double       vp_createAngleValueFromGradians (double cent)                       { return cent * 90.0 / 100.0; }
static double       vp_createUnitValue (double master, double sub, double a)            { return master * 1000.0 + sub * 1.0 + a; }
static DPoint3d     vp_createParsedPointValue (double x, double y, double z)            { DPoint3d result; result.Init(x, y, z); return result;}

/*================================================================================**//**
* @bsiclass                                                     Kevin.Nyman     11/10
+===============+===============+===============+===============+===============+======*/
struct  AngleParserTestDataSimple
    {
    Utf8String  m_inputString;
    double      m_expectedValue;
    };

/*================================================================================**//**
* @bsiclass                                                     Kevin.Nyman     05/11
+===============+===============+===============+===============+===============+======*/
struct  ValueParserTestData 
    {
    Utf8CP      m_inputString;
    double      m_expectedValue;
    };

/*================================================================================**//**
* @bsiclass                                                     Kevin.Nyman     05/11
+===============+===============+===============+===============+===============+======*/
struct  DirectionParserTestData
    {
    Utf8CP        m_inputString;
    double        m_expectedValue;
    };

/*================================================================================**//**
* @bsiclass                                                     Kevin.Nyman     11/10
+===============+===============+===============+===============+===============+======*/
struct  DistanceParserTestDataSimple
    {
    Utf8CP        m_inputString;
    double          m_expectedValue;
    size_t          m_szParsed;
    };

/*================================================================================**//**
* @bsiclass                                                     Kevin.Nyman     11/10
+===============+===============+===============+===============+===============+======*/
struct  PointParserTestDataSimple 
    {
    Utf8CP          m_inputString;
    DPoint3d        m_expectedValue;
    bool            m_expectSuccess;
    bool            m_is3d;
    double          m_scale;
    };

/*================================================================================**//**
* @bsiclass                                                     Kevin.Nyman     11/10
+===============+===============+===============+===============+===============+======*/
struct  AngleParserTestDataWithMode
    {
    Utf8CP              m_inputString;
    double              m_expectedValue;
    AngleMode           m_mode;
    BentleyStatus       m_expectedParseResult;
    };

/*================================================================================**//**
* @bsiclass                                                     Majd.Uddin     03/16
+===============+===============+===============+===============+===============+======*/
struct  AreaVolumeParserTestData
{
    Utf8CP              m_inputString;
    double              m_expectedValue;
};

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void    vp_doParseAngleTest (AngleMode mode, AngleParserTestDataSimple const& testData)
    {
    AngleParserPtr parser = AngleParser::Create();
    parser->SetAngleMode (mode);

    double angle = 0.0;
    ASSERT_EQ (SUCCESS, parser->ToValue (angle, testData.m_inputString.c_str()));

    if (AngleMode::Radians == mode)
        ASSERT_NEAR(RAD(angle), RAD(testData.m_expectedValue), EPSILON) << testData.m_inputString;
    else
        ASSERT_NEAR(angle, testData.m_expectedValue, EPSILON) << testData.m_inputString;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void    vp_doParseDirectionTestError (AngleMode mode, DirectionParserTestData const& testData)
    {
    DirectionParserPtr parser = DirectionParser::Create();
    parser->GetAngleParser().SetAngleMode (mode);

    double  dirValue = 0.0;
    ASSERT_EQ (ERROR, parser->ToValue (dirValue, testData.m_inputString)) << "Input String: '" << testData.m_inputString << "'";
    ASSERT_EQ (0.0, dirValue);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void    vp_doParseDirectionTest (AngleMode mode, DirectionParserTestData const& testData, double trueNorth=0.0)
    {
    DirectionParserPtr parser = DirectionParser::Create();
    parser->GetAngleParser().SetAngleMode (mode);

    if (trueNorth != 0.0)
        parser->SetTrueNorthValue (trueNorth);

    double  dirValue = 0.0;
    ASSERT_EQ (SUCCESS, parser->ToValue (dirValue, testData.m_inputString)) << L"Input String: " << testData.m_inputString;

    if (AngleMode::Radians == mode)
        ASSERT_NEAR(RAD(dirValue), RAD(testData.m_expectedValue), EPSILON) << L"Input String: " << testData.m_inputString;
    else
        ASSERT_NEAR(dirValue, testData.m_expectedValue, EPSILON) << L"Input String: " << testData.m_inputString;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void    vp_doParseUnitTest (DistanceParserTestDataSimple const& testData)
    {
    DistanceParserPtr parser = DistanceParser::Create();

    parser->SetMasterUnitLabel   ("m");
    parser->SetSubUnitLabel      ("mm");
    parser->SetMasterUnitScale   (1000.0);
    parser->SetSubUnitScale      (1.0);

    double uorValue = 0.0;
    ASSERT_EQ (SUCCESS, parser->ToValue (uorValue, testData.m_inputString));

    ASSERT_NEAR(uorValue, testData.m_expectedValue, EPSILON) << testData.m_inputString;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void    vp_doParseValueTest (ValueParserTestData const& testData)
    {
    DoubleParserPtr parser = DoubleParser::Create();
    double          value = 0.0;

    ASSERT_EQ (SUCCESS, parser->ToValue (value, testData.m_inputString));

    EXPECT_NEAR(value, testData.m_expectedValue, EPSILON) << testData.m_inputString;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Majd.Uddin     03/16
+---------------+---------------+---------------+---------------+---------------+------*/
static void    vp_doParseAreaTest(AreaVolumeParserTestData const& testData)
{
    AreaParserPtr parser = AreaParser::Create();
    double          value = 0.0;

    ASSERT_EQ(SUCCESS, parser->ToValue(value, testData.m_inputString));

    EXPECT_NEAR(value, testData.m_expectedValue, EPSILON) << testData.m_inputString;
}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Majd.Uddin     03/16
+---------------+---------------+---------------+---------------+---------------+------*/
static void    vp_doParseVolumeTest(AreaVolumeParserTestData const& testData)
{
    VolumeParserPtr parser = VolumeParser::Create();
    double          value = 0.0;

    ASSERT_EQ(SUCCESS, parser->ToValue(value, testData.m_inputString));

    EXPECT_NEAR(value, testData.m_expectedValue, EPSILON) << testData.m_inputString;
}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void    vp_doParsePointTest (PointParserTestDataSimple const& testData)
    {
    PointParserPtr parser = PointParser::Create();

    parser->GetDistanceParser().SetMasterUnitLabel   ("m");
    parser->GetDistanceParser().SetSubUnitLabel      ("mm");
    parser->GetDistanceParser().SetMasterUnitScale   (1000.0);
    parser->GetDistanceParser().SetSubUnitScale      (1.0);
    parser->GetDistanceParser().SetScale (testData.m_scale);
    parser->SetIs3d (testData.m_is3d);

    DPoint3d        ptValue;
    BentleyStatus   retVal = parser->ToValue (ptValue, testData.m_inputString);

    if ( ! testData.m_expectSuccess)
        {
        ASSERT_NE (SUCCESS, retVal);
        return;
        }

    ASSERT_EQ (SUCCESS, retVal);

    DPoint3d expected;
    expected.Init (testData.m_expectedValue.x * testData.m_scale, testData.m_expectedValue.y * testData.m_scale,  testData.m_expectedValue.z);

    if (testData.m_is3d)
        expected.z *= testData.m_scale; 

    ASSERT_TRUE (ptValue.IsEqual (expected)) << testData.m_inputString;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void    vp_doParseAngleTestWithMode(AngleParserTestDataWithMode const& testData)
    {
    AngleParserPtr parser = AngleParser::Create();
    parser->SetAngleMode(testData.m_mode); 

    double angleVal;
    BentleyStatus result = parser->ToValue (angleVal, testData.m_inputString);
    EXPECT_EQ (testData.m_expectedParseResult, result);
    EXPECT_NEAR(testData.m_expectedValue, angleVal, EPSILON) << testData.m_inputString;
    }

//=======================================================================================
// @bsiclass                                                    Umar.Hayat     11/2015
//=======================================================================================
struct AngleParserTest : public ::testing::Test
{
    ScopedDgnHost autoDgnHost;
};
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AngleParserTest, ParseSimple)
    {
    AngleParserTestDataSimple testDataArray[] = 
        {
        { "30",         vp_createAngleValueFromMDS (30, 0, 0)}, // not sure if this is valid
        { "30^",        vp_createAngleValueFromMDS (30, 0, 0)},
        { "30^20",      vp_createAngleValueFromMDS (30, 20, 0)},
        { "30^20'",     vp_createAngleValueFromMDS (30, 20, 0)},
        { "30^20'10",   vp_createAngleValueFromMDS (30, 20, 10)},
        { "30^20'10\"", vp_createAngleValueFromMDS (30, 20, 10)},
        
        { "30^10\"",    vp_createAngleValueFromMDS (30, 0, 10)},

        { "30",         vp_createAngleValueFromMDS (30, 0, 0)},
        { "30:",        vp_createAngleValueFromMDS (30, 0, 0)},
        { "30:20",      vp_createAngleValueFromMDS (30, 20, 0)},
        { "30:20:",     vp_createAngleValueFromMDS (30, 20, 0)},
        { "30:20:10",   vp_createAngleValueFromMDS (30, 20, 10)},

        { "30",         vp_createAngleValueFromMDS (30, 0, 0)},
        { "30d",        vp_createAngleValueFromMDS (30, 0, 0)},
        { "30d20",      vp_createAngleValueFromMDS (30, 20, 0)},
        { "30d20m",     vp_createAngleValueFromMDS (30, 20, 0)},
        { "30d20m10",   vp_createAngleValueFromMDS (30, 20, 10)},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParseAngleTest (AngleMode::DegMinSec, testDataArray[iTest]);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AngleParserTest, ParseSimple_Whitespace)
{
    AngleParserTestDataSimple testDataArray[] =
    {
        { "  30", vp_createAngleValueFromMDS(30, 0, 0) }, // not sure if this is valid
        { " 30^", vp_createAngleValueFromMDS(30, 0, 0) },
        { "30^20  ", vp_createAngleValueFromMDS(30, 20, 0) },
        { " 30^20' ", vp_createAngleValueFromMDS(30, 20, 0) },
        { "30^20'10  ", vp_createAngleValueFromMDS(30, 20, 10) },
    };

    for (int iTest = 0; iTest < _countof(testDataArray); iTest++)
        vp_doParseAngleTest(AngleMode::Degrees, testDataArray[iTest]);
}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AngleParserTest, ParseDecimalsInvolved)
{
    AngleParserTestDataSimple testDataArray[] =
    {
        { "30.5", vp_createAngleValueFromMDS(30, 30, 0) }, // not sure if this is valid
        { "30.5^", vp_createAngleValueFromMDS(30, 30, 0) },
        { "30.5^30", vp_createAngleValueFromMDS(31, 0, 0) },
        { "30^20'", vp_createAngleValueFromMDS(30, 20, 0) },
        { "30^20.5'10", vp_createAngleValueFromMDS(30, 20, 40) },
        { "30.5^20'10.5\"", vp_createAngleValueFromMDS(30, 50, 10.5) },

        { "30.5:20:10.5", vp_createAngleValueFromMDS(30, 50, 10.5) },
        { "30.5d20m10.5", vp_createAngleValueFromMDS(30, 50, 10.5) },
        { "0d0m10 1/4", vp_createAngleValueFromMDS(0, 0, 10.25) },
        { "30 1/2d20m10 1/4", vp_createAngleValueFromMDS(30, 50, 10.25) },
    };

    for (int iTest = 0; iTest < _countof(testDataArray); iTest++)
        vp_doParseAngleTest(AngleMode::DegMin, testDataArray[iTest]);
}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AngleParserTest, ParseFractions)
{
    AngleParserTestDataSimple testDataArray[] =
    {
        { "1/2^2\"", vp_createAngleValueFromMDS(0, 30, 2) },
        { "10 1/2^2\"", vp_createAngleValueFromMDS(10, 30, 2) },
        { "1/2^2\"", vp_createAngleValueFromMDS(0, 30, 2) },
        { "0^1/2\"", vp_createAngleValueFromMDS(0, 0, 0.5) },
    };

    for (int iTest = 0; iTest < _countof(testDataArray); iTest++)
        vp_doParseAngleTest(AngleMode::DegMin, testDataArray[iTest]);
}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AngleParserTest, ParseSimple_Radians)
{
    AngleParserTestDataSimple testDataArray[] =
    {
        { "0", vp_createAngleValueFromMDS(0, 0, 0) },
        { "3.1415926535r", vp_createAngleValueFromMDS(180, 0, 0) },
        { "3.1415926535", 180.0 },
        { "3.1415926535", vp_createAngleValueFromRadians(3.1415926535) },
    };

    for (int iTest = 0; iTest < _countof(testDataArray); iTest++)
        vp_doParseAngleTest(AngleMode::Radians, testDataArray[iTest]);
}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AngleParserTest, ParseSimple_Radians_Fractions)
{
    AngleParserTestDataSimple testDataArray[] =
    {
        { "0", vp_createAngleValueFromMDS(0, 0, 0) },
        { "1 1/2", vp_createAngleValueFromRadians(1.5) },
    };

    for (int iTest = 0; iTest < _countof(testDataArray); iTest++)
        vp_doParseAngleTest(AngleMode::Radians, testDataArray[iTest]);
}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AngleParserTest, ParseSimple_Gradians)
{
    AngleParserTestDataSimple testDataArray[] =
    {
        { "0", vp_createAngleValueFromMDS(0, 0, 0) },
        { "100.0", vp_createAngleValueFromGradians(100.0) },
        { "100.0g", vp_createAngleValueFromGradians(100.0) },
    };

    for (int iTest = 0; iTest < _countof(testDataArray); iTest++)
        vp_doParseAngleTest(AngleMode::Centesimal, testDataArray[iTest]);
}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AngleParserTest, ParseSimple_Radians_Equations)
{
    AngleParserTestDataSimple testDataArray[] =
    {
        { "0", vp_createAngleValueFromMDS(0, 0, 0) },
        { "3.1415926535", vp_createAngleValueFromMDS(180, 0, 0) },
        { "3.1415926535/2.0", vp_createAngleValueFromMDS(90, 0, 0) },
        { "-3.1415926535/2.0", vp_createAngleValueFromMDS(-90, 0, 0) },
    };

    for (int iTest = 0; iTest < _countof(testDataArray); iTest++)
        vp_doParseAngleTest(AngleMode::Radians, testDataArray[iTest]);
}

#ifdef KN_WIP_Parsing
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AngleParserTest, ParseSimple_Inconsistency)
{
    AngleParserTestDataSimple testDataArray[] =
    {
        { "30d20m10s", vp_createAngleValueFromMDS(30, 20, 10) }, // The s is not interpreted as seconds but instead as South - therefore the whole value gets negated.
        { "1 /2^2\"", vp_createAngleValueFromMDS(0, 30, 2) }, // why is this one ok?
        { "0^ 1\"", vp_createAngleValueFromMDS(0, 0, 1) }, // this isn't ok but...
        { "0^1 /2\"", vp_createAngleValueFromMDS(0, 0, 0.5) }, // why is this one ok?
        { "0^1   /2\"", vp_createAngleValueFromMDS(0, 0, 0.5) }, // this one isn't - more than one space.
        // This seems like a bug in the function to me 
    };

    for (int iTest = 0; iTest < _countof(testDataArray); iTest++)
        vp_doParseAngleTest(AngleMode::DegMin, testDataArray[iTest]);
}
#endif

/*--------------------------------------------------------------------------------**//**
* The test was marked as should return ERROR. But now the Parser handles it and returns SUCCESS
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AngleParserTest, ParseWithWrongTypes_ShouldWork)
{
    AngleParserTestDataWithMode testDataArray[] =
    {
        { "0", vp_createAngleValueFromMDS(0, 0, 0), AngleMode::Radians, SUCCESS },
        { "180^", 180.0, AngleMode::Radians, SUCCESS },
        { "3.141592653589793g", 180.0, AngleMode::Radians, SUCCESS },
    };

    for (int iTest = 0; iTest < _countof(testDataArray); iTest++)
        vp_doParseAngleTestWithMode(testDataArray[iTest]);
}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AngleParserTest, ParseSimpleWhiteSpaceInBetween)
{
    AngleParserTestDataSimple testDataArray[] =
    {
        { " 30^ 20' ", vp_createAngleValueFromMDS(30, 0, 0) }, // This shows that the 20' gets disregarded because of the white space.
        // This behavior should be questioned in the new api - spaces might be considered token separators so the Angle Parser is
        // not responsible for removing them anyway.
    };

    for (int iTest = 0; iTest < _countof(testDataArray); iTest++)
        vp_doParseAngleTest(AngleMode::DegMin, testDataArray[iTest]);
}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AngleParserTest, ParseSimple_DegreeSymbol)
{
    WString deg1 = L"30";
    deg1 += 0x00b0 /* Unicode code point for degree*/;
    deg1 += L"20'10\"";
    WString deg2 = L"30";
    deg2 += 0x00b0 /* Unicode code point for degree*/;
    deg2 += L"20'10";

    AngleParserTestDataSimple testDataArray[] =
    {
        { Utf8String(deg1), vp_createAngleValueFromMDS(30, 20, 10) },
        { Utf8String(deg2), vp_createAngleValueFromMDS(30, 20, 10) },
    };

    for (int iTest = 0; iTest < _countof(testDataArray); iTest++)
        vp_doParseAngleTest(AngleMode::DegMin, testDataArray[iTest]);
}

//=======================================================================================
// @bsiclass                                                    Umar.Hayat     11/2015
//=======================================================================================
struct DirectionParserTest : public DgnDbTestFixture {};
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DirectionParserTest, ParseSimple)
    {
    DirectionParserTestData testDataArray[] = 
        {
        { "north",          vp_createAngleValueFromMDS (90, 0, 0)},
        { "NORTH   ",       vp_createAngleValueFromMDS (90, 0, 0)},
        { "  north   ",     vp_createAngleValueFromMDS (90, 0, 0)},
        { "south",          vp_createAngleValueFromMDS (270, 0, 0)},
        { "sOUth ",         vp_createAngleValueFromMDS (270, 0, 0)},
        { "  south   ",     vp_createAngleValueFromMDS (270, 0, 0)},

        { "East ",          vp_createAngleValueFromMDS (0, 0, 0)},
        { "10  east         ",vp_createAngleValueFromMDS (10, 0, 0)},

        { "west ",          vp_createAngleValueFromMDS (180, 0, 0)},
        { "n 10  west   ",  vp_createAngleValueFromMDS (100, 0, 0)},

        { "n=23West",       vp_createAngleValueFromMDS (113, 0, 0)},
        { "n/23Wes",        vp_createAngleValueFromMDS (113, 0, 0)},

        { "n=23East",       vp_createAngleValueFromMDS (67, 0, 0)},
        { "n/23Ea",         vp_createAngleValueFromMDS (67, 0, 0)},

        { "north 23W",      vp_createAngleValueFromMDS (113, 0, 0)},
        { "north 23^W",     vp_createAngleValueFromMDS (113, 0, 0)},
        { "South 3^20e",    vp_createAngleValueFromMDS (273, 20, 0)},
        { "South 3^20W",    vp_createAngleValueFromMDS (266, 40, 0)},

        { "north 23W",      vp_createAngleValueFromMDS (113, 0, 0)},
        { "north 23^W",     vp_createAngleValueFromMDS (113, 0, 0)},
        { "South 3^20W",    vp_createAngleValueFromMDS (266, 40, 0)},

        { "north=23W",      vp_createAngleValueFromMDS (113, 0, 0)},
        { "north/23W",      vp_createAngleValueFromMDS (113, 0, 0)},

        { "north=23West",   vp_createAngleValueFromMDS (113, 0, 0)},
        { "north/23Wes",    vp_createAngleValueFromMDS (113, 0, 0)},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParseDirectionTest (AngleMode::DegMinSec, testDataArray[iTest]);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DirectionParserTest, ParseSimpleTrueNorth)
    {
    DirectionParserTestData testDataArray[] = 
        {
        { "north",          vp_createAngleValueFromMDS (91, 0, 0)},
        { "NORTH   ",       vp_createAngleValueFromMDS (91, 0, 0)},
        { "  north   ",     vp_createAngleValueFromMDS (91, 0, 0)},
        { "south",          vp_createAngleValueFromMDS (271, 0, 0)},
        { "sOUth ",         vp_createAngleValueFromMDS (271, 0, 0)},
        { "  south   ",     vp_createAngleValueFromMDS (271, 0, 0)},

        { "East ",          vp_createAngleValueFromMDS (1, 0, 0)},
        { "10  east   ",    vp_createAngleValueFromMDS (11, 0, 0)},

        { "west ",          vp_createAngleValueFromMDS (181, 0, 0)},
        { "n 10  west   ",  vp_createAngleValueFromMDS (101, 0, 0)},

        { "n=23West",       vp_createAngleValueFromMDS (114, 0, 0)},
        { "n/23Wes",        vp_createAngleValueFromMDS (114, 0, 0)},

        { "n=23East",       vp_createAngleValueFromMDS (68, 0, 0)},
        { "n/23Ea",         vp_createAngleValueFromMDS (68, 0, 0)},

        { "north 23W",      vp_createAngleValueFromMDS (114, 0, 0)},
        { "north 23^W",     vp_createAngleValueFromMDS (114, 0, 0)},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParseDirectionTest (AngleMode::DegMinSec, testDataArray[iTest], 1.0);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DirectionParserTest, ParseErrors)
    {
    DirectionParserTestData testDataArray[] = 
        {
        { "nrth",  0.0 },
        { "NORTH 30Est", 0.0}, 
        { "30Wst", 0.0}, 
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParseDirectionTestError (AngleMode::DegMinSec, testDataArray[iTest]);
    }

/*--------------------------------------------------------------------------------**//**
* I made parsers ignore leading whitespace. Old API would not do that.
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DirectionParserTest, ParseSimple_LegacyErrors)
    {
    DirectionParserTestData testDataArray[] = 
        {
        { "east",  0.0},       // this returns success.
        { " east", 0.0}, 
        { "  east ", 0.0}, 
        { "west",  180.0 },    // this returns success.
        { " west", 180.0}, 
        { "  west", 180.0}, 
        };

    //All of above now pass as white spaces are ignored.
    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParseDirectionTest (AngleMode::DegMinSec, testDataArray[iTest]);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DirectionParserTest, ParseSimpleAnglesOnly)
    {
    DirectionParserTestData testDataArray[] = 
        {
        { "30^",        vp_createAngleValueFromMDS (30, 0, 0)},
        { "30^20",      vp_createAngleValueFromMDS (30, 20, 0)},
        { "30^20'",     vp_createAngleValueFromMDS (30, 20, 0)},
        { "30^20'10",   vp_createAngleValueFromMDS (30, 20, 10)},
        { "30^20'10\"", vp_createAngleValueFromMDS (30, 20, 10)},

        { "30^10\"",    vp_createAngleValueFromMDS (30, 0, 10)},

        { "30",         vp_createAngleValueFromMDS (30, 0, 0)},
        { "30:",        vp_createAngleValueFromMDS (30, 0, 0)},
        { "30:20",      vp_createAngleValueFromMDS (30, 20, 0)},
        { "30:20:",     vp_createAngleValueFromMDS (30, 20, 0)},
        { "30:20:10",   vp_createAngleValueFromMDS (30, 20, 10)},

        { "30",         vp_createAngleValueFromMDS (30, 0, 0)},
        { "30d",        vp_createAngleValueFromMDS (30, 0, 0)},
        { "30d20",      vp_createAngleValueFromMDS (30, 20, 0)},
        { "30d20m",     vp_createAngleValueFromMDS (30, 20, 0)},
        { "30d20m10",   vp_createAngleValueFromMDS (30, 20, 10)},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParseDirectionTest (AngleMode::DegMinSec, testDataArray[iTest]);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Majd.Uddin     03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DirectionParserTest, BaseDirection_ClockWise)
{
    DirectionParserPtr dirPar = DirectionParser::Create();

    ASSERT_EQ(0.0, dirPar->GetTrueNorthValue()); //True North should be 0.0 as per Init()
    ASSERT_FALSE(dirPar->GetClockwise()); //by default clockwise is false
    ASSERT_EQ(0.0, dirPar->GetBaseDirection()); //Base direction should be 0.0 as per Init()

    double  dirValue = 0.0;
    ASSERT_EQ(SUCCESS, dirPar->ToValue(dirValue, "10 east"));
    ASSERT_EQ(10.0, dirValue); //BeseDir is 0.0 and it is not clockwise

    dirPar->SetClockwise(true);
    ASSERT_TRUE(dirPar->GetClockwise());
    ASSERT_EQ(SUCCESS, dirPar->ToValue(dirValue, "10 east"));
    ASSERT_EQ(-10.0, dirValue); //BeseDir is 0.0 and it is clockwise

    dirPar->SetBaseDirection(90.0);
    ASSERT_EQ(90.0, dirPar->GetBaseDirection());
    ASSERT_EQ(SUCCESS, dirPar->ToValue(dirValue, "10 east"));
    ASSERT_EQ(80.0, dirValue); //BeseDir is 0.0 and it is clockwise

    dirPar->SetClockwise(false);
    ASSERT_EQ(SUCCESS, dirPar->ToValue(dirValue, "10 east"));
    ASSERT_EQ(-80.0, dirValue); //BeseDir is 90.0 and it is not clockwise

    dirPar->SetTrueNorthValue(90.0);
    ASSERT_EQ(SUCCESS, dirPar->ToValue(dirValue, "10 east"));
    ASSERT_EQ(10.0, dirValue); //TrueNorth will be added to previous value

    dirPar->SetClockwise(true);
    ASSERT_EQ(SUCCESS, dirPar->ToValue(dirValue, "10 east"));
    ASSERT_EQ(170.0, dirValue); //TrueNorth will be added to previous value of 80


    ASSERT_EQ(DirectionMode::Azimuth, dirPar->GetDirectionMode());
    dirPar->SetDirectionMode(DirectionMode::Bearing);
    ASSERT_EQ(DirectionMode::Bearing, dirPar->GetDirectionMode());
    ASSERT_EQ(SUCCESS, dirPar->ToValue(dirValue, "10 east"));
    ASSERT_EQ(100.0, dirValue); //For Bearing mode, Clockwise or Base Direction doesn't matter. So it should be 10 + True North

    dirPar->SetTrueNorthValue(0.0);
    ASSERT_EQ(SUCCESS, dirPar->ToValue(dirValue, "10 east"));
    ASSERT_EQ(10.0, dirValue); //For Bearing mode, Clockwise or Base Direction doesn't matter. So it should be 10 + True North

}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Majd.Uddin     03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DirectionParserTest, Clone)
{
    DirectionParserPtr dirPar = DirectionParser::Create();
    dirPar->SetClockwise(true);
    dirPar->SetBaseDirection(270.0);

    DirectionParserPtr clonePar = dirPar->Clone();

    ASSERT_TRUE(clonePar->GetClockwise());
    ASSERT_EQ(270.0, clonePar->GetBaseDirection());
}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Majd.Uddin     03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DirectionParserTest, FromModel)
{
    SetupSeedProject();
    DgnDbR db = *m_db;

    DgnModelPtr seedModel = db.Models().GetModel(db.Models().QueryFirstModelId());
    seedModel->FillModel();
    EXPECT_TRUE(seedModel != nullptr);

    //Set direction Anti-Clockwise and set a Base Direction
    GeometricModelP geomModelP = seedModel->ToGeometricModelP();
    geomModelP->GetDisplayInfoR().SetDirectionClockwise(true);
    geomModelP->GetDisplayInfoR().SetDirectionBaseDir(270.0);

    DirectionParserPtr dirPar = DirectionParser::Create(*geomModelP);

    ASSERT_TRUE(dirPar->GetClockwise());
    ASSERT_EQ(270.0, dirPar->GetBaseDirection());
    ASSERT_EQ(0.0, dirPar->GetTrueNorthValue());
    
}



//=======================================================================================
// @bsiclass                                                    Umar.Hayat     11/2015
//=======================================================================================
struct DistanceParserTest : public ::testing::Test
{
    ScopedDgnHost autoDgnHost;
};
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DistanceParserTest, ParseSimple)
    {
    DistanceParserTestDataSimple testDataArray[] = 
        {
        { "112",        vp_createUnitValue (112, 0, 0),         3},
        { "1..10..0",   vp_createUnitValue (1, 10, 0),          8},
        { "1:10:0",     vp_createUnitValue (1, 10, 0),          6},
        { "1;10;1",     vp_createUnitValue (1, 10, 1),          6},
        { "1m10mm1 ",   vp_createUnitValue (1, 10, 0),          7}, // I do not like the trailing 1 gets discarded.
        { " 1m-10mm",   vp_createUnitValue (1, 10, 0),          8}, 
        { "1:-10:0",    vp_createUnitValue (-1, -10, 0),        7}, // a negative unit turns all of them negative.
        { "-1:-10:0",   vp_createUnitValue (-1, -10, 0),        8}, // a negative unit turns all of them negative.
        { "1'10\"0",    vp_createUnitValue (1, 0, 0),           6}, // This seems wrong.
        { "1/10'",      vp_createUnitValue (0, 100, 0),         5}, // This seems wrong.
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParseUnitTest (testDataArray[iTest]);
    }

#ifdef KN_WIP_Parsing
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DistanceParserTest, ParseSimple_Inconsistency)
    {
    DistanceParserTestDataSimple testDataArray[] = 
        {
        { " 1m     -10mm",      vp_createUnitValue(1, 10, 0),  (size_t)-1},  // this is accepted - but it is not ok - inconsistent with all the other ways that white space is treated.
        { " 1m     -    10mm",  vp_createUnitValue (1, 10, 0), (size_t)-1},  // this is not accepted but is ok, consistant
        { " 1m      10mm",      vp_createUnitValue (1, 10, 0), (size_t)-1},  // this is not accepted but is ok, consistant
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParseUnitTest (testDataArray[iTest]);
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DistanceParserTest, ParseSimple_ShowingFailing)
{
    DistanceParserTestDataSimple testDataArray[] =
    {
        { "1M10MM", vp_createUnitValue(1, 10, 0), 6 },
        { "1:10MM", vp_createUnitValue(1, 10, 0), 6 },
        { "1m10mm1", vp_createUnitValue(1, 10, 1), 7 },
        { "2m 20mm 2", vp_createUnitValue(2, 20, 2), 9 },
        { "1:-10:0", vp_createUnitValue(1, -10, 0), 7 },
    };

    for (int iTest = 0; iTest < _countof(testDataArray); iTest++)
        vp_doParseUnitTest(testDataArray[iTest]);
}

#endif 

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DistanceParserTest, ParseScientific)
    {
    DistanceParserTestDataSimple testDataArray[] = 
        {
        { " 1.5E0  ",   vp_createUnitValue (1.5, 0, 0), 6},
        { "1.5E",       vp_createUnitValue (1.5, 0, 0), (size_t)-1},
        { "1.5E1",      vp_createUnitValue (15.0, 0, 0), 5},
        { "1.5E-1",     vp_createUnitValue (0.15, 0, 0), 6},
        { "-1.5E-1",    vp_createUnitValue (-0.15, 0, 0), 7},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParseUnitTest (testDataArray[iTest]);
    }

//=======================================================================================
// @bsiclass                                                    Umar.Hayat     11/2015
//=======================================================================================
struct PointParserTest : public ::testing::Test
{
    ScopedDgnHost autoDgnHost;
};
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (PointParserTest, ParseSimple)
    {
    PointParserTestDataSimple testDataArray[] = 
        {
        { "0m0mm,0m0mm,0m0mm",    vp_createParsedPointValue  (0.0, 0.0, 0.0),     true, true, 1.0}, 
        { "0m0mm,0m0mm,1m0mm",    vp_createParsedPointValue  (0.0, 0.0, 1000.0),  true, true, 1.0}, 
        { "0m0mm,0m0mm,-1m1mm",   vp_createParsedPointValue  (0.0, 0.0, -1001.0), true, true, 1.0}, 
        { " 0m0mm,0m0mm,-1m1mm",  vp_createParsedPointValue  (0.0, 0.0, -1001.0), true, true, 1.0}, 
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParsePointTest (testDataArray[iTest]);
    }

/*--------------------------------------------------------------------------------**//**
* Legacy version expected only 2 units passed in. It would consider everything after the first ',' the y value.
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (PointParserTest, ParseSimple2dLegacy)
    {
    PointParserTestDataSimple testDataArray[] = 
        {
        { "0m0mm,0m0mm",    vp_createParsedPointValue (0.0, 0.0, 0.0),      true, false, 1.0}, 
        { "0m1mm,1m0mm",    vp_createParsedPointValue (1.0, 1000.0, 0.0) ,  true, false, 0.5}, 
        { "0m1mm,-1m0mm",   vp_createParsedPointValue (1.0, -1000.0, 0.0) , true, false, 2.0}, 
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParsePointTest (testDataArray[iTest]);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (PointParserTest, ParseSimple2d)
    {
    PointParserTestDataSimple testDataArray[] = 
        {
        { "0m0mm,0m0mm,0m0mm",     vp_createParsedPointValue (0.0, 0.0, 0.0),    true, false, 1.0}, 
        { "0m0mm,0m0mm,1m0mm",     vp_createParsedPointValue (0.0, 0.0, 0.0),    true, false, 1.0}, 
        { "0m0mm,0m0mm,-1m1mm",    vp_createParsedPointValue (0.0, 0.0, 0.0),    true, false, 1.0}, 
        { "0m1mm,1m0mm,-1m1mm",    vp_createParsedPointValue (1.0, 1000.0, 0.0), true, false, 2.0}, 
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParsePointTest (testDataArray[iTest]);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (PointParserTest, TestCoordSplitting)
    {
    PointParserTestDataSimple testDataArray[] = 
        {
        // In String,       Expected Point                                          Expect Success, Is3d,   Scale
        { "1,2,3",         vp_createParsedPointValue (1000.0, 2000.0, 3000.0),     true,           true,   1.0}, 
        { " 1, 2, 3",      vp_createParsedPointValue (1000.0, 2000.0, 3000.0),     true,           true,   1.0}, 
        { " 1 , 2 , 3 ",   vp_createParsedPointValue (1000.0, 2000.0, 3000.0),     true,           true,   1.0}, 
        { "  1,  2,  3",   vp_createParsedPointValue (1000.0, 2000.0, 3000.0),     true,           true,   1.0}, 
        { "\t1,\t2,\t3",   vp_createParsedPointValue (1000.0, 2000.0, 3000.0),     true,           true,   1.0}, 
        { "1,2",           vp_createParsedPointValue (1000.0, 2000.0, 0),          true,           true,   1.0}, 
        { "1,2,",          vp_createParsedPointValue (1000.0, 2000.0, 0),          true,           true,   1.0}, 
        { " 1 , 2 , ",     vp_createParsedPointValue (1000.0, 2000.0, 0),          true,           true,   1.0}, 
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParsePointTest (testDataArray[iTest]);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (PointParserTest, TestUnusualInputs)
    {
    PointParserTestDataSimple testDataArray[] = 
        {
        // In String,       Expected Point                                          Expect Success, Is3d,   Scale
        { "a,2,3",         vp_createParsedPointValue (0, 0, 0),                    false,          true,   1.0}, 
        { "1,b,3",         vp_createParsedPointValue (0, 0, 0),                    false,          true,   1.0}, 
        { "1,2,c",         vp_createParsedPointValue (0, 0, 0),                    false,          true,   1.0}, 
        { " a , 2 , 3 ",   vp_createParsedPointValue (0, 0, 0),                    false,          true,   1.0}, 
        { " 1 , b , 3 ",   vp_createParsedPointValue (0, 0, 0),                    false,          true,   1.0}, 
        { " 1 , 2 , c ",   vp_createParsedPointValue (0, 0, 0),                    false,          true,   1.0}, 
        { "1, 2, 3, ABC",  vp_createParsedPointValue (1000.0, 2000.0, 3000.0),     true,           true,   1.0}, 
        { "1, 2, 3,    ",  vp_createParsedPointValue (1000.0, 2000.0, 3000.0),     true,           true,   1.0}, 
        { "1, 2, 3,",      vp_createParsedPointValue (1000.0, 2000.0, 3000.0),     true,           true,   1.0}, 
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParsePointTest (testDataArray[iTest]);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (PointParserTest, TestUnusualInputs2d)
    {
    PointParserTestDataSimple testDataArray[] = 
        {
        // In String,       Expected Point                                          Expect Success, Is3d,   Scale
        { "a,2",           vp_createParsedPointValue (0, 0, 0),                    false,          false,  1.0}, 
        { "1,b",           vp_createParsedPointValue (0, 0, 0),                    false,          false,  1.0}, 
        { " a , 2 ",       vp_createParsedPointValue (0, 0, 0),                    false,          false,  1.0}, 
        { " 1 , b ",       vp_createParsedPointValue (0, 0, 0),                    false,          false,  1.0}, 
        { "1, 2, ABC",     vp_createParsedPointValue (1000.0, 2000.0, 0),          true,           false,  1.0}, 
        { "1, 2,    ",     vp_createParsedPointValue (1000.0, 2000.0, 0),          true,           false,  1.0}, 
        { "1, 2,",         vp_createParsedPointValue (1000.0, 2000.0, 0),          true,           false,  1.0}, 
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParsePointTest (testDataArray[iTest]);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (ValueParser, ParseSimple)
    {
    ScopedDgnHost autoDgnHost;
    ValueParserTestData testDataArray[] = 
        {
        { "0",                 0.0},
        { "0.0",               0.0},
        { "0.25",              0.25},
        { "   20343.00   ",    20343.0},          // old behavior would have not parsed this.
        { "20343.00   ",       20343.0},
        { "100 1/4",           100.25},
        { "100.5 1/4",         100.75},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParseValueTest (testDataArray[iTest]);
    }
#ifdef KN_WIP_Parsing
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (ValueParser, ParseThousandSeparaters)
    {
    ScopedDgnHost autoDgnHost;
    ValueParserTestData testDataArray[] = 
        {
        { "1,000",             1000.0},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParseValueTest (testDataArray[iTest]);
    }
#endif

//=======================================================================================
// @bsiclass                                                    Majd.Uddin     03/2016
//=======================================================================================
struct AreaVolumeParserTest : public DgnDbTestFixture {};

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Majd.Uddin     03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AreaVolumeParserTest, AreaParserCreateFromModel)
{
    SetupSeedProject();
    DgnDbR db = *m_db;

    DgnModelPtr seedModel = db.Models().GetModel(db.Models().QueryFirstModelId());
    seedModel->FillModel();
    EXPECT_TRUE(seedModel != nullptr);

    GeometricModelP geomModelP = seedModel->ToGeometricModelP();

    AreaParserPtr areaPar = AreaParser::Create(*geomModelP);
    
    ASSERT_EQ (1.0, areaPar->GetMasterUnitsScale()); //The model's master Units are in mm.
    ASSERT_EQ (1.0, areaPar->GetScale()); //The scale is 1.0

    areaPar->SetScale(2.0);
    ASSERT_EQ(2.0, areaPar->GetScale());

    AreaParserPtr areaPar2 = areaPar->Clone();
    ASSERT_EQ(2.0, areaPar2->GetScale());
}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Majd.Uddin     03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AreaVolumeParserTest, VolumeParserCreateFromModel)
{
    SetupSeedProject();
    DgnDbR db = *m_db;

    DgnModelPtr seedModel = db.Models().GetModel(db.Models().QueryFirstModelId());
    seedModel->FillModel();
    EXPECT_TRUE(seedModel != nullptr);

    GeometricModelP geomModelP = seedModel->ToGeometricModelP();

    VolumeParserPtr volPar = VolumeParser::Create(*geomModelP);

    ASSERT_EQ(1.0, volPar->GetMasterUnitsScale()); //The model's master Units are in mm.
    ASSERT_EQ(1.0, volPar->GetScale()); //The scale is 1.0

    VolumeParserPtr volPar2 = volPar->Clone();
    ASSERT_EQ(1.0, volPar2->GetScale());
}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Majd.Uddin     03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AreaVolumeParserTest, AreaParserSimple)
{
    AreaVolumeParserTestData testDataArray[] =
    {
        { "1000MM2", 1000.0 },
        { "-500M2", 500.0 },
        { "  1000  ", 1000.0},
        { "1000/2 CM2", 500.0},
        { "123.45678FT2", 123.45678},
        { "55.0 IN2", 55.0},
        { "0", 0.0},
    };

    for (int iTest = 0; iTest < _countof(testDataArray); iTest++)
        vp_doParseAreaTest(testDataArray[iTest]);
}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Majd.Uddin     03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AreaVolumeParserTest, VolumeParserSimple)
{
    AreaVolumeParserTestData testDataArray[] =
    {
        { "1000MM3", 1000.0 },
        { "-500M3", 500.0 },
        { "  1000  ", 1000.0 },
        { "1000/2 CM3", 500.0 },
        { "123.45678CFT", 123.45678 },
        { "55.0 CIN", 55.0 },
        { "0", 0.0 },

    };

    for (int iTest = 0; iTest < _countof(testDataArray); iTest++)
        vp_doParseVolumeTest(testDataArray[iTest]);
}


