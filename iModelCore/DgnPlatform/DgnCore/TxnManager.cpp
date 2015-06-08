/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/TxnManager.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>


DPILOG_DEFINE(TxnManager)

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   06/15
//=======================================================================================
struct ChangeBlobHeader
{
    enum {DB_Signature06 = 0x0600};
    uint32_t m_signature;    // write this so we can detect errors on read
    uint32_t m_size;

    ChangeBlobHeader(uint32_t size) {m_signature = DB_Signature06; m_size=size;}
    ChangeBlobHeader(BeSQLite::SnappyReader& in) {uint32_t actuallyRead; in._Read((Byte*) this, sizeof(*this), actuallyRead);}
};

/*---------------------------------------------------------------------------------**//**
* we keep a statement cache just for TxnManager statements
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr TxnManager::GetTxnStatement(Utf8CP sql) const
    {
    CachedStatementPtr ptr;
    m_stmts.GetPreparedStatement(ptr, *m_dgndb.GetDbFile(), sql);
    return ptr;
    }
    
/*---------------------------------------------------------------------------------**//**
* Save a changeset for the current txn into the TXN_Change table in the DgnDb. This compresses the changeset.
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult TxnManager::SaveCurrentChange(ChangeSet& changeset, Utf8CP operation)
    {
    enum Column : int   {SessionId=1,TxnId=2,Deleted=3,Operation=4,Settings=5,Mark=6,Change=7};
    CachedStatementPtr stmt = GetTxnStatement("INSERT INTO " TXN_TABLE_Change " (SessionId,TxnId,Deleted,Operation,Settings,Mark,Change) VALUES(?,?,?,?,?,?,?)");

    stmt->BindGuid(Column::SessionId, m_curr.m_sessionId);
    stmt->BindInt(Column::TxnId,  m_curr.m_txnId);
    stmt->BindInt(Column::Deleted,  false);
    stmt->BindText(Column::Operation, operation ? operation : m_curr.m_description.c_str(), Statement::MakeCopy::No);
    stmt->BindInt(Column::Settings,  m_curr.m_settingsChange);

    if (!m_curr.m_mark.empty())
        {
        stmt->BindText(Column::Mark, m_curr.m_mark, Statement::MakeCopy::No);
        m_curr.m_mark.clear();
        }

    m_snappyTo.Init();
    ChangeBlobHeader header(changeset.GetSize());
    m_snappyTo.Write((ByteCP) &header, sizeof(header));
    m_snappyTo.Write((ByteCP) changeset.GetData(), changeset.GetSize());

    uint32_t zipSize = m_snappyTo.GetCompressedSize();
    if (0 < zipSize)
        {
        if (1 == m_snappyTo.GetCurrChunk())
            stmt->BindBlob(Column::Change, m_snappyTo.GetChunkData(0), zipSize, Statement::MakeCopy::No);
        else
            stmt->BindZeroBlob(Column::Change, zipSize); // more than one chunk in geom stream
        }

    auto rc = stmt->Step();
    if (BE_SQLITE_DONE != rc)
        return rc;

    if (1 == m_snappyTo.GetCurrChunk())
        return BE_SQLITE_DONE;

    StatusInt status = m_snappyTo.SaveToRow(m_dgndb, TXN_TABLE_Change, "Change", m_dgndb.GetLastInsertRowId());
    return (SUCCESS != status) ? BE_SQLITE_ERROR : BE_SQLITE_DONE;
    }

/*---------------------------------------------------------------------------------**//**
* truncate all changes in the TXN_Change table at TxnId (note: entries for TxnId are removed).
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::TruncateChanges(TxnId id)
    {
    CachedStatementPtr stmt = GetTxnStatement("DELETE FROM " TXN_TABLE_Change " WHERE SessionId=? AND TxnId>=?");
    stmt->BindGuid(1, m_curr.m_sessionId);
    stmt->BindInt(2, id.GetValue());
    stmt->Step();
    }

/*---------------------------------------------------------------------------------**//**
* read the entry at the given rowid
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult TxnManager::ReadEntry(TxnRowId rowid, ChangeEntry& entry)
    {
    CachedStatementPtr stmt = GetTxnStatement("SELECT SessionId,TxnId,Operation,Settings,Mark FROM " TXN_TABLE_Change " WHERE rowid=?");
    stmt->BindInt64(1, rowid);
    auto rc = stmt->Step();
    if (rc != BE_SQLITE_ROW)
        return  rc;

    entry.m_sessionId = stmt->GetValueGuid(0);
    entry.m_txnId = TxnId(stmt->GetValueInt(1));
    entry.m_settingsChange = stmt->GetValueInt(2) == 1;
    entry.m_description = stmt->GetValueText(3);
    entry.m_mark = stmt->GetValueText(4);
    return BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2007
+---------------+---------------+---------------+---------------+---------------+------*/
TxnManager::TxnManager(DgnDbR project) : m_dgndb(project), m_stmts(20)
    {
    m_isActive          = false;
    m_undoInProgress    = false;
    m_inDynamics        = false;
    m_propagateChanges  = true;

    m_curr.m_settingsChange = false;
    m_curr.m_txnId = TxnId(0);

    if (m_dgndb.IsReadonly())
        return;

    m_dgndb.AddChangeTracker(*this);
    m_dgndb.CreateTable(TEMP_TABLE(TXN_TABLE_Elements), "ElementId INTEGER NOT NULL PRIMARY KEY,ModelId INTEGER NOT NULL,ChangeType INT,LastMod TIMESTAMP");
    m_dgndb.CreateTable(TEMP_TABLE(TXN_TABLE_Depend), "ECInstanceId INTEGER NOT NULL PRIMARY KEY,ModelId INTEGER NOT NULL,ChangeType INT");
    m_dgndb.ExecuteSql("CREATE INDEX " TEMP_TABLE(TXN_TABLE_Elements) "_Midx ON " TXN_TABLE_Elements "(ModelId)");
    m_dgndb.ExecuteSql("CREATE INDEX " TEMP_TABLE(TXN_TABLE_Depend) "_Midx ON " TXN_TABLE_Depend "(ModelId)");
    }

