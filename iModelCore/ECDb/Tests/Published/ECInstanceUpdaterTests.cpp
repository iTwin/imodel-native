/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/ECDB/Published/ECInstanceUpdaterTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "ECInstanceAdaptersTestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

struct ECInstanceUpdaterTests : ECInstanceAdaptersTestFixture
    {
    protected:
        void UpdateInstances(Utf8CP className, Utf8CP schemaName, int numberOfInstances, bool populateAllProperties)
            {
            SetTestProject(CreateTestProject("updateInstances.ecdb", L"KitchenSink.01.00.ecschema.xml"));
            ECDbR db = GetTestProject().GetECDb();
            ECClassCP testClass = db.GetSchemaManager().GetECClass (schemaName, className);

            ECInstanceInserter inserter(db, *testClass);
            ECInstanceUpdater* updater = nullptr;
            for (int i = 0; i < numberOfInstances; i++)
                {
                IECInstancePtr instance = ECDbTestProject::CreateArbitraryECInstance(*testClass, ECDbTestProject::PopulatePrimitiveValueWithRandomValues);
                    
                auto status = inserter.Insert (*instance);
                ASSERT_EQ(SUCCESS, status);
                IECInstancePtr updatedInstance;
                
                if (populateAllProperties)
                    {
                    updatedInstance = instance->CreateCopyThroughSerialization();
                    ECDbTestProject::PopulateECInstance(updatedInstance, ECDbTestProject::PopulatePrimitiveValueWithRandomValues);
                    }
                else
                    {
                    updatedInstance = testClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
                    ECValue v;
                    v.SetLong(101);
                    updatedInstance->SetValue(L"LongMember", v);
                    instance->GetValue(v, L"BooleanMember");
                    updatedInstance->SetValue(L"BooleanMember", ECValue(!(v.GetBoolean())));
                    updatedInstance->SetValue(L"DoubleMember", ECValue(3.1415));
                    }
                updatedInstance->SetInstanceId(instance->GetInstanceId().c_str());

                if (nullptr == updater)
                    {
                    if (populateAllProperties)
                        updater = new ECInstanceUpdater(db, *testClass);
                    else
                        updater = new ECInstanceUpdater(db, *updatedInstance);
                    }

                status = updater->Update(*updatedInstance);
                ASSERT_EQ (SUCCESS, status);

                SqlPrintfString ecSql ("SELECT c0.[ECInstanceId], c0.GetECClassId() as ECClassId, * FROM %s.%s c0 WHERE ECInstanceId=%s", Utf8String(schemaName).c_str(), Utf8String(className).c_str(), Utf8String(instance->GetInstanceId()).c_str());
                ECSqlStatement statement;
                ECSqlStatus prepareStatus = statement.Prepare (db, ecSql.GetUtf8CP());
                ECInstanceECSqlSelectAdapter dataAdapter (statement);
                ASSERT_TRUE (ECSqlStatus::Success == prepareStatus);
                ECSqlStepStatus result;

                instance->GetAsMemoryECInstanceP()->MergePropertiesFromInstance(*updatedInstance);
                while ((result = statement.Step()) == ECSqlStepStatus::HasRow)
                    {
                    IECInstancePtr selectedInstance = dataAdapter.GetInstance ();
                    bool equal = ECDbTestUtility::CompareECInstances (*instance, *selectedInstance);
                    ASSERT_TRUE (equal) << L"Updated instance from ecdb not as expected.";
                    }
                }
            delete updater;
            }
    };

TEST_F(ECInstanceUpdaterTests, UpdateSingleInstanceOfPrimitiveClass)
    {
    UpdateInstances("PrimitiveClass", "KitchenSink", 1, true);
    }

TEST_F(ECInstanceUpdaterTests, UpdateMultipleInstancesOfPrimitiveClass)
    {
    UpdateInstances("PrimitiveClass", "KitchenSink", 100, true);
    }

