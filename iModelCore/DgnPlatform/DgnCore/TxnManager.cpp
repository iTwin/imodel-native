/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/TxnManager.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

DPILOG_DEFINE(Txns)

BEGIN_UNNAMED_NAMESPACE
static bool s_aSummaryExists=false;

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   06/15
//=======================================================================================
struct ChangeBlobHeader
{
    enum {DB_Signature06 = 0x0600};
    uint32_t m_signature;    // write this so we can detect errors on read
    uint32_t m_size;

    ChangeBlobHeader(uint32_t size) {m_signature = DB_Signature06; m_size=size;}
    ChangeBlobHeader(SnappyReader& in) {uint32_t actuallyRead; in._Read((Byte*) this, sizeof(*this), actuallyRead);}
};
END_UNNAMED_NAMESPACE

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
* Save a changeset for the current Txn into the TXN_Change table in the DgnDb. This compresses the changeset.
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult TxnManager::SaveCurrentChange(ChangeSet& changeset, Utf8CP operation)
    {
    enum Column : int   {SessionId=1,TxnId=2,Deleted=3,Operation=4,Settings=5,Mark=6,Change=7};
    CachedStatementPtr stmt = GetTxnStatement("INSERT INTO " DGN_TABLE_Txns "(SessionId,TxnId,Deleted,Operation,Settings,Mark,Change) VALUES(?,?,?,?,?,?,?)");

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

    StatusInt status = m_snappyTo.SaveToRow(m_dgndb, DGN_TABLE_Txns, "Change", m_dgndb.GetLastInsertRowId());
    return (SUCCESS != status) ? BE_SQLITE_ERROR : BE_SQLITE_DONE;
    }

