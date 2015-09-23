/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECInstanceInserterTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "ECInstanceAdaptersTestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE
struct ECInstanceInserterTests : ECInstanceAdaptersTestFixture
    {
    protected:
        void InsertInstances(Utf8CP className, Utf8CP schemaName, int numberOfInstances, bool populateAllProperties);
        void InsertRelationshipInstances(Utf8CP relationshipClassName, Utf8CP targetClassName, Utf8CP sourceClassName, Utf8CP schemaName, int numberOfSourceInstances, int numberOfTargetInstancesPerSource);
    };

void ECInstanceInserterTests::InsertInstances(Utf8CP className, Utf8CP schemaName, int numberOfInstances, bool populateAllProperties)
    {
    SetTestProject(CreateTestProject("insertInstances.ecdb", L"KitchenSink.01.00.ecschema.xml"));
    ECDbR db = GetTestProject().GetECDb();
    ECClassCP testClass = db.Schemas().GetECClass (schemaName, className);
    
    ECInstanceInserter inserter(db, *testClass);
    bvector<IECInstancePtr> instances;
    for (int i = 0; i < numberOfInstances; i++)
        {
        IECInstancePtr instance;
        if (populateAllProperties)
            instance = ECDbTestProject::CreateArbitraryECInstance (*testClass, ECDbTestProject::PopulatePrimitiveValueWithRandomValues);
        else
            instance = testClass->GetDefaultStandaloneEnabler()->CreateInstance(0);

        auto status = inserter.Insert (*instance);
        ASSERT_EQ(SUCCESS, status);
        instances.push_back(instance);
        }

    ECSqlStatement countStatement;
    Utf8String ecSql;
    ecSql.Sprintf("SELECT count(*) FROM ONLY [%s].[%s]", schemaName, className);
    countStatement.Prepare(db, ecSql.c_str());
    DbResult result;

    int total = 0;
    while ((result = countStatement.Step()) == BE_SQLITE_ROW)
        total = countStatement.GetValueInt(0);
    ASSERT_EQ(numberOfInstances, total);

    ecSql.Sprintf("SELECT c0.[ECInstanceId], c0.GetECClassId() as ECClassId , * FROM [%s].[%s] c0", schemaName, className);
    ECSqlStatement queryStatement;
    queryStatement.Prepare(db, ecSql.c_str());
    int i = 0;
    bool areEqual;
    ECInstanceECSqlSelectAdapter dataAdapter (queryStatement);
    while ((result = queryStatement.Step()) == BE_SQLITE_ROW)
        {
        IECInstancePtr expected = instances[i];
        IECInstancePtr actual = dataAdapter.GetInstance ();
        areEqual = ECDbTestUtility::CompareECInstances (*expected, *actual);
        ASSERT_TRUE(areEqual);
        i++;
        }
    }

extern IECRelationshipInstancePtr CreateRelationship (ECRelationshipClassCR relationshipClass, IECInstanceR source, IECInstanceR target);

