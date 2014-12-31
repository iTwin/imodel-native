/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/ECDB/NonPublished/ECDb/ECDbProfile_Test.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitTests/NonPublished/ECDb/ECDbTestProject.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

static const SchemaVersion EXPECTED_PROFILEVERSION (1, 0, 0, 4);

static const PropertySpec PROFILEVERSION_PROPSPEC ("SchemaVersion", "ec_Db");

static Utf8CP const PROFILE_TABLE = "ec_Schema";
static Utf8CP const ECINSTANCEIDSEQUENCE_KEY = "ec_ecidsequence";

//helper functions (find impls at bottom)
Utf8String CopyTestFile (Utf8CP fileName);
void AssertIsProfile1_0_File (Utf8CP ecdbPath);
void AssertIsProfile1_0_File (Db& ecdb, Utf8CP ecdbPath);
void AssertIsProfileUpToDate (ECDbCR ecdb);
void AssertProfileUpgrade (Utf8CP ecdbPath, Db::OpenParams const& openParams, SchemaVersion const& expectedProfileVersion);
void AssertProfileUpgrade (ECDbR ecdb, Utf8CP ecdbPath, Db::OpenParams const& openParams, SchemaVersion const& expectedProfileVersion);

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
        ASSERT_FALSE (db.TryGetRepositoryLocalValueIndex (sequenceIndex, ECINSTANCEIDSEQUENCE_KEY));
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
        ASSERT_TRUE (ecdb.TryGetRepositoryLocalValueIndex (sequenceIndex, ECINSTANCEIDSEQUENCE_KEY));

        int64_t lastECInstanceId = -1LL;
        EXPECT_EQ (BE_SQLITE_OK, ecdb.QueryRepositoryLocalValue (lastECInstanceId, sequenceIndex)) << L"ECInstanceId sequence not found in ECDb file which was newly created";

        AssertIsProfileUpToDate (ecdb);
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
    Db::OpenParams openParams (Db::OPEN_Readonly);
    RunUpgradeTest (openParams);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbProfile, UpgradeReadwriteFileTest)
    {
    Db::OpenParams openParams (Db::OPEN_ReadWrite);
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
    AssertProfileUpgrade (emptyECDb1_0Path.c_str (), tempOpenParams, EXPECTED_PROFILEVERSION);
    
    tempOpenParams = openParams;
    AssertProfileUpgrade (nonemptyECDb1_0Path.c_str (), openParams, EXPECTED_PROFILEVERSION);
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
        DbResult stat = ecdb.OpenBeSQLiteDb (dbPath.c_str (), Db::OpenParams(Db::OPEN_ReadWrite, DefaultTxn_Yes));
        EXPECT_EQ (BE_SQLITE_ERROR_InvalidProfileVersion, stat) << L"Opening SQLite file without ECDb profile from ECDb instance failed.";
        }
    BeTest::SetFailOnAssert (true);

    //check that the file is still no ECDb file
        {
        Db noecDb;
        ASSERT_EQ (BE_SQLITE_OK, noecDb.OpenBeSQLiteDb (dbPath.c_str (), Db::OpenParams (Db::OPEN_Readonly)));
        ASSERT_FALSE  (noecDb.HasProperty (PROFILEVERSION_PROPSPEC)) << L"Non-ECDb file after an attempt to open it with ECDb API is not expected to have become an ECDb file.";
        ASSERT_FALSE  (noecDb.TableExists (PROFILE_TABLE)) << L"Non-ECDb file after an attempt to open it with ECDb API is not expected to have become an ECDb file.";
        }
    }