/*---------------------------------------------------------------------------------**//**
* Get the rowid of the first entry for a supplied TxnId 
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TxnRowId TxnManager::FirstRow(TxnId id)
    {
    CachedStatementPtr stmt = GetTxnStatement("SELECT ROWID FROM " TXN_TABLE_Change " WHERE SessionId=? AND TxnId=? ORDER BY TxnId");
    stmt->BindInt(1, id.GetValue());

    auto rc = stmt->Step();
    UNUSED_VARIABLE(rc);
    BeAssert(rc==BE_SQLITE_ROW);
    return (rc==BE_SQLITE_ROW) ? TxnRowId(stmt->GetValueInt64(0)) : TxnRowId();
    }

/*---------------------------------------------------------------------------------**//**
* Get the rowid of the last entry for a supplied TxnId 
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TxnRowId TxnManager::LastRow(TxnId id)
    {
    CachedStatementPtr stmt = GetTxnStatement("SELECT ROWID FROM " TXN_TABLE_Change " WHERE SessionId=? AND TxnId=? ORDER BY TxnId DESC");
    stmt->BindInt(1, id.GetValue());

    auto rc = stmt->Step();
    UNUSED_VARIABLE(rc);
    BeAssert(rc==BE_SQLITE_ROW);
    return (rc==BE_SQLITE_ROW) ? TxnRowId(stmt->GetValueInt64(0)) : TxnRowId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/04
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::BeginMultiTxnOperation()
    {
    m_multiTxnOp.push_back(GetCurrTxnId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/04
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::EndMultiTxnOperation()
    {
    if (m_multiTxnOp.empty())
        {
        BeAssert(0);
        return;
        }

    m_multiTxnOp.pop_back();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/04
+---------------+---------------+---------------+---------------+---------------+------*/
TxnId TxnManager::GetMultiTxnOperationStart()
    {
    return m_multiTxnOp.empty() ? TxnId(0) : m_multiTxnOp.back();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::CancelChanges(BeSQLite::ChangeSet& direct)
    {
    UndoChangeSet inverted;     // invert it so we can reverse these changes
    inverted.FromData(direct.GetSize(), direct.GetData(), true);

    ApplyChangeSet(inverted, TxnDirection::Undo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::SetUndoInProgress(bool yesNo)
    {
    BeAssert(m_undoInProgress != yesNo);
    BeAssert(IsActive());

    // we're about to reverse or reinstate a committed transaction. Before that can happen, we need to cancel
    // any pending uncommitted changes.
    if (yesNo && HasChanges())
        {
        UndoChangeSet direct;        // get the current changeset holding the uncommitted changes
        direct.FromChangeTrack(*this);

        CancelChanges(direct);

        // clear the undo tracker since these changes are gone
        Restart();
        }

    m_undoInProgress = yesNo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TxnManager::PropagateChanges(BeSQLite::ChangeSet& changeset)
    {
    if (!m_propagateChanges)
        return BSISUCCESS;

    TxnSummary summary(m_dgndb, changeset);
    if (summary.HasModelDependencyChanges())
        {
        // If there were changes to model dependencies, then we have to recompute model dependency order, 
        // updating the "DependencyIndex" column of the Model table.
        // This must be done before invoking the ElementDependencyChangeMonitor below. This must be part of *indirect* 
        // changes, so that they will be re-played during change-merging.
        SetIndirectChanges(true);
        summary.UpdateModelDependencyIndex();
        SetIndirectChanges(false);

        if (summary.HasFatalErrors())
            return BSIERROR;
        }

    SetIndirectChanges(true);
    DgnElementDependencyGraph graph(m_dgndb, summary);
    graph.InvokeAffectedDependencyHandlers(summary);
    SetIndirectChanges(false);

    T_HOST.GetTxnAdmin()._OnTxnBoundary(summary);

    return summary.HasFatalErrors() ? BSIERROR : BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TxnManager::TrackChangesForTable TxnManager::_FilterTable(Utf8CP tableName) 
    {
    // Skip the range tree tables - they hold redundant data that will be automatically updated when the changeset is applied.
    // They all start with the string defined by DGNELEMENT_VTABLE_3dRTree
    if (0 == strncmp(DGN_VTABLE_RTree3d, tableName, sizeof(DGN_VTABLE_RTree3d)-1))
        return  TrackChangesForTable::No;

    return TrackChangesForTable::Yes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/05
+---------------+---------------+---------------+---------------+---------------+------*/
ChangeTracker::OnCommitStatus TxnManager::_OnCommit(bool isCommit, Utf8CP operation) 
    {
    if (!isCommit)
        return OnCommitStatus::Continue;

    if (!HasChanges())
        return OnCommitStatus::Continue;

    ClearReversedTxns();

    TxnId startPos = GetCurrTxnId(); // in case we have to roll back

    // Grab and hold in memory all of the direct changes. We'll use this changeset to drive indirect changes, 
    // and we'll also save this changeset toward the end of the function.
    UndoChangeSet changeset;
    auto rc = changeset.FromChangeTrack(*this);
    if (BE_SQLITE_OK != rc)
        {
        CancelToPos(startPos);
        return OnCommitStatus::Abort;
        }

    BeAssert(0 != changeset.GetSize());

    Restart(); // clear current tracker before propagating changes.

    BentleyStatus status = PropagateChanges(changeset);   // Propagate indirect changes

    if (HasChanges())
        {
        UndoChangeSet indirectChanges; 
        indirectChanges.FromChangeTrack(*this);
        Restart();

        changeset.ConcatenateWith(indirectChanges);
        }

    if (BSISUCCESS != status)
        {
        LOG.errorv("Cancelling txn due to fatal validation error.");
        CancelChanges(changeset); 
        }
    else
        {
        SaveCurrentChange(changeset, operation);
        if (m_multiTxnOp.empty())
            m_curr.m_txnId.Next();
        }

    return OnCommitStatus::Continue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnSummary::ReportError(ValidationError& e)
    {
    m_validationErrors.push_back(e);

    auto sev = (e.GetSeverity() == ValidationError::Severity::Fatal) ? "Fatal" : "Warning";
    LOG.errorv("Validation error. Severity:%s Class:[%s] Description:[%s]", sev, typeid(e).name(), e.GetDescription());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool TxnSummary::HasFatalErrors() const
    {
    for (auto const& e : m_validationErrors)
        {
        if (e.GetSeverity() == ValidationError::Severity::Fatal)
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnSummary::AddAffectedElement(DgnElementId const& eid, DgnModelId mid, ChangeType changeType)
    {
    if (!mid.IsValid())
        mid = m_dgndb.Elements().QueryModelId(eid);

    if (!m_addElementStatement.IsValid())
        m_dgndb.GetCachedStatement(m_addElementStatement, "INSERT INTO " TEMP_TABLE(TXN_TABLE_Elements) "(ElementId,ModelId,ChangeType) VALUES(?,?,?)");

    m_addElementStatement->Reset();
    m_addElementStatement->ClearBindings();
    m_addElementStatement->BindId(1, eid);
    m_addElementStatement->BindId(2, mid);
    m_addElementStatement->BindInt(3, (int) changeType);
    m_addElementStatement->Step();

    m_modelsInTxn.insert(mid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnSummary::AddAffectedDependency(BeSQLite::EC::ECInstanceId const& relid, ChangeType changeType)
    {
    CachedECSqlStatementPtr selectSourceElementModelId = GetDgnDb().GetPreparedECSqlStatement(
        "SELECT element.ModelId FROM " DGN_SCHEMA(DGN_CLASSNAME_Element) " AS element, " DGN_SCHEMA(DGN_RELNAME_ElementDrivesElement) " AS DEP"
        " WHERE (DEP.ECInstanceId=?) AND (element.ECInstanceId=DEP.SourceECInstanceId)");
    selectSourceElementModelId->BindId(1, relid);
    auto stat = selectSourceElementModelId->Step();
    BeAssert(stat == ECSqlStepStatus::HasRow);
    DgnModelId mid = selectSourceElementModelId->GetValueId<DgnModelId>(0);

    if (!m_addElementDepStatement.IsValid())
        m_dgndb.GetCachedStatement(m_addElementDepStatement, "INSERT INTO " TEMP_TABLE(TXN_TABLE_Depend) " (ECInstanceId,ModelId,ChangeType) VALUES(?,?,?)");

    m_addElementDepStatement->Reset();
    m_addElementDepStatement->ClearBindings();
    m_addElementDepStatement->BindId(1, relid);
    m_addElementDepStatement->BindId(2, mid);
    m_addElementDepStatement->BindInt(3, (int) changeType);
    m_addElementDepStatement->Step();

    m_modelsInTxn.insert(mid);
    }

TxnSummary::TxnSummary(DgnDbR db) : m_dgndb(db), m_modelDepsChanged(false), m_elementDepsChanged(false) {BeAssert(!db.IsReadonly());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
TxnSummary::TxnSummary(DgnDbR db, ChangeSet& changeset) : m_dgndb(db), m_modelDepsChanged(false), m_elementDepsChanged(false)
    {
    BeAssert(!db.IsReadonly());
    Utf8String currTable;
    DgnDomain::TableHandler* tblHandler = 0;

    Changes changes(changeset);
    for (auto change : changes)
        {
        Utf8CP tableName;
        int nCols,indirect;
        DbOpcode opcode;
        DbResult rc = change.GetOperation(&tableName, &nCols, &opcode, &indirect);
        BeAssert(rc==BE_SQLITE_OK);
        UNUSED_VARIABLE(rc);

        if (0 != strcmp(currTable.c_str(), tableName)) // changes within a changeset are grouped by table
            {
            currTable = tableName;
            tblHandler = DgnDomains::FindTableHandler(tableName);
            }

        if (NULL == tblHandler)
            continue;

        switch (opcode)
            {
            case DbOpcode::Delete:
                tblHandler->_OnDelete(*this, change);
                break;
            case DbOpcode::Insert:
                tblHandler->_OnAdd(*this, change);
                break;
            case DbOpcode::Update:
                tblHandler->_OnUpdate(*this, change);
                break;
            default:
                BeAssert(false);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TxnSummary::~TxnSummary()
    {
    m_dgndb.ExecuteSql("DELETE FROM " TEMP_TABLE(TXN_TABLE_Elements));
    m_dgndb.ExecuteSql("DELETE FROM " TEMP_TABLE(TXN_TABLE_Depend));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult TxnManager::ApplyChangeSet(BeSQLite::ChangeSet& changeset, TxnDirection isUndo)
    {
    TxnSummary summary(m_dgndb, changeset);

    // notify monitors that changeset is about to be applied
    T_HOST.GetTxnAdmin()._OnTxnReverse(summary, isUndo);

    bool wasTracking = EnableTracking(false);
    DbResult rc = changeset.ApplyChanges(m_dgndb);
    EnableTracking(wasTracking);

    if (rc != BE_SQLITE_OK)
        {
        LOG.errorv("ApplyChangeSet failed with %x", rc);
        BeAssert(false);
        return rc;
        }

    if (true)
        {
#if defined (NEEDS_WORK_TXN_MANAGER)
        if (cleanup == HowToCleanUpElements::CallApplied)
            m_dgndb.Elements().OnChangesetApplied(summary);
#endif

        // At this point, all of the changes to all tables have been applied.
        T_HOST.GetTxnAdmin()._OnTxnReversed(summary, isUndo);
        }

#if defined (NEEDS_WORK_TXN_MANAGER)
    if (cleanup == HowToCleanUpElements::CallCancelled)
        m_dgndb.Elements().OnChangesetCanceled(summary);
#endif

    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ReadChangeSet(UndoChangeSet& changeset, TxnRowId rowId, TxnDirection direction)
    {
    if (ZIP_SUCCESS != m_snappyFrom.Init(m_dgndb, TXN_TABLE_Change, "Changes", rowId))
        {
        BeAssert(false);
        return;
        }

    ChangeBlobHeader header(m_snappyFrom);
    if ((ChangeBlobHeader::DB_Signature06 != header.m_signature) || 0 == header.m_size)
        {
        BeAssert(false);
        return;
        }

    ScopedArray<Byte, 8192> changesBlob(header.m_size);
    uint32_t actuallyRead;
    m_snappyFrom.ReadAndFinish(changesBlob.GetData(), header.m_size, actuallyRead);

    if (actuallyRead != header.m_size)
        {
        BeAssert(false);
        return;
        }

    changeset.FromData(header.m_size, changesBlob.GetData(), direction==TxnDirection::Undo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ApplyChanges(TxnRowId rowId, TxnDirection direction)
    {
    UndoChangeSet changeset;
    ReadChangeSet(changeset, rowId, direction);
    auto rc = ApplyChangeSet(changeset, direction);
    if (BE_SQLITE_OK != rc)
        return;

    // Mark this row as deleted/undeleted depending on the way we just applied the changes.
    CachedStatementPtr stmt = GetTxnStatement("UPDATE " TXN_TABLE_Change " SET Deleted=? WHERE rowid=?");
    stmt->BindInt(1, direction==TxnDirection::Undo);
    stmt->BindInt64(2, rowId);
    rc = stmt->Step();
    BeAssert(rc==BE_SQLITE_DONE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::CancelChanges(TxnRowId txnId)
    {
#if defined (NEEDS_WORK_TXN_MANAGER)
    Utf8String txnDescr; 
    uint64_t txnSource;
    m_db.ReadEntry(txnId, txnSource, txnDescr);

    UndoDb::ChangedFiles files(m_db, txnId);
    for (auto entry : files)
        {
        UndoChangeSet changeset;
        entry.GetChangeSet(changeset, TxnDirection::Undo);

        TxnSummary summary(m_dgndb, txnId, txnDescr, txnSource, changeset);
        m_dgndb.Elements().OnChangesetCanceled(summary);
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   02/04
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ReverseTxnRange(TxnRange& txnRange, Utf8StringP undoStr, bool multiStep)
    {
    SetUndoInProgress(true);
    
    TxnRowId last = LastRow(TxnId(txnRange.GetLast()-1));
    TxnRowId first = FirstRow(txnRange.GetFirst());

    for (TxnRowId curr=last; curr.IsValid() && curr >= first; curr.Prev())
        {
//        if (!IsSettingTxn(curr))
            ApplyChanges(curr, TxnDirection::Undo);
        }

    SetUndoInProgress(false);

    if (undoStr)
        {
        ChangeEntry entry;
        ReadEntry(first, entry);
        Utf8String fmtString = DgnCoreL10N::GetString(DgnCoreL10N::UNDOMSG_FMT_UNDONE());
        undoStr->assign(entry.m_description + fmtString);
        }

    m_curr.m_txnId = txnRange.GetFirst();

    // save in undone txn log
    RevTxn revTxn(txnRange, multiStep);
    m_reversedTxn.push_back(revTxn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt TxnManager::ReverseToPos(TxnId pos)
    {
    if (!PrepareForUndo())
        return ERROR;

    TxnId last = GetCurrTxnId();
    if (pos > last)
        return ERROR;

    TxnRange range(pos, last);
    return ReverseActions(range, true, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt TxnManager::CancelToPos(TxnId pos)
    {
    if (!PrepareForUndo())
        return ERROR;

    StatusInt status = ReverseToPos(pos);
    if (SUCCESS == status)
        ClearReversedTxns();

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt TxnManager::ReverseActions(TxnRange& txnRange, bool multiStep, bool showMsg)
    {
    Utf8String undoStr;
    ReverseTxnRange(txnRange, &undoStr, multiStep);     // do the actual undo now.

    while (GetCurrTxnId() < GetMultiTxnOperationStart())
        EndMultiTxnOperation();

    T_HOST.GetTxnAdmin()._OnUndoRedoFinished(m_dgndb, TxnDirection::Undo);

    if (showMsg)
        NotificationManager::OutputMessage(NotifyMessageDetails(OutputMessagePriority::Info, undoStr.c_str()));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool TxnManager::PrepareForUndo()
    {
    if (IsActive() && IsUndoPossible())
        {
        T_HOST.GetTxnAdmin()._OnPrepareForUndoRedo();
        return  true;
        }

    T_HOST.GetTxnAdmin()._OnNothingToUndo();
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   02/04
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ReverseTxns(int numActions)
    {
    if (!PrepareForUndo())
        return;

    TxnId last = GetCurrTxnId();
    TxnId first(last - numActions);

    TxnRange range(first, last);
    ReverseActions(range, numActions>1, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/08
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ReverseSingleTxn()
    {
    ReverseTxns(1);
    }

#if defined (NEEDS_WORK_TXN_MANAGER)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   02/04
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ReverseToMark(Utf8StringR name)
    {
    if (!PrepareForUndo())
        return;

    TxnId markId(m_db.FindMark(name, GetCurrTxnId()));

    TxnRange range(markId, GetCurrTxnId());
    ReverseActions(range, true, true);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* reverse (undo) all previous transactions
* @bsimethod                                                    Keith.Bentley   02/04
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ReverseAll(bool prompt)
    {
    if (!PrepareForUndo())
        return;

    if (prompt && !T_HOST.GetTxnAdmin()._OnPromptReverseAll())
        {
        T_HOST.GetTxnAdmin()._RestartTool();
        return;
        }

    TxnRange range(TxnId(0), GetCurrTxnId());
    ReverseActions(range, true, true);
    }

/*---------------------------------------------------------------------------------**//**
* Reinstate ("redo") a range of transactions. Also returns the string that identifies what was reinstated.
* @bsimethod                                                    Keith.Bentley   02/04
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ReinstateTxn(TxnRange& revTxn, Utf8StringP redoStr)
    {
    BeAssert(m_curr.m_txnId == revTxn.GetFirst());

    SetUndoInProgress(true);

    TxnRowId last  = LastRow(revTxn.GetLast());
    TxnRowId first = FirstRow(revTxn.GetFirst());

    for (TxnRowId curr=first; curr.IsValid() && curr < last; curr.Next())
        {
//        if (!IsSettingTxn(curr))
            ApplyChanges(curr, TxnDirection::Redo);
        }

    SetUndoInProgress(false);

    if (redoStr)
        {
        ChangeEntry entry;
        ReadEntry(first, entry);
        Utf8String fmtString = DgnCoreL10N::GetString(DgnCoreL10N::UNDOMSG_FMT_REDONE());
        redoStr->assign(entry.m_description + fmtString);
        }

    m_curr.m_txnId = revTxn.GetLast();
    m_reversedTxn.pop_back();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt TxnManager::ReinstateActions(RevTxn& revTxn)
    {
    Utf8String redoStr;
    ReinstateTxn(revTxn.m_range, &redoStr);     // do the actual redo now.

    T_HOST.GetTxnAdmin()._OnUndoRedoFinished(m_dgndb, TxnDirection::Redo);

    NotificationManager::OutputMessage(NotifyMessageDetails(OutputMessagePriority::Info, redoStr.c_str()));
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt TxnManager::ReinstateTxn()
    {
    if (!IsActive() || !IsRedoPossible())
        {
        T_HOST.GetTxnAdmin()._OnNothingToRedo();
        return ERROR;
        }

    T_HOST.GetTxnAdmin()._OnPrepareForUndoRedo();

    RevTxn*  revTxn = &m_reversedTxn.back();
    return  ReinstateActions(*revTxn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TxnManager::GetUndoString()
    {
    if (!IsUndoPossible())
        return "";

    TxnRowId row = FirstRow(TxnId(GetCurrTxnId()-1));
    ChangeEntry entry;
    return ReadEntry(row, entry)==BE_SQLITE_ROW ? entry.m_description : "";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TxnManager::GetRedoString()
    {
    if (!IsRedoPossible())
        return "";

    RevTxn*  revTxn = &m_reversedTxn.back();
    if (revTxn->m_multiStep)
        return "";

    TxnRowId row = FirstRow(revTxn->m_range.GetFirst());
    ChangeEntry entry;
    return ReadEntry(row, entry)==BE_SQLITE_ROW ? entry.m_description : "";
    }

/*---------------------------------------------------------------------------------**//**
* Cancel any undone (rolled-back) transactions. Table Handlers use this to free any memory associated with
* the transactions, since they will no longer be reachable.
* @bsimethod                                                    Keith.Bentley   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ClearReversedTxns()
    {
    if (m_reversedTxn.empty())
        return; // nothing currently undone, nothing to do

    // these transactions are no longer reinstateable. Throw them away.
    m_reversedTxn.clear();
    CachedStatementPtr stmt = GetTxnStatement("DELETE FROM " TXN_TABLE_Change " WHERE Deleted=1");
    stmt->Step();
    }


#if defined (NEEDS_WORK_TXN_MANAGER)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TxnManager::SaveUndoMark(Utf8CP name)
    {
    if (!IsActive())
        return ERROR;

    ClearReversedTxns();

    m_db.SaveMark(TxnId(m_currentTxnID-1), name);
    return  SUCCESS;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/08
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::TxnAdmin::AddTxnMonitor(TxnMonitor& monitor)
    {
    m_monitors.push_back(&monitor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/08
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::TxnAdmin::DropTxnMonitor(TxnMonitor& monitor)
    {
    auto it = std::find(m_monitors.begin(), m_monitors.end(), &monitor);
    if (it != m_monitors.end())
        *it = NULL; // removed from list by CallMonitors
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename CALLER> void DgnPlatformLib::Host::TxnAdmin::CallMonitors(CALLER const& caller)
    {
    for (auto curr = m_monitors.begin(); curr!=m_monitors.end(); )
        {
        if (*curr == NULL)
            curr = m_monitors.erase(curr);
        else
            {
            try {caller(**curr); ++curr;}
            catch(...) {}
            }
        }
    }

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   07/13
//=======================================================================================
struct TxnReverseCaller
{
    TxnSummaryCR m_summary;
    TxnDirection m_isUndo;
    TxnReverseCaller(TxnSummaryCR summary, TxnDirection isUndo) : m_summary(summary), m_isUndo(isUndo) {}
    void operator()(TxnMonitorR monitor) const {monitor._OnTxnReverse(m_summary, m_isUndo);}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   07/13
//=======================================================================================
struct TxnReversedCaller
{
    TxnSummaryCR m_summary;
    TxnDirection m_isUndo;
    TxnReversedCaller(TxnSummaryCR summary, TxnDirection isUndo) : m_summary(summary), m_isUndo(isUndo) {}
    void operator()(TxnMonitorR monitor) const {monitor._OnTxnReversed(m_summary, m_isUndo);}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   07/13
//=======================================================================================
struct TxnBoundaryCaller
{
    TxnSummaryCR m_summary;
    TxnBoundaryCaller(TxnSummaryCR summary) : m_summary(summary) {}
    void operator()(TxnMonitorR monitor) const {monitor._OnTxnBoundary(m_summary);}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   07/13
//=======================================================================================
struct UndoRedoFinishedCaller
    {
    DgnDbR   m_dgndb;
    TxnDirection m_isUndo;
    UndoRedoFinishedCaller(DgnDbR project, TxnDirection isUndo) : m_dgndb(project), m_isUndo(isUndo) {}
    void operator()(TxnMonitorR handler) const {handler._OnUndoRedoFinished(m_dgndb, m_isUndo);}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::TxnAdmin::_OnTxnBoundary(TxnSummaryCR summary)
    {
    CallMonitors(TxnBoundaryCaller(summary));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::TxnAdmin::_OnTxnReverse(TxnSummaryCR summary, TxnDirection isUndo)
    {
    CallMonitors(TxnReverseCaller(summary, isUndo));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::TxnAdmin::_OnTxnReversed(TxnSummaryCR summary, TxnDirection isUndo)
    {
    CallMonitors(TxnReversedCaller(summary, isUndo));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::TxnAdmin::_OnUndoRedoFinished(DgnDbR project, TxnDirection isUndo)
    {
    CallMonitors(UndoRedoFinishedCaller(project, isUndo));
    }

