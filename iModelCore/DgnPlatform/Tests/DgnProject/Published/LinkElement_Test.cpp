/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/DgnProject/Published/LinkElement_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     02/2015
//=======================================================================================
class LinkElementTest : public GenericDgnModelTestFixture
{
protected:
    DgnElementCPtr InsertAnnotationElement();
    LinkModelPtr InsertLinkModel(DgnDbR dgndb, Utf8CP modelName);

public: 
    LinkElementTest () : GenericDgnModelTestFixture (__FILE__, false /*2D*/, false /*needBriefcase*/) {}
}; // LinkElementTest

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Ramanujam.Raman   05/2016
//---------------------------------------------------------------------------------------
LinkModelPtr LinkElementTest::InsertLinkModel(DgnDbR dgndb, Utf8CP modelName)
    {
    LinkModelPtr model = new LinkModel(LinkModel::CreateParams(dgndb, DgnModel::CreateModelCode(modelName)));
    return (DgnDbStatus::Success != model->Insert()) ? nullptr : model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Umar.Hayat     02/2016
//---------------------------------------------------------------------------------------
DgnElementCPtr LinkElementTest::InsertAnnotationElement()
    {
    DgnDbR db = *m_testDgnManager.GetDgnProjectP();
    DgnModelId modelId = db.Models().QueryFirstModelId();
    if(!modelId.IsValid())
        return nullptr;

    DgnModelPtr modelP = db.Models().GetModel(modelId);
    if(!modelP.IsValid())
        return nullptr;

    DgnCategoryId categoryId = DgnCategory::QueryFirstCategoryId(db);
    if(!categoryId.IsValid())
        return nullptr;

    DgnClassId elementClassId = DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_AnnotationElement2d));
    if(!elementClassId.IsValid())
        return nullptr;
    AnnotationElement2dPtr elementPtr = AnnotationElement2d::Create(AnnotationElement2d::CreateParams(db, modelId, elementClassId, categoryId));
    if(!elementPtr.IsValid())
        return nullptr;
    return db.Elements().Insert(*elementPtr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
TEST_F(LinkElementTest, RoundTripUrlLink)
    {
    ASSERT_TRUE(NULL != m_testDgnManager.GetDgnProjectP());
    DgnDbR db = *m_testDgnManager.GetDgnProjectP();

    DgnElementCPtr annotation = InsertAnnotationElement();
    ASSERT_TRUE(annotation.IsValid());
    
    LinkModelPtr linkModel = InsertLinkModel(db, "TestLinkModel");
    ASSERT_TRUE(linkModel.IsValid());

    static const Utf8CP LINK1_DISPLAY_LABEL = "Url Link 1";
    static const Utf8CP LINK1_URL = "http://www.google.com";
    
    UrlLinkPtr link1 = UrlLink::Create(UrlLink::CreateParams(*linkModel));
    link1->SetLabel(LINK1_DISPLAY_LABEL);
    link1->SetUrl(LINK1_URL);

    UrlLinkCPtr link1A = link1->Insert();
    ASSERT_TRUE(link1A.IsValid());

    BentleyStatus status = link1A->AddToSource(annotation->GetElementId());
    ASSERT_TRUE(status == SUCCESS);

    // TODO: Flush caches here. 

    UrlLinkCPtr link1B = UrlLink::Get(db, link1A->GetElementId());
    ASSERT_TRUE(link1B.IsValid());
    EXPECT_TRUE(link1B->GetElementId() == link1A->GetElementId());
    EXPECT_TRUE(0 == strcmp(link1B->GetLabel(), LINK1_DISPLAY_LABEL));

    EXPECT_TRUE(0 == strcmp(link1B->GetUrl(), LINK1_URL));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
TEST_F(LinkElementTest, UrlLinkQuery)
    {
    //.............................................................................................
    ASSERT_TRUE(NULL != m_testDgnManager.GetDgnProjectP());
    DgnDbR db = *m_testDgnManager.GetDgnProjectP();

    DgnElementCPtr result = InsertAnnotationElement();
    ASSERT_TRUE(result->GetElementId().IsValid());

    LinkModelPtr linkModel = InsertLinkModel(db, "TestLinkModel");
    ASSERT_TRUE(linkModel.IsValid());

    int modelLinkCount = (int) UrlLink::QueryByModel(db, linkModel->GetModelId()).size();
    ASSERT_EQ(0, modelLinkCount);

    int sourceLinkCount = (int) UrlLink::QueryBySource(db, result->GetElementId()).size();
    ASSERT_EQ(0, sourceLinkCount);

    //.............................................................................................
    static const size_t NUM_LINKS = 5;

    for (size_t iLink = 0; iLink < NUM_LINKS; ++iLink)
        {
        UrlLinkPtr link = UrlLink::Create(UrlLink::CreateParams(*linkModel));
        link->SetLabel(Utf8PrintfString("Label %" PRIu64, (uint64_t)iLink).c_str());
        link->SetUrl("URL");

        UrlLinkCPtr linkC = link->Insert();
        ASSERT_TRUE(linkC.IsValid());
        }

    modelLinkCount = (int) UrlLink::QueryByModel(db, linkModel->GetModelId()).size();
    ASSERT_EQ(NUM_LINKS, modelLinkCount);

    sourceLinkCount = (int) UrlLink::QueryBySource(db, result->GetElementId()).size();
    ASSERT_EQ(0, sourceLinkCount);

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
    int whereLinkCount = (int) UrlLink::QueryByWhere(db, LINK_ECSQL_PREFIX ".Label LIKE '%2'").size();
    ASSERT_EQ(1, whereLinkCount);

    //.............................................................................................
    BentleyStatus status = link->RemoveFromSource(result->GetElementId());
    ASSERT_TRUE(status == SUCCESS);

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
// @bsimethod                                                   Jeff.Marker     02/2015
//---------------------------------------------------------------------------------------
TEST_F(LinkElementTest, OtherIterators)
    {
    ASSERT_TRUE(NULL != m_testDgnManager.GetDgnProjectP());
    DgnDbR db = *m_testDgnManager.GetDgnProjectP();

    DgnElementCPtr result1 = InsertAnnotationElement();
    ASSERT_TRUE(result1.IsValid());

    DgnElementCPtr result2 = InsertAnnotationElement();
    ASSERT_TRUE(result2.IsValid());

    LinkModelPtr linkModel = InsertLinkModel(db, "TestLinkModel");
    ASSERT_TRUE(linkModel.IsValid());

    EmbeddedFileLinkPtr link1 = EmbeddedFileLink::Create(EmbeddedFileLink::CreateParams(*linkModel, "EmbeddedDocumentName1")); link1->SetLabel("link1"); link1->Insert();
    EmbeddedFileLinkPtr link2 = EmbeddedFileLink::Create(EmbeddedFileLink::CreateParams(*linkModel, "EmbeddedDocumentName2")); link2->SetLabel("link2"); link2->Insert();
    EmbeddedFileLinkPtr link3 = EmbeddedFileLink::Create(EmbeddedFileLink::CreateParams(*linkModel, "EmbeddedDocumentName3")); link3->SetLabel("link3"); link3->Insert();
    EmbeddedFileLinkPtr link4 = EmbeddedFileLink::Create(EmbeddedFileLink::CreateParams(*linkModel, "EmbeddedDocumentName4")); link4->SetLabel("link4"); link4->Insert();

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

    ASSERT_EQ(1, (int) EmbeddedFileLink::QueryByWhere(db, LINK_ECSQL_PREFIX ".Label LIKE '%1'").size());
    ASSERT_EQ(1, (int) EmbeddedFileLink::QueryByWhere(db, LINK_ECSQL_PREFIX ".Label LIKE '%2'").size());
    ASSERT_EQ(1, (int) EmbeddedFileLink::QueryByWhere(db, LINK_ECSQL_PREFIX ".Label LIKE '%3'").size());
    ASSERT_EQ(1, (int) EmbeddedFileLink::QueryByWhere(db, LINK_ECSQL_PREFIX ".Label LIKE '%4'").size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Umar.Hayat     02/2016
//---------------------------------------------------------------------------------------
TEST_F(LinkElementTest, Update)
    {
    //.............................................................................................
    ASSERT_TRUE(NULL != m_testDgnManager.GetDgnProjectP());
    DgnDbR db = *m_testDgnManager.GetDgnProjectP();

    DgnElementCPtr result = InsertAnnotationElement();
    ASSERT_TRUE(result.IsValid());

    LinkModelPtr linkModel = InsertLinkModel(db, "TestLinkModel");
    ASSERT_TRUE(linkModel.IsValid());

    static const Utf8CP LINK1_DISPLAY_LABEL = "Url Link 1";
    static const Utf8CP LINK1_URL = "http://www.google.com";
    
    UrlLinkPtr link1 = UrlLink::Create(UrlLink::CreateParams(*linkModel));
    link1->SetLabel(LINK1_DISPLAY_LABEL);
    link1->SetUrl(LINK1_URL);

    UrlLinkCPtr link1b = link1->Insert();
    ASSERT_TRUE(link1b.IsValid());
    ASSERT_TRUE(link1->GetElementId().IsValid());
    EXPECT_TRUE(link1b->GetElementId() == link1->GetElementId());

    link1b = UrlLink::Get(db, link1->GetElementId());
    ASSERT_TRUE(link1b.IsValid());
    EXPECT_TRUE(link1b->GetElementId() == link1->GetElementId());

    EXPECT_STREQ(link1b->GetLabel(), LINK1_DISPLAY_LABEL);
    EXPECT_STREQ(link1b->GetUrl(), LINK1_URL);

    // Update
    link1->SetLabel("Url Link 2");
    link1->SetUrl("http://www.outlook.com");
    link1->Update();

    link1b = UrlLink::Get(db, link1->GetElementId());
    EXPECT_STREQ("Url Link 2", link1b->GetLabel());
    EXPECT_STREQ("http://www.outlook.com", link1b->GetUrl());
    }

