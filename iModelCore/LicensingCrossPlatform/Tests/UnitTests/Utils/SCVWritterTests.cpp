/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Utils/SCVWritterTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "SCVWritterTests.h"

#include <Licensing\Utils\SCVWritter.h>

USING_NAMESPACE_BENTLEY_LICENSING

USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS

Utf8String ReadFile(BeFileNameCR filePath)
    {
    bvector<Byte> fileContents;

    BeFile file;
    BeFileStatus status;

    status = file.Open(filePath, BeFileAccess::Read);
    EXPECT_EQ(BeFileStatus::Success, status);

    status = file.ReadEntireFile(fileContents);
    EXPECT_EQ(BeFileStatus::Success, status);

    status = file.Close();
    EXPECT_EQ(BeFileStatus::Success, status);

    Utf8String stringContents;
    stringContents.append(fileContents.begin(), fileContents.end());
    return stringContents;
    }

TEST_F(SCVWritterTests, AddRow_OneRowWithOneItem)
    {
    SCVWritter writter;
    writter.AddRow("v1");

    EXPECT_EQ("v1", writter.ToString());
    }

TEST_F(SCVWritterTests, AddRow_OneRowWithMultipleItems)
    {
    SCVWritter writter;
    writter.AddRow("v1", 66, "v3");

    EXPECT_EQ("v1,66,v3", writter.ToString());
    }

TEST_F(SCVWritterTests, AddRow_MultipleRowsWithMultipleItems)
    {
    SCVWritter writter;
    writter.AddRow("v1", 66, "v3");
    writter.AddRow("v2", 99, "v4");

    EXPECT_EQ("v1,66,v3\nv2,99,v4", writter.ToString());
    }

TEST_F(SCVWritterTests, WriteToFile_MultipleRows_WritesEverything)
    {
    SCVWritter writter;
    writter.AddRow("v1", 66, "v3");
    writter.AddRow("v2", 99, "v4");

    BeFileName filePath;
    BeTest::GetHost().GetTempDir(filePath);
    filePath.AppendToPath(L"TestFile.scv");

    EXPECT_SUCCESS(writter.WriteToFile(filePath));

    EXPECT_EQ("v1,66,v3\nv2,99,v4", ReadFile(filePath));
    }