/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/DgnProject/Published/DgnLink_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"

// Rebuild API:             bb re DgnPlatformDll
// Republish API:           bb re DgnPlatform:PublishedApi
// Rebuild test:            bb re DgnProjectUnitTests BeGTestExe
// All code:                bb re DgnPlatform:PublishedApi DgnPlatformDll DgnProjectUnitTests BeGTestExe
// Republish seed files:    bb re UnitTests_Documents
// Run test:                %SrcRoot%BeGTest\RunTests.py -ax64 --gtest_filter="DgnLinksTest.*"

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     02/2015
//=======================================================================================
class DgnLinkTest : public GenericDgnModelTestFixture
{
    public: DgnLinkTest () : GenericDgnModelTestFixture (__FILE__, false /*2D*/, false /*needBriefcase*/) {}

}; // DgnLinkTest

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
TEST_F(DgnLinkTest, RoundTripUrlLink)
    {
    //.............................................................................................
    ASSERT_TRUE(NULL != m_testDgnManager.GetDgnProjectP());
    DgnDbR db = *m_testDgnManager.GetDgnProjectP();

    DgnModelId modelId = db.Models().QueryFirstModelId();
    ASSERT_TRUE(modelId.IsValid());

    DgnModelPtr modelP = db.Models().GetModel(modelId);
    ASSERT_TRUE(modelP.IsValid());

    DgnCategoryId categoryId = DgnCategory::QueryFirstCategoryId(db);
    ASSERT_TRUE(categoryId.IsValid());

    DgnClassId elementClassId = DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_AnnotationElement2d));
    ASSERT_TRUE(elementClassId.IsValid());

    AnnotationElement2dPtr elementPtr = AnnotationElement2d::Create(AnnotationElement2d::CreateParams(db, modelId, elementClassId, categoryId));
    ASSERT_TRUE(elementPtr.IsValid());
    DgnElementCPtr result = db.Elements().Insert(*elementPtr);
    ASSERT_TRUE(result.IsValid());
    
    static const Utf8CP LINK1_DISPLAY_LABEL = "Url Link 1";
    static const Utf8CP LINK1_URL = "http://www.google.com";
    
    DgnLinkPtr link1 = DgnLink::Create(db);
    link1->SetDisplayLabel(LINK1_DISPLAY_LABEL);
    link1->SetUrl(LINK1_URL);

    EXPECT_TRUE(DgnLinkType::Url == link1->GetType());
    EXPECT_FALSE(link1->GetId().IsValid());
    ASSERT_TRUE(SUCCESS == db.Links().InsertOnElement(result->GetElementId(), *link1));
    ASSERT_TRUE(link1->GetId().IsValid());

    DgnLinkPtr link1b = db.Links().QueryById(link1->GetId());
    ASSERT_TRUE(link1b.IsValid());
    EXPECT_TRUE(link1b->GetId() == link1->GetId());
    ASSERT_TRUE(DgnLinkType::Url == link1b->GetType());
    EXPECT_TRUE(0 == strcmp(link1b->GetDisplayLabel().c_str(), LINK1_DISPLAY_LABEL));
    
    Utf8String link1bUrl;
    EXPECT_TRUE(SUCCESS == link1b->GetUrl(link1bUrl));
    EXPECT_TRUE(0 == strcmp(link1bUrl.c_str(), LINK1_URL));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
