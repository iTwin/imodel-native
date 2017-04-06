/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ProfileTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

static const ProfileVersion EXPECTED_PROFILEVERSION (3, 109, 0, 0);

static const PropertySpec PROFILEVERSION_PROPSPEC ("SchemaVersion", "ec_Db");

static Utf8CP const PROFILE_TABLE = "ec_Schema";
static Utf8CP const ECINSTANCEIDSEQUENCE_KEY = "ec_ecinstanceidsequence";

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
    EXPECT_EQ(BE_SQLITE_ERROR, db.QueryProperty(profileVersion, PROFILEVERSION_PROPSPEC)) << L"BeSQLite file is not expected to contain the ECDb profile version.";

    size_t sequenceIndex = 0;
    ASSERT_FALSE(db.GetBLVCache().TryGetIndex(sequenceIndex, ECINSTANCEIDSEQUENCE_KEY));
    db.CloseDb();
    }

    //now create an ECDb file
    {
    ECDbR ecdb = SetupECDb("ecdbprofiletest.ecdb");

    EXPECT_TRUE(ecdb.TableExists(PROFILE_TABLE)) << "ECDb profile table not found in ECDb file which was newly created.";

    Utf8String actualProfileVersionStr;
    EXPECT_EQ(BE_SQLITE_ROW, ecdb.QueryProperty(actualProfileVersionStr, PROFILEVERSION_PROPSPEC)) << L"ECDb file is expected to contain an entry for the ECDb profile version in be_prop.";
    ProfileVersion actualProfileVersion(actualProfileVersionStr.c_str());
    EXPECT_TRUE(EXPECTED_PROFILEVERSION == actualProfileVersion) << "Unexpected ECDb profile version of new ECDb file. Actual version: " << actualProfileVersionStr.c_str();

    size_t sequenceIndex = 0;
    ASSERT_TRUE(ecdb.GetBLVCache().TryGetIndex(sequenceIndex, ECINSTANCEIDSEQUENCE_KEY));

    uint64_t lastECInstanceId = -1LL;
    EXPECT_EQ(BE_SQLITE_OK, ecdb.GetBLVCache().QueryValue(lastECInstanceId, sequenceIndex)) << L"ECInstanceId sequence not found in ECDb file which was newly created";
    }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbTestFixture, ProfileSchemas)
    {
    ECDbCR ecdb = SetupECDb("empty.ecdb");

    ECSchemaCP systemSchema = ecdb.Schemas().GetSchema("ECDbSystem");
    ASSERT_TRUE(systemSchema != nullptr);

    //Terminology of system/standard schemas is not clear yet for the EC3 world. Right now, the profile schemas are neither of that.
    ASSERT_FALSE(systemSchema->IsSystemSchema());

    ECSchemaCP fileInfoSchema = ecdb.Schemas().GetSchema("ECDbFileInfo");
    ASSERT_TRUE(fileInfoSchema != nullptr);

    ASSERT_FALSE(fileInfoSchema->IsSystemSchema());
    }

