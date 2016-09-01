/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECInstanceInserterTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "../BackDoor/PublicAPI/BackDoor/ECDb/ECDbTestProject.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE
struct ECInstanceInserterTests : ECDbTestFixture
    {
    protected:
        void InsertInstances(Utf8CP className, Utf8CP schemaName, int numberOfInstances, bool populateAllProperties);
        void InsertRelationshipInstances(Utf8CP relationshipClassName, Utf8CP targetClassName, Utf8CP sourceClassName, Utf8CP schemaName, int numberOfSourceInstances, int numberOfTargetInstancesPerSource);
    };

void ECInstanceInserterTests::InsertInstances(Utf8CP className, Utf8CP schemaName, int numberOfInstances, bool populateAllProperties)
    {
    ECDbR ecdb = SetupECDb("insertInstances.ecdb", BeFileName(L"KitchenSink.01.00.ecschema.xml"));

    ECClassCP testClass = ecdb.Schemas().GetECClass(schemaName, className);

    ECInstanceInserter inserter(ecdb, *testClass);
    bvector<IECInstancePtr> instances;
    for (int i = 0; i < numberOfInstances; i++)
        {
        IECInstancePtr instance;
        if (populateAllProperties)
            instance = ECDbTestUtility::CreateArbitraryECInstance(*testClass, ECDbTestUtility::PopulatePrimitiveValueWithRandomValues);
        else
            instance = testClass->GetDefaultStandaloneEnabler()->CreateInstance(0);

        auto status = inserter.Insert(*instance);
        ASSERT_EQ(SUCCESS, status);
        instances.push_back(instance);
        }

    ECSqlStatement countStatement;
    Utf8String ecSql;
    ecSql.Sprintf("SELECT count(*) FROM ONLY [%s].[%s]", schemaName, className);
    countStatement.Prepare(ecdb, ecSql.c_str());
    DbResult result;

    int total = 0;
    while ((result = countStatement.Step()) == BE_SQLITE_ROW)
        total = countStatement.GetValueInt(0);
    ASSERT_EQ(numberOfInstances, total);

    ecSql.Sprintf("SELECT c0.[ECInstanceId], c0.ECClassId , * FROM [%s].[%s] c0", schemaName, className);
    ECSqlStatement queryStatement;
    queryStatement.Prepare(ecdb, ecSql.c_str());
    int i = 0;
    bool areEqual;
    ECInstanceECSqlSelectAdapter dataAdapter(queryStatement);
    while ((result = queryStatement.Step()) == BE_SQLITE_ROW)
        {
        IECInstancePtr expected = instances[i];
        IECInstancePtr actual = dataAdapter.GetInstance();
        areEqual = ECDbTestUtility::CompareECInstances(*expected, *actual);
        ASSERT_TRUE(areEqual);
        i++;
        }
    }

extern IECRelationshipInstancePtr CreateRelationship(ECRelationshipClassCR relationshipClass, IECInstanceR source, IECInstanceR target);

