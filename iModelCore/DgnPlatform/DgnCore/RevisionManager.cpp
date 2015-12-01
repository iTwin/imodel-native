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

    // NEEDSWORK_MAYBE? If the change group is empty, the output callback is never invoked, therefore file never created
    BentleyStatus EnsureFileExists()
        {
        if (!m_pathname.DoesPathExist())
            {
            BeFile file;
            if (BeFileStatus::Success != file.Create(m_pathname.c_str(), true))
                return ERROR;
            }

        return SUCCESS;
        }
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
DgnRevisionPtr DgnRevision::Create(Utf8StringCR revisionId, Utf8StringCR parentRevisionId, Utf8StringCR dbGuid)
    {
    if (revisionId.empty() || revisionId.length() != SHA1::HashBytes * 2)
        {
        BeAssert(false && "Invalid revision id passed in");
        return nullptr;
        }

    BeFileName changeStreamPathname = BuildChangeStreamPathname(revisionId);
    if (changeStreamPathname.empty())
        return nullptr;

    DgnRevisionPtr revision = new DgnRevision(revisionId, parentRevisionId, dbGuid);
    revision->SetChangeStreamFile(changeStreamPathname);
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
    if (SUCCESS != status)
        {
        BeAssert(false && "Cannot get temporary directory");
        return tempPathname;
        }

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
bool DgnRevision::Validate(DgnDbCR dgndb) const
    {
    if (m_id.empty())
        {
        BeAssert(false && "The revision id is empty");
        return false;
        }

    Utf8String dbGuid = dgndb.GetDbGuid().ToString();
    if (m_dbGuid != dbGuid)
        {
        BeAssert(false && "The revision did not originate in the specified DgnDb");
        return false;
        }

    if (!m_changeStreamFile.DoesPathExist())
        {
        BeAssert(false && "File containing the change stream doesn't exist. Cannot validate.");
        return false;
        }

    ChangeStreamFileReader fs (m_changeStreamFile);
    Utf8String id = DgnRevisionIdGenerator::GenerateId(m_parentId, fs);
    if (m_id != id)
        {
        BeAssert(false && "The contents of the change stream file don't match the DgnRevision");
        return false;
        }

    return true;
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
BentleyStatus RevisionManager::SetParentRevisionId(Utf8StringCR revisionId)
    {
    BeAssert(revisionId.length() == SHA1::HashBytes * 2);
    DbResult result = m_dgndb.SaveBriefcaseLocalValue("ParentRevisionId", revisionId);
    if (BE_SQLITE_DONE != result)
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
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
BentleyStatus RevisionManager::UpdateInitialParentRevisionId()
    {
    DbResult result = m_dgndb.SaveBriefcaseLocalValue("InitialParentRevisionId", GetParentRevisionId());
    if (BE_SQLITE_DONE != result)
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
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
BentleyStatus RevisionManager::MergeRevisions(bvector<DgnRevisionPtr> const& mergeRevisions)
    {
    TxnManagerR txnMgr = m_dgndb.Txns();
    if (mergeRevisions.empty())
        {
        BeAssert(false && "Nothing to merge");
        return SUCCESS;
        }

    if (!txnMgr.IsTracking())
        {
        BeAssert(false && "Using revisions requires that change tracking is enabled");
        return ERROR;
        }

    if (txnMgr.HasChanges() || txnMgr.IsInDynamics())
        {
        BeAssert(false && "There are unsaved changes in the current transaction. Call db.SaveChanges() or db.AbandonChanges() before merging");
        return ERROR;
        }

    if (IsCreatingRevision())
        {
        BeAssert(false && "There is already a revision being created. Call AbandonCreateRevision() or FinishCreateRevision() before merging");
        return ERROR;
        }

    Utf8String dbGuid = m_dgndb.GetDbGuid().ToString();

    bvector<BeFileName> changeStreamFiles;
    for (DgnRevisionPtr const& revision : mergeRevisions)
        {
        if (!revision->Validate(m_dgndb))
            return ERROR;

        changeStreamFiles.push_back(revision->GetChangeStreamFile());
        }
        
    ChangeStreamFileReader revisionStream (changeStreamFiles);
    BentleyStatus status = txnMgr.MergeChanges(revisionStream);
    if (SUCCESS != status)
        return status;
    
    DgnRevisionPtr const& currentRevision = *(mergeRevisions.end() - 1);
    status = SetParentRevisionId(currentRevision->GetId());
    if (SUCCESS != status)
        return status;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
BentleyStatus RevisionManager::GroupChanges(ChangeGroup& changeGroup) const
    {
    TxnManagerR txnMgr = m_dgndb.Txns();
    
    TxnManager::TxnId startTxnId = txnMgr.QueryNextTxnId(TxnManager::TxnId(0));
    TxnManager::TxnId endTxnId = txnMgr.GetCurrentTxnId();

    for (TxnManager::TxnId currTxnId = startTxnId; currTxnId < endTxnId; currTxnId = txnMgr.QueryNextTxnId(currTxnId))
        {
        BeAssert(currTxnId.IsValid());

        AbortOnConflictChangeSet sqlChangeSet;
        txnMgr.ReadChangeSet(sqlChangeSet, currTxnId, TxnAction::None);

        DbResult result = changeGroup.AddChanges(sqlChangeSet.GetSize(), sqlChangeSet.GetData());
        if (result != BE_SQLITE_OK)
            {
            BeAssert(false && "Failed to group sqlite changesets due to either schema changes- see error codes in sqlite3changegroup_add()");
            return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
DgnRevisionPtr RevisionManager::CreateRevisionObject(ChangeGroup& changeGroup)
    {
    Utf8String parentRevId = GetParentRevisionId();
    Utf8String revId = DgnRevisionIdGenerator::GenerateId(parentRevId, changeGroup);
    Utf8String dbGuid = m_dgndb.GetDbGuid().ToString();

    DgnRevisionPtr revision = DgnRevision::Create(revId, parentRevId, dbGuid);
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
BentleyStatus RevisionManager::WriteChangesToFile(BeFileNameCR pathname, ChangeGroup& changeGroup)
    {
    ChangeStreamFileWriter writer(pathname);
    DbResult result = writer.FromChangeGroup(changeGroup);
    if (BE_SQLITE_OK != result)
        {
        BeAssert("Could not write revision to a file");
        return ERROR;
        }

    // NEEDSWORK_MAYBE? If the change group is empty, the output callback is never invoked, therefore file never created
    return writer.EnsureFileExists();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
DgnRevisionPtr RevisionManager::StartCreateRevision()
    {
    TxnManagerR txnMgr = m_dgndb.Txns();
    if (txnMgr.HasChanges() || txnMgr.IsInDynamics())
        {
        BeAssert(false && "There are unsaved changes in the current transaction. Call db.SaveChanges() or db.AbandonChanges() first");
        return nullptr;
        }

    if (IsCreatingRevision())
        {
        BeAssert(false && "There is already a revision being uploaded. Call AbandonCreateRevision() or FinishCreateRevision() first");
        return nullptr;
        }

    ChangeGroup changeGroup;
    BentleyStatus status = GroupChanges(changeGroup);
    if (SUCCESS != status)
        return nullptr;

    DgnRevisionPtr currentRevision = CreateRevisionObject(changeGroup);
    if (currentRevision.IsNull())
        return nullptr;

    status = WriteChangesToFile(currentRevision->GetChangeStreamFile(), changeGroup);
    if (SUCCESS != status)
        return nullptr;

    m_currentRevision = currentRevision;
    m_currentRevisionEndTxnId = txnMgr.GetCurrentTxnId();

    return m_currentRevision;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
BentleyStatus RevisionManager::FinishCreateRevision()
    {
    if (!IsCreatingRevision())
        {
        BeAssert(false && "No revision is currently being created");
        return ERROR;
        }

    DgnDbStatus dbStatus = m_dgndb.Txns().DeleteFromStartTo(m_currentRevisionEndTxnId);
    if (DgnDbStatus::Success != dbStatus)
        return ERROR;

    BentleyStatus status = SetParentRevisionId(m_currentRevision->GetId());
    if (SUCCESS != status)
        return ERROR;

    status = UpdateInitialParentRevisionId();
    BeAssert(SUCCESS == status);

    m_currentRevisionEndTxnId = TxnManager::TxnId(); // Invalid id
    m_currentRevision = nullptr;
    return SUCCESS;
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
