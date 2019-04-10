/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECInstanceInserterTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "NestedStructArrayTestSchemaHelper.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass                                     Carole.MacDonald                  08/14
//+---------------+---------------+---------------+---------------+---------------+------
struct ECSqlAdapterTestFixture : ECDbTestFixture
    {};

//---------------------------------------------------------------------------------------
// @bsiclass                                     Carole.MacDonald                  08/14
//+---------------+---------------+---------------+---------------+---------------+------
struct ECInstanceInserterTests : ECSqlAdapterTestFixture
    {
    protected:
        void InsertInstances(Utf8CP className, Utf8CP schemaName, int numberOfInstances, bool populateAllProperties);
        void InsertRelationshipInstances(Utf8CP relationshipClassName, Utf8CP targetClassName, Utf8CP sourceClassName, Utf8CP schemaName, int numberOfSourceInstances, int numberOfTargetInstancesPerSource);
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Carole.MacDonald                  08/14
//+---------------+---------------+---------------+---------------+---------------+------
void ECInstanceInserterTests::InsertInstances(Utf8CP className, Utf8CP schemaName, int numberOfInstances, bool populateAllProperties)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("insertInstances.ecdb", SchemaItem::CreateForFile("KitchenSink.01.00.00.ecschema.xml")));

    ECClassCP testClass = m_ecdb.Schemas().GetClass(schemaName, className);

    ECInstanceInserter inserter(m_ecdb, *testClass, nullptr);
    bvector<IECInstancePtr> instances;
    for (int i = 0; i < numberOfInstances; i++)
        {
        IECInstancePtr instance = testClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
        if (populateAllProperties)
            ECInstancePopulator::Populate(*instance);

        ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(*instance));
        instances.push_back(instance);
        }

    ECSqlStatement countStatement;
    Utf8String ecSql;
    ecSql.Sprintf("SELECT count(*) FROM ONLY [%s].[%s]", schemaName, className);
    countStatement.Prepare(m_ecdb, ecSql.c_str());
    DbResult result;

    int total = 0;
    while ((result = countStatement.Step()) == BE_SQLITE_ROW)
        total = countStatement.GetValueInt(0);
    ASSERT_EQ(numberOfInstances, total);

    ecSql.Sprintf("SELECT c0.[ECInstanceId], c0.ECClassId , * FROM [%s].[%s] c0", schemaName, className);
    ECSqlStatement queryStatement;
    queryStatement.Prepare(m_ecdb, ecSql.c_str());
    int i = 0;
    ECInstanceECSqlSelectAdapter dataAdapter(queryStatement);
    while ((result = queryStatement.Step()) == BE_SQLITE_ROW)
        {
        IECInstancePtr expected = instances[i];
        IECInstancePtr actual = dataAdapter.GetInstance();

        Json::Value expectedJson, actualJson;
        ASSERT_EQ(SUCCESS, JsonEcInstanceWriter::WriteInstanceToJson(expectedJson, *expected, nullptr, true));
        ASSERT_EQ(SUCCESS, JsonEcInstanceWriter::WriteInstanceToJson(actualJson, *actual, nullptr, true));
        ASSERT_EQ(JsonValue(expectedJson), JsonValue(actualJson));
        i++;
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Carole.MacDonald                  08/14
//+---------------+---------------+---------------+---------------+---------------+------
void ECInstanceInserterTests::InsertRelationshipInstances(Utf8CP relationshipClassName, Utf8CP sourceClassName, Utf8CP targetClassName, Utf8CP schemaName, int numberOfSourceInstances, int numberOfTargetInstancesPerSource)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("insertInstances.ecdb", SchemaItem::CreateForFile("KitchenSink.01.00.00.ecschema.xml")));

    ECClassCP sourceClass = m_ecdb.Schemas().GetClass(schemaName, sourceClassName);
    ECClassCP targetClass = m_ecdb.Schemas().GetClass(schemaName, targetClassName);

    ECClassCP tempClass = m_ecdb.Schemas().GetClass(schemaName, relationshipClassName);
    ECRelationshipClassCP relationshipClass = tempClass->GetRelationshipClassCP();
    StandaloneECRelationshipEnablerPtr relationshipEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*relationshipClass);

    ECInstanceInserter sourceInserter(m_ecdb, *sourceClass, nullptr);
    ECInstanceInserter targetInserter(m_ecdb, *targetClass, nullptr);
    ECInstanceInserter relationshipInserter(m_ecdb, *relationshipClass, nullptr);

    for (int sourceIndex = 0; sourceIndex < numberOfSourceInstances; sourceIndex++)
        {
        IECInstancePtr sourceInstance = sourceClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
        ECInstancePopulator::Populate(*sourceInstance);

        ASSERT_EQ(BE_SQLITE_OK, sourceInserter.Insert(*sourceInstance));
        for (int targetIndex = 0; targetIndex < numberOfTargetInstancesPerSource; targetIndex++)
            {
            IECInstancePtr targetInstance = targetClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
            ECInstancePopulator::Populate(*targetInstance);
            ECInstanceKey relationshipId;
            ASSERT_EQ(BE_SQLITE_OK, targetInserter.Insert(*targetInstance));

            IECRelationshipInstancePtr relationshipInstance = relationshipEnabler->CreateRelationshipInstance();
            ASSERT_TRUE(relationshipInstance != nullptr);
            relationshipInstance->SetSource(sourceInstance.get());
            relationshipInstance->SetTarget(targetInstance.get());
            relationshipInstance->SetInstanceId("source->target");
            ASSERT_EQ(BE_SQLITE_OK, relationshipInserter.Insert(relationshipId, *relationshipInstance));
            }
        }

    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceInserterTests, InsertIntoRelationships)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("InsertIntoelationships.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    ECClassCP navPropRelClass = m_ecdb.Schemas().GetClass("ECSqlTest", "PSAHasP_N1");
    ASSERT_TRUE(navPropRelClass != nullptr);
    ECInstanceInserter inserter(m_ecdb, *navPropRelClass, nullptr);
    ASSERT_FALSE(inserter.IsValid()) << "Cannot insert into nav prop relationship class " << navPropRelClass->GetFullName();

    ECClassCP linkTableRelClass = m_ecdb.Schemas().GetClass("ECSqlTest", "PSAHasPSA_NN");
    ASSERT_TRUE(linkTableRelClass != nullptr);

    ECInstanceInserter inserter2(m_ecdb, *linkTableRelClass, nullptr);
    ASSERT_TRUE(inserter2.IsValid()) << "Expected to be able to insert into link table relationship class " << linkTableRelClass->GetFullName();
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Carole.MacDonald                  08/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceInserterTests, InsertSingleInstanceOfPrimitiveClass)
    {
    InsertInstances("PrimitiveClass", "KitchenSink", 1, true);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Carole.MacDonald                  08/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceInserterTests, InsertMultipleInstancesOfPrimitiveClass)
    {
    InsertInstances("PrimitiveClass", "KitchenSink", 100, true);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Carole.MacDonald                  08/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceInserterTests, InsertSingleInstanceOfPrimitiveClassWithNullValues)
    {
    InsertInstances("PrimitiveClass", "KitchenSink", 1, false);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Carole.MacDonald                  08/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceInserterTests, InsertMultipleInstancesOfPrimitiveClassWithNullValues)
    {
    InsertInstances("PrimitiveClass", "KitchenSink", 100, false);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Carole.MacDonald                  08/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceInserterTests, InsertIntoStructClass)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("insertInstances.ecdb", SchemaItem::CreateForFile("KitchenSink.01.00.00.ecschema.xml")));

    ECClassCP structClass = m_ecdb.Schemas().GetClass("KitchenSink", "Struct1");
    ASSERT_TRUE(structClass != nullptr);
    ECInstanceInserter inserter(m_ecdb, *structClass, nullptr);
    ASSERT_FALSE(inserter.IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsitest                                     Krischan.Eberle            02/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceInserterTests, InsertIntoStructArray)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecinstanceinserterstructarray.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
          <ECEntityClass typeName="MyClass">
                <ECStructArrayProperty propertyName="Locations" typeName="LocationStruct"/>
          </ECEntityClass>
          <ECStructClass typeName="LocationStruct">
                <ECProperty propertyName="Street" typeName="string"/>
                <ECStructProperty propertyName="City" typeName="CityStruct"/>
          </ECStructClass>
         <ECStructClass typeName="CityStruct">
               <ECProperty propertyName="Name" typeName="string"/>
               <ECProperty propertyName="State" typeName="string"/>
               <ECProperty propertyName="Country" typeName="string"/>
               <ECProperty propertyName="Zip" typeName="int"/>
         </ECStructClass>
        </ECSchema>
        )xml")));

    ECClassCP testClass = m_ecdb.Schemas().GetClass("TestSchema", "MyClass");
    ASSERT_TRUE(testClass != nullptr);

    ECClassCP locationStruct = m_ecdb.Schemas().GetClass("TestSchema", "LocationStruct");
    ASSERT_TRUE(locationStruct != nullptr);

    ECInstanceKey key;
    {
    ECInstanceInserter inserter(m_ecdb, *testClass, nullptr);
    ASSERT_TRUE(inserter.IsValid());

    IECInstancePtr instance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();

    uint32_t arrayIndex = 0;
    //not set at all(just created by AddArrayElement call)
    ASSERT_EQ(ECObjectsStatus::Success, instance->AddArrayElements("Locations", 1));

    //set nullptr struct as element
    ASSERT_EQ(ECObjectsStatus::Success, instance->AddArrayElements("Locations", 1));
    ECValue structValue;
    ASSERT_EQ(SUCCESS, structValue.SetStruct(nullptr));
    ASSERT_TRUE(structValue.IsNull()) << "expected current ECObjects behavior";
    arrayIndex++;
    ASSERT_EQ(ECObjectsStatus::Success, instance->SetValue("Locations", structValue, arrayIndex));

    //set empty struct as element
    ASSERT_EQ(ECObjectsStatus::Success, instance->AddArrayElements("Locations", 1));
    IECInstancePtr locationStructInstance = locationStruct->GetDefaultStandaloneEnabler()->CreateInstance();
    structValue.Clear();
    ASSERT_EQ(SUCCESS, structValue.SetStruct(locationStructInstance.get()));
    ASSERT_FALSE(structValue.IsNull()) << "expected current ECObjects behavior";
    arrayIndex++;
    ASSERT_EQ(ECObjectsStatus::Success, instance->SetValue("Locations", structValue, arrayIndex));

    //set struct with a single property value set as element
    ASSERT_EQ(ECObjectsStatus::Success, instance->AddArrayElements("Locations", 1));
    locationStructInstance = locationStruct->GetDefaultStandaloneEnabler()->CreateInstance();
    ASSERT_EQ(ECObjectsStatus::Success, locationStructInstance->SetValue("Street", ECValue("mainstreet")));
    structValue.Clear();
    ASSERT_EQ(SUCCESS, structValue.SetStruct(locationStructInstance.get()));
    arrayIndex++;
    ASSERT_EQ(ECObjectsStatus::Success, instance->SetValue("Locations", structValue, arrayIndex));

    //set struct with a member of the nested struct set
    ASSERT_EQ(ECObjectsStatus::Success, instance->AddArrayElements("Locations", 1));
    locationStructInstance = locationStruct->GetDefaultStandaloneEnabler()->CreateInstance();
    ASSERT_EQ(ECObjectsStatus::Success, locationStructInstance->SetValue("City.Zip", ECValue(34000)));
    structValue.Clear();
    ASSERT_EQ(SUCCESS, structValue.SetStruct(locationStructInstance.get()));
    arrayIndex++;
    ASSERT_EQ(ECObjectsStatus::Success, instance->SetValue("Locations", structValue, arrayIndex));

    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(key, *instance));
    }

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT Locations FROM ts_MyClass WHERE Id=?"));
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    Utf8String actualJson(stmt.GetValueText(0));
    actualJson.ReplaceAll(" ", "");
    ASSERT_STRCASEEQ(R"json([null,null,null,{"Street":"mainstreet"},{"City":{"Zip":34000}}])json", actualJson.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Muhammad Hassan                    02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceInserterTests, InsertIntoNestedStructArray)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("InsertStructArray.ecdb", SchemaItem::CreateForFile("NestedStructArrayTest.01.00.00.ecschema.xml")));

    ECInstanceList instanceList = NestedStructArrayTestSchemaHelper::CreateECInstances(m_ecdb, 1, "ClassP");

    Utf8String inXml, outXml;
    for (IECInstancePtr inst : instanceList)
        {
        ECInstanceInserter inserter(m_ecdb, inst->GetClass(), nullptr);
        ASSERT_TRUE(inserter.IsValid());
        ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(*inst, true));
        inst->WriteToXmlString(inXml, true, true);
        inXml += "\r\n";
        }

    bvector<IECInstancePtr> out;
    ECSqlStatement stmt;
    auto prepareStatus = stmt.Prepare(m_ecdb, "SELECT * FROM ONLY nsat.ClassP ORDER BY ECInstanceId");
    ASSERT_TRUE(prepareStatus == ECSqlStatus::Success);
    ECInstanceECSqlSelectAdapter classPReader(stmt);
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        auto inst = classPReader.GetInstance();
        out.push_back(inst);
        inst->WriteToXmlString(outXml, true, true);
        outXml += "\r\n";
        }

    ASSERT_EQ(instanceList.size(), out.size());
    ASSERT_TRUE(inXml == outXml);
    }

