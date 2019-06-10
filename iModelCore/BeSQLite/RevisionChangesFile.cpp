/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <BeSQLite/RevisionChangesFile.h>
#include <Logging/bentleylogging.h>
#include <Bentley/ScopedArray.h>

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
RevisionChangesFileWriter::RevisionChangesFileWriter(BeFileNameCR pathname, bool containsSchemaChanges, DbSchemaChangeSetCR dbSchemaChanges, Db const& dgnDb) : m_pathname(pathname), m_prefix(""), m_db(dgnDb), m_outLzmaFileStream(nullptr)
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
