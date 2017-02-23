/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/CustomHandledPropertyTest.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>

USING_NAMESPACE_BENTLEY_DPTEST

//----------------------------------------------------------------------------------------
// @bsiclass                                                    Ridha.Malik     02/17
//----------------------------------------------------------------------------------------
struct GetSetCustomHandledProprty : public DgnDbTestFixture
    {

    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GetSetCustomHandledProprty, ReadOnly)
    {
    //test Custom Attributes when we get them
    SetupSeedProject();
    DgnClassId classId(m_db->Schemas().GetECClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_CLASS_NAME));
    TestElement::CreateParams params(*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d(), DgnCode());
    TestElement el(params);

    //Check a few CustomhandleProperties 
    ECN::ECValue checkValue1, checkValue2;
    uint32_t LMindex , Spindex, Mindex, Gsindex ,Orgindex; 
    DateTime dateTime = DateTime(DateTime::Kind::Utc, 2016, 2, 14, 9, 58, 17, 456);
    ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(LMindex, "LastMod"));
    ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(Mindex, "Model"));
    ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(Spindex, "InSpatialIndex"));
    ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(Gsindex, "GeometryStream"));

    // Try to set readonly property
    ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue1, "LastMod"));
    ASSERT_EQ(DgnDbStatus::ReadOnly,el.SetPropertyValue(LMindex,ECN::ECValue(dateTime)));
    ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue2, LMindex));
    ASSERT_TRUE(checkValue1.Equals(checkValue2));
    checkValue1.Clear();
    checkValue2.Clear();

    ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue1, "Model"));
    ASSERT_EQ(DgnDbStatus::ReadOnly, el.SetPropertyValue(Mindex, ECN::ECValue(2)));
    ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue2, Mindex));
    ASSERT_TRUE(checkValue1.Equals(checkValue2));
    checkValue1.Clear();
    checkValue2.Clear();

    ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue1, "InSpatialIndex"));
    ASSERT_EQ(DgnDbStatus::ReadOnly, el.SetPropertyValue(Spindex, ECN::ECValue(true)));
    ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue2, Spindex));
    ASSERT_TRUE(checkValue1.Equals(checkValue2));
    checkValue1.Clear();
    checkValue2.Clear();

    DgnElementCPtr eleid = el.Insert();
    ASSERT_TRUE(eleid.IsValid());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GetSetCustomHandledProprty, InaccessibleProperty)
    {
    //test Custom Attributes when we get them
    SetupSeedProject();
    DgnClassId classId(m_db->Schemas().GetECClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_CLASS_NAME));
    TestElement::CreateParams params(*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d(), DgnCode());
    TestElement el(params);

    ECN::ECValue checkValue1;
    uint32_t Gsindex; 
    const static int DataSize = 10;
    Byte DummyData[DataSize] = { 1,2,3,4,5,6,7,8,9,10 };
    ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(Gsindex, "GeometryStream"));

    //Try to set inaccessible property
    ASSERT_EQ(DgnDbStatus::BadRequest, el.GetPropertyValue(checkValue1, "GeometryStream"));
    ASSERT_EQ(DgnDbStatus::BadRequest, el.SetPropertyValue(Gsindex, ECN::ECValue(DummyData, sizeof(DummyData))));

    DgnElementCPtr eleid = el.Insert();
    ASSERT_TRUE(eleid.IsValid());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GetSetCustomHandledProprty, 3dElementProprties)
    {
    //test Custom Attributes when we get them
    SetupSeedProject();
    DgnElementId eleid;
    ECN::ECValue checkValue1, checkValue2;
    uint32_t Orgindex, Yawindex, Pitchindex, Rollindex, BBlindex, BBHindex;
    if (true)
        {
        DgnClassId classId(m_db->Schemas().GetECClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_CLASS_NAME));
        TestElement::CreateParams params(*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d(), DgnCode());
        TestElement el(params);
        //Check a few CustomhandleProperties 
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(Orgindex, "Origin"));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(Yawindex, "Yaw"));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(Pitchindex, "Pitch"));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(Rollindex, "Roll"));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(BBlindex, "BBoxLow"));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(BBHindex, "BBoxHigh"));

        // Try to set Invaild value type 
        ASSERT_EQ(DgnDbStatus::BadArg, el.SetPropertyValue(Orgindex, ECN::ECValue(true)));
        ASSERT_EQ(DgnDbStatus::BadArg, el.SetPropertyValue(BBlindex, ECN::ECValue(true)));
        ASSERT_EQ(DgnDbStatus::BadArg, el.SetPropertyValue(BBHindex, ECN::ECValue(true)));
        ASSERT_EQ(DgnDbStatus::BadArg, el.SetPropertyValue(Yawindex, ECN::ECValue(true)));
        ASSERT_EQ(DgnDbStatus::BadArg, el.SetPropertyValue(Rollindex, ECN::ECValue(true)));
        ASSERT_EQ(DgnDbStatus::BadArg, el.SetPropertyValue(Pitchindex, ECN::ECValue(true)));

        ASSERT_EQ(DgnDbStatus::Success, el.SetPropertyValue(Orgindex, ECN::ECValue(DPoint3d::From(2, 1, 1))));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue1, Orgindex));
        ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(DPoint3d::From(2, 1, 1))));
        checkValue1.Clear();
        ASSERT_EQ(DgnDbStatus::Success, el.SetPropertyValue(BBlindex, ECN::ECValue(DPoint3d::From(2, 2, 2))));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue1, BBlindex));
        ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(DPoint3d::From(2, 2, 2))));
        checkValue1.Clear();
        ASSERT_EQ(DgnDbStatus::Success, el.SetPropertyValue(BBHindex, ECN::ECValue(DPoint3d::From(2, 4, 8))));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue1, BBHindex));
        ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(DPoint3d::From(2, 4, 8))));
        checkValue1.Clear();
        ASSERT_EQ(DgnDbStatus::Success, el.SetPropertyValue(Yawindex, ECN::ECValue(4.5)));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue1, Yawindex));
        ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(4.5)));
        checkValue1.Clear();
        ASSERT_EQ(DgnDbStatus::Success, el.SetPropertyValue(Rollindex, ECN::ECValue(6.5)));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue1, Rollindex));
        ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(6.5)));
        checkValue1.Clear();
        ASSERT_EQ(DgnDbStatus::Success, el.SetPropertyValue(Pitchindex, ECN::ECValue(8.5)));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue1, Pitchindex));
        ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(8.5)));
        checkValue1.Clear();
        // Insert the element
        DgnDbStatus stat;
        DgnElementCPtr persistentEl = el.Insert(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat);
        ASSERT_TRUE(persistentEl.IsValid());

        eleid = persistentEl->GetElementId();
        m_db->SaveChanges();
        }
    // Before updatation of element check what stored in DB
    BeFileName fileName = m_db->GetFileName();
    m_db->CloseDb();
    m_db = nullptr;
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);
    {
    TestElementCPtr element = m_db->Elements().Get<TestElement>(eleid);
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue1, Orgindex));
    ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(DPoint3d::From(2, 1, 1))));
    checkValue1.Clear();
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue1, BBlindex));
    ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(DPoint3d::From(2, 2, 2))));
    checkValue1.Clear();
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue1, BBHindex));
    ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(DPoint3d::From(2, 4, 8))));
    checkValue1.Clear();
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue1, Yawindex));
    ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(4.5)));
    checkValue1.Clear();
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue1, Rollindex));
    ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(6.5)));
    checkValue1.Clear();
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue1, Pitchindex));
    ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(8.5)));
    checkValue1.Clear();
    }
    m_db->CloseDb();
    m_db = nullptr;
    //Update Properties
    OpenDb(m_db, fileName, Db::OpenMode::ReadWrite, true);
    {
    TestElementPtr eledit = m_db->Elements().GetForEdit<TestElement>(eleid);
    ASSERT_EQ(DgnDbStatus::Success, eledit->SetPropertyValue(Orgindex, ECN::ECValue(DPoint3d::From(2, 2, 1))));
    ASSERT_EQ(DgnDbStatus::Success, eledit->GetPropertyValue(checkValue1, Orgindex));
    ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(DPoint3d::From(2, 2, 1))));
    checkValue1.Clear();
    ASSERT_EQ(DgnDbStatus::Success, eledit->SetPropertyValue(BBlindex, ECN::ECValue(DPoint3d::From(2, 2, 4))));
    ASSERT_EQ(DgnDbStatus::Success, eledit->GetPropertyValue(checkValue1, BBlindex));
    ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(DPoint3d::From(2, 2, 4))));
    checkValue1.Clear();
    ASSERT_EQ(DgnDbStatus::Success, eledit->SetPropertyValue(BBHindex, ECN::ECValue(DPoint3d::From(2, 5, 8))));
    ASSERT_EQ(DgnDbStatus::Success, eledit->GetPropertyValue(checkValue1, BBHindex));
    ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(DPoint3d::From(2, 5, 8))));
    checkValue1.Clear();
    ASSERT_EQ(DgnDbStatus::Success, eledit->SetPropertyValue(Yawindex, ECN::ECValue(5.5)));
    ASSERT_EQ(DgnDbStatus::Success, eledit->GetPropertyValue(checkValue1, Yawindex));
    ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(5.5)));
    checkValue1.Clear();
    ASSERT_EQ(DgnDbStatus::Success, eledit->SetPropertyValue(Rollindex, ECN::ECValue(7.5)));
    ASSERT_EQ(DgnDbStatus::Success, eledit->GetPropertyValue(checkValue1, Rollindex));
    ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(7.5)));
    checkValue1.Clear();
    ASSERT_EQ(DgnDbStatus::Success, eledit->SetPropertyValue(Pitchindex, ECN::ECValue(9.5)));
    ASSERT_EQ(DgnDbStatus::Success, eledit->GetPropertyValue(checkValue1, Pitchindex));
    ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(9.5)));
    checkValue1.Clear();
    // Update the element
    DgnDbStatus stat;
    DgnElementCPtr updated_element = eledit->Update(&stat);
    ASSERT_EQ(DgnDbStatus::Success, stat);
    ASSERT_TRUE(updated_element.IsValid());
    m_db->SaveChanges();
    }
    m_db->CloseDb();
    m_db = nullptr;
    //check that the stored value was changed
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);
    TestElementCPtr element = m_db->Elements().Get<TestElement>(eleid);
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue1, Orgindex));
    ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(DPoint3d::From(2, 2, 1))));
    checkValue1.Clear();
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue1, BBlindex));
    ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(DPoint3d::From(2, 2, 4))));
    checkValue1.Clear();
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue1, BBHindex));
    ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(DPoint3d::From(2, 5, 8))));
    checkValue1.Clear();
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue1, Yawindex));
    ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(5.5)));
    checkValue1.Clear();
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue1, Rollindex));
    ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(7.5)));
    checkValue1.Clear();
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue1, Pitchindex));
    ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(9.5)));
    checkValue1.Clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GetSetCustomHandledProprty, 2dElementProprties)
    {
    //test Custom Attributes when we get them
    SetupSeedProject();
    DgnElementId eleid;
    ECN::ECValue checkValue1, checkValue2;
    uint32_t Orgindex,Rotindex, BBlindex, BBHindex;
    if (true)
        {
        DgnClassId classId(m_db->Schemas().GetECClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT2d_CLASS_NAME));
        DgnCategoryId categoryId = DgnDbTestUtils::InsertDrawingCategory(*m_db, "TestCategory");
        DocumentListModelPtr drawingListModel = DgnDbTestUtils::InsertDocumentListModel(*m_db, "DrawingListModel");
        DrawingPtr drawing = DgnDbTestUtils::InsertDrawing(*drawingListModel, "Drawing");
        DrawingModelPtr drawingModel = DgnDbTestUtils::InsertDrawingModel(*drawing);
        TestElement2d::CreateParams params(*m_db, drawingModel->GetModelId(), classId, categoryId, Placement2d(),DgnCode());
        TestElement2d el(params);
        ASSERT_TRUE(el.Is2d());
        //Check a few CustomhandleProperties 
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(Orgindex, "Origin"));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(Rotindex, "Rotation"));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(BBlindex, "BBoxLow"));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(BBHindex, "BBoxHigh"));

        // Try to set Invaild value type 
        ASSERT_EQ(DgnDbStatus::BadArg, el.SetPropertyValue(Orgindex, ECN::ECValue(true)));
        ASSERT_EQ(DgnDbStatus::BadArg, el.SetPropertyValue(BBlindex, ECN::ECValue(true)));
        ASSERT_EQ(DgnDbStatus::BadArg, el.SetPropertyValue(BBHindex, ECN::ECValue(true)));
        ASSERT_EQ(DgnDbStatus::BadArg, el.SetPropertyValue(Rotindex, ECN::ECValue(true)));

        ASSERT_EQ(DgnDbStatus::Success, el.SetPropertyValue(Orgindex, ECN::ECValue(DPoint2d::From(2, 1))));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue1, Orgindex));
        ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(DPoint2d::From(2, 1))));
        checkValue1.Clear();
        ASSERT_EQ(DgnDbStatus::Success, el.SetPropertyValue(BBlindex, ECN::ECValue(DPoint2d::From(2, 2))));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue1, BBlindex));
        ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(DPoint2d::From(2, 2))));
        checkValue1.Clear();
        ASSERT_EQ(DgnDbStatus::Success, el.SetPropertyValue(BBHindex, ECN::ECValue(DPoint2d::From(2, 4))));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue1, BBHindex));
        ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(DPoint2d::From(2, 4))));
        checkValue1.Clear();

        ASSERT_EQ(DgnDbStatus::Success, el.SetPropertyValue(Rotindex, ECN::ECValue(6.5)));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue1, Rotindex));
        ASSERT_EQ(checkValue1.GetDouble(),AngleInDegrees::FromRadians(6.5).Degrees());
        checkValue1.Clear();

        // Insert the element
        DgnDbStatus stat;
        DgnElementCPtr persistentEl = el.Insert(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat);
        ASSERT_TRUE(persistentEl.IsValid());

        eleid = persistentEl->GetElementId();
        m_db->SaveChanges();
        }
    // Before updatation of element check what stored in DB
    BeFileName fileName = m_db->GetFileName();
    m_db->CloseDb();
    m_db = nullptr;
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);
    {
    TestElement2dCPtr element = m_db->Elements().Get<TestElement2d>(eleid);
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue1, Orgindex));
    ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(DPoint2d::From(2, 1))));
    checkValue1.Clear();
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue1, BBlindex));
    ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(DPoint2d::From(2, 2))));
    checkValue1.Clear();
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue1, BBHindex));
    ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(DPoint2d::From(2, 4))));
    checkValue1.Clear();
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue1, Rotindex));
    ASSERT_EQ(checkValue1.GetDouble(), AngleInDegrees::FromRadians(6.5).Degrees());
    checkValue1.Clear();
    }
    m_db->CloseDb();
    m_db = nullptr;
    //Update Properties
    OpenDb(m_db, fileName, Db::OpenMode::ReadWrite, true);
    {
    TestElement2dPtr eledit = m_db->Elements().GetForEdit<TestElement2d>(eleid);
    ASSERT_EQ(DgnDbStatus::Success, eledit->SetPropertyValue(Orgindex, ECN::ECValue(DPoint2d::From(2,2))));
    ASSERT_EQ(DgnDbStatus::Success, eledit->GetPropertyValue(checkValue1, Orgindex));
    ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(DPoint2d::From(2, 2))));
    checkValue1.Clear();
    ASSERT_EQ(DgnDbStatus::Success, eledit->SetPropertyValue(BBlindex, ECN::ECValue(DPoint2d::From(2, 4))));
    ASSERT_EQ(DgnDbStatus::Success, eledit->GetPropertyValue(checkValue1, BBlindex));
    ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(DPoint2d::From(2, 4))));
    checkValue1.Clear();
    ASSERT_EQ(DgnDbStatus::Success, eledit->SetPropertyValue(BBHindex, ECN::ECValue(DPoint2d::From(2, 8))));
    ASSERT_EQ(DgnDbStatus::Success, eledit->GetPropertyValue(checkValue1, BBHindex));
    ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(DPoint2d::From(2, 8))));
    checkValue1.Clear();
    checkValue1.Clear();
    ASSERT_EQ(DgnDbStatus::Success, eledit->SetPropertyValue(Rotindex, ECN::ECValue(3.5)));
    ASSERT_EQ(DgnDbStatus::Success, eledit->GetPropertyValue(checkValue1, Rotindex));
    ASSERT_EQ(checkValue1.GetDouble(), AngleInDegrees::FromRadians(3.5).Degrees());
    checkValue1.Clear();
    // Update the element
    DgnDbStatus stat;
    DgnElementCPtr updated_element = eledit->Update(&stat);
    ASSERT_EQ(DgnDbStatus::Success, stat);
    ASSERT_TRUE(updated_element.IsValid());
    m_db->SaveChanges();
    }
    m_db->CloseDb();
    m_db = nullptr;
    //check that the stored value was changed
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);
    TestElement2dCPtr element = m_db->Elements().Get<TestElement2d>(eleid);
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue1, Orgindex));
    ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(DPoint2d::From(2, 2))));
    checkValue1.Clear();
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue1, BBlindex));
    ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(DPoint2d::From(2, 4))));
    checkValue1.Clear();
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue1, BBHindex));
    ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(DPoint2d::From(2, 8))));
    checkValue1.Clear();
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue1, Rotindex));
    ASSERT_EQ(checkValue1.GetDouble(), AngleInDegrees::FromRadians(3.5).Degrees());
    checkValue1.Clear();
    }
    /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GetSetCustomHandledProprty, CategoryProperties)
    {
    SetupSeedProject();
    DgnElementId eleid;
    ECN::ECValue checkValue;
    uint32_t catindex, rankindex, scatindex, scpropindex;
    DgnCategoryId categoryId;
    DgnSubCategoryId subcatid;
    DgnSubCategory::Appearance subappearence;
    if (true)
        {
        DgnClassId classId(m_db->Schemas().GetECClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT2d_CLASS_NAME));

        DrawingCategory category(*m_db, "TestCategory");
        ASSERT_EQ(DgnDbStatus::Success,category.GetPropertyIndex(catindex,"Descr"));
        ASSERT_EQ(DgnDbStatus::Success, category.SetPropertyValue(catindex, ECN::ECValue("Descr")));
        ASSERT_EQ(DgnDbStatus::Success, category.GetPropertyValue(checkValue, catindex));
        ASSERT_TRUE(checkValue.Equals(ECN::ECValue("Descr")));

        ASSERT_EQ(DgnDbStatus::Success, category.GetPropertyIndex(rankindex, "Rank"));
        ASSERT_EQ(DgnDbStatus::Success, category.SetPropertyValue(rankindex, ECN::ECValue(3)));
        ASSERT_EQ(DgnDbStatus::Success, category.GetPropertyValue(checkValue, rankindex));
        ASSERT_TRUE(checkValue.Equals(ECN::ECValue(3)));
        DgnSubCategory::Appearance appearance;
        DrawingCategoryCPtr persistentCategory = category.Insert(appearance);
        EXPECT_TRUE(persistentCategory.IsValid());
        categoryId=persistentCategory->GetCategoryId();
        m_db->SaveChanges();
        }
    BeFileName fileName = m_db->GetFileName();
    m_db->CloseDb();
    m_db = nullptr;
    //Check what stored in Db 
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);
    {
     DgnCategoryCPtr category = m_db->Elements().Get<DgnCategory>(categoryId);

     ASSERT_EQ(DgnDbStatus::Success, category->GetPropertyValue(checkValue, catindex));
     ASSERT_TRUE(checkValue.Equals(ECN::ECValue("Descr")));

     ASSERT_EQ(DgnDbStatus::Success, category->GetPropertyValue(checkValue, rankindex));
     ASSERT_TRUE(checkValue.Equals(ECN::ECValue(3)));
    }
    m_db->CloseDb();
    m_db = nullptr;
    //Update Properties
    OpenDb(m_db, fileName, Db::OpenMode::ReadWrite, true);
    {
    DgnCategoryPtr editcategory = m_db->Elements().GetForEdit<DgnCategory>(categoryId);

    ASSERT_EQ(DgnDbStatus::Success, editcategory->SetPropertyValue(catindex, ECN::ECValue("New Descr")));
    ASSERT_EQ(DgnDbStatus::Success, editcategory->GetPropertyValue(checkValue, catindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue("New Descr")));

    ASSERT_EQ(DgnDbStatus::Success, editcategory->SetPropertyValue(rankindex, ECN::ECValue(2)));
    ASSERT_EQ(DgnDbStatus::Success, editcategory->GetPropertyValue(checkValue, rankindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue(2)));

    subcatid=editcategory->GetDefaultSubCategoryId();
    DgnSubCategoryPtr editsubcategory = m_db->Elements().GetForEdit<DgnSubCategory>(subcatid);

    //Verify Subcategory description is readonly
    ASSERT_EQ(DgnDbStatus::Success, editsubcategory->GetPropertyIndex(scatindex, "Descr"));
    ASSERT_EQ(DgnDbStatus::ReadOnly, editsubcategory->SetPropertyValue(scatindex, ECN::ECValue("SubDescr")));
    subappearence.SetInvisible(false);
    subappearence.SetWeight(10);
    subappearence.SetColor(ColorDef::White());
    subappearence.SetTransparency(0.1);
    subappearence.SetDisplayPriority(2);
    ASSERT_EQ(DgnDbStatus::Success, editsubcategory->GetPropertyIndex(scpropindex, "Properties"));
    ASSERT_EQ(DgnDbStatus::Success, editsubcategory->SetPropertyValue(scpropindex, ECN::ECValue(subappearence.ToJson().c_str())));
    ASSERT_EQ(DgnDbStatus::Success, editsubcategory->GetPropertyValue(checkValue, scpropindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue(subappearence.ToJson().c_str())));
    DgnDbStatus  stat;
    editcategory->Update(&stat);
    ASSERT_EQ(stat,DgnDbStatus::Success);
    editsubcategory->Update(&stat);
    ASSERT_EQ(stat, DgnDbStatus::Success);
    m_db->SaveChanges();
    }
    m_db->CloseDb();
    m_db = nullptr;
    //Update Properties
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);
    DgnCategoryCPtr category = m_db->Elements().Get<DgnCategory>(categoryId);

    ASSERT_EQ(DgnDbStatus::Success, category->GetPropertyValue(checkValue, catindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue("New Descr")));

    ASSERT_EQ(DgnDbStatus::Success, category->GetPropertyValue(checkValue, rankindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue(2)));

    DgnSubCategoryCPtr subcategory = m_db->Elements().Get<DgnSubCategory>(subcatid);

    //Verify Subcategory description is readonly
    ASSERT_EQ(DgnDbStatus::Success, subcategory->GetPropertyValue(checkValue, scpropindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue(subappearence.ToJson().c_str())));
    }