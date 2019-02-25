/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/UsageDbTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BeSQLite/BeSQLite.h>

#include "UsageDbTests.h"

#include "../../Licensing/UsageDb.h"

USING_NAMESPACE_BENTLEY_LICENSING

USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS

void UsageDbTests::SetUpTestCase()
	{
	BeFileName tmpDir;
	BeTest::GetHost().GetTempDir(tmpDir);

	BeSQLiteLib::Initialize(tmpDir);

	Test::SetUpTestCase();
	}

Utf8String ReadAllFile(BeFileNameCR path)
    {
    BeFile file;
    file.Open(path, BeFileAccess::Read);

    ByteStream byteStream;
    if (BeFileStatus::Success != file.ReadEntireFile(byteStream))
        return nullptr;

    Utf8String contents((Utf8CP)byteStream.GetData(), byteStream.GetSize());
    file.Close();

    return contents;
    }

BentleyStatus OpenOrCreateTestDb(UsageDb& db, BeFileNameCR dbFileName = BeFileName("TestDb.db"))
    {
    BeFileName tmpDir;
    BeTest::GetHost().GetTempDir(tmpDir);
    tmpDir.AppendToPath(dbFileName);
    return db.OpenOrCreate(tmpDir);
    }

TEST_F(UsageDbTests, OpenOrCreate_OpenExistingDb_ContainsInsertedRows)
    {
    UsageDb db;
    EXPECT_SUCCESS(OpenOrCreateTestDb(db));

    Utf8String eventTime = DateTime::GetCurrentTimeUtc().ToString();

    EXPECT_SUCCESS(db.RecordUsage(99, "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "TestDeviceId",
                                  "IXravQ3f71wUupkp+tLBK+vGmCc=", "qa2_devuser2@mailinator.com", "3Q746c3/YJfSzlDyMbrq6oMUbMQ=",
                                  "7a265495-71a8-4557-bbaf-de57f31b26b8", "4d701223-37ca-4ffb-b91c-f650a937d6fd", 2545, "", 1000000000000,
                                  "00000000-0000-0000-0000-000000000000", "c0d1ed44-3b6c-4316-9f3e-e856c85b4995", eventTime,
                                  1.0, "RealTime", "US", "Production"));

    EXPECT_TRUE(db.GetLastUsageRecordedTime().Equals(eventTime));

    EXPECT_SUCCESS(OpenOrCreateTestDb(db));
    EXPECT_TRUE(db.GetLastUsageRecordedTime().Equals(eventTime));

    db.Close();

    EXPECT_SUCCESS(OpenOrCreateTestDb(db));
    EXPECT_TRUE(db.GetLastUsageRecordedTime().Equals(eventTime));
    }

TEST_F(UsageDbTests, OpenOrCreate_DifferentDb_OpensNewDb)
    {
    UsageDb db;
    EXPECT_SUCCESS(OpenOrCreateTestDb(db));

    Utf8String eventTime = DateTime::GetCurrentTimeUtc().ToString();

    EXPECT_SUCCESS(db.RecordUsage(99, "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "TestDeviceId",
                                  "IXravQ3f71wUupkp+tLBK+vGmCc=", "qa2_devuser2@mailinator.com", "3Q746c3/YJfSzlDyMbrq6oMUbMQ=",
                                  "7a265495-71a8-4557-bbaf-de57f31b26b8", "4d701223-37ca-4ffb-b91c-f650a937d6fd", 2545, "", 1000000000000,
                                  "00000000-0000-0000-0000-000000000000", "c0d1ed44-3b6c-4316-9f3e-e856c85b4995", eventTime,
                                  1.0, "RealTime", "US", "Production"));

    EXPECT_SUCCESS(OpenOrCreateTestDb(db, BeFileName("new.db")));

    EXPECT_FALSE(db.GetLastUsageRecordedTime().Equals(eventTime));
    }

