/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Cache/Util/FileUtilTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "FileUtilTests.h"

#include <WebServices/Cache/Util/FileUtil.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

using namespace ::testing;
using std::shared_ptr;

// Convert forward slashes to backslashes for tests on Android and iOS
WString FixSlashes(WString path)
    {
    path.ReplaceAll(L"/", L"\\");
    return path;
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FileUtilTests, TruncateFileName_VeryLongNameWithoutExtension_TruncatesNameThatFitsIntoBeFileName)
    {
    Utf8String fileName(10000, 'x');

    Utf8String truncatedFileName = FileUtil::TruncateFileName(fileName);
    EXPECT_FALSE(truncatedFileName.empty());
    EXPECT_LT(truncatedFileName.size(), fileName.size());

    EXPECT_NE(0, BeFileName(truncatedFileName).size());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FileUtilTests, TruncateFileNameUtf8_NameLengthEqualToMax_LeavesSameLength)
    {
    Utf8String a = "a.b";
    Utf8String b = FileUtil::TruncateFileNameUtf8(a, 3);
    EXPECT_STREQ(a.c_str(), b.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FileUtilTests, TruncateFileNameUtf16_NameLengthEqualToMax_LeavesSameLength)
    {
    Utf8String a = "a.b";
    Utf8String b = FileUtil::TruncateFileNameUtf16(a, 3);
    EXPECT_STREQ(a.c_str(), b.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FileUtilTests, TruncateFileName_NameLengthSmallerThanMax_LeavesSameLength)
    {
    Utf8String a = "a.b";
    Utf8String b = FileUtil::TruncateFileName(a);
    EXPECT_STREQ(a.c_str(), b.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FileUtilTests, TruncateFileNameUtf8_Empty_ReturnsEmpty)
    {
    EXPECT_TRUE(FileUtil::TruncateFileNameUtf8("", 10).empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FileUtilTests, TruncateFileNameUtf16_Empty_ReturnsEmpty)
    {
    EXPECT_TRUE(FileUtil::TruncateFileNameUtf16("", 10).empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FileUtilTests, TruncateFileNameUtf8_VariousLongNames_TruncatesNameToSameLength)
    {
    EXPECT_STREQ("xxxxxxxxxx", FileUtil::TruncateFileNameUtf8(Utf8String(10000, 'x'), 10).c_str());
    EXPECT_STREQ("xxxxx.test", FileUtil::TruncateFileNameUtf8(Utf8String(10000, 'x') + ".test", 10).c_str());
    EXPECT_STREQ("xxxxxxxxxx", FileUtil::TruncateFileNameUtf8(Utf8String(10000, 'x') + "." + Utf8String(10000, 'e'), 10).c_str());
    EXPECT_STREQ(".eeeeeeeee", FileUtil::TruncateFileNameUtf8("." + Utf8String(10000, 'e'), 10).c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FileUtilTests, TruncateFileNameUtf16_VariousLongNames_TruncatesNameToSameLength)
    {
    EXPECT_STREQ("xxxxxxxxxx", FileUtil::TruncateFileNameUtf16(Utf8String(10000, 'x'), 10).c_str());
    EXPECT_STREQ("xxxxx.test", FileUtil::TruncateFileNameUtf16(Utf8String(10000, 'x') + ".test", 10).c_str());
    EXPECT_STREQ("xxxxxxxxxx", FileUtil::TruncateFileNameUtf16(Utf8String(10000, 'x') + "." + Utf8String(10000, 'e'), 10).c_str());
    EXPECT_STREQ(".eeeeeeeee", FileUtil::TruncateFileNameUtf16("." + Utf8String(10000, 'e'), 10).c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FileUtilTests, TruncateFileNameUtf8_VariousEdgeCases_TruncatesNameToSpecification)
    {
    EXPECT_STREQ("f.txt", FileUtil::TruncateFileNameUtf8("foobar.txt", 5).c_str());
    EXPECT_STREQ("foob", FileUtil::TruncateFileNameUtf8("foobar.txt", 4).c_str());
    EXPECT_STREQ("foo", FileUtil::TruncateFileNameUtf8("foobar.txt", 3).c_str());
    EXPECT_STREQ("f", FileUtil::TruncateFileNameUtf8("foobar.txt", 1).c_str());
    EXPECT_STREQ("", FileUtil::TruncateFileNameUtf8("foobar.txt", 0).c_str());
    EXPECT_STREQ("f.ba", FileUtil::TruncateFileNameUtf8("f.ba.txt", 4).c_str());
    EXPECT_STREQ(".", FileUtil::TruncateFileNameUtf8(".", 4).c_str());
    EXPECT_STREQ("", FileUtil::TruncateFileNameUtf8(".", 0).c_str());
    EXPECT_STREQ("", FileUtil::TruncateFileNameUtf8("", 10).c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FileUtilTests, TruncateFileNameUtf16_VariousEdgeCases_TruncatesNameToSpecification)
    {
    EXPECT_STREQ("f.txt", FileUtil::TruncateFileNameUtf16("foobar.txt", 5).c_str());
    EXPECT_STREQ("foob", FileUtil::TruncateFileNameUtf16("foobar.txt", 4).c_str());
    EXPECT_STREQ("foo", FileUtil::TruncateFileNameUtf16("foobar.txt", 3).c_str());
    EXPECT_STREQ("f", FileUtil::TruncateFileNameUtf16("foobar.txt", 1).c_str());
    EXPECT_STREQ("", FileUtil::TruncateFileNameUtf16("foobar.txt", 0).c_str());
    EXPECT_STREQ("f.ba", FileUtil::TruncateFileNameUtf16("f.ba.txt", 4).c_str());
    EXPECT_STREQ(".", FileUtil::TruncateFileNameUtf16(".", 4).c_str());
    EXPECT_STREQ("", FileUtil::TruncateFileNameUtf16(".", 0).c_str());
    EXPECT_STREQ("", FileUtil::TruncateFileNameUtf16("", 10).c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FileUtilTests, TruncateFileNameUtf8_UnicodeCharacterInName_TruncatesToBytes)
    {
    Utf8String a(L"\u0105bc.txt"); // Unicode character of 2 bytes, total 7 characters or 8 bytes
    Utf8String b(L"\u0105.txt");   // Expect to remove 2 characters leaving 6 bytes
    Utf8String c = FileUtil::TruncateFileNameUtf8(a, 6);
    EXPECT_STREQ(b.c_str(), c.c_str());
    }


/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FileUtilTests, TruncateFileNameUtf16_UnicodeCharacterInName_TruncatesToCharacters)
    {
    Utf8String a(L"\u0105bc.txt"); // Unicode character of 2 bytes, total 7 characters or 8 bytes
    Utf8String b(L"\u0105b.txt");  // Expect to remove 1 character leaving 6 characters
    Utf8String c = FileUtil::TruncateFileNameUtf16(a, 6);
    EXPECT_STREQ(b.c_str(), c.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FileUtilTests, TruncateFileNameUtf8_UnicodeCharacterInNameAndTrimToHalfUnicodeCharacter_TruncatesToBytesPreservingValidCharacters)
    {
    Utf8String a(L"a\u0105b.txt"); // Unicode character of 2 bytes, total 7 characters or 8 bytes
    Utf8String b(L"a.txt");        // Expect to remove 2 characters characters leaving 4 bytes
    Utf8String c = FileUtil::TruncateFileNameUtf8(a, 5);
    EXPECT_STREQ(b.c_str(), c.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FileUtilTests, TruncateFilePath_MaxPathMoreThanPath_DoesNothing)
    {
    BeFileName path(L"\\Foo\\Bar.txt");
    BentleyStatus status = FileUtil::TruncateFilePath(path, 100);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_STREQ(L"\\Foo\\Bar.txt", FixSlashes(path).c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FileUtilTests, TruncateFilePath_MaxPathLessThanPath_TruncatesFileName)
    {
    BeFileName path(L"\\A\\Bar.txt");
    BentleyStatus status = FileUtil::TruncateFilePath(path, path.length() - 1);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_STREQ(L"\\A\\Ba.txt", FixSlashes(path).c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FileUtilTests, TruncateFilePath_NoFolders_TruncatesFileName)
    {
    BeFileName path(L"Bar.txt");
    BentleyStatus status = FileUtil::TruncateFilePath(path, path.length() - 1);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_STREQ(L"Ba.txt", path.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FileUtilTests, TruncateFilePath_PathIsFolder_Error)
    {
    BeFileName path(L"\\Foo\\Bar\\");
    BentleyStatus status = FileUtil::TruncateFilePath(path, path.length() - 1);

    EXPECT_EQ(ERROR, status);
    EXPECT_STREQ(L"\\Foo\\Bar\\", FixSlashes(path).c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FileUtilTests, TruncateFilePath_FileWithNoExtension_TruncatesFileName)
    {
    BeFileName path(L"\\Foo\\Bar");
    BentleyStatus status = FileUtil::TruncateFilePath(path, path.length() - 1);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_STREQ(L"\\Foo\\Ba", FixSlashes(path).c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FileUtilTests, TruncateFilePath_MaxPathSoSmallThatFileNameWouldBeEmpty_Error)
    {
    BeFileName path(L"\\A\\B.txt");
    BentleyStatus status = FileUtil::TruncateFilePath(path, path.length() - 1);

    EXPECT_EQ(ERROR, status);
    EXPECT_STREQ(L"\\A\\B.txt", FixSlashes(path).c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FileUtilTests, TruncateFilePath_MaxPathSoSmallThatExtensionWouldBeChanged_Error)
    {
    BeFileName path(L"\\A\\B.txt");
    BentleyStatus status = FileUtil::TruncateFilePath(path, path.length() - 2);

    EXPECT_EQ(ERROR, status);
    EXPECT_STREQ(L"\\A\\B.txt", FixSlashes(path).c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FileUtilTests, TruncateFilePath_MaxPathSmallerThanDirectory_Error)
    {
    BeFileName path(L"\\Folder\\Bar.txt");
    BentleyStatus status = FileUtil::TruncateFilePath(path, 3);

    EXPECT_EQ(ERROR, status);
    EXPECT_STREQ(L"\\Folder\\Bar.txt", FixSlashes(path).c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FileUtilTests, SanitizeFileName_ValidFileName_SameReturned)
    {
    EXPECT_STREQ("Bar.txt", FileUtil::SanitizeFileName("Bar.txt").c_str());
    EXPECT_STREQ(Utf8String(L"\u0444.txt").c_str(), FileUtil::SanitizeFileName(Utf8String(L"\u0444.txt")).c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FileUtilTests, SanitizeFileName_FileNameIncludesInvalidCharacters_InvalidCharactersRemoved)
    {
    EXPECT_STREQ("BarFoo.txt", FileUtil::SanitizeFileName(R"(Bar<>:"/\\|?*Foo.tx<>:"/\\|?*t)").c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FileUtilTests, SanitizeFileName_FileNameHasTrailingWhitespace_WhiteSpaceTrimmed)
    {
    EXPECT_STREQ("B\ta r.t xt", FileUtil::SanitizeFileName(" \t B\ta r.t xt  \t\n\r").c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FileUtilTests, CopyFileContent_FilesExist_CopiesContent)
    {
    auto source = StubFile("A");
    auto target = StubFile("B");

    ASSERT_EQ(SUCCESS, FileUtil::CopyFileContent(source, target));

    EXPECT_EQ("A", SimpleReadFile(source));
    EXPECT_EQ("A", SimpleReadFile(target));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                 Julius.Cepukenas                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FileUtilTests, CopyFileContent_ProgressCallbackExistAndFileExists_RecordsProgress)
    {
    auto size = 100;
    auto source = StubFileWithSize(size);
    auto target = StubFilePath();
    int i = 0;
    auto onProgress = [&] (double bytesTransfered, double bytesTotal)
        {
        if (i == 0)
            {
            //Initial callback to onProgress
            EXPECT_EQ(bytesTotal, bytesTransfered);
            EXPECT_EQ(0, bytesTransfered);
            EXPECT_EQ(0, bytesTotal);
            }
        else
            {
            EXPECT_EQ(bytesTotal, bytesTransfered);
            EXPECT_EQ(100, bytesTransfered);
            EXPECT_EQ(100, bytesTotal);
            }
        i++;
        };

    ASSERT_EQ(SUCCESS, FileUtil::CopyFileContent(source, target, onProgress));
    EXPECT_EQ(4, i);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                 Julius.Cepukenas                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FileUtilTests, CopyFileContent_ProgressCallbackExistAnFilesDoNotExists_DoNotRecordProgress)
    {
    auto source = StubFilePath();
    auto target = StubFilePath();
    int i = 0;
    auto onProgress = [&i] (double bytesTransfered, double bytesTotal)
        {
        i++;
        };

    ASSERT_EQ(ERROR, FileUtil::CopyFileContent(source, target, onProgress));
    EXPECT_EQ(0, i);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                 Julius.Cepukenas                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FileUtilTests, CopyFileContent_CancelationTokenExists_CopiesContent)
    {
    ICancellationTokenPtr ct = SimpleCancellationToken::Create();
    auto source = StubFile();
    auto target = StubFile();

    ASSERT_EQ(SUCCESS, FileUtil::CopyFileContent(source, target, nullptr, ct));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                 Julius.Cepukenas                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FileUtilTests, CopyFileContent_CanceledCancelationTokenExists_Error)
    {
    ICancellationTokenPtr ct = SimpleCancellationToken::Create(true);
    auto source = StubFile();
    auto target = StubFile();

    ASSERT_EQ(ERROR, FileUtil::CopyFileContent(source, target, nullptr, (ICancellationTokenPtr) ct));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FileUtilTests, CopyFileContent_TargetDoesNotExist_CopiesContent)
    {
    auto source = StubFile("A");
    auto target = StubFilePath();

    ASSERT_FALSE(target.DoesPathExist());

    ASSERT_EQ(SUCCESS, FileUtil::CopyFileContent(source, target));

    EXPECT_EQ("A", SimpleReadFile(source));
    EXPECT_EQ("A", SimpleReadFile(target));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FileUtilTests, CopyFileContent_SourceDoesNotExist_Error)
    {
    auto source = StubFilePath();
    auto target = StubFilePath();

    ASSERT_FALSE(source.DoesPathExist());

    ASSERT_EQ(ERROR, FileUtil::CopyFileContent(source, target));

    ASSERT_FALSE(target.DoesPathExist());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FileUtilTests, CopyFileContent_SourceIsLargeFile_CopiesContents)
    {
    auto size = 1024 * 100;
    auto source = StubFileWithSize(size);
    auto target = StubFilePath();

    ASSERT_EQ(SUCCESS, FileUtil::CopyFileContent(source, target));

    EXPECT_EQ(size, FileUtil::GetFileSize(target));
    }
