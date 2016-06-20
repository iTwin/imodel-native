/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceECDbReferentialIntegrityTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//**********************************************************************************
// Reproduces delete with referential integrity performance issue reported by Vincas
//**********************************************************************************
void RelateInstances(ECDb& ecdb, ECClassCP sourceClass, ECInstanceId sourceECInstanceId, ECClassCP targetClass, ECInstanceId targetECInstanceId, ECRelationshipClassCP relClass)
    {
    StandaloneECRelationshipEnablerPtr relationshipEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*relClass);
    StandaloneECRelationshipInstancePtr relationshipInstance = relationshipEnabler->CreateRelationshipInstance();

    IECInstancePtr sourceInstance = sourceClass->GetDefaultStandaloneEnabler()->CreateInstance();
    IECInstancePtr targetInstance = targetClass->GetDefaultStandaloneEnabler()->CreateInstance();

    ECDbTestUtility::SetECInstanceId(*sourceInstance, sourceECInstanceId);
    ECDbTestUtility::SetECInstanceId(*targetInstance, targetECInstanceId);
    relationshipInstance->SetSource(sourceInstance.get());
    relationshipInstance->SetTarget(targetInstance.get());

    ECInstanceInserter inserter(ecdb, *relClass);
    ASSERT_TRUE(inserter.IsValid());
    ECInstanceKey ecInstanceKey;
    BentleyStatus insertStatus = inserter.Insert(ecInstanceKey, *relationshipInstance.get());
    ASSERT_EQ(SUCCESS, insertStatus);
    }

ECInstanceKey InsertInstance(ECDbR ecdb, ECClassCP ecClass)
    {
    JsonInserter inserter(ecdb, *ecClass);
    Json::Value instance(Json::objectValue);
    ECInstanceKey instanceKey;
    inserter.Insert(instanceKey, instance);
    EXPECT_TRUE(instanceKey.IsValid());
    return instanceKey;
    }

void CreateDeleteReferentialIntegrityTestDb(BeFileNameR testDbPath)
    {
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    BeFileName assetsDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsDir);
    ECDb::Initialize(tempDir, &assetsDir);

    /* Create a new empty db */
    BeFileName outputDir;
    BeTest::GetHost().GetOutputRoot(outputDir);
    testDbPath = BeFileName(nullptr, outputDir.GetName(), L"fieldengineerdeletereferentialintegrity.ecdb", nullptr);
    if (BeFileName::DoesPathExist(testDbPath.GetName()))
        {
        // Delete any previously created file
        BeFileNameStatus fileDeleteStatus = BeFileName::BeDeleteFile(testDbPath.GetName());
        ASSERT_TRUE(fileDeleteStatus == BeFileNameStatus::Success);
        }

    ECDb ecdb;
    DbResult stat = ecdb.CreateNewDb(testDbPath.GetNameUtf8().c_str());
    ASSERT_TRUE(stat == BE_SQLITE_OK);

    // Test schema
    Utf8CP schemaXml =
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"TS\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"

        "  <ECEntityClass typeName=\"TestClass\" >"
        "    <ECProperty propertyName=\"TestProperty\" typeName=\"string\" />"
        "  </ECEntityClass>"

        "  <ECRelationshipClass typeName = \"TestClassRelationship\" strength = \"holding\" strengthDirection = \"forward\"  modifier='Sealed'>"
        "    <Source cardinality = \"(0, 1)\" polymorphic = \"True\">"
        "      <Class class = \"TestClass\" />"
        "    </Source>"
        "    <Target cardinality = \"(0, N)\" polymorphic = \"True\">"
        "      <Class class = \"TestClass\" />"
        "    </Target>"
        "  </ECRelationshipClass>"

        // Classes below reduces performance significantly
        "  <ECEntityClass typeName=\"CachedInfoClass\" >"
        "    <ECProperty propertyName=\"TestProperty\" typeName=\"string\" />"
        "  </ECEntityClass>"

        "  <ECRelationshipClass typeName = \"CachedInfoClassRelationship\" strength = \"holding\" strengthDirection = \"backward\"  modifier='Sealed'>"
        "    <Source cardinality = \"(0, 1)\" polymorphic = \"True\">"
        "      <Class class = \"CachedInfoClass\" />"
        "    </Source>"
        "    <Target cardinality = \"(0, N)\" polymorphic = \"True\">"
        "      <Class class = \"TestClass\" />"
        "    </Target>"
        "  </ECRelationshipClass>"

        "</ECSchema>";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema = nullptr;
    ECSchema::ReadFromXmlString(schema, schemaXml, *context);

    // Create DB
    ECSchemaCachePtr schemaCache = ECSchemaCache::Create();
    schemaCache->AddSchema(*schema);

    ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportECSchemas(*schemaCache));
    }

