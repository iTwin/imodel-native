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
DOMAIN_DEFINE_MEMBERS(DgnPlatformTestDomain)

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
static CurveVectorPtr computeShape(double len)
{

    DPoint3d pts[6];
    pts[0] = DPoint3d::From(-len, -len);
    pts[1] = DPoint3d::From(+len, -len);
    pts[2] = DPoint3d::From(+len, +len);
    pts[3] = DPoint3d::From(-len, +len);
    pts[4] = pts[0];
    pts[5] = pts[0];
    pts[5].z = 1;

    return CurveVector::CreateLinear(pts, _countof(pts), CurveVector::BOUNDARY_TYPE_Open);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Majd.Uddin      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnPlatformTestDomain::DgnPlatformTestDomain() : DgnDomain(TMTEST_SCHEMA_NAME, "Test Schema", 1)
{
    RegisterHandler(TestElementHandler::GetHandler());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TestElementPtr TestElement::Create(DgnDbR db, DgnModelId mid, DgnCategoryId categoryId, Utf8CP elementCode)
{
    TestElementPtr testElement = new TestElement(CreateParams(db, mid, DgnClassId(GetTestElementECClass(db)->GetId()), categoryId));

    static const double PLANE_LEN = 100;
    //  Add some hard-wired geometry
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateWorld(*testElement);
    EXPECT_TRUE(builder->Append(*computeShape(PLANE_LEN)));
    if (SUCCESS != builder->SetGeomStreamAndPlacement(*testElement))
        return nullptr;

    return testElement;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Majd.Uddin    06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TestElementPtr TestElement::Create(DgnDbR db, ElemDisplayParamsCR ep, DgnModelId mid, DgnCategoryId categoryId, Utf8CP elementCode)
{
    TestElementPtr testElement = new TestElement(CreateParams(db, mid, DgnClassId(GetTestElementECClass(db)->GetId()), categoryId));

    static const double PLANE_LEN = 100;
    //  Add some hard-wired geometry
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateWorld(*testElement);
    EXPECT_TRUE(builder->Append(ep));
    EXPECT_TRUE(builder->Append(*computeShape(PLANE_LEN)));
    if (SUCCESS != builder->SetGeomStreamAndPlacement(*testElement))
        return nullptr;

    return testElement;
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestElement::_InsertInDb()
{
    DgnDbStatus status = T_Super::_InsertInDb();
    if (DgnDbStatus::Success != status)
        return status;

    CachedECSqlStatementPtr insertStmt = GetDgnDb().GetPreparedECSqlStatement("INSERT INTO " TMTEST_SCHEMA_NAME "." TMTEST_TEST_ITEM_CLASS_NAME "(ECInstanceId," TMTEST_TEST_ITEM_TestItemProperty ") VALUES(?,?)");
    insertStmt->BindId(1, GetElementId());
    insertStmt->BindText(2, m_testItemProperty.c_str(), IECSqlBinder::MakeCopy::No);
    if (ECSqlStepStatus::Done != insertStmt->Step())
        return DgnDbStatus::ElementWriteError;

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
        return DgnDbStatus::ElementWriteError;

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
        return DgnDbStatus::ElementWriteError;

    return DgnDbStatus::Success;

}

/*---------------------------------------------------------------------------------**//**
* Inserts TestElement
* @bsimethod                                     Majd.Uddin                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnDbTestFixture::InsertElement(Utf8CP elementCode, DgnModelId mid, DgnCategoryId categoryId)
{
    if (!mid.IsValid())
        mid = m_defaultModelId;

    if (!categoryId.IsValid())
        categoryId = m_defaultCategoryId;

    TestElementPtr el = TestElement::Create(*m_db, mid, categoryId, elementCode);
    return m_db->Elements().Insert(*el);
}

/*---------------------------------------------------------------------------------**//**
* Inserts TestElement with Display Properties
* @bsimethod                                     Majd.Uddin                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnDbTestFixture::InsertElement(Utf8CP elementCode, ElemDisplayParamsCR ep, DgnModelId mid, DgnCategoryId categoryId)
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
    schemaFile.AppendToPath(L"ECSchemas/" TMTEST_SCHEMA_NAMEW L".01.00.ecschema.xml");

    //BentleyStatus status = m_db->Domains().FindDomain("DgnPlatformTest")->ImportSchema(*m_db, schemaFile);
    BentleyStatus status = DgnPlatformTestDomain::GetDomain().ImportSchema(*m_db, schemaFile);
    ASSERT_TRUE(BentleyStatus::SUCCESS == status);

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


