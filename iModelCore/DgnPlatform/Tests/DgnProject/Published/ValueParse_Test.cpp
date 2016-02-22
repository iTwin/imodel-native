/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/ValueParse_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnPlatformApi.h>
#include <Bentley/BeTest.h>
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/BackDoor.h"

USING_NAMESPACE_BENTLEY_DGN
USING_DGNDB_UNIT_TESTS_NAMESPACE

double const EPSILON = 0.000000001;

//#define KN_WIP_Parsing
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
    WCharCP       m_inputString;
    double      m_expectedValue;
    };

/*================================================================================**//**
* @bsiclass                                                     Kevin.Nyman     05/11
+===============+===============+===============+===============+===============+======*/
struct  ValueParserTestData 
    {
    WCharCP       m_inputString;
    double      m_expectedValue;
    };

/*================================================================================**//**
* @bsiclass                                                     Kevin.Nyman     05/11
+===============+===============+===============+===============+===============+======*/
struct  DirectionParserTestData
    {
    WCharCP       m_inputString;
    double        m_expectedValue;
    };

/*================================================================================**//**
* @bsiclass                                                     Kevin.Nyman     11/10
+===============+===============+===============+===============+===============+======*/
struct  DistanceParserTestDataSimple
    {
    WCharCP       m_inputString;
    double          m_expectedValue;
    size_t          m_szParsed;
    };

/*================================================================================**//**
* @bsiclass                                                     Kevin.Nyman     11/10
+===============+===============+===============+===============+===============+======*/
struct  PointParserTestDataSimple 
    {
    WCharCP         m_inputString;
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
    WCharCP             m_inputString;
    double              m_expectedValue;
    AngleMode           m_mode;
    BentleyStatus       m_expectedParseResult;
    };

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void    vp_doParseAngleTest (AngleMode mode, AngleParserTestDataSimple const& testData)
    {
    AngleParserPtr parser = AngleParser::Create();
    parser->SetAngleMode (mode);

    double angle = 0.0;
    ASSERT_EQ (SUCCESS, parser->ToValue (angle, testData.m_inputString));

    if (AngleMode::Radians == mode)
        ASSERT_NEAR (RAD (angle), RAD (testData.m_expectedValue), EPSILON) << testData.m_inputString;
    else
        ASSERT_NEAR (angle, testData.m_expectedValue, EPSILON) << testData.m_inputString;
    }

