/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceECInstanceECSqlSelectAdapterTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceCRUDTestsHelper.h"
BEGIN_ECDBUNITTESTS_NAMESPACE

#define OPCOUNT 100000

//------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle    01/2017
//+---------------+---------------+---------------+---------------+-------------
struct PerformanceECInstanceECSqlSelectAdapterTests : public ECDbTestFixture
    {};

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle    01/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECInstanceECSqlSelectAdapterTests, Constructor)
    {
    ECDbCR ecdb = SetupECDb("performanceecinstanceecsqlselectadapter.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), 10);
    ASSERT_TRUE(ecdb.IsDbOpen());

    auto assertCtor = [] (ECDbCR ecdb, Utf8CP ecsql, int opCount)
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql)) << ecsql;

        StopWatch timer(true);
        for (int i = 0; i < opCount; i++)
            {
            ECInstanceECSqlSelectAdapter adapter(stmt);
            ASSERT_TRUE(adapter.IsValid()) << stmt.GetECSql();
            }
        timer.Stop();

        Utf8String logMessage;
        logMessage.Sprintf("ECSQL: %s", stmt.GetECSql());
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), logMessage.c_str(), opCount);
        };

    std::vector<Utf8CP> testECSqls {"SELECT B,Bi,D,Dt,DtUtc,P2D,P3D,MyPSA FROM ecsql.P",
        "SELECT ECInstanceId,ECClassId FROM ecsql.P",
        "SELECT ECInstanceId,ECClassId,B,Bi,D,Dt,DtUtc,P2D,P3D,MyPSA FROM ecsql.P",
        "SELECT * FROM ecsql.P",
        "SELECT * FROM ecsql.PSA",
        "SELECT * FROM ecsql.TH5",
        "SELECT * FROM ecsql.TC5",
        "SELECT ECInstanceId,ECClassId,SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId FROM ecsql.PSAHasP"};

    for (Utf8CP ecsql : testECSqls)
        {
        assertCtor(ecdb, ecsql, OPCOUNT);
        }
    }

END_ECDBUNITTESTS_NAMESPACE
