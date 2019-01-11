/*--------------------------------------------------------------------------------------+
|
|     $Source: BeLzma.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// lzma stuff includes Windows.h! Include it first!
#include "liblzma/Types.h"
#include "liblzma/Lzma2Enc.h"
#include "liblzma/Lzma2Dec.h"
#undef min
#undef max

#include <BeSQLite/BeLzma.h>
#include <Logging/bentleylogging.h>

#define LOG (*NativeLogging::LoggingManager::GetLogger(L"BeSQLite"))

#ifndef __LZMA2_ENC_H
typedef struct _CLzma2EncProps CLzma2EncProps;
#endif

BEGIN_BENTLEY_SQLITE_NAMESPACE

// Functions needed for 7z.  These are for a C API that simulates C++.
static void *allocFor7z(void *p, size_t size)   {return malloc(size);}
static void freeFor7z(void *p, void *address)    {free(address);}
static SRes readFor7z(void *p, void *buf, size_t *size);
static size_t writeFor7z(void *p, const void *buf, size_t size);
static SRes progressFor7z(void *p, UInt64 inSize, UInt64 outSize);

//=======================================================================================
// Provides an implementation of the 7z C struct ISeqInStream.
// @bsiclass                                                    John.Gooding    01/2013
//=======================================================================================
struct SeqInStreamImpl : ::ISeqInStream
    {
    private:
        ILzmaInputStream& m_stream;

    public:
        SeqInStreamImpl(ILzmaInputStream& in) : m_stream(in) { Read = readFor7z; }
        ILzmaInputStream& GetStream() { return m_stream; }
        SRes ReadData(void *buf, size_t *size) { return Read(this, buf, size); }
        uint64_t GetSize() { return m_stream._GetSize(); }
    };

//=======================================================================================
// Provides an implementation of the 7z C struct ISeqOutStream.
// @bsiclass                                                    John.Gooding    01/2013
//=======================================================================================
struct SeqOutStreamImpl : ::ISeqOutStream
    {
    private:
        ILzmaOutputStream&  m_stream;

    public:
        ILzmaOutputStream& GetStream() { return m_stream; }
        SeqOutStreamImpl(ILzmaOutputStream& outStream) : m_stream(outStream) { Write = writeFor7z; }
        size_t WriteData(const void *buf, size_t size) { return Write(this, buf, size); }
        void SetAlwaysFlush(bool flushOnEveryWrite) { m_stream._SetAlwaysFlush(flushOnEveryWrite); }
    };

//=======================================================================================
// @bsiclass                                                    John.Gooding    01/2013
//=======================================================================================
struct  ICompressProgressImpl : ::ICompressProgress
    {
    ICompressProgressTracker*   m_tracker;
    ICompressProgressImpl(ICompressProgressTracker* tracker) : m_tracker(tracker) { Progress = progressFor7z; }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
static SRes readFor7z(void *p, void *buf, size_t *size)
    {
    SeqInStreamImpl* pImpl = static_cast <SeqInStreamImpl*>(p);

    uint32_t bytesRead = 0;
    ZipErrors zipError = pImpl->GetStream()._Read(buf, (uint32_t) *size, bytesRead);
    *size = bytesRead;

    return ZIP_SUCCESS == zipError ? SZ_OK : SZ_ERROR_READ;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
static size_t writeFor7z(void *p, const void *buf, size_t size)
    {
    SeqOutStreamImpl* pImpl = static_cast<SeqOutStreamImpl*>(p);
    uint32_t bytesWritten;
    pImpl->GetStream()._Write(buf, (uint32_t) size, bytesWritten);

    return bytesWritten;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
static SRes progressFor7z(void *p, UInt64 inSize, UInt64 outSize)
    {
    ICompressProgressImpl* impl = (ICompressProgressImpl*) p;

    BeAssert(impl->m_tracker);  //  SetICompressProgress does not use a ICompressProgressImpl with m_tracker == NULL;

    if (impl->m_tracker->_Progress(inSize, (int64_t) outSize) != ZIP_SUCCESS)
        return SZ_ERROR_PROGRESS;

    return SZ_OK;
    }

#define DECODE_INPUT_BUFFER_SIZE (32 * 1024)
#define ENCODE_INPUT_BUFFER_SIZE (32 * 1024)
#define DECODE_OUTPUT_BUFFER_SIZE (32 * 1024)

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
static ZipErrors translate7ZipError(SRes code, bool readingFromFile)
    {
    switch (code)
        {
            case SZ_OK:
                return ZIP_SUCCESS;
            case SZ_ERROR_DATA:
            case SZ_ERROR_ARCHIVE:
            case SZ_ERROR_NO_ARCHIVE:
                return ZIP_ERROR_BAD_DATA;

            case SZ_ERROR_INPUT_EOF:
                return ZIP_ERROR_END_OF_DATA;

            case SZ_ERROR_READ:
                if (readingFromFile)
                    return ZIP_ERROR_READ_ERROR;

                return ZIP_ERROR_BLOB_READ_ERROR;

            case SZ_ERROR_OUTPUT_EOF:
            case SZ_ERROR_WRITE:
                return ZIP_ERROR_WRITE_ERROR;

            case SZ_ERROR_PROGRESS:
                return ZIP_ERROR_ABORTED;  //  ICompressProgress returned something other than ZIP_SUCCESS
        }

    BeAssert(ZIP_SUCCESS == code);
    return ZIP_ERROR_UNKNOWN;
    }

//---------------------------------------------------------------------------------------
//  This is based on LZMA2_DIC_SIZE_FROM_PROP.  This is the size of the dictionary
//  they allocate when decompressing.  It makes sense for us to use same
//  boundaries as the LZMA library.
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
static uint32_t getDictionarySize(uint32_t dictionarySize)
    {
    for (unsigned i = 0; i < 40; i++)
        {
        uint32_t computed = (2 | (i & 1)) << (i / 2 + 11);
        if (dictionarySize <= computed)
            return computed;
        }

    return dictionarySize;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson    07/2018
//---------------------------------------------------------------------------------------
BeFileStatus BlockFilesLzmaInStream::OpenNextFile()
    {
    if (m_file.IsOpen())
        m_file.Close();

    if (m_nextFile == m_files.size())
        return BeFileStatus::ReadError;

    BeFileName fileName = m_files[m_nextFile];

    m_bytesRead = 0;
    BeFileStatus result = m_file.Open(fileName.GetName(), BeFileAccess::Read);
    if (BeFileStatus::Success != result)
        return result;

    ++m_nextFile;

    BeFileName::GetFileSize(m_fileSize, fileName.GetName());
    m_file.SetPointer(0, BeFileSeekOrigin::Begin);

    return BeFileStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson    07/2018
//---------------------------------------------------------------------------------------
ZipErrors BlockFilesLzmaInStream::_Read(void *data, uint32_t size, uint32_t &actuallyRead)
    {
    BeFileStatus result = m_file.Read(data, &actuallyRead, size);
    if ((BeFileStatus::Success == result) && (actuallyRead == 0))
        {
        if (BeFileStatus::Success != OpenNextFile())
            {
            actuallyRead = 0;
            return ZIP_SUCCESS; // It is normal for _InputPage to be called one more time after all data has been read. In fact, returning 0 here is what tells SQLite that the stream is done.
            }
        result = m_file.Read(data, &actuallyRead, size);
        }

    m_bytesRead += actuallyRead;

    if (BeFileStatus::Success != result)
        {
        // LOG.errorv("BeFileLzmaInStream::_Read result = %d, m_bytesRead = %lld, filesize = %lld, error = %x", result, m_bytesRead, m_fileSize, m_file.GetLastError());
        }

    return BeFileStatus::Success != result ? ZIP_ERROR_READ_ERROR : ZIP_SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2013
//---------------------------------------------------------------------------------------
StatusInt BeFileLzmaInStream::OpenInputFile(BeFileNameCR fileName)
    {
    if (m_file.IsOpen())
        return BSIERROR;

    m_bytesRead = 0;
    BeFileStatus    result = m_file.Open(fileName.GetName(), BeFileAccess::Read);
    if (BeFileStatus::Success != result)
        return (StatusInt)result;

    BeFileName::GetFileSize(m_fileSize, fileName.GetName());
    m_file.SetPointer(0, BeFileSeekOrigin::Begin);

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2013
//---------------------------------------------------------------------------------------
ZipErrors BeFileLzmaInStream::_Read(void* data, uint32_t size, uint32_t& actuallyRead)
    {
    BeFileStatus result = m_file.Read(data, &actuallyRead, size);
    m_bytesRead += actuallyRead;

    if (BeFileStatus::Success != result)
        {
        LOG.errorv("BeFileLzmaInStream::_Read result = %d, m_bytesRead = %lld, filesize = %lld, error = %x", result, m_bytesRead, m_fileSize, m_file.GetLastError());
        }

    return BeFileStatus::Success != result ? ZIP_ERROR_READ_ERROR : ZIP_SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
BeFileStatus BeFileLzmaOutStream::CreateOutputFile(BeFileNameCR fileName, bool createAlways)
    {
    if (m_file.IsOpen())
        return BeFileStatus::UnknownError;

    m_bytesWritten = 0;
    BeFileStatus    result = m_file.Create(fileName.GetName(), createAlways);
    if (BeFileStatus::Success != result)
        return result;

    m_file.SetPointer(0, BeFileSeekOrigin::Begin);

    return BeFileStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2013
//---------------------------------------------------------------------------------------
void BeFileLzmaOutStream::_SetAlwaysFlush(bool flushOnEveryWrite) {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
ZipErrors BeFileLzmaOutStream::_Write(void const* data, uint32_t size, uint32_t& bytesWritten)
    {
    //  The LZMA2 multi-threading support ensures that calls to _Read are sequential and do not overlap, so this code does not need to
    //  be concerned with preventing race conditions
    BeFileStatus result = m_file.Write(&bytesWritten, data, size);
    //  We check m_bytesWritten at the end to verify that we have processed exactly the expected number of bytes.
    m_bytesWritten += bytesWritten;

    if (bytesWritten != size)
        LOG.errorv("BeFileLzmaOutStream::_Write %u requested, %u written", size, bytesWritten);
    if (BeFileStatus::Success != result)
        return ZIP_ERROR_WRITE_ERROR;

    return ZIP_SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
ZipErrors MemoryLzmaInStream::_Read(void* data, uint32_t size, uint32_t& actuallyRead)
    {
    //  The LZMA2 multithreading ensures that calls to _Read are sequential and do not overlap, so this code does not need to
    //  be concerned with preventing race conditions
    uint32_t readFromHeader = 0;
    if (m_headerData)
        {
        readFromHeader = std::min(m_headerSize - m_headerOffset, size);
        memcpy(data, (Byte const*) m_headerData + m_headerOffset, readFromHeader);
        if ((m_headerOffset += readFromHeader) >= m_headerSize)
            m_headerData = nullptr; //  don't use it again
        BeAssert(size >= readFromHeader);
        size -= readFromHeader;
        data = (Byte*) data + readFromHeader;
        }

    actuallyRead = std::min(size, m_size - m_offset);

    memcpy(data, (Byte const*) m_data + m_offset, actuallyRead);
    m_offset += actuallyRead;

    actuallyRead += readFromHeader;
    return ZIP_SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
ZipErrors MemoryLzmaOutStream::_Write(void const* data, uint32_t writeSize, uint32_t&bytesWritten)
    {
    //  The LZMA2 multi-threading support  ensures that calls to _Read are sequential and do not overlap, so this code does not need to
    //  be concerned with preventing race conditions
    size_t oldSize = m_buffer.size();
    m_buffer.resize(oldSize + writeSize);
    memcpy(&m_buffer[oldSize], data, writeSize);
    bytesWritten = writeSize;
    return ZIP_SUCCESS;
    }

//=======================================================================================
// Helper utility to encode a buffer
// Note: This utility exists only since Lzma2Encode is commented out for some unknown reason in the Lzma vendor API. 
// @bsiclass                                                  Ramanujam.Raman    11/16
//=======================================================================================
struct LzmaBufferEncoder
{
private:
    MemoryLzmaInStream m_inStream;
    SeqInStreamImpl m_inSeqStream;
    bvector<Byte> m_outBuffer;
    MemoryLzmaOutStream m_outStream;
    SeqOutStreamImpl m_outSeqStream;
    
public:
    LzmaBufferEncoder() : m_inStream(nullptr, 0), m_outStream(m_outBuffer), m_inSeqStream(m_inStream), m_outSeqStream(m_outStream)
        {}

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                  Ramanujam.Raman    11/16
    //---------------------------------------------------------------------------------------
    ZipErrors EncodeBuffer(::CLzma2EncHandle encHandle, void* &pOutData, int &nOutData, void const* pInData, int nInData, ICompressProgressImpl* progressImpl)
        {
        m_outBuffer.resize(0);
        m_outBuffer.reserve(nInData); // The compressed data ought to be smaller

        BeAssert(nInData > 0);
        m_inStream.SetData(pInData, (uint32_t) nInData);
        int res = Lzma2Enc_Encode(encHandle, &m_outSeqStream, &m_inSeqStream, progressImpl);
        if (SZ_OK != res)
            return translate7ZipError(res, true);

        pOutData = (void*) &m_outBuffer[0];
        nOutData = (int) m_outBuffer.size();

        return ZIP_SUCCESS;
        }
};

//=======================================================================================
// @bsiclass                                                  Ramanujam.Raman    11/16
//=======================================================================================
struct LzmaEncoder::Impl
{
private:
    ::ISzAlloc m_szAlloc;
    CLzma2EncProps* m_enc2Props;
    ::CLzma2EncHandle m_encHandle;
    bool m_supportRandomAccess;
    ICompressProgressImpl* m_progressImpl;

    LzmaBufferEncoder m_bufferEncoder;

    SeqInStreamImpl* m_inStream;
    SeqOutStreamImpl* m_outStream;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                  Ramanujam.Raman    11/16
    //---------------------------------------------------------------------------------------
    void InitProps(uint32_t dictionarySize)
        {
        m_enc2Props = new CLzma2EncProps();
        Lzma2EncProps_Init(m_enc2Props);
        m_enc2Props->lzmaProps.dictSize = (int) getDictionarySize(dictionarySize);
        m_enc2Props->lzmaProps.level = 7;
        m_enc2Props->numTotalThreads = 8;
        Lzma2EncProps_Normalize(m_enc2Props);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                  Ramanujam.Raman    11/16
    //---------------------------------------------------------------------------------------
    void FinalizeProps()
        {
        delete m_enc2Props;
        m_enc2Props = nullptr;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                  Ramanujam.Raman    11/16
    //---------------------------------------------------------------------------------------
    void InitStreams(ILzmaOutputStream* outStream, ILzmaInputStream* inStream)
        {
        BeAssert(m_inStream == nullptr && m_outStream == nullptr);
        if (inStream != nullptr)
            m_inStream = new SeqInStreamImpl(*inStream);
        if (outStream != nullptr)
            m_outStream = new SeqOutStreamImpl(*outStream);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                  Ramanujam.Raman    11/16
    //---------------------------------------------------------------------------------------
    void FinalizeStreams()
        {
        if (m_inStream != nullptr)
            {
            delete m_inStream;
            m_inStream = nullptr;
            }

        if (m_outStream != nullptr)
            {
            delete m_outStream;
            m_outStream = nullptr;
            }
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                  Ramanujam.Raman    11/16
    //---------------------------------------------------------------------------------------
    ZipErrors InitHandle()
        {
        BeAssert(m_encHandle == 0);

        m_encHandle = Lzma2Enc_Create(&m_szAlloc, &m_szAlloc);
        if (m_encHandle == 0)
            return ZIP_ERROR_UNKNOWN;

        int res = Lzma2Enc_SetProps(m_encHandle, m_enc2Props);
        if (SZ_OK != res)
            return ZIP_ERROR_UNKNOWN;

        return ZIP_SUCCESS;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                  Ramanujam.Raman    11/16
    //---------------------------------------------------------------------------------------
    void FinalizeHandle()
        {
        if (m_encHandle != 0)
            {
            Lzma2Enc_Destroy(m_encHandle);
            m_encHandle = 0;
            }
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                  Ramanujam.Raman    11/16
    //---------------------------------------------------------------------------------------
    void FinalizeProgressTracker()
        {
        if (m_progressImpl)
            {
            delete m_progressImpl;
            m_progressImpl = nullptr;
            }
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                  Ramanujam.Raman    11/16
    //---------------------------------------------------------------------------------------
    ZipErrors WriteHeader()
        {
        Byte header;
        size_t headerSize = sizeof(header);
        header = Lzma2Enc_WriteProperties(m_encHandle);

        if (m_outStream->WriteData(&header, headerSize) != headerSize)
            return ZIP_ERROR_WRITE_ERROR;

        if (m_supportRandomAccess)
            {
            //  Setting alwaysFlush to true forces PropertyBlobOutStream to create a new embedded blob for each write. Since LZMA2 calls
            //  _Write whenever it finishes processing a a block this creates a one-to-one mapping from input blocks to embedded blobs.
            //  That makes it possible to read and expand any given block, randomly accessing the blocks.
            m_outStream->SetAlwaysFlush(true);
            }

        return ZIP_SUCCESS;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                  Ramanujam.Raman    11/16
    //---------------------------------------------------------------------------------------
    ZipErrors WriteEndOfStream()
        {
        Byte eos = 0;
        if (m_outStream->WriteData(&eos, 1) != 1)
            {
            BeAssert(false);
            return ZIP_ERROR_UNKNOWN;
            }
        return ZIP_SUCCESS;
        }

public:
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                  Ramanujam.Raman    11/16
    //---------------------------------------------------------------------------------------
    Impl(uint32_t dictionarySize, bool supportRandomAccess) : m_inStream(nullptr), m_outStream(nullptr), m_supportRandomAccess(supportRandomAccess), m_progressImpl(nullptr), m_encHandle(0)
        {
        m_szAlloc.Alloc = allocFor7z;
        m_szAlloc.Free = freeFor7z;
        InitProps(dictionarySize);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                  Ramanujam.Raman    11/16
    //---------------------------------------------------------------------------------------
    ~Impl()
        {
        FinalizeProgressTracker();
        FinalizeHandle();
        FinalizeStreams();
        FinalizeProps();
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                  Ramanujam.Raman    11/16
    //---------------------------------------------------------------------------------------
    ZipErrors StartCompress(ILzmaOutputStream& outStream)
        {
        if (m_outStream != nullptr)
            {
            BeAssert(false && "Cannot call StartCompress again without calling FinishCompress");
            return ZIP_ERROR_UNKNOWN;
            }

        InitStreams(&outStream, nullptr);
        
        ZipErrors zipStatus = InitHandle();
        if (ZIP_SUCCESS != zipStatus)
            return zipStatus;

        zipStatus = WriteHeader();
        if (ZIP_SUCCESS != zipStatus)
            return zipStatus;

        return ZIP_SUCCESS;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                  Ramanujam.Raman    11/16
    //---------------------------------------------------------------------------------------
    ZipErrors FinishCompress()
        {
        if (m_outStream == nullptr)
            {
            BeAssert(false && "Cannot call FinishCompress without calling StartCompress");
            return ZIP_ERROR_UNKNOWN;
            }

        ZipErrors zipStatus = WriteEndOfStream(); // We skip the end of stream '0' on incremental writes, and instead do it at the end. 

        FinalizeHandle();
        FinalizeStreams();

        return zipStatus;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                  Ramanujam.Raman    11/16
    //---------------------------------------------------------------------------------------
    void SetProgressTracker(ICompressProgressTracker* progressTracker)
        {
        if (m_progressImpl)
            FinalizeProgressTracker();

        if (progressTracker)
            m_progressImpl = new ICompressProgressImpl(progressTracker);
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                  Ramanujam.Raman    11/16
    //---------------------------------------------------------------------------------------
    ZipErrors CompressNextPage(void const* pData, int nData)
        {
        if (m_outStream == nullptr)
            {
            BeAssert(false && "Cannot call CompressNextPage without calling StartCompress");
            return ZIP_ERROR_UNKNOWN;
            }
        
        void *pOutData = nullptr;
        int nOutData;
        
        ZipErrors zipStatus = m_bufferEncoder.EncodeBuffer(m_encHandle, pOutData, nOutData, pData, nData, m_progressImpl);
        if (zipStatus != ZIP_SUCCESS)
            return zipStatus;

        // Skip the last end of stream character (0)
        nOutData--;
        BeAssert(nOutData >= 0 && *((Byte*) pOutData + nOutData) == 0);

        if (m_outStream->WriteData(pOutData, nOutData) != nOutData)
            {
            BeAssert(false);
            return ZIP_ERROR_WRITE_ERROR;
            }

        return ZIP_SUCCESS;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   John.Gooding    11/2013
    //---------------------------------------------------------------------------------------
    void SetBlockSize(uint32_t blockSize)
        {
        m_enc2Props->blockSize = blockSize < m_enc2Props->lzmaProps.dictSize ? m_enc2Props->lzmaProps.dictSize : blockSize;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                  Ramanujam.Raman    11/16
    //---------------------------------------------------------------------------------------
    ZipErrors CompressStream(ILzmaOutputStream& outStream, ILzmaInputStream& inStream)
        {
        if (m_inStream != nullptr || m_outStream != nullptr)
            {
            BeAssert(false && "In the middle of another streaming operation. Finish that first!!");
            return ZIP_ERROR_WRITE_ERROR;
            }

        InitStreams(&outStream, &inStream);

        ZipErrors zipStatus = InitHandle();
        if (ZIP_SUCCESS != zipStatus)
            return zipStatus;

        zipStatus = WriteHeader();
        if (ZIP_SUCCESS != zipStatus)
            return zipStatus;

        int res = Lzma2Enc_Encode(m_encHandle, m_outStream, m_inStream, m_progressImpl);

        FinalizeHandle();
        FinalizeStreams();

        return (SZ_OK == res) ? ZIP_SUCCESS : translate7ZipError(res, true);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   John.Gooding    01/2013
    //---------------------------------------------------------------------------------------
    ZipErrors CompressBuffer(bvector<Byte>& out, void const *input, uint32_t sizeInput)
        {
        MemoryLzmaInStream  inStream(input, sizeInput);
        MemoryLzmaOutStream outStream(out);

        return CompressStream(outStream, inStream);
        }

};

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Ramanujam.Raman    11/16
//---------------------------------------------------------------------------------------
LzmaEncoder::LzmaEncoder(uint32_t dictionarySize /*= 1 << 24*/, bool supportRandomAccess /*=false*/)
    {
    m_impl = new LzmaEncoder::Impl(dictionarySize, supportRandomAccess);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Ramanujam.Raman    11/16
//---------------------------------------------------------------------------------------
LzmaEncoder::~LzmaEncoder()
    {
    delete m_impl;
    m_impl = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Ramanujam.Raman    11/16
//---------------------------------------------------------------------------------------
ZipErrors LzmaEncoder::CompressNextPage(void const* pData, int nData)
    {
    return m_impl->CompressNextPage(pData, nData);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Ramanujam.Raman    11/16
//---------------------------------------------------------------------------------------
void LzmaEncoder::SetProgressTracker(ICompressProgressTracker* progressTracker)
    {
    m_impl->SetProgressTracker(progressTracker);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Ramanujam.Raman    11/16
//---------------------------------------------------------------------------------------
ZipErrors LzmaEncoder::StartCompress(ILzmaOutputStream& outStream)
    {
    return m_impl->StartCompress(outStream);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Ramanujam.Raman    11/16
//---------------------------------------------------------------------------------------
ZipErrors LzmaEncoder::FinishCompress()
    {
    return m_impl->FinishCompress();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2013
//---------------------------------------------------------------------------------------
void LzmaEncoder::SetBlockSize(uint32_t blockSize)
    {
    m_impl->SetBlockSize(blockSize);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
ZipErrors LzmaEncoder::CompressStream(ILzmaOutputStream& outStream, ILzmaInputStream& inStream)
    {
    return m_impl->CompressStream(outStream, inStream);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
ZipErrors LzmaEncoder::CompressBuffer(bvector<Byte>& out, void const *input, uint32_t sizeInput)
    {
    return m_impl->CompressBuffer(out, input, sizeInput);
    }

//=======================================================================================
// @bsiclass                                                  Ramanujam.Raman    11/16
//=======================================================================================
struct LzmaDecoder::Impl
{
private:
    ::ISzAlloc m_szAlloc;
    CLzma2Dec m_decodeState;
    bool m_initState;
    ICompressProgressImpl* m_progressImpl;

    SeqInStreamImpl* m_inStream;
    SeqOutStreamImpl* m_outStream;

    Byte m_inBuf[DECODE_INPUT_BUFFER_SIZE];
    size_t m_inSize;
    size_t m_inPos;
    uint64_t m_totalRead;
    uint64_t m_totalWritten;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                  Ramanujam.Raman    11/16
    //---------------------------------------------------------------------------------------
    void InitStreams(ILzmaOutputStream* outStream, ILzmaInputStream* inStream)
        {
        BeAssert(m_inStream == nullptr && m_outStream == nullptr);
        if (inStream != nullptr)
            m_inStream = new SeqInStreamImpl(*inStream);
        if (outStream != nullptr)
            m_outStream = new SeqOutStreamImpl(*outStream);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                  Ramanujam.Raman    11/16
    //---------------------------------------------------------------------------------------
    void FinalizeStreams()
        {
        if (m_inStream != nullptr)
            {
            delete m_inStream;
            m_inStream = nullptr;
            }

        if (m_outStream != nullptr)
            {
            delete m_outStream;
            m_outStream = nullptr;
            }
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                  Ramanujam.Raman    11/16
    //---------------------------------------------------------------------------------------
    ZipErrors InitState()
        {
        Lzma2Dec_Construct(&m_decodeState);

        //  1 byte describing the properties and 8 bytes of uncompressed size
        Byte header;
        size_t  readSize = sizeof(header);
        if (m_inStream->ReadData(&header, &readSize) != BSISUCCESS)
            return ZIP_ERROR_BAD_DATA;

        Lzma2Dec_Allocate(&m_decodeState, header, &m_szAlloc);
        Lzma2Dec_Init(&m_decodeState);

        m_initState = true;
        return ZIP_SUCCESS;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                  Ramanujam.Raman    11/16
    //---------------------------------------------------------------------------------------
    void FinalizeState()
        {
        if (m_initState)
            {
            Lzma2Dec_Free(&m_decodeState, &m_szAlloc);
            m_initState = false;
            }
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                  Ramanujam.Raman    11/16
    //---------------------------------------------------------------------------------------
    void FinalizeProgressTracker()
        {
        if (m_progressImpl)
            {
            delete m_progressImpl;
            m_progressImpl = nullptr;
            }
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                  Ramanujam.Raman    11/16
    //---------------------------------------------------------------------------------------
    ZipErrors DecompressStreamLow()
        {
        Byte outBuf[DECODE_OUTPUT_BUFFER_SIZE];

        for (;;)
            {
            int nOut = DECODE_OUTPUT_BUFFER_SIZE;
            ZipErrors zipStatus = DecompressNextPage(&outBuf[0], &nOut);
            if (zipStatus != ZIP_SUCCESS)
                return zipStatus;

            if (m_outStream->WriteData(outBuf, (size_t) nOut) != (size_t) nOut)
                return ZIP_ERROR_WRITE_ERROR;

            if (nOut == 0)
                return ZIP_SUCCESS;
            }
        }

public:
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                  Ramanujam.Raman    11/16
    //---------------------------------------------------------------------------------------
    Impl() : m_inStream(nullptr), m_outStream(nullptr), m_initState(false), m_progressImpl(nullptr)
        {
        m_szAlloc.Alloc = allocFor7z;
        m_szAlloc.Free = freeFor7z;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                  Ramanujam.Raman    11/16
    //---------------------------------------------------------------------------------------
    ~Impl()
        {
        FinalizeProgressTracker();
        FinalizeState();
        FinalizeStreams();
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                  Ramanujam.Raman    11/16
    //---------------------------------------------------------------------------------------
    void SetProgressTracker(ICompressProgressTracker* progressTracker)
        {
        if (m_progressImpl)
            FinalizeProgressTracker();

        if (progressTracker)
            m_progressImpl = new ICompressProgressImpl(progressTracker);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                  Ramanujam.Raman    11/16
    //---------------------------------------------------------------------------------------
    ZipErrors StartDecompress(ILzmaInputStream& inStream)
        {
        if (m_inStream != nullptr)
            {
            BeAssert(false && "Cannot call StartDecompress again without calling FinishDecompress");
            return ZIP_ERROR_UNKNOWN;
            }

        InitStreams(nullptr, &inStream);
        
        ZipErrors zipStatus = InitState();
        if (ZIP_SUCCESS != zipStatus)
            return zipStatus;

        m_inSize = m_inPos = 0;
        m_totalRead = m_totalWritten = 0;
        return ZIP_SUCCESS;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                  Ramanujam.Raman    11/16
    //---------------------------------------------------------------------------------------
    void FinishDecompress()
        {
        if (m_inStream == nullptr)
            {
            BeAssert(false && "Cannot call FinishDecompress without calling StartDecompress");
            return;
            }

        FinalizeState();
        FinalizeStreams();
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                  Ramanujam.Raman    11/16
    //---------------------------------------------------------------------------------------
    ZipErrors DecompressNextPage(void* pData, int* pnData)
        {
        if (m_inStream == nullptr)
            {
            BeAssert(false && "Cannot call DecompressNextPage without calling StartDecompress");
            return ZIP_ERROR_UNKNOWN;
            }

        for (;;)
            {
            if (m_inPos == m_inSize)
                {
                m_inSize = DECODE_INPUT_BUFFER_SIZE;
                //  We ignore the read error here. It probably means end-of-stream.  We count on LZMA detecting that it
                //  has hit the end of the input data without hitting the end of the stream.  As a failsafe Export verifies that the output file is the expected size.
                m_inStream->ReadData(m_inBuf, &m_inSize);

                if (m_inSize == 0)
                    {
                    *pnData = 0;
                    return ZIP_SUCCESS;
                    }

                m_totalRead += m_inSize;
                m_inPos = 0;
                }

            SizeT inProcessed = m_inSize - m_inPos;
            SizeT outProcessed = *pnData;

            ELzmaStatus status;
            SRes res = Lzma2Dec_DecodeToBuf(&m_decodeState, (Byte*) pData, &outProcessed, m_inBuf + m_inPos, &inProcessed, LZMA_FINISH_ANY, &status);

            m_inPos += inProcessed;

            if (nullptr != m_progressImpl)
                res = m_progressImpl->Progress(m_progressImpl, m_totalRead, m_totalWritten);

            if (res != SZ_OK)
                return translate7ZipError(res, false);

            if (outProcessed > 0)
                {
                *pnData = (int) outProcessed;
                m_totalWritten += outProcessed;
                return ZIP_SUCCESS;
                }
            
            if (inProcessed == 0 /* && outProcessed == 0 */)
                {
                BeAssert(false && "Nothing in the input has been processed - is this even possible?");
                return ZIP_ERROR_END_OF_DATA;
                //return (status == LZMA_STATUS_FINISHED_WITH_MARK) ? ZIP_SUCCESS : ZIP_ERROR_END_OF_DATA;
                }
            }

        return ZIP_SUCCESS;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                  Ramanujam.Raman    11/16
    //---------------------------------------------------------------------------------------
    ZipErrors DecompressStream(ILzmaOutputStream& outStream, ILzmaInputStream& inStream)
        {
        if (m_inStream != nullptr || m_outStream != nullptr)
            {
            BeAssert(false && "In the middle of another streaming operation. Finish that first!!");
            return ZIP_ERROR_WRITE_ERROR;
            }

        InitStreams(&outStream, &inStream);

        ZipErrors zipStatus = InitState();
        if (ZIP_SUCCESS != zipStatus)
            return zipStatus;

        m_inSize = m_inPos = 0;
        m_totalRead = m_totalWritten = 0;

        zipStatus = DecompressStreamLow();

        FinalizeState();
        FinalizeStreams();

        return zipStatus;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   John.Gooding    11/2013
    //---------------------------------------------------------------------------------------
    ZipErrors DecompressBuffer(bvector<Byte>&out, void const* inputBuffer, uint32_t inputSize)
        {
        MemoryLzmaInStream  inStream(inputBuffer, inputSize);
        MemoryLzmaOutStream outStream(out);

        return DecompressStream(outStream, inStream);
        }

};

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Ramanujam.Raman    11/16
//---------------------------------------------------------------------------------------
LzmaDecoder::LzmaDecoder()
    {
    m_impl = new LzmaDecoder::Impl();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Ramanujam.Raman    11/16
//---------------------------------------------------------------------------------------
LzmaDecoder::~LzmaDecoder()
    {
    delete m_impl;
    m_impl = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Ramanujam.Raman    11/16
//---------------------------------------------------------------------------------------
ZipErrors LzmaDecoder::DecompressNextPage(void *pData, int *pnData)
    {
    return m_impl->DecompressNextPage(pData, pnData);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Ramanujam.Raman    11/16
//---------------------------------------------------------------------------------------
ZipErrors LzmaDecoder::StartDecompress(ILzmaInputStream& inStream)
    {
    return m_impl->StartDecompress(inStream);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Ramanujam.Raman    11/16
//---------------------------------------------------------------------------------------
void LzmaDecoder::FinishDecompress()
    {
    m_impl->FinishDecompress();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Ramanujam.Raman    11/16
//---------------------------------------------------------------------------------------
void LzmaDecoder::SetProgressTracker(ICompressProgressTracker* progressTracker)
    {
    m_impl->SetProgressTracker(progressTracker);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Ramanujam.Raman    11/16
//---------------------------------------------------------------------------------------
ZipErrors LzmaDecoder::DecompressStream(ILzmaOutputStream& outStream, ILzmaInputStream& inStream)
    {
    return m_impl->DecompressStream(outStream, inStream);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2013
//---------------------------------------------------------------------------------------
ZipErrors LzmaDecoder::DecompressBuffer(bvector<Byte>&out, void const* inputBuffer, uint32_t inputSize)
    {
    return m_impl->DecompressBuffer(out, inputBuffer, inputSize);
    }

END_BENTLEY_SQLITE_NAMESPACE