//---------------------------------------------------------------------------------------
// Test to verify TFS 107173: ECDb profile creation should fail if it exists
// @bsimethod                                     Majd.Uddin                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbTestFixture, CreateProfileFailsIfAlreadyCreated)
    {
    ECDbR ecdb = SetupECDb("ecdbprofiletest2.ecdb");

    EXPECT_TRUE(ecdb.TableExists(PROFILE_TABLE)) << "ECDb profile table not found in ECDb file which was newly created.";

    //Drop a few tables
    EXPECT_EQ(BE_SQLITE_OK, ecdb.DropTable("ec_ClassMap"));
    EXPECT_EQ(BE_SQLITE_OK, ecdb.DropTable("be_Local"));
    ecdb.CloseDb();

    WString ecdbFileNameW("ecdbprofiletest2.ecdb", BentleyCharEncoding::Utf8);
    BeFileName ecdbFilePath;
    BeTest::GetHost().GetOutputRoot(ecdbFilePath);
    ecdbFilePath.AppendToPath(ecdbFileNameW.c_str());
    Utf8String ecdbFilePathUtf8 = ecdbFilePath.GetNameUtf8();

    //create the Db again, it should fail at already existing
    DbResult stat = ecdb.CreateNewDb(ecdbFilePathUtf8.c_str());
    EXPECT_EQ(BE_SQLITE_ERROR_FileExists, stat);

    //create the Db again with SetFailIfDbExist set to false i.e. force re-creation
    BeSQLite::Db::CreateParams params;
    params.SetFailIfDbExist(false);
    stat = ecdb.CreateNewDb(ecdbFilePathUtf8.c_str(), BeSQLite::BeGuid(true), params);
    EXPECT_EQ(BE_SQLITE_ERROR, stat);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  04/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbTestFixture, ProfileUpgrade)
    {
    auto cloneTestFile = [] (BeFileNameR clonedFilePath, BeFileNameCR sourceFilePath)
        {
        if (!sourceFilePath.DoesPathExist())
            return BeFileNameStatus::FileNotFound;

        BeTest::GetHost().GetOutputRoot(clonedFilePath);
        clonedFilePath.AppendToPath(sourceFilePath.GetFileNameAndExtension().c_str());

        if (clonedFilePath.DoesPathExist())
            {
            BeFileNameStatus stat = clonedFilePath.BeDeleteFile();
            if (BeFileNameStatus::Success != stat)
                return stat;
            }

        return BeFileName::BeCopyFile(sourceFilePath, clonedFilePath);
        };

    auto openParamsToString = [] (ECDb::OpenParams const& params)
        {
        Utf8CP openModeStr = nullptr;
        switch (params.m_openMode)
            {
                case Db::OpenMode::Create:
                    openModeStr = "Create";
                    break;
                case Db::OpenMode::Readonly:
                    openModeStr = "Readonly";
                    break;
                case Db::OpenMode::ReadWrite:
                    openModeStr = "ReadWrite";
                    break;
                case Db::OpenMode::SharedCache:
                    openModeStr = "SharedCache";
                    break;
                default:
                    BeAssert(false);
                    return Utf8String();
            }

        Utf8CP defaultTxnModeStr = nullptr;
        switch (params.m_startDefaultTxn)
            {
                case DefaultTxn::Exclusive:
                    defaultTxnModeStr = "Exclusive";
                    break;
                case DefaultTxn::Immediate:
                    defaultTxnModeStr = "Immediate";
                    break;
                case DefaultTxn::No:
                    defaultTxnModeStr = "No";
                    break;
                case DefaultTxn::Yes:
                    defaultTxnModeStr = "Yes";
                    break;
                default:
                    BeAssert(false);
                    BeAssert(false);
                    return Utf8String();
            }

        Utf8String str;
        str.Sprintf("OpenMode: %s - DefaultTransactionMode: %s", openModeStr, defaultTxnModeStr);
        return str;
        };

    BeFileName q2testFilePath;
    BeTest::GetHost().GetDocumentsRoot(q2testFilePath);
    q2testFilePath.AppendToPath(L"ECDb");
    q2testFilePath.AppendToPath(WString("testq2.ecdb", BentleyCharEncoding::Utf8).c_str());

    const std::vector<ECDb::OpenMode> openModes {ECDb::OpenMode::Readonly, ECDb::OpenMode::ReadWrite};
    const std::vector<DefaultTxn> defaultTxnModes {DefaultTxn::No, DefaultTxn::Yes, DefaultTxn::Exclusive, DefaultTxn::Immediate};

    //Q2 file
    for (ECDb::OpenMode openMode : openModes)
        {
        for (DefaultTxn defaultTxnMode : defaultTxnModes)
            {
            if (openMode == ECDb::OpenMode::Readonly)
                {
                if (defaultTxnMode == DefaultTxn::Exclusive || defaultTxnMode == DefaultTxn::Immediate)
                    continue; //this combination is not valid
                }

            BeFileName clonedTestFilePath;
            ASSERT_EQ(BeFileNameStatus::Success, cloneTestFile(clonedTestFilePath, q2testFilePath)) << Utf8String(clonedTestFilePath.GetFileNameAndExtension()).c_str();

            ECDb ecdb;
            ECDb::OpenParams openParams(openMode, defaultTxnMode);
            const DbResult stat = ecdb.OpenBeSQLiteDb(clonedTestFilePath, ECDb::OpenParams(openMode, defaultTxnMode));
            EXPECT_EQ(BE_SQLITE_OK, stat) << ECDb::InterpretDbResult(stat) << " - " << openParamsToString(openParams).c_str() << " - " << Utf8String(clonedTestFilePath.GetFileNameAndExtension()).c_str();
            }
        }

    //Up-to-date file
    BeFileName uptodateFilePath;
    BeTest::GetHost().GetOutputRoot(uptodateFilePath);
    uptodateFilePath.AppendToPath(WString("uptodateprofiletest.ecdb", BentleyCharEncoding::Utf8).c_str());

    for (ECDb::OpenMode openMode : openModes)
        {
        for (DefaultTxn defaultTxnMode : defaultTxnModes)
            {
            if (openMode == ECDb::OpenMode::Readonly)
                {
                if (defaultTxnMode == DefaultTxn::Exclusive || defaultTxnMode == DefaultTxn::Immediate)
                    continue; //this combination is not valid
                }

            {
            ECDb ecdb;
            ASSERT_EQ(BE_SQLITE_OK, ecdb.CreateNewDb(uptodateFilePath));
            ASSERT_EQ(BE_SQLITE_OK, ecdb.SaveChanges());
            ecdb.CloseDb();
            }

            ECDb ecdb;
            ECDb::OpenParams openParams(openMode, defaultTxnMode);
            const DbResult stat = ecdb.OpenBeSQLiteDb(uptodateFilePath, ECDb::OpenParams(openMode, defaultTxnMode));
            EXPECT_EQ(BE_SQLITE_OK, stat) << ECDb::InterpretDbResult(stat) << " - " << openParamsToString(openParams).c_str() << " - " << Utf8String(uptodateFilePath.GetFileNameAndExtension()).c_str();

            ecdb.CloseDb();
            ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeDeleteFile(uptodateFilePath));
            }
        }
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
    ECDbR ecdb = SetupECDb("ecdbprofiletest.ecdb");

    std::vector<std::tuple<ProfileVersion, Db::OpenMode, DbResult, bool>> testVersions {
            {ProfileVersion(3,6,99,0), Db::OpenMode::Readonly, BE_SQLITE_ERROR_ProfileTooOld, false},
            {ProfileVersion(3,6,99,0), Db::OpenMode::ReadWrite, BE_SQLITE_ERROR_ProfileTooOld, false},
            {ProfileVersion(3,7,0,0), Db::OpenMode::Readonly, BE_SQLITE_ERROR_ProfileTooOld, false},
            {ProfileVersion(3,7,0,0), Db::OpenMode::ReadWrite, BE_SQLITE_ERROR_ProfileTooOld, false},
            {ProfileVersion(3,7,0,1), Db::OpenMode::Readonly, BE_SQLITE_ERROR_ProfileTooOld, false},
            {ProfileVersion(3,7,0,1), Db::OpenMode::ReadWrite, BE_SQLITE_ERROR_ProfileTooOld, false},
            {ProfileVersion(3,7,3,1), Db::OpenMode::Readonly, BE_SQLITE_ERROR_ProfileTooOld, false},
            {ProfileVersion(3,7,3,1), Db::OpenMode::ReadWrite, BE_SQLITE_ERROR_ProfileTooOld, false},
            {ProfileVersion(3,7,3,2), Db::OpenMode::Readonly, BE_SQLITE_ERROR_ProfileTooOld, false},
            {ProfileVersion(3,7,3,2), Db::OpenMode::ReadWrite, BE_SQLITE_ERROR_ProfileTooOld, false},
            {ProfileVersion(3,7,3,3), Db::OpenMode::Readonly, BE_SQLITE_ERROR_ProfileTooOld, false},
            {ProfileVersion(3,7,3,3), Db::OpenMode::ReadWrite, BE_SQLITE_ERROR_ProfileTooOld, false},
            {ProfileVersion(3,7,4,3), Db::OpenMode::Readonly, BE_SQLITE_ERROR_ProfileTooOld, false},
            {ProfileVersion(3,7,4,3), Db::OpenMode::ReadWrite, BE_SQLITE_ERROR_ProfileTooOld, false},
            {ProfileVersion(3,100,0,0), Db::OpenMode::Readonly, BE_SQLITE_ERROR_ProfileTooOld, false},
            {ProfileVersion(3,100,0,0), Db::OpenMode::ReadWrite, BE_SQLITE_ERROR_ProfileTooOld, false},
            {ProfileVersion(3,100,0,1), Db::OpenMode::Readonly, BE_SQLITE_ERROR_ProfileTooOld, false},
            {ProfileVersion(3,100,0,1), Db::OpenMode::ReadWrite, BE_SQLITE_ERROR_ProfileTooOld, false},
            {ProfileVersion(3,109,0,0), Db::OpenMode::Readonly, BE_SQLITE_OK, false},
            {ProfileVersion(3,109,0,0), Db::OpenMode::ReadWrite, BE_SQLITE_OK, false},
            {ProfileVersion(3,109,0,1), Db::OpenMode::Readonly, BE_SQLITE_OK, false},
            {ProfileVersion(3,109,0,1), Db::OpenMode::ReadWrite, BE_SQLITE_OK, false},
            {ProfileVersion(3,109,1,0), Db::OpenMode::Readonly, BE_SQLITE_OK, false},
            {ProfileVersion(3,109,1,0), Db::OpenMode::ReadWrite, BE_SQLITE_ERROR_ProfileTooNewForReadWrite, false},
            {ProfileVersion(3,110,0,0), Db::OpenMode::Readonly, BE_SQLITE_ERROR_ProfileTooNew, false},
            {ProfileVersion(3,110,0,0), Db::OpenMode::ReadWrite, BE_SQLITE_ERROR_ProfileTooNew, false},
            {ProfileVersion(4,0,0,0), Db::OpenMode::Readonly, BE_SQLITE_ERROR_ProfileTooNew, false},
            {ProfileVersion(4,0,0,0), Db::OpenMode::ReadWrite, BE_SQLITE_ERROR_ProfileTooNew, false}
        };

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(ecdb, "UPDATE be_Prop SET StrData=? WHERE Namespace='ec_Db' AND Name='SchemaVersion'"));
    for (std::tuple<ProfileVersion, Db::OpenMode, DbResult, bool> const& testVersion : testVersions)
        {
        Utf8String schemaVersionJson = std::get<0>(testVersion).ToJson();
        ASSERT_EQ(BE_SQLITE_OK, stmt.BindText(1, schemaVersionJson, Statement::MakeCopy::Yes));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << schemaVersionJson.c_str();
        ASSERT_EQ(1, ecdb.GetModifiedRowCount()) << schemaVersionJson.c_str();
        stmt.Reset();
        stmt.ClearBindings();

        Db::OpenMode openMode = std::get<1>(testVersion);
        DbResult expectedResult = std::get<2>(testVersion);
        bool expectedNeedsUpgrade = std::get<3>(testVersion);
        
        bool actualNeedsUpgrade = false;
        ASSERT_EQ(expectedResult, ecdb.CheckECDbProfileVersion(actualNeedsUpgrade, openMode == Db::OpenMode::Readonly)) << schemaVersionJson.c_str() << " OpenMode readonly:" << (openMode == Db::OpenMode::Readonly ? "yes" : "no");
        ASSERT_EQ(expectedNeedsUpgrade, actualNeedsUpgrade) << schemaVersionJson.c_str() << " OpenMode readonly:" << (openMode == Db::OpenMode::Readonly ? "yes" : "no");
        }

    ecdb.AbandonChanges();
    }

END_ECDBUNITTESTS_NAMESPACE
