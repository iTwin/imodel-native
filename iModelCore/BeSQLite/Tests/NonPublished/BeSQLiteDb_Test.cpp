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
DbResult SetupDb(Db& db, WCharCP dbName)
    {
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    BeSQLiteLib::Initialize(tempDir);

    BeFileName dbFileName;
    BeTest::GetHost().GetOutputRoot(dbFileName);
    dbFileName.AppendToPath(dbName);

    if (BeFileName::DoesPathExist(dbFileName))
        BeFileName::BeDeleteFile(dbFileName);
    
    DbResult result = db.CreateNewDb(dbFileName.GetNameUtf8().c_str());
    EXPECT_EQ (BE_SQLITE_OK, result) << "Db Creation failed";
    if (result == BE_SQLITE_OK)
        db.SaveChanges();

    return result;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     11/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST(BeSQLiteDb, CheckProfileVersionWithEmptyProfileName)
    {
    SchemaVersion dummy(2, 4, 5, 3);

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

        ExpectedResult() 
            : m_statInReadWriteMode(BE_SQLITE_ERROR_ProfileUpgradeFailedCannotOpenForWrite), m_statInReadOnlyMode(BE_SQLITE_ERROR_ProfileUpgradeFailed), m_fileIsAutoUpgradable(false) 
            {}

        ExpectedResult(DbResult stat, bool fileIsAutoUpgradable) 
            : m_statInReadWriteMode(stat), m_statInReadOnlyMode(stat), m_fileIsAutoUpgradable(fileIsAutoUpgradable) 
            {}

        ExpectedResult(DbResult statInReadWriteMode, DbResult statInReadOnlyMode, bool fileIsAutoUpgradable) 
            : m_statInReadWriteMode(statInReadWriteMode), m_statInReadOnlyMode(statInReadOnlyMode), m_fileIsAutoUpgradable(fileIsAutoUpgradable) 
            {}
        };

    SchemaVersion expectedProfileVersion(2, 4, 5, 3);
    SchemaVersion minimumAutoUpgradeProfileVersion(1, 9, 0, 0);

    std::map<SchemaVersion, ExpectedResult> testDataset;
    testDataset[SchemaVersion(0,0,0,0)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooOld, false);
    testDataset[SchemaVersion(1,0,0,0)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooOld, false);
    testDataset[SchemaVersion(1,8,99,0)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooOld, false);
    testDataset[SchemaVersion(1,9,0,0)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooOld, true);
    testDataset[SchemaVersion(1,9,2,3)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooOld, true);

    testDataset[SchemaVersion(1,9,2,4)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooOld, true);
    testDataset[SchemaVersion(1,9,2,5)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooOld, true);
    testDataset[SchemaVersion(1,9,3,0)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooOld, true);
    testDataset[SchemaVersion(1,9,9,0)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooOld, true);
    testDataset[SchemaVersion(2,0,0,0)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooOld, true);
    testDataset[SchemaVersion(2,1,0,0)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooOld, true);
    testDataset[SchemaVersion(2,4,0,0)] = ExpectedResult(BE_SQLITE_OK, true);
    testDataset[SchemaVersion(2,4,5,0)] = ExpectedResult(BE_SQLITE_OK, true);
    testDataset[SchemaVersion(2,4,5,2)] = ExpectedResult(BE_SQLITE_OK, true);

    testDataset[SchemaVersion(2,4,5,3)] = ExpectedResult(BE_SQLITE_OK, false);

    testDataset[SchemaVersion(2,4,5,4)] = ExpectedResult(BE_SQLITE_OK, false);
    testDataset[SchemaVersion(2,4,5,33)] = ExpectedResult(BE_SQLITE_OK, false);

    testDataset[SchemaVersion(2,4,6,0)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooNewForReadWrite, BE_SQLITE_OK, false);
    testDataset[SchemaVersion(2,4,6,99)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooNewForReadWrite, BE_SQLITE_OK, false);
    testDataset[SchemaVersion(2,4,99,0)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooNewForReadWrite, BE_SQLITE_OK, false);
    testDataset[SchemaVersion(2,4,99,99)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooNewForReadWrite, BE_SQLITE_OK, false);

    testDataset[SchemaVersion(2,5,0,0)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooNew, false);
    testDataset[SchemaVersion(2,5,0,1)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooNew, false);
    testDataset[SchemaVersion(2,99,0,1)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooNew, false);
    testDataset[SchemaVersion(3,0,0,0)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooNew, false);
    testDataset[SchemaVersion(99,99,99,99)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooNew, false);

    for (auto const& testItem : testDataset)
        {
        SchemaVersion const& actualProfileVersion = testItem.first;
        ExpectedResult const& expectedResult = testItem.second;

        bool actualfileIsAutoUpgradable = false;
        DbResult actualStat = Db::CheckProfileVersion(actualfileIsAutoUpgradable, expectedProfileVersion, actualProfileVersion,
                                                         minimumAutoUpgradeProfileVersion, true, "Test");

        EXPECT_EQ (expectedResult.m_fileIsAutoUpgradable, actualfileIsAutoUpgradable) << "OpenMode: read-only - Expected version: " << expectedProfileVersion.ToJson().c_str() << " - Actual version: " << actualProfileVersion.ToJson().c_str();
        EXPECT_EQ (expectedResult.m_statInReadOnlyMode, actualStat) << "OpenMode: read-only - Expected version: " << expectedProfileVersion.ToJson().c_str() << " - Actual version: " << actualProfileVersion.ToJson().c_str();

        //now re-run check with read-write open mode
        actualfileIsAutoUpgradable = false;
        actualStat = Db::CheckProfileVersion(actualfileIsAutoUpgradable, expectedProfileVersion, actualProfileVersion,
            minimumAutoUpgradeProfileVersion, false, "Test");

        EXPECT_EQ (expectedResult.m_fileIsAutoUpgradable, actualfileIsAutoUpgradable) << "OpenMode: read-write - Expected version: " << expectedProfileVersion.ToJson().c_str() << " - Actual version: " << actualProfileVersion.ToJson().c_str();
        EXPECT_EQ (expectedResult.m_statInReadWriteMode, actualStat) << "OpenMode: read-write - Expected version: " << expectedProfileVersion.ToJson().c_str() << " - Actual version: " << actualProfileVersion.ToJson().c_str();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  10/14
//+---------------+---------------+---------------+---------------+---------------+------
void AssertProfile(Db& besqliteDb, bool embedFileTableExists, bool lastModifiedColumnExists, Utf8CP assertMessage)
    {
    ASSERT_EQ (embedFileTableExists, besqliteDb.TableExists("be_EmbedFile")) << assertMessage;
    ASSERT_EQ (lastModifiedColumnExists, besqliteDb.ColumnExists("be_EmbedFile", "LastModified")) << assertMessage;

    Utf8String actualVersionString;
    auto stat = besqliteDb.QueryProperty(actualVersionString, Properties::SchemaVersion());
    ASSERT_EQ (BE_SQLITE_ROW, stat);
    SchemaVersion actualVersion(0, 0, 0, 0);
    actualVersion.FromJson(actualVersionString.c_str());

    if (embedFileTableExists && lastModifiedColumnExists)
        {
        ASSERT_EQ (BEDB_CURRENT_VERSION_Major, actualVersion.GetMajor()) << assertMessage;
        ASSERT_EQ (BEDB_CURRENT_VERSION_Minor, actualVersion.GetMinor()) << assertMessage;
        ASSERT_EQ (BEDB_CURRENT_VERSION_Sub1, actualVersion.GetSub1()) << assertMessage;
        ASSERT_EQ (BEDB_CURRENT_VERSION_Sub2, actualVersion.GetSub2()) << assertMessage;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String CopyTestFile(WCharCP fileName)
    {
    BeFileName sourceDir;
    BeTest::GetHost().GetDocumentsRoot(sourceDir);
    sourceDir.AppendToPath(L"DgnDb");

    BeFileName targetDir;
    BeTest::GetHost().GetOutputRoot(targetDir);

    BeFileName sourcePath(nullptr, sourceDir.GetName(), fileName, nullptr);
    BeFileName targetPath(nullptr, targetDir.GetName(), fileName, nullptr);

    if (targetPath.DoesPathExist())
        targetPath.BeDeleteFile();

    if (BeFileNameStatus::Success != BeFileName::BeCopyFile(sourcePath, targetPath, false))
        return nullptr;

    return Utf8String(targetPath.GetNameUtf8());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  10/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (BeSQLiteDb, ProfileTest)
    {
        {
        Db db;
        auto stat = SetupDb(db, L"besqliteprofile_uptodate.db");
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Creation of test BeSQLite DB failed.";
        AssertProfile(db, true, true, "Test scenario: BeSQLite file created from scratch.");
        }

        {
        Utf8String testFilePath = CopyTestFile(L"besqliteprofile3_1_0_0_noembedfiletable.db");
        Db db;
        auto stat = db.OpenBeSQLiteDb(testFilePath.c_str(), Db::OpenParams(Db::OpenMode::Readonly));
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Opening test file " << testFilePath.c_str() << " failed.";
        AssertProfile(db, false, false, "Test scenario: Open old BeSQLite file without embedded file table.");
        }

        {
        Utf8String testFilePath = CopyTestFile(L"besqliteprofile3_1_0_0_noembedfiletable.db");
        Db db;
        auto stat = db.OpenBeSQLiteDb(testFilePath.c_str(), Db::OpenParams(Db::OpenMode::ReadWrite));

        ASSERT_EQ (BE_SQLITE_OK, stat) << "Opening test file " << testFilePath.c_str() << " failed.";
        stat = db.UpgradeBeSQLiteProfile();
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Upgrading test file " << testFilePath.c_str() << " failed.";

        AssertProfile(db, true, true, "Test scenario: Upgrade old BeSQLite file without embedded file table.");
        }

        {
        Utf8String testFilePath = CopyTestFile(L"besqliteprofile3_1_0_0_noembedfiletable.db");
        Db db;
        auto stat = db.OpenBeSQLiteDb(testFilePath.c_str(), Db::OpenParams(Db::OpenMode::ReadWrite));
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Opening test file " << testFilePath.c_str() << " failed.";

        BeFileName testFile;
        BeTest::GetHost().GetDocumentsRoot(testFile);
        testFile.AppendToPath(L"DgnDb").AppendToPath(L"StartupCompany.json");

        stat = db.EmbeddedFiles().Import("test", testFile.GetNameUtf8().c_str(), ".json");
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Embedding test file failed.";

        AssertProfile(db, true, true, "Test scenario: Embedding file in old BeSQLite file without embedded file table.");
        }

        {
        Utf8String testFilePath = CopyTestFile(L"besqliteprofile3_1_0_0_withembedfiletable.db");
        Db db;
        auto stat = db.OpenBeSQLiteDb(testFilePath.c_str(), Db::OpenParams(Db::OpenMode::Readonly));
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Opening test file " << testFilePath.c_str() << " failed.";
        AssertProfile(db, true, false, "Test scenario: Old BeSQLite file with embedded file table.");
        }

        {
        Utf8String testFilePath = CopyTestFile(L"besqliteprofile3_1_0_0_withembedfiletable.db");
        Db db;
        auto stat = db.OpenBeSQLiteDb(testFilePath.c_str(), Db::OpenParams(Db::OpenMode::ReadWrite));
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Opening test file " << testFilePath.c_str() << " failed.";
        stat = db.UpgradeBeSQLiteProfile();
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Upgrading test file " << testFilePath.c_str() << " failed.";

        AssertProfile(db, true, true, "Test scenario: Upgrade old BeSQLite file with embedded file table.");
        }

        {
        Utf8String testFilePath = CopyTestFile(L"besqliteprofile3_1_0_0_withembedfiletable.db");
        Db db;
        auto stat = db.OpenBeSQLiteDb(testFilePath.c_str(), Db::OpenParams(Db::OpenMode::Readonly));
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Opening test file " << testFilePath.c_str() << " failed.";
        stat = db.UpgradeBeSQLiteProfile();
        ASSERT_EQ (BE_SQLITE_ERROR_ProfileUpgradeFailed, stat) << "Upgrading test file " << testFilePath.c_str() << " is expected to fail if file is opened readonly.";
        }

        {
        Utf8String testFilePath = CopyTestFile(L"besqliteprofile3_1_0_0_withembedfiletable.db");
        Db db;
        auto stat = db.OpenBeSQLiteDb(testFilePath.c_str(), Db::OpenParams(Db::OpenMode::ReadWrite));
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Opening test file " << testFilePath.c_str() << " failed.";

        BeFileName testFile;
        BeTest::GetHost().GetDocumentsRoot(testFile);
        testFile.AppendToPath(L"DgnDb").AppendToPath(L"StartupCompany.json");

        stat = db.EmbeddedFiles().Import("test", testFile.GetNameUtf8().c_str(), ".json");
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Embedding test file failed.";

        AssertProfile(db, true, true, "Test scenario: Embedding file in old BeSQLite file with embedded file table.");
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
        auto stat = SetupDb(db, L"changerepoid.idgndb");
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Creation of test BeSQLite DB failed.";
        dbPath.assign(db.GetDbFileName());
        db.CloseDb();
        }

    Db db;
    DbResult stat = db.OpenBeSQLiteDb(dbPath.c_str(), Db::OpenParams(Db::OpenMode::Readonly));
    ASSERT_EQ (BE_SQLITE_OK, stat) << L"Reopening test DgnDb '" << dbPath.c_str() << L"' failed.";

    BeTest::SetFailOnAssert(false);
    stat = db.ChangeRepositoryId(BeRepositoryId(12345));
    BeTest::SetFailOnAssert(true);
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
        auto stat = SetupDb(db, L"changerepoid.idgndb");
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Creation of test BeSQLite DB failed.";
        dbPath.assign(db.GetDbFileName());

        std::vector<int> localValues = { 1234, 111, -111, 0 };
        std::vector<Utf8String> localValueNames = { "key0", "key1", "key2", "key3" };
        const size_t valueCount = localValues.size();
        for (size_t i = 0; i < valueCount; i++)
            {
            int val = localValues[i];
            size_t keyIndex = 0;
            ASSERT_EQ (BE_SQLITE_OK, db.GetRLVCache().Register(keyIndex, localValueNames[i].c_str())) << "Registration of RLV " << localValueNames[i].c_str() << " is expected to succeed.";
            auto result = db.GetRLVCache().SaveValue(keyIndex, val);
            ASSERT_EQ (BE_SQLITE_OK, result) << L"Saving test RLV '" << localValueNames[i].c_str() << L"=" << val << L"' failed";
            }

        ASSERT_EQ (BE_SQLITE_OK, db.SaveChanges()) << L"Committing repository local values failed.";
        db.CloseDb();
        }

    //reopen DgnDb again, change repo id and close again (to avoid that caches linger around)
    BeRepositoryId expectedRepoId;
    expectedRepoId.Invalidate();

        {
        Db db;
        DbResult stat = db.OpenBeSQLiteDb(dbPath.c_str(), Db::OpenParams(Db::OpenMode::ReadWrite));
        ASSERT_EQ (BE_SQLITE_OK, stat) << L"Reopening test DgnDb '" << dbPath.c_str() << L"' failed.";

        //now change repository id. This should truncate be_local and reinsert the new repo id
        const BeRepositoryId currentRepoId = db.GetRepositoryId();
        expectedRepoId = currentRepoId.GetNextRepositoryId();
        stat = db.ChangeRepositoryId(expectedRepoId);
        ASSERT_EQ (BE_SQLITE_OK, stat) << L"Changing the repository id is not expected to fail.";
        }

    //now reopen from scratch
    Db db;
    DbResult stat = db.OpenBeSQLiteDb(dbPath.c_str(), Db::OpenParams(Db::OpenMode::Readonly));
    ASSERT_EQ (BE_SQLITE_OK, stat) << L"Reopening test DgnDb '" << dbPath.c_str() << L"' failed.";

    Utf8CP const repoIdKey = "be_repositoryId";
    size_t repoIdIndex = 0;
    ASSERT_TRUE (db.GetRLVCache().TryGetIndex(repoIdIndex, repoIdKey));
    //query be_local to check that there is only one row (the repo id)
    Statement statement;
    ASSERT_EQ (BE_SQLITE_OK, statement.Prepare(db, "SELECT Name, Val from " BEDB_TABLE_Local)) << L"Preparing SQL statement to retrieve content of be_local failed";

    int rowCount = 0;
    while (statement.Step() == BE_SQLITE_ROW)
        {
        rowCount++;
        Utf8CP name = statement.GetValueText(0);
        EXPECT_STREQ (repoIdKey, name) << L"be_local after a repository id change should only contain the repository id.";
        //don't mimick the blob deserialization here. Just test that the column is not null. Use the API to check the actual repo id later
        EXPECT_FALSE (statement.IsColumnNull(1)) << "Val column of repository id row in be_local after a repository id change must not be null.";
        }

    EXPECT_EQ (1, rowCount) << L"be_local after a repository id change should only contain one row (the repo id).";

    //check value of repo id through API. As we opened file from scratch the old id cannot be in the cache anymore.
    int64_t actualRepoId;
    db.GetRLVCache().QueryValue(actualRepoId, repoIdIndex);
    EXPECT_EQ (expectedRepoId.GetValue(), (int32_t) actualRepoId) << L"QueryRepositoryLocalValue did not return the right value for repository id after repository id had been changed.";
    }
