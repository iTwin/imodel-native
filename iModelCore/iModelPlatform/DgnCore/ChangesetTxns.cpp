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

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ChangeSet::ConflictResolution ChangesetFileReader::_OnConflict(ChangeSet::ConflictCause cause, Changes::Change iter) {
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
        if (!m_dgndb.Txns().HasPendingTxns()) {
            // This changeset is bad. However, it is already in the timeline. We must allow services such as
            // checkpoint-creation, change history, and other apps to apply any changeset that is in the timeline.
            LOG.info("PRIMARY KEY INSERT CONFLICT - resolved by replacing the existing row with the incoming row");
            iter.Dump(m_dgndb, false, 1);
        } else {
            LOG.fatal("PRIMARY KEY INSERT CONFLICT - rejecting this changeset");
            iter.Dump(m_dgndb, false, 1);
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
        if (m_dgndb.QueryBriefcaseLocalValue(notUsed, "DebugAllowFkViolations") == BE_SQLITE_ROW) {
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
        return control->_OnConflict(m_dgndb, cause, iter);
    }
#endif

    if (ChangeSet::ConflictCause::Constraint == cause) {
        if (LOG.isSeverityEnabled(NativeLogging::LOG_INFO)) {
            LOG.infov("------------------------------------------------------------------");
            LOG.infov("Conflict detected - Cause: %s", ChangeSet::InterpretConflictCause(cause, 1));
            iter.Dump(m_dgndb, false, 1);
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
        iter.Dump(m_dgndb, false, 1);
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
    bool _IsEmpty() const override final { return false; }
    RefCountedPtr<Changes::Reader> _GetReader() const override { return nullptr; }

    //---------------------------------------------------------------------------------------
    // @bsimethod
    //---------------------------------------------------------------------------------------
    static Utf8String GenerateId(Utf8StringCR parentRevId, BeFileNameCR changesetFile, DgnDbR dgndb) {
        ChangesetFileReader fs(changesetFile, dgndb);

        ChangesetIdGenerator idGen;
        idGen.AddStringToHash(parentRevId);

        auto reader = fs.MakeReader();
        DbResult result;
        Utf8StringCR prefix = reader->GetPrefix(result);
        if (BE_SQLITE_OK != result)
            dgndb.ThrowException(result == BE_SQLITE_ERROR_InvalidChangeSetVersion ? "invalid changeset version" : "corrupted changeset header", result);

        if (!prefix.empty())
            idGen._Append((Byte const*)prefix.c_str(), (int)prefix.SizeInBytes());

        result = idGen.ReadFrom(*reader);
        if (BE_SQLITE_OK != result)
            dgndb.ThrowException("corrupted changeset", result);

        return idGen.m_hash.GetHashString();
    }
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ChangesetProps::Dump(DgnDbR dgndb) const {
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

/**
 * validate the content of a changeset props to ensure it is from the right iModel and its checksum is valid
 */
void ChangesetProps::ValidateContent(DgnDbR dgndb) const {
    if (m_dbGuid != dgndb.GetDbGuid().ToString())
        dgndb.ThrowException("changeset did not originate from this iModel", (int) ChangesetStatus::WrongDgnDb);

    if (!m_fileName.DoesPathExist())
        dgndb.ThrowException("changeset file does not exist", (int) ChangesetStatus::FileNotFound);

    if (m_id != ChangesetIdGenerator::GenerateId(m_parentId, m_fileName, dgndb))
      dgndb.ThrowException("incorrect id for changeset", (int) ChangesetStatus::CorruptedChangeStream);
}

/**
 * determine whether the Changeset has schema changes.
 */
bool ChangesetProps::ContainsSchemaChanges(DgnDbR dgndb) const {
    ChangesetFileReader changeStream(m_fileName, dgndb);

    bool containsSchemaChanges;
    DdlChanges ddlChanges;
    DbResult result = changeStream.MakeReader()->GetSchemaChanges(containsSchemaChanges, ddlChanges);
    if (BE_SQLITE_OK != result)
        dgndb.ThrowException("error reading changeset data", result);

    return containsSchemaChanges;
}

/**
 * clear the iModel-specific briefcase local values for changesets. Called when an iModel's guid changes.
 */
void TxnManager::ClearSavedChangesetValues() {
    m_dgndb.DeleteBriefcaseLocalValue(PARENT_CHANGESET);
    m_dgndb.DeleteBriefcaseLocalValue(PARENT_CS_ID);

    // these are just cruft from old versions.
    m_dgndb.DeleteBriefcaseLocalValue(CURRENT_CS_END_TXN_ID);
    m_dgndb.DeleteBriefcaseLocalValue(LAST_REBASE_ID);
    m_dgndb.DeleteBriefcaseLocalValue("ReversedChangeSetId");
}

/**
 * save the parent-changeset-id in the briefcase local values table. This enables us to tell the most recently applied changes for a briefcase.
 */
void TxnManager::SaveParentChangeset(Utf8StringCR revisionId, int32_t changesetIndex) {
    if (revisionId.length() != SHA1::HashBytes * 2)
        m_dgndb.ThrowException("invalid changesetId", (int)ChangesetStatus::BadVersionId);

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

/**
 * get the parent-changeset-id from the briefcase local values table. This identifies the most recently applied changeset for a briefcase.
 */
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

//=======================================================================================
//! Handles a request to stream output by writing to a ProducerConsumerQueue. Can be used as the producer side of a concurrent pipeline.
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

/**
 * Write the changes from:
 *  1) the DdlChanges
 *  2) data changes
 *  3) rebase information
 *
 * into a file about to become a changeset file.
 */
void TxnManager::WriteChangesToFile(BeFileNameCR pathname, DdlChangesCR ddlChanges, ChangeGroupCR dataChangeGroup, Rebaser* rebaser) {
    ChangesetFileWriter writer(pathname, dataChangeGroup.ContainsEcSchemaChanges(), ddlChanges, m_dgndb);

    if (BE_SQLITE_OK !=  writer.Initialize())
        m_dgndb.ThrowException("unable to initialize change writer", (int) ChangesetStatus::FileWriteError);

    if (nullptr == rebaser) {
        if (BE_SQLITE_OK != writer.FromChangeGroup(dataChangeGroup))
            m_dgndb.ThrowException("unable to save changes to file", (int) ChangesetStatus::FileWriteError);
    } else {
        DbResult rebaseResult = BE_SQLITE_OK;

        folly::ProducerConsumerQueue<bvector<uint8_t>> pageQueue{5};

        ChangeStreamQueueConsumer readFromQueue(pageQueue);
        std::thread writerThread([&] { rebaseResult = rebaser->DoRebase(readFromQueue, writer); });

        ChangeStreamQueueProducer writeToQueue(pageQueue);
        DbResult result = writeToQueue.FromChangeGroup(dataChangeGroup);

        while (!pageQueue.write(bvector<uint8_t>())) // write an empty page to tell the consumer that we are done.
            ;
        writerThread.join();

        if (BE_SQLITE_OK != result || BE_SQLITE_OK != rebaseResult)
            m_dgndb.ThrowException("unable to save changes with rebase", (int) ChangesetStatus::FileWriteError);
    }

    if (!pathname.DoesPathExist())
        m_dgndb.ThrowException("changeset file not created", (int) ChangesetStatus::FileWriteError);
}

/**
 * Iterate over local txn and notify changed instance keys and change type.
 * rootClassFilter is optional but if provided iterator will only iterate over
 * changes if it to one of root class or any of its derived class.
 *
 * Note: For large set of changes this function may consume a lot of none contigious memory.
 */
void TxnManager::ForEachLocalChange(std::function<void(ECInstanceKey const&, DbOpcode)> cb, bvector<Utf8String> const& rootClassFilter, bool includeInMemoryChanges) {
    DbResult rc;

    includeInMemoryChanges = includeInMemoryChanges && HasDataChanges();
    if (!HasPendingTxns() && !includeInMemoryChanges) {
        return;
    }

    // Expand root class filter into all possible derived classes including root class
    bset<ECClassId> allowedClasses;
    if (!rootClassFilter.empty()) {
        bvector<Utf8String> classIdFilter;
        for(auto& qualifiedName : rootClassFilter) {
            if (auto classP = m_dgndb.Schemas().FindClass(qualifiedName)) {
                classIdFilter.push_back(classP->GetId().ToHexStr());
            } else {
                m_dgndb.ThrowException(SqlPrintfString("unknown class '%s' provided as filter", qualifiedName.c_str()).GetUtf8CP(), (int) BE_SQLITE_ERROR);
            }
        }
        if (!classIdFilter.empty()) {
            auto filterStmt = m_dgndb.GetCachedStatement(SqlPrintfString("SELECT [ClassId] FROM [ec_cache_ClassHierarchy] WHERE [BaseClassId] IN (%s) GROUP BY [ClassId]", BeStringUtilities::Join(classIdFilter, ",").c_str()).GetUtf8CP());
            while(filterStmt->Step() == BE_SQLITE_ROW) {
                allowedClasses.insert(filterStmt->GetValueId<ECClassId>(0));
            }
        }
    }

    // Group all local txn
    auto endTxnId = GetCurrentTxnId();
    auto startTxnId = QueryNextTxnId(TxnId(0));
    DdlChanges ddlChanges;
    ChangeGroup dataChangeGroup;
    for (auto currTxnId = startTxnId; currTxnId < endTxnId; currTxnId = QueryNextTxnId(currTxnId)) {
        ChangeSet sqlChangeSet;
        if (BE_SQLITE_OK != ReadDataChanges(sqlChangeSet, currTxnId, TxnAction::None))
            m_dgndb.ThrowException("unable to read data changes", (int) ChangesetStatus::CorruptedTxn);

        rc = sqlChangeSet.AddToChangeGroup(dataChangeGroup);
        if (BE_SQLITE_OK != rc)
            m_dgndb.ThrowException("add to changes failed", (int) rc);
    }

    // Include change that has not been persisted using Db::SaveChanges()
    if (includeInMemoryChanges && HasDataChanges()) {
        ChangeSet inMemChangeSet;
        rc = inMemChangeSet.FromChangeTrack(*this);
        if (BE_SQLITE_OK != rc)
            m_dgndb.ThrowException("fail to add in memory changes", (int) rc);

        inMemChangeSet.AddToChangeGroup(dataChangeGroup);
        if (BE_SQLITE_OK != rc)
            m_dgndb.ThrowException("add to changes failed", (int) rc);
    }

    // Create changeset from change group
    ChangeSet cs;
    rc = cs.FromChangeGroup(dataChangeGroup);
    if (BE_SQLITE_OK != rc)
        m_dgndb.ThrowException("fail to create changeset form change group", (int) rc);

    // Optimization based on fact that in changeset change to same table is contigious
    struct {
        Utf8String zTab;
        int iPk = -1;
        int iClassId = -1;
        ECClassId rootClassId;
        Statement classIdStmt;
    } current;

    // Set of key we already notified va callback
    bset<ECInstanceKey> instanceKeysAlreadyNotified;

    // Iterate over changeset and notify each change if it meet filter criteria
    for(auto& change : cs.GetChanges()) {
        Utf8CP zTab;
        int nCol;
        DbOpcode op;

        rc = change.GetOperation(&zTab, &nCol, &op, nullptr);
        if (BE_SQLITE_OK != rc) {
            m_dgndb.ThrowException("failed to read changeset", (int) rc);
        }

        // if new table then we need to figure out pk and classid column including root classid for table
        if (0 != strcmp(current.zTab.c_str(), zTab)) {
            current.zTab = zTab;
            auto stmt = m_dgndb.GetCachedStatement(R"x(
                SELECT
                    (SELECT [cid] FROM PRAGMA_TABLE_INFO (?1) WHERE [pk] = 1),
                    (SELECT [cid] FROM PRAGMA_TABLE_INFO (?1) WHERE [name] = 'ECClassId'),
                    (SELECT [ExclusiveRootClassId] FROM [ec_Table] WHERE [Name] = 'bis_Element')
            )x");

            stmt->BindText(1, zTab, Statement::MakeCopy::No);
            rc = stmt->Step();
            if (BE_SQLITE_ROW != rc)
                m_dgndb.ThrowException("failed to read changeset", (int) rc);

            // Column index of ECInstanceId column in table.
            current.iPk = stmt->IsColumnNull(0) ? -1 : stmt->GetValueInt(0);

            // Column index of ECClassId column in table if it has one.
            current.iClassId = stmt->IsColumnNull(1) ? -1 : stmt->GetValueInt(1);

            // ExclusiveRootClassId for table which is used when ECClassId column does not exist in table
            current.rootClassId = stmt->IsColumnNull(2) ? ECClassId() : stmt->GetValueId<ECClassId>(2);

            if (current.classIdStmt.IsPrepared()) {
                current.classIdStmt.Finalize();
            }
            rc = current.classIdStmt.Prepare(m_dgndb, SqlPrintfString("SELECT [ECClassId] FROM [%s] WHERE ROWID=?", zTab).GetUtf8CP());
            if (BE_SQLITE_OK != rc)
                m_dgndb.ThrowException("failed to read changeset", (int) rc);
        }

        if (current.iPk < 0) {
            m_dgndb.ThrowException("failed to read local changes", (int) BE_SQLITE_ERROR);
        }

        ECInstanceId id;
        ECClassId classId = current.rootClassId;

        // read primary key or ECInstanceId
        DbValue valId = op == DbOpcode::Insert ? change.GetNewValue(current.iPk) : change.GetOldValue(current.iPk);
        if (!valId.IsValid())
            m_dgndb.ThrowException("failed to read local changes", (int) BE_SQLITE_ERROR);
        id = ECInstanceId(valId.GetValueUInt64());

        // It is possible ECClassId column may not exist in a table and in case we fall back to ExclusiveRootClassId for that table.
        if (current.iClassId != -1) {
            DbValue valClassId = op == DbOpcode::Insert ? change.GetNewValue(current.iClassId) : change.GetOldValue(current.iClassId);
            if (valClassId.IsValid()) {
                classId = ECClassId(valClassId.GetValueUInt64());
            } else {
                if (op == DbOpcode::Update) {
                    current.classIdStmt.Reset();
                    current.classIdStmt.ClearBindings();
                    current.classIdStmt.BindId(1, id);
                    if (current.classIdStmt.Step() == BE_SQLITE_ROW) {
                        classId = current.classIdStmt.GetValueId<ECClassId>(0);
                    }
                } else {
                    m_dgndb.ThrowException("failed to read local changes", (int) BE_SQLITE_ERROR);
                }
            }
        }

        // If root class filter was provided make sure current classid is part of
        // filter class or one of its derived class else skip to next change
        if (!allowedClasses.empty()) {
            if (allowedClasses.find(classId) == allowedClasses.end()) {
                continue;
            }
        }

        auto key = ECInstanceKey(classId, id);

        // Skip if we already seen this instance key e.g. first as bis_Element and later in bis_GeometricElement3d
        if (instanceKeysAlreadyNotified.find(key) != instanceKeysAlreadyNotified.end())
            continue;

        cb(key, op);
        instanceKeysAlreadyNotified.insert(key);
    }
}

/**
 * Create a new changeset file from all of the Txns in this briefcase. After this function returns, the
 * changeset is "in-progress" and its file must be uploaded to iModelHub. After that succeeds, the Txns included
 * in the changeset are deleted in the call to `FinishCreateChangeset`.
 *
 * The changeset file name is the "tempFileNameBase" of the briefcase with ".changeset" appended. For tests, this method
 * takes a "extension" argument that is appended to the filename before ".changeset" is added. That's so tests
 * can save more than one changeset while they work (without mocking iModelHub.)
 */
ChangesetPropsPtr TxnManager::StartCreateChangeset(Utf8CP extension) {
    if (m_changesetInProgress.IsValid())
        m_dgndb.ThrowException("a changeset is currently in progress", (int) ChangesetStatus::IsCreatingChangeset);

    if (!IsTracking())
        m_dgndb.ThrowException("change tracking not enabled", (int) ChangesetStatus::ChangeTrackingNotEnabled);

    if (HasChanges())
        m_dgndb.ThrowException("local uncommitted changes present", (int) ChangesetStatus::HasUncommittedChanges);

    // make sure there's nothing undone before we start to create a changeset
    DeleteReversedTxns();

    if (!HasPendingTxns())
        m_dgndb.ThrowException("no changes are present", (int) ChangesetStatus::NoTransactions);

    TxnManager::TxnId endTxnId = GetCurrentTxnId();
    TxnId startTxnId = QueryNextTxnId(TxnId(0));
    int64_t lastRebaseId = QueryLastRebaseId();

    DdlChanges ddlChanges;
    ChangeGroup dataChangeGroup;
    for (TxnId currTxnId = startTxnId; currTxnId < endTxnId; currTxnId = QueryNextTxnId(currTxnId)) {
        auto txnType = GetTxnType(currTxnId);
        if (txnType == TxnType::EcSchema) // if we have EcSchema changes, set the flag on the change group
            dataChangeGroup.SetContainsEcSchemaChanges();

        if (txnType == TxnType::Ddl) {
            BeAssert(ddlChanges._IsEmpty());
            if (ZIP_SUCCESS != ReadChanges(ddlChanges, currTxnId))
                m_dgndb.ThrowException("unable to read schema changes", (int) ChangesetStatus::CorruptedTxn);
        } else {
            ChangeSet sqlChangeSet;
            if (BE_SQLITE_OK != ReadDataChanges(sqlChangeSet, currTxnId, TxnAction::None))
                m_dgndb.ThrowException("unable to read data changes", (int) ChangesetStatus::CorruptedTxn);

            DbResult result = sqlChangeSet.AddToChangeGroup(dataChangeGroup);
            if (BE_SQLITE_OK != result)
                m_dgndb.ThrowException("add to changes failed", (int) result);
        }
    }

    Rebaser rebaser;
    if (lastRebaseId != 0 && (BE_SQLITE_OK != LoadRebases(rebaser, lastRebaseId)))
        m_dgndb.ThrowException("rebase failed", (int) ChangesetStatus::SQLiteError);

    BeFileName changesetFileName((m_dgndb.GetTempFileBaseName() + (extension ? extension : "") +  ".changeset").c_str());
    WriteChangesToFile(changesetFileName, ddlChanges, dataChangeGroup, (lastRebaseId != 0) ? &rebaser : nullptr);

    auto parentRevId = GetParentChangesetId();
    auto revId = ChangesetIdGenerator::GenerateId(parentRevId, changesetFileName, m_dgndb);
    auto dbGuid = m_dgndb.GetDbGuid().ToString();

    m_changesetInProgress = new ChangesetProps(revId, -1, parentRevId, dbGuid, changesetFileName);
    m_changesetInProgress->m_endTxnId = endTxnId;
    m_changesetInProgress->m_lastRebaseId = lastRebaseId;

    // clean this cruft up from older versions.
    m_dgndb.DeleteBriefcaseLocalValue(CURRENT_CS_END_TXN_ID);
    m_dgndb.DeleteBriefcaseLocalValue(LAST_REBASE_ID);

    auto rc = m_dgndb.SaveChanges();
    if (BE_SQLITE_OK != rc)
        m_dgndb.ThrowException("cannot save changes to start changeset", rc);

    // We've just saved all the local Txns together into a changeset file that can now be pushed to iModelHub. Only after that succeeds we can delete
    // the Txns from this changeset. In the meantime, we can allow further changes to this briefcase that will be included in the *next* changeset.
    // However, we cannot allow the changes we've just included in the changeset to be undone. Therefore, we start a new "session" which
    // makes the current Txns unreachable for undo/redo.
    StartNewSession();
    BeAssert(!IsUndoPossible());
    BeAssert(!IsRedoPossible());

    return m_changesetInProgress;
}

/**
 * This method is called after an in-progress changeset file, created in this session, was successfully uploaded to iModelHub. We can now delete
 * all of the Txns up to the last one included in the changeset.
 * Note: If the upload fails for any reason, or we crashed before it succeeds, a new one must be recreated from the current state of the briefcase.
 */
void TxnManager::FinishCreateChangeset(int32_t changesetIndex, bool keepFile) {
    if (!IsChangesetInProgress())
        m_dgndb.ThrowException("no changeset in progress", (int) ChangesetStatus::IsNotCreatingChangeset);

    TxnId endTxnId = m_changesetInProgress->m_endTxnId;
    if (!endTxnId.IsValid())
        m_dgndb.ThrowException("changeset in progress is not valid", (int) ChangesetStatus::IsNotCreatingChangeset);

    m_dgndb.Txns().DeleteFromStartTo(endTxnId);
    if (0 !=m_changesetInProgress->m_lastRebaseId)
        m_dgndb.Txns().DeleteRebases(m_changesetInProgress->m_lastRebaseId);
    m_changesetInProgress->SetChangesetIndex(changesetIndex);

    SaveParentChangeset(m_changesetInProgress->GetChangesetId(), changesetIndex);

    auto rc = m_dgndb.SaveChanges();
    if (BE_SQLITE_OK != rc)
        m_dgndb.ThrowException("cannot save changes to complete changeset", rc);

    // if there were no *new* changes during the process of uploading the changeset (which is very likely), we can
    // restart the session id back to 0. Otherwise just leave it alone so undo/redo of those changes is still possible.
    if (!HasPendingTxns())
        Initialize();

    m_changesetInProgress->SetDateTime(DateTime::GetCurrentTimeUtc());
    StopCreateChangeset(keepFile);
}

/**
 * free the in-progress ChangesetProps, if present, and optionally delete its changeset file.
 */
void TxnManager::StopCreateChangeset(bool keepFile) {
    if (!keepFile && m_changesetInProgress.IsValid() && m_changesetInProgress->m_fileName.DoesPathExist())
        m_changesetInProgress->m_fileName.BeDeleteFile();

    m_changesetInProgress = nullptr;
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
