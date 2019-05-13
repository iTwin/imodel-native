/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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

    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("CRUDPerformance_SharedTable_SharedColumnsForSubClasses.ecdb"));

    ECSchemaPtr testSchema;
    ECSchema::CreateSchema(testSchema, "testSchema", "ts", 1, 0, 0);
    ASSERT_TRUE(testSchema.IsValid());

    ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext();
    readContext->AddSchemaLocater(m_ecdb.GetSchemaLocater());
    SchemaKey ecdbmapKey = SchemaKey("ECDbMap", 2, 0);
    ECSchemaPtr ecdbmapSchema = readContext->LocateSchema(ecdbmapKey, SchemaMatchType::LatestWriteCompatible);
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
    ASSERT_TRUE(customAttribute->SetValue("MapStrategy", ECValue("TablePerHierarchy")) == ECObjectsStatus::Success);
    ASSERT_TRUE(baseClass->SetCustomAttribute(*customAttribute) == ECObjectsStatus::Success);

    ca = ecdbmapSchema->GetClassCP("ShareColumns");
    ASSERT_TRUE(ca != nullptr);
    customAttribute = ca->GetDefaultStandaloneEnabler()->CreateInstance();
    ASSERT_TRUE(customAttribute != nullptr);
    ASSERT_TRUE(baseClass->SetCustomAttribute(*customAttribute) == ECObjectsStatus::Success);

    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().ImportSchemas(readContext->GetCache().GetSchemas()));

    GenerateECSqlCRUDTestStatements(*testSchema, false);
    ECSqlInsertInstances(m_ecdb, true, 1);
    m_ecdb.SaveChanges();
    m_ecdb.CloseDb();

    BeFileName seedFilePath = BuildECDbPath("CRUDPerformance_SharedTable_SharedColumnsForSubClasses.ecdb");
    ASSERT_EQ(DbResult::BE_SQLITE_OK, CloneECDb("insertTestDb.ecdb", seedFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));
    ECSqlInsertInstances(m_ecdb, true, 1000001);
    ASSERT_GE(m_insertTime, 0.0) << "ECSQL Insert test failed";
    m_ecdb.CloseDb();

    ASSERT_EQ(DbResult::BE_SQLITE_OK, CloneECDb("readTestDb.ecdb", seedFilePath, ECDb::OpenParams(Db::OpenMode::Readonly)));
    ECSqlReadInstances(m_ecdb, false);
    ASSERT_GE(m_selectTime, 0.0) << "ECSQL SELECT test failed";
    m_ecdb.CloseDb();

    ASSERT_EQ(DbResult::BE_SQLITE_OK, CloneECDb("updateTestDb.ecdb", seedFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));
    ECSqlUpdateInstances(m_ecdb, true);
    ASSERT_GE(m_updateTime, 0.0) << "ECSQL UPDATE test failed";
    m_ecdb.CloseDb();

    ASSERT_EQ(DbResult::BE_SQLITE_OK, CloneECDb("DeleteTestDb.ecdb", seedFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));
    ECSqlDeleteInstances(m_ecdb);
    ASSERT_GE(m_deleteTime, 0.0) << "ECSQL DELETE test failed";
    m_ecdb.CloseDb();

    LOG.infov("ECSQL CRUD against a %d layered ECClass Hierarchy with %d properties each.", hierarchyLevel, m_propertiesPerClass);


    LOGTODB(TEST_DETAILS, m_insertTime, (int) m_instancesPerClass, "Insert time");
    LOGTODB(TEST_DETAILS, m_selectTime, (int) m_instancesPerClass, "Select time");
    LOGTODB(TEST_DETAILS, m_updateTime, (int) m_instancesPerClass, "Update time");
    LOGTODB(TEST_DETAILS, m_deleteTime, (int) m_instancesPerClass, "Delete time");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECDbMapCATests, CRUDPerformance_SharedTableForSubClasses)
    {
    m_instancesPerClass = 1000;
    m_propertiesPerClass = 5;
    size_t hierarchyLevel = 7;
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("CRUDPerformance_SharedTableForSubClasses.ecdb"));

    ECSchemaPtr testSchema;
    ECSchema::CreateSchema(testSchema, "testSchema", "ts", 1, 0, 0);
    ASSERT_TRUE(testSchema.IsValid());

    ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext();
    readContext->AddSchemaLocater(m_ecdb.GetSchemaLocater());
    SchemaKey ecdbmapKey = SchemaKey("ECDbMap", 2, 0);
    ECSchemaPtr ecdbmapSchema = readContext->LocateSchema(ecdbmapKey, SchemaMatchType::LatestWriteCompatible);
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
    ASSERT_TRUE(customAttribute->SetValue("MapStrategy", ECValue("TablePerHierarchy")) == ECObjectsStatus::Success);
    ASSERT_TRUE(baseClass->SetCustomAttribute(*customAttribute) == ECObjectsStatus::Success);

    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().ImportSchemas(readContext->GetCache().GetSchemas()));

    GenerateECSqlCRUDTestStatements(*testSchema, false);
    ECSqlInsertInstances(m_ecdb, true, 1);
    m_ecdb.SaveChanges();
    m_ecdb.CloseDb();

    BeFileName seedFilePath = BuildECDbPath("CRUDPerformance_SharedTableForSubClasses.ecdb");
    ASSERT_EQ(DbResult::BE_SQLITE_OK, CloneECDb("insertTestDb.ecdb", seedFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));
    ECSqlInsertInstances(m_ecdb, true, 1000001);
    ASSERT_GE(m_insertTime, 0.0) << "ECSQL Insert test failed";
    m_ecdb.CloseDb();

    ASSERT_EQ(DbResult::BE_SQLITE_OK, CloneECDb("readTestDb.ecdb", seedFilePath, ECDb::OpenParams(Db::OpenMode::Readonly)));
    ECSqlReadInstances(m_ecdb, false);
    ASSERT_GE(m_selectTime, 0.0) << "ECSQL SELECT test failed";
    m_ecdb.CloseDb();

    ASSERT_EQ(DbResult::BE_SQLITE_OK, CloneECDb("updateTestDb.ecdb", seedFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));
    ECSqlUpdateInstances(m_ecdb, true);
    ASSERT_GE(m_updateTime, 0.0) << "ECSQL UPDATE test failed";
    m_ecdb.CloseDb();

    ASSERT_EQ(DbResult::BE_SQLITE_OK, CloneECDb("DeleteTestDb.ecdb", seedFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));
    ECSqlDeleteInstances(m_ecdb);
    ASSERT_GE(m_deleteTime, 0.0) << "ECSQL DELETE test failed";
    m_ecdb.CloseDb();

    LOG.infov("ECSQL CRUD against a %d layered ECClass Hierarchy with %d properties each.", hierarchyLevel, m_propertiesPerClass);

    LOGTODB(TEST_DETAILS, m_insertTime, (int) m_instancesPerClass, "Insert time");
    LOGTODB(TEST_DETAILS, m_selectTime, (int) m_instancesPerClass, "Select time");
    LOGTODB(TEST_DETAILS, m_updateTime, (int) m_instancesPerClass, "Update time");
    LOGTODB(TEST_DETAILS, m_deleteTime, (int) m_instancesPerClass, "Delete time");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECDbMapCATests, CRUDPerformance_DefaultClasses)
    {
    m_instancesPerClass = 1000;
    m_propertiesPerClass = 5;
    size_t hierarchyLevel = 7;
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("CRUDPerformance_DefaultClasses.ecdb"));

    ECSchemaPtr testSchema;
    ECSchema::CreateSchema(testSchema, "testSchema", "ts", 1, 0, 0);
    ASSERT_TRUE(testSchema.IsValid());

    ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext();
    readContext->AddSchema(*testSchema);

    ECEntityClassP baseClass;
    ASSERT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(baseClass, "BaseClass"));
    CreatePrimitiveProperties(*baseClass);

    //Recursively Create Derived Classes of Provided Base Class (2 Derived Classes per Base Class)
    CreateClassHierarchy(*testSchema, hierarchyLevel, *baseClass);

    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().ImportSchemas(readContext->GetCache().GetSchemas()));

    GenerateECSqlCRUDTestStatements(*testSchema, false);
    ECSqlInsertInstances(m_ecdb, true, 1);
    m_ecdb.SaveChanges();
    m_ecdb.CloseDb();

    BeFileName seedFilePath = BuildECDbPath("CRUDPerformance_DefaultClasses.ecdb");
    ASSERT_EQ(DbResult::BE_SQLITE_OK, CloneECDb("insertTestDb.ecdb", seedFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));
    ECSqlInsertInstances(m_ecdb, true, 1000001);
    ASSERT_GE(m_insertTime, 0.0) << "ECSQL Insert test failed";
    m_ecdb.CloseDb();

    ASSERT_EQ(DbResult::BE_SQLITE_OK, CloneECDb("readTestDb.ecdb", seedFilePath, ECDb::OpenParams(Db::OpenMode::Readonly)));
    ECSqlReadInstances(m_ecdb, false);
    ASSERT_GE(m_selectTime, 0.0) << "ECSQL SELECT test failed";
    m_ecdb.CloseDb();

    ASSERT_EQ(DbResult::BE_SQLITE_OK, CloneECDb("updateTestDb.ecdb", seedFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));
    ECSqlUpdateInstances(m_ecdb, true);
    ASSERT_GE(m_updateTime, 0.0) << "ECSQL UPDATE test failed";
    m_ecdb.CloseDb();

    ASSERT_EQ(DbResult::BE_SQLITE_OK, CloneECDb("DeleteTestDb.ecdb", seedFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));
    ECSqlDeleteInstances(m_ecdb);
    ASSERT_GE(m_deleteTime, 0.0) << "ECSQL DELETE test failed";
    m_ecdb.CloseDb();

    LOG.infov("ECSQL CRUD against a %d layered ECClass Hierarchy with %d properties each.", hierarchyLevel, m_propertiesPerClass);

    LOGTODB(TEST_DETAILS, m_insertTime, (int) m_instancesPerClass, "Insert time");
    LOGTODB(TEST_DETAILS, m_selectTime, (int) m_instancesPerClass, "Select time");
    LOGTODB(TEST_DETAILS, m_updateTime, (int) m_instancesPerClass, "Update time");
    LOGTODB(TEST_DETAILS, m_deleteTime, (int) m_instancesPerClass, "Delete time");
    }

END_ECDBUNITTESTS_NAMESPACE