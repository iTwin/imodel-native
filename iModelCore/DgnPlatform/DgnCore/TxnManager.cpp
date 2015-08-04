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
//=======================================================================================
// @bsiclass                                                    Keith.Bentley   06/15
//=======================================================================================
struct ChangesBlobHeader
{
    enum {DB_Signature06 = 0x0600};
    uint32_t m_signature;    // write this so we can detect errors on read
    uint32_t m_size;

    ChangesBlobHeader(uint32_t size) {m_signature = DB_Signature06; m_size=size;}
    ChangesBlobHeader(SnappyReader& in) {uint32_t actuallyRead; in._Read((Byte*) this, sizeof(*this), actuallyRead);}
};
END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TxnManager::UndoChangeSet::ConflictResolution TxnManager::UndoChangeSet::_OnConflict(ConflictCause cause, BeSQLite::Changes::Change change)
    {
    Utf8CP tableName;
    int nCols,indirect;
    DbOpcode opcode;
    change.GetOperation(&tableName, &nCols, &opcode, &indirect);

    if (cause == ConflictCause::NotFound)
        {
        if (opcode == DbOpcode::Delete)      // a delete that is already gone. 
            return ConflictResolution::Skip; // This is caused by propagate delete on a foreign key. It is not a problem.
        }

    BeAssert(false);
    return ConflictResolution::Skip;
    }

