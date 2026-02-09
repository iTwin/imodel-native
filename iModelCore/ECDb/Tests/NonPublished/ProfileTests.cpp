/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

static const PropertySpec PROFILEVERSION_PROPSPEC("SchemaVersion", "ec_Db");

static Utf8CP const PROFILE_TABLE = "ec_Schema";
static Utf8CP const ECINSTANCEIDSEQUENCE_KEY = "ec_instanceidsequence";

struct ProfileTestFixture : public ECDbTestFixture {
};

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ProfileTestFixture, Profile) {
    BeFileName dbPath = BuildECDbPath("ecdbprofiletest.db");
    if (dbPath.DoesPathExist()) {
        // Delete any previously created file
        ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeDeleteFile(dbPath));
    }

    // first create an SQLite file with basic profile only (no ECDb)
    {
        Db db;
        DbResult stat = db.CreateNewDb(dbPath);
        EXPECT_EQ(BE_SQLITE_OK, stat) << L"Creating BeSQLite file failed";

        EXPECT_FALSE(db.TableExists(PROFILE_TABLE)) << "BeSQLite file is not expected to contain tables of the EC profile";

        Utf8String profileVersion;
        EXPECT_EQ(BE_SQLITE_ERROR, db.QueryProperty(profileVersion, PROFILEVERSION_PROPSPEC)) << "BeSQLite file is not expected to contain the ECDb profile version.";

        size_t sequenceIndex = 0;
        ASSERT_FALSE(db.GetBLVCache().TryGetIndex(sequenceIndex, ECINSTANCEIDSEQUENCE_KEY));
        db.SaveChanges();
        db.CloseDb();
    }

    // now create an ECDb file
    {
        ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("ecdbprofiletest.ecdb"));

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
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ProfileTestFixture, ProfileSchemas) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("empty.ecdb"));

    ECSchemaCP systemSchema = m_ecdb.Schemas().GetSchema("ECDbSystem");
    ASSERT_TRUE(systemSchema != nullptr);

    // Terminology of system/standard schemas is not clear yet for the EC3 world. Right now, the profile schemas are neither of that.
    ASSERT_FALSE(systemSchema->IsSystemSchema());

    ECSchemaCP fileInfoSchema = m_ecdb.Schemas().GetSchema("ECDbFileInfo");
    ASSERT_TRUE(fileInfoSchema != nullptr);

    ASSERT_FALSE(fileInfoSchema->IsSystemSchema());
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ProfileTestFixture, GetECDbProfileVersion) {
    // Test GetECDbProfileVersion on freshly created file
    BeFileName ecdbPath = BuildECDbPath("empty.ecdb");
    if (ecdbPath.DoesPathExist())
        ASSERT_EQ(BeFileNameStatus::Success, ecdbPath.BeDeleteFile());

    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.CreateNewDb(ecdbPath));
    EXPECT_FALSE(m_ecdb.GetECDbProfileVersion().IsEmpty()) << "Profile version is expected to be set in the ECDb handle during open";
    EXPECT_EQ(ECDb::CurrentECDbProfileVersion(), m_ecdb.GetECDbProfileVersion()) << "Profile version is expected to be set in the ECDb handle during open";

    // now test that version is set on open
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    EXPECT_FALSE(m_ecdb.GetECDbProfileVersion().IsEmpty()) << "Profile version is expected to be set in the ECDb handle during open";
    EXPECT_EQ(ECDb::CurrentECDbProfileVersion(), m_ecdb.GetECDbProfileVersion()) << "Profile version is expected to be set in the ECDb handle during open";
}

