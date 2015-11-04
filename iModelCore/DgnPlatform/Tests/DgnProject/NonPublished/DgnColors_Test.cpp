/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnColors_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnTrueColor.h>

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
    DgnDbR db = *m_project;

    DgnTrueColor color1(DgnTrueColor::CreateParams(db, ColorDef(255, 254, 253), "TestName1", "TestBook1"));
    EXPECT_TRUE(color1.Insert().IsValid());
    DgnTrueColorId colorId = color1.GetColorId();
    EXPECT_TRUE(colorId.IsValid());

    DgnTrueColor color2(DgnTrueColor::CreateParams(db, ColorDef(2, 3, 33), "Color2"));
    EXPECT_TRUE(color2.Insert().IsValid());
    auto colorId2 = color2.GetColorId();
    EXPECT_TRUE(colorId2.IsValid());

    // It is legal to have two colors with the same RGB value
    DgnTrueColor color3(DgnTrueColor::CreateParams(db, ColorDef(2,3,33), "Color3"));
    EXPECT_TRUE(color3.Insert().IsValid());
    auto colorId3 = color3.GetColorId();
    EXPECT_TRUE(colorId3.IsValid());

    // It is not legal to have two colors with the same book+color name
    DgnTrueColor color3_dup(DgnTrueColor::CreateParams(db, ColorDef(2,3,33), "Color3"));
    EXPECT_FALSE(color3_dup.Insert().IsValid());

    DgnTrueColor color1_dup(DgnTrueColor::CreateParams(db, ColorDef(5,54,3), "TestName1", "TestBook1"));
    EXPECT_FALSE(color1_dup.Insert().IsValid());

    DgnTrueColor color4(DgnTrueColor::CreateParams(db, ColorDef(4,3,33), "Color4"));
    EXPECT_TRUE(color4.Insert().IsValid());
    auto colorId4 = color4.GetColorId();
    EXPECT_TRUE(colorId4.IsValid());

    EXPECT_EQ(4, DgnTrueColor::QueryCount(db));

    int i=0;
    for (auto& it : DgnTrueColor::MakeIterator(db))
        {
        if (it.GetId() == color1.GetColorId())
            {
            ++i;
            EXPECT_TRUE(it.GetColorDef() == color1.GetColorDef());
            EXPECT_TRUE(it.GetName() == color1.GetName());
            EXPECT_TRUE(it.GetBook() == color1.GetBook());
            }
        else if (it.GetId() == color2.GetColorId())
            {
            ++i;
            EXPECT_TRUE(it.GetColorDef() == color2.GetColorDef());
            EXPECT_TRUE(it.GetName() == color2.GetName());
            EXPECT_TRUE(it.GetBook() == color2.GetBook());
            }
        else if (it.GetId() == color3.GetColorId())
            {
            ++i;
            EXPECT_TRUE(it.GetColorDef() == color3.GetColorDef());
            EXPECT_TRUE(it.GetName() == color3.GetName());
            EXPECT_TRUE(it.GetBook() == color3.GetBook());
            }
        else if (it.GetId() == color4.GetColorId())
            {
            ++i;
            EXPECT_TRUE(it.GetColorDef() == color4.GetColorDef());
            EXPECT_TRUE(it.GetName() == color4.GetName());
            EXPECT_TRUE(it.GetBook() == color4.GetBook());
            }
        else
            EXPECT_TRUE(false); // too many entries in iterator
        }

    EXPECT_TRUE(4 == i);

    DgnTrueColorId matchingColorId = DgnTrueColor::FindMatchingColor(color1.GetColorDef(), db);
    EXPECT_TRUE(matchingColorId.IsValid());
    EXPECT_TRUE(color1.GetColorId() == matchingColorId);

    DgnTrueColorCPtr toFind = DgnTrueColor::QueryColor(colorId, db);
    EXPECT_TRUE(toFind.IsValid());
    EXPECT_TRUE(toFind->GetColorId() == color1.GetColorId());
    EXPECT_TRUE(toFind->GetColorDef() == color1.GetColorDef());
    EXPECT_TRUE(toFind->GetName() == color1.GetName());
    EXPECT_TRUE(toFind->GetBook() == color1.GetBook());

    toFind = DgnTrueColor::QueryColorByName("TestName1", "TestBook1", db);
    EXPECT_TRUE(toFind.IsValid());
    EXPECT_TRUE(toFind->GetColorId() == color1.GetColorId());
    EXPECT_TRUE(toFind->GetColorDef() == color1.GetColorDef());
    EXPECT_TRUE(toFind->GetName() == color1.GetName());
    EXPECT_TRUE(toFind->GetBook() == color1.GetBook());

    // No match Case
    EXPECT_FALSE(DgnTrueColor::FindMatchingColor(ColorDef(120, 120, 120), db).IsValid());
    // Color with same definition
    EXPECT_TRUE(DgnTrueColor::FindMatchingColor(ColorDef(2, 3, 33), db).IsValid());

    // Cannot update or delete
    auto cpColor4 = DgnTrueColor::QueryColor(colorId4, db);
    ASSERT_TRUE(cpColor4.IsValid());
    EXPECT_EQ(DgnDbStatus::DeletionProhibited, cpColor4->Delete());
    auto pColor4 = cpColor4->MakeCopy<DgnTrueColor>();
    DgnDbStatus updateStat;
    EXPECT_FALSE(pColor4->Update(&updateStat).IsValid());
    EXPECT_EQ(DgnDbStatus::WrongElement, updateStat);
    }

