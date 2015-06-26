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

#include <Logging/bentleylogging.h>

#define LOG (*NativeLogging::LoggingManager::GetLogger (L"DgnDbPerformance"))

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_DGNDB_UNIT_TESTS_NAMESPACE

//=======================================================================================
//! Test Fixtrue for tests
// @bsiclass                                                     Majd.Uddin      06/15
//=======================================================================================
struct PerformanceElementItem : public DgnDbTestFixture
{
public:
    PerformanceTestingFrameWork     m_testObj;

};

//=======================================================================================
// @bsiclass                                                     Krischan.Eberle      06/15
//=======================================================================================
struct PerformanceElementTestFixture : public DgnDbTestFixture
    {
    };

/*---------------------------------------------------------------------------------**//**
* Test to measure time of Insert, Select, Update and Delete of an Element Item
* @bsimethod                                    Majd.Uddin      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceElementItem, CRUD)
{
    //Read from ecdb: the start, maximum and increment number to run the test
    int startCount = m_testObj.getStartNum();
    int maxCount = m_testObj.getEndNum();
    int increment = m_testObj.getIncrement();

    StopWatch elementTimer("Insert Element", false);

    SetupProject(L"3dMetricGeneral.idgndb", L"ElementItemInsertPerformanceTests.idgndb", BeSQLite::Db::OPEN_ReadWrite);

    int counter;
    double elementTime, elementItemTime, selectTime, updateTime, deleteTime, deleteElementTime;
    for (counter = startCount; counter <= maxCount; counter = counter + increment)
    {
        elementTime = elementItemTime = selectTime = updateTime = deleteTime = deleteElementTime = 0.0;
        for (int i = 1; i <= counter; i++)
        {
            //First insert the Element
            elementTimer.Start();
            auto key1 = InsertElement(Utf8PrintfString("E%d", i));
            //m_db->SaveChanges();
            elementTimer.Stop();
            elementTime = elementTime + elementTimer.GetElapsedSeconds();

            EXPECT_TRUE(key1.GetElementId().IsValid());
            GeometricElementCPtr el = m_db->Elements().GetElement(key1.GetElementId())->ToGeometricElement();
            EXPECT_TRUE(el != nullptr);
            EXPECT_EQ(&el->GetElementHandler(), &TestElementHandler::GetHandler());

            // ECSQL to add ElementItem
            elementTimer.Start();
            EXPECT_TRUE(InsertElementItem(el->GetElementId(), L"Test"));
            //m_db->SaveChanges();
            elementTimer.Stop();
            elementItemTime = elementItemTime + elementTimer.GetElapsedSeconds();

            //Time to select a single ElementItem
            elementTimer.Start();
            EXPECT_TRUE(SelectElementItem(el->GetElementId()));
            //m_db->SaveChanges();
            elementTimer.Stop();
            selectTime = selectTime + elementTimer.GetElapsedSeconds();

            //Now Update data and measure time for Update
            elementTimer.Start();
            EXPECT_TRUE(UpdateElementItem(el->GetElementId(), L"Test - New"));
            //m_db->SaveChanges();
            elementTimer.Stop();
            updateTime = updateTime + elementTimer.GetElapsedSeconds();

            //Now delete data and measure time for Delete
            elementTimer.Start();
            EXPECT_TRUE(DeleteElementItem(el->GetElementId()));
            //m_db->SaveChanges();
            elementTimer.Stop();
            deleteTime = deleteTime + elementTimer.GetElapsedSeconds();

            //Now delete the Element
            elementTimer.Start();
            EXPECT_EQ(DgnDbStatus::Success, TestElementHandler::GetHandler().DeleteElement(*m_db, el->GetElementId()));
            //m_db->SaveChanges();
            elementTimer.Stop();
            deleteElementTime = deleteElementTime + elementTimer.GetElapsedSeconds();

        }
        //m_db->SaveChanges();

        //Write results to Db for analysis
        m_testObj.writeTodb(elementTime, "ElementCRUDPerformance,InsertElementItem_Element", "", counter);
        m_testObj.writeTodb(elementItemTime, "ElementCRUDPerformance,InsertElementItem_Item", "", counter);
        m_testObj.writeTodb(elementTime + elementItemTime, "ElementCRUDPerformance,InsertElementItem_Total", "", counter);
        m_testObj.writeTodb(selectTime, "ElementCRUDPerformance,SelectSignleElementItem", "", counter);
        m_testObj.writeTodb(updateTime, "ElementCRUDPerformance,UpdateElementItem", "", counter);
        m_testObj.writeTodb(deleteTime, "ElementCRUDPerformance,DeleteElementItem", "", counter);
        m_testObj.writeTodb(deleteElementTime, "ElementCRUDPerformance,DeleteElement", "", counter);
        m_testObj.writeTodb(deleteTime + deleteElementTime, "ElementCRUDPerformance,DeleteElementItem_Total", "", counter);

    }

}

//--------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ImportTestSchema(DgnDbR db)
    {
    Utf8CP testSchemaXml =
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
        "</ECSchema>";

    ECN::ECSchemaReadContextPtr schemaContext = ECN::ECSchemaReadContext::CreateContext();
    BeFileName searchDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(searchDir);
    searchDir.AppendToPath(L"ECSchemas").AppendToPath(L"Dgn");
    schemaContext->AddSchemaPath(searchDir.GetName ());

    ECN::ECSchemaPtr schema = nullptr;
    if (ECN::SCHEMA_READ_STATUS_Success != ECN::ECSchema::ReadFromXmlString(schema, testSchemaXml, *schemaContext))
        return ERROR;

    if (SUCCESS != db.Schemas().ImportECSchemas(schemaContext->GetCache()))
        return ERROR;

    db.ClearECDbCache();
    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
DgnModelId InsertDgnModel(DgnDbR db)
    {
    //cannot let ECDb generate the model id as DgnDb has its own sequence and model id 1 is already in use
    ECSqlStatement stmt;
    if (stmt.Prepare(db, "INSERT INTO dgn.Model3d (ECInstanceId, Name) VALUES (200, 'TestModel')") != ECSqlStatus::Success)
        return DgnModelId();

    ECInstanceKey newKey;
    if (stmt.Step(newKey) == ECSqlStepStatus::Done)
        return DgnModelId (newKey.GetECInstanceId().GetValue ());

    return DgnModelId();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
void LogTiming(int instanceCount, StopWatch& timer, Utf8CP scenario)
    {
    LOG.infov("%s> Inserting %d Element4 instances (15 properties) took %.4f seconds.", scenario, instanceCount, timer.GetElapsedSeconds());
    }

DgnCategoryId catId(BeRepositoryId(1), 123);
const int instanceCount = 20000;
Utf8CP const textVal = "bla bla";
const int64_t int64Val = 20000000LL;
const double doubleVal = -3.141516;

//--------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementTestFixture, ElementInsertInDbWithInsertUpdateApproach)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"ElementInsertPerformanceInsertUpdateApproach.idgndb", BeSQLite::Db::OPEN_ReadWrite);
    ASSERT_EQ(SUCCESS, ImportTestSchema(*m_db));      

    DgnModelId modelId = InsertDgnModel(*m_db);
    ASSERT_TRUE(modelId.IsValid());

    StopWatch timer(true);
    //Preparation
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

    //Execution
    Utf8String code;
    for (int i = 0; i < instanceCount; i++)
        {
        insertStmt->BindId(1, modelId);
        insertStmt->BindId(2, catId);
        code.Sprintf("Id-%d", i);
        insertStmt->BindText(3, code.c_str(), IECSqlBinder::MakeCopy::No);

        ECInstanceKey newKey;
        ASSERT_EQ(ECSqlStepStatus::Done, insertStmt->Step(newKey));
        insertStmt->Reset();
        insertStmt->ClearBindings();

        for (CachedECSqlStatementPtr& updateStmt : updateStmts)
            {
            updateStmt->BindText(1, textVal, IECSqlBinder::MakeCopy::No);
            updateStmt->BindInt64(2, int64Val);
            updateStmt->BindDouble(3, doubleVal);
            updateStmt->BindId(4, newKey.GetECInstanceId());
            ASSERT_EQ(ECSqlStepStatus::Done, updateStmt->Step());
            updateStmt->Reset();
            updateStmt->ClearBindings();
            }
        }

    timer.Stop();
    LogTiming(instanceCount, timer, "Insert & Update sub props");
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementTestFixture, ElementInsertInDbWithSingleInsertApproach)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"ElementInsertPerformanceSingleInsertNumberedParams.idgndb", BeSQLite::Db::OPEN_ReadWrite);
    ASSERT_EQ(SUCCESS, ImportTestSchema(*m_db));

    DgnModelId modelId = InsertDgnModel(*m_db);
    ASSERT_TRUE(modelId.IsValid());

    StopWatch timer(true);
    //Preparation
    CachedECSqlStatementPtr insertStmt = m_db->GetPreparedECSqlStatement("INSERT INTO ts.Element4 (ModelId,CategoryId,Code,"
                                                                                                "Prop1_1,Prop1_2,Prop1_3,"
                                                                                                "Prop2_1,Prop2_2,Prop2_3,"
                                                                                                "Prop3_1,Prop3_2,Prop3_3,"
                                                                                                "Prop4_1,Prop4_2,Prop4_3) "
                                                                          "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    ASSERT_TRUE(insertStmt != nullptr);

    CachedECSqlStatement& stmt = *insertStmt;
    //Execution
    Utf8String code;
    for (int i = 0; i < instanceCount; i++)
        {
        stmt.BindId(1, modelId);
        stmt.BindId(2, catId);
        code.Sprintf("Id-%d", i);
        stmt.BindText(3, code.c_str(), IECSqlBinder::MakeCopy::No);

        stmt.BindText(4, textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64(5, int64Val);
        stmt.BindDouble(6, doubleVal);

        stmt.BindText(7, textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64(8, int64Val);
        stmt.BindDouble(9, doubleVal);

        stmt.BindText(10, textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64(11, int64Val);
        stmt.BindDouble(12, doubleVal);

        stmt.BindText(13, textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64(14, int64Val);
        stmt.BindDouble(15, doubleVal);

        ASSERT_EQ(ECSqlStepStatus::Done, stmt.Step());

        stmt.Reset();
        stmt.ClearBindings();
        }

    timer.Stop();
    LogTiming(instanceCount, timer, "Single Insert (numeric parameters)");
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementTestFixture, ElementInsertInDbWithSingleInsertApproachNamedParameters)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"ElementInsertPerformanceSingleInsertNamedParams.idgndb", BeSQLite::Db::OPEN_ReadWrite);
    ASSERT_EQ(SUCCESS, ImportTestSchema(*m_db));

    DgnModelId modelId = InsertDgnModel(*m_db);
    ASSERT_TRUE(modelId.IsValid());

    StopWatch timer(true);
    //Preparation
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
    //Execution
    Utf8String code;
    for (int i = 0; i < instanceCount; i++)
        {
        stmt.BindId(stmt.GetParameterIndex("modelid"), modelId);
        stmt.BindId(stmt.GetParameterIndex("catid"), catId);
        code.Sprintf("Id-%d", i);
        stmt.BindText(stmt.GetParameterIndex("code"), code.c_str(), IECSqlBinder::MakeCopy::No);

        stmt.BindText(stmt.GetParameterIndex("p11"), textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64(stmt.GetParameterIndex("p12"), int64Val);
        stmt.BindDouble(stmt.GetParameterIndex("p13"), doubleVal);

        stmt.BindText(stmt.GetParameterIndex("p21"), textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64(stmt.GetParameterIndex("p22"), int64Val);
        stmt.BindDouble(stmt.GetParameterIndex("p23"), doubleVal);

        stmt.BindText(stmt.GetParameterIndex("p31"), textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64(stmt.GetParameterIndex("p32"), int64Val);
        stmt.BindDouble(stmt.GetParameterIndex("p33"), doubleVal);

        stmt.BindText(stmt.GetParameterIndex("p41"), textVal, IECSqlBinder::MakeCopy::No);
        stmt.BindInt64(stmt.GetParameterIndex("p42"), int64Val);
        stmt.BindDouble(stmt.GetParameterIndex("p43"), doubleVal);

        ASSERT_EQ(ECSqlStepStatus::Done, stmt.Step());

        stmt.Reset();
        stmt.ClearBindings();
        }

    timer.Stop();
    LogTiming(instanceCount, timer, "Single Insert (named parameters)");
    }