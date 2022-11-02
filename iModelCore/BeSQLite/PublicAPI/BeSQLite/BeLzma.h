/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "BeSQLite.h"

BEGIN_BENTLEY_SQLITE_NAMESPACE

//=======================================================================================
//! Interface to receive/write streamed Lzma output
// @bsiclass
//=======================================================================================
struct ILzmaOutputStream
{
    virtual ZipErrors _Write(void const* data, uint32_t size, uint32_t&bytesWritten) = 0;
    virtual void _SetAlwaysFlush(bool flushOnEveryWrite) = 0;
};

//=======================================================================================
//! Interface to send/read streamed Lzma input
// @bsiclass
//=======================================================================================
struct ILzmaInputStream
{
    virtual ZipErrors _Read(void* data, uint32_t size, uint32_t& actuallyRead) = 0;
    virtual uint64_t _GetSize() = 0;
};

//=======================================================================================
//! Utility to stream the contents of a file to LZMA routines from multiple
//! sequential "block" files.
// @bsiclass
//=======================================================================================
struct BlockFilesLzmaInStream : ILzmaInputStream
{
private:
    uint32_t m_nextFile;
    bvector<BeFileName> m_files;
    BeFile m_file;
    uint64_t m_fileSize;
    uint64_t m_bytesRead;

    BE_SQLITE_EXPORT BeFileStatus OpenNextFile();

public:
    BlockFilesLzmaInStream(bvector<BeFileName> const &files) : m_files(files), m_nextFile(0) { OpenNextFile(); }
    virtual ~BlockFilesLzmaInStream() {}
    bool IsReady() const { return m_file.IsOpen(); }
    BE_SQLITE_EXPORT ZipErrors _Read(void *data, uint32_t size, uint32_t &actuallyRead) override;
    uint64_t _GetSize() override { return m_fileSize; }
    uint64_t GetBytesRead() { return m_bytesRead; }
    BeFile &GetBeFile() { return m_file; }
};

//=======================================================================================
//! Utility to stream the contents of a file to LZMA routines
// @bsiclass
//=======================================================================================
struct BeFileLzmaInStream : ILzmaInputStream
{
private:
    BeFile   m_file;
    uint64_t m_fileSize;
    uint64_t m_bytesRead;

public:
    virtual ~BeFileLzmaInStream() {}
    BE_SQLITE_EXPORT StatusInt OpenInputFile(BeFileNameCR fileName);
    BE_SQLITE_EXPORT ZipErrors _Read(void* data, uint32_t size, uint32_t& actuallyRead) override;
    uint64_t _GetSize() override {return m_fileSize;}
    uint64_t GetBytesRead() {return m_bytesRead;}
    BeFile& GetBeFile() {return m_file;}
};

//=======================================================================================
//! Utility to stream the output of LZMA routines to a file
// @bsiclass
//=======================================================================================
struct BeFileLzmaOutStream : ILzmaOutputStream
{
private:
    BeFile      m_file;
    uint64_t    m_bytesWritten;

public:
    virtual ~BeFileLzmaOutStream() {}
    BE_SQLITE_EXPORT BeFileStatus CreateOutputFile(BeFileNameCR fileName, bool createAlways = true);
    BE_SQLITE_EXPORT ZipErrors _Write(void const* data, uint32_t size, uint32_t& bytesWritten) override;
    BE_SQLITE_EXPORT void _SetAlwaysFlush(bool flushOnEveryWrite) override;
    uint64_t GetBytesWritten() {return m_bytesWritten;}
    BeFile& GetBeFile() {return m_file;}
};

//=======================================================================================
//! Utility to stream the contents of a memory buffer to LZMA routines
// @bsiclass
//=======================================================================================
struct MemoryLzmaInStream : ILzmaInputStream
{
private:
    uint32_t    m_offset;
    uint32_t    m_size;
    void const* m_data;

    uint32_t    m_headerOffset;
    uint32_t    m_headerSize;
    void const* m_headerData;

public:
    MemoryLzmaInStream(void const*data, uint32_t size)
        {
        SetData(data, size);
        SetHeaderData(nullptr, 0);
        }