TEST_F(UsageDbTests, OpenOrCreate_OpenAndUpdateExistingDatabase_Success)
    {
    UsageDb db;
    Utf8String fileName;

    fileName.Sprintf("Licensing-%s.db", BeGuid(true).ToString().c_str());
    EXPECT_SUCCESS(OpenOrCreateTestDb(db, BeFileName(fileName)));

    EXPECT_SUCCESS(db.RecordUsage(99, "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "TestDeviceId",
                                  "IXravQ3f71wUupkp+tLBK+vGmCc=", "qa2_devuser2@mailinator.com", "3Q746c3/YJfSzlDyMbrq6oMUbMQ=",
                                  "7a265495-71a8-4557-bbaf-de57f31b26b8", "4d701223-37ca-4ffb-b91c-f650a937d6fd", 2545, "", 1000000000000,
                                  "00000000-0000-0000-0000-000000000000", "c0d1ed44-3b6c-4316-9f3e-e856c85b4995", DateTime::GetCurrentTimeUtc().ToString(),
                                  1.0, "RealTime", "US", "Production"));

    EXPECT_SUCCESS(db.RecordUsage(99, "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "TestDeviceId",
                                  "IXravQ3f71wUupkp+tLBK+vGmCc=", "qa2_devuser2@mailinator.com", "3Q746c3/YJfSzlDyMbrq6oMUbMQ=",
                                  "7a265495-71a8-4557-bbaf-de57f31b26b8", "4d701223-37ca-4ffb-b91c-f650a937d6fd", 2545, "", 1000000000000,
                                  "00000000-0000-0000-0000-000000000000", "c0d1ed44-3b6c-4316-9f3e-e856c85b4995", DateTime::GetCurrentTimeUtc().ToString(),
                                  1.0, "RealTime", "US", "Production"));

    EXPECT_SUCCESS(db.RecordUsage(99, "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "TestDeviceId",
                                  "IXravQ3f71wUupkp+tLBK+vGmCc=", "qa2_devuser2@mailinator.com", "3Q746c3/YJfSzlDyMbrq6oMUbMQ=",
                                  "7a265495-71a8-4557-bbaf-de57f31b26b8", "4d701223-37ca-4ffb-b91c-f650a937d6fd", 2545, "", 1000000000000,
                                  "00000000-0000-0000-0000-000000000000", "c0d1ed44-3b6c-4316-9f3e-e856c85b4995", DateTime::GetCurrentTimeUtc().ToString(),
                                  1.0, "RealTime", "US", "Production"));

    db.Close();

    Db testDb;
    Statement stmt;
    double testSchemaVersion = .90;
    Utf8String updateStatement;
    BeFileName tmpDir;
    BeTest::GetHost().GetTempDir(tmpDir);

    EXPECT_EQ(DbResult::BE_SQLITE_OK, testDb.OpenBeSQLiteDb(tmpDir.AppendToPath(BeFileName(fileName)), Db::OpenParams(Db::OpenMode::ReadWrite)));

    updateStatement.Sprintf("UPDATE eimVersion SET SchemaVersion = %f WHERE rowid = 1", testSchemaVersion);
    stmt.Prepare(testDb, (Utf8CP) updateStatement.c_str());
    DbResult res = stmt.Step();
    EXPECT_EQ(DbResult::BE_SQLITE_DONE, res);

    stmt.Finalize();
    testDb.CloseDb();

    EXPECT_SUCCESS(OpenOrCreateTestDb(db, BeFileName(fileName)));
    }

