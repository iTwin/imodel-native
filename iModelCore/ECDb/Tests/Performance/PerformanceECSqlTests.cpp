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
    static void GetECClassIdList(bvector<ECN::ECClassId>& classIds, ECDbCR ecdb, ECN::ECClassCR ecclass, bool includeSubclasses)
        {
        classIds.push_back(ecclass.GetId());
        if (!includeSubclasses)
            return;

        for (ECN::ECClassCP subclass : ecdb.Schemas().GetDerivedECClasses(ecclass))
            {
            GetECClassIdList(classIds, ecdb, *subclass, includeSubclasses);
            }
        }

    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle         10/15
//=======================================================================================
struct PerformanceECSqlVsSqliteTests : PerformanceECSqlTestFixture
    {
private:
    static const int s_instanceCount = 1000000;
    static BeFileName s_seedFilePath;

protected:
    static bmap<ECClassId,std::vector<ECInstanceId>> s_instanceIds;

    BentleyStatus SetupTestECDb(ECDbR ecdb)
        {
        if (s_seedFilePath.empty())
            {
            if (SUCCESS != CreateSeedECDb(s_seedFilePath, "ecsqlperformance_seed.ecdb", BeFileName("ECSqlTest.01.00.ecschema.xml"), 0))
                return ERROR;

            ECDb seed;
            if (BE_SQLITE_OK != seed.OpenBeSQLiteDb(s_seedFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)))
            return ERROR;

            ECSqlStatement stmt;
            if (ECSqlStatus::Success != stmt.Prepare(seed, "INSERT INTO ecsql.TH3(S,S1,S2,S3) VALUES(?,?,?,?)"))
            return ERROR;

            for (int i = 0; i < s_instanceCount; i++)
                {
                for (int j = 0; j < 4; j++)
                    {
                    Utf8String sval;
                    sval.Sprintf("bla %d_%d", j, i);
                    if (ECSqlStatus::Success != stmt.BindText(j + 1, sval.c_str(), IECSqlBinder::MakeCopy::Yes))
                        return ERROR;
                    }

                ECInstanceKey newKey;
                if (BE_SQLITE_DONE != stmt.Step(newKey))
                    return ERROR;

                stmt.Reset();
                stmt.ClearBindings();

                s_instanceIds[newKey.GetECClassId()].push_back(newKey.GetECInstanceId());
                }

            seed.SaveChanges();
            }

        return CloneECDb(ecdb, "ecsqlperformance.ecdb", s_seedFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)) == BE_SQLITE_OK ? SUCCESS : ERROR;
        }
    };