    void SetData(void const* data, uint32_t size)
        {
        m_data = data;
        m_size = size;
        m_offset = 0;
        }

    void SetHeaderData(void const* headerData, uint32_t headerSize)
        {
        m_headerData = headerData;
        m_headerSize = headerSize;
        m_headerOffset = 0;
        }

    BE_SQLITE_EXPORT virtual ZipErrors _Read(void* data, uint32_t size, uint32_t& actuallyRead) override;
    virtual uint64_t _GetSize() override { return m_size + m_headerSize; }
};

//=======================================================================================
//! Utility to stream the output of LZMA routines to a memory buffer
// @bsiclass
//=======================================================================================
struct MemoryLzmaOutStream : ILzmaOutputStream
{
private:
    bvector<Byte>& m_buffer;

public:
    MemoryLzmaOutStream(bvector<Byte>& v) : m_buffer(v) {}
    virtual ~MemoryLzmaOutStream() {}

    BE_SQLITE_EXPORT ZipErrors _Write(void const* data, uint32_t writeSize, uint32_t& bytesWritten) override;
    void _SetAlwaysFlush(bool flushOnEveryWrite) override {}
};

//=======================================================================================
//! Utility to compress and write streams to the LZMA format
// @bsiclass
//=======================================================================================
struct LzmaEncoder
{
    public:
    struct LzmaParams
        {
        public:
        enum class Algorithm : int
            {
            Fast = 0,
            Normal = 1,
            Default = Normal,
            };
        enum class BtMode : int
            {
            HashChain = 0,
            BinTree = 1,
            Default = BinTree,
            };

        private:
            uint32_t m_dictSize;
            bool m_supportRandomAccess;
            int m_level;
            int m_lc;
            int m_lp;
            int m_pb;
            int m_fb;
            int m_numHashBytes;
            uint32_t m_mc;
            unsigned m_writeEndMark;
            BtMode m_btMode;
            int m_numThreads;
            size_t m_blockSize;
            int m_numBlockThreads;
            int m_numTotalThreads;
            Algorithm m_algo;
            void FromProps(void *);
        public:
            BE_SQLITE_EXPORT LzmaParams(uint32_t dictionarySize = 1 << 24, bool supportRandomAccess = false, int level = 7, int threads = 8);
            LzmaParams& SetDictSize(uint32_t dictSize) {m_dictSize = dictSize; return *this;}
            LzmaParams& SetSupportRandomAccess(uint32_t supportRandomAccess) {m_supportRandomAccess = supportRandomAccess; return *this;}
            LzmaParams& SetLevel(int level) {m_level = level; return *this;}
            LzmaParams& SetNumThreads(int threads) {m_numThreads = threads; return *this;}
            LzmaParams& SetAlog(Algorithm algorithm) {m_algo = algorithm; return *this;}
            LzmaParams& SetLC(int lc) {m_lc = lc; return *this;}
            LzmaParams& SetLP(int lp) {m_lp = lp; return *this;}
            LzmaParams& SetPB(int pb) {m_pb = pb; return *this;}
            LzmaParams& SetFB(int fb) {m_fb = fb; return *this;}
            LzmaParams& SetMC(int mc) {m_mc = mc; return *this;}
            LzmaParams& SetNumHashBytes(int numHashBytes) {m_numHashBytes = numHashBytes; return *this;}
            LzmaParams& SetWriteEndMark(int writeEndMark) {m_writeEndMark = writeEndMark; return *this;}
            LzmaParams& SetBTMode(BtMode btMode) {m_btMode = btMode; return *this;}
            LzmaParams& SetBlockSize(size_t size) {m_blockSize = size; return *this;}
            LzmaParams& SetNumBlockThreads(int threads) {m_numBlockThreads = threads; return *this;}
            LzmaParams& SetNumTotalThreads(int threads) {m_numTotalThreads = threads; return *this;}
            bool GetSupportRandomAccess() const { return m_supportRandomAccess; }
            BE_SQLITE_EXPORT void ToJson(BeJsValue) const;
            BE_SQLITE_EXPORT BentleyStatus FromJson(BeJsConst);
            BE_SQLITE_EXPORT void Normalize();
            BE_SQLITE_EXPORT void ToProps(void *) const;
        };

public:
    struct Impl;

private:
    Impl* m_impl;

public:
    //! Constructor
    //! @param[in] dictionarySize Maximum number of bytes to hold the dictionary used for the compression. With a larger dictionary
    //! the compression becomes better, but slower and memory intensive. Default's about 16MB.
    //! @param[in] supportRandomAccess Pass true to create a one to one mapping from input blocks to
    //! output blobs. This makes it possible to randomly access, and decode any given block.
    BE_SQLITE_EXPORT LzmaEncoder(LzmaParams const& params = LzmaParams());