TEST_F(DgnLinkTest, Iterator)
    {
    //.............................................................................................
    ASSERT_TRUE(NULL != m_testDgnManager.GetDgnProjectP());
    DgnDbR db = *m_testDgnManager.GetDgnProjectP();

    DgnModelId modelId = db.Models().QueryFirstModelId();
    ASSERT_TRUE(modelId.IsValid());

    DgnModelPtr modelP = db.Models().GetModel(modelId);
    ASSERT_TRUE(modelP.IsValid());

    DgnCategoryId categoryId = DgnCategory::QueryFirstCategoryId(db);
    ASSERT_TRUE(categoryId.IsValid());

    DgnClassId elementClassId = DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_AnnotationElement2d));
    ASSERT_TRUE(elementClassId.IsValid());

    AnnotationElement2dPtr elementPtr = AnnotationElement2d::Create(AnnotationElement2d::CreateParams(db, modelId, elementClassId, categoryId));
    ASSERT_TRUE(elementPtr.IsValid());
    DgnElementCPtr result = db.Elements().Insert(*elementPtr);
    ASSERT_TRUE(result->GetElementId().IsValid());
    
    //.............................................................................................
    static const size_t NUM_LINKS = 5;

    bvector<DgnLinkPtr> links;
    for (size_t iLink = 0; iLink < NUM_LINKS; ++iLink)
        {
        DgnLinkPtr link = DgnLink::Create(db);
        link->SetDisplayLabel(Utf8PrintfString("DisplayLabel %" PRIu64, (uint64_t)iLink).c_str());
        link->SetUrl("URL");

        links.push_back(link);
        }

    //.............................................................................................
    EXPECT_TRUE(0 == db.Links().MakeIterator().QueryCount());
    ASSERT_TRUE(SUCCESS == db.Links().InsertOnElement(result->GetElementId(), *links[0]));
    EXPECT_TRUE(1 == db.Links().MakeIterator().QueryCount());

    //.............................................................................................
    for (size_t iLink = 1; iLink < NUM_LINKS; ++iLink)
        ASSERT_TRUE(SUCCESS == db.Links().InsertOnElement(result->GetElementId(), *links[iLink]));

    EXPECT_TRUE(NUM_LINKS == db.Links().MakeIterator().QueryCount());

    //.............................................................................................
    DgnLinks::Iterator customColl1 = db.Links().MakeIterator();
    customColl1.Params().SetWhere("DisplayLabel LIKE '%2'");

    EXPECT_TRUE(1 == customColl1.QueryCount());
    size_t numFoundLinks = 0;
    for (auto iter = customColl1.begin(); customColl1.end() != iter; ++iter)
        ++numFoundLinks;

    EXPECT_TRUE(1 == numFoundLinks);
    
    //.............................................................................................
    db.Links().DeleteFromElement(result->GetElementId(), links[0]->GetId());
    EXPECT_TRUE(NUM_LINKS == db.Links().MakeIterator().QueryCount());

    db.Links().PurgeUnused();
    EXPECT_TRUE((NUM_LINKS - 1) == db.Links().MakeIterator().QueryCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
template<typename COLLECTION> static void checkIteratorCount(COLLECTION const& coll, size_t expectedCount)
    {
    EXPECT_TRUE(expectedCount == coll.QueryCount());
    size_t actualCount = 0;
    for (auto iter = coll.begin(); coll.end() != iter; ++iter)
        ++actualCount;

    EXPECT_TRUE(expectedCount == actualCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
TEST_F(DgnLinkTest, OtherIterators)
    {
    ASSERT_TRUE(NULL != m_testDgnManager.GetDgnProjectP());
    DgnDbR db = *m_testDgnManager.GetDgnProjectP();

    DgnModelId modelId = db.Models().QueryFirstModelId();
    ASSERT_TRUE(modelId.IsValid());

    DgnModelPtr modelP = db.Models().GetModel(modelId);
    ASSERT_TRUE(modelP.IsValid());

    DgnCategoryId categoryId = DgnCategory::QueryFirstCategoryId(db);
    ASSERT_TRUE(categoryId.IsValid());

    DgnClassId elementClassId = DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_AnnotationElement2d));
    ASSERT_TRUE(elementClassId.IsValid());

    AnnotationElement2dPtr element1 = AnnotationElement2d::Create(AnnotationElement2d::CreateParams(db, modelId, elementClassId, categoryId));
    ASSERT_TRUE(element1.IsValid());
    DgnElementCPtr result1 = db.Elements().Insert(*element1);
    ASSERT_TRUE(result1.IsValid());

    AnnotationElement2dPtr element2 = AnnotationElement2d::Create(AnnotationElement2d::CreateParams(db, modelId, elementClassId, categoryId));
    ASSERT_TRUE(element2.IsValid());
    DgnElementCPtr result2 = db.Elements().Insert(*element2);
    ASSERT_TRUE(result2.IsValid());

    DgnLinkPtr link1 = DgnLink::Create(db); link1->SetDisplayLabel("link1"); link1->SetEmbeddedDocumentName("EmbeddedDocumentName1");
    DgnLinkPtr link2 = DgnLink::Create(db); link2->SetDisplayLabel("link2"); link2->SetEmbeddedDocumentName("EmbeddedDocumentName2");
    DgnLinkPtr link3 = DgnLink::Create(db); link3->SetDisplayLabel("link3"); link3->SetEmbeddedDocumentName("EmbeddedDocumentName3");
    DgnLinkPtr link4 = DgnLink::Create(db); link4->SetDisplayLabel("link4"); link4->SetEmbeddedDocumentName("EmbeddedDocumentName4");

    DgnElementId elementId1 = result1->GetElementId();
    DgnElementId elementId2 = result2->GetElementId();

    ASSERT_TRUE(SUCCESS == db.Links().InsertOnElement(elementId1, *link1));
    ASSERT_TRUE(SUCCESS == db.Links().InsertOnElement(elementId1, *link2));
    ASSERT_TRUE(SUCCESS == db.Links().InsertOnElement(elementId2, link2->GetId()));
    ASSERT_TRUE(SUCCESS == db.Links().InsertOnElement(elementId2, *link3));
    ASSERT_TRUE(SUCCESS == db.Links().InsertOnElement(elementId2, *link4));

    checkIteratorCount(db.Links().MakeIterator(), 4);
    
    checkIteratorCount(db.Links().MakeOnElementIterator(elementId1), 2);
    checkIteratorCount(db.Links().MakeOnElementIterator(elementId2), 3);

    checkIteratorCount(db.Links().MakeReferencesLinkIterator(link1->GetId()), 1);
    checkIteratorCount(db.Links().MakeReferencesLinkIterator(link2->GetId()), 2);
    checkIteratorCount(db.Links().MakeReferencesLinkIterator(link3->GetId()), 1);
    checkIteratorCount(db.Links().MakeReferencesLinkIterator(link4->GetId()), 1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Umar.Hayat          02/16
//---------------------------------------------------------------------------------------
TEST_F(DgnLinkTest, ExternalFileLink)
    {
    //.............................................................................................
    ASSERT_TRUE(NULL != m_testDgnManager.GetDgnProjectP());
    DgnDbR db = *m_testDgnManager.GetDgnProjectP();

    DgnModelId modelId = db.Models().QueryFirstModelId();
    ASSERT_TRUE(modelId.IsValid());

    DgnModelPtr modelP = db.Models().GetModel(modelId);
    ASSERT_TRUE(modelP.IsValid());

    DgnCategoryId categoryId = DgnCategory::QueryFirstCategoryId(db);
    ASSERT_TRUE(categoryId.IsValid());

    DgnClassId elementClassId = DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_AnnotationElement));
    ASSERT_TRUE(elementClassId.IsValid());

    AnnotationElementPtr elementPtr = AnnotationElement::Create(AnnotationElement::CreateParams(db, modelId, elementClassId, categoryId));
    ASSERT_TRUE(elementPtr.IsValid());
    DgnElementCPtr result = db.Elements().Insert(*elementPtr);
    ASSERT_TRUE(result.IsValid());
    
    static const Utf8CP LINK1_DISPLAY_LABEL = "External File Link";
    static const Utf8CP LINK1_EXTERNAL_FILE_PATH = "http://www.google.com"; 
    
    DgnLinkPtr link1 = DgnLink::Create(db);
    link1->SetDisplayLabel(LINK1_DISPLAY_LABEL);
    link1->SetExternalFilePaths(nullptr,LINK1_EXTERNAL_FILE_PATH,nullptr);

    EXPECT_TRUE(DgnLinkType::ExternalFile == link1->GetType());
    EXPECT_FALSE(link1->GetId().IsValid());
    ASSERT_TRUE(SUCCESS == db.Links().InsertOnElement(result->GetElementId(), *link1));
    ASSERT_TRUE(link1->GetId().IsValid());

    DgnLinkPtr link1b = db.Links().QueryById(link1->GetId());
    ASSERT_TRUE(link1b.IsValid());
    EXPECT_TRUE(link1b->GetId() == link1->GetId());
    ASSERT_TRUE(DgnLinkType::ExternalFile == link1b->GetType());
    EXPECT_TRUE(0 == strcmp(link1b->GetDisplayLabel().c_str(), LINK1_DISPLAY_LABEL));
    
    Utf8String link1NameDMS;
    Utf8String link1NamePortable;
    Utf8String link1NameLastKnownLocation;
    EXPECT_TRUE(SUCCESS == link1b->GetExternalFilePaths(&link1NameDMS, &link1NamePortable, &link1NameLastKnownLocation));
    EXPECT_TRUE(0 == strcmp(link1NamePortable.c_str(), LINK1_EXTERNAL_FILE_PATH));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Umar.Hayat          02/16
//---------------------------------------------------------------------------------------
TEST_F(DgnLinkTest, EmbeddedFileLink)
    {
    //.............................................................................................
    ASSERT_TRUE(NULL != m_testDgnManager.GetDgnProjectP());
    DgnDbR db = *m_testDgnManager.GetDgnProjectP();

    DgnModelId modelId = db.Models().QueryFirstModelId();
    ASSERT_TRUE(modelId.IsValid());

    DgnModelPtr modelP = db.Models().GetModel(modelId);
    ASSERT_TRUE(modelP.IsValid());

    DgnCategoryId categoryId = DgnCategory::QueryFirstCategoryId(db);
    ASSERT_TRUE(categoryId.IsValid());

    DgnClassId elementClassId = DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_AnnotationElement));
    ASSERT_TRUE(elementClassId.IsValid());

    AnnotationElementPtr elementPtr = AnnotationElement::Create(AnnotationElement::CreateParams(db, modelId, elementClassId, categoryId));
    ASSERT_TRUE(elementPtr.IsValid());
    DgnElementCPtr result = db.Elements().Insert(*elementPtr);
    ASSERT_TRUE(result.IsValid());
    
    static const Utf8CP LINK1_DISPLAY_LABEL = "External File Link";
    static const Utf8CP LINK1_EMBEDDED_FILE_NAME = "SomeEmbeddedDocument.pdf"; 
    
    DgnLinkPtr link1 = DgnLink::Create(db);
    link1->SetDisplayLabel(LINK1_DISPLAY_LABEL);
    link1->SetEmbeddedDocumentName(LINK1_EMBEDDED_FILE_NAME);

    EXPECT_TRUE(DgnLinkType::EmbeddedFile == link1->GetType());
    EXPECT_FALSE(link1->GetId().IsValid());
    ASSERT_TRUE(SUCCESS == db.Links().InsertOnElement(result->GetElementId(), *link1));
    ASSERT_TRUE(link1->GetId().IsValid());

    DgnLinkPtr link1b = db.Links().QueryById(link1->GetId());
    ASSERT_TRUE(link1b.IsValid());
    EXPECT_TRUE(link1b->GetId() == link1->GetId());
    ASSERT_TRUE(DgnLinkType::EmbeddedFile == link1b->GetType());
    EXPECT_TRUE(0 == strcmp(link1b->GetDisplayLabel().c_str(), LINK1_DISPLAY_LABEL));
    
    Utf8String link1DocName;
    EXPECT_TRUE(SUCCESS == link1b->GetEmbeddedDocumentName(link1DocName));
    EXPECT_TRUE(0 == strcmp(link1DocName.c_str(), LINK1_EMBEDDED_FILE_NAME));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Umar.Hayat          02/16
//---------------------------------------------------------------------------------------
TEST_F(DgnLinkTest, ViewLink)
    {
    //.............................................................................................
    ASSERT_TRUE(NULL != m_testDgnManager.GetDgnProjectP());
    DgnDbR db = *m_testDgnManager.GetDgnProjectP();

    DgnModelId modelId = db.Models().QueryFirstModelId();
    ASSERT_TRUE(modelId.IsValid());

    DgnModelPtr modelP = db.Models().GetModel(modelId);
    ASSERT_TRUE(modelP.IsValid());
    
    DgnCategoryId categoryId = DgnCategory::QueryFirstCategoryId(db);
    ASSERT_TRUE(categoryId.IsValid());
    
    DgnClassId elementClassId = DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_AnnotationElement));
    ASSERT_TRUE(elementClassId.IsValid());

    AnnotationElementPtr elementPtr = AnnotationElement::Create(AnnotationElement::CreateParams(db, modelId, elementClassId, categoryId));
    ASSERT_TRUE(elementPtr.IsValid());
    DgnElementCPtr result = db.Elements().Insert(*elementPtr);
    ASSERT_TRUE(result.IsValid());
    
    static const Utf8CP LINK1_DISPLAY_LABEL = "View Link";
    DgnViewId   viewId(2LLU);
    
    DgnLinkPtr link1 = DgnLink::Create(db);
    link1->SetDisplayLabel(LINK1_DISPLAY_LABEL);
    link1->SetViewId(viewId);

    EXPECT_TRUE(DgnLinkType::View == link1->GetType());
    EXPECT_FALSE(link1->GetId().IsValid());
    ASSERT_TRUE(SUCCESS == db.Links().InsertOnElement(result->GetElementId(), *link1));
    ASSERT_TRUE(link1->GetId().IsValid());

    DgnLinkPtr link1b = db.Links().QueryById(link1->GetId());
    ASSERT_TRUE(link1b.IsValid());
    EXPECT_TRUE(link1b->GetId() == link1->GetId());
    ASSERT_TRUE(DgnLinkType::View == link1b->GetType());
    EXPECT_TRUE(0 == strcmp(link1b->GetDisplayLabel().c_str(), LINK1_DISPLAY_LABEL));
    
    DgnViewId viewIdb;
    EXPECT_TRUE(SUCCESS == link1b->GetViewId(viewIdb));
    EXPECT_TRUE(viewIdb == viewId);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Umar.Hayat     02/2016
