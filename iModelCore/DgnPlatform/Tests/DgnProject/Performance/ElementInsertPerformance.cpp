/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Performance/ElementInsertPerformance.cpp $
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
#include "ElementInsertPerformance.h"

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
DOMAIN_DEFINE_MEMBERS(ElementInsertPerformanceTestDomain)

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
ElementInsertPerformanceTestDomain::ElementInsertPerformanceTestDomain() : DgnDomain(ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, "Test Schema", 1)
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
void ElementInsertPerformanceTestDomain::RegisterDomainAndImportSchema(DgnDbR db, Utf8CP schemaXml)
    {
    DgnDomains::RegisterDomain(ElementInsertPerformanceTestDomain::GetDomain()); 

    ECN::ECSchemaReadContextPtr schemaContext = ECN::ECSchemaReadContext::CreateContext();
    BeFileName searchDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(searchDir);
    searchDir.AppendToPath(L"ECSchemas").AppendToPath(L"Dgn");
    schemaContext->AddSchemaPath(searchDir.GetName ());

    ECN::ECSchemaPtr schema = nullptr;
    if (ECN::SCHEMA_READ_STATUS_Success != ECN::ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext))
        return;

    DgnBaseDomain::GetDomain().ImportSchema(db, schemaContext->GetCache());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement1::_InsertInDb(BeSQLite::EC::ECSqlStatement& statement)
    {
    statement.BindText(statement.GetParameterIndex("Prop1_1"), m_prop1_1.c_str(), IECSqlBinder::MakeCopy::No);
    statement.BindInt64(statement.GetParameterIndex("Prop1_2"), m_prop1_2);
    statement.BindDouble(statement.GetParameterIndex("Prop1_3"), m_prop1_3);
    return T_Super::_InsertInDb(statement);
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
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement2::_InsertInDb(BeSQLite::EC::ECSqlStatement& statement)
    {
    statement.BindText(statement.GetParameterIndex("Prop2_1"), m_prop2_1.c_str(), IECSqlBinder::MakeCopy::No);
    statement.BindInt64(statement.GetParameterIndex("Prop2_2"), m_prop2_2);
    statement.BindDouble(statement.GetParameterIndex("Prop2_3"), m_prop2_3);
    return T_Super::_InsertInDb(statement);
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
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement3::_InsertInDb(BeSQLite::EC::ECSqlStatement& statement)
    {
    statement.BindText(statement.GetParameterIndex("Prop3_1"), m_prop3_1.c_str(), IECSqlBinder::MakeCopy::No);
    statement.BindInt64(statement.GetParameterIndex("Prop3_2"), m_prop3_2);
    statement.BindDouble(statement.GetParameterIndex("Prop3_3"), m_prop3_3);
    return T_Super::_InsertInDb(statement);
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
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement4::_InsertInDb(BeSQLite::EC::ECSqlStatement& statement)
    {
    statement.BindText(statement.GetParameterIndex("Prop4_1"), m_prop4_1.c_str(), IECSqlBinder::MakeCopy::No);
    statement.BindInt64(statement.GetParameterIndex("Prop4_2"), m_prop4_2);
    statement.BindDouble(statement.GetParameterIndex("Prop4_3"), m_prop4_3);
    return T_Super::_InsertInDb(statement);
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
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus PerformanceElement4b::_InsertInDb(BeSQLite::EC::ECSqlStatement& statement)
    {
    statement.BindText(statement.GetParameterIndex("Prop4b_1"), m_prop4b_1.c_str(), IECSqlBinder::MakeCopy::No);
    statement.BindInt64(statement.GetParameterIndex("Prop4b_2"), m_prop4b_2);
    statement.BindDouble(statement.GetParameterIndex("Prop4b_3"), m_prop4b_3);
    statement.BindPoint3D(statement.GetParameterIndex("Prop4b_4"), m_prop4b_4);
    return T_Super::_InsertInDb(statement);
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
DgnDbStatus SimpleElement::_InsertInDb(BeSQLite::EC::ECSqlStatement& statement)
    {
    return T_Super::_InsertInDb(statement);
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
//! Test Fixtrue for tests
// @bsiclass                                                     Majd.Uddin      06/15
//=======================================================================================
struct PerformanceElementItem : public DgnDbTestFixture
{

};

/*---------------------------------------------------------------------------------**//**
* Test to measure time of Insert, Select, Update and Delete of an Element Item
* @bsimethod                                    Majd.Uddin      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceElementItem, CRUD)
{
    //Read from ecdb: the start, maximum and increment number to run the test
    int startCount, maxCount,increment;
    if (! PerformanceResultRecorder::getCounters(startCount, maxCount, increment))
        {
        startCount = increment = 100;
        maxCount = 1000;
        }

    StopWatch elementTimer("Insert Element", false);

    SetupProject(L"3dMetricGeneral.idgndb", L"ElementInsertPerformanceTests.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    int counter;
    double insertTime, selectTime, updateTime, deleteTime;
    DgnDbStatus status;

    for (counter = startCount; counter <= maxCount; counter = counter + increment)
    {
        insertTime = selectTime = updateTime = deleteTime = 0.0;
        for (int i = 1; i <= counter; i++)
        {
            //First insert the Element
            elementTimer.Start();
            DgnElementCPtr el = InsertElement(DgnElement::Code(Utf8PrintfString("E%d", i)));
            EXPECT_TRUE(el.IsValid());
            elementTimer.Stop();
            insertTime = insertTime + elementTimer.GetElapsedSeconds();
            EXPECT_EQ(&el->GetElementHandler(), &TestElementHandler::GetHandler());

            //Time to select a single ElementItem
            elementTimer.Start();
            EXPECT_TRUE(SelectElementItem(el->GetElementId()));
            elementTimer.Stop();
            selectTime = selectTime + elementTimer.GetElapsedSeconds();

            //Now Update data and measure time for Update
            TestElementPtr mod = m_db->Elements().GetForEdit<TestElement>(el->GetElementId());
            EXPECT_TRUE(mod.IsValid());
            mod->SetTestItemProperty("Test - New");
            elementTimer.Start();
            mod->Update(&status);
            elementTimer.Stop();
            updateTime = updateTime + elementTimer.GetElapsedSeconds();

            //Now delete data and measure time for Delete
            elementTimer.Start();
            DgnDbStatus status2 = el->Delete();
            EXPECT_EQ(DgnDbStatus::Success, status2);
            elementTimer.Stop();
            deleteTime = deleteTime + elementTimer.GetElapsedSeconds();

        }

        //Write results to Db for analysis
        LOGTODB(TEST_DETAILS, insertTime, "Insert", counter);
        LOGTODB(TEST_DETAILS, selectTime, "Select", counter);
        LOGTODB(TEST_DETAILS, updateTime, "Update", counter);
        LOGTODB(TEST_DETAILS, deleteTime, "Delete", counter);
    }

}

//=======================================================================================
// @bsiclass                                                     Krischan.Eberle      06/15
//=======================================================================================
struct PerformanceElementTestFixture : public DgnDbTestFixture
    {

    protected:
    static const DgnCategoryId s_catId;
    static const int s_instanceCount = 20000;
    static Utf8CP const s_textVal;
    static const int64_t s_int64Val = 20000000LL;
    static const double s_doubleVal;
    static Utf8CP const s_testSchemaXml;

    //PerformanceTestingFrameWork     m_testObj;

    BentleyStatus ImportTestSchema() const;
    DgnModelId InsertDgnModel() const;
    void CommitAndLogTiming(StopWatch& timer, Utf8CP scenario, Utf8String testcaseName, Utf8String testName) const;

    PhysicalModelPtr CreatePhysicalModel() const;

    void TimeInsertion(int numInstances, Utf8CP schemaName, Utf8CP className, Utf8String testcaseName, Utf8String testName);
    };

//static
const DgnCategoryId PerformanceElementTestFixture::s_catId = DgnCategoryId((int64_t)123);
Utf8CP const PerformanceElementTestFixture::s_textVal = "bla bla";
const double PerformanceElementTestFixture::s_doubleVal = -3.1415;
Utf8CP const PerformanceElementTestFixture::s_testSchemaXml =
    "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
    "  <ECSchemaReference name = 'dgn' version = '02.00' prefix = 'dgn' />"
    "  <ECClass typeName='Element1' >"
    "    <BaseClass>dgn:GeometricElement</BaseClass>"
    "    <ECProperty propertyName='Prop1_1' typeName='string' />"
    "    <ECProperty propertyName='Prop1_2' typeName='long' />"
    "    <ECProperty propertyName='Prop1_3' typeName='double' />"
    "  </ECClass>"
    "  <ECClass typeName='Element2' >"
    "    <BaseClass>Element1</BaseClass>"
    "    <ECProperty propertyName='Prop2_1' typeName='string' />"
    "    <ECProperty propertyName='Prop2_2' typeName='long' />"
    "    <ECProperty propertyName='Prop2_3' typeName='double' />"
    "  </ECClass>"
    "  <ECClass typeName='Element3' >"
    "    <BaseClass>Element2</BaseClass>"
    "    <ECProperty propertyName='Prop3_1' typeName='string' />"
    "    <ECProperty propertyName='Prop3_2' typeName='long' />"
    "    <ECProperty propertyName='Prop3_3' typeName='double' />"
    "  </ECClass>"
    "  <ECClass typeName='Element4' >"
    "    <BaseClass>Element3</BaseClass>"
    "    <ECProperty propertyName='Prop4_1' typeName='string' />"
    "    <ECProperty propertyName='Prop4_2' typeName='long' />"
    "    <ECProperty propertyName='Prop4_3' typeName='double' />"
    "  </ECClass>"
    "  <ECClass typeName='Element4b' >"
    "    <BaseClass>Element3</BaseClass>"
    "    <ECProperty propertyName='Prop4b_1' typeName='string' />"
    "    <ECProperty propertyName='Prop4b_2' typeName='long' />"
    "    <ECProperty propertyName='Prop4b_3' typeName='double' />"
    "    <ECProperty propertyName='Prop4b_4' typeName='point3d' />"
    "  </ECClass>"
    "  <ECClass typeName='SimpleElement'>"
    "    <BaseClass>dgn:Element</BaseClass>"
    "  </ECClass>"
    "</ECSchema>";

//--------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformanceElementTestFixture::ImportTestSchema() const
    {
    ECN::ECSchemaReadContextPtr schemaContext = ECN::ECSchemaReadContext::CreateContext();
    BeFileName searchDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(searchDir);
    searchDir.AppendToPath(L"ECSchemas").AppendToPath(L"Dgn");
    schemaContext->AddSchemaPath(searchDir.GetName ());

    ECN::ECSchemaPtr schema = nullptr;
    if (ECN::SCHEMA_READ_STATUS_Success != ECN::ECSchema::ReadFromXmlString(schema, s_testSchemaXml, *schemaContext))
        return ERROR;

    if (SUCCESS != m_db->Schemas().ImportECSchemas(schemaContext->GetCache()))
        return ERROR;

    m_db->ClearECDbCache();

    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
DgnModelId PerformanceElementTestFixture::InsertDgnModel() const
    {
    //cannot let ECDb generate the model id as DgnDb has its own sequence and model id 1 is already in use
    ECSqlStatement stmt;
    if (stmt.Prepare(*m_db, "INSERT INTO dgn.Model3d (ECInstanceId, Name) VALUES (200, 'TestModel')") != ECSqlStatus::Success)
        return DgnModelId();

    ECInstanceKey newKey;
    if (stmt.Step(newKey) == ECSqlStepStatus::Done)
        return DgnModelId (newKey.GetECInstanceId().GetValue ());

    return DgnModelId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
void PerformanceElementTestFixture::TimeInsertion(int numInstances, Utf8CP schemaName, Utf8CP className, Utf8String testcaseName, Utf8String testName)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"ElementInsertPerformanceElementClass.idgndb", BeSQLite::Db::OpenMode::ReadWrite);
    ElementInsertPerformanceTestDomain::RegisterDomainAndImportSchema(*m_db, s_testSchemaXml);

    DgnModelPtr model = CreatePhysicalModel();
    DgnCategoryId catid = m_db->Categories().QueryHighestId();
    DgnClassId mclassId = DgnClassId(m_db->Schemas().GetECClassId(schemaName, className));

    bvector<DgnElementPtr> testElements;
    if (0 == strcmp(className, ELEMENT_PERFORMANCE_SIMPLEELEMENT_CLASS))
        {
        for (int i = 0; i < numInstances; i++)
            {
            SimpleElementPtr element = SimpleElement::Create(*m_db, model->GetModelId(), mclassId, catid);
            ASSERT_TRUE(element != nullptr);
            testElements.push_back(element);
            }
        }
    else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT1_CLASS))
        {
        for (int i = 0; i < numInstances; i++)
            {
            PerformanceElement1Ptr element = PerformanceElement1::Create(*m_db, model->GetModelId(), mclassId, catid);
            ASSERT_TRUE(element != nullptr);
            testElements.push_back(element);
            }
        }
    else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT2_CLASS))
        {
        for (int i = 0; i < numInstances; i++)
            {
            PerformanceElement2Ptr element = PerformanceElement2::Create(*m_db, model->GetModelId(), mclassId, catid);
            ASSERT_TRUE(element != nullptr);
            testElements.push_back(element);
            }
        }
    else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT3_CLASS))
        {
        for (int i = 0; i < numInstances; i++)
            {
            PerformanceElement3Ptr element = PerformanceElement3::Create(*m_db, model->GetModelId(), mclassId, catid);
            ASSERT_TRUE(element != nullptr);
            testElements.push_back(element);
            }
        }
    else if (0 == strcmp(className, ELEMENT_PERFORMANCE_ELEMENT4_CLASS))
        {
        for (int i = 0; i < numInstances; i++)
            {
            PerformanceElement4Ptr element = PerformanceElement4::Create(*m_db, model->GetModelId(), mclassId, catid);
            ASSERT_TRUE(element != nullptr);
            testElements.push_back(element);
            }
        }
    else if (0 == strcmp(className, "PhysicalElement"))
        {
        for (int i = 0; i < numInstances; i++)
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
PhysicalModelPtr PerformanceElementTestFixture::CreatePhysicalModel() const
    {
    DgnClassId mclassId = DgnClassId(m_db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalModel));
    PhysicalModelPtr targetModel = new PhysicalModel(PhysicalModel::CreateParams(*m_db, mclassId, "Instances"));
    EXPECT_EQ( DgnDbStatus::Success , targetModel->Insert() );       /* Insert the new model into the DgnDb */
    return targetModel;
    }
//--------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementTestFixture::CommitAndLogTiming(StopWatch& timer, Utf8CP scenario, Utf8String testcaseName, Utf8String testName) const
    {
    StopWatch commitTimer(true);
    ASSERT_EQ(BE_SQLITE_OK, m_db->SaveChanges());
    commitTimer.Stop();
    LOG.infov("%s> Inserting %d instances (5 inheritence levels, 3 properties per class) took %.4f seconds. Commit time: %.4f seconds", scenario, s_instanceCount,
                            timer.GetElapsedSeconds(),
                            commitTimer.GetElapsedSeconds ());
    LOGTODB(testcaseName, testName, commitTimer.GetElapsedSeconds(), scenario, s_instanceCount);
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementTestFixture, ElementInsertInDbWithSingleInsertApproach)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"ElementInsertPerformanceSingleInsertNumberedParams.idgndb", BeSQLite::Db::OpenMode::ReadWrite);
    ASSERT_EQ(SUCCESS, ImportTestSchema());

    DgnModelId modelId = InsertDgnModel();
    ASSERT_TRUE(modelId.IsValid());

    StopWatch timer(true);

    Utf8String code;
    for (int i = 0; i < s_instanceCount; i++)
        {
        //Call GetPreparedECSqlStatement for each instance (instead of once before) to insert as this is closer to the real world scenario
        CachedECSqlStatementPtr insertStmt = m_db->GetPreparedECSqlStatement("INSERT INTO ts.Element4 (ModelId,CategoryId,Code,"
                                                                             "Prop1_1,Prop1_2,Prop1_3,"
                                                                             "Prop2_1,Prop2_2,Prop2_3,"
                                                                             "Prop3_1,Prop3_2,Prop3_3,"
                                                                             "Prop4_1,Prop4_2,Prop4_3) "
                                                                             "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
        ASSERT_TRUE(insertStmt != nullptr);
        CachedECSqlStatement& stmt = *insertStmt;

        stmt.BindId(1, modelId);
        stmt.BindId(2, s_catId);
        code.Sprintf("Id-%d", i);
        stmt.BindText(3, code.c_str(), IECSqlBinder::MakeCopy::No);

        stmt.BindText(4, s_textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64(5, s_int64Val);
        stmt.BindDouble(6, s_doubleVal);

        stmt.BindText(7, s_textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64(8, s_int64Val);
        stmt.BindDouble(9, s_doubleVal);

        stmt.BindText(10, s_textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64(11, s_int64Val);
        stmt.BindDouble(12, s_doubleVal);

        stmt.BindText(13, s_textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64(14, s_int64Val);
        stmt.BindDouble(15, s_doubleVal);

        ASSERT_EQ(ECSqlStepStatus::Done, stmt.Step());

        stmt.Reset();
        stmt.ClearBindings();
        }

    timer.Stop();
    CommitAndLogTiming(timer, "Single Insert (numeric parameters)", TEST_DETAILS);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementTestFixture, ElementInsertInDbWithInsertUpdateApproach)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"ElementInsertPerformanceInsertUpdateApproach.idgndb", BeSQLite::Db::OpenMode::ReadWrite);
    ASSERT_EQ(SUCCESS, ImportTestSchema());

    DgnModelId modelId = InsertDgnModel();
    ASSERT_TRUE(modelId.IsValid());

    StopWatch timer(true);

    Utf8String code;
    for (int i = 0; i < s_instanceCount; i++)
        {
        //Call GetPreparedECSqlStatement for each instance (instead of once before) to insert as this is closer to the real world scenario
        CachedECSqlStatementPtr insertStmt = m_db->GetPreparedECSqlStatement("INSERT INTO ts.Element4 (ModelId, CategoryId, Code) VALUES (?,?,?)");
        ASSERT_TRUE(insertStmt != nullptr);

        std::vector<CachedECSqlStatementPtr> updateStmts;
        CachedECSqlStatementPtr updateStmt = m_db->GetPreparedECSqlStatement("UPDATE ONLY ts.Element1 SET Prop1_1 = ?, Prop1_2 = ?, Prop1_3 = ? WHERE ECInstanceId=?");
        ASSERT_TRUE(updateStmt != nullptr);
        updateStmts.push_back(updateStmt);

        updateStmt = m_db->GetPreparedECSqlStatement("UPDATE ONLY ts.Element2 SET Prop2_1 = ?, Prop2_2 = ?, Prop2_3 = ? WHERE ECInstanceId=?");
        ASSERT_TRUE(updateStmt != nullptr);
        updateStmts.push_back(updateStmt);

        updateStmt = m_db->GetPreparedECSqlStatement("UPDATE ONLY ts.Element3 SET Prop3_1 = ?, Prop3_2 = ?, Prop3_3 = ? WHERE ECInstanceId=?");
        ASSERT_TRUE(updateStmt != nullptr);
        updateStmts.push_back(updateStmt);

        updateStmt = m_db->GetPreparedECSqlStatement("UPDATE ONLY ts.Element4 SET Prop4_1 = ?, Prop4_2 = ?, Prop4_3 = ? WHERE ECInstanceId=?");
        ASSERT_TRUE(updateStmt != nullptr);
        updateStmts.push_back(updateStmt);

        insertStmt->BindId(1, modelId);
        insertStmt->BindId(2, s_catId);
        code.Sprintf("Id-%d", i);
        insertStmt->BindText(3, code.c_str(), IECSqlBinder::MakeCopy::No);

        ECInstanceKey newKey;
        ASSERT_EQ(ECSqlStepStatus::Done, insertStmt->Step(newKey));
        insertStmt->Reset();
        insertStmt->ClearBindings();

        for (CachedECSqlStatementPtr& updateStmt : updateStmts)
            {
            updateStmt->BindText(1, s_textVal, IECSqlBinder::MakeCopy::No);
            updateStmt->BindInt64(2, s_int64Val);
            updateStmt->BindDouble(3, s_doubleVal);
            updateStmt->BindId(4, newKey.GetECInstanceId());
            ASSERT_EQ(ECSqlStepStatus::Done, updateStmt->Step());
            updateStmt->Reset();
            updateStmt->ClearBindings();
            }
        }

    timer.Stop();
    CommitAndLogTiming(timer, "Insert & Update sub props", TEST_DETAILS);
    }
//--------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementTestFixture, ElementInsertInDbWithSingleInsertApproachNamedParameters)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"ElementInsertPerformanceSingleInsertNamedParams.idgndb", BeSQLite::Db::OpenMode::ReadWrite);
    ASSERT_EQ(SUCCESS, ImportTestSchema());

    DgnModelId modelId = InsertDgnModel();
    ASSERT_TRUE(modelId.IsValid());

    StopWatch timer(true);
    Utf8String code;
    for (int i = 0; i < s_instanceCount; i++)
        {
        //Call GetPreparedECSqlStatement for each instance (instead of once before) to insert as this is closer to the real world scenario
        CachedECSqlStatementPtr insertStmt = m_db->GetPreparedECSqlStatement("INSERT INTO ts.Element4 (ModelId,CategoryId,Code,"
                                                                             "Prop1_1,Prop1_2,Prop1_3,"
                                                                             "Prop2_1,Prop2_2,Prop2_3,"
                                                                             "Prop3_1,Prop3_2,Prop3_3,"
                                                                             "Prop4_1,Prop4_2,Prop4_3) "
                                                                             "VALUES (:modelid,:catid,:code,"
                                                                             ":p11,:p12,:p13,"
                                                                             ":p21,:p22,:p23,"
                                                                             ":p31,:p32,:p33,"
                                                                             ":p41,:p42,:p43)");
        ASSERT_TRUE(insertStmt != nullptr);

        CachedECSqlStatement& stmt = *insertStmt;

        stmt.BindId(stmt.GetParameterIndex("modelid"), modelId);
        stmt.BindId(stmt.GetParameterIndex("catid"), s_catId);
        code.Sprintf("Id-%d", i);
        stmt.BindText(stmt.GetParameterIndex("code"), code.c_str(), IECSqlBinder::MakeCopy::No);

        stmt.BindText(stmt.GetParameterIndex("p11"), s_textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64(stmt.GetParameterIndex("p12"), s_int64Val);
        stmt.BindDouble(stmt.GetParameterIndex("p13"), s_doubleVal);

        stmt.BindText(stmt.GetParameterIndex("p21"), s_textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64(stmt.GetParameterIndex("p22"), s_int64Val);
        stmt.BindDouble(stmt.GetParameterIndex("p23"), s_doubleVal);

        stmt.BindText(stmt.GetParameterIndex("p31"), s_textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64(stmt.GetParameterIndex("p32"), s_int64Val);
        stmt.BindDouble(stmt.GetParameterIndex("p33"), s_doubleVal);

        stmt.BindText(stmt.GetParameterIndex("p41"), s_textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64(stmt.GetParameterIndex("p42"), s_int64Val);
        stmt.BindDouble(stmt.GetParameterIndex("p43"), s_doubleVal);

        ASSERT_EQ(ECSqlStepStatus::Done, stmt.Step());

        stmt.Reset();
        stmt.ClearBindings();
        }

    timer.Stop();
    CommitAndLogTiming(timer, "Single Insert (named parameters)", TEST_DETAILS);
    }

TEST_F(PerformanceElementTestFixture, ElementInsertInDbWithSingleInsertWithElementClass)
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