void RunDeleteReferentialIntegrityTest(bool withRelationsToCachedInfo)
    {
    BeFileName testDbPath;
    CreateDeleteReferentialIntegrityTestDb(testDbPath);

    Utf8CP testSchemaName = "TestSchema";
    Utf8CP testClassName = "TestClass";

    const int childCount = 10000;
    ECInstanceKey testECInstanceKey;

    {
    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(testDbPath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));

    ECClassCP testClass = ecdb.Schemas().GetECClass(testSchemaName, testClassName);

    ECClassCP cachedInfoClass = ecdb.Schemas().GetECClass(testSchemaName, "CachedInfoClass");

    ECClassCP tempClass = ecdb.Schemas().GetECClass(testSchemaName, "TestClassRelationship");
    ECRelationshipClassCP testRelationshipClass = tempClass->GetRelationshipClassCP();

    tempClass = ecdb.Schemas().GetECClass(testSchemaName, "CachedInfoClassRelationship");
    ECRelationshipClassCP cachedInfoRelationshipClass = tempClass->GetRelationshipClassCP();

    // insert test data
    testECInstanceKey = InsertInstance(ecdb, testClass);
    for (int i = 0; i < childCount; i++)
        {
        ECInstanceKey childKey = InsertInstance(ecdb, testClass);
        RelateInstances(ecdb, testClass, testECInstanceKey.GetECInstanceId(), testClass, childKey.GetECInstanceId(), testRelationshipClass);
        if (withRelationsToCachedInfo)
            {
            ECInstanceKey infoKey = InsertInstance(ecdb, cachedInfoClass);
            RelateInstances(ecdb, cachedInfoClass, infoKey.GetECInstanceId(), testClass, childKey.GetECInstanceId(), cachedInfoRelationshipClass);
            }
        }

    ecdb.SaveChanges();
    }

    //reopen to minimize any caching from the creation and population of the test ecdb
    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(testDbPath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));

    ECClassCP testClass = ecdb.Schemas().GetECClass(testSchemaName, testClassName);

    //printf ("Attach to profiler\n");
    //getchar ();

    // Delete
    StopWatch timer(true);
    ECInstanceDeleter deleter(ecdb, *testClass);
    auto status = deleter.Delete(testECInstanceKey.GetECInstanceId());
    timer.Stop();
    ASSERT_EQ(SUCCESS, status);

    //printf ("Detach from profiler\n");
    //getchar ();

    Utf8CP traceMessage = nullptr;
    if (withRelationsToCachedInfo)
        traceMessage = "ECInstanceDeleter::Delete for one instance with %d related child instances which each are related to one cache info instances took %.4f ms.";
    else
        traceMessage = "ECInstanceDeleter::Delete for one instance with 1 set of %d related child instances took %.4f ms.";

    LOG.tracev(traceMessage, childCount, timer.GetElapsedSeconds() * 1000.0);
    }

