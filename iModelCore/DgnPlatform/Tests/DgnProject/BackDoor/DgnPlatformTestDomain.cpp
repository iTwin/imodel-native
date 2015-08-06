/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/DgnProject/BackDoor/DgnPlatformTestDomain.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitTests/BackDoor/DgnProject/DgnPlatformTestDomain.h>
#include <DgnPlatform/DgnCore/GeomPart.h>
#include <DgnPlatform/DgnCore/ElementGeometry.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DPTEST
USING_NAMESPACE_BENTLEY_DGNPLATFORM

HANDLER_DEFINE_MEMBERS(TestElementHandler)
HANDLER_DEFINE_MEMBERS(TestItemHandler)
HANDLER_DEFINE_MEMBERS(TestUniqueAspectHandler)
HANDLER_DEFINE_MEMBERS(TestMultiAspectHandler)
DOMAIN_DEFINE_MEMBERS(DgnPlatformTestDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestItem::_GenerateElementGeometry(GeometricElementR el)
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TestElementPtr TestElement::Create(DgnDbR db, DgnModelId mid, DgnCategoryId categoryId, Utf8CP elementCode)
    {
    TestElementPtr testElement = new TestElement(CreateParams(db, mid, QueryClassId(db), categoryId));
    return testElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestItem::_LoadProperties(DgnElementCR el)
    {
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("SELECT " TMTEST_TEST_ITEM_TestItemProperty " FROM %s WHERE(ECInstanceId=?)", GetFullEcSqlClassName().c_str()));
    stmt->BindId(1, el.GetElementId());
    if (BeSQLite::EC::ECSqlStepStatus::HasRow != stmt->Step())
        return DgnDbStatus::ReadError;
    m_testItemProperty = stmt->GetValueText(0);
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestItem::_UpdateProperties(DgnElementCR el)
    {
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("UPDATE %s SET " TMTEST_TEST_ITEM_TestItemProperty "=? WHERE(ECInstanceId=?)", GetFullEcSqlClassName().c_str()));
    stmt->BindText(1, m_testItemProperty.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    stmt->BindId(2, el.GetElementId());
    return (BeSQLite::EC::ECSqlStepStatus::Done != stmt->Step())? DgnDbStatus::WriteError: DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestUniqueAspect::_LoadProperties(DgnElementCR el)
    {
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("SELECT " TMTEST_TEST_UNIQUE_ASPECT_TestUniqueAspectProperty " FROM %s WHERE(ECInstanceId=?)", GetFullEcSqlClassName().c_str()));
    stmt->BindId(1, GetAspectInstanceId(el));
    if (BeSQLite::EC::ECSqlStepStatus::HasRow != stmt->Step())
        return DgnDbStatus::ReadError;
    m_testUniqueAspectProperty = stmt->GetValueText(0);
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestUniqueAspect::_UpdateProperties(DgnElementCR el)
    {
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("UPDATE %s SET " TMTEST_TEST_UNIQUE_ASPECT_TestUniqueAspectProperty "=? WHERE(ECInstanceId=?)", GetFullEcSqlClassName().c_str()));
    stmt->BindText(1, m_testUniqueAspectProperty.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    stmt->BindId(2, GetAspectInstanceId(el));
    return (BeSQLite::EC::ECSqlStepStatus::Done != stmt->Step())? DgnDbStatus::WriteError: DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestMultiAspect::_LoadProperties(DgnElementCR el)
    {
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("SELECT " TMTEST_TEST_MULTI_ASPECT_TestMultiAspectProperty " FROM %s WHERE(ECInstanceId=?)", GetFullEcSqlClassName().c_str()));
    stmt->BindId(1, GetAspectInstanceId());
    if (BeSQLite::EC::ECSqlStepStatus::HasRow != stmt->Step())
        return DgnDbStatus::ReadError;
    m_testMultiAspectProperty = stmt->GetValueText(0);
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestMultiAspect::_UpdateProperties(DgnElementCR el)
    {
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("UPDATE %s SET " TMTEST_TEST_MULTI_ASPECT_TestMultiAspectProperty "=? WHERE(ECInstanceId=?)", GetFullEcSqlClassName().c_str()));
    stmt->BindText(1, m_testMultiAspectProperty.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    stmt->BindId(2, GetAspectInstanceId());
    return (BeSQLite::EC::ECSqlStepStatus::Done != stmt->Step())? DgnDbStatus::WriteError: DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnPlatformTestDomain::DgnPlatformTestDomain() : DgnDomain(TMTEST_SCHEMA_NAME, "DgnProject Test Schema", 1)
    {
    RegisterHandler(TestElementHandler::GetHandler());
    RegisterHandler(TestItemHandler::GetHandler());
    RegisterHandler(TestUniqueAspectHandler::GetHandler());
    RegisterHandler(TestMultiAspectHandler::GetHandler());
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
    schemaFile.AppendToPath(L"ECSchemas/" TMTEST_SCHEMA_NAMEW L".01.00.ecschema.xml");

    auto status = GetDomain().DgnDomain::ImportSchema(db, schemaFile);
    if (DgnDbStatus::Success != status)
        return status;

    auto schema = db.Schemas().GetECSchema(TMTEST_SCHEMA_NAME, true);
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
