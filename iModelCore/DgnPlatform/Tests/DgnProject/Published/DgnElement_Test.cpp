/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/DgnElement_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"

//----------------------------------------------------------------------------------------
// @bsiclass                                                    Julija.Suboc     07/2013
//----------------------------------------------------------------------------------------
struct DgnElementTests : public DgnDbTestFixture
    {
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Maha Nasir                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgnElementTests, ResetStatistics)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"Element_Test.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    m_defaultModelId = m_db->Models().QueryFirstModelId();
    DgnModelPtr seedModel = m_db->Models().GetModel(m_defaultModelId);
    seedModel->FillModel();
    EXPECT_TRUE (seedModel != nullptr);

    //Inserts a model
    DgnModelPtr M1 = seedModel->Clone("Model1");
    M1->Insert("Test Model 1");
    EXPECT_TRUE (M1 != nullptr);
    m_db->SaveChanges("changeSet1");

    DgnModelId M1id = m_db->Models().QueryModelId("model1");
    EXPECT_TRUE (M1id.IsValid());

    //Inserts 2 elements.
    auto keyE1 = InsertElement(DgnElement::Code("E1"), M1id);
    DgnElementId E1id = keyE1->GetElementId();
    DgnElementCPtr E1 = m_db->Elements().GetElement(E1id);
    EXPECT_TRUE (E1 != nullptr);

    auto keyE2 = InsertElement(DgnElement::Code("E2"), M1id);
    DgnElementId E2id = keyE2->GetElementId();
    DgnElementCPtr E2 = m_db->Elements().GetElement(E2id);
    EXPECT_TRUE (E2 != nullptr);

    DgnModelId model_id = m_db->Elements().QueryModelId(E1id);
    EXPECT_EQ (M1id, model_id);

    //Deletes the first element.
    DgnDbStatus status=E2->Delete();
    EXPECT_EQ ((DgnDbStatus)SUCCESS, status);
    m_db->SaveChanges();

    int64_t memTarget = 0;
    m_db->Elements().Purge(memTarget);

    //Get stats of the element pool.
    DgnElements::Statistics stats = m_db->Elements().GetStatistics();

    uint32_t NewElements = stats.m_newElements;
    EXPECT_EQ (2, NewElements);

    uint32_t RefElements = stats.m_reReferenced;
    EXPECT_EQ (0, RefElements);

    uint32_t UnrefElements = stats.m_unReferenced;
    EXPECT_EQ (0, UnrefElements);

    uint32_t PurgedElements = stats.m_purged;
    EXPECT_EQ (1, PurgedElements);

    m_db->Elements().ResetStatistics();

    stats = m_db->Elements().GetStatistics();

    //Statistics after reset.
    NewElements = stats.m_newElements;
    EXPECT_EQ (0, NewElements);

    PurgedElements = stats.m_purged;
    EXPECT_EQ (0, PurgedElements);

    RefElements = stats.m_reReferenced;
    EXPECT_EQ (0, RefElements);

    UnrefElements = stats.m_unReferenced;
    EXPECT_EQ (0, UnrefElements);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Maha Nasir                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgnElementTests, UpdateElement)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"Element_Test.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    m_defaultModelId = m_db->Models().QueryFirstModelId();
    DgnModelPtr seedModel = m_db->Models().GetModel(m_defaultModelId);
    seedModel->FillModel();
    EXPECT_TRUE (seedModel != nullptr);

    //Inserts a model
    DgnModelPtr m1 = seedModel->Clone("Model1");
    m1->Insert("Test Model 1");
    EXPECT_TRUE(m1 != nullptr);
    m_db->SaveChanges("changeSet1");

    DgnModelId m1id = m_db->Models().QueryModelId("model1");
    EXPECT_TRUE(m1id.IsValid());

    auto keyE1 = InsertElement(DgnElement::Code("E1"), m1id);
    DgnElementId e1id = keyE1->GetElementId();
    DgnElementCPtr e1 = m_db->Elements().GetElement(e1id);
    EXPECT_TRUE(e1 != nullptr);

    DgnClassId classId = e1->QueryClassId(*m_db);
    EXPECT_TRUE(classId.IsValid());

    //Creating a copy of element to edit.
    DgnElementPtr e1Copy = e1->CopyForEdit();
    e1Copy->SetLabel("Updated Test Element");

    DgnElement::Code updatedCode("This is the updated Element.");
    e1Copy->SetCode(updatedCode);

    DgnElementCPtr updatedElement = e1Copy->Update();

    EXPECT_STREQ("Updated Test Element", updatedElement->GetLabel());
    EXPECT_TRUE(updatedCode == updatedElement->GetCode());

    //Codes must be unique
    EXPECT_TRUE(InsertElement(DgnElement::Code("MyCode"), m1id).IsValid());
    DgnDbStatus insertStatus;
    EXPECT_FALSE(InsertElement(DgnElement::Code("MyCode"), m1id, DgnCategoryId(), &insertStatus).IsValid());
    EXPECT_EQ(insertStatus, DgnDbStatus::DuplicateCode);
    }


