/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "BeSQLiteNonPublishedTests.h"
#include "BeSQLite/ChangeSet.h"
#include <vector>
#include <limits>
#include <string>

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
struct SnappyToBlobTests : public ::testing::Test
{
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F (SnappyToBlobTests, WriteGetUncompSize)
{
    SnappyToBlob sblob;
    Byte values[] = {1,0,1,0,0,1};
    Byte* datap = &values[0];
    sblob.Init();
    sblob.Write(datap,6);
    uint32_t size = sblob.GetUnCompressedSize();
    ASSERT_EQ(size,6);
}

//=======================================================================================
// Tests for SnappyFromMemory / SnappyFromBlob handling of malformed (untrusted) input.
// @bsiclass
//=======================================================================================
struct SnappyReaderTests : public ::testing::Test
{
    // SQLite is built with SQLITE_OMIT_AUTOINIT; SnappyToBlob and Db both need BeSQLiteLib initialized.
    void SetUp() override
    {
        BeFileName tempDir;
        BeTest::GetHost().GetTempDir(tempDir);
        BeSQLiteLib::Initialize(tempDir);
    }
    // Malformed input intentionally triggers BeAsserts in the readers.
    struct DisableAsserts
    {
        DisableAsserts() { BeTest::SetFailOnAssert(false); }
        ~DisableAsserts() { BeTest::SetFailOnAssert(true); }
    };

    // Build a raw Snappy stream that declares, and actually expands to, 'declaredLen' bytes of 'A':
    // a 1-byte literal followed by copy ops (2-byte offset of 1) of up to 64 bytes each.
    static bvector<Byte> MakeSnappyStream(uint32_t declaredLen)
    {
        bvector<Byte> out;
        uint32_t v = declaredLen;
        while (v >= 0x80)
        {
            out.push_back((Byte) (v | 0x80));
            v >>= 7;
        }
        out.push_back((Byte) v);

        if (0 == declaredLen)
            return out;

        out.push_back(0x00); // literal tag, length 1
        out.push_back('A');
        uint32_t remaining = declaredLen - 1;
        while (remaining > 0)
        {
            uint32_t n = (remaining > 64) ? 64 : remaining;
            out.push_back((Byte) (((n - 1) << 2) | 2)); // copy with 2-byte offset
            out.push_back(1);
            out.push_back(0);
            remaining -= n;
        }
        return out;
    }

    // Prefix a Snappy stream with the 2-byte little-endian chunk size (which includes the 2 header bytes), as written by SnappyToBlob.
    static bvector<Byte> MakeChunk(bvector<Byte> const& stream)
    {
        uint32_t chunkSize = (uint32_t) stream.size() + 2;
        EXPECT_LE(chunkSize, (uint32_t) std::numeric_limits<uint16_t>::max());
        bvector<Byte> chunk;
        chunk.push_back((Byte) (chunkSize & 0xff));
        chunk.push_back((Byte) ((chunkSize >> 8) & 0xff));
        chunk.insert(chunk.end(), stream.begin(), stream.end());
        return chunk;
    }

    static ZipErrors ReadFromMemory(SnappyFromMemory& reader, bvector<Byte>& blob, bvector<Byte>& out, uint32_t readSize)
    {
        out.resize(readSize);
        uint32_t actuallyRead = 0;
        reader.Init(blob.empty() ? nullptr : blob.data(), (uint32_t) blob.size());
        return reader._Read(out.data(), readSize, actuallyRead);
    }

    static ZipErrors ReadFromMemory(bvector<Byte>& blob, bvector<Byte>& out, uint32_t readSize)
    {
        SnappyFromMemory reader;
        return ReadFromMemory(reader, blob, out, readSize);
    }

