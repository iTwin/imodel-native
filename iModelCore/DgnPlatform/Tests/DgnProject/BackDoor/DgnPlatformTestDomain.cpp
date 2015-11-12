    /*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/DgnProject/BackDoor/DgnPlatformTestDomain.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include "PublicAPI/BackDoor/DgnProject/DgnPlatformTestDomain.h"
#include <DgnPlatform/GeomPart.h>
#include <DgnPlatform/ElementGeometry.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DPTEST
USING_NAMESPACE_BENTLEY_DGNPLATFORM

HANDLER_DEFINE_MEMBERS(TestElementHandler)
HANDLER_DEFINE_MEMBERS(TestElement2dHandler)
#ifdef WIP_ELEMENT_ITEM // *** pending redesign
HANDLER_DEFINE_MEMBERS(TestItemHandler)
#endif
HANDLER_DEFINE_MEMBERS(TestUniqueAspectHandler)
HANDLER_DEFINE_MEMBERS(TestMultiAspectHandler)
HANDLER_DEFINE_MEMBERS(TestGroupHandler)
DOMAIN_DEFINE_MEMBERS(DgnPlatformTestDomain)
HANDLER_DEFINE_MEMBERS(TestElementDrivesElementHandler)

bool TestElementDrivesElementHandler::s_shouldFail;

#ifdef WIP_ELEMENT_ITEM // *** pending redesign
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestItem::_GenerateElementGeometry(GeometricElementR el, GenerateReason)
    {
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateWorld(el);
    
    // We make the element geometry depend on the item's property. 
    //  In a real app, of course, this property would be something realistic, not a string.
    if (m_testItemProperty.EqualsI("Line"))
        builder->Append(*ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::FromZero(), DPoint3d::From(1,1,1))));
    else if (m_testItemProperty.EqualsI("Circle"))
        builder->Append(*ICurvePrimitive::CreateArc(DEllipse3d::FromXYMajorMinor(0,0,0, 10,10, 0,0, Angle::PiOver2())));
    else
        return DgnDbStatus::WriteError;
    
    if (BSISUCCESS != builder->SetGeomStreamAndPlacement(el))
        return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TestElementPtr TestElement::Create(DgnDbR db, DgnModelId mid, DgnCategoryId categoryId, Utf8CP elementCode)
    {
    TestElementPtr testElement = new TestElement(CreateParams(db, mid, QueryClassId(db), categoryId, Placement3d()));
    return testElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TestElementPtr TestElement::Create(DgnDbR db, DgnModelId mid, DgnCategoryId categoryId, DgnElement::Code const& elementCode)
    {
    TestElementPtr testElement = new TestElement(CreateParams(db, mid, QueryClassId(db), categoryId, Placement3d(), elementCode));
    return testElement;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
static CurveVectorPtr computeShape(double len)
    {
    DPoint3d pts[6];
    pts[0] = DPoint3d::From(-len,-len);
    pts[1] = DPoint3d::From(+len,-len);
    pts[2] = DPoint3d::From(+len,+len);
    pts[3] = DPoint3d::From(-len,+len);
    pts[4] = pts[0];
    pts[5] = pts[0];
    pts[5].z = 1;

    return CurveVector::CreateLinear(pts, _countof(pts), CurveVector::BOUNDARY_TYPE_Open);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TestElementPtr TestElement::Create(DgnDbR db, DgnModelId mid, DgnCategoryId categoryId, Utf8CP elementCode, double shapeSize)
    {
    TestElementPtr testElement = new TestElement(CreateParams(db, mid, QueryClassId(db), categoryId));

    //  Add some hard-wired geometry
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateWorld(*testElement);
    builder->Append(*computeShape(shapeSize));
    if (SUCCESS != builder->SetGeomStreamAndPlacement(*testElement))
        return nullptr;

    return testElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TestElement::ChangeElement(double len)
    {
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateWorld(*this);
    builder->Append(*computeShape(len));
    builder->SetGeomStreamAndPlacement(*this);
    }

//---------------------------------------------------------------------------------------
//*@bsimethod                                    Umar.Hayat      07 / 15
//---------------------------------------------------------------------------------------
static CurveVectorPtr computeShape2d(double len)
{
    DPoint2d GTs[6];
    GTs[0] = DPoint2d::From(-len, -len);
    GTs[1] = DPoint2d::From(+len, -len);
    GTs[2] = DPoint2d::From(+len, +len);
    GTs[3] = DPoint2d::From(-len, +len);
    GTs[4] = GTs[0];
    GTs[5] = GTs[0];

    return CurveVector::CreateLinear(GTs, _countof(GTs), CurveVector::BOUNDARY_TYPE_Open);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TestElement2dPtr TestElement2d::Create(DgnDbR db, DgnModelId mid, DgnCategoryId categoryId, Code elementCode, double length)
{
    DgnElementPtr testElement = TestElement2dHandler::GetHandler().Create(TestElement2d::CreateParams(db, mid, db.Domains().GetClassId(TestElement2dHandler::GetHandler()), categoryId, Placement2d(), elementCode));
    if (!testElement.IsValid())
        return nullptr;

    TestElement2d* geom = (TestElement2d*) testElement.get();

    //  Add some hard-wired geometry
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateWorld(*geom);
    builder->Append(*computeShape2d(length));
    return (SUCCESS != builder->SetGeomStreamAndPlacement(*geom)) ? nullptr : geom;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TestGroupPtr TestGroup::Create(DgnDbR db, DgnModelId modelId, DgnCategoryId categoryId)
    {
    DgnClassId classId = db.Domains().GetClassId(TestGroupHandler::GetHandler());
    TestGroupPtr group = new TestGroup(CreateParams(db, modelId, classId, categoryId, Placement3d()));
    BeAssert(group.IsValid());
    return group;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef WIP_ELEMENT_ITEM // *** pending redesign
DgnDbStatus TestItem::_LoadProperties(DgnElementCR el)
    {
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("SELECT " DPTEST_TEST_ITEM_TestItemProperty "," DPTEST_TEST_ITEM_TestItemLength " FROM %s WHERE(ECInstanceId=?)", GetFullEcSqlClassName().c_str()));
    stmt->BindId(1, el.GetElementId());
    if (BE_SQLITE_ROW != stmt->Step())
        return DgnDbStatus::ReadError;
    m_testItemProperty = stmt->GetValueText(0);
    m_length = stmt->GetValueDouble(1);
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestItem::_UpdateProperties(DgnElementCR el)
    {
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("UPDATE %s SET " DPTEST_TEST_ITEM_TestItemProperty "=?," DPTEST_TEST_ITEM_TestItemLength "=? WHERE(ECInstanceId=?)", GetFullEcSqlClassName().c_str()));
    stmt->BindText(1, m_testItemProperty.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    stmt->BindDouble(2, m_length);
    stmt->BindId(3, el.GetElementId());
    return (BE_SQLITE_DONE != stmt->Step())? DgnDbStatus::WriteError: DgnDbStatus::Success;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestUniqueAspect::_LoadProperties(DgnElementCR el)
    {
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("SELECT " DPTEST_TEST_UNIQUE_ASPECT_TestUniqueAspectProperty " FROM %s WHERE(ECInstanceId=?)", GetFullEcSqlClassName().c_str()));
    stmt->BindId(1, GetAspectInstanceId(el));
    if (BE_SQLITE_ROW != stmt->Step())
        return DgnDbStatus::ReadError;
    m_testUniqueAspectProperty = stmt->GetValueText(0);
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestUniqueAspect::_UpdateProperties(DgnElementCR el)
    {
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("UPDATE %s SET " DPTEST_TEST_UNIQUE_ASPECT_TestUniqueAspectProperty "=? WHERE(ECInstanceId=?)", GetFullEcSqlClassName().c_str()));
    stmt->BindText(1, m_testUniqueAspectProperty.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    stmt->BindId(2, GetAspectInstanceId(el));
    return (BE_SQLITE_DONE != stmt->Step())? DgnDbStatus::WriteError: DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestMultiAspect::_LoadProperties(DgnElementCR el)
    {
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("SELECT " DPTEST_TEST_MULTI_ASPECT_TestMultiAspectProperty " FROM %s WHERE(ECInstanceId=?)", GetFullEcSqlClassName().c_str()));
    stmt->BindId(1, GetAspectInstanceId());
    if (BE_SQLITE_ROW != stmt->Step())
        return DgnDbStatus::ReadError;
    m_testMultiAspectProperty = stmt->GetValueText(0);
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestMultiAspect::_UpdateProperties(DgnElementCR el)
    {
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("UPDATE %s SET " DPTEST_TEST_MULTI_ASPECT_TestMultiAspectProperty "=? WHERE(ECInstanceId=?)", GetFullEcSqlClassName().c_str()));
    stmt->BindText(1, m_testMultiAspectProperty.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    stmt->BindId(2, GetAspectInstanceId());
    return (BE_SQLITE_DONE != stmt->Step())? DgnDbStatus::WriteError: DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TestElementDrivesElementHandler::_OnRootChanged(DgnDbR db, ECInstanceId relationshipId, DgnElementId source, DgnElementId target)
    {
    if (s_shouldFail)
        db.Txns().ReportError(*new TxnManager::ValidationError(TxnManager::ValidationError::Severity::Warning, "ABC failed"));

    m_relIds.push_back(relationshipId);

    if (nullptr != s_callback)
        s_callback->_OnRootChanged(db, relationshipId, source, target);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TestElementDrivesElementHandler::UpdateProperty1(DgnDbR db, EC::ECInstanceKeyCR key)
    {
    //  Modify a property of the dependency relationship itself
    ECSqlStatement stmt;
    stmt.Prepare(db, "UPDATE ONLY " DPTEST_SCHEMA_NAME "." DPTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME " SET Property1='changed'  WHERE(ECInstanceId=?)");
    stmt.BindId(1, key.GetECInstanceId());
    ASSERT_EQ( stmt.Step(), BE_SQLITE_DONE );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey TestElementDrivesElementHandler::Insert(DgnDbR db, DgnElementId root, DgnElementId dependent)
    {
    ECSqlInsertBuilder b;
    b.InsertInto(GetECClass(db));
    b.AddValue("SourceECInstanceId", "?");
    b.AddValue("TargetECInstanceId", "?");

    CachedECSqlStatementPtr stmt = db.GetPreparedECSqlStatement(b.ToString().c_str());

    stmt->BindId(1, root);
    stmt->BindId(2, dependent);

    ECInstanceKey rkey;
    if (BE_SQLITE_DONE != stmt->Step(rkey))
        return ECInstanceKey();

    return rkey;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnPlatformTestDomain::DgnPlatformTestDomain() : DgnDomain(DPTEST_SCHEMA_NAME, "DgnProject Test Schema", 1)
    {
    RegisterHandler(TestElementHandler::GetHandler());
    RegisterHandler(TestElement2dHandler::GetHandler());
    RegisterHandler(TestGroupHandler::GetHandler());
#ifdef WIP_ELEMENT_ITEM // *** pending redesign
    RegisterHandler(TestItemHandler::GetHandler());
#endif
    RegisterHandler(TestUniqueAspectHandler::GetHandler());
    RegisterHandler(TestMultiAspectHandler::GetHandler());
    RegisterHandler(TestElementDrivesElementHandler::GetHandler());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnPlatformTestDomain::Register()
    {
    DgnDomains::RegisterDomain(GetDomain()); 
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnPlatformTestDomain::ImportSchema(DgnDbR db)
    {
    BeFileName schemaFile(T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    schemaFile.AppendToPath(L"ECSchemas/" DPTEST_SCHEMA_NAMEW L".01.00.ecschema.xml");

    auto status = GetDomain().DgnDomain::ImportSchema(db, schemaFile);
    if (DgnDbStatus::Success != status)
        return status;

    auto schema = db.Schemas().GetECSchema(DPTEST_SCHEMA_NAME, true);
    if (nullptr == schema)
        return DgnDbStatus::BadSchema;

    if (!TestElement::QueryClassId(db).IsValid())
        return DgnDbStatus::BadSchema;

    if (nullptr == TestUniqueAspect::GetECClass(db))
        return DgnDbStatus::BadSchema;
    
    if (nullptr == TestMultiAspect::GetECClass(db))
        return DgnDbStatus::BadSchema;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnPlatformTestDomain::ImportDummySchema(DgnDbR db)
    {
    BeFileName schemaFile(T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    schemaFile.AppendToPath(L"ECSchemas/" DPTEST_DUMMY_SCHEMA_NAMEW L".01.00.ecschema.xml");

    WString schemaBaseNameW;
    schemaFile.ParseName(NULL, NULL, &schemaBaseNameW, NULL);
    Utf8String schemaBaseName(schemaBaseNameW);

    BeFileName schemaDir = schemaFile.GetDirectoryName();

    ECN::ECSchemaReadContextPtr contextPtr = ECN::ECSchemaReadContext::CreateContext();
    contextPtr->AddSchemaLocater(db.GetSchemaLocater());
    contextPtr->AddSchemaPath(schemaDir.GetName());

    ECN::ECSchemaPtr schemaPtr;
    ECN::SchemaReadStatus readSchemaStatus = ECN::ECSchema::ReadFromXmlFile(schemaPtr, schemaFile.GetName(), *contextPtr);
    if (ECN::SCHEMA_READ_STATUS_Success != readSchemaStatus)
        return DgnDbStatus::ReadError;

    if (BentleyStatus::SUCCESS != db.Schemas().ImportECSchemas(contextPtr->GetCache()))
        return DgnDbStatus::BadSchema;

    return DgnDbStatus::Success;
    }

TestElementDrivesElementHandler::Callback* TestElementDrivesElementHandler::s_callback = nullptr;