BeFileName PerformanceECSqlVsSqliteTests::s_seedFilePath;
bmap<ECClassId, std::vector<ECInstanceId>> PerformanceECSqlVsSqliteTests::s_instanceIds;

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECSqlVsSqliteTests, SqliteUpdateWithClassIdFilter)
    {
    ECDb ecdb;
    ASSERT_EQ(SUCCESS, SetupTestECDb(ecdb));

    ECClassCP th3Class = ecdb.Schemas().GetECClass("ECSqlTest", "TH3");
    ASSERT_TRUE(th3Class != nullptr);
    bvector<ECClassId> classIds;
    GetECClassIdList(classIds, ecdb, *th3Class, true);

    Utf8String sql("UPDATE [ecsqltest_THBase] SET [S]='S', [S1]='S1', [S2]='S2', [S3]='S3' WHERE [ECInstanceId]=? AND (");
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
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(ecdb, sql.c_str())) << sql.c_str();
    LOG.infov("SQL with class id filter: %s", stmt.GetSql());

    std::vector<ECInstanceId> const& instanceIds = s_instanceIds[th3Class->GetId()];

    StopWatch timer(true);
    for (ECInstanceId const& id : instanceIds)
        {
        if (BE_SQLITE_OK != stmt.BindId(1, id))
            {
            FAIL() << "SQL UPDATE bind failed";
            return;
            }

        if (BE_SQLITE_DONE != stmt.Step())
            {
            FAIL() << "SQL UPDATE Step failed";
            return;
            }

        stmt.Reset();
        stmt.ClearBindings();
        }

    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "SQLite UPDATE with ECClassId filter", (int) instanceIds.size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECSqlVsSqliteTests, SqliteUpdateWithoutClassIdFilter)
    {
    ECDb ecdb;
    ASSERT_EQ(SUCCESS, SetupTestECDb(ecdb));

    ECClassId th3ClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "TH3");
    ASSERT_NE(ECClass::UNSET_ECCLASSID, th3ClassId);
    std::vector<ECInstanceId> const& instanceIds = s_instanceIds[th3ClassId];

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(ecdb, "UPDATE [ecsqltest_THBase] SET [S]='S', [S1]='S1', [S2]='S2', [S3]='S3' WHERE [ECInstanceId]=?"));
    LOG.infov("SQL w/o class id filter: %s", stmt.GetSql());

    StopWatch timer(true);
    for (ECInstanceId const& id : instanceIds)
        {
        if (BE_SQLITE_OK != stmt.BindId(1, id))
            {
            FAIL() << "SQL UPDATE bind failed";
            return;
            }

        if (BE_SQLITE_DONE != stmt.Step())
            {
            FAIL() << "SQL UPDATE Step failed";
            return;
            }

        stmt.Reset();
        stmt.ClearBindings();
        }

    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "SQLite UPDATE w/o ECClassId filter", (int) instanceIds.size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECSqlVsSqliteTests, ECSqlUpdateWithClassIdFilter)
    {
    ECDb ecdb;
    ASSERT_EQ(SUCCESS, SetupTestECDb(ecdb));

    ECClassId th3ClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "TH3");
    ASSERT_NE(ECClass::UNSET_ECCLASSID, th3ClassId);
    std::vector<ECInstanceId> const& instanceIds = s_instanceIds[th3ClassId];

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "UPDATE ecsql.TH3 SET S='S', S1='S1', S2='S2', S3='S3' WHERE ECInstanceId=?"));
    LOG.infov("Translated SQL: %s", stmt.GetNativeSql());

    StopWatch timer(true);
    for (ECInstanceId const& id : instanceIds)
        {
        if (ECSqlStatus::Success != stmt.BindId(1, id))
            {
            FAIL() << "ECSQL UPDATE bind failed";
            return;
            }

        if (BE_SQLITE_DONE != stmt.Step())
            {
            FAIL() << "ECSQL UPDATE Step failed";
            return;
            }

        stmt.Reset();
        stmt.ClearBindings();
        }

    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "ECSQL UPDATE", (int) instanceIds.size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECSqlVsSqliteTests, ECSqlUpdateWithoutClassIdFilter)
    {
    ECDb ecdb;
    ASSERT_EQ(SUCCESS, SetupTestECDb(ecdb));

    ECClassId th3ClassId = ecdb.Schemas().GetECClassId("ECSqlTest", "TH3");
    ASSERT_NE(ECClass::UNSET_ECCLASSID, th3ClassId);
    std::vector<ECInstanceId> const& instanceIds = s_instanceIds[th3ClassId];

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "UPDATE ecsql.TH3 SET S='S', S1='S1', S2='S2', S3='S3' WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter"));
    LOG.infov("Translated SQL: %s", stmt.GetNativeSql());

    StopWatch timer(true);
    for (ECInstanceId const& id : instanceIds)
        {
        if (ECSqlStatus::Success != stmt.BindId(1, id))
            {
            FAIL() << "ECSQL UPDATE bind failed";
            return;
            }

        if (BE_SQLITE_DONE != stmt.Step())
            {
            FAIL() << "ECSQL UPDATE Step failed";
            return;
            }

        stmt.Reset();
        stmt.ClearBindings();
        }

    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "ECSQL UPDATE OPTIONS NoECClassIdFilter", (int) instanceIds.size());
    }

END_ECDBUNITTESTS_NAMESPACE