//---------------------------------------------------------------------------------------
// @bsitest                                     Krischan.Eberle            06/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceInserterTests, InsertSingleInstanceOfComplexClass)
    {
    InsertInstances("TestClass", "KitchenSink", 1, true);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Carole.MacDonald                  08/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceInserterTests, InsertSingleRuleInstance)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("insertRulesInstances.ecdb", SchemaItem::CreateForFile("ECRules.01.00.00.ecschema.xml")));
    ECSchemaCP rulesECSchema = m_ecdb.Schemas().GetSchema("ECRules");
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
    ECInstanceInserter inserter(m_ecdb, testInstance->GetClass(), nullptr);
    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(*testInstance));

    Utf8CP ecsql = "SELECT * FROM ECRules.RuleSet";
    ECSqlStatement queryStatement;
    ASSERT_EQ(ECSqlStatus::Success, queryStatement.Prepare(m_ecdb, ecsql));
    ECInstanceECSqlSelectAdapter dataAdapter(queryStatement);
    DbResult result;
    m_ecdb.SaveChanges();
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
    ASSERT_EQ(SUCCESS, SetupECDb("insertwithuserprovidedecinstanceid.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    ECClassCP testClass = m_ecdb.Schemas().GetClass("ECSqlTest", "P");
    ASSERT_TRUE(testClass != nullptr);

    ECInstanceInserter inserter(m_ecdb, *testClass, nullptr);

    auto assertInsert = [] (ECDbCR ecdb, IECInstanceCR testInstance, ECInstanceId expectedId)
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


    auto runInsertTest = [&testClass, &inserter, &assertInsert] (ECDbCR ecdb, IECInstanceR testInstance, Utf8CP testScenario)
        {
        ECInstanceKey generatedKey;
        ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(generatedKey, testInstance)) << testScenario << ": Inserting instance without instance id is expected to succeed if auto generation is enabled";
        assertInsert(ecdb, testInstance, generatedKey.GetInstanceId());

        ECInstanceId userProvidedId((uint64_t)(generatedKey.GetInstanceId().GetValue() + 1111LL));

        ASSERT_EQ(BE_SQLITE_ERROR, inserter.Insert(generatedKey, testInstance, true, &userProvidedId)) << testScenario << ": When passing autogenerateECInstanceId=true user provided id must not be passed";

        ASSERT_EQ(BE_SQLITE_ERROR, inserter.Insert(generatedKey, testInstance, false)) << testScenario << ": Inserting instance without instance id is expected to fail if auto generation is disabled";

        testInstance.SetInstanceId("blabla");
        ASSERT_EQ(BE_SQLITE_ERROR, inserter.Insert(generatedKey, testInstance, false)) << testScenario << ": Inserting instance with invalid instance id is expected to fail if auto generation is disabled";

        testInstance.SetInstanceId("0");
        ASSERT_EQ(BE_SQLITE_ERROR, inserter.Insert(generatedKey, testInstance, false)) << testScenario << ": Inserting instance with invalid instance id is expected to fail if auto generation is disabled";

        //now pass a valid instance id
        ECInstanceKey userProvidedKey;
        ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(userProvidedKey, testInstance, false, &userProvidedId)) << testScenario << ": Inserting instance with instance id of type ECInstanceId is expected to succeed if auto generation is disabled";
        ASSERT_EQ(userProvidedId.GetValue(), userProvidedKey.GetInstanceId().GetValue());
        ASSERT_EQ(testClass->GetId(), userProvidedKey.GetClassId());
        assertInsert(ecdb, testInstance, userProvidedKey.GetInstanceId());

        //now set a valid instance id in test instance
        userProvidedId = ECInstanceId((uint64_t)(userProvidedId.GetValue() + 100LL));
        Utf8Char instanceIdStr[BeInt64Id::ID_STRINGBUFFER_LENGTH];
        userProvidedId.ToString(instanceIdStr);
        testInstance.SetInstanceId(instanceIdStr);

        ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(userProvidedKey, testInstance, false)) << testScenario << ": Inserting instance with instance id of type ECInstanceId is expected to succeed if auto generation is disabled";
        ASSERT_EQ(userProvidedId.GetValue(), userProvidedKey.GetInstanceId().GetValue());
        ASSERT_EQ(testClass->GetId(), userProvidedKey.GetClassId());
        assertInsert(ecdb, testInstance, userProvidedKey.GetInstanceId());

        ASSERT_EQ(BE_SQLITE_CONSTRAINT_PRIMARYKEY, inserter.Insert(userProvidedKey, testInstance, false)) << testScenario << ": inserting instance with same instance id twice should result in constraint violation error";
        };

    //Test #1: insert empty instance
    IECInstancePtr testInstance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
    runInsertTest(m_ecdb, *testInstance, "Empty instance");

    //Test #2: insert non-empty instance
    testInstance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ECValue v(123);
    testInstance->SetValue("I", v);
    runInsertTest(m_ecdb, *testInstance, "Non-empty instance");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Krischan.Eberle                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceInserterTests, InsertReadonlyProperty)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("insertreadonlyproperty.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                                     "<ECSchema schemaName='testSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                                                     "    <ECEntityClass typeName='A' >"
                                                                     "        <ECProperty propertyName='P1' typeName='int' readOnly='True'/>"
                                                                     "        <ECProperty propertyName='P2' typeName='string' readOnly='True'/>"
                                                                     "        <ECProperty propertyName='P3' typeName='long' />"
                                                                     "    </ECEntityClass>"
                                                                     "</ECSchema>")));


    const int p1Value = 100;
    Utf8CP p2Value = "sample string";
    const int64_t p3Value = INT64_C(1000);

    ECClassCP ecClass = m_ecdb.Schemas().GetClass("testSchema", "A");
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

    ECInstanceInserter inserter(m_ecdb, *ecClass, nullptr);
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(key, *instance));

    Utf8String validateECSql;
    validateECSql.Sprintf("SELECT NULL FROM ts.A WHERE ECInstanceId=%s AND P1=%d AND P2 LIKE '%s' AND P3=%" PRId64,
                          key.GetInstanceId().ToString().c_str(), p1Value, p2Value, p3Value);

    ECSqlStatement validateStmt;
    ASSERT_EQ(ECSqlStatus::Success, validateStmt.Prepare(m_ecdb, validateECSql.c_str()));
    ASSERT_EQ(BE_SQLITE_ROW, validateStmt.Step());
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
    ASSERT_EQ(SUCCESS, SetupECDb("insertwithcurrenttimestamptrigger.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));
    auto testClass = m_ecdb.Schemas().GetClass("ECSqlTest", "ClassWithLastModProp");
    ASSERT_TRUE(testClass != nullptr);

    //scenario 1: double-check what SQLite does with default values if the INSERT statement
    //specifies NULL or a parameter:
    {
    ECInstanceId id(UINT64_C(1000));
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "INSERT INTO ecsqltest_ClassWithLastModProp (Id,I,FirstName) VALUES (?, 1,'INSERT without specifying LastMod column')"));
    stmt.BindId(1, id);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    AssertCurrentTimeStamp(m_ecdb, id, false, "SQLITE INSERT with not specifying the last mod column");
    }

    {
    ECInstanceId id(UINT64_C(1002));
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "INSERT INTO ecsqltest_ClassWithLastModProp (Id,I,FirstName) VALUES (?, 1,'INSERT with literal NULL')"));
    stmt.BindId(1, id);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    AssertCurrentTimeStamp(m_ecdb, id, false, "SQLITE INSERT with literal NULL");
    }

    {
    ECInstanceId id(UINT64_C(1003));
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "INSERT INTO ecsqltest_ClassWithLastModProp (Id,I,FirstName,LastMod) VALUES (?,1,'INSERT with bound parameter',?)"));
    stmt.BindId(1, id);
    stmt.BindDouble(2, 24565.5);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    AssertCurrentTimeStamp(m_ecdb, id, false, "SQLITE INSERT with bound parameter");
    }

    //scenario 2: test thta ECInstanceInserter works fine

    auto testInstance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();

    ECValue v(1);
    ASSERT_EQ(ECObjectsStatus::Success, testInstance->SetValue("I", v));

    v.Clear();
    v.SetUtf8CP("ECInstanceInserter");
    ASSERT_EQ(ECObjectsStatus::Success, testInstance->SetValue("FirstName", v));

    ECInstanceInserter inserter(m_ecdb, *testClass, nullptr);
    ASSERT_TRUE(inserter.IsValid());

    //scenario 1: Don't set current time prop at all in ECInstance
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(key, *testInstance));

    AssertCurrentTimeStamp(m_ecdb, key.GetInstanceId(), false, "ECInstanceInserter INSERT");
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceInserterTests, CloseDbAfterInstanceInsertion)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("InstanceInserterDb.ecdb"));

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
    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().ImportSchemas(schemaCache->GetSchemas()));
    m_ecdb.SaveChanges();

    StandaloneECEnablerPtr enabler = testClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr testClassInstance = enabler->CreateInstance();
    testClassInstance->SetValue("TestProperty", ECValue("firstProperty"));

    {
    //wrap inserter in a nested block to make sure it (and its internal ECSqlStatement) is destroyed before the DB is closed
    ECInstanceInserter inserter(m_ecdb, *testClass, nullptr);
    ASSERT_TRUE(inserter.IsValid());
    ECInstanceKey instanceKey;
    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(instanceKey, *testClassInstance));
    }

    m_ecdb.CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceInserterTests, InsertInstanceWithOutProvidingSourceTargetClassIds)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("InsertRelationshipInstances.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='SchemaWithReuseColumn' nameSpacePrefix='rc' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
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
        "        <ECNavigationProperty propertyName='A1' relationshipName='RelationshipClassA' direction='Backward'/>"
        "        <ECNavigationProperty propertyName='A2' relationshipName='RelationshipClassB' direction='Backward'/>"
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
        "</ECSchema>")));

    ECInstanceKey key1, key2, key3, key4, key5;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(key1, "INSERT INTO rc.ClassA (P1) VALUES('classA')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(key2, "INSERT INTO rc.ClassAB (P1, P2) VALUES('ClassA', 1001.01)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(key3, "INSERT INTO rc.ClassC (P3) VALUES(1)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(key4, "INSERT INTO rc.ClassC (P3) VALUES(2)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(key5, "INSERT INTO rc.ClassD (P4) VALUES(4)"));

    ECSqlStatement stmt;
    ASSERT_EQ(stmt.Prepare(m_ecdb, "UPDATE rc.ClassC SET A1.Id=? WHERE ECInstanceId = ?"), ECSqlStatus::Success);
    stmt.BindId(1, key1.GetInstanceId());
    stmt.BindId(2, key3.GetInstanceId());
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    stmt.Finalize();

    ASSERT_EQ(stmt.Prepare(m_ecdb, "UPDATE rc.ClassC SET A1.Id=? WHERE ECInstanceId = ?"), ECSqlStatus::Success);
    stmt.BindId(1, key1.GetInstanceId());
    stmt.BindId(2, key4.GetInstanceId());
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    stmt.Finalize();

    ASSERT_EQ(stmt.Prepare(m_ecdb, "UPDATE rc.ClassC SET A2.Id=? WHERE ECInstanceId = ?"), ECSqlStatus::Success);
    stmt.BindId(1, key1.GetInstanceId());
    stmt.BindId(2, key3.GetInstanceId());
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    stmt.Finalize();

    ASSERT_EQ(stmt.Prepare(m_ecdb, "UPDATE rc.ClassC SET A2.Id=? WHERE ECInstanceId = ?"), ECSqlStatus::Success);
    stmt.BindId(1, key2.GetInstanceId());
    stmt.BindId(2, key4.GetInstanceId());
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    stmt.Finalize();

    ASSERT_EQ(stmt.Prepare(m_ecdb, "INSERT INTO rc.RelationshipClassC (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)"), ECSqlStatus::Success);
    stmt.BindId(1, key1.GetInstanceId());
    stmt.BindId(2, key1.GetClassId());
    stmt.BindId(3, key3.GetInstanceId());
    stmt.BindId(4, key3.GetClassId());
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    stmt.Finalize();
    //Instance insertion query without specifing Souce/TargetClassId's shouldn't work for a link table relationship if constraint isn't a single table or isn't polymorphic Constraint
    ASSERT_EQ(stmt.Prepare(m_ecdb, "INSERT INTO rc.RelationshipClassC (SourceECInstanceId, TargetECInstanceId) VALUES (?, ?)"), ECSqlStatus::Success);
    stmt.Finalize();

    //Instance insertion query without specifing Souce/TargetClassId's should work for a link table relationship if each constraint is a single table
    ASSERT_EQ(stmt.Prepare(m_ecdb, "INSERT INTO rc.RelationshipClassD (SourceECInstanceId, TargetECInstanceId) VALUES (?, ?)"), ECSqlStatus::Success);
    stmt.BindId(1, key4.GetInstanceId());
    stmt.BindId(2, key5.GetInstanceId());
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  09/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceInserterTests, InsertTimeOfDayValues)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("InsertTimeOfDayValues.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.01" alias="CoreCA"/>
            <ECEntityClass typeName="CalendarEntry" modifier="None">
                <ECProperty propertyName="StartTime" typeName="dateTime">
                    <ECCustomAttributes>
                        <DateTimeInfo xmlns="CoreCustomAttributes.01.00.01">
                            <DateTimeComponent>TimeOfDay</DateTimeComponent>
                        </DateTimeInfo>
                    </ECCustomAttributes>
                </ECProperty>
                <ECProperty propertyName="EndTime" typeName="dateTime">
                    <ECCustomAttributes>
                        <DateTimeInfo xmlns="CoreCustomAttributes.01.00.01">
                            <DateTimeComponent>TimeOfDay</DateTimeComponent>
                        </DateTimeInfo>
                    </ECCustomAttributes>
                </ECProperty>
            </ECEntityClass>
        </ECSchema>)xml")));

    ECClassCP calendarEntryClass = m_ecdb.Schemas().GetClass("TestSchema", "CalendarEntry");
    ASSERT_TRUE(calendarEntryClass != nullptr);
    IECInstancePtr inst = calendarEntryClass->GetDefaultStandaloneEnabler()->CreateInstance();

    ECValue val(DateTime::CreateTimeOfDay(8, 0));
    ASSERT_EQ(ECObjectsStatus::Success, inst->SetValue("StartTime", val));
    val = ECValue(DateTime::CreateTimeOfDay(17, 30, 45, 500));
    ASSERT_EQ(ECObjectsStatus::Success, inst->SetValue("EndTime", val));

    ECInstanceKey key1, key2;

    {
    ECInstanceInserter inserter(m_ecdb, *calendarEntryClass, nullptr);
    ASSERT_TRUE(inserter.IsValid());
    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(key1, *inst));

    inst = calendarEntryClass->GetDefaultStandaloneEnabler()->CreateInstance();

    val = ECValue(DateTime::CreateTimeOfDay(0, 0));
    ASSERT_EQ(ECObjectsStatus::Success, inst->SetValue("StartTime", val));
    val = ECValue(DateTime::CreateTimeOfDay(24, 0));
    ASSERT_EQ(ECObjectsStatus::Success, inst->SetValue("EndTime", val));
    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(key2, *inst));
    }

    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

    EXPECT_EQ(JsonValue("[{\"StartTime\": \"08:00:00.000\", \"EndTime\":\"17:30:45.500\"}]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT StartTime,EndTime FROM ts.CalendarEntry WHERE ECInstanceId=%s", key1.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[{\"StartTime\": \"00:00:00.000\", \"EndTime\":\"00:00:00.000\"}]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT StartTime,EndTime FROM ts.CalendarEntry WHERE ECInstanceId=%s", key2.GetInstanceId().ToString().c_str()).c_str()));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId,StartTime,EndTime FROM ts.CalendarEntry"));
    ECInstanceECSqlSelectAdapter adapter(stmt);
    ASSERT_TRUE(adapter.IsValid());
    auto assertTimeOfDay = [] (DateTime const& expectedStartTime, DateTime const& expectedEndTime, IECInstanceCR instance)
        {
        ECValue v;
        ASSERT_EQ(ECObjectsStatus::Success, instance.GetValue(v, "StartTime"));
        DateTime actualStartTime = v.GetDateTime();
        EXPECT_TRUE(actualStartTime.IsTimeOfDay());
        ASSERT_EQ(expectedStartTime, actualStartTime.GetTimeOfDay());

        ASSERT_EQ(ECObjectsStatus::Success, instance.GetValue(v, "EndTime"));
        DateTime actualEndTime = v.GetDateTime();
        EXPECT_TRUE(actualStartTime.IsTimeOfDay());
        ASSERT_EQ(expectedEndTime, actualEndTime.GetTimeOfDay());
        };

    while (stmt.Step() == BE_SQLITE_ROW)
        {
        IECInstancePtr inst = adapter.GetInstance();
        ASSERT_TRUE(inst != nullptr);
        if (inst->GetInstanceId().EqualsIAscii(key1.GetInstanceId().ToString()))
            assertTimeOfDay(DateTime::CreateTimeOfDay(8, 0), DateTime::CreateTimeOfDay(17, 30, 45, 500), *inst);
        else
            assertTimeOfDay(DateTime::CreateTimeOfDay(0, 0), DateTime::CreateTimeOfDay(0, 0), *inst); // 24:00 times can only be passed, but not read out as that will always amout to 00:00
        }
    }

