/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/Util/FileUtilTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "FileUtilTests.h"

#include <WebServices/Cache/Util/FileUtil.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

using namespace ::testing;
using std::shared_ptr;

TEST_F(FileUtilTests, TruncateFileName_VeryLongName_TruncatesNameThatFitsIntoBeFileNamePreservingExtension)
    {
    Utf8String fileName = Utf8String(10000, 'x') + ".test";

    Utf8String truncatedFileName = FileUtil::TruncateFileName(fileName);
    EXPECT_FALSE(truncatedFileName.empty());
    EXPECT_LT(truncatedFileName.size(), fileName.size());

    BeFileName path(truncatedFileName);
    EXPECT_NE(0, path.size());
    EXPECT_STREQ(L"test", path.GetExtension().c_str());
    }

TEST_F(FileUtilTests, TruncateFileName_VeryLongNameWithoutExtension_TruncatesNameThatFitsIntoBeFileName)
    {
    Utf8String fileName(10000, 'x');

    Utf8String truncatedFileName = FileUtil::TruncateFileName(fileName);
    EXPECT_FALSE(truncatedFileName.empty());
    EXPECT_LT(truncatedFileName.size(), fileName.size());

    EXPECT_NE(0, BeFileName(truncatedFileName).size());
    }

TEST_F(FileUtilTests, TruncateFileNameUtf8_NameLengthEqualToMax_LeavesSameLength)
    {
    Utf8String a = "a.b";
    Utf8String b = FileUtil::TruncateFileNameUtf8(a, 3);
    EXPECT_STREQ(a.c_str(), b.c_str());
    }

TEST_F(FileUtilTests, TruncateFileNameUtf16_NameLengthEqualToMax_LeavesSameLength)
    {
    Utf8String a = "a.b";
    Utf8String b = FileUtil::TruncateFileNameUtf16(a, 3);
    EXPECT_STREQ(a.c_str(), b.c_str());
    }

TEST_F(FileUtilTests, TruncateFileName_NameLengthSmallerThanMax_LeavesSameLength)
    {
    Utf8String a = "a.b";
    Utf8String b = FileUtil::TruncateFileName(a);
    EXPECT_STREQ(a.c_str(), b.c_str());
    }

TEST_F(FileUtilTests, TruncateFileNameUtf8_Empty_ReturnsEmpty)
    {
    EXPECT_TRUE(FileUtil::TruncateFileNameUtf8("", 10).empty());
    }

TEST_F(FileUtilTests, TruncateFileNameUtf16_Empty_ReturnsEmpty)
    {
    EXPECT_TRUE(FileUtil::TruncateFileNameUtf16("", 10).empty());
    }

TEST_F(FileUtilTests, TruncateFileNameUtf8_VariousLongNames_TruncatesNameToSameLength)
    {
    EXPECT_STREQ("xxxxxxxxxx", FileUtil::TruncateFileNameUtf8(Utf8String(10000, 'x'), 10).c_str());
    EXPECT_STREQ("xxxxx.test", FileUtil::TruncateFileNameUtf8(Utf8String(10000, 'x') + ".test", 10).c_str());
    EXPECT_STREQ("xxxxxxxxxx", FileUtil::TruncateFileNameUtf8(Utf8String(10000, 'x') + "." + Utf8String(10000, 'e'), 10).c_str());
    EXPECT_STREQ(".eeeeeeeee", FileUtil::TruncateFileNameUtf8("." + Utf8String(10000, 'e'), 10).c_str());
    }

TEST_F(FileUtilTests, TruncateFileNameUtf16_VariousLongNames_TruncatesNameToSameLength)
    {
    EXPECT_STREQ("xxxxxxxxxx", FileUtil::TruncateFileNameUtf16(Utf8String(10000, 'x'), 10).c_str());
    EXPECT_STREQ("xxxxx.test", FileUtil::TruncateFileNameUtf16(Utf8String(10000, 'x') + ".test", 10).c_str());
    EXPECT_STREQ("xxxxxxxxxx", FileUtil::TruncateFileNameUtf16(Utf8String(10000, 'x') + "." + Utf8String(10000, 'e'), 10).c_str());
    EXPECT_STREQ(".eeeeeeeee", FileUtil::TruncateFileNameUtf16("." + Utf8String(10000, 'e'), 10).c_str());
    }

TEST_F(FileUtilTests, TruncateFileNameUtf8_UnicodeCharacterInName_TruncatesToBytes)
    {
    Utf8String a(L"\u0105bc.txt"); // Unicode character of 2 bytes, total 7 characters or 8 bytes
    Utf8String b(L"\u0105.txt");   // Expect to remove 2 characters leaving 6 bytes
    Utf8String c = FileUtil::TruncateFileNameUtf8(a, 6);
    EXPECT_STREQ(b.c_str(), c.c_str());
    }


TEST_F(FileUtilTests, TruncateFileNameUtf16_UnicodeCharacterInName_TruncatesToCharacters)
    {
    Utf8String a(L"\u0105bc.txt"); // Unicode character of 2 bytes, total 7 characters or 8 bytes
    Utf8String b(L"\u0105b.txt");  // Expect to remove 1 character leaving 6 characters
    Utf8String c = FileUtil::TruncateFileNameUtf16(a, 6);
    EXPECT_STREQ(b.c_str(), c.c_str());
    }

TEST_F(FileUtilTests, TruncateFileNameUtf8_UnicodeCharacterInNameAndTrimToHalfUnicodeCharacter_TruncatesToBytesPreservingValidCharacters)
    {
    Utf8String a(L"a\u0105b.txt"); // Unicode character of 2 bytes, total 7 characters or 8 bytes
    Utf8String b(L"a.txt");        // Expect to remove 2 characters characters leaving 4 bytes
    Utf8String c = FileUtil::TruncateFileNameUtf8(a, 5);
    EXPECT_STREQ(b.c_str(), c.c_str());
    }

