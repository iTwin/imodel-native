/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <BeSQLite/ChangesetFile.h>
#include <Bentley/Logging.h>
#include <algorithm>
#include <cstring>
#include <limits>
#include <map>
#include <new>

USING_NAMESPACE_BENTLEY_SQLITE

#define CHANGESET_FORMAT_VERSION  0x10
#define CHANGESET_LZMA_MARKER   "ChangeSetLzma"
#define JSON_PROP_DDL                   "DDL"
#define JSON_PROP_ContainsSchemaChanges "ContainsSchemaChanges"

#define LOG (NativeLogging::CategoryLogger("BeSQLite"))

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void UIntToByteArray(Byte bytes[], uint32_t size) {
    // Note: Not using a union to convert since it may not be portable
    bytes[0] = (size >> 24) & 0xFF;
    bytes[1] = (size >> 16) & 0xFF;
    bytes[2] = (size >> 8) & 0xFF;
    bytes[3] = size & 0xFF;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
uint32_t ByteArrayToUInt(Byte bytes[]) {
    return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | (bytes[3]);
}

//---------------------------------------------------------------------------------------
// Reads the length-prefixed prefix from the start of a decompressed changeset stream.
// The stream contents are untrusted: the declared size is validated, the buffer is
// grown only as data actually arrives, and the string is built with an explicit length
// so a prefix without a NUL terminator can never cause reads past the buffer.
// @bsimethod
//---------------------------------------------------------------------------------------
static BentleyStatus ReadPrefixFromStream(LzmaDecoder& lzmaDecoder, Utf8StringR prefix) {
    prefix.clear();

    Byte sizeBytes[4];
    int readSizeBytes = 4;
    ZipErrors zipStatus = lzmaDecoder.DecompressNextPage(sizeBytes, &readSizeBytes);
    if (zipStatus != ZIP_SUCCESS || readSizeBytes != 4) {
        LOG.error("Changeset is corrupt: could not read size of the changeset prefix");
        return ERROR;
    }

    const uint32_t size = ByteArrayToUInt(sizeBytes);
    if (size == 0)
        return SUCCESS;

    if (size > (uint32_t) std::numeric_limits<int>::max()) {
        LOG.errorv("Changeset is corrupt: invalid changeset prefix size %u", (unsigned) size);
        return ERROR;
    }

    const size_t maxChunkSize = 64 * 1024;
    bvector<Byte> prefixBytes;
    while (prefixBytes.size() < size) {
        const size_t offset = prefixBytes.size();
        const size_t chunkSize = std::min<size_t>(size - offset, maxChunkSize);
        prefixBytes.resize(offset + chunkSize);

        int readSize = (int) chunkSize;
        zipStatus = lzmaDecoder.DecompressNextPage(prefixBytes.data() + offset, &readSize);
        if (zipStatus != ZIP_SUCCESS || readSize <= 0 || (size_t) readSize > chunkSize) {
            LOG.errorv("Changeset is corrupt: changeset prefix is truncated (expected %u bytes, read %u)", (unsigned) size, (unsigned) offset);
            prefix.clear();
            return ERROR;
        }

        prefixBytes.resize(offset + readSize);
    }

    // Writers include the NUL terminator in the size; stop at the first NUL but never read past the buffer.
    Utf8CP prefixChars = reinterpret_cast<Utf8CP>(prefixBytes.data());
    prefix.assign(prefixChars, strnlen(prefixChars, prefixBytes.size()));
    return SUCCESS;
}

//=======================================================================================
// LZMA Header written to the beginning of the changeset file
// @bsiclass
//=======================================================================================
struct ChangesetLzmaHeader {
private:
    uint16_t m_sizeOfHeader;
    char m_idString[15];
    uint16_t m_formatVersionNumber;
    uint16_t m_compressionType;

public:
    static const int formatVersionNumber = CHANGESET_FORMAT_VERSION;
    enum CompressionType {
        LZMA2 = 2
    };

    ChangesetLzmaHeader() {
        CharCP idString = CHANGESET_LZMA_MARKER;
        BeAssert((strlen(idString) + 1) <= sizeof(m_idString));
        memset(this, 0, sizeof(*this));
        m_sizeOfHeader = (uint16_t)sizeof(ChangesetLzmaHeader);
        strcpy(m_idString, idString);
        m_compressionType = CompressionType::LZMA2;
        m_formatVersionNumber = formatVersionNumber;
    }

    int GetVersion() { return m_formatVersionNumber; }

    bool IsValid() {
        if (m_sizeOfHeader != sizeof(ChangesetLzmaHeader))
            return false;

        if (strcmp(m_idString, CHANGESET_LZMA_MARKER))
            return false;

        if (formatVersionNumber != m_formatVersionNumber)
            return false;

        return m_compressionType == LZMA2;
    }
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult ChangesetFileWriter::StartOutput() {
    BeAssert(m_outLzmaFileStream == nullptr);
    m_outLzmaFileStream = new BeFileLzmaOutStream();

    BeFileName::CreateNewDirectory(m_pathname.GetDirectoryName());
    BeFileStatus fileStatus = m_outLzmaFileStream->CreateOutputFile(m_pathname, true); // overwrites any existing file
    if (fileStatus != BeFileStatus::Success) {
        LOG.fatalv(L"%ls - OutLzmaFileStream::CreateOutputFile failed", m_pathname.c_str());
        BeAssert(false);
        return BE_SQLITE_ERROR;
    }

    ChangesetLzmaHeader header;
    uint32_t bytesWritten = 0;
    ZipErrors zipStatus = m_outLzmaFileStream->_Write(&header, sizeof(header), bytesWritten);
    if (zipStatus != ZIP_SUCCESS || bytesWritten != sizeof(header)) {
        BeAssert(false);
        return BE_SQLITE_ERROR;
    }

    zipStatus = m_lzmaEncoder.StartCompress(*m_outLzmaFileStream);
    if (zipStatus != ZIP_SUCCESS) {
        BeAssert(false);
        return BE_SQLITE_ERROR;
    }
    m_compressionStarted = true;

    return WritePrefix();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult ChangesetFileWriter::Finish() {
    if (m_outLzmaFileStream == nullptr)
        return m_outputStatus;

    if (m_compressionStarted) {
        try {
            FlushInput();
        } catch (std::bad_alloc const&) {
            m_outputStatus = BE_SQLITE_NOMEM;
        }
        if (m_lzmaEncoder.FinishCompress() != ZIP_SUCCESS && m_outputStatus == BE_SQLITE_OK)
            m_outputStatus = BE_SQLITE_ERROR;
        m_compressionStarted = false;
    }

    m_pendingInput.clear();
    delete m_outLzmaFileStream;
    m_outLzmaFileStream = nullptr;
    return m_outputStatus;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult ChangesetFileWriter::FlushInput() {
    if (m_outputStatus == BE_SQLITE_OK && !m_pendingInput.empty()) {
        if (m_lzmaEncoder.CompressNextPage(m_pendingInput.data(), m_pendingInput.size()) != ZIP_SUCCESS)
            m_outputStatus = BE_SQLITE_ERROR;
    }
    m_pendingInput.clear();
    return m_outputStatus;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult ChangesetFileWriter::_Append(Byte const* pData, int nData) {
    if (nullptr == m_outLzmaFileStream) {
        BeAssert(false && "Call initialize before streaming the contents of a change set/summary");
        return BE_SQLITE_ERROR;
    }

    if (m_outputStatus != BE_SQLITE_OK)
        return m_outputStatus;
    if (nData == 0)
        return BE_SQLITE_OK;
    if (nData < 0 || pData == nullptr)
        return m_outputStatus = BE_SQLITE_ERROR;

    static size_t const inputBufferSize = 256 * 1024;
    size_t inputSize = static_cast<size_t>(nData);
    if (!m_bufferInput || inputSize >= inputBufferSize) {
        if (FlushInput() != BE_SQLITE_OK)
            return m_outputStatus;
        // Large records already amortize initialization and can be encoded without an input copy.
        if (m_lzmaEncoder.CompressNextPage(pData, inputSize) != ZIP_SUCCESS)
            m_outputStatus = BE_SQLITE_ERROR;
        return m_outputStatus;
    }

    if (m_pendingInput.size() + inputSize > inputBufferSize && FlushInput() != BE_SQLITE_OK)
        return m_outputStatus;

    m_pendingInput.reserve(inputBufferSize);
    m_pendingInput.insert(m_pendingInput.end(), pData, pData + inputSize);
    return m_pendingInput.size() == inputBufferSize ? FlushInput() : BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ChangeSet::ConflictResolution ChangesetFileWriter::_OnConflict(ChangeSet::ConflictCause cause, Changes::Change iter) {
    if (m_db) {
        iter.Dump(*m_db, false, 1);
    }
    BeAssert(false);
    return ChangeSet::ConflictResolution::Abort;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult ChangesetFileWriter::WritePrefix() {
    uint32_t size = m_prefix.empty() ? 0 : (uint32_t)m_prefix.SizeInBytes();
    Byte sizeBytes[4];
    UIntToByteArray(sizeBytes, size);

    DbResult result = _Append(sizeBytes, sizeof(sizeBytes));
    if (result != BE_SQLITE_OK || size == 0)
        return result;

    return _Append(reinterpret_cast<Byte const*>(m_prefix.c_str()), static_cast<int>(size));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ChangesetFileWriter::ChangesetFileWriter(BeFileNameCR pathname, bool containsEcSchemaChanges, DdlChangesCR ddlChanges, Db const *dgnDb, BeSQLite::LzmaEncoder::LzmaParams const &lzmaParams) : m_lzmaEncoder(lzmaParams), m_pathname(pathname), m_outLzmaFileStream(nullptr), m_prefix(""), m_db(dgnDb), m_bufferInput(!lzmaParams.GetSupportRandomAccess()) {
    if (!containsEcSchemaChanges && ddlChanges._IsEmpty())
        return;

    BeJsDocument jsonPrefix;
    jsonPrefix[JSON_PROP_ContainsSchemaChanges] = true;
    if (ddlChanges.GetSize() > 0)
        jsonPrefix[JSON_PROP_DDL] = ddlChanges.ToString();

    m_prefix = jsonPrefix.Stringify();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult ChangesetFileWriter::Initialize() {
    if (m_initialized) {
        BeAssert(false && "Call initialize only once");
        return BE_SQLITE_ERROR;
    }

    m_initialized = true;
    m_outputStatus = StartOutput();
    if (m_outputStatus != BE_SQLITE_OK)
        Finish();
    return m_outputStatus;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult ChangesetFileReaderBase::Reader::StartInput() {
    BeAssert(m_inLzmaFileStream == nullptr);
    m_inLzmaFileStream = new BlockFilesLzmaInStream(m_base.m_files);
    m_prefix = "";

    if (!m_inLzmaFileStream->IsReady()) {
        BeAssert(false);
        return BE_SQLITE_ERROR;
    }

    ChangesetLzmaHeader header;
    uint32_t actuallyRead;
    m_inLzmaFileStream->_Read(&header, sizeof(header), actuallyRead);
    if (actuallyRead != sizeof(header) || !header.IsValid()) {
        BeAssert(false && "Attempt to read an invalid revision version");
        return BE_SQLITE_ERROR_InvalidChangeSetVersion;
    }

    ZipErrors zipStatus = m_lzmaDecoder.StartDecompress(*m_inLzmaFileStream);
    if (zipStatus != ZIP_SUCCESS) {
        BeAssert(false);
        return BE_SQLITE_ERROR;
    }

    return ReadPrefix();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ChangesetFileReaderBase::Reader::FinishInput() {
    if (m_inLzmaFileStream == nullptr)
        return;

    m_lzmaDecoder.FinishDecompress();
    delete m_inLzmaFileStream;
    m_inLzmaFileStream = nullptr;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult ChangesetFileReaderBase::Reader::_Read(Byte* pData, int* pnData) {
    if (nullptr == m_inLzmaFileStream) {
        DbResult result = StartInput();
        if (result != BE_SQLITE_OK)
            return result;
    }

    ZipErrors zipErrors = m_lzmaDecoder.DecompressNextPage(pData, pnData);
    return (zipErrors == ZIP_SUCCESS) ? BE_SQLITE_OK : BE_SQLITE_ERROR;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8StringCR ChangesetFileReaderBase::Reader::GetPrefix(DbResult& result) {
    result = BE_SQLITE_OK;
    if (nullptr == m_inLzmaFileStream)
        result = StartInput();

    return m_prefix;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult ChangesetFileReaderBase::Reader::GetSchemaChanges(bool& containsSchemaChanges, DdlChangesR ddlChanges) {
    DbResult result;
    GetPrefix(result);
    if (result != BE_SQLITE_OK)
        return result;

    containsSchemaChanges = false;
    ddlChanges.Clear();

    if (m_prefix.empty())
        return BE_SQLITE_OK;

    BeJsDocument prefixJson(m_prefix);
    if (prefixJson.isMember(JSON_PROP_ContainsSchemaChanges))
        containsSchemaChanges = prefixJson[JSON_PROP_ContainsSchemaChanges].asBool();

    if (prefixJson.isMember(JSON_PROP_DDL))
        ddlChanges.AddDDL(prefixJson[JSON_PROP_DDL].asCString());

    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BeSQLite::DbResult ChangesetFileReaderBase::Reader::ReadPrefix() {
    return (SUCCESS == ReadPrefixFromStream(m_lzmaDecoder, m_prefix)) ? BE_SQLITE_OK : BE_SQLITE_ERROR;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BeSQLite::ChangeSet::ConflictResolution ChangesetFileReaderBase::_OnConflict(BeSQLite::ChangeSet::ConflictCause cause, BeSQLite::Changes::Change iter) {
    Utf8CP tableName = nullptr;
    int nCols, indirect;
    DbOpcode opcode;
    iter.GetOperation(&tableName, &nCols, &opcode, &indirect);
    if (ChangeSet::ConflictCause::NotFound == cause) {
        if (DbOpcode::Delete == opcode) {
            // Caused by CASCADE DELETE on a foreign key, and is usually not a problem.
            return ChangeSet::ConflictResolution::Skip;
        } else if ((DbOpcode::Update == opcode) && (0 == ::strncmp(tableName, "ec_", 3))) {
            // Caused by a ON DELETE SET NULL constraint on a foreign key - this is known to happen with "ec_" tables, but needs investigation if it happens otherwise
            return ChangeSet::ConflictResolution::Skip;
        }
    }
    if (ChangeSet::ConflictCause::Constraint == cause) {
        LOG.errorv("Unexpected Constraint conflict - opcode=%d, table=%s", opcode, tableName);
        if (auto db = GetDb()) {
            iter.Dump(*db, false, 1);
        }
        return ChangeSet::ConflictResolution::Abort;
    }

    // All other conflicts
    LOG.errorv("Unexpected conflict - opcode=%d, cause=%d, table=%s", opcode, cause, tableName);
    if (auto db = GetDb()) {
        iter.Dump(*db, false, 1);
    }
    return ChangeSet::ConflictResolution::Abort;
}

Utf8String RevisionUtility::GetChangesetId(BeFileName changesetFile) {
    WString fileName = changesetFile.GetFileNameWithoutExtension();
    return Utf8String(fileName);
}

BentleyStatus RevisionUtility::ReadChangesetPrefix(BeSQLite::LzmaDecoder& lzmaDecoder, Utf8StringR prefix) {
    return ReadPrefixFromStream(lzmaDecoder, prefix);
}

BentleyStatus RevisionUtility::OpenChangesetForReading(BeSQLite::LzmaDecoder& lzmaDecoder, BlockFilesLzmaInStream& inLzmaFileStream)
    {
    if (!inLzmaFileStream.IsReady())
        {
        BeAssert(false);
        return ERROR;
        }

    ChangesetLzmaHeader  header;
    uint32_t actuallyRead;
    inLzmaFileStream._Read(&header, sizeof(header), actuallyRead);
    if (actuallyRead != sizeof(header) || !header.IsValid())
        {
        BeAssert(false && "Attempt to read an invalid revision version");
        return ERROR;
        }

    ZipErrors zipStatus = lzmaDecoder.StartDecompress(inLzmaFileStream);
    if (zipStatus != ZIP_SUCCESS)
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }
BentleyStatus RevisionUtility::ExportPrefixFile(BeFileName targetDir, Utf8StringCR changesetId, Utf8StringCR prefix)
    {
    WString changesetIdW(changesetId.c_str(), true);
    if (!BeFileName::DoesPathExist(targetDir.GetName()))
        BeFileName::CreateNewDirectory(targetDir.GetDirectoryName());

    BeFileName outFilePath;
    outFilePath = outFilePath.Combine({targetDir.GetName(), (changesetIdW + L".cs-prefix").c_str()});
    BeFile prefixFile;
    prefixFile.Create(outFilePath.GetName(), true);
    auto status = prefixFile.Write(nullptr, prefix.c_str(), (uint32_t)prefix.size()) == BeFileStatus::Success? SUCCESS : ERROR;
    prefixFile.Close();
    return status;
    }
BentleyStatus RevisionUtility::GetUncompressSize(BeSQLite::LzmaDecoder& lzmaDecoder, uint32_t& uncompressSize)
    {
    const int kMaxDecompressBytes = 1024 * 64;
    int decompressBytesRead;
    uncompressSize = 0;
    Byte buffer[kMaxDecompressBytes];
    ZipErrors zipStatus;
    do
        {
        decompressBytesRead = kMaxDecompressBytes;
        zipStatus = lzmaDecoder.DecompressNextPage(buffer, &decompressBytesRead);
        if (zipStatus != ZIP_SUCCESS)
            return ERROR;
        uncompressSize += decompressBytesRead;

        } while (zipStatus == ZIP_SUCCESS && decompressBytesRead > 0);

    return SUCCESS;
    }

BentleyStatus RevisionUtility::ExportChangesetFile(BeFileName targetDir, Utf8StringCR changesetId, BeSQLite::LzmaDecoder& lzmaDecoder)
    {
    WString changesetIdW(changesetId.c_str(), true);
    if (!BeFileName::DoesPathExist(targetDir.GetName()))
        BeFileName::CreateNewDirectory(targetDir.GetDirectoryName());

    BeFileName outFilePath;
    outFilePath = outFilePath.Combine({targetDir.GetName(), (changesetIdW + L".cs-raw").c_str()});
    BeFile rawChangesetFile;
    rawChangesetFile.Create(outFilePath.GetName(), true);
    const int kMaxDecompressBytes = 1024 * 64;
    int decompressBytesRead;
    Byte buffer[kMaxDecompressBytes];
    ZipErrors zipStatus;
    do
        {
        decompressBytesRead = kMaxDecompressBytes;
        zipStatus = lzmaDecoder.DecompressNextPage(buffer, &decompressBytesRead);
        if (zipStatus != ZIP_SUCCESS)
            return ERROR;

        if (decompressBytesRead > 0 && rawChangesetFile.Write(nullptr, buffer, decompressBytesRead) != BeFileStatus::Success)
            return ERROR;

        } while (zipStatus == ZIP_SUCCESS && decompressBytesRead > 0);
    rawChangesetFile.Close();
    return SUCCESS;
    }
BentleyStatus RevisionUtility::DisassembleRevision(Utf8CP sourceFile, Utf8CP targetDir)
    {
    BeFileName source, target;
    source.SetNameUtf8(sourceFile);
    target.SetNameUtf8(targetDir);
    BeSQLite::LzmaDecoder lzmaDecoder;
    BlockFilesLzmaInStream inLzmaFileStream({ source });
    Utf8String changesetId = RevisionUtility::GetChangesetId(source);

    if (RevisionUtility::OpenChangesetForReading(lzmaDecoder, inLzmaFileStream) != SUCCESS)
        return ERROR;

    Utf8String prefix;
    if (RevisionUtility::ReadChangesetPrefix(lzmaDecoder, prefix) != SUCCESS)
        return ERROR;

    if (!prefix.empty() && RevisionUtility::ExportPrefixFile(target, changesetId, prefix) != SUCCESS)
        return ERROR;

    if (RevisionUtility::ExportChangesetFile(target, changesetId, lzmaDecoder) != SUCCESS)
        return ERROR;

    lzmaDecoder.FinishDecompress();
    return SUCCESS;
    }

BentleyStatus RevisionUtility::GetUncompressSize(Utf8CP sourceFile, uint32_t& compressSize, uint32_t &uncompressSize, uint32_t &prefixSize)
    {
    BeFileName source;
    source.SetNameUtf8(sourceFile);
    BeSQLite::LzmaDecoder lzmaDecoder;
    BlockFilesLzmaInStream inLzmaFileStream({ source });
    uint64_t diskSize;
    source.GetFileSize(diskSize);
    compressSize = static_cast<uint32_t>(diskSize);
    if (RevisionUtility::OpenChangesetForReading(lzmaDecoder, inLzmaFileStream) != SUCCESS)
        return ERROR;

    Utf8String prefix;
    if (RevisionUtility::ReadChangesetPrefix(lzmaDecoder, prefix) != SUCCESS)
        return ERROR;

    prefixSize = static_cast<uint32_t>(prefix.size());
    if (RevisionUtility::GetUncompressSize(lzmaDecoder, uncompressSize) != SUCCESS)
        return ERROR;

    lzmaDecoder.FinishDecompress();
    return SUCCESS;
    }
BentleyStatus RevisionUtility::AssembleRevision(Utf8CP inPrefixFile, Utf8CP inChangesetFile, Utf8CP outputFile, LzmaEncoder::LzmaParams params)
    {
    ZipErrors zipStatus;
    BeFileName inPrefixFileName, inChangesetFileName, outputFileName;
    const bool hasPrefix = Utf8String::IsNullOrEmpty(inPrefixFile);
    if (hasPrefix)
        inPrefixFileName.SetNameUtf8(inPrefixFile);

    inChangesetFileName.SetNameUtf8(inChangesetFile);
    outputFileName.SetNameUtf8(inChangesetFile);

    if (!BeFileName::DoesPathExist(outputFileName.GetName()))
        BeFileName::CreateNewDirectory(outputFileName.GetDirectoryName());

    ChangesetLzmaHeader header;
    BeSQLite::LzmaEncoder lzmaEncoder(params);
    BeFileLzmaOutStream outLzmaFileStream;
    BeFileStatus fileStatus = outLzmaFileStream.CreateOutputFile(outputFileName, true /* createAlways */);
    if (fileStatus != BeFileStatus::Success)
        {
        BeAssert(false);
        return ERROR;
        }

    uint32_t bytesWritten;
    zipStatus = outLzmaFileStream._Write(&header, sizeof(header), bytesWritten);
    if (zipStatus != ZIP_SUCCESS)
        {
        BeAssert(false);
        return ERROR;
        }

    zipStatus = lzmaEncoder.StartCompress(outLzmaFileStream);
    if (zipStatus != ZIP_SUCCESS)
        {
        BeAssert(false);
        return ERROR;
        }

    // prefix ==========================================================
    Utf8String prefix;
    if (hasPrefix)
        {
        BeFile prefixFile;
        if (prefixFile.Open(inPrefixFileName.GetName(), BeFileAccess::Read) != BeFileStatus::Success)
            return ERROR;

        ByteStream prefixBuffer;
        if (prefixFile.ReadEntireFile(prefixBuffer) != BeFileStatus::Success)
            return ERROR;

        prefixBuffer.Append('\0');
        prefix = reinterpret_cast<Utf8CP>(prefixBuffer.GetData());
        }

    if (RevisionUtility::WritePrefix(lzmaEncoder, prefix) != SUCCESS)
        return ERROR;

    if (RevisionUtility::WriteChangeset(lzmaEncoder, inChangesetFileName) != SUCCESS)
        return ERROR;

    lzmaEncoder.FinishCompress();
    return SUCCESS;
    }

BentleyStatus RevisionUtility::WriteChangeset(BeSQLite::LzmaEncoder& lzmaEncoder, BeFileName inChangesetFileName)
    {
    const int kMaxDecompressBytes = 1024 * 64;
    uint32_t bytesRead;
    Byte buffer[kMaxDecompressBytes];
    BeFile rawChangesetFile;
    BeFileStatus readStatus;
    if (rawChangesetFile.Open(inChangesetFileName.GetName(), BeFileAccess::Read) != BeFileStatus::Success)
        return ERROR;
    do
        {
        readStatus = rawChangesetFile.Read(buffer, &bytesRead, kMaxDecompressBytes);
        if (readStatus != BeFileStatus::Success)
            return ERROR;

        if (bytesRead > 0 && lzmaEncoder.CompressNextPage(buffer, bytesRead) != ZIP_SUCCESS)
            return ERROR;

        } while (readStatus == BeFileStatus::Success && bytesRead == kMaxDecompressBytes);

    return SUCCESS;
    }

BentleyStatus RevisionUtility::WritePrefix(BeSQLite::LzmaEncoder& lzmaEncoder, Utf8StringCR prefix)
    {
    Byte sizeBytes[4];
    uint32_t size = prefix.empty() ? 0 : (uint32_t)prefix.SizeInBytes();
    UIntToByteArray(sizeBytes, size);

    ZipErrors zipStatus = lzmaEncoder.CompressNextPage(sizeBytes, 4);
    if (zipStatus != ZIP_SUCCESS)
        return ERROR;

    if (size > 0)
        {
        zipStatus = lzmaEncoder.CompressNextPage(prefix.c_str(), size);
        if (zipStatus != ZIP_SUCCESS)
            return ERROR;
        }
        return SUCCESS;
    }

// --------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus RevisionUtility::RecompressRevision(Utf8CP sourceFile, Utf8CP targetFile, LzmaEncoder::LzmaParams params)
    {
    BeFileName source, target;
    source.SetNameUtf8(sourceFile);
    target.SetNameUtf8(targetFile);
    ZipErrors zipStatus;
    BeSQLite::LzmaDecoder lzmaDecoder;
    BlockFilesLzmaInStream inLzmaFileStream({source});
    if (!inLzmaFileStream.IsReady())
        {
        BeAssert(false);
        return ERROR;
        }

    ChangesetLzmaHeader  header;
    uint32_t actuallyRead;
    inLzmaFileStream._Read(&header, sizeof(header), actuallyRead);
    if (actuallyRead != sizeof(header) || !header.IsValid())
        {
        BeAssert(false && "Attempt to read an invalid revision version");
        return ERROR;
        }

    zipStatus = lzmaDecoder.StartDecompress(inLzmaFileStream);
    if (zipStatus != ZIP_SUCCESS)
        {
        BeAssert(false);
        return ERROR;
        }

    Utf8String prefix;
    if (ReadPrefixFromStream(lzmaDecoder, prefix) != SUCCESS)
        return ERROR;

    // write ========================================================================
    BeSQLite::LzmaEncoder lzmaEncoder(params);
    BeFileLzmaOutStream outLzmaFileStream;
    BeFileName::CreateNewDirectory(target.GetDirectoryName());
    BeFileStatus fileStatus = outLzmaFileStream.CreateOutputFile(target, true /* createAlways */);
    if (fileStatus != BeFileStatus::Success)
        {
        BeAssert(false);
        return ERROR;
        }

    uint32_t bytesWritten;
    zipStatus = outLzmaFileStream._Write(&header, sizeof(header), bytesWritten);
    if (zipStatus != ZIP_SUCCESS)
        {
        BeAssert(false);
        return ERROR;
        }

    zipStatus = lzmaEncoder.StartCompress(outLzmaFileStream);
    if (zipStatus != ZIP_SUCCESS)
        {
        BeAssert(false);
        return ERROR;
        }

    uint32_t prefixSizeWrite = prefix.empty() ? 0 : (uint32_t)prefix.SizeInBytes();
    UIntToByteArray(sizeBytes, prefixSizeWrite);

    zipStatus = lzmaEncoder.CompressNextPage(sizeBytes, 4);
    if (zipStatus != ZIP_SUCCESS)
        return ERROR;

    if (prefixSizeWrite > 0)
        {
        zipStatus = lzmaEncoder.CompressNextPage(prefix.c_str(), prefixSizeWrite);
        if (zipStatus != ZIP_SUCCESS)
            return ERROR;
        }
    const int kMaxDecompressBytes = 1024 * 64;
    int decompressBytesRead;
    Byte buffer[kMaxDecompressBytes];
    do
        {
        decompressBytesRead = kMaxDecompressBytes;
        zipStatus = lzmaDecoder.DecompressNextPage(buffer, &decompressBytesRead);
        if (zipStatus != ZIP_SUCCESS)
            return ERROR;

        if (decompressBytesRead > 0 && lzmaEncoder.CompressNextPage(buffer, static_cast<size_t>(decompressBytesRead)) != ZIP_SUCCESS )
            return ERROR;

        } while (zipStatus == ZIP_SUCCESS && decompressBytesRead > 0);

    lzmaDecoder.FinishDecompress();
    lzmaEncoder.FinishCompress();
    return SUCCESS;
    }
struct OperationStatistics final : NonCopyableClass
    {
    private:
        DbOpcode m_op;
        int m_cols;
        uint32_t m_changes;
        uint32_t m_indirect;
        explicit OperationStatistics(DbOpcode op) : m_op(op),m_cols(0),m_changes(0),m_indirect(0) {}
    public:
        void Record(int cols, int indirect)
            {
            m_changes++;
            if (indirect)
                m_indirect++;
            if (m_cols < cols)
                m_cols = cols;
            }
        int GetMaxColumns() const { return m_cols; }
        uint32_t GetRowsChanged() const { return m_changes; }
        uint32_t GetRowIndirectlyChanged() const { return m_indirect; }
        Utf8CP GetOpName() const
            {
            switch (m_op)
                {
                case DbOpcode::Delete: return "deleted";
                case DbOpcode::Insert: return "inserted";
                case DbOpcode::Update: return "updated";
                }
            return nullptr;
            }
        void GetSummary(BeJsValue summary)
            {
            summary["changes"] = m_changes;
            summary["columns"] = m_cols;
            summary["indirect"] = m_indirect;
            }
        bool Empty() const {return m_changes==0;}
        static std::unique_ptr<OperationStatistics> Create(DbOpcode op) { return std::unique_ptr<OperationStatistics>(new OperationStatistics(op)); }
    };
struct TableStatistics final : NonCopyableClass
    {
    private:
        Utf8String m_name;
        std::map<DbOpcode, std::unique_ptr<OperationStatistics>> m_opStats;
        explicit TableStatistics(Utf8CP name) : m_name(name) {}

    public:
        Utf8StringCR GetName() const { return m_name; }
        uint32_t GetTotalChanges()
            {
            return GetOp(DbOpcode::Insert)->GetRowsChanged() + GetOp(DbOpcode::Update)->GetRowsChanged() + GetOp(DbOpcode::Delete)->GetRowsChanged();
            }
        OperationStatistics* GetOp(DbOpcode op)
            {
            auto it = m_opStats.find(op);
            if (it != m_opStats.end())
                return it->second.get();

            auto opCode = OperationStatistics::Create(op);
            auto opCodeP = opCode.get();
            m_opStats[op] = std::move(opCode);
            return opCodeP;
            }
        void GetSummary(BeJsValue summary)
            {
            summary["table"] = m_name;
            summary["rowsChanged"] = GetTotalChanges();
            auto inserted = GetOp(DbOpcode::Insert);
            if (!inserted->Empty())
                inserted->GetSummary(summary[inserted->GetOpName()]);

            auto updated = GetOp(DbOpcode::Update);
            if (!updated->Empty())
                updated->GetSummary(summary[updated->GetOpName()]);

            auto deleted = GetOp(DbOpcode::Delete);
            if (!deleted->Empty())
                deleted->GetSummary(summary[deleted->GetOpName()]);
            }
        static std::unique_ptr<TableStatistics> Create(Utf8CP name) { return std::unique_ptr<TableStatistics>(new TableStatistics(name)); }
    };

struct ChangesetStatistics final : NonCopyableClass
    {
    private:
        std::map<Utf8String, std::unique_ptr<TableStatistics>> m_tableStats;
        bvector<TableStatistics *> GetTables(std::function<bool(TableStatistics *lhs, TableStatistics *rhs)> predicate = nullptr) const
            {
            bvector<TableStatistics*> tables;
            for (auto const& kv: m_tableStats)
                tables.push_back(kv.second.get());
            if (predicate)
                std::sort(tables.begin(), tables.end(), predicate);
            return tables;
            }
    public:
        void GetSummary(BeJsValue summary) const
            {
            uint32_t nInserted = 0;
            uint32_t nUpdated = 0;
            uint32_t nDeleted = 0;
            uint32_t nTables = 0;
            for (auto table: GetTables())
                {
                nTables++;
                nInserted += table->GetOp(DbOpcode::Insert)->GetRowsChanged();
                nUpdated += table->GetOp(DbOpcode::Update)->GetRowsChanged();
                nDeleted += table->GetOp(DbOpcode::Delete)->GetRowsChanged();
                }
            const uint32_t nTotal = nInserted + nUpdated + nDeleted;
            summary["rowsChanged"] = nTotal;
            summary["tablesChanged"] = nTables;

            auto byOp = summary["byOp"];
            byOp["rowInserted"] = nInserted;
            byOp["rowsUpdated"] = nUpdated;
            byOp["rowDeleted"] = nDeleted;

            auto byTables = summary["byTables"];
            for(auto table : GetTables([](TableStatistics *lhs, TableStatistics *rhs) { return lhs->GetTotalChanges() > rhs->GetTotalChanges();}))
                table->GetSummary(byTables.appendArray());
            }
        TableStatistics* GetTable(Utf8CP tableName)
            {
            auto it = m_tableStats.find(tableName);
            if (it != m_tableStats.end())
                return it->second.get();

            m_tableStats[tableName] = TableStatistics::Create(tableName);
            return GetTable(tableName);
            }
        static std::unique_ptr<ChangesetStatistics> Create() { return std::unique_ptr<ChangesetStatistics>(new ChangesetStatistics()); }
    };

BentleyStatus RevisionUtility::ComputeStatistics(Utf8CP changesetFile, bool addPrefix, BeJsValue out)
    {
    BeFileName input;
    input.AppendUtf8(changesetFile);
    ChangesetFileReaderBase reader({input});
    auto stats = ChangesetStatistics::Create();
    Utf8String changesetId = GetChangesetId(input);
    for( const auto& change : reader.GetChanges())
        {
        Utf8CP tableName;
        int nCols,indirect;
        DbOpcode opcode;
        change.GetOperation(&tableName, &nCols, &opcode, &indirect);
        stats->GetTable(tableName)->GetOp(opcode)->Record(nCols, indirect);
        }

    DdlChanges  ddlChanges;
    bool hasSchemaChanges;
    reader.MakeReader()->GetSchemaChanges(hasSchemaChanges, ddlChanges);

    out.SetEmptyObject();
    out["changesetId"] = GetChangesetId(input);
    uint32_t compressSize, uncompressSize, prefixSize;
    RevisionUtility::GetUncompressSize(changesetFile, compressSize, uncompressSize, prefixSize);
    out["compressSize"] = compressSize;
    out["uncompressSize"] = uncompressSize;
    out["hasSchemaChanges"] = hasSchemaChanges;
    out["prefixSize"] = prefixSize;
    stats->GetSummary(out["statistics"]);
    if (hasSchemaChanges && addPrefix)
        out["schemaChanges"] = ddlChanges.ToString();

    return SUCCESS;
    }

BentleyStatus RevisionUtility::DumpChangesetToDb(Utf8CP changesetFile, Utf8CP dbFile, bool includeCols)
    {
    BeFileName input;
    input.AppendUtf8(changesetFile);

    BeFileName output;
    Db db;
    output.AppendUtf8(dbFile);
    if (output.DoesPathExist())
        {
        if (db.OpenBeSQLiteDb(output, Db::OpenParams(Db::OpenMode::ReadWrite)) != BE_SQLITE_OK)
            output.BeDeleteFile();
        }
    if (!db.IsDbOpen())
        {
        BeFileName::CreateNewDirectory(output.GetDirectoryName());
        if (db.CreateNewDb(output) != BE_SQLITE_OK)
            return ERROR;
        }

    if (!db.TableExists("changeset"))
        {
        if (db.ExecuteDdl("CREATE TABLE changeset(Id INTEGER PRIMARY KEY, wsgId TEXT)") != BE_SQLITE_OK)
            return ERROR;
        if (db.ExecuteDdl("CREATE TABLE changeset_row(Id INTEGER PRIMARY KEY, changesetId INTEGER REFERENCES changeset(Id) ON DELETE CASCADE, PK INTEGER, tableName TEXT,op TEXT, indirect INTEGER)") != BE_SQLITE_OK)
            return ERROR;
        if (db.ExecuteDdl("CREATE TABLE changeset_schema(Id INTEGER PRIMARY KEY, changesetId INTEGER REFERENCES changeset(Id) ON DELETE CASCADE, DDL TEXT)") != BE_SQLITE_OK)
            return ERROR;
        if (db.ExecuteDdl("CREATE TABLE changeset_col(Id INTEGER PRIMARY KEY, changedRowId INTEGER REFERENCES changeset_row(Id) ON DELETE CASCADE, oldVal, newVal)") != BE_SQLITE_OK)
            return ERROR;
        }
    auto changesetExists = [&](Utf8String& wsgId) {
        auto stmt = db.GetCachedStatement("SELECT NULL FROM changeset WHERE wsgId = ?");
        stmt->BindText(1, wsgId.c_str(), Statement::MakeCopy::No);
        return stmt->Step() == BE_SQLITE_ROW;
    };
    auto insertChangeset = [&](Utf8String& wsgId) {
        auto stmt = db.GetCachedStatement("INSERT INTO changeset(wsgId) VALUES (?)");
        stmt->BindText(1, wsgId.c_str(), Statement::MakeCopy::No);
        if (stmt->Step() != BE_SQLITE_DONE)
            return (int64_t)0;
        return db.GetLastInsertRowId();
    };
    auto insertChangesetSchema = [&](int64_t changesetId, Utf8String& ddl) {
        auto stmt = db.GetCachedStatement("INSERT INTO changeset_schema(changesetId,ddl) VALUES (?,?)");
        stmt->BindInt64(1, changesetId);
        stmt->BindText(2, ddl.c_str(), Statement::MakeCopy::No);
        if (stmt->Step() != BE_SQLITE_DONE)
            return (int64_t)0;
        return db.GetLastInsertRowId();
    };
    auto insertChangesetRow = [&](int64_t changesetId, Utf8CP tableName, int indirect, int64_t pk, DbOpcode op) {
        auto stmt = db.GetCachedStatement("INSERT INTO changeset_row(changesetId, pk, tableName, indirect, op) VALUES (?,?,?,?,?)");
        stmt->BindInt64(1, changesetId);
        if (pk > 0)
            stmt->BindInt64(2, pk);

        stmt->BindText(3, tableName, Statement::MakeCopy::No);
        stmt->BindInt(4, indirect);
        stmt->BindText(5, op==DbOpcode::Delete ?"delete" :(op==DbOpcode::Insert? "insert" : "update") , Statement::MakeCopy::No);
        if (stmt->Step() != BE_SQLITE_DONE)
            return (int64_t)0;
        return db.GetLastInsertRowId();
    };
    auto insertChangesetCol = [&](int64_t rowId, DbValue oldValue, DbValue newValue) {
        auto stmt = db.GetCachedStatement("INSERT INTO changeset_col(changedRowId, oldVal, newVal) VALUES (?,?,?)");
        stmt->BindInt64(1, rowId);
        if (oldValue.IsValid())
            stmt->BindDbValue(2, oldValue);
        if (newValue.IsValid())
            stmt->BindDbValue(3, newValue);
    };
    Utf8String changesetIdStr = GetChangesetId(input);
    if (changesetExists(changesetIdStr)){
        return SUCCESS;
    }

    const auto changesetId = insertChangeset(changesetIdStr);
    if (changesetId == 0)
        return ERROR;


    ChangesetFileReaderBase reader({input});
    auto stats = ChangesetStatistics::Create();
    DbValue defaultVal(nullptr);
    for( const auto& change : reader.GetChanges())
        {
        Utf8CP tableName;
        int nCols, indirect, nPkCols;
        DbOpcode opcode;
        Byte* pPkCols;
        if (change.GetOperation(&tableName, &nCols, &opcode, &indirect) != BE_SQLITE_OK)
            return ERROR;
        if(change.GetPrimaryKeyColumns(&pPkCols, &nPkCols) != BE_SQLITE_OK)
            return ERROR;
        int64_t pk = 0;
        for (int i = 0; i < nPkCols; ++i)
            {
            if (pPkCols[i])
                {
                if (pk > 0 )
                    break;

                DbValue v(nullptr);
                if (opcode == DbOpcode::Insert)
                    v = change.GetNewValue(i);
                else
                    v = change.GetOldValue(i);

                if (v.IsValid() && v.GetValueType() == DbValueType::IntegerVal)
                    pk = v.GetValueInt64();
                }
            }

        auto changedRowId = insertChangesetRow(changesetId, tableName, indirect, pk, opcode);
        if (changedRowId == 0)
            return ERROR;

        if (!includeCols)
            continue;

        for (int i = 0; i < nPkCols; ++i)
            {
            if (opcode == DbOpcode::Delete)
                insertChangesetCol(changedRowId, change.GetOldValue(i), defaultVal);
            else if (opcode == DbOpcode::Insert)
                insertChangesetCol(changedRowId, defaultVal, change.GetNewValue(i));
            else
                insertChangesetCol(changedRowId, change.GetOldValue(i), change.GetNewValue(i));
            }
        }

        DdlChanges ddlChanges;
        bool hasSchemaChanges;
        reader.MakeReader()->GetSchemaChanges(hasSchemaChanges, ddlChanges);
        if (hasSchemaChanges) {
            auto ddl = ddlChanges.ToString();
            insertChangesetSchema(changesetId, ddl);
        }

        return db.SaveChanges() == BE_SQLITE_OK? SUCCESS: ERROR;
    }
