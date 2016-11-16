/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECInstanceUpdaterTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
            ECDbR ecdb = SetupECDb("updateInstances.ecdb", BeFileName(L"KitchenSink.01.00.ecschema.xml"));
            ECClassCP testClass = ecdb.Schemas().GetECClass(schemaName, className);

            ECInstanceInserter inserter(ecdb, *testClass, nullptr);
            ECInstanceUpdater* updater = nullptr;
            for (int i = 0; i < numberOfInstances; i++)
                {
                IECInstancePtr instance = ECDbTestUtility::CreateArbitraryECInstance(*testClass, ECDbTestUtility::PopulatePrimitiveValueWithRandomValues);

                ASSERT_EQ(BE_SQLITE_DONE, inserter.Insert(*instance));
                IECInstancePtr updatedInstance;

                if (populateAllProperties)
                    {
                    updatedInstance = instance->CreateCopyThroughSerialization();
                    ECDbTestUtility::PopulateECInstance(updatedInstance, ECDbTestUtility::PopulatePrimitiveValueWithRandomValues);
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
                        updater = new ECInstanceUpdater(ecdb, *testClass, nullptr);
                    else
                        updater = new ECInstanceUpdater(ecdb, *updatedInstance, nullptr);
                    }

                if (testClass->GetPropertyCount() == 0)
                    {
                    ASSERT_FALSE(updater->IsValid());
                    return;
                    }
                else
                    ASSERT_TRUE(updater->IsValid());

                ASSERT_EQ(BE_SQLITE_DONE, updater->Update(*updatedInstance));

                SqlPrintfString ecSql("SELECT c0.[ECInstanceId], c0.ECClassId, * FROM %s.%s c0 WHERE ECInstanceId=%s", Utf8String(schemaName).c_str(), Utf8String(className).c_str(), Utf8String(instance->GetInstanceId()).c_str());
                ECSqlStatement statement;
                ECSqlStatus prepareStatus = statement.Prepare(ecdb, ecSql.GetUtf8CP());
                ECInstanceECSqlSelectAdapter dataAdapter(statement);
                ASSERT_TRUE(ECSqlStatus::Success == prepareStatus);
                DbResult result;

                instance->GetAsMemoryECInstanceP()->MergePropertiesFromInstance(*updatedInstance);
                while ((result = statement.Step()) == BE_SQLITE_ROW)
                    {
                    IECInstancePtr selectedInstance = dataAdapter.GetInstance();
                    bool equal = ECDbTestUtility::CompareECInstances(*instance, *selectedInstance);
                    ASSERT_TRUE(equal) << "Updated instance from ecdb not as expected.";
                    }
                }
            delete updater;
            }
    };

TEST_F(ECInstanceUpdaterAgainstPrimitiveClassTests, UpdateSingleInstanceOfPrimitiveClass)
    {
    UpdateInstances("PrimitiveClass", "KitchenSink", 1, true);
    }

