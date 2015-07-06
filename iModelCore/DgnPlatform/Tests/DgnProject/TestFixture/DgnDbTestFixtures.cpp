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
static CurveVectorPtr computeShape()
{
    static const double PLANE_LEN = 100;

    DPoint3d pts[6];
    pts[0] = DPoint3d::From(-PLANE_LEN, -PLANE_LEN);
    pts[1] = DPoint3d::From(+PLANE_LEN, -PLANE_LEN);
    pts[2] = DPoint3d::From(+PLANE_LEN, +PLANE_LEN);
    pts[3] = DPoint3d::From(-PLANE_LEN, +PLANE_LEN);
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
* @bsimethod                                    Majd.Uddin      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECClassCP TestElementHandler::GetTestElementECClass(DgnDbR db)
{
    return db.Schemas().GetECClass(TMTEST_SCHEMA_NAME, TMTEST_TEST_ELEMENT_CLASS_NAME);
}

/*---------------------------------------------------------------------------------**//**
* Inserts TestElement
* @bsimethod                                     Majd.Uddin                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementKey TestElementHandler::InsertElement(DgnDbR db, DgnModelId mid, DgnCategoryId categoryId, Utf8CP elementCode)
{
    DgnElementPtr testElement = TestElementHandler::Create(TestElement::CreateParams(db, mid, DgnClassId(GetTestElementECClass(db)->GetId()), categoryId, Placement3d(), elementCode));
    GeometricElementP geomElem = const_cast<GeometricElementP>(testElement->ToGeometricElement());

    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateWorld(*geomElem);

    builder->Append(*computeShape());

    if (SUCCESS != builder->SetGeomStreamAndPlacement(*geomElem))
        return DgnElementKey();

    return db.Elements().Insert(*testElement)->GetElementKey();
}

/*---------------------------------------------------------------------------------**//**
* Inserts TestElement with Display Properties
* @bsimethod                                     Majd.Uddin                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementKey TestElementHandler::InsertElement(DgnDbR db, DgnModelId mid, DgnCategoryId categoryId, Utf8CP elementCode, ElemDisplayParamsCR ep)
{
    DgnElementPtr testElement = TestElementHandler::Create(TestElement::CreateParams(db, mid, DgnClassId(GetTestElementECClass(db)->GetId()), categoryId, Placement3d(), elementCode));
    GeometricElementP geomElem = const_cast<GeometricElementP>(testElement->ToGeometricElement());
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateWorld(*geomElem);
    
    EXPECT_TRUE (builder->Append(ep));
    EXPECT_TRUE (builder->Append(*computeShape()));
    
    if (SUCCESS != builder->SetGeomStreamAndPlacement(*geomElem))
        return DgnElementKey();
    
    return db.Elements().Insert(*testElement)->GetElementKey();
    
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Majd.Uddin                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestElementHandler::DeleteElement(DgnDbR db, DgnElementId eid)
{
    return db.Elements().Delete(eid);
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
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementKey DgnDbTestFixture::InsertElement(Utf8CP elementCode, DgnModelId mid, DgnCategoryId categoryId)
{
    if (!mid.IsValid())
        mid = m_defaultModelId;

    if (!categoryId.IsValid())
        categoryId = m_defaultCategoryId;

    return TestElementHandler::GetHandler().InsertElement(*m_db, mid, categoryId, elementCode);
}

/*---------------------------------------------------------------------------------**//**
* Insert Element with Display Properties
* @bsimethod                                    Majd.Uddin      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementKey DgnDbTestFixture::InsertElement(Utf8CP elementCode, ElemDisplayParamsCR ep, DgnModelId mid, DgnCategoryId categoryId)
{
    if (!mid.IsValid())
        mid = m_defaultModelId;
    
    if (!categoryId.IsValid())
        categoryId = m_defaultCategoryId;
    
    return TestElementHandler::GetHandler().InsertElement(*m_db, mid, categoryId, elementCode, ep);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Majd.Uddin      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnDbTestFixture::InsertElementItem(DgnElementId id, WCharCP propValue)
{
    Utf8String stmt("INSERT INTO ");
    stmt.append(TMTEST_SCHEMA_NAME);
    stmt.append(".");
    stmt.append(TMTEST_TEST_ITEM_CLASS_NAME);
    stmt.append("(ECInstanceId, ");
    stmt.append(TMTEST_TEST_ITEM_TestItemPropertyA);
    stmt.append(") Values (?, ?);");

    CachedECSqlStatementPtr insertStmt = m_db->GetPreparedECSqlStatement(stmt.c_str());
    if (insertStmt.IsNull())
        return false;
    if (insertStmt->BindId(1, id) != ECSqlStatus::Success)
        return false;
    if (insertStmt->BindText(2, ECN::ECValue(propValue).GetUtf8CP(), BeSQLite::EC::IECSqlBinder::MakeCopy::No) != ECSqlStatus::Success)
        return false;
    if (insertStmt->Step() != BeSQLite::EC::ECSqlStepStatus::Done)
        return false;

    return true;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Majd.Uddin      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnDbTestFixture::UpdateElementItem(DgnElementId id, WCharCP propValue)
{
    Utf8String stmt("UPDATE ");
    stmt.append(TMTEST_SCHEMA_NAME);
    stmt.append(".");
    stmt.append(TMTEST_TEST_ITEM_CLASS_NAME);
    stmt.append(" SET ");
    stmt.append(TMTEST_TEST_ITEM_TestItemPropertyA);
    stmt.append("=? WHERE ECInstanceId = ?;");

    CachedECSqlStatementPtr upStmt = m_db->GetPreparedECSqlStatement(stmt.c_str());
    if (upStmt.IsNull())
        return false;
    if (upStmt->BindId(2, id) != ECSqlStatus::Success)
        return false;
    if (upStmt->BindText(1, ECN::ECValue(propValue).GetUtf8CP(), BeSQLite::EC::IECSqlBinder::MakeCopy::No) != ECSqlStatus::Success)
        return false;
    if (upStmt->Step() != BeSQLite::EC::ECSqlStepStatus::Done)
        return false;

    return true;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Majd.Uddin      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnDbTestFixture::DeleteElementItem(DgnElementId id)
{
    Utf8String stmt("DELETE FROM ");
    stmt.append(TMTEST_SCHEMA_NAME);
    stmt.append(".");
    stmt.append(TMTEST_TEST_ITEM_CLASS_NAME);
    stmt.append(" WHERE ECInstanceId = ?;");

    CachedECSqlStatementPtr delStmt = m_db->GetPreparedECSqlStatement(stmt.c_str());
    if (delStmt.IsNull())
        return false;
    if (delStmt->BindId(1, id) != ECSqlStatus::Success)
        return false;
    if (delStmt->Step() != BeSQLite::EC::ECSqlStepStatus::Done)
        return false;

    return true;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Majd.Uddin      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnDbTestFixture::SelectElementItem(DgnElementId id)
{
    Utf8String stmt("SELECT ");
    stmt.append(TMTEST_TEST_ITEM_TestItemPropertyA);
    stmt.append(" FROM ");
    stmt.append(TMTEST_SCHEMA_NAME);
    stmt.append(".");
    stmt.append(TMTEST_TEST_ITEM_CLASS_NAME);
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


