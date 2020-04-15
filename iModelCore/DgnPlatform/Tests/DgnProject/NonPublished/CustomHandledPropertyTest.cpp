/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
TEST_F(GetSetCustomHandledProprty, ElementProperties)
    {
    SetupSeedProject();
    ECN::ECValue checkValue1, checkValue2;
    uint32_t LMindex, Spindex, Mindex, fgindex, codeScopeIndex, codeSpecIndex, codeValueIndex, jsindex, uvindex;
    DateTime dateTime = DateTime(DateTime::Kind::Utc, 2016, 2, 14, 9, 58, 17, 456);
    DgnCode code = DgnCode::CreateEmpty();
    BeGuid federationGuid(true);
    Json::Value json(Json::objectValue);
    json["dummy"] = "double";
    DgnElementId eleid;
    if (true)
    {
        DgnClassId classId(m_db->Schemas().GetClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_CLASS_NAME));
        TestElement::CreateParams params(*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d(), code);
        TestElement el(params);
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(LMindex, "LastMod"));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(Mindex, "Model"));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(Spindex, "InSpatialIndex"));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(fgindex, "FederationGuid"));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(codeSpecIndex, "CodeSpec"));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(codeScopeIndex, "CodeScope"));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(codeValueIndex, "CodeValue"));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(uvindex, "UserLabel"));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(jsindex, "JsonProperties"));

        // Try to set readonly property
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue1, LMindex));
        ASSERT_EQ(DgnDbStatus::ReadOnly, el.SetPropertyValue(LMindex, ECN::ECValue(dateTime)));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue2, LMindex));
        ASSERT_TRUE(checkValue1.Equals(checkValue2));
        checkValue1.Clear();
        checkValue2.Clear();

        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue1, Mindex));
        ASSERT_EQ(DgnDbStatus::ReadOnly, el.SetPropertyValue(Mindex, ECN::ECValue(2)));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue2, Mindex));
        ASSERT_TRUE(checkValue1.Equals(checkValue2));
        checkValue1.Clear();
        checkValue2.Clear();

        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue1, Spindex));
        ASSERT_EQ(DgnDbStatus::ReadOnly, el.SetPropertyValue(Spindex, ECN::ECValue(true)));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue2, Spindex));
        ASSERT_TRUE(checkValue1.Equals(checkValue2));
        checkValue1.Clear();
        checkValue2.Clear();

        ASSERT_EQ(DgnDbStatus::BadArg, el.SetPropertyValue(fgindex, ECN::ECValue(1)));
        ASSERT_EQ(DgnDbStatus::Success, el.SetPropertyValue(fgindex, ECN::ECValue((Byte*)&federationGuid, sizeof(federationGuid))));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue1, fgindex));
        ASSERT_TRUE(checkValue1.Equals(ECN::ECValue((Byte*)&federationGuid, sizeof(federationGuid))));
        checkValue1.Clear();

        ASSERT_EQ(DgnDbStatus::ReadOnly, el.SetPropertyValue(codeSpecIndex, ECN::ECValue(1)));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue1, codeSpecIndex));
        ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(code.GetCodeSpecId())));
        checkValue1.Clear();

        ASSERT_EQ(DgnDbStatus::ReadOnly, el.SetPropertyValue(codeScopeIndex, ECN::ECValue(1)));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue1, codeScopeIndex));
        ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(code.GetScopeElementId(*m_db))));
        checkValue1.Clear();

        ASSERT_EQ(DgnDbStatus::ReadOnly, el.SetPropertyValue(codeValueIndex, ECN::ECValue(1)));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue1, codeValueIndex));
        ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(code.GetValueUtf8().c_str())));
        checkValue1.Clear();

        ASSERT_EQ(DgnDbStatus::BadArg, el.SetPropertyValue(uvindex, ECN::ECValue(1)));
        ASSERT_EQ(DgnDbStatus::Success, el.SetPropertyValue(uvindex, ECN::ECValue("userlabel")));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue1, uvindex));
        ASSERT_TRUE(checkValue1.Equals(ECN::ECValue("userlabel")));

        ASSERT_EQ(DgnDbStatus::BadArg, el.SetPropertyValue(jsindex, ECN::ECValue(1)));
        ASSERT_EQ(DgnDbStatus::Success, el.SetPropertyValue(jsindex, ECN::ECValue(Json::FastWriter::ToString(json).c_str())));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue1, jsindex));
        ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(Json::FastWriter::ToString(json).c_str())));
        checkValue1.Clear();
        DgnElementCPtr ele = el.Insert();
        ASSERT_TRUE(ele.IsValid());
        eleid = ele->GetElementId();
    }
    // check what stored in DB
    BeFileName fileName = m_db->GetFileName();
    m_db->CloseDb();
    m_db = nullptr;
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);
    {
     DgnElementCPtr el= m_db->Elements().GetElement(eleid);
     ASSERT_EQ(DgnDbStatus::Success, el->GetPropertyValue(checkValue1, fgindex));
     ASSERT_TRUE(checkValue1.Equals(ECN::ECValue((Byte*)&federationGuid, sizeof(federationGuid))));
     checkValue1.Clear();
     ASSERT_EQ(DgnDbStatus::Success, el->GetPropertyValue(checkValue1, codeSpecIndex));
     ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(code.GetCodeSpecId())));
     checkValue1.Clear();
     ASSERT_EQ(DgnDbStatus::Success, el->GetPropertyValue(checkValue1, codeScopeIndex));
     ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(code.GetScopeElementId(*m_db))));
     checkValue1.Clear();
     ASSERT_EQ(DgnDbStatus::Success, el->GetPropertyValue(checkValue1, codeValueIndex));
     ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(code.GetValueUtf8().c_str())));
     checkValue1.Clear();
     ASSERT_EQ(DgnDbStatus::Success, el->GetPropertyValue(checkValue1, uvindex));
     ASSERT_TRUE(checkValue1.Equals(ECN::ECValue("userlabel")));
     checkValue1.Clear();
     ASSERT_EQ(DgnDbStatus::Success, el->GetPropertyValue(checkValue1, jsindex));
     ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(Json::FastWriter::ToString(json).c_str())));
     checkValue1.Clear();
    }
    m_db->CloseDb();
    m_db = nullptr;
    OpenDb(m_db, fileName, Db::OpenMode::ReadWrite, true);
    {
    DgnElementPtr el = m_db->Elements().GetForEdit<DgnElement>(eleid);

    ASSERT_EQ(DgnDbStatus::Success, el->SetPropertyValue(fgindex, ECN::ECValue((Byte*)&federationGuid, sizeof(federationGuid))));
    ASSERT_EQ(DgnDbStatus::Success, el->GetPropertyValue(checkValue1, fgindex));
    ASSERT_TRUE(checkValue1.Equals(ECN::ECValue((Byte*)&federationGuid, sizeof(federationGuid))));
    checkValue1.Clear();
    ASSERT_EQ(DgnDbStatus::Success, el->SetPropertyValue(uvindex, ECN::ECValue("label")));
    ASSERT_EQ(DgnDbStatus::Success, el->GetPropertyValue(checkValue1, uvindex));
    ASSERT_TRUE(checkValue1.Equals(ECN::ECValue("label")));
    json["Dum"] = "12";
    ASSERT_EQ(DgnDbStatus::Success, el->SetPropertyValue(jsindex, ECN::ECValue(Json::FastWriter::ToString(json).c_str())));
    ASSERT_EQ(DgnDbStatus::Success, el->GetPropertyValue(checkValue1, jsindex));
    ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(Json::FastWriter::ToString(json).c_str())));
    checkValue1.Clear();
    ASSERT_TRUE(el->Update().IsValid());
    }
    m_db->CloseDb();
    m_db = nullptr;
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);
    {
    DgnElementCPtr el = m_db->Elements().Get<DgnElement>(eleid);

    ASSERT_EQ(DgnDbStatus::Success, el->GetPropertyValue(checkValue1, fgindex));
    ASSERT_TRUE(checkValue1.Equals(ECN::ECValue((Byte*)&federationGuid, sizeof(federationGuid))));
    checkValue1.Clear();
    ASSERT_EQ(DgnDbStatus::Success, el->GetPropertyValue(checkValue1, uvindex));
    ASSERT_TRUE(checkValue1.Equals(ECN::ECValue("label")));
    ASSERT_EQ(DgnDbStatus::Success, el->GetPropertyValue(checkValue1, jsindex));
    ASSERT_TRUE(checkValue1.Equals(ECN::ECValue(Json::FastWriter::ToString(json).c_str())));
    }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GetSetCustomHandledProprty, InaccessibleProperty)
    {
    //test Custom Attributes when we get them
    SetupSeedProject();
    DgnClassId classId(m_db->Schemas().GetClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_CLASS_NAME));
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
TEST_F(GetSetCustomHandledProprty, ElementProperties3d)
    {
    //test Custom Attributes when we get them
    SetupSeedProject();
    DgnElementId eleid;
    ECN::ECValue checkValue1, checkValue2;
    uint32_t Orgindex, Yawindex, Pitchindex, Rollindex, BBlindex, BBHindex;
    if (true)
        {
        DgnClassId classId(m_db->Schemas().GetClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_CLASS_NAME));
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
TEST_F(GetSetCustomHandledProprty, ElementProperties2d)
    {
    //test Custom Attributes when we get them
    SetupSeedProject();
    DgnElementId eleid;
    ECN::ECValue checkValue1, checkValue2;
    uint32_t Orgindex,Rotindex, BBlindex, BBHindex;
    if (true)
        {
        DgnClassId classId(m_db->Schemas().GetClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT2d_CLASS_NAME));
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
        ASSERT_EQ(checkValue1.GetDouble(),AngleInDegrees::FromDegrees(6.5).Degrees());
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
    ASSERT_EQ(checkValue1.GetDouble(), AngleInDegrees::FromDegrees(6.5).Degrees());
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
    ASSERT_EQ(checkValue1.GetDouble(), AngleInDegrees::FromDegrees(3.5).Degrees());
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
    ASSERT_EQ(checkValue1.GetDouble(), AngleInDegrees::FromDegrees(3.5).Degrees());
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
        DgnClassId classId(m_db->Schemas().GetClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT2d_CLASS_NAME));

        DrawingCategory category(m_db->GetDictionaryModel(), "TestCategory");
        ASSERT_EQ(DgnDbStatus::Success,category.GetPropertyIndex(catindex,"Description"));
        ASSERT_EQ(DgnDbStatus::BadArg, category.SetPropertyValue(catindex, ECN::ECValue(true)));
        ASSERT_EQ(DgnDbStatus::Success, category.SetPropertyValue(catindex, ECN::ECValue("Description")));
        ASSERT_EQ(DgnDbStatus::Success, category.GetPropertyValue(checkValue, catindex));
        ASSERT_TRUE(checkValue.Equals(ECN::ECValue("Description")));

        ASSERT_EQ(DgnDbStatus::Success, category.GetPropertyIndex(rankindex, "Rank"));
        ASSERT_EQ(DgnDbStatus::BadArg, category.SetPropertyValue(rankindex, ECN::ECValue("r")));
        ASSERT_EQ(DgnDbStatus::Success, category.SetPropertyValue(rankindex, ECN::ECValue(3)));
        ASSERT_EQ(DgnDbStatus::Success, category.GetPropertyValue(checkValue, rankindex));
        ASSERT_TRUE(checkValue.Equals(ECN::ECValue(3)));
        DgnSubCategory::Appearance  appearance;
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
     ASSERT_TRUE(checkValue.Equals(ECN::ECValue("Description")));

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
    ASSERT_EQ(DgnDbStatus::Success, editsubcategory->GetPropertyIndex(scatindex, "Description"));
    ASSERT_EQ(DgnDbStatus::ReadOnly, editsubcategory->SetPropertyValue(scatindex, ECN::ECValue("SubDescr")));
    subappearence.SetInvisible(false);
    subappearence.SetWeight(10);
    subappearence.SetColor(ColorDef::White());
    subappearence.SetTransparency(0.1);
    subappearence.SetDisplayPriority(2);
    ASSERT_EQ(DgnDbStatus::Success, editsubcategory->GetPropertyIndex(scpropindex, "Properties"));
    ASSERT_EQ(DgnDbStatus::BadArg, editsubcategory->SetPropertyValue(scpropindex, ECN::ECValue(1)));
    ASSERT_EQ(DgnDbStatus::Success, editsubcategory->SetPropertyValue(scpropindex, ECN::ECValue(subappearence.ToJson().ToString().c_str())));
    ASSERT_EQ(DgnDbStatus::Success, editsubcategory->GetPropertyValue(checkValue, scpropindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue(subappearence.ToJson().ToString().c_str())));
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
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue(subappearence.ToJson().ToString().c_str())));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GetSetCustomHandledProprty, Annotation)
    {
    SetupSeedProject();
    uint32_t Tsdescrindex, Fsdescrindex, Lsdescrindex, tasdescrindex,dataindex;
    ECN::ECValue checkValue;
    AnnotationTextStyleId Tsid;
    AnnotationFrameStyleId Fsid;
    AnnotationLeaderStyleId Lsid;
    TextAnnotationSeedId tasid;
    if (true)
        {
        //Create AnnotationTextStyle
        AnnotationTextStylePtr textStyle = AnnotationTextStyle::Create(m_db->GetDictionaryModel());
        textStyle->SetName("MyStyle");
        ASSERT_EQ(textStyle->GetName(), "MyStyle");
        const static int DataSize = 10;
        Byte DummyData[DataSize] = { 1,2,3,4,5,6,7,8,9,10 };
        ASSERT_EQ(DgnDbStatus::Success, textStyle->GetPropertyIndex(Tsdescrindex, "Description"));
        ASSERT_EQ(DgnDbStatus::Success, textStyle->GetPropertyIndex(dataindex, "Data"));

        ASSERT_EQ(DgnDbStatus::BadArg, textStyle->SetPropertyValue(Tsdescrindex, ECN::ECValue(1)));
        ASSERT_EQ(DgnDbStatus::Success, textStyle->SetPropertyValue(Tsdescrindex, ECN::ECValue("MyTesxtStyle Descr")));
        ASSERT_EQ(DgnDbStatus::Success, textStyle->GetPropertyValue(checkValue, Tsdescrindex));
        ASSERT_TRUE(checkValue.Equals(ECN::ECValue("MyTesxtStyle Descr")));
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::BadRequest, textStyle->SetPropertyValue(dataindex, ECN::ECValue(DummyData,sizeof(DummyData))));
        ASSERT_EQ(DgnDbStatus::BadRequest, textStyle->GetPropertyValue(checkValue, dataindex));
        AnnotationTextStyleCPtr Ts = textStyle->Insert();
        Tsid = Ts->GetElementId();
        ASSERT_TRUE(Tsid.IsValid());
        //Create AnnotationFrameStyle
        AnnotationFrameStylePtr FrameStyle = AnnotationFrameStyle::Create(m_db->GetDictionaryModel());
        FrameStyle->SetName("MyFraneStyle");
        ASSERT_EQ(FrameStyle->GetName(), "MyFraneStyle");
        ASSERT_EQ(DgnDbStatus::Success, FrameStyle->GetPropertyIndex(Fsdescrindex, "Description"));
        ASSERT_EQ(DgnDbStatus::Success, FrameStyle->GetPropertyIndex(dataindex, "Data"));

        ASSERT_EQ(DgnDbStatus::BadArg,  FrameStyle->SetPropertyValue(Fsdescrindex, ECN::ECValue(1)));
        ASSERT_EQ(DgnDbStatus::Success, FrameStyle->SetPropertyValue(Fsdescrindex, ECN::ECValue("MyFrameStyle Descr")));
        ASSERT_EQ(DgnDbStatus::Success, FrameStyle->GetPropertyValue(checkValue, Fsdescrindex));
        ASSERT_TRUE(checkValue.Equals(ECN::ECValue("MyFrameStyle Descr")));
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::BadRequest, FrameStyle->SetPropertyValue(dataindex, ECN::ECValue(DummyData, sizeof(DummyData))));
        ASSERT_EQ(DgnDbStatus::BadRequest, FrameStyle->GetPropertyValue(checkValue, dataindex));
        AnnotationFrameStyleCPtr Fs = FrameStyle->Insert();
        Fsid = Fs->GetElementId();
        ASSERT_TRUE(Fsid.IsValid());
        //Create AnnotationLeaderStyle
        AnnotationLeaderStylePtr LeaderStyle = AnnotationLeaderStyle::Create(m_db->GetDictionaryModel());
        LeaderStyle->SetName("MyLeaderStyle");
        ASSERT_EQ(LeaderStyle->GetName(), "MyLeaderStyle");
        ASSERT_EQ(DgnDbStatus::Success, LeaderStyle->GetPropertyIndex(Lsdescrindex, "Description"));
        ASSERT_EQ(DgnDbStatus::Success, LeaderStyle->GetPropertyIndex(dataindex, "Data"));

        ASSERT_EQ(DgnDbStatus::BadArg,  LeaderStyle->SetPropertyValue(Lsdescrindex, ECN::ECValue(1)));
        ASSERT_EQ(DgnDbStatus::Success, LeaderStyle->SetPropertyValue(Lsdescrindex, ECN::ECValue("MyLeaderStyle Descr")));
        ASSERT_EQ(DgnDbStatus::Success, LeaderStyle->GetPropertyValue(checkValue, Lsdescrindex));
        ASSERT_TRUE(checkValue.Equals(ECN::ECValue("MyLeaderStyle Descr")));
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::BadRequest, LeaderStyle->SetPropertyValue(dataindex, ECN::ECValue(DummyData, sizeof(DummyData))));
        ASSERT_EQ(DgnDbStatus::BadRequest, LeaderStyle->GetPropertyValue(checkValue, dataindex));
        AnnotationLeaderStyleCPtr Ls = LeaderStyle->Insert();
        Lsid = Ls->GetElementId();
        ASSERT_TRUE(Lsid.IsValid());

        //Create TextAnnotationSeed
        TextAnnotationSeedPtr Textannoseed = TextAnnotationSeed::Create(m_db->GetDictionaryModel());
        Textannoseed->SetName("TextAnnotationSeed");
        ASSERT_EQ(Textannoseed->GetName(), "TextAnnotationSeed");
        ASSERT_EQ(DgnDbStatus::Success, Textannoseed->GetPropertyIndex(tasdescrindex, "Description"));
        ASSERT_EQ(DgnDbStatus::Success, Textannoseed->GetPropertyIndex(dataindex, "Data"));
        ASSERT_EQ(DgnDbStatus::Success, Textannoseed->SetPropertyValue(tasdescrindex, ECN::ECValue("TextAnnotationSeed Descr")));
        ASSERT_EQ(DgnDbStatus::Success, Textannoseed->GetPropertyValue(checkValue, tasdescrindex));
        ASSERT_TRUE(checkValue.Equals(ECN::ECValue("TextAnnotationSeed Descr")));
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::BadRequest, Textannoseed->SetPropertyValue(dataindex, ECN::ECValue(DummyData, sizeof(DummyData))));
        ASSERT_EQ(DgnDbStatus::BadRequest, Textannoseed->GetPropertyValue(checkValue, dataindex));
        TextAnnotationSeedCPtr Textannoseedc = Textannoseed->Insert();
        tasid = Textannoseedc->GetElementId();
        ASSERT_TRUE(tasid.IsValid());

        m_db->SaveChanges();
        }
    BeFileName fileName = m_db->GetFileName();
    m_db->CloseDb();
    m_db = nullptr;
    //Check what stored in Db
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);
    {
    AnnotationTextStyleCPtr textStyle = m_db->Elements().Get<AnnotationTextStyle>(Tsid);
    ASSERT_EQ(DgnDbStatus::Success, textStyle->GetPropertyValue(checkValue, Tsdescrindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue("MyTesxtStyle Descr")));
    checkValue.Clear();
    AnnotationFrameStyleCPtr FrameStyle = m_db->Elements().Get<AnnotationFrameStyle>(Fsid);
    ASSERT_EQ(DgnDbStatus::Success, FrameStyle->GetPropertyValue(checkValue, Fsdescrindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue("MyFrameStyle Descr")));
    checkValue.Clear();
    AnnotationLeaderStyleCPtr LeaderStyle = m_db->Elements().Get<AnnotationLeaderStyle>(Lsid);
    ASSERT_EQ(DgnDbStatus::Success, LeaderStyle->GetPropertyValue(checkValue, Lsdescrindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue("MyLeaderStyle Descr")));
    checkValue.Clear();
    TextAnnotationSeedCPtr Textannoseed = m_db->Elements().Get<TextAnnotationSeed>(tasid);
    ASSERT_EQ(DgnDbStatus::Success, Textannoseed->GetPropertyValue(checkValue, tasdescrindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue("TextAnnotationSeed Descr")));
    checkValue.Clear();
    }
    m_db->CloseDb();
    m_db = nullptr;
    //Update Properties
    OpenDb(m_db, fileName, Db::OpenMode::ReadWrite, true);
    {
    AnnotationTextStylePtr textStyle = m_db->Elements().GetForEdit<AnnotationTextStyle>(Tsid);
    ASSERT_EQ(DgnDbStatus::Success, textStyle->SetPropertyValue(Tsdescrindex, ECN::ECValue("NewTesxtStyle Descr")));
    ASSERT_EQ(DgnDbStatus::Success, textStyle->GetPropertyValue(checkValue, Tsdescrindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue("NewTesxtStyle Descr")));
    checkValue.Clear();
    AnnotationFrameStylePtr FrameStyle = m_db->Elements().GetForEdit<AnnotationFrameStyle>(Fsid);
    ASSERT_EQ(DgnDbStatus::Success, FrameStyle->SetPropertyValue(Fsdescrindex, ECN::ECValue("NewFrameStyle Descr")));
    ASSERT_EQ(DgnDbStatus::Success, FrameStyle->GetPropertyValue(checkValue, Fsdescrindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue("NewFrameStyle Descr")));
    checkValue.Clear();
    AnnotationLeaderStylePtr LeaderStyle = m_db->Elements().GetForEdit<AnnotationLeaderStyle>(Lsid);
    ASSERT_EQ(DgnDbStatus::Success, LeaderStyle->SetPropertyValue(Lsdescrindex, ECN::ECValue("NewLeaderStyle Descr")));
    ASSERT_EQ(DgnDbStatus::Success, LeaderStyle->GetPropertyValue(checkValue, Lsdescrindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue("NewLeaderStyle Descr")));
    checkValue.Clear();
    TextAnnotationSeedPtr Textannoseed = m_db->Elements().GetForEdit<TextAnnotationSeed>(tasid);
    ASSERT_EQ(DgnDbStatus::Success, Textannoseed->SetPropertyValue(tasdescrindex, ECN::ECValue("NewTextAnnotationSeed Descr")));
    ASSERT_EQ(DgnDbStatus::Success, Textannoseed->GetPropertyValue(checkValue, tasdescrindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue("NewTextAnnotationSeed Descr")));
    checkValue.Clear();

    ASSERT_TRUE(textStyle->Update().IsValid());
    ASSERT_TRUE(FrameStyle->Update().IsValid());
    ASSERT_TRUE(LeaderStyle->Update().IsValid());
    ASSERT_TRUE(Textannoseed->Update().IsValid());
    }
    m_db->CloseDb();
    m_db = nullptr;
    //Check values update in Db
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);
    {
    AnnotationTextStyleCPtr textStyle = m_db->Elements().Get<AnnotationTextStyle>(Tsid);
    ASSERT_EQ(DgnDbStatus::Success, textStyle->GetPropertyValue(checkValue, Tsdescrindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue("NewTesxtStyle Descr")));
    checkValue.Clear();
    AnnotationFrameStyleCPtr FrameStyle = m_db->Elements().Get<AnnotationFrameStyle>(Fsid);
    ASSERT_EQ(DgnDbStatus::Success, FrameStyle->GetPropertyValue(checkValue, Fsdescrindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue("NewFrameStyle Descr")));
    checkValue.Clear();
    AnnotationLeaderStyleCPtr LeaderStyle = m_db->Elements().Get<AnnotationLeaderStyle>(Lsid);
    ASSERT_EQ(DgnDbStatus::Success, LeaderStyle->GetPropertyValue(checkValue, Lsdescrindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue("NewLeaderStyle Descr")));
    checkValue.Clear();
    TextAnnotationSeedCPtr Textannoseed = m_db->Elements().Get<TextAnnotationSeed>(tasid);
    ASSERT_EQ(DgnDbStatus::Success, Textannoseed->GetPropertyValue(checkValue, tasdescrindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue("NewTextAnnotationSeed Descr")));
    checkValue.Clear();
    }
    m_db->CloseDb();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GetSetCustomHandledProprty, Linkelement)
    {
    SetupSeedProject();
    uint32_t ulindex, udescindex,rindex,enindex,edescindex;
    ECN::ECValue checkValue;
    DgnElementId linkid, emlinkid;

    if (true)
        {
        LinkModelPtr linkModel = DgnDbTestUtils::InsertLinkModel(*m_db, "TestLinkModel");
        //UrlLink
        UrlLinkPtr link = UrlLink::Create(UrlLink::CreateParams(*linkModel));
        ASSERT_EQ(DgnDbStatus::Success, link->GetPropertyIndex(ulindex, "Url"));
        ASSERT_EQ(DgnDbStatus::Success, link->GetPropertyIndex(udescindex, "Description"));

        ASSERT_EQ(DgnDbStatus::BadArg, link->SetPropertyValue(ulindex, ECN::ECValue(1)));
        ASSERT_EQ(DgnDbStatus::Success, link->SetPropertyValue(ulindex, ECN::ECValue("http://www.google.com")));
        ASSERT_EQ(DgnDbStatus::Success, link->GetPropertyValue(checkValue, ulindex));
        ASSERT_TRUE(checkValue.Equals(ECN::ECValue("http://www.google.com")));
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::BadArg, link->SetPropertyValue(udescindex, ECN::ECValue(1)));
        ASSERT_EQ(DgnDbStatus::Success, link->SetPropertyValue(udescindex, ECN::ECValue("Description")));
        ASSERT_EQ(DgnDbStatus::Success, link->GetPropertyValue(checkValue, udescindex));
        ASSERT_TRUE(checkValue.Equals(ECN::ECValue("Description")));
        checkValue.Clear();
        UrlLinkCPtr linkele = link->Insert();
        ASSERT_TRUE(linkele.IsValid());
        linkid = linkele->GetElementId();
        ASSERT_TRUE(linkid.IsValid());
        //Repositorylink
        RepositoryLinkPtr rlink = RepositoryLink::Create(*linkModel, "http://www.outlook.com", "Rlink Lable");
        BeGuid guid;
        guid.Create();
        rlink->SetRepositoryGuid(guid);
        ASSERT_TRUE(guid == rlink->GetRepositoryGuid());
        ASSERT_TRUE(guid == rlink->GetPropertyValueGuid("RepositoryGuid"));
        ASSERT_EQ(DgnDbStatus::Success, rlink->GetPropertyIndex(rindex, "RepositoryGuid"));
        ASSERT_EQ(DgnDbStatus::Success, rlink->GetPropertyValue(checkValue, rindex));
        size_t checkGuidSize;
        BeGuid* checkGuid = (BeGuid*)checkValue.GetBinary(checkGuidSize);
        ASSERT_EQ(sizeof(BeGuid), checkGuidSize);
        ASSERT_TRUE(guid == *checkGuid);
        ASSERT_EQ(DgnDbStatus::Success, rlink->GetPropertyIndex(rindex, "Description"));
        ASSERT_EQ(DgnDbStatus::Success, rlink->SetPropertyValue(rindex, ECN::ECValue("Description")));
        ASSERT_EQ(DgnDbStatus::Success, rlink->GetPropertyValue(checkValue, rindex));
        ASSERT_STREQ("Description", checkValue.GetUtf8CP());
        ASSERT_TRUE(rlink->Insert().IsValid());
        //EmbeddedFileLink
        EmbeddedFileLinkPtr emlink = EmbeddedFileLink::Create(EmbeddedFileLink::CreateParams(*linkModel, ""));
        ASSERT_EQ(DgnDbStatus::Success, emlink->GetPropertyIndex(enindex, "Name"));
        ASSERT_EQ(DgnDbStatus::Success, emlink->GetPropertyIndex(edescindex, "Description"));

        ASSERT_EQ(DgnDbStatus::BadArg, emlink->SetPropertyValue(enindex, ECN::ECValue(1)));
        ASSERT_EQ(DgnDbStatus::Success, emlink->SetPropertyValue(enindex, ECN::ECValue("EmFile")));
        ASSERT_EQ(DgnDbStatus::Success, emlink->GetPropertyValue(checkValue, enindex));
        ASSERT_TRUE(checkValue.Equals(ECN::ECValue("EmFile")));
        checkValue.Clear();

        ASSERT_EQ(DgnDbStatus::BadArg, emlink->SetPropertyValue(edescindex, ECN::ECValue(1)));
        ASSERT_EQ(DgnDbStatus::Success, emlink->SetPropertyValue(edescindex, ECN::ECValue("Description")));
        ASSERT_EQ(DgnDbStatus::Success, emlink->GetPropertyValue(checkValue, edescindex));
        ASSERT_TRUE(checkValue.Equals(ECN::ECValue("Description")));
        checkValue.Clear();
        EmbeddedFileLinkCPtr emlinkele = emlink->Insert();
        ASSERT_TRUE(emlinkele.IsValid());
        emlinkid = emlinkele->GetElementId();
        ASSERT_TRUE(emlinkid.IsValid());
        m_db->SaveChanges();
        }
    BeFileName fileName = m_db->GetFileName();
    m_db->CloseDb();
    m_db = nullptr;
    //Check what stored in Db
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, false);
    {
    UrlLinkCPtr link = UrlLink::Get(*m_db,linkid);
    ASSERT_TRUE(link.IsValid());
    ASSERT_EQ(DgnDbStatus::Success, link->GetPropertyValue(checkValue, ulindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue("http://www.google.com")));
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, link->GetPropertyValue(checkValue, udescindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue("Description")));
    checkValue.Clear();

    EmbeddedFileLinkCPtr emlink = m_db->Elements().Get<EmbeddedFileLink>(emlinkid);
    ASSERT_TRUE(emlink.IsValid());
    ASSERT_EQ(DgnDbStatus::Success, emlink->GetPropertyValue(checkValue, enindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue("EmFile")));
    checkValue.Clear();

    ASSERT_EQ(DgnDbStatus::Success, emlink->GetPropertyValue(checkValue, edescindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue("Description")));
    checkValue.Clear();
    }
    m_db->CloseDb();
    m_db = nullptr;
    //Update properties
    OpenDb(m_db, fileName, Db::OpenMode::ReadWrite, true);
    {
    UrlLinkPtr link = UrlLink::GetForEdit(*m_db,linkid);

    ASSERT_EQ(DgnDbStatus::Success, link->SetPropertyValue(ulindex, ECN::ECValue("https://www.google.com")));
    ASSERT_EQ(DgnDbStatus::Success, link->GetPropertyValue(checkValue, ulindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue("https://www.google.com")));
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, link->SetPropertyValue(udescindex, ECN::ECValue("NDescr")));
    ASSERT_EQ(DgnDbStatus::Success, link->GetPropertyValue(checkValue, udescindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue("NDescr")));
    checkValue.Clear();

    EmbeddedFileLinkPtr emlink = EmbeddedFileLink::GetForEdit(*m_db,emlinkid);
    ASSERT_EQ(DgnDbStatus::Success, emlink->SetPropertyValue(enindex, ECN::ECValue("EmFile1")));
    ASSERT_EQ(DgnDbStatus::Success, emlink->GetPropertyValue(checkValue, enindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue("EmFile1")));
    checkValue.Clear();

    ASSERT_EQ(DgnDbStatus::Success, emlink->SetPropertyValue(edescindex, ECN::ECValue("NDescr")));
    ASSERT_EQ(DgnDbStatus::Success, emlink->GetPropertyValue(checkValue, edescindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue("NDescr")));
    checkValue.Clear();;

    ASSERT_TRUE(link->Update().IsValid());
    ASSERT_TRUE(emlink->Update().IsValid());
    }
    m_db->CloseDb();
    m_db = nullptr;
    //Check what stored in Db
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, false);
    {
    UrlLinkCPtr link = m_db->Elements().Get<UrlLink>(linkid);
    ASSERT_EQ(DgnDbStatus::Success, link->GetPropertyValue(checkValue, ulindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue("https://www.google.com")));
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, link->GetPropertyValue(checkValue, udescindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue("NDescr")));
    checkValue.Clear();

    EmbeddedFileLinkCPtr emlink = m_db->Elements().Get< EmbeddedFileLink>(emlinkid);
    ASSERT_EQ(DgnDbStatus::Success, emlink->GetPropertyValue(checkValue, enindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue("EmFile1")));
    checkValue.Clear();

    ASSERT_EQ(DgnDbStatus::Success, emlink->GetPropertyValue(checkValue, edescindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue("NDescr")));
    checkValue.Clear();
    }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GetSetCustomHandledProprty, GeometryPart)
    {
    SetupSeedProject();
    uint32_t gindex, blindex, bhindex;
    ECN::ECValue checkValue;
    DgnGeometryPartId existingPartId;
    if (true)
    {
    DgnGeometryPartPtr geomPartPtr = DgnGeometryPart::Create(m_db->GetDictionaryModel(), "Test-GeomPart");
    EXPECT_TRUE(geomPartPtr != NULL);
    GeometryBuilderPtr builder = GeometryBuilder::CreateGeometryPart(*m_db, false);
    EXPECT_EQ(SUCCESS, builder->Finish(*geomPartPtr));
    ASSERT_EQ(DgnDbStatus::Success, geomPartPtr->GetPropertyIndex(gindex, "GeometryStream"));
    ASSERT_EQ(DgnDbStatus::Success, geomPartPtr->GetPropertyIndex(blindex, "BBoxLow"));
    ASSERT_EQ(DgnDbStatus::Success, geomPartPtr->GetPropertyIndex(bhindex, "BBoxHigh"));

    ASSERT_EQ(DgnDbStatus::BadRequest, geomPartPtr->SetPropertyValue(gindex, ECN::ECValue(1234)));
    ASSERT_EQ(DgnDbStatus::BadRequest, geomPartPtr->GetPropertyValue(checkValue, gindex));
    checkValue.Clear();

    ASSERT_EQ(DgnDbStatus::BadArg, geomPartPtr->SetPropertyValue(blindex, ECN::ECValue(true)));
    ASSERT_EQ(DgnDbStatus::Success, geomPartPtr->SetPropertyValue(blindex, ECN::ECValue(DPoint3d::From(0,1,2))));
    ASSERT_EQ(DgnDbStatus::Success, geomPartPtr->GetPropertyValue(checkValue, blindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue(DPoint3d::From(0, 1, 2))));
    checkValue.Clear();

    ASSERT_EQ(DgnDbStatus::BadArg, geomPartPtr->SetPropertyValue(bhindex, ECN::ECValue(true)));
    ASSERT_EQ(DgnDbStatus::Success, geomPartPtr->SetPropertyValue(bhindex, ECN::ECValue(DPoint3d::From(0, 2, 2))));
    ASSERT_EQ(DgnDbStatus::Success, geomPartPtr->GetPropertyValue(checkValue, bhindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue(DPoint3d::From(0, 2, 2))));
    checkValue.Clear();
    ASSERT_TRUE(m_db->Elements().Insert<DgnGeometryPart>(*geomPartPtr).IsValid());
    existingPartId = DgnGeometryPart::QueryGeometryPartId(*m_db, geomPartPtr->GetCode());
    EXPECT_TRUE(existingPartId.IsValid());
    m_db->SaveChanges();
    }
    BeFileName fileName = m_db->GetFileName();
    m_db->CloseDb();
    m_db = nullptr;
    //Check what stored in Db
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);
    {
    DgnGeometryPartCPtr geoele = m_db->Elements().Get<DgnGeometryPart>(existingPartId);
    ASSERT_TRUE(geoele.IsValid());
    ASSERT_EQ(DgnDbStatus::Success, geoele->GetPropertyValue(checkValue, blindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue(DPoint3d::From(0, 1, 2))));
    checkValue.Clear();

    ASSERT_EQ(DgnDbStatus::Success, geoele->GetPropertyValue(checkValue, bhindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue(DPoint3d::From(0, 2, 2))));
    checkValue.Clear();
    }
    m_db->CloseDb();
    m_db = nullptr;
    //Update Properties
    OpenDb(m_db, fileName, Db::OpenMode::ReadWrite, true);
    {
    DgnGeometryPartPtr geoele = m_db->Elements().GetForEdit<DgnGeometryPart>(existingPartId);
    ASSERT_TRUE(geoele.IsValid());
    ASSERT_EQ(DgnDbStatus::Success, geoele->SetPropertyValue(blindex, ECN::ECValue(DPoint3d::From(1, 1, 2))));
    ASSERT_EQ(DgnDbStatus::Success, geoele->GetPropertyValue(checkValue, blindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue(DPoint3d::From(1, 1, 2))));
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, geoele->SetPropertyValue(bhindex, ECN::ECValue(DPoint3d::From(2, 2, 2))));
    ASSERT_EQ(DgnDbStatus::Success, geoele->GetPropertyValue(checkValue, bhindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue(DPoint3d::From(2, 2, 2))));
    checkValue.Clear();
    ASSERT_TRUE(geoele->Update().IsValid());
    }
    m_db->CloseDb();
    m_db = nullptr;
    //Check updated Properties
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);
    {
    DgnGeometryPartPtr geoele = m_db->Elements().GetForEdit<DgnGeometryPart>(existingPartId);
    ASSERT_TRUE(geoele.IsValid());
    ASSERT_EQ(DgnDbStatus::Success, geoele->GetPropertyValue(checkValue, blindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue(DPoint3d::From(1, 1, 2))));
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, geoele->GetPropertyValue(checkValue, bhindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue(DPoint3d::From(2, 2, 2))));
    checkValue.Clear();
    }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GetSetCustomHandledProprty, Viewdefinition2d)
    {
    SetupSeedProject();
    m_db->Schemas().CreateClassViewsInDb();
    uint32_t orindex,exindex,raindex;
    ECN::ECValue checkValue;
    DgnViewId viewId;
    if (true)
        {
        DefinitionModelR dictionary = m_db->GetDictionaryModel();
        auto categories = new CategorySelector(dictionary, "");
        for (ElementIteratorEntryCR categoryEntry : DrawingCategory::MakeIterator(*m_db))
        categories->AddCategory(categoryEntry.GetId<DgnCategoryId>());

        auto style = new DisplayStyle2d(dictionary, "");
        auto flags = style->GetViewFlags();
        flags.SetRenderMode(Render::RenderMode::SmoothShade);
        style->SetViewFlags(flags);

        DocumentListModelPtr drawingListModel = DgnDbTestUtils::InsertDocumentListModel(*m_db, "DrawingListModel");
        DrawingPtr drawing = DgnDbTestUtils::InsertDrawing(*drawingListModel, "TestDrawing");
        DrawingModelPtr drawingModel = DgnDbTestUtils::InsertDrawingModel(*drawing);

        DrawingViewDefinition view(dictionary, "Default", drawingModel->GetModelId(),*categories, *style);
        ASSERT_EQ(DgnDbStatus::Success, view.GetPropertyIndex(orindex, "Origin"));
        ASSERT_EQ(DgnDbStatus::Success, view.GetPropertyIndex(exindex, "Extents"));
        ASSERT_EQ(DgnDbStatus::Success, view.GetPropertyIndex(raindex, "RotationAngle"));

        ASSERT_EQ(DgnDbStatus::BadArg, view.SetPropertyValue(orindex, ECN::ECValue(true)));
        ASSERT_EQ(DgnDbStatus::Success, view.SetPropertyValue(orindex, ECN::ECValue(DPoint2d::From(0, 1))));
        ASSERT_EQ(DgnDbStatus::Success, view.GetPropertyValue(checkValue, orindex));
        ASSERT_TRUE(checkValue.Equals(ECN::ECValue(DPoint2d::From(0, 1))));
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::BadArg, view.SetPropertyValue(exindex, ECN::ECValue(true)));
        ASSERT_EQ(DgnDbStatus::Success, view.SetPropertyValue(exindex, ECN::ECValue(DPoint2d::From(1, 1))));
        ASSERT_EQ(DgnDbStatus::Success, view.GetPropertyValue(checkValue, exindex));
        ASSERT_TRUE(checkValue.Equals(ECN::ECValue(DPoint2d::From(1, 1))));
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::BadArg, view.SetPropertyValue(raindex, ECN::ECValue(true)));
        ASSERT_EQ(DgnDbStatus::Success, view.SetPropertyValue(raindex, ECN::ECValue(2.5)));
        ASSERT_EQ(DgnDbStatus::Success, view.GetPropertyValue(checkValue, raindex));
        ASSERT_EQ(checkValue.GetDouble(),ECN::ECValue(2.5).GetDouble());
        checkValue.Clear();
        ASSERT_TRUE(view.Insert().IsValid());

        viewId = view.GetViewId();
        }
    BeFileName fileName = m_db->GetFileName();
    m_db->CloseDb();
    m_db = nullptr;
    //Check what stored in Db
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);
    {
    DrawingViewDefinitionCPtr view= m_db->Elements().Get<DrawingViewDefinition>(viewId);

    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, orindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue(DPoint2d::From(0, 1))));
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, exindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue(DPoint2d::From(1, 1))));
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, raindex));
    ASSERT_EQ(checkValue.GetDouble(), ECN::ECValue(2.5).GetDouble());
    checkValue.Clear();
    }
    m_db->CloseDb();
    m_db = nullptr;
    //upadate properties
    OpenDb(m_db, fileName, Db::OpenMode::ReadWrite, true);
    {
    DrawingViewDefinitionPtr view = m_db->Elements().GetForEdit<DrawingViewDefinition>(viewId);

    ASSERT_EQ(DgnDbStatus::Success, view->SetPropertyValue(orindex, ECN::ECValue(DPoint2d::From(1, 1))));
    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, orindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue(DPoint2d::From(1, 1))));
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, view->SetPropertyValue(exindex, ECN::ECValue(DPoint2d::From(2, 2))));
    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, exindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue(DPoint2d::From(2, 2))));
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, view->SetPropertyValue(raindex, ECN::ECValue(2.8)));
    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, raindex));
    ASSERT_EQ(checkValue.GetDouble(), ECN::ECValue(2.8).GetDouble());
    checkValue.Clear();
    ASSERT_TRUE(view->Update().IsValid());
    }
    m_db->CloseDb();
    m_db = nullptr;
    //Check upadated properties stored in Db
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);
    {
    DrawingViewDefinitionCPtr view = m_db->Elements().Get<DrawingViewDefinition>(viewId);

    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, orindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue(DPoint2d::From(1, 1))));
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, exindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue(DPoint2d::From(2,2))));
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, raindex));
    ASSERT_EQ(checkValue.GetDouble(), ECN::ECValue(2.8).GetDouble());
    checkValue.Clear();
    }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GetSetCustomHandledProprty, Viewdefinition3d)
    {
    SetupSeedProject();
    uint32_t orindex,exindex,yindex,pindex,rindex,caindex, laindex, fdindex,epindex;
    ECN::ECValue checkValue;
    DgnViewId viewId;
    if (true)
        {
        DefinitionModelR dictionary = m_db->GetDictionaryModel();
        auto categories = new CategorySelector(dictionary, "");
        for (ElementIteratorEntryCR categoryEntry : SpatialCategory::MakeIterator(*m_db))
        categories->AddCategory(categoryEntry.GetId<DgnCategoryId>());

        auto style = new DisplayStyle3d(dictionary, "");
        auto flags = style->GetViewFlags();
        flags.SetRenderMode(Render::RenderMode::SmoothShade);
        style->SetViewFlags(flags);

        auto models = new ModelSelector(dictionary, "");
        ModelIterator modIter = m_db->Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_SpatialModel));
        for (ModelIteratorEntryCR entry : modIter)
            {
            auto id = entry.GetModelId();
            auto model = m_db->Models().GetModel(id);

            if (model.IsValid())
                models->AddModel(id);
             }

        SpatialViewDefinition view(dictionary, "Default", *categories, *style, *models);
        view.SetStandardViewRotation(StandardView::Iso);
        ASSERT_EQ(DgnDbStatus::Success, view.GetPropertyIndex(orindex, "Origin"));
        ASSERT_EQ(DgnDbStatus::Success, view.GetPropertyIndex(exindex, "Extents"));
        ASSERT_EQ(DgnDbStatus::Success, view.GetPropertyIndex(yindex, "Yaw"));
        ASSERT_EQ(DgnDbStatus::Success, view.GetPropertyIndex(rindex, "Roll"));
        ASSERT_EQ(DgnDbStatus::Success, view.GetPropertyIndex(pindex, "Pitch"));
        ASSERT_EQ(DgnDbStatus::Success, view.GetPropertyIndex(caindex, "IsCameraOn"));
        ASSERT_EQ(DgnDbStatus::Success, view.GetPropertyIndex(laindex, "LensAngle"));
        ASSERT_EQ(DgnDbStatus::Success, view.GetPropertyIndex(epindex, "EyePoint"));
        ASSERT_EQ(DgnDbStatus::Success, view.GetPropertyIndex(fdindex, "FocusDistance"));

        ASSERT_EQ(DgnDbStatus::BadArg, view.SetPropertyValue(orindex, ECN::ECValue(true)));
        ASSERT_EQ(DgnDbStatus::Success, view.SetPropertyValue(orindex, ECN::ECValue(DPoint3d::From(0, 1, 1))));
        ASSERT_EQ(DgnDbStatus::Success, view.GetPropertyValue(checkValue, orindex));
        ASSERT_TRUE(checkValue.Equals(ECN::ECValue(DPoint3d::From(0, 1, 1))));
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::BadArg, view.SetPropertyValue(exindex, ECN::ECValue(true)));
        ASSERT_EQ(DgnDbStatus::Success, view.SetPropertyValue(exindex, ECN::ECValue(DPoint3d::From(1, 1, 1))));
        ASSERT_EQ(DgnDbStatus::Success, view.GetPropertyValue(checkValue, exindex));
        ASSERT_TRUE(checkValue.Equals(ECN::ECValue(DPoint3d::From(1, 1, 1))));
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::BadArg, view.SetPropertyValue(yindex, ECN::ECValue(true)));
        ASSERT_EQ(DgnDbStatus::Success, view.SetPropertyValue(yindex, ECN::ECValue(2.5)));
        ASSERT_EQ(DgnDbStatus::Success, view.GetPropertyValue(checkValue, yindex));
        ASSERT_EQ(checkValue,ECN::ECValue(2.5));
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::BadArg, view.SetPropertyValue(pindex, ECN::ECValue(true)));
        ASSERT_EQ(DgnDbStatus::Success, view.SetPropertyValue(pindex, ECN::ECValue(3.5)));
        ASSERT_EQ(DgnDbStatus::Success, view.GetPropertyValue(checkValue, pindex));
        ASSERT_EQ(checkValue,ECN::ECValue(3.5));
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::BadArg, view.SetPropertyValue(rindex, ECN::ECValue(true)));
        ASSERT_EQ(DgnDbStatus::Success, view.SetPropertyValue(rindex, ECN::ECValue(4.5)));
        ASSERT_EQ(DgnDbStatus::Success, view.GetPropertyValue(checkValue, rindex));
        ASSERT_EQ(checkValue, ECN::ECValue(4.5));
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::ReadOnly, view.SetPropertyValue(caindex, ECN::ECValue(true)));
        ASSERT_EQ(DgnDbStatus::BadArg, view.SetPropertyValue(laindex, ECN::ECValue(true)));
        ASSERT_EQ(DgnDbStatus::Success, view.SetPropertyValue(laindex, ECN::ECValue(5.5)));
        ASSERT_EQ(DgnDbStatus::Success, view.GetPropertyValue(checkValue, laindex));
        ASSERT_EQ(checkValue.GetDouble(), ECN::ECValue(5.5).GetDouble());
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::BadArg, view.SetPropertyValue(fdindex, ECN::ECValue(true)));
        ASSERT_EQ(DgnDbStatus::Success, view.SetPropertyValue(fdindex, ECN::ECValue(7.5)));
        ASSERT_EQ(DgnDbStatus::Success, view.GetPropertyValue(checkValue, fdindex));
        ASSERT_TRUE(checkValue.Equals(ECN::ECValue(7.5)));
        checkValue.Clear();
        ASSERT_EQ(DgnDbStatus::BadArg, view.SetPropertyValue(epindex, ECN::ECValue(true)));
        ASSERT_EQ(DgnDbStatus::Success, view.SetPropertyValue(epindex, ECN::ECValue(DPoint3d::From(0,1,2))));
        ASSERT_EQ(DgnDbStatus::Success, view.GetPropertyValue(checkValue, epindex));
        ASSERT_TRUE(checkValue.Equals(ECN::ECValue(DPoint3d::From(0, 1, 2))));
        checkValue.Clear();
        ASSERT_TRUE(view.Insert().IsValid());

        viewId = view.GetViewId();
        }
    BeFileName fileName = m_db->GetFileName();
    m_db->CloseDb();
    m_db = nullptr;
    //Check what stored in Db
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);
    {
    SpatialViewDefinitionCPtr view= m_db->Elements().Get<SpatialViewDefinition>(viewId);

    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, orindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue(DPoint3d::From(0, 1, 1))));
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, exindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue(DPoint3d::From(1, 1, 1))));
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, yindex));
    ASSERT_EQ(checkValue, ECN::ECValue(2.5));
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, pindex));
    ASSERT_EQ(checkValue, ECN::ECValue(3.5));
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, rindex));
    ASSERT_EQ(checkValue, ECN::ECValue(4.5));
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, caindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue(false)));
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, laindex));
    ASSERT_EQ(checkValue.GetDouble(), ECN::ECValue(5.5).GetDouble());
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, fdindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue(7.5)));
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, epindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue(DPoint3d::From(0, 1, 2))));
    checkValue.Clear();
    }
    m_db->CloseDb();
    m_db = nullptr;
    //upadate properties
    OpenDb(m_db, fileName, Db::OpenMode::ReadWrite, true);
    {
    SpatialViewDefinitionPtr view = m_db->Elements().GetForEdit<SpatialViewDefinition>(viewId);

    ASSERT_EQ(DgnDbStatus::Success, view->SetPropertyValue(orindex, ECN::ECValue(DPoint3d::From(1, 1, 1))));
    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, orindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue(DPoint3d::From(1, 1, 1))));
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, view->SetPropertyValue(exindex, ECN::ECValue(DPoint3d::From(1, 2, 1))));
    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, exindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue(DPoint3d::From(1, 2, 1))));
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, view->SetPropertyValue(yindex, ECN::ECValue(2.8)));
    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, yindex));
    ASSERT_EQ(checkValue, ECN::ECValue(2.8));
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, view->SetPropertyValue(pindex, ECN::ECValue(3.0)));
    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, pindex));
    ASSERT_EQ(checkValue, ECN::ECValue(3.0));
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, view->SetPropertyValue(rindex, ECN::ECValue(4.8)));
    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, rindex));
    ASSERT_EQ(checkValue, ECN::ECValue(4.8));
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, view->SetPropertyValue(laindex, ECN::ECValue(6.0)));
    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, laindex));
    ASSERT_EQ(checkValue.GetDouble(), ECN::ECValue(6.0).GetDouble());
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, view->SetPropertyValue(fdindex, ECN::ECValue(5.5)));
    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, fdindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue(5.5)));
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, view->SetPropertyValue(epindex, ECN::ECValue(DPoint3d::From(2, 1, 2))));
    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, epindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue(DPoint3d::From(2, 1, 2))));
    checkValue.Clear();
    ASSERT_TRUE(view->Update().IsValid());
    }
    m_db->CloseDb();
    m_db = nullptr;
    //Check upadated properties stored in Db
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);
    {
    SpatialViewDefinitionCPtr view = m_db->Elements().Get<SpatialViewDefinition>(viewId);

    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, orindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue(DPoint3d::From(1, 1, 1))));
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, exindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue(DPoint3d::From(1, 2, 1))));
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, yindex));
    ASSERT_EQ(checkValue, ECN::ECValue(2.8));
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, pindex));
    ASSERT_EQ(checkValue, ECN::ECValue(3.0));
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, rindex));
    ASSERT_EQ(checkValue, ECN::ECValue(4.8));
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, laindex));
    ASSERT_EQ(checkValue.GetDouble(), ECN::ECValue(6.0).GetDouble());
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, fdindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue(5.5)));
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, view->GetPropertyValue(checkValue, epindex));
    ASSERT_TRUE(checkValue.Equals(ECN::ECValue(DPoint3d::From(2, 1, 2))));
    checkValue.Clear();
    }
    }