TEST_F(UsageDbTests, WriteUsageToCSVFile_MultipleRecords_CreatesCorrectFile)
    {
    UsageDb db;
    Utf8String fileName;

    fileName.Sprintf("Licensing-%s.db", BeGuid(true).ToString().c_str());
    EXPECT_SUCCESS(OpenOrCreateTestDb(db, BeFileName(fileName)));

    Utf8String eventTime1 = DateTime::GetCurrentTimeUtc().ToString();

    EXPECT_SUCCESS(db.RecordUsage(99, "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "TestDeviceId",
                                  "IXravQ3f71wUupkp+tLBK+vGmCc=", "qa2_devuser2@mailinator.com", "3Q746c3/YJfSzlDyMbrq6oMUbMQ=",
                                  "7a265495-71a8-4557-bbaf-de57f31b26b8", "4d701223-37ca-4ffb-b91c-f650a937d6fd", 2545, "", 1000000000000,
                                  "00000000-0000-0000-0000-000000000000", "c0d1ed44-3b6c-4316-9f3e-e856c85b4995", eventTime1,
                                  1.0, "RealTime", "US", "Production"));

    Utf8String eventTime2 = DateTime::GetCurrentTimeUtc().ToString();

    EXPECT_SUCCESS(db.RecordUsage(99, "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "TestDeviceId",
                                  "IXravQ3f71wUupkp+tLBK+vGmCc=", "qa2_devuser2@mailinator.com", "3Q746c3/YJfSzlDyMbrq6oMUbMQ=",
                                  "7a265495-71a8-4557-bbaf-de57f31b26b8", "4d701223-37ca-4ffb-b91c-f650a937d6fd", 2545, "", 1000000000000,
                                  "00000000-0000-0000-0000-000000000000", "c0d1ed44-3b6c-4316-9f3e-e856c85b4995", eventTime2,
                                  1.0, "RealTime", "US", "Production"));

    Utf8String eventTime3 = DateTime::GetCurrentTimeUtc().ToString();

    EXPECT_SUCCESS(db.RecordUsage(99, "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "TestDeviceId",
                                  "IXravQ3f71wUupkp+tLBK+vGmCc=", "qa2_devuser2@mailinator.com", "3Q746c3/YJfSzlDyMbrq6oMUbMQ=",
                                  "7a265495-71a8-4557-bbaf-de57f31b26b8", "4d701223-37ca-4ffb-b91c-f650a937d6fd", 2545, "", 1000000000000,
                                  "00000000-0000-0000-0000-000000000000", "c0d1ed44-3b6c-4316-9f3e-e856c85b4995", eventTime3,
                                  1.0, "RealTime", "US", "Production"));

    BeFileName tmpFile;
    Utf8String csvFile;
    csvFile.Sprintf("Usages-%s.csv", BeGuid(true).ToString().c_str());
    BeTest::GetHost().GetTempDir(tmpFile);
    tmpFile.AppendToPath(BeFileName(csvFile));
    EXPECT_SUCCESS(db.WriteUsageToCSVFile(tmpFile));

    auto fileContent = ReadAllFile(tmpFile);

    EXPECT_TRUE(fileContent.Contains(eventTime1));
    EXPECT_TRUE(fileContent.Contains(eventTime2));
    EXPECT_TRUE(fileContent.Contains(eventTime3));
    }

