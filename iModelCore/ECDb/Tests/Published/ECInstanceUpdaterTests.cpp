/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECInstanceUpdaterTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE
struct ECInstanceUpdaterTests : ECDbTestFixture
    {};

struct ECInstanceUpdaterAgainstPrimitiveClassTests : ECInstanceUpdaterTests
    {
    protected:
        void UpdateInstances(Utf8CP className, Utf8CP schemaName, int numberOfInstances, bool populateAllProperties)
            {
            ASSERT_EQ(SUCCESS, SetupECDb("updateInstances.ecdb", SchemaItem::CreateForFile("KitchenSink.01.00.00.ecschema.xml")));
            ECClassCP testClass = m_ecdb.Schemas().GetClass(schemaName, className);

            ECInstanceInserter inserter(m_ecdb, *testClass, nullptr);
            ECInstanceUpdater* updater = nullptr;
            for (int i = 0; i < numberOfInstances; i++)
                {
                IECInstancePtr instance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
                ECInstancePopulator::Populate(*instance);
                ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(*instance));
                
                IECInstancePtr updatedInstance;
                if (populateAllProperties)
                    {
                    updatedInstance = instance->CreateCopyThroughSerialization();
                    ECInstancePopulator::Populate(*updatedInstance);
                    }
                else
                    {
                    updatedInstance = testClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
                    ECValue v;
                    v.SetLong(101);
                    updatedInstance->SetValue("LongMember", v);
                    instance->GetValue(v, "BooleanMember");
                    updatedInstance->SetValue("BooleanMember", ECValue(!(v.GetBoolean())));
                    // updatedInstance->SetValue("DoubleMember", ECValue(3.1415));
                    }
                updatedInstance->SetInstanceId(instance->GetInstanceId().c_str());

                if (nullptr == updater)
                    {
                    if (populateAllProperties)
                        updater = new ECInstanceUpdater(m_ecdb, *testClass, nullptr);
                    else
                        updater = new ECInstanceUpdater(m_ecdb, *updatedInstance, nullptr);
                    }

                if (testClass->GetPropertyCount() == 0)
                    {
                    ASSERT_FALSE(updater->IsValid());
                    return;
                    }
                else
                    ASSERT_TRUE(updater->IsValid());

                ASSERT_EQ(BE_SQLITE_OK, updater->Update(*updatedInstance));

                SqlPrintfString ecSql("SELECT c0.[ECInstanceId], c0.ECClassId, * FROM %s.%s c0 WHERE ECInstanceId=%s", Utf8String(schemaName).c_str(), Utf8String(className).c_str(), Utf8String(instance->GetInstanceId()).c_str());
                ECSqlStatement statement;
                ECSqlStatus prepareStatus = statement.Prepare(m_ecdb, ecSql.GetUtf8CP());
                ECInstanceECSqlSelectAdapter dataAdapter(statement);
                ASSERT_TRUE(ECSqlStatus::Success == prepareStatus);
                DbResult result;

                instance->GetAsMemoryECInstanceP()->MergePropertiesFromInstance(*updatedInstance);

                while ((result = statement.Step()) == BE_SQLITE_ROW)
                    {
                    IECInstancePtr selectedInstance = dataAdapter.GetInstance();

                    Json::Value expectedInstanceJson, selectedInstanceJson;
                    ASSERT_EQ(SUCCESS, JsonEcInstanceWriter::WriteInstanceToJson(expectedInstanceJson, *instance, nullptr, true));
                    ASSERT_EQ(SUCCESS, JsonEcInstanceWriter::WriteInstanceToJson(selectedInstanceJson, *selectedInstance, nullptr, true));
                    ASSERT_EQ(JsonValue(expectedInstanceJson), JsonValue(selectedInstanceJson)) << "Updated instance from ecdb not as expected.";
                    }
                }
            delete updater;
            }
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest                                   Carole.MacDonald                     06/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECInstanceUpdaterAgainstPrimitiveClassTests, UpdateSingleInstanceOfPrimitiveClass)
    {
    UpdateInstances("PrimitiveClass", "KitchenSink", 1, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                   Carole.MacDonald                     06/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECInstanceUpdaterAgainstPrimitiveClassTests, UpdateSingleInstanceOfPrimitiveClassWithIncompleteInstance)
    {
    UpdateInstances("PrimitiveClass", "KitchenSink", 1, false);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceUpdaterTests, UpdateRelationships)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("UpdateRelationships.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    ECClassCP navPropRelClass = m_ecdb.Schemas().GetClass("ECSqlTest", "PSAHasP_N1");
    ASSERT_TRUE(navPropRelClass != nullptr);
    ECInstanceUpdater updater(m_ecdb, *navPropRelClass, nullptr);
    ASSERT_FALSE(updater.IsValid()) << "Cannot update nav prop relationship class " << navPropRelClass->GetFullName();

    ECClassCP linkTableRelClass = m_ecdb.Schemas().GetClass("ECSqlTest", "PSAHasPSA_NN");
    ASSERT_TRUE(linkTableRelClass != nullptr);

    ECInstanceUpdater updater2(m_ecdb, *linkTableRelClass, nullptr);
    ASSERT_TRUE(updater2.IsValid()) << "Expected to be able to update into link table relationship class " << linkTableRelClass->GetFullName();
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Muhammad.Zaighum                  01/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceUpdaterTests, UpdateWithCurrentTimeStampTrigger)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("updatewithcurrenttimestamptrigger.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));
    ECClassCP testClass = m_ecdb.Schemas().GetClass("ECSqlTest", "ClassWithLastModProp");
    ASSERT_TRUE(testClass != nullptr);

    auto tryGetLastMod = [] (DateTime& lastMod, ECDbR ecdb, ECInstanceId id)
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT LastMod FROM ecsql.ClassWithLastModProp WHERE ECInstanceId=?"));

        stmt.BindId(1, id);
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_FALSE(stmt.IsValueNull(0));

        lastMod = stmt.GetValueDateTime(0);
        };

    //insert test instance
    StandaloneECInstancePtr testInstance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();

    ECValue v(1);
    ASSERT_EQ(ECObjectsStatus::Success, testInstance->SetValue("I", v));

    v.Clear();
    v.SetUtf8CP("John");
    ASSERT_EQ(ECObjectsStatus::Success, testInstance->SetValue("FirstName", v));
    v.Clear();
    v.SetUtf8CP("Smith");
    ASSERT_EQ(ECObjectsStatus::Success, testInstance->SetValue("LastName", v));

    ECInstanceId testId;
    {
    ECInstanceInserter inserter(m_ecdb, *testClass, nullptr);
    ASSERT_TRUE(inserter.IsValid());
    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(*testInstance));
    ASSERT_EQ(SUCCESS, ECInstanceId::FromString(testId, testInstance->GetInstanceId().c_str()));
    }

    DateTime firstLastMod;
    tryGetLastMod(firstLastMod, m_ecdb, testId);
    ASSERT_TRUE(firstLastMod.IsValid());

    //now update an ECInstance
    BeThreadUtilities::BeSleep(1000); //so that new last mod differs significantly from old last mod
    v.Clear();

    v.SetInteger(2);
    ASSERT_EQ(ECObjectsStatus::Success, testInstance->SetValue("I", v));

    ECInstanceUpdater updater(m_ecdb, *testClass, nullptr);
    ASSERT_TRUE(updater.IsValid());
    ASSERT_EQ(BE_SQLITE_OK, updater.Update(*testInstance));

    DateTime newLastMod;
    tryGetLastMod(newLastMod, m_ecdb, testId);
    ASSERT_TRUE(newLastMod.IsValid());

    uint64_t firstLastModJdMsec = 0ULL;
    ASSERT_EQ(SUCCESS, firstLastMod.ToJulianDay(firstLastModJdMsec));

    uint64_t newLastModJdMsec = 0ULL;
    ASSERT_EQ(SUCCESS, newLastMod.ToJulianDay(newLastModJdMsec));

    uint64_t timeSpan = newLastModJdMsec - firstLastModJdMsec;
    ASSERT_GT(timeSpan, 100) << "New LastMod must be later than old LastMod. Just test that it is a bit older (versus the exact difference) to keep the test robust.";
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Krischan.Eberle                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceUpdaterTests, ReadonlyAndCalculatedProperties)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("updatereadonlyproperty.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                                     "<ECSchema schemaName='testSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                                     "   <ECSchemaReference version='01.12' alias='bsca' name='Bentley_Standard_CustomAttributes'/>"
                                                                     "    <ECEntityClass typeName='A' >"
                                                                     "        <ECProperty propertyName='RInt' typeName='int' readOnly='True'/>"
                                                                     "        <ECProperty propertyName='RString' typeName='string' readOnly='True'/>"
                                                                     "        <ECProperty propertyName='Length' typeName='int' />"
                                                                     "        <ECProperty propertyName='Square' typeName='string' >"
                                                                     "          <ECCustomAttributes>"
                                                                     "            <CalculatedECPropertySpecification xmlns='Bentley_Standard_CustomAttributes.01.12'>"
                                                                     "              <ECExpression>this.Length * this.Length</ECExpression>"
                                                                     "              <FailureValue>-</FailureValue>"
                                                                     "            </CalculatedECPropertySpecification>"
                                                                     "          </ECCustomAttributes>"
                                                                     "        </ECProperty>"
                                                                     "    </ECEntityClass>"
                                                                     "</ECSchema>")));

    const int oldRInt = 100;
    const int newRInt = 200;
    Utf8CP oldRString = "old";
    Utf8CP newRString = "new";
    const int oldLength = 2;
    Utf8CP oldSquare = "4";
    const int newLength = 3;
    Utf8CP newSquare = "9";

    ECInstanceKey key;

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.A (RInt, RString, Length, Square) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, oldRInt));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, oldRString, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(3, oldLength));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(4, oldSquare, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step(key));
    }

    ECClassCP ecClass = m_ecdb.Schemas().GetClass("testSchema", "A");
    ASSERT_TRUE(ecClass != nullptr);

    IECInstancePtr updatedInstance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
    Utf8Char idStrBuffer[BeInt64Id::ID_STRINGBUFFER_LENGTH];
    key.GetInstanceId().ToString(idStrBuffer);
    updatedInstance->SetInstanceId(idStrBuffer);

    ECValue v;
    v.SetInteger(newRInt);
    ASSERT_EQ(ECObjectsStatus::Success, updatedInstance->SetValue("RInt", v));

    v.Clear();
    v.SetUtf8CP(newRString);
    ASSERT_EQ(ECObjectsStatus::Success, updatedInstance->SetValue("RString", v));

    v.Clear();
    v.SetInteger(newLength);
    ASSERT_EQ(ECObjectsStatus::Success, updatedInstance->SetValue("Length", v));

    //calc prop gets reevaluated automatically, so no need to set it here

    //Be default readonly props cannot be updated, so updater skips readonly props
    ECInstanceUpdater updater(m_ecdb, *ecClass, nullptr);
    ASSERT_TRUE(updater.IsValid());

    ECSqlStatement validateStmt;
    ASSERT_EQ(ECSqlStatus::Success, validateStmt.Prepare(m_ecdb, "SELECT RInt, RString, Length, Square FROM ts.A WHERE ECInstanceId=?"));

    {
    Savepoint sp(m_ecdb, "default updater");

    ASSERT_EQ(BE_SQLITE_OK, updater.Update(*updatedInstance));

    ASSERT_EQ(ECSqlStatus::Success, validateStmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, validateStmt.Step());

    ASSERT_EQ(oldRInt, validateStmt.GetValueInt(0)) << "Readonly property RInt is expected to not be modified";
    ASSERT_STREQ(oldRString, validateStmt.GetValueText(1)) << "Readonly property RInt is expected to not be modified";
    ASSERT_EQ(newLength, validateStmt.GetValueInt(2)) << "Read-write property Length is expected to be modified";
    ASSERT_STREQ(newSquare, validateStmt.GetValueText(3)) << "Calculated property Square is expected to be modified";
    validateStmt.Reset();
    validateStmt.ClearBindings();

    sp.Cancel();
    }

    {
    //now use updater with option to update readonly props
    ECInstanceUpdater readonlyUpdater(m_ecdb, *ecClass, nullptr, "ReadonlyPropertiesAreUpdatable");
    ASSERT_TRUE(readonlyUpdater.IsValid());
    ASSERT_EQ(BE_SQLITE_OK, readonlyUpdater.Update(*updatedInstance));

    ASSERT_EQ(ECSqlStatus::Success, validateStmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, validateStmt.Step());

    ASSERT_EQ(newRInt, validateStmt.GetValueInt(0)) << "Readonly property RInt is expected to be modified";
    ASSERT_STREQ(newRString, validateStmt.GetValueText(1)) << "Readonly property RInt is expected to be modified";
    ASSERT_EQ(newLength, validateStmt.GetValueInt(2)) << "Read-write property Length is expected to be modified";
    ASSERT_STREQ(newSquare, validateStmt.GetValueText(3)) << "Calculated property Square is expected to be modified";
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceUpdaterTests, UpdaterBasedOnListOfPropertyIndices)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("updaterbasesonparameterindices.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='testSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECEntityClass typeName='A' >"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "        <ECProperty propertyName='P2' typeName='string' />"
        "        <ECProperty propertyName='P3' typeName='long' />"
        "    </ECEntityClass>"
        "</ECSchema>")));

    ECInstanceKey key;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.A (P1,P2,P3) VALUES(100, 'old', 1000)"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step(key));
    }

    ECClassCP ecClass = m_ecdb.Schemas().GetClass("testSchema", "A");
    ASSERT_TRUE(ecClass != nullptr);

    bvector<uint32_t> propertiesToBind {1, 2, 3};
    ECInstanceUpdater instanceUpdater(m_ecdb, *ecClass, nullptr, propertiesToBind);
    ASSERT_TRUE(instanceUpdater.IsValid());

    //new values for properties
    const int newP1Value = 200;
    Utf8CP newP2Value = "new";
    const int64_t newP3Value = INT64_C(2000);

    //create New Instance
    IECInstancePtr updatedInstance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
    Utf8Char idStrBuffer[BeInt64Id::ID_STRINGBUFFER_LENGTH];
    key.GetInstanceId().ToString(idStrBuffer);
    updatedInstance->SetInstanceId(idStrBuffer);

    ECValue v;
    v.SetInteger(newP1Value);
    ASSERT_EQ(ECObjectsStatus::Success, updatedInstance->SetValue("P1", v));

    v.Clear();
    v.SetUtf8CP(newP2Value);
    ASSERT_EQ(ECObjectsStatus::Success, updatedInstance->SetValue("P2", v));

    v.Clear();
    v.SetLong(newP3Value);
    ASSERT_EQ(ECObjectsStatus::Success, updatedInstance->SetValue("P3", v));

    //update inserted instance
    ASSERT_EQ(BE_SQLITE_OK, instanceUpdater.Update(*updatedInstance));

    //validate updated Instance
    Utf8String validateECSql;
    validateECSql.Sprintf("SELECT NULL FROM ts.A WHERE ECInstanceId=%s AND P1=%d AND P2 LIKE '%s' AND P3=%" PRId64,
                          key.GetInstanceId().ToString().c_str(), newP1Value, newP2Value, newP3Value);

    ECSqlStatement validateStmt;
    ASSERT_EQ(ECSqlStatus::Success, validateStmt.Prepare(m_ecdb, validateECSql.c_str()));
    ASSERT_EQ(BE_SQLITE_ROW, validateStmt.Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceUpdaterTests, UpdaterBasedOnListOfPropertiesToBind)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("updaterbasesonlistofproperties.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='testSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECEntityClass typeName='A' >"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "        <ECProperty propertyName='P2' typeName='string' />"
        "        <ECProperty propertyName='P3' typeName='long' />"
        "    </ECEntityClass>"
        "</ECSchema>")));

    ECInstanceKey key;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.A (P1,P2,P3) VALUES(100, 'old', 1000)"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step(key));
    }

    ECClassCP ecClass = m_ecdb.Schemas().GetClass("testSchema", "A");
    ASSERT_TRUE(ecClass != nullptr);

    ECPropertyCP p1 = ecClass->GetPropertyP("P1");
    ASSERT_TRUE(p1 != nullptr);

    ECPropertyCP p2 = ecClass->GetPropertyP("P2");
    ASSERT_TRUE(p2 != nullptr);

    ECPropertyCP p3 = ecClass->GetPropertyP("P3");
    ASSERT_TRUE(p3 != nullptr);

    bvector<ECPropertyCP> propertiesToBind {p1, p2, p3};
    ECInstanceUpdater instanceUpdater(m_ecdb, *ecClass, nullptr, propertiesToBind);
    ASSERT_TRUE(instanceUpdater.IsValid());

    //new values for properties
    const int newP1Value = 200;
    Utf8CP newP2Value = "new";
    const int64_t newP3Value = INT64_C(2000);

    //create New Instance
    IECInstancePtr updatedInstance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
    Utf8Char idStrBuffer[BeInt64Id::ID_STRINGBUFFER_LENGTH];
    key.GetInstanceId().ToString(idStrBuffer);
    updatedInstance->SetInstanceId(idStrBuffer);

    ECValue v;
    v.SetInteger(newP1Value);
    ASSERT_EQ(ECObjectsStatus::Success, updatedInstance->SetValue("P1", v));

    v.Clear();
    v.SetUtf8CP(newP2Value);
    ASSERT_EQ(ECObjectsStatus::Success, updatedInstance->SetValue("P2", v));

    v.Clear();
    v.SetLong(newP3Value);
    ASSERT_EQ(ECObjectsStatus::Success, updatedInstance->SetValue("P3", v));

    //update inserted instance
    ASSERT_EQ(BE_SQLITE_OK, instanceUpdater.Update(*updatedInstance));

    //validate updated Instance
    Utf8String validateECSql;
    validateECSql.Sprintf("SELECT NULL FROM ts.A WHERE ECInstanceId=%s AND P1=%d AND P2 LIKE '%s' AND P3=%" PRId64,
                          key.GetInstanceId().ToString().c_str(), newP1Value, newP2Value, newP3Value);

    ECSqlStatement validateStmt;
    ASSERT_EQ(ECSqlStatus::Success, validateStmt.Prepare(m_ecdb, validateECSql.c_str()));
    ASSERT_EQ(BE_SQLITE_ROW, validateStmt.Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceUpdaterTests, UpdateArrayProperty)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("updateArrayProperty.ecdb", SchemaItem::CreateForFile("KitchenSink.01.00.00.ecschema.xml")));

    ECN::ECClassCP testClass = m_ecdb.Schemas().GetClass("KitchenSink", "TestClass");
    ASSERT_TRUE(testClass != nullptr);

    StandaloneECEnablerPtr testEnabler = testClass->GetDefaultStandaloneEnabler();
    ECN::StandaloneECInstancePtr testInstance = testEnabler->CreateInstance();
    testInstance->AddArrayElements("SmallIntArray", 3);
    ECValue v;
    v.SetInteger(0);
    testInstance->SetValue("SmallIntArray", v, 0);
    v.SetInteger(1);
    testInstance->SetValue("SmallIntArray", v, 1);
    v.SetInteger(2);
    testInstance->SetValue("SmallIntArray", v, 2);

    ECInstanceInserter inserter(m_ecdb, *testClass, nullptr);
    ASSERT_TRUE(inserter.IsValid());
    ECInstanceKey instanceKey;
    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(instanceKey, *testInstance));

    Utf8CP ecSql = "SELECT ECInstanceId, ECClassId, SmallIntArray FROM KitchenSink.TestClass";
    ECSqlStatement ecStatement;
    ECSqlStatus status = ecStatement.Prepare(m_ecdb, ecSql);
    ASSERT_TRUE(ECSqlStatus::Success == status);
    ECInstanceECSqlSelectAdapter dataAdapter(ecStatement);

    ASSERT_TRUE(BE_SQLITE_ROW == ecStatement.Step());
    IECInstancePtr selectedInstance = dataAdapter.GetInstance();
    ASSERT_TRUE(selectedInstance.IsValid());

    ASSERT_TRUE(ECObjectsStatus::Success == selectedInstance->GetValue(v, "SmallIntArray"));
    ASSERT_TRUE(v.IsArray());
    ArrayInfo info = v.GetArrayInfo();
    ASSERT_EQ(3, info.GetCount());

    selectedInstance->ClearArray("SmallIntArray");
    selectedInstance->AddArrayElements("SmallIntArray", 2);
    v.SetInteger(0);
    selectedInstance->SetValue("SmallIntArray", v, 0);
    v.SetInteger(1);
    selectedInstance->SetValue("SmallIntArray", v, 1);

    ECInstanceUpdater updater(m_ecdb, *testClass, nullptr);
    ASSERT_TRUE(updater.IsValid());
    ASSERT_EQ(BE_SQLITE_OK, updater.Update(*selectedInstance));

    ECSqlStatement ecStatement2;
    ECSqlStatus status2 = ecStatement2.Prepare(m_ecdb, ecSql);
    ASSERT_TRUE(ECSqlStatus::Success == status2);
    ECInstanceECSqlSelectAdapter dataAdapter2(ecStatement2);

    ASSERT_TRUE(BE_SQLITE_ROW == ecStatement2.Step());
    IECInstancePtr updatedInstance = dataAdapter2.GetInstance();
    ASSERT_TRUE(updatedInstance.IsValid());

    ASSERT_TRUE(ECObjectsStatus::Success == updatedInstance->GetValue(v, "SmallIntArray"));
    ASSERT_TRUE(v.IsArray());
    ArrayInfo info2 = v.GetArrayInfo();
    ASSERT_EQ(2, info2.GetCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
//Failing case, if number of property indices less then 1, updater should be invalid
TEST_F(ECInstanceUpdaterTests, InvalidListOfPropertyIndices)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("updaterbasesonparameterindices.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='testSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECEntityClass typeName='A'>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>")));

    ECClassCP ecClass = m_ecdb.Schemas().GetClass("testSchema", "A");
    ASSERT_TRUE(ecClass != nullptr);

    bvector<uint32_t> propertiesToBind {};
    ECInstanceUpdater instanceUpdater(m_ecdb, *ecClass, nullptr, propertiesToBind);
    ASSERT_FALSE(instanceUpdater.IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
//Failing case, Update should fail it updater is invalid
TEST_F(ECInstanceUpdaterTests, InvalidUpdater)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("updaterbasesonparameterindices.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='testSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECEntityClass typeName='A' >"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>")));

    ECClassCP ecClass = m_ecdb.Schemas().GetClass("testSchema", "A");
    ASSERT_TRUE(ecClass != nullptr);

    bvector<uint32_t> propertiesToBind {};
    ECInstanceUpdater instanceUpdater(m_ecdb, *ecClass, nullptr, propertiesToBind);
    ASSERT_FALSE(instanceUpdater.IsValid());

    IECInstancePtr updatedInstance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();

    ASSERT_EQ(BE_SQLITE_ERROR, instanceUpdater.Update(*updatedInstance));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceUpdaterTests, LargeNumbersOfPropertiesMappingToOverflow)
    {
    std::vector<int> propCounts {60, 63, 64, 70, 126, 127, 140, 189, 190, 210, 280};
    for (int propCount : propCounts)
        {
        Utf8String propertiesXml;
        for (int i = 0; i < propCount; i++)
            {
            Utf8String propertyXml;
            propertyXml.Sprintf("<ECProperty propertyName='Prop%d' typeName='string'/>", i + 1);
            propertiesXml.append(propertyXml);
            }


        Utf8String ecschemaXml;
        ecschemaXml.Sprintf("<?xml version='1.0' encoding='utf-8'?>"
                            "<ECSchema schemaName='testSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                            "    <ECEntityClass typeName='ClassWithPrimitiveProps'>"
                            "        <ECCustomAttributes>"
                            "           <ClassMap xmlns='ECDbMap.02.00'>"
                            "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                            "            </ClassMap>"
                            "           <ShareColumns xmlns='ECDbMap.02.00'>"
                            "           </ShareColumns>"
                            "        </ECCustomAttributes>"
                            "%s"
                            "    </ECEntityClass>"
                            "    <ECStructClass typeName='LargeStruct'>"
                            "%s"
                            "    </ECStructClass>"
                            "    <ECEntityClass typeName='ClassWithStruct'>"
                            "        <ECCustomAttributes>"
                            "           <ClassMap xmlns='ECDbMap.02.00'>"
                            "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                            "            </ClassMap>"
                            "           <ShareColumns xmlns='ECDbMap.02.00'>"
                            "           </ShareColumns>"
                            "        </ECCustomAttributes>"
                            "        <ECStructProperty propertyName='MyStructProp' typeName='LargeStruct'/>"
                            "    </ECEntityClass>"
                            "</ECSchema>", propertiesXml.c_str(), propertiesXml.c_str());

        Utf8String fileName;
        fileName.Sprintf("ecinstanceupdater_%d_propsmappedtooverflow.ecdb", propCount);

        if (m_ecdb.IsDbOpen())
            m_ecdb.CloseDb();

        ASSERT_EQ(SUCCESS, SetupECDb(fileName.c_str(), SchemaItem(ecschemaXml.c_str())));

        ECClassCP classWithPrims = m_ecdb.Schemas().GetClass("testSchema", "ClassWithPrimitiveProps");
        ASSERT_TRUE(classWithPrims != nullptr);

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.ClassWithPrimitiveProps(ECInstanceId) VALUES(NULL)")) << "Prop count: " << propCount;
        ECInstanceKey classWithPrimsKey;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(classWithPrimsKey)) << "Prop count: " << propCount;
        stmt.Finalize();

        ECInstanceUpdater classWithPrimsUpdater(m_ecdb, *classWithPrims, nullptr);
        ASSERT_TRUE(classWithPrimsUpdater.IsValid()) << "ECClass: " << classWithPrims->GetName().c_str() << " Prop count: " << propCount << m_ecdb.GetLastError().c_str();

        IECInstancePtr instance = classWithPrims->GetDefaultStandaloneEnabler()->CreateInstance();
        ASSERT_EQ(ECObjectsStatus::Success, instance->SetInstanceId(classWithPrimsKey.GetInstanceId().ToString().c_str())) << "ECClass: " << classWithPrims->GetName().c_str() << " Prop count: " << propCount;
        ASSERT_EQ(BE_SQLITE_OK, classWithPrimsUpdater.Update(*instance)) << "ECClass: " << classWithPrims->GetName().c_str() << " Prop count: " << propCount;

        ECClassCP classWithStruct = m_ecdb.Schemas().GetClass("testSchema", "ClassWithStruct");
        ASSERT_TRUE(classWithStruct != nullptr);

        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.ClassWithStruct(ECInstanceId) VALUES(NULL)")) << "Prop count: " << propCount;
        ECInstanceKey classWithStructKey;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(classWithStructKey)) << "Prop count: " << propCount;
        stmt.Finalize();

        ECInstanceUpdater classWithStructUpdater(m_ecdb, *classWithStruct, nullptr);
        instance = classWithStruct->GetDefaultStandaloneEnabler()->CreateInstance();
        ASSERT_EQ(ECObjectsStatus::Success, instance->SetInstanceId(classWithStructKey.GetInstanceId().ToString().c_str())) << "ECClass: " << classWithStruct->GetName().c_str() << " Prop count: " << propCount;

        ASSERT_EQ(BE_SQLITE_OK, classWithStructUpdater.Update(*instance)) << "ECClass: " << classWithStruct->GetName().c_str() << "Prop count: " << propCount;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  09/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceUpdaterTests, UpdateTimeOfDayValues)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("UpdateTimeOfDayValues.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
            <ECEntityClass typeName="CalendarEntry" modifier="None">
                <ECProperty propertyName="StartTime" typeName="dateTime">
                </ECProperty>
                <ECProperty propertyName="EndTime" typeName="dateTime" />
            </ECEntityClass>
        </ECSchema>)xml")));

    ECInstanceKey key1, key2;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(key1, "INSERT INTO ts.CalendarEntry(StartTime,EndTime) VALUES(TIME '08:00', TIME '17:30:45.500')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(key2, "INSERT INTO ts.CalendarEntry(StartTime,EndTime) VALUES(TIME '00:00', TIME '24:00')"));

    ECClassCP calendarEntryClass = m_ecdb.Schemas().GetClass("TestSchema", "CalendarEntry");
    ASSERT_TRUE(calendarEntryClass != nullptr);



    {

    IECInstancePtr inst = calendarEntryClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ECValue val(DateTime::CreateTimeOfDay(8, 30));
    ASSERT_EQ(ECObjectsStatus::Success, inst->SetValue("StartTime", val));
    val = ECValue(DateTime::CreateTimeOfDay(20, 0));
    ASSERT_EQ(ECObjectsStatus::Success, inst->SetValue("EndTime", val));
    inst->SetInstanceId(key1.GetInstanceId().ToString().c_str());

    ECInstanceUpdater updater(m_ecdb, *inst, nullptr);
    ASSERT_TRUE(updater.IsValid());

    ASSERT_EQ(BE_SQLITE_OK, updater.Update(*inst));
    }

    {
    // just update end date for the second instance
    IECInstancePtr inst = calendarEntryClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ECValue val(DateTime::CreateTimeOfDay(23, 59, 59, 999));
    ASSERT_EQ(ECObjectsStatus::Success, inst->SetValue("EndTime", val));
    inst->SetInstanceId(key2.GetInstanceId().ToString().c_str());
    ECInstanceUpdater updater(m_ecdb, *inst, nullptr);
    ASSERT_TRUE(updater.IsValid());
    ASSERT_EQ(BE_SQLITE_OK, updater.Update(*inst));
    }

    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

    // WIP: Once we can use the TimeOfDay component in the schema, we need to adjust the expected time strings
    EXPECT_EQ(JsonValue("[{\"StartTime\": \"2000-01-01T08:30:00.000\", \"EndTime\":\"2000-01-01T20:00:00.000\"}]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT StartTime,EndTime FROM ts.CalendarEntry WHERE ECInstanceId=%s", key1.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[{\"StartTime\": \"2000-01-01T00:00:00.000\", \"EndTime\":\"2000-01-01T23:59:59.999\"}]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT StartTime,EndTime FROM ts.CalendarEntry WHERE ECInstanceId=%s", key2.GetInstanceId().ToString().c_str()).c_str()));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId,StartTime,EndTime FROM ts.CalendarEntry"));
    ECInstanceECSqlSelectAdapter adapter(stmt);
    ASSERT_TRUE(adapter.IsValid());
    auto assertTimeOfDay = [] (DateTime const& expectedStartTime, DateTime const& expectedEndTime, IECInstanceCR instance)
        {
        ECValue v;
        ASSERT_EQ(ECObjectsStatus::Success, instance.GetValue(v, "StartTime"));
        DateTime actualStartTime = v.GetDateTime();
        EXPECT_FALSE(actualStartTime.IsTimeOfDay()) << "Schema has changed to use TimeOfDay CA";
        ASSERT_EQ(expectedStartTime, actualStartTime.GetTimeOfDay());

        ASSERT_EQ(ECObjectsStatus::Success, instance.GetValue(v, "EndTime"));
        DateTime actualEndTime = v.GetDateTime();
        EXPECT_FALSE(actualStartTime.IsTimeOfDay()) << "Schema has changed to use TimeOfDay CA";
        ASSERT_EQ(expectedEndTime, actualEndTime.GetTimeOfDay());
        };

    while (stmt.Step() == BE_SQLITE_ROW)
        {
        IECInstancePtr inst = adapter.GetInstance();
        ASSERT_TRUE(inst != nullptr);
        if (inst->GetInstanceId().EqualsIAscii(key1.GetInstanceId().ToString()))
            assertTimeOfDay(DateTime::CreateTimeOfDay(8, 30), DateTime::CreateTimeOfDay(20, 0), *inst);
        else
            assertTimeOfDay(DateTime::CreateTimeOfDay(0, 0), DateTime::CreateTimeOfDay(23, 59, 59, 999), *inst);
        }
    }

END_ECDBUNITTESTS_NAMESPACE
