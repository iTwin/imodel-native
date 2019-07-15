/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include "DgnPlatformTestDomain.h"
#include <DgnPlatform/GeomPart.h>
#include <DgnPlatform/ElementGeometry.h>
#include <ECDb/ECDbApi.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DPTEST
USING_NAMESPACE_BENTLEY_DGN

DOMAIN_DEFINE_MEMBERS(DgnPlatformTestDomain)
HANDLER_DEFINE_MEMBERS(ImodelJsTestElementHandler)
HANDLER_DEFINE_MEMBERS(TestElementHandler)
HANDLER_DEFINE_MEMBERS(TestSpatialLocationHandler)
HANDLER_DEFINE_MEMBERS(TestPhysicalTypeHandler)
HANDLER_DEFINE_MEMBERS(TestGraphicalType2dHandler)
HANDLER_DEFINE_MEMBERS(TestElement2dHandler)
HANDLER_DEFINE_MEMBERS(TestInformationRecordHandler)
HANDLER_DEFINE_MEMBERS(TestUniqueAspectHandler)
HANDLER_DEFINE_MEMBERS(TestMultiAspectHandler)
HANDLER_DEFINE_MEMBERS(TestGroupHandler)
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
TestElementPtr TestElement::Create(DgnDbR db, ECN::ECClassCR subclass, DgnModelId mid, DgnCategoryId categoryId, Utf8CP elementCode)
    {
    if (!subclass.Is(db.Schemas().GetClass(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_CLASS_NAME)))
        return nullptr;
    TestElementPtr testElement = new TestElement(CreateParams(db, mid, DgnClassId(subclass.GetId().GetValue()), categoryId, Placement3d()));
    return testElement;
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
    GeometryBuilderPtr builder = GeometryBuilder::Create(*testElement); // Create at origin...
    builder->Append(*computeShape(shapeSize));
    if (SUCCESS != builder->Finish(*testElement))
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
    GeometryBuilderPtr builder = GeometryBuilder::Create(*testElement); // Create at origin...
    EXPECT_TRUE(builder->Append(ep));
    EXPECT_TRUE(builder->Append(*computeShape(shapeSize)));
    if (SUCCESS != builder->Finish(*testElement))
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
void TestElementHandler::_RegisterPropertyAccessors(ECSqlClassInfo& params, ECN::ClassLayoutCR layout)
    {
    T_Super::_RegisterPropertyAccessors(params, layout);

#define GETPROPSTR(MEMVAR) [](ECN::ECValueR value, DgnElementCR elIn){auto& el = (TestElement&)elIn; value = ECN::ECValue(el.MEMVAR.c_str()); return DgnDbStatus::Success;}
#define SETPROPSTR(MEMVAR) [](DgnElement& elIn, ECN::ECValueCR value){auto& el = (TestElement&)elIn; el.MEMVAR = value.ToString(); return DgnDbStatus::Success;}
#define GETPROP(MEMVAR) [](ECN::ECValueR value, DgnElementCR elIn){auto& el = (TestElement&)elIn; value = ECN::ECValue(el.MEMVAR); return DgnDbStatus::Success;}
#define SETPROP(MEMVAR,TYPE) [](DgnElement& elIn, ECN::ECValueCR value){auto& el = (TestElement&)elIn; el.MEMVAR = value.Get ## TYPE(); return DgnDbStatus::Success;}

#define GETSETPROPSTR(MEMVAR) GETPROPSTR(MEMVAR), SETPROPSTR(MEMVAR)
#define GETSETPROPDBL(MEMVAR) GETPROP(MEMVAR), SETPROP(MEMVAR,Double)
#define GETSETPROPINT(MEMVAR) GETPROP(MEMVAR), SETPROP(MEMVAR,Integer)
#define GETSETPROPP3(MEMVAR) GETPROP(MEMVAR), SETPROP(MEMVAR,Point3d)

    params.RegisterPropertyAccessors(layout, DPTEST_TEST_ELEMENT_TestElementProperty, GETSETPROPSTR(m_testElemProperty));
    params.RegisterPropertyAccessors(layout, DPTEST_TEST_ELEMENT_IntegerProperty1,    GETSETPROPINT(m_intProps[0]));
    params.RegisterPropertyAccessors(layout, DPTEST_TEST_ELEMENT_IntegerProperty2,    GETSETPROPINT(m_intProps[1]));
    params.RegisterPropertyAccessors(layout, DPTEST_TEST_ELEMENT_IntegerProperty3,    GETSETPROPINT(m_intProps[2]));
    params.RegisterPropertyAccessors(layout, DPTEST_TEST_ELEMENT_IntegerProperty4,    GETSETPROPINT(m_intProps[3]));
    params.RegisterPropertyAccessors(layout, DPTEST_TEST_ELEMENT_DoubleProperty1,     GETSETPROPDBL(m_doubleProps[0]));
    params.RegisterPropertyAccessors(layout, DPTEST_TEST_ELEMENT_DoubleProperty2,     GETSETPROPDBL(m_doubleProps[1]));
    params.RegisterPropertyAccessors(layout, DPTEST_TEST_ELEMENT_DoubleProperty3,     GETSETPROPDBL(m_doubleProps[2]));
    params.RegisterPropertyAccessors(layout, DPTEST_TEST_ELEMENT_DoubleProperty4,     GETSETPROPDBL(m_doubleProps[3]));
    params.RegisterPropertyAccessors(layout, DPTEST_TEST_ELEMENT_PointProperty1,      GETSETPROPP3(m_pointProps[0]));
    params.RegisterPropertyAccessors(layout, DPTEST_TEST_ELEMENT_PointProperty2,      GETSETPROPP3(m_pointProps[1]));
    params.RegisterPropertyAccessors(layout, DPTEST_TEST_ELEMENT_PointProperty3,      GETSETPROPP3(m_pointProps[2]));
    params.RegisterPropertyAccessors(layout, DPTEST_TEST_ELEMENT_PointProperty4,      GETSETPROPP3(m_pointProps[3]));
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
    m_pointProps[0] = stmt.GetValuePoint3d(params.GetSelectIndex(DPTEST_TEST_ELEMENT_PointProperty1));
    m_pointProps[1] = stmt.GetValuePoint3d(params.GetSelectIndex(DPTEST_TEST_ELEMENT_PointProperty2));
    m_pointProps[2] = stmt.GetValuePoint3d(params.GetSelectIndex(DPTEST_TEST_ELEMENT_PointProperty3));
    m_pointProps[3] = stmt.GetValuePoint3d(params.GetSelectIndex(DPTEST_TEST_ELEMENT_PointProperty4));

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TestElement::_BindWriteParams(ECSqlStatement& stmt, ForInsert forInsert)
    {
    T_Super::_BindWriteParams(stmt, forInsert);

    stmt.BindText(stmt.GetParameterIndex(DPTEST_TEST_ELEMENT_TestElementProperty), m_testElemProperty.c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindInt(stmt.GetParameterIndex(DPTEST_TEST_ELEMENT_IntegerProperty1), m_intProps[0]);
    stmt.BindInt(stmt.GetParameterIndex(DPTEST_TEST_ELEMENT_IntegerProperty2), m_intProps[1]);
    stmt.BindInt(stmt.GetParameterIndex(DPTEST_TEST_ELEMENT_IntegerProperty3), m_intProps[2]);
    stmt.BindInt(stmt.GetParameterIndex(DPTEST_TEST_ELEMENT_IntegerProperty4), m_intProps[3]);
    stmt.BindDouble(stmt.GetParameterIndex(DPTEST_TEST_ELEMENT_DoubleProperty1), m_doubleProps[0]);
    stmt.BindDouble(stmt.GetParameterIndex(DPTEST_TEST_ELEMENT_DoubleProperty2), m_doubleProps[1]);
    stmt.BindDouble(stmt.GetParameterIndex(DPTEST_TEST_ELEMENT_DoubleProperty3), m_doubleProps[2]);
    stmt.BindDouble(stmt.GetParameterIndex(DPTEST_TEST_ELEMENT_DoubleProperty4), m_doubleProps[3]);
    stmt.BindPoint3d(stmt.GetParameterIndex(DPTEST_TEST_ELEMENT_PointProperty1), m_pointProps[0]);
    stmt.BindPoint3d(stmt.GetParameterIndex(DPTEST_TEST_ELEMENT_PointProperty2), m_pointProps[1]);
    stmt.BindPoint3d(stmt.GetParameterIndex(DPTEST_TEST_ELEMENT_PointProperty3), m_pointProps[2]);
    stmt.BindPoint3d(stmt.GetParameterIndex(DPTEST_TEST_ELEMENT_PointProperty4), m_pointProps[3]);
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
    GeometryBuilderPtr builder = GeometryBuilder::Create(*this);
    builder->Append(*computeShape(len));
    builder->Finish(*this);
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
    GeometryBuilderPtr builder = GeometryBuilder::Create(*testElement); // Create at origin...
    EXPECT_TRUE(builder->Append(*computeShape2d(length)));
    return (SUCCESS != builder->Finish(*testElement)) ? nullptr : testElement;
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
* @bsimethod                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TestSpatialLocationPtr TestSpatialLocation::Create(SpatialModelR model, DgnCategoryId categoryId)
    {
    DgnDbR db = model.GetDgnDb();
    DgnClassId classId = db.Domains().GetClassId(TestSpatialLocationHandler::GetHandler());
    return new TestSpatialLocation(CreateParams(db, model.GetModelId(), classId, categoryId, Placement3d()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TestPhysicalTypePtr TestPhysicalType::Create(DefinitionModelR model, Utf8CP name)
    {
    DgnDbR db = model.GetDgnDb();
    DgnClassId classId = db.Domains().GetClassId(TestPhysicalTypeHandler::GetHandler());
    DgnCode code = CreateCode(model, name);
    return new TestPhysicalType(CreateParams(db, model.GetModelId(), classId, code));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TestGraphicalType2dPtr TestGraphicalType2d::Create(DefinitionModelR model, Utf8CP name)
    {
    DgnDbR db = model.GetDgnDb();
    DgnClassId classId = db.Domains().GetClassId(TestGraphicalType2dHandler::GetHandler());
    DgnCode code = CreateCode(model, name);
    return new TestGraphicalType2d(CreateParams(db, model.GetModelId(), classId, code));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall    03/17
+---------------+---------------+---------------+---------------+---------------+------*/
TestInformationRecordPtr TestInformationRecord::Create(InformationRecordModelR model)
    {
    DgnDbR db = model.GetDgnDb();
    DgnClassId classId = db.Domains().GetClassId(TestInformationRecordHandler::GetHandler());
    TestInformationRecordPtr element = new TestInformationRecord(CreateParams(db, model.GetModelId(), classId));
    BeAssert(element.IsValid());
    return element;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestUniqueAspect::_LoadProperties(DgnElementCR el)
    {
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("SELECT " DPTEST_TEST_UNIQUE_ASPECT_TestUniqueAspectProperty " FROM %s WHERE(Element.Id=?)", GetFullEcSqlClassName().c_str()).c_str());
    stmt->BindId(1, el.GetElementId());
    if (BE_SQLITE_ROW != stmt->Step())
        return DgnDbStatus::ReadError;
    m_testUniqueAspectProperty = stmt->GetValueText(0);
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestUniqueAspect::_UpdateProperties(DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetNonSelectPreparedECSqlStatement(Utf8PrintfString("UPDATE %s SET " DPTEST_TEST_UNIQUE_ASPECT_TestUniqueAspectProperty "=? WHERE(Element.Id=?)", GetFullEcSqlClassName().c_str()).c_str(), writeToken);
    stmt->BindText(1, m_testUniqueAspectProperty.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    stmt->BindId(2, el.GetElementId());
    return (BE_SQLITE_DONE != stmt->Step())? DgnDbStatus::WriteError: DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    07/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestUniqueAspect::_GetPropertyValue(ECN::ECValueR value, Utf8CP propertyName, PropertyArrayIndex const&) const
    {
    if (0 == strcmp(propertyName, DPTEST_TEST_UNIQUE_ASPECT_TestUniqueAspectProperty))
        {
        value.SetUtf8CP(m_testUniqueAspectProperty.c_str());
        return DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, DPTEST_TEST_UNIQUE_ASPECT_LengthProperty))
        {
        value.SetDouble(m_length);
        return DgnDbStatus::Success;
        }

    return DgnDbStatus::BadArg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    07/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestUniqueAspect::_SetPropertyValue(Utf8CP propertyName, ECN::ECValueCR value, PropertyArrayIndex const&)
    {
    if (0 == strcmp(propertyName, DPTEST_TEST_UNIQUE_ASPECT_TestUniqueAspectProperty))
        {
        m_testUniqueAspectProperty = value.ToString();
        return DgnDbStatus::Success;
        }

    if (0 == strcmp(propertyName, DPTEST_TEST_UNIQUE_ASPECT_LengthProperty))
        {
        m_length = value.GetDouble();
        return DgnDbStatus::Success;
        }

    return DgnDbStatus::BadArg;
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
DgnDbStatus TestMultiAspect::_UpdateProperties(DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    m_token = writeToken;
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetNonSelectPreparedECSqlStatement(Utf8PrintfString("UPDATE %s SET " DPTEST_TEST_MULTI_ASPECT_TestMultiAspectProperty "=? WHERE(ECInstanceId=?)", GetFullEcSqlClassName().c_str()).c_str(), writeToken);
    stmt->BindText(1, m_testMultiAspectProperty.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    stmt->BindId(2, GetAspectInstanceId());
    return (BE_SQLITE_DONE != stmt->Step())? DgnDbStatus::WriteError: DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    07/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestMultiAspect::_GetPropertyValue(ECN::ECValueR value, Utf8CP propertyName, PropertyArrayIndex const&) const
    {
    if (0 == strcmp(propertyName, DPTEST_TEST_MULTI_ASPECT_TestMultiAspectProperty))
        {
        value.SetUtf8CP(m_testMultiAspectProperty.c_str());
        return DgnDbStatus::Success;
        }

    return DgnDbStatus::BadArg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    07/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestMultiAspect::_SetPropertyValue(Utf8CP propertyName, ECN::ECValueCR value, PropertyArrayIndex const&)
    {
    if (0 == strcmp(propertyName, DPTEST_TEST_MULTI_ASPECT_TestMultiAspectProperty))
        {
        m_testMultiAspectProperty = value.ToString();
        return DgnDbStatus::Success;
        }

    return DgnDbStatus::BadArg;
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
    auto inst = db.Schemas().GetClass(key.GetClassId())->GetDefaultStandaloneEnabler()->CreateInstance();
    inst->SetValue("Property1", ECN::ECValue(value));
    ASSERT_EQ(BE_SQLITE_OK, db.UpdateLinkTableRelationshipProperties(key, *inst));
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
    ECInstanceKey rkey;
    db.InsertLinkTableRelationship(rkey, (ECN::ECRelationshipClassCR)(GetECClass(db)), root, dependent);
    return rkey;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnPlatformTestDomain::DgnPlatformTestDomain() : DgnDomain(DPTEST_SCHEMA_NAME, "DgnPlatform Test Schema", 1)
    {
    RegisterHandler(ImodelJsTestElementHandler::GetHandler());
    RegisterHandler(TestElementHandler::GetHandler());
    RegisterHandler(TestSpatialLocationHandler::GetHandler());
    RegisterHandler(TestPhysicalTypeHandler::GetHandler());
    RegisterHandler(TestGraphicalType2dHandler::GetHandler());
    RegisterHandler(TestElement2dHandler::GetHandler());
    RegisterHandler(TestGroupHandler::GetHandler());
    RegisterHandler(TestInformationRecordHandler::GetHandler());
    RegisterHandler(TestUniqueAspectHandler::GetHandler());
    RegisterHandler(TestMultiAspectHandler::GetHandler());
    RegisterHandler(TestElementDrivesElementHandler::GetHandler());
    
    }

TestElementDrivesElementHandler::Callback* TestElementDrivesElementHandler::s_callback = nullptr;

