/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "BeSQLiteNonPublishedTests.h"
#include <BeSQLite/TempFile.h>
#include <Bentley/BeDirectoryIterator.h>

//---------------------------------------------------------------------------------------
// Data written at any offset reads back, reading past the end is a short read, and nothing
// named like a SQLite temp file is left in the temp directory once the file is closed.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(TempFile, RoundTripAndCleanup)
    {
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    BeSQLiteLib::Initialize(tempDir);
    auto countTempFiles = [&]()
        {
        bvector<BeFileName> files;
        BeDirectoryIterator::WalkDirsAndMatch(files, tempDir, L"etilqs_*", false);
        return files.size();
        };
    const size_t before = countTempFiles();

    TempFile file;
    ASSERT_EQ(BE_SQLITE_OK, file.Open());
    ASSERT_TRUE(file.IsOpen());
    EXPECT_EQ(BE_SQLITE_MISUSE, file.Open());

    std::vector<Byte> first(100000), second(5000);
    for (size_t i = 0; i < first.size(); ++i)
        first[i] = (Byte) (i * 7);
    for (size_t i = 0; i < second.size(); ++i)
        second[i] = (Byte) (i * 13 + 1);
    ASSERT_EQ(BE_SQLITE_OK, file.Write(first.data(), first.size(), 0));
    ASSERT_EQ(BE_SQLITE_OK, file.Write(second.data(), second.size(), first.size()));

    std::vector<Byte> readBack(second.size());
    ASSERT_EQ(BE_SQLITE_OK, file.Read(readBack.data(), readBack.size(), first.size()));
    EXPECT_TRUE(readBack == second);
    readBack.resize(first.size());
    ASSERT_EQ(BE_SQLITE_OK, file.Read(readBack.data(), readBack.size(), 0));
    EXPECT_TRUE(readBack == first);
    EXPECT_EQ(BE_SQLITE_IOERR_SHORT_READ, file.Read(readBack.data(), 10, first.size() + second.size() - 5));

#if !defined(BENTLEYCONFIG_OS_WINDOWS)
    // Unix removes the name as soon as the file is opened, so a killed process leaves nothing behind.
    EXPECT_EQ(before, countTempFiles());
#endif

    file.Close();
    EXPECT_FALSE(file.IsOpen());
    file.Close();
    EXPECT_EQ(BE_SQLITE_MISUSE, file.Write(first.data(), 1, 0));
    EXPECT_EQ(before, countTempFiles());
    }
