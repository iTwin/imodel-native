/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnMaterial_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"

USING_NAMESPACE_BENTLEY_SQLITE

/*---------------------------------------------------------------------------------**//**
* Test fixture for testing DgnColors
* @bsimethod                                    Algirdas.Mikoliunas            03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnMaterialsTest : public ::testing::Test
{
public:
    ScopedDgnHost  m_host;
    DgnDbPtr       m_project;

    void SetupProject(WCharCP projFile, Db::OpenMode mode)
        {
        DgnDbTestDgnManager tdm(projFile, __FILE__, mode);
        m_project = tdm.GetDgnProjectP();
        ASSERT_TRUE(m_project != NULL);
        }
};

/*---------------------------------------------------------------------------------**//**
* Insert true color
* @bsimethod                                    Algirdas.Mikoliunas            03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnMaterialsTest, InsertTrueColor)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", Db::OpenMode::ReadWrite);
    DgnColors& colors = m_project->Colors();

    DgnColors::Color color1(ColorDef(255, 254, 253), "TestName1", "TestBook1");
    DgnTrueColorId colorId = colors.Insert(color1);
    EXPECT_TRUE(colorId.IsValid());
    EXPECT_TRUE(colorId == color1.GetId());

    DgnColors::Color color2(ColorDef(5,54,3), "TestName1", "TestBook1");
    DgnTrueColorId colorId2 = colors.Insert(color2);
    EXPECT_TRUE(!colorId2.IsValid());

    DgnColors::Color color3(ColorDef(2,3,33));
    DgnTrueColorId colorId3 = colors.Insert(color3);
    EXPECT_TRUE(colorId3.IsValid());

    DgnColors::Color color4(ColorDef(2,3,33));
    DgnTrueColorId colorId4 = colors.Insert(color4);
    EXPECT_TRUE(colorId4.IsValid());

    auto iter = colors.MakeIterator();
    EXPECT_TRUE(3 == iter.QueryCount());

    EXPECT_TRUE(colorId == colors.FindMatchingColor(color1.GetColor()));

    DgnColors::Color toFind = colors.QueryColor(colorId);
    EXPECT_TRUE(toFind.IsValid());
    EXPECT_TRUE(toFind.GetId() == color1.GetId());
    EXPECT_TRUE(toFind.GetColor() == color1.GetColor());
    EXPECT_TRUE(toFind.GetName() == color1.GetName());
    EXPECT_TRUE(toFind.GetBook() == color1.GetBook());

    toFind = colors.QueryColorByName("TestName1", "TestBook1");
    EXPECT_TRUE(toFind.IsValid());
    EXPECT_TRUE(toFind.GetId() == color1.GetId());
    EXPECT_TRUE(toFind.GetColor() == color1.GetColor());
    EXPECT_TRUE(toFind.GetName() == color1.GetName());
    EXPECT_TRUE(toFind.GetBook() == color1.GetBook());
    }