//---------------------------------------------------------------------------------------
// Test to verify TFS 107173: ECDb profile creation should fail if it exists
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ProfileTestFixture, CreateProfileFailsIfAlreadyCreated) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("ecdbprofiletest2.ecdb"));

    EXPECT_TRUE(GetHelper().TableExists(PROFILE_TABLE)) << "ECDb profile table not found in ECDb file which was newly created.";

    // Drop a few tables
    EXPECT_EQ(BE_SQLITE_OK, m_ecdb.DropTable("ec_ClassMap"));
    EXPECT_EQ(BE_SQLITE_OK, m_ecdb.DropTable("be_Local"));
    m_ecdb.SaveChanges();
    m_ecdb.CloseDb();

    WString ecdbFileNameW("ecdbprofiletest2.ecdb", BentleyCharEncoding::Utf8);
    BeFileName ecdbFilePath;
    BeTest::GetHost().GetOutputRoot(ecdbFilePath);
    ecdbFilePath.AppendToPath(ecdbFileNameW.c_str());
    Utf8String ecdbFilePathUtf8 = ecdbFilePath.GetNameUtf8();

    // create the Db again, it should fail at already existing
    DbResult stat = m_ecdb.CreateNewDb(ecdbFilePathUtf8.c_str());
    EXPECT_EQ(BE_SQLITE_ERROR_FileExists, stat);

    // create the Db again with SetFailIfDbExist set to false i.e. force re-creation
    BeSQLite::Db::CreateParams params;
    params.SetFailIfDbExist(false);
    stat = m_ecdb.CreateNewDb(ecdbFilePathUtf8.c_str(), params);
    EXPECT_EQ(BE_SQLITE_ERROR, stat);
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ProfileTestFixture, ProfileCreation) {
    auto defaultTxnModeToString = [](DefaultTxn txnMode) {
        switch (txnMode) {
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

    const std::vector<DefaultTxn> defaultTxnModes{DefaultTxn::No, DefaultTxn::Yes, DefaultTxn::Exclusive, DefaultTxn::Immediate};

    BeFileName testFilePath;
    BeTest::GetHost().GetOutputRoot(testFilePath);
    testFilePath.AppendToPath(WString("profiletest.ecdb", BentleyCharEncoding::Utf8).c_str());

    for (DefaultTxn defaultTxnMode : defaultTxnModes) {
        if (testFilePath.DoesPathExist())
            ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeDeleteFile(testFilePath));

        ECDb::CreateParams params;
        params.SetStartDefaultTxn(defaultTxnMode);
        ECDb ecdb;
        ASSERT_EQ(BE_SQLITE_OK, ecdb.CreateNewDb(testFilePath, params)) << "DefaultTxn mode: " << defaultTxnModeToString(defaultTxnMode);
        ecdb.SaveChanges();
    }
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ProfileTestFixture, CheckECDbProfileVersion) {
    std::vector<std::pair<ProfileVersion, ProfileState>> expectedProfileStates{
        {ProfileVersion(3, 6, 99, 0), ProfileState::Older(ProfileState::CanOpen::No, false)},
        {ProfileVersion(3, 7, 0, 0), ProfileState::Older(ProfileState::CanOpen::No, false)},
        {ProfileVersion(3, 7, 0, 1), ProfileState::Older(ProfileState::CanOpen::No, false)},
        {ProfileVersion(3, 7, 3, 1), ProfileState::Older(ProfileState::CanOpen::No, false)},
        {ProfileVersion(3, 7, 3, 2), ProfileState::Older(ProfileState::CanOpen::No, false)},
        {ProfileVersion(3, 7, 3, 3), ProfileState::Older(ProfileState::CanOpen::No, false)},
        {ProfileVersion(3, 7, 4, 3), ProfileState::Older(ProfileState::CanOpen::No, false)},
        {ProfileVersion(3, 100, 0, 0), ProfileState::Older(ProfileState::CanOpen::No, false)},
        {ProfileVersion(3, 100, 0, 1), ProfileState::Older(ProfileState::CanOpen::No, false)},
        {ProfileVersion(3, 100, 1, 1), ProfileState::Older(ProfileState::CanOpen::No, false)},
        {ProfileVersion(4, 0, 0, 0), ProfileState::Older(ProfileState::CanOpen::Readwrite, true)},
        {ProfileVersion(4, 0, 0, 1), ProfileState::Older(ProfileState::CanOpen::Readwrite, true)},
        {ProfileVersion(4, 0, 0, 2), ProfileState::Older(ProfileState::CanOpen::Readwrite, true)},
        {ProfileVersion(4, 0, 0, 3), ProfileState::Older(ProfileState::CanOpen::Readwrite, true)},
        {ProfileVersion(4, 0, 0, 4), ProfileState::Older(ProfileState::CanOpen::Readwrite, true)},
        {ProfileVersion(4, 0, 0, 5), ProfileState::UpToDate()},
        {ProfileVersion(4, 0, 1, 0), ProfileState::Newer(ProfileState::CanOpen::Readwrite)},
        {ProfileVersion(4, 0, 1, 1), ProfileState::Newer(ProfileState::CanOpen::Readwrite)},
        {ProfileVersion(4, 0, 1, 99), ProfileState::Newer(ProfileState::CanOpen::Readwrite)},
        {ProfileVersion(4, 0, 2, 0), ProfileState::Newer(ProfileState::CanOpen::Readwrite)},
        {ProfileVersion(4, 0, 10, 99), ProfileState::Newer(ProfileState::CanOpen::Readwrite)},
        {ProfileVersion(4, 1, 0, 0), ProfileState::Newer(ProfileState::CanOpen::Readonly)},
        {ProfileVersion(5, 0, 0, 0), ProfileState::Newer(ProfileState::CanOpen::No)}};

    for (std::pair<ProfileVersion, ProfileState> const& testVersion : expectedProfileStates) {
        ProfileVersion const& testProfileVersion = testVersion.first;
        ProfileState const& expectedProfileState = testVersion.second;
        EXPECT_EQ(expectedProfileState, ECDb::CheckProfileVersion(ECDb::CurrentECDbProfileVersion(), testProfileVersion, ECDb::MinimumUpgradableECDbProfileVersion(), "ECDb")) << testVersion.first.ToJson();
    }

    std::vector<ProfileVersion> expectedTooOld = {ProfileVersion(3, 6, 99, 0), ProfileVersion(3, 7, 0, 0), ProfileVersion(3, 7, 0, 1), ProfileVersion(3, 7, 3, 1), ProfileVersion(3, 7, 3, 2), ProfileVersion(3, 7, 4, 3), ProfileVersion(3, 100, 0, 0), ProfileVersion(3, 100, 0, 1), ProfileVersion(3, 100, 1, 1)};
    std::vector<ProfileVersion> expectedOlderReadWriteAndUpgradable = {ProfileVersion(4, 0, 0, 0), ProfileVersion(4, 0, 0, 1), ProfileVersion(4, 0, 0, 2), ProfileVersion(4, 0, 0, 3), ProfileVersion(4, 0, 0, 4)};
    ProfileVersion expectedUpToDate = ProfileVersion(4, 0, 0, 5);
    std::vector<ProfileVersion> expectedNewerReadWrite = {ProfileVersion(4, 0, 0, 6), ProfileVersion(4, 0, 1, 0), ProfileVersion(4, 0, 1, 3), ProfileVersion(4, 0, 2, 0), ProfileVersion(4, 0, 10, 99)};
    std::vector<ProfileVersion> expectedNewerReadonly = {ProfileVersion(4, 1, 1, 0), ProfileVersion(4, 1, 0, 0)};
    std::vector<ProfileVersion> expectedTooNew = {ProfileVersion(5, 1, 0, 0), ProfileVersion(99, 0, 0, 0)};

    auto fakeModifyProfileVersion = [](BeFileNameCR filePath, ProfileVersion const& version) {
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

    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("ecdbprofiletest.ecdb"));
    BeFileName filePath(m_ecdb.GetDbFileName());
    CloseECDb();

    for (ProfileVersion const& testVersion : expectedTooOld) {
        fakeModifyProfileVersion(filePath, testVersion);
        ASSERT_EQ(BE_SQLITE_ERROR_ProfileTooOld, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::Readonly))) << testVersion.ToString();
        ASSERT_EQ(BE_SQLITE_ERROR_ProfileTooOld, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite))) << testVersion.ToString();
        ASSERT_EQ(BE_SQLITE_ERROR_ProfileTooOld, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite, Db::ProfileUpgradeOptions::Upgrade))) << testVersion.ToString();
    }

    for (ProfileVersion const& testVersion : expectedOlderReadWriteAndUpgradable) {
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
        // testing actual upgrade does not work as the file per se is not in the right state (the test just fake-modifies the profile version)
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

    for (ProfileVersion const& testVersion : expectedNewerReadWrite) {
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

    for (ProfileVersion const& testVersion : expectedNewerReadonly) {
        fakeModifyProfileVersion(filePath, testVersion);
        ASSERT_EQ(BE_SQLITE_ERROR_ProfileTooNewForReadWrite, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite))) << testVersion.ToString();
        ASSERT_EQ(BE_SQLITE_ERROR_ProfileTooNewForReadWrite, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite, Db::ProfileUpgradeOptions::Upgrade))) << expectedUpToDate.ToString();
        ASSERT_EQ(BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::Readonly))) << testVersion.ToString();
        EXPECT_EQ(testVersion, m_ecdb.GetECDbProfileVersion()) << testVersion.ToString();
        CloseECDb();
    }

    for (ProfileVersion const& testVersion : expectedTooNew) {
        fakeModifyProfileVersion(filePath, testVersion);
        ASSERT_EQ(BE_SQLITE_ERROR_ProfileTooNew, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite))) << testVersion.ToString();
        ASSERT_EQ(BE_SQLITE_ERROR_ProfileTooNew, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite, Db::ProfileUpgradeOptions::Upgrade))) << expectedUpToDate.ToString();
        ASSERT_EQ(BE_SQLITE_ERROR_ProfileTooNew, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::Readonly))) << testVersion.ToString();
    }
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ProfileTestFixture, ImportSchemaByProfileVersion) {
    auto fakeModifyProfileVersion = [](BeFileNameCR filePath, ProfileVersion const& version) {
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

    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("ecdbprofileschemaimporttest.ecdb"));
    BeFileName filePath(m_ecdb.GetDbFileName());
    CloseECDb();

    auto import = [this](SchemaItem const& schema, SchemaManager::SchemaImportOptions options) {
        BentleyStatus stat = ERROR;
        Savepoint sp(m_ecdb, "");
        {
            stat = GetHelper().ImportSchema(schema, options);
        }
        sp.Cancel();
        return stat;
    };

    SchemaItem schema(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                              <ECSchema schemaName="Schema1" alias="s1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                <ECEntityClass typeName="Foo" >
                                  <ECProperty propertyName="Length" typeName="double" />
                                </ECEntityClass>
                              </ECSchema>)xml");

    fakeModifyProfileVersion(filePath, ProfileVersion(4, 0, 0, 3));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite)));
    ASSERT_EQ(SUCCESS, import(schema, SchemaManager::SchemaImportOptions::None)) << "Import schema to newer sub2 profile";
    CloseECDb();

    fakeModifyProfileVersion(filePath, ProfileVersion(4, 0, 1, 0));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite)));
    ASSERT_EQ(ERROR, import(schema, SchemaManager::SchemaImportOptions::None)) << "Import schema to newer sub2 profile should fail";
    CloseECDb();
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ProfileTestFixture, ImportRequiresVersionCustomAttribute) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDbForCurrentTest());

    {
        SchemaItem schema(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema1" alias="s1" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.02" alias="ecdbmap"/>
        <ECCustomAttributes>
            <ImportRequiresVersion xmlns="ECDbMap.02.00.02">
                <ECDbRuntimeVersion>999.9.9.9</ECDbRuntimeVersion>
            </ImportRequiresVersion>
        </ECCustomAttributes>
        <ECEntityClass typeName="Foo" >
            <ECProperty propertyName="Length" typeName="double" />
        </ECEntityClass>
        </ECSchema>)xml");

        TestIssueListener issueListener;
        m_ecdb.AddIssueListener(issueListener);
        ASSERT_EQ(BentleyStatus::ERROR, ImportSchema(schema));

        ASSERT_FALSE(issueListener.IsEmpty()) << "Should raise an issue.";
        ASSERT_STREQ(Utf8PrintfString("ECSchema Schema1.01.00.01 requires ECDb version 999.9.9.9, but the current runtime version is only %s.", m_ecdb.GetECDbProfileVersion().ToString().c_str()).c_str(), issueListener.GetLastMessage().c_str());
    }

    CloseECDb();
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ProfileTestFixture, InvalidImportRequiresVersionCustomAttribute) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDbForCurrentTest());

    TestIssueListener issueListener;
    m_ecdb.AddIssueListener(issueListener);

    {  // no version property
        SchemaItem schema(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema1" alias="s1" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.02" alias="ecdbmap"/>
        <ECCustomAttributes>
            <ImportRequiresVersion xmlns="ECDbMap.02.00.02">
            </ImportRequiresVersion>
        </ECCustomAttributes>
        <ECEntityClass typeName="Foo" >
            <ECProperty propertyName="Length" typeName="double" />
        </ECEntityClass>
        </ECSchema>)xml");

        issueListener.ClearIssues();
        ASSERT_EQ(BentleyStatus::ERROR, ImportSchema(schema));

        ASSERT_FALSE(issueListener.IsEmpty()) << "Should raise an issue.";
        ASSERT_STREQ("ECSchema Schema1.01.00.01 has a ImportRequiresVersion custom attribute with a missing or invalid ECDbRuntimeVersion property.", issueListener.GetLastMessage().c_str());
    }

    {  // invalid version property
        SchemaItem schema(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema1" alias="s1" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.02" alias="ecdbmap"/>
        <ECCustomAttributes>
            <ImportRequiresVersion xmlns="ECDbMap.02.00.02">
                <ECDbRuntimeVersion>POIFOPKSOFGJG</ECDbRuntimeVersion>
            </ImportRequiresVersion>
        </ECCustomAttributes>
        <ECEntityClass typeName="Foo" >
            <ECProperty propertyName="Length" typeName="double" />
        </ECEntityClass>
        </ECSchema>)xml");

        issueListener.ClearIssues();
        ASSERT_EQ(BentleyStatus::ERROR, ImportSchema(schema));

        ASSERT_FALSE(issueListener.IsEmpty()) << "Should raise an issue.";
        ASSERT_STREQ("ECSchema Schema1.01.00.01 has a ImportRequiresVersion custom attribute with a missing or invalid ECDbRuntimeVersion property.", issueListener.GetLastMessage().c_str());
    }

    CloseECDb();
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ProfileTestFixture, ReferenceOlderSchemaWhenImportRestrictedNewerSchemaAlreadyExists) {
    // Example Scenario:
    //  Current ECDbRuntimeVersion is 4.0.0.4.
    //  A newer imodel contains 2 schemas "TestBaseSchema.1.0.0" with no import restriction and "TestBaseSchema.1.0.1" which contains an import restriction for the next ECDbRuntimeVersion 4.0.0.5.
    //  Try to import a new schema "TestSchema.1.0.0" which references "TestBaseSchema.1.0.0".
    // Expected Result: Import should succeed
    ASSERT_EQ(BE_SQLITE_OK, SetupECDbForCurrentTest());

    // Import the initial version of base schema "TestBaseSchema.1.0.0" which does not contain the "ImportRequiresVersion" CA
    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestBaseSchema" alias="tbs1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.02" alias="ecdbmap"/>

            <ECEntityClass typeName="Line" >
                <ECProperty propertyName="Length" typeName="double" />
            </ECEntityClass>
        </ECSchema>)xml")));
    auto baseSchema = m_ecdb.Schemas().GetSchema("TestBaseSchema");
    ASSERT_NE(baseSchema, nullptr);
    ASSERT_EQ(baseSchema->GetVersionMinor(), 0U);

    // Perform a schema upgrade of the base schema "TestBaseSchema.1.0.1", which now contains the "ImportRequiresVersion" CA
    const auto currVersion = ECDb::CurrentECDbProfileVersion();
    const auto baseSchemaStr = R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestBaseSchema" alias="tbs1" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.02" alias="ecdbmap"/>
            <ECCustomAttributes>
                <ImportRequiresVersion xmlns="ECDbMap.02.00.02">
                    <ECDbRuntimeVersion>%s</ECDbRuntimeVersion>
                </ImportRequiresVersion>
            </ECCustomAttributes>

            <ECEntityClass typeName="Line" >
                <ECProperty propertyName="Length" typeName="double" />
            </ECEntityClass>
        </ECSchema>)xml";
    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(Utf8PrintfString(baseSchemaStr, currVersion.ToString().c_str()))));
    baseSchema = m_ecdb.Schemas().GetSchema("TestBaseSchema");
    ASSERT_NE(baseSchema, nullptr);
    ASSERT_EQ(baseSchema->GetVersionMinor(), 1U);

    // Following step is only done to simulate a newer imodel which contains the schema that restricts imports for the current ECDbRuntimeVersion.
    const auto importRequiresVersionCA = baseSchema->GetCustomAttribute("ImportRequiresVersion");
    ASSERT_TRUE(importRequiresVersionCA.IsValid());
    ECValue ecdbRuntimeVersionValue;
    importRequiresVersionCA->GetValue(ecdbRuntimeVersionValue, "ECDbRuntimeVersion");
    ASSERT_STREQ(ecdbRuntimeVersionValue.ToString().c_str(), currVersion.ToString().c_str());

    const auto nextVersion = ProfileVersion(currVersion.GetMajor(), currVersion.GetMinor(), currVersion.GetSub1(), currVersion.GetSub2() + 1).ToString();
    importRequiresVersionCA->SetValue("ECDbRuntimeVersion", ECValue(nextVersion.c_str()));
    importRequiresVersionCA->GetValue(ecdbRuntimeVersionValue, "ECDbRuntimeVersion");
    ASSERT_STREQ(ecdbRuntimeVersionValue.ToString().c_str(), nextVersion.c_str());

    // Import the new schema "TestSchema" that references the older version "TestBaseSchema.1.0.0" that does not have an import restriction.
    const auto newSchemaStr = R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="TestBaseSchema" version="1.0.0" alias="tbs1"/>

            <ECEntityClass typeName="Plane" >
                <BaseClass>tbs1:Line</BaseClass>
                <ECProperty propertyName="Width" typeName="double" />
            </ECEntityClass>
        </ECSchema>)xml";

    TestIssueListener issueListener;
    m_ecdb.AddIssueListener(issueListener);
    EXPECT_EQ(SUCCESS, ImportSchema(SchemaItem(newSchemaStr)));

    EXPECT_TRUE(issueListener.IsEmpty()) << "No issues expected.";
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ProfileTestFixture, ApplyImportRequiresVersionToExistingSchema) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDbForCurrentTest());

    TestIssueListener issueListener;
    m_ecdb.AddIssueListener(issueListener);

    {
        SchemaItem schema(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema1" alias="s1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECEntityClass typeName="Foo" >
            <ECProperty propertyName="Length" typeName="double" />
        </ECEntityClass>
        </ECSchema>)xml");

        ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schema));
        ASSERT_TRUE(issueListener.IsEmpty());
    }

    {  // apply valid ImportRequiresVersion ca to existing schema
        SchemaItem schema(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema1" alias="s1" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.02" alias="ecdbmap"/>
        <ECCustomAttributes>
            <ImportRequiresVersion xmlns="ECDbMap.02.00.02">
                <ECDbRuntimeVersion>4.0.0.1</ECDbRuntimeVersion>
            </ImportRequiresVersion>
        </ECCustomAttributes>
        <ECEntityClass typeName="Foo" >
            <ECProperty propertyName="Length" typeName="double" />
        </ECEntityClass>
        </ECSchema>)xml");

        issueListener.ClearIssues();
        ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schema));

        ASSERT_TRUE(issueListener.IsEmpty());
    }

    {  // apply valid ImportRequiresVersion ca to existing schema which evaluates to false
        SchemaItem schema(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema1" alias="s1" version="1.0.3" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.02" alias="ecdbmap"/>
        <ECCustomAttributes>
            <ImportRequiresVersion xmlns="ECDbMap.02.00.02">
                <ECDbRuntimeVersion>999.9.9.9</ECDbRuntimeVersion>
            </ImportRequiresVersion>
        </ECCustomAttributes>
        <ECEntityClass typeName="Foo" >
            <ECProperty propertyName="Length" typeName="double" />
        </ECEntityClass>
        </ECSchema>)xml");

        issueListener.ClearIssues();
        ASSERT_EQ(BentleyStatus::ERROR, ImportSchema(schema));

        ASSERT_FALSE(issueListener.IsEmpty()) << "Should raise an issue.";
        ASSERT_STREQ(Utf8PrintfString("ECSchema Schema1.01.00.03 requires ECDb version 999.9.9.9, but the current runtime version is only %s.", m_ecdb.GetECDbProfileVersion().ToString().c_str()).c_str(), issueListener.GetLastMessage().c_str());
    }

    CloseECDb();
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ProfileTestFixture, UseRequiresVersionOnEntity) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDbForCurrentTest());

    {
        SchemaItem schema(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema1" alias="s1" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.02" alias="ecdbmap"/>
        <ECEntityClass typeName="Foo" >
            <ECCustomAttributes>
                <UseRequiresVersion xmlns="ECDbMap.02.00.02">
                    <ECDbRuntimeVersion>999.9.9.9</ECDbRuntimeVersion>
                </UseRequiresVersion>
            </ECCustomAttributes>
            <ECProperty propertyName="Length" typeName="double" />
        </ECEntityClass>
        </ECSchema>)xml");

        ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schema));

        TestIssueListener issueListener;
        m_ecdb.AddIssueListener(issueListener);

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT * from s1.Foo"));

        ASSERT_FALSE(issueListener.IsEmpty()) << "Should raise an issue.";
        ASSERT_STREQ("Invalid ECClass in ECSQL: Cannot use ECClass 'Schema1:Foo' because it requires a newer version of ECDb.", issueListener.GetLastMessage().c_str());
    }

    CloseECDb();
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ProfileTestFixture, TestUseRequiresVersionOnEntityPasses) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDbForCurrentTest());

    {  // same as before but validation should pass
        SchemaItem schema(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema1" alias="s1" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.02" alias="ecdbmap"/>
        <ECEntityClass typeName="Foo" >
            <ECCustomAttributes>
                <UseRequiresVersion xmlns="ECDbMap.02.00.02">
                    <ECDbRuntimeVersion>2.0.0.0</ECDbRuntimeVersion>
                    <ECSqlVersion>1.0.0.0</ECSqlVersion>
                </UseRequiresVersion>
            </ECCustomAttributes>
            <ECProperty propertyName="Length" typeName="double" />
        </ECEntityClass>
        </ECSchema>)xml");

        ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schema));

        TestIssueListener issueListener;
        m_ecdb.AddIssueListener(issueListener);

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * from s1.Foo"));

        ASSERT_TRUE(issueListener.IsEmpty()) << "Should not raise an issue.";
    }

    CloseECDb();
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ProfileTestFixture, UseRequiresVersionOnEntityInherited) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDbForCurrentTest());

    {
        SchemaItem schema(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema1" alias="s1" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.02" alias="ecdbmap"/>
        <ECEntityClass typeName="Foo">
            <ECCustomAttributes>
                <UseRequiresVersion xmlns="ECDbMap.02.00.02">
                    <ECSqlVersion>999.9.9.9</ECSqlVersion>
                </UseRequiresVersion>
            </ECCustomAttributes>
            <ECProperty propertyName="Length" typeName="double" />
        </ECEntityClass>
        <ECEntityClass typeName="Bar">
            <BaseClass>Foo</BaseClass>
        </ECEntityClass>
        </ECSchema>)xml");

        ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schema));

        TestIssueListener issueListener;
        m_ecdb.AddIssueListener(issueListener);

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT * from s1.Bar"));

        ASSERT_FALSE(issueListener.IsEmpty()) << "Should raise an issue.";
        ASSERT_STREQ("Invalid ECClass in ECSQL: Cannot use ECClass 'Schema1:Bar' because it requires a newer version of ECDb.", issueListener.GetLastMessage().c_str());
    }

    CloseECDb();
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ProfileTestFixture, UseRequiresVersionOnCA) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDbForCurrentTest());
    {
        SchemaItem schema(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema1" alias="s1" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.02" alias="ecdbmap"/>
        <ECCustomAttributeClass typeName="Bar" modifier="Sealed" appliesTo="EntityClass">
            <ECCustomAttributes>
                <UseRequiresVersion xmlns="ECDbMap.02.00.02">
                    <ECDbRuntimeVersion>999.9.9.9</ECDbRuntimeVersion>
                </UseRequiresVersion>
            </ECCustomAttributes>
        </ECCustomAttributeClass>
        <ECEntityClass typeName="Foo" >
            <ECCustomAttributes>
                <Bar></Bar>
            </ECCustomAttributes>
            <ECProperty propertyName="Length" typeName="double" />
        </ECEntityClass>
        </ECSchema>)xml");

        ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schema));

        TestIssueListener issueListener;
        m_ecdb.AddIssueListener(issueListener);

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT * from s1.Foo"));

        ASSERT_FALSE(issueListener.IsEmpty()) << "Should raise an issue.";
        ASSERT_STREQ("Invalid ECClass in ECSQL: Cannot use ECClass 'Schema1:Foo' because it requires a newer version of ECDb.", issueListener.GetLastMessage().c_str());
    }

    CloseECDb();
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ProfileTestFixture, UseRequiresVersionOnCAIndirect) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDbForCurrentTest());

    {  // use a custom attribute that uses a custom attribute on an entity
        SchemaItem schema(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema1" alias="s1" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.02" alias="ecdbmap"/>
        <ECCustomAttributeClass typeName="Foo" modifier="Sealed" appliesTo="CustomAttributeClass">
            <ECCustomAttributes>
                <UseRequiresVersion xmlns="ECDbMap.02.00.02">
                    <ECDbRuntimeVersion>999.9.9.9</ECDbRuntimeVersion>
                </UseRequiresVersion>
            </ECCustomAttributes>
        </ECCustomAttributeClass>

        <ECCustomAttributeClass typeName="Bar" modifier="Sealed" appliesTo="EntityClass">
            <ECCustomAttributes>
                <Foo></Foo>
            </ECCustomAttributes>
        </ECCustomAttributeClass>
        <ECEntityClass typeName="MyEntity" >
            <ECCustomAttributes>
                <Bar></Bar>
            </ECCustomAttributes>
            <ECProperty propertyName="Length" typeName="double" />
        </ECEntityClass>
        </ECSchema>)xml");

        ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schema));

        TestIssueListener issueListener;
        m_ecdb.AddIssueListener(issueListener);

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT * from s1.MyEntity"));

        ASSERT_FALSE(issueListener.IsEmpty()) << "Should raise an issue.";
        ASSERT_STREQ("Invalid ECClass in ECSQL: Cannot use ECClass 'Schema1:MyEntity' because it requires a newer version of ECDb.", issueListener.GetLastMessage().c_str());
    }

    CloseECDb();
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ProfileTestFixture, UseRequiresVersionOnCAIndirectInherited) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDbForCurrentTest());

    {  // use a entity which uses a struct class that has been flagged as
        SchemaItem schema(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema1" alias="s1" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.02" alias="ecdbmap"/>
        <ECCustomAttributeClass typeName="Foo" modifier="Sealed" appliesTo="CustomAttributeClass">
            <ECCustomAttributes>
                <UseRequiresVersion xmlns="ECDbMap.02.00.02">
                    <ECDbRuntimeVersion>999.9.9.9</ECDbRuntimeVersion>
                </UseRequiresVersion>
            </ECCustomAttributes>
        </ECCustomAttributeClass>

        <ECCustomAttributeClass typeName="Bar" modifier="Sealed" appliesTo="EntityClass">
            <ECCustomAttributes>
                <Foo></Foo>
            </ECCustomAttributes>
        </ECCustomAttributeClass>
        <ECEntityClass typeName="MyEntity" >
            <ECCustomAttributes>
                <Bar></Bar>
            </ECCustomAttributes>
            <ECProperty propertyName="Length" typeName="double" />
        </ECEntityClass>

        <ECEntityClass typeName="MySubclass" >
            <BaseClass>MyEntity</BaseClass>
        </ECEntityClass>
        </ECSchema>)xml");

        ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schema));

        TestIssueListener issueListener;
        m_ecdb.AddIssueListener(issueListener);

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT * from s1.MySubclass"));

        ASSERT_FALSE(issueListener.IsEmpty()) << "Should raise an issue.";
        ASSERT_STREQ("Invalid ECClass in ECSQL: Cannot use ECClass 'Schema1:MySubclass' because it requires a newer version of ECDb.", issueListener.GetLastMessage().c_str());
    }

    CloseECDb();
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ProfileTestFixture, ApplyUseRequiresVersionOnExistingCA) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDbForCurrentTest());

    TestIssueListener issueListener;
    m_ecdb.AddIssueListener(issueListener);

    SchemaItem schema(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema1" alias="s1" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.02" alias="ecdbmap"/>
        <ECCustomAttributeClass typeName="Foo" modifier="Sealed" appliesTo="CustomAttributeClass">
        </ECCustomAttributeClass>

        <ECCustomAttributeClass typeName="Bar" modifier="Sealed" appliesTo="EntityClass">
            <ECCustomAttributes>
                <Foo></Foo>
            </ECCustomAttributes>
        </ECCustomAttributeClass>
        <ECEntityClass typeName="MyEntity" >
            <ECCustomAttributes>
                <Bar></Bar>
            </ECCustomAttributes>
            <ECProperty propertyName="Length" typeName="double" />
        </ECEntityClass>

        <ECEntityClass typeName="MySubclass" >
            <BaseClass>MyEntity</BaseClass>
        </ECEntityClass>
        </ECSchema>)xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schema));
    {
        issueListener.ClearIssues();
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * from s1.MySubclass"));

        ASSERT_TRUE(issueListener.IsEmpty()) << "Should not raise an issue.";
    }

    SchemaItem schema2(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema1" alias="s1" version="1.0.2" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.02" alias="ecdbmap"/>
        <ECCustomAttributeClass typeName="Foo" modifier="Sealed" appliesTo="CustomAttributeClass">
            <ECCustomAttributes>
                <UseRequiresVersion xmlns="ECDbMap.02.00.02">
                    <ECDbRuntimeVersion>999.9.9.9</ECDbRuntimeVersion>
                </UseRequiresVersion>
            </ECCustomAttributes>
        </ECCustomAttributeClass>

        <ECCustomAttributeClass typeName="Bar" modifier="Sealed" appliesTo="EntityClass">
            <ECCustomAttributes>
                <Foo></Foo>
            </ECCustomAttributes>
        </ECCustomAttributeClass>
        <ECEntityClass typeName="MyEntity" >
            <ECCustomAttributes>
                <Bar></Bar>
            </ECCustomAttributes>
            <ECProperty propertyName="Length" typeName="double" />
        </ECEntityClass>

        <ECEntityClass typeName="MySubclass" >
            <BaseClass>MyEntity</BaseClass>
        </ECEntityClass>
        </ECSchema>)xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schema2));

    {
        issueListener.ClearIssues();
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT * from s1.MySubclass"));

        ASSERT_FALSE(issueListener.IsEmpty()) << "Should raise an issue.";
        ASSERT_STREQ("Invalid ECClass in ECSQL: Cannot use ECClass 'Schema1:MySubclass' because it requires a newer version of ECDb.", issueListener.GetLastMessage().c_str());
    }

    CloseECDb();
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ProfileTestFixture, TestUseRequiresVersionOnStruct) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDbForCurrentTest());

    {  // Apply an unsupported CA to a struct and struct property and then try to select (currently the CA on structs has no effect)
        SchemaItem schema(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema1" alias="s1" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.02" alias="ecdbmap"/>
        <ECCustomAttributeClass typeName="Foo" modifier="Sealed" appliesTo="Any">
            <ECCustomAttributes>
                <UseRequiresVersion xmlns="ECDbMap.02.00.02">
                    <ECDbRuntimeVersion>999.9.9.9</ECDbRuntimeVersion>
                </UseRequiresVersion>
            </ECCustomAttributes>
        </ECCustomAttributeClass>
        <ECStructClass typeName="MyStruct">
            <ECCustomAttributes>
                <Foo xmlns="Schema1.01.00.01" />
            </ECCustomAttributes>
            <ECProperty propertyName='Prop1' typeName='string' />
        </ECStructClass>
        <ECEntityClass typeName="MyEntity" >
            <ECStructProperty propertyName="StructProp" typeName="MyStruct">
            <ECCustomAttributes>
                <Foo xmlns="Schema1.01.00.01" />
            </ECCustomAttributes>
            </ECStructProperty>
        </ECEntityClass>
        </ECSchema>)xml");

        ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schema));

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * from s1.MyEntity"));
    }

    CloseECDb();
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ProfileTestFixture, TestUseRequiresVersionOnRelationship) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDbForCurrentTest());

    {  // Apply an unsupported CA to a struct and struct property and then try to select
        SchemaItem schema(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema1" alias="s1" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.02" alias="ecdbmap"/>
        <ECEntityClass typeName="MyEntity" >
        </ECEntityClass>

        <ECRelationshipClass typeName="MyEntityRefersToMyEntity" strength="referencing" modifier="None">
            <ECCustomAttributes>
                <UseRequiresVersion xmlns="ECDbMap.02.00.02">
                    <ECDbRuntimeVersion>999.9.9.9</ECDbRuntimeVersion>
                </UseRequiresVersion>
            </ECCustomAttributes>
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                <Class class="MyEntity"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="is referenced by" polymorphic="true">
                <Class class="MyEntity"/>
            </Target>
        </ECRelationshipClass>
        </ECSchema>)xml");

        ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schema));

        {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT * from s1.MyEntityRefersToMyEntity"));
        }
    }

    CloseECDb();
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ProfileTestFixture, TestUseRequiresVersionOnRelationshipIndirect) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDbForCurrentTest());

    {  // Apply an unsupported CA to a struct and struct property and then try to select
        SchemaItem schema(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema1" alias="s1" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.02" alias="ecdbmap"/>
        <ECCustomAttributeClass typeName="Foo" modifier="Sealed" appliesTo="Any">
            <ECCustomAttributes>
                <UseRequiresVersion xmlns="ECDbMap.02.00.02">
                    <ECDbRuntimeVersion>999.9.9.9</ECDbRuntimeVersion>
                </UseRequiresVersion>
            </ECCustomAttributes>
        </ECCustomAttributeClass>
        <ECEntityClass typeName="MyEntity" >
        </ECEntityClass>

        <ECRelationshipClass typeName="MyEntityRefersToMyEntity" strength="referencing" modifier="None">
            <ECCustomAttributes>
                <Foo xmlns="Schema1.01.00.01" />
            </ECCustomAttributes>
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                <Class class="MyEntity"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="is referenced by" polymorphic="true">
                <Class class="MyEntity"/>
            </Target>
        </ECRelationshipClass>
        </ECSchema>)xml");

        ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schema));

        {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT * from s1.MyEntityRefersToMyEntity"));
        }
    }

    CloseECDb();
}

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ProfileTestFixture, TestUseRequiresVersionOnRelationshipConstraint) {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDbForCurrentTest());

    {  // Apply an unsupported CA to a struct and struct property and then try to select (currently the CA on structs has no effect)
        SchemaItem schema(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema1" alias="s1" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.02" alias="ecdbmap"/>
        <ECCustomAttributeClass typeName="Foo" modifier="Sealed" appliesTo="Any">
            <ECCustomAttributes>
                <UseRequiresVersion xmlns="ECDbMap.02.00.02">
                    <ECDbRuntimeVersion>999.9.9.9</ECDbRuntimeVersion>
                </UseRequiresVersion>
            </ECCustomAttributes>
        </ECCustomAttributeClass>
        <ECEntityClass typeName="MyEntity" >
            <ECCustomAttributes>
                <Foo xmlns="Schema1.01.00.01" />
            </ECCustomAttributes>
        </ECEntityClass>

        <ECRelationshipClass typeName="MyEntityRefersToMyEntity" strength="referencing" modifier="None">
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                <Class class="MyEntity"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="is referenced by" polymorphic="true">
                <Class class="MyEntity"/>
            </Target>
        </ECRelationshipClass>
        </ECSchema>)xml");

        ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schema));

        {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * from s1.MyEntityRefersToMyEntity"));
        }

        {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT * from s1.MyEntity"));
        }
    }
    CloseECDb();
}

END_ECDBUNITTESTS_NAMESPACE
