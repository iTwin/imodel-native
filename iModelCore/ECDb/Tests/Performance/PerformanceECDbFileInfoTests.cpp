/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceECDbFileInfoTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"
#include <algorithm>

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// @bsiclass                                                Krischan.Eberle         11/15
//=======================================================================================
struct PerformanceECDbFileInfoTests : ECDbTestFixture
    {
private:
    static BeFileName s_seedFilePath;

protected:
    struct FileInfoStats
        {
        int m_fileInfoCount;
        int m_ownershipCount;

        FileInfoStats() : m_fileInfoCount(-1), m_ownershipCount(-1) {}
        FileInfoStats(int fileInfoCount, int ownershipCount) : m_fileInfoCount(fileInfoCount), m_ownershipCount(ownershipCount) {}
        };

    static const int s_initialInstanceCountPerClass = 1000;
    static const int s_fileInfoCountPerClass = 200;

    virtual void TearDown() override 
        {
        GetECDb().AbandonChanges();
        }

    static bool IsSchemaWithoutFilebackedInstances(ECN::ECSchemaCR schema)
        {
        return schema.IsStandardSchema() || schema.IsSystemSchema() || schema.GetName().EqualsI("ecdb_fileinfo");
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                      Affan.Khan                  10/15
    //+---------------+---------------+---------------+---------------+---------------+------
    BentleyStatus SetupTestECDb()
        {
        if (s_fileInfoCountPerClass > s_initialInstanceCountPerClass)
            {
            BeAssert(false && "s_fileInfoCountPerClass must not be higher than s_initialInstanceCountPerClass");
            return ERROR;
            }

        Utf8String seedFileName;
        bool mustCreateSeed = false;
        if (s_seedFilePath.empty())
            {
            seedFileName.Sprintf("performance_ecdbfileinfo_seed_%d.ecdb", DateTime::GetCurrentTimeUtc().GetDayOfYear());

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
            ECDbR seed = SetupECDb(seedFileName.c_str(), BeFileName(L"ECSqlTest.01.00.ecschema.xml"), s_initialInstanceCountPerClass);

            //test file
            BeFileName testFilePath;
            BeTest::GetHost().GetDocumentsRoot(testFilePath);
            testFilePath.AppendToPath(L"ECDb");
            testFilePath.AppendToPath(L"StartupCompany.json");

            ECClassId embeddedFileInfoClassId = GetECDb().Schemas().GetECClassId("ECDb_FileInfo", "EmbeddedFileInfo");
            if (!embeddedFileInfoClassId.IsValid())
                return ERROR;

            ECSqlStatement insertExternalFileInfoStmt;
            ECSqlStatement insertOwnershipStmt;
            if (ECSqlStatus::Success != insertExternalFileInfoStmt.Prepare(GetECDb(), "INSERT INTO ecdbf.ExternalFileInfo(Name, RootFolder, RelativePath) VALUES(?,1,'mydocs/something/')"))
                return ERROR;
            if (ECSqlStatus::Success != insertOwnershipStmt.Prepare(GetECDb(), "INSERT INTO ecdbf.FileInfoOwnership(OwnerId, OwnerECClassId, FileInfoId, FileInfoECClassId) VALUES(?,?,?,?)"))
                return ERROR;

            bvector<ECN::ECSchemaCP> schemas;
            GetECDb().Schemas().GetECSchemas(schemas, true);
            for (ECSchemaCP schema : schemas)
                {
                if (IsSchemaWithoutFilebackedInstances(*schema))
                    continue;

                for (ECClassCP ecclass : schema->GetClasses())
                    {
                    if (ECClassModifier::Abstract == ecclass->GetClassModifier())
                        continue;

                    Utf8String ecsql;
                    ecsql.Sprintf("SELECT ECInstanceId, GetECClassId() FROM ONLY %s LIMIT %d",
                                  ecclass->GetECSqlName().c_str(), s_fileInfoCountPerClass);

                    ECSqlStatement stmt;
                    if (ECSqlStatus::Success != stmt.Prepare(GetECDb(), ecsql.c_str()))
                        continue; //classes that cannot be used in ECSQL are just ignored

                    int i = 0;
                    while (stmt.Step() == BE_SQLITE_ROW)
                        {
                        ECInstanceId ownerId = stmt.GetValueId<ECInstanceId>(0);
                        i++;
                        Utf8String fileInfoName;
                        fileInfoName.Sprintf("file_for_%s_%llu", ecclass->GetFullName(), ownerId.GetValue());

                        ECInstanceKey newKey;
                        if (i % 2 == 0)
                            {
                            insertExternalFileInfoStmt.BindText(1, fileInfoName.c_str(), IECSqlBinder::MakeCopy::No);
                            if (BE_SQLITE_DONE != insertExternalFileInfoStmt.Step(newKey))
                                {
                                LOG.fatal(GetECDb().GetLastError().c_str());
                                return ERROR;
                                }

                            insertExternalFileInfoStmt.Reset();
                            insertExternalFileInfoStmt.ClearBindings();
                            }
                        else
                            {
                            DbResult stat = BE_SQLITE_OK;
                            BeBriefcaseBasedId newId = GetECDb().EmbeddedFiles().Import(&stat, fileInfoName.c_str(), testFilePath.GetNameUtf8().c_str(), "dummy");
                            if (BE_SQLITE_OK != stat)
                                {
                                LOG.fatal(GetECDb().GetLastError().c_str());
                                return ERROR;
                                }

                            newKey = ECInstanceKey(embeddedFileInfoClassId, ECInstanceId(newId.GetValue()));
                            }

                        insertOwnershipStmt.BindId(1, ownerId);
                        insertOwnershipStmt.BindInt64(2, stmt.GetValueInt64(1));
                        insertOwnershipStmt.BindId(3, newKey.GetECInstanceId());
                        insertOwnershipStmt.BindId(4, newKey.GetECClassId());
                        if (BE_SQLITE_DONE != insertOwnershipStmt.Step())
                            {
                            LOG.fatal(GetECDb().GetLastError().c_str());
                            return ERROR;
                            }

                        insertOwnershipStmt.Reset();
                        insertOwnershipStmt.ClearBindings();
                        }
                    }
                }

            insertExternalFileInfoStmt.Finalize();
            insertOwnershipStmt.Finalize();
            s_seedFilePath.AssignUtf8(seed.GetDbFileName());
            seed.SaveChanges();
            seed.CloseDb();
            }

        return CloneECDb(m_ecdb, "performance_ecdbfileinfo.ecdb", s_seedFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)) == BE_SQLITE_OK ? SUCCESS : ERROR;
        }

    BentleyStatus DeleteOwners(int& deletedCount, ECClassId ownerClassId, int count) const
        {
        deletedCount = 0;
        ECClassCP ownerClass = GetECDb().Schemas().GetECClass(ownerClassId);
        if (ownerClass == nullptr)
            return ERROR;

        Utf8CP ecclassECSqlToken = ownerClass->GetECSqlName().c_str();
        Utf8String getCountECSql;
        getCountECSql.Sprintf("SELECT count(*) FROM ONLY %s", ecclassECSqlToken);
        ECSqlStatement getCountStmt;
        if (ECSqlStatus::Success != getCountStmt.Prepare(GetECDb(), getCountECSql.c_str()))
            return SUCCESS;//some classes cannot be used in ECSQL. ignore them.

        if (BE_SQLITE_ROW != getCountStmt.Step())
            return ERROR;

        const int beforeCount = getCountStmt.GetValueInt(0);
        getCountStmt.Reset();

        if (beforeCount == 0)
            return SUCCESS; //ignore empty classes

        Utf8String ecsql;
        if (count >= 0)
            ecsql.Sprintf("DELETE FROM ONLY %s WHERE ECInstanceId IN (SELECT OwnerId FROM ONLY ecdbf.FileInfoOwnership WHERE OwnerECClassId=%llu LIMIT %d)",
                          ecclassECSqlToken, ownerClassId.GetValue(), count);
        else
            ecsql.Sprintf("DELETE FROM ONLY %s", ecclassECSqlToken);

        ECSqlStatement deleteOwnerStmt;
        if (ECSqlStatus::Success != deleteOwnerStmt.Prepare(GetECDb(), ecsql.c_str()))
            return ERROR;

        if (BE_SQLITE_DONE != deleteOwnerStmt.Step())
            return ERROR;

        if (BE_SQLITE_ROW != getCountStmt.Step())
            return ERROR;

        const int afterCount = getCountStmt.GetValueInt(0);
        deletedCount = beforeCount - afterCount;
        if (count < 0)
            return afterCount == 0 ? SUCCESS : ERROR;

        if ((beforeCount - count) != afterCount)
            {
            BeAssert(false && "Unexpected count after deletion.");
            return ERROR;
            }

        return SUCCESS;
        }

    BentleyStatus RetrieveFileInfoStats(FileInfoStats& stats) const
        {
        ECSqlStatement stmt;
        if (ECSqlStatus::Success != stmt.Prepare(GetECDb(), "SELECT count(*) FROM ONLY ecdbf.FileInfoOwnership"))
            return ERROR;

        if (BE_SQLITE_ROW != stmt.Step())
            return ERROR;

        stats.m_ownershipCount = stmt.GetValueInt(0);

        stmt.Finalize();
        if (ECSqlStatus::Success != stmt.Prepare(GetECDb(), "SELECT count(*) FROM ecdbf.FileInfo"))
            return ERROR;

        if (BE_SQLITE_ROW != stmt.Step())
            return ERROR;

        stats.m_fileInfoCount = stmt.GetValueInt(0);
        return SUCCESS;
        }


    void LogTiming(Utf8CP scenario, StopWatch& timer) const
        {
        Utf8String testDescr;
        testDescr.Sprintf("%s [Initial instance count per class: %d - FileInfos assigned to %d instances per class]",
                            scenario, s_initialInstanceCountPerClass, s_fileInfoCountPerClass);
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), testDescr.c_str(), 1);
        }

    };

