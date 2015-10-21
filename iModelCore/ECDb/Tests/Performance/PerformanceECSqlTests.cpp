/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceECSqlTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// @bsiclass                                                Krischan.Eberle         09/15
//=======================================================================================
struct PerformanceECSqlTestFixture : public ECDbTestFixture
    {
protected:
    void GetECClassIdList(bvector<ECN::ECClassId>& classIds, ECN::ECClassCR ecclass, bool includeSubclasses) const
        {
        classIds.push_back(ecclass.GetId());
        if (!includeSubclasses)
            return;

        for (ECN::ECClassCP subclass : GetECDb().Schemas().GetDerivedECClasses(ecclass))
            {
            GetECClassIdList(classIds, *subclass, includeSubclasses);
            }
        }

    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle         10/15
//=======================================================================================
struct PerformanceECSqlVsSqliteTests : PerformanceECSqlTestFixture
    {};

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECSqlVsSqliteTests, UpdateWithWhereClauseWithPrimaryKey)
    {
    const int instanceCount = 1000000;

    std::vector<ECInstanceId> th3Ids;
    {
    SetupECDb("ecsqlperformance.ecdb", BeFileName("ECSqlTest.01.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::ReadWrite), 0);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ecsql.TH3(S,S1,S2,S3) VALUES(?,?,?,?)"));
    for (int i = 0; i < instanceCount; i++)
        {
        for (int j = 0; j < 4; j++)
            {
            Utf8String sval;
            sval.Sprintf("bla %d_%d",j, i);
            ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(j+1, sval.c_str(), IECSqlBinder::MakeCopy::Yes));
            }

        ECInstanceKey newKey;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey));
        stmt.Reset();
        stmt.ClearBindings();

        th3Ids.push_back(newKey.GetECInstanceId());
        }

    GetECDb().SaveChanges();
    }

    ASSERT_EQ(instanceCount, (int) th3Ids.size());

    //ECSQL
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "UPDATE ecsql.TH3 SET S='S', S1='S1', S2='S2', S3='S3' WHERE ECInstanceId=?"));
    LOG.infov("Translated SQL: %s", stmt.GetNativeSql());

    StopWatch timer(true);
    for (ECInstanceId const& id : th3Ids)
        {
        if (ECSqlStatus::Success != stmt.BindId(1, id))
            return;

        if (BE_SQLITE_DONE != stmt.Step())
            return;

        stmt.Reset();
        stmt.ClearBindings();
        }

    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "ECSQL UPDATE", instanceCount);
    GetECDb().AbandonChanges();
    }

    //SQL w/o classid filter
    {
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(GetECDb(), "UPDATE [ecsqltest_THBase] SET [S]='S', [S1]='S1', [S2]='S2', [S3]='S3' WHERE [ECInstanceId]=?"));
    LOG.infov("SQL w/o class id filter: %s", stmt.GetSql());

    StopWatch timer(true);
    for (ECInstanceId const& id : th3Ids)
        {
        if (BE_SQLITE_OK != stmt.BindId(1, id))
            return;

        if (BE_SQLITE_DONE != stmt.Step())
            return;

        stmt.Reset();
        stmt.ClearBindings();
        }

    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "SQLite UPDATE w/o ECClassId filter", instanceCount);
    GetECDb().AbandonChanges();
    }

    //SQL with classid filter
    {
    ECClassCP th3Class = GetECDb().Schemas().GetECClass("ECSqlTest", "TH3");
    ASSERT_TRUE(th3Class != nullptr);
    bvector<ECClassId> classIds;
    GetECClassIdList(classIds, *th3Class, true);

    Utf8String sql ("UPDATE [ecsqltest_THBase] SET [S]='S', [S1]='S1', [S2]='S2', [S3]='S3' WHERE [ECInstanceId]=? AND (");
    bool isFirstItem = true;
    for (ECClassId classId : classIds)
        {
        if (!isFirstItem)
            sql.append(" OR ");

        Utf8String classIdStr;
        classIdStr.Sprintf("ECClassId=%lld", classId);
        sql.append(classIdStr);

        isFirstItem = false;
        }
    sql.append(")");

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(GetECDb(), sql.c_str())) << sql.c_str();
    LOG.infov("SQL with class id filter: %s", stmt.GetSql());

    StopWatch timer(true);
    for (ECInstanceId const& id : th3Ids)
        {
        if (BE_SQLITE_OK != stmt.BindId(1, id))
            return;

        if (BE_SQLITE_DONE != stmt.Step())
            return;

        stmt.Reset();
        stmt.ClearBindings();
        }

    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "SQLite UPDATE with ECClassId filter", instanceCount);
    GetECDb().AbandonChanges();
    }

    }

END_ECDBUNITTESTS_NAMESPACE