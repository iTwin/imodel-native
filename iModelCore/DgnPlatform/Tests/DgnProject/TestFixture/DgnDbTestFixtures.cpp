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
    TestElementPtr testElement = new TestElement(CreateParams(db, mid, DgnClassId(GetTestElementECClass(db)->GetId()), categoryId, Placement3d(), nullptr, elementCode));

    //  Add some hard-wired geometry
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateWorld(*testElement);
    EXPECT_TRUE(builder->Append(*GeomHelper::computeShape()));
    if (SUCCESS != builder->SetGeomStreamAndPlacement(*testElement))
        return nullptr;

    return testElement;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Majd.Uddin    06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TestElementPtr TestElement::Create(DgnDbR db, ElemDisplayParamsCR ep, DgnModelId mid, DgnCategoryId categoryId, Code elementCode)
{
    TestElementPtr testElement = new TestElement(CreateParams(db, mid, DgnClassId(GetTestElementECClass(db)->GetId()), categoryId, Placement3d(), nullptr, elementCode));

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
DgnDbStatus TestElement::_InsertSecondary()
{
    DgnDbStatus stat = T_Super::_InsertSecondary();
    if (DgnDbStatus::Success != stat)
        return stat;

    CachedECSqlStatementPtr insertStmt = GetDgnDb().GetPreparedECSqlStatement("INSERT INTO " TMTEST_SCHEMA_NAME "." TMTEST_TEST_ITEM_CLASS_NAME "(ECInstanceId," TMTEST_TEST_ITEM_TestItemProperty ") VALUES(?,?)");
    insertStmt->BindId(1, GetElementId());
    insertStmt->BindText(2, m_testItemProperty.c_str(), IECSqlBinder::MakeCopy::No);
    if (ECSqlStepStatus::Done != insertStmt->Step())
        return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
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
    if (upStmt->Step() != BeSQLite::EC::ECSqlStepStatus::Done)
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
    if (delStmt->Step() != BeSQLite::EC::ECSqlStepStatus::Done)
        return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;

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
    if (selStmt->Step() != BeSQLite::EC::ECSqlStepStatus::HasRow)
        return false;

    return true;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TestElement2dPtr TestElement2d::Create(DgnDbR db, DgnModelId mid, DgnCategoryId categoryId, Code elementCode)
{
    DgnElementPtr testElement = TestElement2dHandler::GetHandler().Create(TestElement2d::CreateParams(db, mid, db.Domains().GetClassId(TestElement2dHandler::GetHandler()), categoryId, Placement2d(), nullptr, elementCode));
    if (!testElement.IsValid())
        return nullptr;

    TestElement2d* geom = (TestElement2d*) testElement.get();

    //  Add some hard-wired geometry
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateWorld(*geom);
    EXPECT_TRUE(builder->Append(*GeomHelper::computeShape2d()));
    return (SUCCESS != builder->SetGeomStreamAndPlacement(*geom)) ? nullptr : geom;
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


