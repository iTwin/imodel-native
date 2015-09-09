/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Performance/PerformanceElementsTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <Bentley/BeTest.h>
#include <Bentley/BeTimeUtilities.h>
#include <ECDb/ECDbApi.h>
#include "PerformanceTestFixture.h"
#include "../TestFixture/DgnDbTestFixtures.h"
#include "PerformanceElements.h"

#include <Logging/bentleylogging.h>

#define LOG (*NativeLogging::LoggingManager::GetLogger (L"DgnDbPerformance"))

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_DGNDB_UNIT_TESTS_NAMESPACE
using namespace ElementInsertPerformanceTestNamespace;

HANDLER_DEFINE_MEMBERS(SimpleElementHandler)
HANDLER_DEFINE_MEMBERS(PerformanceElement1Handler)
HANDLER_DEFINE_MEMBERS(PerformanceElement2Handler)
HANDLER_DEFINE_MEMBERS(PerformanceElement3Handler)
HANDLER_DEFINE_MEMBERS(PerformanceElement4Handler)
HANDLER_DEFINE_MEMBERS(PerformanceElement4bHandler)
DOMAIN_DEFINE_MEMBERS(PerformanceElementsTestDomain)

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElementsTestDomain::PerformanceElementsTestDomain() : DgnDomain(ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, "Test Schema", 1)
    {
    RegisterHandler(SimpleElementHandler::GetHandler());
    RegisterHandler(PerformanceElement1Handler::GetHandler());
    RegisterHandler(PerformanceElement2Handler::GetHandler());
    RegisterHandler(PerformanceElement4Handler::GetHandler());
    RegisterHandler(PerformanceElement4bHandler::GetHandler());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
void PerformanceElementsTestDomain::RegisterDomainAndImportSchema(DgnDbR db, ECN::ECSchemaR schema)
    {
    DgnDomains::RegisterDomain(PerformanceElementsTestDomain::GetDomain()); 

    ECN::ECSchemaReadContextPtr schemaContext = ECN::ECSchemaReadContext::CreateContext();
    schemaContext->AddSchema(schema);
    DgnBaseDomain::GetDomain().ImportSchema(db, schemaContext->GetCache());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
void PerformanceElement1::_GetInsertParams(bvector<Utf8String>& insertParams)
    {
    T_Super::_GetInsertParams(insertParams);
    insertParams.push_back("Prop1_1");
    insertParams.push_back("Prop1_2");
    insertParams.push_back("Prop1_3");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement1::_BindInsertParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    statement.BindText(statement.GetParameterIndex("Prop1_1"), m_prop1_1.c_str(), IECSqlBinder::MakeCopy::No);
    statement.BindInt64(statement.GetParameterIndex("Prop1_2"), m_prop1_2);
    statement.BindDouble(statement.GetParameterIndex("Prop1_3"), m_prop1_3);
    return T_Super::_BindInsertParams(statement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement1::_UpdateInDb()
    {
    DgnDbStatus status = T_Super::_UpdateInDb();
    if (DgnDbStatus::Success != status)
        return status;

    CachedECSqlStatementPtr statement = GetDgnDb().GetPreparedECSqlStatement("UPDATE ONLY [" ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME "].[" ELEMENT_PERFORMANCE_ELEMENT1_CLASS "] SET [Prop1_1]=?,[Prop1_2]=?,[Prop1_3]=? WHERE ECInstanceId=?");
    if (!statement.IsValid())
        return DgnDbStatus::WriteError;

    Utf8String newString("New Prop1_1 string");
    statement->BindText(1, newString.c_str(), IECSqlBinder::MakeCopy::No);
    statement->BindInt64(2, 42);
    statement->BindDouble(3, 9.87654);

    statement->BindId(4, GetElementId());

    if (ECSqlStepStatus::Done != statement->Step())
        return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement1Ptr PerformanceElement1::Create(Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category)
    {
    PerformanceElement1Ptr ptr = new PerformanceElement1(PhysicalElement::CreateParams(db, modelId, classId, category));
    if (!ptr.IsValid())
        return nullptr;
    return ptr.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement1CPtr PerformanceElement1::Insert()
    {
    return GetDgnDb().Elements().Insert<PerformanceElement1>(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
void PerformanceElement2::_GetInsertParams(bvector<Utf8String>& insertParams)
    {
    T_Super::_GetInsertParams(insertParams);
    insertParams.push_back("Prop2_1");
    insertParams.push_back("Prop2_2");
    insertParams.push_back("Prop2_3");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement2::_BindInsertParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    statement.BindText(statement.GetParameterIndex("Prop2_1"), m_prop2_1.c_str(), IECSqlBinder::MakeCopy::No);
    statement.BindInt64(statement.GetParameterIndex("Prop2_2"), m_prop2_2);
    statement.BindDouble(statement.GetParameterIndex("Prop2_3"), m_prop2_3);
    return T_Super::_BindInsertParams(statement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement2::_UpdateInDb()
    {
    DgnDbStatus status = T_Super::_UpdateInDb();
    if (DgnDbStatus::Success != status)
        return status;

    CachedECSqlStatementPtr statement = GetDgnDb().GetPreparedECSqlStatement("UPDATE ONLY [" ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME "].[" ELEMENT_PERFORMANCE_ELEMENT2_CLASS "] SET [Prop2_1]=?,[Prop2_2]=?,[Prop2_3]=? WHERE ECInstanceId=?");
    if (!statement.IsValid())
        return DgnDbStatus::WriteError;

    Utf8String newString("New Prop2_1 string");
    statement->BindText(1, newString.c_str(), IECSqlBinder::MakeCopy::No);
    statement->BindInt64(2, 42);
    statement->BindDouble(3, 9.87654);

    statement->BindId(4, GetElementId());

    if (ECSqlStepStatus::Done != statement->Step())
        return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement2Ptr PerformanceElement2::Create(Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category)
    {
    PerformanceElement2Ptr ptr = new PerformanceElement2(PhysicalElement::CreateParams(db, modelId, classId, category));
    if (!ptr.IsValid())
        return nullptr;
    return ptr.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement2CPtr PerformanceElement2::Insert()
    {
    return GetDgnDb().Elements().Insert<PerformanceElement2>(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
void PerformanceElement3::_GetInsertParams(bvector<Utf8String>& insertParams)
    {
    T_Super::_GetInsertParams(insertParams);
    insertParams.push_back("Prop3_1");
    insertParams.push_back("Prop3_2");
    insertParams.push_back("Prop3_3");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement3::_BindInsertParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    statement.BindText(statement.GetParameterIndex("Prop3_1"), m_prop3_1.c_str(), IECSqlBinder::MakeCopy::No);
    statement.BindInt64(statement.GetParameterIndex("Prop3_2"), m_prop3_2);
    statement.BindDouble(statement.GetParameterIndex("Prop3_3"), m_prop3_3);
    return T_Super::_BindInsertParams(statement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement3::_UpdateInDb()
    {
    DgnDbStatus status = T_Super::_UpdateInDb();
    if (DgnDbStatus::Success != status)
        return status;

    CachedECSqlStatementPtr statement = GetDgnDb().GetPreparedECSqlStatement("UPDATE ONLY [" ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME "].[" ELEMENT_PERFORMANCE_ELEMENT3_CLASS "] SET [Prop3_1]=?,[Prop3_2]=?,[Prop3_3]=? WHERE ECInstanceId=?");
    if (!statement.IsValid())
        return DgnDbStatus::WriteError;

    Utf8String newString("New Prop3_1 string");
    statement->BindText(1, newString.c_str(), IECSqlBinder::MakeCopy::No);
    statement->BindInt64(2, 42);
    statement->BindDouble(3, 9.87654);

    statement->BindId(4, GetElementId());

    if (ECSqlStepStatus::Done != statement->Step())
        return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement3Ptr PerformanceElement3::Create(Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category)
    {
    PerformanceElement3Ptr ptr = new PerformanceElement3(PhysicalElement::CreateParams(db, modelId, classId, category));
    if (!ptr.IsValid())
        return nullptr;
    return ptr.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement3CPtr PerformanceElement3::Insert()
    {
    return GetDgnDb().Elements().Insert<PerformanceElement3>(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
void PerformanceElement4::_GetInsertParams(bvector<Utf8String>& insertParams)
    {
    T_Super::_GetInsertParams(insertParams);
    insertParams.push_back("Prop4_1");
    insertParams.push_back("Prop4_2");
    insertParams.push_back("Prop4_3");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement4::_BindInsertParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    statement.BindText(statement.GetParameterIndex("Prop4_1"), m_prop4_1.c_str(), IECSqlBinder::MakeCopy::No);
    statement.BindInt64(statement.GetParameterIndex("Prop4_2"), m_prop4_2);
    statement.BindDouble(statement.GetParameterIndex("Prop4_3"), m_prop4_3);
    return T_Super::_BindInsertParams(statement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement4::_UpdateInDb()
    {
    DgnDbStatus status = T_Super::_UpdateInDb();
    if (DgnDbStatus::Success != status)
        return status;

    CachedECSqlStatementPtr statement = GetDgnDb().GetPreparedECSqlStatement("UPDATE ONLY [" ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME "].[" ELEMENT_PERFORMANCE_ELEMENT4_CLASS "] SET [Prop4_1]=?,[Prop4_2]=?,[Prop4_3]=? WHERE ECInstanceId=?");
    if (!statement.IsValid())
        return DgnDbStatus::WriteError;

    Utf8String newString("New Prop4_1 string");
    statement->BindText(1, newString.c_str(), IECSqlBinder::MakeCopy::No);
    statement->BindInt64(2, 42);
    statement->BindDouble(3, 9.87654);

    statement->BindId(4, GetElementId());

    if (ECSqlStepStatus::Done != statement->Step())
        return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement4Ptr PerformanceElement4::Create(Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category)
    {
    PerformanceElement4Ptr ptr = new PerformanceElement4(PhysicalElement::CreateParams(db, modelId, classId, category));
    if (!ptr.IsValid())
        return nullptr;
    return ptr.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement4CPtr PerformanceElement4::Insert()
    {
    return GetDgnDb().Elements().Insert<PerformanceElement4>(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
void PerformanceElement4b::_GetInsertParams(bvector<Utf8String>& insertParams)
    {
    T_Super::_GetInsertParams(insertParams);
    insertParams.push_back("Prop4b_1");
    insertParams.push_back("Prop4b_2");
    insertParams.push_back("Prop4b_3");
    insertParams.push_back("Prop4b_4");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement4b::_BindInsertParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    statement.BindText(statement.GetParameterIndex("Prop4b_1"), m_prop4b_1.c_str(), IECSqlBinder::MakeCopy::No);
    statement.BindInt64(statement.GetParameterIndex("Prop4b_2"), m_prop4b_2);
    statement.BindDouble(statement.GetParameterIndex("Prop4b_3"), m_prop4b_3);
    statement.BindPoint3D(statement.GetParameterIndex("Prop4b_4"), m_prop4b_4);
    return T_Super::_BindInsertParams(statement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement4b::_UpdateInDb()
    {
    DgnDbStatus status = T_Super::_UpdateInDb();
    if (DgnDbStatus::Success != status)
        return status;

    CachedECSqlStatementPtr statement = GetDgnDb().GetPreparedECSqlStatement("UPDATE ONLY [" ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME "].[" ELEMENT_PERFORMANCE_ELEMENT4b_CLASS "] SET [Prop4b_1]=?,[Prop4b_2]=?,[Prop4b_3]=? WHERE ECInstanceId=?");
    if (!statement.IsValid())
        return DgnDbStatus::WriteError;

    Utf8String newString("New Prop4b_1 string");
    statement->BindText(1, newString.c_str(), IECSqlBinder::MakeCopy::No);
    statement->BindInt64(2, 42);
    statement->BindDouble(3, 9.87654);

    statement->BindId(4, GetElementId());

    if (ECSqlStepStatus::Done != statement->Step())
        return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement4bPtr PerformanceElement4b::Create(Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category)
    {
    PerformanceElement4bPtr ptr = new PerformanceElement4b(PhysicalElement::CreateParams(db, modelId, classId, category));
    if (!ptr.IsValid())
        return nullptr;
    return ptr.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
PerformanceElement4bCPtr PerformanceElement4b::Insert()
    {
    return GetDgnDb().Elements().Insert<PerformanceElement4b>(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
SimpleElementPtr SimpleElement::Create(Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category)
    {
    SimpleElementPtr ptr = new SimpleElement(DgnElement::CreateParams(db, modelId, classId, category));
    if (!ptr.IsValid())
        return nullptr;
    return ptr.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
SimpleElementCPtr SimpleElement::Insert()
    {
    return GetDgnDb().Elements().Insert<SimpleElement>(*this).get();
    }

//=======================================================================================
// @bsiclass                                                  Carole.MacDonald    09/15
//=======================================================================================
struct PerformanceElementsTestFixture : public PerformanceElementTestFixture
    {
    protected:
    //PerformanceTestingFrameWork     m_testObj;
    void TimeInsertion(int numInstances, Utf8CP schemaName, Utf8CP className, Utf8String testcaseName, Utf8String testName);
    void TimeUpdate(int numInstances, Utf8CP schemaName, Utf8CP className);
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
void PerformanceElementsTestFixture::TimeInsertion(int numInstances, Utf8CP schemaName, Utf8CP className, Utf8String testcaseName, Utf8String testName)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"ElementInsertPerformanceElementClass.idgndb", BeSQLite::Db::OpenMode::ReadWrite);
    ECN::ECSchemaReadContextPtr schemaContext = ECN::ECSchemaReadContext::CreateContext();
    BeFileName searchDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(searchDir);
    searchDir.AppendToPath(L"ECSchemas").AppendToPath(L"Dgn");
    schemaContext->AddSchemaPath(searchDir.GetName ());

    ECN::ECSchemaPtr schema = nullptr;
    if (ECN::SCHEMA_READ_STATUS_Success != ECN::ECSchema::ReadFromXmlString(schema, s_testSchemaXml, *schemaContext))
        return;

    PerformanceElementsTestDomain::RegisterDomainAndImportSchema(*m_db, *schema);

    DgnModelPtr model = CreatePhysicalModel();
    DgnCategoryId catid = m_db->Categories().QueryHighestId();
    DgnClassId mclassId = DgnClassId(m_db->Schemas().GetECClassId(schemaName, className));
    bvector<DgnElementPtr> testElements;
    for (int i = 0; i < numInstances; i++)
        {
        if (0 == strcmp(className, ELEMENT_PERFORMANCE_SIMPLEELEMENT_CLASS))
            {
            SimpleElementPtr element = SimpleElement::Create(*m_db, model->GetModelId(), mclassId, catid);
            ASSERT_TRUE(element != nullptr);
            testElements.push_back(element);
            }
        else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT1_CLASS))
            {
            PerformanceElement1Ptr element = PerformanceElement1::Create(*m_db, model->GetModelId(), mclassId, catid);
            ASSERT_TRUE(element != nullptr);
            testElements.push_back(element);
            }
        else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT2_CLASS))
            {
            PerformanceElement2Ptr element = PerformanceElement2::Create(*m_db, model->GetModelId(), mclassId, catid);
            ASSERT_TRUE(element != nullptr);
            testElements.push_back(element);
            }
        else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT3_CLASS))
            {
            PerformanceElement3Ptr element = PerformanceElement3::Create(*m_db, model->GetModelId(), mclassId, catid);
            ASSERT_TRUE(element != nullptr);
            testElements.push_back(element);
            }
        else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT4_CLASS))
            {
            PerformanceElement4Ptr element = PerformanceElement4::Create(*m_db, model->GetModelId(), mclassId, catid);
            ASSERT_TRUE(element != nullptr);
            testElements.push_back(element);
            }
        else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT4b_CLASS))
            {
            PerformanceElement4bPtr element = PerformanceElement4b::Create(*m_db, model->GetModelId(), mclassId, catid);
            ASSERT_TRUE(element != nullptr);
            testElements.push_back(element);
            }
        else if (0 == strcmp(className, "PhysicalElement"))
            {
            PhysicalElementPtr element = PhysicalElement::Create(PhysicalElement::CreateParams(*m_db, model->GetModelId(), mclassId, catid));
            ASSERT_TRUE(element != nullptr);
            testElements.push_back(element);
            }
        }

    DgnDbStatus stat = DgnDbStatus::Success;
    StopWatch timer(true);
    for (DgnElementPtr& element : testElements)
        {
        element->Insert(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat);
        }
    timer.Stop();

    LOGTODB(testcaseName, testName, timer.GetElapsedSeconds(), Utf8PrintfString("Inserting %d %s elements", numInstances, className).c_str(), numInstances);
    //m_testObj.writeTodb(insertTime, "PerformanceElementTestFixture.TimeInsertion", Utf8PrintfString("Inserting %d %s elements", numInstances, className).c_str(), numInstances);
    m_db->SaveChanges();
    m_db->CloseDb();
    }

TEST_F(PerformanceElementsTestFixture, ElementInsert)
    {
    TimeInsertion(1000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_SIMPLEELEMENT_CLASS, TEST_DETAILS);
    TimeInsertion(10000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_SIMPLEELEMENT_CLASS, TEST_DETAILS);
    TimeInsertion(100000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_SIMPLEELEMENT_CLASS, TEST_DETAILS);
    TimeInsertion(1000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_ELEMENT1_CLASS, TEST_DETAILS);
    TimeInsertion(10000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_ELEMENT1_CLASS, TEST_DETAILS);
    TimeInsertion(100000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_ELEMENT1_CLASS, TEST_DETAILS);
    TimeInsertion(1000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_ELEMENT4b_CLASS, TEST_DETAILS);
    TimeInsertion(10000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_ELEMENT4b_CLASS, TEST_DETAILS);
    TimeInsertion(100000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_ELEMENT4b_CLASS, TEST_DETAILS);
    TimeInsertion(1000, DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement, TEST_DETAILS);
    TimeInsertion(10000, DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement, TEST_DETAILS);
    TimeInsertion(100000, DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement, TEST_DETAILS);
    }