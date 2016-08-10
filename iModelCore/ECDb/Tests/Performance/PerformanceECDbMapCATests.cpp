/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceECDbMapCATests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceCRUDTestsHelper.h"
BEGIN_ECDBUNITTESTS_NAMESPACE

struct PerformanceECDbMapCATests : public PerformanceCRUDTestsHelper
    {};

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECDbMapCATests, CRUDPerformance_SharedTable_SharedColumnsForSubClasses)
    {
    m_instancesPerClass = 1000;
    m_propertiesPerClass = 5;
    size_t hierarchyLevel = 7;

    ECDbR ecdb = SetupECDb("CRUDPerformance_SharedTable_SharedColumnsForSubClasses.ecdb");

    ECSchemaPtr testSchema;
    ECSchema::CreateSchema(testSchema, "testSchema", 1, 0);
    ASSERT_TRUE(testSchema.IsValid());
    testSchema->SetAlias("ts");

    ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext();
    readContext->AddSchemaLocater(ecdb.GetSchemaLocater());
    SchemaKey ecdbmapKey = SchemaKey("ECDbMap", 2, 0);
    ECSchemaPtr ecdbmapSchema = readContext->LocateSchema(ecdbmapKey, SchemaMatchType::LatestCompatible);
    ASSERT_TRUE(ecdbmapSchema.IsValid());
    readContext->AddSchema(*testSchema);
    testSchema->AddReferencedSchema(*ecdbmapSchema);

    ECEntityClassP baseClass;
    ASSERT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(baseClass, "BaseClass"));
    CreatePrimitiveProperties(*baseClass);

    //Recursively Create Derived Classes of Provided Base Class (2 Derived Classes per Base Class)
    CreateClassHierarchy(*testSchema, hierarchyLevel, *baseClass);

    ECClassCP ca = ecdbmapSchema->GetClassCP("ClassMap");
    EXPECT_TRUE(ca != nullptr);
    StandaloneECInstancePtr customAttribute = ca->GetDefaultStandaloneEnabler()->CreateInstance();
    EXPECT_TRUE(customAttribute != nullptr);
    ASSERT_TRUE(customAttribute->SetValue("MapStrategy.Strategy", ECValue("TablePerHierarchy")) == ECObjectsStatus::Success);
    ASSERT_TRUE(customAttribute->SetValue("MapStrategy.Options", ECValue("SharedColumnsForSubclasses")) == ECObjectsStatus::Success);
    ASSERT_TRUE(baseClass->SetCustomAttribute(*customAttribute) == ECObjectsStatus::Success);

    ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportECSchemas(readContext->GetCache()));

    GenerateECSqlCRUDTestStatements(*testSchema, false);
    ECSqlInsertInstances(ecdb, true, 1);
    ecdb.SaveChanges();
    ecdb.CloseDb();

    BeFileName seedFilePath = ECDbTestUtility::BuildECDbPath("CRUDPerformance_SharedTable_SharedColumnsForSubClasses.ecdb");
    ASSERT_EQ(DbResult::BE_SQLITE_OK, CloneECDb(ecdb, "insertTestDb.ecdb", seedFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));
    ECSqlInsertInstances(ecdb, true, 1000001);
    ASSERT_GE(m_insertTime, 0.0) << "ECSQL Insert test failed";
    ecdb.CloseDb();

    ASSERT_EQ(DbResult::BE_SQLITE_OK, CloneECDb(ecdb, "readTestDb.ecdb", seedFilePath, ECDb::OpenParams(Db::OpenMode::Readonly)));
    ECSqlReadInstances(ecdb, false);
    ASSERT_GE(m_selectTime, 0.0) << "ECSQL SELECT test failed";
    ecdb.CloseDb();

    ASSERT_EQ(DbResult::BE_SQLITE_OK, CloneECDb(ecdb, "updateTestDb.ecdb", seedFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));
    ECSqlUpdateInstances(ecdb, true);
    ASSERT_GE(m_updateTime, 0.0) << "ECSQL UPDATE test failed";
    ecdb.CloseDb();

    ASSERT_EQ(DbResult::BE_SQLITE_OK, CloneECDb(ecdb, "DeleteTestDb.ecdb", seedFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));
    ECSqlDeleteInstances(ecdb);
    ASSERT_GE(m_deleteTime, 0.0) << "ECSQL DELETE test failed";
    ecdb.CloseDb();

    LOG.infov("ECSQL CRUD against a %d layered ECClass Hierachy with %d properties each.", hierarchyLevel, m_propertiesPerClass);


    LOGTODB(TEST_DETAILS, m_insertTime, "Insert time", (int) m_instancesPerClass);
    LOGTODB(TEST_DETAILS, m_selectTime, "Select time", (int) m_instancesPerClass);
    LOGTODB(TEST_DETAILS, m_updateTime, "Update time", (int) m_instancesPerClass);
    LOGTODB(TEST_DETAILS, m_deleteTime, "Delete time", (int) m_instancesPerClass);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECDbMapCATests, CRUDPerformance_SharedTableForSubClasses)
    {
    m_instancesPerClass = 1000;
    m_propertiesPerClass = 5;
    size_t hierarchyLevel = 7;
    ECDbR ecdb = SetupECDb("CRUDPerformance_SharedTableForSubClasses.ecdb");

    ECSchemaPtr testSchema;
    ECSchema::CreateSchema(testSchema, "testSchema", 1, 0);
    ASSERT_TRUE(testSchema.IsValid());
    testSchema->SetAlias("ts");

    ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext();
    readContext->AddSchemaLocater(ecdb.GetSchemaLocater());
    SchemaKey ecdbmapKey = SchemaKey("ECDbMap", 2, 0);
    ECSchemaPtr ecdbmapSchema = readContext->LocateSchema(ecdbmapKey, SchemaMatchType::LatestCompatible);
    ASSERT_TRUE(ecdbmapSchema.IsValid());
    readContext->AddSchema(*testSchema);
    testSchema->AddReferencedSchema(*ecdbmapSchema);

    ECEntityClassP baseClass;
    ASSERT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(baseClass, "BaseClass"));
    CreatePrimitiveProperties(*baseClass);

    //Recursively Create Derived Classes of Provided Base Class (2 Derived Classes per Base Class)
    CreateClassHierarchy(*testSchema, hierarchyLevel, *baseClass);

    ECClassCP ca = ecdbmapSchema->GetClassCP("ClassMap");
    EXPECT_TRUE(ca != nullptr);
    StandaloneECInstancePtr customAttribute = ca->GetDefaultStandaloneEnabler()->CreateInstance();
    EXPECT_TRUE(customAttribute != nullptr);
    ASSERT_TRUE(customAttribute->SetValue("MapStrategy.Strategy", ECValue("TablePerHierarchy")) == ECObjectsStatus::Success);
    ASSERT_TRUE(baseClass->SetCustomAttribute(*customAttribute) == ECObjectsStatus::Success);

    ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportECSchemas(readContext->GetCache()));

    GenerateECSqlCRUDTestStatements(*testSchema, false);
    ECSqlInsertInstances(ecdb, true, 1);
    ecdb.SaveChanges();
    ecdb.CloseDb();

    BeFileName seedFilePath = ECDbTestUtility::BuildECDbPath("CRUDPerformance_SharedTableForSubClasses.ecdb");
    ASSERT_EQ(DbResult::BE_SQLITE_OK, CloneECDb(ecdb, "insertTestDb.ecdb", seedFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));
    ECSqlInsertInstances(ecdb, true, 1000001);
    ASSERT_GE(m_insertTime, 0.0) << "ECSQL Insert test failed";
    ecdb.CloseDb();

    ASSERT_EQ(DbResult::BE_SQLITE_OK, CloneECDb(ecdb, "readTestDb.ecdb", seedFilePath, ECDb::OpenParams(Db::OpenMode::Readonly)));
    ECSqlReadInstances(ecdb, false);
    ASSERT_GE(m_selectTime, 0.0) << "ECSQL SELECT test failed";
    ecdb.CloseDb();

    ASSERT_EQ(DbResult::BE_SQLITE_OK, CloneECDb(ecdb, "updateTestDb.ecdb", seedFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));
    ECSqlUpdateInstances(ecdb, true);
    ASSERT_GE(m_updateTime, 0.0) << "ECSQL UPDATE test failed";
    ecdb.CloseDb();

    ASSERT_EQ(DbResult::BE_SQLITE_OK, CloneECDb(ecdb, "DeleteTestDb.ecdb", seedFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));
    ECSqlDeleteInstances(ecdb);
    ASSERT_GE(m_deleteTime, 0.0) << "ECSQL DELETE test failed";
    ecdb.CloseDb();

    LOG.infov("ECSQL CRUD against a %d layered ECClass Hierachy with %d properties each.", hierarchyLevel, m_propertiesPerClass);

    LOGTODB(TEST_DETAILS, m_insertTime, "Insert time", (int) m_instancesPerClass);
    LOGTODB(TEST_DETAILS, m_selectTime, "Select time", (int) m_instancesPerClass);
    LOGTODB(TEST_DETAILS, m_updateTime, "Update time", (int) m_instancesPerClass);
    LOGTODB(TEST_DETAILS, m_deleteTime, "Delete time", (int) m_instancesPerClass);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECDbMapCATests, CRUDPerformance_DefaultClasses)
    {
    m_instancesPerClass = 1000;
    m_propertiesPerClass = 5;
    size_t hierarchyLevel = 7;
    ECDbR ecdb = SetupECDb("CRUDPerformance_DefaultClasses.ecdb");

    ECSchemaPtr testSchema;
    ECSchema::CreateSchema(testSchema, "testSchema", 1, 0);
    ASSERT_TRUE(testSchema.IsValid());
    testSchema->SetAlias("ts");

    ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext();
    readContext->AddSchema(*testSchema);

    ECEntityClassP baseClass;
    ASSERT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(baseClass, "BaseClass"));
    CreatePrimitiveProperties(*baseClass);

    //Recursively Create Derived Classes of Provided Base Class (2 Derived Classes per Base Class)
    CreateClassHierarchy(*testSchema, hierarchyLevel, *baseClass);

    ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportECSchemas(readContext->GetCache()));

    GenerateECSqlCRUDTestStatements(*testSchema, false);
    ECSqlInsertInstances(ecdb, true, 1);
    ecdb.SaveChanges();
    ecdb.CloseDb();

    BeFileName seedFilePath = ECDbTestUtility::BuildECDbPath("CRUDPerformance_DefaultClasses.ecdb");
    ASSERT_EQ(DbResult::BE_SQLITE_OK, CloneECDb(ecdb, "insertTestDb.ecdb", seedFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));
    ECSqlInsertInstances(ecdb, true, 1000001);
    ASSERT_GE(m_insertTime, 0.0) << "ECSQL Insert test failed";
    ecdb.CloseDb();

    ASSERT_EQ(DbResult::BE_SQLITE_OK, CloneECDb(ecdb, "readTestDb.ecdb", seedFilePath, ECDb::OpenParams(Db::OpenMode::Readonly)));
    ECSqlReadInstances(ecdb, false);
    ASSERT_GE(m_selectTime, 0.0) << "ECSQL SELECT test failed";
    ecdb.CloseDb();

    ASSERT_EQ(DbResult::BE_SQLITE_OK, CloneECDb(ecdb, "updateTestDb.ecdb", seedFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));
    ECSqlUpdateInstances(ecdb, true);
    ASSERT_GE(m_updateTime, 0.0) << "ECSQL UPDATE test failed";
    ecdb.CloseDb();

    ASSERT_EQ(DbResult::BE_SQLITE_OK, CloneECDb(ecdb, "DeleteTestDb.ecdb", seedFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));
    ECSqlDeleteInstances(ecdb);
    ASSERT_GE(m_deleteTime, 0.0) << "ECSQL DELETE test failed";
    ecdb.CloseDb();

    LOG.infov("ECSQL CRUD against a %d layered ECClass Hierachy with %d properties each.", hierarchyLevel, m_propertiesPerClass);

    LOGTODB(TEST_DETAILS, m_insertTime, "Insert time", (int) m_instancesPerClass);
    LOGTODB(TEST_DETAILS, m_selectTime, "Select time", (int) m_instancesPerClass);
    LOGTODB(TEST_DETAILS, m_updateTime, "Update time", (int) m_instancesPerClass);
    LOGTODB(TEST_DETAILS, m_deleteTime, "Delete time", (int) m_instancesPerClass);
    }

END_ECDBUNITTESTS_NAMESPACE