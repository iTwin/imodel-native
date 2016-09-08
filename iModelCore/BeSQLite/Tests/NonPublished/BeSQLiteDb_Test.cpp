/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/BeSQLiteDb_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
    EXPECT_EQ(BE_SQLITE_OK, result) << "Db Creation failed";
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

    ASSERT_FALSE(fileIsAutoUpgradable);
    ASSERT_EQ(BE_SQLITE_INTERNAL, actualStat);

    BeTest::SetFailOnAssert(false);
    actualStat = Db::CheckProfileVersion(fileIsAutoUpgradable, dummy, dummy,
                                         dummy, true, "");
    BeTest::SetFailOnAssert(true);

    ASSERT_FALSE(fileIsAutoUpgradable);
    ASSERT_EQ(BE_SQLITE_INTERNAL, actualStat);
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
    testDataset[SchemaVersion(0, 0, 0, 0)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooOld, false);
    testDataset[SchemaVersion(1, 0, 0, 0)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooOld, false);
    testDataset[SchemaVersion(1, 8, 99, 0)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooOld, false);
    testDataset[SchemaVersion(1, 9, 0, 0)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooOld, true);
    testDataset[SchemaVersion(1, 9, 2, 3)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooOld, true);

    testDataset[SchemaVersion(1, 9, 2, 4)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooOld, true);
    testDataset[SchemaVersion(1, 9, 2, 5)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooOld, true);
    testDataset[SchemaVersion(1, 9, 3, 0)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooOld, true);
    testDataset[SchemaVersion(1, 9, 9, 0)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooOld, true);
    testDataset[SchemaVersion(2, 0, 0, 0)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooOld, true);
    testDataset[SchemaVersion(2, 1, 0, 0)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooOld, true);
    testDataset[SchemaVersion(2, 4, 0, 0)] = ExpectedResult(BE_SQLITE_OK, true);
    testDataset[SchemaVersion(2, 4, 5, 0)] = ExpectedResult(BE_SQLITE_OK, true);
    testDataset[SchemaVersion(2, 4, 5, 2)] = ExpectedResult(BE_SQLITE_OK, true);

    testDataset[SchemaVersion(2, 4, 5, 3)] = ExpectedResult(BE_SQLITE_OK, false);

    testDataset[SchemaVersion(2, 4, 5, 4)] = ExpectedResult(BE_SQLITE_OK, false);
    testDataset[SchemaVersion(2, 4, 5, 33)] = ExpectedResult(BE_SQLITE_OK, false);

    testDataset[SchemaVersion(2, 4, 6, 0)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooNewForReadWrite, BE_SQLITE_OK, false);
    testDataset[SchemaVersion(2, 4, 6, 99)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooNewForReadWrite, BE_SQLITE_OK, false);
    testDataset[SchemaVersion(2, 4, 99, 0)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooNewForReadWrite, BE_SQLITE_OK, false);
    testDataset[SchemaVersion(2, 4, 99, 99)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooNewForReadWrite, BE_SQLITE_OK, false);

    testDataset[SchemaVersion(2, 5, 0, 0)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooNew, false);
    testDataset[SchemaVersion(2, 5, 0, 1)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooNew, false);
    testDataset[SchemaVersion(2, 99, 0, 1)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooNew, false);
    testDataset[SchemaVersion(3, 0, 0, 0)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooNew, false);
    testDataset[SchemaVersion(99, 99, 99, 99)] = ExpectedResult(BE_SQLITE_ERROR_ProfileTooNew, false);

    for (auto const& testItem : testDataset)
        {
        SchemaVersion const& actualProfileVersion = testItem.first;
        ExpectedResult const& expectedResult = testItem.second;

        bool actualfileIsAutoUpgradable = false;
        DbResult actualStat = Db::CheckProfileVersion(actualfileIsAutoUpgradable, expectedProfileVersion, actualProfileVersion,
                                                      minimumAutoUpgradeProfileVersion, true, "Test");

        EXPECT_EQ(expectedResult.m_fileIsAutoUpgradable, actualfileIsAutoUpgradable) << "OpenMode: read-only - Expected version: " << expectedProfileVersion.ToJson().c_str() << " - Actual version: " << actualProfileVersion.ToJson().c_str();
        EXPECT_EQ(expectedResult.m_statInReadOnlyMode, actualStat) << "OpenMode: read-only - Expected version: " << expectedProfileVersion.ToJson().c_str() << " - Actual version: " << actualProfileVersion.ToJson().c_str();

        //now re-run check with read-write open mode
        actualfileIsAutoUpgradable = false;
        actualStat = Db::CheckProfileVersion(actualfileIsAutoUpgradable, expectedProfileVersion, actualProfileVersion,
                                             minimumAutoUpgradeProfileVersion, false, "Test");

        EXPECT_EQ(expectedResult.m_fileIsAutoUpgradable, actualfileIsAutoUpgradable) << "OpenMode: read-write - Expected version: " << expectedProfileVersion.ToJson().c_str() << " - Actual version: " << actualProfileVersion.ToJson().c_str();
        EXPECT_EQ(expectedResult.m_statInReadWriteMode, actualStat) << "OpenMode: read-write - Expected version: " << expectedProfileVersion.ToJson().c_str() << " - Actual version: " << actualProfileVersion.ToJson().c_str();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST(BeSQLiteDb, ChangeBriefcaseIdInReadonlyMode)
    {
    Utf8String dbPath;

    //prepare test dgn db
    {
    Db db;
    auto stat = SetupDb(db, L"changerepoid.ibim");
    ASSERT_EQ(BE_SQLITE_OK, stat) << "Creation of test BeSQLite DB failed.";
    dbPath.assign(db.GetDbFileName());
    db.CloseDb();
    }

    Db db;
    DbResult stat = db.OpenBeSQLiteDb(dbPath.c_str(), Db::OpenParams(Db::OpenMode::Readonly));
    ASSERT_EQ(BE_SQLITE_OK, stat) << L"Reopening test Bim '" << dbPath.c_str() << L"' failed.";

    BeTest::SetFailOnAssert(false);
    stat = db.ChangeBriefcaseId(BeBriefcaseId(12345));
    BeTest::SetFailOnAssert(true);
    ASSERT_EQ(BE_SQLITE_READONLY, stat) << L"Calling ChangeBriefcaseId on readonly Bim file is expected to fail.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST(BeSQLiteDb, ChangeBriefcaseId)
    {
    Utf8String dbPath;

    //prepare test dgn db
    {
    Db db;
    auto stat = SetupDb(db, L"changebriefcaseid.db");
    ASSERT_EQ(BE_SQLITE_OK, stat) << "Creation of test BeSQLite DB failed.";
    dbPath.assign(db.GetDbFileName());

    std::vector<int> localValues = {1234, 111, -111, 0};
    std::vector<Utf8String> localValueNames = {"key0", "key1", "key2", "key3"};
    const size_t valueCount = localValues.size();
    for (size_t i = 0; i < valueCount; i++)
        {
        int val = localValues[i];
        size_t keyIndex = 0;
        ASSERT_EQ(BE_SQLITE_OK, db.GetBLVCache().Register(keyIndex, localValueNames[i].c_str())) << "Registration of RLV " << localValueNames[i].c_str() << " is expected to succeed.";
        auto result = db.GetBLVCache().SaveValue(keyIndex, val);
        ASSERT_EQ(BE_SQLITE_OK, result) << "Saving test BLV '" << localValueNames[i].c_str() << "=" << val << "' failed";
        }

    ASSERT_EQ(BE_SQLITE_OK, db.SaveChanges()) << "Committing briefcase local values failed.";
    db.CloseDb();
    }

    //reopen Bim again, change briefcase id and close again (to avoid that caches linger around)
    BeBriefcaseId expectedBriefcaseId;
    expectedBriefcaseId.Invalidate();

    {
    Db db;
    DbResult stat = db.OpenBeSQLiteDb(dbPath.c_str(), Db::OpenParams(Db::OpenMode::ReadWrite));
    ASSERT_EQ(BE_SQLITE_OK, stat) << "Reopening test Bim '" << dbPath.c_str() << "' failed.";

    //now change briefcase id. This should truncate be_local and reinsert the new briefcase id
    const BeBriefcaseId currentBriefcaseId = db.GetBriefcaseId();
    expectedBriefcaseId = currentBriefcaseId.GetNextBriefcaseId();
    stat = db.ChangeBriefcaseId(expectedBriefcaseId);
    ASSERT_EQ(BE_SQLITE_OK, stat) << "Changing the briefcase id is not expected to fail.";
    }

    //now reopen from scratch
    Db db;
    DbResult stat = db.OpenBeSQLiteDb(dbPath.c_str(), Db::OpenParams(Db::OpenMode::Readonly));
    ASSERT_EQ(BE_SQLITE_OK, stat) << "Reopening test Bim '" << dbPath.c_str() << "' failed.";

    //query be_local to check that there is only one row (the repo id)
    Statement statement;
    ASSERT_EQ(BE_SQLITE_OK, statement.Prepare(db, "SELECT Name, Val from " BEDB_TABLE_Local)) << "Preparing SQL statement to retrieve content of be_local failed";
    int rowCount = 0;
    while (statement.Step() == BE_SQLITE_ROW)
        {
        rowCount++;
        Utf8CP name = statement.GetValueText(0);
        // NB: "repository" here really means "briefcase", but we don't want to break existing DgnDbs.
        ASSERT_STREQ("be_repositoryid", name) << "be_local after a briefcase id change should only contain the briefcase id.";
        //don't mimick the blob deserialization here. Just test that the column is not null. Use the API to check the actual repo id later
        ASSERT_FALSE(statement.IsColumnNull(1)) << "Val column of briefcase id row in be_local after a briefcase id change must not be null.";
        }

    ASSERT_EQ(1, rowCount) << "be_local after a briefcase id change should only contain one row (the repo id).";
    }