    //! Destructor
    BE_SQLITE_EXPORT ~LzmaEncoder();

    //! Set the progress tracker
    //! @param[in] progressTracker Implementation of progress tracker (pass null to unset)
    BE_SQLITE_EXPORT void SetProgressTracker(ICompressProgressTracker* progressTracker);

    //! Set the block size for the compression
    void SetBlockSize(uint32_t blockSize); //!< @private

    //! Start incremental compression to the supplied output stream
    //! @param[in] outStream Output stream to write to
    BE_SQLITE_EXPORT ZipErrors StartCompress(ILzmaOutputStream& outStream);

    //! Write the next page of data after compression
    //! @param[out] pData Uncompressed buffer of data to be written after compression
    //! @param[in,out] nData Size of buffer
    //! @return ZIP_SUCCESS if successfully extracted data. Return appropriate error otherwise.
    BE_SQLITE_EXPORT ZipErrors CompressNextPage(void const* pData, int nData);

    //! Finish incremental compression.
    BE_SQLITE_EXPORT ZipErrors FinishCompress();

    //! Compress the entire contents of supplied input stream to the supplied output stream
    BE_SQLITE_EXPORT ZipErrors CompressStream(ILzmaOutputStream& out, ILzmaInputStream& in);

    //! Compress a buffer of bytes
    BE_SQLITE_EXPORT ZipErrors CompressBuffer(bvector<Byte>& out, void const *input, uint32_t sizeInput);
};

//=======================================================================================
//! Utility to read and decompress streams in the LZMA format
// @bsiclass
//=======================================================================================
struct LzmaDecoder
{
public:
    struct Impl;

private:
    Impl* m_impl;

public:
    //! Constructor
    BE_SQLITE_EXPORT LzmaDecoder();

    //! Destructor
    BE_SQLITE_EXPORT ~LzmaDecoder();

    //! Set the progress tracker
    //! @param[in] progressTracker Implementation of progress tracker (pass null to unset)
    BE_SQLITE_EXPORT void SetProgressTracker(ICompressProgressTracker* progressTracker);

    //! Start incremental decompression from the supplied input stream
    //! @param[in] inStream Input stream to read from
    BE_SQLITE_EXPORT ZipErrors StartDecompress(ILzmaInputStream& inStream);

    //! Read and decompress the next page
    //! @param[out] pData Buffer to copy the data to (allocated by the client)
    //! @param[in,out] pnData Caller sets this to the size of the buffer. The method then sets it to the actual
    //! number of bytes copied. If the input is exhausted, the value is set to 0.
    //! @return ZIP_SUCCESS if successfully extracted data. Returns appropriate error otherwise.
    BE_SQLITE_EXPORT ZipErrors DecompressNextPage(void* pData, int* pnData);

    //! Finish incremental decompression
    BE_SQLITE_EXPORT void FinishDecompress();

    //! Decompress the entire contents of the supplied input stream to the supplied output stream
    BE_SQLITE_EXPORT ZipErrors DecompressStream(ILzmaOutputStream& outStream, ILzmaInputStream& inStream);

    //! Decompress a buffer of bytes
    BE_SQLITE_EXPORT ZipErrors DecompressBuffer(bvector<Byte>&out, void const*inputBuffer, uint32_t inputSize);
};

END_BENTLEY_SQLITE_NAMESPACE
