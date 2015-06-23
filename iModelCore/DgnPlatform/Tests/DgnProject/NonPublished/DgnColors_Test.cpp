/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnColors_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <Bentley/BeTimeUtilities.h>
#include <DgnPlatform/DgnCore/ColorUtil.h>

#if defined (_MSC_VER)
#pragma warning (disable:4702)
#endif

USING_NAMESPACE_BENTLEY_SQLITE

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
    DgnTrueColorId colorId = colorTable.Insert(colorDef, "TestName1", "TestBook1");

    DgnColors::Iterator iter = colorTable.MakeIterator();
    ASSERT_EQ(1, iter.QueryCount());

    EXPECT_TRUE(colorId.IsValid());
    EXPECT_EQ(colorId.GetValue(), colorTable.FindMatchingColor(colorDef).GetValue());

    ColorDef toFind;
    EXPECT_EQ(SUCCESS, colorTable.QueryColor(toFind, nullptr, nullptr, colorId));
    EXPECT_TRUE(toFind == colorDef);

    ColorDef toFind1;
    EXPECT_EQ(SUCCESS, colorTable.QueryColorByName(toFind1, "TestName1", "TestBook1"));
    EXPECT_TRUE(toFind1 == colorDef);
    }

