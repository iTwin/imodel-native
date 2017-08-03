/*--------------------------------------------------------------------------------------+
|
|     $Source: LicensingCrossPlatform/Tests/UnitTests/Published/UsageDbTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

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

TEST_F (UsageDbTests, OpenOrCreate_OpenExistingDb_ContainsInsertedRows)
    {
	UsageDb db;
    EXPECT_SUCCESS(OpenOrCreateTestDb(db));

    EXPECT_SUCCESS(db.InsertNewRecord(1000, 1000));
    EXPECT_EQ(1000, db.GetLastRecordEndTime());

    EXPECT_SUCCESS(OpenOrCreateTestDb(db));
    EXPECT_EQ(1000, db.GetLastRecordEndTime());

    db.Close();

    EXPECT_SUCCESS(OpenOrCreateTestDb(db));
    EXPECT_EQ(1000, db.GetLastRecordEndTime());
    }

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

TEST_F(UsageDbTests, GetLastRecordEndTime_SingleRecord_ReturnEndTime)
    {
    UsageDb db;
    EXPECT_SUCCESS(OpenOrCreateTestDb(db));

    EXPECT_SUCCESS(db.InsertNewRecord(1000, 2000));
    EXPECT_EQ(2000, db.GetLastRecordEndTime());
    }

TEST_F(UsageDbTests, GetLastRecordEndTime_MultipleRecords_ReturnEndTime)
    {
    UsageDb db;
    EXPECT_SUCCESS(OpenOrCreateTestDb(db));

    EXPECT_SUCCESS(db.InsertNewRecord(1000, 2000));
    EXPECT_SUCCESS(db.InsertNewRecord(3000, 4000));
    EXPECT_SUCCESS(db.InsertNewRecord(5000, 6000));
    EXPECT_EQ(6000, db.GetLastRecordEndTime());
    }

TEST_F(UsageDbTests, UpdateLastRecordEndTime_NoRecords_RetursError)
    {
    UsageDb db;
    EXPECT_SUCCESS(OpenOrCreateTestDb(db));

    EXPECT_ERROR(db.UpdateLastRecordEndTime(6000));
    }

TEST_F(UsageDbTests, UpdateLastRecordEndTime_SingleRecord_UpdatesSuccessfull)
    {
    UsageDb db;
    EXPECT_SUCCESS(OpenOrCreateTestDb(db));

    EXPECT_SUCCESS(db.InsertNewRecord(1000, 2000));

    EXPECT_SUCCESS(db.UpdateLastRecordEndTime(6000));
    EXPECT_EQ(6000, db.GetLastRecordEndTime());
    }

TEST_F(UsageDbTests, UpdateLastRecordEndTime_MultipleRecords_UpdatesLastRecordSuccessfully)
    {
    UsageDb db;
    EXPECT_SUCCESS(OpenOrCreateTestDb(db));

    EXPECT_SUCCESS(db.InsertNewRecord(1000, 2000));
    EXPECT_SUCCESS(db.InsertNewRecord(2000, 3000));
    EXPECT_SUCCESS(db.InsertNewRecord(4000, 5000));
    EXPECT_SUCCESS(db.UpdateLastRecordEndTime(6000));
    EXPECT_EQ(6000, db.GetLastRecordEndTime());
    }

TEST_F(UsageDbTests, GetRecordCount_NoRecords_ReturnsZero)
    {
    UsageDb db;
    EXPECT_SUCCESS(OpenOrCreateTestDb(db));

    EXPECT_EQ(0, db.GetRecordCount());
    }

TEST_F(UsageDbTests, GetRecordCount_SingleRecord_ReturnsOne)
    {
    UsageDb db;
    EXPECT_SUCCESS(OpenOrCreateTestDb(db));

    EXPECT_SUCCESS(db.InsertNewRecord(2000, 3000));
    EXPECT_EQ(1, db.GetRecordCount());
    }

TEST_F(UsageDbTests, GetRecordCount_MultipleRecords_ReturnsCorrectCount)
    {
    UsageDb db;
    EXPECT_SUCCESS(OpenOrCreateTestDb(db));

    EXPECT_SUCCESS(db.InsertNewRecord(1000, 2000));
    EXPECT_SUCCESS(db.InsertNewRecord(2000, 3000));
    EXPECT_SUCCESS(db.InsertNewRecord(4000, 5000));

    EXPECT_EQ(3, db.GetRecordCount());
    }

TEST_F(UsageDbTests, WriteUsageToSCVFile_NoRecords_CreatesCorrectFile)
    {
    UsageDb db;
    EXPECT_SUCCESS(OpenOrCreateTestDb(db));

    BeFileName tmpFile;
    BeTest::GetHost().GetTempDir(tmpFile);
    tmpFile.AppendToPath(L"test.scv");
    EXPECT_SUCCESS(db.WriteUsageToSCVFile(tmpFile));

    Utf8String expectedContent = "StartTime,EndTime";
    auto fileContent = ReadAllFile(tmpFile);
    EXPECT_EQ(expectedContent, fileContent);
    }

TEST_F(UsageDbTests, WriteUsageToSCVFile_MultipleRecords_CreatesCorrectFile)
    {
    UsageDb db;
    EXPECT_SUCCESS(OpenOrCreateTestDb(db));

    EXPECT_SUCCESS(db.InsertNewRecord(1000, 2000));
    EXPECT_SUCCESS(db.InsertNewRecord(2000, 3000));
    EXPECT_SUCCESS(db.InsertNewRecord(4000, 5000));

    BeFileName tmpFile;
    BeTest::GetHost().GetTempDir(tmpFile);
    tmpFile.AppendToPath(L"test.scv");
    EXPECT_SUCCESS(db.WriteUsageToSCVFile(tmpFile));

    Utf8String expectedContent = "StartTime,EndTime\n"
                                  "1000,2000\n"
                                  "2000,3000\n"
                                  "4000,5000";
    auto fileContent = ReadAllFile(tmpFile);
 
    EXPECT_EQ(expectedContent, fileContent);
    }