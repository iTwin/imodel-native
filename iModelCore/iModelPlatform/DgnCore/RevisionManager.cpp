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

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ChangeSet::ConflictResolution ChangesetFileReader::_OnConflict(ChangeSet::ConflictCause cause, Changes::Change iter) {
    DgnDbCR dgndb = *(DgnDbCP) &GetDb();
    TxnManagerCR txns = ((DgnDbP) &dgndb)->Txns();
    Utf8CP tableName = nullptr;
    int nCols, indirect;
    DbOpcode opcode;
    DbResult result = iter.GetOperation(&tableName, &nCols, &opcode, &indirect);
    BeAssert(result == BE_SQLITE_OK);

    UNUSED_VARIABLE(result);

    // Handle some special cases
    if (cause == ChangeSet::ConflictCause::Conflict) {
        // From the SQLite docs: "CHANGESET_CONFLICT is passed as the second argument to the conflict handler while processing an INSERT change if the operation would result in duplicate primary key values."
        // This is always a fatal error - it can happen only if the app started with a briefcase that is behind the tip and then uses the same primary key values (e.g., ElementIds)
        // that have already been used by some other app using the SAME briefcase ID that recently pushed changes. That can happen only if the app makes changes without first pulling and acquiring locks.
        if (!txns.HasPendingTxns()) {
            // This changeset is bad. However, it is already in the timeline. We must allow services such as
            // checkpoint-creation, change history, and other apps to apply any changeset that is in the timeline.
            LOG.info("PRIMARY KEY INSERT CONFLICT - resolved by replacing the existing row with the incoming row");
            iter.Dump(dgndb, false, 1);
        } else {
            LOG.fatal("PRIMARY KEY INSERT CONFLICT - rejecting this changeset");
            iter.Dump(dgndb, false, 1);
            return ChangeSet::ConflictResolution::Abort;
        }
    }

    if (cause == ChangeSet::ConflictCause::ForeignKey) {
        // Note: No current or conflicting row information is provided if it's a FKey conflict
        // Since we abort on FKey conflicts, always try and provide details about the error
        int nConflicts = 0;
        result = iter.GetFKeyConflicts(&nConflicts);
        BeAssert(result == BE_SQLITE_OK);

        uint64_t notUsed;
        // Note: There is no performance implication of follow code as it happen toward end of
        // apply_changeset only once so we be querying value for 'DebugAllowFkViolations' only once.
        if (dgndb.QueryBriefcaseLocalValue(notUsed, "DebugAllowFkViolations") == BE_SQLITE_ROW) {
            LOG.errorv("Detected %d foreign key conflicts in changeset. Continuing merge as 'DebugAllowFkViolations' flag is set. Run 'PRAGMA foreign_key_check' to get list of violations.", nConflicts);
            return ChangeSet::ConflictResolution::Skip;
        } else {
            LOG.errorv("Detected %d foreign key conflicts in ChangeSet. Aborting merge.", nConflicts);
            return ChangeSet::ConflictResolution::Abort;
        }
    }

    if (cause == ChangeSet::ConflictCause::NotFound) {
        /*
         * Note: If ConflictCause = NotFound, the primary key was not found, and returning ConflictResolution::Replace is
         * not an option at all - this will cause a BE_SQLITE_MISUSE error.
         */
        if (opcode == DbOpcode::Delete) {
            // Caused by CASCADE DELETE on a foreign key, and is usually not a problem.
            return ChangeSet::ConflictResolution::Skip;
        }

        if (opcode == DbOpcode::Update && 0 == ::strncmp(tableName, "ec_", 3)) {
            // Caused by a ON DELETE SET NULL constraint on a foreign key - this is known to happen with "ec_" tables, but needs investigation if it happens otherwise
            return ChangeSet::ConflictResolution::Skip;
        }

#if defined(WORK_ON_CHANGE_MERGING)
        if (!letControlHandleThis)
#endif
        {
            // Refer to comment below
            return opcode == DbOpcode::Update ? ChangeSet::ConflictResolution::Skip : ChangeSet::ConflictResolution::Replace;
        }
    }

#if defined(WORK_ON_CHANGE_MERGING)
    if (letControlHandleThis) {
        // if we have a concurrency control, then we allow it to decide how to handle conflicts with local changes.
        // (We don't call the control in the case where there are no local changes. As explained below, we always want the incoming changes in that case.)
        return control->_OnConflict(dgndb, cause, iter);
    }
#endif

    if (ChangeSet::ConflictCause::Constraint == cause) {
        if (LOG.isSeverityEnabled(NativeLogging::LOG_INFO)) {
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
     * + Also see comments in TxnManager::MergeDataChanges()
     */
    if (LOG.isSeverityEnabled(NativeLogging::LOG_INFO)) {
        LOG.infov("------------------------------------------------------------------");
        LOG.infov("Conflict detected - Cause: %s", ChangeSet::InterpretConflictCause(cause, 1));
        iter.Dump(dgndb, false, 1);
        LOG.infov("Conflicting resolved by replacing the existing entry with the change");
    }
    return ChangeSet::ConflictResolution::Replace;
}

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ChangesetIdGenerator : ChangeStream {
    SHA1 m_hash;

    //---------------------------------------------------------------------------------------
    // @bsimethod
    //---------------------------------------------------------------------------------------
    static int HexCharToInt(char input) {
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
    void AddStringToHash(Utf8StringCR hashString) {
        Byte hashValue[SHA1::HashBytes];
        if (hashString.empty()) {
            memset(hashValue, 0, SHA1::HashBytes);
        } else {
            BeAssert(hashString.length() == SHA1::HashBytes * 2);
            for (int ii = 0; ii < SHA1::HashBytes; ii++) {
                char hexChar1 = hashString.at(2 * ii);
                char hexChar2 = hashString.at(2 * ii + 1);
                hashValue[ii] = (Byte)(16 * HexCharToInt(hexChar1) + HexCharToInt(hexChar2));
            }
        }

        m_hash.Add(hashValue, SHA1::HashBytes);
    }

    //---------------------------------------------------------------------------------------
    // @bsimethod
    //---------------------------------------------------------------------------------------
    DbResult _Append(Byte const* pData, int nData) override {
        m_hash.Add(pData, nData);
        return BE_SQLITE_OK;
    }

    //---------------------------------------------------------------------------------------
    // @bsimethod
    //---------------------------------------------------------------------------------------
    ChangeSet::ConflictResolution _OnConflict(ChangeSet::ConflictCause cause, Changes::Change iter) override {
        return ChangeSet::ConflictResolution::Abort;
    }

public:
    ChangesetIdGenerator() {}
    bool _IsEmpty() const override final { return false; }
    RefCountedPtr<Changes::Reader> _GetReader() const override { return nullptr; }

    //---------------------------------------------------------------------------------------
    // @bsimethod
    //---------------------------------------------------------------------------------------
    static ChangesetStatus GenerateId(Utf8StringR revId, Utf8StringCR parentRevId, BeFileNameCR revisionFile, DgnDbCR dgndb) {
        revId.clear();
        ChangesetFileReader fs(revisionFile, dgndb);

        ChangesetIdGenerator idGen;
        idGen.AddStringToHash(parentRevId);

        auto reader = fs.MakeReader();
        DbResult result;
        Utf8StringCR prefix = reader->GetPrefix(result);
        if (BE_SQLITE_OK != result) {
            BeAssert(false);
            return (result == BE_SQLITE_ERROR_InvalidChangeSetVersion) ? ChangesetStatus::InvalidVersion : ChangesetStatus::CorruptedChangeStream;
        }

        if (!prefix.empty())
            idGen._Append((Byte const*)prefix.c_str(), (int)prefix.SizeInBytes());

        result = idGen.ReadFrom(*reader);
        if (BE_SQLITE_OK != result) {
            BeAssert(false);
            return ChangesetStatus::CorruptedChangeStream;
        }

        revId = idGen.m_hash.GetHashString();
        return ChangesetStatus::Success;
    }
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ChangesetProps::Dump(DgnDbCR dgndb) const {
// Don't log "sensitive" information in production builds.
#if !defined(NDEBUG)
    LOG.infov("Id : %s", m_id.c_str());
    LOG.infov("ParentId : %s", m_parentId.c_str());
    LOG.infov("DbGuid: %s", m_dbGuid.c_str());
    LOG.infov("User Name: %s", m_userName.c_str());
    LOG.infov("Summary: %s", m_summary.c_str());
    LOG.infov("File: %ls", m_fileName.GetNameUtf8().c_str());
    LOG.infov("DateTime: %s", m_dateTime.ToString().c_str());

    ChangesetFileReader reader(m_fileName, dgndb);

    bool containsSchemaChanges;
    DdlChanges ddlChanges;
    DbResult result = reader.MakeReader()->GetSchemaChanges(containsSchemaChanges, ddlChanges);
    UNUSED_VARIABLE(result);
    BeAssert(result == BE_SQLITE_OK);

    LOG.infov("Contains Schema Changes: %s", containsSchemaChanges ? "yes" : "no");
    LOG.infov("Contains Ddl Changes: %s", (ddlChanges.GetSize() > 0) ? "yes" : "no");
    if (ddlChanges.GetSize() > 0)
        ddlChanges.Dump("DDL: ");

    reader.Dump("ChangeSet:\n", dgndb, false, 0);
#endif
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ChangesetProps::ValidateContent(DgnDbCR dgndb) const {
    if (m_id.length() != SHA1::HashBytes * 2)
        dgndb.ThrowException("ChangesetId is not a valid SHA1 hash", (int) ChangesetStatus::InvalidId);

    Utf8String dbGuid = dgndb.GetDbGuid().ToString();
    if (m_dbGuid != dbGuid)
        dgndb.ThrowException("changeset did not originate from this iModel", (int) ChangesetStatus::WrongDgnDb);

    if (!m_fileName.DoesPathExist())
        dgndb.ThrowException("changeset file does not exist", (int) ChangesetStatus::FileNotFound);

    Utf8String id;
    ChangesetStatus status = ChangesetIdGenerator::GenerateId(id, m_parentId, m_fileName, dgndb);

    if (status != ChangesetStatus::Success)
        dgndb.ThrowException("corrupt changeset file", (int) status);

    if (m_id != id)
      dgndb.ThrowException("incorrect hash in changeset", (int) ChangesetStatus::CorruptedChangeStream);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool ChangesetProps::ContainsSchemaChanges(DgnDbCR dgndb) const {
    ChangesetFileReader changeStream(m_fileName, dgndb);

    bool containsSchemaChanges;
    DdlChanges ddlChanges;
    DbResult result = changeStream.MakeReader()->GetSchemaChanges(containsSchemaChanges, ddlChanges);
    if (BE_SQLITE_OK != result) {
        BeAssert(false);
        return false;
    }

    return containsSchemaChanges;
}

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ClearSavedChangesetValues() {
    m_dgndb.DeleteBriefcaseLocalValue(CURRENT_CS_END_TXN_ID);
    m_dgndb.DeleteBriefcaseLocalValue(LAST_REBASE_ID);
    m_dgndb.DeleteBriefcaseLocalValue(PARENT_CS_ID);
    m_dgndb.DeleteBriefcaseLocalValue("ReversedChangeSetId"); // clean up from old versions
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void TxnManager::SaveParentChangeset(Utf8StringCR revisionId, int32_t changesetIndex) {
    if (revisionId.length() != SHA1::HashBytes * 2)
        m_dgndb.ThrowException("invalid changesetId", (int) ChangesetStatus::BadVersionId);

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
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String TxnManager::GetParentChangesetId() const {
    Utf8String revisionId;
    DbResult result = m_dgndb.QueryBriefcaseLocalValue(revisionId, PARENT_CS_ID);
    return (BE_SQLITE_ROW == result) ? revisionId : "";
}

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::GetParentChangesetIndex(int32_t& index, Utf8StringR id) const {
    id = GetParentChangesetId();
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
ChangesetStatus TxnManager::CombineTxns(DdlChangesR ddlChanges, ChangeGroupR dataChangeGroup, TxnId endTxnId) {
    TxnId startTxnId = QueryNextTxnId(TxnId(0));
    if (!startTxnId.IsValid() || startTxnId >= endTxnId)
        return ChangesetStatus::NoTransactions;

    for (TxnId currTxnId = startTxnId; currTxnId < endTxnId; currTxnId = QueryNextTxnId(currTxnId)) {
        auto txnType = GetTxnType(currTxnId);

        if (txnType == TxnType::EcSchema) // if we have EcSchema changes, set the flag on the changegroup
            dataChangeGroup.SetContainsEcSchemaChanges();

        if (txnType == TxnType::Ddl) {
            BeAssert(ddlChanges._IsEmpty());
            if (ReadChanges(ddlChanges, currTxnId) != ZIP_SUCCESS) {
                LOG.error(L"Unable to read schema changes - ChangesetStatus::CorruptedTxn");
                return ChangesetStatus::CorruptedTxn;
            }
        } else {
            ChangeSet sqlChangeSet;
            if (ReadDataChanges(sqlChangeSet, currTxnId, TxnAction::None) != BE_SQLITE_OK) {
                LOG.error(L"Unable to read data changes - ChangesetStatus::CorruptedTxn");
                return ChangesetStatus::CorruptedTxn;
            }

            DbResult result = sqlChangeSet.AddToChangeGroup(dataChangeGroup);
            if (result != BE_SQLITE_OK) {
                LOG.errorv(L"sqlite3changegroup_add failed: %ls", WString(BeSQLiteLib::GetErrorName(result), BentleyCharEncoding::Utf8).c_str());
                return ChangesetStatus::SQLiteError;
            }
        }
    }
    return ChangesetStatus::Success;
}

//=======================================================================================
//! Handles a request to stream output by writing to a ProducerConsumerQueue. Can be used as the producer side of a concurrent pipeline.
// @bsiclass
//=======================================================================================
struct ChangeStreamQueueProducer : ChangeStream {
    folly::ProducerConsumerQueue<bvector<uint8_t>>& m_q;
    ChangeStreamQueueProducer(folly::ProducerConsumerQueue<bvector<uint8_t>>& q) : m_q(q) {}
    DbResult _Append(Byte const* pData, int nData) override {
        while (!m_q.write(pData, pData + nData))
            ; // spin until the queue has room
        return BE_SQLITE_OK;
    }
    ConflictResolution _OnConflict(ConflictCause clause, Changes::Change iter) override {
        BeAssert(false);
        return ConflictResolution::Abort;
    }

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
ChangesetStatus TxnManager::WriteChangesToFile(BeFileNameCR pathname, DdlChangesCR ddlChanges, ChangeGroupCR dataChangeGroup, Rebaser* rebaser) {
    ChangesetFileWriter writer(pathname, dataChangeGroup.ContainsEcSchemaChanges(), ddlChanges, m_dgndb);

    DbResult result = writer.Initialize();
    if (BE_SQLITE_OK != result)
        return ChangesetStatus::FileWriteError;

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
        return ChangesetStatus::FileWriteError;
    }

    return pathname.DoesPathExist() ? ChangesetStatus::Success : ChangesetStatus::NoTransactions;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ChangesetPropsPtr TxnManager::StartCreateChangeset(ChangesetStatus* outStatus) {
    ChangesetStatus ALLOW_NULL_OUTPUT(status, outStatus);

    if (m_changesetInProgress.IsValid()) {
        status = ChangesetStatus::IsCreatingRevision;
        return nullptr;
    }

    TxnManagerR txnMgr = m_dgndb.Txns();
    if (!txnMgr.IsTracking()) {
        BeAssert(false && "Creating revisions requires that change tracking is enabled");
        status = ChangesetStatus::ChangeTrackingNotEnabled;
        return nullptr;
    }

    txnMgr.DeleteReversedTxns(); // make sure there's nothing undone before we create a new revision

    if (txnMgr.HasChanges()) {
        BeAssert(false && "There are unsaved changes in the current transaction. Call db.SaveChanges() or db.AbandonChanges() first");
        status = ChangesetStatus::HasUncommittedChanges;
        return nullptr;
    }

    if (!txnMgr.HasPendingTxns()) {
        status = ChangesetStatus::NoTransactions;
        return nullptr;
    }

    TxnManager::TxnId endTxnId = txnMgr.GetCurrentTxnId();
    int64_t lastRebaseId = txnMgr.QueryLastRebaseId();

    txnMgr.StartNewSession();
    BeAssert(!txnMgr.IsUndoPossible());
    BeAssert(!txnMgr.IsRedoPossible());

    DdlChanges ddlChanges;
    ChangeGroup dataChangeGroup;
    status = txnMgr.CombineTxns(ddlChanges, dataChangeGroup, endTxnId);
    if (ChangesetStatus::Success != status)
        return nullptr;

    Rebaser rebaser;
    if (lastRebaseId != 0) {
        if (txnMgr.LoadRebases(rebaser, lastRebaseId) != BE_SQLITE_OK)
            return nullptr;
    }

    BeFileName changesetFileName((m_dgndb.GetTempFileBaseName() + "-changeset").c_str());

    status = WriteChangesToFile(changesetFileName, ddlChanges, dataChangeGroup, (lastRebaseId != 0) ? &rebaser : nullptr);
    if (ChangesetStatus::Success != status)
        return nullptr;

    Utf8String parentRevId = GetParentChangesetId();
    Utf8String revId;
    status = ChangesetIdGenerator::GenerateId(revId, parentRevId, changesetFileName, m_dgndb);
    BeAssert(status == ChangesetStatus::Success);
    Utf8String dbGuid = m_dgndb.GetDbGuid().ToString();

    m_changesetInProgress = new ChangesetProps(revId, -1, parentRevId, dbGuid, changesetFileName);
    m_changesetInProgress->m_endTxnId = endTxnId;
    m_changesetInProgress->m_lastRebaseId = lastRebaseId;

    m_dgndb.DeleteBriefcaseLocalValue(CURRENT_CS_END_TXN_ID);
    m_dgndb.DeleteBriefcaseLocalValue(LAST_REBASE_ID);

    return m_changesetInProgress;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ChangesetStatus TxnManager::FinishCreateChangeset(int32_t changesetIndex) {
    if (!IsChangesetInProgress())
        return ChangesetStatus::IsNotCreatingChangeset;

    TxnId endTxnId = m_changesetInProgress->m_endTxnId;
    BeAssert(endTxnId.IsValid());

    m_dgndb.Txns().DeleteFromStartTo(endTxnId);
    if (0 !=m_changesetInProgress->m_lastRebaseId)
        m_dgndb.Txns().DeleteRebases(m_changesetInProgress->m_lastRebaseId);
    m_changesetInProgress->SetChangesetIndex(changesetIndex);

    SaveParentChangeset(m_changesetInProgress->GetChangesetId(), changesetIndex);
    m_changesetInProgress = nullptr;
    return ChangesetStatus::Success;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void TxnManager::AbandonCreateChangeset() {
    m_changesetInProgress = nullptr;
}

void TxnManager::ThrowIfChangesetInProgress() {
    if (IsChangesetInProgress())
        m_dgndb.ThrowException("changeset creation is in progress", (int) ChangesetStatus::IsCreatingRevision);
}

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
ChangesetStatus TxnManager::ProcessRevisions(bvector<ChangesetPropsCP> const &revisions, RevisionProcessOption processOptions) {
    ChangesetStatus status;
    switch (processOptions) {
    case RevisionProcessOption::Merge:
        for (ChangesetPropsCP revision : revisions) {
            status = MergeChangeset(*revision);
            if (ChangesetStatus::Success != status)
                return status;
        }
        break;
    case RevisionProcessOption::Reverse:
        for (ChangesetPropsCP revision : revisions) {
            ReverseChangeset(*revision);
        }
        break;
    default:
        BeAssert(false && "Invalid revision process option");
    }

    return ChangesetStatus::Success;
}

END_BENTLEY_DGNPLATFORM_NAMESPACE
