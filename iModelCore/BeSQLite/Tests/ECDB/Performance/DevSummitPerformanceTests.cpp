/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/ECDB/Performance/DevSummitPerformanceTests.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitTests/NonPublished/ECDb/ECDbTestProject.h>
#include "TestSchemaHelper.h"
#include "PerformanceTestFixture.h"

using namespace BentleyApi::ECN;

BEGIN_ECDBUNITTESTS_NAMESPACE
struct PerformanceDevSummitTests : PerformanceTestFixture 
    {
    private:
        ECDbTestProject m_testProject;

    public:
        virtual void InitializeTestDb () override {}

        void ExecuteInstanceCountQuery()
            {
            BeFileName dbName("d:\\temp\\data\\SDM_Composite.i.idgndb");
            auto stat = m_testProject.Open(dbName.GetNameUtf8().c_str());
            EXPECT_EQ (BE_SQLITE_OK, stat);

            Utf8String countName("Get instance counts");
            StopWatch countTimer(countName.c_str(), false);

            Utf8String ecSql;
            ecSql.Sprintf("SELECT ecc3.Name as ECClassName, ecs2.Name as ECSchemaName " 
                "FROM " 
                    "(SELECT ecc.[ECClassId], ecc.[ECSchemaId] " 
                    "FROM ec_Class ecc " 
                    "WHERE ecc.[IsDomainClass] = '1' AND ecc.[ECSchemaId] NOT IN " 
                        "( " 
                        "SELECT ca.ContainerId " 
                        "FROM ec_CustomAttribute ca " 
                        "WHERE ca.ECClassId = ( " 
                                        "SELECT ecc2.ECClassId " 
                                        "FROM ec_Class ecc2, ec_Schema " 
                                        "WHERE ecc2.Name='SystemSchema' AND ecc2.ECSchemaId = ec_Schema.ECSchemaId AND ec_Schema.Name='Bentley_Standard_CustomAttributes') " 
                        ") " 
                    ") t1, ec_Class ecc3, ec_Schema ecs2 " 
                "WHERE t1.[ECClassId] = ecc3.[ECClassId] AND t1.ECSchemaId = ecs2.[ECSchemaId] ");

            ECSqlStatement ecStatement;
            ecStatement.Prepare(m_testProject.GetECDb(), ecSql.c_str());
            ECSqlStepStatus result;
            ECSqlStepStatus countResult;
            size_t total = 0;
            while ((result = ecStatement.Step()) == ECSqlStepStatus::HasRow)
                {
                Utf8CP ecClassName = ecStatement.GetValueText(0);
                Utf8CP ecSchemaName = ecStatement.GetValueText(1);
                ECSqlStatement ecCountQuery;
                Utf8String ecCountSql;
                ecCountSql.Sprintf("SELECT count(*) FROM [%s].[%s] ", ecClassName, ecSchemaName);
                ecCountQuery.Prepare(m_testProject.GetECDb(), ecSql.c_str());
                while ((countResult = ecCountQuery.Step()) == ECSqlStepStatus::HasRow)
                    total += ecCountQuery.GetValueInt(0);
                }
            countTimer.Stop ();
            ASSERT_EQ(260555, total);
            bmap<Utf8String, double> results;
            results[countName] = countTimer.GetElapsedSeconds();
            LogResultsToFile(results);
            }

        void ExecuteQuery(bool selectProperties, bool useWhereClause, bool useOrderBy, int expectedNumResults, bmap<Utf8String, double>& results)
            {
            Utf8String ecSql;
            Utf8String ecSelect("");
            Utf8String ecWhere("");
            Utf8String ecOrder("");

            if (selectProperties)
                ecSelect.Sprintf(", *");
            if (useWhereClause)
                ecWhere.Sprintf(" WHERE WIDTH=?");
            if (useOrderBy)
                ecOrder.Sprintf(" ORDER BY [LENGTH] DESC");

            ecSql.Sprintf("SELECT ECInstanceId%s FROM [ProStructures].[Shapes]%s%s", ecSelect.c_str(), ecWhere.c_str(), ecOrder.c_str());
            Utf8String description;
            Utf8String overallTimerName;

            description.Sprintf("Query for Shapes%s%s%s", selectProperties ? " (all Properties)" : "", useWhereClause ? " using WHERE clause" : "", useOrderBy ? " with ORDER BY" : "" );
            overallTimerName.Sprintf("%s (overall)", description.c_str());

            StopWatch overallTimer(overallTimerName.c_str(), false);
            ECSqlStatement ecStatement;        
            ecStatement.Prepare(m_testProject.GetECDb(), ecSql.c_str()); 

            ECInstanceECSqlSelectAdapter dataAdapter (ecStatement);

            ECSqlStepStatus result;
            if (useWhereClause)
                ecStatement.BindDouble(1, 166.0);

            int counter = 0;
            overallTimer.Start();
            while ((result = ecStatement.Step ()) == ECSqlStepStatus::HasRow)
                {
                IECInstancePtr instance = dataAdapter.GetInstance ();
                counter++;
                }
            overallTimer.Stop();
            ASSERT_EQ(expectedNumResults, counter);

            results[overallTimerName] = overallTimer.GetElapsedSeconds();
            }

        void DoSingleTest(bool selectProperties, bool useWhereClause, bool useOrderBy, int expectedNumResults, bmap<Utf8String, double>& results)
            {
            BeFileName dbName("d:\\temp\\data\\PUG_STR_AllRefs_AP.i.idgndb");
            auto stat = m_testProject.Open(dbName.GetNameUtf8().c_str());
            EXPECT_EQ (BE_SQLITE_OK, stat);
            ExecuteQuery(selectProperties, useWhereClause, useOrderBy, expectedNumResults, results);
            m_testProject.GetECDb().CloseDb();
            }
        void ExecuteQueries()
            {
            bmap<Utf8String, double> results;
            DoSingleTest(false, false, false, 43162, results);
            DoSingleTest(true, false, false, 43162, results);
            DoSingleTest(true, true, false, 4170, results);
            DoSingleTest(true, true, true, 4170, results);
            LogResultsToFile(results);
            }

        void RunInsertTest(bool useECPersistence, bmap<Utf8String, double>& results)
            {
            ECDbR db = m_testProject.Create("insertInstances.ecdb", L"KitchenSink.01.00.ecschema.xml", false);
            ECClassP testClass = nullptr;
            db.GetEC().GetSchemaManager().GetECClass (testClass, "KitchenSink", "PrimitiveClass");  // WIP_FNV: start taking ECSchemaName, ECClassName, instead
            bvector<IECInstancePtr> instances;
            for (int i = 0; i < 20000; i++)
                {
                IECInstancePtr instance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
                //auto instance = ECDbTestProject::CreateArbitraryECInstance(*testClass, ECDbTestProject::PopulatePrimitiveValueWithRandomValues);

                ECValue value;
                for (ECPropertyCP ecProperty : testClass->GetProperties(true))
                    {
                    if (ecProperty->GetIsPrimitive ())
                        {
                        AssignRandomECValue (value, *ecProperty);
                        instance->SetValue (ecProperty->GetName().c_str(), value);
                        }
                    }
                instances.push_back(instance);
                }
            Utf8String overallTimerName;
            overallTimerName.Sprintf("Inserting instances %s", useECPersistence ? "using ECPersistence" : "using ECSql" );
            StopWatch overallTimer(overallTimerName.c_str(), false);
            if (useECPersistence)
                {
                ECPersistencePtr persistence = db.GetEC().GetECPersistence (NULL, *testClass);
                ASSERT_FALSE(persistence.IsNull());
                ECInstanceId ecInstanceId;
                overallTimer.Start();
                for (int i = 0; i < 20000; i++)
                    {
                    persistence->Insert (&ecInstanceId, *instances[i]);
                    }
                overallTimer.Stop();
                }
            else
                {
                ECInstanceInserter inserter(db, *testClass);
                overallTimer.Start();
                ECInstanceKey instanceKey;
                for (int i = 0; i < 20000; i++)
                    {
                    inserter.Insert (instanceKey, *instances[i]);
                    }
                overallTimer.Stop();
                }
            results[overallTimerName] = overallTimer.GetElapsedSeconds();

            ECSqlStatement statement;
            Utf8String ecSql;
            ecSql.Sprintf("SELECT count(*) from KitchenSink.PrimitiveClass");
            statement.Prepare(db, ecSql.c_str());
            ECSqlStepStatus result;

            int total = 0;
            while ((result = statement.Step()) == ECSqlStepStatus::HasRow)
                total = statement.GetValueInt(0);
            ASSERT_EQ(20000, total);
            }

        void InsertTests()
            {
            bmap<Utf8String, double> results;
            RunInsertTest(true, results);
            m_testProject.GetECDb().CloseDb();
            RunInsertTest(false, results);
            m_testProject.GetECDb().CloseDb();
            LogResultsToFile(results);
            }

        void RunUpdateTest(bool useECPersistence, bmap<Utf8String, double>& results)
            {
            ECDbR db = m_testProject.Create("updateInstances.ecdb", L"KitchenSink.01.00.ecschema.xml", false);
            ECClassP testClass = nullptr;
            db.GetEC().GetSchemaManager().GetECClass (testClass, "KitchenSink", "PrimitiveClass");  // WIP_FNV: start taking ECSchemaName, ECClassName, instead
            ECPersistencePtr persistence = db.GetEC().GetECPersistence (NULL, *testClass);
            ASSERT_FALSE(persistence.IsNull());
            ECInstanceId ecInstanceId;

            for (int i = 0; i < 20000; i++)
                {
                IECInstancePtr instance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
                ECValue value;
                for (ECPropertyCP ecProperty : testClass->GetProperties(true))
                    {
                    if (ecProperty->GetIsPrimitive ())
                        {
                        AssignRandomECValue (value, *ecProperty);
                        instance->SetValue (ecProperty->GetName().c_str(), value);
                        }
                    }
                persistence->Insert (&ecInstanceId, *instance);
                }

            bvector<IECInstancePtr> instances;
            ECSqlStatement statement;
            Utf8String ecSql;
            ecSql.Sprintf("SELECT ECInstanceId, GetECClassId() as ECClassId, * from DGNECPlugin_Test.PIPE_Extra");
            statement.Prepare(db, ecSql.c_str());
            ECSqlStepStatus result;
            ECInstanceECSqlSelectAdapter dataAdapter (statement);

            ECValue value;
            while ((result = statement.Step()) == ECSqlStepStatus::HasRow)
                {
                IECInstancePtr instance = dataAdapter.GetInstance ();
                for (ECPropertyCP ecProperty : testClass->GetProperties(true))
                    {
                    if (ecProperty->GetIsPrimitive ())
                        {
                        AssignRandomECValue (value, *ecProperty);
                        instance->SetValue (ecProperty->GetName().c_str(), value);
                        }
                    }
                instances.push_back(instance);
                }

            Utf8String overallTimerName;
            overallTimerName.Sprintf("Updating instances %s", useECPersistence ? "using ECPersistence" : "using ECSql" );
            StopWatch overallTimer(overallTimerName.c_str(), false);
            if (useECPersistence)
                {
                overallTimer.Start();
                for (int i = 0; i < 20000; i++)
                    persistence->Update(*instances[i]);
                overallTimer.Stop();
                }
            else
                {
                ECInstanceUpdater updater(db, *testClass);
                overallTimer.Start();
                for (int i = 0; i < 20000; i++)
                    updater.Update(*instances[i]);
                overallTimer.Stop();
                }
            results[overallTimerName] = overallTimer.GetElapsedSeconds();

            }

        void UpdateTests()
            {
            bmap<Utf8String, double> results;
            RunUpdateTest(true, results);
            m_testProject.GetECDb().CloseDb();
            RunUpdateTest(false, results);
            m_testProject.GetECDb().CloseDb();
            LogResultsToFile(results);

            }    };

// Tests are commented out because the datafiles are not checked in

//TEST_F(PerformanceDevSummitTests, InstanceCounts)
//    {
//    ExecuteInstanceCountQuery();
//    }

//TEST_F(PerformanceDevSummitTests, Queries)
//    {
//    ExecuteQueries();
//    }
//
//TEST_F(PerformanceDevSummitTests, InsertComplexInstances)
//    {
//    InsertTests();
//    }
//
//TEST_F(PerformanceDevSummitTests, UpdateComplexInstances)
//    {
//    UpdateTests();
//    }

END_ECDBUNITTESTS_NAMESPACE