BeFileName PerformanceECDbFileInfoTests::s_seedFilePath;

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                  11/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECDbFileInfoTests, CreateSeedFile)
    {
    //separate out code that creates and populates the seed files, so that multiple runs of the actual
    //perf timings can be done without influence of the heavy work to create the seed file.
    ASSERT_EQ(SUCCESS, SetupTestECDb());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                  11/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECDbFileInfoTests, PurgeAfterSingleDeletion)
    {
    ASSERT_EQ(SUCCESS, SetupTestECDb());
    
    ECClassId testClassId = GetECDb().Schemas().GetECClassId("ECSqlTest", "P");
    ASSERT_TRUE(testClassId.IsValid());

    FileInfoStats beforeDeleteStats;
    ASSERT_EQ(SUCCESS, RetrieveFileInfoStats(beforeDeleteStats));
    ASSERT_EQ(beforeDeleteStats.m_fileInfoCount, beforeDeleteStats.m_ownershipCount) << "Before deleting anything";

    int deletedCount = -1;
    ASSERT_EQ(SUCCESS, DeleteOwners(deletedCount, testClassId, 1));
    ASSERT_EQ(1, deletedCount) << "After deleting one owner";

    FileInfoStats beforePurgeStats;
    ASSERT_EQ(SUCCESS, RetrieveFileInfoStats(beforePurgeStats));
    ASSERT_EQ(beforeDeleteStats.m_fileInfoCount, beforePurgeStats.m_fileInfoCount) << "FileInfo count after deleting one owner without purging";
    ASSERT_EQ(beforeDeleteStats.m_ownershipCount, beforePurgeStats.m_ownershipCount) << "Ownership count after deleting one owner without purging";
    ASSERT_EQ(beforePurgeStats.m_fileInfoCount, beforePurgeStats.m_ownershipCount) << "Before purging";

    BentleyStatus purgeStat = SUCCESS;
    StopWatch timer(true);
    purgeStat = GetECDb().Purge(ECDb::PurgeMode::FileInfoOwnerships);
    timer.Stop();
    ASSERT_EQ(SUCCESS, purgeStat);

    FileInfoStats afterPurgeStats;
    ASSERT_EQ(SUCCESS, RetrieveFileInfoStats(afterPurgeStats));
    ASSERT_EQ(beforePurgeStats.m_fileInfoCount, afterPurgeStats.m_fileInfoCount);
    ASSERT_EQ(beforePurgeStats.m_ownershipCount - deletedCount, afterPurgeStats.m_ownershipCount);

    LogTiming("Purge after deletion of one file backed instance", timer);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                  11/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECDbFileInfoTests, PurgeAfterDeletionOfAllInstancesOfSingleClass)
    {
    ASSERT_EQ(SUCCESS, SetupTestECDb());
    ECClassId testClassId = GetECDb().Schemas().GetECClassId("ECSqlTest", "P");
    ASSERT_TRUE(testClassId.IsValid());
    
    int deletedCount = -1;
    ASSERT_EQ(SUCCESS, DeleteOwners(deletedCount, testClassId, -1));

    FileInfoStats beforePurgeStats;
    ASSERT_EQ(SUCCESS, RetrieveFileInfoStats(beforePurgeStats));
    ASSERT_EQ(beforePurgeStats.m_fileInfoCount, beforePurgeStats.m_ownershipCount);

    BentleyStatus purgeStat = SUCCESS;
    StopWatch timer(true);
    purgeStat = GetECDb().Purge(ECDb::PurgeMode::FileInfoOwnerships);
    timer.Stop();
    ASSERT_EQ(SUCCESS, purgeStat);

    FileInfoStats afterPurgeStats;
    ASSERT_EQ(SUCCESS, RetrieveFileInfoStats(afterPurgeStats));
    const int deletedFileBackedInstances = std::min(s_fileInfoCountPerClass, deletedCount);
    ASSERT_EQ(beforePurgeStats.m_fileInfoCount, afterPurgeStats.m_fileInfoCount);
    ASSERT_EQ(beforePurgeStats.m_ownershipCount - deletedFileBackedInstances, afterPurgeStats.m_ownershipCount);

    LogTiming("Purge after deletion of all instances of a single file-backed ECClass", timer);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                  11/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECDbFileInfoTests, PurgeAfterDeletionOfOneInstancePerClass)
    {
    ASSERT_EQ(SUCCESS, SetupTestECDb());

    int deletedCount = 0;
    bvector<ECN::ECSchemaCP> schemas;
    GetECDb().Schemas().GetECSchemas(schemas, true);
    for (ECSchemaCP schema : schemas)
        {
        if (IsSchemaWithoutFilebackedInstances(*schema))
            continue;

        for (ECClassCP ecclass : schema->GetClasses())
            {
            if (ECClassModifier::Abstract == ecclass->GetClassModifier())
                continue;

            int deletedCountPerClass = 0;
            printf("className: %s\n", ecclass->GetName().c_str());
            ASSERT_EQ(SUCCESS, DeleteOwners(deletedCountPerClass, ecclass->GetId(), 1));
            deletedCount += deletedCountPerClass;
            }
        }

    FileInfoStats beforePurgeStats;
    ASSERT_EQ(SUCCESS, RetrieveFileInfoStats(beforePurgeStats));
    ASSERT_EQ(beforePurgeStats.m_fileInfoCount, beforePurgeStats.m_ownershipCount);

    BentleyStatus purgeStat = SUCCESS;
    StopWatch timer(true);
    purgeStat = GetECDb().Purge(ECDb::PurgeMode::FileInfoOwnerships);
    timer.Stop();
    ASSERT_EQ(SUCCESS, purgeStat);

    FileInfoStats afterPurgeStats;
    ASSERT_EQ(SUCCESS, RetrieveFileInfoStats(afterPurgeStats));
    ASSERT_EQ(beforePurgeStats.m_fileInfoCount, afterPurgeStats.m_fileInfoCount);
    ASSERT_EQ(beforePurgeStats.m_ownershipCount - deletedCount, afterPurgeStats.m_ownershipCount);

    LogTiming("Purge after deletion of one instance per file-backed ECClass", timer);
    }

END_ECDBUNITTESTS_NAMESPACE