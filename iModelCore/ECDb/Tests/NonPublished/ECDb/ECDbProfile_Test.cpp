/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/ECDb/ECDbProfile_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../PublicApi/NonPublished/ECDb/ECDbTestProject.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

static const SchemaVersion EXPECTED_PROFILEVERSION (2, 0, 0, 0);

static const PropertySpec PROFILEVERSION_PROPSPEC ("SchemaVersion", "ec_Db");

static Utf8CP const PROFILE_TABLE = "ec_Schema";
static Utf8CP const ECINSTANCEIDSEQUENCE_KEY = "ec_ecinstanceidsequence";

//helper functions (find impls at bottom)
Utf8String CopyTestFile (Utf8CP fileName);
void AssertIsProfile1_0_File (Utf8CP ecdbPath);
void AssertIsProfile1_0_File (Db& ecdb, Utf8CP ecdbPath);
void AssertProfileUpgrade (Utf8CP ecdbPath, Db::OpenParams const& openParams, bool isExpectedToUpgrade, SchemaVersion const* expectedProfileVersion = nullptr);

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbProfile, CreationTest)
    {
    ECDbTestProject::Initialize ();

    Utf8String dbPath = ECDbTestProject::BuildECDbPath ("ecdbprofiletest.db");
    WString dbPathW;
    BeStringUtilities::Utf8ToWChar (dbPathW, dbPath.c_str ());
    if (BeFileName::DoesPathExist (dbPathW.c_str ()))
        {
        // Delete any previously created file
        BeFileNameStatus fileDeleteStatus = BeFileName::BeDeleteFile (dbPathW.c_str ());
        ASSERT_TRUE (fileDeleteStatus == BeFileNameStatus::Success);
        }


    //first create an SQLite file with basic profile only (no ECDb)
        {
        Db db;
        DbResult stat = db.CreateNewDb (dbPath.c_str ());
        EXPECT_EQ (BE_SQLITE_OK, stat) << L"Creating BeSQLite file failed";

        EXPECT_FALSE (db.TableExists (PROFILE_TABLE)) << "BeSQLite file is not expected to contain tables of the EC profile";

        Utf8String profileVersion;
        EXPECT_EQ  (BE_SQLITE_ERROR, db.QueryProperty (profileVersion, PROFILEVERSION_PROPSPEC)) << L"BeSQLite file is not expected to contain the ECDb profile version.";

        size_t sequenceIndex = 0;
        ASSERT_FALSE (db.GetRLVCache().TryGetIndex(sequenceIndex, ECINSTANCEIDSEQUENCE_KEY));
        db.CloseDb ();
        }

    //now create an ECDb file
        {
        ECDbTestProject testProject;
        ECDbR ecdb = testProject.Create ("ecdbprofiletest.ecdb");

        EXPECT_TRUE (ecdb.TableExists (PROFILE_TABLE)) << "ECDb profile table not found in ECDb file which was newly created.";

        Utf8String actualProfileVersionStr;
        EXPECT_EQ  (BE_SQLITE_ROW, ecdb.QueryProperty (actualProfileVersionStr, PROFILEVERSION_PROPSPEC)) << L"ECDb file is expected to contain an entry for the ECDb profile version in be_prop.";
        SchemaVersion actualProfileVersion (actualProfileVersionStr.c_str ());
        EXPECT_TRUE (EXPECTED_PROFILEVERSION == actualProfileVersion) << "Unexpected ECDb profile version of new ECDb file.";

        size_t sequenceIndex = 0;
        ASSERT_TRUE (ecdb.GetRLVCache().TryGetIndex (sequenceIndex, ECINSTANCEIDSEQUENCE_KEY));

        int64_t lastECInstanceId = -1LL;
        EXPECT_EQ (BE_SQLITE_OK, ecdb.GetRLVCache().QueryValue(lastECInstanceId, sequenceIndex)) << L"ECInstanceId sequence not found in ECDb file which was newly created";
        }
    }