void ECInstanceInserterTests::InsertRelationshipInstances(Utf8CP relationshipClassName, Utf8CP sourceClassName, Utf8CP targetClassName, Utf8CP schemaName, int numberOfSourceInstances, int numberOfTargetInstancesPerSource)
    {
    ECDbR ecdb = SetupECDb("insertInstances.ecdb", BeFileName(L"KitchenSink.01.00.ecschema.xml"));

    ECClassCP sourceClass = ecdb.Schemas().GetECClass(schemaName, sourceClassName);
    ECClassCP targetClass = ecdb.Schemas().GetECClass(schemaName, targetClassName);

    ECClassCP tempClass = ecdb.Schemas().GetECClass(schemaName, relationshipClassName);
    ECRelationshipClassCP relationshipClass = tempClass->GetRelationshipClassCP();

    ECInstanceInserter sourceInserter(ecdb, *sourceClass);
    ECInstanceInserter targetInserter(ecdb, *targetClass);
    ECInstanceInserter relationshipInserter(ecdb, *relationshipClass);

    for (int sourceIndex = 0; sourceIndex < numberOfSourceInstances; sourceIndex++)
        {
        IECInstancePtr sourceInstance = ECDbTestUtility::CreateArbitraryECInstance(*sourceClass, ECDbTestUtility::PopulatePrimitiveValueWithRandomValues);
        ASSERT_EQ(SUCCESS, sourceInserter.Insert(*sourceInstance));
        for (int targetIndex = 0; targetIndex < numberOfTargetInstancesPerSource; targetIndex++)
            {
            IECInstancePtr targetInstance = ECDbTestUtility::CreateArbitraryECInstance(*targetClass, ECDbTestUtility::PopulatePrimitiveValueWithRandomValues);
            ECInstanceKey relationshipId;
            ASSERT_EQ(SUCCESS, targetInserter.Insert(*targetInstance));

            IECRelationshipInstancePtr relationshipInstance = CreateRelationship(*relationshipClass, *sourceInstance, *targetInstance);
            ASSERT_EQ(SUCCESS, relationshipInserter.Insert(relationshipId, *relationshipInstance));
            }
        }

    }

TEST_F(ECInstanceInserterTests, InsertSingleInstanceOfPrimitiveClass)
    {
    InsertInstances("PrimitiveClass", "KitchenSink", 1, true);
    }

TEST_F(ECInstanceInserterTests, InsertMultipleInstancesOfPrimitiveClass)
    {
    InsertInstances("PrimitiveClass", "KitchenSink", 100, true);
    }

TEST_F(ECInstanceInserterTests, InsertSingleInstanceOfPrimitiveClassWithNullValues)
    {
    InsertInstances("PrimitiveClass", "KitchenSink", 1, false);
    }

TEST_F(ECInstanceInserterTests, InsertMultipleInstancesOfPrimitiveClassWithNullValues)
    {
    InsertInstances("PrimitiveClass", "KitchenSink", 100, false);
    }

TEST_F(ECInstanceInserterTests, InsertIntoStructClass)
    {
    ECDbR ecdb = SetupECDb("insertInstances.ecdb", BeFileName(L"KitchenSink.01.00.ecschema.xml"));

    ECClassCP structClass = ecdb.Schemas().GetECClass("KitchenSink", "Struct1");
    ASSERT_TRUE(structClass != nullptr);
    ECInstanceInserter inserter(ecdb, *structClass);
    ASSERT_FALSE(inserter.IsValid());
    }

TEST_F(ECInstanceInserterTests, InsertSingleRelationshipInstance)
    {
    InsertRelationshipInstances("FolderHasDocuments", "Folder", "Document", "KitchenSink", 1, 1);
    }

TEST_F(ECInstanceInserterTests, InsertMultipleRelationshipInstances)
    {
    InsertRelationshipInstances("FolderHasDocuments", "Folder", "Document", "KitchenSink", 10, 10);
    }

TEST_F(ECInstanceInserterTests, InsertSelfJoinRelationshipInstances)
    {
    InsertRelationshipInstances("FolderHasSubFolders", "Folder", "Folder", "KitchenSink", 10, 10);
    }