void ECInstanceInserterTests::InsertRelationshipInstances(Utf8CP relationshipClassName, Utf8CP sourceClassName, Utf8CP targetClassName, Utf8CP schemaName, int numberOfSourceInstances, int numberOfTargetInstancesPerSource)
    {
    SetTestProject(CreateTestProject("insertInstances.ecdb", L"KitchenSink.01.00.ecschema.xml"));
    ECDbR db = GetTestProject().GetECDb();

    ECClassCP sourceClass = db.Schemas().GetECClass (schemaName, sourceClassName);
    ECClassCP targetClass = db.Schemas().GetECClass (schemaName, targetClassName);

    ECClassCP tempClass = db. Schemas ().GetECClass (schemaName, relationshipClassName);
    ECRelationshipClassCP relationshipClass = tempClass->GetRelationshipClassCP ();

    ECInstanceInserter sourceInserter(db, *sourceClass);
    ECInstanceInserter targetInserter(db, *targetClass);
    ECInstanceInserter relationshipInserter(db, *relationshipClass);

    for (int sourceIndex = 0; sourceIndex < numberOfSourceInstances; sourceIndex++)
        {
        IECInstancePtr sourceInstance = ECDbTestProject::CreateArbitraryECInstance (*sourceClass, ECDbTestProject::PopulatePrimitiveValueWithRandomValues);
        ASSERT_EQ(SUCCESS, sourceInserter.Insert(*sourceInstance));
        for (int targetIndex = 0; targetIndex < numberOfTargetInstancesPerSource; targetIndex++)
            {
            IECInstancePtr targetInstance = ECDbTestProject::CreateArbitraryECInstance (*targetClass, ECDbTestProject::PopulatePrimitiveValueWithRandomValues);
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

TEST_F(ECInstanceInserterTests, InsertMultipleInstancesOfStructClass)
    {
    InsertInstances("Struct2", "KitchenSink", 100, true);
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
TEST_F (ECInstanceInserterTests, InsertSingleRuleInstance)
    {
    SetTestProject(CreateTestProject("insertRulesInstances.ecdb", L"ECRules.01.00.ecschema.xml"));
    ECDbR db = GetTestProject().GetECDb();
    ECSchemaCP rulesECSchema = db. Schemas ().GetECSchema ("ECRules");
    ASSERT_TRUE (rulesECSchema != nullptr);

    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext (*rulesECSchema);

    BeFileName instanceXmlFile;
    BeTest::GetHost().GetDocumentsRoot(instanceXmlFile);
    instanceXmlFile.AppendToPath(L"DgnDb");
    instanceXmlFile.AppendToPath(L"ECDb");
    instanceXmlFile.AppendToPath(L"Schemas");
    instanceXmlFile.AppendToPath(L"RuleSetInstance1.xml");

    IECInstancePtr testInstance = nullptr;
    InstanceReadStatus instanceStatus = IECInstance::ReadFromXmlFile (testInstance, instanceXmlFile.GetName (), *instanceContext);
    ASSERT_EQ (INSTANCE_READ_STATUS_Success, instanceStatus);
    WString orignalXml, afterXml;
    testInstance->WriteToXmlString (orignalXml, false, false);
    ECInstanceInserter inserter(db, testInstance->GetClass());
    auto status = inserter.Insert (*testInstance);
    ASSERT_EQ(SUCCESS, status);

    Utf8CP ecsql = "SELECT GetECClassId() as ECClassId, * FROM ECRules.RuleSet";
    ECSqlStatement queryStatement;
    ASSERT_EQ(ECSqlStatus::Success, queryStatement.Prepare(db, ecsql));
    ECInstanceECSqlSelectAdapter dataAdapter (queryStatement);
    DbResult result;
    db.SaveChanges ();
    while ((result = queryStatement.Step()) == BE_SQLITE_ROW)
        {
        IECInstancePtr actual = dataAdapter.GetInstance ();
        actual->SetInstanceId (nullptr);
        actual->WriteToXmlString (afterXml, false, false);
        ASSERT_TRUE (orignalXml == afterXml);
        }

    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECInstanceInserterTests, InsertWithUserProvidedECInstanceId)
    {
    SetTestProject (CreateTestProject ("insertwithuserprovidedecinstanceid.ecdb", L"ECSqlTest.01.00.ecschema.xml"));
    ECDbR ecdb = GetTestProject ().GetECDb ();

    ECClassCP testClass = ecdb. Schemas ().GetECClass ("ECSqlTest", "P");
    ASSERT_TRUE (testClass != nullptr);

    ECInstanceInserter inserter (ecdb, *testClass);

    auto assertInsert = [&ecdb] (IECInstanceCR testInstance, ECInstanceId expectedId)
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT I, GetECClassId() FROM ecsql.P WHERE ECInstanceId = ?"));
        stmt.BindId (1, expectedId);

        ASSERT_EQ (BE_SQLITE_ROW, stmt.Step ());

        IECSqlValue const& colVal = stmt.GetValue (0);

        ECValue v;
        ASSERT_EQ (ECOBJECTS_STATUS_Success, testInstance.GetValue (v, colVal.GetColumnInfo ().GetPropertyPath ().ToString ().c_str ()));
        ASSERT_EQ (v.IsNull (), colVal.IsNull ());
        if (!v.IsNull ())
            ASSERT_EQ (v.GetInteger (), colVal.GetInt ());

        ASSERT_EQ ((int64_t) testInstance.GetClass ().GetId (), stmt.GetValueInt64 (1));
        ASSERT_EQ (BE_SQLITE_DONE, stmt.Step ()) << "Only one instance for given instance id is expected to be returned";
        };


    auto runInsertTest = [&testClass, &inserter, &assertInsert] (IECInstanceR testInstance, Utf8CP testScenario)
        {
        ECInstanceKey generatedKey;
        auto status = inserter.Insert (generatedKey, testInstance);
        ASSERT_EQ (SUCCESS, status) << testScenario << ": Inserting instance without instance id is expected to succeed if auto generation is enabled";
        assertInsert (testInstance, generatedKey.GetECInstanceId ());

        ECInstanceId userProvidedId (generatedKey.GetECInstanceId ().GetValue () + 1111LL);

        status = inserter.Insert (generatedKey, testInstance, true, &userProvidedId);
        ASSERT_EQ (ERROR, status) << testScenario << ": When passing autogenerateECInstanceId=true user provided id must not be passed";

        status = inserter.Insert (generatedKey, testInstance, false);
        ASSERT_EQ (ERROR, status) << testScenario << ": Inserting instance without instance id is expected to fail if auto generation is disabled";

        testInstance.SetInstanceId ("blabla");
        status = inserter.Insert (generatedKey, testInstance, false);
        ASSERT_EQ (ERROR, status) << testScenario << ": Inserting instance with invalid instance id is expected to fail if auto generation is disabled";

        testInstance.SetInstanceId ("0");
        status = inserter.Insert (generatedKey, testInstance, false);
        ASSERT_EQ (ERROR, status) << testScenario << ": Inserting instance with invalid instance id is expected to fail if auto generation is disabled";

        //now pass a valid instance id
        ECInstanceKey userProvidedKey;
        status = inserter.Insert (userProvidedKey, testInstance, false, &userProvidedId);
        ASSERT_EQ (SUCCESS, status) << testScenario << ": Inserting instance with instance id of type ECInstanceId is expected to succeed if auto generation is disabled";
        ASSERT_EQ (userProvidedId.GetValue (), userProvidedKey.GetECInstanceId ().GetValue ());
        ASSERT_EQ (testClass->GetId (), userProvidedKey.GetECClassId ());
        assertInsert (testInstance, userProvidedKey.GetECInstanceId ());

        //now set a valid instance id in test instance
        userProvidedId = ECInstanceId (userProvidedId.GetValue () + 100LL);
        Utf8Char instanceIdStr[ECInstanceIdHelper::ECINSTANCEID_STRINGBUFFER_LENGTH];
        ASSERT_TRUE (ECInstanceIdHelper::ToString (instanceIdStr, ECInstanceIdHelper::ECINSTANCEID_STRINGBUFFER_LENGTH, userProvidedId));
        testInstance.SetInstanceId (instanceIdStr);

        status = inserter.Insert (userProvidedKey, testInstance, false);
        ASSERT_EQ (SUCCESS, status) << testScenario << ": Inserting instance with instance id of type ECInstanceId is expected to succeed if auto generation is disabled";
        ASSERT_EQ (userProvidedId.GetValue (), userProvidedKey.GetECInstanceId ().GetValue ());
        ASSERT_EQ (testClass->GetId (), userProvidedKey.GetECClassId ());
        assertInsert (testInstance, userProvidedKey.GetECInstanceId ());

        status = inserter.Insert (userProvidedKey, testInstance, false);
        ASSERT_EQ (ERROR, status) << testScenario << ": inserting instance with same instance id twice should result in constraint violation error";
        };

    //Test #1: insert empty instance
    IECInstancePtr testInstance = testClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    runInsertTest (*testInstance, "Empty instance");

    //Test #2: insert non-empty instance
    testInstance = testClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ECValue v (123);
    testInstance->SetValue ("I", v);
    runInsertTest (*testInstance, "Non-empty instance");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/15
//+---------------+---------------+---------------+---------------+---------------+------
void ExecuteECSqlCommand (ECSqlStatement& stmt, ECDbR db, Utf8CP ecsql)
    {
    ASSERT_EQ (stmt.Prepare (db, ecsql), ECSqlStatus::Success);
    ASSERT_EQ (stmt.Step (), BE_SQLITE_DONE);
    stmt.Finalize ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/15
//+---------------+---------------+---------------+---------------+---------------+------
void AssertCurrentTimeStamp (ECDbR ecdb, ECInstanceId const& id, bool expectedIsNull, Utf8CP assertMessage)
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT LastMod FROM ecsql.ClassWithLastModProp WHERE ECInstanceId=?"));
    stmt.BindId (1, id);

    int rowCount = 0;
    while (stmt.Step () == BE_SQLITE_ROW)
        {
        rowCount++;
        ASSERT_EQ (expectedIsNull, stmt.IsValueNull (0)) << assertMessage;
        }

    ASSERT_EQ (1, rowCount) << assertMessage;
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECInstanceInserterTests, InsertWithCurrentTimeStampTrigger)
    {
    SetTestProject (CreateTestProject ("insertwithcurrenttimestamptrigger.ecdb", L"ECSqlTest.01.00.ecschema.xml"));
    ECDbR ecdb = GetTestProject ().GetECDb ();
    auto testClass = ecdb.Schemas ().GetECClass ("ECSqlTest", "ClassWithLastModProp");
    ASSERT_TRUE (testClass != nullptr);

    //scenario 1: double-check what SQLite does with default values if the INSERT statement
    //specifies NULL or a parameter:
        {
        BeRepositoryBasedId id (BeRepositoryId (2), 1);
        Statement stmt;
        ASSERT_EQ (BE_SQLITE_OK, stmt.Prepare (ecdb, "INSERT INTO ecsqltest_ClassWithLastModProp (ECInstanceId,I,S) VALUES (?, 1,'INSERT without specifying LastMod column')"));
        stmt.BindId (1, id);
        ASSERT_EQ (BE_SQLITE_DONE, stmt.Step ());

        AssertCurrentTimeStamp (ecdb, id, false, "SQLITE INSERT with not specifying the last mod column");
        }

        {
        BeRepositoryBasedId id (BeRepositoryId (2), 2);
        Statement stmt;
        ASSERT_EQ (BE_SQLITE_OK, stmt.Prepare (ecdb, "INSERT INTO ecsqltest_ClassWithLastModProp (ECInstanceId,I,S) VALUES (?, 1,'INSERT with literal NULL')"));
        stmt.BindId (1, id);
        ASSERT_EQ (BE_SQLITE_DONE, stmt.Step ());

        AssertCurrentTimeStamp (ecdb, id, false, "SQLITE INSERT with literal NULL");
        }

        {
        BeRepositoryBasedId id (BeRepositoryId (2), 5);
        Statement stmt;
        ASSERT_EQ (BE_SQLITE_OK, stmt.Prepare (ecdb, "INSERT INTO ecsqltest_ClassWithLastModProp (ECInstanceId,I,S,LastMod) VALUES (?,1,'INSERT with bound parameter',?)"));
        stmt.BindId (1, id);
        stmt.BindDouble (2, 24565.5);
        ASSERT_EQ (BE_SQLITE_DONE, stmt.Step ());

        AssertCurrentTimeStamp (ecdb, id, false, "SQLITE INSERT with bound parameter");
        }

    //scenario 2: test thta ECInstanceInserter works fine

    auto testInstance = testClass->GetDefaultStandaloneEnabler ()->CreateInstance ();

    ECValue v (1);
    ASSERT_EQ (ECOBJECTS_STATUS_Success, testInstance->SetValue ("I", v));

    v.Clear ();
    v.SetUtf8CP ("ECInstanceInserter");
    ASSERT_EQ (ECOBJECTS_STATUS_Success, testInstance->SetValue ("S", v));

    ECInstanceInserter inserter (ecdb, *testClass);
    ASSERT_TRUE (inserter.IsValid ());

    //scenario 1: Don't set current time prop at all in ECInstance
    ECInstanceKey key;
    ASSERT_EQ (SUCCESS, inserter.Insert (key, *testInstance));

    AssertCurrentTimeStamp (ecdb, key.GetECInstanceId (), false, "ECInstanceInserter INSERT");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/15
//+---------------+---------------+---------------+---------------+---------------+------
void ExecuteECSql (ECSqlStatement& stmt, ECInstanceKey& key, ECDbR ecdb, Utf8CP ecsql)
    {
    ASSERT_EQ (stmt.Prepare (ecdb, ecsql), ECSqlStatus::Success);
    ASSERT_EQ (stmt.Step (key), BE_SQLITE_DONE);
    stmt.Finalize ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECInstanceInserterTests, GroupByClauseWithAndWithOutFunctions)
    {
    auto const schema =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='SchemaWithReuseColumn' nameSpacePrefix='rc' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.00' prefix='bsca' />"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='ClassA' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                   <Strategy>SharedTable</Strategy>"
        "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "        <ECProperty propertyName='Id' typeName='int' />"
        "    </ECClass>"
        "    <ECClass typeName='ClassAB' isDomainClass='True'>"
        "        <BaseClass>ClassA</BaseClass>"
        "    </ECClass>"
        "    <ECClass typeName='ClassAC' isDomainClass='True'>"
        "        <BaseClass>ClassA</BaseClass>"
        "    </ECClass>"
        "</ECSchema>";

    ECDbTestProject saveTestProject;
    ECDbR db = saveTestProject.Create ("TestECDbGroupByClauseWithFunctions.ecdb");
    ECSchemaPtr testSchema;
    auto readContext = ECSchemaReadContext::CreateContext ();
    ECSchema::ReadFromXmlString (testSchema, schema, *readContext);
    ASSERT_TRUE (testSchema != nullptr);
    auto importStatus = db.Schemas().ImportECSchemas (readContext->GetCache ());
    ASSERT_TRUE (importStatus == BentleyStatus::SUCCESS);

    ECSqlStatement stmt;
    ExecuteECSqlCommand (stmt, db, "INSERT INTO rc.ClassA VALUES(1000, 1)");
    ExecuteECSqlCommand (stmt, db, "INSERT INTO rc.ClassA VALUES(1500, 1)");
    ExecuteECSqlCommand (stmt, db, "INSERT INTO rc.ClassAB VALUES(2000, 2)");
    ExecuteECSqlCommand (stmt, db, "INSERT INTO rc.ClassAB VALUES(2500, 2)");
    ExecuteECSqlCommand (stmt, db, "INSERT INTO rc.ClassAC VALUES(3000, 3)");
    ExecuteECSqlCommand (stmt, db, "INSERT INTO rc.ClassAC VALUES(3500, 3)");

    ASSERT_EQ (stmt.Prepare (db, "SELECT AVG(Price), count(*) FROM rc.ClassA GROUP BY Id"), ECSqlStatus::Success);
    int count = 0;
    Utf8String expectedAvgValues = "21250.022250.023250.0";
    Utf8String actualAvgValues;
    while (stmt.Step () != BE_SQLITE_DONE)
        {
        actualAvgValues.append (stmt.GetValueText (1));
        actualAvgValues.append (stmt.GetValueText (0));
        count++;
        }
    ASSERT_EQ (count, 3);
    ASSERT_EQ (expectedAvgValues, actualAvgValues);
    stmt.Finalize ();

    count = 0;
    actualAvgValues = "";
    ASSERT_EQ (stmt.Prepare (db, "SELECT AVG(Price), count(*) FROM rc.ClassA GROUP BY GetECClassId()"), ECSqlStatus::Success);
    while (stmt.Step () != BE_SQLITE_DONE)
        {
        actualAvgValues.append (stmt.GetValueText (1));
        actualAvgValues.append (stmt.GetValueText (0));
        count++;
        }
    ASSERT_EQ (count, 3);
    ASSERT_EQ (expectedAvgValues, actualAvgValues);
    stmt.Finalize ();

    db.CloseDb ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECInstanceInserterTests, CloseDbAfterInstanceInsertion)
    {
    ECDb ecdb;
    BeFileName applicationSchemaDir;
    BeTest::GetHost ().GetDgnPlatformAssetsDirectory (applicationSchemaDir);
    BeFileName temporaryDir;
    BeTest::GetHost ().GetOutputRoot (temporaryDir);
    ECDb::Initialize (temporaryDir, &applicationSchemaDir);
    auto stat = ECDbTestUtility::CreateECDb (ecdb, nullptr, L"InstanceInserterDb.ecdb");
    ASSERT_EQ (stat, BE_SQLITE_OK);

    ECSchemaPtr testSchema;
    ECClassP testClass = nullptr;
    PrimitiveECPropertyP premitiveProperty = nullptr;
    ECSchema::CreateSchema (testSchema, "TestSchema", 1, 0);
    ASSERT_TRUE (testSchema.IsValid ());
    testSchema->SetNamespacePrefix ("ts");
    testSchema->SetDescription ("Dynamic Test Schema");
    testSchema->SetDisplayLabel ("Test Schema");

    ASSERT_TRUE (testSchema->CreateClass (testClass, "TestClass") == ECOBJECTS_STATUS_Success);
    ASSERT_TRUE (testClass->CreatePrimitiveProperty (premitiveProperty, "TestProperty", PrimitiveType::PRIMITIVETYPE_String) == ECOBJECTS_STATUS_Success);

    ECSchemaCachePtr schemaCache = ECSchemaCache::Create ();
    ASSERT_EQ (SUCCESS, schemaCache->AddSchema (*testSchema));
    ASSERT_EQ (SUCCESS, ecdb.Schemas().ImportECSchemas (*schemaCache, ECDbSchemaManager::ImportOptions ()));
    ecdb.SaveChanges ();

    StandaloneECEnablerPtr enabler = testClass->GetDefaultStandaloneEnabler ();
    ECN::StandaloneECInstancePtr testClassInstance = enabler->CreateInstance ();
    testClassInstance->SetValue ("TestProperty", ECValue ("firstProperty"));

        {
        //wrap inserter in a nested block to make sure it (and its internal ECSqlStatement) is destroyed before the DB is closed
        ECInstanceInserter inserter(ecdb, *testClass);
        ASSERT_TRUE(inserter.IsValid());
        ECInstanceKey instanceKey;
        auto insertStatus = inserter.Insert(instanceKey, *testClassInstance);
        ASSERT_EQ(SUCCESS, insertStatus);
        }
    
    ecdb.CloseDb ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECInstanceInserterTests, InsertInstanceWithOutProvidingSourceTargetClassIds)
    {
    auto const schema =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='SchemaWithReuseColumn' nameSpacePrefix='rc' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.00' prefix='bsca' />"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='ClassA' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                   <Strategy>SharedTable</Strategy>"
        "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P1' typeName='string' />"
        "    </ECClass>"
        "    <ECClass typeName='ClassAB' isDomainClass='True'>"
        "        <BaseClass>ClassA</BaseClass>"
        "        <ECProperty propertyName='P2' typeName='double' />"
        "    </ECClass>"
        "    <ECClass typeName='ClassC' isDomainClass='True'>"
        "        <ECProperty propertyName='P3' typeName='int' />"
        "    </ECClass>"
        "    <ECClass typeName='ClassD' isDomainClass='True'>"
        "        <ECProperty propertyName='P4' typeName='int' />"
        "    </ECClass>"
        "      <ECRelationshipClass typeName = 'RelationshipClassA' isDomainClass = 'True' strength = 'referencing' strengthDirection = 'forward'> "
        "          <Source cardinality = '(0,1)' polymorphic = 'True'> "
        "              <Class class = 'ClassA'/> "
        "          </Source> "
        "          <Target cardinality = '(0,N)' polymorphic = 'True'> "
        "              <Class class = 'ClassC'/> "
        "          </Target> "
        "      </ECRelationshipClass> "
        "      <ECRelationshipClass typeName = 'RelationshipClassB' isDomainClass = 'True' strength = 'referencing' strengthDirection = 'forward'> "
        "          <Source cardinality = '(0,1)' polymorphic = 'True'> "
        "              <Class class = 'ClassA'/> "
        "          </Source> "
        "          <Target cardinality = '(0,1)' polymorphic = 'True'> "
        "              <Class class = 'ClassC'/> "
        "          </Target> "
        "      </ECRelationshipClass> "
        "      <ECRelationshipClass typeName = 'RelationshipClassC' isDomainClass = 'True' strength = 'referencing' strengthDirection = 'forward'> "//relationship Constaining polymorphic Constraint
        "          <Source cardinality = '(0,N)' polymorphic = 'True'> "
        "              <Class class = 'ClassA'/> "
        "          </Source> "
        "          <Target cardinality = '(0,N)' polymorphic = 'True'> "
        "              <Class class = 'ClassC'/> "
        "          </Target> "
        "      </ECRelationshipClass> "
        "      <ECRelationshipClass typeName = 'RelationshipClassD' isDomainClass = 'True' strength = 'referencing' strengthDirection = 'forward'> "
        "          <Source cardinality = '(0,N)' polymorphic = 'True'> "
        "              <Class class = 'ClassC'/> "
        "          </Source> "
        "          <Target cardinality = '(0,N)' polymorphic = 'True'> "
        "              <Class class = 'ClassD'/> "
        "          </Target> "
        "      </ECRelationshipClass> "
        "</ECSchema>";

    ECDbTestProject saveTestProject;
    ECDbR db = saveTestProject.Create ("InsertRelationshipInstances.ecdb");
    ECSchemaPtr testSchema;
    auto readContext = ECSchemaReadContext::CreateContext ();
    ECSchema::ReadFromXmlString (testSchema, schema, *readContext);
    ASSERT_TRUE (testSchema != nullptr);
    auto importStatus = db.Schemas ().ImportECSchemas (readContext->GetCache ());
    ASSERT_TRUE (importStatus == BentleyStatus::SUCCESS);

    ECSqlStatement stmt;
    ECInstanceKey key1, key2, key3, key4, key5;
    ExecuteECSql (stmt, key1, db, "INSERT INTO rc.ClassA (P1) VALUES('classA')");
    ExecuteECSql (stmt, key2, db, "INSERT INTO rc.ClassAB (P1, P2) VALUES('ClassA', 1001.01)");
    ExecuteECSql (stmt, key3, db, "INSERT INTO rc.ClassC (P3) VALUES(1)");
    ExecuteECSql (stmt, key4, db, "INSERT INTO rc.ClassC (P3) VALUES(2)");
    ExecuteECSql (stmt, key5, db, "INSERT INTO rc.ClassD (P4) VALUES(4)");

    ASSERT_EQ (stmt.Prepare (db, "INSERT INTO rc.RelationshipClassA (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)"), ECSqlStatus::Success);
    stmt.BindId (1, key1.GetECInstanceId ());
    stmt.BindInt64 (2, key1.GetECClassId ());
    stmt.BindId (3, key3.GetECInstanceId ());
    stmt.BindInt64 (4, key3.GetECClassId ());
    ASSERT_EQ (stmt.Step (), BE_SQLITE_DONE);
    stmt.Finalize ();
    //Instance insertion query without specifing Souce/TargetClassId's should be successful for a 1:N, end tabler relationship
    ASSERT_EQ (stmt.Prepare (db, "INSERT INTO rc.RelationshipClassA (SourceECInstanceId, TargetECInstanceId) VALUES (?, ?)"), ECSqlStatus::Success);
    stmt.BindId (1, key1.GetECInstanceId ());
    stmt.BindId (2, key4.GetECInstanceId ());
    ASSERT_EQ (stmt.Step (), BE_SQLITE_DONE);
    stmt.Finalize ();

    ASSERT_EQ (stmt.Prepare (db, "INSERT INTO rc.RelationshipClassB (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)"), ECSqlStatus::Success);
    stmt.BindId (1, key1.GetECInstanceId ());
    stmt.BindInt64 (2, key1.GetECClassId ());
    stmt.BindId (3, key3.GetECInstanceId ());
    stmt.BindInt64 (4, key3.GetECClassId ());
    ASSERT_EQ (stmt.Step (), BE_SQLITE_DONE);
    stmt.Finalize ();
    //Instance insertion query without specifing Souce/TargetClassId's should be successful for a 1:1, end tabler relationship
    ASSERT_EQ (stmt.Prepare (db, "INSERT INTO rc.RelationshipClassB (SourceECInstanceId, TargetECInstanceId) VALUES (?, ?)"), ECSqlStatus::Success);
    stmt.BindId (1, key2.GetECInstanceId ());
    stmt.BindId (2, key4.GetECInstanceId ());
    ASSERT_EQ (stmt.Step (), BE_SQLITE_DONE);
    stmt.Finalize ();

    ASSERT_EQ (stmt.Prepare (db, "INSERT INTO rc.RelationshipClassC (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)"), ECSqlStatus::Success);
    stmt.BindId (1, key1.GetECInstanceId ());
    stmt.BindInt64 (2, key1.GetECClassId ());
    stmt.BindId (3, key3.GetECInstanceId ());
    stmt.BindInt64 (4, key3.GetECClassId ());
    ASSERT_EQ (stmt.Step (), BE_SQLITE_DONE);
    stmt.Finalize ();
    //Instance insertion query without specifing Souce/TargetClassId's shouldn't work for a link table relationship if constraint isn't a single table or isn't polymorphic Constraint
    ASSERT_EQ (stmt.Prepare (db, "INSERT INTO rc.RelationshipClassC (SourceECInstanceId, TargetECInstanceId) VALUES (?, ?)"), ECSqlStatus::Success);
    stmt.Finalize ();

    //Instance insertion query without specifing Souce/TargetClassId's should work for a link table relationship if each constraint is a single table
    ASSERT_EQ (stmt.Prepare (db, "INSERT INTO rc.RelationshipClassD (SourceECInstanceId, TargetECInstanceId) VALUES (?, ?)"), ECSqlStatus::Success);
    stmt.BindId (1, key4.GetECInstanceId ());
    stmt.BindId (2, key5.GetECInstanceId ());
    ASSERT_EQ (stmt.Step (), BE_SQLITE_DONE);
    stmt.Finalize ();
    }

END_ECDBUNITTESTS_NAMESPACE
