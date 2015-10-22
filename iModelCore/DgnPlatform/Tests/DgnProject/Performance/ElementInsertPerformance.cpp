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

#include <Logging/bentleylogging.h>

#define LOG (*NativeLogging::LoggingManager::GetLogger (L"DgnDbPerformance"))

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_DGNDB_UNIT_TESTS_NAMESPACE

//=======================================================================================
//! Test Fixture for tests
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
    int startCount, maxCount,increment;
    startCount = increment = 100;
    maxCount = 1000;

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
            DgnElementCPtr el = InsertElement(DgnElement::Code());
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

//static
const DgnCategoryId PerformanceElementTestFixture::s_catId = DgnCategoryId((uint64_t)123);
const DgnAuthorityId PerformanceElementTestFixture::s_codeAuthorityId = DgnAuthorityId((uint64_t) 1);
Utf8CP const PerformanceElementTestFixture::s_textVal = "bla bla";
const double PerformanceElementTestFixture::s_doubleVal = -3.1415;
Utf8CP const PerformanceElementTestFixture::s_testSchemaXml =
    "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
    "  <ECSchemaReference name = 'dgn' version = '02.00' prefix = 'dgn' />"
    "  <ECClass typeName='Element1' >"
    "    <BaseClass>dgn:PhysicalElement</BaseClass>"
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


//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
PhysicalModelPtr PerformanceElementTestFixture::CreatePhysicalModel() const
    {
    DgnClassId mclassId = DgnClassId(m_db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalModel));
    PhysicalModelPtr targetModel = new PhysicalModel(PhysicalModel::CreateParams(*m_db, mclassId, DgnModel::CreateModelCode("Instances")));
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
    LOGTODB(testcaseName, testName, timer.GetElapsedSeconds(), scenario, s_instanceCount);
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementTestFixture, ElementInsertInDbWithSingleInsertApproach)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"ElementInsertPerformanceSingleInsertNumberedParams.idgndb", BeSQLite::Db::OpenMode::ReadWrite);
    ASSERT_EQ(SUCCESS, ImportTestSchema());

    PhysicalModelPtr model = CreatePhysicalModel();
    ASSERT_TRUE(model != nullptr);
    DgnModelId modelId = model->GetModelId();
    ASSERT_TRUE(modelId.IsValid());

    StopWatch timer(true);

    Utf8String code;
    for (int i = 0; i < s_instanceCount; i++)
        {
        //Call GetPreparedECSqlStatement for each instance (instead of once before) to insert as this is closer to the real world scenario
        CachedECSqlStatementPtr insertStmt = m_db->GetPreparedECSqlStatement("INSERT INTO ts.Element4 (ModelId,CategoryId,CodeAuthorityId,Code,CodeNameSpace,"
                                                                             "Prop1_1,Prop1_2,Prop1_3,"
                                                                             "Prop2_1,Prop2_2,Prop2_3,"
                                                                             "Prop3_1,Prop3_2,Prop3_3,"
                                                                             "Prop4_1,Prop4_2,Prop4_3) "
                                                                             "VALUES (?,?,?,?,'',?,?,?,?,?,?,?,?,?,?,?,?)");
        ASSERT_TRUE(insertStmt != nullptr);
        CachedECSqlStatement& stmt = *insertStmt;

        stmt.BindId(1, modelId);
        stmt.BindId(2, s_catId);
        stmt.BindId(3, s_codeAuthorityId);
        code.Sprintf("Id-%d", i);
        stmt.BindText(4, code.c_str(), IECSqlBinder::MakeCopy::No);

        stmt.BindText(5, s_textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64(6, s_int64Val);
        stmt.BindDouble(7, s_doubleVal);

        stmt.BindText(8, s_textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64(9, s_int64Val);
        stmt.BindDouble(10, s_doubleVal);

        stmt.BindText(11, s_textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64(12, s_int64Val);
        stmt.BindDouble(13, s_doubleVal);

        stmt.BindText(14, s_textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64(15, s_int64Val);
        stmt.BindDouble(16, s_doubleVal);

        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

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

    PhysicalModelPtr model = CreatePhysicalModel();
    ASSERT_TRUE(model != nullptr);
    DgnModelId modelId = model->GetModelId();
    ASSERT_TRUE(modelId.IsValid());

    StopWatch timer(true);

    Utf8String code;
    for (int i = 0; i < s_instanceCount; i++)
        {
        //Call GetPreparedECSqlStatement for each instance (instead of once before) to insert as this is closer to the real world scenario
        CachedECSqlStatementPtr insertStmt = m_db->GetPreparedECSqlStatement("INSERT INTO ts.Element4 (ModelId,CategoryId,CodeAuthorityId,Code,CodeNameSpace) VALUES (?,?,?,?,'')");
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
        insertStmt->BindId(3, s_codeAuthorityId);
        code.Sprintf("Id-%d", i);
        insertStmt->BindText(4, code.c_str(), IECSqlBinder::MakeCopy::No);

        ECInstanceKey newKey;
        ASSERT_EQ(BE_SQLITE_DONE, insertStmt->Step(newKey));
        insertStmt->Reset();
        insertStmt->ClearBindings();

        for (CachedECSqlStatementPtr& updateStmt : updateStmts)
            {
            updateStmt->BindText(1, s_textVal, IECSqlBinder::MakeCopy::No);
            updateStmt->BindInt64(2, s_int64Val);
            updateStmt->BindDouble(3, s_doubleVal);
            updateStmt->BindId(4, newKey.GetECInstanceId());
            ASSERT_EQ(BE_SQLITE_DONE, updateStmt->Step());
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

    PhysicalModelPtr model = CreatePhysicalModel();
    ASSERT_TRUE(model != nullptr);
    DgnModelId modelId = model->GetModelId();
    ASSERT_TRUE(modelId.IsValid());

    StopWatch timer(true);
    Utf8String code;
    for (int i = 0; i < s_instanceCount; i++)
        {
        //Call GetPreparedECSqlStatement for each instance (instead of once before) to insert as this is closer to the real world scenario
        CachedECSqlStatementPtr insertStmt = m_db->GetPreparedECSqlStatement("INSERT INTO ts.Element4 (ModelId,CategoryId,CodeAuthorityId,Code,CodeNameSpace,"
                                                                             "Prop1_1,Prop1_2,Prop1_3,"
                                                                             "Prop2_1,Prop2_2,Prop2_3,"
                                                                             "Prop3_1,Prop3_2,Prop3_3,"
                                                                             "Prop4_1,Prop4_2,Prop4_3) "
                                                                             "VALUES (:modelid,:catid,:authorityid,:code,'',"
                                                                             ":p11,:p12,:p13,"
                                                                             ":p21,:p22,:p23,"
                                                                             ":p31,:p32,:p33,"
                                                                             ":p41,:p42,:p43)");
        ASSERT_TRUE(insertStmt != nullptr);

        CachedECSqlStatement& stmt = *insertStmt;

        stmt.BindId(stmt.GetParameterIndex("modelid"), modelId);
        stmt.BindId(stmt.GetParameterIndex("catid"), s_catId);
        stmt.BindId(stmt.GetParameterIndex("authorityid"), s_codeAuthorityId);
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

        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

        stmt.Reset();
        stmt.ClearBindings();
        }

    timer.Stop();
    CommitAndLogTiming(timer, "Single Insert (named parameters)", TEST_DETAILS);
    }
