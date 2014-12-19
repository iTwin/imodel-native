/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/ECDB/Published/ECInstanceUpdaterTests.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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
            ECClassP testClass = nullptr;
            db.GetEC().GetSchemaManager().GetECClass (testClass, schemaName, className);

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


END_ECDBUNITTESTS_NAMESPACE