    static ZipErrors ReadFromDbBlob(bvector<Byte> const& blob, bvector<Byte>& out, uint32_t readSize)
    {
        Db db;
        Db::CreateParams params;
        params.m_skipFileCheck = true;
        EXPECT_EQ(BE_SQLITE_OK, db.CreateNewDb(BEDB_MemoryDb, params));
        EXPECT_EQ(BE_SQLITE_OK, db.CreateTable("snappyTest", "val BLOB"));

        Statement stmt;
        EXPECT_EQ(BE_SQLITE_OK, stmt.Prepare(db, "INSERT INTO snappyTest (val) VALUES (?)"));
        EXPECT_EQ(BE_SQLITE_OK, stmt.BindBlob(1, blob.data(), (int) blob.size(), Statement::MakeCopy::Yes));
        EXPECT_EQ(BE_SQLITE_DONE, stmt.Step());
        stmt.Finalize();
        int64_t rowId = db.GetLastInsertRowId();

        out.resize(readSize);
        uint32_t actuallyRead = 0;
        ZipErrors stat;
        {
            SnappyFromBlob reader;
            stat = reader.Init(db, "snappyTest", "val", rowId);
            if (ZIP_SUCCESS == stat)
                stat = reader.ReadAndFinish(out.data(), readSize, actuallyRead);
        }
        db.AbandonChanges();
        db.CloseDb();
        return stat;
    }
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SnappyReaderTests, RoundTrip)
{
    bvector<Byte> input(100000);
    for (size_t i = 0; i < input.size(); ++i)
        input[i] = (Byte) ((i * 31) ^ (i >> 7));

    SnappyToBlob writer;
    writer.DoSnappy(input.data(), (uint32_t) input.size());
    ByteStream compressed;
    writer.SaveTo(compressed);
    bvector<Byte> blob(compressed.GetData(), compressed.GetData() + compressed.GetSize());

    bvector<Byte> out;
    ASSERT_EQ(ZIP_SUCCESS, ReadFromMemory(blob, out, (uint32_t) input.size()));
    ASSERT_TRUE(input == out);

    ASSERT_EQ(ZIP_SUCCESS, ReadFromMemory(SnappyFromMemory::GetForThread(), blob, out, (uint32_t) input.size()));
    ASSERT_TRUE(input == out);

    ASSERT_EQ(ZIP_SUCCESS, ReadFromDbBlob(blob, out, (uint32_t) input.size()));
    ASSERT_TRUE(input == out);
}

//---------------------------------------------------------------------------------------
// A hand-crafted chunk whose declared size fits in the uncompressed buffer is accepted.
// This also verifies MakeSnappyStream produces valid Snappy data.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SnappyReaderTests, CraftedChunkAtBufferLimit)
{
    const uint32_t maxSize = SnappyReader::SNAPPY_UNCOMPRESSED_BUFFER_SIZE;
    bvector<Byte> blob = MakeChunk(MakeSnappyStream(maxSize));
    bvector<Byte> expected(maxSize, 'A');

    bvector<Byte> out;
    ASSERT_EQ(ZIP_SUCCESS, ReadFromMemory(blob, out, maxSize));
    ASSERT_TRUE(expected == out);

    ASSERT_EQ(ZIP_SUCCESS, ReadFromMemory(SnappyFromMemory::GetForThread(), blob, out, maxSize));
    ASSERT_TRUE(expected == out);

    ASSERT_EQ(ZIP_SUCCESS, ReadFromDbBlob(blob, out, maxSize));
    ASSERT_TRUE(expected == out);
}

//---------------------------------------------------------------------------------------
// A chunk declaring an uncompressed size larger than the reader's buffer must be rejected
// rather than decompressed (which would overflow the buffer).
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SnappyReaderTests, OversizedUncompressedLengthIsRejected)
{
    DisableAsserts noAsserts;
    for (uint32_t declared : {SnappyReader::SNAPPY_UNCOMPRESSED_BUFFER_SIZE + 1, (uint32_t) 40000, (uint32_t) 0x10000})
    {
        bvector<Byte> blob = MakeChunk(MakeSnappyStream(declared));
        bvector<Byte> out;
        EXPECT_NE(ZIP_SUCCESS, ReadFromMemory(blob, out, declared)) << declared;
        EXPECT_NE(ZIP_SUCCESS, ReadFromMemory(SnappyFromMemory::GetForThread(), blob, out, declared)) << declared;
        EXPECT_NE(ZIP_SUCCESS, ReadFromDbBlob(blob, out, declared)) << declared;
    }

    // declared length far larger than the data backing it (varint 0xffffffff, then a 1-byte literal)
    bvector<Byte> stream = {0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 'A'};
    bvector<Byte> blob = MakeChunk(stream);
    bvector<Byte> out;
    EXPECT_NE(ZIP_SUCCESS, ReadFromMemory(blob, out, 100));
    EXPECT_NE(ZIP_SUCCESS, ReadFromDbBlob(blob, out, 100));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SnappyReaderTests, MalformedChunksAreRejected)
{
    DisableAsserts noAsserts;
    bvector<Byte> out;

    // declared uncompressed size of zero
    bvector<Byte> zeroLen = MakeChunk(MakeSnappyStream(0));
    EXPECT_NE(ZIP_SUCCESS, ReadFromMemory(zeroLen, out, 10));
    EXPECT_NE(ZIP_SUCCESS, ReadFromDbBlob(zeroLen, out, 10));

    // chunk sizes that do not cover any data (chunk size includes the 2 header bytes)
    for (Byte badSize : {(Byte) 0, (Byte) 1, (Byte) 2})
    {
        bvector<Byte> blob = MakeChunk(MakeSnappyStream(100));
        blob[0] = badSize;
        blob[1] = 0;
        EXPECT_NE(ZIP_SUCCESS, ReadFromMemory(blob, out, 100)) << (int) badSize;
        EXPECT_NE(ZIP_SUCCESS, ReadFromDbBlob(blob, out, 100)) << (int) badSize;
    }

    // chunk size larger than the remaining blob bytes
    bvector<Byte> truncated = MakeChunk(MakeSnappyStream(100));
    truncated.resize(truncated.size() - 1);
    EXPECT_NE(ZIP_SUCCESS, ReadFromMemory(truncated, out, 100));
    EXPECT_NE(ZIP_SUCCESS, ReadFromDbBlob(truncated, out, 100));

    // blob too short to hold the 2-byte chunk header
    bvector<Byte> oneByte = {0x10};
    EXPECT_NE(ZIP_SUCCESS, ReadFromMemory(oneByte, out, 10));
    EXPECT_NE(ZIP_SUCCESS, ReadFromDbBlob(oneByte, out, 10));

    // corrupt body: copy op referencing data before the start of the output
    bvector<Byte> badCopy = MakeSnappyStream(100);
    badCopy[3] = 0x02 | (3 << 2); // first copy op now length 4...
    badCopy[4] = 0x10;            // ...with offset 16, before start of output
    bvector<Byte> badCopyBlob = MakeChunk(badCopy);
    EXPECT_NE(ZIP_SUCCESS, ReadFromMemory(badCopyBlob, out, 100));
    EXPECT_NE(ZIP_SUCCESS, ReadFromDbBlob(badCopyBlob, out, 100));

    // asking for more data than the blob contains fails once the blob is exhausted
    bvector<Byte> valid = MakeChunk(MakeSnappyStream(100));
    EXPECT_NE(ZIP_SUCCESS, ReadFromMemory(valid, out, 101));
    EXPECT_NE(ZIP_SUCCESS, ReadFromDbBlob(valid, out, 101));
}