TEST_F(ECInstanceUpdaterAgainstPrimitiveClassTests, UpdateSingleInstanceOfPrimitiveClassWithIncompleteInstance)
    {
    UpdateInstances("PrimitiveClass", "KitchenSink", 1, false);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Muhammad.Zaighum                  01/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceUpdaterTests, UpdateWithCurrentTimeStampTrigger)
    {
    ECDbR ecdb = SetupECDb("updatewithcurrenttimestamptrigger.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));
    ECClassCP testClass = ecdb.Schemas().GetECClass("ECSqlTest", "ClassWithLastModProp");
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
    ECInstanceInserter inserter(ecdb, *testClass, nullptr);
    ASSERT_TRUE(inserter.IsValid());
    ASSERT_EQ(BE_SQLITE_DONE, inserter.Insert(*testInstance));
    ASSERT_EQ(SUCCESS, ECInstanceId::FromString(testId, testInstance->GetInstanceId().c_str()));
    }

    DateTime firstLastMod;
    tryGetLastMod(firstLastMod, ecdb, testId);
    ASSERT_TRUE(firstLastMod.IsValid());

    //now update an ECInstance
    BeThreadUtilities::BeSleep(1000); //so that new last mod differs significantly from old last mod
    v.Clear();

    v.SetInteger(2);
    ASSERT_EQ(ECObjectsStatus::Success, testInstance->SetValue("I", v));

    ECInstanceUpdater updater(ecdb, *testClass, nullptr);
    ASSERT_TRUE(updater.IsValid());
    ASSERT_EQ(BE_SQLITE_DONE, updater.Update(*testInstance));

    DateTime newLastMod;
    tryGetLastMod(newLastMod, ecdb, testId);
    ASSERT_TRUE(newLastMod.IsValid());

    uint64_t firstLastModJdHns = 0ULL;
    ASSERT_EQ(SUCCESS, firstLastMod.ToJulianDay(firstLastModJdHns));

    uint64_t newLastModJdHns = 0ULL;
    ASSERT_EQ(SUCCESS, newLastMod.ToJulianDay(newLastModJdHns));

    uint64_t timeSpan = newLastModJdHns - firstLastModJdHns;
    const uint64_t timeSpan_1sec_in_hns = 5000000ULL;
    ASSERT_GT(timeSpan, timeSpan_1sec_in_hns) << "New LastMod must be at least 1 second later than old LastMod as test was paused for 1 sec before updating";
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Krischan.Eberle                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceUpdaterTests, ReadonlyAndCalculatedProperties)
    {
    ECDbR ecdb = SetupECDb("updatereadonlyproperty.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                                     "<ECSchema schemaName='testSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                                                     "   <ECSchemaReference version='01.12' prefix='bsca' name='Bentley_Standard_CustomAttributes'/>"
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
                                                                     "</ECSchema>"));

    ASSERT_TRUE(ecdb.IsDbOpen());

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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.A (RInt, RString, Length, Square) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, oldRInt));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, oldRString, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(3, oldLength));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(4, oldSquare, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step(key));
    }

    ECClassCP ecClass = ecdb.Schemas().GetECClass("testSchema", "A");
    ASSERT_TRUE(ecClass != nullptr);

    IECInstancePtr updatedInstance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
    Utf8Char idStrBuffer[BeInt64Id::ID_STRINGBUFFER_LENGTH];
    key.GetECInstanceId().ToString(idStrBuffer);
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
    ECInstanceUpdater updater(ecdb, *ecClass, nullptr);
    ASSERT_TRUE(updater.IsValid());

    ECSqlStatement validateStmt;
    ASSERT_EQ(ECSqlStatus::Success, validateStmt.Prepare(ecdb, "SELECT RInt, RString, Length, Square FROM ts.A WHERE ECInstanceId=?"));

    {
    Savepoint sp(ecdb, "default updater");

    ASSERT_EQ(BE_SQLITE_DONE, updater.Update(*updatedInstance));

    ASSERT_EQ(ECSqlStatus::Success, validateStmt.BindId(1, key.GetECInstanceId()));
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
    ECInstanceUpdater readonlyUpdater(ecdb, *ecClass, nullptr, "ReadonlyPropertiesAreUpdatable");
    ASSERT_TRUE(readonlyUpdater.IsValid());
    ASSERT_EQ(BE_SQLITE_DONE, readonlyUpdater.Update(*updatedInstance));

    ASSERT_EQ(ECSqlStatus::Success, validateStmt.BindId(1, key.GetECInstanceId()));
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
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='testSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEntityClass typeName='A' >"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "        <ECProperty propertyName='P2' typeName='string' />"
        "        <ECProperty propertyName='P3' typeName='long' />"
        "    </ECEntityClass>"
        "</ECSchema>");

    ECDbR ecdb = SetupECDb("updaterbasesonparameterindices.ecdb", schemaItem);
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECInstanceKey key;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.A (P1,P2,P3) VALUES(100, 'old', 1000)"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step(key));
    }

    ECClassCP ecClass = ecdb.Schemas().GetECClass("testSchema", "A");
    ASSERT_TRUE(ecClass != nullptr);

    bvector<uint32_t> propertiesToBind {1, 2, 3};
    ECInstanceUpdater instanceUpdater(ecdb, *ecClass, nullptr, propertiesToBind);
    ASSERT_TRUE(instanceUpdater.IsValid());

    //new values for properties
    const int newP1Value = 200;
    Utf8CP newP2Value = "new";
    const int64_t newP3Value = INT64_C(2000);

    //create New Instance
    IECInstancePtr updatedInstance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
    Utf8Char idStrBuffer[BeInt64Id::ID_STRINGBUFFER_LENGTH];
    key.GetECInstanceId().ToString(idStrBuffer);
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
    ASSERT_EQ(BE_SQLITE_DONE, instanceUpdater.Update(*updatedInstance));

    //validate updated Instance
    Utf8String validateECSql;
    validateECSql.Sprintf("SELECT NULL FROM ts.A WHERE ECInstanceId=%s AND P1=%d AND P2 LIKE '%s' AND P3=%" PRId64,
                          key.GetECInstanceId().ToString().c_str(), newP1Value, newP2Value, newP3Value);

    ECSqlStatement validateStmt;
    ASSERT_EQ(ECSqlStatus::Success, validateStmt.Prepare(ecdb, validateECSql.c_str()));
    ASSERT_EQ(BE_SQLITE_ROW, validateStmt.Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceUpdaterTests, UpdaterBasedOnListOfPropertiesToBind)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='testSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEntityClass typeName='A' >"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "        <ECProperty propertyName='P2' typeName='string' />"
        "        <ECProperty propertyName='P3' typeName='long' />"
        "    </ECEntityClass>"
        "</ECSchema>");

    ECDbR ecdb = SetupECDb("updaterbasesonlistofproperties.ecdb", schemaItem);
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECInstanceKey key;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.A (P1,P2,P3) VALUES(100, 'old', 1000)"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step(key));
    }

    ECClassCP ecClass = ecdb.Schemas().GetECClass("testSchema", "A");
    ASSERT_TRUE(ecClass != nullptr);

    ECPropertyCP p1 = ecClass->GetPropertyP("P1");
    ASSERT_TRUE(p1 != nullptr);

    ECPropertyCP p2 = ecClass->GetPropertyP("P2");
    ASSERT_TRUE(p2 != nullptr);

    ECPropertyCP p3 = ecClass->GetPropertyP("P3");
    ASSERT_TRUE(p3 != nullptr);

    bvector<ECPropertyCP> propertiesToBind {p1, p2, p3};
    ECInstanceUpdater instanceUpdater(ecdb, *ecClass, nullptr, propertiesToBind);
    ASSERT_TRUE(instanceUpdater.IsValid());

    //new values for properties
    const int newP1Value = 200;
    Utf8CP newP2Value = "new";
    const int64_t newP3Value = INT64_C(2000);

    //create New Instance
    IECInstancePtr updatedInstance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
    Utf8Char idStrBuffer[BeInt64Id::ID_STRINGBUFFER_LENGTH];
    key.GetECInstanceId().ToString(idStrBuffer);
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
    ASSERT_EQ(BE_SQLITE_DONE, instanceUpdater.Update(*updatedInstance));

    //validate updated Instance
    Utf8String validateECSql;
    validateECSql.Sprintf("SELECT NULL FROM ts.A WHERE ECInstanceId=%s AND P1=%d AND P2 LIKE '%s' AND P3=%" PRId64,
                          key.GetECInstanceId().ToString().c_str(), newP1Value, newP2Value, newP3Value);

    ECSqlStatement validateStmt;
    ASSERT_EQ(ECSqlStatus::Success, validateStmt.Prepare(ecdb, validateECSql.c_str()));
    ASSERT_EQ(BE_SQLITE_ROW, validateStmt.Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceUpdaterTests, UpdateArrayProperty)
    {
    ECDbCR db = SetupECDb("updateArrayProperty.ecdb", BeFileName(L"KitchenSink.01.00.ecschema.xml"));

    ECN::ECClassCP testClass = db.Schemas().GetECClass("KitchenSink", "TestClass");
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

    ECInstanceInserter inserter(db, *testClass, nullptr);
    ASSERT_TRUE(inserter.IsValid());
    ECInstanceKey instanceKey;
    ASSERT_EQ(BE_SQLITE_DONE, inserter.Insert(instanceKey, *testInstance));

    Utf8CP ecSql = "SELECT ECInstanceId, ECClassId, SmallIntArray FROM KitchenSink.TestClass";
    ECSqlStatement ecStatement;
    ECSqlStatus status = ecStatement.Prepare(db, ecSql);
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

    ECInstanceUpdater updater(db, *testClass, nullptr);
    ASSERT_TRUE(updater.IsValid());
    ASSERT_EQ(BE_SQLITE_DONE, updater.Update(*selectedInstance));

    ECSqlStatement ecStatement2;
    ECSqlStatus status2 = ecStatement2.Prepare(db, ecSql);
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
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='testSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEntityClass typeName='A' >"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>");

    ECDbR ecdb = SetupECDb("updaterbasesonparameterindices.ecdb", schemaItem);
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECClassCP ecClass = ecdb.Schemas().GetECClass("testSchema", "A");
    ASSERT_TRUE(ecClass != nullptr);

    bvector<uint32_t> propertiesToBind {};
    ECInstanceUpdater instanceUpdater(ecdb, *ecClass, nullptr, propertiesToBind);
    ASSERT_FALSE(instanceUpdater.IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
//Failing case, Update should fail it updater is invalid
TEST_F(ECInstanceUpdaterTests, InvalidUpdater)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='testSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEntityClass typeName='A' >"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>");

    ECDbR ecdb = SetupECDb("updaterbasesonparameterindices.ecdb", schemaItem);
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECClassCP ecClass = ecdb.Schemas().GetECClass("testSchema", "A");
    ASSERT_TRUE(ecClass != nullptr);

    bvector<uint32_t> propertiesToBind {};
    ECInstanceUpdater instanceUpdater(ecdb, *ecClass, nullptr, propertiesToBind);
    ASSERT_FALSE(instanceUpdater.IsValid());

    IECInstancePtr updatedInstance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();

    ASSERT_EQ(BE_SQLITE_ERROR, instanceUpdater.Update(*updatedInstance));
    }

END_ECDBUNITTESTS_NAMESPACE