/*---------------------------------------------------------------------------------**//**
* truncate all changes in the TXN_Change table at TxnId (note: entries for TxnId are removed).
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::TruncateChanges(TxnId id)
    {
    CachedStatementPtr stmt = GetTxnStatement("DELETE FROM " DGN_TABLE_Txns " WHERE SessionId=? AND TxnId>=?");
    stmt->BindGuid(1, m_curr.m_sessionId);
    stmt->BindInt(2, id.GetValue());
    stmt->Step();
    }

/*---------------------------------------------------------------------------------**//**
* Read the entry information at the given rowid (does not read the changeset itself)
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult TxnManager::ReadEntry(TxnRowId rowid, ChangeEntry& entry)
    {
    CachedStatementPtr stmt = GetTxnStatement("SELECT SessionId,TxnId,Operation,Settings,Mark FROM " DGN_TABLE_Txns " WHERE rowid=?");
    stmt->BindInt64(1, rowid);
    auto rc = stmt->Step();
    if (rc != BE_SQLITE_ROW)
        return  rc;

    entry.m_sessionId = stmt->GetValueGuid(0);
    entry.m_txnId = TxnId(stmt->GetValueInt(1));
    entry.m_description = stmt->GetValueText(2);
    entry.m_settingsChange = stmt->GetValueInt(3) == 1;
    entry.m_mark = stmt->GetValueText(4);
    return BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2007
+---------------+---------------+---------------+---------------+---------------+------*/
TxnManager::TxnManager(DgnDbR project) : m_dgndb(project), m_stmts(20)
    {
    m_undoInProgress    = false;
    m_inDynamics        = false;
    m_propagateChanges  = true;

    m_curr.m_settingsChange = false;
    m_curr.m_txnId = TxnId(0);

    if (m_dgndb.IsReadonly())
        return;

    m_dgndb.SetChangeTracker(this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::AddTxnTable(DgnDomain::TableHandler* tableHandler)
    {
    if (m_dgndb.IsReadonly())
        return;

    // we can get called multiple times with the same tablehandler. Ignore all but the first one.
    TxnTablePtr table= tableHandler->_Create(m_dgndb);
    if (m_tablesByName.Insert(table->_GetTableName(), table).second)
        m_tables.push_back(table.get()); // order matters for the calling sequence, so we have to store it both sorted by name and insertion order
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
TxnTable* TxnManager::FindTxnTable(Utf8CP tableName)
    {
    auto it = m_tablesByName.find(tableName);
    return it != m_tablesByName.end() ? it->second.get() : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
dgn_TxnTable::Element& TxnSummary::Elements() const
    {
    return *(dgn_TxnTable::Element*) m_dgndb.Txns().FindTxnTable(dgn_TxnTable::Element::MyTableName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
dgn_TxnTable::ElementDep& TxnSummary::ElementDependencies() const
    {
    return *(dgn_TxnTable::ElementDep*) m_dgndb.Txns().FindTxnTable(dgn_TxnTable::ElementDep::MyTableName());
    }

/*---------------------------------------------------------------------------------**//**
* Get the rowid of the first entry for a supplied TxnId
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TxnRowId TxnManager::FirstRow(TxnId id)
    {
    CachedStatementPtr stmt = GetTxnStatement("SELECT ROWID FROM " DGN_TABLE_Txns " WHERE SessionId=? AND TxnId=? ORDER BY TxnId LIMIT 1");
    stmt->BindGuid(1, m_curr.m_sessionId);
    stmt->BindInt(2, id.GetValue());

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
    CachedStatementPtr stmt = GetTxnStatement("SELECT ROWID FROM " DGN_TABLE_Txns " WHERE SessionId=? AND TxnId=? ORDER BY TxnId DESC LIMIT 1");
    stmt->BindGuid(1, m_curr.m_sessionId);
    stmt->BindInt(2, id.GetValue());

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
void TxnManager::CancelChanges(ChangeSet& direct)
    {
    UndoChangeSet inverted;     // invert it so we can reverse these changes
    inverted.FromData(direct.GetSize(), direct.GetData(), true);

    ApplyChangeSet(inverted, TxnDirection::Backwards);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::SetUndoInProgress(bool yesNo)
    {
    BeAssert(m_undoInProgress != yesNo);

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
BentleyStatus TxnManager::PropagateChanges(TxnSummaryR summary)
    {
    if (!m_propagateChanges)
        return BSISUCCESS;

    SetIndirectChanges(true);
    for (auto table :  m_tables)
        {
        table->_PropagateChanges(summary);
        if (summary.HasFatalErrors())
            break;
        }
    SetIndirectChanges(false);

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
    if (!isCommit || !HasChanges())
        return OnCommitStatus::Continue;

    DeleteReversedTxns(); // these Txns are no longer reachable.

    TxnId startPos = GetCurrTxnId(); // in case we have to roll back

    // Create changeset from modified tables. We'll use this changeset to drive indirect changes,
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

    BentleyStatus status;
    if (true) // to make sure TxnSummary is destroyed before we call CancelChanges below
        {
        TxnSummary summary(m_dgndb, TxnDirection::Forward);
        summary.AddChangeSet(changeset);

        status = PropagateChanges(summary);   // Propagate indirect changes

        // At this point, all of the changes to all tables have been applied.
        if (SUCCESS == status)
            T_HOST.GetTxnAdmin()._OnTxnCommit(summary);

        if (HasChanges())
            {
            UndoChangeSet indirectChanges;
            indirectChanges.FromChangeTrack(*this);
            Restart();

            summary.AddChangeSet(indirectChanges); // so summary will hold entire changeset
            changeset.ConcatenateWith(indirectChanges);
            }
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
            m_curr.m_txnId.Next(); // only increment the TxnId if we're not doing a multi-step operation
        }

    return OnCommitStatus::Continue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TxnSummary::TxnSummary(DgnDbR db, TxnDirection direction) : m_dgndb(db), m_direction(direction)
    {
    BeAssert(!db.IsReadonly());
    BeAssert(!s_aSummaryExists); // only one summary at a time may exist because they use temporary tables

    for (auto table:  db.Txns().m_tables)
        table->_OnTxnSummaryStart(*this);

    s_aSummaryExists=true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
TxnSummary::~TxnSummary()
    {
    BeAssert(s_aSummaryExists);
    s_aSummaryExists=false;

    for (auto table :  m_dgndb.Txns().m_tables)
        table->_OnTxnSummaryEnd(*this);
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
* Add all of the changes in a changeset to the TxnSummary. TxnTables store information about the changes in their own state
* if they need to hold on to them so they can react after the changeset is applied.
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnSummary::AddChangeSet(ChangeSet& changeset)
    {
    BeAssert(!m_dgndb.IsReadonly());
    Utf8String currTable;
    TxnTable* txnTable = 0;
    TxnManager& txns = m_dgndb.Txns();

    Changes changes(changeset); // this is a "changeset iterator"

    // Walk through each changed row in the changeset. They are ordered by table, so we know that all changes to one table will be seen
    // before we see any changes to another table.
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
            txnTable = txns.FindTxnTable(tableName);
            }

        if (nullptr == txnTable)
            continue; // this table does not have a TxnTable for it, skip it

        switch (opcode)
            {
            case DbOpcode::Delete:
                txnTable->_OnDelete(*this, change);
                break;
            case DbOpcode::Insert:
                txnTable->_OnAdd(*this, change);
                break;
            case DbOpcode::Update:
                txnTable->_OnUpdate(*this, change);
                break;
            default:
                BeAssert(false);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Apply a changeset to the database. Notify all TxnTables about what's in the Changeset, both before
* and after it is applied.
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult TxnManager::ApplyChangeSet(ChangeSet& changeset, TxnDirection direction)
    {
    TxnSummary summary(m_dgndb, direction); // this sends notifications to the TxnTables that a changeset is about to be applied
    summary.AddChangeSet(changeset); // each TxnTable is notified about changes to their table here.

    T_HOST.GetTxnAdmin()._OnTxnReverse(summary);   // notify monitors that changeset is about to be applied

    bool wasTracking = EnableTracking(false);
    DbResult rc = changeset.ApplyChanges(m_dgndb); // this actually updates the database with the changes
    EnableTracking(wasTracking);

    if (rc != BE_SQLITE_OK)
        {
        BeAssert(false);
        return rc;
        }

    // At this point, all of the changes to all tables have been applied. Notify TxnTables that it is completed.
    for (auto table : m_tables)
        table->_OnChangesetApplied(summary);

    T_HOST.GetTxnAdmin()._OnTxnReversed(summary);

    // destructor for TxnSummary notifies TxnTables that the changeset is complete.
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* Changesets are stored as compressed blobs in the DGN_TABLE_Txns table. Read one by rowid.
* If the TxnDirection is backwards, invert the changeset.
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ReadChangeSet(UndoChangeSet& changeset, TxnRowId rowId, TxnDirection direction)
    {
    if (ZIP_SUCCESS != m_snappyFrom.Init(m_dgndb, DGN_TABLE_Txns, "Change", rowId))
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

    changeset.FromData(header.m_size, changesBlob.GetData(), direction==TxnDirection::Backwards);
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
    CachedStatementPtr stmt = GetTxnStatement("UPDATE " DGN_TABLE_Txns " SET Deleted=? WHERE rowid=?");
    stmt->BindInt(1, direction==TxnDirection::Backwards);
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
        entry.GetChangeSet(changeset, TxnDirection::Backwards);

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

    TxnRowId last  = LastRow(TxnId(txnRange.GetLast()-1));
    TxnRowId first = FirstRow(txnRange.GetFirst());

    for (TxnRowId curr=last; curr.IsValid() && curr >= first; curr.Prev())
        ApplyChanges(curr, TxnDirection::Backwards);

    m_dgndb.SaveChanges(); // make sure we save the updated Txn data to disk.

    SetUndoInProgress(false);

    if (undoStr)
        {
        ChangeEntry entry;
        ReadEntry(first, entry);
        Utf8String fmtString = DgnCoreL10N::GetString(DgnCoreL10N::Undone());
        undoStr->assign(entry.m_description + fmtString);
        }

    m_curr.m_txnId = txnRange.GetFirst(); // we reuse txnids

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
        DeleteReversedTxns();

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

    T_HOST.GetTxnAdmin()._OnUndoRedoFinished(m_dgndb, TxnDirection::Backwards);

    if (showMsg)
        NotificationManager::OutputMessage(NotifyMessageDetails(OutputMessagePriority::Info, undoStr.c_str()));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool TxnManager::PrepareForUndo()
    {
    if (IsUndoPossible())
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
StatusInt TxnManager::ReverseTxns(int numActions)
    {
    if (!PrepareForUndo())
        return ERROR;

    TxnId last = GetCurrTxnId();
    TxnId first(last - numActions);

    TxnRange range(first, last);
    return ReverseActions(range, numActions>1, true);
    }

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

    TxnRowId last  = LastRow(TxnId(revTxn.GetLast()-1));
    TxnRowId first = FirstRow(revTxn.GetFirst());

    for (TxnRowId curr=first; curr.IsValid() && curr <= last; curr.Next())
        ApplyChanges(curr, TxnDirection::Forward);

    SetUndoInProgress(false);

    m_dgndb.SaveChanges(); // make sure we save the updated Txn data to disk.

    if (redoStr)
        {
        ChangeEntry entry;
        ReadEntry(first, entry);
        Utf8String fmtString = DgnCoreL10N::GetString(DgnCoreL10N::Redone());
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

    T_HOST.GetTxnAdmin()._OnUndoRedoFinished(m_dgndb, TxnDirection::Forward);

    NotificationManager::OutputMessage(NotifyMessageDetails(OutputMessagePriority::Info, redoStr.c_str()));
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt TxnManager::ReinstateTxn()
    {
    if (!IsRedoPossible())
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
* Cancel any undone (rolled-back) transactions.
* @bsimethod                                                    Keith.Bentley   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::DeleteReversedTxns()
    {
    if (m_reversedTxn.empty())
        return; // nothing currently undone, nothing to do

    // these transactions are no longer reinstateable. Throw them away.
    m_reversedTxn.clear();
    CachedStatementPtr stmt = GetTxnStatement("DELETE FROM " DGN_TABLE_Txns " WHERE Deleted=1");
    stmt->Step();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Element::_OnTxnSummaryStart(TxnSummary&)
    {
    if (m_stmt.IsPrepared())
        return;

    m_dgndb.CreateTable(TEMP_TABLE(TXN_TABLE_Elements), "ElementId INTEGER NOT NULL PRIMARY KEY,ModelId INTEGER NOT NULL,ChangeType INT,LastMod TIMESTAMP");
    m_dgndb.ExecuteSql("CREATE INDEX " TEMP_TABLE(TXN_TABLE_Elements) "_Midx ON " TXN_TABLE_Elements "(ModelId)");
    m_stmt.Prepare(m_dgndb, "INSERT INTO " TEMP_TABLE(TXN_TABLE_Elements) "(ElementId,ModelId,ChangeType,LastMod) VALUES(?,?,?,?)");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Element::_OnTxnSummaryEnd(TxnSummary&)
    {
    m_dgndb.ExecuteSql("DELETE FROM " TEMP_TABLE(TXN_TABLE_Elements));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Model::_OnTxnSummaryStart(TxnSummary& summary)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Model::_OnTxnSummaryEnd(TxnSummary& summary)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::ElementDep::_OnTxnSummaryStart(TxnSummary&)
    {
    if (m_stmt.IsPrepared())
        return;

    m_dgndb.CreateTable(TEMP_TABLE(TXN_TABLE_Depend), "ECInstanceId INTEGER NOT NULL PRIMARY KEY,ModelId INTEGER NOT NULL,ChangeType INT");
    m_dgndb.ExecuteSql("CREATE INDEX " TEMP_TABLE(TXN_TABLE_Depend) "_Midx ON " TXN_TABLE_Depend "(ModelId)");
    m_stmt.Prepare(m_dgndb, "INSERT INTO " TEMP_TABLE(TXN_TABLE_Depend) " (ECInstanceId,ModelId,ChangeType) VALUES(?,?,?)");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::ElementDep::_OnTxnSummaryEnd(TxnSummary& summary)
    {
    m_dgndb.ExecuteSql("DELETE FROM " TEMP_TABLE(TXN_TABLE_Depend));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::ElementDep::UpdateSummary(TxnSummary& summary, Changes::Change change, TxnSummary::ChangeType changeType)
    {
    Changes::Change::Stage stage = (TxnSummary::ChangeType::Insert == changeType) ? Changes::Change::Stage::New : Changes::Change::Stage::Old;
    ECInstanceId instanceId(change.GetValue(0, stage).GetValueInt64()); // primary key is column 0
    AddDependency(instanceId, changeType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::ModelDep::_OnAdd(TxnSummary& summary, Changes::Change const& change)
    {
    SetChanges();
    CheckDirection(summary, change.GetNewValue(0).GetValueId<EC::ECInstanceId>());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::ModelDep::_OnUpdate(TxnSummary& summary, Changes::Change const& change)
    {
    SetChanges();
    CheckDirection(summary, change.GetOldValue(0).GetValueId<EC::ECInstanceId>());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Element::AddElement(DgnElementId elementId, DgnModelId modelId, double lastMod, TxnSummary::ChangeType changeType)
    {
    enum Column : int {ElementId=1,ModelId=2,ChangeType=3,LastMod=4};

    BeAssert(modelId.IsValid());
    BeAssert(elementId.IsValid());

    m_stmt.BindId(Column::ElementId, elementId);
    m_stmt.BindId(Column::ModelId, modelId);
    m_stmt.BindInt(Column::ChangeType, (int) changeType);
    m_stmt.BindDouble(Column::LastMod, lastMod);

    auto rc = m_stmt.Step();
    BeAssert(rc==BE_SQLITE_DONE);
    UNUSED_VARIABLE(rc);

    m_stmt.Reset();
    m_stmt.ClearBindings();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2015
//---------------------------------------------------------------------------------------
void dgn_TxnTable::Element::AddChange(TxnSummary& summary, Changes::Change const& change, TxnSummary::ChangeType changeType)
    {
    Changes::Change::Stage stage;
    switch (changeType)
        {
        case TxnSummary::ChangeType::Insert:
            stage = Changes::Change::Stage::New;
            break;

        case TxnSummary::ChangeType::Update:
        case TxnSummary::ChangeType::Delete:
            stage = Changes::Change::Stage::Old;
            break;
        default:
            BeAssert(false);
            return;
        }

    DgnElementId elementId = DgnElementId(change.GetValue(0, stage).GetValueInt64());
    DgnModelId modelId;

    if (TxnSummary::ChangeType::Update == changeType)
        {
        // for updates, the element table must be queried for ModelId since the change set will only contain changed columns
        modelId = summary.GetDgnDb().Elements().QueryModelId(elementId);
        }
    else
        modelId = DgnModelId(change.GetValue(2, stage).GetValueInt64());   // assumes DgnModelId is column 2

    if (changeType == TxnSummary::ChangeType::Update)
        stage = Changes::Change::Stage::New;
    double lastMod = change.GetValue(7, stage).GetValueDouble();           // assumes LastMod is column 7

    AddElement(elementId, modelId, lastMod, changeType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
dgn_TxnTable::Element::Iterator::Entry dgn_TxnTable::Element::Iterator::begin() const
    {
    if (!m_stmt.IsValid())
        m_db->GetCachedStatement(m_stmt, "SELECT ElementId,ModelId,ChangeType,LastMod FROM " TEMP_TABLE(TXN_TABLE_Elements));
    else
        m_stmt->Reset();

    return Entry(m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

DgnElementId dgn_TxnTable::Element::Iterator::Entry::GetElementId() const {return m_sql->GetValueId<DgnElementId>(0);}
DgnModelId dgn_TxnTable::Element::Iterator::Entry::GetModelId() const {return m_sql->GetValueId<DgnModelId>(1);}
TxnSummary::ChangeType dgn_TxnTable::Element::Iterator::Entry::GetChangeType() const {return (TxnSummary::ChangeType) m_sql->GetValueInt(2);}
double dgn_TxnTable::Element::Iterator::Entry::GetLastMod() const {return m_sql->GetValueDouble(3);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::ElementDep::AddDependency(EC::ECInstanceId const& relid, TxnSummary::ChangeType changeType)
    {
    CachedECSqlStatementPtr stmt  = m_dgndb.GetPreparedECSqlStatement(
        "SELECT element.ModelId FROM " DGN_SCHEMA(DGN_CLASSNAME_Element) " AS element, " DGN_SCHEMA(DGN_RELNAME_ElementDrivesElement) " AS DEP"
        " WHERE (DEP.ECInstanceId=?) AND (element.ECInstanceId=DEP.SourceECInstanceId)");
    stmt->BindId(1, relid);
    auto stat = stmt->Step();
    BeAssert(stat == ECSqlStepStatus::HasRow);
    DgnModelId mid = stmt->GetValueId<DgnModelId>(0);

    m_stmt.BindId(1, relid);
    m_stmt.BindId(2, mid);
    m_stmt.BindInt(3, (int) changeType);
    auto rc = m_stmt.Step();
    BeAssert(rc==BE_SQLITE_DONE);
    UNUSED_VARIABLE(rc);

    m_stmt.Reset();
    m_stmt.ClearBindings();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  03/2015
//---------------------------------------------------------------------------------------
void dgn_TxnTable::ModelDep::CheckDirection(TxnSummary& summary, ECInstanceId relid)
    {
    Statement stmt(summary.GetDgnDb(), "SELECT RootModelId,DependentModelId FROM " DGN_TABLE(DGN_RELNAME_ModelDrivesModel) " WHERE(ECInstanceId=?)");
    stmt.BindId(1, relid);
    if (stmt.Step() != BE_SQLITE_ROW)
        {
        BeAssert(false); // model was just added or modified -- it has to exist!
        return;
        }
    DgnModelId rootModel = stmt.GetValueId<DgnModelId>(0);
    DgnModelId depModel = stmt.GetValueId<DgnModelId>(1);

    DgnModels::Model root, dep;
    summary.GetDgnDb().Models().QueryModelById(&root, rootModel);
    summary.GetDgnDb().Models().QueryModelById(&dep, depModel);

    if (root.GetModelType() > dep.GetModelType())
        {
        //  A Physical model cannot depend on a Drawing model
        summary.ReportError(*new DgnElementDependencyGraph::DirectionValidationError(""));
        }
    }

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
    TxnReverseCaller(TxnSummaryCR summary) : m_summary(summary) {}
    void operator()(TxnMonitorR monitor) const {monitor._OnTxnReverse(m_summary);}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   07/13
//=======================================================================================
struct TxnReversedCaller
{
    TxnSummaryCR m_summary;
    TxnReversedCaller(TxnSummaryCR summary) : m_summary(summary) {}
    void operator()(TxnMonitorR monitor) const {monitor._OnTxnReversed(m_summary);}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   07/13
//=======================================================================================
struct TxnCommitCaller
{
    TxnSummaryCR m_summary;
    TxnCommitCaller(TxnSummaryCR summary) : m_summary(summary) {}
    void operator()(TxnMonitorR monitor) const {monitor._OnTxnCommit(m_summary);}
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
void DgnPlatformLib::Host::TxnAdmin::_OnTxnCommit(TxnSummaryCR summary)
    {
    CallMonitors(TxnCommitCaller(summary));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::TxnAdmin::_OnTxnReverse(TxnSummaryCR summary)
    {
    CallMonitors(TxnReverseCaller(summary));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::TxnAdmin::_OnTxnReversed(TxnSummaryCR summary)
    {
    CallMonitors(TxnReversedCaller(summary));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::TxnAdmin::_OnUndoRedoFinished(DgnDbR project, TxnDirection isUndo)
    {
    CallMonitors(UndoRedoFinishedCaller(project, isUndo));
    }
