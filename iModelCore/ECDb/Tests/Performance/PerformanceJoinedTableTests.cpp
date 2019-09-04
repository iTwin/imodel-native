/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// @bsiclass                                                Affan.Khan         10/15
//=======================================================================================
struct PerformanceJoinedTableTests: ECDbTestFixture
    {
    private:
        static BeFileName s_seedFilePath;

    protected:
        static const uint64_t s_firstInstanceId = UINT64_C(1);
        static const int s_initialInstanceCount = 10000;
        static const int s_opCount = 5000;

        //---------------------------------------------------------------------------------------
        // @bsimethod                                      Affan.Khan                  10/15
        //+---------------+---------------+---------------+---------------+---------------+------
        BentleyStatus SetupTestECDb()
            {
            Utf8String seedFileName;
            bool mustCreateSeed = false;
            if (s_seedFilePath.empty())
                {
                seedFileName.Sprintf("performance_joinedtable_seed_%d.ecdb", DateTime::GetCurrentTimeUtc().GetDayOfYear());

                BeFileName seedPath = BuildECDbPath(seedFileName.c_str());
                //if seed file exists on disk, we reuse it. This is risky if other tests happen to create file with same name
                //but we add the current day of the year to the file name, to make sure it would never be reused after 24h.
                if (seedPath.DoesPathExist())
                    s_seedFilePath = seedPath;
                else
                    mustCreateSeed = true;
                }

            if (mustCreateSeed)
                {
                SchemaItem testSchema(
                    "<?xml version='1.0' encoding='utf-8'?>"
                    "<ECSchema schemaName='JoinedTableTest' alias='dgn' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                    "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                    "    <ECEntityClass typeName='Foo' >"
                    "        <ECCustomAttributes>"
                    "            <ClassMap xmlns='ECDbMap.02.00'>"
                    "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                    "            </ClassMap>"
                    "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                    "        </ECCustomAttributes>"
                    "        <ECProperty propertyName='F1l' typeName='long'/>"
                    "        <ECProperty propertyName='F2s' typeName='string'/>"
                    "        <ECProperty propertyName='F3l' typeName='long'/>"
                    "        <ECProperty propertyName='F4s' typeName='string'/>"
                    "    </ECEntityClass>"
                    "   <ECEntityClass typeName='Goo' >"
                    "        <BaseClass>Foo</BaseClass>"
                    "        <ECProperty propertyName='G1l' typeName='long'/>"
                    "        <ECProperty propertyName='G2s' typeName='string'/>"
                    "        <ECProperty propertyName='G3l' typeName='long'/>"
                    "        <ECProperty propertyName='G4s' typeName='string'/>"
                    "    </ECEntityClass>"
                    "   <ECEntityClass typeName='Boo' >"
                    "        <BaseClass>Goo</BaseClass>"
                    "        <ECProperty propertyName='B1l' typeName='long'/>"
                    "        <ECProperty propertyName='B2s' typeName='string'/>"
                    "        <ECProperty propertyName='B3l' typeName='long'/>"
                    "        <ECProperty propertyName='B4s' typeName='string'/>"
                    "    </ECEntityClass>"
                    "</ECSchema>");

                if (SUCCESS != SetupECDb(seedFileName.c_str(), testSchema))
                    return ERROR;

                ECSqlStatement stmt;
                if (ECSqlStatus::Success != stmt.Prepare(m_ecdb, "INSERT INTO dgn.Boo(ECInstanceId, F1l,F2s,F3l,F4s,G1l,G2s,G3l,G4s,B1l,B2s,B3l,B4s) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?)"))
                    return ERROR;

                for (int i = 0; i < s_initialInstanceCount; i++)
                    {
                    Utf8String txt = Utf8PrintfString("test_%d", i).c_str();
                    auto k = s_firstInstanceId + i;
                    if (ECSqlStatus::Success != stmt.BindId(1, ECInstanceId(k)))
                        return ERROR;

                    if (ECSqlStatus::Success != stmt.BindInt(2, i)) return ERROR;
                    if (ECSqlStatus::Success != stmt.BindText(3, txt.c_str(), IECSqlBinder::MakeCopy::No)) return ERROR;
                    if (ECSqlStatus::Success != stmt.BindInt(4, i)) return ERROR;
                    if (ECSqlStatus::Success != stmt.BindText(5, txt.c_str(), IECSqlBinder::MakeCopy::No)) return ERROR;
                    if (ECSqlStatus::Success != stmt.BindInt(6, i)) return ERROR;
                    if (ECSqlStatus::Success != stmt.BindText(7, txt.c_str(), IECSqlBinder::MakeCopy::No)) return ERROR;
                    if (ECSqlStatus::Success != stmt.BindInt(8, i)) return ERROR;
                    if (ECSqlStatus::Success != stmt.BindText(9, txt.c_str(), IECSqlBinder::MakeCopy::No)) return ERROR;
                    if (ECSqlStatus::Success != stmt.BindInt(10, i)) return ERROR;
                    if (ECSqlStatus::Success != stmt.BindText(11, txt.c_str(), IECSqlBinder::MakeCopy::No)) return ERROR;
                    if (ECSqlStatus::Success != stmt.BindInt(12, i)) return ERROR;
                    if (ECSqlStatus::Success != stmt.BindText(13, txt.c_str(), IECSqlBinder::MakeCopy::No)) return ERROR;
                    if (stmt.Step() != BE_SQLITE_DONE)
                        return ERROR;

                    stmt.Reset();
                    stmt.ClearBindings();
                    }

                stmt.Finalize();
                s_seedFilePath.AssignUtf8(m_ecdb.GetDbFileName());
                m_ecdb.SaveChanges();
                m_ecdb.CloseDb();
                }

            return CloneECDb("performance_joinedTable.ecdb", s_seedFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)) == BE_SQLITE_OK ? SUCCESS : ERROR;
            }


        //---------------------------------------------------------------------------------------
        // @bsimethod                                      Affan.Khan                  10/15
        //+---------------+---------------+---------------+---------------+---------------+------
        void LogTiming(StopWatch& timer, Utf8CP testDescription, int actualOpCount)
            {
            ASSERT_EQ(s_opCount, actualOpCount) << "Unexpected actual op count";
            Utf8String totalTestDescr;
            totalTestDescr.Sprintf("%s, initial instance count %d", testDescription, s_initialInstanceCount);
            LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), actualOpCount, totalTestDescr.c_str());
            }
    };

