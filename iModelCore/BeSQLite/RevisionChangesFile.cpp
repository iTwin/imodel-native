/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <BeSQLite/RevisionChangesFile.h>
#include <Logging/bentleylogging.h>
#include <Bentley/ScopedArray.h>
#include <map>
USING_NAMESPACE_BENTLEY_SQLITE

#define REVISION_FORMAT_VERSION  0x10
#define CHANGESET_LZMA_MARKER   "ChangeSetLzma"
#define JSON_PROP_DDL                   "DDL"
#define JSON_PROP_ContainsSchemaChanges "ContainsSchemaChanges"

#define LOG (*NativeLogging::LoggingManager::GetLogger(L"BeSQLite"))

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    01/2017
//---------------------------------------------------------------------------------------
void UIntToByteArray(Byte bytes[], uint32_t size)
    {
    // Note: Not using a union to convert since it may not be portable
    bytes[0] = (size >> 24) & 0xFF;
    bytes[1] = (size >> 16) & 0xFF;
    bytes[2] = (size >> 8) & 0xFF;
    bytes[3] = size & 0xFF;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    01/2017
//---------------------------------------------------------------------------------------
uint32_t ByteArrayToUInt(Byte bytes[])
    {
    return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | (bytes[3]);
    }

//=======================================================================================
// LZMA Header written to the top of the revision file
// @bsiclass                                                 Ramanujam.Raman   10/15
//=======================================================================================
struct RevisionLzmaHeader
    {
    private:
        uint16_t m_sizeOfHeader;
        char    m_idString[15];
        uint16_t m_formatVersionNumber;
        uint16_t m_compressionType;

    public:
        static const int formatVersionNumber = REVISION_FORMAT_VERSION;
        enum CompressionType
            {
            LZMA2 = 2
            };

        RevisionLzmaHeader()
            {
            CharCP idString = CHANGESET_LZMA_MARKER;
            BeAssert((strlen(idString) + 1) <= sizeof(m_idString));
            memset(this, 0, sizeof(*this));
            m_sizeOfHeader = (uint16_t)sizeof(RevisionLzmaHeader);
            strcpy(m_idString, idString);
            m_compressionType = CompressionType::LZMA2;
            m_formatVersionNumber = formatVersionNumber;
            }

        int GetVersion() { return m_formatVersionNumber; }

        bool IsValid()
            {
            if (m_sizeOfHeader != sizeof(RevisionLzmaHeader))
                return false;

            if (strcmp(m_idString, CHANGESET_LZMA_MARKER))
                return false;

            if (formatVersionNumber != m_formatVersionNumber)
                return false;

            return m_compressionType == LZMA2;
            }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2016
//---------------------------------------------------------------------------------------
DbResult RevisionChangesFileWriter::StartOutput()
    {
    BeAssert(m_outLzmaFileStream == nullptr);
    m_outLzmaFileStream = new BeFileLzmaOutStream();

    BeFileName::CreateNewDirectory(m_pathname.GetDirectoryName());
    BeFileStatus fileStatus = m_outLzmaFileStream->CreateOutputFile(m_pathname, true /* createAlways */);
    if (fileStatus != BeFileStatus::Success)
        {
        LOG.fatalv(L"%ls - OutLzmaFileStream::CreateOutputFile failed", m_pathname.c_str());
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    RevisionLzmaHeader header;
    uint32_t bytesWritten;
    ZipErrors zipStatus = m_outLzmaFileStream->_Write(&header, sizeof(header), bytesWritten);
    if (zipStatus != ZIP_SUCCESS)
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    zipStatus = m_lzmaEncoder.StartCompress(*m_outLzmaFileStream);
    if (zipStatus != ZIP_SUCCESS)
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    return WritePrefix();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2016
//---------------------------------------------------------------------------------------
void RevisionChangesFileWriter::FinishOutput()
    {
    if (m_outLzmaFileStream == nullptr)
        return;

    m_lzmaEncoder.FinishCompress();

    delete m_outLzmaFileStream;
    m_outLzmaFileStream = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
DbResult RevisionChangesFileWriter::_OutputPage(const void *pData, int nData)
    {
    if (nullptr == m_outLzmaFileStream)
        {
        BeAssert(false && "Call initialize before streaming the contents of a change set/summary");
        return BE_SQLITE_ERROR;
        }

    ZipErrors zipErrors = m_lzmaEncoder.CompressNextPage(pData, nData);
    return (zipErrors == ZIP_SUCCESS) ? BE_SQLITE_OK : BE_SQLITE_ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
void RevisionChangesFileWriter::_Reset()
    {
    FinishOutput();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
ChangeSet::ConflictResolution RevisionChangesFileWriter::_OnConflict(ChangeSet::ConflictCause cause, Changes::Change iter)
    {
    iter.Dump(m_db, false, 1);
    BeAssert(false);
    return ChangeSet::ConflictResolution::Abort;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    01/2017
//---------------------------------------------------------------------------------------
DbResult RevisionChangesFileWriter::WritePrefix()
    {
    uint32_t size = m_prefix.empty() ? 0 : (uint32_t)m_prefix.SizeInBytes();
    Byte sizeBytes[4];
    UIntToByteArray(sizeBytes, size);

    ZipErrors zipErrors = m_lzmaEncoder.CompressNextPage(sizeBytes, 4);
    if (zipErrors != ZIP_SUCCESS)
        return BE_SQLITE_ERROR;

    if (size == 0)
        return BE_SQLITE_OK;

    zipErrors = m_lzmaEncoder.CompressNextPage(m_prefix.c_str(), size);
    return (zipErrors == ZIP_SUCCESS) ? BE_SQLITE_OK : BE_SQLITE_ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2017
//---------------------------------------------------------------------------------------
void RevisionChangesFileWriter::InitPrefix(bool containsSchemaChanges, DbSchemaChangeSetCR dbSchemaChanges)
    {
    m_prefix = "";
    if (!containsSchemaChanges)
        return;

    Json::Value jsonPrefix = Json::objectValue;
    jsonPrefix[JSON_PROP_ContainsSchemaChanges] = true;
    if (dbSchemaChanges.GetSize() > 0)
        jsonPrefix[JSON_PROP_DDL] = dbSchemaChanges.ToString();

    m_prefix = Json::FastWriter().write(jsonPrefix);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
RevisionChangesFileWriter::RevisionChangesFileWriter(BeFileNameCR pathname, bool containsSchemaChanges, DbSchemaChangeSetCR dbSchemaChanges, Db const& dgnDb, BeSQLite::LzmaEncoder::LzmaParams const& lzmaParams) : m_pathname(pathname), m_prefix(""), m_db(dgnDb), m_outLzmaFileStream(nullptr), m_lzmaEncoder(lzmaParams)
    {
    InitPrefix(containsSchemaChanges, dbSchemaChanges);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    01/2017
//---------------------------------------------------------------------------------------
DbResult RevisionChangesFileWriter::Initialize()
    {
    if (m_outLzmaFileStream != nullptr)
        {
        BeAssert(false && "Call initialize only once");
        return BE_SQLITE_ERROR;
        }

    return StartOutput();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    01/2017
//---------------------------------------------------------------------------------------
RevisionChangesFileWriter::~RevisionChangesFileWriter() { _Reset(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    01/2017
//---------------------------------------------------------------------------------------
DbResult RevisionChangesFileWriter::CallOutputPage(const void* pData, int nData) { return _OutputPage(pData, nData); }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2015
//---------------------------------------------------------------------------------------
DbResult RevisionChangesFileReaderBase::StartInput()
    {
    BeAssert(m_inLzmaFileStream == nullptr);
    m_inLzmaFileStream = new BlockFilesLzmaInStream(m_files);
    m_prefix = "";

    if (!m_inLzmaFileStream->IsReady())
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }


    RevisionLzmaHeader  header;
    uint32_t actuallyRead;
    m_inLzmaFileStream->_Read(&header, sizeof(header), actuallyRead);
    if (actuallyRead != sizeof(header) || !header.IsValid())
        {
        BeAssert(false && "Attempt to read an invalid revision version");
        return BE_SQLITE_ERROR_InvalidRevisionVersion;
        }

    ZipErrors zipStatus = m_lzmaDecoder.StartDecompress(*m_inLzmaFileStream);
    if (zipStatus != ZIP_SUCCESS)
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    return ReadPrefix();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2015
//---------------------------------------------------------------------------------------
void RevisionChangesFileReaderBase::FinishInput()
    {
    if (m_inLzmaFileStream == nullptr)
        return;

    m_lzmaDecoder.FinishDecompress();
    delete m_inLzmaFileStream;
    m_inLzmaFileStream = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
DbResult RevisionChangesFileReaderBase::_InputPage(void *pData, int *pnData)
    {
    if (nullptr == m_inLzmaFileStream)
        {
        DbResult result = StartInput();
        if (result != BE_SQLITE_OK)
            return result;
        }

    ZipErrors zipErrors = m_lzmaDecoder.DecompressNextPage((Byte*)pData, pnData);
    return (zipErrors == ZIP_SUCCESS) ? BE_SQLITE_OK : BE_SQLITE_ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    01/2017
//---------------------------------------------------------------------------------------
Utf8StringCR RevisionChangesFileReaderBase::GetPrefix(DbResult& result)
    {
    result = BE_SQLITE_OK;
    if (nullptr == m_inLzmaFileStream)
        result = StartInput();

    return m_prefix;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    01/2017
//---------------------------------------------------------------------------------------
DbResult RevisionChangesFileReaderBase::GetSchemaChanges(bool& containsSchemaChanges, DbSchemaChangeSetR dbSchemaChanges)
    {
    DbResult result;
    /* unused - Utf8StringCR prefix = */GetPrefix(result);
    if (result != BE_SQLITE_OK)
        return result;

    containsSchemaChanges = false;
    dbSchemaChanges.Clear();

    if (m_prefix.empty())
        return BE_SQLITE_OK;

    Json::Value prefixJson;
    if (!Json::Reader::Parse(m_prefix.c_str(), prefixJson))
        {
        BeAssert(false && "Prefix seems corrupted");
        return BE_SQLITE_ERROR;
        }

    if (prefixJson.isMember(JSON_PROP_ContainsSchemaChanges))
        containsSchemaChanges = prefixJson[JSON_PROP_ContainsSchemaChanges].asBool();

    if (prefixJson.isMember(JSON_PROP_DDL))
        dbSchemaChanges.AddDDL(prefixJson[JSON_PROP_DDL].asCString());

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    01/2017
//---------------------------------------------------------------------------------------
BeSQLite::DbResult RevisionChangesFileReaderBase::ReadPrefix()
    {
    Byte sizeBytes[4];
    int readSizeBytes = 4;
    ZipErrors zipErrors = m_lzmaDecoder.DecompressNextPage((Byte*)sizeBytes, &readSizeBytes);
    if (zipErrors != ZIP_SUCCESS || readSizeBytes != 4)
        {
        BeAssert(false && "Couldn't read size of the schema changes");
        return BE_SQLITE_ERROR;
        }

    int size = (int)ByteArrayToUInt(sizeBytes);
    if (size == 0)
        return BE_SQLITE_OK;

    ScopedArray<Byte> prefixBytes(size);
    int bytesRead = 0;
    while (bytesRead < size)
        {
        int readSize = size - bytesRead;
        zipErrors = m_lzmaDecoder.DecompressNextPage((Byte*)prefixBytes.GetData() + bytesRead, &readSize);
        if (zipErrors != ZIP_SUCCESS)
            {
            BeAssert(false && "Error reading revision prefix stream");
            return BE_SQLITE_ERROR;
            }

        bytesRead += readSize;
        }
    BeAssert(bytesRead == size);

    m_prefix = (Utf8CP)prefixBytes.GetData();
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
void RevisionChangesFileReaderBase::_Reset()
    {
    FinishInput();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Sam.Wilson                       07/2018
//---------------------------------------------------------------------------------------
BeSQLite::ChangeSet::ConflictResolution RevisionChangesFileReaderBase::_OnConflict(BeSQLite::ChangeSet::ConflictCause cause, BeSQLite::Changes::Change iter)
    {
    Utf8CP tableName = nullptr;
    int nCols, indirect;
    DbOpcode opcode;
    iter.GetOperation(&tableName, &nCols, &opcode, &indirect);
    if (ChangeSet::ConflictCause::NotFound == cause)
        {
        if (DbOpcode::Delete == opcode)
            {
            // Caused by CASCADE DELETE on a foreign key, and is usually not a problem.
            return ChangeSet::ConflictResolution::Skip;
            }
        else if ((DbOpcode::Update == opcode) && (0 == ::strncmp(tableName, "ec_", 3)))
            {
            // Caused by a ON DELETE SET NULL constraint on a foreign key - this is known to happen with "ec_" tables, but needs investigation if it happens otherwise
            return ChangeSet::ConflictResolution::Skip;
            }
        }
    if (ChangeSet::ConflictCause::Constraint == cause)
        {
        LOG.errorv("Unexpected Constraint conflict - opcode=%d, table=%s", opcode, tableName);
        iter.Dump(GetDb(), false, 1);
        return ChangeSet::ConflictResolution::Abort;
        }

    // All other conflicts
    LOG.errorv("Unexpected conflict - opcode=%d, cause=%d, table=%s", opcode, cause, tableName);
    iter.Dump(GetDb(), false, 1);
    return ChangeSet::ConflictResolution::Abort;
    }
Utf8String RevisionUtility::GetChangesetId(BeFileName changesetFile)
    {
    WString fileName = changesetFile.GetFileNameWithoutExtension();
    return Utf8String(fileName);
    }
BentleyStatus RevisionUtility::ReadChangesetPrefix(BeSQLite::LzmaDecoder& lzmaDecoder, Utf8StringR prefix)
    {
    Byte sizeBytes[4];
    int readSizeBytes = 4;
    ZipErrors zipStatus = lzmaDecoder.DecompressNextPage((Byte*)sizeBytes, &readSizeBytes);
    if (zipStatus != ZIP_SUCCESS || readSizeBytes != 4)
        {
        BeAssert(false && "Couldn't read size of the schema changes");
        return ERROR;
        }

    const int size = (int)ByteArrayToUInt(sizeBytes);
    if (size > 0)
        {
        ScopedArray<Byte> prefixBytes(size);
        int bytesRead = 0;
        while (bytesRead < size)
            {
            int readSize = size - bytesRead;
            zipStatus = lzmaDecoder.DecompressNextPage((Byte*)prefixBytes.GetData() + bytesRead, &readSize);
            if (zipStatus != ZIP_SUCCESS)
                {
                BeAssert(false && "Error reading revision prefix stream");
                return ERROR;
             }

            bytesRead += readSize;
            }
        BeAssert(bytesRead == size);
        prefix = (Utf8CP)prefixBytes.GetData();
        }

    return SUCCESS;
    }
BentleyStatus RevisionUtility::OpenChangesetForReading(BeSQLite::LzmaDecoder& lzmaDecoder, BlockFilesLzmaInStream& inLzmaFileStream)
    {
    if (!inLzmaFileStream.IsReady())
        {
        BeAssert(false);
        return ERROR;
        }

    RevisionLzmaHeader  header;
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

    RevisionLzmaHeader header;
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
// @bsimethod                                Affan.Khan                       04/2020
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

    RevisionLzmaHeader  header;
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
    
    Byte sizeBytes[4];
    int readSizeBytes = 4;
    zipStatus = lzmaDecoder.DecompressNextPage((Byte*)sizeBytes, &readSizeBytes);
    if (zipStatus != ZIP_SUCCESS || readSizeBytes != 4)
        {
        BeAssert(false && "Couldn't read size of the schema changes");
        return ERROR;
        }
    Utf8String prefix;
    const int prefixSizeRead = (int)ByteArrayToUInt(sizeBytes);
    if (prefixSizeRead > 0)
        { 
        ScopedArray<Byte> prefixBytes(prefixSizeRead);
        int bytesRead = 0;
        while (bytesRead < prefixSizeRead)
            {
            int readSize = prefixSizeRead - bytesRead;
            zipStatus = lzmaDecoder.DecompressNextPage((Byte*)prefixBytes.GetData() + bytesRead, &readSize);
            if (zipStatus != ZIP_SUCCESS)
                {
                BeAssert(false && "Error reading revision prefix stream");
                return ERROR;
                }

            bytesRead += readSize;
            }
        BeAssert(bytesRead == prefixSizeRead);
        Utf8String prefix = (Utf8CP)prefixBytes.GetData();
        }
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
         
        if (decompressBytesRead > 0 && lzmaEncoder.CompressNextPage(buffer, decompressBytesRead) != ZIP_SUCCESS )
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
        Json::Value GetSummary()
            {
            Json::Value summary = Json::Value(Json::ValueType::objectValue);
            summary["changes"] = m_changes;
            summary["columns"] = m_cols;
            summary["indirect"] = m_indirect;
            return summary;
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
        Json::Value GetSummary()
            {
            Json::Value summary = Json::Value(Json::ValueType::objectValue);
            summary["table"] = m_name;
            summary["rowsChanged"] = GetTotalChanges();
            auto inserted = GetOp(DbOpcode::Insert);
            if (!inserted->Empty())
                summary[inserted->GetOpName()] = inserted->GetSummary();

            auto updated = GetOp(DbOpcode::Update);
            if (!updated->Empty())
                summary[updated->GetOpName()] = updated->GetSummary();

            auto deleted = GetOp(DbOpcode::Delete);
            if (!deleted->Empty())
                summary[deleted->GetOpName()] = deleted->GetSummary();

            return summary;
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
        Json::Value GetSummary() const
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
            Json::Value summary = Json::Value(Json::ValueType::objectValue);
            const uint32_t nTotal = nInserted + nUpdated + nDeleted;
            summary["rowsChanged"] = nTotal;
            summary["tablesChanged"] = nTables;

            Json::Value byOp = Json::Value(Json::ValueType::objectValue);
            byOp["rowInserted"] = nInserted;
            byOp["rowsUpdated"] = nUpdated;
            byOp["rowDeleted"] = nDeleted;
            summary["byOp"] = byOp;

            Json::Value byTables = Json::Value(Json::ValueType::arrayValue);
            for(auto table : GetTables([](TableStatistics *lhs, TableStatistics *rhs) { return lhs->GetTotalChanges() > rhs->GetTotalChanges();}))
                byTables.append(table->GetSummary());
            summary["byTables"] = byTables;

            return summary;
            }
    public:
        TableStatistics* GetTable(Utf8CP tableName)
            {
            auto it = m_tableStats.find(tableName);
            if (it != m_tableStats.end())
                return it->second.get();

            m_tableStats[tableName] = TableStatistics::Create(tableName);
            return GetTable(tableName);
            }
        Json::Value Statistics() const 
            {
            return GetSummary();
            }
        static std::unique_ptr<ChangesetStatistics> Create() { return std::unique_ptr<ChangesetStatistics>(new ChangesetStatistics()); }
    };
BentleyStatus RevisionUtility::ComputeStatistics(Utf8CP changesetFile, bool addPrefix, Json::Value& out)
    {
    BeFileName input;
    input.AppendUtf8(changesetFile);
    Db unused;
    RevisionChangesFileReaderBase reader({input}, unused);
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

    DbSchemaChangeSet schemaChangeset;
    bool hasSchemaChanges;
    reader.GetSchemaChanges(hasSchemaChanges, schemaChangeset);

    out = Json::Value(Json::ValueType::objectValue);
    out["changesetId"] = GetChangesetId(input);
    uint32_t compressSize, uncompressSize, prefixSize;
    RevisionUtility::GetUncompressSize(changesetFile, compressSize, uncompressSize, prefixSize);
    out["compressSize"] = compressSize;
    out["uncompressSize"] = uncompressSize;
    out["hasSchemaChanges"] = hasSchemaChanges;
    out["prefixSize"] = prefixSize;
    out["statistics"] = stats->Statistics();
    if (hasSchemaChanges && addPrefix)
        out["schemaChanges"] = schemaChangeset.ToString();

    return SUCCESS;
    }