//---------------------------------------------------------------------------------------
TEST_F(DgnLinkTest, Update)
    {
    //.............................................................................................
    ASSERT_TRUE(NULL != m_testDgnManager.GetDgnProjectP());
    DgnDbR db = *m_testDgnManager.GetDgnProjectP();

    DgnModelId modelId = db.Models().QueryFirstModelId();
    ASSERT_TRUE(modelId.IsValid());

    DgnModelPtr modelP = db.Models().GetModel(modelId);
    ASSERT_TRUE(modelP.IsValid());

    DgnCategoryId categoryId = DgnCategory::QueryFirstCategoryId(db);
    ASSERT_TRUE(categoryId.IsValid());

    DgnClassId elementClassId = DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_AnnotationElement));
    ASSERT_TRUE(elementClassId.IsValid());

    AnnotationElementPtr elementPtr = AnnotationElement::Create(AnnotationElement::CreateParams(db, modelId, elementClassId, categoryId));
    ASSERT_TRUE(elementPtr.IsValid());
    DgnElementCPtr result = db.Elements().Insert(*elementPtr);
    ASSERT_TRUE(result.IsValid());
    
    static const Utf8CP LINK1_DISPLAY_LABEL = "Url Link 1";
    static const Utf8CP LINK1_URL = "http://www.google.com";
    
    DgnLinkPtr link1 = DgnLink::Create(db);
    link1->SetDisplayLabel(LINK1_DISPLAY_LABEL);
    link1->SetUrl(LINK1_URL);
    
    EXPECT_TRUE(DgnLinkType::Url == link1->GetType());
    EXPECT_FALSE(link1->GetId().IsValid());
    ASSERT_TRUE(SUCCESS == db.Links().InsertOnElement(result->GetElementId(), *link1));
    ASSERT_TRUE(link1->GetId().IsValid());

    DgnLinkPtr link1b = db.Links().QueryById(link1->GetId());
    ASSERT_TRUE(link1b.IsValid());
    EXPECT_TRUE(link1b->GetId() == link1->GetId());
    ASSERT_TRUE(DgnLinkType::Url == link1b->GetType());
    EXPECT_TRUE(0 == strcmp(link1b->GetDisplayLabel().c_str(), LINK1_DISPLAY_LABEL));
    Utf8String link1bUrl;
    EXPECT_TRUE(SUCCESS == link1b->GetUrl(link1bUrl));
    EXPECT_TRUE(0 == strcmp(link1bUrl.c_str(), LINK1_URL));

    // Update
    link1->SetDisplayLabel("Url Link 2");
    link1->SetUrl("http://www.outlook.com");
    ASSERT_TRUE(SUCCESS == db.Links().Update(*link1));
    link1b = db.Links().QueryById(link1->GetId());
    EXPECT_STREQ("Url Link 2", link1b->GetDisplayLabel().c_str());
    EXPECT_TRUE(SUCCESS == link1b->GetUrl(link1bUrl));
    EXPECT_STREQ("http://www.outlook.com" , link1bUrl.c_str());
    }