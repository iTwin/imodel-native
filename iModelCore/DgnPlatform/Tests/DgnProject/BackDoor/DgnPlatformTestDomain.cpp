    /*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/DgnProject/BackDoor/DgnPlatformTestDomain.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include "PublicAPI/BackDoor/DgnProject/DgnPlatformTestDomain.h"
#include "TestDomainElements.h"
#include <DgnPlatform/GeomPart.h>
#include <DgnPlatform/ElementGeometry.h>
#include <ECDb/ECDbApi.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DPTEST
USING_NAMESPACE_BENTLEY_DGNPLATFORM

HANDLER_DEFINE_MEMBERS(TestElementHandler)
HANDLER_DEFINE_MEMBERS(TestElement2dHandler)
HANDLER_DEFINE_MEMBERS(TestUniqueAspectHandler)
HANDLER_DEFINE_MEMBERS(TestMultiAspectHandler)
HANDLER_DEFINE_MEMBERS(TestGroupHandler)
DOMAIN_DEFINE_MEMBERS(DgnPlatformTestDomain)
HANDLER_DEFINE_MEMBERS(TestElementDrivesElementHandler)

bool TestElementDrivesElementHandler::s_shouldFail;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TestElement::TestElement(CreateParams const& params) : T_Super(params)
    {
    for (auto& i : m_intProps)
        i = 0;

    for (auto& d : m_doubleProps)
        d = 0.0;

    for (auto& p : m_pointProps)
        p = DPoint3d::FromXYZ(0.0, 0.0, 0.0);
    }

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
TestElementPtr TestElement::Create(DgnDbR db, DgnModelId mid, DgnCategoryId categoryId, DgnCode const& elementCode)
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
    GeometryBuilderPtr builder = GeometryBuilder::CreateWorld(*testElement);
    builder->Append(*computeShape(shapeSize));
    if (SUCCESS != builder->SetGeometryStreamAndPlacement(*testElement))
        return nullptr;

    return testElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Majd.Uddin    06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TestElementPtr TestElement::Create(Dgn::DgnDbR db, Render::GeometryParamsCR ep, Dgn::DgnModelId mid, Dgn::DgnCategoryId categoryId, DgnCode elementCode, double shapeSize)
    {
    TestElementPtr testElement = new TestElement(CreateParams(db, mid, DgnClassId(GetTestElementECClass(db)->GetId()), categoryId, Placement3d(), elementCode));

    //  Add some hard-wired geometry
    GeometryBuilderPtr builder = GeometryBuilder::CreateWorld(*testElement);
    EXPECT_TRUE(builder->Append(ep));
    EXPECT_TRUE(builder->Append(*computeShape(shapeSize)));
    if (SUCCESS != builder->SetGeometryStreamAndPlacement(*testElement))
        return nullptr;

    return testElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
TestElementPtr TestElement::CreateWithoutGeometry(DgnDbR db, DgnModelId mid, DgnCategoryId categoryId)
    {
    return new TestElement(CreateParams(db, mid, DgnClassId(GetTestElementECClass(db)->GetId()), categoryId, Placement3d(), DgnCode()));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestElement::_InsertInDb()
{
    DgnDbStatus stat = T_Super::_InsertInDb();
    if (DgnDbStatus::Success != stat)
        return stat;
    return DgnDbStatus::Success;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestElement::_SetProperty(Utf8CP propName, ECN::ECValueCR value)
    {
#define SETSTRPROP(CASEN,PVAL) if (0 == strcmp(propName, CASEN)) {PVAL = value.ToString(); return DgnDbStatus::Success;}
#define SETINTPROP(CASEN,PVAL) if (0 == strcmp(propName, CASEN)) {PVAL = value.GetInteger(); return DgnDbStatus::Success;}
#define SETDBLPROP(CASEN,PVAL) if (0 == strcmp(propName, CASEN)) {PVAL = value.GetDouble(); return DgnDbStatus::Success;}
#define SETPNTPROP(CASEN,PVAL) if (0 == strcmp(propName, CASEN)) {PVAL = value.GetPoint3D(); return DgnDbStatus::Success;}

    SETSTRPROP(DPTEST_TEST_ELEMENT_TestElementProperty, m_testElemProperty)
    SETINTPROP(DPTEST_TEST_ELEMENT_IntegerProperty1, m_intProps[0])
    SETINTPROP(DPTEST_TEST_ELEMENT_IntegerProperty2, m_intProps[1])
    SETINTPROP(DPTEST_TEST_ELEMENT_IntegerProperty3, m_intProps[2])
    SETINTPROP(DPTEST_TEST_ELEMENT_IntegerProperty4, m_intProps[3])
    SETDBLPROP(DPTEST_TEST_ELEMENT_DoubleProperty1, m_doubleProps[0])
    SETDBLPROP(DPTEST_TEST_ELEMENT_DoubleProperty2, m_doubleProps[1])
    SETDBLPROP(DPTEST_TEST_ELEMENT_DoubleProperty3, m_doubleProps[2])
    SETDBLPROP(DPTEST_TEST_ELEMENT_DoubleProperty4, m_doubleProps[3])
    SETPNTPROP(DPTEST_TEST_ELEMENT_PointProperty1, m_pointProps[0])
    SETPNTPROP(DPTEST_TEST_ELEMENT_PointProperty2, m_pointProps[1])
    SETPNTPROP(DPTEST_TEST_ELEMENT_PointProperty3, m_pointProps[2])
    SETPNTPROP(DPTEST_TEST_ELEMENT_PointProperty4, m_pointProps[3])

    return T_Super::_SetProperty(propName, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestElement::_GetProperty(ECN::ECValueR value, Utf8CP propName) const
    {
#define GETSTRPROP(CASEN,PVAL) if (0 == strcmp(propName, CASEN)) {value = ECN::ECValue(PVAL.c_str()); return DgnDbStatus::Success;}
#define GETINTPROP(CASEN,PVAL) if (0 == strcmp(propName, CASEN)) {value = ECN::ECValue(PVAL); return DgnDbStatus::Success;}
#define GETDBLPROP(CASEN,PVAL) if (0 == strcmp(propName, CASEN)) {value = ECN::ECValue(PVAL); return DgnDbStatus::Success;}
#define GETPNTPROP(CASEN,PVAL) if (0 == strcmp(propName, CASEN)) {value = ECN::ECValue(PVAL); return DgnDbStatus::Success;}

    GETSTRPROP(DPTEST_TEST_ELEMENT_TestElementProperty, m_testElemProperty)
    GETINTPROP(DPTEST_TEST_ELEMENT_IntegerProperty1, m_intProps[0])
    GETINTPROP(DPTEST_TEST_ELEMENT_IntegerProperty2, m_intProps[1])
    GETINTPROP(DPTEST_TEST_ELEMENT_IntegerProperty3, m_intProps[2])
    GETINTPROP(DPTEST_TEST_ELEMENT_IntegerProperty4, m_intProps[3])
    GETDBLPROP(DPTEST_TEST_ELEMENT_DoubleProperty1, m_doubleProps[0])
    GETDBLPROP(DPTEST_TEST_ELEMENT_DoubleProperty2, m_doubleProps[1])
    GETDBLPROP(DPTEST_TEST_ELEMENT_DoubleProperty3, m_doubleProps[2])
    GETDBLPROP(DPTEST_TEST_ELEMENT_DoubleProperty4, m_doubleProps[3])
    GETPNTPROP(DPTEST_TEST_ELEMENT_PointProperty1, m_pointProps[0])
    GETPNTPROP(DPTEST_TEST_ELEMENT_PointProperty2, m_pointProps[1])
    GETPNTPROP(DPTEST_TEST_ELEMENT_PointProperty3, m_pointProps[2])
    GETPNTPROP(DPTEST_TEST_ELEMENT_PointProperty4, m_pointProps[3])

    return T_Super::_GetProperty(value, propName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestElement::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ReadSelectParams(stmt, params);
    if (DgnDbStatus::Success != status)
        return status;

    m_testElemProperty = stmt.GetValueText(params.GetSelectIndex(DPTEST_TEST_ELEMENT_TestElementProperty));
    m_intProps[0] = stmt.GetValueInt(params.GetSelectIndex(DPTEST_TEST_ELEMENT_IntegerProperty1));
    m_intProps[1] = stmt.GetValueInt(params.GetSelectIndex(DPTEST_TEST_ELEMENT_IntegerProperty2));
    m_intProps[2] = stmt.GetValueInt(params.GetSelectIndex(DPTEST_TEST_ELEMENT_IntegerProperty3));
    m_intProps[3] = stmt.GetValueInt(params.GetSelectIndex(DPTEST_TEST_ELEMENT_IntegerProperty4));
    m_doubleProps[0] = stmt.GetValueDouble(params.GetSelectIndex(DPTEST_TEST_ELEMENT_DoubleProperty1));
    m_doubleProps[1] = stmt.GetValueDouble(params.GetSelectIndex(DPTEST_TEST_ELEMENT_DoubleProperty2));
    m_doubleProps[2] = stmt.GetValueDouble(params.GetSelectIndex(DPTEST_TEST_ELEMENT_DoubleProperty3));
    m_doubleProps[3] = stmt.GetValueDouble(params.GetSelectIndex(DPTEST_TEST_ELEMENT_DoubleProperty4));
    m_pointProps[0] = stmt.GetValuePoint3D(params.GetSelectIndex(DPTEST_TEST_ELEMENT_PointProperty1));
    m_pointProps[1] = stmt.GetValuePoint3D(params.GetSelectIndex(DPTEST_TEST_ELEMENT_PointProperty2));
    m_pointProps[2] = stmt.GetValuePoint3D(params.GetSelectIndex(DPTEST_TEST_ELEMENT_PointProperty3));
    m_pointProps[3] = stmt.GetValuePoint3D(params.GetSelectIndex(DPTEST_TEST_ELEMENT_PointProperty4));

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestElement::_BindInsertParams(ECSqlStatement& stmt)
    {
    auto status = T_Super::_BindInsertParams(stmt);
    if (DgnDbStatus::Success == status)
        BindParams(stmt);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TestElement::BindParams(ECSqlStatement& stmt) const
    {
    stmt.BindText(stmt.GetParameterIndex(DPTEST_TEST_ELEMENT_TestElementProperty), m_testElemProperty.c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindInt(stmt.GetParameterIndex(DPTEST_TEST_ELEMENT_IntegerProperty1), m_intProps[0]);
    stmt.BindInt(stmt.GetParameterIndex(DPTEST_TEST_ELEMENT_IntegerProperty2), m_intProps[1]);
    stmt.BindInt(stmt.GetParameterIndex(DPTEST_TEST_ELEMENT_IntegerProperty3), m_intProps[2]);
    stmt.BindInt(stmt.GetParameterIndex(DPTEST_TEST_ELEMENT_IntegerProperty4), m_intProps[3]);
    stmt.BindDouble(stmt.GetParameterIndex(DPTEST_TEST_ELEMENT_DoubleProperty1), m_doubleProps[0]);
    stmt.BindDouble(stmt.GetParameterIndex(DPTEST_TEST_ELEMENT_DoubleProperty2), m_doubleProps[1]);
    stmt.BindDouble(stmt.GetParameterIndex(DPTEST_TEST_ELEMENT_DoubleProperty3), m_doubleProps[2]);
    stmt.BindDouble(stmt.GetParameterIndex(DPTEST_TEST_ELEMENT_DoubleProperty4), m_doubleProps[3]);
    stmt.BindPoint3D(stmt.GetParameterIndex(DPTEST_TEST_ELEMENT_PointProperty1), m_pointProps[0]);
    stmt.BindPoint3D(stmt.GetParameterIndex(DPTEST_TEST_ELEMENT_PointProperty2), m_pointProps[1]);
    stmt.BindPoint3D(stmt.GetParameterIndex(DPTEST_TEST_ELEMENT_PointProperty3), m_pointProps[2]);
    stmt.BindPoint3D(stmt.GetParameterIndex(DPTEST_TEST_ELEMENT_PointProperty4), m_pointProps[3]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestElement::_BindUpdateParams(ECSqlStatement& stmt)
    {
    auto status = T_Super::_BindUpdateParams(stmt);
    if (DgnDbStatus::Success == status)
        BindParams(stmt);

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
        for (size_t i = 0; i < 4; i++)
            {
            m_intProps[i] = testEl->m_intProps[i];
            m_doubleProps[i] = testEl->m_doubleProps[i];
            m_pointProps[i] = testEl->m_pointProps[i];
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TestElement::ChangeElement(double len)
    {
    GeometryBuilderPtr builder = GeometryBuilder::CreateWorld(*this);
    builder->Append(*computeShape(len));
    builder->SetGeometryStreamAndPlacement(*this);
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
TestElement2dPtr TestElement2d::Create(DgnDbR db, DgnModelId mid, DgnCategoryId categoryId, DgnCode elementCode, double length)
{
    TestElement2dPtr testElement = new TestElement2d(CreateParams(db, mid, QueryClassId(db), categoryId));
    if (!testElement.IsValid())
        return nullptr;

    //  Add some hard-wired geometry
    GeometryBuilderPtr builder = GeometryBuilder::CreateWorld(*testElement);
    EXPECT_TRUE(builder->Append(*computeShape2d(length)));
    return (SUCCESS != builder->SetGeometryStreamAndPlacement(*testElement)) ? nullptr : testElement;
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
DgnDbStatus TestUniqueAspect::_LoadProperties(DgnElementCR el)
    {
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("SELECT " DPTEST_TEST_UNIQUE_ASPECT_TestUniqueAspectProperty " FROM %s WHERE(ElementId=?)", GetFullEcSqlClassName().c_str()).c_str());
    stmt->BindId(1, el.GetElementId());
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
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("UPDATE %s SET " DPTEST_TEST_UNIQUE_ASPECT_TestUniqueAspectProperty "=? WHERE(ElementId=?)", GetFullEcSqlClassName().c_str()).c_str());
    stmt->BindText(1, m_testUniqueAspectProperty.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    stmt->BindId(2, el.GetElementId());
    return (BE_SQLITE_DONE != stmt->Step())? DgnDbStatus::WriteError: DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestMultiAspect::_LoadProperties(DgnElementCR el)
    {
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("SELECT " DPTEST_TEST_MULTI_ASPECT_TestMultiAspectProperty " FROM %s WHERE(ECInstanceId=?)", GetFullEcSqlClassName().c_str()).c_str());
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
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("UPDATE %s SET " DPTEST_TEST_MULTI_ASPECT_TestMultiAspectProperty "=? WHERE(ECInstanceId=?)", GetFullEcSqlClassName().c_str()).c_str());
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
* @bsimethod                                    Sam.Wilson      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TestElementDrivesElementHandler::_ProcessDeletedDependency(DgnDbR db, dgn_TxnTable::ElementDep::DepRelData const& relData)
    {
    m_deletedRels.push_back(relData);

    if (nullptr != s_callback)
        s_callback->_ProcessDeletedDependency(db, relData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TestElementDrivesElementHandler::UpdateProperty1(DgnDbR db, EC::ECInstanceKeyCR key)
    {
    SetProperty1(db, "changed", key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TestElementDrivesElementHandler::SetProperty1(DgnDbR db, Utf8CP value, EC::ECInstanceKeyCR key)
    {
    ECSqlStatement stmt;
    stmt.Prepare(db, "UPDATE ONLY " DPTEST_SCHEMA_NAME "." DPTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME " SET Property1=?  WHERE(ECInstanceId=?)");
    stmt.BindText(1, value, IECSqlBinder::MakeCopy::No);
    stmt.BindId(2, key.GetECInstanceId());
    ASSERT_EQ( stmt.Step(), BE_SQLITE_DONE );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TestElementDrivesElementHandler::GetProperty1(DgnDbR db, EC::ECInstanceId id)
    {
    ECSqlStatement stmt;
    stmt.Prepare(db, "SELECT Property1 FROM " DPTEST_SCHEMA_NAME "." DPTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME " WHERE(ECInstanceId=?)");
    stmt.BindId(1, id);
    EXPECT_EQ(stmt.Step(), BE_SQLITE_ROW);
    return stmt.GetValueText(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey TestElementDrivesElementHandler::Insert(DgnDbR db, DgnElementId root, DgnElementId dependent)
    {
    Utf8String ecsql("INSERT INTO ");
    ecsql.append(GetECClass(db).GetECSqlName()).append("(SourceECInstanceId,TargetECInstanceId) VALUES(?,?)");

    CachedECSqlStatementPtr stmt = db.GetPreparedECSqlStatement(ecsql.c_str());

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
    RegisterHandler(TestUniqueAspectHandler::GetHandler());
    RegisterHandler(TestMultiAspectHandler::GetHandler());
    RegisterHandler(TestElementDrivesElementHandler::GetHandler());


    RegisterHandler(TestElementSub1Handler::GetHandler());
    RegisterHandler(TestElementSub2Handler::GetHandler());
    RegisterHandler(TestElementSub3Handler::GetHandler()); 
    RegisterHandler(TestElementComplexHandler::GetHandler());
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

    BeFileName schemaDir = schemaFile.GetDirectoryName();

    ECN::ECSchemaReadContextPtr contextPtr = ECN::ECSchemaReadContext::CreateContext();
    contextPtr->AddSchemaLocater(db.GetSchemaLocater());
    contextPtr->AddSchemaPath(schemaDir.GetName());

    ECN::ECSchemaPtr schemaPtr;
    ECN::SchemaReadStatus readSchemaStatus = ECN::ECSchema::ReadFromXmlFile(schemaPtr, schemaFile.GetName(), *contextPtr);
    if (ECN::SchemaReadStatus::Success != readSchemaStatus)
        return DgnDbStatus::ReadError;

    if (BentleyStatus::SUCCESS != db.Schemas().ImportECSchemas(contextPtr->GetCache()))
        return DgnDbStatus::BadSchema;

    return DgnDbStatus::Success;
    }

TestElementDrivesElementHandler::Callback* TestElementDrivesElementHandler::s_callback = nullptr;

