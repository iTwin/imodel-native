/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/ECDB/Published/ECInstanceInserterTests.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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
    ECClassP testClass = nullptr;
    db.GetEC().GetSchemaManager().GetECClass (testClass, schemaName, className);
    
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
    ECSqlStepStatus result;

    int total = 0;
    while ((result = countStatement.Step()) == ECSqlStepStatus::HasRow)
        total = countStatement.GetValueInt(0);
    ASSERT_EQ(numberOfInstances, total);

    ecSql.Sprintf("SELECT c0.[ECInstanceId], c0.GetECClassId() as ECClassId , * FROM [%s].[%s] c0", schemaName, className);
    ECSqlStatement queryStatement;
    queryStatement.Prepare(db, ecSql.c_str());
    int i = 0;
    bool areEqual;
    ECInstanceECSqlSelectAdapter dataAdapter (queryStatement);
    while ((result = queryStatement.Step()) == ECSqlStepStatus::HasRow)
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

    ECClassP sourceClass = nullptr;
    db.GetEC().GetSchemaManager().GetECClass (sourceClass, schemaName, sourceClassName);

    ECClassP targetClass = nullptr;
    db.GetEC().GetSchemaManager().GetECClass (targetClass, schemaName, targetClassName);

    ECClassP tempClass = nullptr;
    ECRelationshipClassCP relationshipClass = nullptr;
    db.GetEC().GetSchemaManager().GetECClass (tempClass, schemaName, relationshipClassName);
    relationshipClass = tempClass->GetRelationshipClassCP();

    ECInstanceInserter sourceInserter(db, *sourceClass);
    ECInstanceInserter targetInserter(db, *targetClass);
    ECInstanceInserter relationshipInserter(db, *relationshipClass);

    for (int sourceIndex = 0; sourceIndex < numberOfSourceInstances; sourceIndex++)
        {
        IECInstancePtr sourceInstance = ECDbTestProject::CreateArbitraryECInstance (*sourceClass, ECDbTestProject::PopulatePrimitiveValueWithRandomValues);
        ASSERT_EQ((int) ECSqlStatus::Success, (int)sourceInserter.Insert(*sourceInstance));
        for (int targetIndex = 0; targetIndex < numberOfTargetInstancesPerSource; targetIndex++)
            {
            IECInstancePtr targetInstance = ECDbTestProject::CreateArbitraryECInstance (*targetClass, ECDbTestProject::PopulatePrimitiveValueWithRandomValues);
            ECInstanceKey relationshipId;
            ASSERT_EQ((int) ECSqlStatus::Success, (int)targetInserter.Insert(*targetInstance));

            IECRelationshipInstancePtr relationshipInstance = CreateRelationship(*relationshipClass, *sourceInstance, *targetInstance);
            ASSERT_EQ((int) ECSqlStatus::Success, (int)relationshipInserter.Insert(relationshipId, *relationshipInstance));
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
    ECSchemaP rulesECSchema = nullptr;
    ASSERT_EQ (SUCCESS, db.GetEC().GetSchemaManager().GetECSchema(rulesECSchema, "ECRules"));

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
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) queryStatement.Prepare(db, ecsql));
    ECInstanceECSqlSelectAdapter dataAdapter (queryStatement);
    ECSqlStepStatus result;
    db.SaveChanges ();
    while ((result = queryStatement.Step()) == ECSqlStepStatus::HasRow)
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

    ECClassP testClass = nullptr;
    ASSERT_EQ (SUCCESS, ecdb.GetEC ().GetSchemaManager ().GetECClass (testClass, "ECSqlTest", "P"));

    ECInstanceInserter inserter (ecdb, *testClass);

    auto assertInsert = [&ecdb] (IECInstanceCR testInstance, ECInstanceId expectedId)
        {
        ECSqlStatement stmt;
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stmt.Prepare (ecdb, "SELECT I, GetECClassId() FROM ecsql.P WHERE ECInstanceId = ?"));
        stmt.BindId (1, expectedId);

        ASSERT_EQ ((int) ECSqlStepStatus::HasRow, (int) stmt.Step ());

        IECSqlValue const& colVal = stmt.GetValue (0);
        WString propPath (colVal.GetColumnInfo ().GetPropertyPath ().ToString ().c_str (), BentleyCharEncoding::Utf8);

        ECValue v;
        ASSERT_EQ (ECOBJECTS_STATUS_Success, testInstance.GetValue (v, propPath.c_str ()));
        ASSERT_EQ (v.IsNull (), colVal.IsNull ());
        if (!v.IsNull ())
            ASSERT_EQ (v.GetInteger (), colVal.GetInt ());

        ASSERT_EQ ((Int64) testInstance.GetClass ().GetId (), stmt.GetValueInt64 (1));
        ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) stmt.Step ()) << "Only one instance for given instance id is expected to be returned";
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

        testInstance.SetInstanceId (L"blabla");
        status = inserter.Insert (generatedKey, testInstance, false);
        ASSERT_EQ (ERROR, status) << testScenario << ": Inserting instance with instance id which is not of type ECInstanceId is expected to fail if auto generation is disabled";

        testInstance.SetInstanceId (L"-1111");
        status = inserter.Insert (generatedKey, testInstance, false);
        ASSERT_EQ (ERROR, status) << testScenario << ": Inserting instance with instance id which is not of type ECInstanceId is expected to fail if auto generation is disabled";

        //now pass a valid instance id
        ECInstanceKey userProvidedKey;
        status = inserter.Insert (userProvidedKey, testInstance, false, &userProvidedId);
        ASSERT_EQ (SUCCESS, status) << testScenario << ": Inserting instance with instance id of type ECInstanceId is expected to succeed if auto generation is disabled";
        ASSERT_EQ (userProvidedId.GetValue (), userProvidedKey.GetECInstanceId ().GetValue ());
        ASSERT_EQ (testClass->GetId (), userProvidedKey.GetECClassId ());
        assertInsert (testInstance, userProvidedKey.GetECInstanceId ());

        //now set a valid instance id in test instance
        userProvidedId = ECInstanceId (userProvidedId.GetValue () + 100LL);
        WChar instanceIdStr[ECInstanceIdHelper::ECINSTANCEID_STRINGBUFFER_LENGTH];
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
    testInstance->SetValue (L"I", v);
    runInsertTest (*testInstance, "Non-empty instance");
    }


END_ECDBUNITTESTS_NAMESPACE