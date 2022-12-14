/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"

//=======================================================================================
// @bsiclass
//=======================================================================================
struct LinkElementTest : public GenericDgnModel2dTestFixture
{
protected:
    DgnElementCPtr InsertAnnotationElement();
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnElementCPtr LinkElementTest::InsertAnnotationElement()
    {
    DgnDbR db = *GetDgnDb();
    DgnModelId modelId = GetDrawingModelId();
    if(!modelId.IsValid())
        return nullptr;

    DgnModelPtr modelP = db.Models().GetModel(modelId);
    if(!modelP.IsValid())
        return nullptr;

    DgnCategoryId categoryId = DgnDbTestUtils::GetFirstDrawingCategoryId(db);
    if(!categoryId.IsValid())
        return nullptr;

    DgnClassId elementClassId = DgnClassId(db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_AnnotationElement2d));
    if(!elementClassId.IsValid())
        return nullptr;
    AnnotationElement2dPtr elementPtr = AnnotationElement2d::Create(AnnotationElement2d::CreateParams(db, modelId, elementClassId, categoryId));
    if(!elementPtr.IsValid())
        return nullptr;
    return db.Elements().Insert(*elementPtr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(LinkElementTest, RoundTripUrlLink)
    {
    DgnDbR db = *GetDgnDb(L"RoundTripUrlLink");
    DgnDbTestUtils::InsertDrawingCategory(db, "TestDrawingCategory");

    DgnElementCPtr annotation = InsertAnnotationElement();
    ASSERT_TRUE(annotation.IsValid());
    
    LinkModelPtr linkModel = DgnDbTestUtils::InsertLinkModel(db, "TestLinkModel");
    static const Utf8CP LINK1_DISPLAY_LABEL = "Url Link 1";
    static const Utf8CP LINK1_URL = "http://www.google.com";
    static const Utf8CP LINK2_DISPLAY_LABEL = "Url Link 2";
    static const Utf8CP LINK2_URL = "http://www.facebook.com";
    static const Utf8CP LINK1_DESCRIPTION = "This is Url Link Element";
    UrlLinkPtr link1 = UrlLink::Create(UrlLink::CreateParams(*linkModel));
    link1->SetUserLabel(LINK1_DISPLAY_LABEL);
    link1->SetUrl(LINK1_URL);
    link1->SetDescription(LINK1_DESCRIPTION);
    UrlLinkPtr link2 = UrlLink::Create(UrlLink::CreateParams(*linkModel));
    link2->SetUserLabel(LINK2_DISPLAY_LABEL);
    link2->SetUrl(LINK2_URL);

    UrlLinkCPtr link1A = link1->Insert();
    ASSERT_TRUE(link1A.IsValid());

    UrlLinkCPtr link2A = link2->Insert();
    ASSERT_TRUE(link2A.IsValid());

    BentleyStatus status = link1A->AddToSource(annotation->GetElementId());
    ASSERT_TRUE(status == SUCCESS);

    BentleyStatus status2 = link2A->AddToSource(annotation->GetElementId());
    ASSERT_TRUE(status2 == SUCCESS);

    // TODO: Flush caches here. 

    UrlLinkCPtr link1B = UrlLink::Get(db, link1A->GetElementId());
    ASSERT_TRUE(link1B.IsValid());
    EXPECT_TRUE(link1B->GetElementId() == link1A->GetElementId());
    EXPECT_TRUE(0 == strcmp(link1B->GetUserLabel(), LINK1_DISPLAY_LABEL));

    EXPECT_TRUE(0 == strcmp(link1B->GetUrl(), LINK1_URL));
    EXPECT_TRUE(0 == strcmp(link1B->GetDescription(), LINK1_DESCRIPTION));

    UrlLinkCPtr link2B = UrlLink::Get(db, link2A->GetElementId());
    ASSERT_TRUE(link2B.IsValid());
    EXPECT_TRUE(link2B->GetElementId() == link2A->GetElementId());
    EXPECT_TRUE(0 == strcmp(link2B->GetUserLabel(), LINK2_DISPLAY_LABEL));
    EXPECT_TRUE(0 == strcmp(link2B->GetUrl(), LINK2_URL));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(LinkElementTest, UrlLinkQuery)
    {
    //.............................................................................................
    DgnDbR db = *GetDgnDb(L"UrlLinkQuery");
    DgnDbTestUtils::InsertDrawingCategory(db, "TestDrawingCategory");

    DgnElementCPtr result = InsertAnnotationElement();
    ASSERT_TRUE(result->GetElementId().IsValid());

    LinkModelPtr linkModel = DgnDbTestUtils::InsertLinkModel(db, "TestLinkModel");
    int modelLinkCount = (int) UrlLink::QueryByModel(db, linkModel->GetModelId()).size();
    ASSERT_EQ(0, modelLinkCount);

    int sourceLinkCount = (int) UrlLink::QueryBySource(db, result->GetElementId()).size();
    ASSERT_EQ(0, sourceLinkCount);

    int LinkCount = (int)UrlLink::Query(db).size();
    ASSERT_EQ(0, LinkCount);
    //.............................................................................................
    static const size_t NUM_LINKS = 5;

    for (size_t iLink = 0; iLink < NUM_LINKS; ++iLink)
        {
        UrlLinkPtr link = UrlLink::Create(UrlLink::CreateParams(*linkModel));
        link->SetUserLabel(Utf8PrintfString("UserLabel %" PRIu64, (uint64_t)iLink).c_str());
        link->SetUrl("URL");

        UrlLinkCPtr linkC = link->Insert();
        ASSERT_TRUE(linkC.IsValid());
        }

    modelLinkCount = (int) UrlLink::QueryByModel(db, linkModel->GetModelId()).size();
    ASSERT_EQ(NUM_LINKS, modelLinkCount);

    sourceLinkCount = (int) UrlLink::QueryBySource(db, result->GetElementId()).size();
    ASSERT_EQ(0, sourceLinkCount);

    LinkCount = (int)UrlLink::Query(db).size();
    ASSERT_EQ(NUM_LINKS, LinkCount);

    ////.............................................................................................
    UrlLinkCPtr link;
    for (DgnElementId id : UrlLink::QueryByModel(db, linkModel->GetModelId()))
        {
        link = UrlLink::Get(db, id);
        ASSERT_TRUE(link.IsValid());

        BentleyStatus status = link->AddToSource(result->GetElementId());
        ASSERT_TRUE(status == SUCCESS);
        }

    db.SaveChanges();

    sourceLinkCount = (int) UrlLink::QueryBySource(db, result->GetElementId()).size();
    ASSERT_EQ(NUM_LINKS, sourceLinkCount);

    //.............................................................................................
    int whereLinkCount = (int) UrlLink::QueryByWhere(db, LINK_ECSQL_PREFIX ".UserLabel LIKE '%2'").size();
    ASSERT_EQ(1, whereLinkCount);

    //.............................................................................................
    ASSERT_TRUE(link->IsFromSource(result->GetElementId()));

    //.............................................................................................
    BentleyStatus status = link->RemoveFromSource(result->GetElementId());
    ASSERT_TRUE(status == SUCCESS);

    ASSERT_FALSE(link->IsFromSource(result->GetElementId()));

    modelLinkCount = (int) UrlLink::QueryByModel(db, linkModel->GetModelId()).size();
    ASSERT_EQ(NUM_LINKS, modelLinkCount); // Removing the link doesn't delete it

    status = UrlLink::PurgeOrphaned(db);
    ASSERT_TRUE(status == SUCCESS);

    modelLinkCount = (int) UrlLink::QueryByModel(db, linkModel->GetModelId()).size();
    ASSERT_EQ(NUM_LINKS - 1, modelLinkCount);

    status = UrlLink::RemoveAllFromSource(db, result->GetElementId());
    ASSERT_TRUE(status == SUCCESS);

    status = UrlLink::PurgeOrphaned(db);
    ASSERT_TRUE(status == SUCCESS);

    modelLinkCount = (int) UrlLink::QueryByModel(db, linkModel->GetModelId()).size();
    ASSERT_EQ(0, modelLinkCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(LinkElementTest, OtherIterators)
    {
    DgnDbR db = *GetDgnDb(L"OtherIterators");
    DgnDbTestUtils::InsertDrawingCategory(db, "TestDrawingCategory");

    DgnElementCPtr result1 = InsertAnnotationElement();
    ASSERT_TRUE(result1.IsValid());

    DgnElementCPtr result2 = InsertAnnotationElement();
    ASSERT_TRUE(result2.IsValid());

    LinkModelPtr linkModel = DgnDbTestUtils::InsertLinkModel(db, "TestLinkModel");

    EmbeddedFileLinkPtr link1 = EmbeddedFileLink::Create(EmbeddedFileLink::CreateParams(*linkModel, "EmbeddedDocumentName1")); link1->SetUserLabel("link1"); link1->Insert();
    EmbeddedFileLinkPtr link2 = EmbeddedFileLink::Create(EmbeddedFileLink::CreateParams(*linkModel, "EmbeddedDocumentName2")); link2->SetUserLabel("link2"); link2->Insert();
    EmbeddedFileLinkPtr link3 = EmbeddedFileLink::Create(EmbeddedFileLink::CreateParams(*linkModel, "EmbeddedDocumentName3")); link3->SetUserLabel("link3"); link3->Insert();
    EmbeddedFileLinkPtr link4 = EmbeddedFileLink::Create(EmbeddedFileLink::CreateParams(*linkModel, "EmbeddedDocumentName4")); link4->SetUserLabel("link4"); link4->Insert();

    DgnElementId elementId1 = result1->GetElementId();
    DgnElementId elementId2 = result2->GetElementId();

    ASSERT_TRUE(SUCCESS == link1->AddToSource(elementId1));
    ASSERT_TRUE(SUCCESS == link2->AddToSource(elementId1));
    ASSERT_TRUE(SUCCESS == link2->AddToSource(elementId2));
    ASSERT_TRUE(SUCCESS == link3->AddToSource(elementId2));
    ASSERT_TRUE(SUCCESS == link4->AddToSource(elementId2));

    ASSERT_EQ(4, (int) EmbeddedFileLink::QueryByWhere(db, nullptr).size());
    ASSERT_EQ(2, (int) EmbeddedFileLink::QueryBySource(db, elementId1).size());
    ASSERT_EQ(3, (int) EmbeddedFileLink::QueryBySource(db, elementId2).size());
    
    ASSERT_EQ(1, (int) link1->QuerySources().size());
    ASSERT_EQ(2, (int) link2->QuerySources().size());
    ASSERT_EQ(1, (int) link3->QuerySources().size());
    ASSERT_EQ(1, (int) link4->QuerySources().size());

    ASSERT_EQ(1, (int) EmbeddedFileLink::QueryByWhere(db, LINK_ECSQL_PREFIX ".UserLabel LIKE '%1'").size());
    ASSERT_EQ(1, (int) EmbeddedFileLink::QueryByWhere(db, LINK_ECSQL_PREFIX ".UserLabel LIKE '%2'").size());
    ASSERT_EQ(1, (int) EmbeddedFileLink::QueryByWhere(db, LINK_ECSQL_PREFIX ".UserLabel LIKE '%3'").size());
    ASSERT_EQ(1, (int) EmbeddedFileLink::QueryByWhere(db, LINK_ECSQL_PREFIX ".UserLabel LIKE '%4'").size());
    ASSERT_EQ(4, (int)EmbeddedFileLink::Query(db).size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(LinkElementTest, Update)
    {
    //.............................................................................................
    DgnDbR db = *GetDgnDb(L"Update");
    DgnDbTestUtils::InsertDrawingCategory(db, "TestDrawingCategory");

    DgnElementCPtr result = InsertAnnotationElement();
    ASSERT_TRUE(result.IsValid());

    LinkModelPtr linkModel = DgnDbTestUtils::InsertLinkModel(db, "TestLinkModel");

    static const Utf8CP LINK1_DISPLAY_LABEL = "Url Link 1";
    static const Utf8CP LINK1_URL = "http://www.google.com";
    
    UrlLinkPtr link1 = UrlLink::Create(UrlLink::CreateParams(*linkModel));
    link1->SetUserLabel(LINK1_DISPLAY_LABEL);
    link1->SetUrl(LINK1_URL);

    UrlLinkCPtr link1b = link1->Insert();
    ASSERT_TRUE(link1b.IsValid());
    ASSERT_TRUE(link1->GetElementId().IsValid());
    EXPECT_TRUE(link1b->GetElementId() == link1->GetElementId());

    link1b = UrlLink::Get(db, link1->GetElementId());
    ASSERT_TRUE(link1b.IsValid());
    EXPECT_TRUE(link1b->GetElementId() == link1->GetElementId());

    EXPECT_STREQ(link1b->GetUserLabel(), LINK1_DISPLAY_LABEL);
    EXPECT_STREQ(link1b->GetUrl(), LINK1_URL);

    // Update
    link1->SetUserLabel("Url Link 2");
    link1->SetUrl("http://www.outlook.com");
    link1->Update();

    link1b = UrlLink::Get(db, link1->GetElementId());
    EXPECT_STREQ("Url Link 2", link1b->GetUserLabel());
    EXPECT_STREQ("http://www.outlook.com", link1b->GetUrl());

    EmbeddedFileLinkPtr link1E = EmbeddedFileLink::Create(EmbeddedFileLink::CreateParams(*linkModel, "EmbeddedDocumentName1")); 
    link1E->SetUserLabel("link1"); 
    link1E->Insert();
    ASSERT_EQ(1, (int)EmbeddedFileLink::Query(db).size());
    EXPECT_STREQ(link1E->GetName(), "EmbeddedDocumentName1");

    //Update EmbeddedFileLink
    link1E->SetName("UpdatedEmbeddedDocumentName1");
    link1E->Update();
    EXPECT_STREQ(link1E->GetName(), "UpdatedEmbeddedDocumentName1");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(LinkElementTest, RepositoryLinkCRUD)
    {
    DgnDbR db = *GetDgnDb(L"RepositoryLinkCRUD");
    DgnElementId linkElementId1;
    DgnElementId linkElementId2;
    Utf8CP testUrl = "http://www.outlook.com";
    Utf8CP testDescription = "TestDescription";
    DgnCode testLabel1, testLabel2;
    // Insert test RepositoryLink
    LinkModelPtr model = DgnDbTestUtils::InsertLinkModel(db, "TestLinkModel");
    testLabel1 = RepositoryLink::CreateUniqueCode(*model, "TestLabel");
    RepositoryLinkPtr link = RepositoryLink::Create(*model, testUrl, testLabel1.GetValueUtf8CP(), testDescription);
    EXPECT_TRUE(link.IsValid());
    EXPECT_TRUE(link->Insert().IsValid());
    linkElementId1 = link->GetElementId();

    EXPECT_STREQ(link->GetUrl(), testUrl);
    EXPECT_STREQ(link->GetUserLabel(), testLabel1.GetValueUtf8CP());
    EXPECT_STREQ(link->GetDescription(), testDescription);

    // Flush cache and re-check element
    {
    db.Elements().ClearCache();
    RepositoryLinkCPtr link = db.Elements().Get<RepositoryLink>(linkElementId1);
    EXPECT_TRUE(link.IsValid());
    EXPECT_STREQ(link->GetUrl(), testUrl);
    EXPECT_STREQ(link->GetUserLabel(), testLabel1.GetValueUtf8CP());
    EXPECT_STREQ(link->GetDescription(), testDescription);
    }
    // Update test Repositrylink
    {
    link = db.Elements().GetForEdit<RepositoryLink>(linkElementId1);
    link->SetUrl("http://www.facebook.com");
    ASSERT_TRUE(link->Update().IsValid());
    }
    // Flush cache and re-check element
    {
    db.Elements().ClearCache();
    RepositoryLinkCPtr link = db.Elements().Get<RepositoryLink>(linkElementId1);
    EXPECT_TRUE(link.IsValid());
    EXPECT_STREQ(link->GetUrl(), "http://www.facebook.com");
    EXPECT_STREQ(link->GetUserLabel(), testLabel1.GetValueUtf8CP());
    EXPECT_STREQ(link->GetDescription(), testDescription);
    }
    // Insert another RepositoryLink with Duplicate DgnCode it should return false then insert with unique code
    {
    RepositoryLinkPtr link2 = RepositoryLink::Create(*model, testUrl, "TestLabel", testDescription);
    ASSERT_TRUE(link2.IsValid());
    BeTest::SetFailOnAssert(false);
    ASSERT_FALSE(link2->Insert().IsValid());
    BeTest::SetFailOnAssert(true);
    testLabel2 = RepositoryLink::CreateUniqueCode(*model, "TestLabel");
    link2 = RepositoryLink::Create(*model, testUrl, testLabel2.GetValueUtf8CP(), testDescription);
    ASSERT_TRUE(link2.IsValid());
    ASSERT_TRUE(link2->Insert().IsValid());
    linkElementId2 = link2->GetElementId();

    EXPECT_STREQ(link2->GetUrl(), testUrl);
    EXPECT_STREQ(link2->GetUserLabel(), testLabel2.GetValueUtf8CP());
    EXPECT_STREQ(link2->GetDescription(), testDescription);
    }
    // Flush cache and re-check element
    {
    db.Elements().ClearCache();
    RepositoryLinkCPtr link = db.Elements().Get<RepositoryLink>(linkElementId2);
    EXPECT_TRUE(link.IsValid());
    EXPECT_STREQ(link->GetUrl(), testUrl);
    EXPECT_STREQ(link->GetUserLabel(), testLabel2.GetValueUtf8CP());
    EXPECT_STREQ(link->GetDescription(), testDescription);
    }
    }
