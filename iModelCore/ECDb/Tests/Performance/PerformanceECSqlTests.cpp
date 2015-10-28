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
    static const int s_initialInstanceCount = 1000000;
    static const int s_opCount = 500000;
    static BeFileName s_seedFilePath;

protected:
    bmap<ECClassId,std::vector<ECInstanceId>> s_instanceIds;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                      Krischan.Eberle                  10/15
    //+---------------+---------------+---------------+---------------+---------------+------
    BentleyStatus CreateSampleIdPopulation(ECDbCR ecdb)
        {
        if (!s_instanceIds.empty())
            return SUCCESS;

        ECDb seed;
        if (BE_SQLITE_OK != seed.OpenBeSQLiteDb(s_seedFilePath, ECDb::OpenParams(Db::OpenMode::Readonly)))
            return ERROR;

        ECSqlStatement stmt;
        if (ECSqlStatus::Success != stmt.Prepare(seed, "SELECT GetECClassId(), ECInstanceId FROM ONLY ecsql.TH3"))
            return ERROR;

        const int increment = s_initialInstanceCount / s_opCount;
        if (increment * s_opCount > s_initialInstanceCount)
            {
            EXPECT_LE(increment * s_opCount, s_initialInstanceCount);
            return ERROR;
            }

        for (int i = 0; i < s_opCount; i++)
            {
            for (int j = 0; j < increment; j++)
                {
                if (BE_SQLITE_ROW != stmt.Step())
                    return ERROR;
                }

            ECClassId classId = stmt.GetValueInt64(0);
            ECInstanceId id = stmt.GetValueId<ECInstanceId>(1);
            if (classId == ECClass::UNSET_ECCLASSID || !id.IsValid())
                return ERROR;

            s_instanceIds[classId].push_back(id);
            }

        return SUCCESS;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                      Krischan.Eberle                  10/15
    //+---------------+---------------+---------------+---------------+---------------+------
    BentleyStatus SetupTestECDb(ECDbR ecdb)
        {
        Utf8String seedFileName;
        bool mustCreateSeed = false;
        if (s_seedFilePath.empty())
            {
            seedFileName.Sprintf("ecsqlvssqliteperformance_seed_%d.ecdb", DateTime::GetCurrentTimeUtc().GetDayOfYear());

            BeFileName seedPath = ECDbTestUtility::BuildECDbPath(seedFileName.c_str());
            //if seed file exists on disk, we reuse it. This is risky if other tests happen to create file with same name
            //but we add the current day of the year to the file name, to make sure it would never be reused after 24h.
            if (seedPath.DoesPathExist())
                s_seedFilePath = seedPath;
            else
                mustCreateSeed = true;
            }

        if (mustCreateSeed)
            {
            if (SUCCESS != CreateSeedECDb(s_seedFilePath, seedFileName.c_str(), BeFileName("ECSqlTest.01.00.ecschema.xml"), 0))
                return ERROR;

            ECDb seed;
            if (BE_SQLITE_OK != seed.OpenBeSQLiteDb(s_seedFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)))
                return ERROR;

            ECSqlStatement stmt;
            if (ECSqlStatus::Success != stmt.Prepare(seed, "INSERT INTO ecsql.TH3(S,S1,S2,S3) VALUES(?,?,?,?)"))
                return ERROR;

            for (int i = 0; i < s_initialInstanceCount; i++)
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
                }

            seed.SaveChanges();
            }
        
        if (SUCCESS != CreateSampleIdPopulation(ecdb))
            return ERROR;

        return CloneECDb(ecdb, "ecsqlvssqliteperformance.ecdb", s_seedFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)) == BE_SQLITE_OK ? SUCCESS : ERROR;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                      Krischan.Eberle                  10/15
    //+---------------+---------------+---------------+---------------+---------------+------
    void LogTiming(StopWatch& timer, Utf8CP testDescription, int actualOpCount)
        {
        ASSERT_EQ(s_opCount, actualOpCount) << "Unexpected actual op count";
        Utf8String totalTestDescr;
        totalTestDescr.Sprintf("%s [initial instance count: %d]", testDescription, s_initialInstanceCount);
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), totalTestDescr, actualOpCount);
        }
    };

BeFileName PerformanceECSqlVsSqliteTests::s_seedFilePath;

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECSqlVsSqliteTests, CreateSeedFile)
    {
    //separate out code that creates and populates the seed files, so that multiple runs of the actual
    //perf timings can be done without influence of the heavy work to create the seed file.
    ECDb ecdb;
    ASSERT_EQ(SUCCESS, SetupTestECDb(ecdb));
    }

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
    LogTiming(timer, "SQLite UPDATE with ECClassId filter", (int) instanceIds.size());
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
    LogTiming(timer, "SQLite UPDATE w/o ECClassId filter", (int) instanceIds.size());
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
    LogTiming(timer, "ECSQL UPDATE", (int) instanceIds.size());
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

    //printf("Attach to profiler...\r\n"); getchar();
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
    //printf("Detach...\r\n"); getchar();
    LogTiming(timer, "ECSQL UPDATE OPTIONS NoECClassIdFilter", (int) instanceIds.size());
    }

END_ECDBUNITTESTS_NAMESPACE