//---------------------------------------------------------------------------------------
// Test for TFS 112251, the Adapter should check for the class before operation
// @bsimethod                                   Majd.Uddin                   08/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlAdapterTestFixture, CheckClassBeforeOperation)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("StartupCompany.ecdb", SchemaItem::CreateForFile("StartupCompany.02.00.00.ecschema.xml")));

    //Get two classes and create instance of second
    ECClassCP employee = m_ecdb.Schemas().GetClass("StartupCompany", "Employee");
    ASSERT_TRUE(employee != nullptr);

    ECClassCP project = m_ecdb.Schemas().GetClass("StartupCompany", "Project");
    ASSERT_TRUE(project != nullptr);
    IECInstancePtr projectInstance = project->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstancePopulator::Populate(*projectInstance);

    //ECInstance Adapters
    ECInstanceInserter inserter(m_ecdb, *employee, nullptr);
    ECInstanceKey instanceKey;
    ASSERT_EQ(BE_SQLITE_ERROR, inserter.Insert(instanceKey, *projectInstance));

    ECInstanceUpdater updater(m_ecdb, *employee, nullptr);
    ASSERT_EQ(BE_SQLITE_ERROR, updater.Update(*projectInstance));

    ECInstanceDeleter deleter(m_ecdb, *employee, nullptr);
    ASSERT_EQ(BE_SQLITE_ERROR, deleter.Delete(*projectInstance));

    //Json Adapters
    // Read JSON input from file
    BeFileName jsonInputFile;
    BeTest::GetHost().GetDocumentsRoot(jsonInputFile);
    jsonInputFile.AppendToPath(L"ECDb");
    jsonInputFile.AppendToPath(L"JsonTestClass.json");

    ScopedDisableFailOnAssertion disableFailOnAssertion;
    // Parse JSON value using JsonCpp
    Json::Value jsonInput;
    ASSERT_EQ(SUCCESS, TestUtilities::ReadFile(jsonInput, jsonInputFile));

    JsonInserter jsonInserter(m_ecdb, *employee, nullptr);
    ASSERT_EQ(BE_SQLITE_ERROR, jsonInserter.Insert(instanceKey, jsonInput));

    JsonUpdater jsonUpdater(m_ecdb, *employee, nullptr);
    ASSERT_EQ(BE_SQLITE_ERROR, jsonUpdater.Update(instanceKey.GetInstanceId(), jsonInput));
    }


END_ECDBUNITTESTS_NAMESPACE
