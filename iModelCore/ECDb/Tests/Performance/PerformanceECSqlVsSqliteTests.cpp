/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceCRUDTestsHelper.h"
BEGIN_ECDBUNITTESTS_NAMESPACE

struct PerformanceECSqlVsSqliteTests : public PerformanceCRUDTestsHelper
    {};

//---------------------------------------------------------------------------------------
// Scenario: No of Instances 1000000, No of Properties 4:128
//           One time Prepare and then Loop through Binding, Step, ReSet and ClearBinding
//           Bind ECInstanceId instead of auto Generating it
//
// @bsiMethod                                      Muhammad Hassan                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECSqlVsSqliteTests, SqlVsECSqlInsertPerformance_BindPropertyValues)
    {
    for (size_t i = 2; i <= 7; i++)
        {
        m_propertiesPerClass = (size_t) pow(2, i);
        ECDb ecdb;
        Utf8String dbName;
        dbName.Sprintf("ecsqlInsertperformance_%d.ecdb", m_propertiesPerClass);
        SetUpTestECDb(ecdb, dbName);

        ECSchemaPtr testSchema;
        ASSERT_TRUE(ECObjectsStatus::Success == ecdb.Schemas().GetSchema("testSchema", true)->CopySchema(testSchema));
        ASSERT_TRUE(testSchema != nullptr);
        GenerateECSqlCRUDTestStatements(*testSchema, false);

        //Insert Performance using ECSql statements.
        m_instancesPerClass = 50000;
        ECSqlInsertInstances(ecdb, true, 1000001);
        ASSERT_GE(m_insertTime, 0.0) << "ECSQL Insert test failed";

        Utf8String testDescription;
        testDescription.Sprintf("ECSQL INSERT against ECClass with %d properties.", m_propertiesPerClass);
        LOGTODB(TEST_DETAILS, m_insertTime, (int) m_instancesPerClass, testDescription.c_str());

        ecdb.CloseDb();

        //Insert Performance using Sql statements.
        m_instancesPerClass = 50000;
        m_insertTime = 0.0;
        dbName.Sprintf("sqliteInsertperformance_%d.ecdb", m_propertiesPerClass);
        SetUpTestECDb(ecdb, dbName);

        ECClassCP baseClass = nullptr;
        baseClass = ecdb.Schemas().GetClass("testSchema", "BaseClass");
        GenerateSqlCRUDTestStatements(baseClass, false);

        SqlInsertInstances(ecdb, const_cast<ECClassR> (*baseClass), true, 1000001);
        ASSERT_GE(m_insertTime, 0.0) << "SQL Insert test failed";

        testDescription.Sprintf("SQL INSERT against table of ECClass with %d properties.", m_propertiesPerClass);
        LOGTODB(TEST_DETAILS, m_insertTime, (int) m_instancesPerClass, testDescription.c_str());
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECSqlVsSqliteTests, SqlVsECSqlInsertPerformance_DoNotBindPropertyValues)
    {
    for (size_t i = 2; i <= 7; i++)
        {
        m_propertiesPerClass = (size_t) pow(2, i);
        ECDb ecdb;
        Utf8String dbName;
        dbName.Sprintf("ecsqlInsertperformance_%d.ecdb", m_propertiesPerClass);
        SetUpTestECDb(ecdb, dbName);

        ECSchemaPtr testSchema;
        ASSERT_TRUE(ECObjectsStatus::Success == ecdb.Schemas().GetSchema("testSchema", true)->CopySchema(testSchema));
        ASSERT_TRUE(testSchema != nullptr);
        GenerateECSqlCRUDTestStatements(*testSchema, false);

        //Insert Performance using ECSql statements.
        m_instancesPerClass = 50000;
        ECSqlInsertInstances(ecdb, true, 1000001);
        ASSERT_GE(m_insertTime, 0.0) << "ECSQL Insert test failed";

        Utf8String testDescription;
        testDescription.Sprintf("ECSQL INSERT against ECClass with %d properties.", m_propertiesPerClass);
        LOGTODB(TEST_DETAILS, m_insertTime, (int) m_instancesPerClass, testDescription.c_str());

        ecdb.CloseDb();

        //Insert Performance using Sql statements.
        m_instancesPerClass = 50000;
        m_insertTime = 0.0;
        dbName.Sprintf("sqliteInsertperformance_%d.ecdb", m_propertiesPerClass);
        SetUpTestECDb(ecdb, dbName);

        ECClassCP baseClass = nullptr;
        baseClass = ecdb.Schemas().GetClass("testSchema", "BaseClass");
        GenerateSqlCRUDTestStatements(baseClass, false);

        SqlInsertInstances(ecdb, const_cast<ECClassR> (*baseClass), true, 1000001);
        ASSERT_GE(m_insertTime, 0.0) << "SQL Insert test failed";

        testDescription.Sprintf("SQL INSERT against table of ECClass with %d properties.", m_propertiesPerClass);
        LOGTODB(TEST_DETAILS, m_insertTime, (int) m_instancesPerClass, testDescription.c_str());
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECSqlVsSqliteTests, SqlVsECSqlReadPerformance_IterateResultSet)
    {
    m_instancesPerClass = 1000000;

    for (size_t i = 2; i <= 7; i++)
        {
        m_propertiesPerClass = (size_t) pow(2, i);
        ECDb ecdb;
        Utf8String dbName;
        dbName.Sprintf("ecsqlSelectperformance_%d.ecdb", m_propertiesPerClass);
        SetUpTestECDb(ecdb, dbName);

        ECSchemaPtr testSchema;
        ASSERT_TRUE(ECObjectsStatus::Success == ecdb.Schemas().GetSchema("testSchema", true)->CopySchema(testSchema));
        ASSERT_TRUE(testSchema != nullptr);

        //Read Performance using ECSql
        GenerateECSqlCRUDTestStatements(*testSchema, false);

        ECSqlReadInstances(ecdb, true);
        ASSERT_GE(m_selectTime, 0.0) << "ECSQL SELECT test failed";

        Utf8String testDescription;
        testDescription.Sprintf("ECSQL SELECT Prop1 Prop2 Prop3... FROM ONLY against ECClass with %d properties. - with result set iteration.", m_propertiesPerClass);
        LOGTODB(TEST_DETAILS, m_selectTime, (int) m_instancesPerClass, testDescription.c_str());
        ecdb.CloseDb();

        dbName.Sprintf("sqliteSelectperformance_%d.ecdb", m_propertiesPerClass);
        SetUpTestECDb(ecdb, dbName);

        ECClassCP baseClass = nullptr;
        baseClass = ecdb.Schemas().GetClass("testSchema", "BaseClass");

        //READ Performance using Sql statement.
        GenerateSqlCRUDTestStatements(baseClass, false);
        SqlReadInstances(ecdb, const_cast<ECClassR> (*baseClass), true);
        ASSERT_GE(m_selectTime, 0.0) << "SQL SELECT test failed";

        testDescription.Sprintf("SQL SELECT Prop1 Prop2 Prop3... FROM table of ECClass with %d properties. - with result set iteration.", m_propertiesPerClass);
        LOGTODB(TEST_DETAILS, m_selectTime, (int) m_instancesPerClass, testDescription.c_str());
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECSqlVsSqliteTests, SqlVsECSqlUpdatePerformance_DoNotBindPropertyValues)
    {
    m_instancesPerClass = 1000000;

    for (size_t i = 2; i <= 7; i++)
        {
        m_propertiesPerClass = (size_t) pow(2, i);
        ECDb ecdb;
        Utf8String dbName;
        dbName.Sprintf("ecsqlSelectperformance_%d.ecdb", m_propertiesPerClass);
        SetUpTestECDb(ecdb, dbName);

        ECSchemaPtr testSchema;
        ASSERT_TRUE(ECObjectsStatus::Success == ecdb.Schemas().GetSchema("testSchema", true)->CopySchema(testSchema));
        ASSERT_TRUE(testSchema != nullptr);

        //UPDATE Performance using ECSql statement.
        GenerateECSqlCRUDTestStatements(*testSchema, true);

        ECSqlUpdateInstances(ecdb, false);
        ASSERT_GE(m_updateTime, 0.0) << "ECSQL UPDATE test failed";

        Utf8String testDescription;
        testDescription.Sprintf("ECSQL UPDATE ONLY against ECClass with %d properties.", m_propertiesPerClass);
        LOGTODB(TEST_DETAILS, m_updateTime, (int) m_instancesPerClass, testDescription.c_str());
        ecdb.CloseDb();

        dbName.Sprintf("sqliteSelectperformance_%d.ecdb", m_propertiesPerClass);
        SetUpTestECDb(ecdb, dbName);

        ECClassCP baseClass = nullptr;
        baseClass = ecdb.Schemas().GetClass("testSchema", "BaseClass");

        //UPDATE Performance using Sql statement.
        GenerateSqlCRUDTestStatements(baseClass, true);

        SqlUpdateInstances(ecdb, const_cast<ECClassR> (*baseClass), false);
        ASSERT_GE(m_updateTime, 0.0) << "SQL UPDATE test failed";

        testDescription.Sprintf("SQL UPDATE against table of ECClass with %d properties.", m_propertiesPerClass);
        LOGTODB(TEST_DETAILS, m_updateTime, (int) m_instancesPerClass, testDescription.c_str());
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECSqlVsSqliteTests, SqlVsECSqlUpdatePerformance_BindPropertyValues)
    {
    m_instancesPerClass = 1000000;

    for (size_t i = 2; i <= 7; i++)
        {
        m_propertiesPerClass = (size_t) pow(2, i);
        ECDb ecdb;
        Utf8String dbName;
        dbName.Sprintf("ecsqlUpdateperformance_%d.ecdb", m_propertiesPerClass);
        SetUpTestECDb(ecdb, dbName);

        ECSchemaPtr testSchema;
        ASSERT_TRUE(ECObjectsStatus::Success == ecdb.Schemas().GetSchema("testSchema", true)->CopySchema(testSchema));
        ASSERT_TRUE(testSchema != nullptr);

        //UPDATE Performance using ECSql statement.
        GenerateECSqlCRUDTestStatements(*testSchema, false);

        ECSqlUpdateInstances(ecdb, true);
        ASSERT_GE(m_updateTime, 0.0) << "ECSQL UPDATE test failed";

        Utf8String testDescription;
        testDescription.Sprintf("ECSQL UPDATE ONLY against ECClass with %d properties.", m_propertiesPerClass);
        LOGTODB(TEST_DETAILS, m_updateTime, (int) m_instancesPerClass, testDescription.c_str());
        ecdb.CloseDb();

        dbName.Sprintf("sqliteUpdateperformance_%d.ecdb", m_propertiesPerClass);
        SetUpTestECDb(ecdb, dbName);

        ECClassCP baseClass = nullptr;
        baseClass = ecdb.Schemas().GetClass("testSchema", "BaseClass");

        //UPDATE Performance using Sql statement.
        GenerateSqlCRUDTestStatements(baseClass, false);

        SqlUpdateInstances(ecdb, const_cast<ECClassR> (*baseClass), true);
        ASSERT_GE(m_updateTime, 0.0) << "SQL UPDATE test failed";

        testDescription.Sprintf("SQL UPDATE against table of ECClass with %d properties.", m_propertiesPerClass);
        LOGTODB(TEST_DETAILS, m_updateTime, (int) m_instancesPerClass, testDescription.c_str());
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECSqlVsSqliteTests, SqlVsECSqlDeletePerformance)
    {
    m_instancesPerClass = 1000000;

    for (size_t i = 2; i <= 7; i++)
        {
        m_propertiesPerClass = (size_t) pow(2, i);
        ECDb ecdb;
        Utf8String dbName;
        dbName.Sprintf("ecsqlDeleteperformance_%d.ecdb", m_propertiesPerClass);
        SetUpTestECDb(ecdb, dbName);

        ECSchemaPtr testSchema;
        ASSERT_TRUE(ECObjectsStatus::Success == ecdb.Schemas().GetSchema("testSchema", true)->CopySchema(testSchema));
        ASSERT_TRUE(testSchema != nullptr);

        //DELETE Performance using ECSql statement.
        GenerateECSqlCRUDTestStatements(*testSchema, true);

        ECSqlDeleteInstances(ecdb);
        ASSERT_GE(m_deleteTime, 0.0) << "ECSQL DELETE test failed";

        Utf8String testDescription;
        testDescription.Sprintf("ECSQL DELETE FROM ONLY against ECClass with %d properties.", m_propertiesPerClass);
        LOGTODB(TEST_DETAILS, m_deleteTime, (int) m_instancesPerClass, testDescription.c_str());
        ecdb.CloseDb();

        dbName.Sprintf("sqliteDeleteperformance_%d.ecdb", m_propertiesPerClass);
        SetUpTestECDb(ecdb, dbName);

        ECClassCP baseClass = nullptr;
        baseClass = ecdb.Schemas().GetClass("testSchema", "BaseClass");

        //DELETE Performance using Sql statement.
        GenerateSqlCRUDTestStatements(baseClass, true);
        SqlDeleteInstances(ecdb, const_cast<ECClassR> (*baseClass));
        ASSERT_GE(m_deleteTime, 0.0) << "SQL DELETE test failed";

        testDescription.Sprintf("SQL DELETE against table ECClass with %d properties.", m_propertiesPerClass);
        LOGTODB(TEST_DETAILS, m_deleteTime, (int) m_instancesPerClass, testDescription.c_str());
        }
    }

END_ECDBUNITTESTS_NAMESPACE