TEST_F(ECInstanceUpdaterTests, UpdateSingleInstanceOfPrimitiveClassWithIncompleteInstance)
    {
    UpdateInstances("PrimitiveClass", "KitchenSink", 1, false);
    }

TEST_F(ECInstanceUpdaterTests, UpdateMultipleInstancesOfPrimitiveClassWithIncompleteInstance)
    {
    UpdateInstances("PrimitiveClass", "KitchenSink", 100, false);
    }


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECInstanceUpdaterTests, UpdateWithCurrentTimeStampTrigger)
    {
    SetTestProject (CreateTestProject ("updatewithcurrenttimestamptrigger.ecdb", L"ECSqlTest.01.00.ecschema.xml"));
    ECDbR ecdb = GetTestProject ().GetECDb ();
    auto testClass = ecdb.GetSchemaManager ().GetECClass ("ECSqlTest", "ClassWithLastModProp");
    ASSERT_TRUE (testClass != nullptr);

    auto tryGetLastMod = [] (DateTime& lastMod, ECDbR ecdb, ECInstanceId id)
        {
        ECSqlStatement stmt;
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stmt.Prepare (ecdb, "SELECT LastMod FROM ecsql.ClassWithLastModProp WHERE ECInstanceId=?"));

        stmt.BindId (1, id);
        ASSERT_EQ ((int) ECSqlStepStatus::HasRow, (int) stmt.Step ());
        ASSERT_FALSE (stmt.IsValueNull (0));

        lastMod = stmt.GetValueDateTime (0);
        };

    //insert test instance
    auto testInstance = testClass->GetDefaultStandaloneEnabler ()->CreateInstance ();

    ECValue v (1);
    ASSERT_EQ (ECOBJECTS_STATUS_Success, testInstance->SetValue (L"I", v));

    v.Clear ();
    v.SetUtf8CP ("ECInstanceInserter");
    ASSERT_EQ (ECOBJECTS_STATUS_Success, testInstance->SetValue (L"S", v));


    ECInstanceId testId;

        {
        ECInstanceInserter inserter (ecdb, *testClass);
        ASSERT_TRUE (inserter.IsValid ());

        ASSERT_EQ (SUCCESS, inserter.Insert (*testInstance));

        ASSERT_TRUE (ECInstanceIdHelper::FromString (testId, testInstance->GetInstanceId ().c_str ()));
        }


    DateTime firstLastMod;
    tryGetLastMod (firstLastMod, ecdb, testId);
    ASSERT_TRUE (firstLastMod.IsValid ());

    //now update an ECInstance
    BeThreadUtilities::BeSleep (1000); //so that new last mod differs significantly from old last mod
    v.Clear ();
    v.SetInteger (2);
    ASSERT_EQ (ECOBJECTS_STATUS_Success, testInstance->SetValue (L"I", v));

    ECInstanceUpdater updater (ecdb, *testClass);
    ASSERT_TRUE (updater.IsValid ());
    ASSERT_EQ (SUCCESS, updater.Update (*testInstance));

    DateTime newLastMod;
    tryGetLastMod (newLastMod, ecdb, testId);
    ASSERT_TRUE (newLastMod.IsValid ());

    uint64_t firstLastModJdHns = 0ULL;
    ASSERT_EQ (SUCCESS, firstLastMod.ToJulianDay (firstLastModJdHns));

    uint64_t newLastModJdHns = 0ULL;
    ASSERT_EQ (SUCCESS, newLastMod.ToJulianDay (newLastModJdHns));

    uint64_t timeSpan = newLastModJdHns - firstLastModJdHns;
    const uint64_t timeSpan_1sec_in_hns = 10000000ULL;
    ASSERT_GT (timeSpan, timeSpan_1sec_in_hns) << "New LastMod must be at least 1 second later than old LastMod as test was paused for 1 sec before updating";
    }

END_ECDBUNITTESTS_NAMESPACE
