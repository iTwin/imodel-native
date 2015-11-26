/*--------------------------------------------------------------------------------------+       22
|
|  $Source: Tests/DgnProject/NonPublished/DgnFonts_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../TestFixture/DgnDbTestFixtures.h"
#include <DgnPlatform/DgnFontData.h>

/*---------------------------------------------------------------------------------**//**
* Test fixture for testing DgnFonts
* @bsimethod                                                    Umar.Hayat      09/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct FontTests : public DgnDbTestFixture
{

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat     09/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FontTests, CRUD_DbFontMapDirect)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"CRUD_DbFontMapDirect.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    DgnFonts::DbFontMapDirect& map = m_db->Fonts().DbFontMap();

    EXPECT_TRUE(0 == map.MakeIterator().QueryCount());

    BeFileName shxFilepath;
    ASSERT_TRUE( SUCCESS == DgnDbTestDgnManager::GetTestDataOut(shxFilepath, L"Fonts\\Cdm.shx", L"Cdm.shx", __FILE__) )<<"Unable to test file";
    DgnFontPtr shxFont = DgnFontPersistence::File::FromShxFile(shxFilepath);

    BeFileName ttfFontPath;
    ASSERT_TRUE(SUCCESS == DgnDbTestDgnManager::GetTestDataOut(ttfFontPath, L"Fonts\\Teleindicadores1.ttf", L"Teleindicadores1.ttf", __FILE__)) << "Unable to test file";
    bvector<BeFileName> pathList;
    pathList.push_back(ttfFontPath);
    T_DgnFontPtrs ttfFontList = DgnFontPersistence::File::FromTrueTypeFiles(pathList, nullptr);
    ASSERT_TRUE(1 == ttfFontList.size());
    DgnFontPtr ttfFont = ttfFontList.at(0);
    
    // Insert
    // 
    DgnFontId fontId1;
    EXPECT_TRUE ( SUCCESS == map.Insert(*shxFont, fontId1) );
    EXPECT_TRUE(fontId1.IsValid());
    DgnFontId fontId2;
    EXPECT_TRUE(SUCCESS == map.Insert(*ttfFont, fontId2));
    EXPECT_TRUE(fontId2.IsValid());

    // ToDo: insert RSC as well
    
    // Insert Duplicate 
    BeTest::SetFailOnAssert(false);
    DgnFontId fontId3;
    EXPECT_FALSE ( SUCCESS == map.Insert(*shxFont, fontId3) );
    //EXPECT_FALSE (fontId3.IsValid());
    BeTest::SetFailOnAssert(true);
    
    EXPECT_TRUE(2 == map.MakeIterator().QueryCount());
    int count = 0;
    for (auto iter : map.MakeIterator())
    {
        if (iter.GetId() == fontId1)
        {
            EXPECT_TRUE(shxFont->GetName() == iter.GetName());
            EXPECT_TRUE(shxFont->GetType() == iter.GetType());
        }
        else if (iter.GetId() == fontId2)
        {
            EXPECT_TRUE(ttfFont->GetName() == iter.GetName());
            EXPECT_TRUE(ttfFont->GetType() == iter.GetType());
        }
        else
            EXPECT_TRUE(false) << "This font should not be here ";
        count++;
    }
    EXPECT_TRUE(2 == count);

    // Query
    //
    DgnFontPtr toFind = map.QueryById(fontId1);
    EXPECT_TRUE(toFind.IsValid());
    EXPECT_TRUE(shxFont->GetName() == toFind->GetName());
    EXPECT_TRUE(shxFont->GetType() == toFind->GetType());

    toFind = map.QueryByTypeAndName(DgnFontType::Shx, "Cdm");
    EXPECT_TRUE(toFind.IsValid());

    EXPECT_TRUE(map.ExistsById(fontId1));
    EXPECT_TRUE(map.ExistsByTypeAndName(DgnFontType::Shx,"Cdm"));

    DgnFontId idToFind = map.QueryIdByTypeAndName(DgnFontType::Shx, "Cdm");
    EXPECT_TRUE(idToFind.IsValid());

    // Update 
    //
    
    //
    //EXPECT_TRUE(SUCCESS == map.Update(*shxFont, fontId1));
    //toFind = map.QueryById(fontId1);
    //EXPECT_TRUE(toFind.IsValid());
    //EXPECT_TRUE(shxFont->GetName() == toFind->GetName());
    //EXPECT_TRUE(shxFont->GetType() == toFind->GetType());

    // Delete
    //
    EXPECT_EQ(SUCCESS, map.Delete(fontId1));
    EXPECT_FALSE(map.ExistsById(fontId1));
    EXPECT_TRUE(1 == map.MakeIterator().QueryCount());

    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat     09/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FontTests, CRUD_DbFaceDataDirect)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"CRUD_DbFaceDataDirect.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    DgnFonts::DbFaceDataDirect& faceData = m_db->Fonts().DbFaceData();

    EXPECT_TRUE(0 == faceData.MakeIterator().QueryCount());

    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat     09/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FontTests, DgnFontManager)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"DgnFontManager.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    DgnFontCR font = DgnFontManager::GetLastResortTrueTypeFont();
    EXPECT_TRUE(font.IsResolved());
    EXPECT_TRUE(DgnFontType::TrueType == font.GetType());
    DgnFontCR font2 = DgnFontManager::GetLastResortRscFont();
    EXPECT_TRUE(font2.IsResolved());
    EXPECT_TRUE(DgnFontType::Rsc == font2.GetType());
    DgnFontCR font3 = DgnFontManager::GetLastResortShxFont();
    EXPECT_TRUE(font3.IsResolved());
    EXPECT_TRUE(DgnFontType::Shx == font3.GetType());
    DgnFontCR font4 = DgnFontManager::GetAnyLastResortFont();
    EXPECT_TRUE(font4.IsResolved());
    DgnFontCR font5 = DgnFontManager::GetDecoratorFont();
    EXPECT_TRUE(font5.IsResolved());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat     09/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FontTests, CreateMissingFont)
    {
    DgnFontPtr font = DgnFontPersistence::Missing::CreateMissingFont(DgnFontType::TrueType, "MissingTTF");
    EXPECT_TRUE(font.IsValid());
    EXPECT_TRUE(font->GetType() == DgnFontType::TrueType);
    EXPECT_TRUE(font->GetName() == "MissingTTF" );
    EXPECT_TRUE(!font->IsResolved());


    DgnFontPtr font2 = DgnFontPersistence::Missing::CreateMissingFont(DgnFontType::Shx, "MissingShx");
    EXPECT_TRUE(font2.IsValid());
    EXPECT_TRUE(font2->GetType() == DgnFontType::Shx);
    EXPECT_TRUE(font2->GetName() == "MissingShx");
    EXPECT_TRUE(!font2->IsResolved());


    DgnFontPtr font3 = DgnFontPersistence::Missing::CreateMissingFont(DgnFontType::Rsc, "MissingRsc");
    EXPECT_TRUE(font3.IsValid());
    EXPECT_TRUE(font3->GetType() == DgnFontType::Rsc);
    EXPECT_TRUE(font3->GetName() == "MissingRsc");
    EXPECT_TRUE(!font3->IsResolved());

    }
#if defined (BENTLEY_WIN32) // Windows Desktop-only
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat     09/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FontTests, RegistryFonts)
    {
    DgnFontPtr font = DgnFontPersistence::OS::FromGlobalTrueTypeRegistry("Arial");
    EXPECT_TRUE(font.IsValid());
    EXPECT_TRUE(font->GetType() == DgnFontType::TrueType);
    EXPECT_TRUE(font->GetName() == "Arial" );
    EXPECT_TRUE(font->IsResolved());
    }
#endif