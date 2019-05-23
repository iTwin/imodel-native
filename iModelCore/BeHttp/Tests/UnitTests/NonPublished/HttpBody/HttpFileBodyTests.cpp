/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "../WebTestsHelper.h"
#include "../FSTest.h"

USING_NAMESPACE_BENTLEY_HTTP_UNIT_TESTS

struct HttpFileBodyTests : public ::testing::Test {};

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, Open_NonExistingFile_CreatesEmptyFile)
    {
    auto path = FSTest::StubFilePath();
    auto body = HttpFileBody::Create(path);

    EXPECT_FALSE(path.DoesPathExist());
    body->Open();
    EXPECT_TRUE(path.DoesPathExist());

    EXPECT_EQ("", FSTest::ReadFile(path));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, Open_FileWithContent_DoesNothing)
    {
    auto path = FSTest::StubFile("ABC");
    auto body = HttpFileBody::Create(path);

    body->Open();

    EXPECT_EQ("ABC", FSTest::ReadFile(path));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, Open_FileLockedForWrite_DoesNothing)
    {
    auto path = FSTest::StubFile("ABC");
    ASSERT_EQ(BeFileNameStatus::Success, path.SetFileReadOnly(true));
    auto body = HttpFileBody::Create(path);

    body->Open();

    ASSERT_TRUE(path.IsFileReadOnly());
    EXPECT_EQ("ABC", FSTest::ReadFile(path));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, SetPosition_NonExistingFile_FilePositionSet)
    {
    auto path = FSTest::StubFilePath();
    auto body = HttpFileBody::Create(path);

    ASSERT_EQ(SUCCESS, body->SetPosition(10));

    uint64_t position = 0;
    ASSERT_EQ(SUCCESS, body->GetPosition(position));
    EXPECT_EQ(10, position);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, SetPosition_EmptyFile_FilePositionSet)
    {
    auto path = FSTest::StubFile("");
    auto body = HttpFileBody::Create(path);

    ASSERT_EQ(SUCCESS, body->SetPosition(10));

    uint64_t position = 0;
    ASSERT_EQ(SUCCESS, body->GetPosition(position));
    EXPECT_EQ(10, position);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, SetPosition_PositionToContent_FilePositionSet)
    {
    auto path = FSTest::StubFile("ABCD");
    auto body = HttpFileBody::Create(path);

    ASSERT_EQ(SUCCESS, body->SetPosition(2));

    uint64_t position = 0;
    ASSERT_EQ(SUCCESS, body->GetPosition(position));
    EXPECT_EQ(2, position);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, Reset_NonExistingFile_SucceedsAndDoesNothing)
    {
    auto path = FSTest::StubFilePath();
    auto body = HttpFileBody::Create(path);

    ASSERT_EQ(SUCCESS, body->Reset());
    EXPECT_FALSE(path.DoesPathExist());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, Reset_FileWithContent_DeletesFile)
    {
    auto path = FSTest::StubFile("TestContent");
    auto body = HttpFileBody::Create(path);

    ASSERT_EQ(SUCCESS, body->Reset());
    EXPECT_FALSE(path.DoesPathExist());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, Reset_PositionSet_ClearsPosition)
    {
    auto body = HttpFileBody::Create(FSTest::StubFilePath());
    body->SetPosition(10);

    ASSERT_EQ(SUCCESS, body->Reset());

    uint64_t pos = 0;
    body->GetPosition(pos);
    EXPECT_EQ(0, pos);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, Reset_NoFileAtPath_Succeeds)
    {
    auto path = FSTest::StubFilePath();
    EXPECT_FALSE(path.DoesPathExist());

    auto body = HttpFileBody::Create(path);
    ASSERT_EQ(SUCCESS, body->Reset());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, Write_NonExistingFile_CreatesFileAndWritesContent)
    {
    auto path = FSTest::StubFilePath();
    auto body = HttpFileBody::Create(path);
    body->Open();

    ASSERT_EQ(3, body->Write("ABC", 3));
    EXPECT_EQ("ABC", FSTest::ReadFile(path));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, Write_FileWithContent_OverridesContentAtPosition)
    {
    auto path = FSTest::StubFile("ABC");
    auto body = HttpFileBody::Create(path);
    body->Open();

    ASSERT_EQ(1, body->Write("D", 1));
    EXPECT_EQ("DBC", FSTest::ReadFile(path));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
// N.B. Some builds, such as those in Linux containers, run as root, and as such, it is difficult to create a path you cannot access... I feel it is not worth hacking around that fact.
#if !defined(BENTLEYCONFIG_OS_LINUX)
TEST_F(HttpFileBodyTests, Write_FileLockedForWrite_WritesNothing)
    {
    auto path = FSTest::StubFile("ABC");
    ASSERT_EQ(BeFileNameStatus::Success, path.SetFileReadOnly(true));

    auto body = HttpFileBody::Create(path);
    BeTest::SetFailOnAssert(false);
    body->Open();
    EXPECT_EQ(0, body->Write("D", 1));
    BeTest::SetFailOnAssert(true);

    EXPECT_EQ("ABC", FSTest::ReadFile(path));
    }
#endif

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, Write_FileWithContentWithoutOpen_OverridesContentAtPosition)
    {
    auto path = FSTest::StubFile("ABC");
    auto body = HttpFileBody::Create(path);

    ASSERT_EQ(1, body->Write("D", 1));
    EXPECT_EQ("DBC", FSTest::ReadFile(path));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, Write_FileWithContentAndPositionSet_OverridesContentAtPosition)
    {
    auto path = FSTest::StubFile("ABC");
    auto body = HttpFileBody::Create(path);
    body->Open();

    ASSERT_EQ(1, body->Write("D", 1));
    EXPECT_EQ("DBC", FSTest::ReadFile(path));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, Write_AfterResetCalled_Succeeds)
    {
    auto path = FSTest::StubFile("A");
    auto body = HttpFileBody::Create(path);
    ASSERT_EQ(SUCCESS, body->Reset());

    ASSERT_EQ(1, body->Write("B", 1));
    EXPECT_EQ("B", FSTest::ReadFile(path));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, Write_MultipleTimes_IncrementsPosition)
    {
    auto path = FSTest::StubFile("ABC");
    auto body = HttpFileBody::Create(path);
    body->Open();

    uint64_t position = 0;
    ASSERT_EQ(SUCCESS, body->GetPosition(position));
    EXPECT_EQ(0, position);

    ASSERT_EQ(2, body->Write("DE", 2));
    ASSERT_EQ(SUCCESS, body->GetPosition(position));
    EXPECT_EQ(2, position);
    EXPECT_EQ("DEC", FSTest::ReadFile(path));

    ASSERT_EQ(2, body->Write("FG", 2));
    ASSERT_EQ(SUCCESS, body->GetPosition(position));
    EXPECT_EQ(4, position);
    EXPECT_EQ("DEFG", FSTest::ReadFile(path));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, Write_PositionSetToContent_OverridesCotentAtPosition)
    {
    auto path = FSTest::StubFile("ABCD");
    auto body = HttpFileBody::Create(path);
    body->Open();

    ASSERT_EQ(SUCCESS, body->SetPosition(1));
    ASSERT_EQ(2, body->Write("FE", 2));
    EXPECT_EQ("AFED", FSTest::ReadFile(path));

    uint64_t position = 0;
    ASSERT_EQ(SUCCESS, body->GetPosition(position));
    EXPECT_EQ(3, position);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, Read_NonExistingFile_ReadsNothing)
    {
    auto path = FSTest::StubFilePath();
    auto body = HttpFileBody::Create(path);
    body->Open();

    char buffer[32] = "";
    EXPECT_EQ(0, body->Read(buffer, 32));
    EXPECT_STREQ("", buffer);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, Read_EmptyFile_ReadsNothing)
    {
    auto path = FSTest::StubFile("");
    auto body = HttpFileBody::Create(path);
    body->Open();

    char buffer[32] = "";
    EXPECT_EQ(0, body->Read(buffer, 32));
    EXPECT_STREQ("", buffer);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, Read_FileWithContent_ReadsContent)
    {
    auto path = FSTest::StubFile("ABC");
    auto body = HttpFileBody::Create(path);
    body->Open();

    char buffer[32] = "";
    ASSERT_EQ(3, body->Read(buffer, 32));
    EXPECT_STREQ("ABC", buffer);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, Read_FileWithContentWithoutOpen_ReadsNothing)
    {
    auto path = FSTest::StubFile("ABC");
    auto body = HttpFileBody::Create(path);

    char buffer[32] = "";
    ASSERT_EQ(0, body->Read(buffer, 32));
    EXPECT_STREQ("", buffer);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, Read_FileWithContentAfterReset_ReadsNothing)
    {
    auto path = FSTest::StubFile("ABC");
    auto body = HttpFileBody::Create(path);
    ASSERT_EQ(SUCCESS, body->Reset());

    char buffer[32] = "";
    ASSERT_EQ(0, body->Read(buffer, 32));
    EXPECT_STREQ("", buffer);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, Read_FileLockedForWrite_ReadsContent)
    {
    auto path = FSTest::StubFile("ABC");
    ASSERT_EQ(BeFileNameStatus::Success, path.SetFileReadOnly(true));

    auto body = HttpFileBody::Create(path);
    body->Open();

    char buffer[32] = "";
    ASSERT_EQ(3, body->Read(buffer, 32));
    EXPECT_STREQ("ABC", buffer);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, Read_MultipleTimes_IncrementsPosition)
    {
    auto path = FSTest::StubFile("ABC");
    auto body = HttpFileBody::Create(path);
    body->Open();

    char buffer[32] = "";
    uint64_t position = 0;

    ASSERT_EQ(SUCCESS, body->GetPosition(position));
    EXPECT_EQ(0, position);

    ASSERT_EQ(2, body->Read(buffer, 2));
    EXPECT_STREQ("AB", buffer);
    ASSERT_EQ(SUCCESS, body->GetPosition(position));
    EXPECT_EQ(2, position);

    std::fill_n(buffer, 32, (char)0);
    ASSERT_EQ(1, body->Read(buffer, 2));
    EXPECT_STREQ("C", buffer);
    ASSERT_EQ(SUCCESS, body->GetPosition(position));
    EXPECT_EQ(3, position);

    ASSERT_EQ(0, body->Read(buffer, 2));
    ASSERT_EQ(SUCCESS, body->GetPosition(position));
    EXPECT_EQ(3, position);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, Read_PositionSetToContent_ReadsContentAtPosition)
    {
    auto path = FSTest::StubFile("ABCD");
    auto body = HttpFileBody::Create(path);
    body->Open();

    ASSERT_EQ(SUCCESS, body->SetPosition(1));
    char buffer[32] = "";
    ASSERT_EQ(2, body->Read(buffer, 2));
    EXPECT_STREQ("BC", buffer);

    uint64_t position = 0;
    ASSERT_EQ(SUCCESS, body->GetPosition(position));
    EXPECT_EQ(3, position);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, GetLength_NonExistingFile_ReturnsZero)
    {
    auto path = FSTest::StubFilePath();
    auto body = HttpFileBody::Create(path);
    EXPECT_EQ(0, body->GetLength());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, GetLength_EmptyFile_ReturnsZero)
    {
    auto path = FSTest::StubFile("");
    auto body = HttpFileBody::Create(path);
    EXPECT_EQ(0, body->GetLength());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, GetLength_FileWithContentWithoutOpen_ReturnsLength)
    {
    auto path = FSTest::StubFile("ABC");
    auto body = HttpFileBody::Create(path);
    EXPECT_EQ(3, body->GetLength());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, GetLength_FileWithContentAfterOpen_ReturnsLength)
    {
    auto path = FSTest::StubFile("ABC");
    auto body = HttpFileBody::Create(path);
    body->Open();
    EXPECT_EQ(3, body->GetLength());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, GetLength_FileWithContentAfterReset_ReturnsZero)
    {
    auto path = FSTest::StubFile("ABC");
    auto body = HttpFileBody::Create(path);
    ASSERT_EQ(SUCCESS, body->Reset());
    EXPECT_EQ(0, body->GetLength());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, GetLength_FileWithContentAfterWrite_ReturnsNewLength)
    {
    auto path = FSTest::StubFile("ABC");
    auto body = HttpFileBody::Create(path);
    body->Open();

    ASSERT_EQ(3, body->GetLength());
    ASSERT_EQ(6, body->Write("NewNew", 6));
    EXPECT_EQ(6, body->GetLength());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, GetLength_FileLockedForWrite_ReturnsLength)
    {
    auto path = FSTest::StubFile("ABC");
    ASSERT_EQ(BeFileNameStatus::Success, path.SetFileReadOnly(true));

    auto body = HttpFileBody::Create(path);
    EXPECT_EQ(3, body->GetLength());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, AsString_NoFileAtPath_ReturnsEmptyString)
    {
    auto body = HttpFileBody::Create(FSTest::StubFilePath());
    EXPECT_EQ("", body->AsString());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpFileBodyTests, AsString_FileWithContentAtPath_ReturnsContents)
    {
    auto body = HttpFileBody::Create(FSTest::StubFile("TestContent"));
    EXPECT_EQ("TestContent", body->AsString());
    }
