/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/UsageDbTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BeSQLite/BeSQLite.h>

#include "UsageDbTests.h"

#include "../../../Licensing/UsageDb.h"

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

//TEST_F (UsageDbTests, OpenOrCreate_OpenExistingDb_ContainsInsertedRows)
//    {
//	UsageDb db;
//    EXPECT_SUCCESS(OpenOrCreateTestDb(db));
//
//    EXPECT_SUCCESS(db.InsertNewRecord(1000, 1000));
//    EXPECT_EQ(1000, db.GetLastRecordEndTime());
//
//    EXPECT_SUCCESS(OpenOrCreateTestDb(db));
//    EXPECT_EQ(1000, db.GetLastRecordEndTime());
//
//    db.Close();
//
//    EXPECT_SUCCESS(OpenOrCreateTestDb(db));
//    EXPECT_EQ(1000, db.GetLastRecordEndTime());
//    }

TEST_F(UsageDbTests, OpenOrCreate_DifferentDb_OpensNewDb)
    {
    UsageDb db;
    EXPECT_SUCCESS(OpenOrCreateTestDb(db));

    EXPECT_SUCCESS(db.InsertNewRecord(1000, 1000));

    EXPECT_SUCCESS(OpenOrCreateTestDb(db, BeFileName("new.db")));
    EXPECT_EQ(0, db.GetLastRecordEndTime());
    }

TEST_F(UsageDbTests, GetLastRecordEndTime_NoRecords_ReturnZero)
    {
    UsageDb db;
    EXPECT_SUCCESS(OpenOrCreateTestDb(db));

    EXPECT_EQ(0, db.GetLastRecordEndTime());
    }

//TEST_F(UsageDbTests, GetLastRecordEndTime_SingleRecord_ReturnEndTime)
//    {
//    UsageDb db;
//    EXPECT_SUCCESS(OpenOrCreateTestDb(db));
//
//    EXPECT_SUCCESS(db.InsertNewRecord(1000, 2000));
//    EXPECT_EQ(2000, db.GetLastRecordEndTime());
//    }

//TEST_F(UsageDbTests, GetLastRecordEndTime_MultipleRecords_ReturnEndTime)
//    {
//    UsageDb db;
//    EXPECT_SUCCESS(OpenOrCreateTestDb(db));
//
//    EXPECT_SUCCESS(db.InsertNewRecord(1000, 2000));
//    EXPECT_SUCCESS(db.InsertNewRecord(3000, 4000));
//    EXPECT_SUCCESS(db.InsertNewRecord(5000, 6000));
//    EXPECT_EQ(6000, db.GetLastRecordEndTime());
//    }

//TEST_F(UsageDbTests, UpdateLastRecordEndTime_NoRecords_RetursError)
//    {
//    UsageDb db;
//    EXPECT_SUCCESS(OpenOrCreateTestDb(db));
//
//    EXPECT_ERROR(db.UpdateLastRecordEndTime(6000));
//    }

//TEST_F(UsageDbTests, UpdateLastRecordEndTime_SingleRecord_UpdatesSuccessfull)
//    {
//    UsageDb db;
//    EXPECT_SUCCESS(OpenOrCreateTestDb(db));
//
//    EXPECT_SUCCESS(db.InsertNewRecord(1000, 2000));
//
//    EXPECT_SUCCESS(db.UpdateLastRecordEndTime(6000));
//    EXPECT_EQ(6000, db.GetLastRecordEndTime());
//    }

//TEST_F(UsageDbTests, UpdateLastRecordEndTime_MultipleRecords_UpdatesLastRecordSuccessfully)
//    {
//    UsageDb db;
//    EXPECT_SUCCESS(OpenOrCreateTestDb(db));
//
//    EXPECT_SUCCESS(db.InsertNewRecord(1000, 2000));
//    EXPECT_SUCCESS(db.InsertNewRecord(2000, 3000));
//    EXPECT_SUCCESS(db.InsertNewRecord(4000, 5000));
//    EXPECT_SUCCESS(db.UpdateLastRecordEndTime(6000));
//    EXPECT_EQ(6000, db.GetLastRecordEndTime());
//    }

TEST_F(UsageDbTests, GetRecordCount_NoRecords_ReturnsZero)
    {
    UsageDb db;
    EXPECT_SUCCESS(OpenOrCreateTestDb(db));

    EXPECT_EQ(0, db.GetRecordCount());
    }

//TEST_F(UsageDbTests, GetRecordCount_SingleRecord_ReturnsOne)
//    {
//    UsageDb db;
//    EXPECT_SUCCESS(OpenOrCreateTestDb(db));
//
//    EXPECT_SUCCESS(db.InsertNewRecord(2000, 3000));
//    EXPECT_EQ(1, db.GetRecordCount());
//    }

//TEST_F(UsageDbTests, GetRecordCount_MultipleRecords_ReturnsCorrectCount)
//    {
//    UsageDb db;
//    EXPECT_SUCCESS(OpenOrCreateTestDb(db));
//
//    EXPECT_SUCCESS(db.InsertNewRecord(1000, 2000));
//    EXPECT_SUCCESS(db.InsertNewRecord(2000, 3000));
//    EXPECT_SUCCESS(db.InsertNewRecord(4000, 5000));
//
//    EXPECT_EQ(3, db.GetRecordCount());
//    }

//TEST_F(UsageDbTests, WriteUsageToSCVFile_NoRecords_CreatesCorrectFile)
//    {
//    UsageDb db;
//    EXPECT_SUCCESS(OpenOrCreateTestDb(db));
//
//    BeFileName tmpFile;
//    BeTest::GetHost().GetTempDir(tmpFile);
//    tmpFile.AppendToPath(L"test.scv");
//    EXPECT_SUCCESS(db.WriteUsageToSCVFile(tmpFile));
//
//    Utf8String expectedContent = "StartTime,EndTime";
//    auto fileContent = ReadAllFile(tmpFile);
//    EXPECT_EQ(expectedContent, fileContent);
//    }

//TEST_F(UsageDbTests, WriteUsageToSCVFile_MultipleRecords_CreatesCorrectFile)
//    {
//    UsageDb db;
//    EXPECT_SUCCESS(OpenOrCreateTestDb(db));
//
//    EXPECT_SUCCESS(db.InsertNewRecord(1000, 2000));
//    EXPECT_SUCCESS(db.InsertNewRecord(2000, 3000));
//    EXPECT_SUCCESS(db.InsertNewRecord(4000, 5000));
//
//    BeFileName tmpFile;
//    BeTest::GetHost().GetTempDir(tmpFile);
//    tmpFile.AppendToPath(L"test.scv");
//    EXPECT_SUCCESS(db.WriteUsageToSCVFile(tmpFile));
//
//    Utf8String expectedContent = "StartTime,EndTime\n"
//                                  "1000,2000\n"
//                                  "2000,3000\n"
//                                  "4000,5000";
//    auto fileContent = ReadAllFile(tmpFile);
// 
//    EXPECT_EQ(expectedContent, fileContent);
//    }

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
