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
        ColorDef     m_colorDef;
        Utf8String      m_bookName;
        Utf8String      m_colorName;

        TestModelColor(ColorDef colorDef, Utf8String bookName, Utf8String colorName)
            {
            m_colorDef = colorDef;
            m_bookName = bookName;
            m_colorName = colorName;
            };
        void IsEqual (TestModelColor testColor)
            {
            EXPECT_EQ (testColor.m_colorDef.GetValue(), (int)m_colorDef.GetValue()) << "color values don't match";
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
        DgnDbPtr      project;

        void SetupProject (WCharCP projFile, Db::OpenMode mode);
    };

/*---------------------------------------------------------------------------------**//**
* Set up method that opens an existing .dgndb project file
* @bsimethod                                    Algirdas.Mikoliunas            03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnColorsTest::SetupProject (WCharCP projFile, Db::OpenMode mode)
    {
    
    DgnDbTestDgnManager tdm (projFile, __FILE__, mode);
    project = tdm.GetDgnProjectP();
    ASSERT_TRUE( project != NULL);
    }

/*---------------------------------------------------------------------------------**//**
* Insert true color
* @bsimethod                                    Algirdas.Mikoliunas            03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnColorsTest, InsertTrueColor)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", Db::OPEN_ReadWrite);
    DgnColors& colorTable = project->Colors();

    ColorDef colorDef (255, 254, 253);
    DgnTrueColorId colorId = colorTable.InsertColor(colorDef, "TestName1", "TestBook1");

    EXPECT_TRUE(colorId.IsValid());
    EXPECT_EQ(colorId.GetValue(), colorTable.FindMatchingColor(colorDef).GetValue());
    }

