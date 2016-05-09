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

            ECInstanceInserter inserter(ecdb, *testClass);
            ECInstanceUpdater* updater = nullptr;
            for (int i = 0; i < numberOfInstances; i++)
                {
                IECInstancePtr instance = ECDbTestUtility::CreateArbitraryECInstance(*testClass, ECDbTestUtility::PopulatePrimitiveValueWithRandomValues);
                    
                auto status = inserter.Insert (*instance);
                ASSERT_EQ(SUCCESS, status);
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
                        updater = new ECInstanceUpdater(ecdb, *testClass);
                    else
                        updater = new ECInstanceUpdater(ecdb, *updatedInstance);
                    }

                if (testClass->GetPropertyCount() == 0)
                    {
                    ASSERT_FALSE(updater->IsValid());
                    return;
                    }
                else
                    ASSERT_TRUE(updater->IsValid());

                status = updater->Update(*updatedInstance);
                ASSERT_EQ (SUCCESS, status);

                SqlPrintfString ecSql ("SELECT c0.[ECInstanceId], c0.GetECClassId() as ECClassId, * FROM %s.%s c0 WHERE ECInstanceId=%s", Utf8String(schemaName).c_str(), Utf8String(className).c_str(), Utf8String(instance->GetInstanceId()).c_str());
                ECSqlStatement statement;
                ECSqlStatus prepareStatus = statement.Prepare (ecdb, ecSql.GetUtf8CP());
                ECInstanceECSqlSelectAdapter dataAdapter (statement);
                ASSERT_TRUE (ECSqlStatus::Success == prepareStatus);
                DbResult result;

                instance->GetAsMemoryECInstanceP()->MergePropertiesFromInstance(*updatedInstance);
                while ((result = statement.Step()) == BE_SQLITE_ROW)
                    {
                    IECInstancePtr selectedInstance = dataAdapter.GetInstance ();
                    bool equal = ECDbTestUtility::CompareECInstances (*instance, *selectedInstance);
                    ASSERT_TRUE (equal) << "Updated instance from ecdb not as expected.";
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
    v.SetUtf8CP("ECInstanceInserter");
    ASSERT_EQ(ECObjectsStatus::Success, testInstance->SetValue("S", v));

    ECInstanceId testId;
    {
    ECInstanceInserter inserter(ecdb, *testClass);
    ASSERT_TRUE(inserter.IsValid());
    ASSERT_EQ(SUCCESS, inserter.Insert(*testInstance));
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

    ECInstanceUpdater updater(ecdb, *testClass);
    ASSERT_TRUE(updater.IsValid());
    ASSERT_EQ(SUCCESS, updater.Update(*testInstance));

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
TEST_F(ECInstanceUpdaterTests, UpdateReadonlyProperty)
    {
    ECDbR ecdb = SetupECDb("updatereadonlyproperty.ecdb", SchemaItem ("<?xml version='1.0' encoding='utf-8'?>"
                                                                         "<ECSchema schemaName='testSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                                                         "    <ECEntityClass typeName='A' >"
                                                                         "        <ECProperty propertyName='P1' typeName='int' readOnly='True'/>"
                                                                         "        <ECProperty propertyName='P2' typeName='string' readOnly='True'/>"
                                                                         "        <ECProperty propertyName='P3' typeName='long' />"
                                                                         "    </ECEntityClass>"
                                                                         "</ECSchema>"));

    ASSERT_TRUE(ecdb.IsDbOpen());

    const int oldP1Value = 100;
    const int newP1Value = 200;
    Utf8CP oldP2Value = "old";
    Utf8CP newP2Value = "new";
    const int64_t oldP3Value = INT64_C(1000);
    const int64_t newP3Value = INT64_C(2000);

    ECInstanceKey key;

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.A (P1,P2,P3) VALUES(?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, oldP1Value));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, oldP2Value, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(3, oldP3Value));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step(key));
    }

    ECClassCP ecClass = ecdb.Schemas().GetECClass("testSchema", "A");
    ASSERT_TRUE(ecClass != nullptr);

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

    //Updater will skip any readonly properties therefore after update properties will retain there old values as new values will not be updated.
    ECInstanceUpdater updater(ecdb, *ecClass);
    ASSERT_EQ(SUCCESS, updater.Update(*updatedInstance));

    Utf8String validateECSql;
    validateECSql.Sprintf("SELECT NULL FROM ts.A WHERE ECInstanceId=%llu AND P1=%d AND P2 LIKE '%s' AND P3=%lld",
                          key.GetECInstanceId().GetValue(), oldP1Value, oldP2Value, newP3Value);

    ECSqlStatement validateStmt;
    ASSERT_EQ(ECSqlStatus::Success, validateStmt.Prepare(ecdb, validateECSql.c_str()));
    ASSERT_EQ(BE_SQLITE_ROW, validateStmt.Step());
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
    ECInstanceUpdater instanceUpdater(ecdb, *ecClass, propertiesToBind);
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
    ASSERT_EQ(BentleyStatus::SUCCESS, instanceUpdater.Update(*updatedInstance));

    //validate updated Instance
    Utf8String validateECSql;
    validateECSql.Sprintf("SELECT NULL FROM ts.A WHERE ECInstanceId=%llu AND P1=%d AND P2 LIKE '%s' AND P3=%lld",
                          key.GetECInstanceId().GetValue(), newP1Value, newP2Value, newP3Value);

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
    ECInstanceUpdater instanceUpdater(ecdb, *ecClass, propertiesToBind);
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
    ASSERT_EQ(BentleyStatus::SUCCESS, instanceUpdater.Update(*updatedInstance));

    //validate updated Instance
    Utf8String validateECSql;
    validateECSql.Sprintf("SELECT NULL FROM ts.A WHERE ECInstanceId=%llu AND P1=%d AND P2 LIKE '%s' AND P3=%lld",
                          key.GetECInstanceId().GetValue(), newP1Value, newP2Value, newP3Value);

    ECSqlStatement validateStmt;
    ASSERT_EQ(ECSqlStatus::Success, validateStmt.Prepare(ecdb, validateECSql.c_str()));
    ASSERT_EQ(BE_SQLITE_ROW, validateStmt.Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceUpdaterTests, UpdateArrayProperty)
    {
    ECDb&db = SetupECDb("updateArrayProperty.ecdb", BeFileName(L"KitchenSink.01.00.ecschema.xml"));

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

    ECInstanceInserter inserter(db, *testClass);
    ASSERT_TRUE(inserter.IsValid());
    ECInstanceKey instanceKey;
    auto insertStatus = inserter.Insert(instanceKey, *testInstance);
    ASSERT_EQ(SUCCESS, insertStatus);

    Utf8CP ecSql = "SELECT ECInstanceId, GetECClassId() as ECClassId, SmallIntArray FROM KitchenSink.TestClass";
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

    ECInstanceUpdater updater(db, *testClass);
    ASSERT_TRUE(updater.IsValid());
    auto updateStatus = updater.Update(*selectedInstance);
    ASSERT_EQ(SUCCESS, updateStatus);

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
    ECInstanceUpdater instanceUpdater(ecdb, *ecClass, propertiesToBind);
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
    ECInstanceUpdater instanceUpdater(ecdb, *ecClass, propertiesToBind);
    ASSERT_FALSE(instanceUpdater.IsValid());

    IECInstancePtr updatedInstance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();

    ASSERT_EQ(BentleyStatus::ERROR, instanceUpdater.Update(*updatedInstance));
    }

END_ECDBUNITTESTS_NAMESPACE
