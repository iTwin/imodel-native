/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/BeSQLiteDb_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "BeSQLiteNonPublishedTests.h"
#include <map>
#include <vector>

//---------------------------------------------------------------------------------------
// Creating a new Db for the test
// @bsimethod                                    Krischan.Eberle                   01/14
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SetupDb (Db& db, WCharCP dbName)
    {
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir (tempDir);
    BeSQLiteLib::Initialize (tempDir);

    BeFileName dbFileName;
    BeTest::GetHost ().GetOutputRoot (dbFileName);
    dbFileName.AppendToPath (dbName);

    if (BeFileName::DoesPathExist (dbFileName))
        BeFileName::BeDeleteFile (dbFileName);
    
    DbResult result = db.CreateNewDb (dbFileName.GetNameUtf8 ().c_str ());
    EXPECT_EQ (BE_SQLITE_OK, result) << "Db Creation failed";
    if (result == BE_SQLITE_OK)
        db.SaveChanges ();

    return result;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     11/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST(BeSQLiteDb, CheckProfileVersionWithEmptyProfileName)
    {
    SchemaVersion dummy (2, 4, 5, 3);

    bool fileIsAutoUpgradable = false;

    BeTest::SetFailOnAssert(false);
    DbResult actualStat = Db::CheckProfileVersion(fileIsAutoUpgradable, dummy, dummy,
        dummy, true, nullptr);
    BeTest::SetFailOnAssert(true);

    ASSERT_FALSE (fileIsAutoUpgradable);
    ASSERT_EQ (BE_SQLITE_INTERNAL, actualStat);

    BeTest::SetFailOnAssert(false);
    actualStat = Db::CheckProfileVersion(fileIsAutoUpgradable, dummy, dummy,
        dummy, true, "");
    BeTest::SetFailOnAssert(true);

    ASSERT_FALSE (fileIsAutoUpgradable);
    ASSERT_EQ (BE_SQLITE_INTERNAL, actualStat);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     11/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST(BeSQLiteDb, CheckProfileVersion)
    {
    struct ExpectedResult
        {
        DbResult m_statInReadWriteMode;
        DbResult m_statInReadOnlyMode;
        bool m_fileIsAutoUpgradable;

        ExpectedResult () 
            : m_statInReadWriteMode (BE_SQLITE_ERROR_ProfileUpgradeFailedCannotOpenForWrite), m_statInReadOnlyMode (BE_SQLITE_ERROR_ProfileUpgradeFailed), m_fileIsAutoUpgradable (false) 
            {}

        ExpectedResult (DbResult stat, bool fileIsAutoUpgradable) 
            : m_statInReadWriteMode (stat), m_statInReadOnlyMode (stat), m_fileIsAutoUpgradable (fileIsAutoUpgradable) 
            {}

        ExpectedResult (DbResult statInReadWriteMode, DbResult statInReadOnlyMode, bool fileIsAutoUpgradable) 
            : m_statInReadWriteMode (statInReadWriteMode), m_statInReadOnlyMode (statInReadOnlyMode), m_fileIsAutoUpgradable (fileIsAutoUpgradable) 
            {}
        };

    SchemaVersion expectedProfileVersion (2, 4, 5, 3);
    SchemaVersion minimumAutoUpgradeProfileVersion (1, 9, 0, 0);

    std::map<SchemaVersion, ExpectedResult> testDataset;
    testDataset[SchemaVersion(0,0,0,0)] = ExpectedResult (BE_SQLITE_ERROR_ProfileTooOld, false);
    testDataset[SchemaVersion(1,0,0,0)] = ExpectedResult (BE_SQLITE_ERROR_ProfileTooOld, false);
    testDataset[SchemaVersion(1,8,99,0)] = ExpectedResult (BE_SQLITE_ERROR_ProfileTooOld, false);
    testDataset[SchemaVersion(1,9,0,0)] = ExpectedResult (BE_SQLITE_ERROR_ProfileTooOld, true);
    testDataset[SchemaVersion(1,9,2,3)] = ExpectedResult (BE_SQLITE_ERROR_ProfileTooOld, true);

    testDataset[SchemaVersion(1,9,2,4)] = ExpectedResult (BE_SQLITE_ERROR_ProfileTooOld, true);
    testDataset[SchemaVersion(1,9,2,5)] = ExpectedResult (BE_SQLITE_ERROR_ProfileTooOld, true);
    testDataset[SchemaVersion(1,9,3,0)] = ExpectedResult (BE_SQLITE_ERROR_ProfileTooOld, true);
    testDataset[SchemaVersion(1,9,9,0)] = ExpectedResult (BE_SQLITE_ERROR_ProfileTooOld, true);
    testDataset[SchemaVersion(2,0,0,0)] = ExpectedResult (BE_SQLITE_ERROR_ProfileTooOld, true);
    testDataset[SchemaVersion(2,1,0,0)] = ExpectedResult (BE_SQLITE_ERROR_ProfileTooOld, true);
    testDataset[SchemaVersion(2,4,0,0)] = ExpectedResult (BE_SQLITE_OK, true);
    testDataset[SchemaVersion(2,4,5,0)] = ExpectedResult (BE_SQLITE_OK, true);
    testDataset[SchemaVersion(2,4,5,2)] = ExpectedResult (BE_SQLITE_OK, true);

    testDataset[SchemaVersion(2,4,5,3)] = ExpectedResult (BE_SQLITE_OK, false);

    testDataset[SchemaVersion(2,4,5,4)] = ExpectedResult (BE_SQLITE_OK, false);
    testDataset[SchemaVersion(2,4,5,33)] = ExpectedResult (BE_SQLITE_OK, false);

    testDataset[SchemaVersion(2,4,6,0)] = ExpectedResult (BE_SQLITE_ERROR_ProfileTooNewForReadWrite, BE_SQLITE_OK, false);
    testDataset[SchemaVersion(2,4,6,99)] = ExpectedResult (BE_SQLITE_ERROR_ProfileTooNewForReadWrite, BE_SQLITE_OK, false);
    testDataset[SchemaVersion(2,4,99,0)] = ExpectedResult (BE_SQLITE_ERROR_ProfileTooNewForReadWrite, BE_SQLITE_OK, false);
    testDataset[SchemaVersion(2,4,99,99)] = ExpectedResult (BE_SQLITE_ERROR_ProfileTooNewForReadWrite, BE_SQLITE_OK, false);

    testDataset[SchemaVersion(2,5,0,0)] = ExpectedResult (BE_SQLITE_ERROR_ProfileTooNew, false);
    testDataset[SchemaVersion(2,5,0,1)] = ExpectedResult (BE_SQLITE_ERROR_ProfileTooNew, false);
    testDataset[SchemaVersion(2,99,0,1)] = ExpectedResult (BE_SQLITE_ERROR_ProfileTooNew, false);
    testDataset[SchemaVersion(3,0,0,0)] = ExpectedResult (BE_SQLITE_ERROR_ProfileTooNew, false);
    testDataset[SchemaVersion(99,99,99,99)] = ExpectedResult (BE_SQLITE_ERROR_ProfileTooNew, false);

    for (auto const& testItem : testDataset)
        {
        SchemaVersion const& actualProfileVersion = testItem.first;
        ExpectedResult const& expectedResult = testItem.second;

        bool actualfileIsAutoUpgradable = false;
        DbResult actualStat = Db::CheckProfileVersion(actualfileIsAutoUpgradable, expectedProfileVersion, actualProfileVersion,
                                                         minimumAutoUpgradeProfileVersion, true, "Test");

        EXPECT_EQ (expectedResult.m_fileIsAutoUpgradable, actualfileIsAutoUpgradable) << "OpenMode: read-only - Expected version: " << expectedProfileVersion.ToJson().c_str () << " - Actual version: " << actualProfileVersion.ToJson().c_str ();
        EXPECT_EQ (expectedResult.m_statInReadOnlyMode, actualStat) << "OpenMode: read-only - Expected version: " << expectedProfileVersion.ToJson().c_str () << " - Actual version: " << actualProfileVersion.ToJson().c_str ();

        //now re-run check with read-write open mode
        actualfileIsAutoUpgradable = false;
        actualStat = Db::CheckProfileVersion(actualfileIsAutoUpgradable, expectedProfileVersion, actualProfileVersion,
            minimumAutoUpgradeProfileVersion, false, "Test");

        EXPECT_EQ (expectedResult.m_fileIsAutoUpgradable, actualfileIsAutoUpgradable) << "OpenMode: read-write - Expected version: " << expectedProfileVersion.ToJson().c_str () << " - Actual version: " << actualProfileVersion.ToJson().c_str ();
        EXPECT_EQ (expectedResult.m_statInReadWriteMode, actualStat) << "OpenMode: read-write - Expected version: " << expectedProfileVersion.ToJson().c_str () << " - Actual version: " << actualProfileVersion.ToJson().c_str ();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  10/14
//+---------------+---------------+---------------+---------------+---------------+------
void AssertProfile (Db& besqliteDb, bool embedFileTableExists, bool lastModifiedColumnExists, Utf8CP assertMessage)
    {
    ASSERT_EQ (embedFileTableExists, besqliteDb.TableExists ("be_EmbedFile")) << assertMessage;
    ASSERT_EQ (lastModifiedColumnExists, besqliteDb.ColumnExists ("be_EmbedFile", "LastModified")) << assertMessage;

    Utf8String actualVersionString;
    auto stat = besqliteDb.QueryProperty (actualVersionString, Properties::SchemaVersion ());
    ASSERT_EQ (BE_SQLITE_ROW, stat);
    SchemaVersion actualVersion (0, 0, 0, 0);
    actualVersion.FromJson (actualVersionString.c_str ());

    if (embedFileTableExists && lastModifiedColumnExists)
        {
        ASSERT_EQ (BEDB_CURRENT_VERSION_Major, actualVersion.GetMajor ()) << assertMessage;
        ASSERT_EQ (BEDB_CURRENT_VERSION_Minor, actualVersion.GetMinor ()) << assertMessage;
        ASSERT_EQ (BEDB_CURRENT_VERSION_Sub1, actualVersion.GetSub1 ()) << assertMessage;
        ASSERT_EQ (BEDB_CURRENT_VERSION_Sub2, actualVersion.GetSub2 ()) << assertMessage;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String CopyTestFile (WCharCP fileName)
    {
    BeFileName sourceDir;
    BeTest::GetHost ().GetDocumentsRoot (sourceDir);
    sourceDir.AppendToPath (L"DgnDb");

    BeFileName targetDir;
    BeTest::GetHost ().GetOutputRoot (targetDir);

    BeFileName sourcePath (nullptr, sourceDir.GetName (), fileName, nullptr);
    BeFileName targetPath (nullptr, targetDir.GetName (), fileName, nullptr);

    if (targetPath.DoesPathExist ())
        targetPath.BeDeleteFile ();

    if (BeFileNameStatus::Success != BeFileName::BeCopyFile (sourcePath, targetPath, false))
        return nullptr;

    return Utf8String (targetPath.GetNameUtf8 ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  10/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (BeSQLiteDb, ProfileTest)
    {
        {
        Db db;
        auto stat = SetupDb (db, L"besqliteprofile_uptodate.db");
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Creation of test BeSQLite DB failed.";
        AssertProfile (db, true, true, "Test scenario: BeSQLite file created from scratch.");
        }

        {
        Utf8String testFilePath = CopyTestFile (L"besqliteprofile3_1_0_0_noembedfiletable.db");
        Db db;
        auto stat = db.OpenBeSQLiteDb (testFilePath.c_str (), Db::OpenParams (Db::OpenMode::Readonly));
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Opening test file " << testFilePath.c_str () << " failed.";
        AssertProfile (db, false, false, "Test scenario: Open old BeSQLite file without embedded file table.");
        }

        {
        Utf8String testFilePath = CopyTestFile (L"besqliteprofile3_1_0_0_noembedfiletable.db");
        Db db;
        auto stat = db.OpenBeSQLiteDb (testFilePath.c_str (), Db::OpenParams (Db::OpenMode::ReadWrite));

        ASSERT_EQ (BE_SQLITE_OK, stat) << "Opening test file " << testFilePath.c_str () << " failed.";
        stat = db.UpgradeBeSQLiteProfile ();
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Upgrading test file " << testFilePath.c_str () << " failed.";

        AssertProfile (db, true, true, "Test scenario: Upgrade old BeSQLite file without embedded file table.");
        }

        {
        Utf8String testFilePath = CopyTestFile (L"besqliteprofile3_1_0_0_noembedfiletable.db");
        Db db;
        auto stat = db.OpenBeSQLiteDb (testFilePath.c_str (), Db::OpenParams (Db::OpenMode::ReadWrite));
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Opening test file " << testFilePath.c_str () << " failed.";

        BeFileName testFile;
        BeTest::GetHost ().GetDocumentsRoot (testFile);
        testFile.AppendToPath (L"DgnDb").AppendToPath (L"StartupCompany.json");

        stat = db.EmbeddedFiles ().Import ("test", testFile.GetNameUtf8 ().c_str (), ".json");
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Embedding test file failed.";

        AssertProfile (db, true, true, "Test scenario: Embedding file in old BeSQLite file without embedded file table.");
        }

        {
        Utf8String testFilePath = CopyTestFile (L"besqliteprofile3_1_0_0_withembedfiletable.db");
        Db db;
        auto stat = db.OpenBeSQLiteDb (testFilePath.c_str (), Db::OpenParams (Db::OpenMode::Readonly));
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Opening test file " << testFilePath.c_str () << " failed.";
        AssertProfile (db, true, false, "Test scenario: Old BeSQLite file with embedded file table.");
        }

        {
        Utf8String testFilePath = CopyTestFile (L"besqliteprofile3_1_0_0_withembedfiletable.db");
        Db db;
        auto stat = db.OpenBeSQLiteDb (testFilePath.c_str (), Db::OpenParams (Db::OpenMode::ReadWrite));
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Opening test file " << testFilePath.c_str () << " failed.";
        stat = db.UpgradeBeSQLiteProfile ();
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Upgrading test file " << testFilePath.c_str () << " failed.";

        AssertProfile (db, true, true, "Test scenario: Upgrade old BeSQLite file with embedded file table.");
        }

        {
        Utf8String testFilePath = CopyTestFile (L"besqliteprofile3_1_0_0_withembedfiletable.db");
        Db db;
        auto stat = db.OpenBeSQLiteDb (testFilePath.c_str (), Db::OpenParams (Db::OpenMode::Readonly));
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Opening test file " << testFilePath.c_str () << " failed.";
        stat = db.UpgradeBeSQLiteProfile ();
        ASSERT_EQ (BE_SQLITE_ERROR_ProfileUpgradeFailed, stat) << "Upgrading test file " << testFilePath.c_str () << " is expected to fail if file is opened readonly.";
        }

        {
        Utf8String testFilePath = CopyTestFile (L"besqliteprofile3_1_0_0_withembedfiletable.db");
        Db db;
        auto stat = db.OpenBeSQLiteDb (testFilePath.c_str (), Db::OpenParams (Db::OpenMode::ReadWrite));
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Opening test file " << testFilePath.c_str () << " failed.";

        BeFileName testFile;
        BeTest::GetHost ().GetDocumentsRoot (testFile);
        testFile.AppendToPath (L"DgnDb").AppendToPath (L"StartupCompany.json");

        stat = db.EmbeddedFiles ().Import ("test", testFile.GetNameUtf8 ().c_str (), ".json");
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Embedding test file failed.";

        AssertProfile (db, true, true, "Test scenario: Embedding file in old BeSQLite file with embedded file table.");
        }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST (BeSQLiteDb, ChangeRepositoryIdInReadonlyMode)
    {
    Utf8String dbPath;

    //prepare test dgn db
        {
        Db db;
        auto stat = SetupDb (db, L"changerepoid.idgndb");
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Creation of test BeSQLite DB failed.";
        dbPath.assign (db.GetDbFileName ());
        db.CloseDb ();
        }

    Db db;
    DbResult stat = db.OpenBeSQLiteDb (dbPath.c_str (), Db::OpenParams (Db::OpenMode::Readonly));
    ASSERT_EQ (BE_SQLITE_OK, stat) << L"Reopening test DgnDb '" << dbPath.c_str () << L"' failed.";

    BeTest::SetFailOnAssert (false);
    stat = db.ChangeRepositoryId (BeRepositoryId (12345));
    BeTest::SetFailOnAssert (true);
    ASSERT_EQ (BE_SQLITE_READONLY, stat) << L"Calling ChangeRepositoryId on readonly DgnDb file is expected to fail.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST (BeSQLiteDb, ChangeRepositoryId)
    {
    Utf8String dbPath;

    //prepare test dgn db
        {
        Db db;
        auto stat = SetupDb (db, L"changerepoid.idgndb");
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Creation of test BeSQLite DB failed.";
        dbPath.assign (db.GetDbFileName ());

        std::vector<int> localValues = { 1234, 111, -111, 0 };
        std::vector<Utf8String> localValueNames = { "key0", "key1", "key2", "key3" };
        const size_t valueCount = localValues.size ();
        for (size_t i = 0; i < valueCount; i++)
            {
            int val = localValues[i];
            size_t keyIndex = 0;
            ASSERT_EQ (BE_SQLITE_OK, db.GetRLVCache().Register(keyIndex, localValueNames[i].c_str ())) << "Registration of RLV " << localValueNames[i].c_str () << " is expected to succeed.";
            auto result = db.GetRLVCache().SaveValue (keyIndex, val);
            ASSERT_EQ (BE_SQLITE_OK, result) << L"Saving test RLV '" << localValueNames[i].c_str () << L"=" << val << L"' failed";
            }

        ASSERT_EQ (BE_SQLITE_OK, db.SaveChanges ()) << L"Committing repository local values failed.";
        db.CloseDb ();
        }

    //reopen DgnDb again, change repo id and close again (to avoid that caches linger around)
    BeRepositoryId expectedRepoId;
    expectedRepoId.Invalidate ();

        {
        Db db;
        DbResult stat = db.OpenBeSQLiteDb (dbPath.c_str (), Db::OpenParams (Db::OpenMode::ReadWrite));
        ASSERT_EQ (BE_SQLITE_OK, stat) << L"Reopening test DgnDb '" << dbPath.c_str () << L"' failed.";

        //now change repository id. This should truncate be_local and reinsert the new repo id
        const BeRepositoryId currentRepoId = db.GetRepositoryId ();
        expectedRepoId = currentRepoId.GetNextRepositoryId ();
        stat = db.ChangeRepositoryId (expectedRepoId);
        ASSERT_EQ (BE_SQLITE_OK, stat) << L"Changing the repository id is not expected to fail.";
        }

    //now reopen from scratch
    Db db;
    DbResult stat = db.OpenBeSQLiteDb (dbPath.c_str (), Db::OpenParams (Db::OpenMode::Readonly));
    ASSERT_EQ (BE_SQLITE_OK, stat) << L"Reopening test DgnDb '" << dbPath.c_str () << L"' failed.";

    Utf8CP const repoIdKey = "be_repositoryId";
    size_t repoIdIndex = 0;
    ASSERT_TRUE (db.GetRLVCache().TryGetIndex(repoIdIndex, repoIdKey));
    //query be_local to check that there is only one row (the repo id)
    Statement statement;
    ASSERT_EQ (BE_SQLITE_OK, statement.Prepare (db, "SELECT Name, Val from " BEDB_TABLE_Local)) << L"Preparing SQL statement to retrieve content of be_local failed";

    int rowCount = 0;
    while (statement.Step () == BE_SQLITE_ROW)
        {
        rowCount++;
        Utf8CP name = statement.GetValueText (0);
        EXPECT_STREQ (repoIdKey, name) << L"be_local after a repository id change should only contain the repository id.";
        //don't mimick the blob deserialization here. Just test that the column is not null. Use the API to check the actual repo id later
        EXPECT_FALSE (statement.IsColumnNull (1)) << "Val column of repository id row in be_local after a repository id change must not be null.";
        }

    EXPECT_EQ (1, rowCount) << L"be_local after a repository id change should only contain one row (the repo id).";

    //check value of repo id through API. As we opened file from scratch the old id cannot be in the cache anymore.
    int64_t actualRepoId;
    db.GetRLVCache().QueryValue (actualRepoId, repoIdIndex);
    EXPECT_EQ (expectedRepoId.GetValue (), (int32_t) actualRepoId) << L"QueryRepositoryLocalValue did not return the right value for repository id after repository id had been changed.";
    }



void RunDeleteCascadingTest (int primaryRowCount, int secondaryRowCountPerPrimaryRow, int ternaryRowCountPerPrimaryRow, bool withIndexOnSecondaryTable);
Utf8String PopulateDeleteCascadingTestDb (WCharCP dbFileName, bool withIndex, bool withTrigger, int primaryRowCount, int secondaryRowCountPerPrimaryRow, int ternaryRowCountPerPrimaryRow);
void GenerateDeleteCascadingTestSqlLists (std::vector<Utf8String>& primarySqlList, std::vector<Utf8String>& secondarySqlList, std::vector<Utf8String>& ternarySqlList);

//---------------------------------------------------------------------------------------
//! Tests performance of a cascading delete without using foreign keys. Instead
//! SQLite triggers are compared to executing the cascading statements manually.
//! This particular test is run with indices on the column that acts as foreign key to the parent table
// @bsimethod                                   Krischan.Eberle                     01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (PerformanceSqlite, DeleteCascadingWithIndex)
    {
    const int primaryRowCount = 100;
    const int secondaryRowCountPerPrimaryRow = 1000;
    const int ternaryRowCountPerPrimaryRow = 100;
    RunDeleteCascadingTest (primaryRowCount, secondaryRowCountPerPrimaryRow, ternaryRowCountPerPrimaryRow, true);
    }

//---------------------------------------------------------------------------------------
//! Tests performance of a cascading delete without using foreign keys. Instead
//! SQLite triggers are compared to executing the cascading statements manually
//! This particular test is run @p without indices on the column that acts as foreign key to the parent table
// @bsimethod                                   Krischan.Eberle                     01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (PerformanceSqlite, DeleteCascadingWithoutIndex)
    {
    const int primaryRowCount = 10;
    const int secondaryRowCountPerPrimaryRow = 100;
    const int ternaryRowCountPerPrimaryRow = 100;
    RunDeleteCascadingTest (primaryRowCount, secondaryRowCountPerPrimaryRow, ternaryRowCountPerPrimaryRow, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     01/14
//+---------------+---------------+---------------+---------------+---------------+------
void RunDeleteCascadingTest (int primaryRowCount, int secondaryRowCountPerPrimaryRow, int ternaryRowCountPerPrimaryRow, bool withIndexOnSecondaryTable)
    {
    Utf8String dbWithTriggerPath = PopulateDeleteCascadingTestDb (L"deletecascadingwithtrigger.db", withIndexOnSecondaryTable, true, primaryRowCount, secondaryRowCountPerPrimaryRow, ternaryRowCountPerPrimaryRow);
    ASSERT_FALSE (dbWithTriggerPath.empty ()) << "Populating test db failed";
    Utf8String dbWithoutTriggerPath = PopulateDeleteCascadingTestDb (L"deletecascadingwithouttrigger.db", withIndexOnSecondaryTable, false, primaryRowCount, secondaryRowCountPerPrimaryRow, ternaryRowCountPerPrimaryRow);
    ASSERT_FALSE (dbWithoutTriggerPath.empty ()) << "Populating test db failed";

    Db dbWithTrigger;
    auto stat = dbWithTrigger.OpenBeSQLiteDb (dbWithTriggerPath.c_str (), Db::OpenParams (Db::OpenMode::ReadWrite));
    ASSERT_EQ (BE_SQLITE_OK, stat);

    Db dbWithoutTrigger;
    stat = dbWithoutTrigger.OpenBeSQLiteDb (dbWithoutTriggerPath.c_str (), Db::OpenParams (Db::OpenMode::ReadWrite));
    ASSERT_EQ (BE_SQLITE_OK, stat);

    std::vector<Utf8String> primarySqlList;
    std::vector<Utf8String> secondarySqlList;
    std::vector<Utf8String> ternarySqlList;
    GenerateDeleteCascadingTestSqlLists (primarySqlList, secondarySqlList, ternarySqlList);

    LOG.tracev ("Delete cascading test. Primary table row count: %d. Secondary rows per primary row: %d Ternary rows per secondary row: %d",
        primaryRowCount, secondaryRowCountPerPrimaryRow, ternaryRowCountPerPrimaryRow);
    for (size_t i = 0; i < primarySqlList.size (); i++)
        {
        Utf8CP primarySql = primarySqlList[i].c_str ();
        Utf8CP secondarySql = secondarySqlList[i].c_str ();
        Utf8CP ternarySql = ternarySqlList[i].c_str ();
        LOG.tracev ("Executing %s [Secondary: %s Ternary: %s]", primarySql, secondarySql, ternarySql);
            {
            Savepoint savepoint (dbWithTrigger, "");
            Statement stmt;
            ASSERT_EQ (BE_SQLITE_OK, stmt.Prepare (dbWithTrigger, primarySql)) << "Preparation of " << primarySql << " failed: " << dbWithTrigger.GetLastError ();
            StopWatch timer (true);
            auto stat = stmt.Step ();
            timer.Stop ();
            ASSERT_EQ (BE_SQLITE_DONE, stat) << "Executing test delete failed";
            int rowsAffected = dbWithTrigger.GetModifiedRowCount ();
            savepoint.Cancel ();

            LOG.tracev ("With trigger: %.4f msec - deleted %d primary rows, %d secondary rows, %d ternary rows",
                timer.GetElapsedSeconds () * 1000,
                rowsAffected, rowsAffected * secondaryRowCountPerPrimaryRow,
                rowsAffected * secondaryRowCountPerPrimaryRow * ternaryRowCountPerPrimaryRow);
            }

            {
            Savepoint savepoint (dbWithoutTrigger, "");

            //execute ternary statement first
            Statement ternaryStmt;
            ASSERT_EQ (BE_SQLITE_OK, ternaryStmt.Prepare (dbWithoutTrigger, ternarySql)) << "Preparation of " << ternarySql << " failed: " << dbWithoutTrigger.GetLastError ();
            StopWatch ternaryTimer (true);
            stat = ternaryStmt.Step ();
            ternaryTimer.Stop ();
            ASSERT_EQ (BE_SQLITE_DONE, stat) << "Executing test delete failed";
            int ternaryRowsAffected = dbWithoutTrigger.GetModifiedRowCount ();


            //execute secondary statement first
            Statement secondaryStmt;
            ASSERT_EQ (BE_SQLITE_OK, secondaryStmt.Prepare (dbWithoutTrigger, secondarySql)) << "Preparation of " << secondarySql << " failed: " << dbWithoutTrigger.GetLastError ();
            StopWatch secondaryTimer (true);
            stat = secondaryStmt.Step ();
            secondaryTimer.Stop ();
            ASSERT_EQ (BE_SQLITE_DONE, stat) << "Executing test delete failed";
            int secondaryRowsAffected = dbWithoutTrigger.GetModifiedRowCount ();

            Statement primaryStmt;
            ASSERT_EQ (BE_SQLITE_OK, primaryStmt.Prepare (dbWithoutTrigger, primarySql)) << "Preparation of " << primarySql << " failed: " << dbWithoutTrigger.GetLastError ();
            StopWatch primaryTimer (true);
            auto stat = primaryStmt.Step ();
            primaryTimer.Stop ();
            ASSERT_EQ (BE_SQLITE_DONE, stat) << "Executing test delete failed";
            int rowsAffected = dbWithoutTrigger.GetModifiedRowCount ();

            ASSERT_EQ (rowsAffected * secondaryRowCountPerPrimaryRow, secondaryRowsAffected) << primarySql << " - " << secondarySql << " - " << ternarySql;
            ASSERT_EQ (rowsAffected * secondaryRowCountPerPrimaryRow * ternaryRowCountPerPrimaryRow, ternaryRowsAffected) << primarySql << " - " << secondarySql << " - " << ternarySql;
            savepoint.Cancel ();

            double primaryTiming = primaryTimer.GetElapsedSeconds () * 1000;
            double secondaryTiming = secondaryTimer.GetElapsedSeconds () * 1000;
            double ternaryTiming = ternaryTimer.GetElapsedSeconds () * 1000;
            LOG.tracev ("Without trigger: %.4f msec (Primary delete %.4f msec, secondary delete %.4f msec, ternary delete %.4f) - deleted %d primary rows, %d secondary rows, %d ternary rows",
            primaryTiming + secondaryTiming + ternaryTiming, primaryTiming, secondaryTiming, ternaryTiming,
            rowsAffected, secondaryRowsAffected, ternaryRowsAffected);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     01/14
//+---------------+---------------+---------------+---------------+---------------+------
void GenerateDeleteCascadingTestSqlLists (std::vector<Utf8String>& primarySqlList, std::vector<Utf8String>& secondarySqlList, std::vector<Utf8String>& ternarySqlList)
    {
    primarySqlList.push_back ("DELETE FROM PrimaryTable WHERE Id = 5");
    secondarySqlList.push_back ("DELETE FROM SecondaryTable WHERE PrimaryId IN (SELECT Id FROM PrimaryTable WHERE Id = 5)");
    ternarySqlList.push_back ("DELETE FROM TernaryTable WHERE SecondaryId IN "
                              "(SELECT Id FROM SecondaryTable WHERE PrimaryId IN "
                              "(SELECT Id FROM PrimaryTable WHERE Id = 5)"
                              ") AND "
                              "PrimaryId IN (SELECT Id FROM PrimaryTable WHERE Id = 5)");

    //this is an optimum counterpart to the previous which however is hard to come up with in a generic fashion.
    primarySqlList.push_back ("DELETE FROM PrimaryTable WHERE Id = 5");
    secondarySqlList.push_back ("DELETE FROM SecondaryTable WHERE PrimaryId = 5");
    ternarySqlList.push_back ("DELETE FROM TernaryTable WHERE PrimaryId = 5");



    primarySqlList.push_back ("DELETE FROM PrimaryTable WHERE Id IN (2, 8)");
    secondarySqlList.push_back ("DELETE FROM SecondaryTable WHERE PrimaryId IN (SELECT Id FROM PrimaryTable WHERE Id IN (2, 8))");
    ternarySqlList.push_back ("DELETE FROM TernaryTable WHERE SecondaryId IN (SELECT Id FROM SecondaryTable WHERE PrimaryId IN (SELECT Id FROM PrimaryTable WHERE Id IN (2, 8)))"
                              " AND PrimaryId IN (SELECT Id FROM PrimaryTable WHERE Id IN (2, 8))");

    //this is an optimum counterpart to the previous which however is hard to come up with in a generic fashion.
    primarySqlList.push_back ("DELETE FROM PrimaryTable WHERE Id IN (2, 8)");
    secondarySqlList.push_back ("DELETE FROM SecondaryTable WHERE PrimaryId IN (2, 8)");
    ternarySqlList.push_back ("DELETE FROM TernaryTable WHERE PrimaryId IN (2, 8)");




    primarySqlList.push_back ("DELETE FROM PrimaryTable WHERE Id % 2 = 0");
    secondarySqlList.push_back ("DELETE FROM SecondaryTable WHERE PrimaryId IN (SELECT Id FROM PrimaryTable WHERE Id % 2 = 0)");
    ternarySqlList.push_back ("DELETE FROM TernaryTable WHERE SecondaryId IN (SELECT Id FROM SecondaryTable WHERE PrimaryId IN (SELECT Id FROM PrimaryTable WHERE Id % 2 = 0))"
                              " AND PrimaryId IN (SELECT Id FROM PrimaryTable WHERE Id % 2 = 0)");

    //this is an optimum counterpart to the previous which however is hard to come up with in a generic fashion.
    primarySqlList.push_back ("DELETE FROM PrimaryTable WHERE Id % 2 = 0");
    secondarySqlList.push_back ("DELETE FROM SecondaryTable WHERE PrimaryId % 2 = 0");
    ternarySqlList.push_back ("DELETE FROM TernaryTable WHERE PrimaryId % 2 = 0");





    primarySqlList.push_back ("DELETE FROM PrimaryTable WHERE I IN (-2, -8)");
    secondarySqlList.push_back ("DELETE FROM SecondaryTable WHERE PrimaryId IN (SELECT Id FROM PrimaryTable WHERE I IN (-2, -8))");
    ternarySqlList.push_back ("DELETE FROM TernaryTable WHERE SecondaryId IN (SELECT Id FROM SecondaryTable WHERE PrimaryId IN (SELECT Id FROM PrimaryTable WHERE I IN (-2, -8)))"
                              " AND PrimaryId IN (SELECT Id FROM PrimaryTable WHERE I IN (-2, -8))");


    primarySqlList.push_back ("DELETE FROM PrimaryTable WHERE (-1 * I) % 2 = 0");
    secondarySqlList.push_back ("DELETE FROM SecondaryTable WHERE PrimaryId IN (SELECT Id FROM PrimaryTable WHERE (-1 * I) % 2 = 0)");
    ternarySqlList.push_back ("DELETE FROM TernaryTable WHERE SecondaryId IN (SELECT Id FROM SecondaryTable WHERE PrimaryId IN (SELECT Id FROM PrimaryTable WHERE (-1 * I) % 2 = 0))"
                              " AND PrimaryId IN (SELECT Id FROM PrimaryTable WHERE (-1 * I) % 2 = 0)");


    primarySqlList.push_back ("DELETE FROM PrimaryTable");
    secondarySqlList.push_back ("DELETE FROM SecondaryTable WHERE PrimaryId IN (SELECT Id FROM PrimaryTable)");
    ternarySqlList.push_back ("DELETE FROM TernaryTable WHERE SecondaryId IN (SELECT Id FROM SecondaryTable WHERE PrimaryId IN (SELECT Id FROM PrimaryTable))"
                               " AND PrimaryId IN (SELECT Id FROM PrimaryTable)");

    //this is an optimum counterpart to the previous which however is hard to come up with in a generic fashion.
    primarySqlList.push_back ("DELETE FROM PrimaryTable");
    secondarySqlList.push_back ("DELETE FROM SecondaryTable");
    ternarySqlList.push_back ("DELETE FROM TernaryTable");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     01/14
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String PopulateDeleteCascadingTestDb (WCharCP dbFileName, bool withIndex, bool withTrigger, int primaryRowCount, int secondaryRowsPerPrimaryRow, int ternaryRowsPerSecondaryRow)
    {
    Db db;
    SetupDb (db, dbFileName);

    Utf8CP createTableSqlTemplate = "CREATE TABLE PrimaryTable (Id INTEGER PRIMARY KEY, D DOUBLE, Name TEXT, I INT);"
        "CREATE TABLE SecondaryTable (PrimaryId INTEGER, Id INTEGER, Name TEXT, L INT64 %s);"
        "CREATE TABLE TernaryTable (PrimaryId INTEGER, SecondaryId INTEGER, Id INTEGER, Name TEXT, D DOUBLE %s);";

    Utf8String createTableSql;
    if (withIndex)
        createTableSql.Sprintf (createTableSqlTemplate, ", PRIMARY KEY (PrimaryId, Id)", ", PRIMARY KEY (PrimaryId, SecondaryId, Id)");
    else
        createTableSql.Sprintf (createTableSqlTemplate, "", "");

    auto stat = db.ExecuteSql (createTableSql.c_str ());
    if (stat != BE_SQLITE_OK)
        {
        LOG.error (db.GetLastError ());
        return "";
        }

    if (withTrigger)
        {
        stat = db.ExecuteSql ("CREATE TRIGGER DELETE_SECONDARY_ROWS AFTER DELETE ON PrimaryTable "
                                "BEGIN "
                                "DELETE FROM SecondaryTable WHERE PrimaryId = old.Id; "
                                "END;"
                                "CREATE TRIGGER DELETE_TERNARY_ROWS AFTER DELETE ON SecondaryTable "
                                "BEGIN "
                                "DELETE FROM TernaryTable WHERE PrimaryId = old.PrimaryId AND SecondaryId = old.Id;"
                                "END;"
                                );
        if (stat != BE_SQLITE_OK)
            {
            LOG.error (db.GetLastError ());
            return "";
            }
        }


    Statement primaryStmt;
    stat = primaryStmt.Prepare (db, "INSERT INTO PrimaryTable (Id, D, Name, I) VALUES (?,?,?,?)");
    if (stat != BE_SQLITE_OK)
        {
        LOG.error (db.GetLastError ());
        return "";
        }

    for (int i = 0; i < primaryRowCount; i++)
        {
        primaryStmt.Reset ();
        primaryStmt.ClearBindings ();
        primaryStmt.BindInt (1, i);
        primaryStmt.BindDouble (2, i * 3.1415);
        primaryStmt.BindText (3, "Some text", Statement::MakeCopy::Yes);
        primaryStmt.BindInt (4, -i);

        stat = primaryStmt.Step ();
        if (stat != BE_SQLITE_DONE)
            {
            LOG.error (db.GetLastError ());
            return "";
            }
        }

    Statement secondaryStmt;
    stat = secondaryStmt.Prepare (db, "INSERT INTO SecondaryTable (PrimaryId, Id, Name, L) VALUES (?,?,?,?)");
    if (stat != BE_SQLITE_OK)
        {
        LOG.error (db.GetLastError ());
        return "";
        }

    for (int i = 0; i < primaryRowCount; i++)
        {
        for (int j = 0; j < secondaryRowsPerPrimaryRow; j++)
            {
            secondaryStmt.Reset ();
            secondaryStmt.ClearBindings ();
            secondaryStmt.BindInt (1, i);
            secondaryStmt.BindInt (2, j);
            secondaryStmt.BindText (3, "Some text", Statement::MakeCopy::Yes);
            secondaryStmt.BindInt (4, i*j);

            stat = secondaryStmt.Step ();
            if (stat != BE_SQLITE_DONE)
                {
                LOG.error (db.GetLastError ());
                return "";
                }
            }
        }

    Statement ternaryStmt;
    stat = ternaryStmt.Prepare (db, "INSERT INTO TernaryTable (PrimaryId, SecondaryId, Id, Name, D) VALUES (?,?,?,?,?)");
    if (stat != BE_SQLITE_OK)
        {
        LOG.error (db.GetLastError ());
        return "";
        }

    for (int i = 0; i < primaryRowCount; i++)
        {
        for (int j = 0; j < secondaryRowsPerPrimaryRow; j++)
            {
            for (int k = 0; k < ternaryRowsPerSecondaryRow; k++)
                {
                ternaryStmt.Reset ();
                ternaryStmt.ClearBindings ();
                ternaryStmt.BindInt (1, i);
                ternaryStmt.BindInt (2, j);
                ternaryStmt.BindInt (3, k);
                ternaryStmt.BindText (4, "Some text", Statement::MakeCopy::Yes);
                ternaryStmt.BindDouble (5, i*j*k*3.14);

                stat = ternaryStmt.Step ();
                if (stat != BE_SQLITE_DONE)
                    {
                    LOG.error (db.GetLastError ());
                    return "";
                    }
                }
            }
        }

    db.SaveChanges ();
    return db.GetDbFileName ();
    }