//---------------------------------------------------------------------------------------
// Test to verify TFS 107173: ECDb profile creation should fail if it exists
// @bsimethod                                     Majd.Uddin                  07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbProfile, FailIfAlreadyCreated)
{
    ECDbTestProject testProject;
    ECDbR ecdb = testProject.Create("ecdbprofiletest2.ecdb");

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
    stat = ecdb.CreateNewDb(ecdbFilePathUtf8.c_str(), BeDbGuid(), params);
    EXPECT_EQ(BE_SQLITE_ERROR, stat);
}

void RunUpgradeTest (Db::OpenParams const& openParams);

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbProfile, UpgradeReadonlyFileTest)
    {
    Db::OpenParams openParams (Db::OpenMode::Readonly);
    RunUpgradeTest (openParams);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbProfile, UpgradeReadwriteFileTest)
    {
    Db::OpenParams openParams (Db::OpenMode::ReadWrite);
    RunUpgradeTest (openParams);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/12
//+---------------+---------------+---------------+---------------+---------------+------
void RunUpgradeTest (Db::OpenParams const& openParams)
    {
    ECDbTestProject::Initialize ();

    Utf8String emptyECDb1_0Path = CopyTestFile ("profile1_0_empty.ecdb");
    Utf8String nonemptyECDb1_0Path = CopyTestFile ("profile1_0_nonempty.ecdb");

    //Before upgrading check that 1.0 files really are 1.0 files
    AssertIsProfile1_0_File (emptyECDb1_0Path.c_str ());
    AssertIsProfile1_0_File (nonemptyECDb1_0Path.c_str ());

    // OpenParams get modified during upgrade. So do not reuse them for two different files
    Db::OpenParams tempOpenParams (openParams);
    AssertProfileUpgrade (emptyECDb1_0Path.c_str (), tempOpenParams, false);
    
    tempOpenParams = openParams;
    AssertProfileUpgrade (nonemptyECDb1_0Path.c_str (), openParams, false);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbProfile, OpenNonECDbFileTest)
    {
    ECDbTestProject::Initialize ();

    Utf8String dbPath = ECDbTestProject::BuildECDbPath ("noecdbprofile.db");

        {
        WString dbPathW;
        BeStringUtilities::Utf8ToWChar (dbPathW, dbPath.c_str ());
        if (BeFileName::DoesPathExist (dbPathW.c_str ()))
            {
            // Delete any previously created file
            BeFileNameStatus fileDeleteStatus = BeFileName::BeDeleteFile (dbPathW.c_str ());
            ASSERT_TRUE (fileDeleteStatus == BeFileNameStatus::Success);
            }

        //create a non-ECDb file and close it again
        Db noecDb;
        DbResult stat = noecDb.CreateNewDb (dbPath.c_str ());
        EXPECT_EQ (BE_SQLITE_OK, stat) << L"Creating SQLite file without ECDb profile failed.";
        noecDb.CloseDb ();
        }

    //Now open the non-ECDb file with ECDb API. This should NOT upgrade it to an ECDb file!
    BeTest::SetFailOnAssert (false);
        {
        ECDb ecdb;
        DbResult stat = ecdb.OpenBeSQLiteDb (dbPath.c_str (), Db::OpenParams(Db::OpenMode::ReadWrite));
        EXPECT_EQ (BE_SQLITE_ERROR_InvalidProfileVersion, stat) << L"Opening SQLite file without ECDb profile from ECDb instance failed.";
        }
    BeTest::SetFailOnAssert (true);

    //check that the file is still no ECDb file
        {
        Db noecDb;
        ASSERT_EQ (BE_SQLITE_OK, noecDb.OpenBeSQLiteDb (dbPath.c_str (), Db::OpenParams (Db::OpenMode::Readonly)));
        ASSERT_FALSE  (noecDb.HasProperty (PROFILEVERSION_PROPSPEC)) << L"Non-ECDb file after an attempt to open it with ECDb API is not expected to have become an ECDb file.";
        ASSERT_FALSE  (noecDb.TableExists (PROFILE_TABLE)) << L"Non-ECDb file after an attempt to open it with ECDb API is not expected to have become an ECDb file.";
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethods                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
void AssertProfileUpgrade (Utf8CP ecdbPath, Db::OpenParams const& openParams, bool isExpectedToUpgrade, SchemaVersion const* expectedProfileVersion)
    {
    ECDb ecdb;
    const auto openStat = ecdb.OpenBeSQLiteDb (ecdbPath, openParams);
    if (isExpectedToUpgrade)
        {
        ASSERT_EQ (BE_SQLITE_OK, openStat) << "Opening (and upgrading) ECDb file '" << ecdbPath << "' failed unexpectedly.";

        Utf8String actualProfileVersionStr;
        ASSERT_EQ (BE_SQLITE_ROW, ecdb.QueryProperty (actualProfileVersionStr, PROFILEVERSION_PROPSPEC)) << "ECDb file '" << ecdbPath << "' is expected to contain an entry for the ECDb profile version in be_prop.";
        SchemaVersion actualProfileVersion (actualProfileVersionStr.c_str ());
        if (expectedProfileVersion != nullptr)
            ASSERT_TRUE ((*expectedProfileVersion) == actualProfileVersion) << "ECDb file '" << ecdbPath << "' has unexpected version of ECDb profile. Expected: " << expectedProfileVersion->ToJson ().c_str () << " Actual: " << actualProfileVersionStr.c_str ();
        }
    else
        ASSERT_EQ (BE_SQLITE_ERROR_ProfileTooOld, openStat) << "Opening ECDb file '" << ecdbPath << "' succeeded unexpectedly.";
    }

//---------------------------------------------------------------------------------------
// @bsimethods                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
void AssertIsProfile1_0_File (Utf8CP ecdbPath)
    {
    //Using class Db ensures that the ECDb profile upgrade does not take place
    Db ecdb;
    AssertIsProfile1_0_File (ecdb, ecdbPath);
    }

//---------------------------------------------------------------------------------------
// @bsimethods                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
void AssertIsProfile1_0_File (Db& ecdb, Utf8CP ecdbPath)
    {
    //Using class Db ensures that the ECDb profile upgrade does not take place
    ASSERT_EQ (BE_SQLITE_OK, ecdb.OpenBeSQLiteDb (ecdbPath, Db::OpenParams (Db::OpenMode::Readonly)));
    ASSERT_FALSE  (ecdb.HasProperty (PROFILEVERSION_PROPSPEC)) << "ECDb file '" << ecdbPath << "' is expected to have profile 1.0. So it not expected to have a profile version entry in be_prop.";
    ASSERT_TRUE (ecdb.TableExists (PROFILE_TABLE)) << "ECDb file '" << ecdbPath << "' is expected to have profile 1.0. So it expected to contain a table from the ECDb profile.";;
    }


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbProfile, VerifyNoAutoUpgradeForLegacyFilesWithoutCGColumns)
    {
    ECDbTestProject::Initialize ();

    Utf8String legacyECDbPath = CopyTestFile ("legacyfilewithoutcommongeometrycolumns.ecdb");
    ECDb legacyECDb;
    ASSERT_EQ (BE_SQLITE_ERROR_ProfileTooOld, legacyECDb.OpenBeSQLiteDb (legacyECDbPath.c_str (), ECDb::OpenParams (ECDb::OpenMode::Readonly))) << "Opening legacy ECDb file '" << legacyECDbPath.c_str () << "' succeeded unexpectedly.";
    }

//---------------------------------------------------------------------------------------
// @bsimethods                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String CopyTestFile (Utf8CP fileName)
    {
    WString fileNameW (fileName, BentleyCharEncoding::Utf8);

    BeFileName sourceDir;
    BeTest::GetHost ().GetDocumentsRoot (sourceDir);
    sourceDir.AppendToPath (L"DgnDb");
    sourceDir.AppendToPath (L"ECDb");

    BeFileName targetDir;
    BeTest::GetHost ().GetOutputRoot (targetDir);

    BeFileName sourcePath (nullptr, sourceDir.GetName (), fileNameW.c_str (), nullptr);
    BeFileName targetPath (nullptr, targetDir.GetName (), fileNameW.c_str (), nullptr);

    if (targetPath.DoesPathExist ())
        targetPath.BeDeleteFile ();

    if (BeFileNameStatus::Success != BeFileName::BeCopyFile (sourcePath, targetPath, false))
        return nullptr;
    
    return Utf8String (targetPath.GetNameUtf8 ());
    }


END_ECDBUNITTESTS_NAMESPACE