TEST_F(ECInstanceInserterTests, InsertSingleInstanceOfComplexClass)
    {
    InsertInstances("TestClass", "KitchenSink", 1, true);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Carole.MacDonald                  08/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceInserterTests, InsertSingleRuleInstance)
    {
    ECDbR ecdb = SetupECDb("insertRulesInstances.ecdb", BeFileName(L"ECRules.01.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen()) << "Setup of test ECDb with ECRules ECSchema failed";
    ECSchemaCP rulesECSchema = ecdb.Schemas().GetECSchema("ECRules");
    ASSERT_TRUE(rulesECSchema != nullptr);

    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext(*rulesECSchema);

    BeFileName instanceXmlFile;
    BeTest::GetHost().GetDocumentsRoot(instanceXmlFile);
    instanceXmlFile.AppendToPath(L"ECDb");
    instanceXmlFile.AppendToPath(L"RuleSetInstance1.xml");

    IECInstancePtr testInstance = nullptr;
    InstanceReadStatus instanceStatus = IECInstance::ReadFromXmlFile(testInstance, instanceXmlFile.GetName(), *instanceContext);
    ASSERT_EQ(InstanceReadStatus::Success, instanceStatus);
    WString orignalXml, afterXml;
    testInstance->WriteToXmlString(orignalXml, false, false);
    ECInstanceInserter inserter(ecdb, testInstance->GetClass());
    auto status = inserter.Insert(*testInstance);
    ASSERT_EQ(SUCCESS, status);

    Utf8CP ecsql = "SELECT * FROM ECRules.RuleSet";
    ECSqlStatement queryStatement;
    ASSERT_EQ(ECSqlStatus::Success, queryStatement.Prepare(ecdb, ecsql));
    ECInstanceECSqlSelectAdapter dataAdapter(queryStatement);
    DbResult result;
    ecdb.SaveChanges();
    while ((result = queryStatement.Step()) == BE_SQLITE_ROW)
        {
        IECInstancePtr actual = dataAdapter.GetInstance();
        actual->SetInstanceId(nullptr);
        actual->WriteToXmlString(afterXml, false, false);
        ASSERT_TRUE(orignalXml == afterXml);
        }

    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceInserterTests, InsertWithUserProvidedECInstanceId)
    {
    ECDbR ecdb = SetupECDb("insertwithuserprovidedecinstanceid.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));

    ECClassCP testClass = ecdb.Schemas().GetECClass("ECSqlTest", "P");
    ASSERT_TRUE(testClass != nullptr);

    ECInstanceInserter inserter(ecdb, *testClass);

    auto assertInsert = [&ecdb] (IECInstanceCR testInstance, ECInstanceId expectedId)
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT I, ECClassId FROM ecsql.P WHERE ECInstanceId = ?"));
        stmt.BindId(1, expectedId);

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

        IECSqlValue const& colVal = stmt.GetValue(0);

        ECValue v;
        ASSERT_EQ(ECObjectsStatus::Success, testInstance.GetValue(v, colVal.GetColumnInfo().GetPropertyPath().ToString().c_str()));
        ASSERT_EQ(v.IsNull(), colVal.IsNull());
        if (!v.IsNull())
            ASSERT_EQ(v.GetInteger(), colVal.GetInt());

        ASSERT_EQ(testInstance.GetClass().GetId().GetValue(), stmt.GetValueId<ECClassId>(1).GetValue());
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Only one instance for given instance id is expected to be returned";
        };


    auto runInsertTest = [&testClass, &inserter, &assertInsert] (IECInstanceR testInstance, Utf8CP testScenario)
        {
        ECInstanceKey generatedKey;
        auto status = inserter.Insert(generatedKey, testInstance);
        ASSERT_EQ(SUCCESS, status) << testScenario << ": Inserting instance without instance id is expected to succeed if auto generation is enabled";
        assertInsert(testInstance, generatedKey.GetECInstanceId());

        ECInstanceId userProvidedId(generatedKey.GetECInstanceId().GetValue() + 1111LL);

        status = inserter.Insert(generatedKey, testInstance, true, &userProvidedId);
        ASSERT_EQ(ERROR, status) << testScenario << ": When passing autogenerateECInstanceId=true user provided id must not be passed";

        status = inserter.Insert(generatedKey, testInstance, false);
        ASSERT_EQ(ERROR, status) << testScenario << ": Inserting instance without instance id is expected to fail if auto generation is disabled";

        testInstance.SetInstanceId("blabla");
        status = inserter.Insert(generatedKey, testInstance, false);
        ASSERT_EQ(ERROR, status) << testScenario << ": Inserting instance with invalid instance id is expected to fail if auto generation is disabled";

        testInstance.SetInstanceId("0");
        status = inserter.Insert(generatedKey, testInstance, false);
        ASSERT_EQ(ERROR, status) << testScenario << ": Inserting instance with invalid instance id is expected to fail if auto generation is disabled";

        //now pass a valid instance id
        ECInstanceKey userProvidedKey;
        status = inserter.Insert(userProvidedKey, testInstance, false, &userProvidedId);
        ASSERT_EQ(SUCCESS, status) << testScenario << ": Inserting instance with instance id of type ECInstanceId is expected to succeed if auto generation is disabled";
        ASSERT_EQ(userProvidedId.GetValue(), userProvidedKey.GetECInstanceId().GetValue());
        ASSERT_EQ(testClass->GetId(), userProvidedKey.GetECClassId());
        assertInsert(testInstance, userProvidedKey.GetECInstanceId());

        //now set a valid instance id in test instance
        userProvidedId = ECInstanceId(userProvidedId.GetValue() + 100LL);
        Utf8Char instanceIdStr[BeInt64Id::ID_STRINGBUFFER_LENGTH];
        userProvidedId.ToString(instanceIdStr);
        testInstance.SetInstanceId(instanceIdStr);

        status = inserter.Insert(userProvidedKey, testInstance, false);
        ASSERT_EQ(SUCCESS, status) << testScenario << ": Inserting instance with instance id of type ECInstanceId is expected to succeed if auto generation is disabled";
        ASSERT_EQ(userProvidedId.GetValue(), userProvidedKey.GetECInstanceId().GetValue());
        ASSERT_EQ(testClass->GetId(), userProvidedKey.GetECClassId());
        assertInsert(testInstance, userProvidedKey.GetECInstanceId());

        status = inserter.Insert(userProvidedKey, testInstance, false);
        ASSERT_EQ(ERROR, status) << testScenario << ": inserting instance with same instance id twice should result in constraint violation error";
        };

    //Test #1: insert empty instance
    IECInstancePtr testInstance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
    runInsertTest(*testInstance, "Empty instance");

    //Test #2: insert non-empty instance
    testInstance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ECValue v(123);
    testInstance->SetValue("I", v);
    runInsertTest(*testInstance, "Non-empty instance");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Krischan.Eberle                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceInserterTests, InsertReadonlyProperty)
    {
    ECDbR ecdb = SetupECDb("insertreadonlyproperty.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                                     "<ECSchema schemaName='testSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                                                     "    <ECEntityClass typeName='A' >"
                                                                     "        <ECProperty propertyName='P1' typeName='int' readOnly='True'/>"
                                                                     "        <ECProperty propertyName='P2' typeName='string' readOnly='True'/>"
                                                                     "        <ECProperty propertyName='P3' typeName='long' />"
                                                                     "    </ECEntityClass>"
                                                                     "</ECSchema>"));

    ASSERT_TRUE(ecdb.IsDbOpen());

    const int p1Value = 100;
    Utf8CP p2Value = "sample string";
    const int64_t p3Value = INT64_C(1000);

    ECClassCP ecClass = ecdb.Schemas().GetECClass("testSchema", "A");
    ASSERT_TRUE(ecClass != nullptr);

    IECInstancePtr instance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ECValue v;
    v.SetInteger(p1Value);
    ASSERT_EQ(ECObjectsStatus::Success, instance->SetValue("P1", v));

    v.Clear();
    v.SetUtf8CP(p2Value);
    ASSERT_EQ(ECObjectsStatus::Success, instance->SetValue("P2", v));

    v.Clear();
    v.SetLong(p3Value);
    ASSERT_EQ(ECObjectsStatus::Success, instance->SetValue("P3", v));

    ECInstanceInserter inserter(ecdb, *ecClass);
    ECInstanceKey key;
    ASSERT_EQ(SUCCESS, inserter.Insert(key, *instance));

    Utf8String validateECSql;
    validateECSql.Sprintf("SELECT NULL FROM ts.A WHERE ECInstanceId=%s AND P1=%d AND P2 LIKE '%s' AND P3=%" PRId64,
                          key.GetECInstanceId().ToString().c_str(), p1Value, p2Value, p3Value);

    ECSqlStatement validateStmt;
    ASSERT_EQ(ECSqlStatus::Success, validateStmt.Prepare(ecdb, validateECSql.c_str()));
    ASSERT_EQ(BE_SQLITE_ROW, validateStmt.Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/15
//+---------------+---------------+---------------+---------------+---------------+------
void ExecuteECSqlCommand(ECSqlStatement& stmt, ECDbR db, Utf8CP ecsql)
    {
    ASSERT_EQ(stmt.Prepare(db, ecsql), ECSqlStatus::Success);
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/15
//+---------------+---------------+---------------+---------------+---------------+------
void AssertCurrentTimeStamp(ECDbR ecdb, ECInstanceId id, bool expectedIsNull, Utf8CP assertMessage)
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT LastMod FROM ecsql.ClassWithLastModProp WHERE ECInstanceId=?"));
    stmt.BindId(1, id);

    int rowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        rowCount++;
        ASSERT_EQ(expectedIsNull, stmt.IsValueNull(0)) << assertMessage;
        }

    ASSERT_EQ(1, rowCount) << assertMessage;
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Muhammad.Zaighum                  01/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceInserterTests, InsertWithCurrentTimeStampTrigger)
    {
    ECDbR ecdb = SetupECDb("insertwithcurrenttimestamptrigger.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));
    auto testClass = ecdb.Schemas().GetECClass("ECSqlTest", "ClassWithLastModProp");
    ASSERT_TRUE(testClass != nullptr);

    //scenario 1: double-check what SQLite does with default values if the INSERT statement
    //specifies NULL or a parameter:
    {
    ECInstanceId id(UINT64_C(1000));
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(ecdb, "INSERT INTO ecsqltest_ClassWithLastModProp (ECInstanceId,I,FirstName) VALUES (?, 1,'INSERT without specifying LastMod column')"));
    stmt.BindId(1, id);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    AssertCurrentTimeStamp(ecdb, id, false, "SQLITE INSERT with not specifying the last mod column");
    }

    {
    ECInstanceId id(UINT64_C(1002));
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(ecdb, "INSERT INTO ecsqltest_ClassWithLastModProp (ECInstanceId,I,FirstName) VALUES (?, 1,'INSERT with literal NULL')"));
    stmt.BindId(1, id);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    AssertCurrentTimeStamp(ecdb, id, false, "SQLITE INSERT with literal NULL");
    }

    {
    ECInstanceId id(UINT64_C(1003));
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(ecdb, "INSERT INTO ecsqltest_ClassWithLastModProp (ECInstanceId,I,FirstName,LastMod) VALUES (?,1,'INSERT with bound parameter',?)"));
    stmt.BindId(1, id);
    stmt.BindDouble(2, 24565.5);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    AssertCurrentTimeStamp(ecdb, id, false, "SQLITE INSERT with bound parameter");
    }

    //scenario 2: test thta ECInstanceInserter works fine

    auto testInstance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();

    ECValue v(1);
    ASSERT_EQ(ECObjectsStatus::Success, testInstance->SetValue("I", v));

    v.Clear();
    v.SetUtf8CP("ECInstanceInserter");
    ASSERT_EQ(ECObjectsStatus::Success, testInstance->SetValue("FirstName", v));

    ECInstanceInserter inserter(ecdb, *testClass);
    ASSERT_TRUE(inserter.IsValid());

    //scenario 1: Don't set current time prop at all in ECInstance
    ECInstanceKey key;
    ASSERT_EQ(SUCCESS, inserter.Insert(key, *testInstance));

    AssertCurrentTimeStamp(ecdb, key.GetECInstanceId(), false, "ECInstanceInserter INSERT");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/15
//+---------------+---------------+---------------+---------------+---------------+------
void ExecuteECSql(ECSqlStatement& stmt, ECInstanceKey& key, ECDbR ecdb, Utf8CP ecsql)
    {
    ASSERT_EQ(stmt.Prepare(ecdb, ecsql), ECSqlStatus::Success);
    ASSERT_EQ(stmt.Step(key), BE_SQLITE_DONE);
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceInserterTests, GroupByClauseWithAndWithOutFunctions)
    {
    auto const schema =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='SchemaWithReuseColumn' nameSpacePrefix='rc' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.00' prefix='bsca' />"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='ClassA' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "        <ECProperty propertyName='Id' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='ClassAB' >"
        "        <BaseClass>ClassA</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='ClassAC' >"
        "        <BaseClass>ClassA</BaseClass>"
        "    </ECEntityClass>"
        "</ECSchema>";

    ECDbTestProject saveTestProject;
    ECDbR db = saveTestProject.Create("TestECDbGroupByClauseWithFunctions.ecdb");
    ECSchemaPtr testSchema;
    auto readContext = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(testSchema, schema, *readContext);
    ASSERT_TRUE(testSchema != nullptr);
    auto importStatus = db.Schemas().ImportECSchemas(readContext->GetCache());
    ASSERT_TRUE(importStatus == BentleyStatus::SUCCESS);

    ECSqlStatement stmt;
    ExecuteECSqlCommand(stmt, db, "INSERT INTO rc.ClassA VALUES(1000, 1)");
    ExecuteECSqlCommand(stmt, db, "INSERT INTO rc.ClassA VALUES(1500, 1)");
    ExecuteECSqlCommand(stmt, db, "INSERT INTO rc.ClassAB VALUES(2000, 2)");
    ExecuteECSqlCommand(stmt, db, "INSERT INTO rc.ClassAB VALUES(2500, 2)");
    ExecuteECSqlCommand(stmt, db, "INSERT INTO rc.ClassAC VALUES(3000, 3)");
    ExecuteECSqlCommand(stmt, db, "INSERT INTO rc.ClassAC VALUES(3500, 3)");

    ASSERT_EQ(stmt.Prepare(db, "SELECT AVG(Price), count(*) FROM rc.ClassA GROUP BY Id"), ECSqlStatus::Success);
    int count = 0;
    Utf8String expectedAvgValues = "21250.022250.023250.0";
    Utf8String actualAvgValues;
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        actualAvgValues.append(stmt.GetValueText(1));
        actualAvgValues.append(stmt.GetValueText(0));
        count++;
        }
    ASSERT_EQ(count, 3);
    ASSERT_EQ(expectedAvgValues, actualAvgValues);
    stmt.Finalize();

    count = 0;
    actualAvgValues = "";
    ASSERT_EQ(stmt.Prepare(db, "SELECT AVG(Price), count(*) FROM rc.ClassA GROUP BY ECClassId"), ECSqlStatus::Success);
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        actualAvgValues.append(stmt.GetValueText(1));
        actualAvgValues.append(stmt.GetValueText(0));
        count++;
        }
    ASSERT_EQ(count, 3);
    ASSERT_EQ(expectedAvgValues, actualAvgValues);
    stmt.Finalize();

    db.CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceInserterTests, CloseDbAfterInstanceInsertion)
    {
    ECDb ecdb;
    BeFileName applicationSchemaDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(applicationSchemaDir);
    BeFileName temporaryDir;
    BeTest::GetHost().GetOutputRoot(temporaryDir);
    ECDb::Initialize(temporaryDir, &applicationSchemaDir);
    auto stat = ECDbTestUtility::CreateECDb(ecdb, nullptr, L"InstanceInserterDb.ecdb");
    ASSERT_EQ(stat, BE_SQLITE_OK);

    ECSchemaPtr testSchema;
    ECEntityClassP testClass = nullptr;
    PrimitiveECPropertyP premitiveProperty = nullptr;
    ECSchema::CreateSchema(testSchema, "TestSchema", "ts", 1, 0, 0);
    ASSERT_TRUE(testSchema.IsValid());
    testSchema->SetDescription("Dynamic Test Schema");
    testSchema->SetDisplayLabel("Test Schema");

    ASSERT_TRUE(testSchema->CreateEntityClass(testClass, "TestClass") == ECObjectsStatus::Success);
    ASSERT_TRUE(testClass->CreatePrimitiveProperty(premitiveProperty, "TestProperty", PrimitiveType::PRIMITIVETYPE_String) == ECObjectsStatus::Success);

    ECSchemaCachePtr schemaCache = ECSchemaCache::Create();
    ASSERT_EQ(ECObjectsStatus::Success, schemaCache->AddSchema(*testSchema));
    ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportECSchemas(*schemaCache));
    ecdb.SaveChanges();

    StandaloneECEnablerPtr enabler = testClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr testClassInstance = enabler->CreateInstance();
    testClassInstance->SetValue("TestProperty", ECValue("firstProperty"));

    {
    //wrap inserter in a nested block to make sure it (and its internal ECSqlStatement) is destroyed before the DB is closed
    ECInstanceInserter inserter(ecdb, *testClass);
    ASSERT_TRUE(inserter.IsValid());
    ECInstanceKey instanceKey;
    auto insertStatus = inserter.Insert(instanceKey, *testClassInstance);
    ASSERT_EQ(SUCCESS, insertStatus);
    }

    ecdb.CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceInserterTests, InsertInstanceWithOutProvidingSourceTargetClassIds)
    {
    auto const schema =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='SchemaWithReuseColumn' nameSpacePrefix='rc' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.00' prefix='bsca' />"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='ClassA' >"
        "        <ECCustomAttributes>"
        "         <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P1' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='ClassAB' >"
        "        <BaseClass>ClassA</BaseClass>"
        "        <ECProperty propertyName='P2' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='ClassC' >"
        "        <ECProperty propertyName='P3' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='ClassD' >"
        "        <ECProperty propertyName='P4' typeName='int' />"
        "    </ECEntityClass>"
        "      <ECRelationshipClass typeName = 'RelationshipClassA' strength = 'referencing' strengthDirection = 'forward'  modifier='Sealed'> "
        "          <Source cardinality = '(0,1)' polymorphic = 'True'> "
        "              <Class class = 'ClassA'/> "
        "          </Source> "
        "          <Target cardinality = '(0,N)' polymorphic = 'True'> "
        "              <Class class = 'ClassC'/> "
        "          </Target> "
        "      </ECRelationshipClass> "
        "      <ECRelationshipClass typeName = 'RelationshipClassB' strength = 'referencing' strengthDirection = 'forward'  modifier='Sealed'> "
        "          <Source cardinality = '(0,1)' polymorphic = 'True'> "
        "              <Class class = 'ClassA'/> "
        "          </Source> "
        "          <Target cardinality = '(0,1)' polymorphic = 'True'> "
        "              <Class class = 'ClassC'/> "
        "          </Target> "
        "      </ECRelationshipClass> "
        "      <ECRelationshipClass typeName = 'RelationshipClassC' strength = 'referencing' strengthDirection = 'forward'  modifier='Sealed'> "//relationship Constaining polymorphic Constraint
        "          <Source cardinality = '(0,N)' polymorphic = 'True'> "
        "              <Class class = 'ClassA'/> "
        "          </Source> "
        "          <Target cardinality = '(0,N)' polymorphic = 'True'> "
        "              <Class class = 'ClassC'/> "
        "          </Target> "
        "      </ECRelationshipClass> "
        "      <ECRelationshipClass typeName = 'RelationshipClassD' strength = 'referencing' strengthDirection = 'forward'  modifier='Sealed'> "
        "          <Source cardinality = '(0,N)' polymorphic = 'True'> "
        "              <Class class = 'ClassC'/> "
        "          </Source> "
        "          <Target cardinality = '(0,N)' polymorphic = 'True'> "
        "              <Class class = 'ClassD'/> "
        "          </Target> "
        "      </ECRelationshipClass> "
        "</ECSchema>";

    ECDbTestProject saveTestProject;
    ECDbR db = saveTestProject.Create("InsertRelationshipInstances.ecdb");
    ECSchemaPtr testSchema;
    auto readContext = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(testSchema, schema, *readContext);
    ASSERT_TRUE(testSchema != nullptr);
    auto importStatus = db.Schemas().ImportECSchemas(readContext->GetCache());
    ASSERT_TRUE(importStatus == BentleyStatus::SUCCESS);

    ECSqlStatement stmt;
    ECInstanceKey key1, key2, key3, key4, key5;
    ExecuteECSql(stmt, key1, db, "INSERT INTO rc.ClassA (P1) VALUES('classA')");
    ExecuteECSql(stmt, key2, db, "INSERT INTO rc.ClassAB (P1, P2) VALUES('ClassA', 1001.01)");
    ExecuteECSql(stmt, key3, db, "INSERT INTO rc.ClassC (P3) VALUES(1)");
    ExecuteECSql(stmt, key4, db, "INSERT INTO rc.ClassC (P3) VALUES(2)");
    ExecuteECSql(stmt, key5, db, "INSERT INTO rc.ClassD (P4) VALUES(4)");

    ASSERT_EQ(stmt.Prepare(db, "INSERT INTO rc.RelationshipClassA (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)"), ECSqlStatus::Success);
    stmt.BindId(1, key1.GetECInstanceId());
    stmt.BindId(2, key1.GetECClassId());
    stmt.BindId(3, key3.GetECInstanceId());
    stmt.BindId(4, key3.GetECClassId());
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    stmt.Finalize();
    //Instance insertion query without specifing Souce/TargetClassId's should be successful for a 1:N, end tabler relationship
    ASSERT_EQ(stmt.Prepare(db, "INSERT INTO rc.RelationshipClassA (SourceECInstanceId, TargetECInstanceId) VALUES (?, ?)"), ECSqlStatus::Success);
    stmt.BindId(1, key1.GetECInstanceId());
    stmt.BindId(2, key4.GetECInstanceId());
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    stmt.Finalize();

    ASSERT_EQ(stmt.Prepare(db, "INSERT INTO rc.RelationshipClassB (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)"), ECSqlStatus::Success);
    stmt.BindId(1, key1.GetECInstanceId());
    stmt.BindId(2, key1.GetECClassId());
    stmt.BindId(3, key3.GetECInstanceId());
    stmt.BindId(4, key3.GetECClassId());
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    stmt.Finalize();
    //Instance insertion query without specifing Souce/TargetClassId's should be successful for a 1:1, end tabler relationship
    ASSERT_EQ(stmt.Prepare(db, "INSERT INTO rc.RelationshipClassB (SourceECInstanceId, TargetECInstanceId) VALUES (?, ?)"), ECSqlStatus::Success);
    stmt.BindId(1, key2.GetECInstanceId());
    stmt.BindId(2, key4.GetECInstanceId());
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    stmt.Finalize();

    ASSERT_EQ(stmt.Prepare(db, "INSERT INTO rc.RelationshipClassC (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)"), ECSqlStatus::Success);
    stmt.BindId(1, key1.GetECInstanceId());
    stmt.BindId(2, key1.GetECClassId());
    stmt.BindId(3, key3.GetECInstanceId());
    stmt.BindId(4, key3.GetECClassId());
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    stmt.Finalize();
    //Instance insertion query without specifing Souce/TargetClassId's shouldn't work for a link table relationship if constraint isn't a single table or isn't polymorphic Constraint
    ASSERT_EQ(stmt.Prepare(db, "INSERT INTO rc.RelationshipClassC (SourceECInstanceId, TargetECInstanceId) VALUES (?, ?)"), ECSqlStatus::Success);
    stmt.Finalize();

    //Instance insertion query without specifing Souce/TargetClassId's should work for a link table relationship if each constraint is a single table
    ASSERT_EQ(stmt.Prepare(db, "INSERT INTO rc.RelationshipClassD (SourceECInstanceId, TargetECInstanceId) VALUES (?, ?)"), ECSqlStatus::Success);
    stmt.BindId(1, key4.GetECInstanceId());
    stmt.BindId(2, key5.GetECInstanceId());
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    stmt.Finalize();
    }

END_ECDBUNITTESTS_NAMESPACE
