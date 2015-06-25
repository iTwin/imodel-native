//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/DgnLink_Test.cpp $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

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
    public: DgnLinkTest () : GenericDgnModelTestFixture (__FILE__, false /*2D*/) {}

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

    DgnModelP modelP = db.Models().GetModel(modelId);
    ASSERT_TRUE(nullptr != modelP);

    DgnCategoryId categoryId = db.Categories().MakeIterator().begin().GetCategoryId();
    ASSERT_TRUE(categoryId.IsValid());

    DgnClassId elementClassId = DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_Element));
    ASSERT_TRUE(elementClassId.IsValid());

    DgnElementPtr elementPtr = DgnElement::Create(DgnElement::CreateParams(db, modelId, elementClassId, categoryId));
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
    ASSERT_TRUE(SUCCESS == db.Links().InsertOnElement(result->GetElementKey(), *link1));
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

    DgnModelP modelP = db.Models().GetModel(modelId);
    ASSERT_TRUE(nullptr != modelP);

    DgnCategoryId categoryId = db.Categories().MakeIterator().begin().GetCategoryId();
    ASSERT_TRUE(categoryId.IsValid());

    DgnClassId elementClassId = DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_Element));
    ASSERT_TRUE(elementClassId.IsValid());

    DgnElementPtr elementPtr = DgnElement::Create(DgnElement::CreateParams(db, modelId, elementClassId, categoryId));
    ASSERT_TRUE(elementPtr.IsValid());
    DgnElementCPtr result = db.Elements().Insert(*elementPtr);
    ASSERT_TRUE(result->GetElementKey().IsValid());
    
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
    ASSERT_TRUE(SUCCESS == db.Links().InsertOnElement(result->GetElementKey(), *links[0]));
    EXPECT_TRUE(1 == db.Links().MakeIterator().QueryCount());

    //.............................................................................................
    for (size_t iLink = 1; iLink < NUM_LINKS; ++iLink)
        ASSERT_TRUE(SUCCESS == db.Links().InsertOnElement(result->GetElementKey(), *links[iLink]));

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
    db.Links().DeleteFromElement(result->GetElementKey(), links[0]->GetId());
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

    DgnModelP modelP = db.Models().GetModel(modelId);
    ASSERT_TRUE(nullptr != modelP);

    DgnCategoryId categoryId = db.Categories().MakeIterator().begin().GetCategoryId();
    ASSERT_TRUE(categoryId.IsValid());

    DgnClassId elementClassId = DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_Element));
    ASSERT_TRUE(elementClassId.IsValid());

    DgnElementPtr element1 = DgnElement::Create(DgnElement::CreateParams(db, modelId, elementClassId, categoryId));
    ASSERT_TRUE(element1.IsValid());
    DgnElementCPtr result1 = db.Elements().Insert(*element1);
    ASSERT_TRUE(result1.IsValid());

    DgnElementPtr element2 = DgnElement::Create(DgnElement::CreateParams(db, modelId, elementClassId, categoryId));
    ASSERT_TRUE(element2.IsValid());
    DgnElementCPtr result2 = db.Elements().Insert(*element2);
    ASSERT_TRUE(result2.IsValid());

    DgnLinkPtr link1 = DgnLink::Create(db); link1->SetDisplayLabel("link1"); link1->SetEmbeddedDocumentName("EmbeddedDocumentName1");
    DgnLinkPtr link2 = DgnLink::Create(db); link2->SetDisplayLabel("link2"); link2->SetEmbeddedDocumentName("EmbeddedDocumentName2");
    DgnLinkPtr link3 = DgnLink::Create(db); link3->SetDisplayLabel("link3"); link3->SetEmbeddedDocumentName("EmbeddedDocumentName3");
    DgnLinkPtr link4 = DgnLink::Create(db); link4->SetDisplayLabel("link4"); link4->SetEmbeddedDocumentName("EmbeddedDocumentName4");

    DgnElementKey key1 = result1->GetElementKey();
    DgnElementKey key2 = result2->GetElementKey();

    ASSERT_TRUE(SUCCESS == db.Links().InsertOnElement(key1, *link1));
    ASSERT_TRUE(SUCCESS == db.Links().InsertOnElement(key1, *link2));
    ASSERT_TRUE(SUCCESS == db.Links().InsertOnElement(key2, link2->GetId()));
    ASSERT_TRUE(SUCCESS == db.Links().InsertOnElement(key2, *link3));
    ASSERT_TRUE(SUCCESS == db.Links().InsertOnElement(key2, *link4));

    checkIteratorCount(db.Links().MakeIterator(), 4);
    
    checkIteratorCount(db.Links().MakeOnElementIterator(key1), 2);
    checkIteratorCount(db.Links().MakeOnElementIterator(key2), 3);

    checkIteratorCount(db.Links().MakeReferencesLinkIterator(link1->GetId()), 1);
    checkIteratorCount(db.Links().MakeReferencesLinkIterator(link2->GetId()), 2);
    checkIteratorCount(db.Links().MakeReferencesLinkIterator(link3->GetId()), 1);
    checkIteratorCount(db.Links().MakeReferencesLinkIterator(link4->GetId()), 1);
    }
