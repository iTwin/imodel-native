/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnColors_Test.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <Bentley/BeTimeUtilities.h>
#include <DgnPlatform/DgnCore/ColorUtil.h>

#if defined (_MSC_VER)
#pragma warning (disable:4702)
#endif

USING_NAMESPACE_BENTLEY_SQLITE

//=======================================================================================
// @bsiclass                                     Algirdas.Mikoliunas            02/2013
//=======================================================================================
struct TestModelColor
    {
    public:
        IntColorDef     m_colorDef;
        Utf8String      m_bookName;
        Utf8String      m_colorName;

        TestModelColor(IntColorDef colorDef, Utf8String bookName, Utf8String colorName)
            {
            m_colorDef = colorDef;
            m_bookName = bookName;
            m_colorName = colorName;
            };
        void IsEqual (TestModelColor testColor)
            {
            EXPECT_EQ ((int)testColor.m_colorDef.m_rgb.red, (int)m_colorDef.m_rgb.red) << "Red color value don't match";
            EXPECT_EQ ((int)testColor.m_colorDef.m_rgb.green, (int)m_colorDef.m_rgb.green) << "Green color value don't match";
            EXPECT_EQ ((int)testColor.m_colorDef.m_rgb.blue, (int)m_colorDef.m_rgb.blue) << "Blue color value don't match";
            EXPECT_STREQ (testColor.m_bookName.c_str(), m_bookName.c_str()) << "Book names don't match";
            EXPECT_STREQ (testColor.m_colorName.c_str(), m_colorName.c_str()) << "Color names don't match";
            };
    };