void RunDeleteReferentialIntegrityTestUsingECSql(bool withRelationsToCachedInfo)
    {
    BeFileName testDbPath;
    CreateDeleteReferentialIntegrityTestDb(testDbPath);

    Utf8CP testSchemaName = "TestSchema";
    Utf8CP testClassName = "TestClass";

    const int childCount = 10000;
    ECInstanceKey testECInstanceKey;

    {
    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(testDbPath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));

    ECClassCP testClass = ecdb.Schemas().GetECClass(testSchemaName, testClassName);

    ECClassCP cachedInfoClass = ecdb.Schemas().GetECClass(testSchemaName, "CachedInfoClass");

    ECClassCP tempClass = ecdb.Schemas().GetECClass(testSchemaName, "TestClassRelationship");
    ECRelationshipClassCP testRelationshipClass = tempClass->GetRelationshipClassCP();

    tempClass = ecdb.Schemas().GetECClass(testSchemaName, "CachedInfoClassRelationship");
    ECRelationshipClassCP cachedInfoRelationshipClass = tempClass->GetRelationshipClassCP();

    // insert test data
    testECInstanceKey = InsertInstance(ecdb, testClass);
    for (int i = 0; i < childCount; i++)
        {
        ECInstanceKey childKey = InsertInstance(ecdb, testClass);
        RelateInstances(ecdb, testClass, testECInstanceKey.GetECInstanceId(), testClass, childKey.GetECInstanceId(), testRelationshipClass);
        if (withRelationsToCachedInfo)
            {
            ECInstanceKey infoKey = InsertInstance(ecdb, cachedInfoClass);
            RelateInstances(ecdb, cachedInfoClass, infoKey.GetECInstanceId(), testClass, childKey.GetECInstanceId(), cachedInfoRelationshipClass);
            }
        }

    ecdb.SaveChanges();
    }

    //reopen to minimize any caching from the creation and population of the test ecdb
    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(testDbPath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));

    //printf ("Attach to profiler\n");
    //getchar ();

    // Delete
    StopWatch timer(true);

    ECSqlStatement stmt;
    auto preparedStatus = stmt.Prepare(ecdb, SqlPrintfString("DELETE FROM ONLY [%s].[%s] WHERE ECInstanceId = ?", testSchemaName, testClassName).GetUtf8CP());
    ASSERT_EQ(ECSqlStatus::Success, preparedStatus);
    stmt.BindId(1, testECInstanceKey.GetECInstanceId());
    auto stepStatus = stmt.Step();
    ASSERT_EQ(BE_SQLITE_DONE, stepStatus);
    timer.Stop();
    //printf ("Detach from profiler\n");
    //getchar ();

    Utf8CP traceMessage = nullptr;
    if (withRelationsToCachedInfo)
        traceMessage = "ECSQL DELETE for one instance with %d related child instances which each are related to one cache info instances took %.4f ms.";
    else
        traceMessage = "ECSQL DELETE for one instance with 1 set of %d related child instances took %.4f ms.";

    LOG.tracev(traceMessage, childCount, timer.GetElapsedSeconds() * 1000.0);
    }


//---------------------------------------------------------------------------------
// @bsiclass                                   Krischan.Eberle            02/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST(PerformanceFieldEngineer, DeleteReferentialIntegrityWithTwoRelationshipsWithECSql)
    {
    RunDeleteReferentialIntegrityTestUsingECSql(true);
    }

//---------------------------------------------------------------------------------
// @bsiclass                                   Krischan.Eberle            02/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST(PerformanceFieldEngineer, DeleteReferentialIntegrityWithOneRelationshipWithECSql)
    {
    RunDeleteReferentialIntegrityTestUsingECSql(false);
    }

//---------------------------------------------------------------------------------
// @bsiclass                                   Krischan.Eberle            02/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST(PerformanceFieldEngineer, DeleteReferentialIntegrityWithTwoRelationships)
    {
    RunDeleteReferentialIntegrityTest(true);
    }

//---------------------------------------------------------------------------------
// @bsiclass                                   Krischan.Eberle            02/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST(PerformanceFieldEngineer, DeleteReferentialIntegrityWithOneRelationship)
    {
    RunDeleteReferentialIntegrityTest(false);
    }

END_ECDBUNITTESTS_NAMESPACE