//---------------------------------------------------------------------------------------
// Note: This tests operates against large ECDb / DgnDb files which are not checked into the
// test data repository.
// The test expects the following files in DgnPlatformTest/DgnDbUnitTests/ECDb
// - openplant3d_profile1_0_empty.ecdb An ECDb v. 1.0 file into which the OpenPlant_3D.01.02 ECSchema
//                                     was imported, but is empty otherwise.
// - 81_SiteLayout.i.idgndb An iDgnDb file with ECDb profile 1.0 created from the v8i i-model 81_SiteLayout.i.dgn
//                          (using the publisher from graphite03)
//
// @bsiclass                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST(PerformanceECDbProfile, CreateVersusUpgradeECDbFile)
    {
    ECDbTestProject::Initialize ();

    // Create an empty ECDb file with the same schema used for the upgrade profile tests
        {
        ECDbTestProject project;
        printf ("Creating empty file with OpenPlant 3D schema. Attach to profiler and press any key..."); getchar ();
        project.Create ("openplant3d_empty.ecdb", L"OpenPlant_3D.01.02.ecschema.xml", false);
        }
    printf ("Detach from profiler and press any key..."); getchar ();

    // Upgrade idgndb file with ECDb profile v1.0 (generated by graphite03 publisher) with same large schema used above
        {
        auto dgnDbPath = CopyTestFile ("81_SiteLayout.i.idgndb");
        printf ("Upgrading 81_SiteLayout.i.idgndb. Attach to profiler and press any key..."); getchar ();
        ECDb ecdb;
        ASSERT_EQ (BE_SQLITE_OK, ecdb.OpenBeSQLiteDb (dgnDbPath.c_str (), Db::OpenParams (Db::OPEN_Readonly)));
        }
    printf ("Detach from profiler and press any key..."); getchar ();

    // Upgrade v1.0 ECDb files that doesn't contain data, but just the imported large schema
        {
        auto ecdbPath = CopyTestFile ("openplant3d_profile1_0_empty.ecdb");

        printf ("Upgrading openplant3d_profile1_0_empty.ecdb. Attach to profiler and press any key..."); getchar ();
        ECDb ecdb;
        ASSERT_EQ (BE_SQLITE_OK, ecdb.OpenBeSQLiteDb (ecdbPath.c_str (), Db::OpenParams (Db::OPEN_Readonly)));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethods                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
void AssertProfileUpgrade (Utf8CP ecdbPath, Db::OpenParams const& openParams, SchemaVersion const& expectedProfileVersion)
    {
    ECDb ecdb;
    AssertProfileUpgrade (ecdb, ecdbPath, openParams, expectedProfileVersion);
    }

//---------------------------------------------------------------------------------------
// @bsimethods                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
void AssertProfileUpgrade (ECDbR ecdb, Utf8CP ecdbPath, Db::OpenParams const& openParams, SchemaVersion const& expectedProfileVersion)
    {
    ASSERT_EQ (BE_SQLITE_OK, ecdb.OpenBeSQLiteDb (ecdbPath, openParams)) << "Opening (and upgrading) ECDb file '" << ecdbPath << "' failed unexpectedly.";

    Utf8String actualProfileVersionStr;
    EXPECT_EQ  (BE_SQLITE_ROW, ecdb.QueryProperty (actualProfileVersionStr, PROFILEVERSION_PROPSPEC)) << "ECDb file '" << ecdbPath << "' is expected to contain an entry for the ECDb profile version in be_prop.";
    SchemaVersion actualProfileVersion (actualProfileVersionStr.c_str ());
    EXPECT_TRUE (expectedProfileVersion == actualProfileVersion) << "ECDb file '" << ecdbPath << "' has unexpected version of ECDb profile. Expected: " << expectedProfileVersion.ToJson ().c_str () << " Actual: " << actualProfileVersionStr.c_str ();

    // now assert a few changes that happened during the upgrade
    AssertIsProfileUpToDate (ecdb);
    }

//---------------------------------------------------------------------------------------
// @bsimethods                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
void AssertIsProfileUpToDate (ECDbCR ecdb)
    {
    ASSERT_TRUE (ecdb.ColumnExists ("ec_Property", "MinOccurs")) << "Column expected after upgrade";
    ASSERT_TRUE (ecdb.ColumnExists ("ec_Property", "MaxOccurs")) << "Column expected after upgrade";

    //assert that indices were re-created, too
      {
      Statement stmt;
      stmt.Prepare (ecdb, "SELECT NULL FROM sqlite_master WHERE type='index' AND tbl_name='ec_Schema' LIMIT 1");
      ASSERT_TRUE (stmt.Step () == BE_SQLITE_ROW) << "Index on ec_Schema is expected to still exist after upgrade";
      }

    //Assert some hold-off changes which were originally planned, but rolled back to preserve backwards compatibility with 03 apps
    ASSERT_TRUE (ecdb.ColumnExists ("ec_ClassMap", "SQLCreateTable")) << "Column expected to still exist after upgrade. Removed deferred.";
    ASSERT_TRUE (ecdb.ColumnExists ("ec_RelationshipConstraint", "RoleLable")) << "Misspelt column expected to still exist after upgrade";
    ASSERT_FALSE (ecdb.ColumnExists ("ec_RelationshipConstraint", "RoleLabel")) << "Correctly spelt column not expected to exist as column name fix was deferred";

    ASSERT_TRUE (ecdb.ColumnExists ("ec_RelationshipConstraintClass", "RelationKeyDbColumnName")) << "Column expected to still exist after upgrade. Removed deferred.";

    ASSERT_TRUE (ecdb.ColumnExists ("ec_Schema", "MapVersion")) << "Column expected to still exist after upgrade. Removed deferred.";
    ASSERT_TRUE (ecdb.ColumnExists ("ec_Schema", "SchemaAsBlob")) << "Column expected to still exist after upgrade. Removed deferred.";
    ASSERT_TRUE (ecdb.ColumnExists ("ec_Schema", "IsReadonly")) << "Column expected to still exist after upgrade. Removed deferred.";
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
    ASSERT_EQ (BE_SQLITE_OK, ecdb.OpenBeSQLiteDb (ecdbPath, Db::OpenParams (Db::OPEN_Readonly)));
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
    ASSERT_EQ (BE_SQLITE_OK, legacyECDb.OpenBeSQLiteDb (legacyECDbPath.c_str (), ECDb::OpenParams (ECDb::OPEN_Readonly))) << "Opening legacy ECDb file '" << legacyECDbPath.c_str () << "' failed unexpectedly.";

    Utf8CP table1Name = "ecsql_ArrayOfPASpatial";
    Utf8CP cgPropName = "Geometry";
    Utf8CP cgArrayPropName = "Geometry_Array";

    Utf8CP table2Name = "ecsql_SSpatial";
    Utf8CP class2StructPropName = "PASpatialProp";
    auto getClass2CGPropPath = [&class2StructPropName] (Utf8CP cgPropName)
        {
        Utf8String propPath (class2StructPropName);
        propPath.append (".").append (cgPropName);
        return std::move (propPath);
        };

    ASSERT_FALSE (legacyECDb.ColumnExists (table1Name, cgPropName)) << "CG column not expected to exist after opening legacy file";
    ASSERT_FALSE (legacyECDb.ColumnExists (table1Name, cgArrayPropName)) << "CG column not expected to exist after opening legacy file";
    ASSERT_FALSE (legacyECDb.ColumnExists (table2Name, getClass2CGPropPath (cgPropName).c_str ())) << "CG column not expected to exist after opening legacy file";
    ASSERT_FALSE (legacyECDb.ColumnExists (table2Name, getClass2CGPropPath (cgArrayPropName).c_str ())) << "CG column not expected to exist after opening legacy file";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbProfile, ECSqlSelectAgainstLegacyFilesWithoutCGColumns)
    {
    ECDbTestProject::Initialize ();

    Utf8String legacyECDbPath = CopyTestFile ("legacyfilewithoutcommongeometrycolumns.ecdb");
    ECDb legacyECDb;
    ASSERT_EQ (BE_SQLITE_OK, legacyECDb.OpenBeSQLiteDb (legacyECDbPath.c_str (), ECDb::OpenParams (ECDb::OPEN_Readonly))) << "Opening legacy ECDb file '" << legacyECDbPath.c_str () << "' failed unexpectedly.";

    Utf8CP class1Name = "ecsql.PASpatial";
    Utf8CP cgPropName = "Geometry";
    Utf8CP cgArrayPropName = "Geometry_Array";

    Utf8CP class2Name = "ecsql.SSpatial";
    Utf8CP class2StructPropName = "PASpatialProp";

        {
        ECSqlStatement stmt;
        Utf8String ecsql;
        ecsql.Sprintf ("SELECT * FROM %s LIMIT 1", class1Name);
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stmt.Prepare (legacyECDb, ecsql.c_str ()));
        while (stmt.Step () == ECSqlStepStatus::HasRow)
            {
            int colCount = stmt.GetColumnCount ();
            for (int i = 0; i < colCount; i++)
                {
                IECSqlValue const& val = stmt.GetValue (i);
                ECTypeDescriptor const& type = val.GetColumnInfo ().GetDataType ();
                if (type.GetPrimitiveType () == PRIMITIVETYPE_IGeometry)
                    {
                    if (type.IsPrimitive ())
                        {
                        ASSERT_TRUE (val.IsNull ());
                        ASSERT_TRUE (val.GetGeometry () == nullptr);
                        }
                    else if (type.IsPrimitiveArray ())
                        {
                        IECSqlArrayValue const& arrayVal = val.GetArray ();
                        ASSERT_EQ (0, arrayVal.GetArrayLength ());
                        }
                    }
                }
            }
        }

        {
        ECSqlStatement stmt;
        Utf8String ecsql;
        ecsql.Sprintf ("SELECT %s, %s FROM %s LIMIT 1", cgPropName, cgArrayPropName, class1Name);
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stmt.Prepare (legacyECDb, ecsql.c_str ()));
        while (stmt.Step () == ECSqlStepStatus::HasRow)
            {
            IECSqlValue const& val0 = stmt.GetValue (0);
            ASSERT_TRUE (val0.IsNull ());
            ASSERT_TRUE (val0.GetGeometry () == nullptr);

            IECSqlValue const& val1 = stmt.GetValue (1);
            IECSqlArrayValue const& arrayVal = val1.GetArray ();
            ASSERT_EQ (0, arrayVal.GetArrayLength ());
            }
         }

        {
        ECSqlStatement stmt;
        Utf8String ecsql;
        ecsql.Sprintf ("SELECT * FROM %s WHERE %s IS NULL OR %s IS NOT NULL", class1Name, cgPropName, cgArrayPropName);
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stmt.Prepare (legacyECDb, ecsql.c_str ()));
        ASSERT_EQ ((int) ECSqlStepStatus::HasRow, (int) stmt.Step ()) << "ECSQL " << ecsql.c_str () << " is expected to return at least one row.";
        }

        {
        ECSqlStatement stmt;
        Utf8String ecsql;
        ecsql.Sprintf ("SELECT %s FROM %s LIMIT 1", class2StructPropName, class2Name);
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stmt.Prepare (legacyECDb, ecsql.c_str ()));
        while (stmt.Step () == ECSqlStepStatus::HasRow)
            {
            IECSqlStructValue const& structVal = stmt.GetValueStruct (0);
            int memberCount = structVal.GetMemberCount ();
            for (int i = 0; i < memberCount; i++)
                {
                IECSqlValue const& val = structVal.GetValue (i);
                ECTypeDescriptor const& type = val.GetColumnInfo ().GetDataType ();
                if (type.GetPrimitiveType () == PRIMITIVETYPE_IGeometry)
                    {
                    if (type.IsPrimitive ())
                        {
                        ASSERT_TRUE (val.IsNull ());
                        ASSERT_TRUE (val.GetGeometry () == nullptr);
                        }
                    else if (type.IsPrimitiveArray ())
                        {
                        IECSqlArrayValue const& arrayVal = val.GetArray ();
                        ASSERT_EQ (0, arrayVal.GetArrayLength ());
                        }
                    }
                }
            }
         }

        {
        ECSqlStatement stmt;
        Utf8String ecsql;
        ecsql.Sprintf ("SELECT * FROM %s WHERE %s.%s IS NOT NULL AND %s.%s IS NOT NULL", class2Name, class2StructPropName, cgPropName, class2StructPropName, cgArrayPropName);
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stmt.Prepare (legacyECDb, ecsql.c_str ()));
        ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) stmt.Step ()) << "ECSQL " << ecsql.c_str () << " is expected to not return any row.";
        }
    }


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbProfile, ECSqlInsertAgainstLegacyFilesWithoutCGColumns)
    {
    ECDbTestProject::Initialize ();

    Utf8String legacyECDbPath = CopyTestFile ("legacyfilewithoutcommongeometrycolumns.ecdb");
    ECDb legacyECDb;
    ASSERT_EQ (BE_SQLITE_OK, legacyECDb.OpenBeSQLiteDb (legacyECDbPath.c_str (), ECDb::OpenParams (ECDb::OPEN_ReadWrite))) << "Opening legacy ECDb file '" << legacyECDbPath.c_str () << "' failed unexpectedly.";

    Utf8CP class1Name = "ecsql.PASpatial";
    Utf8CP cgPropName = "Geometry";
    Utf8CP cgArrayPropName = "Geometry_Array";

    Utf8CP class2Name = "ecsql.SSpatial";
    Utf8CP class2StructPropName = "PASpatialProp";

        {
        ECSqlStatement stmt;
        Utf8String ecsql;
        ecsql.Sprintf ("INSERT INTO %s (%s, %s) VALUES (?, ?)", class1Name, cgPropName, cgArrayPropName);
        ASSERT_EQ ((int) ECSqlStatus::InvalidECSql, (int) stmt.Prepare (legacyECDb, ecsql.c_str ()));
        }

        {
        ECSqlStatement stmt;
        Utf8String ecsql;
        ecsql.Sprintf ("INSERT INTO %s (%s) VALUES (?)", class2Name, class2StructPropName);
        ASSERT_EQ ((int) ECSqlStatus::InvalidECSql, (int) stmt.Prepare (legacyECDb, ecsql.c_str ()));
        }

        {
        ECSqlStatement stmt;
        Utf8String ecsql;
        ecsql.Sprintf ("INSERT INTO %s (%s.%s, %s.%s) VALUES (?, ?)", class2Name, class2StructPropName, cgPropName, class2StructPropName, cgArrayPropName);
        ASSERT_EQ ((int) ECSqlStatus::InvalidECSql, (int) stmt.Prepare (legacyECDb, ecsql.c_str ()));
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbProfile, ECSqlUpdateAgainstLegacyFilesWithoutCGColumns)
    {
    ECDbTestProject::Initialize ();

    Utf8String legacyECDbPath = CopyTestFile ("legacyfilewithoutcommongeometrycolumns.ecdb");
    ECDb legacyECDb;
    ASSERT_EQ (BE_SQLITE_OK, legacyECDb.OpenBeSQLiteDb (legacyECDbPath.c_str (), ECDb::OpenParams (ECDb::OPEN_ReadWrite))) << "Opening legacy ECDb file '" << legacyECDbPath.c_str () << "' failed unexpectedly.";

    Utf8CP class1Name = "ecsql.PASpatial";
    Utf8CP cgPropName = "Geometry";
    Utf8CP cgArrayPropName = "Geometry_Array";

    Utf8CP class2Name = "ecsql.SSpatial";
    Utf8CP class2StructPropName = "PASpatialProp";

        {
        ECSqlStatement stmt;
        Utf8String ecsql;
        ecsql.Sprintf ("UPDATE ONLY %s SET %s = ?, %s = ?", class1Name, cgPropName, cgArrayPropName);
        ASSERT_EQ ((int) ECSqlStatus::InvalidECSql, (int) stmt.Prepare (legacyECDb, ecsql.c_str ()));
        }

        {
        ECSqlStatement stmt;
        Utf8String ecsql;
        ecsql.Sprintf ("UPDATE ONLY %s SET %s = ?", class2Name, class2StructPropName);
        ASSERT_EQ ((int) ECSqlStatus::InvalidECSql, (int) stmt.Prepare (legacyECDb, ecsql.c_str ()));
        }

        {
        ECSqlStatement stmt;
        Utf8String ecsql;
        ecsql.Sprintf ("UPDATE ONLY %s SET %s.%s = ?, %s.%s = ?", class2Name, class2StructPropName, cgPropName, class2StructPropName, cgArrayPropName);
        ASSERT_EQ ((int) ECSqlStatus::InvalidECSql, (int) stmt.Prepare (legacyECDb, ecsql.c_str ()));
        }

    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbProfile, ECSqlDeleteAgainstLegacyFilesWithoutCGColumns)
    {
    struct TestEventHandler : ECSqlEventHandler
        {
    private:
        int m_rowsAffected;

        virtual void _OnEvent (EventType eventType, ECSqlEventArgs const& args) override { m_rowsAffected = (int) args.GetInstanceKeys ().size (); }

    public:
        TestEventHandler ()
            : ECSqlEventHandler (), m_rowsAffected (-1)
            {}

        int GetRowsAffected () const { return m_rowsAffected; }
        };

    ECDbTestProject::Initialize ();

    Utf8String legacyECDbPath = CopyTestFile ("legacyfilewithoutcommongeometrycolumns.ecdb");
    ECDb legacyECDb;
    ASSERT_EQ (BE_SQLITE_OK, legacyECDb.OpenBeSQLiteDb (legacyECDbPath.c_str (), ECDb::OpenParams (ECDb::OPEN_ReadWrite))) << "Opening legacy ECDb file '" << legacyECDbPath.c_str () << "' failed unexpectedly.";

    Utf8CP class1Name = "ecsql.PASpatial";
    Utf8CP cgPropName = "Geometry";
    Utf8CP cgArrayPropName = "Geometry_Array";

    Utf8CP class2Name = "ecsql.SSpatial";
    Utf8CP class2StructPropName = "PASpatialProp";

        {
        ECSqlStatement stmt;
        TestEventHandler eh;
        stmt.RegisterEventHandler (eh);
        Utf8String ecsql;
        ecsql.Sprintf ("DELETE FROM ONLY %s", class1Name);
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stmt.Prepare (legacyECDb, ecsql.c_str ()));
        ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) stmt.Step ());
        ASSERT_GT (eh.GetRowsAffected (), 0);
        }

        {
        ECSqlStatement stmt;
        Utf8String ecsql;
        ecsql.Sprintf ("DELETE FROM ONLY %s WHERE %s IS NULL OR %s IS NOT NULL", class1Name, cgPropName, cgArrayPropName);
        ASSERT_EQ ((int) ECSqlStatus::InvalidECSql, (int) stmt.Prepare (legacyECDb, ecsql.c_str ())) << "Geometry props in WHERE clause not supported for legacy ECDb files with unmapped CG props";
        }

        {
        ECSqlStatement stmt;
        Utf8String ecsql;
        ecsql.Sprintf ("DELETE FROM ONLY %s WHERE %s IS NOT NULL", class1Name, cgPropName);
        ASSERT_EQ ((int) ECSqlStatus::InvalidECSql, (int) stmt.Prepare (legacyECDb, ecsql.c_str ())) << "Geometry props in WHERE clause not supported for legacy ECDb files with unmapped CG props";
        }

        {
        ECSqlStatement stmt;
        TestEventHandler eh;
        stmt.RegisterEventHandler (eh);
        Utf8String ecsql;
        ecsql.Sprintf ("DELETE FROM ONLY %s", class2Name);
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stmt.Prepare (legacyECDb, ecsql.c_str ()));
        ASSERT_EQ ((int) ECSqlStepStatus::Done, (int) stmt.Step ());
        ASSERT_GT (eh.GetRowsAffected (), 0);
        }

        {
        ECSqlStatement stmt;
        Utf8String ecsql;
        ecsql.Sprintf ("DELETE FROM ONLY %s WHERE %s.%s IS NULL OR %s.%s IS NOT NULL", class2Name, class2StructPropName, cgPropName, class2StructPropName, cgArrayPropName);
        ASSERT_EQ ((int) ECSqlStatus::InvalidECSql, (int) stmt.Prepare (legacyECDb, ecsql.c_str ())) << "Geometry props in WHERE clause not supported for legacy ECDb files with unmapped CG props";
        }

        {
        ECSqlStatement stmt;
        Utf8String ecsql;
        ecsql.Sprintf ("DELETE FROM ONLY %s WHERE %s.%s IS NOT NULL", class2Name, class2StructPropName, cgArrayPropName);
        ASSERT_EQ ((int) ECSqlStatus::InvalidECSql, (int) stmt.Prepare (legacyECDb, ecsql.c_str ())) << "Geometry props in WHERE clause not supported for legacy ECDb files with unmapped CG props";
        }
    }


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbProfile, ECInstanceECSqlSelectAdapterAgainstLegacyFilesWithoutCGColumns)
    {
    ECDbTestProject::Initialize ();

    Utf8String legacyECDbPath = CopyTestFile ("legacyfilewithoutcommongeometrycolumns.ecdb");
    ECDb legacyECDb;
    ASSERT_EQ (BE_SQLITE_OK, legacyECDb.OpenBeSQLiteDb (legacyECDbPath.c_str (), ECDb::OpenParams (ECDb::OPEN_Readonly))) << "Opening legacy ECDb file '" << legacyECDbPath.c_str () << "' failed unexpectedly.";

    Utf8CP class1Name = "ecsql.PASpatial";
    Utf8CP cgPropName = "Geometry";
    Utf8CP cgArrayPropName = "Geometry_Array";

        {
        ECSqlStatement stmt;
        Utf8String ecsql;
        ecsql.Sprintf ("SELECT * FROM %s LIMIT 1", class1Name);
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stmt.Prepare (legacyECDb, ecsql.c_str ()));

        ECInstanceECSqlSelectAdapter adapter (stmt);
        while (stmt.Step () == ECSqlStepStatus::HasRow)
            {
            ECN::IECInstancePtr instance = adapter.GetInstance ();
            ASSERT_TRUE (instance != nullptr);

            ECN::ECValue v;
            ASSERT_EQ (ECOBJECTS_STATUS_Success, instance->GetValue (v, WString(cgPropName, BentleyCharEncoding::Utf8).c_str ()));
            ASSERT_TRUE (v.IsNull ());

            v.Clear ();
            ASSERT_EQ (ECOBJECTS_STATUS_Success, instance->GetValue (v, WString (cgArrayPropName, BentleyCharEncoding::Utf8).c_str ()));
            ASSERT_EQ (0, v.GetArrayInfo ().GetCount ());
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbProfile, ECInstanceInserterUpdaterDeleterAgainstLegacyFilesWithoutCGColumns)
    {
    ECDbTestProject::Initialize ();

    Utf8String legacyECDbPath = CopyTestFile ("legacyfilewithoutcommongeometrycolumns.ecdb");
    ECDb legacyECDb;
    ASSERT_EQ (BE_SQLITE_OK, legacyECDb.OpenBeSQLiteDb (legacyECDbPath.c_str (), ECDb::OpenParams (ECDb::OPEN_ReadWrite))) << "Opening legacy ECDb file '" << legacyECDbPath.c_str () << "' failed unexpectedly.";

    ECN::ECClassCP ecClass = legacyECDb.GetSchemaManager ().GetECClass ("ecsql", "PASpatial", ECDbSchemaManager::ResolveSchema::BySchemaNamespacePrefix);
    ASSERT_TRUE (ecClass != nullptr);

    //get test instance
    ECN::IECInstancePtr instance = nullptr;
        {
        ECSqlStatement stmt;
        Utf8String ecsql;
        ecsql.Sprintf ("SELECT * FROM ecsql.PASpatial LIMIT 1", ecClass);
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stmt.Prepare (legacyECDb, ecsql.c_str ()));

        ECInstanceECSqlSelectAdapter adapter (stmt);
        ASSERT_EQ ((int) ECSqlStepStatus::HasRow, (int) stmt.Step ());
        instance = adapter.GetInstance ();
        }

    ASSERT_TRUE (instance != nullptr);

    ECN::ECValue v;
    v.SetIsNull (true);
    ASSERT_EQ (ECOBJECTS_STATUS_Success, instance->SetValue (L"Geometry", v));

    ECInstanceInserter inserter (legacyECDb, *ecClass);
    ASSERT_FALSE (inserter.IsValid ());
    ASSERT_EQ (ERROR, inserter.Insert (*instance));

    ECInstanceUpdater updater (legacyECDb, *ecClass);
    ASSERT_FALSE (updater.IsValid ());
    ASSERT_EQ (ERROR, updater.Update (*instance));

    //Deleter works as it doesn't need the CG columns in the SQLite SQL statement (it just does DELETE FROM Foo WHERE ECInstanceId = nn).
    ECInstanceDeleter deleter (legacyECDb, *ecClass);
    ASSERT_TRUE (deleter.IsValid ());
    ASSERT_EQ (SUCCESS, deleter.Delete (*instance));
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
