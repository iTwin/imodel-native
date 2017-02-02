/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/RevisionManager.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <Bentley/SHA1.h>
#include <BeSqlite/BeLzma.h>
#include <DgnPlatform/DgnChangeSummary.h>

USING_NAMESPACE_BENTLEY_SQLITE

#define REVISION_LZMA_MARKER   "RevLzma"
#define CURRENT_REV_END_TXN_ID "CurrentRevisionEndTxnId"
// #define DEBUG_REVISION_KEEP_FILES 1

//=======================================================================================
// LZMA Header written to the top of the revision file
// @bsiclass                                                 Ramanujam.Raman   10/15
//=======================================================================================
struct RevisionLzmaHeader
{
private:
    uint16_t m_sizeOfHeader;
    char    m_idString[10];
    uint16_t m_formatVersionNumber;
    uint16_t m_compressionType;

public:
    static const int formatVersionNumber = 0x10;
    enum CompressionType
        {
        LZMA2 = 2
        };

    RevisionLzmaHeader()
        {
        CharCP idString = REVISION_LZMA_MARKER;
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

        if (strcmp(m_idString, REVISION_LZMA_MARKER))
            return false;

        if (formatVersionNumber != m_formatVersionNumber)
            return false;

        return m_compressionType == LZMA2;
        }
};

//=======================================================================================
//! Writes the contents of a change stream to a file
// @bsiclass                                                 Ramanujam.Raman   10/15
//=======================================================================================
struct RevisionChangesFileWriter : ChangeStream
{
private:
    BeSQLite::LzmaEncoder m_lzmaEncoder;
    BeFileName m_pathname;
    BeFileLzmaOutStream* m_outLzmaFileStream;
    DgnDbCR m_dgndb; // Only for debugging