BeFileName PerformanceJoinedTableTests::s_seedFilePath;

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceJoinedTableTests, CreateSeedFile)
    {
    //separate out code that creates and populates the seed files, so that multiple runs of the actual
    //perf timings can be done without influence of the heavy work to create the seed file.
    ASSERT_EQ(SUCCESS, SetupTestECDb());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceJoinedTableTests, Insert)
    {
    ASSERT_EQ(SUCCESS, SetupTestECDb());

    ECSqlStatement booInsert;
    // NOT USED: const int instanceIdIncrement = DetermineECInstanceIdIncrement();
    ASSERT_EQ(booInsert.Prepare(m_ecdb, "INSERT INTO dgn.Boo(ECInstanceId, F1l,F2s,F3l,F4s,G1l,G2s,G3l,G4s,B1l,B2s,B3l,B4s) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?)"), ECSqlStatus::Success);
    StopWatch timer(true);

    for (int i = 0; i < s_opCount; i++)
        {
        
        ECInstanceId id(s_firstInstanceId + s_initialInstanceCount + i);
        Utf8String txt = Utf8PrintfString("test_%d", i).c_str();
        ASSERT_EQ(booInsert.BindId(1, id), ECSqlStatus::Success);
        ASSERT_EQ(booInsert.BindInt(2, i), ECSqlStatus::Success);
        ASSERT_EQ(booInsert.BindText(3, txt.c_str(), IECSqlBinder::MakeCopy::No), ECSqlStatus::Success);
        ASSERT_EQ(booInsert.BindInt(4, i), ECSqlStatus::Success);
        ASSERT_EQ(booInsert.BindText(5, txt.c_str(), IECSqlBinder::MakeCopy::No), ECSqlStatus::Success);
        ASSERT_EQ(booInsert.BindInt(6, i), ECSqlStatus::Success);
        ASSERT_EQ(booInsert.BindText(7, txt.c_str(), IECSqlBinder::MakeCopy::No), ECSqlStatus::Success);
        ASSERT_EQ(booInsert.BindInt(8, i), ECSqlStatus::Success);
        ASSERT_EQ(booInsert.BindText(9, txt.c_str(), IECSqlBinder::MakeCopy::No), ECSqlStatus::Success);
        ASSERT_EQ(booInsert.BindInt(10, i), ECSqlStatus::Success);
        ASSERT_EQ(booInsert.BindText(11, txt.c_str(), IECSqlBinder::MakeCopy::No), ECSqlStatus::Success);
        ASSERT_EQ(booInsert.BindInt(12, i), ECSqlStatus::Success);
        ASSERT_EQ(booInsert.BindText(13, txt.c_str(), IECSqlBinder::MakeCopy::No), ECSqlStatus::Success);
        ASSERT_EQ(booInsert.Step(), BE_SQLITE_DONE);
        ASSERT_EQ(booInsert.Reset(), ECSqlStatus::Success);
        ASSERT_EQ(booInsert.ClearBindings(), ECSqlStatus::Success);
        }
    timer.Stop();
    LogTiming(timer, "ECSQL INSERT (JoinedTablePerDirectSubclass) ", s_opCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceJoinedTableTests, Update)
    {
    ASSERT_EQ(SUCCESS, SetupTestECDb());

    ECSqlStatement booUpdate;
    ASSERT_EQ(booUpdate.Prepare(m_ecdb, "UPDATE dgn.Boo SET F1l = ?,F2s = ?,F3l = ?,F4s = ?,G1l = ?,G2s = ?,G3l = ?,G4s = ?,B1l = ?,B2s = ?,B3l = ?,B4s = ? WHERE ECInstanceId = ?"), ECSqlStatus::Success);
    StopWatch timer(true);
    for (int i = 0; i < s_opCount; i++)
        {
        ECInstanceId id(s_firstInstanceId + i);
        Utf8String txt = Utf8PrintfString("test_updated_%d", i).c_str();
        auto updatedInt = i + 2;
        ASSERT_EQ(booUpdate.BindInt(1, updatedInt), ECSqlStatus::Success);
        ASSERT_EQ(booUpdate.BindText(2, txt.c_str(), IECSqlBinder::MakeCopy::No), ECSqlStatus::Success);
        ASSERT_EQ(booUpdate.BindInt(3, updatedInt), ECSqlStatus::Success);
        ASSERT_EQ(booUpdate.BindText(4, txt.c_str(), IECSqlBinder::MakeCopy::No), ECSqlStatus::Success);
        ASSERT_EQ(booUpdate.BindInt(5, updatedInt), ECSqlStatus::Success);
        ASSERT_EQ(booUpdate.BindText(6, txt.c_str(), IECSqlBinder::MakeCopy::No), ECSqlStatus::Success);
        ASSERT_EQ(booUpdate.BindInt(7, updatedInt), ECSqlStatus::Success);
        ASSERT_EQ(booUpdate.BindText(8, txt.c_str(), IECSqlBinder::MakeCopy::No), ECSqlStatus::Success);
        ASSERT_EQ(booUpdate.BindInt(9, updatedInt), ECSqlStatus::Success);
        ASSERT_EQ(booUpdate.BindText(10, txt.c_str(), IECSqlBinder::MakeCopy::No), ECSqlStatus::Success);
        ASSERT_EQ(booUpdate.BindInt(11, updatedInt), ECSqlStatus::Success);
        ASSERT_EQ(booUpdate.BindText(12, txt.c_str(), IECSqlBinder::MakeCopy::No), ECSqlStatus::Success);
        ASSERT_EQ(booUpdate.BindId(13, id), ECSqlStatus::Success);
        ASSERT_EQ(booUpdate.Step(), BE_SQLITE_DONE);
        ASSERT_EQ(booUpdate.Reset(), ECSqlStatus::Success);
        ASSERT_EQ(booUpdate.ClearBindings(), ECSqlStatus::Success);
        }

    timer.Stop();
    LogTiming(timer, "ECSQL UPDATE (JoinedTablePerDirectSubclass) ", s_opCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceJoinedTableTests, Select)
    {
    ASSERT_EQ(SUCCESS, SetupTestECDb());

    ECSqlStatement booSelect;
    ASSERT_EQ(booSelect.Prepare(m_ecdb, "SELECT F1l,F2s,F3l,F4s,G1l,G2s,G3l,G4s,B1l,B2s,B3l,B4s FROM dgn.Boo WHERE ECInstanceId = ?"), ECSqlStatus::Success);
    StopWatch timer(true);
    for (int i = 0; i < s_opCount; i++)
        {
        ECInstanceId id(s_firstInstanceId + i);
        booSelect.ClearBindings();
        booSelect.Reset();
        booSelect.BindId(1, id);
        if (booSelect.Step() == BE_SQLITE_ROW)
            {
            booSelect.GetValueInt(0);
            booSelect.GetValueText(1);
            booSelect.GetValueInt(2);
            booSelect.GetValueText(3);
            booSelect.GetValueInt(4);
            booSelect.GetValueText(5);
            booSelect.GetValueInt(6);
            booSelect.GetValueText(7);
            booSelect.GetValueInt(8);
            booSelect.GetValueText(9);
            booSelect.GetValueInt(10);
            booSelect.GetValueText(11);
            }
        else
            {
            FAIL() << "Must find row";
            }
        }

    timer.Stop();
    LogTiming(timer, "ECSQL SELECT (JoinedTablePerDirectSubclass) ", s_opCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceJoinedTableTests, Delete)
    {
    ASSERT_EQ(SUCCESS, SetupTestECDb());

    ECSqlStatement booDelete;
    ASSERT_EQ(booDelete.Prepare(m_ecdb, "DELETE FROM dgn.Boo WHERE ECInstanceId = ?"), ECSqlStatus::Success);
    StopWatch timer(true);
    for (auto i = 0; i < s_opCount; i++)
        {
        ECInstanceId id(s_firstInstanceId + i);
        booDelete.ClearBindings();
        booDelete.Reset();
        booDelete.BindId(1, id);
        if (booDelete.Step() != BE_SQLITE_DONE)
            {
            FAIL() << "Must delete row";
            }
        }

    timer.Stop();
    LogTiming(timer, "ECSQL DELETE (JoinedTablePerDirectSubclass) ", s_opCount);
    }


END_ECDBUNITTESTS_NAMESPACE
