/*--------------------------------------------------------------------------------------+       22
|
|  $Source: Tests/DgnProject/NonPublished/DgnFonts_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
    SetupProject(L"3dMetricGeneral.ibim", L"CRUD_DbFontMapDirect.ibim", BeSQLite::Db::OpenMode::ReadWrite);

    DgnFonts::DbFontMapDirect& map = m_db->Fonts().DbFontMap();

    EXPECT_TRUE(0 == map.MakeIterator().QueryCount());

    BeFileName shxFilepath;
    ASSERT_TRUE( SUCCESS == DgnDbTestDgnManager::GetTestDataOut(shxFilepath, L"Fonts\\Cdm.shx", L"Cdm.shx", __FILE__) )<<"Unable to test file";
    DgnFontPtr shxFont = DgnFontPersistence::File::FromShxFile(shxFilepath);

    BeFileName ttfFontPath;
    ASSERT_TRUE(SUCCESS == DgnDbTestDgnManager::GetTestDataOut(ttfFontPath, L"Fonts\\Karla-Regular.ttf", L"Karla-Regular.ttf", __FILE__)) << "Unable to test file";
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

    toFind = map.QueryByTypeAndName(DgnFontType::Shx, "NotExist");
    EXPECT_TRUE(!toFind.IsValid());

    toFind = map.QueryByTypeAndName(DgnFontType::Shx, "Cdm");
    EXPECT_TRUE(toFind.IsValid());

    EXPECT_TRUE(map.ExistsById(fontId1));
    EXPECT_TRUE(map.ExistsByTypeAndName(DgnFontType::Shx,"Cdm"));

    DgnFontId idToFind = map.QueryIdByTypeAndName(DgnFontType::Shx, "Cdm");
    EXPECT_TRUE(idToFind.IsValid());

    idToFind = map.QueryIdByTypeAndName(DgnFontType::Shx, "NotExist");
    EXPECT_TRUE(!idToFind.IsValid());


    // Update 
    DgnShxFontP shxFont2 = (DgnShxFontP)shxFont.get();
    ASSERT_TRUE(nullptr != shxFont2);
    DgnShxFont::Metadata shxMetadata = shxFont2->GetMetadataR();
    shxMetadata.m_codePage = LangCodePage::Unicode;
    EXPECT_TRUE(SUCCESS == map.Update(*shxFont, fontId1));
    toFind = map.QueryById(fontId1);
    EXPECT_TRUE(toFind.IsValid());
    DgnShxFont::Metadata shxMetadataUpdated = ((DgnShxFontP)toFind.get())->GetMetadataR();
    EXPECT_TRUE(shxFont->GetName() == toFind->GetName());
    EXPECT_TRUE(shxFont->GetType() == toFind->GetType()); 
    EXPECT_TRUE(LangCodePage::Unicode == shxMetadataUpdated.m_codePage);

    // Delete
    //
    EXPECT_EQ(SUCCESS, map.Delete(fontId1));
    EXPECT_FALSE(map.ExistsById(fontId1));
    EXPECT_TRUE(1 == map.MakeIterator().QueryCount());

    }
    /*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat     09/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FontTests, QueryFonts)
    {
    SetupProject(L"3dMetricGeneral.ibim", L"QueryFonts.ibim", BeSQLite::Db::OpenMode::ReadWrite);

    DgnFonts::DbFontMapDirect& map = m_db->Fonts().DbFontMap();

    BeFileName shxFilepath;
    ASSERT_TRUE( SUCCESS == DgnDbTestDgnManager::GetTestDataOut(shxFilepath, L"Fonts\\Cdm.shx", L"Cdm.shx", __FILE__) )<<"Unable to test file";
    DgnFontPtr shxFont = DgnFontPersistence::File::FromShxFile(shxFilepath);

    DgnFontId fontId1;
    EXPECT_TRUE ( SUCCESS == map.Insert(*shxFont, fontId1) );
    EXPECT_TRUE(fontId1.IsValid());

    // Query
    //--------------------------------------------------------------------------------------
    DgnFontPtr toFind = map.QueryById(fontId1);
    EXPECT_TRUE(toFind.IsValid());
    EXPECT_TRUE(shxFont->GetName() == toFind->GetName());
    EXPECT_TRUE(shxFont->GetType() == toFind->GetType());

    //--------------------------------------------------------------------------------------
    DgnFontCP toFindP = m_db->Fonts().FindFontById(fontId1);
    EXPECT_TRUE(nullptr != toFindP);
    if (toFindP)
        EXPECT_TRUE(shxFont->GetName() == toFindP->GetName());

    //--------------------------------------------------------------------------------------
    toFindP = m_db->Fonts().FindFontByTypeAndName(DgnFontType::Shx, "Cdm");
    EXPECT_TRUE(nullptr != toFindP);
    if (toFindP)
        EXPECT_TRUE(shxFont->GetName() == toFindP->GetName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat     09/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FontTests, CRUD_DbFaceDataDirect)
    {
    SetupProject(L"3dMetricGeneral.ibim", L"CRUD_DbFaceDataDirect.ibim", BeSQLite::Db::OpenMode::ReadWrite);

    DgnFonts::DbFaceDataDirect& faceData = m_db->Fonts().DbFaceData();

    EXPECT_TRUE(0 == faceData.MakeIterator().QueryCount());

    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat     09/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FontTests, DgnFontManager)
    {
    SetupProject(L"3dMetricGeneral.ibim", L"DgnFontManager.ibim", BeSQLite::Db::OpenMode::ReadWrite);

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
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FontTests, EmbedTTFont)
    {
    SetupProject(L"3dMetricGeneral.ibim", L"EmbedTTFont.ibim", BeSQLite::Db::OpenMode::ReadWrite);

    DgnFonts& dbFonts = m_db->Fonts();

    BeFileName ttfFontPath;
    ASSERT_TRUE(SUCCESS == DgnDbTestDgnManager::GetTestDataOut(ttfFontPath, L"Fonts\\Karla-Regular.ttf", L"Karla-Regular.ttf", __FILE__)) << "Unable to test file";
    bvector<BeFileName> pathList;
    pathList.push_back(ttfFontPath);
    T_DgnFontPtrs ttfFontList = DgnFontPersistence::File::FromTrueTypeFiles(pathList, nullptr);
    ASSERT_TRUE(1 == ttfFontList.size());
    DgnFontPtr ttfFont = ttfFontList.at(0);
    
    // Insert
    // 
    DgnFontId fontId2;
    EXPECT_TRUE(SUCCESS == dbFonts.DbFontMap().Insert(*ttfFont, fontId2));
    EXPECT_TRUE(fontId2.IsValid());

    EXPECT_FALSE(DgnFontPersistence::Db::IsAnyFaceEmbedded(*ttfFont, dbFonts.DbFaceData()));

    EXPECT_TRUE(SUCCESS == DgnFontPersistence::Db::Embed(dbFonts.DbFaceData(), *ttfFont));
    EXPECT_TRUE(DgnFontPersistence::Db::IsAnyFaceEmbedded(*ttfFont, dbFonts.DbFaceData()));

    EXPECT_TRUE(DbResult::BE_SQLITE_OK ==  m_db->SaveChanges("Font embedded"));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FontTests, EmbedSHxFont)
    {
    SetupProject(L"3dMetricGeneral.ibim", L"EmbedSHxFont.ibim", BeSQLite::Db::OpenMode::ReadWrite);

    DgnFonts& dbFonts = m_db->Fonts();

    BeFileName shxFilepath;
    ASSERT_TRUE(SUCCESS == DgnDbTestDgnManager::GetTestDataOut(shxFilepath, L"Fonts\\Cdm.shx", L"Cdm.shx", __FILE__)) << "Unable to test file";
    DgnFontPtr shxFont = DgnFontPersistence::File::FromShxFile(shxFilepath);

    // Insert
    // 
    DgnFontId fontId1;
    EXPECT_TRUE(SUCCESS == dbFonts.DbFontMap().Insert(*shxFont, fontId1));
    EXPECT_TRUE(fontId1.IsValid());

    EXPECT_FALSE(DgnFontPersistence::Db::IsAnyFaceEmbedded(*shxFont, dbFonts.DbFaceData()));

    EXPECT_TRUE(SUCCESS == DgnFontPersistence::Db::Embed(dbFonts.DbFaceData(), *shxFont));
    EXPECT_TRUE(DgnFontPersistence::Db::IsAnyFaceEmbedded(*shxFont, dbFonts.DbFaceData()));

    EXPECT_TRUE(DbResult::BE_SQLITE_OK ==  m_db->SaveChanges("Font embedded"));
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
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FontTests, EmbeddedInUserDefinedFontTable)
    {
    SetupProject(L"3dMetricGeneral.ibim", L"EmbedTTFont.ibim", BeSQLite::Db::OpenMode::ReadWrite);

    DgnFonts dbFonts(*m_db, "TestFontTable");
    ASSERT_TRUE( SUCCESS  == dbFonts.DbFontMap().CreateFontTable());

    BeFileName ttfFontPath;
    ASSERT_TRUE(SUCCESS == DgnDbTestDgnManager::GetTestDataOut(ttfFontPath, L"Fonts\\Karla-Regular.ttf", L"Karla-Regular.ttf", __FILE__)) << "Unable to test file";
    bvector<BeFileName> pathList;
    pathList.push_back(ttfFontPath);
    T_DgnFontPtrs ttfFontList = DgnFontPersistence::File::FromTrueTypeFiles(pathList, nullptr);
    ASSERT_TRUE(1 == ttfFontList.size());
    DgnFontPtr ttfFont = ttfFontList.at(0);
    
    // Insert
    // 
    DgnFontId fontId2;
    EXPECT_TRUE(SUCCESS == dbFonts.DbFontMap().Insert(*ttfFont, fontId2));
    EXPECT_TRUE(fontId2.IsValid());

    EXPECT_FALSE(DgnFontPersistence::Db::IsAnyFaceEmbedded(*ttfFont, dbFonts.DbFaceData()));

    EXPECT_TRUE(SUCCESS == DgnFontPersistence::Db::Embed(dbFonts.DbFaceData(), *ttfFont));
    EXPECT_TRUE(DgnFontPersistence::Db::IsAnyFaceEmbedded(*ttfFont, dbFonts.DbFaceData()));

    EXPECT_TRUE(DbResult::BE_SQLITE_OK ==  m_db->SaveChanges("Font embedded"));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar.Hayat     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FontTests, DbFaceDataDirect)
    {
    SetupProject(L"3dMetricGeneral.ibim", L"DbFaceDataDirect.ibim", BeSQLite::Db::OpenMode::ReadWrite);

    DgnFonts& dbFonts = m_db->Fonts();
    DgnFonts::DbFaceDataDirect& faceData = dbFonts.DbFaceData();
    const static int FontDataSize = 10;
    Byte fontDummyData[FontDataSize] = {1,2,3,4,5,6,7,8,9,10};

    //------------------------------------------------------------------------------------------------------------------
    // Insert
    DgnFonts::DbFaceDataDirect::FaceKey key1a(DgnFontType::TrueType, "Exton Fonts", DgnFonts::DbFaceDataDirect::FaceKey::FACE_NAME_Regular);
    DgnFonts::DbFaceDataDirect::FaceKey key1b(DgnFontType::TrueType, "Exton Fonts", DgnFonts::DbFaceDataDirect::FaceKey::FACE_NAME_Bold);
    DgnFonts::DbFaceDataDirect::FaceKey key1c(DgnFontType::TrueType, "Exton Fonts", DgnFonts::DbFaceDataDirect::FaceKey::FACE_NAME_Italic);
    DgnFonts::DbFaceDataDirect::FaceKey key1d(DgnFontType::TrueType, "Exton Fonts", DgnFonts::DbFaceDataDirect::FaceKey::FACE_NAME_BoldItalic);
    DgnFonts::DbFaceDataDirect::FaceKey key2a(DgnFontType::Shx, "Islamabad Fonts", DgnFonts::DbFaceDataDirect::FaceKey::FACE_NAME_Regular);
    DgnFonts::DbFaceDataDirect::FaceKey key3a(DgnFontType::Rsc, "Islamabad Fonts", DgnFonts::DbFaceDataDirect::FaceKey::FACE_NAME_BoldItalic);
    DgnFonts::DbFaceDataDirect::FaceKey unusedKey(DgnFontType::TrueType, "UnUsed Fonts", DgnFonts::DbFaceDataDirect::FaceKey::FACE_NAME_BoldItalic);
    DgnFonts::DbFaceDataDirect::T_FaceMap faceMap1;
    DgnFonts::DbFaceDataDirect::T_FaceMap faceMap2;
    DgnFonts::DbFaceDataDirect::T_FaceMap faceMap3;
    faceMap1.Insert(0, key1a);
    faceMap1.Insert(1, key1b);
    faceMap1.Insert(2, key1c);
    faceMap1.Insert(3, key1d);
    faceMap2.Insert(0, key2a);
    faceMap3.Insert(0, key3a);
    EXPECT_TRUE(SUCCESS == faceData.Insert(fontDummyData, FontDataSize, faceMap1));
    EXPECT_TRUE(SUCCESS == faceData.Insert(fontDummyData, FontDataSize, faceMap2));
    EXPECT_TRUE(SUCCESS == faceData.Insert(fontDummyData, FontDataSize, faceMap3));
    EXPECT_EQ(3, faceData.MakeIterator().QueryCount());
    
    // Exist
    EXPECT_TRUE(faceData.Exists(key1b));
    EXPECT_TRUE(faceData.Exists(key2a));
    EXPECT_FALSE(faceData.Exists(unusedKey));

    int count = 0;
    //------------------------------------------------------------------------------------------------------------------
    // Query
    for (DgnFonts::DbFaceDataDirect::Iterator::Entry entry : faceData.MakeIterator())
        {
        count++;
        }
    EXPECT_EQ(3, count);

    // Delete
    EXPECT_TRUE (SUCCESS == faceData.Delete(key1d));
    EXPECT_FALSE(faceData.Exists(key1d));
    EXPECT_TRUE(SUCCESS == faceData.Delete(key3a));
    EXPECT_FALSE(faceData.Exists(key3a));
    // Double Delete
    EXPECT_TRUE(SUCCESS != faceData.Delete(key1d));

    m_db->SaveChanges();
    }