/*---------------------------------------------------------------------------------**//**
* Test fixture for testing DgnColors
* @bsimethod                                    Algirdas.Mikoliunas            03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnColorsTest : public ::testing::Test
    {
    public:
        ScopedDgnHost           m_host;
        DgnProjectPtr      project;

        void SetupProject (WCharCP projFile, FileOpenMode mode);
    };

/*---------------------------------------------------------------------------------**//**
* Set up method that opens an existing .dgndb project file
* @bsimethod                                    Algirdas.Mikoliunas            03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnColorsTest::SetupProject (WCharCP projFile, FileOpenMode mode)
    {
    
    DgnDbTestDgnManager tdm (projFile, __FILE__, mode);
    project = tdm.GetDgnProjectP();
    ASSERT_TRUE( project != NULL);
    }

/*---------------------------------------------------------------------------------**//**
* Work with Colors
* @bsimethod                                    Majd.Uddin                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnColorsTest, ExtractColor)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", OPENMODE_READONLY);
    
    DgnColors& colorTable = project->Colors();
    DgnColorMapCP colorMap = colorTable.GetDgnColorMap();
    EXPECT_TRUE (colorMap != NULL);
    
    IntColorDef colorDef;
    UInt32 colorIndex;
    bool isTrueColor = false;

    EXPECT_EQ (SUCCESS, colorTable.Extract (&colorDef, &colorIndex, &isTrueColor, NULL, NULL, 2));
    EXPECT_EQ (65280, (UInt32)colorDef);

    EXPECT_FALSE (isTrueColor);
    }

/*---------------------------------------------------------------------------------**//**
* Extract color error
* @bsimethod                                    Algirdas.Mikoliunas               03/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnColorsTest, ExtractColorError)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", OPENMODE_READONLY);
    
    DgnColors& colorTable = project->Colors();
    DgnColorMapCP colorMap = colorTable.GetDgnColorMap();
    EXPECT_TRUE (colorMap != NULL);
    
    IntColorDef colorDef;
    UInt32 colorIndex;
    bool isTrueColor;

    EXPECT_EQ (ERROR, colorTable.Extract (&colorDef, &colorIndex, &isTrueColor, NULL, NULL, COLOR_BYLEVEL));
    EXPECT_EQ (ERROR, colorTable.Extract (&colorDef, &colorIndex, &isTrueColor, NULL, NULL, COLOR_BYCELL));
    }

/*---------------------------------------------------------------------------------**//**
* Extract non exidting colors
* @bsimethod                                    Algirdas.Mikoliunas             02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnColorsTest, ExtractNonExistingColor)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", OPENMODE_READONLY);

    DgnColors& colorTable = project->Colors();
    DgnColorMapCP colorMap = colorTable.GetDgnColorMap();
    EXPECT_TRUE (colorMap != NULL);

    IntColorDef colorDef;
    UInt32 colorIndex;
    bool isTrueColor;

    EXPECT_EQ (ERROR, colorTable.Extract (&colorDef, &colorIndex, &isTrueColor, NULL, NULL, -1));
    }

/*---------------------------------------------------------------------------------**//**
* Method GetElementColor test
* @bsimethod                                    Algirdas.Mikoliunas               03/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnColorsTest, GetNonExistingElementColor)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", OPENMODE_READONLY);
    
    DgnColors& colorTable = project->Colors();
    DgnColorMapCP colorMap = colorTable.GetDgnColorMap();
    EXPECT_TRUE (colorMap != NULL);
    
    IntColorDef colorDef (255, 254, 253);

    EXPECT_EQ (INVALID_COLOR, colorTable.GetElementColor(colorDef, NULL, NULL, false));
    }

/*---------------------------------------------------------------------------------**//**
* Method GetElementColor test inserting new color
* @bsimethod                                    Algirdas.Mikoliunas               03/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnColorsTest, GetElementColorInsert)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", OPENMODE_READWRITE);
    
    DgnColors& colorTable = project->Colors();
    DgnColorMapCP colorMap = colorTable.GetDgnColorMap();
    EXPECT_TRUE (colorMap != NULL);
    
    IntColorDef colorDef (255, 254, 253);
    IntColorDef extractColorDef;
    UInt32 colorIndex;
    bool isTrueColor;

    UInt32 colorId = colorTable.GetElementColor(colorDef, "Test1", "Test2", true);
    EXPECT_NE(INVALID_COLOR, colorId);

    EXPECT_EQ (SUCCESS, colorTable.Extract (&extractColorDef, &colorIndex, &isTrueColor, NULL, NULL, colorId));
    }


/*---------------------------------------------------------------------------------**//**
* Method GetElementColor test existing color
* @bsimethod                                    Algirdas.Mikoliunas               03/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnColorsTest, GetElementColor)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", OPENMODE_READWRITE);
    
    DgnColors& colorTable = project->Colors();
    DgnColorMapCP colorMap = colorTable.GetDgnColorMap();
    EXPECT_TRUE (colorMap != NULL);
    
    Utf8String souceBookName, sourceColorName;
    RgbColorDef sourceColorDef;
    memset (&sourceColorDef, 0, sizeof (sourceColorDef));
    
    EXPECT_EQ(SUCCESS, colorTable.QueryTrueColorInfo (&sourceColorDef, &sourceColorName, &souceBookName, DgnTrueColorId(2)));
    
    IntColorDef colorDef (sourceColorDef.red, sourceColorDef.green, sourceColorDef.blue);

    UInt32 colorId = colorTable.GetElementColor(colorDef, souceBookName.c_str(), sourceColorName.c_str(), false);
    EXPECT_NE(INVALID_COLOR, colorId);
    }

/*---------------------------------------------------------------------------------**//**
* Create color element
* @bsimethod                                    Algirdas.Mikoliunas            02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnColorsTest, CreateElement)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", OPENMODE_READWRITE);

    DgnColors& colorTable = project->Colors();

    IntColorDef colorDef (255, 254, 253);
    UInt32 colorElement = colorTable.CreateElementColor (colorDef, "TestBookName", "TestColorName");
    ASSERT_NE(INVALID_COLOR, colorElement);
    
    IntColorDef intColorDef;
    UInt32 colorIndex;
    bool isTrueColor;
    Utf8String bookName, colorName;

    EXPECT_EQ (SUCCESS, colorTable.Extract (&intColorDef, &colorIndex, &isTrueColor, &bookName, &colorName, colorElement));
    
    TestModelColor testColor1(colorDef, "TestBookName", "TestColorName");
    TestModelColor testColor2(intColorDef, bookName, colorName);
    testColor1.IsEqual(testColor2);
    }

/*---------------------------------------------------------------------------------**//**
* Insert true color
* @bsimethod                                    Algirdas.Mikoliunas            03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnColorsTest, InsertTrueColor)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", OPENMODE_READWRITE);
    DgnColors& colorTable = project->Colors();

    RgbColorDef colorDef;
    colorDef.red = 255;
    colorDef.green = 254;
    colorDef.blue = 253;

    DgnTrueColorId colorId = colorTable.InsertTrueColor(colorDef, "TestName1", "TestBook1");

    EXPECT_TRUE(colorId.IsValid());
    EXPECT_EQ(colorId.GetValue(), colorTable.QueryTrueColorId(colorDef).GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* Import color element
* @bsimethod                                    Algirdas.Mikoliunas            02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnColorsTest, ImportElement)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", OPENMODE_READWRITE);

    DgnDbTestDgnManager tdmSource (L"SubStation_NoFence.i.idgndb", __FILE__, OPENMODE_READONLY);
    DgnProjectP projectSource = tdmSource.GetDgnProjectP();
    ASSERT_TRUE( projectSource != NULL);
    
    DgnColors& colorTableSource = projectSource->Colors();
    DgnColors& colorTable = project->Colors();
    
    UInt32 newColorIndex, colorIndex;
    DgnTrueColorId sourceColorId(13);
    RgbColorDef sourceColorDef;
    memset (&sourceColorDef, 0, sizeof (sourceColorDef));
    Utf8String sourceColorName, souceBookName;

    // Query color data
    EXPECT_EQ(SUCCESS, colorTableSource.QueryTrueColorInfo (&sourceColorDef, &sourceColorName, &souceBookName, sourceColorId));

    // Query color index from data
    colorIndex = colorTableSource.FindElementColor (sourceColorDef);
    EXPECT_NE(INVALID_COLOR, colorIndex);

    // Import color from one table to another
    newColorIndex = colorTable.CreateElementColor(sourceColorDef, souceBookName.c_str(), sourceColorName.c_str());
    EXPECT_NE(INVALID_COLOR, newColorIndex);
    
    IntColorDef intColorDef;
    bool isTrueColor;
    Utf8String bookName, colorName;

    // Try retrieve imported color
    EXPECT_EQ (SUCCESS, colorTable.Extract (&intColorDef, &colorIndex, &isTrueColor, &bookName, &colorName, newColorIndex));
    
    TestModelColor testColor1(intColorDef, bookName, colorName);
    TestModelColor testColor2(sourceColorDef, souceBookName, sourceColorName);
    testColor1.IsEqual(testColor2);
    }
