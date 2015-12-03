/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/RevisionManager.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <BeSQLite/SHA1.h>

USING_NAMESPACE_BENTLEY_SQLITE

//=======================================================================================
//! Writes the contents of a change stream to a file
// @bsiclass                                                 Ramanujam.Raman   10/15
//=======================================================================================
struct ChangeStreamFileWriter : ChangeStream
{
private:
    BeFileName m_pathname;
    BeFile m_file;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                Ramanujam.Raman                    10/2015
    //---------------------------------------------------------------------------------------
    DbResult _OutputPage(const void *pData, int nData) override
        {
        BeFileStatus fileStatus;

        if (!m_file.IsOpen())
            {
            fileStatus = m_file.Create(m_pathname.c_str(), true);
            if (fileStatus != BeFileStatus::Success)
                {
                BeAssert(false);
                return BE_SQLITE_ERROR;
                }
            }

        fileStatus = m_file.Write(nullptr, pData, (uint32_t) nData);
        if (fileStatus != BeFileStatus::Success)
            {
            BeAssert(false);
            return BE_SQLITE_ERROR;
            }

        return BE_SQLITE_OK;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                Ramanujam.Raman                    10/2015
    //---------------------------------------------------------------------------------------
    void _Reset() override
        {
        if (m_file.IsOpen())
            m_file.Close();
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                Ramanujam.Raman                    10/2015
    //---------------------------------------------------------------------------------------
    ChangeSet::ConflictResolution _OnConflict(ChangeSet::ConflictCause clause, Changes::Change iter)
        {
        BeAssert(false);
        return ChangeSet::ConflictResolution::Abort;
        }

public:
    //---------------------------------------------------------------------------------------
    // @bsimethod                                Ramanujam.Raman                    10/2015
    //---------------------------------------------------------------------------------------
    ChangeStreamFileWriter(BeFileNameCR pathname) : m_pathname(pathname) {}
    ~ChangeStreamFileWriter() {}
};

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
BentleyStatus ChangeStreamFileReader::CloseCurrentFile()
    {
    BeAssert(m_currentFileIndex >= 0);
    if (!m_currentFile.IsOpen())
        {
        BeAssert(false);
        return ERROR;
        }
    BeFileStatus fileStatus = m_currentFile.Close();
    if (fileStatus != BeFileStatus::Success)
        return ERROR;

    m_currentTotalBytes = 0;
    m_currentByteIndex = 0;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
BentleyStatus ChangeStreamFileReader::OpenNextFile(bool& completedAllFiles)
    {
    m_currentFileIndex++;
    if (m_currentFileIndex >= (int) m_pathnames.size())
        {
        completedAllFiles = true;
        return SUCCESS;
        }
    completedAllFiles = false;

    BeFileStatus fileStatus = m_currentFile.Open(m_pathnames[m_currentFileIndex], BeFileAccess::Read);
    if (fileStatus != BeFileStatus::Success)
        {
        BeAssert(false);
        return ERROR;
        }
    fileStatus = m_currentFile.GetSize(m_currentTotalBytes);
    if (fileStatus != BeFileStatus::Success)
        {
        BeAssert(false);
        return ERROR;
        }

    m_currentByteIndex = 0;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
BentleyStatus ChangeStreamFileReader::ReadNextPage(void *pData, int *pnData)
    {
    uint32_t bytesRead;
    BeFileStatus fileStatus = m_currentFile.Read(pData, &bytesRead, *pnData);
    if (fileStatus != BeFileStatus::Success)
        {
        BeAssert(false);
        return ERROR;
        }

    *pnData = (int) bytesRead;
    m_currentByteIndex += bytesRead;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
bool ChangeStreamFileReader::IsCurrentFileComplete() const
    {
    return m_currentByteIndex >= m_currentTotalBytes;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
DbResult ChangeStreamFileReader::_InputPage(void *pData, int *pnData)
    {
    BentleyStatus status;

    if (m_currentFileIndex < 0 || IsCurrentFileComplete())
        {
        if (m_currentFileIndex >= 0)
            {
            status = CloseCurrentFile();
            if (SUCCESS != status)
                return BE_SQLITE_ERROR;
            }

        bool completedAllFiles;
        status = OpenNextFile(completedAllFiles);
        if (SUCCESS != status)
            return BE_SQLITE_ERROR;

        if (completedAllFiles)
            {
            *pnData = 0;
            return BE_SQLITE_OK;
            }
        }

    status = ReadNextPage(pData, pnData);
    if (SUCCESS != status)
        return BE_SQLITE_ERROR;

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
void ChangeStreamFileReader::_Reset()
    {
    if (m_currentFile.IsOpen())
        CloseCurrentFile();
    m_currentFileIndex = -1;
    m_currentTotalBytes = 0;
    m_currentByteIndex = 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
ChangeSet::ConflictResolution ChangeStreamFileReader::_OnConflict(ChangeSet::ConflictCause clause, Changes::Change iter)
    {
    BeAssert(false);
    return ChangeSet::ConflictResolution::Abort;
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
    ChangeSet::ConflictResolution _OnConflict(ChangeSet::ConflictCause clause, Changes::Change iter)
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

    BeFileName changeStreamPathname = BuildChangeStreamPathname(revisionId);
    
    DgnRevisionPtr revision = new DgnRevision(revisionId, parentRevisionId, dbGuid);
    revision->SetChangeStreamFile(changeStreamPathname);
    status = RevisionStatus::Success;
    return revision;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
DgnRevision::~DgnRevision()
    {
    if (m_changeStreamFile.DoesPathExist())
        {
        BeFileNameStatus status = m_changeStreamFile.BeDeleteFile();
        BeAssert(BeFileNameStatus::Success == status && "Could not delete temporary change stream file");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
// static
BeFileName DgnRevision::BuildChangeStreamPathname(Utf8String revisionId)
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
    printf("Id : %s\n", m_id.c_str());
    printf("ParentId : %s\n", m_parentId.c_str());
    printf("Initial ParentId : %s\n", m_initialParentId.c_str());
    printf("DbGuid: %s\n", m_dbGuid.c_str());
    printf("User Name: %s\n", m_userName.c_str());
    printf("Summary: %s\n", m_summary.c_str());
    printf("ChangeStreamFile: %ls\n", m_changeStreamFile.c_str());
    printf("DateTime: %s\n", m_dateTime.ToUtf8String().c_str());

    ChangeStreamFileReader fs(m_changeStreamFile);
    fs.Dump("Contents:\n", dgndb, false, 0);

    printf("\n");
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

    if (!m_changeStreamFile.DoesPathExist())
        {
        BeAssert(false && "File containing the change stream doesn't exist. Cannot validate.");
        return RevisionStatus::FileNotFound;
        }

    ChangeStreamFileReader fs (m_changeStreamFile);
    Utf8String id = DgnRevisionIdGenerator::GenerateId(m_parentId, fs);
    if (m_id != id)
        {
        BeAssert(false && "The contents of the change stream file don't match the DgnRevision");
        return RevisionStatus::CorruptedChangeStream;
        }

    return RevisionStatus::Success;
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
RevisionStatus RevisionManager::SetParentRevisionId(Utf8StringCR revisionId)
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
    DbResult result = m_dgndb.QueryBriefcaseLocalValue("ParentRevisionId", revisionId);
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
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
Utf8String RevisionManager::GetInitialParentRevisionId() const
    {
    Utf8String revisionId;
    DbResult result = m_dgndb.QueryBriefcaseLocalValue("InitialParentRevisionId", revisionId);
    return (BE_SQLITE_ROW == result) ? revisionId : "";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
RevisionStatus RevisionManager::MergeRevisions(bvector<DgnRevisionPtr> const& mergeRevisions)
    {
    TxnManagerR txnMgr = m_dgndb.Txns();
    if (mergeRevisions.empty())
        {
        BeAssert(false && "Nothing to merge");
        return RevisionStatus::NothingToMerge;
        }

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

    if (txnMgr.IsInDynamics())
        {
        BeAssert(false && "Cannot merge revisions if in the middle of a dynamic transaction");
        return RevisionStatus::InDynamicTransaction;
        }

    if (IsCreatingRevision())
        {
        BeAssert(false && "There is already a revision being created. Call AbandonCreateRevision() or FinishCreateRevision() first");
        return RevisionStatus::IsCreatingRevision;
        }

    Utf8String dbGuid = m_dgndb.GetDbGuid().ToString();

    RevisionStatus status;
    bvector<BeFileName> changeStreamFiles;
    for (DgnRevisionPtr const& revision : mergeRevisions)
        {
        status = revision->Validate(m_dgndb);
        if (RevisionStatus::Success != status)
            return status;

        changeStreamFiles.push_back(revision->GetChangeStreamFile());
        }
        
    ChangeStreamFileReader revisionStream (changeStreamFiles);
    if (SUCCESS != txnMgr.MergeChanges(revisionStream))
        return RevisionStatus::MergeError;
    
    DgnRevisionPtr const& currentRevision = *(mergeRevisions.end() - 1);
    status = SetParentRevisionId(currentRevision->GetId());
    if (RevisionStatus::Success != status)
        return status;

    return RevisionStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
RevisionStatus RevisionManager::GroupChanges(ChangeGroup& changeGroup) const
    {
    TxnManagerR txnMgr = m_dgndb.Txns();
    
    TxnManager::TxnId startTxnId = txnMgr.QueryNextTxnId(TxnManager::TxnId(0));
    if (!startTxnId.IsValid())
        return RevisionStatus::NoTransactions;

    TxnManager::TxnId endTxnId = txnMgr.GetCurrentTxnId();

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

    revision->SetInitialParentId(GetInitialParentRevisionId());
    revision->SetDateTime(DateTime::GetCurrentTimeUtc());
    return revision;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
// static
RevisionStatus RevisionManager::WriteChangesToFile(BeFileNameCR pathname, ChangeGroup& changeGroup)
    {
    ChangeStreamFileWriter writer(pathname);
    DbResult result = writer.FromChangeGroup(changeGroup);
    if (BE_SQLITE_OK != result || !pathname.DoesPathExist())
        {
        BeAssert("Could not write revision to a file");
        return RevisionStatus::FileWriteError;
        }

    return RevisionStatus::Success;
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

    if (txnMgr.IsInDynamics())
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

    ChangeGroup changeGroup;
    status = GroupChanges(changeGroup);
    if (RevisionStatus::Success != status)
        return nullptr;

    DgnRevisionPtr currentRevision = CreateRevisionObject(outStatus, changeGroup);
    if (currentRevision.IsNull())
        return nullptr;

    status = WriteChangesToFile(currentRevision->GetChangeStreamFile(), changeGroup);
    if (RevisionStatus::Success != status)
        return nullptr;

    m_currentRevision = currentRevision;
    m_currentRevisionEndTxnId = m_dgndb.Txns().GetCurrentTxnId();
    
    return m_currentRevision;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
RevisionStatus RevisionManager::FinishCreateRevision()
    {
    if (!IsCreatingRevision())
        {
        BeAssert(false && "No revision is currently being created");
        return RevisionStatus::IsNotCreatingRevision;
        }

    if (DgnDbStatus::Success != m_dgndb.Txns().DeleteFromStartTo(m_currentRevisionEndTxnId))
        return RevisionStatus::SQLiteError;

    RevisionStatus status = SetParentRevisionId(m_currentRevision->GetId());
    if (RevisionStatus::Success != status)
        return status;

    status = UpdateInitialParentRevisionId();
    if (RevisionStatus::Success != status)
        return status;

    m_currentRevisionEndTxnId = TxnManager::TxnId(); // Invalid id
    m_currentRevision = nullptr;
    return RevisionStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
void RevisionManager::AbandonCreateRevision()
    {
    BeAssert(IsCreatingRevision());

    m_currentRevisionEndTxnId = TxnManager::TxnId(); // Invalid id
    m_currentRevision = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TxnManager::TxnId RevisionManager::GetCurrentRevisionEndTxnId() const
    {
    return m_currentRevisionEndTxnId;
    }
