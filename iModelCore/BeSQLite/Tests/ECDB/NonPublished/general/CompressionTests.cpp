/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/ECDB/NonPublished/general/CompressionTests.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//  #include "LzmaTests.h"
#include <Bentley/BeTest.h>
#include <Bentley/ScopedArray.h>
#include <Bentley/BeTimeUtilities.h>
#include <BeSQLite/BeSQLite.h>

#include <zlib/zlib.h>
#include <zlib/zip/zip.h>
#include <zlib/zip/unzip.h>

//=======================================================================================
// @bsiclass                                                    John.Gooding    01/2013
//=======================================================================================
struct          LzmaProgressTracker : BeSQLite::ICompressProgressTracker
{
    StatusInt _Progress (UInt64 inSize, Int64 outSize) override { return BSIERROR; }
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
static void runCompressFileTest()
    {
    LzmaProgressTracker abortTracker;

    BeFileName  nameOriginalFile;
    BeFileName  nameCompressedFile;

    BeTest::GetHost().GetDocumentsRoot (nameOriginalFile);
    nameOriginalFile.AppendToPath(L"DgnDb");
    nameOriginalFile.AppendToPath(L"SubStation_NoFence.i.idgndb");

    BeTest::GetHost().GetOutputRoot(nameCompressedFile);
    nameCompressedFile.AppendToPath(L"DgnDb");
    nameCompressedFile.AppendToPath(L"CompressTest");
    BeFileName::CreateNewDirectory(nameCompressedFile.GetName());
    nameCompressedFile.AppendToPath(L"SubStation_NoFence.imodel");

    BeSQLite::LzmaEncoder   encoder(2 * 1024 * 1024);
    BeSQLite::ZipErrors result = encoder.CompressDgnDb(nameCompressedFile, nameOriginalFile, &abortTracker, true);
    EXPECT_EQ(BeSQLite::ZIP_ERROR_ABORTED, result);
    result = encoder.CompressDgnDb(nameCompressedFile, nameOriginalFile, nullptr, true);
    EXPECT_EQ(BeSQLite::ZIP_SUCCESS, result);

    BeFileName nameUncompressedFile;
    BeTest::GetHost().GetOutputRoot(nameUncompressedFile);
    nameUncompressedFile.AppendToPath(L"DgnDb");
    nameUncompressedFile.AppendToPath(L"CompressTest");
    nameUncompressedFile.AppendToPath(L"SubStation_NoFence.i.idgndb");

    BeSQLite::LzmaDecoder decoder;
    result = decoder.UncompressDgnDb(nameUncompressedFile, nameCompressedFile, &abortTracker);
    EXPECT_EQ(BeSQLite::ZIP_ERROR_ABORTED, result);

    result = decoder.UncompressDgnDb(nameUncompressedFile, nameCompressedFile, nullptr);
    EXPECT_EQ(BeSQLite::ZIP_SUCCESS, result);

    UInt64 sizeOriginal, sizeUncompressed;
    BeFileName::GetFileSize(sizeOriginal, nameOriginalFile.GetName());
    BeFileName::GetFileSize(sizeUncompressed, nameUncompressedFile.GetName());
    EXPECT_TRUE(sizeUncompressed == sizeOriginal);

    BeFile  origFile;
    BeFileStatus status = origFile.Open(nameOriginalFile.GetName(), BeFileAccess::Read);
    EXPECT_TRUE(BeFileStatus::Success ==status);

    BeFile  decompFile;
    status = decompFile.Open(nameUncompressedFile.GetName(), BeFileAccess::Read);

    UInt32 bytesRead;

    bvector<byte> origBuffer;
    origBuffer.resize(static_cast <size_t> (sizeOriginal));
    byte*origData = &origBuffer[0];

    bvector<byte> decompBuffer;
    decompBuffer.resize(static_cast <size_t> (sizeOriginal));
    byte*decompData = &decompBuffer[0];

    origFile.Read(origData, &bytesRead, (UInt32)sizeOriginal);
    EXPECT_TRUE(bytesRead == sizeOriginal);

    decompFile.Read(decompData, &bytesRead, (UInt32)sizeOriginal);
    EXPECT_EQ(bytesRead, sizeOriginal);

    EXPECT_TRUE(!memcmp(origData, decompData, static_cast <size_t> (sizeOriginal)));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
TEST(CompressionTests, LzmaFile)
    {
    runCompressFileTest();
    }

struct Compresser
    {
    UInt32      m_calls;
    UInt32      m_totalCompressTime;
    UInt32      m_totalUncompressTime;
    UInt64      m_totalInputSize;
    UInt64      m_totalCompressedSize;

    void __PrintResults (CharCP header)
        {
        double compressRatio = m_totalCompressedSize/(double)m_totalInputSize;
        printf ("%s compress times (%d) decompress times (%d) compression ratio (%f) number of calls (%d)\n",
                header, m_totalCompressTime, m_totalUncompressTime, compressRatio, m_calls);
        }

    void UpdateResults (UInt32 compressTime, UInt32 decompressTime, UInt64 inputSize, UInt64 compressedSize)
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
    virtual void CompressAndUncompress(bvector<byte>&out, byte*input, size_t lenInput) = 0;
    virtual void PrintResults() = 0;
    };

struct LzmaCompresser : Compresser
    {
private:
    UInt32      m_level;
public:
    LzmaCompresser () : m_level(13) {  }
    void PrintResults ()
        {
        char buffer [100];
        sprintf (buffer, "%s level (%d), ", "LZMA2", m_level);
        __PrintResults(buffer);
        }

    void SetProperties (UInt32 level)
        {
        m_level = level;
        }

    virtual void CompressAndUncompress(bvector<byte>&out, byte*input, size_t lenInput)
        {
        bvector<byte>   compressed;
        compressed.resize(lenInput/3); //  avoid too many resizes

        UInt64 beforeCompress = BeTimeUtilities::QueryMillisecondsCounter();
        BeSQLite::LzmaEncoder     encoder(1 << (m_level+10));
        encoder.Compress(compressed, input, (UInt32)lenInput, nullptr, true);

        UInt64 afterCompress = BeTimeUtilities::QueryMillisecondsCounter();
        BeSQLite::LzmaDecoder  decoder;

        decoder.Uncompress(out, &compressed[0], (UInt32)compressed.size());

        UInt64 afterUncompress = BeTimeUtilities::QueryMillisecondsCounter();

        EXPECT_TRUE(out.size() == lenInput);
        EXPECT_TRUE(!memcmp(&out[0], input, lenInput));

        UpdateResults((UInt32)(afterCompress - beforeCompress), (UInt32)(afterUncompress - afterCompress), lenInput, compressed.size());
        }

    void Compress(bvector<byte>&compressed, byte*input, size_t lenInput)
        {
        compressed.resize(lenInput/3); //  avoid too many resizes

        BeSQLite::LzmaEncoder     encoder(1 << (m_level+10));
        encoder.Compress(compressed, input, (UInt32)lenInput, nullptr, true);
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
        sprintf (buffer, "%s level (%d), ", "ZIP", m_level);
        __PrintResults(buffer);
        }

    ZipCompresser () : m_level (-1) {}
    void SetProperties (int level ) { m_level = level; }
    virtual void CompressAndUncompress(bvector<byte>&out, byte*input, size_t lenInput)
        {
        uLong  compressedLen = compressBound((uLong)lenInput);

        bvector<byte>   compressed;
        compressed.resize(compressedLen);
        out.resize(lenInput);

        UInt64 beforeCompress = BeTimeUtilities::QueryMillisecondsCounter();
        compress2(&compressed[0], &compressedLen, input, (uLong)lenInput, m_level);

        UInt64 afterCompress = BeTimeUtilities::QueryMillisecondsCounter();

        uLong  uncompLen = (uLong)lenInput;
        uncompress(&out[0], &uncompLen, &compressed[0], compressedLen);

        UInt64 afterUncompress = BeTimeUtilities::QueryMillisecondsCounter();

        EXPECT_TRUE(out.size() == lenInput);
        EXPECT_TRUE(!memcmp(&out[0], input, lenInput));

        UpdateResults((UInt32)(afterCompress - beforeCompress), (UInt32)(afterUncompress - afterCompress), lenInput, compressedLen);
        }

    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
static void doCompressBufferTests(Compresser&compresser, BeFileName&fileName)
    {
    UInt32 inputSize = 100000;
    ScopedArray<byte>  scoped(inputSize);
    byte* inputBuffer = scoped.GetData();

    BeFile file;
    file.Open(fileName.GetName(), BeFileAccess::Read);
    bvector<byte> result;

    compresser.Reset();
    unsigned  compressIterations = 3;
    for (unsigned i = 0; i < compressIterations; ++i)
        {
        UInt32  bytesRead;
        file.Read(inputBuffer, &bytesRead, inputSize);
        if (bytesRead < inputSize)
            break;

        compresser.CompressAndUncompress(result, &inputBuffer[0], inputSize);
        }
    //  compresser.PrintResults();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
TEST(CompressionTests, LzmaBuffer)
    {
    BeFileName  fileName;

    BeTest::GetHost().GetDocumentsRoot (fileName);
    fileName.AppendToPath(L"DgnDb");
    fileName.AppendToPath(L"SubStation_NoFence.i.idgndb");

    UInt32 steps[] = { 5, 10, 0};
    LzmaCompresser lzmaCompresser;

    for (unsigned i = 0; steps[i] != 0; ++i)
        {
        lzmaCompresser.SetProperties(steps[i]);
        doCompressBufferTests(lzmaCompresser, fileName);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
TEST(CompressionTests, ZipBuffer)
    {
    BeFileName  fileName;

    BeTest::GetHost().GetDocumentsRoot (fileName);
    fileName.AppendToPath(L"DgnDb");
    fileName.AppendToPath(L"SiteLayout.i.idgndb");

    UInt32 levels[] = { 5, 9, 0 };
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
    UInt64      m_beforeDecompress;
    UInt64      m_afterDecompress;
    UInt64      m_resultSize;
public:
    void BeforeDecompress () { m_beforeDecompress = BeTimeUtilities::QueryMillisecondsCounter(); }
    void AfterDecompress () { m_afterDecompress = BeTimeUtilities::QueryMillisecondsCounter(); }
    void SetResultSize (UInt64 resultSize) { m_resultSize = resultSize; }
    UInt32 GetResultSize () { return (UInt32)m_resultSize; }
    UInt32 GetDecompressTime () { return (UInt32)(m_afterDecompress-m_beforeDecompress); }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
static void runDirectComparison(WCharCP fileName, UInt32 dictionarySize, UInt32 inputLen)
    {
    bvector<CompressTestResults>  lzmaResults;
    bvector<CompressTestResults>  zipResults;

    UInt64 fileSize;

    BeFileName::GetFileSize(fileSize, fileName);
    if (fileSize < inputLen)
        return;

    wprintf (L"processing %ls with dictionary size %d, source size = %d\n", fileName, dictionarySize, inputLen);
    BeFile file;
    file.Open(fileName, BeFileAccess::Read, BeFileSharing::None);

    BeSQLite::LzmaEncoder encoder (false, dictionarySize);
    BeSQLite::LzmaDecoder decoder;

    ScopedArray<byte>  scoped(inputLen);
    byte* inputBuffer = scoped.GetData();

    UInt32 nDecompIters = 8;
    bvector<byte> uncompressed;
    uncompressed.resize(inputLen);

    unsigned int limitReadIterations = 3000;
    //  Process all using LZMA
    for (unsigned i = 0; i < limitReadIterations; i++)
        {
        UInt32  bytesRead;
        file.Read(inputBuffer, &bytesRead, inputLen);
        if (bytesRead < inputLen)
            break;

        bvector<byte> compressed;
        encoder.Compress(compressed, inputBuffer, inputLen, nullptr);
        CompressTestResults  current;
        current.SetResultSize(compressed.size());
        current.BeforeDecompress();
        for (unsigned decompIters = 0; decompIters < nDecompIters; decompIters++)
            {
            decoder.UncompressToBuffer(&uncompressed[0], inputLen, &compressed[0], (UInt32)compressed.size());
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
    bvector<byte> compressed;
    compressed.resize(compressedBufferLen);

    //  Process all using zip
    for (unsigned i = 0; i < limitReadIterations; i++)
        {
        UInt32  bytesRead;
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

    UInt32 zipSmaller = 0;
    UInt32 lzmaFaster = 0;
    UInt32 lzmaTotalTime = 0;
    UInt32 zipTotalTime = 0;
    UInt64 lzmaTotalResultSize = 0;
    UInt64 zipTotalResultSize = 0;
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

    printf ("process %d results, zipSmaller %d times, lzmaFaster %d times\n", (UInt32)zipResults.size(), zipSmaller, lzmaFaster);
    double lzmaAvgTime = (double)lzmaTotalTime/(UInt32)zipResults.size();
    double zipAvgTime = (double)zipTotalTime/(UInt32)zipResults.size();
    UInt32 lzmaAvgSize = lzmaTotalResultSize/(UInt32)zipResults.size();
    UInt32 zipAvgSize = zipTotalResultSize/(UInt32)zipResults.size();
    //  Note that the buffer is uncompressed nDecompIters times between the calls to BeforeDecompress and AfterDecompress.
    printf ("lzma avg uncompress time = %f, avg compressed size = %d\n", lzmaAvgTime/nDecompIters, lzmaAvgSize);
    printf ("zip avg uncompress time = %f, avg compressed size = %d\n", zipAvgTime/nDecompIters, zipAvgSize);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
TEST(CompressionTests, DirectComparisonSmallInput)
    {
    UInt32 sizes [] = { 32 * 1024, 1 << 17, 2 * 1024 * 1024, 0 };
    UInt32 inputLen = 100000;

    for (unsigned i = 0; sizes[i] > 0; i++)
        {
        BeFileName  fileName;

        BeTest::GetHost().GetDocumentsRoot (fileName);
        fileName.AppendToPath(L"DgnDb");
        fileName.AppendToPath(L"SiteLayout.i.idgndb");

        runDirectComparison(fileName.GetName(), sizes[i], inputLen);
        runDirectComparison(L"E:\\graphite\\xfer\\DgnDb\\OPSPS.i.idgndb", sizes[i], inputLen);
        runDirectComparison(L"E:\\graphite\\xfer\\DgnDb\\Plant - 201_total.i.idgndb", sizes[i], inputLen);
        runDirectComparison(L"E:\\graphite\\out2010\\Winx64\\build\\DgnPlatform\\DgnHandlers\\TextStyleHandlers.obj", sizes[i], inputLen);
        runDirectComparison(L"E:\\graphite\\out2010\\Winx64\\build\\DgnPlatform\\DgnHandlers\\dgnhandlers.pdb", sizes[i], inputLen);
        runDirectComparison(L"E:\\graphite\\out2010\\Winx64\\Product\\BeGTest\\DgnHandlers5.dll", sizes[i], inputLen);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
TEST(CompressionTests, DirectComparisonBigInput)
    {
    UInt32 sizes [] = { 2 * 1024 * 1024, 0 };
    UInt32 inputLen = 2 * 1024 * 1024;

    for (unsigned i = 0; sizes[i] > 0; i++)
        {
        BeFileName  fileName;

        BeTest::GetHost().GetDocumentsRoot (fileName);
        fileName.AppendToPath(L"DgnDb");
        fileName.AppendToPath(L"SiteLayout.i.idgndb");

        runDirectComparison(fileName.GetName(), sizes[i], inputLen);
        runDirectComparison(L"E:\\graphite\\xfer\\DgnDb\\OPSPS.i.idgndb", sizes[i], inputLen);
        runDirectComparison(L"E:\\graphite\\xfer\\DgnDb\\Plant - 201_total.i.idgndb", sizes[i], inputLen);
        runDirectComparison(L"E:\\graphite\\out2010\\Winx64\\build\\DgnPlatform\\DgnHandlers\\dgnhandlers.pdb", sizes[i], inputLen);
        runDirectComparison(L"E:\\graphite\\out2010\\Winx64\\Product\\BeGTest\\DgnHandlers5.dll", sizes[i], inputLen);
        }
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Shaun.Sewall    11/2013
//---------------------------------------------------------------------------------------
TEST(CompressionTests, CreateZipFile)
    {
    BeFileName  zipFileName;
    BeTest::GetHost().GetTempDir (zipFileName);
    zipFileName.AppendToPath (L"CompressionTests_CreateZipFile.zip");

    BeFileName  fileToZip;
    BeTest::GetHost().GetDocumentsRoot (fileToZip);
    fileToZip.AppendToPath (L"DgnDb");
    fileToZip.AppendToPath (L"SiteLayout.i.idgndb");
    EXPECT_TRUE (fileToZip.DoesPathExist());

    Utf8String nameInZip = BeFileName(BeFileName::NameAndExt, fileToZip.GetName()).GetNameUtf8();

    zipFile zf = zipOpen (zipFileName.GetNameUtf8().c_str(), APPEND_STATUS_CREATE);
    EXPECT_TRUE (nullptr != zf);

    int status = zipOpenNewFileInZip (zf, nameInZip.c_str(), nullptr, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_DEFAULT_COMPRESSION);
    EXPECT_EQ (status, 0);

    BeFile file;
    status = (StatusInt) file.Open (fileToZip, BeFileAccess::Read);
    EXPECT_EQ (status, 0);

    bvector<byte> fileBytes;
    status = (StatusInt)file.ReadEntireFile (fileBytes);
    EXPECT_EQ (status, 0);

    status = zipWriteInFileInZip (zf, fileBytes.data(), (int) fileBytes.size());
    EXPECT_EQ (status, 0);

    status = zipCloseFileInZip (zf);
    EXPECT_EQ (status, 0);

    Utf8CP comment = "CompressionTests_CreateZipFile";
    status = zipClose (zf, comment);
    EXPECT_EQ (status, 0);

    // inspect the zip file that was just created

    unzFile uf = unzOpen (zipFileName.GetNameUtf8().c_str());
    EXPECT_TRUE (nullptr != uf);

    unz_global_info unzGlobalInfo;
    status = unzGetGlobalInfo (uf, &unzGlobalInfo);
    EXPECT_EQ (status, 0);
    EXPECT_EQ (unzGlobalInfo.number_entry, 1);
    EXPECT_EQ (unzGlobalInfo.size_comment, strlen (comment));

    status = unzClose (uf);
    EXPECT_EQ (status, 0);
    }
