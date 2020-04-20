/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <Bentley/SHA1.h>
#include <DgnPlatform/DgnChangeSummary.h>
#include <ECDb/ChangeIterator.h>
#include <folly/ProducerConsumerQueue.h>
#include <thread>

USING_NAMESPACE_BENTLEY_SQLITE

#define CURRENT_CS_END_TXN_ID   "CurrentChangeSetEndTxnId"
#define LAST_REBASE_ID          "LastRebaseId"
#define INITIAL_PARENT_CS_ID    "InitialParentChangeSetId"
#define PARENT_CS_ID           "ParentChangeSetId"
#define REVERSED_CS_ID         "ReversedChangeSetId"
#define CONTAINS_SCHEMA_CHANGES "ContainsSchemaChanges"
#define CHANGESET_REL_DIR L"DgnDbChangeSets"
#define CHANGESET_FILE_EXT L"cs"

// #define DEBUG_REVISION_KEEP_FILES 1
// #define DUMP_REVISION 1

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2017
//---------------------------------------------------------------------------------------
ChangeSet::ConflictResolution RevisionChangesFileReader::_OnConflict(ChangeSet::ConflictCause cause, Changes::Change iter)
    {
    DgnDb const* dgndb = dynamic_cast<DgnDb const*>(&GetDb());
    return RevisionManager::ConflictHandler(*dgndb, cause, iter);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2017
//---------------------------------------------------------------------------------------
ChangeSet::ConflictResolution ApplyRevisionChangeSet::_OnConflict(ChangeSet::ConflictCause cause, Changes::Change iter)
    {
    return RevisionManager::ConflictHandler(m_dgndb, cause, iter);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
// static
ChangeSet::ConflictResolution RevisionManager::ConflictHandler(DgnDbCR dgndb, ChangeSet::ConflictCause cause, Changes::Change iter)
    {
    Utf8CP tableName = nullptr;
    int nCols, indirect;
    DbOpcode opcode;
    DbResult result = iter.GetOperation(&tableName, &nCols, &opcode, &indirect);
    BeAssert(result == BE_SQLITE_OK);

    UNUSED_VARIABLE(result);

    auto control = dgndb.GetOptimisticConcurrencyControl();
    bool letControlHandleThis = (nullptr != control) && (const_cast<DgnDbR>(dgndb).Txns().HasLocalChanges());

    if (LOG.isSeverityEnabled(NativeLogging::LOG_INFO))
        {
        LOG.infov("------------------------------------------------------------------");
        LOG.infov("Conflict detected - Cause: %s", ChangeSet::InterpretConflictCause(cause, 1));
        if (cause != ChangeSet::ConflictCause::ForeignKey)
            {
            iter.Dump(dgndb, false, 1);
            }
        }

    // Handle some special cases
    if (cause == ChangeSet::ConflictCause::ForeignKey)
        {
        // Note: No current or conflicting row information is provided if it's a FKey conflict
        // Since we abort on FKey conflicts, always try and provide details about the error
        int nConflicts = 0;
        result = iter.GetFKeyConflicts(&nConflicts);
        BeAssert(result == BE_SQLITE_OK);
        LOG.errorv("Detected %d foreign key conflicts in ChangeSet. Aborting merge.", nConflicts);
        return ChangeSet::ConflictResolution::Abort;
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
            LOG.infov("Conflict resolved by keeping the existing entry and skipping the change");
            return ChangeSet::ConflictResolution::Skip;
            }


        if (opcode == DbOpcode::Update && 0 == ::strncmp(tableName, "ec_", 3))
            {
            // Caused by a ON DELETE SET NULL constraint on a foreign key - this is known to happen with "ec_" tables, but needs investigation if it happens otherwise
            LOG.infov("Conflict resolved by keeping the existing entry and skipping the change");
            return ChangeSet::ConflictResolution::Skip;
            }

        if (!letControlHandleThis)
            {
            // Refer to comment below
            return opcode == DbOpcode::Update ? ChangeSet::ConflictResolution::Skip : ChangeSet::ConflictResolution::Replace;
            }
        }

    if (letControlHandleThis)
        {
        // if we have a concurrency control, then we allow it to decide how to handle conflicts with local changes.
        // (We don't call the control in the case where there are no local changes. As explained below, we always want the incoming changes in that case.)
        return control->_OnConflict(dgndb, cause, iter);
        }

    if (ChangeSet::ConflictCause::Constraint == cause)
        {
        LOG.warning("Constraint conflict handled by rejecting incoming change. Constraint conflicts are NOT expected. These happen most often when two clients both insert elements with the same code. That indicates a bug in the client or the code server.");
        iter.Dump(dgndb, false, 1);
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
     * + Also see comments in TxnManager::MergeDataChangesInRevision()
     */
    LOG.infov("Conflicting resolved by replacing the existing entry with the change");
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
    static RevisionStatus GenerateId(Utf8StringR revId, Utf8StringCR parentRevId, BeFileNameCR revisionFile, DgnDbCR dgndb)
        {
        revId.clear();
        RevisionChangesFileReader fs(revisionFile, dgndb);

        DgnRevisionIdGenerator idgen;
        idgen.AddStringToHash(parentRevId);

        DbResult result;
        Utf8StringCR prefix = fs.GetPrefix(result);
        if (BE_SQLITE_OK != result)
            {
            BeAssert(false);
            return (result == BE_SQLITE_ERROR_InvalidRevisionVersion) ? RevisionStatus::InvalidVersion : RevisionStatus::CorruptedChangeStream;
            }

        if (!prefix.empty())
            idgen._OutputPage(prefix.c_str(), (int) prefix.SizeInBytes());

        result = idgen.FromChangeStream(fs);
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
    revision->m_revChangesFile = changeStreamPathname;
    revision->m_ownsRevChangesFile = true;
    status = RevisionStatus::Success;
    return revision;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
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
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
// static
BeFileName DgnRevision::BuildRevisionChangesPathname(Utf8String revisionId)
    {
    BeFileName tempPathname;
    BentleyStatus status = T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectory(tempPathname, CHANGESET_REL_DIR);
    BeAssert(SUCCESS == status && "Cannot get temporary directory");
    tempPathname.AppendToPath(WPrintfString(L"%d", BeThreadUtilities::GetCurrentProcessId()).c_str());
    BeFileName::CreateNewDirectory(tempPathname.c_str());
    tempPathname.AppendToPath(WString(revisionId.c_str(), true).c_str());
    tempPathname.AppendExtension(CHANGESET_FILE_EXT);
    return tempPathname;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
void DgnRevision::Dump(DgnDbCR dgndb) const
    {
// Don't log "sensitive" information in production builds.
#if !defined(PRG)
    LOG.infov("Id : %s", m_id.c_str());
    LOG.infov("ParentId : %s", m_parentId.c_str());
    LOG.infov("Initial ParentId : %s", m_initialParentId.c_str());
    LOG.infov("DbGuid: %s", m_dbGuid.c_str());
    LOG.infov("User Name: %s", m_userName.c_str());
    LOG.infov("Summary: %s", m_summary.c_str());
    LOG.infov("ChangeStreamFile: %ls", m_revChangesFile.c_str());
    LOG.infov("DateTime: %s", m_dateTime.ToString().c_str());

    RevisionChangesFileReader fs(m_revChangesFile, dgndb);

    bool containsSchemaChanges;
    DbSchemaChangeSet dbSchemaChangeSet;
    DbResult result = fs.GetSchemaChanges(containsSchemaChanges, dbSchemaChangeSet);
    BeAssert(result == BE_SQLITE_OK);

    LOG.infov("Contains Schema Changes: %s", containsSchemaChanges ? "yes" : "no");
    LOG.infov("Contains DbSchema Changes: %s", (dbSchemaChangeSet.GetSize() > 0) ? "yes" : "no");
    if (dbSchemaChangeSet.GetSize() > 0)
        dbSchemaChangeSet.Dump("DDL: ");

    fs.Dump("ChangeSet:\n", dgndb, false, 0);
#endif
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

    Utf8String id;
    RevisionStatus status = DgnRevisionIdGenerator::GenerateId(id, m_parentId, m_revChangesFile, dgndb);
    if (status != RevisionStatus::Success)
        return status;

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
static DgnCode GetCodeFromChangeOrDb(DgnDbCR db, ChangeIterator::ColumnIterator const& columnIter, Changes::Change::Stage stage)
    {
    DbDupValue codeSpecId = GetValueFromChangeOrDb(columnIter, "CodeSpec.Id", stage);
    DbDupValue scopeElementId = GetValueFromChangeOrDb(columnIter, "CodeScope.Id", stage);
    DbDupValue value = GetValueFromChangeOrDb(columnIter, "CodeValue", stage);

    CodeSpecCPtr codeSpec = db.CodeSpecs().GetCodeSpec(codeSpecId.GetValueId<CodeSpecId>());
    if (!codeSpec.IsValid())
        return DgnCode();

    // Filter code specs for internal-used-only codes (e.g. GeometryParts)
    bset<CodeSpecId> filteredCodeSpecIds = db.GetExistingBriefcaseManager()->GetFilteredCodeSpecIds();
    if (filteredCodeSpecIds.find(codeSpec->GetCodeSpecId()) != filteredCodeSpecIds.end())
        return DgnCode();

    DgnElementCPtr scopeElement = db.Elements().GetElement(scopeElementId.GetValueId<DgnElementId>());
    if (!scopeElement.IsValid())
        return DgnCode();

    return codeSpec->CreateCode(*scopeElement, value.GetValueText());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Paul.Connelly   12/15
//---------------------------------------------------------------------------------------
static void insertCode(DgnCodeSet& into, DgnCodeCR code, DgnCodeSet& ifNotIn)
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
// @bsimethod                                Sam.Wilson                         07/2019
//---------------------------------------------------------------------------------------
static void detectLockedModels(DgnModelIdSet& exclusiveModelIds, DgnDbCR dgndb)
    {
    IBriefcaseManager* mgr = dgndb.GetExistingBriefcaseManager();
    IOwnedLocksIteratorPtr locksIter = (mgr != nullptr) ? mgr->GetOwnedLocks() : nullptr;
    while(locksIter.IsValid() && locksIter->IsValid())
        {
        DgnLockCR lock = **locksIter;
        if (lock.GetType() == LockableType::Model && lock.GetLevel() == LockLevel::Exclusive)
            {
            DgnModelId modelId(lock.GetId().GetValue());
            exclusiveModelIds.insert(modelId);
            }

        ++(*locksIter);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Sam.Wilson                         07/2019
//---------------------------------------------------------------------------------------
static void getModelIdFromIter(DgnModelId& modelId, DgnModelId& oldModelId, ChangeIterator::RowEntry const& entry, ChangeIterator::ColumnIterator columnIter)
    {
    switch (entry.GetDbOpcode())
        {
        case DbOpcode::Insert:  modelId = GetModelIdFromChangeOrDb(columnIter, Changes::Change::Stage::New); break;
        case DbOpcode::Delete:  modelId = GetModelIdFromChangeOrDb(columnIter, Changes::Change::Stage::Old); break;
        case DbOpcode::Update:
            {
            modelId = GetModelIdFromChangeOrDb(columnIter, Changes::Change::Stage::New);
            oldModelId = GetModelIdFromChangeOrDb(columnIter, Changes::Change::Stage::Old);
            break;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Sam.Wilson                         07/2019
//---------------------------------------------------------------------------------------
static DgnModelId getCurrentModelIdFromIter(ChangeIterator::RowEntry const& entry, ChangeIterator::ColumnIterator columnIter)
    {
    DgnModelId modelId, oldModelId;
    getModelIdFromIter(modelId, oldModelId, entry, columnIter);
    return modelId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2016
//---------------------------------------------------------------------------------------
void DgnRevision::ExtractCodes(DgnCodeSet& assignedCodes, DgnCodeSet& discardedCodes, DgnDbCR dgndb) const
    {
    ECClassCP elemClass = dgndb.Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_CLASS_Element);
    BeAssert(elemClass != nullptr);

    auto bcMgr = dgndb.GetExistingBriefcaseManager();
    DgnModelIdSet exclusiveModelIds;
    bool ignoreCodesInLockedModels = ((bcMgr != nullptr) && !bcMgr->ShouldReportCodesInLockedModels());
    if (ignoreCodesInLockedModels)
        detectLockedModels(exclusiveModelIds, dgndb);

    RevisionChangesFileReader revisionReader(m_revChangesFile, dgndb);
    ChangeIterator changeIter(dgndb, revisionReader);

    assignedCodes.clear();
    discardedCodes.clear();

    for (ChangeIterator::RowEntry const& entry : changeIter)
        {
        if (!entry.IsMapped())
            continue;

        ECClassCP primaryClass = entry.GetPrimaryClass();
        BeAssert(primaryClass != nullptr);

        if (!entry.IsPrimaryTable() || !primaryClass->Is(elemClass))
            continue;

        DbOpcode dbOpcode = entry.GetDbOpcode();
        ChangeIterator::ColumnIterator columnIter = entry.MakeColumnIterator(*primaryClass); // Note: ColumnIterator needs to be in the stack to access column

        if (ignoreCodesInLockedModels)
            {
            auto modelId = getCurrentModelIdFromIter(entry, columnIter);
            if (exclusiveModelIds.find(modelId) != exclusiveModelIds.end())
                continue;
            if (bcMgr->GetNormalChannelParentOf(modelId, nullptr).IsValid())    // Feature #160330: if element is in normal channel, then its model should be *regarded* as locked for Code-scoping purposes
                continue;
            }

        DgnCode oldCode = (dbOpcode == DbOpcode::Insert) ? DgnCode() : GetCodeFromChangeOrDb(dgndb, columnIter, Changes::Change::Stage::Old);
        DgnCode newCode = (dbOpcode == DbOpcode::Delete) ? DgnCode() : GetCodeFromChangeOrDb(dgndb, columnIter, Changes::Change::Stage::New);

        if (oldCode == newCode)
            continue;

        insertCode(discardedCodes, oldCode, assignedCodes);
        insertCode(assignedCodes, newCode, discardedCodes);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2016
//---------------------------------------------------------------------------------------
void DgnRevision::ExtractLocks(DgnLockSet& usedLocks, DgnDbCR dgndb, bool extractInserted, bool avoidExclusiveModelElements) const
    {
    if (dgndb.GetExistingBriefcaseManager() && !dgndb.GetExistingBriefcaseManager()->ShouldIncludeUsedLocksInChangeSet())
        {
        usedLocks.clear();
        return;
        }

    LockRequest lockRequest;

    if (ContainsSchemaChanges(dgndb))
        {
        // The briefcase *must* hold the Schemas lock.
        LockableId schemasLock(dgndb.Schemas());
        if (dgndb.GetExistingBriefcaseManager()->QueryLockLevel(schemasLock) != LockLevel::Exclusive)
            {
            BeAssert(false && "If a revision contains schema changes, then the briefcase must hold the Schemas lock");
            LOG.fatalv("Briefcase has made schema changes but does not hold the Schemas lock");
            }
        lockRequest.InsertLock(schemasLock, LockLevel::Exclusive);
        }

    ECClassCP elemClass = dgndb.Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_CLASS_Element);
    BeAssert(elemClass != nullptr);
    ECClassCP modelClass = dgndb.Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_CLASS_Model);
    BeAssert(modelClass != nullptr);
    ECClassCP codeSpecClass = dgndb.Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_CLASS_CodeSpec);
    BeAssert(codeSpecClass != nullptr);

    RevisionChangesFileReader changeStream(m_revChangesFile, dgndb);
    ChangeIterator changeIter(dgndb, changeStream);

    // TFS#788401: Get the set of exclusively locked model Ids, true by default
    DgnModelIdSet exclusiveModelIds;
    if (avoidExclusiveModelElements)
        {
        detectLockedModels(exclusiveModelIds, dgndb);
        for (auto modelId: exclusiveModelIds)
            lockRequest.InsertLock(LockableId(modelId), LockLevel::Exclusive);
        }

    for (ChangeIterator::RowEntry const& entry : changeIter)
        {
        if (!entry.IsMapped()|| entry.GetIndirect() || !entry.IsPrimaryTable())
            continue;

        ECClassCP primaryClass = entry.GetPrimaryClass();
        BeAssert(primaryClass != nullptr);

        // Check for indirect changes cause by a ElementDrivesElement relationship
        if (!primaryClass->Is(elemClass))
            continue;

        ChangeIterator::ColumnIterator columnIter = entry.MakeColumnIterator(*primaryClass); // Note: ColumnIterator needs to be in the stack to access column

        DgnModelId modelId, oldModelId;
        getModelIdFromIter(modelId, oldModelId, entry, columnIter);

        if ((entry.GetDbOpcode() == DbOpcode::Update) && (oldModelId != modelId))
            lockRequest.InsertLock(LockableId(oldModelId), LockLevel::Shared);

        // TFS#788401: Avoid inserting locks if exclusively locked models
        if (exclusiveModelIds.Contains(modelId))
            continue;

        BeAssert(modelId.IsValid());
        lockRequest.InsertLock(LockableId(modelId), LockLevel::Shared);
        // TFS#788401: We don't want to extract locks for inserted elements, as once it is pushed to the server
        if (extractInserted || entry.GetDbOpcode() != DbOpcode::Insert)
            lockRequest.InsertLock(LockableId(DgnElementId(entry.GetPrimaryInstanceId().GetValueUnchecked())), LockLevel::Exclusive);
        }

    // Any models or CodeSpecs directly changed?
    for (ChangeIterator::RowEntry const& entry : changeIter)
        {
        if (!entry.IsMapped() || entry.GetIndirect() || !entry.IsPrimaryTable())
            continue;

        ECClassCP primaryClass = entry.GetPrimaryClass();
        BeAssert(primaryClass != nullptr);

        // TFS#788401: We don't want to extract locks for inserted models unless we set extractInserted to true
        if (primaryClass->Is(modelClass) && (extractInserted || entry.GetDbOpcode() != DbOpcode::Insert))
            {
            // Remove "Shared" if it exists
            LockableId modelLockId(LockableType::Model, DgnModelId(entry.GetPrimaryInstanceId().GetValueUnchecked()));
            if (lockRequest.Contains(modelLockId))
                lockRequest.Remove(modelLockId);
            // Insert "Exclusive"
            lockRequest.InsertLock(modelLockId, LockLevel::Exclusive);
            }
        // If a model is inserted, we don't want to have any locks related to it (same as inserted elements)
        else if (!extractInserted && primaryClass->Is(modelClass) && entry.GetDbOpcode() == DbOpcode::Insert)
            lockRequest.Remove(LockableId(LockableType::Model, DgnModelId(entry.GetPrimaryInstanceId().GetValueUnchecked())));
        else if (primaryClass->Is(codeSpecClass))
            lockRequest.InsertCodeSpecsLock(dgndb);
        }

    // Anything changed at all?
    if (!lockRequest.IsEmpty())
        lockRequest.Insert(dgndb, LockLevel::Shared);

    usedLocks.clear();
    lockRequest.ExtractLockSet(usedLocks);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2017
//---------------------------------------------------------------------------------------
bool DgnRevision::ContainsSchemaChanges(DgnDbCR dgndb) const
    {
    RevisionChangesFileReader changeStream(m_revChangesFile, dgndb);

    bool containsSchemaChanges;
    DbSchemaChangeSet dbSchemaChanges;
    DbResult result = changeStream.GetSchemaChanges(containsSchemaChanges, dbSchemaChanges);
    if (BE_SQLITE_OK != result)
        {
        BeAssert(false);
        return false;
        }

    return containsSchemaChanges; // TODO: Consider caching this flag
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
RevisionManager::RevisionManager(DgnDbR dgndb) : m_dgndb(dgndb)
    {
    m_tempRevisionPathname = BuildTempRevisionPathname();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
RevisionStatus RevisionManager::SaveParentRevisionId(Utf8StringCR revisionId)
    {
    if (revisionId.length() != SHA1::HashBytes * 2)
        return RevisionStatus::BadVersionId;

    DbResult result = m_dgndb.SaveBriefcaseLocalValue(PARENT_CS_ID, revisionId);
    if (BE_SQLITE_DONE != result)
        {
        BeAssert(false);
        return RevisionStatus::SQLiteError;
        }

    return RevisionStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2017
//---------------------------------------------------------------------------------------
RevisionStatus RevisionManager::SaveReversedRevisionId(Utf8StringCR revisionId)
    {
    DbResult result = m_dgndb.SaveBriefcaseLocalValue(REVERSED_CS_ID, revisionId);
    if (BE_SQLITE_DONE != result)
        {
        BeAssert(false);
        return RevisionStatus::SQLiteError;
        }

    return RevisionStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2017
//---------------------------------------------------------------------------------------
RevisionStatus RevisionManager::DeleteReversedRevisionId()
    {
    DbResult result = m_dgndb.DeleteBriefcaseLocalValue(REVERSED_CS_ID);
    if (BE_SQLITE_DONE != result)
        {
        BeAssert(false);
        return RevisionStatus::SQLiteError;
        }

    return RevisionStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2017
//---------------------------------------------------------------------------------------
bool RevisionManager::HasReversedRevisions() const
    {
    Utf8String reversedParentId;
    DbResult result = m_dgndb.QueryBriefcaseLocalValue(reversedParentId, REVERSED_CS_ID);
    return (result == BE_SQLITE_ROW);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
Utf8String RevisionManager::GetParentRevisionId() const
    {
    Utf8String revisionId;
    DbResult result = m_dgndb.QueryBriefcaseLocalValue(revisionId, PARENT_CS_ID);
    return (BE_SQLITE_ROW == result) ? revisionId : "";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2017
//---------------------------------------------------------------------------------------
Utf8String RevisionManager::GetReversedRevisionId() const
    {
    Utf8String revisionId;
    DbResult result = m_dgndb.QueryBriefcaseLocalValue(revisionId, REVERSED_CS_ID);
    return (BE_SQLITE_ROW == result) ? revisionId : "";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    04/2018
//---------------------------------------------------------------------------------------
RevisionStatus RevisionManager::SaveInitialParentRevisionId(Utf8StringCR revisionId)
    {
    if (revisionId.length() != SHA1::HashBytes * 2)
        return RevisionStatus::BadVersionId;

    DbResult result = m_dgndb.SaveBriefcaseLocalValue(INITIAL_PARENT_CS_ID, revisionId);
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
RevisionStatus RevisionManager::UpdateInitialParentRevisionId()
    {
    return SaveInitialParentRevisionId(GetParentRevisionId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2017
//---------------------------------------------------------------------------------------
DbResult RevisionManager::SaveContainsSchemaChanges()
    {
    return m_dgndb.SaveBriefcaseLocalValue(CONTAINS_SCHEMA_CHANGES, 1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2017
//---------------------------------------------------------------------------------------
DbResult RevisionManager::ClearContainsSchemaChanges()
    {
    return m_dgndb.DeleteBriefcaseLocalValue(CONTAINS_SCHEMA_CHANGES);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2017
//---------------------------------------------------------------------------------------
bool RevisionManager::QueryContainsSchemaChanges() const
    {
    uint64_t value;
    return BE_SQLITE_ROW == m_dgndb.QueryBriefcaseLocalValue(value, CONTAINS_SCHEMA_CHANGES);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2016
//---------------------------------------------------------------------------------------
Utf8String RevisionManager::QueryInitialParentRevisionId() const
    {
    Utf8String revisionId;
    DbResult result = m_dgndb.QueryBriefcaseLocalValue(revisionId, INITIAL_PARENT_CS_ID);
    return (BE_SQLITE_ROW == result) ? revisionId : "";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2016
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
// @bsimethod                                Ramanujam.Raman                    11/2016
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
// @bsimethod                                Ramanujam.Raman                    11/2016
//---------------------------------------------------------------------------------------
TxnManager::TxnId RevisionManager::QueryCurrentRevisionEndTxnId() const
    {
    uint64_t val;
    DbResult result = m_dgndb.QueryBriefcaseLocalValue(val, CURRENT_CS_END_TXN_ID);
    return (BE_SQLITE_ROW == result) ? TxnManager::TxnId(val) : TxnManager::TxnId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Sam.Wilson                         03/2018
//---------------------------------------------------------------------------------------
int64_t RevisionManager::QueryLastRebaseId() const
    {
    uint64_t val;
    DbResult result = m_dgndb.QueryBriefcaseLocalValue(val, LAST_REBASE_ID);
    return (BE_SQLITE_ROW == result) ? val : 0;
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

    m_currentRevision = CreateRevision(nullptr, endTxnId, QueryLastRebaseId());

    return m_currentRevision;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2017
//---------------------------------------------------------------------------------------
RevisionStatus RevisionManager::MergeRevision(DgnRevisionCR revision)
    {
    if (revision.ContainsSchemaChanges(m_dgndb))
        {
        BeAssert(false && "Cannot merge a revision containing schema changes when the DgnDb is already open. Close the DgnDb and reopen with the upgrade schema options set to the revision.");
        return RevisionStatus::MergeSchemaChangesOnOpen;
        }

    return DoMergeRevision(revision);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
RevisionStatus RevisionManager::DoMergeRevision(DgnRevisionCR revision)
    {
    PRECONDITION(!m_dgndb.IsReadonly() && m_dgndb.AreTxnsAllowed() &&"Cannot merge changes into this database", RevisionStatus::CannotMergeIntoReadonly);

    TxnManagerR txnMgr = m_dgndb.Txns();

    PRECONDITION(!txnMgr.HasChanges() && "There are unsaved changes in the current transaction. Call db.SaveChanges() or db.AbandonChanges() first", RevisionStatus::HasUncommittedChanges);
    PRECONDITION(!txnMgr.InDynamicTxn() && "Cannot merge revisions if in the middle of a dynamic transaction", RevisionStatus::InDynamicTransaction);
    PRECONDITION(!IsCreatingRevision() && "There is already a revision being created. Call AbandonCreateRevision() or FinishCreateRevision() first", RevisionStatus::IsCreatingRevision)
    PRECONDITION(!HasReversedRevisions() && "Cannot merge changes into a database that has reversed revisions", RevisionStatus::CannotMergeIntoReversed);

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
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
RevisionStatus RevisionManager::GroupChanges(DbSchemaChangeSetR dbSchemaChangeSet, ChangeGroupR dataChangeGroup, TxnManager::TxnId endTxnId) const
    {
    TxnManagerR txnMgr = m_dgndb.Txns();

    TxnManager::TxnId startTxnId = txnMgr.QueryNextTxnId(TxnManager::TxnId(0));
    if (!startTxnId.IsValid() || startTxnId >= endTxnId)
        return RevisionStatus::NoTransactions;

    for (TxnManager::TxnId currTxnId = startTxnId; currTxnId < endTxnId; currTxnId = txnMgr.QueryNextTxnId(currTxnId))
        {
        if (txnMgr.IsSchemaChangeTxn(currTxnId))
            {
            txnMgr.ReadDbSchemaChanges(dbSchemaChangeSet, currTxnId);
            }
        else
            {
            AbortOnConflictChangeSet sqlChangeSet;
            txnMgr.ReadDataChanges(sqlChangeSet, currTxnId, TxnAction::None);

            DbResult result = dataChangeGroup.AddChanges(sqlChangeSet.GetSize(), sqlChangeSet.GetData());
            if (result != BE_SQLITE_OK)
                {
                BeAssert(false && "Failed to group sqlite changesets - see error codes in sqlite3changegroup_add()");
                return RevisionStatus::SQLiteError;
                }
            }
        }

    return RevisionStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
DgnRevisionPtr RevisionManager::CreateRevisionObject(RevisionStatus* outStatus, BeFileNameCR tempRevisionPathname)
    {
    Utf8String parentRevId = GetParentRevisionId();
    Utf8String revId;
    RevisionStatus status = DgnRevisionIdGenerator::GenerateId(revId, parentRevId, tempRevisionPathname, m_dgndb);
    BeAssert(status == RevisionStatus::Success);
    Utf8String dbGuid = m_dgndb.GetDbGuid().ToString();

    DgnRevisionPtr revision = DgnRevision::Create(outStatus, revId, parentRevId, dbGuid);
    if (revision.IsNull())
        return revision;

    LOG.tracev(L"CreateRevisionObject %ls for cs %ls", tempRevisionPathname.c_str(), revision->GetRevisionChangesFile().c_str());

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

    revision->SetInitialParentId(QueryInitialParentRevisionId());
    revision->SetDateTime(DateTime::GetCurrentTimeUtc());

    return revision;
    }

//=======================================================================================
//! Handles a request to stream output by writing to a ProducerConsumerQueue. Can be used as the producer side of a concurrent pipeline.
// @bsiclass                                                 Sam.Wilson     03/2018
//=======================================================================================
struct ChangeStreamQueueProducer : ChangeStream
    {
    folly::ProducerConsumerQueue<bvector<uint8_t>>& m_q;
    ChangeStreamQueueProducer(folly::ProducerConsumerQueue<bvector<uint8_t>>& q) : m_q(q) {}
    DbResult _OutputPage(const void *pData, int nData) override
        {
        while (!m_q.write((uint8_t*)const_cast<void*>(pData), (uint8_t*)const_cast<void*>(pData) + nData))
            ;   // spin until the queue has room
        return BE_SQLITE_OK;
        }
    ConflictResolution _OnConflict(ConflictCause clause, Changes::Change iter) override {BeAssert(false); return ConflictResolution::Abort;}
    };

//=======================================================================================
//! Satisfies a request for input by reading from a ProducerConsumerQueue. Can be used as the consumer side of a concurrent pipeline.
// @bsiclass                                                 Sam.Wilson     03/2018
//=======================================================================================
struct ChangeStreamQueueConsumer : ChangeStream
    {
    folly::ProducerConsumerQueue<bvector<uint8_t>>& m_q;
    bvector<uint8_t> m_remaining;

    ChangeStreamQueueConsumer(folly::ProducerConsumerQueue<bvector<uint8_t>>& q) : m_q(q) {}

    DbResult _InputPage(void *pData, int *pnData) override
        {
        if (!m_remaining.empty())
            {
            // If the queued buffer was bigger than the caller's buffer, then we return
            // the remaining portion in the amounts that the caller can deal with.
            size_t retcount = std::min((size_t)*pnData, m_remaining.size());
            memcpy(pData, m_remaining.data(), retcount);
            *pnData = (int)retcount;
            m_remaining.erase(m_remaining.begin(), m_remaining.begin() + retcount);
            return BE_SQLITE_OK;
            }

        bvector<uint8_t>* pval;
        do  {
            pval = m_q.frontPtr();
            }
        while (!pval); // spin until we get a value;

        if (pval->empty())
            {
            *pnData = 0;
            }
        else
            {
            // return as much of the queued buffer as the caller's buffer can hold.
            size_t retcount = std::min((size_t)*pnData, pval->size());
            memcpy(pData, pval->data(), retcount);
            *pnData = (int)retcount;

            // if the queued buffer has more, save it for the next call
            if (retcount < pval->size())
                m_remaining.assign(pval->data() + retcount, pval->data() + pval->size());
            }

        m_q.popFront();
        return BE_SQLITE_OK;
        }
    ConflictResolution _OnConflict(ConflictCause clause, Changes::Change iter) override {BeAssert(false); return ConflictResolution::Abort;}
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
RevisionStatus RevisionManager::WriteChangesToFile(BeFileNameCR pathname, DbSchemaChangeSetCR dbSchemaChangeSet, ChangeGroupCR dataChangeGroup, Rebaser* rebaser)
    {
    bool containsSchemaChanges = QueryContainsSchemaChanges() || (dbSchemaChangeSet.GetSize() > 0); // Note: Our workflows really disallow DbSchemaChanges without corresponding ECSchema changes, but we allow this here to for testing cases.
    RevisionChangesFileWriter writer(pathname, containsSchemaChanges, dbSchemaChangeSet, m_dgndb);

    DbResult result = writer.Initialize();
    if (BE_SQLITE_OK != result)
        return RevisionStatus::FileWriteError;

    if (nullptr == rebaser)
        {
        result = writer.FromChangeGroup(dataChangeGroup);
        }
    else
        {
        DbResult rebaseResult = BE_SQLITE_OK;

        folly::ProducerConsumerQueue<bvector<uint8_t>> pageQueue{5};

        ChangeStreamQueueConsumer readFromQueue(pageQueue);
        std::thread writerThread([&] {rebaseResult = rebaser->DoRebase(readFromQueue, writer);});

        ChangeStreamQueueProducer writeToQueue(pageQueue);
        result = writeToQueue.FromChangeGroup(dataChangeGroup);

        while (!pageQueue.write(bvector<uint8_t>()))    // write an empty page to tell the consumer that we are done.
	        ;
        writerThread.join();

        if (BE_SQLITE_OK == result)
            result = rebaseResult;
        }

    if (BE_SQLITE_OK != result)
        {
        BeAssert(false && "Could not write data changes to the revision file");
        return RevisionStatus::FileWriteError;
        }

    return pathname.DoesPathExist() ? RevisionStatus::Success : RevisionStatus::NoTransactions;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
// static
BeFileName RevisionManager::BuildTempRevisionPathname()
    {
    BeFileName tempPathname;
    BentleyStatus status = T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectory(tempPathname, CHANGESET_REL_DIR);
    BeAssert(SUCCESS == status && "Cannot get temporary directory");
    tempPathname.AppendToPath(WPrintfString(L"%d", BeThreadUtilities::GetCurrentProcessId()).c_str());
    BeFileName::CreateNewDirectory(tempPathname.c_str());
    tempPathname.AppendToPath(L"CurrentChangeSet").c_str();
    tempPathname.AppendExtension(CHANGESET_FILE_EXT);
    return tempPathname;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2016
//---------------------------------------------------------------------------------------
DgnRevisionPtr RevisionManager::CreateRevision(RevisionStatus* outStatus, TxnManager::TxnId endTxnId, int64_t lastRebaseId)
    {
    RevisionStatus ALLOW_NULL_OUTPUT(status, outStatus);

    DbSchemaChangeSet dbSchemaChangeSet;
    ChangeGroup dataChangeGroup;
    status = GroupChanges(dbSchemaChangeSet, dataChangeGroup, endTxnId);
    if (RevisionStatus::Success != status)
        return nullptr;

    Rebaser rebaser;
    if (lastRebaseId != 0)
        m_dgndb.Txns().LoadRebases(rebaser, lastRebaseId);

    status = WriteChangesToFile(m_tempRevisionPathname, dbSchemaChangeSet, dataChangeGroup, (lastRebaseId != 0)? &rebaser: nullptr);
    if (RevisionStatus::Success != status)
        return nullptr;

    return CreateRevisionObject(outStatus, m_tempRevisionPathname);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2015
//---------------------------------------------------------------------------------------
DgnRevisionPtr RevisionManager::StartCreateRevision(RevisionStatus* outStatus /* = nullptr */)
    {
    RevisionStatus ALLOW_NULL_OUTPUT(status, outStatus);

    LOG.tracev(L"RevisonManager::StartCreateRevision for %ls", m_dgndb.GetFileName().c_str());

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

    m_dgndb.Txns().DeleteFromStartTo(endTxnId);
    m_dgndb.Txns().DeleteRebases(QueryLastRebaseId());

    RevisionStatus status = SaveParentRevisionId(currentRevision->GetId());
    if (RevisionStatus::Success != status)
        return status;

    status = UpdateInitialParentRevisionId();
    if (RevisionStatus::Success != status)
        return status;

    if (QueryContainsSchemaChanges())
        {
        DbResult result = ClearContainsSchemaChanges();
        if (result != BE_SQLITE_DONE)
            {
            BeAssert(false);
            return RevisionStatus::SQLiteError;
            }
        }

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

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    12/2017
//---------------------------------------------------------------------------------------
RevisionStatus RevisionManager::ReverseRevision(DgnRevisionCR revision)
    {
    if (revision.ContainsSchemaChanges(m_dgndb))
        {
        BeAssert(false && "Cannot reverse a revision containing schema changes when the DgnDb is already open. Close the DgnDb and reopen with the upgrade schema options set to the revision.");
        return RevisionStatus::ReverseOrReinstateSchemaChangesOnOpen;
        }

    return DoReverseRevision(revision);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2017
//---------------------------------------------------------------------------------------
RevisionStatus RevisionManager::DoReverseRevision(DgnRevisionCR revision)
    {
    PRECONDITION(!m_dgndb.IsReadonly() && "Cannot reverse changes in a Readonly database", RevisionStatus::CannotMergeIntoReadonly);
    TxnManagerR txnMgr = m_dgndb.Txns();

    if (txnMgr.HasLocalChanges())
        {
        BeAssert(false && "Cannot reverse revisions if there are local changes.");
        return RevisionStatus::HasLocalChanges;
        }

    if (IsCreatingRevision())
        {
        BeAssert(false && "Cannot reverse revisions when one's being created. Call AbandonCreateRevision() or FinishCreateRevision() first");
        return RevisionStatus::IsCreatingRevision;
        }

    RevisionStatus status = revision.Validate(m_dgndb);
    if (RevisionStatus::Success != status)
        return status;

    Utf8String currentParentRevId = HasReversedRevisions() ? GetReversedRevisionId() : GetParentRevisionId();
    if (currentParentRevId != revision.GetId())
        {
        BeAssert(false && "Parent of revision should match the parent revision id of the Db");
        return RevisionStatus::ParentMismatch;
        }

#ifdef DUMP_REVISION
    revision.Dump(m_dgndb);
#endif

    return txnMgr.ApplyRevision(revision, true /*=invert*/);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    12/2017
//---------------------------------------------------------------------------------------
RevisionStatus RevisionManager::ReinstateRevision(DgnRevisionCR revision)
    {
    if (revision.ContainsSchemaChanges(m_dgndb))
        {
        BeAssert(false && "Cannot reverse a revision containing schema changes when the DgnDb is already open. Close the DgnDb and reopen with the upgrade schema options set to the revision.");
        return RevisionStatus::ReverseOrReinstateSchemaChangesOnOpen;
        }

    return DoReinstateRevision(revision);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    02/2017
//---------------------------------------------------------------------------------------
RevisionStatus RevisionManager::DoReinstateRevision(DgnRevisionCR revision)
    {
    TxnManagerR txnMgr = m_dgndb.Txns();
    BeAssert(!IsCreatingRevision());
    if (txnMgr.HasChanges())
        {
        BeAssert(false && "Cannot reinstate revisions if there are local changes. Abandon them first.");
        return RevisionStatus::HasLocalChanges;
        }

    RevisionStatus status = revision.Validate(m_dgndb);
    if (RevisionStatus::Success != status)
        return status;

    Utf8String currentParentRevId = HasReversedRevisions() ? GetReversedRevisionId() : GetParentRevisionId();
    if (currentParentRevId != revision.GetParentId())
        {
        BeAssert(false && "Parent of revision should match the parent revision id of the Db");
        return RevisionStatus::ParentMismatch;
        }

#ifdef DUMP_REVISION
    revision.Dump(m_dgndb);
#endif

    return txnMgr.ApplyRevision(revision, false /*=invert*/);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Sam.Wilson                         01/2018
//---------------------------------------------------------------------------------------
void OptimisticConcurrencyControlBase::_ConfigureBriefcaseManager(IBriefcaseManager& b)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Sam.Wilson                         01/2018
//---------------------------------------------------------------------------------------
void OptimisticConcurrencyControlBase::_OnProcessRequest(IBriefcaseManager::Request& req, IBriefcaseManager&, IBriefcaseManager::RequestPurpose)
    {
    std::swap(m_locksTemp, req.Locks().GetLockSet()); // prepare to steal the locks

    BeAssert(req.Locks().GetLockSet().empty());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Sam.Wilson                         01/2018
//---------------------------------------------------------------------------------------
void OptimisticConcurrencyControlBase::_OnProcessedRequest(IBriefcaseManager::Request& req, IBriefcaseManager&, IBriefcaseManager::RequestPurpose, IBriefcaseManager::Response& response)
    {
    if (response.Result() != RepositoryStatus::Success)
        {
        std::swap(m_locksTemp, req.Locks().GetLockSet()); // restore the locks that we stole
        }
    else
        {
        m_locks.insert(m_locksTemp.begin(), m_locksTemp.end()); // take ownership of the locks, so that we can report them to the caller (just for information purposes)
        m_locksTemp.clear();
        }

    BeAssert(m_locksTemp.empty());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Sam.Wilson                         01/2018
//---------------------------------------------------------------------------------------
void OptimisticConcurrencyControlBase::_OnExtractRequest(IBriefcaseManager::Request& req, IBriefcaseManager&)
    {
    BeAssert(m_locksTemp.empty());

    // In optimistic concurrency, we don't acquire locks. Instead, we report them.
    auto& rlocks = req.Locks().GetLockSet();
    m_locks.insert(rlocks.begin(), rlocks.end());   // steal the locks
    rlocks.clear();

    BeAssert(req.Locks().GetLockSet().empty());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Sam.Wilson                         01/2018
//---------------------------------------------------------------------------------------
void OptimisticConcurrencyControlBase::_OnExtractedRequest(IBriefcaseManager::Request&, IBriefcaseManager&)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Sam.Wilson                         01/2018
//---------------------------------------------------------------------------------------
void OptimisticConcurrencyControlBase::_OnQueryHeld(DgnLockSet& locks, DgnCodeSet&, IBriefcaseManager&)
    {
    m_locks.insert(locks.begin(), locks.end()); // steal the locks
    locks.clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Sam.Wilson                         01/2018
//---------------------------------------------------------------------------------------
void OptimisticConcurrencyControlBase::_OnQueriedHeld(DgnLockSet&, DgnCodeSet&, IBriefcaseManager&)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Sam.Wilson                         03/2018
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
// @bsimethod                                Sam.Wilson                         01/2018
//---------------------------------------------------------------------------------------
BeSQLite::ChangeSet::ConflictResolution OptimisticConcurrencyControl::HandleConflict(OnConflict onConflict, DgnDbCR db,
                                                                                     Utf8CP tableName, BeSQLite::Changes::Change change,
                                                                                     BeSQLite::DbOpcode opcode, bool indirect)
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
// @bsimethod                                Sam.Wilson                         01/2018
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
// @bsimethod                                Sam.Wilson                         01/2018
//---------------------------------------------------------------------------------------
OptimisticConcurrencyControl::OptimisticConcurrencyControl(Policy policy)
    {
    m_policy = policy;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    04/17
//--------------------------------------------------------------------------------------
RevisionStatus RevisionManager::DoProcessRevisions(bvector<DgnRevisionCP> const& revisions, RevisionProcessOption processOptions)
    {
    RevisionStatus status;
    switch (processOptions)
        {
        case RevisionProcessOption::Merge:
            for (DgnRevisionCP revision : revisions)
                {
                status = DoMergeRevision(*revision);
                if (RevisionStatus::Success != status)
                    return status;
                }
            break;
        case RevisionProcessOption::Reverse:
            for (DgnRevisionCP revision : revisions)
                {
                status = DoReverseRevision(*revision);
                if (RevisionStatus::Success != status)
                    return status;
                }
            break;
        case RevisionProcessOption::Reinstate:
            for (DgnRevisionCP revision : revisions)
                {
                status = DoReinstateRevision(*revision);
                if (RevisionStatus::Success != status)
                    return status;
                }
            break;
        default:
            BeAssert(false && "Invalid revision upgrade option");
        }

    return RevisionStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    12/2017
//---------------------------------------------------------------------------------------
RevisionStatus RevisionManager::ProcessRevisions(bvector<DgnRevisionCP> const& revisions, RevisionProcessOption processOptions)
    {
    for (DgnRevisionCP revision : revisions)
        {
        if (!EXPECTED_CONDITION(!revision->ContainsSchemaChanges(m_dgndb)) && "Cannot process a revision containing schema changes when the DgnDb is already open. Close the DgnDb and reopen with the upgrade schema options set to the revision.")
            return RevisionStatus::ProcessSchemaChangesOnOpen;
        }

    return DoProcessRevisions(revisions, processOptions);
    }

END_BENTLEY_DGNPLATFORM_NAMESPACE
