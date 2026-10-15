/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/ScopedArray.h>
#include <Bentley/BeTimeUtilities.h>
#include <BeSQLite/BeSQLite.h>
#include <BeSQLite/BeLzma.h>

#include <zlib/zlib.h>
#include <zlib/zip/zip.h>
#include <zlib/zip/unzip.h>

//=======================================================================================
// @bsiclass
//=======================================================================================
struct          LzmaProgressTracker : BeSQLite::ICompressProgressTracker
{
    StatusInt _Progress (uint64_t inSize, int64_t outSize) override { return BSIERROR; }
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static void runCompressFileTest()
    {
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    BentleyApi::BeSQLite::BeSQLiteLib::Initialize(tempDir);

    LzmaProgressTracker abortTracker;

    BeFileName  nameOriginalFile;
    BeFileName  nameCompressedFile;

    // First need to create a new db
    BeTest::GetHost().GetOutputRoot(nameOriginalFile);
    nameOriginalFile.AppendToPath(L"CompressTest");
    BeFileName::CreateNewDirectory(nameOriginalFile.GetName());
    nameOriginalFile.AppendToPath(L"compressSeed.db");
    BeSQLite::Db db;
    db.CreateNewDb(nameOriginalFile);
    db.SaveChanges();
    db.CloseDb();

    BeTest::GetHost().GetOutputRoot(nameCompressedFile);
    nameCompressedFile.AppendToPath(L"CompressTest");
    nameCompressedFile.AppendToPath(L"compressSeed.imodel");

    uint32_t dictionarySize = 2 * 1024 * 1024;
    bool supportRandomAccess = true;
    BeSQLite::ZipErrors result;

    result = BeSQLite::LzmaUtility::CompressDb(nameCompressedFile, nameOriginalFile, &abortTracker, dictionarySize, supportRandomAccess);
    EXPECT_EQ(BeSQLite::ZIP_ERROR_ABORTED, result);

    result = BeSQLite::LzmaUtility::CompressDb(nameCompressedFile, nameOriginalFile, nullptr, dictionarySize, supportRandomAccess);
    EXPECT_EQ(BeSQLite::ZIP_SUCCESS, result);

    BeFileName nameUncompressedFile;
    BeTest::GetHost().GetOutputRoot(nameUncompressedFile);
    nameUncompressedFile.AppendToPath(L"CompressTest");
    nameUncompressedFile.AppendToPath(L"uncompress.db");

    result = BeSQLite::LzmaUtility::DecompressDb(nameUncompressedFile, nameCompressedFile, &abortTracker);
    EXPECT_EQ(BeSQLite::ZIP_ERROR_ABORTED, result);

    result = BeSQLite::LzmaUtility::DecompressDb(nameUncompressedFile, nameCompressedFile, nullptr);
    EXPECT_EQ(BeSQLite::ZIP_SUCCESS, result);

    uint64_t sizeOriginal, sizeUncompressed;
    BeFileName::GetFileSize(sizeOriginal, nameOriginalFile.GetName());
    BeFileName::GetFileSize(sizeUncompressed, nameUncompressedFile.GetName());
    EXPECT_TRUE(sizeUncompressed == sizeOriginal);

    BeFile  origFile;
    BeFileStatus status = origFile.Open(nameOriginalFile.GetName(), BeFileAccess::Read);
    EXPECT_TRUE(BeFileStatus::Success ==status);

    BeFile  decompFile;
    status = decompFile.Open(nameUncompressedFile.GetName(), BeFileAccess::Read);

    uint32_t bytesRead;

    bvector<Byte> origBuffer;
    origBuffer.resize(static_cast <size_t> (sizeOriginal));
    Byte*origData = &origBuffer[0];

    bvector<Byte> decompBuffer;
    decompBuffer.resize(static_cast <size_t> (sizeOriginal));
    Byte*decompData = &decompBuffer[0];

    origFile.Read(origData, &bytesRead, (uint32_t)sizeOriginal);
    EXPECT_TRUE(bytesRead == sizeOriginal);

    decompFile.Read(decompData, &bytesRead, (uint32_t)sizeOriginal);
    EXPECT_EQ(bytesRead, sizeOriginal);

    EXPECT_TRUE(!memcmp(origData, decompData, static_cast <size_t> (sizeOriginal)));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(CompressionTests, LzmaFile)
    {
    runCompressFileTest();
    }

struct Compresser
    {
    uint32_t    m_calls;
    uint32_t    m_totalCompressTime;
    uint32_t    m_totalUncompressTime;
    uint64_t    m_totalInputSize;
    uint64_t    m_totalCompressedSize;

    void __PrintResults (CharCP header)
        {
        double compressRatio = m_totalCompressedSize/(double)m_totalInputSize;
        printf ("%s compress times (%d) decompress times (%d) compression ratio (%f) number of calls (%d)\n",
                header, m_totalCompressTime, m_totalUncompressTime, compressRatio, m_calls);
        }

    void UpdateResults (uint32_t compressTime, uint32_t decompressTime, uint64_t inputSize, uint64_t compressedSize)
        {
        m_calls += 1;
        m_totalCompressTime += compressTime;
        m_totalUncompressTime += decompressTime;
        m_totalInputSize += inputSize;
        m_totalCompressedSize += compressedSize;
        }

    void Reset ()
        {
        m_calls = m_totalCompressTime = m_totalUncompressTime = 0;
        m_totalInputSize  = m_totalCompressedSize = 0;
        }
    Compresser () { Reset(); }
    virtual void CompressAndUncompress(bvector<Byte>&out, Byte*input, size_t lenInput) = 0;
    virtual void PrintResults() = 0;
    };

struct LzmaCompresser : Compresser
    {
private:
    uint32_t    m_level;
public:
    LzmaCompresser () : m_level(13) {  }
    void PrintResults ()
        {
        char buffer [100];
        snprintf (buffer, sizeof(buffer), "%s level (%d), ", "LZMA2", m_level);
        __PrintResults(buffer);
        }

    void SetProperties (uint32_t level)
        {
        m_level = level;
        }

    virtual void CompressAndUncompress(bvector<Byte>&out, Byte*input, size_t lenInput)
        {
        bvector<Byte>   compressed;
        compressed.reserve(lenInput/3);
        out.clear();

        uint64_t beforeCompress = BeTimeUtilities::QueryMillisecondsCounter();
        BeSQLite::LzmaEncoder  encoder(BeSQLite::LzmaEncoder::LzmaParams(1 << (m_level+10), true /*=supportRandomAccess*/));
        encoder.CompressBuffer(compressed, input, (uint32_t)lenInput);

        uint64_t afterCompress = BeTimeUtilities::QueryMillisecondsCounter();
        BeSQLite::LzmaDecoder  decoder;
        decoder.DecompressBuffer(out, &compressed[0], (uint32_t)compressed.size());

        uint64_t afterUncompress = BeTimeUtilities::QueryMillisecondsCounter();

        EXPECT_TRUE(out.size() == lenInput);
        EXPECT_TRUE(!memcmp(&out[0], input, lenInput));

        UpdateResults((uint32_t)(afterCompress - beforeCompress), (uint32_t)(afterUncompress - afterCompress), lenInput, compressed.size());
        }

    void Compress(bvector<Byte>&compressed, Byte*input, size_t lenInput)
        {
        compressed.clear();
        compressed.reserve(lenInput/3);

        BeSQLite::LzmaEncoder encoder(BeSQLite::LzmaEncoder::LzmaParams(1 << (m_level+10), true /*=supportRandomAccess*/));
        encoder.CompressBuffer(compressed, input, (uint32_t)lenInput);
        }
    };

struct ZipCompresser : Compresser
    {
private:
    int         m_level;
public:
    ZipCompresser (int level) : m_level(level) {  }
    void PrintResults ()
        {
        char buffer [100];
        snprintf (buffer, sizeof(buffer), "%s level (%d), ", "ZIP", m_level);
        __PrintResults(buffer);
        }

    ZipCompresser () : m_level (-1) {}
    void SetProperties (int level ) { m_level = level; }
    virtual void CompressAndUncompress(bvector<Byte>&out, Byte*input, size_t lenInput)
        {
        uLong  compressedLen = compressBound((uLong)lenInput);

        bvector<Byte>   compressed;
        compressed.resize(compressedLen);
        out.resize(lenInput);

        uint64_t beforeCompress = BeTimeUtilities::QueryMillisecondsCounter();
        compress2(&compressed[0], &compressedLen, input, (uLong)lenInput, m_level);

        uint64_t afterCompress = BeTimeUtilities::QueryMillisecondsCounter();

        uLong  uncompLen = (uLong)lenInput;
        uncompress(&out[0], &uncompLen, &compressed[0], compressedLen);

        uint64_t afterUncompress = BeTimeUtilities::QueryMillisecondsCounter();

        EXPECT_TRUE(out.size() == lenInput);
        EXPECT_TRUE(!memcmp(&out[0], input, lenInput));

        UpdateResults((uint32_t)(afterCompress - beforeCompress), (uint32_t)(afterUncompress - afterCompress), lenInput, compressedLen);
        }

    };

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static void doCompressBufferTests(Compresser&compresser, BeFileName&fileName)
    {
    uint32_t inputSize = 100000;
    ScopedArray<Byte>  scoped(inputSize);
    Byte* inputBuffer = scoped.GetData();

    BeFile file;
    file.Open(fileName.GetName(), BeFileAccess::Read);
    bvector<Byte> result;

    compresser.Reset();
    unsigned  compressIterations = 3;
    for (unsigned i = 0; i < compressIterations; ++i)
        {
        uint32_t bytesRead;
        file.Read(inputBuffer, &bytesRead, inputSize);
        if (bytesRead < inputSize)
            break;

        compresser.CompressAndUncompress(result, &inputBuffer[0], inputSize);
        }
    //  compresser.PrintResults();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(CompressionTests, LzmaBuffer)
    {
    BeFileName  fileName;

    BeTest::GetHost().GetDocumentsRoot (fileName);
    fileName.AppendToPath(L"Bim");
    fileName.AppendToPath(L"SubStation_NoFence.i.ibim");

    uint32_t steps[] = { 5, 10, 0};
    LzmaCompresser lzmaCompresser;

    for (unsigned i = 0; steps[i] != 0; ++i)
        {
        lzmaCompresser.SetProperties(steps[i]);
        doCompressBufferTests(lzmaCompresser, fileName);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(CompressionTests, ZipBuffer)
    {
    BeFileName  fileName;

    BeTest::GetHost().GetDocumentsRoot (fileName);
    fileName.AppendToPath(L"Bim");
    fileName.AppendToPath(L"SiteLayout.i.ibim");

    uint32_t levels[] = { 5, 9, 0 };
    ZipCompresser zipCompresser;

    for (unsigned i = 0; levels[i] != 0; ++i)
        {
        zipCompresser.SetProperties(levels[i]);
        doCompressBufferTests(zipCompresser, fileName);
        }
     }

#if defined (RUN_LZMA_ZIP_COMPARISONS)
struct CompressTestResults
    {
private:
    uint64_t    m_beforeDecompress;
    uint64_t    m_afterDecompress;
    uint64_t    m_resultSize;
public:
    void BeforeDecompress () { m_beforeDecompress = BeTimeUtilities::QueryMillisecondsCounter(); }
    void AfterDecompress () { m_afterDecompress = BeTimeUtilities::QueryMillisecondsCounter(); }
    void SetResultSize (uint64_t resultSize) { m_resultSize = resultSize; }
    uint32_t GetResultSize () { return (uint32_t)m_resultSize; }
    uint32_t GetDecompressTime () { return (uint32_t)(m_afterDecompress-m_beforeDecompress); }
    };

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static void runDirectComparison(WCharCP fileName, uint32_t dictionarySize, uint32_t inputLen)
    {
    bvector<CompressTestResults>  lzmaResults;
    bvector<CompressTestResults>  zipResults;

    uint64_t fileSize;

    BeFileName::GetFileSize(fileSize, fileName);
    if (fileSize < inputLen)
        return;

    wprintf (L"processing %ls with dictionary size %d, source size = %d\n", fileName, dictionarySize, inputLen);
    BeFile file;
    file.Open(fileName, BeFileAccess::Read, BeFileSharing::None);

    BeSQLite::LzmaEncoder encoder (false, dictionarySize);
    BeSQLite::LzmaDecoder decoder;

    ScopedArray<Byte>  scoped(inputLen);
    Byte* inputBuffer = scoped.GetData();

    uint32_t nDecompIters = 8;
    bvector<Byte> uncompressed;
    uncompressed.resize(inputLen);

    unsigned int limitReadIterations = 3000;
    //  Process all using LZMA
    for (unsigned i = 0; i < limitReadIterations; i++)
        {
        uint32_t bytesRead;
        file.Read(inputBuffer, &bytesRead, inputLen);
        if (bytesRead < inputLen)
            break;

        bvector<Byte> compressed;
        encoder.Compress(compressed, inputBuffer, inputLen, nullptr);
        CompressTestResults  current;
        current.SetResultSize(compressed.size());
        current.BeforeDecompress();
        for (unsigned decompIters = 0; decompIters < nDecompIters; decompIters++)
            {
            decoder.UncompressToBuffer(&uncompressed[0], inputLen, &compressed[0], (uint32_t)compressed.size());
            if (0 == decompIters)
                {
                EXPECT_TRUE (!memcmp(&uncompressed[0], inputBuffer, inputLen));
                }
            }
        current.AfterDecompress();
        lzmaResults.push_back(current);
        }

    printf ("finished LZMA tests\n");
    file.SetPointer(0, BeFileSeekOrigin::Begin);
    const uLong  compressedBufferLen = compressBound((uLong)inputLen);
    bvector<Byte> compressed;
    compressed.resize(compressedBufferLen);

    //  Process all using zip
    for (unsigned i = 0; i < limitReadIterations; i++)
        {
        uint32_t bytesRead;
        file.Read(inputBuffer, &bytesRead, inputLen);
        if (bytesRead < inputLen)
            break;

        uLong  compressedLen = compressedBufferLen;
        compress2(&compressed[0], &compressedLen, inputBuffer, (uLong)inputLen, 7);

        CompressTestResults  current;
        current.SetResultSize(compressedLen);
        current.BeforeDecompress();
        for (unsigned decompIters = 0; decompIters < nDecompIters; decompIters++)
            {
            uLong  uncompLen = (uLong)inputLen;
            uncompress(&uncompressed[0], &uncompLen, &compressed[0], compressedLen);
            if (0 == decompIters)
                {
                EXPECT_TRUE (!memcmp(&uncompressed[0], inputBuffer, inputLen));
                }
            }
        current.AfterDecompress();
        zipResults.push_back(current);
        }

    BeAssert (zipResults.size() == lzmaResults.size());

    uint32_t zipSmaller = 0;
    uint32_t lzmaFaster = 0;
    uint32_t lzmaTotalTime = 0;
    uint32_t zipTotalTime = 0;
    uint64_t lzmaTotalResultSize = 0;
    uint64_t zipTotalResultSize = 0;
    for (unsigned i = 0; i < zipResults.size(); i++)
        {
        CompressTestResults& zipResult = zipResults[i];
        CompressTestResults& lzmaResult = lzmaResults[i];
        if (zipResult.GetResultSize() < lzmaResult.GetResultSize())
            zipSmaller++;
        if (zipResult.GetDecompressTime() > lzmaResult.GetDecompressTime())
            lzmaFaster++;

        zipTotalResultSize += zipResult.GetResultSize();
        zipTotalTime += zipResult.GetDecompressTime();
        lzmaTotalResultSize += lzmaResult.GetResultSize();
        lzmaTotalTime += lzmaResult.GetDecompressTime();
        }

    printf ("process %d results, zipSmaller %d times, lzmaFaster %d times\n", (uint32_t)zipResults.size(), zipSmaller, lzmaFaster);
    double lzmaAvgTime = (double)lzmaTotalTime/(uint32_t)zipResults.size();
    double zipAvgTime = (double)zipTotalTime/(uint32_t)zipResults.size();
    uint32_t lzmaAvgSize = lzmaTotalResultSize/(uint32_t)zipResults.size();
    uint32_t zipAvgSize = zipTotalResultSize/(uint32_t)zipResults.size();
    //  Note that the buffer is uncompressed nDecompIters times between the calls to BeforeDecompress and AfterDecompress.
    printf ("lzma avg uncompress time = %f, avg compressed size = %d\n", lzmaAvgTime/nDecompIters, lzmaAvgSize);
    printf ("zip avg uncompress time = %f, avg compressed size = %d\n", zipAvgTime/nDecompIters, zipAvgSize);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(CompressionTests, DirectComparisonSmallInput)
    {
    uint32_t sizes [] = { 32 * 1024, 1 << 17, 2 * 1024 * 1024, 0 };
    uint32_t inputLen = 100000;

    for (unsigned i = 0; sizes[i] > 0; i++)
        {
        BeFileName  fileName;

        BeTest::GetHost().GetDocumentsRoot (fileName);
        fileName.AppendToPath(L"Bim");
        fileName.AppendToPath(L"SiteLayout.i.ibim");

        runDirectComparison(fileName.GetName(), sizes[i], inputLen);
        runDirectComparison(L"E:\\graphite\\xfer\\Bim\\OPSPS.i.ibim", sizes[i], inputLen);
        runDirectComparison(L"E:\\graphite\\xfer\\Bim\\Plant - 201_total.i.ibim", sizes[i], inputLen);
        runDirectComparison(L"E:\\graphite\\out2010\\Winx64\\build\\DgnPlatform\\DgnHandlers\\TextStyleHandlers.obj", sizes[i], inputLen);
        runDirectComparison(L"E:\\graphite\\out2010\\Winx64\\build\\DgnPlatform\\DgnHandlers\\dgnhandlers.pdb", sizes[i], inputLen);
        runDirectComparison(L"E:\\graphite\\out2010\\Winx64\\Product\\BeGTest\\DgnHandlers5.dll", sizes[i], inputLen);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(CompressionTests, DirectComparisonBigInput)
    {
    uint32_t sizes [] = { 2 * 1024 * 1024, 0 };
    uint32_t inputLen = 2 * 1024 * 1024;

    for (unsigned i = 0; sizes[i] > 0; i++)
        {
        BeFileName  fileName;

        BeTest::GetHost().GetDocumentsRoot (fileName);
        fileName.AppendToPath(L"Bim");
        fileName.AppendToPath(L"SiteLayout.i.ibim");

        runDirectComparison(fileName.GetName(), sizes[i], inputLen);
        runDirectComparison(L"E:\\graphite\\xfer\\Bim\\OPSPS.i.ibim", sizes[i], inputLen);
        runDirectComparison(L"E:\\graphite\\xfer\\Bim\\Plant - 201_total.i.ibim", sizes[i], inputLen);
        runDirectComparison(L"E:\\graphite\\out2010\\Winx64\\build\\DgnPlatform\\DgnHandlers\\dgnhandlers.pdb", sizes[i], inputLen);
        runDirectComparison(L"E:\\graphite\\out2010\\Winx64\\Product\\BeGTest\\DgnHandlers5.dll", sizes[i], inputLen);
        }
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static bvector<Byte> makeLzmaRegressionData(uint32_t size, uint32_t seed)
    {
    bvector<Byte> data;
    data.resize(size);

    uint32_t state = seed;
    for (uint32_t i = 0; i < size; ++i)
        {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        data[i] = static_cast<Byte>(state >> 24);
        }

    return data;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static void appendLzmaRegressionData(bvector<Byte>& target, Byte const* data, uint32_t size)
    {
    if (size == 0)
        return;

    size_t oldSize = target.size();
    target.resize(oldSize + size);
    memcpy(&target[oldSize], data, size);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static void expectLzmaDataEqual(bvector<Byte> const& expected, bvector<Byte> const& actual)
    {
    EXPECT_EQ(expected.size(), actual.size());
    if (expected.size() == actual.size() && !expected.empty())
        EXPECT_EQ(0, memcmp(&expected[0], &actual[0], expected.size()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static bool decodeLzmaInPages(BeSQLite::LzmaDecoder& decoder, bvector<Byte>& decoded, bvector<Byte> const& compressed)
    {
    if (compressed.empty())
        return false;

    BeSQLite::MemoryLzmaInStream input(&compressed[0], static_cast<uint32_t>(compressed.size()));
    if (decoder.StartDecompress(input) != BeSQLite::ZIP_SUCCESS)
        return false;

    static int const pageSizes[] = { 1, 17, 4096, 64 * 1024, 13 };
    static unsigned const numPageSizes = sizeof(pageSizes) / sizeof(pageSizes[0]);

    for (unsigned pageIndex = 0;; ++pageIndex)
        {
        int pageSize = pageSizes[pageIndex % numPageSizes];
        bvector<Byte> page;
        page.resize(pageSize);

        int bytesInPage = pageSize;
        BeSQLite::ZipErrors status = decoder.DecompressNextPage(&page[0], &bytesInPage);
        if (status != BeSQLite::ZIP_SUCCESS || bytesInPage < 0 || bytesInPage > pageSize)
            {
            decoder.FinishDecompress();
            return false;
            }

        if (bytesInPage == 0)
            break;

        size_t oldSize = decoded.size();
        decoded.resize(oldSize + bytesInPage);
        memcpy(&decoded[oldSize], &page[0], bytesInPage);
        }

    decoder.FinishDecompress();
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(CompressionTests, LzmaCompressBufferRoundTrip)
    {
    const uint32_t blockSize = 64 * 1024;
    bvector<Byte> input = makeLzmaRegressionData(3 * blockSize + 123, 0x13579bdf);
    memset(input.data() + blockSize, 'a', blockSize);

    for (int threads : { 1, 4 })
        {
        BeSQLite::LzmaEncoder::LzmaParams params(64 * 1024, false, 7, threads);
        params.SetBlockSize(blockSize);
        BeSQLite::LzmaEncoder encoder(params);

        bvector<Byte> compressed;
        BeSQLite::ZipErrors status = encoder.CompressBuffer(compressed, &input[0], static_cast<uint32_t>(input.size()));
        EXPECT_EQ(BeSQLite::ZIP_SUCCESS, status);
        EXPECT_TRUE(!compressed.empty());
        if (status != BeSQLite::ZIP_SUCCESS || compressed.empty())
            continue;

        bvector<Byte> decoded;
        BeSQLite::LzmaDecoder decoder;
        status = decoder.DecompressBuffer(decoded, &compressed[0], static_cast<uint32_t>(compressed.size()));
        EXPECT_EQ(BeSQLite::ZIP_SUCCESS, status);
        if (status == BeSQLite::ZIP_SUCCESS)
            expectLzmaDataEqual(input, decoded);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(CompressionTests, LzmaIncrementalPages)
    {
    const uint32_t blockSize = 64 * 1024;
    Byte prefix[] = { 0x00, 0x7f, 0x80, 0xff, 0x11, 0x22, 0x33 };
    bvector<Byte> largePage = makeLzmaRegressionData(blockSize + 257, 0x2468ace1);
    bvector<Byte> noisyPage = makeLzmaRegressionData(48 * 1024 + 3, 0xdeadbeef);
    memset(largePage.data() + blockSize / 2, 'b', blockSize / 2);

    bvector<Byte> expected;
    appendLzmaRegressionData(expected, prefix, sizeof(prefix));
    appendLzmaRegressionData(expected, nullptr, 0);
    appendLzmaRegressionData(expected, &largePage[0], static_cast<uint32_t>(largePage.size()));
    appendLzmaRegressionData(expected, &noisyPage[0], static_cast<uint32_t>(noisyPage.size()));

    for (int threads : { 1, 4 })
        {
        BeSQLite::LzmaEncoder::LzmaParams params(64 * 1024, false, 7, threads);
        params.SetBlockSize(blockSize);
        BeSQLite::LzmaEncoder encoder(params);
        bvector<Byte> compressed;
        BeSQLite::MemoryLzmaOutStream output(compressed);

        ASSERT_EQ(BeSQLite::ZIP_SUCCESS, encoder.StartCompress(output));
        ASSERT_EQ(BeSQLite::ZIP_SUCCESS, encoder.CompressNextPage(prefix, static_cast<int>(sizeof(prefix))));
        ASSERT_EQ(BeSQLite::ZIP_SUCCESS, encoder.CompressNextPage(nullptr, 0));
        ASSERT_EQ(BeSQLite::ZIP_SUCCESS, encoder.CompressNextPage(&largePage[0], static_cast<int>(largePage.size())));
        ASSERT_EQ(BeSQLite::ZIP_SUCCESS, encoder.CompressNextPage(&noisyPage[0], static_cast<int>(noisyPage.size())));
        ASSERT_EQ(BeSQLite::ZIP_SUCCESS, encoder.FinishCompress());

        bvector<Byte> decoded;
        BeSQLite::LzmaDecoder decoder;
        EXPECT_TRUE(decodeLzmaInPages(decoder, decoded, compressed));
        expectLzmaDataEqual(expected, decoded);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(CompressionTests, LzmaIncrementalHandleReuse)
    {
    BeSQLite::LzmaEncoder::LzmaParams params(64 * 1024, false, 7, 4);
    BeSQLite::LzmaEncoder encoder(params);
    BeSQLite::LzmaDecoder decoder;

    for (unsigned streamIndex = 0; streamIndex < 2; ++streamIndex)
        {
        uint32_t inputSize = streamIndex == 0 ? 19 : 96 * 1024 + 11;
        bvector<Byte> input = makeLzmaRegressionData(inputSize, 0x10203040 + streamIndex);
        bvector<Byte> compressed;
        BeSQLite::MemoryLzmaOutStream output(compressed);

        BeSQLite::ZipErrors status = encoder.StartCompress(output);
        EXPECT_EQ(BeSQLite::ZIP_SUCCESS, status);
        if (status != BeSQLite::ZIP_SUCCESS)
            return;

        status = encoder.CompressNextPage(&input[0], static_cast<int>(input.size()));
        EXPECT_EQ(BeSQLite::ZIP_SUCCESS, status);
        if (status != BeSQLite::ZIP_SUCCESS)
            {
            encoder.FinishCompress();
            return;
            }

        status = encoder.FinishCompress();
        EXPECT_EQ(BeSQLite::ZIP_SUCCESS, status);
        if (status != BeSQLite::ZIP_SUCCESS)
            return;

        bvector<Byte> decoded;
        EXPECT_TRUE(decodeLzmaInPages(decoder, decoded, compressed));
        expectLzmaDataEqual(input, decoded);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(CompressionTests, LzmaEmptyInputRoundTrip)
    {
    BeSQLite::LzmaEncoder::LzmaParams params(64 * 1024, false, 7, 1);
    BeSQLite::LzmaEncoder encoder(params);
    bvector<Byte> compressed;

    BeSQLite::ZipErrors status = encoder.CompressBuffer(compressed, nullptr, 0);
    EXPECT_EQ(BeSQLite::ZIP_SUCCESS, status);
    EXPECT_TRUE(!compressed.empty());
    if (status != BeSQLite::ZIP_SUCCESS || compressed.empty())
        return;

    bvector<Byte> decoded;
    BeSQLite::LzmaDecoder decoder;
    EXPECT_EQ(BeSQLite::ZIP_SUCCESS, decoder.DecompressBuffer(decoded, &compressed[0], static_cast<uint32_t>(compressed.size())));
    EXPECT_TRUE(decoded.empty());
    }

struct LzmaCancellingProgressTracker : BeSQLite::ICompressProgressTracker
    {
    uint32_t m_calls = 0;

    StatusInt _Progress(uint64_t, int64_t) override
        {
        ++m_calls;
        return BSIERROR;
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(CompressionTests, LzmaProgressCancellation)
    {
    bvector<Byte> input = makeLzmaRegressionData(256 * 1024, 0xabcdef01);
    BeSQLite::LzmaEncoder::LzmaParams params(64 * 1024, false, 7, 4);
    params.SetBlockSize(64 * 1024);
    BeSQLite::LzmaEncoder encoder(params);
    LzmaCancellingProgressTracker tracker;
    encoder.SetProgressTracker(&tracker);

    bvector<Byte> compressed;
    EXPECT_EQ(BeSQLite::ZIP_ERROR_ABORTED, encoder.CompressBuffer(compressed, &input[0], static_cast<uint32_t>(input.size())));
    EXPECT_TRUE(tracker.m_calls > 0);
    }

struct LzmaFailingOutputStream : BeSQLite::ILzmaOutputStream
    {
    uint32_t m_calls = 0;
    uint32_t m_successfulWrites = 0;

    BeSQLite::ZipErrors _Write(void const*, uint32_t size, uint32_t& bytesWritten) override
        {
        ++m_calls;
        bytesWritten = size;
        return m_calls <= m_successfulWrites ? BeSQLite::ZIP_SUCCESS : BeSQLite::ZIP_ERROR_WRITE_ERROR;
        }

    void _SetAlwaysFlush(bool) override {}
    };

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(CompressionTests, LzmaOutputWriteFailure)
    {
    BeSQLite::LzmaEncoder::LzmaParams params(64 * 1024, false, 7, 1);
    BeSQLite::LzmaEncoder encoder(params);
    LzmaFailingOutputStream output;

    EXPECT_EQ(BeSQLite::ZIP_ERROR_WRITE_ERROR, encoder.StartCompress(output));
    EXPECT_EQ(1u, output.m_calls);

    bvector<Byte> input = makeLzmaRegressionData(128 * 1024, 0x12345678);
    BeSQLite::MemoryLzmaInStream inputStream(input.data(), static_cast<uint32_t>(input.size()));
    BeSQLite::LzmaEncoder streamEncoder(BeSQLite::LzmaEncoder::LzmaParams(64 * 1024, false, 7, 4));
    output.m_calls = 0;
    output.m_successfulWrites = 1;
    EXPECT_EQ(BeSQLite::ZIP_ERROR_WRITE_ERROR, streamEncoder.CompressStream(output, inputStream));
    EXPECT_TRUE(output.m_calls > 1);
    }