/*---------------------------------------------------------------------------------**//**
* We keep a statement cache just for TxnManager statements
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr TxnManager::GetTxnStatement(Utf8CP sql) const
    {
    CachedStatementPtr ptr;
    m_stmts.GetPreparedStatement(ptr, *m_dgndb.GetDbFile(), sql);
    return ptr;
    }

/*---------------------------------------------------------------------------------**//**
* Save a changeset for the current Txn into the DGN_TABLE_Txns table in the DgnDb. This compresses the changeset.
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult TxnManager::SaveCurrentChange(ChangeSet& changeset, Utf8CP operation)
    {
    enum Column : int {Id=1,Deleted=2,Grouped=3,Operation=4,Change=5};
    CachedStatementPtr stmt = GetTxnStatement("INSERT INTO " DGN_TABLE_Txns "(Id,Deleted,Grouped,Operation,Change) VALUES(?,?,?,?,?)");

    stmt->BindInt64(Column::Id, m_curr.GetValue());
    stmt->BindInt(Column::Deleted,  false);
    if (nullptr != operation)
        stmt->BindText(Column::Operation, operation, Statement::MakeCopy::No);

    // if we're in a multi-txn operation, and if the current TxnId is greater than the first txn, mark it as "grouped"
    stmt->BindInt(Column::Grouped, !m_multiTxnOp.empty() && (m_curr < m_multiTxnOp.back()));

    m_curr.Increment(); // increment txn id before we exit

    m_snappyTo.Init();
    ChangesBlobHeader header(changeset.GetSize());
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
* Read the description of a Txn
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TxnManager::GetTxnDescription(TxnId rowid)
    {
    CachedStatementPtr stmt = GetTxnStatement("SELECT Operation FROM " DGN_TABLE_Txns " WHERE Id=?");
    stmt->BindInt64(1, rowid.GetValue());

    auto rc = stmt->Step();
    return rc != BE_SQLITE_ROW ? "" : stmt->GetValueText(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool TxnManager::IsMultiTxnMember(TxnId rowid)
    {
    CachedStatementPtr stmt = GetTxnStatement("SELECT Grouped FROM " DGN_TABLE_Txns " WHERE Id=?");
    stmt->BindInt64(1, rowid.GetValue());

    auto rc = stmt->Step();
    return rc != BE_SQLITE_ROW ? false : TO_BOOL(stmt->GetValueInt(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TxnManager::TxnManager(DgnDbR dgndb) : m_dgndb(dgndb), m_stmts(20)
    {
    m_inDynamics        = false;
    m_propagateChanges  = true;
    m_action = TxnAction::None;

    m_dgndb.SetChangeTracker(this);

    Statement stmt;
    stmt.Prepare(m_dgndb, "SELECT MAX(Id) FROM " DGN_TABLE_Txns);
    DbResult result = stmt.Step();
    BeAssert(result == BE_SQLITE_ROW);

    TxnId last = stmt.GetValueInt(0); // this is where we left off last session
    m_curr = TxnId(SessionId(last.GetSession().GetValue()+1), 0); // increment the session id, reset to index to 0.

    // whenever we open a Briefcase for write access, enable tracking
    if (!m_dgndb.IsReadonly() && m_dgndb.IsBriefcase())
        EnableTracking(true);
    }

/*---------------------------------------------------------------------------------**//**
* add a new TxnTable to this TxnManager. TxnTables are responsible for reacting to changes to a given SQLite table, by name.
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::AddTxnTable(DgnDomain::TableHandler* tableHandler)
    {
    if (m_dgndb.IsReadonly())
        return;

    // we can get called multiple times with the same tablehandler. Ignore all but the first one.
    TxnTablePtr table= tableHandler->_Create(*this);
    if (m_tablesByName.Insert(table->_GetTableName(), table).second)
        m_tables.push_back(table.get()); // order matters for the calling sequence, so we have to store it both sorted by name and insertion order
    }

/*---------------------------------------------------------------------------------**//**
* Find a TxnTable by name
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
TxnTable* TxnManager::FindTxnTable(Utf8CP tableName) const
    {
    auto it = m_tablesByName.find(tableName);
    return it != m_tablesByName.end() ? it->second.get() : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* Get the TxnTable that handles changes to the dgn_Element table
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
dgn_TxnTable::Element& TxnManager::Elements() const
    {
    return *(dgn_TxnTable::Element*) FindTxnTable(dgn_TxnTable::Element::MyTableName());
    }

/*---------------------------------------------------------------------------------**//**
* Get the TxnTable that handles changes to the dgn_ElementDrivesElement table
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
dgn_TxnTable::ElementDep& TxnManager::ElementDependencies() const
    {
    return *(dgn_TxnTable::ElementDep*) FindTxnTable(dgn_TxnTable::ElementDep::MyTableName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TxnManager::TxnId TxnManager::QueryPreviousTxnId(TxnId curr) const
    {
    CachedStatementPtr stmt = GetTxnStatement("SELECT Id FROM " DGN_TABLE_Txns " WHERE Id<? ORDER BY Id DESC LIMIT 1");
    stmt->BindInt64(1, curr.GetValue());

    auto rc = stmt->Step();
    return (rc != BE_SQLITE_ROW) ? TxnId() : TxnId(stmt->GetValueInt64(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TxnManager::TxnId TxnManager::QueryNextTxnId(TxnId curr) const
    {
    CachedStatementPtr stmt = GetTxnStatement("SELECT Id FROM " DGN_TABLE_Txns " WHERE Id>? ORDER BY Id LIMIT 1");
    stmt->BindInt64(1, curr.GetValue());

    auto rc = stmt->Step();
    return (rc != BE_SQLITE_ROW) ? TxnId() : TxnId(stmt->GetValueInt64(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/04
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::BeginMultiTxnOperation()
    {
    m_multiTxnOp.push_back(GetCurrentTxnId());
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
TxnManager::TxnId TxnManager::GetMultiTxnOperationStart()
    {
    return m_multiTxnOp.empty() ? GetSessionStartId() : m_multiTxnOp.back();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TxnManager::PropagateChanges()
    {
    if (!m_propagateChanges)
        return BSISUCCESS;

    SetIndirectChanges(true);
    for (auto table :  m_tables)
        {
        table->_PropagateChanges();
        if (HasFatalErrors())
            break;
        }
    SetIndirectChanges(false);

    return HasFatalErrors() ? BSIERROR : BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* When journalling changes, SQLite calls this method to determine whether changes to a specific table are eligible or not.
* @note tables with no primary key are skipped automatically.
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TxnManager::TrackChangesForTable TxnManager::_FilterTable(Utf8CP tableName)
    {
    // Skip the range tree tables - they hold redundant data that will be automatically updated when the changeset is applied.
    // They all start with the string defined by DGNELEMENT_VTABLE_3dRTree
    if (0 == strncmp(DGN_VTABLE_RTree3d, tableName, sizeof(DGN_VTABLE_RTree3d)-1))
        return  TrackChangesForTable::No;

    if (0 == strncmp(DGN_TABLE_Txns, tableName, sizeof(DGN_TABLE_Txns)-1))
        return  TrackChangesForTable::No;

    return TrackChangesForTable::Yes;
    }

/*---------------------------------------------------------------------------------**//**
* The supplied changeset represents all of the pending uncommited changes in the current transaction.
* Use the SQLite "ROLLBACK" statement to reverse all of those changes in the database, and then call the OnChangesetApplied method
* on the inverted changeset to allow TxnTables to react to the fact that the changes were abandoned.
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
ChangeTracker::OnCommitStatus TxnManager::CancelChanges(ChangeSet& changeset)
    {
    changeset.Invert();

    DbResult rc = GetDgnDb().ExecuteSql("ROLLBACK");
    if (rc != BE_SQLITE_OK)
        return OnCommitStatus::Abort;

    OnChangesetApplied(changeset, TxnAction::Abandon);
    return OnCommitStatus::Completed;
    }

/*---------------------------------------------------------------------------------**//**
* The supplied changeset was just applied to the database by the supplied action. That means the the database now potentially reflects a different
* state than the in-memory objects for the affected tables. Use the changeset to send _OnReversedxxx events to the TxnTables for each changed row,
* so they can update in-memory state as necessary.
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::OnChangesetApplied(ChangeSet& changeset, TxnAction action)
    {
    m_action = action;
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
                txnTable->_OnReversedAdd(change);
                break;
            case DbOpcode::Insert:
                txnTable->_OnReversedDelete(change);
                break;
            case DbOpcode::Update:
                txnTable->_OnReversedUpdate(change);
                break;
            default:
                BeAssert(false);
            }
        }
    m_action = TxnAction::None;
    }

/*---------------------------------------------------------------------------------**//**
* A changeset is about to be committed (or, in the case of "what if" testing, look like it is committed). Let each
* TxnTable prepare its state
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::OnBeginValidate()
    {
    m_action = TxnAction::Commit;
    for (auto table : m_tables)
        table->_OnValidate();
    }

/*---------------------------------------------------------------------------------**//**
* A changeset was just be committed. Let each TxnTable clean up its state. Also clear the validation errors.
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::OnEndValidate()
    {
    for (auto table : m_tables)
        table->_OnValidated();

    m_validationErrors.clear();
    m_action = TxnAction::None;
    }

/*---------------------------------------------------------------------------------**//**
* Called from Db::SaveChanges or Db::AbandonChanges when the TxnManager change tracker has changes.
* This method creates a changeset from the change tracker.
* If this is a "cancel", it rolls back the current Txn, and calls _OnReversedxxx methods on all affected TxnTables.
* If this is a commit, it calls the _OnValidatexxx methods for the TxnTables, and then calls "_PropagateChanges"
* It saves the resultant changeset (the combination of direct changes plus indirect changes) as a Txn in the database.
* The Txn may be undone in this session via the "ReverseTxn" method.
* When this method returns, the entire transaction is committed in BeSQLite. In this manner, it is impossible to make
* changes to the database that aren't also saved in the dgn_Txn table.
* @bsimethod                                    Keith.Bentley                   01/05
+---------------+---------------+---------------+---------------+---------------+------*/
ChangeTracker::OnCommitStatus TxnManager::_OnCommit(bool isCommit, Utf8CP operation)
    {
    DeleteReversedTxns(); // these Txns are no longer reachable.

    // Create changeset from modified tables. We'll use this changeset to drive indirect changes.
    UndoChangeSet changeset;
    auto rc = changeset.FromChangeTrack(*this);
    UNUSED_VARIABLE(rc);
    BeAssert(BE_SQLITE_OK == rc);
    BeAssert(0 != changeset.GetSize());
    Restart();  // clear the change tracker, since we have captured all the changes in the changeset

    if (!isCommit)
        return CancelChanges(changeset);

    OnBeginValidate();
    AddChangeSet(changeset);

    BentleyStatus status = PropagateChanges();   // Propagate to generate indirect changes

    if (HasChanges()) // did we have any indirect changes?
        {
        UndoChangeSet indirectChanges;
        indirectChanges.FromChangeTrack(*this);
        Restart();
        changeset.ConcatenateWith(indirectChanges); // combine direct and indirect changes into a single changeset
        }

    if (BSISUCCESS != status)
        {
        LOG.errorv("Cancelling txn due to fatal validation error.");
        OnEndValidate();
        return CancelChanges(changeset); // roll back entire txn
        }

    // At this point, all of the changes to all tables have been applied. Tell TxnMonitors
    T_HOST.GetTxnAdmin()._OnCommit(*this);

    SaveCurrentChange(changeset, operation); // save changeset into DgnDb itself, along with the description of the operation we're performing

    OnEndValidate();
    return OnCommitStatus::Continue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ReportError(ValidationError& e)
    {
    m_validationErrors.push_back(e);

    auto sev = (e.GetSeverity() == ValidationError::Severity::Fatal) ? "Fatal" : "Warning";
    LOG.errorv("Validation error. Severity:%s Class:[%s] Description:[%s]", sev, typeid(e).name(), e.GetDescription());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool TxnManager::HasFatalErrors() const
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
void TxnManager::AddChangeSet(ChangeSet& changeset)
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
                txnTable->_OnValidateDelete(change);
                break;
            case DbOpcode::Insert:
                txnTable->_OnValidateAdd(change);
                break;
            case DbOpcode::Update:
                txnTable->_OnValidateUpdate(change);
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
DbResult TxnManager::ApplyChangeSet(ChangeSet& changeset, TxnAction action)
    {
    BeAssert(!HasChanges());

    bool wasTracking = EnableTracking(false);
    DbResult rc = changeset.ApplyChanges(m_dgndb); // this actually updates the database with the changes
    BeAssert (rc == BE_SQLITE_OK);
    EnableTracking(wasTracking);

    OnChangesetApplied(changeset, action);

    T_HOST.GetTxnAdmin()._OnReversedChanges(*this);

    BeAssert(!HasChanges());
    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* Changesets are stored as compressed blobs in the DGN_TABLE_Txns table. Read one by rowid.
* If the TxnDirection is backwards, invert the changeset.
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ReadChangeSet(UndoChangeSet& changeset, TxnId rowId, TxnAction action)
    {
    if (ZIP_SUCCESS != m_snappyFrom.Init(m_dgndb, DGN_TABLE_Txns, "Change", rowId.GetValue()))
        {
        BeAssert(false);
        return;
        }

    ChangesBlobHeader header(m_snappyFrom);
    if ((ChangesBlobHeader::DB_Signature06 != header.m_signature) || 0 == header.m_size)
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

    changeset.FromData(header.m_size, changesBlob.GetData(), action==TxnAction::Reverse);
    }

/*---------------------------------------------------------------------------------**//**
* Read a changeset from the dgn_Txn table, potentially inverting it (depending on whether we'performing undo or redo),
* and then apply the changeset to the DgnDb.
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ApplyChanges(TxnId rowId, TxnAction action)
    {
    UndoChangeSet changeset;
    ReadChangeSet(changeset, rowId, action);

    auto rc = ApplyChangeSet(changeset, action);
    if (BE_SQLITE_OK != rc)
        return;

    // Mark this row as deleted/undeleted depending on which way we just applied the changes.
    CachedStatementPtr stmt = GetTxnStatement("UPDATE " DGN_TABLE_Txns " SET Deleted=? WHERE Id=?");
    stmt->BindInt(1, action==TxnAction::Reverse);
    stmt->BindInt64(2, rowId.GetValue());
    rc = stmt->Step();
    BeAssert(rc==BE_SQLITE_DONE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   02/04
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ReverseTxnRange(TxnRange& txnRange, Utf8StringP undoStr)
    {
    if (HasChanges())
        m_dgndb.AbandonChanges();

    for (TxnId curr=QueryPreviousTxnId(txnRange.GetLast()); curr.IsValid() && curr >= txnRange.GetFirst(); curr=QueryPreviousTxnId(curr))
        ApplyChanges(curr, TxnAction::Reverse);

    BeAssert(!HasChanges());

    m_dgndb.SaveChanges(); // make sure we save the updated Txn data to disk.

    if (undoStr)
        {
        Utf8String fmtString = DgnCoreL10N::GetString(DgnCoreL10N::Undone());
        undoStr->assign(GetTxnDescription(txnRange.GetFirst()) + fmtString);
        }

    m_curr = txnRange.GetFirst(); // we reuse txnids

    // save in undone txn log
    m_reversedTxn.push_back(txnRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/04
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TxnManager::ReverseTo(TxnId pos)
    {
    if (!PrepareForUndo() || !pos.IsValid())
        return DgnDbStatus::NothingToUndo;

    TxnId last = GetCurrentTxnId();
    if (pos >= last)
        return DgnDbStatus::NothingToUndo;

    TxnRange range(pos, last);
    return ReverseActions(range, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/04
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TxnManager::CancelTo(TxnId pos)
    {
    DgnDbStatus status = ReverseTo(pos);
    if (DgnDbStatus::Success == status)
        DeleteReversedTxns();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TxnManager::ReverseActions(TxnRange& txnRange, bool showMsg)
    {
    Utf8String undoStr;
    ReverseTxnRange(txnRange, &undoStr);     // do the actual undo now.

    while (GetCurrentTxnId() < GetMultiTxnOperationStart())
        EndMultiTxnOperation();

    T_HOST.GetTxnAdmin()._OnUndoRedo(*this, TxnAction::Reverse);

    if (showMsg)
        NotificationManager::OutputMessage(NotifyMessageDetails(OutputMessagePriority::Info, undoStr.c_str()));

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool TxnManager::PrepareForUndo()
    {
    if (IsUndoPossible())
        {
        T_HOST.GetTxnAdmin()._OnPrepareForUndoRedo();
        return true;
        }

    T_HOST.GetTxnAdmin()._OnNothingToUndo();
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   02/04
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TxnManager::ReverseTxns(int numActions, AllowPreviousSessions allowPrevious)
    {
    if (!PrepareForUndo())
        return DgnDbStatus::NothingToUndo;

    TxnId last = GetCurrentTxnId();
    TxnId first = last;

    while (numActions > 0)
        {
        TxnId prev = QueryPreviousTxnId(first);
        if (!prev.IsValid())
            break;

        if (!IsMultiTxnMember(prev))
            --numActions;

        first = prev;
        }

    if ((allowPrevious != AllowPreviousSessions::Yes) && first<GetSessionStartId())
        first = GetSessionStartId();

    if (first == last)
        return DgnDbStatus::NothingToUndo;

    TxnRange range(first, last);
    return ReverseActions(range, true);
    }

/*---------------------------------------------------------------------------------**//**
* reverse (undo) all previous transactions
* @bsimethod                                                    Keith.Bentley   02/04
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TxnManager::ReverseAll(bool prompt)
    {
    if (!PrepareForUndo())
        return DgnDbStatus::NothingToUndo;

    if (prompt && !T_HOST.GetTxnAdmin()._OnPromptReverseAll())
        {
        T_HOST.GetTxnAdmin()._RestartTool();
        return DgnDbStatus::NothingToUndo;
        }

    TxnRange range(GetSessionStartId(), GetCurrentTxnId());
    return ReverseActions(range, true);
    }

/*---------------------------------------------------------------------------------**//**
* Reinstate ("redo") a range of transactions. Also returns the string that identifies what was reinstated.
* @bsimethod                                                    Keith.Bentley   02/04
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::ReinstateTxn(TxnRange& revTxn, Utf8StringP redoStr)
    {
    BeAssert(m_curr == revTxn.GetFirst());

    if (HasChanges())
        m_dgndb.AbandonChanges();

    TxnId last  = QueryPreviousTxnId(revTxn.GetLast());
    for (TxnId curr=revTxn.GetFirst(); curr.IsValid() && curr <= last; curr=QueryNextTxnId(curr))
        ApplyChanges(curr, TxnAction::Reinstate);

    m_dgndb.SaveChanges(); // make sure we save the updated Txn data to disk.

    if (redoStr)
        {
        Utf8String fmtString = DgnCoreL10N::GetString(DgnCoreL10N::Redone());
        redoStr->assign(GetTxnDescription(revTxn.GetFirst()) + fmtString);
        }

    m_curr = revTxn.GetLast();
    m_reversedTxn.pop_back();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TxnManager::ReinstateActions(TxnRange& revTxn)
    {
    Utf8String redoStr;
    ReinstateTxn(revTxn, &redoStr);     // do the actual redo now.

    T_HOST.GetTxnAdmin()._OnUndoRedo(*this, TxnAction::Reinstate);

    NotificationManager::OutputMessage(NotifyMessageDetails(OutputMessagePriority::Info, redoStr.c_str()));
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TxnManager::ReinstateTxn()
    {
    if (!IsRedoPossible())
        {
        T_HOST.GetTxnAdmin()._OnNothingToRedo();
        return DgnDbStatus::NothingToRedo;
        }

    T_HOST.GetTxnAdmin()._OnPrepareForUndoRedo();
    TxnRange*  revTxn = &m_reversedTxn.back();
    return  ReinstateActions(*revTxn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TxnManager::GetUndoString()
    {
    if (!IsUndoPossible())
        return "";

    return GetTxnDescription(QueryPreviousTxnId(GetCurrentTxnId()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TxnManager::GetRedoString()
    {
    if (!IsRedoPossible())
        return "";

    TxnRange*  revTxn = &m_reversedTxn.back();
    return GetTxnDescription(revTxn->GetFirst());
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
void dgn_TxnTable::Element::_OnValidate()
    {
    m_changes = false;
    if (m_stmt.IsPrepared())
        return;

    m_txnMgr.GetDgnDb().CreateTable(TEMP_TABLE(TXN_TABLE_Elements), "ElementId INTEGER NOT NULL PRIMARY KEY,ModelId INTEGER NOT NULL,ChangeType INT");
    m_txnMgr.GetDgnDb().ExecuteSql("CREATE INDEX " TEMP_TABLE(TXN_TABLE_Elements) "_Midx ON " TXN_TABLE_Elements "(ModelId)");
    m_stmt.Prepare(m_txnMgr.GetDgnDb(), "INSERT INTO " TEMP_TABLE(TXN_TABLE_Elements) "(ElementId,ModelId,ChangeType) VALUES(?,?,?)");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Element::_OnValidated()
    {
    // for cancel, the temp table is automatically rolled back, so we don't (can't actually, because there's no Txn active) need to empty it.
    if (m_changes)
        m_txnMgr.GetDgnDb().ExecuteSql("DELETE FROM " TEMP_TABLE(TXN_TABLE_Elements));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Model::_OnValidate()
    {
    m_changes = false;
    if (m_stmt.IsPrepared())
        return;

    m_txnMgr.GetDgnDb().CreateTable(TEMP_TABLE(TXN_TABLE_Models), "ModelId INTEGER NOT NULL PRIMARY KEY,ChangeType INT");
    m_stmt.Prepare(m_txnMgr.GetDgnDb(), "INSERT INTO " TEMP_TABLE(TXN_TABLE_Models) "(ModelId,ChangeType) VALUES(?,?)");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Model::_OnValidated()
    {
    // for cancel, the temp table is automatically rolled back, so we don't (can't actually, because there's no Txn active) need to empty it.
    if (m_changes)
        m_txnMgr.GetDgnDb().ExecuteSql("DELETE FROM " TEMP_TABLE(TXN_TABLE_Models));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2015
//---------------------------------------------------------------------------------------
void dgn_TxnTable::Model::AddChange(Changes::Change const& change, ChangeType changeType)
    {
    Changes::Change::Stage stage;
    switch (changeType)
        {
        case ChangeType::Insert:
            stage = Changes::Change::Stage::New;
            break;

        case ChangeType::Update:
        case ChangeType::Delete:
            stage = Changes::Change::Stage::Old;
            break;
        default:
            BeAssert(false);
            return;
        }

    DgnModelId modelId = DgnModelId(change.GetValue(0, stage).GetValueInt64());
    BeAssert(modelId.IsValid());
    
    enum Column : int {ModelId=1,ChangeType=2};

    m_changes = true;
    m_stmt.BindId(Column::ModelId, modelId);
    m_stmt.BindInt(Column::ChangeType, (int) changeType);

    auto rc = m_stmt.Step();
    BeAssert(rc==BE_SQLITE_DONE);
    UNUSED_VARIABLE(rc);

    m_stmt.Reset();
    m_stmt.ClearBindings();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Model::_OnReversedAdd(BeSQLite::Changes::Change const& change)
    {
    DgnModelId modelId = change.GetOldValue(0).GetValueId<DgnModelId>();
    DgnModelPtr model = m_txnMgr.GetDgnDb().Models().FindModel(modelId);
    if (!model.IsValid())
        return;

    m_txnMgr.GetDgnDb().Models().DropLoadedModel(*model);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Model::_OnReversedUpdate(BeSQLite::Changes::Change const& change)
    {
    DgnModelId modelId = change.GetOldValue(0).GetValueId<DgnModelId>();
    DgnModelPtr model = m_txnMgr.GetDgnDb().Models().FindModel(modelId);
    if (!model.IsValid())
        return;

    model->ReadProperties();
    model->_OnUpdated();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::BeProperties::_OnReversedUpdate(BeSQLite::Changes::Change const& change)
    {
    switch (m_txnMgr.GetCurrentAction())
        {
        case TxnAction::Abandon: // only these two actions need to preserve the changed state.
        case TxnAction::Reverse:
            change.OnPropertyUpdateReversed(m_txnMgr.GetDgnDb());
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::ElementDep::_OnValidate()
    {
    m_changes = false;
    if (m_stmt.IsPrepared())
        return;

    m_txnMgr.GetDgnDb().CreateTable(TEMP_TABLE(TXN_TABLE_Depend), "ECInstanceId INTEGER NOT NULL PRIMARY KEY,ModelId INTEGER NOT NULL,ChangeType INT");
    m_txnMgr.GetDgnDb().ExecuteSql("CREATE INDEX " TEMP_TABLE(TXN_TABLE_Depend) "_Midx ON " TXN_TABLE_Depend "(ModelId)");
    m_stmt.Prepare(m_txnMgr.GetDgnDb(), "INSERT INTO " TEMP_TABLE(TXN_TABLE_Depend) " (ECInstanceId,ModelId,ChangeType) VALUES(?,?,?)");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::ElementDep::_OnValidated()
    {
    // for cancel, the temp table is automatically rolled back, so we don't (can't actually, because there's no Txn active) need to empty it.
    if (m_changes && TxnAction::Abandon != m_txnMgr.GetCurrentAction())
        m_txnMgr.GetDgnDb().ExecuteSql("DELETE FROM " TEMP_TABLE(TXN_TABLE_Depend));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::ElementDep::UpdateSummary(Changes::Change change, ChangeType changeType)
    {
    m_changes = true;
    Changes::Change::Stage stage = (ChangeType::Insert == changeType) ? Changes::Change::Stage::New : Changes::Change::Stage::Old;
    ECInstanceId instanceId(change.GetValue(0, stage).GetValueInt64()); // primary key is column 0
    AddDependency(instanceId, changeType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::ModelDep::_OnValidateAdd(Changes::Change const& change)
    {
    SetChanges();
    CheckDirection(change.GetNewValue(0).GetValueId<EC::ECInstanceId>());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::ModelDep::_OnValidateUpdate(Changes::Change const& change)
    {
    SetChanges();
    CheckDirection(change.GetOldValue(0).GetValueId<EC::ECInstanceId>());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Element::AddElement(DgnElementId elementId, DgnModelId modelId, ChangeType changeType)
    {
    enum Column : int {ElementId=1,ModelId=2,ChangeType=3,LastMod=4};

    BeAssert(modelId.IsValid());
    BeAssert(elementId.IsValid());

    m_changes = true;
    m_stmt.BindId(Column::ElementId, elementId);
    m_stmt.BindId(Column::ModelId, modelId);
    m_stmt.BindInt(Column::ChangeType, (int) changeType);

    auto rc = m_stmt.Step();
    BeAssert(rc==BE_SQLITE_DONE);
    UNUSED_VARIABLE(rc);

    m_stmt.Reset();
    m_stmt.ClearBindings();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2015
//---------------------------------------------------------------------------------------
void dgn_TxnTable::Element::AddChange(Changes::Change const& change, ChangeType changeType)
    {
    Changes::Change::Stage stage;
    switch (changeType)
        {
        case ChangeType::Insert:
            stage = Changes::Change::Stage::New;
            break;

        case ChangeType::Update:
        case ChangeType::Delete:
            stage = Changes::Change::Stage::Old;
            break;
        default:
            BeAssert(false);
            return;
        }

    DgnElementId elementId = DgnElementId(change.GetValue(0, stage).GetValueInt64());
    DgnModelId modelId;

    if (ChangeType::Update == changeType)
        {
        // for updates, the element table must be queried for ModelId since the change set will only contain changed columns
        modelId = m_txnMgr.GetDgnDb().Elements().QueryModelId(elementId);
        }
    else
        modelId = DgnModelId(change.GetValue(2, stage).GetValueInt64());   // assumes DgnModelId is column 2

    AddElement(elementId, modelId, changeType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
dgn_TxnTable::Element::Iterator::Entry dgn_TxnTable::Element::Iterator::begin() const
    {
    if (!m_stmt.IsValid())
        m_db->GetCachedStatement(m_stmt, "SELECT ElementId,ModelId,ChangeType FROM " TEMP_TABLE(TXN_TABLE_Elements));
    else
        m_stmt->Reset();

    return Entry(m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

DgnElementId dgn_TxnTable::Element::Iterator::Entry::GetElementId() const {return m_sql->GetValueId<DgnElementId>(0);}
DgnModelId dgn_TxnTable::Element::Iterator::Entry::GetModelId() const {return m_sql->GetValueId<DgnModelId>(1);}
TxnTable::ChangeType dgn_TxnTable::Element::Iterator::Entry::GetChangeType() const {return (TxnTable::ChangeType) m_sql->GetValueInt(2);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::ElementDep::AddDependency(EC::ECInstanceId const& relid, ChangeType changeType)
    {
    CachedECSqlStatementPtr stmt  = m_txnMgr.GetDgnDb().GetPreparedECSqlStatement(
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
void dgn_TxnTable::ModelDep::CheckDirection(ECInstanceId relid)
    {
    CachedStatementPtr stmt = m_txnMgr.GetTxnStatement("SELECT RootModelId,DependentModelId FROM " DGN_TABLE(DGN_RELNAME_ModelDrivesModel) " WHERE(ECInstanceId=?)");
    stmt->BindId(1, relid);
    if (stmt->Step() != BE_SQLITE_ROW)
        {
        BeAssert(false); // model was just added or modified -- it has to exist!
        return;
        }

    DgnModelId rootModel = stmt->GetValueId<DgnModelId>(0);
    DgnModelId depModel  = stmt->GetValueId<DgnModelId>(1);

    DgnModels::Model root, dep;
    m_txnMgr.GetDgnDb().Models().QueryModelById(&root, rootModel);
    m_txnMgr.GetDgnDb().Models().QueryModelById(&dep, depModel);

    if (root.GetModelType() > dep.GetModelType())
        {
        //  A Physical model cannot depend on a Drawing model
        m_txnMgr.ReportError(*new DgnElementDependencyGraph::DirectionValidationError(""));
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
struct TxnReversedCaller
{
    TxnManagerR m_mgr;
    TxnReversedCaller(TxnManagerR summary) : m_mgr(summary) {}
    void operator()(TxnMonitorR monitor) const {monitor._OnReversedChanges(m_mgr);}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   07/13
//=======================================================================================
struct TxnCommitCaller
{
    TxnManagerR m_mgr;
    TxnCommitCaller(TxnManagerR mgr) : m_mgr(mgr) {}
    void operator()(TxnMonitorR monitor) const {monitor._OnCommit(m_mgr);}
};


//=======================================================================================
// @bsiclass                                                    Keith.Bentley   07/13
//=======================================================================================
struct UndoRedoFinishedCaller
    {
    TxnManagerR m_mgr;
    TxnAction m_action;
    UndoRedoFinishedCaller(TxnManager& mgr, TxnAction action) : m_mgr(mgr), m_action(action) {}
    void operator()(TxnMonitorR handler) const {handler._OnUndoRedo(m_mgr, m_action);}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::TxnAdmin::_OnCommit(TxnManagerR mgr)
    {
    CallMonitors(TxnCommitCaller(mgr));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::TxnAdmin::_OnReversedChanges(TxnManagerR summary)
    {
    CallMonitors(TxnReversedCaller(summary));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::TxnAdmin::_OnUndoRedo(TxnManager& mgr, TxnAction action)
    {
    CallMonitors(UndoRedoFinishedCaller(mgr, action));
    }
