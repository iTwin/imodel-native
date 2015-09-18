/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/QueryTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../NonPublished/PublicApi/NonPublished/ECDb/ECDbTestProject.h"
#include "TestSchemaHelper.h"
#include "PerformanceTestFixture.h"

using namespace BentleyApi::ECN;

BEGIN_ECDBUNITTESTS_NAMESPACE
struct PerformanceQueryTests : PerformanceTestFixture 
    {
    private:
        ECDbTestProject m_testProject;
                void ExecuteSql(Utf8StringR ecSql, Utf8StringR timerName, bmap<Utf8String, double>& results, int expectedNumberOfResults,double &overAllTime)
            {
            ECSqlStatement ecStatement;        
            ecStatement.Prepare(m_testProject.GetECDb(), ecSql.c_str()); 

            ECInstanceECSqlSelectAdapter dataAdapter (ecStatement);

            ECSqlStepStatus result;
            Utf8String overallTimerName;
            overallTimerName.Sprintf("%s (overall time)", timerName.c_str());
            StopWatch overallTimer(overallTimerName.c_str(), false);

            Utf8String instanceTimerName;
            instanceTimerName.Sprintf("%s (instance creation time)", timerName.c_str());
            StopWatch instanceTimer(instanceTimerName.c_str(), false);
            double elapsedSeconds = 0;
            int counter = 0;
            overallTimer.Start();
            while ((result = ecStatement.Step ()) == ECSqlStepStatus::HasRow)
                {
                instanceTimer.Start();
                IECInstancePtr instance = dataAdapter.GetInstance ();
                instanceTimer.Stop();
                elapsedSeconds += instanceTimer.GetElapsedSeconds();
                counter++;
                }
            overallTimer.Stop();
            results[instanceTimerName] = elapsedSeconds;
            results[overallTimerName] = overallTimer.GetElapsedSeconds();
            overAllTime = overallTimer.GetElapsedSeconds();
            ASSERT_EQ(expectedNumberOfResults, counter);
            }

        void LoadByInstanceId(Utf8String instanceId, Utf8String className, bmap<Utf8String, double>& results, int expectedNumberOfResults)
            {
            Utf8String ecSql;
            ecSql.Sprintf("SELECT c0.* FROM ONLY [KitchenSink].[%s] c0 WHERE c0.ECInstanceId =('%s' )", className.c_str(), instanceId.c_str());
            Utf8String timerName;
            timerName.Sprintf("Query One Instance By Id Of Class '%s'", className.c_str());
            double ellapsedSecond;
            ExecuteSql(ecSql, timerName, results, expectedNumberOfResults, ellapsedSecond);
            LOGTODB(TEST_DETAILS, ellapsedSecond * 1000.0, "LoadByInstanceId");
            }

        void QueryAllInstancesByClass(Utf8String className, bmap<Utf8String, double>& results, int expectedNumberOfResults)
            {
            Utf8String ecSql;
            ecSql.Sprintf("SELECT c0.* FROM ONLY [KitchenSink].[%s] c0", className.c_str());
            Utf8String timerName;
            timerName.Sprintf("Query All Instances Of Class '%s'", className.c_str());
            double ellapsedSecond;
            ExecuteSql(ecSql, timerName, results, expectedNumberOfResults, ellapsedSecond);
            LOGTODB(TEST_DETAILS, ellapsedSecond * 1000.0, "QueryAllInstances");
            }

        void QueryAllInstancesByClassWithOrderBy(Utf8String className, Utf8String propertyName, bmap<Utf8String, double>& results, int expectedNumberOfResults)
            {
            Utf8String ecSql;
            ecSql.Sprintf("SELECT c0.* FROM ONLY [KitchenSink].[%s] c0 ORDER BY %s ASC", className.c_str(), propertyName.c_str());
            Utf8String timerName;
            timerName.Sprintf("Query All Instances Of Class '%s' With OrderBy", className.c_str());
            double ellapsedSecond;
            ExecuteSql(ecSql, timerName, results, expectedNumberOfResults, ellapsedSecond);
            LOGTODB(TEST_DETAILS, ellapsedSecond * 1000.0, "QueryAllInstancesByClassWithOrderBy");
            }

    public:
        void ConnectToDb()
            {
            BeFileName seedPath;
            BeTest::GetHost().GetDocumentsRoot (seedPath);
            seedPath.AppendToPath(L"DgnDb");
            seedPath.AppendToPath(L"ECDb");
            seedPath.AppendToPath(L"performanceQueryTests.ecdb");

            auto stat = m_testProject.Open(seedPath.GetNameUtf8().c_str());
            EXPECT_EQ (BE_SQLITE_OK, stat);
            }
        virtual void InitializeTestDb () override {}

        void LoadByInstanceId()
            {
            ConnectToDb();
            bmap<Utf8String, double> results;
            LoadByInstanceId("10000", "TestClass", results, 1);
            LoadByInstanceId("30000", "Test2Class", results, 1);
            LoadByInstanceId("110001", "Folder", results, 1);
            LoadByInstanceId("130001", "Document", results, 1);
            LogResultsToFile(results);
            m_testProject.GetECDb().CloseDb();
            }

        void QueryAllInstances()
            {
            ConnectToDb();
            bmap<Utf8String, double> results;
            QueryAllInstancesByClass("TestClass", results, 20000);
            QueryAllInstancesByClass("Test2Class", results, 20000);
            QueryAllInstancesByClass("Folder", results, 20000);
            QueryAllInstancesByClass("Document", results, 20000);
            LogResultsToFile(results);
            m_testProject.GetECDb().CloseDb();
            }

        void QueryAllInstancesWithOrderBy()
            {
            ConnectToDb();
            bmap<Utf8String, double> results;
            QueryAllInstancesByClassWithOrderBy("TestClass", "BaseClassMember", results, 20000);
            QueryAllInstancesByClassWithOrderBy("Folder", "Name", results, 20000);
            QueryAllInstancesByClassWithOrderBy("Document", "Name", results, 20000);
            LogResultsToFile(results);
            m_testProject.GetECDb().CloseDb();
            }
    };

TEST_F(PerformanceQueryTests, LoadByInstanceIDForAllClasses)
    {
    LoadByInstanceId();
    }

TEST_F(PerformanceQueryTests, QueryAllInstancesOfEachClass)
    {
    QueryAllInstances();
    }

TEST_F(PerformanceQueryTests, QueryAllInstancesOfEachClassWithOrderBy)
    {
    QueryAllInstancesWithOrderBy();
    }
END_ECDBUNITTESTS_NAMESPACE
