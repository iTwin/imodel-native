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

TEST_F (UsageDbTests, OpenOrCreate)
    {
	BeFileName tmpDir;
	BeTest::GetHost().GetTempDir(tmpDir);
	tmpDir.AppendToPath(L"TestDb.db");

	UsageDb db;
    EXPECT_SUCCESS(db.OpenOrCreate(tmpDir));

    EXPECT_EQ(0, db.GetTestRecordCount());
    db.TestInsert();
    EXPECT_EQ(1, db.GetTestRecordCount());

    db.Close();

    EXPECT_SUCCESS(db.OpenOrCreate(tmpDir));
    EXPECT_EQ(1, db.GetTestRecordCount());
    }