TEST_F(UsageDbTests, WriteFeatureToCSVFile_MultipleRecords_CreatesCorrectFile)
    {
    UsageDb db;
    Utf8String fileName;
    using namespace std::chrono_literals;

    fileName.Sprintf("Licensing-%s.db", BeGuid(true).ToString().c_str());
    EXPECT_SUCCESS(OpenOrCreateTestDb(db, BeFileName(fileName)));

    Utf8String userData = "Name^Parasolid#Description^Comprehensive capabilities extend to over 750 functions that include a wealth of model creation and editing utilities.#"
                          "Version^21.0.244#Vendor^Siemens, Inc.#";

    Utf8String eventTime1 = DateTime::GetCurrentTimeUtc().ToString();

    EXPECT_SUCCESS(db.RecordFeature(99, "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "TestDeviceId",
                                    "IXravQ3f71wUupkp+tLBK+vGmCc=", "qa2_devuser2@mailinator.com", "3Q746c3/YJfSzlDyMbrq6oMUbMQ=",
                                    "7a265495-71a8-4557-bbaf-de57f31b26b8", "4d701223-37ca-4ffb-b91c-f650a937d6fd", 2545, "", 1000000000000,
                                    "00000000-0000-0000-0000-000000000000", "c0d1ed44-3b6c-4316-9f3e-e856c85b4995", eventTime1,
                                    1.0, "RealTime", "US", "Production", "be0761b2-896f-4ad6-a7c1-a08169340f54", eventTime1, eventTime1, userData));

    Utf8String eventTime2 = DateTime::GetCurrentTimeUtc().ToString();

    EXPECT_SUCCESS(db.RecordFeature(99, "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "TestDeviceId",
                                    "IXravQ3f71wUupkp+tLBK+vGmCc=", "qa2_devuser2@mailinator.com", "3Q746c3/YJfSzlDyMbrq6oMUbMQ=",
                                    "7a265495-71a8-4557-bbaf-de57f31b26b8", "4d701223-37ca-4ffb-b91c-f650a937d6fd", 2545, "", 1000000000000,
                                    "00000000-0000-0000-0000-000000000000", "c0d1ed44-3b6c-4316-9f3e-e856c85b4995", eventTime2,
                                    1.0, "RealTime", "US", "Production", "be0761b2-896f-4ad6-a7c1-a08169340f54", eventTime2, eventTime2, userData));

    Utf8String eventTime3 = DateTime::GetCurrentTimeUtc().ToString();

    EXPECT_SUCCESS(db.RecordFeature(99, "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "TestDeviceId",
                                    "IXravQ3f71wUupkp+tLBK+vGmCc=", "qa2_devuser2@mailinator.com", "3Q746c3/YJfSzlDyMbrq6oMUbMQ=",
                                    "7a265495-71a8-4557-bbaf-de57f31b26b8", "4d701223-37ca-4ffb-b91c-f650a937d6fd", 2545, "", 1000000000000,
                                    "00000000-0000-0000-0000-000000000000", "c0d1ed44-3b6c-4316-9f3e-e856c85b4995", eventTime3,
                                    1.0, "RealTime", "US", "Production", "be0761b2-896f-4ad6-a7c1-a08169340f54", eventTime3, eventTime3, userData));

    BeFileName tmpFile;
    Utf8String csvFile;
    csvFile.Sprintf("Features-%s.csv", BeGuid(true).ToString().c_str());
    BeTest::GetHost().GetTempDir(tmpFile);
    tmpFile.AppendToPath(BeFileName(csvFile));
    EXPECT_SUCCESS(db.WriteFeatureToCSVFile(tmpFile));

    auto fileContent = ReadAllFile(tmpFile);

    EXPECT_TRUE(fileContent.Contains(eventTime1));
    EXPECT_TRUE(fileContent.Contains(eventTime2));
    EXPECT_TRUE(fileContent.Contains(eventTime3));
    }

TEST_F(UsageDbTests, CleanUpUsages_Success)
    {
    UsageDb db;

    EXPECT_SUCCESS(OpenOrCreateTestDb(db));

    db.RecordUsage(99, "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "TestDeviceId",
                   "IXravQ3f71wUupkp+tLBK+vGmCc=", "qa2_devuser2@mailinator.com", "3Q746c3/YJfSzlDyMbrq6oMUbMQ=",
                   "7a265495-71a8-4557-bbaf-de57f31b26b8", "4d701223-37ca-4ffb-b91c-f650a937d6fd", 2545, "", 1000000000000,
                   "00000000-0000-0000-0000-000000000000", "c0d1ed44-3b6c-4316-9f3e-e856c85b4995", DateTime::GetCurrentTimeUtc().ToString(),
                   1.0, "RealTime", "US", "Production");

    db.RecordUsage(99, "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "TestDeviceId",
                   "IXravQ3f71wUupkp+tLBK+vGmCc=", "qa2_devuser2@mailinator.com", "3Q746c3/YJfSzlDyMbrq6oMUbMQ=",
                   "7a265495-71a8-4557-bbaf-de57f31b26b8", "4d701223-37ca-4ffb-b91c-f650a937d6fd", 2545, "", 1000000000000,
                   "00000000-0000-0000-0000-000000000000", "c0d1ed44-3b6c-4316-9f3e-e856c85b4995", DateTime::GetCurrentTimeUtc().ToString(),
                   1.0, "RealTime", "US", "Production");

    db.RecordUsage(99, "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "TestDeviceId",
                   "IXravQ3f71wUupkp+tLBK+vGmCc=", "qa2_devuser2@mailinator.com", "3Q746c3/YJfSzlDyMbrq6oMUbMQ=",
                   "7a265495-71a8-4557-bbaf-de57f31b26b8", "4d701223-37ca-4ffb-b91c-f650a937d6fd", 2545, "", 1000000000000,
                   "00000000-0000-0000-0000-000000000000", "c0d1ed44-3b6c-4316-9f3e-e856c85b4995", DateTime::GetCurrentTimeUtc().ToString(),
                   1.0, "RealTime", "US", "Production");

    db.RecordUsage(99, "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "TestDeviceId",
                   "IXravQ3f71wUupkp+tLBK+vGmCc=", "qa2_devuser2@mailinator.com", "3Q746c3/YJfSzlDyMbrq6oMUbMQ=",
                   "7a265495-71a8-4557-bbaf-de57f31b26b8", "4d701223-37ca-4ffb-b91c-f650a937d6fd", 2545, "", 1000000000000,
                   "00000000-0000-0000-0000-000000000000", "c0d1ed44-3b6c-4316-9f3e-e856c85b4995", DateTime::GetCurrentTimeUtc().ToString(),
                   1.0, "RealTime", "US", "Production");

    db.RecordUsage(99, "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "TestDeviceId",
                   "IXravQ3f71wUupkp+tLBK+vGmCc=", "qa2_devuser2@mailinator.com", "3Q746c3/YJfSzlDyMbrq6oMUbMQ=",
                   "7a265495-71a8-4557-bbaf-de57f31b26b8", "4d701223-37ca-4ffb-b91c-f650a937d6fd", 2545, "", 1000000000000,
                   "00000000-0000-0000-0000-000000000000", "c0d1ed44-3b6c-4316-9f3e-e856c85b4995", DateTime::GetCurrentTimeUtc().ToString(),
                   1.0, "RealTime", "US", "Production");

    EXPECT_SUCCESS(db.CleanUpUsages());
    EXPECT_EQ(0, db.GetUsageRecordCount());
    }

