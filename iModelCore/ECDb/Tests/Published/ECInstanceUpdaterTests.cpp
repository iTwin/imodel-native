/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECInstanceUpdaterTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
            ECDb ecdb;
            CreateECDb(ecdb, "updateInstances.ecdb", BeFileName(L"KitchenSink.01.00.ecschema.xml"));
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
                    updatedInstance->SetValue("DoubleMember", ECValue(3.1415));
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
TEST_F (ECInstanceUpdaterTests, UpdateWithCurrentTimeStampTrigger)
    {
    ECDb ecdb;
    CreateECDb(ecdb, "updatewithcurrenttimestamptrigger.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));
    auto testClass = ecdb.Schemas ().GetECClass ("ECSqlTest", "ClassWithLastModProp");
    ASSERT_TRUE (testClass != nullptr);

    auto tryGetLastMod = [] (DateTime& lastMod, ECDbR ecdb, ECInstanceId id)
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT LastMod FROM ecsql.ClassWithLastModProp WHERE ECInstanceId=?"));

        stmt.BindId (1, id);
        ASSERT_EQ (BE_SQLITE_ROW, stmt.Step ());
        ASSERT_FALSE (stmt.IsValueNull (0));

        lastMod = stmt.GetValueDateTime (0);
        };

    //insert test instance
    auto testInstance = testClass->GetDefaultStandaloneEnabler ()->CreateInstance ();

    ECValue v (1);
    ASSERT_EQ (ECOBJECTS_STATUS_Success, testInstance->SetValue ("I", v));

    v.Clear ();
    v.SetUtf8CP ("ECInstanceInserter");
    ASSERT_EQ (ECOBJECTS_STATUS_Success, testInstance->SetValue ("S", v));


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
    ASSERT_EQ (ECOBJECTS_STATUS_Success, testInstance->SetValue ("I", v));

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
    const uint64_t timeSpan_1sec_in_hns = 5000000ULL;
    ASSERT_GT (timeSpan, timeSpan_1sec_in_hns) << "New LastMod must be at least 1 second later than old LastMod as test was paused for 1 sec before updating";
    }

END_ECDBUNITTESTS_NAMESPACE
