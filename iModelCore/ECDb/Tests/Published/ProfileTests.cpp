/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

static const PropertySpec PROFILEVERSION_PROPSPEC ("SchemaVersion", "ec_Db");

static Utf8CP const PROFILE_TABLE = "ec_Schema";
static Utf8CP const ECINSTANCEIDSEQUENCE_KEY = "ec_instanceidsequence";

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbTestFixture, Profile)
    {
    BeFileName dbPath = BuildECDbPath("ecdbprofiletest.db");
    if (dbPath.DoesPathExist())
        {
        // Delete any previously created file
        ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeDeleteFile(dbPath));
        }


    //first create an SQLite file with basic profile only (no ECDb)
    {
    Db db;
    DbResult stat = db.CreateNewDb(dbPath);
    EXPECT_EQ(BE_SQLITE_OK, stat) << L"Creating BeSQLite file failed";

    EXPECT_FALSE(db.TableExists(PROFILE_TABLE)) << "BeSQLite file is not expected to contain tables of the EC profile";

    Utf8String profileVersion;
    EXPECT_EQ(BE_SQLITE_ERROR, db.QueryProperty(profileVersion, PROFILEVERSION_PROPSPEC)) << "BeSQLite file is not expected to contain the ECDb profile version.";

    size_t sequenceIndex = 0;
    ASSERT_FALSE(db.GetBLVCache().TryGetIndex(sequenceIndex, ECINSTANCEIDSEQUENCE_KEY));
    db.CloseDb();
    }

    //now create an ECDb file
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("ecdbprofiletest.ecdb"));

    EXPECT_TRUE(GetHelper().TableExists(PROFILE_TABLE)) << "ECDb profile table not found in ECDb file which was newly created.";

    Utf8String actualProfileVersionStr;
    EXPECT_EQ(BE_SQLITE_ROW, m_ecdb.QueryProperty(actualProfileVersionStr, PROFILEVERSION_PROPSPEC)) << L"ECDb file is expected to contain an entry for the ECDb profile version in be_prop.";
    ProfileVersion actualProfileVersion(actualProfileVersionStr.c_str());
    EXPECT_EQ(ECDb::CurrentECDbProfileVersion(), actualProfileVersion) << "Unexpected ECDb profile version of new ECDb file. Actual version: " << actualProfileVersionStr.c_str();

    size_t sequenceIndex = 0;
    ASSERT_TRUE(m_ecdb.GetBLVCache().TryGetIndex(sequenceIndex, ECINSTANCEIDSEQUENCE_KEY));

    uint64_t lastECInstanceId = -1LL;
    EXPECT_EQ(BE_SQLITE_OK, m_ecdb.GetBLVCache().QueryValue(lastECInstanceId, sequenceIndex)) << L"ECInstanceId sequence not found in ECDb file which was newly created";
    }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbTestFixture, ProfileSchemas)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("empty.ecdb"));

    ECSchemaCP systemSchema = m_ecdb.Schemas().GetSchema("ECDbSystem");
    ASSERT_TRUE(systemSchema != nullptr);

    //Terminology of system/standard schemas is not clear yet for the EC3 world. Right now, the profile schemas are neither of that.
    ASSERT_FALSE(systemSchema->IsSystemSchema());

    ECSchemaCP fileInfoSchema = m_ecdb.Schemas().GetSchema("ECDbFileInfo");
    ASSERT_TRUE(fileInfoSchema != nullptr);

    ASSERT_FALSE(fileInfoSchema->IsSystemSchema());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbTestFixture, GetECDbProfileVersion)
    {
    //Test GetECDbProfileVersion on freshly created file
    BeFileName ecdbPath = BuildECDbPath("empty.ecdb");
    if (ecdbPath.DoesPathExist())
        ASSERT_EQ(BeFileNameStatus::Success, ecdbPath.BeDeleteFile());

    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.CreateNewDb(ecdbPath));
    EXPECT_FALSE(m_ecdb.GetECDbProfileVersion().IsEmpty()) << "Profile version is expected to be set in the ECDb handle during open";
    EXPECT_EQ(ECDb::CurrentECDbProfileVersion(), m_ecdb.GetECDbProfileVersion()) << "Profile version is expected to be set in the ECDb handle during open";

    //now test that version is set on open
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    EXPECT_FALSE(m_ecdb.GetECDbProfileVersion().IsEmpty()) << "Profile version is expected to be set in the ECDb handle during open";
    EXPECT_EQ(ECDb::CurrentECDbProfileVersion(), m_ecdb.GetECDbProfileVersion()) << "Profile version is expected to be set in the ECDb handle during open";
    }