TEST_F(UsageDbTests, OfflineGracePeriod_Success)
	{
	UsageDb db;

	EXPECT_SUCCESS(OpenOrCreateTestDb(db));
	
	Utf8String testString = "TESTSTRING";
	Utf8String testString2 = "ANOTHERSTRING";

	EXPECT_EQ(db.GetOfflineGracePeriodStart(), "");
	EXPECT_SUCCESS(db.SetOfflineGracePeriodStart(testString));
	EXPECT_EQ(db.GetOfflineGracePeriodStart(), testString);
	EXPECT_SUCCESS(db.ResetOfflineGracePeriod());
	EXPECT_EQ(db.GetOfflineGracePeriodStart(), "");
	EXPECT_SUCCESS(db.SetOfflineGracePeriodStart(testString2));
	EXPECT_EQ(db.GetOfflineGracePeriodStart(), testString2);
	EXPECT_SUCCESS(db.ResetOfflineGracePeriod());
	EXPECT_EQ(db.GetOfflineGracePeriodStart(), "");
	}

TEST_F(UsageDbTests, DeleteAllOtherUserPolicies_Success)
	{
	UsageDb db;

	EXPECT_SUCCESS(OpenOrCreateTestDb(db));

	Utf8String userId = "userA";
	Utf8String policyIdToKeep = "3";
	// Put in some dummy policies for user
	db.AddOrUpdatePolicyFile("1", userId, "", "", "");
	db.AddOrUpdatePolicyFile("2", userId, "", "", "");
	db.AddOrUpdatePolicyFile(policyIdToKeep, userId, "", "", "");
	// Put in a policy for a different user
	db.AddOrUpdatePolicyFile("4", "userB", "", "", "");

	// attempt to delete all other user's policies
	db.DeleteAllOtherUserPolicyFiles(policyIdToKeep, userId);

	// Ids 1-2 should no longer exist
	ASSERT_EQ(db.GetPolicyFile("1"), Json::Value::GetNull());
	ASSERT_EQ(db.GetPolicyFile("2"), Json::Value::GetNull());
	// Ids 3 and 4 should still be in the database
	ASSERT_NE(db.GetPolicyFile(policyIdToKeep), Json::Value::GetNull());
	ASSERT_NE(db.GetPolicyFile("4"), Json::Value::GetNull());
	}
