/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/HttpBody/HttpFileBodyTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "HttpFileBodyTests.h"

USING_NAMESPACE_BENTLEY_UNIT_TESTS

TEST_F (HttpFileBodyTests, AsString_NoFileAtPath_ReturnsEmptyString)
    {
    auto body = HttpFileBody::Create (FSTest::StubFilePath ());
    EXPECT_EQ ("", body->AsString ());
    }

TEST_F (HttpFileBodyTests, AsString_FileWithContentAtPath_ReturnsContents)
    {
    auto body = HttpFileBody::Create (FSTest::StubFile ("TestContent"));
    EXPECT_EQ ("TestContent", body->AsString ());
    }

TEST_F (HttpFileBodyTests, SetPosition_ValuePassed_PositionSet)
    {
    auto body = HttpFileBody::Create (FSTest::StubFilePath ());
    body->SetPosition (10);

    uint64_t pos = 0;
    body->GetPosition (pos);
    EXPECT_EQ (10, pos);
    }

TEST_F (HttpFileBodyTests, Reset_FilledBody_DeletesFile)
    {
    auto path = FSTest::StubFile ("TestContent");
    EXPECT_TRUE (path.DoesPathExist ());

    auto body = HttpFileBody::Create (path);

    ASSERT_EQ (SUCCESS, body->Reset ());
    EXPECT_EQ ("", body->AsString ());
    EXPECT_FALSE (path.DoesPathExist ());
    }

TEST_F (HttpFileBodyTests, Reset_PositionSet_ClearsPosition)
    {
    auto body = HttpFileBody::Create (FSTest::StubFilePath ());
    body->SetPosition (10);

    ASSERT_EQ (SUCCESS, body->Reset ());

    uint64_t pos = 0;
    body->GetPosition (pos);
    EXPECT_EQ (0, pos);
    }

TEST_F (HttpFileBodyTests, Reset_NoFileAtPath_Succeeds)
    {
    auto path = FSTest::StubFilePath ();
    EXPECT_FALSE (path.DoesPathExist ());

    auto body = HttpFileBody::Create (path);
    ASSERT_EQ (SUCCESS, body->Reset ());
    }

TEST_F (HttpFileBodyTests, Write_AfterResetCalled_Succeeds)
    {
    auto path = FSTest::StubFile ("A");
    auto body = HttpFileBody::Create (path);
    body->Reset ();

    ASSERT_EQ (1, body->Write ("B", 1));

    EXPECT_EQ ("B", body->AsString ());
    EXPECT_EQ ("B", FSTest::ReadFile (path));
    }

TEST_F(HttpFileBodyTests, GetPosition_NotExistingFile_CreatesEmptyFileAndReturnsZero)
    {
    auto path = FSTest::StubFilePath();
    auto body = HttpFileBody::Create(path);

    EXPECT_FALSE(path.DoesPathExist());

    uint64_t position = 100;
    EXPECT_EQ(SUCCESS, body->GetPosition(position));

    EXPECT_EQ(0, position);
    EXPECT_TRUE(path.DoesPathExist());
    uint64_t size = 100;
    path.GetFileSize(size);
    EXPECT_EQ(0, size);
    }

TEST_F(HttpFileBodyTests, GetPosition_ExistingFile_ReturnsZeroWithoutAffectingFile)
    {
    auto path = FSTest::StubFile("A");
    auto body = HttpFileBody::Create(path);

    EXPECT_EQ("A", FSTest::ReadFile(path));

    uint64_t position = 100;
    EXPECT_EQ(SUCCESS, body->GetPosition(position));

    EXPECT_EQ(0, position);
    EXPECT_EQ("A", FSTest::ReadFile(path));
    }

TEST_F(HttpFileBodyTests, SetPosition_NotExistingFile_CreatesEmptyFile)
    {
    auto path = FSTest::StubFilePath();
    auto body = HttpFileBody::Create(path);

    EXPECT_FALSE(path.DoesPathExist());

    EXPECT_EQ(SUCCESS, body->SetPosition(2));

    EXPECT_TRUE(path.DoesPathExist());

    uint64_t size = 100;
    path.GetFileSize(size);
    EXPECT_EQ(0, size);
    }

TEST_F(HttpFileBodyTests, SetPosition_ExistingFileAndPosition_ReturnsZeroWithoutAffectingFile)
    {
    auto path = FSTest::StubFile("ABCD");
    auto body = HttpFileBody::Create(path);

    EXPECT_EQ(SUCCESS, body->SetPosition(2));

    EXPECT_EQ("ABCD", FSTest::ReadFile(path));

    uint64_t size = 100;
    path.GetFileSize(size);
    EXPECT_EQ(4, size);
    }
