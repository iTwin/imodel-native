/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeSQLiteDb, BeBriefcaseBasedIdTest)
    {
    BeBriefcaseId bc(0x103);
    BeBriefcaseBasedId id1(bc, 0x108d7de7e);
    EXPECT_TRUE(bc == id1.GetBriefcaseId());
    EXPECT_TRUE(0x108d7de7e == id1.GetLocalId());

    Utf8String val=id1.ToHexStr();
    EXPECT_TRUE(val == "0x1030108d7de7e");

    BeBriefcaseBasedId id2 = BeBriefcaseBasedId::CreateFromJson(Json::Value(val));
    EXPECT_TRUE(id2 == id1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     11/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST(BeSQLiteDb, CheckProfileVersion)
    {
    ProfileVersion expectedProfileVersion(2, 4, 5, 3);
    ProfileVersion minimumUpgradeProfileVersion(2, 4, 0, 0);

    std::map<ProfileVersion, ProfileState> testDataset {{ProfileVersion(0, 0, 0, 0), ProfileState::Older(ProfileState::CanOpen::No, false)},
    {ProfileVersion(1, 0, 0, 0), ProfileState::Older(ProfileState::CanOpen::No, false)},
    {ProfileVersion(1, 9, 0, 0), ProfileState::Older(ProfileState::CanOpen::No, false)},
    {ProfileVersion(2, 4, 0, 0), ProfileState::Older(ProfileState::CanOpen::Readonly, true)},
    {ProfileVersion(2, 4, 2, 3), ProfileState::Older(ProfileState::CanOpen::Readonly, true)},
    {ProfileVersion(2, 4, 5, 0), ProfileState::Older(ProfileState::CanOpen::Readwrite, true)},
    {ProfileVersion(2, 4, 5, 2), ProfileState::Older(ProfileState::CanOpen::Readwrite, true)},

    {ProfileVersion(2, 4, 5, 3), ProfileState::UpToDate()},

    {ProfileVersion(2, 4, 5, 4), ProfileState::Newer(ProfileState::CanOpen::Readwrite)},
    {ProfileVersion(2, 4, 5, 33), ProfileState::Newer(ProfileState::CanOpen::Readwrite)},

    {ProfileVersion(2, 4, 6, 0), ProfileState::Newer(ProfileState::CanOpen::Readonly)},
    {ProfileVersion(2, 4, 6, 99), ProfileState::Newer(ProfileState::CanOpen::Readonly)},
    {ProfileVersion(2, 4, 99, 0), ProfileState::Newer(ProfileState::CanOpen::Readonly)},
    {ProfileVersion(2, 4, 99, 99), ProfileState::Newer(ProfileState::CanOpen::Readonly)},

    {ProfileVersion(2, 5, 0, 0), ProfileState::Newer(ProfileState::CanOpen::No)},
    {ProfileVersion(2, 5, 0, 1), ProfileState::Newer(ProfileState::CanOpen::No)},
    {ProfileVersion(2, 99, 0, 1), ProfileState::Newer(ProfileState::CanOpen::No)},
    {ProfileVersion(3, 0, 0, 0), ProfileState::Newer(ProfileState::CanOpen::No)},
    {ProfileVersion(99, 99, 99, 99), ProfileState::Newer(ProfileState::CanOpen::No)}};

    for (std::pair<ProfileVersion, ProfileState> const& testItem : testDataset)
        {
        ProfileVersion const& actualProfileVersion = testItem.first;
        ProfileState const& expectedState = testItem.second;

        ProfileState actualState = Db::CheckProfileVersion(expectedProfileVersion, actualProfileVersion, minimumUpgradeProfileVersion, "Test");

        EXPECT_EQ(expectedState, actualState) <<  "Expected version: " << expectedProfileVersion.ToJson().c_str() << " - Actual version: " << actualProfileVersion.ToJson().c_str();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST(BeSQLiteDb, AssignBriefcaseIdInReadonlyMode)
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
    stat = db.SetAsBriefcase(BeBriefcaseId(12345));
    BeTest::SetFailOnAssert(true);
    ASSERT_EQ(BE_SQLITE_READONLY, stat) << L"Calling SetAsBriefcase on readonly Bim file is expected to fail.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST(BeSQLiteDb, AssignBriefcaseId)
    {
    Utf8String dbPath;

    //prepare test dgn db
    {
    Db db;
    auto stat = SetupDb(db, L"assignbriefcaseid.db");
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
    stat = db.SetAsBriefcase(expectedBriefcaseId);
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
