/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <Bentley/SHA1.h>
#include <DgnPlatform/DgnChangeSummary.h>
#include <ECDb/ChangeIterator.h>
#include <folly/ProducerConsumerQueue.h>
#include <thread>

USING_NAMESPACE_BENTLEY_SQLITE

#define CURRENT_CS_END_TXN_ID "CurrentChangeSetEndTxnId"
#define LAST_REBASE_ID "LastRebaseId"
#define PARENT_CS_ID "ParentChangeSetId"
#define PARENT_CHANGESET "parentChangeset"
#define CHANGESET_REL_DIR L"ChangeSets"
#define CHANGESET_FILE_EXT L"changeset"

// #define DEBUG_REVISION_KEEP_FILES 1
// #define DUMP_REVISION 1

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ChangeSet::ConflictResolution RevisionChangesFileReader::_OnConflict(ChangeSet::ConflictCause cause, Changes::Change iter)
    {
    DgnDb const* dgndb = dynamic_cast<DgnDb const*>(&GetDb());
    return RevisionManager::ConflictHandler(*dgndb, cause, iter);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ChangeSet::ConflictResolution ApplyRevisionChangeSet::_OnConflict(ChangeSet::ConflictCause cause, Changes::Change iter)
    {
    return RevisionManager::ConflictHandler(m_dgndb, cause, iter);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ChangeSet::ConflictResolution RevisionManager::ConflictHandler(DgnDbCR dgndb, ChangeSet::ConflictCause cause, Changes::Change iter)
    {
    Utf8CP tableName = nullptr;
    int nCols, indirect;
    DbOpcode opcode;
    DbResult result = iter.GetOperation(&tableName, &nCols, &opcode, &indirect);
    BeAssert(result == BE_SQLITE_OK);

    UNUSED_VARIABLE(result);

    // Handle some special cases
    if (cause == ChangeSet::ConflictCause::Conflict)
        {
        // From the SQLite docs: "CHANGESET_CONFLICT is passed as the second argument to the conflict handler while processing an INSERT change if the operation would result in duplicate primary key values."
        // This is always a fatal error - it can happen only if the app started with a briefcase that is behind the tip and then uses the same primary key values (e.g., ElementIds)
        // that have already been used by some other app using the SAME briefcase ID that recently pushed changes. That can happen only if the app makes changes without first pulling and acquiring locks.
        if (!const_cast<DgnDbR>(dgndb).Txns().HasPendingTxns())
            {
            // This changeset is bad. However, it is already in the timeline. We must allow services such as
            // checkpoint-creation, change history, and other apps to apply any changeset that is in the timeline.
            LOG.info("PRIMARY KEY INSERT CONFLICT - resolved by replacing the existing row with the incoming row");
            iter.Dump(dgndb, false, 1);
            }
        else
            {
            LOG.fatal("PRIMARY KEY INSERT CONFLICT - rejecting this changeset");
            iter.Dump(dgndb, false, 1);
            return ChangeSet::ConflictResolution::Abort;
            }
        }

    if (cause == ChangeSet::ConflictCause::ForeignKey)
        {
        // Note: No current or conflicting row information is provided if it's a FKey conflict
        // Since we abort on FKey conflicts, always try and provide details about the error
        int nConflicts = 0;
        result = iter.GetFKeyConflicts(&nConflicts);
        BeAssert(result == BE_SQLITE_OK);
        LOG.errorv("Detected %d foreign key conflicts in ChangeSet. Aborting merge.", nConflicts);
        return ChangeSet::ConflictResolution::Abort ;
        }

    if (cause == ChangeSet::ConflictCause::NotFound)
        {
        /*
        * Note: If ConflictCause = NotFound, the primary key was not found, and returning ConflictResolution::Replace is
        * not an option at all - this will cause a BE_SQLITE_MISUSE error.
        */
        if (opcode == DbOpcode::Delete)
            {
            // Caused by CASCADE DELETE on a foreign key, and is usually not a problem.
            return ChangeSet::ConflictResolution::Skip;
            }


        if (opcode == DbOpcode::Update && 0 == ::strncmp(tableName, "ec_", 3))
            {
            // Caused by a ON DELETE SET NULL constraint on a foreign key - this is known to happen with "ec_" tables, but needs investigation if it happens otherwise
            return ChangeSet::ConflictResolution::Skip;
            }

#if defined (WORK_ON_CHANGE_MERGING)
        if (!letControlHandleThis)
#endif
            {
            // Refer to comment below
            return opcode == DbOpcode::Update ? ChangeSet::ConflictResolution::Skip : ChangeSet::ConflictResolution::Replace;
            }
        }

#if defined (WORK_ON_CHANGE_MERGING)
    if (letControlHandleThis)
        {
        // if we have a concurrency control, then we allow it to decide how to handle conflicts with local changes.
        // (We don't call the control in the case where there are no local changes. As explained below, we always want the incoming changes in that case.)
        return control->_OnConflict(dgndb, cause, iter);
        }
#endif

    if (ChangeSet::ConflictCause::Constraint == cause)
        {
        if (LOG.isSeverityEnabled(NativeLogging::LOG_INFO))
            {
            LOG.infov("------------------------------------------------------------------");
            LOG.infov("Conflict detected - Cause: %s", ChangeSet::InterpretConflictCause(cause, 1));
            iter.Dump(dgndb, false, 1);
            }

        LOG.warning("Constraint conflict handled by rejecting incoming change. Constraint conflicts are NOT expected. These happen most often when two clients both insert elements with the same code. That indicates a bug in the client or the code server.");
        return ChangeSet::ConflictResolution::Skip;
        }

    /*
     * If we don't have a control, we always accept the incoming revision in cases of conflicts:
     *
     * + In a briefcase with no local changes, the state of a row in the Db (i.e., the final state of a previous revision)
     *   may not exactly match the initial state of the incoming revision. This will cause a conflict.
     *      - The final state of the incoming (later) revision will always be setup exactly right to accommodate
     *        cases where dependency handlers won't be available (for instance on the server), and we have to rely on
     *        the revision to correctly set the final state of the row in the Db. Therefore it's best to resolve the
     *        conflict in favor of the incoming change.
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
     * + Also see comments in TxnManager::MergeDataChangesInRevision()
     */
    if (LOG.isSeverityEnabled(NativeLogging::LOG_INFO))
        {
        LOG.infov("------------------------------------------------------------------");
        LOG.infov("Conflict detected - Cause: %s", ChangeSet::InterpretConflictCause(cause, 1));
        iter.Dump(dgndb, false, 1);
        LOG.infov("Conflicting resolved by replacing the existing entry with the change");
        }
    return ChangeSet::ConflictResolution::Replace;
    }

//=======================================================================================
//! Generates the DgnRevision Id
// @bsiclass
//=======================================================================================
struct DgnRevisionIdGenerator : ChangeStream
{
private:
    SHA1 m_hash;

    //---------------------------------------------------------------------------------------
    // @bsimethod
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
    // @bsimethod
    //---------------------------------------------------------------------------------------
    void AddStringToHash(Utf8StringCR hashString)
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
    // @bsimethod
    //---------------------------------------------------------------------------------------
    Utf8String GetHashString()
        {
        return m_hash.GetHashString();
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod
    //---------------------------------------------------------------------------------------
    DbResult _Append(Byte const* pData, int nData) override
        {
        m_hash.Add(pData, nData);
        return BE_SQLITE_OK;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod
    //---------------------------------------------------------------------------------------
    ChangeSet::ConflictResolution _OnConflict(ChangeSet::ConflictCause cause, Changes::Change iter) override
        {
        BeAssert(false);
        return ChangeSet::ConflictResolution::Abort;
        }

public:
    DgnRevisionIdGenerator() {}
    bool _IsEmpty() const override final { return false; }
    RefCountedPtr<Changes::Reader> _GetReader() const override { return nullptr; }

    //---------------------------------------------------------------------------------------
    // @bsimethod
    //---------------------------------------------------------------------------------------
    static RevisionStatus GenerateId(Utf8StringR revId, Utf8StringCR parentRevId, BeFileNameCR revisionFile, DgnDbCR dgndb)
        {
        revId.clear();
        RevisionChangesFileReader fs(revisionFile, dgndb);

        DgnRevisionIdGenerator idgen;
        idgen.AddStringToHash(parentRevId);

        auto reader = fs.MakeReader();
        DbResult result;
        Utf8StringCR prefix = reader->GetPrefix(result);
        if (BE_SQLITE_OK != result)
            {
            BeAssert(false);
            return (result == BE_SQLITE_ERROR_InvalidChangeSetVersion) ? RevisionStatus::InvalidVersion : RevisionStatus::CorruptedChangeStream;
            }

        if (!prefix.empty())
            idgen._Append((Byte const*) prefix.c_str(), (int) prefix.SizeInBytes());

        result = idgen.ReadFrom(*reader);
        if (BE_SQLITE_OK != result)
            {
            BeAssert(false);
            return RevisionStatus::CorruptedChangeStream;
            }

        revId = idgen.GetHashString();
        return RevisionStatus::Success;
        }
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnRevisionPtr DgnRevision::Create(Utf8StringCR changesetId, int32_t changesetIndex, Utf8StringCR parentRevisionId, Utf8StringCR dbGuid, BeFileNameCP fileName) {
    if (changesetId.empty() || changesetId.length() != SHA1::HashBytes * 2) {
        BeAssert(false && "Empty or invalid revision id passed in");
        return nullptr;
    }

    DgnRevisionPtr revision = new DgnRevision(changesetId, changesetIndex, parentRevisionId, dbGuid);
    if (nullptr == fileName) {
        revision->m_revChangesFile = BuildRevisionChangesPathname(changesetId);
        revision->m_ownsRevChangesFile = true;
    } else {
        revision->m_revChangesFile.SetName(fileName->GetName());
        revision->m_ownsRevChangesFile = false;
    }
    return revision;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnRevision::~DgnRevision()
    {
#ifndef DEBUG_REVISION_KEEP_FILES
    if (m_ownsRevChangesFile && m_revChangesFile.DoesPathExist())
        {
        BeFileNameStatus status = m_revChangesFile.BeDeleteFile();
        if (BeFileNameStatus::Success != status)
            {
            LOG.errorv(L"Could not delete temporary change stream file %ls - error %x", m_revChangesFile.c_str(), status);
            BeAssert(BeFileNameStatus::Success == status && "Could not delete temporary change stream file");
            }
        }
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BeFileName DgnRevision::BuildRevisionChangesPathname(Utf8String revisionId)
    {
    BeFileName tempPathname;
    BentleyStatus status = T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectory(tempPathname, CHANGESET_REL_DIR);
    UNUSED_VARIABLE(status);
    BeAssert(SUCCESS == status && "Cannot get temporary directory");
    tempPathname.AppendToPath(WPrintfString(L"%d", BeThreadUtilities::GetCurrentProcessId()).c_str());
    BeFileName::CreateNewDirectory(tempPathname.c_str());
    tempPathname.AppendToPath(WString(revisionId.c_str(), true).c_str());
    tempPathname.AppendExtension(CHANGESET_FILE_EXT);
    return tempPathname;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void DgnRevision::Dump(DgnDbCR dgndb) const
    {
// Don't log "sensitive" information in production builds.
#if !defined(NDEBUG)
    LOG.infov("Id : %s", m_id.c_str());
    LOG.infov("ParentId : %s", m_parentId.c_str());
    LOG.infov("DbGuid: %s", m_dbGuid.c_str());
    LOG.infov("User Name: %s", m_userName.c_str());
    LOG.infov("Summary: %s", m_summary.c_str());
    LOG.infov("ChangeStreamFile: %ls", m_revChangesFile.c_str());
    LOG.infov("DateTime: %s", m_dateTime.ToString().c_str());

    RevisionChangesFileReader fs(m_revChangesFile, dgndb);

    bool containsSchemaChanges;
    DdlChanges ddlChanges;
    DbResult result = fs.MakeReader()->GetSchemaChanges(containsSchemaChanges, ddlChanges);
    UNUSED_VARIABLE(result);
    BeAssert(result == BE_SQLITE_OK);

    LOG.infov("Contains Schema Changes: %s", containsSchemaChanges ? "yes" : "no");
    LOG.infov("Contains Ddl Changes: %s", (ddlChanges.GetSize() > 0) ? "yes" : "no");
    if (ddlChanges.GetSize() > 0)
        ddlChanges.Dump("DDL: ");

    fs.Dump("ChangeSet:\n", dgndb, false, 0);
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
RevisionStatus DgnRevision::Validate(DgnDbCR dgndb) const
    {
    if (m_id.empty() || m_id.length() != SHA1::HashBytes * 2)
        {
        BeAssert(false && "The revision id is empty");
        LOG.errorv("Changeset Id (%s) is not a valid SHA1 hash", m_id.c_str());
        return RevisionStatus::InvalidId;
        }

    Utf8String dbGuid = dgndb.GetDbGuid().ToString();
    if (m_dbGuid != dbGuid)
        {
        BeAssert(false && "The revision did not originate in the specified DgnDb");
        LOG.errorv("The changeset did not originate in this bim file. this.DbGuid (%s) <> changeset.DbGuid (%s)", dbGuid.c_str(), m_dbGuid.c_str());
        return RevisionStatus::WrongDgnDb;
        }

    if (!m_revChangesFile.DoesPathExist())
        {
        BeAssert(false && "File containing the change stream doesn't exist. Cannot validate.");
        LOG.errorv("Changeset (id=%s) file not found. (%s)", m_id.c_str(), m_revChangesFile.GetNameUtf8().c_str());
        return RevisionStatus::FileNotFound;
        }

    Utf8String id;
    RevisionStatus status = DgnRevisionIdGenerator::GenerateId(id, m_parentId, m_revChangesFile, dgndb);
    if (status != RevisionStatus::Success)
        return status;

    if (m_id != id)
        {
        BeAssert(false && "The contents of the change stream file don't match the DgnRevision");
        LOG.errorv("Changeset SHA1 hash does not match its content. expected: %s <> actual: %s", m_id.c_str(), id.c_str());
        return RevisionStatus::CorruptedChangeStream;
        }

    return RevisionStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool DgnRevision::ContainsSchemaChanges(DgnDbCR dgndb) const
    {
    RevisionChangesFileReader changeStream(m_revChangesFile, dgndb);

    bool containsSchemaChanges;
    DdlChanges ddlChanges;
    DbResult result = changeStream.MakeReader()->GetSchemaChanges(containsSchemaChanges, ddlChanges);
    if (BE_SQLITE_OK != result)
        {
        BeAssert(false);
        return false;
        }

    return containsSchemaChanges; // TODO: Consider caching this flag
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
RevisionManager::RevisionManager(DgnDbR dgndb) : m_dgndb(dgndb)
    {
    m_tempRevisionPathname = BuildTempRevisionPathname();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RevisionManager::ClearSavedValues() {
    m_dgndb.DeleteBriefcaseLocalValue(CURRENT_CS_END_TXN_ID);
    m_dgndb.DeleteBriefcaseLocalValue(LAST_REBASE_ID);
    m_dgndb.DeleteBriefcaseLocalValue(PARENT_CS_ID);
    m_dgndb.DeleteBriefcaseLocalValue("ReversedChangeSetId"); // clean up from old versions
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
RevisionStatus RevisionManager::SaveParentRevision(Utf8StringCR revisionId, int32_t changesetIndex) {
    if (revisionId.length() != SHA1::HashBytes * 2)
        return RevisionStatus::BadVersionId;

    // Early versions of iModels didn't save the changesetIndex, which is unfortunate since that's what we usually need. You can
    // get it by a round-trip query to IModelHub, but it's often convenient to know it locally, so we now store it in the be_local table.

    // Its value can only be used if the id in the PARENT_CHANGESET matches PARENT_CS_ID, since older software may update the latter without
    // updating the former. Eventually, we should be able to remove this, since it's redundant with the value stored in PARENT_CHANGESET. We set it
    // here for backwards compatibility for old apps before changesetIndex was persisted, and so we can tell that PARENT_CHANGESET is valid.
    m_dgndb.SaveBriefcaseLocalValue(PARENT_CS_ID, revisionId);

    // A changesetIndex value greater than 0 means that we obtained the value from iModelHub successfully. Old apps didn't save this value
    // so we can't rely on it. This can only be used if the value of "id" matches the value in PARENT_CS_ID
    if (changesetIndex > 0) {
        BeJsDocument parentChangeset;
        parentChangeset["id"] = revisionId;
        parentChangeset["index"] = changesetIndex;
        auto jsonStr = parentChangeset.Stringify();
        m_dgndb.SaveBriefcaseLocalValue(PARENT_CHANGESET, jsonStr);
    } else {
        // the application hasn't been converted to supply changesetIndex. Remove any values saved by other apps, because it's now out of sync.
        m_dgndb.DeleteBriefcaseLocalValue(PARENT_CHANGESET);
    }

    return RevisionStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RevisionManager::HasParentRevision() const {
    Utf8String revisionId;
    return BE_SQLITE_ROW == m_dgndb.QueryBriefcaseLocalValue(revisionId, PARENT_CS_ID);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String RevisionManager::GetParentRevisionId() const {
    Utf8String revisionId;
    DbResult result = m_dgndb.QueryBriefcaseLocalValue(revisionId, PARENT_CS_ID);
    return (BE_SQLITE_ROW == result) ? revisionId : "";
}

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RevisionManager::GetParentRevision(int32_t& index, Utf8StringR id) const {
    id = GetParentRevisionId();
    index = id.empty() ? 0 : -1; // not known

    // the PARENT_CHANGESET member may or may not exist. If it does, and only if its id member is equal to parentId, use it's index value
    Utf8String json;
    if (BE_SQLITE_ROW != m_dgndb.QueryBriefcaseLocalValue(json, PARENT_CHANGESET))
        return;

    BeJsDocument jsonObj(json);
    if (jsonObj.isStringMember("id") && jsonObj.isNumericMember("index") && id.Equals(jsonObj["id"].asString()))
        index = jsonObj["index"].GetInt();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
RevisionStatus RevisionManager::SaveCurrentRevisionEndTxnId(TxnManager::TxnId txnId, int64_t rebaseId)
    {
    DbResult result = m_dgndb.SaveBriefcaseLocalValue(CURRENT_CS_END_TXN_ID, txnId.GetValue());

    if (BE_SQLITE_DONE == result)
        result = m_dgndb.SaveBriefcaseLocalValue(LAST_REBASE_ID, rebaseId);

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
// @bsimethod
//---------------------------------------------------------------------------------------
RevisionStatus RevisionManager::DeleteCurrentRevisionEndTxnId()
    {
    DbResult result = m_dgndb.DeleteBriefcaseLocalValue(CURRENT_CS_END_TXN_ID);
    if (BE_SQLITE_DONE == result)
        result = m_dgndb.DeleteBriefcaseLocalValue(LAST_REBASE_ID);

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
// @bsimethod
//---------------------------------------------------------------------------------------
TxnManager::TxnId RevisionManager::QueryCurrentRevisionEndTxnId() const
    {
    uint64_t val;
    DbResult result = m_dgndb.QueryBriefcaseLocalValue(val, CURRENT_CS_END_TXN_ID);
    return (BE_SQLITE_ROW == result) ? TxnManager::TxnId(val) : TxnManager::TxnId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int64_t RevisionManager::QueryLastRebaseId() const
    {
    uint64_t val;
    DbResult result = m_dgndb.QueryBriefcaseLocalValue(val, LAST_REBASE_ID);
    return (BE_SQLITE_ROW == result) ? val : 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool RevisionManager::IsCreatingRevision() const
    {
    return QueryCurrentRevisionEndTxnId().IsValid();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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

    m_currentRevision = CreateRevision(nullptr, endTxnId, QueryLastRebaseId());

    return m_currentRevision;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
RevisionStatus RevisionManager::MergeRevision(DgnRevisionCR revision)
    {
    PRECONDITION(!m_dgndb.IsReadonly() && "Cannot merge changes into this database", RevisionStatus::CannotMergeIntoReadonly);

    TxnManagerR txnMgr = m_dgndb.Txns();
    PRECONDITION(!txnMgr.HasChanges() && "There are unsaved changes in the current transaction. Call db.SaveChanges() or db.AbandonChanges() first", RevisionStatus::HasUncommittedChanges);
    PRECONDITION(!txnMgr.InDynamicTxn() && "Cannot merge revisions if in the middle of a dynamic transaction", RevisionStatus::InDynamicTransaction);
    PRECONDITION(!IsCreatingRevision() && "There is already a revision being created. Call AbandonCreateRevision() or FinishCreateRevision() first", RevisionStatus::IsCreatingRevision)

    RevisionStatus status = revision.Validate(m_dgndb);
    if (RevisionStatus::Success != status)
        return status;

    PRECONDITION(GetParentRevisionId() == revision.GetParentId() && "Parent of revision should match the parent revision id of the Db", RevisionStatus::ParentMismatch);

#ifdef DUMP_REVISION
    revision.Dump(m_dgndb);
#endif

    return txnMgr.MergeRevision(revision);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
RevisionStatus RevisionManager::GroupChanges(DdlChangesR ddlChanges, ChangeGroupR dataChangeGroup, TxnManager::TxnId endTxnId) const {
    TxnManagerR txnMgr = m_dgndb.Txns();

    TxnManager::TxnId startTxnId = txnMgr.QueryNextTxnId(TxnManager::TxnId(0));
    if (!startTxnId.IsValid() || startTxnId >= endTxnId)
        return RevisionStatus::NoTransactions;

    for (TxnManager::TxnId currTxnId = startTxnId; currTxnId < endTxnId; currTxnId = txnMgr.QueryNextTxnId(currTxnId)) {
        auto txnType = txnMgr.GetTxnType(currTxnId);

        if (txnType == TxnType::EcSchema) // if we have EcSchema changes, set the flag on the changegroup
            dataChangeGroup.SetContainsEcSchemaChanges();

        if (txnType == TxnType::Ddl) {
            BeAssert(ddlChanges._IsEmpty());
            if (txnMgr.ReadChanges(ddlChanges, currTxnId) != ZIP_SUCCESS) {
                LOG.error(L"Unable to read schema changes - RevisionStatus::CorruptedTxn");
                return RevisionStatus::CorruptedTxn;
            }
        } else {
            ChangeSet sqlChangeSet;
            if (txnMgr.ReadDataChanges(sqlChangeSet, currTxnId, TxnAction::None) != BE_SQLITE_OK) {
                LOG.error(L"Unable to read data changes - RevisionStatus::CorruptedTxn");
                return RevisionStatus::CorruptedTxn;
            }

            DbResult result = sqlChangeSet.AddToChangeGroup(dataChangeGroup);
            if (result != BE_SQLITE_OK) {
                LOG.errorv(L"sqlite3changegroup_add failed: %ls", WString(BeSQLiteLib::GetErrorName(result), BentleyCharEncoding::Utf8).c_str());
                BeAssert(false && "Failed to group sqlite changesets - see error codes in sqlite3changegroup_add()");
                return RevisionStatus::SQLiteError;
            }
        }
    }
    return RevisionStatus::Success;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnRevisionPtr RevisionManager::CreateRevisionObject(BeFileNameCR tempRevisionPathname)
    {
    Utf8String parentRevId = GetParentRevisionId();
    Utf8String revId;
    RevisionStatus status = DgnRevisionIdGenerator::GenerateId(revId, parentRevId, tempRevisionPathname, m_dgndb);
    UNUSED_VARIABLE(status);
    BeAssert(status == RevisionStatus::Success);
    Utf8String dbGuid = m_dgndb.GetDbGuid().ToString();

    DgnRevisionPtr revision = DgnRevision::Create(revId, -1, parentRevId, dbGuid);
    if (revision.IsNull())
        return revision;

    BeFileNameCR revisionPathname = revision->GetRevisionChangesFile();
    if (revisionPathname.DoesPathExist() && BeFileNameStatus::Success != revisionPathname.BeDeleteFile()) // Note: Need to delete since BeMoveFile doesn't overwrite
        {
        BeAssert(false && "Could not setup file containing revision changes");
        return nullptr;
        }

    if (BeFileNameStatus::Success != BeFileName::BeMoveFile(tempRevisionPathname.c_str(), revision->GetRevisionChangesFile().c_str(), 2))
        {
        BeAssert(false && "Could not rename temp revision changes file");
        return nullptr;
        }

    revision->SetDateTime(DateTime::GetCurrentTimeUtc());

    return revision;
    }

//=======================================================================================
//! Handles a request to stream output by writing to a ProducerConsumerQueue. Can be used as the producer side of a concurrent pipeline.
// @bsiclass
//=======================================================================================
struct ChangeStreamQueueProducer : ChangeStream
    {
    folly::ProducerConsumerQueue<bvector<uint8_t>>& m_q;
    ChangeStreamQueueProducer(folly::ProducerConsumerQueue<bvector<uint8_t>>& q) : m_q(q) {}
    DbResult _Append(Byte const* pData, int nData) override
        {
        while (!m_q.write(pData, pData + nData))
            ;   // spin until the queue has room
        return BE_SQLITE_OK;
        }
    ConflictResolution _OnConflict(ConflictCause clause, Changes::Change iter) override {BeAssert(false); return ConflictResolution::Abort;}

    RefCountedPtr<Changes::Reader> _GetReader() const override { return nullptr; }
    bool _IsEmpty() const override { return false; }
    };

//=======================================================================================
//! Satisfies a request for input by reading from a ProducerConsumerQueue. Can be used as the consumer side of a concurrent pipeline.
// @bsiclass
//=======================================================================================
struct ChangeStreamQueueConsumer : ChangeStream {
    folly::ProducerConsumerQueue<bvector<uint8_t>>& m_q;
    ChangeStreamQueueConsumer(folly::ProducerConsumerQueue<bvector<uint8_t>>& q) : m_q(q) {}

    struct Reader : Changes::Reader {
        ChangeStreamQueueConsumer const& m_consumer;
        Reader(ChangeStreamQueueConsumer const& consumer) : m_consumer(consumer) {}
        bvector<uint8_t> m_remaining;

        DbResult _Read(Byte* pData, int* pnData) override {
            if (!m_remaining.empty()) {
                // If the queued buffer was bigger than the caller's buffer, then we return
                // the remaining portion in the amounts that the caller can deal with.
                size_t retcount = std::min((size_t)*pnData, m_remaining.size());
                memcpy(pData, m_remaining.data(), retcount);
                *pnData = (int)retcount;
                m_remaining.erase(m_remaining.begin(), m_remaining.begin() + retcount);
                return BE_SQLITE_OK;
            }

            bvector<uint8_t>* pval;
            do {
                pval = m_consumer.m_q.frontPtr();
            } while (!pval); // spin until we get a value;

            if (pval->empty()) {
                *pnData = 0;
            } else {
                // return as much of the queued buffer as the caller's buffer can hold.
                size_t retcount = std::min((size_t)*pnData, pval->size());
                memcpy(pData, pval->data(), retcount);
                *pnData = (int)retcount;

                // if the queued buffer has more, save it for the next call
                if (retcount < pval->size())
                    m_remaining.assign(pval->data() + retcount, pval->data() + pval->size());
            }

            m_consumer.m_q.popFront();
            return BE_SQLITE_OK;
        }
    };
    bool _IsEmpty() const override { return false; }
    DbResult _Append(Byte const* pData, int nData) override { return BE_SQLITE_ERROR; }
    RefCountedPtr<Changes::Reader> _GetReader() const override { return new Reader(*this); }
    ConflictResolution _OnConflict(ConflictCause clause, Changes::Change iter) override {
        BeAssert(false);
        return ConflictResolution::Abort;
    }
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
RevisionStatus RevisionManager::WriteChangesToFile(BeFileNameCR pathname, DdlChangesCR ddlChanges, ChangeGroupCR dataChangeGroup, Rebaser* rebaser) {
    RevisionChangesFileWriter writer(pathname, dataChangeGroup.ContainsEcSchemaChanges(), ddlChanges, m_dgndb);

    DbResult result = writer.Initialize();
    if (BE_SQLITE_OK != result)
        return RevisionStatus::FileWriteError;

    if (nullptr == rebaser) {
        result = writer.FromChangeGroup(dataChangeGroup);
    } else {
        DbResult rebaseResult = BE_SQLITE_OK;

        folly::ProducerConsumerQueue<bvector<uint8_t>> pageQueue{5};

        ChangeStreamQueueConsumer readFromQueue(pageQueue);
        std::thread writerThread([&] { rebaseResult = rebaser->DoRebase(readFromQueue, writer); });

        ChangeStreamQueueProducer writeToQueue(pageQueue);
        result = writeToQueue.FromChangeGroup(dataChangeGroup);

        while (!pageQueue.write(bvector<uint8_t>())) // write an empty page to tell the consumer that we are done.
            ;
        writerThread.join();

        if (BE_SQLITE_OK == result)
            result = rebaseResult;
    }

    if (BE_SQLITE_OK != result) {
        BeAssert(false && "Could not write data changes to the revision file");
        return RevisionStatus::FileWriteError;
    }

    return pathname.DoesPathExist() ? RevisionStatus::Success : RevisionStatus::NoTransactions;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BeFileName RevisionManager::BuildTempRevisionPathname()
    {
    BeFileName tempPathname;
    BentleyStatus status = T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectory(tempPathname, CHANGESET_REL_DIR);
    UNUSED_VARIABLE(status);
    BeAssert(SUCCESS == status && "Cannot get temporary directory");
    tempPathname.AppendToPath(WPrintfString(L"%d", BeThreadUtilities::GetCurrentProcessId()).c_str());
    BeFileName::CreateNewDirectory(tempPathname.c_str());
    tempPathname.AppendToPath(L"CurrentChangeSet");
    tempPathname.AppendExtension(CHANGESET_FILE_EXT);
    return tempPathname;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnRevisionPtr RevisionManager::CreateRevision(RevisionStatus* outStatus, TxnManager::TxnId endTxnId, int64_t lastRebaseId)
    {
    RevisionStatus ALLOW_NULL_OUTPUT(status, outStatus);

    DdlChanges ddlChanges;
    ChangeGroup dataChangeGroup;
    status = GroupChanges(ddlChanges, dataChangeGroup, endTxnId);
    if (RevisionStatus::Success != status)
        return nullptr;

    Rebaser rebaser;
    if (lastRebaseId != 0)
        if (m_dgndb.Txns().LoadRebases(rebaser, lastRebaseId) != BE_SQLITE_OK)
            return nullptr;

    status = WriteChangesToFile(m_tempRevisionPathname, ddlChanges, dataChangeGroup, (lastRebaseId != 0)? &rebaser: nullptr);
    if (RevisionStatus::Success != status)
        return nullptr;

    return CreateRevisionObject(m_tempRevisionPathname);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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

    txnMgr.DeleteReversedTxns(); // make sure there's nothing undone before we create a new revision

    if (txnMgr.HasChanges())
        {
        BeAssert(false && "There are unsaved changes in the current transaction. Call db.SaveChanges() or db.AbandonChanges() first");
        status = RevisionStatus::HasUncommittedChanges;
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

    if (!txnMgr.HasPendingTxns()) {
        status = RevisionStatus::NoTransactions;
        return nullptr;
    }

    TxnManager::TxnId endTxnId = txnMgr.GetCurrentTxnId();
    int64_t lastRebaseId = txnMgr.QueryLastRebaseId();

    DgnRevisionPtr currentRevision = CreateRevision(outStatus, endTxnId, lastRebaseId);
    if (!currentRevision.IsValid())
        return nullptr;

    status = SaveCurrentRevisionEndTxnId(endTxnId, lastRebaseId);
    if (RevisionStatus::Success != status)
        return nullptr;

    m_currentRevision = currentRevision;
    return m_currentRevision;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
RevisionStatus RevisionManager::FinishCreateRevision(int32_t changesetIndex) {
    DgnRevisionPtr currentRevision = GetCreatingRevision();
    if (!currentRevision.IsValid()) {
        BeAssert(false && "No revision is currently being created");
        return RevisionStatus::IsNotCreatingRevision;
    }

    // currently, the C++ IModelHub api doesn't have the changesetIndex to pass here, but they put it into the current revision, use it instead of the argument.
    // Ultimately, this method should ONLY be called from typescript where it can properly supply the index.
    if (changesetIndex <= 0)
        changesetIndex = currentRevision->GetChangesetIndex();

    TxnManager::TxnId endTxnId = QueryCurrentRevisionEndTxnId();
    BeAssert(endTxnId.IsValid());

    m_dgndb.Txns().DeleteFromStartTo(endTxnId);
    m_dgndb.Txns().DeleteRebases(QueryLastRebaseId());
    currentRevision->SetChangesetIndex(changesetIndex);

    RevisionStatus status = SaveParentRevision(currentRevision->GetChangesetId(), changesetIndex);
    if (RevisionStatus::Success != status)
        return status;

    DeleteCurrentRevisionEndTxnId();
    m_currentRevision = nullptr;

    return RevisionStatus::Success;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void RevisionManager::AbandonCreateRevision() {
    BeAssert(IsCreatingRevision());
    DeleteCurrentRevisionEndTxnId();
    m_currentRevision = nullptr;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
RevisionStatus RevisionManager::ReverseRevision(DgnRevisionCR revision) {
    PRECONDITION(!m_dgndb.IsReadonly() && "Cannot reverse changes in a Readonly database", RevisionStatus::CannotMergeIntoReadonly);
    if (revision.ContainsSchemaChanges(m_dgndb)) {
        BeAssert(false && "Cannot reverse a revision containing schema changes.");
        return RevisionStatus::ReverseOrReinstateSchemaChanges;
    }

    TxnManagerR txnMgr = m_dgndb.Txns();

    if (txnMgr.HasLocalChanges()) {
        BeAssert(false && "Cannot reverse revisions if there are local changes.");
        return RevisionStatus::HasLocalChanges;
    }

    if (IsCreatingRevision()) {
        BeAssert(false && "Cannot reverse revisions when one's being created. Call AbandonCreateRevision() or FinishCreateRevision() first");
        return RevisionStatus::IsCreatingRevision;
    }

    RevisionStatus status = revision.Validate(m_dgndb);
    if (RevisionStatus::Success != status)
        return status;

    if (GetParentRevisionId() != revision.GetChangesetId()) {
        BeAssert(false && "Parent of revision should match current parent revision id");
        return RevisionStatus::ParentMismatch;
    }

#ifdef DUMP_REVISION
    revision.Dump(m_dgndb);
#endif

    return txnMgr.ReverseRevision(revision);
}

#if defined (WORK_ON_CHANGE_MERGING)
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static DgnElementId getElementId(DgnDbCR db, Utf8CP tableName, BeSQLite::Changes::Change change, BeSQLite::DbOpcode opcode)
    {
    if (0 != strncmp("bis_", tableName, 4))
        return DgnElementId();

    auto stage = (BeSQLite::DbOpcode::Insert == opcode)? BeSQLite::Changes::Change::Stage::New: BeSQLite::Changes::Change::Stage::Old;

    if (0 == strcmp(BIS_TABLE(BIS_CLASS_Element), tableName))
        return DgnElementId(change.GetValue(0, stage).GetValueUInt64());

    if (0 == strcmp(BIS_TABLE(BIS_CLASS_ElementMultiAspect), tableName) || 0 == strcmp(BIS_TABLE(BIS_CLASS_ElementUniqueAspect), tableName))
        {
        auto stmt = db.GetCachedStatement(Utf8PrintfString("SELECT ElementId from %s WHERE (Id = ?)", tableName).c_str());
        stmt->BindInt64(1, change.GetValue(0, stage).GetValueUInt64());
        stmt->Step();
        return stmt->GetValueId<DgnElementId>(0);
        }

    // This may be a non-element-related table. We don't report it.
    // This may be a split table, in which case, we'll always see and get the ElementId from the triggered change to the corresponding
    //  row in the bis_Element table.
    return DgnElementId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BeSQLite::ChangeSet::ConflictResolution OptimisticConcurrencyControl::HandleConflict(OnConflict onConflict, DgnDbCR db,
    Utf8CP tableName, BeSQLite::Changes::Change change, BeSQLite::DbOpcode opcode, bool indirect)
    {
    if (indirect)
        return BeSQLite::ChangeSet::ConflictResolution::Replace;

    BeSQLite::ChangeSet::ConflictResolution res;
    bset<DgnElementId>* eset;

    if (OptimisticConcurrencyControl::OnConflict::RejectIncomingChange == onConflict)
        {
        res = BeSQLite::ChangeSet::ConflictResolution::Skip;
        eset = &m_conflictingElementsRejected;
        }
    else
        {
        res = BeSQLite::ChangeSet::ConflictResolution::Replace;
        eset = &m_conflictingElementsAccepted;
        }

    if (!indirect)
        {
        DgnElementId elementId = getElementId(db, tableName, change, opcode);
        if (elementId.IsValid())
            eset->insert(elementId);
        }

    return res;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BeSQLite::ChangeSet::ConflictResolution OptimisticConcurrencyControl::_OnConflict(DgnDbCR db, BeSQLite::ChangeSet::ConflictCause cause, BeSQLite::Changes::Change change)
    {
    Utf8CP tableName = nullptr;
    int nCols, indirect;
    BeSQLite::DbOpcode opcode;
    change.GetOperation(&tableName, &nCols, &opcode, &indirect);

    BeAssert((nCols > 0) && "first col must be ID - we don't track tables with no ID");

    OptimisticConcurrencyControl::OnConflict onConflict;

    switch (cause)
        {
        case BeSQLite::ChangeSet::ConflictCause::Data:
            // SQLITE_CHANGESET_DATA
            //  The conflict handler is invoked with CHANGESET_DATA as the second argument when processing a DELETE or UPDATE change if a row with the required PRIMARY KEY fields is present in the database, but one or more other (non primary-key) fields modified by the update do not contain the expected "before" values.
            //    opcode=DELETE   => updateVsDelete
            //    opcode=UPDATE   => updateVsUpdate
            if (BeSQLite::DbOpcode::Delete == opcode)
                onConflict = m_policy.updateVsDelete;
            else
                {
                BeAssert(BeSQLite::DbOpcode::Update == opcode);
                onConflict = m_policy.updateVsUpdate;
                }
            break;

        case BeSQLite::ChangeSet::ConflictCause::NotFound:
            // SQLITE_CHANGESET_NOTFOUND
            // The conflict handler is invoked with CHANGESET_NOTFOUND as the second argument when processing a DELETE or UPDATE change if a row with the required PRIMARY KEY fields is not present in the database.
            //    opcode=DELETE   => (deleteVsDelete) => nop
            //    opcode=UPDATE   => deleteVsUpdate
            if (BeSQLite::DbOpcode::Delete == opcode)
                {
/*<==*/         return BeSQLite::ChangeSet::ConflictResolution::Skip;       // NOP!
                }

            BeAssert(BeSQLite::DbOpcode::Update == opcode);
            onConflict = m_policy.deleteVsUpdate;
            break;

        case BeSQLite::ChangeSet::ConflictCause::Constraint:
            // *** TBD: SQLITE_CHANGESET_CONSTRAINT
            // If any other constraint violation occurs while applying a change (i.e. a UNIQUE, CHECK or NOT NULL constraint), the conflict handler is invoked with CHANGESET_CONSTRAINT as the second argument.
            //    opcode=DELETE   => attempt to delete a row that is referenced as a FK by some existing row  ==> We have to reject the incoming delete!
            //    opcode=INSERT   => new row violates a UNIQUE constraint                                     ==> We have to reject the incoming insert! << Can only happen on unique properties other than codes. What properties are unique???
            //    opcode=UPDATE   => some column value violates a UNIQUE or NOT NULL constraint               ==> We have to reject the incoming change! <<             "                 "                       "
            LOG.warning("Constraint Conflict should never happen");
            onConflict = OptimisticConcurrencyControl::OnConflict::RejectIncomingChange;
            break;

        case BeSQLite::ChangeSet::ConflictCause::Conflict:
        case BeSQLite::ChangeSet::ConflictCause::ForeignKey:

            // *** SQLITE_CHANGESET_CONFLICT (CHANGESET_CONFLICT is passed as the second argument to the conflict handler while processing an INSERT change if the operation would result in duplicate primary key values.)
            // *** Should never happen, since we use briefcase-based ids, etc.

            // *** TBD: SQLITE_CHANGESET_FOREIGN_KEY
            // If foreign key handling is enabled, and applying a changeset leaves the database in a state containing foreign key violations, the conflict handler is invoked with CHANGESET_FOREIGN_KEY as the second argument exactly once before the changeset is committed. If the conflict handler returns CHANGESET_OMIT, the changes, including those that caused the foreign key constraint violation, are committed. Or, if it returns CHANGESET_ABORT, the changeset is rolled back.
            // This is a special status that is passed to the callback just before the changeset is committed. It applies to the changeset as a whole.
            // If the conflict handler returns CHANGESET_OMIT, the changes, including those that caused the foreign key constraint violation, are committed. Or, if it returns CHANGESET_ABORT, the changeset is rolled back.
            //    ==> This should never happen, since we will reject incoming changes that would cause constraint violations, as described above.

            BeAssert(false && "Conflict cause Conflict and ForeignKey should never happen");
            LOG.warning("NotFound Conflict should never happen");
            onConflict = OptimisticConcurrencyControl::OnConflict::AcceptIncomingChange;
            break;

        default:
            BeAssert(false && "Unexpected conflict cause");
            onConflict = OptimisticConcurrencyControl::OnConflict::AcceptIncomingChange;
            break;
        }

    return HandleConflict(onConflict, db, tableName, change, opcode, indirect != 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
OptimisticConcurrencyControl::OptimisticConcurrencyControl(Policy policy)
    {
    m_policy = policy;
    }
#endif

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
RevisionStatus RevisionManager::ProcessRevisions(bvector<DgnRevisionCP> const &revisions, RevisionProcessOption processOptions) {
    RevisionStatus status;
    switch (processOptions) {
    case RevisionProcessOption::Merge:
        for (DgnRevisionCP revision : revisions) {
            status = MergeRevision(*revision);
            if (RevisionStatus::Success != status)
                return status;
        }
        break;
    case RevisionProcessOption::Reverse:
        for (DgnRevisionCP revision : revisions) {
            status = ReverseRevision(*revision);
            if (RevisionStatus::Success != status)
                return status;
        }
        break;
    default:
        BeAssert(false && "Invalid revision process option");
    }

    return RevisionStatus::Success;
}

END_BENTLEY_DGNPLATFORM_NAMESPACE
