/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/DevSummitPerformanceTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../NonPublished/PublicApi/NonPublished/ECDb/ECDbTestProject.h"
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
                    "(SELECT ecc.Id ECClassId, ecc.ECSchemaId " 
                    "FROM ec_Class ecc " 
                    "WHERE ecc.IsDomainClass = '1' AND ecc.ECSchemaId NOT IN " 
                        "( " 
                        "SELECT ca.ContainerId " 
                        "FROM ec_CustomAttribute ca " 
                        "WHERE ca.ECClassId = ( " 
                                        "SELECT ecc2.Id " 
                                        "FROM ec_Class ecc2, ec_Schema " 
                                        "WHERE ecc2.Name='SystemSchema' AND ecc2.ECSchemaId = ec_Schema.Id AND ec_Schema.Name='Bentley_Standard_CustomAttributes') " 
                        ") " 
                    ") t1, ec_Class ecc3, ec_Schema ecs2 " 
                "WHERE t1.ECClassId = ecc3.Id AND t1.ECSchemaId = ecs2.ECSchemaId ");

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
};

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

END_ECDBUNITTESTS_NAMESPACE