#ifdef KN_WIP_Parsing
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
#endif

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
        ASSERT_NEAR (RAD (dirValue), RAD (testData.m_expectedValue), EPSILON) << L"Input String: " << testData.m_inputString;
    else
        ASSERT_NEAR (dirValue, testData.m_expectedValue, EPSILON) << L"Input String: " << testData.m_inputString;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void    vp_doParseUnitTest (DistanceParserTestDataSimple const& testData)
    {
    DistanceParserPtr parser = DistanceParser::Create();

    parser->SetMasterUnitLabel   (L"m");
    parser->SetSubUnitLabel      (L"mm");
    parser->SetMasterUnitScale   (1000.0);
    parser->SetSubUnitScale      (1.0);

    double uorValue = 0.0;
    ASSERT_EQ (SUCCESS, parser->ToValue (uorValue, testData.m_inputString));

    ASSERT_NEAR (uorValue, testData.m_expectedValue, EPSILON) << testData.m_inputString;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void    vp_doParseValueTest (ValueParserTestData const& testData)
    {
    DoubleParserPtr parser = DoubleParser::Create();
    double          value = 0.0;

    ASSERT_EQ (SUCCESS, parser->ToValue (value, testData.m_inputString));

    EXPECT_NEAR (value, testData.m_expectedValue, EPSILON) << testData.m_inputString;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void    vp_doParsePointTest (PointParserTestDataSimple const& testData)
    {
    PointParserPtr parser = PointParser::Create();

    parser->GetDistanceParser().SetMasterUnitLabel   (L"m");
    parser->GetDistanceParser().SetSubUnitLabel      (L"mm");
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

#ifdef KN_WIP_Parsing
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
    EXPECT_NEAR (testData.m_expectedValue, angleVal, EPSILON) << testData.m_inputString;
    }
#endif

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
        { L"30",  vp_createAngleValueFromMDS (30, 0, 0)}, // not sure if this is valid
        { L"30^",  vp_createAngleValueFromMDS (30, 0, 0)},
        { L"30^20",  vp_createAngleValueFromMDS (30, 20, 0)},
        { L"30^20'",  vp_createAngleValueFromMDS (30, 20, 0)},
        { L"30^20'10",  vp_createAngleValueFromMDS (30, 20, 10)},
        { L"30^20'10\"",  vp_createAngleValueFromMDS (30, 20, 10)},
        
        { L"30^10\"",  vp_createAngleValueFromMDS (30, 0, 10)},

        { L"30",    vp_createAngleValueFromMDS (30, 0, 0)},
        { L"30:",    vp_createAngleValueFromMDS (30, 0, 0)},
        { L"30:20",    vp_createAngleValueFromMDS (30, 20, 0)},
        { L"30:20:",    vp_createAngleValueFromMDS (30, 20, 0)},
        { L"30:20:10",    vp_createAngleValueFromMDS (30, 20, 10)},

        { L"30",   vp_createAngleValueFromMDS (30, 0, 0)},
        { L"30d",   vp_createAngleValueFromMDS (30, 0, 0)},
        { L"30d20",   vp_createAngleValueFromMDS (30, 20, 0)},
        { L"30d20m",   vp_createAngleValueFromMDS (30, 20, 0)},
        { L"30d20m10",    vp_createAngleValueFromMDS (30, 20, 10)},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParseAngleTest (AngleMode::DegMinSec, testDataArray[iTest]);
    }
//=======================================================================================
// @bsiclass                                                    Umar.Hayat     11/2015
//=======================================================================================
struct DirectionParserTest : public ::testing::Test
{
    ScopedDgnHost autoDgnHost;
};
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DirectionParserTest, ParseSimple)
    {
    DirectionParserTestData testDataArray[] = 
        {
        { L"north",  vp_createAngleValueFromMDS (90, 0, 0)},
        { L"NORTH   ",  vp_createAngleValueFromMDS (90, 0, 0)},
        { L"  north   ",  vp_createAngleValueFromMDS (90, 0, 0)},
        { L"south",  vp_createAngleValueFromMDS (270, 0, 0)},
        { L"sOUth ",  vp_createAngleValueFromMDS (270, 0, 0)},
        { L"  south   ",  vp_createAngleValueFromMDS (270, 0, 0)},

        { L"East ",  vp_createAngleValueFromMDS (0, 0, 0)},
        { L"10  east   ",  vp_createAngleValueFromMDS (10, 0, 0)},

        { L"west ",  vp_createAngleValueFromMDS (180, 0, 0)},
        { L"n 10  west   ",  vp_createAngleValueFromMDS (100, 0, 0)},

        { L"n=23West",  vp_createAngleValueFromMDS (113, 0, 0)},
        { L"n/23Wes",  vp_createAngleValueFromMDS (113, 0, 0)},

        { L"n=23East",  vp_createAngleValueFromMDS (67, 0, 0)},
        { L"n/23Ea",  vp_createAngleValueFromMDS (67, 0, 0)},

        { L"north 23W",  vp_createAngleValueFromMDS (113, 0, 0)},
        { L"north 23^W",  vp_createAngleValueFromMDS (113, 0, 0)},
        { L"South 3^20e",  vp_createAngleValueFromMDS (273, 20, 0)},
        { L"South 3^20W",  vp_createAngleValueFromMDS (266, 40, 0)},

        { L"north 23W",  vp_createAngleValueFromMDS (113, 0, 0)},
        { L"north 23^W",  vp_createAngleValueFromMDS (113, 0, 0)},
        { L"South 3^20W",  vp_createAngleValueFromMDS (266, 40, 0)},

        { L"north=23W",  vp_createAngleValueFromMDS (113, 0, 0)},
        { L"north/23W",  vp_createAngleValueFromMDS (113, 0, 0)},

        { L"north=23West",  vp_createAngleValueFromMDS (113, 0, 0)},
        { L"north/23Wes",  vp_createAngleValueFromMDS (113, 0, 0)},
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
        { L"north",  vp_createAngleValueFromMDS (91, 0, 0)},
        { L"NORTH   ",  vp_createAngleValueFromMDS (91, 0, 0)},
        { L"  north   ",  vp_createAngleValueFromMDS (91, 0, 0)},
        { L"south",  vp_createAngleValueFromMDS (271, 0, 0)},
        { L"sOUth ",  vp_createAngleValueFromMDS (271, 0, 0)},
        { L"  south   ",  vp_createAngleValueFromMDS (271, 0, 0)},

        { L"East ",  vp_createAngleValueFromMDS (1, 0, 0)},
        { L"10  east   ",  vp_createAngleValueFromMDS (11, 0, 0)},

        { L"west ",  vp_createAngleValueFromMDS (181, 0, 0)},
        { L"n 10  west   ",  vp_createAngleValueFromMDS (101, 0, 0)},

        { L"n=23West",  vp_createAngleValueFromMDS (114, 0, 0)},
        { L"n/23Wes",  vp_createAngleValueFromMDS (114, 0, 0)},

        { L"n=23East",  vp_createAngleValueFromMDS (68, 0, 0)},
        { L"n/23Ea",  vp_createAngleValueFromMDS (68, 0, 0)},

        { L"north 23W",  vp_createAngleValueFromMDS (114, 0, 0)},
        { L"north 23^W",  vp_createAngleValueFromMDS (114, 0, 0)},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParseDirectionTest (AngleMode::DegMinSec, testDataArray[iTest], 1.0);
    }

#ifdef KN_WIP_Parsing
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DirectionParserTest, ParseErrors)
    {
    DirectionParserTestData testDataArray[] = 
        {
        { L"nrth",  0.0 },
        { L"NORTH 30Est", 0.0}, 
        { L"30Wst", 0.0}, 
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
        { L"east",  0.0},       // this returns success.
        { L" east", 0.0}, 
        { L"  east ", 0.0}, 
        { L"west",  180.0 },    // this returns success.
        { L" west", 180.0}, 
        { L"  west", 180.0}, 
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParseDirectionTestError (AngleMode::DegMinSec, testDataArray[iTest]);
    }
#endif
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DirectionParserTest, ParseSimpleAnglesOnly)
    {
    DirectionParserTestData testDataArray[] = 
        {
        { L"30^",  vp_createAngleValueFromMDS (30, 0, 0)},
        { L"30^20",  vp_createAngleValueFromMDS (30, 20, 0)},
        { L"30^20'",  vp_createAngleValueFromMDS (30, 20, 0)},
        { L"30^20'10",  vp_createAngleValueFromMDS (30, 20, 10)},
        { L"30^20'10\"",  vp_createAngleValueFromMDS (30, 20, 10)},
        
        { L"30^10\"",  vp_createAngleValueFromMDS (30, 0, 10)},

        { L"30",    vp_createAngleValueFromMDS (30, 0, 0)},
        { L"30:",    vp_createAngleValueFromMDS (30, 0, 0)},
        { L"30:20",    vp_createAngleValueFromMDS (30, 20, 0)},
        { L"30:20:",    vp_createAngleValueFromMDS (30, 20, 0)},
        { L"30:20:10",    vp_createAngleValueFromMDS (30, 20, 10)},

        { L"30",   vp_createAngleValueFromMDS (30, 0, 0)},
        { L"30d",   vp_createAngleValueFromMDS (30, 0, 0)},
        { L"30d20",   vp_createAngleValueFromMDS (30, 20, 0)},
        { L"30d20m",   vp_createAngleValueFromMDS (30, 20, 0)},
        { L"30d20m10",    vp_createAngleValueFromMDS (30, 20, 10)},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParseDirectionTest (AngleMode::DegMinSec, testDataArray[iTest]);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AngleParserTest, ParseSimple_Whitespace)
    {
    AngleParserTestDataSimple testDataArray[] = 
        {
        { L"  30",  vp_createAngleValueFromMDS (30, 0, 0)}, // not sure if this is valid
        { L" 30^",  vp_createAngleValueFromMDS (30, 0, 0)},
        { L"30^20  ",  vp_createAngleValueFromMDS (30, 20, 0)},
        { L" 30^20' ",  vp_createAngleValueFromMDS (30, 20, 0)},
        { L"30^20'10  ",  vp_createAngleValueFromMDS (30, 20, 10)},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParseAngleTest (AngleMode::Degrees, testDataArray[iTest]);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AngleParserTest, ParseDecimalsInvolved)
    {
    AngleParserTestDataSimple testDataArray[] = 
        {
        { L"30.5",  vp_createAngleValueFromMDS (30, 30, 0)}, // not sure if this is valid
        { L"30.5^",  vp_createAngleValueFromMDS (30, 30, 0)},
        { L"30.5^30",  vp_createAngleValueFromMDS (31, 0, 0)},
        { L"30^20'",  vp_createAngleValueFromMDS (30, 20, 0)},
        { L"30^20.5'10",  vp_createAngleValueFromMDS (30, 20, 40)},
        { L"30.5^20'10.5\"",  vp_createAngleValueFromMDS (30, 50, 10.5)},
        
        { L"30.5:20:10.5",  vp_createAngleValueFromMDS (30, 50, 10.5)},
        { L"30.5d20m10.5",  vp_createAngleValueFromMDS (30, 50, 10.5)},
        { L"0d0m10 1/4",  vp_createAngleValueFromMDS (0, 0, 10.25)},
        { L"30 1/2d20m10 1/4",  vp_createAngleValueFromMDS (30, 50, 10.25)},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParseAngleTest (AngleMode::DegMin, testDataArray[iTest]);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AngleParserTest, ParseFractions)
    {
    AngleParserTestDataSimple testDataArray[] = 
        {
        { L"1/2^2\"",  vp_createAngleValueFromMDS (0, 30, 2)},
        { L"10 1/2^2\"",  vp_createAngleValueFromMDS (10, 30, 2)},
        { L"1/2^2\"",  vp_createAngleValueFromMDS (0, 30, 2)},
        { L"0^1/2\"",  vp_createAngleValueFromMDS (0, 0, 0.5)},
       };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParseAngleTest (AngleMode::DegMin, testDataArray[iTest]);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AngleParserTest, ParseSimple_Radians)
    {
    AngleParserTestDataSimple testDataArray[] = 
        {
        { L"0",    vp_createAngleValueFromMDS (0, 0, 0)}, 
        { L"3.1415926535r",   vp_createAngleValueFromMDS (180, 0, 0)}, 
        { L"3.1415926535",    180.0}, 
        { L"3.1415926535",    vp_createAngleValueFromRadians (3.1415926535)}, 
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParseAngleTest (AngleMode::Radians, testDataArray[iTest]);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AngleParserTest, ParseSimple_Radians_Fractions)
    {
    AngleParserTestDataSimple testDataArray[] = 
        {
        { L"0",    vp_createAngleValueFromMDS (0, 0, 0)}, 
        { L"1 1/2",    vp_createAngleValueFromRadians (1.5)}, 
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParseAngleTest (AngleMode::Radians, testDataArray[iTest]);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AngleParserTest, ParseSimple_Gradians)
    {
    AngleParserTestDataSimple testDataArray[] = 
        {
        { L"0",    vp_createAngleValueFromMDS (0, 0, 0)}, 
        { L"100.0",    vp_createAngleValueFromGradians (100.0)}, 
        { L"100.0g",    vp_createAngleValueFromGradians (100.0)}, 
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParseAngleTest (AngleMode::Centesimal, testDataArray[iTest]);
    }

#ifdef KN_WIP_Parsing
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AngleParserTest, ParseSimple_Radians_Equations)
    {
    AngleParserTestDataSimple testDataArray[] = 
        {
        { L"0",    vp_createAngleValueFromMDS (0, 0, 0)}, 
        { L"PI",    vp_createAngleValueFromMDS (180, 0, 0)}, 
        { L"PI/2.0",    vp_createAngleValueFromMDS (90, 0, 0)}, 
        { L"-PI/2.0",    vp_createAngleValueFromMDS (-90, 0, 0)}, 
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParseAngleTest (AngleMode::Radians, testDataArray[iTest]);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AngleParserTest, ParseSimple_Inconsistency)
    {
    AngleParserTestDataSimple testDataArray[] = 
        {
        { L"30d20m10s",    vp_createAngleValueFromMDS (30, 20, 10)}, // The s is not interpreted as seconds but instead as South - therefore the whole value gets negated.
        { L"1 /2^2\"",  vp_createAngleValueFromMDS (0, 30, 2)}, // why is this one ok?
        { L"0^ 1\"",  vp_createAngleValueFromMDS (0, 0, 1)}, // this isn't ok but...
        { L"0^1 /2\"",  vp_createAngleValueFromMDS (0, 0, 0.5)}, // why is this one ok?
        { L"0^1   /2\"",  vp_createAngleValueFromMDS (0, 0, 0.5)}, // this one isn't - more than one space.
        // This seems like a bug in the function to me 
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParseAngleTest (AngleMode::DegMin, testDataArray[iTest]);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AngleParserTest, ParseWithWrongTypes_Expected)
    {
    AngleParserTestDataWithMode testDataArray[] = 
        {
        { L"0",                 vp_createAngleValueFromMDS (0, 0, 0),           AngleMode::Radians, SUCCESS}, 
        { L"180^",              180.0,                                          AngleMode::Radians, ERROR}, 
        { L"3.1415926535g",     180.0,                                          AngleMode::Radians, ERROR}, 
        };
    
    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParseAngleTestWithMode (testDataArray[iTest]);
    }
#endif

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AngleParserTest, ParseSimpleWhiteSpaceInBetween)
    {
    AngleParserTestDataSimple testDataArray[] = 
        {
        { L" 30^ 20' ",  vp_createAngleValueFromMDS (30, 0, 0)}, // This shows that the 20' gets disregarded because of the white space.
        // This behavior should be questioned in the new api - spaces might be considered token separators so the Angle Parser is
        // not responsible for removing them anyway.
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParseAngleTest (AngleMode::DegMin, testDataArray[iTest]);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AngleParserTest, ParseSimple_DegreeSymbol)
    {
    WString deg1 = L"30";
    deg1 += 0x00b0 /* Unicode code point for degree*/;
    deg1 += L"20'10\"";
    WString deg2 = L"30";
    deg2 += 0x00b0 /* Unicode code point for degree*/;
    deg2 += L"20'10";

    AngleParserTestDataSimple testDataArray[] = 
        {
        { deg1.c_str(),   vp_createAngleValueFromMDS (30, 20, 10)},     
        { deg2.c_str(),     vp_createAngleValueFromMDS (30, 20, 10)},     
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParseAngleTest (AngleMode::DegMin, testDataArray[iTest]);
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
        { L"112",   vp_createUnitValue (112, 0, 0),            3},
        { L"1..10..0",   vp_createUnitValue (1, 10, 0),        8},
        { L"1:10:0",   vp_createUnitValue (1, 10, 0),          6},
        { L"1;10;1",   vp_createUnitValue (1, 10, 1),          6},
        { L"1m10mm1 ",   vp_createUnitValue (1, 10, 0),        7}, // I do not like the trailing 1 gets discarded.
        { L" 1m-10mm",   vp_createUnitValue (1, 10, 0),        8}, 
        { L"1:-10:0",   vp_createUnitValue (-1, -10, 0),       7}, // a negative unit turns all of them negative.
        { L"-1:-10:0",   vp_createUnitValue (-1, -10, 0),      8}, // a negative unit turns all of them negative.
        { L"1'10\"0",   vp_createUnitValue (1, 0, 0),          6}, // This seems wrong.
        { L"1/10'",   vp_createUnitValue (0, 100, 0),          5}, // This seems wrong.
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
        { L" 1m     -10mm",   vp_createUnitValue (1, 10, 0),        -1},  // this is accepted - but it is not ok - inconsistent with all the other ways that white space is treated.
        { L" 1m     -    10mm",   vp_createUnitValue (1, 10, 0),        -1},  // this is not accepted but is ok, consistant
        { L" 1m      10mm",   vp_createUnitValue (1, 10, 0),        -1},  // this is not accepted but is ok, consistant
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParseUnitTest (testDataArray[iTest]);
    }
#endif 

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DistanceParserTest, ParseScientific)
    {
    DistanceParserTestDataSimple testDataArray[] = 
        {
        { L" 1.5E0  ",   vp_createUnitValue (1.5, 0, 0), 6},
        { L"1.5E",   vp_createUnitValue (1.5, 0, 0), (size_t)-1},
        { L"1.5E1",   vp_createUnitValue (15.0, 0, 0), 5},
        { L"1.5E-1",   vp_createUnitValue (0.15, 0, 0), 6},
        { L"-1.5E-1",   vp_createUnitValue (-0.15, 0, 0), 7},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParseUnitTest (testDataArray[iTest]);
    }

#ifdef KN_WIP_Parsing
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DistanceParserTest, ParseSimple_ShowingFailing)
    {
    DistanceParserTestDataSimple testDataArray[] = 
        {
        { L"1M10MM",    vp_createUnitValue (1, 10, 0), 6}, 
        { L"1:10MM",    vp_createUnitValue (1, 10, 0), 6}, 
        { L"1m10mm1",   vp_createUnitValue (1, 10, 1), 7}, 
        { L"2m 20mm 2",   vp_createUnitValue (2, 20, 2), 9},
        { L"1:-10:0",   vp_createUnitValue (1, -10, 0), 7},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParseUnitTest (testDataArray[iTest]);
    }
#endif

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
        { L"0m0mm,0m0mm,0m0mm",    vp_createParsedPointValue  (0.0, 0.0, 0.0),     true, true, 1.0}, 
        { L"0m0mm,0m0mm,1m0mm",    vp_createParsedPointValue  (0.0, 0.0, 1000.0),  true, true, 1.0}, 
        { L"0m0mm,0m0mm,-1m1mm",   vp_createParsedPointValue  (0.0, 0.0, -1001.0), true, true, 1.0}, 
        { L" 0m0mm,0m0mm,-1m1mm",  vp_createParsedPointValue  (0.0, 0.0, -1001.0), true, true, 1.0}, 
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
        { L"0m0mm,0m0mm",    vp_createParsedPointValue (0.0, 0.0, 0.0),      true, false, 1.0}, 
        { L"0m1mm,1m0mm",    vp_createParsedPointValue (1.0, 1000.0, 0.0) ,  true, false, 0.5}, 
        { L"0m1mm,-1m0mm",   vp_createParsedPointValue (1.0, -1000.0, 0.0) , true, false, 2.0}, 
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParsePointTest (testDataArray[iTest]);
    }

#ifdef KN_WIP_Parsing
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (PointParserTest, ParseSimple2d)
    {
    PointParserTestDataSimple testDataArray[] = 
        {
        { L"0m0mm,0m0mm,0m0mm",     vp_createParsedPointValue (0.0, 0.0, 0.0),    true, false, 1.0}, 
        { L"0m0mm,0m0mm,1m0mm",     vp_createParsedPointValue (0.0, 0.0, 0.0),    true, false, 1.0}, 
        { L"0m0mm,0m0mm,-1m1mm",    vp_createParsedPointValue (0.0, 0.0, 0.0),    true, false, 1.0}, 
        { L"0m1mm,1m0mm,-1m1mm",    vp_createParsedPointValue (1.0, 1000.0, 0.0), true, false, 2.0}, 
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParsePointTest (testDataArray[iTest]);
    }
#endif

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (PointParserTest, TestCoordSplitting)
    {
    PointParserTestDataSimple testDataArray[] = 
        {
        // In String,       Expected Point                                          Expect Success, Is3d,   Scale
        { L"1,2,3",         vp_createParsedPointValue (1000.0, 2000.0, 3000.0),     true,           true,   1.0}, 
        { L" 1, 2, 3",      vp_createParsedPointValue (1000.0, 2000.0, 3000.0),     true,           true,   1.0}, 
        { L" 1 , 2 , 3 ",   vp_createParsedPointValue (1000.0, 2000.0, 3000.0),     true,           true,   1.0}, 
        { L"  1,  2,  3",   vp_createParsedPointValue (1000.0, 2000.0, 3000.0),     true,           true,   1.0}, 
        { L"\t1,\t2,\t3",   vp_createParsedPointValue (1000.0, 2000.0, 3000.0),     true,           true,   1.0}, 
        { L"1,2",           vp_createParsedPointValue (1000.0, 2000.0, 0),          true,           true,   1.0}, 
        { L"1,2,",          vp_createParsedPointValue (1000.0, 2000.0, 0),          true,           true,   1.0}, 
        { L" 1 , 2 , ",     vp_createParsedPointValue (1000.0, 2000.0, 0),          true,           true,   1.0}, 
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
        { L"a,2,3",         vp_createParsedPointValue (0, 0, 0),                    false,          true,   1.0}, 
        { L"1,b,3",         vp_createParsedPointValue (0, 0, 0),                    false,          true,   1.0}, 
        { L"1,2,c",         vp_createParsedPointValue (0, 0, 0),                    false,          true,   1.0}, 
        { L" a , 2 , 3 ",   vp_createParsedPointValue (0, 0, 0),                    false,          true,   1.0}, 
        { L" 1 , b , 3 ",   vp_createParsedPointValue (0, 0, 0),                    false,          true,   1.0}, 
        { L" 1 , 2 , c ",   vp_createParsedPointValue (0, 0, 0),                    false,          true,   1.0}, 
        { L"1, 2, 3, ABC",  vp_createParsedPointValue (1000.0, 2000.0, 3000.0),     true,           true,   1.0}, 
        { L"1, 2, 3,    ",  vp_createParsedPointValue (1000.0, 2000.0, 3000.0),     true,           true,   1.0}, 
        { L"1, 2, 3,",      vp_createParsedPointValue (1000.0, 2000.0, 3000.0),     true,           true,   1.0}, 
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
        { L"a,2",           vp_createParsedPointValue (0, 0, 0),                    false,          false,  1.0}, 
        { L"1,b",           vp_createParsedPointValue (0, 0, 0),                    false,          false,  1.0}, 
        { L" a , 2 ",       vp_createParsedPointValue (0, 0, 0),                    false,          false,  1.0}, 
        { L" 1 , b ",       vp_createParsedPointValue (0, 0, 0),                    false,          false,  1.0}, 
        { L"1, 2, ABC",     vp_createParsedPointValue (1000.0, 2000.0, 0),          true,           false,  1.0}, 
        { L"1, 2,    ",     vp_createParsedPointValue (1000.0, 2000.0, 0),          true,           false,  1.0}, 
        { L"1, 2,",         vp_createParsedPointValue (1000.0, 2000.0, 0),          true,           false,  1.0}, 
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
        { L"0",                 0.0},
        { L"0.0",               0.0},
        { L"0.25",              0.25},
        { L"   20343.00   ",    20343.0},          // old behavior would have not parsed this.
        { L"20343.00   ",       20343.0},
        { L"100 1/4",           100.25},
        { L"100.5 1/4",         100.75},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParseValueTest (testDataArray[iTest]);
    }

#ifdef KN_WIP_Parsing
/*--------------------------------------------------------------------------------**//**
* TODO: this is not supported yet.
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (ValueParser, ParseThousandSeparaters)
    {
    ScopedDgnHost autoDgnHost;
    ValueParserTestData testDataArray[] = 
        {
        { L"1,000",             1000.0},
        };

    for (int iTest = 0; iTest < _countof (testDataArray); iTest++)
        vp_doParseValueTest (testDataArray[iTest]);
    }
#endif

