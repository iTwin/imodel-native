/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/TestFixture/DgnDbTestFixtures.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "DgnDbTestFixtures.h"

//Macros to define members of DgnDomain and Handler
HANDLER_DEFINE_MEMBERS(TestElementHandler)
HANDLER_DEFINE_MEMBERS(TestElement2dHandler)
DOMAIN_DEFINE_MEMBERS(DgnPlatformTestDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Majd.Uddin      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnPlatformTestDomain::DgnPlatformTestDomain() : DgnDomain(TMTEST_SCHEMA_NAME, "Test Schema", 1)
{
    RegisterHandler(TestElementHandler::GetHandler());
    RegisterHandler(TestElement2dHandler::GetHandler());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TestElementPtr TestElement::Create(DgnDbR db, DgnModelId mid, DgnCategoryId categoryId, Code elementCode)
{
    TestElementPtr testElement = new TestElement(CreateParams(db, mid, DgnClassId(GetTestElementECClass(db)->GetId()), categoryId, Placement3d(), elementCode));

    //  Add some hard-wired geometry
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateWorld(*testElement);
    EXPECT_TRUE(builder->Append(*GeomHelper::computeShape()));
    if (SUCCESS != builder->SetGeomStreamAndPlacement(*testElement))
        return nullptr;

    return testElement;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
TestElementPtr TestElement::CreateWithoutGeometry(DgnDbR db, DgnModelId mid, DgnCategoryId categoryId)
    {
    return new TestElement(CreateParams(db, mid, DgnClassId(GetTestElementECClass(db)->GetId()), categoryId, Placement3d(), DgnElement::Code()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Majd.Uddin    06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TestElementPtr TestElement::Create(DgnDbR db, ElemDisplayParamsCR ep, DgnModelId mid, DgnCategoryId categoryId, Code elementCode)
{
    TestElementPtr testElement = new TestElement(CreateParams(db, mid, DgnClassId(GetTestElementECClass(db)->GetId()), categoryId, Placement3d(), elementCode));

    //  Add some hard-wired geometry
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateWorld(*testElement);
    EXPECT_TRUE(builder->Append(ep));
    EXPECT_TRUE(builder->Append(*GeomHelper::computeShape()));
    if (SUCCESS != builder->SetGeomStreamAndPlacement(*testElement))
        return nullptr;

    return testElement;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestElement::_InsertInDb()
{
    DgnDbStatus stat = T_Super::_InsertInDb();
    if (DgnDbStatus::Success != stat)
        return stat;

    CachedECSqlStatementPtr insertStmt = GetDgnDb().GetPreparedECSqlStatement("INSERT INTO " TMTEST_SCHEMA_NAME "." TMTEST_TEST_ITEM_CLASS_NAME "(ECInstanceId," TMTEST_TEST_ITEM_TestItemProperty ") VALUES(?,?)");
    insertStmt->BindId(1, GetElementId());
    insertStmt->BindText(2, m_testItemProperty.c_str(), IECSqlBinder::MakeCopy::No);
    if (BE_SQLITE_DONE != insertStmt->Step())
        return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestElement::_ExtractSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ExtractSelectParams(stmt, params);
    if (DgnDbStatus::Success == status)
        m_testElemProperty = stmt.GetValueText(params.GetSelectIndex(TMTEST_TEST_ELEMENT_TestElementProperty));

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TestElementHandler::_GetClassParams(ECSqlClassParams& params)
    {
    T_Super::_GetClassParams(params);
    params.Add(TMTEST_TEST_ELEMENT_TestElementProperty);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestElement::_BindInsertParams(ECSqlStatement& stmt)
    {
    auto status = T_Super::_BindInsertParams(stmt);
    if (DgnDbStatus::Success == status)
        stmt.BindText(stmt.GetParameterIndex(TMTEST_TEST_ELEMENT_TestElementProperty), m_testElemProperty.c_str(), IECSqlBinder::MakeCopy::No);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestElement::_BindUpdateParams(ECSqlStatement& stmt)
    {
    auto status = T_Super::_BindUpdateParams(stmt);
    if (DgnDbStatus::Success == status)
        stmt.BindText(stmt.GetParameterIndex(TMTEST_TEST_ELEMENT_TestElementProperty), m_testElemProperty.c_str(), IECSqlBinder::MakeCopy::No);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Majd.Uddin      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestElement::_UpdateInDb()
{
    DgnDbStatus status = T_Super::_UpdateInDb();
    if (DgnDbStatus::Success != status)
        return status;

    Utf8String stmt("UPDATE " TMTEST_SCHEMA_NAME "." TMTEST_TEST_ITEM_CLASS_NAME);
    stmt.append(" SET " TMTEST_TEST_ITEM_TestItemProperty "=? WHERE ECInstanceId = ?;");

    CachedECSqlStatementPtr upStmt = GetDgnDb().GetPreparedECSqlStatement(stmt.c_str());
    if (upStmt.IsNull())
        return DgnDbStatus::SQLiteError;
    if (upStmt->BindId(2, GetElementId()) != ECSqlStatus::Success)
        return DgnDbStatus::SQLiteError;
    if (upStmt->BindText(1, ECN::ECValue(m_testItemProperty.c_str()).GetUtf8CP(), BeSQLite::EC::IECSqlBinder::MakeCopy::No) != ECSqlStatus::Success)
        return DgnDbStatus::SQLiteError;
    if (upStmt->Step() != BE_SQLITE_DONE)
        return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Majd.Uddin      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestElement::_DeleteInDb() const
{
    DgnDbStatus status = T_Super::_DeleteInDb();
    if (DgnDbStatus::Success != status)
        return status;

    Utf8String stmt("DELETE FROM " TMTEST_SCHEMA_NAME "." TMTEST_TEST_ITEM_CLASS_NAME);
    stmt.append(" WHERE ECInstanceId = ?;");

    CachedECSqlStatementPtr delStmt = GetDgnDb().GetPreparedECSqlStatement(stmt.c_str());
    if (delStmt.IsNull())
        return DgnDbStatus::SQLiteError;
    if (delStmt->BindId(1, GetElementId()) != ECSqlStatus::Success)
        return DgnDbStatus::SQLiteError;
    if (delStmt->Step() != BE_SQLITE_DONE)
        return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;

}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TestElement::_CopyFrom(DgnElementCR el)
    {
    T_Super::_CopyFrom(el);
    auto testEl = dynamic_cast<TestElement const*>(&el);
    if (nullptr != testEl)
        {
        m_testElemProperty = testEl->m_testElemProperty;
        m_testItemProperty = testEl->m_testItemProperty;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Inserts TestElement
* @bsimethod                                     Majd.Uddin                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnDbTestFixture::InsertElement(DgnElement::Code elementCode, DgnModelId mid, DgnCategoryId categoryId, DgnDbStatus* result)
{
    if (!mid.IsValid())
        mid = m_defaultModelId;

    if (!categoryId.IsValid())
        categoryId = m_defaultCategoryId;

    TestElementPtr el = TestElement::Create(*m_db, mid, categoryId, elementCode);
    return m_db->Elements().Insert(*el, result);
}

/*---------------------------------------------------------------------------------**//**
* Inserts TestElement with Display Properties
* @bsimethod                                     Majd.Uddin                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnDbTestFixture::InsertElement(DgnElement::Code elementCode, ElemDisplayParamsCR ep, DgnModelId mid, DgnCategoryId categoryId)
{
    if (!mid.IsValid())
        mid = m_defaultModelId;

    if (!categoryId.IsValid())
        categoryId = m_defaultCategoryId;

    TestElementPtr el = TestElement::Create(*m_db, ep, mid, categoryId, elementCode);
    return m_db->Elements().Insert(*el);
}

/*---------------------------------------------------------------------------------**//**
* Set up method that opens an existing .dgndb project file after copying it to out
* baseProjFile is the existing file and testProjFile is what we get
* @bsimethod                                     Majd.Uddin                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbTestFixture::SetupProject(WCharCP baseProjFile, WCharCP testProjFile, BeSQLite::Db::OpenMode mode)
{
    BeFileName outFileName;
    ASSERT_EQ(SUCCESS, DgnDbTestDgnManager::GetTestDataOut(outFileName, baseProjFile, testProjFile, __FILE__));
    DbResult result;
    m_db = DgnDb::OpenDgnDb(&result, outFileName, DgnDb::OpenParams(mode));
    ASSERT_TRUE(m_db.IsValid());
    ASSERT_TRUE(result == BE_SQLITE_OK);

    BeFileName schemaFile(T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    schemaFile.AppendToPath(WString("ECSchemas/" TMTEST_SCHEMA_NAME ".01.00.ecschema.xml", BentleyCharEncoding::Utf8).c_str());

    //BentleyStatus status = m_db->Domains().FindDomain("DgnPlatformTest")->ImportSchema(*m_db, schemaFile);
    auto status = DgnPlatformTestDomain::GetDomain().ImportSchema(*m_db, schemaFile);
    ASSERT_TRUE(DgnDbStatus::Success == status);

    m_defaultModelId = m_db->Models().QueryFirstModelId();
    m_defaultModelP = m_db->Models().GetModel(m_defaultModelId);
    ASSERT_TRUE(m_defaultModelP.IsValid());
    m_defaultModelP->FillModel();

    m_defaultCategoryId = m_db->Categories().MakeIterator().begin().GetCategoryId();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Majd.Uddin      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnDbTestFixture::SelectElementItem(DgnElementId id)
{
    Utf8String stmt("SELECT " TMTEST_TEST_ITEM_TestItemProperty " FROM " TMTEST_SCHEMA_NAME "." TMTEST_TEST_ITEM_CLASS_NAME);
    stmt.append(" WHERE ECInstanceId = ?;");

    CachedECSqlStatementPtr selStmt = m_db->GetPreparedECSqlStatement(stmt.c_str());
    if (selStmt.IsNull())
        return false;
    if (selStmt->BindId(1, id) != ECSqlStatus::Success)
        return false;
    if (selStmt->Step() != BE_SQLITE_ROW)
        return false;

    return true;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TestElement2dPtr TestElement2d::Create(DgnDbR db, DgnModelId mid, DgnCategoryId categoryId, Code elementCode)
{
    TestElement2dPtr testElement = new TestElement2d(TestElement2d::CreateParams(db, mid, db.Domains().GetClassId(TestElement2dHandler::GetHandler()), categoryId, Placement2d(), elementCode));
    if (!testElement.IsValid())
        return nullptr;

    //  Add some hard-wired geometry
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateWorld(*testElement);
    EXPECT_TRUE(builder->Append(*GeomHelper::computeShape2d()));
    return (SUCCESS != builder->SetGeomStreamAndPlacement(*testElement)) ? nullptr : testElement;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementKey DgnDbTestFixture::InsertElement2d(DgnModelId mid, DgnCategoryId categoryId, DgnElement::Code elementCode)
{
    if (!mid.IsValid())
        mid = m_defaultModelId;

    if (!categoryId.IsValid())
        categoryId = m_defaultCategoryId;

    DgnElementPtr el = TestElement2d::Create(*m_db, mid, categoryId, elementCode);

    return m_db->Elements().Insert(*el)->GetElementKey();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementKey DgnDbTestFixture::InsertElementUsingGeomPart2d(Utf8CP gpCode, DgnModelId mid, DgnCategoryId categoryId, DgnElement::Code elementCode)
{
    if (!mid.IsValid())
        mid = m_defaultModelId;

    if (!categoryId.IsValid())
        categoryId = m_defaultCategoryId;

    TestElement2dPtr el = TestElement2d::Create(*m_db, mid, categoryId, elementCode);

    DgnModelP model = m_db->Models().GetModel(mid).get();
    GeometricElementP geomElem = const_cast<GeometricElementP>(el->ToGeometricElement());

    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(*model, categoryId, DPoint2d::From(0.0, 0.0));

    DgnGeomPartId existingPartId = m_db->GeomParts().QueryGeomPartId(gpCode);
    EXPECT_TRUE(existingPartId.IsValid());

    if (!(builder->Append(existingPartId, Transform::From(0.0, 0.0, 0.0))))
        return DgnElementKey();

    if (SUCCESS != builder->SetGeomStreamAndPlacement(*geomElem))
        return DgnElementKey();

    return m_db->Elements().Insert(*el)->GetElementKey();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementKey DgnDbTestFixture::InsertElementUsingGeomPart(Utf8CP gpCode, DgnModelId mid, DgnCategoryId categoryId, DgnElement::Code elementCode)
    {
    if (!mid.IsValid())
        mid = m_defaultModelId;

    if (!categoryId.IsValid())
        categoryId = m_defaultCategoryId;

    DgnElementPtr el = TestElement::Create(*m_db, mid, categoryId, elementCode);

    DgnModelP model = m_db->Models().GetModel(mid).get();
    GeometricElementP geomElem = const_cast<GeometricElementP>(el->ToGeometricElement());

    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(*model, categoryId, DPoint3d::From(0.0, 0.0,0.0));

    DgnGeomPartId existingPartId = m_db->GeomParts().QueryGeomPartId(gpCode);
    EXPECT_TRUE(existingPartId.IsValid());

    if (!(builder->Append(existingPartId, Transform::From(0.0, 0.0, 0.0))))
        return DgnElementKey();

    if (SUCCESS != builder->SetGeomStreamAndPlacement(*geomElem))
        return DgnElementKey();

    return m_db->Elements().Insert(*el)->GetElementKey();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementKey DgnDbTestFixture::InsertElementUsingGeomPart(DgnGeomPartId gpId, DgnModelId mid, DgnCategoryId categoryId, DgnElement::Code elementCode)
    {
    if (!mid.IsValid())
        mid = m_defaultModelId;

    if (!categoryId.IsValid())
        categoryId = m_defaultCategoryId;

    DgnElementPtr el = TestElement::Create(*m_db, mid, categoryId, elementCode);

    DgnModelP model = m_db->Models().GetModel(mid).get();
    GeometricElementP geomElem = const_cast<GeometricElementP>(el->ToGeometricElement());

    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(*model, categoryId, DPoint3d::From(0.0, 0.0,0.0));

    if (!(builder->Append(gpId, Transform::From(0.0, 0.0, 0.0))))
        return DgnElementKey();

    if (SUCCESS != builder->SetGeomStreamAndPlacement(*geomElem))
        return DgnElementKey();

    return m_db->Elements().Insert(*el)->GetElementKey();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbTestFixture::setUpPhysicalView(DgnDbR dgnDb, DgnModelR model, ElementAlignedBox3d elementBox, DgnCategoryId categoryId)
    {
    DgnViews::View view;

    view.SetDgnViewType(DgnClassId(dgnDb.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalView)), DgnViewType::Physical);
    view.SetDgnViewSource(DgnViewSource::Generated);
    view.SetName("TestView");
    view.SetBaseModelId(model.GetModelId());

    EXPECT_TRUE(BE_SQLITE_OK == dgnDb.Views().Insert(view));
    EXPECT_TRUE(view.GetId().IsValid());

    ViewController::MarginPercent viewMargin(0.1, 0.1, 0.1, 0.1);

    PhysicalViewController viewController (dgnDb, view.GetId());
    viewController.SetStandardViewRotation(StandardView::Iso);
    viewController.LookAtVolume(elementBox, nullptr, &viewMargin);
    viewController.GetViewFlagsR().SetRenderMode(DgnRenderMode::SmoothShade);
    viewController.ChangeCategoryDisplay(categoryId, true);
    viewController.ChangeModelDisplay(model.GetModelId(), true);

    EXPECT_TRUE(BE_SQLITE_OK == viewController.Save());
    }