//---------------------------------------------------------------------------------------
// Test to verify TFS 107173: ECDb profile creation should fail if it exists
// @bsimethod                                     Majd.Uddin                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbTestFixture, CreateProfileFailsIfAlreadyCreated)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("ecdbprofiletest2.ecdb"));

    EXPECT_TRUE(GetHelper().TableExists(PROFILE_TABLE)) << "ECDb profile table not found in ECDb file which was newly created.";

    //Drop a few tables
    EXPECT_EQ(BE_SQLITE_OK, m_ecdb.DropTable("ec_ClassMap"));
    EXPECT_EQ(BE_SQLITE_OK, m_ecdb.DropTable("be_Local"));
    m_ecdb.CloseDb();

    WString ecdbFileNameW("ecdbprofiletest2.ecdb", BentleyCharEncoding::Utf8);
    BeFileName ecdbFilePath;
    BeTest::GetHost().GetOutputRoot(ecdbFilePath);
    ecdbFilePath.AppendToPath(ecdbFileNameW.c_str());
    Utf8String ecdbFilePathUtf8 = ecdbFilePath.GetNameUtf8();

    //create the Db again, it should fail at already existing
    DbResult stat = m_ecdb.CreateNewDb(ecdbFilePathUtf8.c_str());
    EXPECT_EQ(BE_SQLITE_ERROR_FileExists, stat);

    //create the Db again with SetFailIfDbExist set to false i.e. force re-creation
    BeSQLite::Db::CreateParams params;
    params.SetFailIfDbExist(false);
    stat = m_ecdb.CreateNewDb(ecdbFilePathUtf8.c_str(), BeSQLite::BeGuid(true), params);
    EXPECT_EQ(BE_SQLITE_ERROR, stat);
    }


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  04/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbTestFixture, ProfileCreation)
    {
    auto defaultTxnModeToString = [] (DefaultTxn txnMode)
        {
        switch (txnMode)
            {
                case DefaultTxn::Exclusive:
                    return "Exclusive";
                case DefaultTxn::Immediate:
                    return "Immediate";
                case DefaultTxn::No:
                    return "No";
                case DefaultTxn::Yes:
                    return "Yes";
                default:
                    BeAssert(false);
                    return "error";
            }
        };

    const std::vector<DefaultTxn> defaultTxnModes {DefaultTxn::No, DefaultTxn::Yes, DefaultTxn::Exclusive, DefaultTxn::Immediate};

    BeFileName testFilePath;
    BeTest::GetHost().GetOutputRoot(testFilePath);
    testFilePath.AppendToPath(WString("profiletest.ecdb", BentleyCharEncoding::Utf8).c_str());

    for (DefaultTxn defaultTxnMode : defaultTxnModes)
        {
        if (testFilePath.DoesPathExist())
            ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeDeleteFile(testFilePath));

        ECDb::CreateParams params;
        params.SetStartDefaultTxn(defaultTxnMode);
        ECDb ecdb;
        ASSERT_EQ(BE_SQLITE_OK, ecdb.CreateNewDb(testFilePath, BeGuid(), params)) << "DefaultTxn mode: " << defaultTxnModeToString(defaultTxnMode);
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbTestFixture, CheckECDbProfileVersion)
    {
    std::vector<std::pair<ProfileVersion, ProfileState>> expectedProfileStates {
            {ProfileVersion(3,6,99,0), ProfileState::Older(ProfileState::CanOpen::No, false)},
            {ProfileVersion(3,7,0,0), ProfileState::Older(ProfileState::CanOpen::No, false)},
            {ProfileVersion(3,7,0,1), ProfileState::Older(ProfileState::CanOpen::No, false)},
            {ProfileVersion(3,7,3,1), ProfileState::Older(ProfileState::CanOpen::No, false)},
            {ProfileVersion(3,7,3,2), ProfileState::Older(ProfileState::CanOpen::No, false)},
            {ProfileVersion(3,7,3,3), ProfileState::Older(ProfileState::CanOpen::No, false)},
            {ProfileVersion(3,7,4,3), ProfileState::Older(ProfileState::CanOpen::No, false)},
            {ProfileVersion(3,100,0,0), ProfileState::Older(ProfileState::CanOpen::No, false)},
            {ProfileVersion(3,100,0,1), ProfileState::Older(ProfileState::CanOpen::No, false)},
            {ProfileVersion(3,100,1,1), ProfileState::Older(ProfileState::CanOpen::No, false)},
            {ProfileVersion(4,0,0,0), ProfileState::Older(ProfileState::CanOpen::Readwrite, true)},
            {ProfileVersion(4,0,0,1), ProfileState::Older(ProfileState::CanOpen::Readwrite, true)},
            {ProfileVersion(4,0,0,2), ProfileState::Older(ProfileState::CanOpen::Readwrite, true)},
            {ProfileVersion(4,0,0,3), ProfileState::UpToDate()},
            {ProfileVersion(4,0,0,4), ProfileState::Newer(ProfileState::CanOpen::Readwrite)},
            {ProfileVersion(4,0,1,0), ProfileState::Newer(ProfileState::CanOpen::Readonly)},
            {ProfileVersion(4,0,1,1), ProfileState::Newer(ProfileState::CanOpen::Readonly)},
            {ProfileVersion(4,0,1,99), ProfileState::Newer(ProfileState::CanOpen::Readonly)},
            {ProfileVersion(4,0,2,0), ProfileState::Newer(ProfileState::CanOpen::Readonly)},
            {ProfileVersion(4,0,10,99), ProfileState::Newer(ProfileState::CanOpen::Readonly)},
            {ProfileVersion(4,1,0,0), ProfileState::Newer(ProfileState::CanOpen::No)},
            {ProfileVersion(5,0,0,0), ProfileState::Newer(ProfileState::CanOpen::No)}
        };

    for (std::pair<ProfileVersion, ProfileState> const& testVersion : expectedProfileStates)
        {
        ProfileVersion const& testProfileVersion = testVersion.first;
        ProfileState const& expectedProfileState = testVersion.second;
        EXPECT_EQ(expectedProfileState, ECDb::CheckProfileVersion(ECDb::CurrentECDbProfileVersion(), testProfileVersion, ECDb::MinimumUpgradableECDbProfileVersion(), "ECDb")) << testVersion.first.ToJson();
        }

    std::vector<ProfileVersion> expectedTooOld = {ProfileVersion(3,6,99,0), ProfileVersion(3,7,0,0),ProfileVersion(3,7,0,1),ProfileVersion(3,7,3,1),ProfileVersion(3,7,3,2),ProfileVersion(3,7,4,3),ProfileVersion(3,100,0,0), ProfileVersion(3,100,0,1), ProfileVersion(3,100,1,1)};
    std::vector<ProfileVersion> expectedOlderReadWriteAndUpgradable = {ProfileVersion(4,0,0,0), ProfileVersion(4,0,0,1)};
    ProfileVersion expectedUpToDate = ProfileVersion(4,0,0,3);
    std::vector<ProfileVersion> expectedNewerReadWrite = {ProfileVersion(4,0,0,4), ProfileVersion(4,0,0,5)};
    std::vector<ProfileVersion> expectedNewerReadonly = {ProfileVersion(4,0,1,0), ProfileVersion(4,0,1,3), ProfileVersion(4,0,2,0), ProfileVersion(4,0,10,99)};
    std::vector<ProfileVersion> expectedTooNew = {ProfileVersion(4,1,0,0), ProfileVersion(5,0,0,0)};

    auto fakeModifyProfileVersion = [] (BeFileNameCR filePath, ProfileVersion const& version)
        {
        Utf8String versionStr = version.ToJson();

        Db db;
        ASSERT_EQ(BE_SQLITE_OK, db.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite))) << versionStr;
        Statement stmt;
        ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(db, "UPDATE be_Prop SET StrData=? WHERE Namespace='ec_Db' AND Name='SchemaVersion'")) << versionStr;
        ASSERT_EQ(BE_SQLITE_OK, stmt.BindText(1, versionStr, Statement::MakeCopy::Yes)) << versionStr;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << versionStr;
        ASSERT_EQ(1, db.GetModifiedRowCount()) << versionStr;
        ASSERT_EQ(BE_SQLITE_OK, db.SaveChanges()) << versionStr;
        };

    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("ecdbprofiletest.ecdb"));
    BeFileName filePath(m_ecdb.GetDbFileName());
    CloseECDb();

    for (ProfileVersion const& testVersion : expectedTooOld)
        {
        fakeModifyProfileVersion(filePath, testVersion);
        ASSERT_EQ(BE_SQLITE_ERROR_ProfileTooOld, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::Readonly))) << testVersion.ToString();
        ASSERT_EQ(BE_SQLITE_ERROR_ProfileTooOld, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite))) << testVersion.ToString();
        ASSERT_EQ(BE_SQLITE_ERROR_ProfileTooOld, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite, Db::ProfileUpgradeOptions::Upgrade))) << testVersion.ToString();
        }

    for (ProfileVersion const& testVersion : expectedOlderReadWriteAndUpgradable)
        {
        fakeModifyProfileVersion(filePath, testVersion);
        ASSERT_EQ(BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite))) << testVersion.ToString();
        EXPECT_EQ(testVersion, m_ecdb.GetECDbProfileVersion()) << testVersion.ToString() << " | No upgrade";
        CloseECDb();
        ASSERT_EQ(BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::Readonly))) << testVersion.ToString();
        EXPECT_EQ(testVersion, m_ecdb.GetECDbProfileVersion()) << testVersion.ToString() << " | No upgrade";
        CloseECDb();
        {
        ScopedDisableFailOnAssertion disableAssertion;
        ASSERT_EQ(BE_SQLITE_READONLY, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::Readonly, Db::ProfileUpgradeOptions::Upgrade))) << testVersion.ToString();
        }
        //testing actual upgrade does not work as the file per se is not in the right state (the test just fake-modifies the profile version)
        }

    fakeModifyProfileVersion(filePath, expectedUpToDate);
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::Readonly))) << expectedUpToDate.ToString();
    EXPECT_EQ(expectedUpToDate, m_ecdb.GetECDbProfileVersion()) << expectedUpToDate.ToString() << " | No upgrade";
    CloseECDb();
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite))) << expectedUpToDate.ToString();
    EXPECT_EQ(expectedUpToDate, m_ecdb.GetECDbProfileVersion()) << expectedUpToDate.ToString() << " | No upgrade";
    CloseECDb();
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite, Db::ProfileUpgradeOptions::Upgrade))) << expectedUpToDate.ToString();
    EXPECT_EQ(expectedUpToDate, m_ecdb.GetECDbProfileVersion()) << expectedUpToDate.ToString() << " | No upgrade";
    CloseECDb();

    for (ProfileVersion const& testVersion : expectedNewerReadWrite)
        {
        fakeModifyProfileVersion(filePath, testVersion);
        ASSERT_EQ(BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::Readonly))) << testVersion.ToString();
        EXPECT_EQ(testVersion, m_ecdb.GetECDbProfileVersion()) << testVersion.ToString();
        CloseECDb();
        ASSERT_EQ(BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite))) << testVersion.ToString();
        EXPECT_EQ(testVersion, m_ecdb.GetECDbProfileVersion()) << testVersion.ToString();
        CloseECDb();
        ASSERT_EQ(BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite, Db::ProfileUpgradeOptions::Upgrade))) << expectedUpToDate.ToString();
        EXPECT_EQ(testVersion, m_ecdb.GetECDbProfileVersion()) << testVersion.ToString();
        CloseECDb();
        }

    for (ProfileVersion const& testVersion : expectedNewerReadonly)
        {
        fakeModifyProfileVersion(filePath, testVersion);
        ASSERT_EQ(BE_SQLITE_ERROR_ProfileTooNewForReadWrite, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite))) << testVersion.ToString();
        ASSERT_EQ(BE_SQLITE_ERROR_ProfileTooNewForReadWrite, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite, Db::ProfileUpgradeOptions::Upgrade))) << expectedUpToDate.ToString();
        ASSERT_EQ(BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::Readonly))) << testVersion.ToString();
        EXPECT_EQ(testVersion, m_ecdb.GetECDbProfileVersion()) << testVersion.ToString();
        CloseECDb();
        }

    for (ProfileVersion const& testVersion : expectedTooNew)
        {
        fakeModifyProfileVersion(filePath, testVersion);
        ASSERT_EQ(BE_SQLITE_ERROR_ProfileTooNew, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite))) << testVersion.ToString();
        ASSERT_EQ(BE_SQLITE_ERROR_ProfileTooNew, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite, Db::ProfileUpgradeOptions::Upgrade))) << expectedUpToDate.ToString();
        ASSERT_EQ(BE_SQLITE_ERROR_ProfileTooNew, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::Readonly))) << testVersion.ToString();
        }
    }

END_ECDBUNITTESTS_NAMESPACE
