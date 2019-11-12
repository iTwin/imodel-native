/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PerformanceCRUDTestsHelper.h"
BEGIN_ECDBUNITTESTS_NAMESPACE

#define OPCOUNT 1000

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
    ASSERT_EQ(SUCCESS, SetupECDb("performanceecinstanceecsqlselectadapter.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, PopulateECDb(10));

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
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), opCount, logMessage.c_str(), true);
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
        assertCtor(m_ecdb, ecsql, OPCOUNT);
        }
    }

END_ECDBUNITTESTS_NAMESPACE
