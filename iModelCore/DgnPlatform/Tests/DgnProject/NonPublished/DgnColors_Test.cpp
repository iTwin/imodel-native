/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnColors_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Umar.Hayat   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnColorTests : public ::testing::Test
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
* @bsimethod                                    Keith.Bentley                   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnColorTests, TrueColors)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", Db::OpenMode::ReadWrite);
    DgnColors& colors = m_project->Colors();

    DgnColors::Color color1(ColorDef(255, 254, 253), "TestName1", "TestBook1");
    DgnTrueColorId colorId = colors.Insert(color1);
    EXPECT_TRUE(colorId.IsValid());
    EXPECT_TRUE(colorId == color1.GetId());

    DgnColors::Color color2(ColorDef(2,3,33), "Color2");
    DgnTrueColorId colorId2 = colors.Insert(color2);
    EXPECT_TRUE(colorId2.IsValid());

    DgnColors::Color color3(ColorDef(2,3,33), "Color3"); // it is legal to have two colors with the same value
    DgnTrueColorId colorId3 = colors.Insert(color3); 
    EXPECT_TRUE(colorId3.IsValid());

    DgnTrueColorId dup3 = colors.Insert(color3); 
    EXPECT_TRUE(!dup3.IsValid());

    DgnColors::Color color4(ColorDef(4,3,33), "Color4");
    DgnTrueColorId colorId4 = colors.Insert(color4); 
    EXPECT_TRUE(colorId4.IsValid());

    DgnColors::Color dupColor(ColorDef(5,54,3), "TestName1", "TestBook1");
    DgnTrueColorId dupColorId = colors.Insert(dupColor);
    EXPECT_TRUE(!dupColorId.IsValid());

    EXPECT_TRUE(4 == colors.MakeIterator().QueryCount());

    int i=0;
    for (auto& it : colors.MakeIterator())
        {
        if (it.GetId() == color1.GetId())
            {
            ++i;
            EXPECT_TRUE(it.GetColor() == color1.GetColor());
            EXPECT_TRUE(it.GetName() == color1.GetName());
            EXPECT_TRUE(it.GetBook() == color1.GetBook());
            }
        else if (it.GetId() == color2.GetId())
            {
            ++i;
            EXPECT_TRUE(it.GetColor() == color2.GetColor());
            EXPECT_TRUE(it.GetName() == color2.GetName());
            EXPECT_TRUE(it.GetBook() == color2.GetBook());
            }
        else if (it.GetId() == color3.GetId())
            {
            ++i;
            EXPECT_TRUE(it.GetColor() == color3.GetColor());
            EXPECT_TRUE(it.GetName() == color3.GetName());
            EXPECT_TRUE(it.GetBook() == color3.GetBook());
            }
        else if (it.GetId() == color4.GetId())
            {
            ++i;
            EXPECT_TRUE(it.GetColor() == color4.GetColor());
            EXPECT_TRUE(it.GetName() == color4.GetName());
            EXPECT_TRUE(it.GetBook() == color4.GetBook());
            }
        else
            EXPECT_TRUE(false); // too many entries in iterator
        }

    EXPECT_TRUE(4 == i);

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

    // No match Case
    EXPECT_FALSE(colors.FindMatchingColor(ColorDef(120, 120, 120)).IsValid());
    // Color with same definition
    EXPECT_TRUE(colors.FindMatchingColor(ColorDef(2, 3, 33)).IsValid());

    }