    //---------------------------------------------------------------------------------------
    // @bsimethod                                Ramanujam.Raman                    11/2016
    //---------------------------------------------------------------------------------------
    BentleyStatus StartOutput()
        {
        BeAssert(m_outLzmaFileStream == nullptr);
        m_outLzmaFileStream = new BeFileLzmaOutStream();

        BeFileStatus fileStatus = m_outLzmaFileStream->CreateOutputFile(m_pathname, true /* createAlways */);
        if (fileStatus != BeFileStatus::Success)
            {
            BeAssert(false);
            return ERROR;
            }

        RevisionLzmaHeader header;
        uint32_t bytesWritten;
        ZipErrors zipStatus = m_outLzmaFileStream->_Write(&header, sizeof(header), bytesWritten);
        if (zipStatus != ZIP_SUCCESS)
            {
            BeAssert(false);
            return ERROR;
            }

        zipStatus = m_lzmaEncoder.StartCompress(*m_outLzmaFileStream);
        if (zipStatus != ZIP_SUCCESS)
            {
            BeAssert(false);
            return ERROR;
            }

        return SUCCESS;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                Ramanujam.Raman                    11/2016
    //---------------------------------------------------------------------------------------
    void FinishOutput()
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
    DbResult _OutputPage(const void *pData, int nData) override
        {
        if (nullptr == m_outLzmaFileStream)
            {
            if (SUCCESS != StartOutput())
                return BE_SQLITE_ERROR;
            }
            
        ZipErrors zipErrors = m_lzmaEncoder.CompressNextPage(pData, nData);
        return (zipErrors == ZIP_SUCCESS) ? BE_SQLITE_OK : BE_SQLITE_ERROR;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                Ramanujam.Raman                    10/2015
    //---------------------------------------------------------------------------------------
    void _Reset() override
        {
        FinishOutput();
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                Ramanujam.Raman                    10/2015
    //---------------------------------------------------------------------------------------
    ChangeSet::ConflictResolution _OnConflict(ChangeSet::ConflictCause cause, Changes::Change iter) override
        {
        iter.Dump(m_dgndb, false, 1);
        BeAssert(false);
        return ChangeSet::ConflictResolution::Abort;
        }

public:
    //---------------------------------------------------------------------------------------
    // @bsimethod                                Ramanujam.Raman                    10/2015
    //---------------------------------------------------------------------------------------
    RevisionChangesFileWriter(BeFileNameCR pathname, DgnDbCR dgnDb) : m_pathname(pathname), m_dgndb(dgnDb), m_outLzmaFileStream(nullptr) {}
    ~RevisionChangesFileWriter() {}
};

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2015
//---------------------------------------------------------------------------------------
BentleyStatus RevisionChangesFileReader::StartInput()
    {
    BeAssert(m_inLzmaFileStream == nullptr);
    m_inLzmaFileStream = new BeFileLzmaInStream();

    StatusInt status = m_inLzmaFileStream->OpenInputFile(m_pathname);
    if (status != SUCCESS)
        {
        BeAssert(false);
        return ERROR;
        }

    RevisionLzmaHeader  header;
    uint32_t actuallyRead;
    m_inLzmaFileStream->_Read(&header, sizeof(header), actuallyRead);
    if (actuallyRead != sizeof(header) || !header.IsValid())
        {
        BeAssert(false);
        return ERROR;
        }

    ZipErrors zipStatus = m_lzmaDecoder.StartDecompress(*m_inLzmaFileStream);
    if (zipStatus != ZIP_SUCCESS)
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
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
        if (SUCCESS != StartInput())
            return BE_SQLITE_ERROR;
        }
    ZipErrors zipErrors = m_lzmaDecoder.DecompressNextPage((Byte*) pData, pnData);
    return (zipErrors == ZIP_SUCCESS) ? BE_SQLITE_OK : BE_SQLITE_ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
void RevisionChangesFileReader::_Reset()
    {
    FinishInput();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
ChangeSet::ConflictResolution RevisionChangesFileReader::_OnConflict(ChangeSet::ConflictCause cause, Changes::Change iter)
    {
    Utf8CP tableName = nullptr;
    int nCols, indirect;
    DbOpcode opcode;
    DbResult result = iter.GetOperation(&tableName, &nCols, &opcode, &indirect);
    BeAssert(result == BE_SQLITE_OK);
    UNUSED_VARIABLE(result);

    if (cause == ConflictCause::NotFound && opcode == DbOpcode::Delete) // a delete that is already gone. 
       return ConflictResolution::Skip; // This is caused by propagate delete on a foreign key. It is not a problem.

    if (LOG.isSeverityEnabled(NativeLogging::LOG_INFO))
        {
        LOG.infov("Conflict detected - incoming revision %s:", indirect ? "skipped" : "replaced");
        BeAssert(tableName != nullptr);
        iter.Dump(m_dgndb, false, 1);
        }

    /*
     * We ALWAYS accept the incoming revision in cases of conflicts:
     *
     * + In a briefcase with no local changes, the state of a row in the Db (i.e., the final state of a previous revision) 
     *   may not exactly match the initial state of the incoming revision. This will cause a conflict.
     *      - The final state of the incoming (later) revision will always be setup exactly right to accommodate 
     *        cases where dependency handlers won't be available (for instance on the server), and we have to rely on 
     *        the revision to correctly set the final state of the row in the Db. Therefore it's best to resolve the 
     *        conflict in favor of the incoming change. 
     *
     * + In a briefcase with local changes, the state of relevant dependent properties (due to propagated indirect changes) 
     *   may not correspond with the initial state of these properties in an incoming revision. This will cause a conflict.
     *      - Resolving the conflict in favor of the incoming revision may cause some dependent properties to be set 
     *        incorrectly, but the dependency handlers will run anyway and set this right. The new changes will be part of 
     *        a subsequent revision generated from that briefcase.
     *
     * + Note that conflicts can NEVER happen between direct changes made locally and direct changes in the incoming revision. 
     *      - Only one user can make a direct change at one time, and the next user has to pull those changes before getting a
     *        lock to the same element
     *
     * + Also see comments in TxnManager::MergeRevision()
     */
    return ChangeSet::ConflictResolution::Replace;
    }

//=======================================================================================
//! Generates the DgnRevision Id
// @bsiclass                                                 Ramanujam.Raman   10/15
//=======================================================================================
struct DgnRevisionIdGenerator : ChangeStream
{
private:
    SHA1 m_hash;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                Ramanujam.Raman                    10/2015
    //---------------------------------------------------------------------------------------
    static int HexCharToInt(char input)
        {
        if (input >= '0' && input <= '9')
            return input - '0';
        if (input >= 'A' && input <= 'F')
            return input - 'A' + 10;
        if (input >= 'a' && input <= 'f')
            return input - 'a' + 10;

        BeAssert(false);
        return 0;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                Ramanujam.Raman                    10/2015
    //---------------------------------------------------------------------------------------
    void AddIdStringToHash(Utf8StringCR hashString)
        {
        Byte hashValue[SHA1::HashBytes];            
        if (hashString.empty())
            {
            memset(hashValue, 0, SHA1::HashBytes);
            }
        else
            {
            BeAssert(hashString.length() == SHA1::HashBytes * 2);
            for (int ii = 0; ii < SHA1::HashBytes; ii++)
                {
                char hexChar1 = hashString.at(2 * ii);
                char hexChar2 = hashString.at(2 * ii + 1);
                hashValue[ii] = (Byte) (16 * HexCharToInt(hexChar1) + HexCharToInt(hexChar2));
                }
            }
            
        m_hash.Add(hashValue, SHA1::HashBytes);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                Ramanujam.Raman                    10/2015
    //---------------------------------------------------------------------------------------
    Utf8String GetHashString()
        {
        return m_hash.GetHashString();
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                Ramanujam.Raman                    10/2015
    //---------------------------------------------------------------------------------------
    DbResult _OutputPage(const void *pData, int nData) override
        {
        m_hash.Add(pData, nData);
        return BE_SQLITE_OK;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                Ramanujam.Raman                    10/2015
    //---------------------------------------------------------------------------------------
    ChangeSet::ConflictResolution _OnConflict(ChangeSet::ConflictCause cause, Changes::Change iter) override
        {
        BeAssert(false);
        return ChangeSet::ConflictResolution::Abort;
        }

public:
    DgnRevisionIdGenerator() {}

    //---------------------------------------------------------------------------------------
    // @bsimethod                                Ramanujam.Raman                    10/2015
    //---------------------------------------------------------------------------------------
    static Utf8String GenerateId(Utf8String parentRevId, ChangeGroup& changeGroup)
        {
        DgnRevisionIdGenerator idgen;
        idgen.AddIdStringToHash(parentRevId);

        DbResult result = idgen.FromChangeGroup(changeGroup);
        if (BE_SQLITE_OK != result)
            return "";

        return idgen.GetHashString();
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                Ramanujam.Raman                    10/2015
    //---------------------------------------------------------------------------------------
    static Utf8String GenerateId(Utf8String parentRevId, ChangeStream& changeStream)
        {
        DgnRevisionIdGenerator idgen;
        idgen.AddIdStringToHash(parentRevId);

        DbResult result = idgen.FromChangeStream(changeStream);
        if (BE_SQLITE_OK != result)
            return "";

        return idgen.GetHashString();
        }
};

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
DgnRevisionPtr DgnRevision::Create(RevisionStatus* outStatus, Utf8StringCR revisionId, Utf8StringCR parentRevisionId, Utf8StringCR dbGuid)
    {
    RevisionStatus ALLOW_NULL_OUTPUT(status, outStatus);
    
    if (revisionId.empty() || revisionId.length() != SHA1::HashBytes * 2)
        {
        status = RevisionStatus::InvalidId;
        BeAssert(false && "Empty or invalid revision id passed in");
        return nullptr;
        }

    BeFileName changeStreamPathname = BuildRevisionChangesPathname(revisionId);
    
    DgnRevisionPtr revision = new DgnRevision(revisionId, parentRevisionId, dbGuid);
    revision->SetRevisionChangesFile(changeStreamPathname);
    status = RevisionStatus::Success;
    return revision;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
DgnRevision::~DgnRevision()
    {
#ifndef DEBUG_REVISION_KEEP_FILES
    if (m_revChangesFile.DoesPathExist())
        {
        BeFileNameStatus status = m_revChangesFile.BeDeleteFile();
        BeAssert(BeFileNameStatus::Success == status && "Could not delete temporary change stream file");
        }
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
// static
BeFileName DgnRevision::BuildRevisionChangesPathname(Utf8String revisionId)
    {
    BeFileName tempPathname;
    BentleyStatus status = T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectory(tempPathname, L"DgnDbRev");
    BeAssert(SUCCESS == status && "Cannot get temporary directory");
    tempPathname.AppendToPath(WString(revisionId.c_str(), true).c_str());
    tempPathname.AppendExtension(L"rev");
    return tempPathname;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
void DgnRevision::Dump(DgnDbCR dgndb) const
    {
    LOG.infov("Id : %s", m_id.c_str());
    LOG.infov("ParentId : %s", m_parentId.c_str());
    LOG.infov("Initial ParentId : %s", m_initialParentId.c_str());
    LOG.infov("DbGuid: %s", m_dbGuid.c_str());
    LOG.infov("User Name: %s", m_userName.c_str());
    LOG.infov("Summary: %s", m_summary.c_str());
    LOG.infov("ChangeStreamFile: %ls", m_revChangesFile.c_str());
    LOG.infov("DateTime: %s", m_dateTime.ToString().c_str());

    RevisionChangesFileReader fs(m_revChangesFile, dgndb);
    fs.Dump("Revision Contents:\n", dgndb, false, 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
RevisionStatus DgnRevision::Validate(DgnDbCR dgndb) const
    {
    if (m_id.empty() || m_id.length() != SHA1::HashBytes * 2)
        {
        BeAssert(false && "The revision id is empty");
        return RevisionStatus::InvalidId;
        }

    Utf8String dbGuid = dgndb.GetDbGuid().ToString();
    if (m_dbGuid != dbGuid)
        {
        BeAssert(false && "The revision did not originate in the specified DgnDb");
        return RevisionStatus::WrongDgnDb;
        }

    if (!m_revChangesFile.DoesPathExist())
        {
        BeAssert(false && "File containing the change stream doesn't exist. Cannot validate.");
        return RevisionStatus::FileNotFound;
        }

    RevisionChangesFileReader fs(m_revChangesFile, dgndb);

    Utf8String id = DgnRevisionIdGenerator::GenerateId(m_parentId, fs);
    if (m_id != id)
        {
        BeAssert(false && "The contents of the change stream file don't match the DgnRevision");
        return RevisionStatus::CorruptedChangeStream;
        }

    return RevisionStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    12/2017
//---------------------------------------------------------------------------------------
static DbDupValue GetValueFromChangeOrDb(ChangeIterator::ColumnIterator const& columnIter, Utf8CP propertyAccessString, Changes::Change::Stage stage)
    {
    ChangeIterator::ColumnEntry column = columnIter.GetColumn(propertyAccessString);
    DbDupValue value = column.GetValue(stage);

    if (!value.IsValid() && columnIter.GetRowEntry().GetDbOpcode() == DbOpcode::Update)
        value = column.QueryValueFromDb();

    return value;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    12/2017
//---------------------------------------------------------------------------------------
template<typename T> static T GetIdFromChangeOrDb(ChangeIterator::ColumnIterator const& columnIter, Utf8CP propertyAccessString, Changes::Change::Stage stage)
    {
    DbDupValue value = GetValueFromChangeOrDb(columnIter, propertyAccessString, stage);
    return value.IsValid() ? value.GetValueId<T>() : T();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    12/2017
//---------------------------------------------------------------------------------------
static DgnModelId GetModelIdFromChangeOrDb(ChangeIterator::ColumnIterator const& columnIter, Changes::Change::Stage stage)
    {
    return GetIdFromChangeOrDb<DgnModelId>(columnIter, "Model.Id", stage);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    12/2017
//---------------------------------------------------------------------------------------
static DgnCode GetCodeFromChangeOrDb(ChangeIterator::ColumnIterator const& columnIter, Changes::Change::Stage stage)
    {
    DbDupValue codeSpecId = GetValueFromChangeOrDb(columnIter, "CodeSpec.Id", stage);
    DbDupValue scope = GetValueFromChangeOrDb(columnIter, "CodeScope", stage);
    DbDupValue value = GetValueFromChangeOrDb(columnIter, "CodeValue", stage);

    DgnCode code;
    if (codeSpecId.IsValid() && scope.IsValid() && value.IsValid())
        code.From(codeSpecId.GetValueId<CodeSpecId>(), value.GetValueText(), scope.GetValueText());

    return code;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Paul.Connelly   12/15
//---------------------------------------------------------------------------------------
static void insertCode(DgnCodeSet& into, DgnCode const& code, DgnCodeSet& ifNotIn)
    {
    if (code.IsEmpty() || !code.IsValid())
        return;

    // At most, we can expect one discard and one assign per unique code.
    BeAssert(into.end() == into.find(code));

    auto existing = ifNotIn.find(code);
    if (ifNotIn.end() != existing)
        {
        // Code was discarded by one and assigned to another within the same changeset...so no net change
        ifNotIn.erase(existing);
        return;
        }

    into.insert(code);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2016
//---------------------------------------------------------------------------------------
void DgnRevision::ExtractCodes(DgnCodeSet& assignedCodes, DgnCodeSet& discardedCodes, DgnDbCR dgndb) const
    {
    ECClassCP elemClass = dgndb.Schemas().GetECClass(BIS_ECSCHEMA_NAME, BIS_CLASS_Element);
    BeAssert(elemClass != nullptr);

    RevisionChangesFileReader changeStream(m_revChangesFile, dgndb);
    ChangeIterator changeIter(dgndb, changeStream);

    assignedCodes.clear();
    discardedCodes.clear();

    for (ChangeIterator::RowEntry const& entry : changeIter)
        {
        if (!entry.IsMapped())
            continue;

        ECClassCP primaryClass = entry.GetPrimaryClass();
        BeAssert(primaryClass != nullptr);

        if (entry.IsJoinedTable() || !primaryClass->Is(elemClass))
            continue;

        DbOpcode dbOpcode = entry.GetDbOpcode();
        ChangeIterator::ColumnIterator columnIter = entry.MakeColumnIterator(*primaryClass); // Note: ColumnIterator needs to be in the stack to access column

        DgnCode oldCode = (dbOpcode == DbOpcode::Insert) ? DgnCode() : GetCodeFromChangeOrDb(columnIter, Changes::Change::Stage::Old);
        DgnCode newCode = (dbOpcode == DbOpcode::Delete) ? DgnCode() : GetCodeFromChangeOrDb(columnIter, Changes::Change::Stage::New);

        if (oldCode == newCode)
            continue;

        insertCode(discardedCodes, oldCode, assignedCodes);
        insertCode(assignedCodes, newCode, discardedCodes);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2016
//---------------------------------------------------------------------------------------
void DgnRevision::ExtractLocks(DgnLockSet& usedLocks, DgnDbCR dgndb) const
    {
    LockRequest lockRequest;

    ECClassCP elemClass = dgndb.Schemas().GetECClass(BIS_ECSCHEMA_NAME, BIS_CLASS_Element);
    BeAssert(elemClass != nullptr);
    ECClassCP modelClass = dgndb.Schemas().GetECClass(BIS_ECSCHEMA_NAME, BIS_CLASS_Model);
    BeAssert(modelClass != nullptr);

    RevisionChangesFileReader changeStream(m_revChangesFile, dgndb);
    ChangeIterator changeIter(dgndb, changeStream);

    for (ChangeIterator::RowEntry const& entry : changeIter)
        {
        if (!entry.IsMapped())
            continue;

        ECClassCP primaryClass = entry.GetPrimaryClass();
        BeAssert(primaryClass != nullptr);

        if (entry.IsJoinedTable() || !primaryClass->Is(elemClass))
            continue;

        ChangeIterator::ColumnIterator columnIter = entry.MakeColumnIterator(*primaryClass); // Note: ColumnIterator needs to be in the stack to access column

        DgnModelId modelId;
        switch (entry.GetDbOpcode())
            {
                case DbOpcode::Insert:  modelId = GetModelIdFromChangeOrDb(columnIter, Changes::Change::Stage::New); break;
                case DbOpcode::Delete:  modelId = GetModelIdFromChangeOrDb(columnIter, Changes::Change::Stage::Old); break;
                case DbOpcode::Update:
                    {
                    modelId = GetModelIdFromChangeOrDb(columnIter, Changes::Change::Stage::New);
                    auto oldModelId = GetModelIdFromChangeOrDb(columnIter, Changes::Change::Stage::Old);
                    if (oldModelId != modelId)
                        lockRequest.InsertLock(LockableId(oldModelId), LockLevel::Shared);

                    break;
                    }
            }

        BeAssert(modelId.IsValid());
        lockRequest.InsertLock(LockableId(modelId), LockLevel::Shared);
        lockRequest.InsertLock(LockableId(DgnElementId(entry.GetPrimaryInstanceId().GetValueUnchecked())), LockLevel::Exclusive);
        }

    // Any models directly changed?
    for (ChangeIterator::RowEntry const& entry : changeIter)
        {
        if (!entry.IsMapped())
            continue;

        ECClassCP primaryClass = entry.GetPrimaryClass();
        BeAssert(primaryClass != nullptr);

        if (entry.IsJoinedTable() || !primaryClass->Is(modelClass))
            continue;

        lockRequest.InsertLock(LockableId(LockableType::Model, DgnModelId(entry.GetPrimaryInstanceId().GetValueUnchecked())), LockLevel::Exclusive);
        }

    // Anything changed at all?
    if (!lockRequest.IsEmpty())
        lockRequest.Insert(dgndb, LockLevel::Shared);

    usedLocks.clear();
    lockRequest.ExtractLockSet(usedLocks);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
RevisionManager::~RevisionManager()
    {
    if (IsCreatingRevision()) 
        AbandonCreateRevision();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
RevisionStatus RevisionManager::SaveParentRevisionId(Utf8StringCR revisionId)
    {
    BeAssert(revisionId.length() == SHA1::HashBytes * 2);
    DbResult result = m_dgndb.SaveBriefcaseLocalValue("ParentRevisionId", revisionId);
    if (BE_SQLITE_DONE != result)
        {
        BeAssert(false);
        return RevisionStatus::SQLiteError;
        }

    return RevisionStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
Utf8String RevisionManager::GetParentRevisionId() const
    {
    Utf8String revisionId;
    DbResult result = m_dgndb.QueryBriefcaseLocalValue(revisionId, "ParentRevisionId");
    return (BE_SQLITE_ROW == result) ? revisionId : "";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
RevisionStatus RevisionManager::UpdateInitialParentRevisionId()
    {
    DbResult result = m_dgndb.SaveBriefcaseLocalValue("InitialParentRevisionId", GetParentRevisionId());
    if (BE_SQLITE_DONE != result)
        {
        BeAssert(false);
        return RevisionStatus::SQLiteError;
        }

    return RevisionStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2016
//---------------------------------------------------------------------------------------
Utf8String RevisionManager::QueryInitialParentRevisionId() const
    {
    Utf8String revisionId;
    DbResult result = m_dgndb.QueryBriefcaseLocalValue(revisionId, "InitialParentRevisionId");
    return (BE_SQLITE_ROW == result) ? revisionId : "";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2016
//---------------------------------------------------------------------------------------
RevisionStatus RevisionManager::SaveCurrentRevisionEndTxnId(TxnManager::TxnId txnId)
    {
    DbResult result = m_dgndb.SaveBriefcaseLocalValue(CURRENT_REV_END_TXN_ID, txnId.GetValue());
    if (BE_SQLITE_DONE != result)
        {
        BeAssert(false);
        return RevisionStatus::SQLiteError;
        }

    result = m_dgndb.SaveChanges();
    if (BE_SQLITE_OK != result)
        {
        BeAssert(false);
        return RevisionStatus::SQLiteError;
        }

    return RevisionStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2016
//---------------------------------------------------------------------------------------
RevisionStatus RevisionManager::DeleteCurrentRevisionEndTxnId()
    {
    DbResult result = m_dgndb.DeleteBriefcaseLocalValue(CURRENT_REV_END_TXN_ID);
    if (BE_SQLITE_DONE != result)
        {
        BeAssert(false);
        return RevisionStatus::SQLiteError;
        }

    result = m_dgndb.SaveChanges();
    if (BE_SQLITE_OK != result)
        {
        BeAssert(false);
        return RevisionStatus::SQLiteError;
        }

    return RevisionStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2016
//---------------------------------------------------------------------------------------
TxnManager::TxnId RevisionManager::QueryCurrentRevisionEndTxnId() const
    {
    uint64_t val;
    DbResult result = m_dgndb.QueryBriefcaseLocalValue(val, CURRENT_REV_END_TXN_ID);
    return (BE_SQLITE_ROW == result) ? TxnManager::TxnId(val) : TxnManager::TxnId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2016
//---------------------------------------------------------------------------------------
bool RevisionManager::IsCreatingRevision() const
    {
    return QueryCurrentRevisionEndTxnId().IsValid();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2016
//---------------------------------------------------------------------------------------
DgnRevisionPtr RevisionManager::GetCreatingRevision()
    {
    if (m_currentRevision.IsValid())
        return m_currentRevision;

    /* Recreate the revision from scratch starting with the saved end transaction id
     * This is to account for the possibility that the client crashed before 
     * FinishCreateRevision() is called. */

    TxnManager::TxnId endTxnId = QueryCurrentRevisionEndTxnId();
    if (!endTxnId.IsValid())
        return nullptr;

    m_currentRevision = CreateRevision(nullptr, endTxnId);
    
    return m_currentRevision;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
RevisionStatus RevisionManager::MergeRevision(DgnRevisionCR revision)
    {
    TxnManagerR txnMgr = m_dgndb.Txns();

    if (!txnMgr.IsTracking())
        {
        BeAssert(false && "Revision API mandates that change tracking is enabled");
        return RevisionStatus::ChangeTrackingNotEnabled;
        }

    if (txnMgr.HasChanges())
        {
        BeAssert(false && "There are unsaved changes in the current transaction. Call db.SaveChanges() or db.AbandonChanges() first");
        return RevisionStatus::TransactionHasUnsavedChanges;
        }

    if (txnMgr.InDynamicTxn())
        {
        BeAssert(false && "Cannot merge revisions if in the middle of a dynamic transaction");
        return RevisionStatus::InDynamicTransaction;
        }

    if (IsCreatingRevision())
        {
        BeAssert(false && "There is already a revision being created. Call AbandonCreateRevision() or FinishCreateRevision() first");
        return RevisionStatus::IsCreatingRevision;
        }

    RevisionStatus status = revision.Validate(m_dgndb);
    if (RevisionStatus::Success != status)
        return status;

    if (GetParentRevisionId() != revision.GetParentId())
        {
        BeAssert(false && "Parent of revision should match the parent revision id of the Db");
        return RevisionStatus::ParentMismatch;
        }

    return txnMgr.MergeRevision(revision);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
RevisionStatus RevisionManager::GroupChanges(ChangeGroup& changeGroup, TxnManager::TxnId endTxnId) const
    {
    TxnManagerR txnMgr = m_dgndb.Txns();

    TxnManager::TxnId startTxnId = txnMgr.QueryNextTxnId(TxnManager::TxnId(0));
    if (!startTxnId.IsValid() || startTxnId >= endTxnId)
        return RevisionStatus::NoTransactions;

    for (TxnManager::TxnId currTxnId = startTxnId; currTxnId < endTxnId; currTxnId = txnMgr.QueryNextTxnId(currTxnId))
        {
        AbortOnConflictChangeSet sqlChangeSet;
        txnMgr.ReadChangeSet(sqlChangeSet, currTxnId, TxnAction::None);

        DbResult result = changeGroup.AddChanges(sqlChangeSet.GetSize(), sqlChangeSet.GetData());
        if (result != BE_SQLITE_OK)
            {
            BeAssert(false && "Failed to group sqlite changesets - see error codes in sqlite3changegroup_add()");
            return RevisionStatus::SQLiteError;
            }
        }

    return RevisionStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
DgnRevisionPtr RevisionManager::CreateRevisionObject(RevisionStatus* outStatus, ChangeGroup& changeGroup)
    {
    Utf8String parentRevId = GetParentRevisionId();
    Utf8String revId = DgnRevisionIdGenerator::GenerateId(parentRevId, changeGroup);
    Utf8String dbGuid = m_dgndb.GetDbGuid().ToString();

    DgnRevisionPtr revision = DgnRevision::Create(outStatus, revId, parentRevId, dbGuid);
    if (revision.IsNull())
        return revision;

    revision->SetInitialParentId(QueryInitialParentRevisionId());
    revision->SetDateTime(DateTime::GetCurrentTimeUtc());

    return revision;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
RevisionStatus RevisionManager::WriteChangesToFile(BeFileNameCR pathname, ChangeGroup& changeGroup)
    {
    RevisionChangesFileWriter writer(pathname, m_dgndb);

    DbResult result = writer.FromChangeGroup(changeGroup);
    if (BE_SQLITE_OK != result)
        {
        BeAssert(false && "Could not write revision to a file");
        return RevisionStatus::FileWriteError;
        }

    return pathname.DoesPathExist() ? RevisionStatus::Success : RevisionStatus::NoTransactions;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2016
//---------------------------------------------------------------------------------------
DgnRevisionPtr RevisionManager::CreateRevision(RevisionStatus* outStatus, TxnManager::TxnId endTxnId)
    {
    RevisionStatus ALLOW_NULL_OUTPUT(status, outStatus);

    ChangeGroup changeGroup;
    status = GroupChanges(changeGroup, endTxnId);
    if (RevisionStatus::Success != status)
        return nullptr;

    DgnRevisionPtr revision = CreateRevisionObject(outStatus, changeGroup);
    if (revision.IsNull())
        return nullptr;

    status = WriteChangesToFile(revision->GetRevisionChangesFile(), changeGroup);
    if (RevisionStatus::Success != status)
        return nullptr;

    return revision;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
DgnRevisionPtr RevisionManager::StartCreateRevision(RevisionStatus* outStatus /* = nullptr */)
    {
    RevisionStatus ALLOW_NULL_OUTPUT(status, outStatus);

    TxnManagerR txnMgr = m_dgndb.Txns();
    if (!txnMgr.IsTracking())
        {
        BeAssert(false && "Creating revisions requires that change tracking is enabled");
        status = RevisionStatus::ChangeTrackingNotEnabled;
        return nullptr;
        }

    if (txnMgr.HasChanges())
        {
        BeAssert(false && "There are unsaved changes in the current transaction. Call db.SaveChanges() or db.AbandonChanges() first");
        status = RevisionStatus::TransactionHasUnsavedChanges;
        return nullptr;
        }

    if (txnMgr.InDynamicTxn())
        {
        BeAssert(false && "Cannot create a revision if in the middle of a dynamic transaction");
        status = RevisionStatus::InDynamicTransaction;
        return nullptr;
        }

    if (IsCreatingRevision())
        {
        BeAssert(false && "There is already a revision being created. Call AbandonCreateRevision() or FinishCreateRevision() first");
        status = RevisionStatus::IsCreatingRevision;
        return nullptr;
        }

    TxnManager::TxnId endTxnId = txnMgr.GetCurrentTxnId();

    DgnRevisionPtr currentRevision = CreateRevision(outStatus, endTxnId);
    if (!currentRevision.IsValid())
        return nullptr;

    status = SaveCurrentRevisionEndTxnId(endTxnId);
    if (RevisionStatus::Success != status)
        return nullptr;

    m_currentRevision = currentRevision;
    return m_currentRevision;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
RevisionStatus RevisionManager::FinishCreateRevision()
    {
    DgnRevisionPtr currentRevision = GetCreatingRevision();
    if (!currentRevision.IsValid())
        {
        BeAssert(false && "No revision is currently being created");
        return RevisionStatus::IsNotCreatingRevision;
        }

    TxnManager::TxnId endTxnId = QueryCurrentRevisionEndTxnId();
    BeAssert(endTxnId.IsValid());

    if (DgnDbStatus::Success != m_dgndb.Txns().DeleteFromStartTo(endTxnId))
        return RevisionStatus::SQLiteError;

    RevisionStatus status = SaveParentRevisionId(currentRevision->GetId());
    if (RevisionStatus::Success != status)
        return status;

    status = UpdateInitialParentRevisionId();
    if (RevisionStatus::Success != status)
        return status;

    m_dgndb.BriefcaseManager().OnFinishRevision(*currentRevision);

    DeleteCurrentRevisionEndTxnId();
    m_currentRevision = nullptr;

    return RevisionStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
void RevisionManager::AbandonCreateRevision()
    {
    BeAssert(IsCreatingRevision());

    DeleteCurrentRevisionEndTxnId();
    m_currentRevision = nullptr;
    }