TEST_F(FileUtilTests, TruncateFileName_VariousLongNames_TruncatesNameToSameLength)
    {
    Utf8String a = FileUtil::TruncateFileName(Utf8String(10000, 'x'));
    Utf8String b = FileUtil::TruncateFileName(Utf8String(10000, 'x') + ".test");
    Utf8String c = FileUtil::TruncateFileName(Utf8String(10000, 'x') + "." + Utf8String(10000, 'e'));
    Utf8String d = FileUtil::TruncateFileName("." + Utf8String(10000, 'e'));
    EXPECT_FALSE(a.empty());
    EXPECT_EQ(a.length(), b.length());
    EXPECT_EQ(a.length(), c.length());
    EXPECT_EQ(a.length(), d.length());
    }

TEST_F(FileUtilTests, TruncateFilePath_MaxPathMoreThanPath_DoesNothing)
    {
    BeFileName path(L"\\Foo\\Bar.txt");
    BentleyStatus status = FileUtil::TruncateFilePath(path, 100);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_STREQ(L"\\Foo\\Bar.txt", path.c_str());
    }

TEST_F(FileUtilTests, TruncateFilePath_MaxPathLessThanPath_TruncatesFileName)
    {
    BeFileName path(L"\\A\\Bar.txt");
    BentleyStatus status = FileUtil::TruncateFilePath(path, path.length() - 1);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_STREQ(L"\\A\\Ba.txt", path.c_str());
    }

TEST_F(FileUtilTests, TruncateFilePath_NoFolders_TruncatesFileName)
    {
    BeFileName path(L"Bar.txt");
    BentleyStatus status = FileUtil::TruncateFilePath(path, path.length() - 1);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_STREQ(L"Ba.txt", path.c_str());
    }

TEST_F(FileUtilTests, TruncateFilePath_PathIsFolder_Error)
    {
    BeFileName path(L"\\Foo\\Bar\\");
    BentleyStatus status = FileUtil::TruncateFilePath(path, path.length() - 1);

    EXPECT_EQ(ERROR, status);
    EXPECT_STREQ(L"\\Foo\\Bar\\", path.c_str());
    }

TEST_F(FileUtilTests, TruncateFilePath_FileWithNoExtension_TruncatesFileName)
    {
    BeFileName path(L"\\Foo\\Bar");
    BentleyStatus status = FileUtil::TruncateFilePath(path, path.length() - 1);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_STREQ(L"\\Foo\\Ba", path.c_str());
    }

TEST_F(FileUtilTests, TruncateFilePath_MaxPathSoSmallThatFileNameWouldBeEmpty_Error)
    {
    BeFileName path(L"\\A\\B.txt");
    BentleyStatus status = FileUtil::TruncateFilePath(path, path.length() - 1);

    EXPECT_EQ(ERROR, status);
    EXPECT_STREQ(L"\\A\\B.txt", path.c_str());
    }

TEST_F(FileUtilTests, TruncateFilePath_MaxPathSoSmallThatExtensionWouldBeChanged_Error)
    {
    BeFileName path(L"\\A\\B.txt");
    BentleyStatus status = FileUtil::TruncateFilePath(path, path.length() - 2);

    EXPECT_EQ(ERROR, status);
    EXPECT_STREQ(L"\\A\\B.txt", path.c_str());
    }

TEST_F(FileUtilTests, TruncateFilePath_MaxPathSmallerThanDirectory_Error)
    {
    BeFileName path(L"\\Folder\\Bar.txt");
    BentleyStatus status = FileUtil::TruncateFilePath(path, 3);

    EXPECT_EQ(ERROR, status);
    EXPECT_STREQ(L"\\Folder\\Bar.txt", path.c_str());
    }

TEST_F(FileUtilTests, CopyFileContent_FilesExist_CopiesContent)
    {
    auto source = FSTest::StubFile("A");
    auto target = FSTest::StubFile("B");

    ASSERT_EQ(SUCCESS, FileUtil::CopyFileContent(source, target));

    EXPECT_EQ("A", FSTest::ReadFile(source));
    EXPECT_EQ("A", FSTest::ReadFile(target));
    }

TEST_F(FileUtilTests, CopyFileContent_TargetDoesNotExist_CopiesContent)
    {
    auto source = FSTest::StubFile("A");
    auto target = FSTest::StubFilePath();

    ASSERT_FALSE(target.DoesPathExist());

    ASSERT_EQ(SUCCESS, FileUtil::CopyFileContent(source, target));

    EXPECT_EQ("A", FSTest::ReadFile(source));
    EXPECT_EQ("A", FSTest::ReadFile(target));
    }

TEST_F(FileUtilTests, CopyFileContent_SourceDoesNotExist_Error)
    {
    auto source = FSTest::StubFilePath();
    auto target = FSTest::StubFilePath();

    ASSERT_FALSE(source.DoesPathExist());

    ASSERT_EQ(ERROR, FileUtil::CopyFileContent(source, target));

    ASSERT_FALSE(target.DoesPathExist());
    }

TEST_F(FileUtilTests, CopyFileContent_SourceIsLargeFile_CopiesContents)
    {
    auto size = 1024 * 100;
    auto source = FSTest::StubFileWithSize(size);
    auto target = FSTest::StubFilePath();

    ASSERT_EQ(SUCCESS, FileUtil::CopyFileContent(source, target));

    EXPECT_EQ(size, FileUtil::GetFileSize(target));
    }
