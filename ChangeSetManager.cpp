/*--------------------------------------------------------------------------------------+
|
|     $Source: JsInterop.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ChangeSetManager.h"
#include <Bentley/ScopedArray.h>
#include <BeSQLite/ChangeSet.h>
#include <BeSQLite/BeLzma.h>

USING_NAMESPACE_BENTLEY_SQLITE

#define REVISION_FORMAT_VERSION 0x10
#define CHANGESET_LZMA_MARKER "ChangeSetLzma"
#define JSON_PROP_ContainsSchemaChanges "ContainsSchemaChanges"
#define JSON_PROP_DDL "DDL"

BEGIN_BENTLEY_SQLITE_NAMESPACE

//=======================================================================================
//! Streams the contents of a file containing serialized change streams
// @bsiclass                                                 Ramanujam.Raman   10/15
//=======================================================================================
struct RevisionChangesFileReader : ChangeStream
{
private:
  Db &m_db; // Used only for debugging
  Utf8String m_prefix;
  bvector<BeFileName> m_files;

  LzmaDecoder m_lzmaDecoder;
  BlockFilesLzmaInStream *m_inLzmaFileStream;

  DbResult StartInput();
  DbResult ReadPrefix();
  void FinishInput();

  DbResult _InputPage(void *pData, int *pnData) override;
  void _Reset() override;
  ChangeSet::ConflictResolution _OnConflict(ChangeSet::ConflictCause clause, Changes::Change iter) override;

public:
  RevisionChangesFileReader(bvector<BeFileName> const &files, Db &db) : m_files(files), m_db(db), m_inLzmaFileStream(nullptr), m_prefix("") {}

  Utf8StringCR GetPrefix(DbResult &result);

  DbResult GetSchemaChanges(bool &containsSchemaChanges, DbSchemaChangeSetR dbSchemaChanges);

  ~RevisionChangesFileReader() { _Reset(); }
};

END_BENTLEY_SQLITE_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    01/2017
//---------------------------------------------------------------------------------------
static uint32_t ByteArrayToUInt(Byte bytes[])
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
  char m_idString[15];
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
// @bsimethod                                Ramanujam.Raman                    11/2015
//---------------------------------------------------------------------------------------
DbResult RevisionChangesFileReader::StartInput()
{
  BeAssert(m_inLzmaFileStream == nullptr);
  m_inLzmaFileStream = new BlockFilesLzmaInStream(m_files);
  m_prefix = "";

  if (!m_inLzmaFileStream->IsReady())
  {
    BeAssert(false);
    return BE_SQLITE_ERROR;
  }

  RevisionLzmaHeader header;
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
void RevisionChangesFileReader::FinishInput()
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
DbResult RevisionChangesFileReader::_InputPage(void *pData, int *pnData)
{
  if (nullptr == m_inLzmaFileStream)
  {
    DbResult result = StartInput();
    if (result != BE_SQLITE_OK)
      return result;
  }

  ZipErrors zipErrors = m_lzmaDecoder.DecompressNextPage((Byte *)pData, pnData);
  return (zipErrors == ZIP_SUCCESS) ? BE_SQLITE_OK : BE_SQLITE_ERROR;
}

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    01/2017
//---------------------------------------------------------------------------------------
Utf8StringCR RevisionChangesFileReader::GetPrefix(DbResult &result)
{
  result = BE_SQLITE_OK;
  if (nullptr == m_inLzmaFileStream)
    result = StartInput();

  return m_prefix;
}

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    01/2017
//---------------------------------------------------------------------------------------
DbResult RevisionChangesFileReader::GetSchemaChanges(bool &containsSchemaChanges, DbSchemaChangeSetR dbSchemaChanges)
{
  DbResult result;
  /* unused - Utf8StringCR prefix = */ GetPrefix(result);
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
DbResult RevisionChangesFileReader::ReadPrefix()
{
  Byte sizeBytes[4];
  int readSizeBytes = 4;
  ZipErrors zipErrors = m_lzmaDecoder.DecompressNextPage((Byte *)sizeBytes, &readSizeBytes);
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
    zipErrors = m_lzmaDecoder.DecompressNextPage((Byte *)prefixBytes.GetData() + bytesRead, &readSize);
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
void RevisionChangesFileReader::_Reset()
{
  FinishInput();
}

//---------------------------------------------------------------------------------------
// @bsimethod                                Sam.Wilson                       07/2018
//---------------------------------------------------------------------------------------
ChangeSet::ConflictResolution RevisionChangesFileReader::_OnConflict(ChangeSet::ConflictCause cause, Changes::Change iter)
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
  // All other conflicts
  BeAssert(false && "conflicts not expected");
  return ChangeSet::ConflictResolution::Replace;
}

//---------------------------------------------------------------------------------------
// @bsimethod                                Sam.Wilson                       07/2018
//---------------------------------------------------------------------------------------
DbResult ChangeSetManager::ApplyChangeSet(bvector<BeFileName> const &blockFileNames)
{
  RevisionChangesFileReader changesetReader(blockFileNames, m_db);

  // Apply DDL, if any
  DbSchemaChangeSet dbSchemaChanges;
  bool containsSchemaChanges;
  DbResult result = changesetReader.GetSchemaChanges(containsSchemaChanges, dbSchemaChanges);
  if (result != BE_SQLITE_OK)
  {
    BeAssert(false);
    return result;
  }

  if (containsSchemaChanges && !dbSchemaChanges.IsEmpty())
  {
    DbResult status = m_db.ExecuteSql(dbSchemaChanges.ToString().c_str());
    if (BE_SQLITE_OK != status)
      return status;
  }

  // Apply normal/data changes (including ec_ table data changes)
  return changesetReader.ApplyChanges(m_db);
}